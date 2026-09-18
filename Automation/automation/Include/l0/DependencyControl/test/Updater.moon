-- Updater tests.
-- Called from test.moon as: (controls\requireTest "Updater") stubHelpers
(stubHelpers) ->
  Updater = require "l0.DependencyControl.Updater"
  domain = require "l0.DependencyControl.domain"
  ModuleLoader = require "l0.DependencyControl.ModuleLoader"
  SemanticVersion = require "l0.DependencyControl.SemanticVersion"
  Lock = require "l0.DependencyControl.Lock"
  UpdateTask = require "l0.DependencyControl.UpdateTask"
  DependencyControl = require "l0.DependencyControl"
  {:stubSelf, :makeNullLogger} = stubHelpers
  UpdateStatus = UpdateTask.UpdateStatus
  UpdateReason = UpdateTask.UpdateReason
  ContextCeiling = UpdateTask.ContextCeiling

  -- A stub updater self for require(): @addTask returns the supplied task (whose run() yields the
  -- scripted code/detail), so require's dispatch is exercised without constructing a real UpdateTask.
  makeRequireUpdater = (task) ->
    stubSelf Updater, {
      logger: makeNullLogger!
      addTask: ((record, tv, af, opt, ch, rsn) => task)
    }

  -- A stub updater self for scheduleUpdate(): its config gates the run and @addTask returns opts.task.
  makeScheduleUpdater = (opts = {}) ->
    stubSelf Updater, {
      config: {c: {updates: {mode: opts.mode, checkInterval: opts.updateInterval or 0}}}
      logger: makeNullLogger!
      addTask: ((record) => opts.task)
    }

  {
    _description: "Tests for Updater: require/scheduleUpdate dispatch, addTask, and lock handling."

    -- require: dispatches on the task's run() result.

    -- run reports up-to-date for a module that wasn't (re)installed → load the existing module
    require_upToDateLoadsModule: (ut) ->
      loadedRef = {loaded: true}
      loadStub = ut\stub(ModuleLoader, "loadModule")\returns loadedRef
      task = {updated: false, ref: {wrong: true}, record: {namespace: "l0.dep", name: "Dep"}, run: ((wait) => UpdateStatus.UpToDate)}
      record = {scriptType: domain.ScriptType.Module, name: "Dep", namespace: "l0.dep", virtual: false}
      ut\assertEquals (Updater.require makeRequireUpdater(task), record, 0), loadedRef
      -- loadModule must receive the record as its module spec, not the namespace string (a string would crash)
      loadStub\assertCalledWith task.record, task.record

    -- a successful (re)install returns the task's freshly-loaded ref along with the status code
    require_successReturnsRef: (ut) ->
      task = {updated: true, ref: {the: "ref"}, record: {namespace: "l0.dep", name: "Dep"}, run: ((wait) => UpdateStatus.Installed)}
      record = {scriptType: domain.ScriptType.Module, name: "Dep", namespace: "l0.dep", virtual: false}
      ref, code = Updater.require makeRequireUpdater(task), record, 0
      ut\assertEquals ref, task.ref
      ut\assertEquals code, UpdateStatus.Installed

    -- a cycle's in-progress update has no ref to hand back yet, but its status isn't a failure and names
    -- the version being installed
    require_updateInProgressReturnsCode: (ut) ->
      task = {updated: false, record: {namespace: "l0.dep", name: "Dep"},
        run: ((wait) => UpdateStatus.UpdateInProgress, "1.3.6")}
      record = {scriptType: domain.ScriptType.Module, name: "Dep", namespace: "l0.dep", virtual: false}
      ref, code, detail = Updater.require makeRequireUpdater(task), record, 0
      ut\assertNil ref
      ut\assertEquals code, UpdateStatus.UpdateInProgress
      ut\assertEquals detail, "1.3.6"

    -- a skipped optional dependency yields no ref but still carries its status code
    require_skippedOptionalReturnsCode: (ut) ->
      task = {updated: false, record: {namespace: "l0.dep", name: "Dep"}, run: ((wait) => UpdateStatus.SkippedOptional)}
      record = {scriptType: domain.ScriptType.Module, name: "Dep", namespace: "l0.dep", virtual: true}
      ref, code = Updater.require makeRequireUpdater(task), record, 0
      ut\assertNil ref
      ut\assertEquals code, UpdateStatus.SkippedOptional

    -- run reports up-to-date but the module then fails to load: the paradox surfaces as a code and detail
    require_upToDateLoadFailureReturnsCode: (ut) ->
      ut\stub(ModuleLoader, "loadModule")\calls -> nil
      task = {updated: false, record: {namespace: "l0.dep", name: "Dep"}, run: ((wait) => UpdateStatus.UpToDate)}
      record = {scriptType: domain.ScriptType.Module, name: "Dep", namespace: "l0.dep", virtual: false}
      ref, code, detail = Updater.require makeRequireUpdater(task), record, 0
      ut\assertNil ref
      ut\assertEquals code, UpdateStatus.UpToDate
      ut\assertString detail

    -- an update error (negative code) is passed through to the caller
    require_errorPropagates: (ut) ->
      task = {updated: false, ref: {}, record: {namespace: "l0.dep", name: "Dep"}, run: ((wait) => return UpdateStatus.NoSuitablePackage, "boom")}
      record = {scriptType: domain.ScriptType.Module, name: "Dep", namespace: "l0.dep", virtual: false}
      ref, code, detail = Updater.require makeRequireUpdater(task), record, 0
      ut\assertNil ref
      ut\assertEquals code, UpdateStatus.NoSuitablePackage
      ut\assertEquals detail, "boom"

    -- scheduleUpdate: guards, then runs a due update.

    scheduleUpdate_disabledRejected: (ut) ->
      updater = makeScheduleUpdater {mode: ContextCeiling.Off}
      ut\assertEquals (Updater.scheduleUpdate updater, {name: "X", namespace: "l0.x"}), UpdateStatus.UpdaterDisabled

    -- background checks sit on the auto-update rung: any lower mode refuses them
    scheduleUpdate_belowAutoUpdateModeRejected: (ut) ->
      updater = makeScheduleUpdater {mode: ContextCeiling.DependencyResolution}
      ut\assertEquals (Updater.scheduleUpdate updater, {name: "X", namespace: "l0.x"}), UpdateStatus.UpdaterDisabled

    scheduleUpdate_virtualRejected: (ut) ->
      updater = makeScheduleUpdater {mode: ContextCeiling.AutoUpdate}
      ut\assertEquals (Updater.scheduleUpdate updater, {virtual: true, name: "X", namespace: "l0.x"}), UpdateStatus.Unmanaged

    scheduleUpdate_withinIntervalSkips: (ut) ->
      updater = makeScheduleUpdater {mode: ContextCeiling.AutoUpdate, updateInterval: 100000}
      record = {virtual: false, name: "X", namespace: "l0.x", config: {c: {lastUpdateCheck: os.time!}}}
      ut\assertEquals (Updater.scheduleUpdate updater, record), UpdateStatus.UpToDate

    -- the entry point is in Aegisub's ?data automation dir (isUserPath false) → don't shadow it
    scheduleUpdate_protectedInstallRejected: (ut) ->
      updater = makeScheduleUpdater {mode: ContextCeiling.AutoUpdate, updateInterval: 0}
      record = {
        virtual: false, name: "X", namespace: "l0.x", scriptType: domain.ScriptType.Module
        config: {c: {}, save: (=>)}
        getEntryPointPath: (=> "data/path", false)
      }
      code, path = Updater.scheduleUpdate updater, record
      ut\assertEquals code, UpdateStatus.ProtectedInstall
      ut\assertEquals path, "data/path"

    scheduleUpdate_runsTaskWhenDue: (ut) ->
      task = {run: (=> UpdateStatus.Installed)}
      updater = makeScheduleUpdater {mode: ContextCeiling.AutoUpdate, updateInterval: 0, :task}
      record = {
        virtual: false, name: "X", namespace: "l0.x", scriptType: domain.ScriptType.Module
        config: {c: {}, save: (=>)}
        getEntryPointPath: (=> "user/path", true)
      }
      ut\assertEquals (Updater.scheduleUpdate updater, record), UpdateStatus.Installed

    -- acquireLock / releaseLock / renewLock: the lock state machine.

    acquireLock_returnsTrueWhenAlreadyHeld: (ut) ->
      updater = stubSelf Updater, {hasLock: true, config: {c: {updates: {waitTimeout: 5}}}}
      ut\assertTrue Updater.acquireLock updater, false

    acquireLock_acquiresAndSetsHasLock: (ut) ->
      fakeLock = {lock: ((timeout) => Lock.LockState.Held, 0), getActiveHolder: (=>)}
      updater = stubSelf Updater, {
        hasLock: false, config: {c: {updates: {waitTimeout: 5}}}, logger: makeNullLogger!
        tasks: {[domain.ScriptType.Module]: {}}
        feedLoader: {cache: {expireAll: ->}}
        lock: fakeLock -- pre-set so the lazy `@lock or= Lock{…}` in acquireLock skips construction
      }
      ut\assertTrue Updater.acquireLock updater, false
      ut\assertTrue updater.hasLock

    acquireLock_failsWhenHeldByOther: (ut) ->
      fakeLock = {lock: ((timeout) => Lock.LockState.Unavailable, 0), getActiveHolder: (=> {holderName: "OtherScript"})}
      updater = stubSelf Updater, {
        hasLock: false, config: {c: {updates: {waitTimeout: 5}}}, logger: makeNullLogger!
        lock: fakeLock
      }
      ok, owner = Updater.acquireLock updater, false
      ut\assertFalse ok
      ut\assertEquals owner, "OtherScript"

    releaseLock_releasesWhenHeld: (ut) ->
      released = {}
      updater = stubSelf Updater, {hasLock: true, lock: {release: (=> released.called = true)}}
      ut\assertTrue Updater.releaseLock updater
      ut\assertFalse updater.hasLock
      ut\assertTrue released.called

    releaseLock_noopWhenNotHeld: (ut) ->
      updater = stubSelf Updater, {hasLock: false}
      ut\assertFalse Updater.releaseLock updater

    renewLock_renewsWhenHeld: (ut) ->
      renewed = {}
      updater = stubSelf Updater, {hasLock: true, lock: {renew: (=> renewed.called = true)}}
      Updater.renewLock updater
      ut\assertTrue renewed.called

    -- addTask: version parsing and task caching.

    addTask_versionParseErrorReturns: (ut) ->
      updater = stubSelf Updater, {tasks: {}}
      record = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "l0.x"}
      task, code, err = Updater.addTask updater, record, "not-a-version"
      ut\assertNil task
      ut\assertEquals code, UpdateStatus.InvalidVersion
      ut\assertNotNil err

    -- a record with a queued task updates that task in place rather than creating a new one
    addTask_updatesExistingTask: (ut) ->
      existing = {targetVersion: 0}
      record = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "l0.x"}
      updater = stubSelf Updater, {
        tasks: {[domain.ScriptType.Module]: {[record.namespace]: existing}}
      }
      task = Updater.addTask updater, record, "2.0.0", {"feed://a"}, true
      ut\assertIs task, existing
      ut\assertEquals existing.targetVersion, SemanticVersion\toPacked "2.0.0"
      ut\assertTrue existing.optional

    -- Breaking a dependency cycle depends on the re-entrant lookup reaching the very task doing the update,
    -- which the namespace key gives us for either record the loader passes — the loaded module's own on the
    -- outdated path, a fresh virtual one on the missing path. Updating that task in place must not wipe the
    -- version it is installing.
    addTask_reentrantResolutionReachesRunningTask: (ut) ->
      running = {targetVersion: 0, __installingVersion: "1.3.6"}
      updater = stubSelf Updater, {tasks: {[domain.ScriptType.Module]: {["a-mo.Tags"]: running}}}
      outdated = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "a-mo.Tags"}
      virtual = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "a-mo.Tags", virtual: true}
      ut\assertIs (Updater.addTask updater, outdated, "1.3.6"), running
      ut\assertIs (Updater.addTask updater, virtual, "1.3.6"), running
      ut\assertEquals running.__installingVersion, "1.3.6"

    -- a record with no queued task gets a fresh UpdateTask, which is cached under its scriptType/namespace
    addTask_createsNewTask: (ut) ->
      record = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "l0.new", validateNamespace: => true}
      updater = stubSelf Updater, {
        tasks: {[domain.ScriptType.Module]: {}}
        logger: makeNullLogger!
        config: {c: {updates: {mode: ContextCeiling.AutoUpdate}, paths: {cache: "?user/cache"}}}
      }
      task = Updater.addTask updater, record, "1.0.0"
      ut\assertNotNil task
      ut\assertIs task.__class, UpdateTask
      ut\assertIs updater.tasks[domain.ScriptType.Module][record.namespace], task

    -- addTask rejects creation for a disabled updater or an invalid namespace: a constructor's return value is
    -- discarded, so the guards live in addTask rather than UpdateTask.new
    addTask_disabledUpdaterRejects: (ut) ->
      record = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "l0.new", validateNamespace: => true}
      updater = stubSelf Updater, {
        tasks: {[domain.ScriptType.Module]: {}}, config: {c: {updates: {mode: ContextCeiling.Off}}}
      }
      task, code = Updater.addTask updater, record, "1.0.0"
      ut\assertNil task
      ut\assertEquals code, UpdateStatus.UpdaterDisabled

    -- the update mode gates addTask by the task's reason: a user-requested action still passes a mode
    -- that blocks background checks
    addTask_modeGatesByReason: (ut) ->
      record = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "l0.new", validateNamespace: => true}
      makeUpdater = -> stubSelf Updater, {
        tasks: {[domain.ScriptType.Module]: {}}
        logger: makeNullLogger!
        config: {c: {updates: {mode: ContextCeiling.UserRequested}, paths: {cache: "?user/cache"}}}
      }
      task, code = Updater.addTask makeUpdater!, record, "1.0.0" -- default reason: AutoUpdate
      ut\assertNil task
      ut\assertEquals code, UpdateStatus.UpdaterDisabled
      task = Updater.addTask makeUpdater!, record, "1.0.0", nil, nil, nil, UpdateReason.UserRequested
      ut\assertNotNil task

    addTask_invalidNamespaceRejects: (ut) ->
      record = {__class: DependencyControl, scriptType: domain.ScriptType.Module, namespace: "bad ns", validateNamespace: => false}
      updater = stubSelf Updater, {
        tasks: {[domain.ScriptType.Module]: {}}, config: {c: {updates: {mode: ContextCeiling.AutoUpdate}}}
      }
      task, code = Updater.addTask updater, record, "1.0.0"
      ut\assertNil task
      ut\assertEquals code, UpdateStatus.InvalidNamespace

    -- the feed-trust model is a process-wide singleton: every Updater shares the one instance
    feedTrust_isSharedSingleton: (ut) ->
      cfg = {c: {feeds: {extraFeeds: {}, trustedFeeds: {}, blockedFeeds: {}}}}
      a, b = Updater("hostA", cfg), Updater("hostB", cfg)
      ut\assertIs a.feedTrust, b.feedTrust

    -- an unset `updates.mode` defaults to auto-update: every context is enabled
    isEnabledFor_defaultsToAllContexts: (ut) ->
      make = (mode) -> stubSelf Updater, {config: {c: {updates: {:mode}}}}
      for reason in *{UpdateReason.UserRequested, UpdateReason.DependencyResolution, UpdateReason.AutoUpdate}
        ut\assertTrue Updater.__isEnabledFor make!, reason

    -- each mode enables exactly the contexts at or below its rung; off enables none
    isEnabledFor_modeGatesByContext: (ut) ->
      make = (mode) -> stubSelf Updater, {config: {c: {updates: {:mode}}}}
      ut\assertFalse Updater.__isEnabledFor make(ContextCeiling.Off), UpdateReason.UserRequested
      ut\assertTrue Updater.__isEnabledFor make(ContextCeiling.UserRequested), UpdateReason.UserRequested
      ut\assertFalse Updater.__isEnabledFor make(ContextCeiling.UserRequested), UpdateReason.DependencyResolution
      ut\assertTrue Updater.__isEnabledFor make(ContextCeiling.DependencyResolution), UpdateReason.DependencyResolution
      ut\assertFalse Updater.__isEnabledFor make(ContextCeiling.DependencyResolution), UpdateReason.AutoUpdate
      ut\assertTrue Updater.__isEnabledFor make(ContextCeiling.AutoUpdate), UpdateReason.AutoUpdate

    _order: {
      "require_upToDateLoadsModule", "require_successReturnsRef", "require_errorPropagates"
      "require_skippedOptionalReturnsCode", "require_updateInProgressReturnsCode"
      "require_upToDateLoadFailureReturnsCode"
      "scheduleUpdate_disabledRejected", "scheduleUpdate_belowAutoUpdateModeRejected"
      "scheduleUpdate_virtualRejected"
      "scheduleUpdate_withinIntervalSkips", "scheduleUpdate_protectedInstallRejected"
      "scheduleUpdate_runsTaskWhenDue"
      "acquireLock_returnsTrueWhenAlreadyHeld", "acquireLock_acquiresAndSetsHasLock"
      "acquireLock_failsWhenHeldByOther", "releaseLock_releasesWhenHeld", "releaseLock_noopWhenNotHeld"
      "renewLock_renewsWhenHeld"
      "addTask_versionParseErrorReturns", "addTask_updatesExistingTask"
      "addTask_reentrantResolutionReachesRunningTask", "addTask_createsNewTask"
      "addTask_disabledUpdaterRejects", "addTask_modeGatesByReason", "addTask_invalidNamespaceRejects"
      "feedTrust_isSharedSingleton", "isEnabledFor_defaultsToAllContexts", "isEnabledFor_modeGatesByContext"
    }
  }
