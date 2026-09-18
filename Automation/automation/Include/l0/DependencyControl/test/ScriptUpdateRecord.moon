-- ScriptUpdateRecord tests: channel management and update record accessors.
-- Called from Tests.moon as: (require "...test.ScriptUpdateRecord")!
->
  domain = require "l0.DependencyControl.domain"
  environment = require "l0.DependencyControl.environment"
  ScriptUpdateRecord = require "l0.DependencyControl.ScriptUpdateRecord"

  {
    _description: "Tests for ScriptUpdateRecord channel management and update record accessors."

    getChannels_basic: (ut) ->
      data = {channels: {release: {default: true, version: "1.0.0", files: {}}, nightly: {version: "2.0.0", files: {}}}, name: "TestScript"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module, false
      channels, default = sur\getChannels!
      ut\assertEquals #channels, 2
      ut\assertEquals default, "release"

    getChannels_noDefault: (ut) ->
      data = {channels: {alpha: {version: "1.0.0", files: {}}, beta: {version: "2.0.0", files: {}}}, name: "TestScript"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module, false
      _, default = sur\getChannels!
      ut\assertNil default

    -- feeds are third-party input, so a package declaring no channels must not take out its caller
    getChannels_noChannels: (ut) ->
      sur = ScriptUpdateRecord "test.NS", {name: "TestScript"}, {c:{}}, domain.ScriptType.Module, false
      channels, default = sur\getChannels!
      ut\assertEquals #channels, 0
      ut\assertNil default

    setChannel_valid: (ut) ->
      data = {channels: {release: {default: true, version: "1.0.0", files: {}}, nightly: {version: "2.0.0", files: {}}}, name: "TestScript"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module, false
      success, channel = sur\setChannel "nightly"
      ut\assertTrue success
      ut\assertEquals channel, "nightly"
      ut\assertEquals sur.version, "2.0.0"

    setChannel_invalid: (ut) ->
      data = {channels: {release: {default: true, version: "1.0.0", files: {}}}, name: "TestScript"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module, false
      success, channel = sur\setChannel "nonexistent"
      ut\assertFalse success
      ut\assertEquals channel, "nonexistent"

    -- getDefaultChannel is where both the client and the feed tooling get their answer, so the sorted
    -- pick and the conflict report are pinned here rather than at either call site
    getDefaultChannel_picksSortedAndReportsConflict: (ut) ->
      ut\assertNil ScriptUpdateRecord.getDefaultChannel nil
      ut\assertNil ScriptUpdateRecord.getDefaultChannel {alpha: {}, beta: {}} -- none flagged
      name, conflicting = ScriptUpdateRecord.getDefaultChannel {stable: {default: true}, main: {}}
      ut\assertEquals name, "stable"
      ut\assertNil conflicting
      name, conflicting = ScriptUpdateRecord.getDefaultChannel {stable: {default: true}, main: {default: true}}
      ut\assertEquals name, "main"
      ut\assertEquals table.concat(conflicting, ","), "main,stable" -- every flagged name, sorted

    -- a feed flagging several channels as the default gets the same answer every time, not one decided
    -- by table order, so an install can't land on a different channel each session
    getChannels_ambiguousDefaultIsDeterministic: (ut) ->
      warned = 0
      data = {name: "TestScript", channels: {
        stable: {default: true, version: "2.0.0", files: {}}
        main: {default: true, version: "1.0.0", files: {}}
        alpha: {version: "3.0.0", files: {}}
      }}
      logger = {warn: ((...) => warned += 1)}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module, false, logger
      channels, default = sur\getChannels!
      ut\assertEquals #channels, 3
      ut\assertEquals default, "main" -- sorted, so the pick doesn't move between runs
      ut\assertEquals warned, 1 -- and the malformed feed is reported

    -- asked to, a package recorded against a channel the feed has since dropped moves to the default
    -- channel instead of resolving to no candidate at all
    setChannel_recordedChannelGoneFallsBack: (ut) ->
      data = {name: "TestScript", channels: {release: {default: true, version: "1.0.0", files: {}}}}
      config = {c: {lastChannel: "main"}}
      sur = ScriptUpdateRecord "test.NS", data, config, domain.ScriptType.Module, false, {warn: (=>)}
      success, channel = sur\setChannel!
      ut\assertTrue success
      ut\assertEquals channel, "release"
      ut\assertEquals config.c.lastChannel, "main" -- selection only; the updater records the move when it persists the source

    -- an explicitly requested channel beats the recorded one, so a channel switch isn't silently undone
    -- by whatever an earlier resolution recorded
    setChannel_explicitRequestOverridesRecorded: (ut) ->
      data = {name: "TestScript", channels: {
        stable: {default: true, version: "2.0.0", files: {}}
        alpha: {version: "3.0.0", files: {}}
      }}
      config = {c: {lastChannel: "stable"}}
      sur = ScriptUpdateRecord "test.NS", data, config, domain.ScriptType.Module, false, {warn: (=>)}
      success, channel = sur\setChannel "alpha"
      ut\assertTrue success
      ut\assertEquals channel, "alpha"
      ut\assertEquals sur.version, "3.0.0"
      ut\assertEquals config.c.lastChannel, "stable" -- untouched
      ut\assertNil config.c.channels -- selection writes no config at all

    -- the configured source names the channel the next update should use, so it wins over the source the
    -- installed copy came from while a channel change hasn't been applied by an update yet
    setChannel_configuredChannelWinsOverInstalled: (ut) ->
      data = {name: "TestScript", channels: {
        stable: {default: true, version: "2.0.0", files: {}}
        alpha: {version: "3.0.0", files: {}}
      }}
      config = {c: {
        currentSource: {channel: "stable", stickiness: domain.SourceChoiceStickiness.Retain}
        configuredSource: {channel: "alpha", stickiness: domain.SourceChoiceStickiness.Retain}
      }}
      sur = ScriptUpdateRecord "test.NS", data, config, domain.ScriptType.Module, false, {warn: (=>)}
      success, channel = sur\setChannel!
      ut\assertTrue success
      ut\assertEquals channel, "alpha"
      ut\assertEquals sur.version, "3.0.0"

    -- the package source's channel wins over the pre-0.7 lastChannel, which froze at whatever the first
    -- feed evaluation wrote and so can disagree with what resolutions have settled on since
    setChannel_packageSourceChannelWinsOverLegacyKey: (ut) ->
      data = {name: "TestScript", channels: {
        stable: {default: true, version: "2.0.0", files: {}}
        main: {version: "1.0.0", files: {}}
      }}
      config = {c: {lastChannel: "main", currentSource: {channel: "stable", stickiness: domain.SourceChoiceStickiness.Retain}}}
      sur = ScriptUpdateRecord "test.NS", data, config, domain.ScriptType.Module, false, {warn: (=>)}
      success, channel = sur\setChannel!
      ut\assertTrue success
      ut\assertEquals channel, "stable"
      ut\assertEquals sur.version, "2.0.0"

    -- a pinned package source keeps the recorded channel and fails, so the pin isn't quietly broken
    setChannel_recordedChannelGonePinnedFails: (ut) ->
      data = {name: "TestScript", channels: {release: {default: true, version: "1.0.0", files: {}}}}
      config = {c: {lastChannel: "main", currentSource: {stickiness: domain.SourceChoiceStickiness.Pinned}}}
      sur = ScriptUpdateRecord "test.NS", data, config, domain.ScriptType.Module, false, {warn: (=>)}
      success, channel = sur\setChannel!
      ut\assertFalse success
      ut\assertEquals channel, "main"
      ut\assertEquals config.c.lastChannel, "main" -- untouched

    checkPlatform_noConstraint: (ut) ->
      data = {channels: {release: {default: true, version: "1.0.0", files: {}}}, name: "T"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      result, platform = sur\checkPlatform!
      ut\assertTrue result
      ut\assertString platform

    checkPlatform_currentPlatform: (ut) ->
      -- platforms in channel data is copied to the instance via setChannel
      data = {channels: {release: {default: true, version: "1.0.0", files: {}, platforms: {environment.platform}}}, name: "T"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      result, _ = sur\checkPlatform!
      ut\assertTrue result

    checkPlatform_notMatching: (ut) ->
      data = {channels: {release: {default: true, version: "1.0.0", files: {}, platforms: {"nonexistent-arch"}}}, name: "T"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      result, _ = sur\checkPlatform!
      ut\assertFalsy result

    getChangelog_noTable: (ut) ->
      data = {channels: {release: {default: true, version: "1.0.0", files: {}}}, name: "T", changelog: "not a table"}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      ut\assertEquals sur\getChangelog(nil), ""

    getChangelog_inRange: (ut) ->
      data = {
        channels: {release: {default: true, version: "1.0.0", files: {}}},
        name: "TestScript",
        changelog: {["1.0.0"]: {"Initial release"}, ["0.5.0"]: {"Beta"}}
      }
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      result = sur\getChangelog nil
      ut\assertString result
      ut\assertContains result, "TestScript"
      ut\assertContains result, "Initial release"

    getChangelog_allOutOfRange: (ut) ->
      data = {channels: {release: {default: true, version: "1.0.0", files: {}}}, name: "T", changelog: {["1.0.0"]: {"Initial release"}}}
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      ut\assertEquals sur\getChangelog(nil, "2.0.0"), ""

    -- a malformed changelog version key (unvalidated feed data) is skipped, not crashed on
    getChangelog_skipsMalformedKey: (ut) ->
      data = {
        channels: {release: {default: true, version: "1.0.0", files: {}}}, name: "TestScript"
        changelog: {["1.0.0"]: {"Initial release"}, ["not-a-version"]: {"Bogus"}}
      }
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      result = sur\getChangelog nil
      ut\assertContains result, "Initial release" -- valid entry still rendered
      ut\assertString result -- and no crash on the malformed key

    -- marked entries are grouped under category headings; the machine type token is dropped and the
    -- scope kept, so the raw marker never shows in-app
    getChangelog_groupsMarkedEntries: (ut) ->
      data = {
        channels: {release: {default: true, version: "1.0.0", files: {}}},
        name: "TestScript",
        changelog: {["1.0.0"]: {"fix(Updater): fixed a thing", "feat: added a thing", "change!: broke a thing"}}
      }
      sur = ScriptUpdateRecord "test.NS", data, {c:{}}, domain.ScriptType.Module
      result = sur\getChangelog nil
      ut\assertContains result, "New Features"
      ut\assertContains result, "Bug Fixes"
      ut\assertContains result, "Changes"
      ut\assertContains result, "Updater: fixed a thing" -- scope kept, type token dropped
      ut\assertContains result, "⚠️ broke a thing" -- breaking tagged within its section
      ut\assertFalsy result\find "fix(Updater):", 1, true -- raw marker never leaks
      ut\assertFalsy result\find "Breaking Changes", 1, true -- breaking is not its own section

    _order: {
      "getChannels_basic", "getChannels_noDefault", "getChannels_noChannels",
      "setChannel_valid", "setChannel_invalid",
      "getDefaultChannel_picksSortedAndReportsConflict", "getChannels_ambiguousDefaultIsDeterministic",
      "setChannel_recordedChannelGoneFallsBack", "setChannel_recordedChannelGonePinnedFails",
      "setChannel_configuredChannelWinsOverInstalled", "setChannel_packageSourceChannelWinsOverLegacyKey",
      "setChannel_explicitRequestOverridesRecorded",
      "checkPlatform_noConstraint", "checkPlatform_currentPlatform", "checkPlatform_notMatching",
      "getChangelog_noTable", "getChangelog_inRange", "getChangelog_allOutOfRange",
      "getChangelog_skipsMalformedKey", "getChangelog_groupsMarkedEntries"
    }
  }
