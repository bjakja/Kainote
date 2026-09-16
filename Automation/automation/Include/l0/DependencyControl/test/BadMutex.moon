-- BadMutex tests: extracted from the main test suite.
-- Called from test.moon as: (controls\requireTest "BadMutex")!
() ->
  BadMutex = require "l0.DependencyControl.shims.BadMutex"

  {
    _description: "Tests for BadMutex: FFI-based process-scoped mutex (over a single named semaphore) that fills in for BM.BadMutex."

    -- API surface

    api_hasTryLock: (ut) ->
      ut\assertFunction BadMutex.tryLock

    api_hasLock: (ut) ->
      ut\assertFunction BadMutex.lock

    api_hasUnlock: (ut) ->
      ut\assertFunction BadMutex.unlock

    -- tryLock / unlock round-trip

    tryLock_acquires: (ut) ->
      result = BadMutex.tryLock!
      ut\assertTrue result
      BadMutex.unlock! -- release so subsequent tests start clean

    tryLock_failsWhenHeld: (ut) ->
      ut\assertTrue BadMutex.tryLock! -- acquire
      result = BadMutex.tryLock! -- second attempt must fail
      BadMutex.unlock!
      ut\assertFalse result

    unlock_releasesLock: (ut) ->
      ut\assertTrue BadMutex.tryLock!
      BadMutex.unlock!
      result = BadMutex.tryLock! -- must succeed again after release
      BadMutex.unlock!
      ut\assertTrue result

    -- BM.BadMutex alias

    registered_asBadMutex: (ut) ->
      -- DepCtrl registers "BM.BadMutex" as an alias via ModuleProvider; requiring it
      -- should resolve to the bundled FFI mutex (or a native one if installed).
      bm = require "BM.BadMutex"
      ut\assertNotNil bm
      ut\assertFunction bm.tryLock

    _order: {
      "api_hasTryLock", "api_hasLock", "api_hasUnlock",
      "tryLock_acquires", "tryLock_failsWhenHeld", "unlock_releasesLock",
      "registered_asBadMutex"
    }
  }
