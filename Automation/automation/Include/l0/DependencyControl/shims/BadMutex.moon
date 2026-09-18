-- A stand-in for the process-scoped singleton BM.BadMutex, using our internal pure-FFI
-- NamedSemaphore implementation.

constants = require "l0.DependencyControl.Constants"
NamedSemaphore = require "l0.DependencyControl.NamedSemaphore"

-- A single fixed, process-private name
semaphore = NamedSemaphore "#{constants.DEPCTRL_SHORT_NAME}_BadMutex_p#{NamedSemaphore.pid}", true

mutex = {
  ---Attempts to acquire the process-scoped mutex without blocking.
  ---@return boolean acquired True when the mutex was taken; false when already held or unavailable.
  tryLock: -> semaphore\tryLock!
  ---Blocks until the process-scoped mutex is acquired.
  ---@return boolean issued True when a wait was issued; false only when the mutex is unavailable.
  lock: -> semaphore\lock!
  ---Releases the process-scoped mutex; call only while holding it.
  ---@return boolean issued True when a release was issued; false only when the mutex is unavailable.
  unlock: -> semaphore\unlock!
  -- the BM.BadMutex version this is compatible with
  version: "0.1.3"
}

return mutex
