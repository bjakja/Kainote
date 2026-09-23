-- Lock tests: extracted from the main test suite.
-- Called from test.moon as: (controls\requireTest "Lock")!
() ->
  Lock = require "l0.DependencyControl.Lock"
  Logger = require "l0.DependencyControl.Logger"
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  {:defaultLogger} = UnitTestSuite\getTestExports Lock

  FILEOPS_MODULE_NAME = "l0.DependencyControl.file-ops"
  TIMER_MODULE_NAME = "l0.DependencyControl.Timer"

  -- A controllable stand-in for the OS lock primitive, installed via the
  -- Lock.__createPrimitive seam so Lock tests never open a real OS handle. Spy/stub its
  -- methods with ut\stub.
  makeFakeSemaphore = (isOpen = true) ->
    {
      isOpen: isOpen
      tryLock: => true
      lock: => true
      unlock: => true
    }

  -- Installs a fake lock primitive through the Lock.__createPrimitive seam for the next Lock
  -- constructed in this test. tryLockBehavior is either a fixed boolean (returned every
  -- call) or a function used as the stub implementation. Returns the fake plus the
  -- tryLock/unlock stubs for assertions.
  installFakeSemaphore = (ut, tryLockBehavior = true) ->
    sem = makeFakeSemaphore!
    tryLockStub = ut\stub sem, "tryLock"
    if type(tryLockBehavior) == "function"
      tryLockStub\calls tryLockBehavior
    else
      tryLockStub\returns tryLockBehavior
    unlockStub = ut\stub sem, "unlock"
    (ut\stub Lock, "__createPrimitive")\returns sem
    return sem, tryLockStub, unlockStub

  -- Minimal JSON holder record for exercising Lock's lease/stale-holder logic via a
  -- stubbed FileOps.readFile.
  craftHolderRecord = (acquiredAt, expiresAt) ->
    ('{"holderName":"Ghost","pid":4321,"acquiredAt":%d,"expiresAt":%d}')\format acquiredAt, expiresAt

  {
    _description: "Tests for the Lock cooperative mutex class."

    -- LockState enum: verifies Enum was called with "LockState" and the correct value mapping

    lockState_values: (ut) ->
      ut\assertEquals Lock.LockState.Unknown, -1
      ut\assertEquals Lock.LockState.Unavailable, 0
      ut\assertEquals Lock.LockState.Available, 1
      ut\assertEquals Lock.LockState.Held, 2

    lockState_name: (ut) ->
      found, val = Lock.LockState\test "Held"
      ut\assertTrue found
      ut\assertEquals val, 2

    -- shared default logger: verifies Logger was constructed with the correct fileBaseName

    defaultLogger_fileBaseName: (ut) ->
      ut\assertEquals defaultLogger.fileBaseName, "DepCtrl.Lock"

    -- constructor

    new_defaults: (ut) ->
      lock = Lock namespace: "ns", resource: "res"
      ut\assertEquals lock.namespace, "ns"
      ut\assertEquals lock.resource, "res"
      ut\assertEquals lock.holderName, "unknown"
      ut\assertEquals lock.expiresAfter, 300
      ut\assertString lock.instanceId

    new_customLogger: (ut) ->
      customLogger = Logger toFile: false, toWindow: false
      lock = Lock namespace: "ns", resource: "res", logger: customLogger
      ut\assertEquals lock.logger, customLogger

    -- state

    state_initial: (ut) ->
      lock = Lock namespace: "ns", resource: "res"
      ut\assertEquals lock.state, Lock.LockState.Unknown

    state_held: (ut) ->
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      lock\lock!
      ut\assertEquals lock.state, Lock.LockState.Held
      lock\release!

    -- LockScope enum

    scope_values: (ut) ->
      ut\assertEquals Lock.Scope.Process, "process"
      ut\assertEquals Lock.Scope.Global, "global"

    scope_defaultsToProcess: (ut) ->
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      ut\assertEquals lock.scope, Lock.Scope.Process

    scope_globalOption: (ut) ->
      installFakeSemaphore ut, true
      lock = Lock namespace: "ns", resource: "res", scope: Lock.Scope.Global, recordHolder: false
      ut\assertEquals lock.scope, Lock.Scope.Global

    -- lock

    lock_success: (ut) ->
      _, tryLockStub = installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      state, timePassed = lock\lock!
      ut\assertEquals state, Lock.LockState.Held
      ut\assertEquals timePassed, 0
      tryLockStub\assertCalledOnce!
      lock\release!

    lock_alreadyHeld: (ut) ->
      _, tryLockStub = installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      lock\lock! -- acquire
      state, timePassed = lock\lock! -- re-enter: already held path
      ut\assertEquals state, Lock.LockState.Held
      tryLockStub\assertCalledOnce! -- semaphore not re-acquired on second call
      lock\release!

    lock_timeout: (ut) ->
      _, tryLockStub = installFakeSemaphore ut, false
      sleepStub = ut\stub TIMER_MODULE_NAME, "sleep"
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      state, timePassed = lock\lock 0
      ut\assertEquals state, Lock.LockState.Unavailable
      tryLockStub\assertCalledOnce!
      sleepStub\assertNotCalled! -- timeout=0 suppresses sleep

    lock_retry: (ut) ->
      callCount = 0
      _, tryLockStub = installFakeSemaphore ut, ->
        callCount += 1
        callCount >= 2 -- fails first, succeeds second
      sleepStub = ut\stub TIMER_MODULE_NAME, "sleep"
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      state, timePassed = lock\lock!
      ut\assertEquals state, Lock.LockState.Held
      tryLockStub\assertCalledTimes 2
      sleepStub\assertCalledOnceWith 250 -- default lockWaitInterval
      lock\release!

    -- a missing OS primitive degrades to a process-local grant rather than failing
    lock_primitiveUnavailable: (ut) ->
      sem = makeFakeSemaphore false -- isOpen = false
      (ut\stub Lock, "__createPrimitive")\returns sem
      ut\stub defaultLogger, "warn"
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      state = lock\lock 0
      ut\assertEquals state, Lock.LockState.Held
      lock\release! -- release so the lingering held state can't fire its GC warning in a later test

    -- tryLock

    tryLock_success: (ut) ->
      _, tryLockStub = installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      state, timePassed = lock\tryLock!
      ut\assertEquals state, Lock.LockState.Held
      tryLockStub\assertCalledOnce!
      lock\release!

    tryLock_fail: (ut) ->
      _, tryLockStub = installFakeSemaphore ut, false
      sleepStub = ut\stub TIMER_MODULE_NAME, "sleep"
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      state, timePassed = lock\tryLock!
      ut\assertEquals state, Lock.LockState.Unavailable
      ut\assertEquals timePassed, 0 -- no wait happened, so none may be reported
      sleepStub\assertNotCalled!
      tryLockStub\assertCalledOnce!

    -- distinct resources map to distinct semaphores, so they can be held at once;
    -- the same resource is mutually exclusive across instances. Uses real semaphores.
    multiResource_independent: (ut) ->
      ut\stub defaultLogger, "trace"
      a = Lock namespace: "ns", resource: "resA", recordHolder: false
      b = Lock namespace: "ns", resource: "resB", recordHolder: false
      ut\assertEquals (a\tryLock!), Lock.LockState.Held
      ut\assertEquals (b\tryLock!), Lock.LockState.Held -- different resource doesn't block
      a\release!
      b\release!

    sameResource_mutuallyExclusive: (ut) ->
      ut\stub defaultLogger, "trace"
      a = Lock namespace: "ns", resource: "shared", recordHolder: false
      b = Lock namespace: "ns", resource: "shared", recordHolder: false
      ut\assertEquals (a\tryLock!), Lock.LockState.Held
      ut\assertEquals (b\tryLock!), Lock.LockState.Unavailable -- held by a
      a\release!
      ut\assertEquals (b\tryLock!), Lock.LockState.Held -- available after release
      b\release!

    -- release

    release_held: (ut) ->
      _, _, unlockStub = installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      lock\lock!
      result, extra = lock\release!
      ut\assertTrue result
      ut\assertEquals extra, Lock.LockState.Available
      unlockStub\assertCalledOnce!

    release_notHeld: (ut) ->
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      result, err = lock\release!
      ut\assertNil result
      ut\assertString err
      ut\assertContains err, "not currently held"

    -- holder side file: written on acquire, removed on release

    holderRecorded_onLock: (ut) ->
      written = {}
      (ut\stub FILEOPS_MODULE_NAME, "writeFile")\calls (path, data) ->
        written.path, written.data = path, data
        true
      removeStub = ut\stub FILEOPS_MODULE_NAME, "remove"
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", holderName: "TestHolder"
      lock\lock!
      ut\assertString written.data
      ut\assertContains written.data, "TestHolder"
      lock\release!
      removeStub\assertCalledOnce! -- holder file cleared on release

    holderRecordsLease: (ut) ->
      written = {}
      (ut\stub FILEOPS_MODULE_NAME, "writeFile")\calls (path, data) ->
        written.data = data
        true
      ut\stub FILEOPS_MODULE_NAME, "remove"
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", expiresAfter: 120
      lock\lock!
      ut\assertContains written.data, "expiresAt" -- lease stamped into the record
      ut\assertContains written.data, "acquiredAt"
      lock\release!

    -- Global scope uses a real OS advisory file lock for cross-instance exclusion
    globalScope_mutuallyExclusive: (ut) ->
      ut\stub defaultLogger, "trace"
      a = Lock namespace: "ns", resource: "globalShared", scope: Lock.Scope.Global, recordHolder: false
      b = Lock namespace: "ns", resource: "globalShared", scope: Lock.Scope.Global, recordHolder: false
      ut\assertEquals (a\tryLock!), Lock.LockState.Held
      ut\assertEquals (b\tryLock!), Lock.LockState.Unavailable -- held by a (same file)
      a\release!
      ut\assertEquals (b\tryLock!), Lock.LockState.Held -- available after release
      b\release!

    -- stale-holder warning: honors the holder's recorded lease

    staleHolder_warnsPastLease: (ut) ->
      now = os.time!
      (ut\stub FILEOPS_MODULE_NAME, "readFile")\returns craftHolderRecord now - 1000, now - 10
      installFakeSemaphore ut, false -- never acquires: takes the heldByOther path
      ut\stub TIMER_MODULE_NAME, "sleep"
      warnStub = ut\stub defaultLogger, "warn"
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res" -- recordHolder defaults true
      lock\lock 0
      warnStub\assertCalled! -- lease lapsed -> stale warning

    staleHolder_silentWithinLease: (ut) ->
      now = os.time!
      (ut\stub FILEOPS_MODULE_NAME, "readFile")\returns craftHolderRecord now - 10, now + 1000
      installFakeSemaphore ut, false
      ut\stub TIMER_MODULE_NAME, "sleep"
      warnStub = ut\stub defaultLogger, "warn"
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res"
      lock\lock 0
      warnStub\assertNotCalled! -- still within the holder's lease

    overrideExpiry_usesOwnExpiry: (ut) ->
      now = os.time!
      -- holder claims a long lease, but overrideExpiry judges against our short expiresAfter
      (ut\stub FILEOPS_MODULE_NAME, "readFile")\returns craftHolderRecord now - 1000, now + 100000
      installFakeSemaphore ut, false
      ut\stub TIMER_MODULE_NAME, "sleep"
      warnStub = ut\stub defaultLogger, "warn"
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", overrideExpiry: true, expiresAfter: 10
      lock\lock 0
      warnStub\assertCalled! -- our (acquiredAt + 10) deadline has passed

    -- renew

    renew_forceRewrites: (ut) ->
      writes = {}
      (ut\stub FILEOPS_MODULE_NAME, "writeFile")\calls (path, data) ->
        writes[#writes + 1] = data
        true
      ut\stub FILEOPS_MODULE_NAME, "remove"
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res"
      lock\lock! -- writes holder record (#1)
      ok = lock\renew -1 -- -1 forces a rewrite (#2)
      ut\assertTrue ok
      ut\assertEquals #writes, 2
      lock\release!

    renew_notHeld: (ut) ->
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", recordHolder: false
      ok, err = lock\renew!
      ut\assertNil ok
      ut\assertString err

    renew_skipsWhenFresh: (ut) ->
      writes = {}
      (ut\stub FILEOPS_MODULE_NAME, "writeFile")\calls (path, data) ->
        writes[#writes + 1] = data
        true
      ut\stub FILEOPS_MODULE_NAME, "remove"
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", expiresAfter: 600
      lock\lock!
      renewed = lock\renew! -- default threshold: lease barely started
      ut\assertFalse renewed
      ut\assertEquals #writes, 1
      lock\release!

    renew_renewsWhenDue: (ut) ->
      writes = {}
      (ut\stub FILEOPS_MODULE_NAME, "writeFile")\calls (path, data) ->
        writes[#writes + 1] = data
        true
      ut\stub FILEOPS_MODULE_NAME, "remove"
      installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      lock = Lock namespace: "ns", resource: "res", expiresAfter: 600
      lock\lock!
      lock._leaseExpiresMono = 0 -- force remaining time below the threshold
      renewed = lock\renew!
      ut\assertTrue renewed
      ut\assertEquals #writes, 2
      lock\release!

    -- guard: scoped acquire + guaranteed release

    guard_runsAndReleases: (ut) ->
      _, _, unlockStub = installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      ran = false
      result = Lock\guard {namespace: "ns", resource: "res", recordHolder: false}, (lock) ->
        ran = true
        "value"
      ut\assertTrue ran
      ut\assertEquals result, "value"
      unlockStub\assertCalledOnce! -- released after the body

    guard_releasesOnError: (ut) ->
      _, _, unlockStub = installFakeSemaphore ut, true
      ut\stub defaultLogger, "trace"
      ok, err = pcall -> Lock\guard {namespace: "ns", resource: "res", recordHolder: false}, -> error "boom"
      ut\assertFalse ok -- the body's error is re-raised
      unlockStub\assertCalledOnce! -- but the lock was still released

    guard_failsToAcquire: (ut) ->
      installFakeSemaphore ut, false
      ut\stub TIMER_MODULE_NAME, "sleep"
      ut\stub defaultLogger, "trace"
      called = false
      result, err = Lock\guard {namespace: "ns", resource: "res", recordHolder: false, timeout: 0}, -> called = true
      ut\assertNil result
      ut\assertString err
      ut\assertFalse called -- body never runs when the lock can't be taken

    -- GC finalizer: unreleased lock is cleaned up and warns on collection

    gc_finalizer: (ut) ->
      sem = makeFakeSemaphore!
      (ut\stub sem, "tryLock")\returns true
      unlockStub = ut\stub sem, "unlock"
      (ut\stub Lock, "__createPrimitive")\returns sem
      warned = false
      warnStub = (ut\stub defaultLogger, "warn")\calls -> warned = true
      ut\stub defaultLogger, "trace"
      do
        lock = Lock namespace: "ns", resource: "res", recordHolder: false
        lock\lock!
      -- run GC until the finalizer fires; a backlog of finalizers from earlier
      -- tests means a fixed couple of passes isn't always enough
      for _ = 1, 20
        collectgarbage "collect"
        break if warned
      warnStub\assertCalledOnce!
      unlockStub\assertCalledOnce!

    _order: {
      "lockState_values", "lockState_name",
      "defaultLogger_fileBaseName",
      "new_defaults", "new_customLogger",
      "state_initial", "state_held",
      "scope_values", "scope_defaultsToProcess", "scope_globalOption",
      "lock_success", "lock_alreadyHeld", "lock_timeout", "lock_retry",
      "lock_primitiveUnavailable",
      "tryLock_success", "tryLock_fail",
      "multiResource_independent", "sameResource_mutuallyExclusive",
      "release_held", "release_notHeld",
      "holderRecorded_onLock", "holderRecordsLease",
      "globalScope_mutuallyExclusive",
      "staleHolder_warnsPastLease", "staleHolder_silentWithinLease", "overrideExpiry_usesOwnExpiry",
      "renew_forceRewrites", "renew_notHeld", "renew_skipsWhenFresh", "renew_renewsWhenDue",
      "guard_runsAndReleases", "guard_releasesOnError", "guard_failsToAcquire",
      "gc_finalizer"
    }
  }
