-- FileLock tests: the cross-process advisory file-lock primitive, exercised against real OS lock files
-- (LockFileEx on Windows, flock on POSIX). Skipped where the lock FFI is unavailable.
-- Called from test.moon as: (controls\requireTest "FileLock")!
() ->
  FileLock = require "l0.DependencyControl.FileLock"
  constants = require "l0.DependencyControl.Constants"

  -- a fresh lock-file path per call (?temp is created by the test runner)
  tmpPath = -> aegisub.decode_path "?temp/#{constants.DEPCTRL_SHORT_NAME}_#{FileLock.__name}_#{'%08X'\format math.random 0, 16^8-1}.lock"

  {
    _description: "FileLock: the cross-process advisory file lock, against real OS lock files."
    -- the lock FFI isn't available on every platform/build; skip rather than fail there
    _condition: -> FileLock.isAvailable, "no file-lock FFI on this platform/build"

    -- a fresh lock opens, acquires, and releases
    acquiresAndReleases: (ut) ->
      lock = FileLock tmpPath!
      ut\assertTrue lock.isOpen
      ut\assertTrue lock\tryLock!
      ut\assertTrue lock\unlock!

    -- two independent locks on the same file contend (the OS lock is per-file/open, not reentrant per
    -- handle): the second can't acquire while the first holds it, and can once it's released
    secondHolderContends: (ut) ->
      path = tmpPath!
      a, b = FileLock(path), FileLock(path)
      ut\assertTrue a\tryLock!
      ut\assertFalse b\tryLock!
      a\unlock!
      ut\assertTrue b\tryLock!
      b\unlock!

    -- the same holder can re-acquire after releasing
    reacquiresAfterUnlock: (ut) ->
      lock = FileLock tmpPath!
      ut\assertTrue lock\tryLock!
      lock\unlock!
      ut\assertTrue lock\tryLock!
      lock\unlock!

    _order: {"acquiresAndReleases", "secondHolderContends", "reacquiresAfterUnlock"}
  }
