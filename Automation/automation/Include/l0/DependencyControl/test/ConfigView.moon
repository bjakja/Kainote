-- ConfigView tests: hive accessor and defaults proxy.
-- Called from Tests.moon as: (require "...test.ConfigView")!
->
  ConfigHandler = require "l0.DependencyControl.ConfigHandler"
  ConfigView = require "l0.DependencyControl.ConfigView"
  fileOps = require "l0.DependencyControl.file-ops"

  {
    _description: "Tests for the ConfigView hive accessor and defaults proxy."

    -- new

    new_orphan: (ut) ->
      view = ConfigView nil, "section"
      ut\assertEquals view.__hivePath[1], "section"
      ut\assertEquals #view.__hivePath, 1
      ut\assertNil view.__configHandler
      ut\assertEquals view.userConfig, {}
      ut\assertNil view.file

    new_withHandler: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/test/config.json"
      handler.config = {section: {key: "value"}}
      view = ConfigView handler, {"section"}
      ut\assertEquals view.__configHandler, handler
      ut\assertEquals view.userConfig.key, "value"
      ut\assertEquals view.file, "/test/config.json"

    new_stringHivePath: (ut) ->
      view = ConfigView nil, "mySection"
      ut\assertEquals view.__hivePath[1], "mySection"
      ut\assertEquals #view.__hivePath, 1

    new_tableHivePath: (ut) ->
      view = ConfigView nil, {"a", "b"}
      ut\assertEquals view.__hivePath[1], "a"
      ut\assertEquals view.__hivePath[2], "b"

    -- isOverlappingView

    isOverlappingView_differentHandler: (ut) ->
      handler1 = ConfigHandler nil
      handler2 = ConfigHandler nil
      view1 = ConfigView handler1, {"section"}
      view2 = ConfigView handler2, {"section"}
      result, err = view1\isOverlappingView view2
      ut\assertNil result
      ut\assertString err

    isOverlappingView_root: (ut) ->
      handler = ConfigHandler nil
      root = ConfigView handler, {}
      child = ConfigView handler, {"section"}
      ut\assertTrue root\isOverlappingView child

    isOverlappingView_overlap: (ut) ->
      handler = ConfigHandler nil
      parent = ConfigView handler, {"a", "b"}
      child = ConfigView handler, {"a", "b", "c"}
      ut\assertTrue parent\isOverlappingView child

    isOverlappingView_disjoint: (ut) ->
      handler = ConfigHandler nil
      viewA = ConfigView handler, {"a"}
      viewB = ConfigView handler, {"b"}
      ut\assertFalse viewA\isOverlappingView viewB

    -- config proxy: read/write behavior

    config_readUser: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {key: "userValue"}}
      view = ConfigView handler, {"section"}, {key: "defaultValue"}
      ut\assertEquals view.config.key, "userValue"

    config_readDefault: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {}}
      view = ConfigView handler, {"section"}, {key: "defaultValue"}
      ut\assertEquals view.config.key, "defaultValue"

    config_write: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {}}
      view = ConfigView handler, {"section"}
      view.config.newKey = "written"
      ut\assertEquals view.userConfig.newKey, "written"

    -- a partially-populated user section still resolves its unset keys from the section default (the
    -- flat->sectioned config migration leaves partial sections behind, so this guards against nil reads)
    config_partialSectionFallsThrough: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {nested: {a: "userA"}}} -- only 'a' is set within 'nested'
      view = ConfigView handler, {"section"}, {nested: {a: "defA", b: "defB"}}
      ut\assertEquals view.config.nested.a, "userA" -- a user-set key wins
      ut\assertEquals view.config.nested.b, "defB" -- an unset sibling falls through to the default

    -- writing into a partial section targets the user section only, never materializing the other defaults
    config_writeIntoPartialSection: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {nested: {a: "userA"}}}
      view = ConfigView handler, {"section"}, {nested: {a: "defA", b: "defB"}}
      view.config.nested.b = "newB"
      ut\assertEquals view.userConfig.nested.b, "newB" -- written to the user section
      ut\assertEquals view.userConfig.nested.a, "userA" -- the existing user value is preserved
      ut\assertNil rawget view.userConfig.nested, "c" -- an untouched default is not stored

    -- writing into a section absent from the user config stores only the written key; the section's other
    -- defaults stay in code and keep reading through
    config_writeIntoAbsentSection: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {}}
      view = ConfigView handler, {"section"}, {nested: {a: "defA", b: "defB"}}
      view.config.nested.b = "newB"
      ut\assertEquals view.userConfig.nested.b, "newB" -- the user section was created with the written key
      ut\assertNil rawget view.userConfig.nested, "a" -- an untouched default is not materialized
      ut\assertEquals view.config.nested.a, "defA" -- and still reads through

    -- a held section view reads and writes the view's live user config, so it stays valid across a
    -- refresh replacing that table
    config_heldSectionViewSurvivesRefresh: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {nested: {a: "one"}}}
      view = ConfigView handler, {"section"}, {nested: {a: "defA", b: "defB"}}
      held = view.config.nested
      handler.config = {section: {nested: {a: "two"}}}
      view\refresh!
      ut\assertEquals held.a, "two" -- reads the refreshed hive
      held.b = "newB"
      ut\assertEquals view.userConfig.nested.b, "newB" -- writes land in the refreshed hive

    -- regression: constructing a view must not overwrite a populated section that holds table-valued keys
    -- (a load once silently wiped trusted/blocked/extra feeds this way)
    config_populatedSectionSurvivesConstruction: (ut) ->
      handler = ConfigHandler nil
      handler.config = {root: {sect: {list: {"userA"}, mode: "never"}}}
      view = ConfigView handler, {"root"}, {sect: {list: {}, mode: "always"}}
      ut\assertEquals view.userConfig.sect.list, {"userA"} -- user list intact, not replaced by default {}
      ut\assertEquals view.userConfig.sect.mode, "never" -- user scalar intact, not reset to "always"

    -- refresh: re-links userConfig to handler's current hive table

    refresh_success: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {key: "initial"}}
      view = ConfigView handler, {"section"}
      ut\assertEquals view.userConfig.key, "initial"
      handler.config.section = {key: "updated"} -- replace table, not just value
      view\refresh!
      ut\assertEquals view.userConfig.key, "updated"

    -- import

    import_simple: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {}}
      view = ConfigView handler, {"section"}
      changesMade = view\import {key: "value", num: 42}
      ut\assertTrue changesMade
      ut\assertEquals view.userConfig.key, "value"
      ut\assertEquals view.userConfig.num, 42

    import_updateOnly: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {existing: "old"}}
      view = ConfigView handler, {"section"}, {existing: "default"}
      view\import {existing: "new", notExisting: "skip"}, nil, true
      ut\assertEquals view.userConfig.existing, "new"
      ut\assertNil view.userConfig.notExisting

    import_skipPrivate: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {}}
      view = ConfigView handler, {"section"}
      view\import {pub: "ok", _priv: "hidden"}
      ut\assertEquals view.userConfig.pub, "ok"
      ut\assertNil view.userConfig._priv

    -- load / save / delete: stub handler methods, verify delegation

    load_noFilePath: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {}}
      view = ConfigView handler, {"section"}
      ut\assertFalse view\load!

    load_delegatesToHandler: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/test/config.json"
      handler.config = {section: {}}
      loadStub = (ut\stub handler, "load")\returns true
      view = ConfigView handler, {"section"}
      result = view\load 500
      ut\assertTrue result
      loadStub\assertCalledOnce!

    save_noFilePath: (ut) ->
      handler = ConfigHandler nil
      handler.config = {section: {}}
      view = ConfigView handler, {"section"}
      ut\assertFalse view\save!

    save_delegatesToHandler: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/test/config.json"
      handler.config = {section: {}}
      saveStub = (ut\stub handler, "save")\returns true
      view = ConfigView handler, {"section"}
      result = view\save 250
      ut\assertTrue result
      saveStub\assertCalledOnce!

    delete_purgesAndSaves: (ut) ->
      handler = ConfigHandler nil
      handler.filePath = "/test/config.json"
      handler.config = {section: {key: "value"}}
      newHive = {}
      purgeStub = (ut\stub handler, "purgeHive")\returns newHive
      saveStub = (ut\stub handler, "save")\returns true
      view = ConfigView handler, {"section"}
      result = view\delete!
      ut\assertTrue result
      purgeStub\assertCalledOnce!
      saveStub\assertCalledOnce!
      ut\assertEquals view.userConfig, newHive

    -- setFile registers the view with the new handler and detaches it from the old, so the handler's
    -- whole-file refreshes reach the view (it was previously left out of the handler's view set)
    setFile_registersWithNewHandler: (ut) ->
      old = ConfigHandler nil
      view = ConfigView old, {"config"}
      old.views[view] = true -- as getView would register it
      target = "#{aegisub.decode_path '?temp'}/dc_m12b_setFile.json"
      ut\assertTrue view\setFile target
      ut\assertTrue view.__configHandler.views[view] -- registered with the new handler
      ut\assertNil old.views[view] -- and detached from the old one

    -- setFile moves the view's hive into the new handler's tree. A hive left in the old one sits
    -- outside what a save on the new handler merges from, so nothing the view holds reaches the file.
    setFile_movesHiveIntoNewHandler: (ut) ->
      old = ConfigHandler nil
      view = ConfigView old, {"modules", "l0.Pkg"}
      old.views[view] = true
      view.userConfig.name = "written while orphaned"
      target = "#{aegisub.decode_path '?temp'}/dc_m12b_setFileHiveMove.json"
      ut\assertTrue view\setFile target
      handlerHive = view.__configHandler\getHive view.__hivePath
      ut\assertIs view.userConfig, handlerHive -- the very table a save merges from
      view.c.name = "written after binding"
      ut\assertEquals handlerHive.name, "written after binding" -- so a later write lands in that tree

    -- setFile loads a file no handler has read yet, so the view starts out on what that file
    -- stores and a later save adds to it
    setFile_loadsUnreadFile: (ut) ->
      target = "#{aegisub.decode_path '?temp'}/dc_m12b_setFileLoads.json"
      fileOps.writeFile target, [[{"modules": {"l0.Pkg": {"stored": "from the file"}}}]], true
      view = ConfigView ConfigHandler(nil), {"modules", "l0.Pkg"}
      ut\assertTrue view\setFile target
      ut\assertEquals view.c.stored, "from the file"

    -- noLoad keeps the read for the caller to make
    setFile_noLoadLeavesFileUnread: (ut) ->
      target = "#{aegisub.decode_path '?temp'}/dc_m12b_setFileNoLoad.json"
      fileOps.writeFile target, [[{"modules": {"l0.Pkg": {"stored": "from the file"}}}]], true
      view = ConfigView ConfigHandler(nil), {"modules", "l0.Pkg"}
      ut\assertTrue view\setFile target, true
      ut\assertNil view.c.stored
      ut\assertTrue view\load!
      ut\assertEquals view.c.stored, "from the file" -- and the caller's own load still gets there

    _order: {
      "new_orphan", "new_withHandler", "new_stringHivePath", "new_tableHivePath",
      "isOverlappingView_differentHandler", "isOverlappingView_root",
      "isOverlappingView_overlap", "isOverlappingView_disjoint",
      "config_readUser", "config_readDefault", "config_write",
      "config_partialSectionFallsThrough", "config_writeIntoPartialSection",
      "config_writeIntoAbsentSection", "config_heldSectionViewSurvivesRefresh",
      "config_populatedSectionSurvivesConstruction",
      "refresh_success",
      "import_simple", "import_updateOnly", "import_skipPrivate",
      "load_noFilePath", "load_delegatesToHandler",
      "save_noFilePath", "save_delegatesToHandler",
      "delete_purgesAndSaves", "setFile_registersWithNewHandler", "setFile_movesHiveIntoNewHandler",
      "setFile_loadsUnreadFile", "setFile_noLoadLeavesFileUnread"
    }
  }
