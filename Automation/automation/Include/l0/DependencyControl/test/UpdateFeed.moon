-- UpdateFeed tests: feed data access, script record retrieval, file deployment,
-- and feed refresh.
-- Called from test.moon as: (controls\requireTest "UpdateFeed") basePath, DepCtrl, stubHelpers
(basePath, DepCtrl, stubHelpers) ->
  domain = require "l0.DependencyControl.domain"
  fileOps = require "l0.DependencyControl.file-ops"
  FileCache = require "l0.DependencyControl.FileCache"
  UpdateFeed = require "l0.DependencyControl.UpdateFeed"
  dkjson = require "l0.dkjson"
  {:stubSelf} = stubHelpers
  FILEOPS_MODULE_NAME = "l0.DependencyControl.file-ops"

  -- Builds a stub feed around unexpanded data for driving expand directly.
  makeExpandFeed = (unexpandedData, feedDir = basePath) ->
    unexpandedData.macros or= {}
    unexpandedData.modules or= {}
    unexpandedData.knownFeeds or= {}
    stubSelf UpdateFeed, {
      _url: "https://example.com/f.json", :unexpandedData, :feedDir,
      fileName: fileOps.joinPath(feedDir, "feed.json"),
      __class: UpdateFeed, logger: DepCtrl.logger
    }

  normalizePath = (path) -> path\gsub "[/\\]", "/"

  -- A small feed whose file-URL template resolves on any channel (no tagSuffix), for resolve() tests. Its
  -- changelog holds a literal @{fileName} the templater never expands, to prove prose keeps its markers.
  resolveFeedJson = [[{
    "dependencyControlFeedFormatVersion": "0.4.0",
    "name": "T",
    "baseUrl": "https://example.com/proj",
    "url": "@{baseUrl}",
    "fileBaseUrl": "https://x.test/",
    "fileBaseUrls": {"script": "@{fileBaseUrl}v@{version}/@{scriptTypeSection}/@{namespacePath}@{fileName}"},
    "localFileBasePaths": {"script": "@{localFileBasePath}@{scriptTypeSection}/@{namespacePath}@{fileName}"},
    "vars": {},
    "modules": {"l0.Thing": {
      "name": "Thing", "author": "a", "url": "@{baseUrl}#@{namespace}",
      "channels": {"release": {"version": "1.2.3", "default": true, "files": [
        {"name": ".moon", "url": "@{fileBaseUrl}", "sha1": "1111111111111111111111111111111111111111"},
        {"name": "/Sub.moon", "url": "@{fileBaseUrl}", "sha1": "2222222222222222222222222222222222222222"}
      ]}},
      "changelog": {"1.2.3": ["Documented the @{fileName} template variable."]}
    }}
  }]]

  writeResolveFeed = (dirName, json = resolveFeedJson) ->
    root = fileOps.joinPath basePath, dirName
    fileOps.mkdir root, false, true
    feedPath = fileOps.joinPath root, "feed.json"
    fileOps.writeFile feedPath, json, true
    feedPath, root

  {
    _description: "Tests for UpdateFeed feed data access, script record retrieval, and file deployment."

    knownFeeds_noData: (ut) ->
      feed = stubSelf UpdateFeed, {data: nil}
      ut\assertTable feed.knownFeeds
      ut\assertEquals #feed.knownFeeds, 0

    knownFeeds_withData: (ut) ->
      feed = stubSelf UpdateFeed, {data: {knownFeeds: {a: "https://example.com/a.json", b: "https://example.com/b.json"}}}
      ut\assertEquals #feed.knownFeeds, 2

    getScript_invalidType: (ut) ->
      feed = {data: {macros: {}, modules: {}, knownFeeds: {}}, logger: DepCtrl.logger, __class: UpdateFeed}
      result, err = UpdateFeed.getScript feed, "test.NS", 99
      ut\assertNil result
      ut\assertString err

    getScript_missing: (ut) ->
      feed = {data: {macros: {}, modules: {}, knownFeeds: {}}, logger: DepCtrl.logger, __class: UpdateFeed}
      result = UpdateFeed.getScript feed, "test.NS", domain.ScriptType.Module
      ut\assertFalse result

    getScript_found: (ut) ->
      feed = {
        data: {modules: {"test.NS": {
          channels: {release: {default: true, version: "1.0.0", files: {}}},
          name: "T"
        }}, macros: {}, knownFeeds: {}},
        logger: DepCtrl.logger, __class: UpdateFeed
      }
      sur = UpdateFeed.getScript feed, "test.NS", domain.ScriptType.Module
      ut\assertTable sur
      ut\assertEquals sur.namespace, "test.NS"
      ut\assertEquals sur.activeChannel, "release"

    getMacro_usesAutomationType: (ut) ->
      -- getMacro calls @getScript, which requires self.getScript to resolve via colon call.
      -- Adding getScript directly to the mock avoids needing a full class metatable.
      feed = {
        data: {macros: {"test.NS": {
          channels: {release: {default: true, version: "1.0.0", files: {}}},
          name: "T"
        }}, modules: {}, knownFeeds: {}},
        logger: DepCtrl.logger, __class: UpdateFeed,
        getScript: UpdateFeed.getScript
      }
      sur = UpdateFeed.getMacro feed, "test.NS"
      ut\assertTable sur
      ut\assertFalse sur.moduleName -- false for Automation (not a module)

    getModule_usesModuleType: (ut) ->
      feed = {
        data: {modules: {"test.NS": {
          channels: {release: {default: true, version: "1.0.0", files: {}}},
          name: "T"
        }}, macros: {}, knownFeeds: {}},
        logger: DepCtrl.logger, __class: UpdateFeed,
        getScript: UpdateFeed.getScript
      }
      sur = UpdateFeed.getModule feed, "test.NS"
      ut\assertTable sur
      ut\assertEquals sur.moduleName, "test.NS" -- set for Module type

    -- getModuleVersion

    getModuleVersion_defaultChannel: (ut) ->
      feed = {
        data: {modules: {"test.NS": {channels: {
          release: {default: true, version: "1.2.3", files: {}}
          nightly: {version: "2.0.0", files: {}}
        }}}},
        __class: UpdateFeed
      }
      ut\assertEquals UpdateFeed.getModuleVersion(feed, "test.NS"), "1.2.3"

    getModuleVersion_fallback: (ut) ->
      feed = {
        data: {modules: {"test.NS": {channels: {alpha: {version: "2.0.0", files: {}}}}}},
        __class: UpdateFeed
      }
      ut\assertEquals UpdateFeed.getModuleVersion(feed, "test.NS"), "2.0.0"

    getModuleVersion_missing: (ut) ->
      feed = {data: {modules: {}}, __class: UpdateFeed}
      ut\assertNil UpdateFeed.getModuleVersion feed, "no.Such.NS"

    -- getVersionsOnChannel / getHighestVersionOnChannel / getPackageVersionOnChannel: channel version queries

    getVersionsOnChannel_distinctAscendingBySemver: (ut) ->
      feed = stubSelf UpdateFeed, {__class: UpdateFeed, rawFeedData: {
        macros: {"a.M": {channels: {stable: {version: "0.7.0"}}}}
        modules: {
          "l0.A": {channels: {stable: {version: "0.7.0"}}}   -- duplicate of the macro's version
          "l0.B": {channels: {stable: {version: "0.7.10"}}}
          "l0.C": {channels: {stable: {version: "0.7.9"}}}
        }
      }}
      versions = feed\getVersionsOnChannel "stable"
      ut\assertItemsEqual versions, {"0.7.0", "0.7.9", "0.7.10"}
      -- ordered by semantic version, so 0.7.10 sorts after 0.7.9 rather than lexically before it
      ut\assertEquals versions[#versions], "0.7.10"

    getVersionsOnChannel_absentChannelIsEmpty: (ut) ->
      feed = stubSelf UpdateFeed, {__class: UpdateFeed, rawFeedData: {
        modules: {"l0.A": {channels: {stable: {version: "1.0.0"}}}}
      }}
      ut\assertEquals #feed\getVersionsOnChannel("nightly"), 0

    getHighestVersionOnChannel_returnsHighest: (ut) ->
      feed = stubSelf UpdateFeed, {__class: UpdateFeed, rawFeedData: {modules: {
        "l0.A": {channels: {stable: {version: "0.7.0"}}}
        "l0.B": {channels: {stable: {version: "0.7.1"}}}
      }}}
      ut\assertEquals feed\getHighestVersionOnChannel("stable"), "0.7.1"

    getHighestVersionOnChannel_defaultsToDefaultChannel: (ut) ->
      feed = stubSelf UpdateFeed, {__class: UpdateFeed, rawFeedData: {modules: {
        "l0.A": {channels: {main: {default: true, version: "0.7.0"}}}
        "l0.B": {channels: {main: {default: true, version: "0.7.1"}}}
      }}}
      ut\assertEquals feed\getHighestVersionOnChannel!, "0.7.1"

    getPackageVersionOnChannel_found: (ut) ->
      feed = stubSelf UpdateFeed, {__class: UpdateFeed, rawFeedData: {modules: {
        "l0.A": {channels: {stable: {version: "0.7.0"}, alpha: {version: "0.8.0"}}}
      }}}
      ut\assertEquals feed\getPackageVersionOnChannel("l0.A", "alpha"), "0.8.0"

    getPackageVersionOnChannel_missingPackageOrChannel: (ut) ->
      feed = stubSelf UpdateFeed, {__class: UpdateFeed, rawFeedData: {modules: {
        "l0.A": {channels: {stable: {version: "0.7.0"}}}
      }}}
      ut\assertNil feed\getPackageVersionOnChannel("l0.Nope", "stable")
      ut\assertNil feed\getPackageVersionOnChannel("l0.A", "nightly")

    -- getProviders: virtual-package resolution lookup over the feed's module section

    getProviders_findsByBareAlias: (ut) ->
      feed = {
        data: {modules: {
          "l0.dkjson": {name: "dkjson", channels: {release: {default: true, version: "2.10.0", files: {}, provides: {"json", "dkjson"}}}}
          "l0.Other": {name: "Other", channels: {release: {default: true, version: "1.0.0", files: {}}}}
        }, macros: {}},
        logger: DepCtrl.logger, __class: UpdateFeed
      }
      providers = UpdateFeed.getProviders feed, "json", {}
      ut\assertEquals #providers, 1
      ut\assertEquals providers[1].namespace, "l0.dkjson"
      ut\assertEquals providers[1].version, "2.10.0"

    -- given DependencyControl's modules config, a provider's candidacy is judged on the channel recorded
    -- for it, so the provides list and version come from the channel an install would actually use
    getProviders_honorsRecordedChannel: (ut) ->
      feed = {
        data: {modules: {
          "l0.dkjson": {name: "dkjson", channels: {
            release: {default: true, version: "2.10.0", files: {}}
            legacy: {version: "2.5.0", files: {}, provides: {"json"}}
          }}
        }, macros: {}},
        logger: DepCtrl.logger, __class: UpdateFeed
      }
      ut\assertEquals #(UpdateFeed.getProviders feed, "json", {}), 0 -- the default channel doesn't provide it
      providers = UpdateFeed.getProviders feed, "json", {"l0.dkjson": {lastChannel: "legacy"}}
      ut\assertEquals #providers, 1
      ut\assertEquals providers[1].version, "2.5.0"

    getProviders_objectAliasEntries: (ut) ->
      feed = {
        data: {modules: {
          "l0.prov": {name: "P", channels: {release: {default: true, version: "1.2.3", files: {}, provides: {{name: "yaml"}}}}}
        }, macros: {}},
        logger: DepCtrl.logger, __class: UpdateFeed
      }
      providers = UpdateFeed.getProviders feed, "yaml", {}
      ut\assertEquals #providers, 1
      ut\assertEquals providers[1].namespace, "l0.prov"

    getProviders_noMatchReturnsEmpty: (ut) ->
      feed = {
        data: {modules: {
          "l0.dkjson": {name: "dkjson", channels: {release: {default: true, version: "2.10.0", files: {}, provides: {"json"}}}}
        }, macros: {}},
        logger: DepCtrl.logger, __class: UpdateFeed
      }
      ut\assertEquals #UpdateFeed.getProviders(feed, "xml", {}), 0

    getProviders_ignoresModulesWithoutProvides: (ut) ->
      feed = {
        data: {modules: {
          "l0.A": {name: "A", channels: {release: {default: true, version: "1.0.0", files: {}}}}
        }, macros: {}},
        logger: DepCtrl.logger, __class: UpdateFeed
      }
      ut\assertEquals #UpdateFeed.getProviders(feed, "json", {}), 0

    getProviders_noModulesSection: (ut) ->
      feed = {data: {macros: {}}, logger: DepCtrl.logger, __class: UpdateFeed}
      ut\assertEquals #UpdateFeed.getProviders(feed, "json", {}), 0

    -- __normalizeModuleAliases: expands bare strings to ModuleAlias tables and preserves table fields

    normalizeModuleAliases_bareStringsToTables: (ut) ->
      result = UpdateFeed\__normalizeModuleAliases {"json", "dkjson"}
      ut\assertEquals #result, 2
      ut\assertEquals result[1].name, "json"
      ut\assertEquals result[2].name, "dkjson"

    normalizeModuleAliases_preservesFields: (ut) ->
      input = {{name: "json", version: "1.2.0"}}
      result = UpdateFeed\__normalizeModuleAliases input
      ut\assertEquals result[1].name, "json"
      ut\assertEquals result[1].version, "1.2.0"
      sameRef = result[1] == input[1]
      ut\assertFalse sameRef -- copied, not the caller's table

    normalizeModuleAliases_dropsNonSchemaFields: (ut) ->
      result = UpdateFeed\__normalizeModuleAliases {{name: "json", version: "1.0.0", optional: true, bogus: "x"}}
      ut\assertEquals result[1].name, "json"
      ut\assertEquals result[1].version, "1.0.0"
      ut\assertNil result[1].optional
      ut\assertNil result[1].bogus

    normalizeModuleAliases_nilAndEmpty: (ut) ->
      ut\assertEquals #UpdateFeed\__normalizeModuleAliases(nil), 0
      ut\assertEquals #UpdateFeed\__normalizeModuleAliases({}), 0

    -- getFileDeployPath

    getFileDeployPath_module: (ut) ->
      (ut\stub aegisub, "decode_path")\calls (path) -> path\gsub("^%?user", basePath)
      result = UpdateFeed.getFileDeployPath UpdateFeed, "l0.NS", domain.ScriptType.Module, "/NS.moon", "script", "?user"
      ut\assertString result
      ut\assertContains result, "NS.moon"
      ut\assertContains result, "l0"

    getFileDeployPath_test: (ut) ->
      (ut\stub aegisub, "decode_path")\calls (path) -> path\gsub("^%?user", basePath)
      result = UpdateFeed.getFileDeployPath UpdateFeed, "l0.NS", domain.ScriptType.Module, "/NS.moon", "test", "?user"
      ut\assertString result
      ut\assertContains result, "DepUnit"

    -- expand rebuilds @data from the unexpanded data each call and never mutates that (possibly shared) source
    expand_rebuildsFromUnexpandedDataWithoutMutatingIt: (ut) ->
      unexpandedData = {name: "TestFeed", description: "made for @{feedName}", macros: {}, modules: {}, knownFeeds: {}}
      feed = stubSelf UpdateFeed, {
        _url: "https://example.com/f.json", :unexpandedData, __class: UpdateFeed, logger: DepCtrl.logger
      }

      data = UpdateFeed.expand feed
      ut\assertEquals data.description, "made for TestFeed" -- template expanded in the working copy
      ut\assertEquals unexpandedData.description, "made for @{feedName}" -- pristine source left untouched
      ut\assertFalse data == unexpandedData -- @data is a fresh copy, not the source

      again = UpdateFeed.expand feed -- a second expand rebuilds from the source
      ut\assertFalse again == data -- a new working copy each call
      ut\assertEquals unexpandedData.description, "made for @{feedName}" -- source still pristine

    -- a fileBaseUrls map collapses to the entry matching each file's type, with @{fileName} baked in
    expand_fileBaseUrlsCollapsePerType: (ut) ->
      feed = makeExpandFeed {
        name: "F"
        fileBaseUrl: "https://x.test/"
        fileBaseUrls: {
          script: "@{fileBaseUrl}v@{version}/@{scriptTypeSection}/@{namespacePath}@{fileName}"
          test: "@{fileBaseUrl}v@{version}/@{scriptTypeSection}/@{namespacePath}/test@{fileName}"
        }
        modules: {
          "l0.NS": {name: "NS", channels: {release: {version: "1.2.3", files: {
            {name: ".moon", url: "@{fileBaseUrl}"}
            {name: "/Sub.moon", url: "@{fileBaseUrl}"}
            {name: ".moon", url: "@{fileBaseUrl}", type: "test"}
          }}}}
        }
      }
      files = UpdateFeed.expand(feed).modules["l0.NS"].channels.release.files
      ut\assertEquals files[1].url, "https://x.test/v1.2.3/modules/l0/NS.moon"
      ut\assertEquals files[2].url, "https://x.test/v1.2.3/modules/l0/NS/Sub.moon"
      ut\assertEquals files[3].url, "https://x.test/v1.2.3/modules/l0/NS/test.moon"

    -- a file type without a fileBaseUrls entry keeps the scalar fileBaseUrl as its base
    expand_fileBaseUrlsFallbackToScalar: (ut) ->
      feed = makeExpandFeed {
        name: "F"
        fileBaseUrl: "https://x.test/raw/"
        fileBaseUrls: {
          test: "@{fileBaseUrl}@{namespacePath}/test@{fileName}"
        }
        modules: {
          "l0.NS": {name: "NS", channels: {release: {version: "1.0.0", files: {
            {name: ".moon", url: "@{fileBaseUrl}@{fileName}"}
            {name: ".moon", url: "@{fileBaseUrl}", type: "test"}
          }}}}
        }
      }
      files = UpdateFeed.expand(feed).modules["l0.NS"].channels.release.files
      ut\assertEquals files[1].url, "https://x.test/raw/.moon"
      ut\assertEquals files[2].url, "https://x.test/raw/l0/NS/test.moon"

    -- a rolling map set on a section container applies to that section only
    expand_sectionScopedOverride: (ut) ->
      feed = makeExpandFeed {
        name: "F"
        fileBaseUrl: "https://x.test/"
        fileBaseUrls: {
          script: "@{fileBaseUrl}@{scriptTypeSection}/@{namespacePath}@{fileName}"
        }
        macros: {
          fileBaseUrls: {
            script: "@{fileBaseUrl}@{scriptTypeSection}/@{namespace}@{fileName}"
          }
          "l0.Macro.NS": {name: "M", channels: {release: {version: "1.0.0", files: {
            {name: ".moon", url: "@{fileBaseUrl}"}
          }}}}
        }
        modules: {
          "l0.NS": {name: "NS", channels: {release: {version: "1.0.0", files: {
            {name: ".moon", url: "@{fileBaseUrl}"}
          }}}}
        }
      }
      data = UpdateFeed.expand feed
      ut\assertEquals data.macros["l0.Macro.NS"].channels.release.files[1].url, "https://x.test/macros/l0.Macro.NS.moon"
      ut\assertEquals data.modules["l0.NS"].channels.release.files[1].url, "https://x.test/modules/l0/NS.moon"

    expand_scriptTypeVariables: (ut) ->
      feed = makeExpandFeed {
        name: "F"
        macros: {
          "l0.Macro.NS": {name: "M", url: "https://x.test/@{scriptType}/@{scriptTypeSection}", channels: {release: {version: "1.0.0", files: {}}}}
        }
        modules: {
          "l0.NS": {name: "NS", url: "https://x.test/@{scriptType}/@{scriptTypeSection}", channels: {release: {version: "1.0.0", files: {}}}}
        }
      }
      data = UpdateFeed.expand feed
      ut\assertEquals data.macros["l0.Macro.NS"].url, "https://x.test/#{domain.ScriptType.Automation}/macros"
      ut\assertEquals data.modules["l0.NS"].url, "https://x.test/#{domain.ScriptType.Module}/modules"

    -- vars entries become variables; a table-valued one serves @{name:key} lookups whose key
    -- part may itself be a variable that only comes into scope at channel depth
    expand_authorVarsAndComputedKeys: (ut) ->
      feed = makeExpandFeed {
        name: "F"
        vars: {
          host: "https://cdn.test"
          tagSuffix: {alpha: "-alpha", release: ""}
        }
        fileBaseUrl: "@{host}/dl/"
        modules: {
          "l0.NS": {name: "NS", channels: {
            alpha: {version: "1.1.0", files: {
              {name: ".moon", url: "@{fileBaseUrl}v@{version}@{tagSuffix:@{channel}}@{fileName}"}
            }}
            release: {version: "1.0.0", files: {
              {name: ".moon", url: "@{fileBaseUrl}v@{version}@{tagSuffix:@{channel}}@{fileName}"}
            }}
          }}
        }
      }
      channels = UpdateFeed.expand(feed).modules["l0.NS"].channels
      ut\assertEquals channels.alpha.files[1].url, "https://cdn.test/dl/v1.1.0-alpha.moon"
      ut\assertEquals channels.release.files[1].url, "https://cdn.test/dl/v1.0.0.moon"

    -- a collapsed localFileBasePaths entry is the complete path; an unmapped type appends the
    -- file name to the scalar localFileBasePath (default "./")
    expand_localFileBasePathsResolveFullPaths: (ut) ->
      feed = makeExpandFeed {
        name: "F"
        localFileBasePaths: {
          script: "@{localFileBasePath}@{scriptTypeSection}/@{namespacePath}@{fileName}"
        }
        modules: {
          "l0.NS": {name: "NS", channels: {release: {version: "1.0.0", files: {
            {name: ".moon"}
            {name: "NS.moon", type: "test"}
          }}}}
        }
      }
      files = UpdateFeed.expand(feed, UpdateFeed.ExpansionMode.Local).modules["l0.NS"].channels.release.files
      ut\assertEquals normalizePath(files[1].localFilePath), normalizePath "#{basePath}/modules/l0/NS.moon"
      ut\assertEquals normalizePath(files[2].localFilePath), normalizePath "#{basePath}/NS.moon"

    -- a v0.3.0-style feed (scalar fileBaseUrl, explicit @{fileBaseUrl}@{fileName} urls) expands unchanged
    expand_legacyScalarBases: (ut) ->
      feed = makeExpandFeed {
        name: "F"
        fileBaseUrl: "https://x.test/@{channel}/@{namespace}"
        modules: {
          "l0.NS": {name: "NS", channels: {release: {version: "1.0.0", files: {
            {name: ".moon", url: "@{fileBaseUrl}@{fileName}"}
          }}}}
        }
      }
      data = UpdateFeed.expand feed
      ut\assertEquals data.modules["l0.NS"].channels.release.files[1].url, "https://x.test/release/l0.NS.moon"

    -- findUnlistedFiles inverts the per-type local path templates and reports on-disk files the
    -- channel doesn't list; a file matching several types goes to the longest-prefix template,
    -- and delete-flagged entries still count as listed
    findUnlistedFiles_discoversUnlistedFiles: (ut) ->
      root = fileOps.joinPath basePath, "discover1"
      fileOps.mkdir fileOps.joinPath(root, "modules", "l0", "NS", "test"), false, true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "NS.moon"), "-- main", true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "NS", "New.moon"), "-- new", true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "NS", "Del.moon"), "-- resurrected", true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "NS", "test.moon"), "-- main test", true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "NS", "test", "New.moon"), "-- new test", true
      feed = makeExpandFeed {
        name: "F"
        fileBaseUrl: "https://x.test/"
        fileBaseUrls: {script: "@{fileBaseUrl}@{namespacePath}@{fileName}"}
        localFileBasePaths: {
          script: "@{localFileBasePath}modules/@{namespacePath}@{fileName}"
          test: "@{localFileBasePath}modules/@{namespacePath}/test@{fileName}"
        }
        modules: {
          "l0.NS": {name: "NS", channels: {release: {version: "1.0.0", default: true, files: {
            {name: ".moon", url: "@{fileBaseUrl}"}
            {name: ".moon", url: "@{fileBaseUrl}", type: "test"}
            {name: "/Del.moon", url: "@{fileBaseUrl}", delete: true}
          }}}}
        }
      }, root
      UpdateFeed.expand feed, UpdateFeed.ExpansionMode.Local
      result = UpdateFeed.findUnlistedFiles feed
      ut\assertEquals #result, 2
      ut\assertEquals result[1].name, "/New.moon"
      ut\assertNil result[1].type
      ut\assertEquals result[1].url, "@{fileBaseUrl}" -- script type has a fileBaseUrls entry
      ut\assertEquals result[1].channel, "release"
      ut\assertContains result[1].localFilePath, "New.moon"
      ut\assertEquals result[2].name, "/New.moon"
      ut\assertEquals result[2].type, "test"
      ut\assertEquals result[2].url, "@{fileBaseUrl}@{fileName}" -- no fileBaseUrls entry for tests

    -- a local path template with an unexpanded variable besides @{fileName} can't be inverted
    findUnlistedFiles_skipsUninvertibleTemplates: (ut) ->
      root = fileOps.joinPath basePath, "discover2"
      fileOps.mkdir fileOps.joinPath(root, "src"), false, true
      fileOps.writeFile fileOps.joinPath(root, "src", "Stray.moon"), "-- stray", true
      feed = makeExpandFeed {
        name: "F"
        localFileBasePaths: {script: "@{localFileBasePath}src/@{undeclared}@{fileName}"}
        modules: {
          "l0.NS": {name: "NS", channels: {release: {version: "1.0.0", default: true, files: {}}}}
        }
      }, root
      UpdateFeed.expand feed, UpdateFeed.ExpansionMode.Local
      result = UpdateFeed.findUnlistedFiles feed
      ut\assertEquals #result, 0

    -- updateFeed with addFiles appends discovered files to the raw channel, hashed and typed
    updateFeed_addFilesAppendsEntries: (ut) ->
      root = fileOps.joinPath basePath, "discover3"
      fileOps.mkdir fileOps.joinPath(root, "modules", "l0", "NS"), false, true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "NS.moon"), "-- main", true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "NS", "New.moon"), "-- new", true
      feedPath = fileOps.joinPath root, "feed.json"
      fileOps.writeFile feedPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0",
        "name": "T",
        "fileBaseUrl": "https://x.test/",
        "fileBaseUrls": {"script": "@{fileBaseUrl}@{namespacePath}@{fileName}"},
        "localFileBasePaths": {"script": "@{localFileBasePath}modules/@{namespacePath}@{fileName}"},
        "modules": {"l0.NS": {"name": "NS", "author": "a", "channels": {"release": {"version": "1.0.0", "default": true,
          "files": [{"name": ".moon", "url": "@{fileBaseUrl}", "sha1": "0000000000000000000000000000000000000000"}]}}}}
      }]], true
      feed = UpdateFeed nil, false, feedPath
      (ut\stub feed, "__refreshVersionRecord")\returns false
      stats = feed\updateFeed {addFiles: true, outPath: false}
      ut\assertTable stats
      files = feed.rawFeedData.modules["l0.NS"].channels.release.files
      ut\assertEquals #files, 2
      ut\assertEquals files[2].name, "/New.moon"
      ut\assertEquals files[2].url, "@{fileBaseUrl}"
      ut\assertMatches files[2].sha1, "^%x+$"
      ut\assertEquals #files[2].sha1, 40
      result = stats.packages[1]
      ut\assertTrue result.changed
      ut\assertEquals result.addedFiles[1].name, "/New.moon"
      ut\assertEquals stats.changed, 1

    -- updateFeed markReleased stamps the release date on channels still unreleased and keeps existing dates
    updateFeed_markReleasedStampsUnreleased: (ut) ->
      root = fileOps.joinPath basePath, "markrel"
      fileOps.mkdir fileOps.joinPath(root, "modules", "l0"), false, true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "Fresh.moon"), "-- fresh", true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "Old.moon"), "-- old", true
      feedPath = fileOps.joinPath root, "feed.json"
      fileOps.writeFile feedPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0",
        "name": "T",
        "fileBaseUrl": "https://x.test/",
        "fileBaseUrls": {"script": "@{fileBaseUrl}@{namespacePath}@{fileName}"},
        "localFileBasePaths": {"script": "@{localFileBasePath}modules/@{namespacePath}@{fileName}"},
        "modules": {
          "l0.Fresh": {"name": "Fresh", "author": "a", "channels": {"release": {"version": "1.0.0", "default": true, "released": null,
            "files": [{"name": ".moon", "url": "@{fileBaseUrl}", "sha1": "0000000000000000000000000000000000000000"}]}}},
          "l0.Old": {"name": "Old", "author": "a", "channels": {"release": {"version": "1.0.0", "default": true, "released": "2020-01-01",
            "files": [{"name": ".moon", "url": "@{fileBaseUrl}", "sha1": "0000000000000000000000000000000000000000"}]}}}
        }
      }]], true
      feed = UpdateFeed nil, false, feedPath
      -- stub both refreshers so no content change resets `released`, isolating the markReleased stamping
      (ut\stub feed, "__refreshVersionRecord")\returns false
      (ut\stub feed, "__refreshFiles")\returns false, {}
      feed\updateFeed {markReleased: "2099-12-31", outPath: false}
      ut\assertEquals feed.rawFeedData.modules["l0.Fresh"].channels.release.released, "2099-12-31" -- stamped
      ut\assertEquals feed.rawFeedData.modules["l0.Old"].channels.release.released, "2020-01-01" -- kept

    -- updateFeed markReleased: a package whose default channel can't be resolved carries the reason in
    -- its result and gets no release stamp on any channel, rather than being passed over in silence
    updateFeed_markReleasedReportsAmbiguousDefault: (ut) ->
      root = fileOps.joinPath basePath, "markrelAmbiguous"
      fileOps.mkdir fileOps.joinPath(root, "modules", "l0"), false, true
      fileOps.writeFile fileOps.joinPath(root, "modules", "l0", "Pkg.moon"), "-- pkg", true
      feedPath = fileOps.joinPath root, "feed.json"
      fileOps.writeFile feedPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0",
        "name": "T",
        "fileBaseUrl": "https://x.test/",
        "fileBaseUrls": {"script": "@{fileBaseUrl}@{namespacePath}@{fileName}"},
        "localFileBasePaths": {"script": "@{localFileBasePath}modules/@{namespacePath}@{fileName}"},
        "modules": {
          "l0.Pkg": {"name": "Pkg", "author": "a", "channels": {
            "main": {"version": "1.0.0", "default": true, "released": null,
              "files": [{"name": ".moon", "url": "@{fileBaseUrl}", "sha1": "0000000000000000000000000000000000000000"}]},
            "stable": {"version": "1.0.0", "default": true, "released": null,
              "files": [{"name": ".moon", "url": "@{fileBaseUrl}", "sha1": "0000000000000000000000000000000000000000"}]}}}
        }
      }]], true
      feed = UpdateFeed nil, false, feedPath
      (ut\stub feed, "__refreshVersionRecord")\returns false
      (ut\stub feed, "__refreshFiles")\returns false, {}
      stats = feed\updateFeed {markReleased: "2099-12-31", outPath: false}
      ut\assertEquals stats.errored, 1
      ut\assertContains stats.packages[1].errors[1], "several channels"
      ut\assertTrue feed.rawFeedData.modules["l0.Pkg"].channels.main.released != "2099-12-31" -- no stamp on either channel
      ut\assertTrue feed.rawFeedData.modules["l0.Pkg"].channels.stable.released != "2099-12-31"

    -- mergeChannels copies the source channel into the destination channel(s), preserves channels not
    -- named, stamps the release date, sets the default flag, tracks top-level metadata, and adds packages
    mergeChannels_copiesPreservingOthers: (ut) ->
      root = fileOps.joinPath basePath, "merge1"
      fileOps.mkdir root, false, true
      srcPath = fileOps.joinPath root, "src.json"
      dstPath = fileOps.joinPath root, "dst.json"
      fileOps.writeFile srcPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "NewName", "baseUrl": "b",
        "modules": {
          "l0.A":   {"name": "A", "author": "x", "channels": {"main": {"version": "0.7.0", "released": null, "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "AAA"}]}}},
          "l0.New": {"name": "N", "author": "x", "channels": {"main": {"version": "0.7.0", "released": null, "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "CCC"}]}}}
        }
      }]], true
      fileOps.writeFile dstPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "OldName", "baseUrl": "old",
        "modules": {
          "l0.A": {"name": "A", "author": "x", "channels": {
            "release": {"version": "0.6.0", "released": "2024-01-01", "default": true,  "files": [{"name": ".moon", "url": "u", "sha1": "OLD"}]},
            "alpha":   {"version": "0.6.0", "released": "2024-01-01", "default": false, "files": [{"name": ".moon", "url": "u", "sha1": "OLD"}]}}}
        }
      }]], true
      source = UpdateFeed nil, false, srcPath
      source\loadFile srcPath, UpdateFeed.ExpansionMode.Local
      dest = UpdateFeed nil, false, dstPath
      dest\loadFile dstPath, UpdateFeed.ExpansionMode.Local
      merged, err = dest\mergeChannels source, {from: "main", to: {"release"}, defaultChannel: "release", released: "2026-07-19", outPath: false}
      ut\assertNil err
      ut\assertEquals #merged, 2
      chA = dest.rawFeedData.modules["l0.A"].channels
      ut\assertEquals chA.release.version, "0.7.0" -- release taken from the source's main channel
      ut\assertEquals chA.release.released, "2026-07-19" -- release date stamped
      ut\assertTrue chA.release.default -- default flag set
      ut\assertEquals chA.alpha.version, "0.6.0" -- alpha channel preserved
      ut\assertEquals chA.alpha.released, "2024-01-01"
      ut\assertEquals dest.rawFeedData.name, "NewName" -- top-level metadata tracks the source
      newCh = dest.rawFeedData.modules["l0.New"].channels
      ut\assertNotNil newCh.release -- new package added, carrying only the to-channel
      ut\assertNil newCh.alpha

    -- Publishing leaves exactly one channel flagged as the default: the flag is cleared from every
    -- channel but the one named, a copy of the source channel left behind by an earlier publish
    -- included. Two channels claiming it leaves a client to pick between them by table order.
    mergeChannels_leavesExactlyOneDefault: (ut) ->
      root = fileOps.joinPath basePath, "merge2"
      fileOps.mkdir root, false, true
      srcPath = fileOps.joinPath root, "src.json"
      dstPath = fileOps.joinPath root, "dst.json"
      fileOps.writeFile srcPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "N", "baseUrl": "b",
        "modules": {
          "l0.A": {"name": "A", "author": "x", "channels": {"main": {"version": "0.8.0", "released": null, "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "AAA"}]}}}
        }
      }]], true
      fileOps.writeFile dstPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "N", "baseUrl": "b",
        "modules": {
          "l0.A": {"name": "A", "author": "x", "channels": {
            "main":    {"version": "0.7.0", "released": "2024-01-01", "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "OLD"}]},
            "release": {"version": "0.6.0", "released": "2024-01-01", "default": false, "files": [{"name": ".moon", "url": "u", "sha1": "OLD"}]},
            "alpha":   {"version": "0.6.0", "released": "2024-01-01", "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "OLD"}]}}}
        }
      }]], true
      source = UpdateFeed nil, false, srcPath
      source\loadFile srcPath, UpdateFeed.ExpansionMode.Local
      dest = UpdateFeed nil, false, dstPath
      dest\loadFile dstPath, UpdateFeed.ExpansionMode.Local
      merged, err = dest\mergeChannels source, {from: "main", to: {"release"}, defaultChannel: "release", outPath: false}
      ut\assertNil err
      ut\assertEquals #merged, 1
      ch = dest.rawFeedData.modules["l0.A"].channels
      ut\assertTrue ch.release.default
      ut\assertFalse ch.alpha.default -- a channel this run didn't write stops claiming the default
      ut\assertEquals ch.alpha.version, "0.6.0" -- but keeps what it was publishing
      ut\assertFalse ch.main.default -- as does the copy of the source channel left by an earlier publish
      ut\assertEquals ch.main.version, "0.7.0"

    -- A section's own template keys travel with the merge: macros are stored flat while modules nest, so
    -- a published feed that lost the macros section's `fileBaseUrls` resolves every macro to a module path.
    mergeChannels_copiesSectionTemplates: (ut) ->
      root = fileOps.joinPath basePath, "merge4"
      fileOps.mkdir root, false, true
      srcPath = fileOps.joinPath root, "src.json"
      dstPath = fileOps.joinPath root, "dst.json"
      fileOps.writeFile srcPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "N", "baseUrl": "b",
        "macros": {
          "fileBaseUrls": {"script": "@{fileBaseUrl}@{namespace}@{fileName}"},
          "l0.Pkg": {"name": "P", "author": "x", "channels": {"main": {"version": "0.8.0", "released": null, "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "AAA"}]}}}
        }
      }]], true
      fileOps.writeFile dstPath, [[{"dependencyControlFeedFormatVersion": "0.4.0", "macros": {}, "modules": {}}]], true
      source = UpdateFeed nil, false, srcPath
      source\loadFile srcPath, UpdateFeed.ExpansionMode.Local
      dest = UpdateFeed nil, false, dstPath
      dest\loadFile dstPath, UpdateFeed.ExpansionMode.Local
      merged, err = dest\mergeChannels source, {from: "main", to: {"stable"}, defaultChannel: "stable", outPath: false}
      ut\assertNil err
      ut\assertEquals #merged, 1
      macros = dest.rawFeedData.macros
      ut\assertNotNil macros["l0.Pkg"].channels.stable -- the package published as usual
      ut\assertEquals macros.fileBaseUrls.script, "@{fileBaseUrl}@{namespace}@{fileName}"

    -- Publishing to a channel that isn't the default leaves the default channel alone, flag and all,
    -- so releasing to alpha only doesn't strip the feed of the default the caller still names.
    mergeChannels_keepsDefaultOnUnwrittenChannel: (ut) ->
      root = fileOps.joinPath basePath, "merge3"
      fileOps.mkdir root, false, true
      srcPath = fileOps.joinPath root, "src.json"
      dstPath = fileOps.joinPath root, "dst.json"
      fileOps.writeFile srcPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "N", "baseUrl": "b",
        "modules": {
          "l0.A": {"name": "A", "author": "x", "channels": {"main": {"version": "0.8.0", "released": null, "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "AAA"}]}}}
        }
      }]], true
      fileOps.writeFile dstPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "N", "baseUrl": "b",
        "modules": {
          "l0.A": {"name": "A", "author": "x", "channels": {
            "release": {"version": "0.6.0", "released": "2024-01-01", "default": true, "files": [{"name": ".moon", "url": "u", "sha1": "OLD"}]},
            "alpha":   {"version": "0.5.0", "released": "2023-01-01", "default": false, "files": [{"name": ".moon", "url": "u", "sha1": "OLD"}]}}}
        }
      }]], true
      source = UpdateFeed nil, false, srcPath
      source\loadFile srcPath, UpdateFeed.ExpansionMode.Local
      dest = UpdateFeed nil, false, dstPath
      dest\loadFile dstPath, UpdateFeed.ExpansionMode.Local
      merged, err = dest\mergeChannels source, {from: "main", to: {"alpha"}, defaultChannel: "release", outPath: false}
      ut\assertNil err
      ch = dest.rawFeedData.modules["l0.A"].channels
      ut\assertTrue ch.release.default -- the named default keeps its flag though this run didn't write it
      ut\assertEquals ch.release.version, "0.6.0"
      ut\assertEquals ch.alpha.version, "0.8.0" -- alpha published from the source's main channel
      ut\assertFalse ch.alpha.default

    -- bumpVersions off a released version starts a new cycle: it rewrites the marked source literal,
    -- bumps the channel version, clears the release date, and refreshes the file hash
    bumpVersions_startsCycleFromReleased: (ut) ->
      root = fileOps.joinPath basePath, "bump1"
      fileOps.mkdir fileOps.joinPath(root, "modules", "l0"), false, true
      srcFile = fileOps.joinPath root, "modules", "l0", "Pkg.moon"
      fileOps.writeFile srcFile, [[version = "0.7.0"  -- @{l0.Pkg:version}]], true
      hash = fileOps.getHash srcFile
      feedJson = [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "F", "fileBaseUrl": "u/",
        "fileBaseUrls": {"script": "@{fileBaseUrl}@{fileName}"},
        "localFileBasePaths": {"script": "@{localFileBasePath}modules/@{namespacePath}@{fileName}"},
        "modules": {"l0.Pkg": {"name": "Pkg", "author": "x", "channels": {"main": {"version": "0.7.0", "released": "2024-01-01", "default": true, "files": [{"name": ".moon", "url": "@{fileBaseUrl}", "sha1": "HASH"}]}}}}
      }]]
      feedPath = fileOps.joinPath root, "feed.json"
      fileOps.writeFile feedPath, (feedJson\gsub "HASH", hash\upper!), true
      feed = UpdateFeed nil, false, feedPath
      feed\loadFile feedPath, UpdateFeed.ExpansionMode.Local
      stats, err = feed\bumpVersions {level: "minor", namespaces: {"l0.Pkg"}, outPath: false}
      ut\assertNil err
      ut\assertEquals stats.target, "0.8.0"
      ut\assertEquals #stats.bumped, 1
      main = feed.rawFeedData.modules["l0.Pkg"].channels.main
      ut\assertEquals main.version, "0.8.0" -- feed version bumped
      ut\assertEquals main.released, dkjson.null -- release date cleared (new build pending)
      ut\assertNotNil (fileOps.readFile srcFile)\match '"0%.8%.0"' -- marked source literal rewritten

    -- bumpVersions: a feed flagging several defaults is refused before anything is rewritten — resolved
    -- by table order, the bump would land on a channel nobody chose
    bumpVersions_refusesAmbiguousDefault: (ut) ->
      root = fileOps.joinPath basePath, "bumpAmbiguous"
      fileOps.mkdir root, false, true
      feedPath = fileOps.joinPath root, "feed.json"
      fileOps.writeFile feedPath, [[{
        "dependencyControlFeedFormatVersion": "0.4.0", "name": "F", "fileBaseUrl": "u/",
        "fileBaseUrls": {"script": "@{fileBaseUrl}@{fileName}"},
        "localFileBasePaths": {"script": "@{localFileBasePath}modules/@{namespacePath}@{fileName}"},
        "modules": {"l0.Pkg": {"name": "Pkg", "author": "x", "channels": {
          "main": {"version": "0.7.0", "released": "2024-01-01", "default": true, "files": []},
          "stable": {"version": "0.7.0", "released": "2024-01-01", "default": true, "files": []}}}}
      }]], true
      feed = UpdateFeed nil, false, feedPath
      feed\loadFile feedPath, UpdateFeed.ExpansionMode.Local
      stats, err = feed\bumpVersions {level: "patch", namespaces: {"l0.Pkg"}, outPath: false}
      ut\assertNil stats
      ut\assertContains err, "several channels"

    -- walkFiles

    walkFiles_yieldsProxies: (ut) ->
      feed = {
        data: {
          modules: {"test.NS": {channels: {release: {version: "1.0.0",
            files: {{name: "NS.moon", localFileBasePath: "./"}}}}}},
          macros: {}
        },
        feedDir: basePath,
        __class: UpdateFeed
      }
      -- walkFiles lazily loads via ensureLoaded; stub it away since data is supplied directly
      ensureLoadedStub = (ut\stub feed, "ensureLoaded")\calls (self) -> self.data
      results = {}
      for file, channel, pkg, section, scriptType in UpdateFeed.walkFiles(feed)
        results[#results + 1] = {:file, :channel, :pkg, :section, :scriptType}
      ut\assertEquals #results, 1
      ut\assertEquals results[1].pkg.namespace, "test.NS"
      ut\assertEquals results[1].channel.name, "release"
      ut\assertEquals results[1].file.name, "NS.moon"
      ut\assertEquals results[1].section, "modules"
      ut\assertEquals results[1].scriptType, domain.ScriptType.Module
      ensureLoadedStub\assertCalledOnce!

    -- walkFiles yields files untouched; the localFilePath accessor is attached by `expand` in local mode,
    -- so here it's supplied directly on the file record.
    walkFiles_passesThroughLocalFilePath: (ut) ->
      feed = {
        data: {
          modules: {"test.NS": {channels: {release: {version: "1.0.0",
            files: {{name: "NS.moon", localFilePath: fileOps.joinPath(basePath, "NS.moon")}}}}}},
          macros: {}
        },
        feedDir: basePath,
        __class: UpdateFeed
      }
      (ut\stub feed, "ensureLoaded")\calls (self) -> self.data
      for file in UpdateFeed.walkFiles(feed)
        ut\assertString file.localFilePath
        ut\assertContains file.localFilePath, "NS.moon"
        break

    -- deployFiles

    deployFiles_copiesToDist: (ut) ->
      feed = {
        data: {modules: {}, macros: {}},
        feedDir: basePath, logger: DepCtrl.logger, __class: UpdateFeed
      }
      srcPath = "#{basePath}/NS.moon"
      dstPath = "#{basePath}/dst/NS.moon"
      fakeFile = setmetatable {}, {__index: (_, k) ->
        if k == "localFilePath" then srcPath
        elseif k == "name" then "NS.moon"
        elseif k == "type" then "script"
      }
      fakeChan = setmetatable {}, {__index: (_, k) -> k == "name" and "release" or nil}
      fakePkg = setmetatable {}, {__index: (_, k) -> k == "namespace" and "test.NS" or nil}
      (ut\stub feed, "walkFiles")\calls (self, scriptTypes) ->
        coroutine.wrap ->
          coroutine.yield fakeFile, fakeChan, fakePkg, "modules", domain.ScriptType.Module
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns true
      (ut\stub UpdateFeed, "getFileDeployPath")\returns dstPath
      (ut\stub FILEOPS_MODULE_NAME, "mkdir")\returns true
      copyStub = (ut\stub FILEOPS_MODULE_NAME, "copy")\returns true
      fileCount, errCount = UpdateFeed.deployFiles feed, basePath, nil, true
      ut\assertEquals fileCount, 1
      ut\assertEquals errCount, 0
      copyStub\assertCalledOnce!

    deployFiles_skipExistingNoClobber: (ut) ->
      feed = {
        data: {modules: {}, macros: {}},
        feedDir: basePath, logger: DepCtrl.logger, __class: UpdateFeed
      }
      srcPath = "#{basePath}/NS.moon"
      dstPath = "#{basePath}/dst/NS.moon"
      fakeFile = setmetatable {}, {__index: (_, k) ->
        if k == "localFilePath" then srcPath
        elseif k == "name" then "NS.moon"
        elseif k == "type" then "script"
      }
      fakeChan = setmetatable {}, {__index: (_, k) -> k == "name" and "release" or nil}
      fakePkg = setmetatable {}, {__index: (_, k) -> k == "namespace" and "test.NS" or nil}
      (ut\stub feed, "walkFiles")\calls (self, scriptTypes) ->
        coroutine.wrap ->
          coroutine.yield fakeFile, fakeChan, fakePkg, "modules", domain.ScriptType.Module
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns true
      (ut\stub UpdateFeed, "getFileDeployPath")\returns dstPath
      copyStub = (ut\stub FILEOPS_MODULE_NAME, "copy")\returns true
      fileCount, errCount = UpdateFeed.deployFiles feed, basePath
      ut\assertEquals fileCount, 0
      ut\assertEquals errCount, 0
      copyStub\assertNotCalled!

    deployFiles_countsMissingSource: (ut) ->
      feed = {
        data: {modules: {}, macros: {}},
        feedDir: basePath, logger: DepCtrl.logger, __class: UpdateFeed
      }
      fakeFile = setmetatable {}, {__index: (_, k) ->
        if k == "localFilePath" then nil
        elseif k == "name" then "NS.moon"
      }
      fakeChan = setmetatable {}, {__index: (_, k) -> k == "name" and "release" or nil}
      fakePkg = setmetatable {}, {__index: (_, k) -> k == "namespace" and "test.NS" or nil}
      (ut\stub feed, "walkFiles")\calls (self, scriptTypes) ->
        coroutine.wrap ->
          coroutine.yield fakeFile, fakeChan, fakePkg, "modules", domain.ScriptType.Module
      fileCount, errCount = UpdateFeed.deployFiles feed, basePath
      ut\assertEquals fileCount, 0
      ut\assertEquals errCount, 1

    -- a file marked for deletion is removed from the dist (when present), not deployed
    deployFiles_removesDeleted: (ut) ->
      feed = {
        data: {modules: {}, macros: {}},
        feedDir: basePath, logger: DepCtrl.logger, __class: UpdateFeed
      }
      dstPath = "#{basePath}/dst/Old.moon"
      fakeFile = setmetatable {}, {__index: (_, k) ->
        if k == "delete" then true
        elseif k == "name" then "Old.moon"
      }
      fakeChan = setmetatable {}, {__index: (_, k) -> k == "name" and "release" or nil}
      fakePkg = setmetatable {}, {__index: (_, k) -> k == "namespace" and "test.NS" or nil}
      (ut\stub feed, "walkFiles")\calls (self, scriptTypes) ->
        coroutine.wrap ->
          coroutine.yield fakeFile, fakeChan, fakePkg, "modules", domain.ScriptType.Module
      (ut\stub UpdateFeed, "getFileDeployPath")\returns dstPath
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns true
      removeStub = (ut\stub FILEOPS_MODULE_NAME, "remove")\returns true
      copyStub = (ut\stub FILEOPS_MODULE_NAME, "copy")\returns true
      fileCount, errCount = UpdateFeed.deployFiles feed, basePath
      ut\assertEquals fileCount, 0
      ut\assertEquals errCount, 0
      removeStub\assertCalledOnce!
      copyStub\assertNotCalled!

    -- a file marked for deletion whose target isn't in the dist is a clean no-op
    deployFiles_deleteMissingIsNoOp: (ut) ->
      feed = {
        data: {modules: {}, macros: {}},
        feedDir: basePath, logger: DepCtrl.logger, __class: UpdateFeed
      }
      fakeFile = setmetatable {}, {__index: (_, k) ->
        if k == "delete" then true
        elseif k == "name" then "Old.moon"
      }
      fakeChan = setmetatable {}, {__index: (_, k) -> k == "name" and "release" or nil}
      fakePkg = setmetatable {}, {__index: (_, k) -> k == "namespace" and "test.NS" or nil}
      (ut\stub feed, "walkFiles")\calls (self, scriptTypes) ->
        coroutine.wrap ->
          coroutine.yield fakeFile, fakeChan, fakePkg, "modules", domain.ScriptType.Module
      (ut\stub UpdateFeed, "getFileDeployPath")\returns "#{basePath}/dst/Old.moon"
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns false
      removeStub = (ut\stub FILEOPS_MODULE_NAME, "remove")\returns true
      fileCount, errCount = UpdateFeed.deployFiles feed, basePath
      ut\assertEquals fileCount, 0
      ut\assertEquals errCount, 0
      removeStub\assertNotCalled!

    -- ensureLoaded

    ensureLoaded_localWithoutFileName_errors: (ut) ->
      result, err = UpdateFeed.ensureLoaded {__class: UpdateFeed}, UpdateFeed.ExpansionMode.Local
      ut\assertNil result
      ut\assertString err

    ensureLoaded_reusesMatchingExpansion: (ut) ->
      data = {modules: {}}
      feed = {data: data, expansionMode: UpdateFeed.ExpansionMode.Local, fileName: "x.json", __class: UpdateFeed}
      ut\assertIs UpdateFeed.ensureLoaded(feed, UpdateFeed.ExpansionMode.Local), data

    -- a fresh on-disk snapshot is served straight from the cache, never touching the network
    ensureLoaded_readsFreshDiskCache: (ut) ->
      cacheDir = fileOps.joinPath basePath, "uf-cache-fresh"
      url = "https://example.com/fresh.json"
      cache = FileCache cacheDir, "test", "feeds", {deserialize: UpdateFeed.deserialize}
      cache\put url, '{"name":"FreshCache"}', "FreshCache" -- default lifetime → fresh right after writing

      feed = stubSelf UpdateFeed, {
        _url: url, url: url, __class: UpdateFeed, logger: DepCtrl.logger
        config: {cache: cache}
      }
      data = UpdateFeed.ensureLoaded feed
      ut\assertEquals data.name, "FreshCache"
      ut\assertNil feed.stale

    -- when the fetch fails and only a stale snapshot exists, ensureLoaded serves it and flags staleness
    ensureLoaded_fallsBackToStaleCacheOffline: (ut) ->
      cacheDir = fileOps.joinPath basePath, "uf-cache-stale"
      url = "https://example.com/stale.json"
      cache = FileCache cacheDir, "test", "feeds", {deserialize: UpdateFeed.deserialize}
      cache\put url, '{"name":"StaleCache"}', "StaleCache", 0 -- expiresAfter 0 → immediately stale

      feed = stubSelf UpdateFeed, {
        _url: url, url: url, __class: UpdateFeed, logger: DepCtrl.logger
        config: {cache: cache} -- stale entry ⇒ attempts a fetch first
        fetch: (...) -> nil, "network down" -- which fails, forcing the offline fallback
      }
      data = UpdateFeed.ensureLoaded feed
      ut\assertEquals data.name, "StaleCache"
      ut\assertTrue feed.stale
      ut\assertNotNil feed.lastFetchedAt

    -- __refreshFiles: returns (changed, errors) and mutates the raw channel in place

    refreshFiles_updatesChangedSha: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "getHash")\returns "deadbeef"
      rawChannel = {files: {{name: "a.moon", sha1: "a1b2c3d4"}}}
      expandedChannel = {files: {{localFilePath: "/x/a.moon"}}}
      changed, errors = UpdateFeed.__refreshFiles {__class: UpdateFeed}, rawChannel, expandedChannel
      ut\assertTrue changed
      ut\assertEquals #errors, 0
      ut\assertEquals rawChannel.files[1].sha1, "DEADBEEF"

    refreshFiles_unchangedSha: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "getHash")\returns "abc123"
      rawChannel = {files: {{name: "a.moon", sha1: "ABC123"}}}
      changed, errors = UpdateFeed.__refreshFiles {__class: UpdateFeed}, rawChannel, {files: {{localFilePath: "/x/a.moon"}}}
      ut\assertFalse changed
      ut\assertEquals #errors, 0

    refreshFiles_missingFileFlagsDelete: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns false -- vanished from disk
      rawChannel = {files: {{name: "gone.moon", sha1: "X"}}}
      changed, errors = UpdateFeed.__refreshFiles {__class: UpdateFeed}, rawChannel, {files: {{localFilePath: "/x/gone.moon"}}}
      ut\assertTrue changed
      ut\assertTrue rawChannel.files[1].delete
      ut\assertEquals #errors, 0

    refreshFiles_sha1FailureCollectsError: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "exists")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "getHash")\returns nil, "boom"
      rawChannel = {files: {{name: "a.moon", sha1: "X"}}}
      changed, errors = UpdateFeed.__refreshFiles {__class: UpdateFeed}, rawChannel, {files: {{localFilePath: "/x/a.moon"}}}
      ut\assertFalse changed
      ut\assertEquals #errors, 1
      ut\assertContains errors[1], "a.moon"

    refreshFiles_noLocalPathCollectsError: (ut) ->
      rawChannel = {files: {{name: "a.moon", sha1: "X"}}}
      changed, errors = UpdateFeed.__refreshFiles {__class: UpdateFeed}, rawChannel, {files: {{}}}
      ut\assertFalse changed
      ut\assertEquals #errors, 1

    -- __updatePackage: returns a per-package result rather than mutating shared state

    updatePackage_notInRaw: (ut) ->
      feed = {rawFeedData: {modules: {}}, data: {modules: {}}, __class: UpdateFeed}
      result = UpdateFeed.__updatePackage feed, domain.ScriptType.Module, "no.Such", nil
      ut\assertFalse result.changed
      ut\assertEquals #result.errors, 1
      ut\assertContains result.errors[1], "no.Such"

    updatePackage_collectsResultAndResetsReleased: (ut) ->
      ns = "test.NS"
      feed = {
        rawFeedData: {modules: {[ns]: {channels: {release: {default: true, released: "2024-01-01", files: {}}}}}},
        data: {modules: {[ns]: {channels: {release: {files: {}}}}}},
        __class: UpdateFeed
      }
      (ut\stub feed, "__refreshVersionRecord")\returns true -- version/deps changed
      (ut\stub feed, "__refreshFiles")\returns false, {}
      result = UpdateFeed.__updatePackage feed, domain.ScriptType.Module, ns, nil
      ut\assertEquals result.namespace, ns
      ut\assertEquals result.channel, "release"
      ut\assertTrue result.changed
      ut\assertEquals #result.errors, 0
      ut\assertNotNil feed.rawFeedData.modules[ns].channels.release.released -- reset to null sentinel

    updatePackage_collectsRefreshError: (ut) ->
      ns = "test.NS"
      feed = {
        rawFeedData: {modules: {[ns]: {channels: {release: {default: true, files: {}}}}}},
        data: {modules: {[ns]: {channels: {release: {files: {}}}}}},
        __class: UpdateFeed
      }
      (ut\stub feed, "__refreshVersionRecord")\returns nil, "no record"
      (ut\stub feed, "__refreshFiles")\returns false, {}
      result = UpdateFeed.__updatePackage feed, domain.ScriptType.Module, ns, nil
      ut\assertEquals #result.errors, 1
      ut\assertContains result.errors[1], "no record"

    -- resolve: flatten a feed's templates into a static feed a pre-0.4.0 client can read

    resolve_expandsUrlsAndStripsPlumbing: (ut) ->
      feedPath = writeResolveFeed "resolve1"
      feed = UpdateFeed nil, false, feedPath
      resolved = feed\resolve {outPath: false}
      ut\assertTable resolved
      files = resolved.modules["l0.Thing"].channels.release.files
      ut\assertEquals files[1].url, "https://x.test/v1.2.3/modules/l0/Thing.moon"
      ut\assertEquals files[2].url, "https://x.test/v1.2.3/modules/l0/Thing/Sub.moon"
      ut\assertNil resolved.fileBaseUrl
      ut\assertNil resolved.fileBaseUrls
      ut\assertNil resolved.localFileBasePaths
      ut\assertNil resolved.vars

    resolve_fileBaseUrlOverrideRedirectsAndNormalizesSlash: (ut) ->
      feedPath = writeResolveFeed "resolve2"
      feed = UpdateFeed nil, false, feedPath
      -- override lacks a trailing slash; resolve adds one so the template joins cleanly
      resolved = feed\resolve {fileBaseUrl: "http://127.0.0.1:9000", outPath: false}
      files = resolved.modules["l0.Thing"].channels.release.files
      ut\assertEquals files[1].url, "http://127.0.0.1:9000/v1.2.3/modules/l0/Thing.moon"

    resolve_carriesSha1AndStampsSchemaVersion: (ut) ->
      feedPath = writeResolveFeed "resolve3"
      feed = UpdateFeed nil, false, feedPath
      resolved = feed\resolve {feedSchemaVersion: "0.3.0", outPath: false}
      ut\assertEquals resolved.dependencyControlFeedFormatVersion, "0.3.0"
      ut\assertEquals resolved.modules["l0.Thing"].channels.release.files[1].sha1,
        "1111111111111111111111111111111111111111"

    resolve_keepsLiteralMarkerInProse: (ut) ->
      feedPath = writeResolveFeed "resolve4"
      feed = UpdateFeed nil, false, feedPath
      resolved = feed\resolve {outPath: false}
      ut\assertTable resolved
      ut\assertContains resolved.modules["l0.Thing"].changelog["1.2.3"][1], "@{fileName}"

    resolve_unresolvedUrlVariableErrors: (ut) ->
      -- a file-URL template referencing an undefined variable must fail rather than ship a broken URL
      json = resolveFeedJson\gsub "v@{version}/", "v@{version}/@{noSuchVar}/"
      feedPath = writeResolveFeed "resolve5", json
      feed = UpdateFeed nil, false, feedPath
      result, err = feed\resolve {outPath: false}
      ut\assertNil result
      ut\assertString err
      ut\assertContains err, "noSuchVar"

    resolve_writesResolvedFeedToOutFile: (ut) ->
      feedPath, root = writeResolveFeed "resolve6"
      feed = UpdateFeed nil, false, feedPath
      outPath = fileOps.joinPath root, "out.json"
      result, err = feed\resolve {outPath: outPath, feedSchemaVersion: "0.3.0"}
      ut\assertNil err
      ut\assertEquals result, outPath
      decoded = dkjson.decode fileOps.readFile outPath
      ut\assertEquals decoded.dependencyControlFeedFormatVersion, "0.3.0"
      ut\assertEquals decoded.modules["l0.Thing"].channels.release.files[1].url,
        "https://x.test/v1.2.3/modules/l0/Thing.moon"

    -- a section carries the format's section-scoped template keys alongside its packages, so a walk
    -- must yield only the latter
    walkPackages_skipsSectionTemplateKeys: (ut) ->
      feed = stubSelf UpdateFeed, {__class: UpdateFeed, data: {
        macros: {
          fileBaseUrls: {script: "https://x.test/@{fileName}"}
          localFileBasePaths: {script: "@{localFileBasePath}@{fileName}"}
          "l0.Macro": {channels: {main: {default: true, version: "1.0.0", files: {}}}}
        }
        modules: {
          "l0.Module": {channels: {main: {default: true, version: "2.0.0", files: {}}}}
        }
      }}
      seen = [pkg.namespace for pkg in feed\walkPackages!]
      ut\assertItemsEqual seen, {"l0.Macro", "l0.Module"}

    _order: {
      "walkPackages_skipsSectionTemplateKeys",
      "knownFeeds_noData", "knownFeeds_withData",
      "getScript_invalidType", "getScript_missing", "getScript_found",
      "getMacro_usesAutomationType", "getModule_usesModuleType",
      "getModuleVersion_defaultChannel", "getModuleVersion_fallback", "getModuleVersion_missing",
      "getProviders_findsByBareAlias", "getProviders_honorsRecordedChannel", "getProviders_objectAliasEntries",
      "getProviders_noMatchReturnsEmpty",
      "getProviders_ignoresModulesWithoutProvides", "getProviders_noModulesSection",
      "normalizeModuleAliases_bareStringsToTables", "normalizeModuleAliases_preservesFields",
      "normalizeModuleAliases_dropsNonSchemaFields", "normalizeModuleAliases_nilAndEmpty",
      "getFileDeployPath_module", "getFileDeployPath_test",
      "expand_rebuildsFromUnexpandedDataWithoutMutatingIt",
      "expand_fileBaseUrlsCollapsePerType", "expand_fileBaseUrlsFallbackToScalar",
      "expand_sectionScopedOverride", "expand_scriptTypeVariables",
      "expand_authorVarsAndComputedKeys", "expand_localFileBasePathsResolveFullPaths",
      "expand_legacyScalarBases",
      "findUnlistedFiles_discoversUnlistedFiles", "findUnlistedFiles_skipsUninvertibleTemplates",
      "updateFeed_addFilesAppendsEntries", "updateFeed_markReleasedStampsUnreleased",
      "updateFeed_markReleasedReportsAmbiguousDefault",
      "mergeChannels_copiesPreservingOthers", "mergeChannels_leavesExactlyOneDefault",
      "mergeChannels_keepsDefaultOnUnwrittenChannel", "mergeChannels_copiesSectionTemplates"
      "bumpVersions_startsCycleFromReleased", "bumpVersions_refusesAmbiguousDefault",
      "walkFiles_yieldsProxies", "walkFiles_passesThroughLocalFilePath",
      "deployFiles_copiesToDist", "deployFiles_skipExistingNoClobber",
      "deployFiles_countsMissingSource", "deployFiles_removesDeleted", "deployFiles_deleteMissingIsNoOp",
      "ensureLoaded_localWithoutFileName_errors", "ensureLoaded_reusesMatchingExpansion",
      "ensureLoaded_readsFreshDiskCache", "ensureLoaded_fallsBackToStaleCacheOffline",
      "refreshFiles_updatesChangedSha", "refreshFiles_unchangedSha", "refreshFiles_missingFileFlagsDelete",
      "refreshFiles_sha1FailureCollectsError", "refreshFiles_noLocalPathCollectsError",
      "updatePackage_notInRaw", "updatePackage_collectsResultAndResetsReleased",
      "updatePackage_collectsRefreshError",
      "resolve_expandsUrlsAndStripsPlumbing", "resolve_fileBaseUrlOverrideRedirectsAndNormalizesSlash",
      "resolve_carriesSha1AndStampsSchemaVersion", "resolve_keepsLiteralMarkerInProse",
      "resolve_unresolvedUrlVariableErrors", "resolve_writesResolvedFeedToOutFile"
    }
  }
