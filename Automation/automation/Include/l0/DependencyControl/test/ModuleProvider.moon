-- ModuleProvider tests: alias registration, searcher-based resolution, and the shared
-- __depCtrlInit runner. Called from test.moon as: (require "...test.ModuleProvider") basePath, DepCtrl
-- (Names are unique per run since the provider registry is process-global.)
(basePath, DepCtrl) ->
  constants = require "l0.DependencyControl.Constants"
  ModuleProvider = require "l0.DependencyControl.ModuleProvider"
  SemanticVersion = require "l0.DependencyControl.SemanticVersion"

  DEPCTRL_MODULE_INIT_HOOK_NAME = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}Init"
  uniqueName = (prefix) -> "#{prefix}_#{'%08X'\format math.random 0, 16^8-1}"

  -- Minimal fake of the DependencyControl class, sufficient for tests that pass it as the
  -- `DependencyControl` argument to runInitializer (or need to verify it was forwarded).
  makeDepCtrlClassMock = -> {__name: constants.DEPCTRL_NAME}

  -- Minimal table that satisfies ModuleProvider.isDepCtrlVersionRecord without
  -- creating a real DependencyControl record (which has config/registry side effects).
  makeDepCtrlRecordMock = -> {__class: makeDepCtrlClassMock!, checkVersion: ->}

  {
    _description: "Tests for ModuleProvider: alias registration, searcher resolution, and the shared __depCtrlInit runner."

    register_andGetProvider: (ut) ->
      name = uniqueName "alias"
      ut\assertTrue ModuleProvider\register name, "some.provider"
      ut\assertEquals ModuleProvider\getProvider(name), "some.provider"

    register_firstWins: (ut) ->
      name = uniqueName "alias"
      ut\assertTrue ModuleProvider\register name, "first.provider"
      ut\assertFalse ModuleProvider\register name, "second.provider" -- already registered
      ut\assertEquals ModuleProvider\getProvider(name), "first.provider"

    registerRecord_normalizesAliases: (ut) ->
      stringAlias, tableAlias = uniqueName("string"), uniqueName "table"
      ModuleProvider\registerRecord {moduleName: "prov.A", provides: {stringAlias}}
      ModuleProvider\registerRecord {moduleName: "prov.B", provides: {{name: tableAlias}}}
      ut\assertEquals ModuleProvider\getProvider(stringAlias), "prov.A"
      ut\assertEquals ModuleProvider\getProvider(tableAlias), "prov.B"

    -- end to end: a require of a registered alias resolves to the provider module
    searcher_resolvesAliasToProvider: (ut) ->
      ModuleProvider\install! -- idempotent; already installed during load
      name = uniqueName "aliasToSemver"
      ModuleProvider\register name, "l0.DependencyControl.SemanticVersion"
      resolved = require name
      package.loaded[name] = nil -- don't leak the alias into the module cache
      ut\assertIs resolved, SemanticVersion

    -- runInitializer: shared __depCtrlInit guard + call (also used by ModuleLoader & UpdateFeed)

    -- a module with no init hook is a no-op: returns false without touching the module
    runInitializer_noInitHook: (ut) ->
      ref = {version: "1.0.0"}
      ut\assertFalse ModuleProvider.runInitializer(ref, makeDepCtrlClassMock!)

    -- an uninitialized module (raw .version) gets its initializer run with the DepCtrl class
    runInitializer_runsWhenUninitialized: (ut) ->
      dcMock = makeDepCtrlClassMock!
      ref = {version: "raw-version-string"}
      initStub = ut\stub ref, DEPCTRL_MODULE_INIT_HOOK_NAME
      ModuleProvider.runInitializer ref, dcMock
      initStub\assertCalledOnceWith dcMock

    -- a module whose .version is already a DepCtrl record must NOT be re-initialized
    runInitializer_skipsWhenInitialized: (ut) ->
      ref = {version: makeDepCtrlRecordMock!}
      initStub = ut\stub ref, DEPCTRL_MODULE_INIT_HOOK_NAME
      ModuleProvider.runInitializer ref, makeDepCtrlClassMock!
      initStub\assertNotCalled!

    -- a failing initializer reports the module's name and the real error
    runInitializer_reportsErrorWithModuleName: (ut) ->
      ref = {version: "raw", moduleName: "l0.Exploder"}
      (ut\stub ref, DEPCTRL_MODULE_INIT_HOOK_NAME)\calls -> error "boom"
      result, err = ModuleProvider.runInitializer ref, makeDepCtrlClassMock!
      ut\assertNil result
      ut\assertContains err, "l0.Exploder"
      ut\assertContains err, "boom"

    _order: {
      "register_andGetProvider", "register_firstWins",
      "registerRecord_normalizesAliases", "searcher_resolvesAliasToProvider",
      "runInitializer_noInitHook", "runInitializer_runsWhenUninitialized",
      "runInitializer_skipsWhenInitialized", "runInitializer_reportsErrorWithModuleName"
    }
  }
