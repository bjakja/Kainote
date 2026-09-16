-- NamedSemaphore tests: the named binary-semaphore primitive, exercised against real OS semaphores
-- (CreateSemaphoreA on Windows, sem_open on POSIX). Skipped where the semaphore FFI is unavailable.
-- Called from test.moon as: (controls\requireTest "NamedSemaphore")!
() ->
  NamedSemaphore = require "l0.DependencyControl.NamedSemaphore"

  -- a fresh, random [A-Za-z0-9_] token per call so concurrent runs don't collide on a persisted POSIX name
  token = -> "DepCtrlTest_#{'%08X'\format math.random 0, 16^8-1}"

  {
    _description: "NamedSemaphore: the cross-process binary semaphore, against real OS semaphores."
    _condition: -> NamedSemaphore.isAvailable, "no semaphore FFI on this platform/build"

    pidIsExposed: (ut) ->
      ut\assertEquals type(NamedSemaphore.pid), "number"

    -- a binary semaphore acquires once (1 -> 0), refuses a second (non-reentrant) tryLock, then releases
    -- and can be acquired again. unlinkOnClose cleans up the OS name on POSIX (no effect on Windows).
    acquiresExclusivelyAndReleases: (ut) ->
      sem = NamedSemaphore token!, true
      ut\assertTrue sem.isOpen
      ut\assertTrue sem\tryLock!
      ut\assertFalse sem\tryLock!
      ut\assertTrue sem\unlock!
      ut\assertTrue sem\tryLock!
      sem\unlock!

    -- a blocking lock() on an available semaphore acquires immediately (without blocking)
    blockingLockAcquires: (ut) ->
      sem = NamedSemaphore token!, true
      ut\assertTrue sem\lock!
      sem\unlock!

    -- two handles to the same name share the one kernel semaphore and contend
    sameNameContends: (ut) ->
      name = token!
      a, b = NamedSemaphore(name, true), NamedSemaphore(name, true)
      ut\assertTrue a\tryLock!
      ut\assertFalse b\tryLock!
      a\unlock!
      ut\assertTrue b\tryLock!
      b\unlock!

    -- collecting one handle must not unlink a name another holder still owns: on POSIX a per-instance
    -- sem_unlink would let the next sem_open create a separate semaphore, so a fresh handle could acquire
    -- the "same" lock while the first holder still holds it (split-brain). Windows self-heals via refcount.
    collectingOneHandleKeepsExclusion: (ut) ->
      name = token!
      a = NamedSemaphore name, true
      ut\assertTrue a\tryLock! -- a holds it (value 0)
      b = NamedSemaphore name, true -- a second handle to the same name...
      b = nil
      collectgarbage "collect" for _ = 1, 3 -- ...is collected (loop forces the finalizer to run); the name must survive
      c = NamedSemaphore name, true -- a fresh open must see the same, still-held semaphore
      ut\assertFalse c\tryLock! -- so it cannot acquire while a holds it
      a\unlock!

    _order: {"pidIsExposed", "acquiresExclusivelyAndReleases", "blockingLockAcquires", "sameNameContends"
      "collectingOneHandleKeepsExclusion"}
  }
