-- ModuleLoader tests: internal module loading helpers.
-- Called from Tests.moon as: (require "...test.ModuleLoader")!
->
  constants = require "l0.DependencyControl.Constants"
  domain = require "l0.DependencyControl.domain"
  ModuleLoader = require "l0.DependencyControl.ModuleLoader"
  ModuleProvider = require "l0.DependencyControl.ModuleProvider"
  SemanticVersion = require "l0.DependencyControl.SemanticVersion"
  UpdateTask = require "l0.DependencyControl.UpdateTask"

  DEPCTRL_DUMMY_MODULE_MARKER = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}Dummy"

  {
    _description: "Tests for ModuleLoader internal module loading helpers."

    -- formatVersionErrorTemplate: pure computation, uses SemanticVersion.toString

    formatVersionErrorTemplate_missing_bare: (ut) ->
      result = ModuleLoader.formatVersionErrorTemplate nil, "MyModule", nil, nil, "not found"
      ut\assertString result
      ut\assertContains result, "MyModule"
      ut\assertContains result, "not found"

    formatVersionErrorTemplate_missing_withVersion: (ut) ->
      result = ModuleLoader.formatVersionErrorTemplate nil, "MyModule", "1.0.0", nil, "not found"
      ut\assertContains result, "(v1.0.0)"

    formatVersionErrorTemplate_missing_withUrl: (ut) ->
      result = ModuleLoader.formatVersionErrorTemplate nil, "MyModule", nil, "http://example.com", "not found"
      ut\assertContains result, ": http://example.com"

    formatVersionErrorTemplate_outdated_scalarRef: (ut) ->
      ref = {version: 65793} -- 1*65536 + 1*256 + 1 = "1.1.1" in base-256 encoding
      result = ModuleLoader.formatVersionErrorTemplate nil, "MyModule", "2.0.0", nil, "too old", ref
      ut\assertContains result, "Installed:"
      ut\assertContains result, "Required: v2.0.0"
      ut\assertContains result, "1.1.1"

    formatVersionErrorTemplate_outdated_tableRef: (ut) ->
      ref = {version: {version: 65793}} -- 1*65536 + 1*256 + 1 = "1.1.1" in base-256 encoding
      result = ModuleLoader.formatVersionErrorTemplate nil, "MyModule", "2.0.0", nil, "too old", ref
      ut\assertContains result, "Installed:"
      ut\assertContains result, "1.1.1"

    -- createDummyRef: tests LOADED_MODULES manipulation

    createDummyRef_nonModule: (ut) ->
      rec = {scriptType: domain.ScriptType.Automation, __class: {ScriptType: domain.ScriptType}}
      result = ModuleLoader.createDummyRef rec
      ut\assertNil result

    createDummyRef_newRef: (ut) ->
      ns = "test.ModuleLoader.createNew"
      rec = {scriptType: domain.ScriptType.Module, namespace: ns, __class: {ScriptType: domain.ScriptType}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = nil
      result = ModuleLoader.createDummyRef rec
      ut\assertTrue result
      ut\assertNotNil LOADED_MODULES[ns]
      ut\assertTrue LOADED_MODULES[ns][DEPCTRL_DUMMY_MODULE_MARKER]
      LOADED_MODULES[ns] = nil

    createDummyRef_existingRef: (ut) ->
      ns = "test.ModuleLoader.createExisting"
      rec = {scriptType: domain.ScriptType.Module, namespace: ns, __class: {ScriptType: domain.ScriptType}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = {existing: true}
      result = ModuleLoader.createDummyRef rec
      ut\assertFalse result
      LOADED_MODULES[ns] = nil

    -- removeDummyRef: tests LOADED_MODULES manipulation

    removeDummyRef_nonModule: (ut) ->
      rec = {scriptType: domain.ScriptType.Automation, __class: {ScriptType: domain.ScriptType}}
      result = ModuleLoader.removeDummyRef rec
      ut\assertNil result

    removeDummyRef_dummy: (ut) ->
      ns = "test.ModuleLoader.removeDummy"
      rec = {scriptType: domain.ScriptType.Module, namespace: ns, __class: {ScriptType: domain.ScriptType}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = {[DEPCTRL_DUMMY_MODULE_MARKER]: true}
      result = ModuleLoader.removeDummyRef rec
      ut\assertTrue result
      ut\assertNil LOADED_MODULES[ns]

    removeDummyRef_nonDummy: (ut) ->
      ns = "test.ModuleLoader.removeNonDummy"
      rec = {scriptType: domain.ScriptType.Module, namespace: ns, __class: {ScriptType: domain.ScriptType}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = {[DEPCTRL_DUMMY_MODULE_MARKER]: false}
      result = ModuleLoader.removeDummyRef rec
      ut\assertFalse result
      LOADED_MODULES[ns] = nil

    -- loadModule: stubs require, controls LOADED_MODULES

    loadModule_cached: (ut) ->
      ns = "test.ModuleLoader.cached"
      mockRef = {loaded: true}
      mdl = {moduleName: ns}
      rec = {namespace: "host.Module", __class: {ScriptType: domain.ScriptType, __name: "DependencyControl"}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = mockRef
      result = ModuleLoader.loadModule rec, mdl, false, false
      ut\assertEquals result, mockRef
      LOADED_MODULES[ns] = nil

    loadModule_success: (ut) ->
      ns = "test.ModuleLoader.success"
      mockRef = {loaded: true}
      mdl = {moduleName: ns}
      rec = {namespace: "host.Module", __class: {ScriptType: domain.ScriptType, __name: "DependencyControl"}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = nil
      (ut\stub _G, "require")\calls (name) -> mockRef
      result = ModuleLoader.loadModule rec, mdl, false, false
      ut\assertEquals result, mockRef
      ut\assertEquals mdl._ref, mockRef
      LOADED_MODULES[ns] = nil

    loadModule_missing: (ut) ->
      ns = "test.ModuleLoader.missing"
      mdl = {moduleName: ns}
      rec = {namespace: "host.Module", __class: {ScriptType: domain.ScriptType, __name: "DependencyControl"}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = nil
      (ut\stub _G, "require")\calls (name) -> error "module '#{name}' not found: no such file"
      result = ModuleLoader.loadModule rec, mdl, false, false
      ut\assertNil result
      ut\assertTrue mdl._missing
      ut\assertNil mdl._error

    loadModule_error: (ut) ->
      ns = "test.ModuleLoader.error"
      mdl = {moduleName: ns}
      rec = {namespace: "host.Module", __class: {ScriptType: domain.ScriptType, __name: "DependencyControl"}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = nil
      (ut\stub _G, "require")\calls (name) -> error "syntax error in module"
      result = ModuleLoader.loadModule rec, mdl, false, false
      ut\assertNil result
      ut\assertFalse mdl._missing
      ut\assertString mdl._error

    -- a reload purges the module's cached submodules so the fresh chunk can't be stitched
    -- to code from the replaced version; a module merely sharing the name prefix is kept.
    loadModule_reloadPurgesSubmoduleCache: (ut) ->
      ns = "test.ModuleLoader.reloadPurge"
      mockRef = {reloaded: true}
      mdl = {moduleName: ns}
      rec = {namespace: "host.Module", __class: {ScriptType: domain.ScriptType, __name: "DependencyControl"}}
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = {stale: true}
      package.loaded[ns] = {stale: true}
      package.loaded["#{ns}.Helper"] = {stale: true}
      package.loaded["#{ns}.sub.Deep"] = {stale: true}
      package.loaded["#{ns}Sibling"] = {sibling: true} -- shares the name prefix but not the namespace
      (ut\stub _G, "require")\calls (name) -> mockRef
      result = ModuleLoader.loadModule rec, mdl, false, true
      ut\assertEquals result, mockRef
      ut\assertNil package.loaded["#{ns}.Helper"]
      ut\assertNil package.loaded["#{ns}.sub.Deep"]
      ut\assertNotNil package.loaded["#{ns}Sibling"]
      package.loaded[ns], package.loaded["#{ns}Sibling"], LOADED_MODULES[ns] = nil

    -- loadModules: stubs loadModule to control loading behavior

    loadModules_skipsModule: (ut) ->
      ns = "test.ModuleLoader.skip"
      mdl = {moduleName: ns}
      loadModuleStub = ut\stub ModuleLoader, "loadModule"
      rec = {moduleName: "host.Module", feed: nil, name: "host",
        __class: {ScriptType: domain.ScriptType, __name: "DependencyControl", updater: nil}}
      success, err = ModuleLoader.loadModules rec, {mdl}, nil, {[ns]: true}
      ut\assertTrue success
      ut\assertEquals err, ""
      loadModuleStub\assertNotCalled!

    loadModules_allLoaded: (ut) ->
      ns = "test.ModuleLoader.allLoaded"
      mockRef = {loaded: true}
      mdl = {moduleName: ns, version: nil, name: ns}
      rec = {namespace: "host.Module", moduleName: "host.Module", feed: nil, name: "host",
        __class: {ScriptType: domain.ScriptType, __name: "DependencyControl", updater: nil}}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) ->
        m._ref = mockRef unless usePrivate
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertTrue success
      ut\assertEquals err, ""

    -- loadModules: a missing module is fetched through the updater; on success it's marked updated.
    -- @@ (the host record's class) must be callable (constructs the to-fetch record) and carry an
    -- `updater` whose `require` performs the fetch.
    loadModules_missingFetchedViaUpdater: (ut) ->
      ns = "test.ModuleLoader.missingFetch"
      mockRef = {fetched: true}
      updater = {require: ((...) => mockRef)}
      recClass = setmetatable {ScriptType: domain.ScriptType, __name: "DependencyControl", :updater},
        {__call: (cls, args) -> {}}
      rec = {feed: nil, moduleName: "host.Module", name: "host", __class: recClass}
      mdl = {moduleName: ns, name: ns, version: nil}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) -> m._missing = true unless usePrivate
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertTrue success
      ut\assertEquals err, ""
      ut\assertTrue mdl._updated
      ut\assertFalse mdl._missing

    -- loadModules: a missing *required* module the updater can't fetch fails, and the circular-dependency
    -- dummy ref is cleared.
    loadModules_missingRequiredFails: (ut) ->
      ns = "test.ModuleLoader.missingFail"
      updater = {require: ((...) => return nil, UpdateTask.UpdateStatus.NoSuitablePackage, "no feed")}
      recClass = setmetatable {ScriptType: domain.ScriptType, __name: "DependencyControl", :updater},
        {__call: (cls, args) -> {}}
      rec = {feed: nil, moduleName: "host.Module", name: "host", __class: recClass}
      mdl = {moduleName: ns, name: ns, version: nil, optional: false}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) -> m._missing = true unless usePrivate
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = {dummy: true}
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertFalse success
      ut\assertContains err, ns
      ut\assertContains err, "no feed" -- the updater's detail reaches the user through the real formatter
      ut\assertNil LOADED_MODULES[ns] -- dummy ref nuked

    -- loadModules: a missing *optional* module the updater skips is left missing without an error
    -- reason and doesn't fail the overall load; the circular-dependency dummy ref is still cleared.
    loadModules_missingOptionalSkipped: (ut) ->
      ns = "test.ModuleLoader.missingOptionalSkip"
      updater = {require: ((...) => return nil, UpdateTask.UpdateStatus.SkippedOptional)}
      recClass = setmetatable {ScriptType: domain.ScriptType, __name: "DependencyControl", :updater},
        {__call: (cls, args) -> {}}
      rec = {feed: nil, moduleName: "host.Module", name: "host", __class: recClass}
      mdl = {moduleName: ns, name: ns, version: nil, optional: true}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) -> m._missing = true unless usePrivate
      LOADED_MODULES = LOADED_MODULES or {}
      LOADED_MODULES[ns] = {dummy: true}
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertTrue success
      ut\assertEquals err, ""
      ut\assertNil mdl._reason
      ut\assertNil LOADED_MODULES[ns] -- dummy ref nuked

    -- loadModules: a required module that fails because one of ITS OWN requirements couldn't be satisfied
    -- surfaces the nested reason (which sub-requirement failed, and why) in the error, so the
    -- RequirementsUnmet template's detail isn't dropped on the way to the UI.
    loadModules_requirementsUnmetSurfacesNestedReason: (ut) ->
      ns = "l0.ASSFoundation"
      innerReason = "— SubInspector.Inspector (v0.7.2)\n—— Reason: no build for your platform (Linux-x64)"
      updater = {require: ((...) => return nil, UpdateTask.UpdateStatus.RequirementsUnmet, innerReason)}
      recClass = setmetatable {ScriptType: domain.ScriptType, __name: "DependencyControl", :updater},
        {__call: (cls, args) -> {}}
      rec = {feed: nil, moduleName: "host.Module", name: "Vector Gradient", __class: recClass}
      mdl = {moduleName: ns, name: ns, version: "0.5.0", optional: false}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) -> m._missing = true unless usePrivate
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertFalse success
      ut\assertContains err, "requirements could not be satisfied"
      ut\assertContains err, "SubInspector.Inspector"
      ut\assertContains err, "no build for your platform"

    -- loadModules: an outdated installed module is force-updated through the updater; the fresh ref
    -- replaces the loaded one. (isDepCtrlVersionRecord is stubbed so the loaded version record is used
    -- as-is rather than wrapped in an unmanaged record.)
    loadModules_outdatedForcesUpdate: (ut) ->
      ns = "test.ModuleLoader.outdated"
      newRef = {updated: true}
      loadedRef = {version: {version: 65793, checkVersion: ((target) => false)}} -- installed but too old
      updater = {require: ((...) => newRef)}
      recClass = setmetatable {ScriptType: domain.ScriptType, __name: "DependencyControl", :updater},
        {__call: (cls, args) -> {}}
      rec = {feed: nil, moduleName: "host.Module", name: "host", __class: recClass}
      mdl = {moduleName: ns, name: ns, version: SemanticVersion\toPacked "2.0.0"}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) -> m._ref = loadedRef unless usePrivate
      ut\stub(ModuleProvider, "isDepCtrlVersionRecord")\returns true
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertTrue success
      ut\assertEquals err, ""
      ut\assertEquals mdl._ref, newRef

    -- loadModules: an outdated *required* module the updater can't update fails with an "outdated" error
    loadModules_outdatedRequiredFails: (ut) ->
      ns = "test.ModuleLoader.outdatedFail"
      loadedRef = {version: {version: 65793, checkVersion: ((target) => false)}}
      updater = {require: ((...) => return nil, UpdateTask.UpdateStatus.NoSuitablePackage, "no newer version")}
      recClass = setmetatable {ScriptType: domain.ScriptType, __name: "DependencyControl", :updater},
        {__call: (cls, args) -> {}}
      rec = {feed: nil, moduleName: "host.Module", name: "host", __class: recClass}
      mdl = {moduleName: ns, name: ns, version: SemanticVersion\toPacked "2.0.0", optional: false}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) -> m._ref = loadedRef unless usePrivate
      ut\stub(ModuleProvider, "isDepCtrlVersionRecord")\returns true
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertFalse success
      ut\assertContains err, ns
      ut\assertContains err, "no newer version" -- the updater's detail reaches the user through the real formatter

    -- loadModules: a requirement caught in a dependency cycle doesn't fail the load — the module already
    -- loaded keeps its place and isn't reported as outdated
    loadModules_updateInProgressAcceptsLoadedRef: (ut) ->
      ns = "test.ModuleLoader.inFlight"
      loadedRef = {version: {version: 65793, checkVersion: ((target) => false)}}
      updater = {require: ((...) => return nil, UpdateTask.UpdateStatus.UpdateInProgress, "2.0.0")}
      recClass = setmetatable {ScriptType: domain.ScriptType, __name: "DependencyControl", :updater},
        {__call: (cls, args) -> {}}
      rec = {feed: nil, moduleName: "host.Module", name: "host", __class: recClass}
      mdl = {moduleName: ns, name: ns, version: SemanticVersion\toPacked "2.0.0", optional: false}
      (ut\stub ModuleLoader, "loadModule")\calls (self, m, usePrivate) -> m._ref = loadedRef unless usePrivate
      ut\stub(ModuleProvider, "isDepCtrlVersionRecord")\returns true
      success, err = ModuleLoader.loadModules rec, {mdl}
      ut\assertTrue success
      ut\assertEquals err, ""
      ut\assertEquals mdl._ref, loadedRef
      ut\assertNil mdl._outdated

    -- checkOptionalModules: mock self with requiredModules

    checkOptionalModules_noneOptional: (ut) ->
      rec = {
        name: "test"
        requiredModules: {{moduleName: "SomeModule", name: "SomeModule", optional: false}}
        __class: {ScriptType: domain.ScriptType, automationDir: {modules: "include"}}
      }
      result, err = ModuleLoader.checkOptionalModules rec, {"SomeModule"}
      ut\assertTrue result
      ut\assertNil err

    checkOptionalModules_missingOptional: (ut) ->
      rec = {
        name: "test"
        requiredModules: {
          {moduleName: "MissingMod", name: "MissingMod", optional: true, _missing: true,
            _reason: "not found", version: nil, url: nil}
        }
        __class: {ScriptType: domain.ScriptType, automationDir: {modules: "include"}}
      }
      result, err = ModuleLoader.checkOptionalModules rec, {"MissingMod"}
      ut\assertFalse result
      ut\assertString err
      ut\assertContains err, "MissingMod"

    _order: {
      "formatVersionErrorTemplate_missing_bare", "formatVersionErrorTemplate_missing_withVersion",
      "formatVersionErrorTemplate_missing_withUrl",
      "formatVersionErrorTemplate_outdated_scalarRef", "formatVersionErrorTemplate_outdated_tableRef",
      "createDummyRef_nonModule", "createDummyRef_newRef", "createDummyRef_existingRef",
      "removeDummyRef_nonModule", "removeDummyRef_dummy", "removeDummyRef_nonDummy",
      "loadModule_cached", "loadModule_success", "loadModule_missing", "loadModule_error",
      "loadModule_reloadPurgesSubmoduleCache",
      "loadModules_skipsModule", "loadModules_allLoaded",
      "loadModules_missingFetchedViaUpdater", "loadModules_missingRequiredFails",
      "loadModules_missingOptionalSkipped", "loadModules_requirementsUnmetSurfacesNestedReason",
      "loadModules_outdatedForcesUpdate", "loadModules_outdatedRequiredFails",
      "loadModules_updateInProgressAcceptsLoadedRef",
      "checkOptionalModules_noneOptional", "checkOptionalModules_missingOptional"
    }
  }
