-- ConfigHandler tests: JSON-backed config manager.
-- Called from Tests.moon as: (require "...test.ConfigHandler")!
->
  ConfigHandler = require "l0.DependencyControl.ConfigHandler"
  ConfigView = require "l0.DependencyControl.ConfigView"
  Lock = require "l0.DependencyControl.Lock"

  FILEOPS_MODULE_NAME = "l0.DependencyControl.file-ops"
  -- ConfigHandler reads and writes config through the vendored dkjson (for its Prettier indent on write).
  DKJSON_MODULE_NAME = "l0.dkjson"

  {
    _description: "Tests for the ConfigHandler JSON-backed config manager."

    -- getSerializableCopy: pure static method, no stubs needed

    getSerializableCopy_simple: (ut) ->
      result = ConfigHandler\getSerializableCopy {a: 1, b: "hello"}
      ut\assertEquals result.a, 1
      ut\assertEquals result.b, "hello"

    getSerializableCopy_privateKeys: (ut) ->
      result = ConfigHandler\getSerializableCopy {pub: 1, _priv: 2}
      ut\assertEquals result.pub, 1
      ut\assertNil result._priv

    getSerializableCopy_nested: (ut) ->
      result = ConfigHandler\getSerializableCopy {outer: {inner: 1, _skip: 2}}
      ut\assertEquals result.outer.inner, 1
      ut\assertNil result.outer._skip

    getSerializableCopy_circular: (ut) ->
      t = {a: 1}
      t.self = t
      result = ConfigHandler\getSerializableCopy t
      ut\assertEquals result.a, 1
      ut\assertEquals type(result.self), "table"
      ut\assertNil result.self.a -- circular ref becomes empty table

    -- new

    new_noPath: (ut) ->
      handler = ConfigHandler nil
      ut\assertNil handler.filePath
      ut\assertNil handler.lock
      ut\assertEquals type(handler.config), "table"

    new_withPath: (ut) ->
      validateStub = (ut\stub FILEOPS_MODULE_NAME, "validateFullPath")\calls (path) -> path, nil
      handler = ConfigHandler "/config/test.json"
      ut\assertEquals handler.filePath, "/config/test.json"
      ut\assertNotNil handler.lock
      -- the Global lock's FileLock validates its own lock-file path too, so match the config-path call
      validateStub\assertCalledWith "/config/test.json", true

    new_badPath: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "validateFullPath")\returns nil, "invalid path"
      ok, err = pcall -> ConfigHandler "/bad/path.json"
      ut\assertFalse ok

    -- @get caches by the validated path, so two raw spellings that normalize to one file share a handler
    get_cachesByValidatedPath: (ut) ->
      canon = "#{aegisub.decode_path '?temp'}/dc_m12a_get_cache.json"
      (ut\stub FILEOPS_MODULE_NAME, "validateFullPath")\calls (p) -> canon, nil
      h1 = ConfigHandler\get "raw-spelling-one", nil, true
      h2 = ConfigHandler\get "raw-spelling-two", nil, true -- validates to the same canonical path
      ut\assertNotNil h1
      ut\assertEquals h1, h2 -- same handler, not a duplicate

    -- getHive: exercises traverseHive + mergeHive internally

    getHive_exists: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {key: "value"}}
      hive, err = handler\getHive {"section"}
      ut\assertNil err
      ut\assertEquals hive.key, "value"

    getHive_missing: (ut) ->
      handler = ConfigHandler nil
      hive, err = handler\getHive {"section"}
      ut\assertNil err
      ut\assertEquals type(hive), "table"
      ut\assertEquals type(handler.config.section), "table" -- path created in config

    getHive_badParent: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: "not_a_table"}
      hive, err = handler\getHive {"section", "child"}
      ut\assertNil hive
      ut\assertString err

    -- getView

    getView_success: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {key: "value"}}
      view, err = handler\getView {"section"}
      ut\assertNil err
      ut\assertNotNil view
      ut\assertEquals view.__hivePath[1], "section"
      ut\assertEquals #view.__hivePath, 1
      ut\assertTrue handler.views[view]

    getView_failure: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: "not_a_table"}
      view, err = handler\getView {"section", "child"}
      ut\assertNil view
      ut\assertString err

    -- __getOverlappingViews

    getOverlappingViews_wrongHandler: (ut) ->
      handler1 = ConfigHandler nil
      handler2 = ConfigHandler nil
      view2 = ConfigView handler2, {"section"}
      overlaps, err = handler1\__getOverlappingViews view2
      ut\assertNil overlaps
      ut\assertString err

    getOverlappingViews_found: (ut) ->
      handler = ConfigHandler nil
      view1 = ConfigView handler, {"section"}
      view2 = ConfigView handler, {"section", "child"}
      handler.views[view1] = true
      handler.views[view2] = true
      overlaps, err = handler\__getOverlappingViews view1
      ut\assertNil err
      ut\assertEquals #overlaps, 1
      ut\assertEquals overlaps[1], view2

    getOverlappingViews_notFound: (ut) ->
      handler = ConfigHandler nil
      view1 = ConfigView handler, {"sectionA"}
      view2 = ConfigView handler, {"sectionB"}
      handler.views[view1] = true
      handler.views[view2] = true
      overlaps, err = handler\__getOverlappingViews view1
      ut\assertNil err
      ut\assertEquals #overlaps, 0

    -- load: stubs fileOps.getAttributes, lock, io.open, json.decode

    load_noFilePath: (ut) ->
      handler = ConfigHandler nil
      result, err = handler\load!
      ut\assertNil result
      ut\assertString err

    load_fileNotFound: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: false, path: "/config/test.json"}
      result = handler\load!
      ut\assertTrue result
      ut\assertEquals handler.config, {}

    load_success: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      handler.lock = {}
      (ut\stub handler.lock, "lock")\returns Lock.LockState.Held, 0
      ut\stub handler.lock, "release"
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: "file", path: "/config/test.json"}
      openStub = (ut\stub io, "open")\calls -> {
        read: (handle, fmt) -> '{"key":"value"}'
        close: (handle) ->
      }
      (ut\stub DKJSON_MODULE_NAME, "decode")\returns {key: "value"}
      result = handler\load!
      ut\assertTrue result
      ut\assertEquals handler.config.key, "value"
      openStub\assertCalledOnceWith "/config/test.json", "r"

    -- load runs the migration callback when the loaded file's $schema differs from the handler's target,
    -- then stamps the target $schema and persists the change
    load_migratesOnSchemaMismatch: (ut) ->
      captured = {}
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      handler.lock = {}
      handler.__targetSchemaId = "schema://v2"
      handler.__migrate = (config, current, target) ->
        captured.current, captured.target = current, target
        config.config = {migrated: true}
        return true
      (ut\stub handler.lock, "lock")\returns Lock.LockState.Held, 0
      ut\stub handler.lock, "release"
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: "file", path: "/config/test.json"}
      (ut\stub io, "open")\calls -> {read: ((h, f) -> "{}"), close: (->)}
      (ut\stub DKJSON_MODULE_NAME, "decode")\returns {config: {flatKey: 1}} -- legacy: no $schema
      saveStub = (ut\stub handler, "save")\returns true
      ut\assertTrue handler\load!
      ut\assertNil captured.current -- the legacy file carried no $schema
      ut\assertEquals captured.target, "schema://v2"
      ut\assertEquals handler.schemaId, "schema://v2" -- exposed on the handler
      ut\assertEquals handler.config["$schema"], "schema://v2" -- stamped into the config
      ut\assertTrue handler.config.config.migrated
      saveStub\assertCalledOnce! -- migration persisted

    -- load skips migration (and the persist) when the file already carries the target $schema
    load_skipsMigrationWhenSchemaMatches: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      handler.lock = {}
      handler.__targetSchemaId = "schema://v2"
      migrated = false
      handler.__migrate = (config, current, target) -> migrated = true
      (ut\stub handler.lock, "lock")\returns Lock.LockState.Held, 0
      ut\stub handler.lock, "release"
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: "file", path: "/config/test.json"}
      (ut\stub io, "open")\calls -> {read: ((h, f) -> "{}"), close: (->)}
      (ut\stub DKJSON_MODULE_NAME, "decode")\returns {["$schema"]: "schema://v2", config: {}}
      saveStub = (ut\stub handler, "save")\returns true
      ut\assertTrue handler\load!
      ut\assertFalse migrated -- current == target: callback never invoked
      ut\assertEquals handler.schemaId, "schema://v2"
      saveStub\assertNotCalled!

    -- save: stubs fileOps.getAttributes, lock, io.open, json.encode

    save_noFilePath: (ut) ->
      handler = ConfigHandler nil
      result, err = handler\save!
      ut\assertNil result
      ut\assertString err

    save_lockFailed: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      handler.lock = {}
      (ut\stub handler.lock, "lock")\returns Lock.LockState.Unavailable, 0
      result, err = handler\save!
      ut\assertNil result
      ut\assertString err

    save_success: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      handler.config = {key: "value"}
      handler.lock = {}
      (ut\stub handler.lock, "lock")\returns Lock.LockState.Held, 0
      ut\stub handler.lock, "release"
      -- readFile sees no existing file, save writes fresh
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: false, path: "/config/test.json"}
      writeHandle = {setvbuf: ->, write: ->, flush: ->, close: ->}
      openStub = (ut\stub io, "open")\returns writeHandle
      (ut\stub DKJSON_MODULE_NAME, "encode")\returns '{"key":"value"}'
      result = handler\save!
      ut\assertTrue result
      openStub\assertCalledOnceWith "/config/test.json", "w"

    -- save with views: exercises mergeHive + cleanHive

    save_withViewMissingHive: (ut) ->
      -- Regression: mirrors the Updater scenario where a virtual module
      -- is installed and its config view is switched from an in-memory
      -- handler (Handler A) to the real file handler (Handler B). Handler B's
      -- @config doesn't yet have this namespace, so mergeHive nils out the
      -- view's path in the freshly-read file config, and cleanHive must
      -- treat that absence as "nothing to purge" instead of crashing.

      -- Handler A: in-memory only, no file backing (virtual module state)
      view = ConfigView\get false, {"section", "key"}
      view.userConfig.someField = "data"

      -- Handler B: real file handler — its in-memory @config knows about
      -- the section (e.g. other modules) but not this view's specific key
      handlerB = ConfigHandler nil
      handlerB.filePath = "/config/test.json"
      handlerB.config = {section: {}}
      handlerB.lock = {}
      (ut\stub handlerB.lock, "lock")\returns Lock.LockState.Held, 0
      ut\stub handlerB.lock, "release"
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: false, path: "/config/test.json"}
      (ut\stub io, "open")\returns {setvbuf: ->, write: ->, flush: ->, close: ->}
      (ut\stub DKJSON_MODULE_NAME, "encode")\returns '{}'

      -- Switch the view from Handler A to Handler B (what setFile does
      -- under the hood after a virtual module has been installed)
      view.__configHandler = handlerB

      result = handlerB\save view
      ut\assertTrue result

    -- A save writes what this handler changed since it read the file, not the hive it happens to hold, so
    -- a second handler over the same file — another Aegisub, or the instance a self-update replaced —
    -- keeps the keys it wrote in between.
    save_keepsAnotherHandlersChanges: (ut) ->
      path = "#{aegisub.decode_path '?temp'}/dc_m12a_baseline.json"
      fileOps = require FILEOPS_MODULE_NAME
      fileOps.writeFile path, [[{"modules": {"l0.Pkg": {"version": "1.0.0", "lastChannel": "main"}}}]], true

      first = ConfigHandler path
      second = ConfigHandler path
      firstView = first\getView {"modules", "l0.Pkg"}
      secondView = second\getView {"modules", "l0.Pkg"}
      ut\assertTrue first\load firstView
      ut\assertTrue second\load secondView

      -- the second handler migrates the channel and writes it away
      secondView.c.lastChannel = "stable"
      secondView.c.configuredSource = {channel: "stable"}
      ut\assertTrue second\save secondView

      -- the first handler, which never saw either key, records a completed install
      firstView.c.version = "2.0.0"
      firstView.c.currentSource = {channel: "main"}
      ut\assertTrue first\save firstView

      written = ConfigHandler path
      writtenView = written\getView {"modules", "l0.Pkg"}
      ut\assertTrue written\load writtenView
      ut\assertEquals writtenView.c.version, "2.0.0" -- the later save landed
      ut\assertEquals writtenView.c.currentSource.channel, "main"
      ut\assertEquals writtenView.c.lastChannel, "stable" -- and did not revert the earlier one
      ut\assertEquals writtenView.c.configuredSource.channel, "stable"

    save_withViewPopulatedHive: (ut) ->
      -- Normal path: cleanHive keeps a hive that has data and save succeeds.
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      handler.config = {section: {key: {value: 42}}}
      handler.lock = {}
      (ut\stub handler.lock, "lock")\returns Lock.LockState.Held, 0
      ut\stub handler.lock, "release"
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: false, path: "/config/test.json"}
      (ut\stub io, "open")\returns {setvbuf: ->, write: ->, flush: ->, close: ->}
      (ut\stub DKJSON_MODULE_NAME, "encode")\returns '{}'
      fakeView = {__hivePath: {"section", "key"}, __class: ConfigView}
      result = handler\save fakeView
      ut\assertTrue result

    -- the config is written as Prettier-flavored JSON (two-space indent) with a trailing newline, so a
    -- hand-edited config stays readable — encode runs through the real dkjson (not stubbed) to prove the format
    save_writesPrettyPrintedJson: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/config/test.json"
      handler.config = {config: {updates: {mode: "off"}}}
      handler.lock = {}
      (ut\stub handler.lock, "lock")\returns Lock.LockState.Held, 0
      ut\stub handler.lock, "release"
      (ut\stub FILEOPS_MODULE_NAME, "getAttributes")\returns {attr: false, path: "/config/test.json"}
      written = {}
      writeHandle = {
        setvbuf: ->
        write: (_, ...) -> written[#written + 1] = table.concat {...}
        flush: ->
        close: ->
      }
      (ut\stub io, "open")\returns writeHandle
      result = handler\save!
      ut\assertTrue result
      out = table.concat written
      ut\assertMatches out, '\n  "config"' -- two-space-indented member (Prettier)
      ut\assertMatches out, "\n$" -- trailing newline

    -- purgeHive

    purgeHive_removesPath: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {key: "value"}, other: {x: 1}}
      view = ConfigView handler, {"section"}
      newHive = handler\purgeHive view
      ut\assertEquals type(newHive), "table"
      ut\assertNil newHive.key -- original content cleared
      ut\assertEquals handler.config.other.x, 1 -- sibling section untouched

    _order: {
      "getSerializableCopy_simple", "getSerializableCopy_privateKeys",
      "getSerializableCopy_nested", "getSerializableCopy_circular",
      "new_noPath", "new_withPath", "new_badPath", "get_cachesByValidatedPath",
      "getHive_exists", "getHive_missing", "getHive_badParent",
      "getView_success", "getView_failure",
      "getOverlappingViews_wrongHandler", "getOverlappingViews_found", "getOverlappingViews_notFound",
      "load_noFilePath", "load_fileNotFound", "load_success",
      "load_migratesOnSchemaMismatch", "load_skipsMigrationWhenSchemaMatches",
      "save_noFilePath", "save_lockFailed", "save_success",
      "save_withViewMissingHive", "save_withViewPopulatedHive", "save_keepsAnotherHandlersChanges",
      "save_writesPrettyPrintedJson",
      "purgeHive_removesPath"
    }
  }
