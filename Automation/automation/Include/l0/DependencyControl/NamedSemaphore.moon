ffi = require "ffi"
Finalizer = require "l0.DependencyControl.Finalizer"
local Hash

local formatName, openImpl, isOpenImpl, tryLockImpl, lockImpl, unlockImpl, closeImpl
local pid, isAvailable, unlinkAtExit

msgs = {
  noImplementation: "No named semaphore implementation is available on this platform/build configuration."
}

if ffi.os == "Windows"
  -- On Windows, the kernel object is ref-counted and destroyed once the last handle closes,
  -- so it self-heals after a holder process exits

  ffiWin = require "l0.DependencyControl.helpers.ffi-windows" -- registers the shared CloseHandle cdef

  pcall ffi.cdef, "unsigned int GetCurrentProcessId(void);"
  pcall ffi.cdef, "void *CreateSemaphoreA(void *attr, long initialCount, long maximumCount, const char *name);"
  pcall ffi.cdef, "unsigned long WaitForSingleObject(void *hHandle, unsigned long dwMilliseconds);"
  pcall ffi.cdef, "bool ReleaseSemaphore(void *hSemaphore, long lReleaseCount, long *lpPreviousCount);"

  WAIT_OBJECT_0 = 0
  INFINITE = 0xFFFFFFFF

  okPid, p = pcall -> tonumber ffi.C.GetCurrentProcessId!
  pid = okPid and p or 0
  isAvailable = true

  formatName = (token) -> token
  openImpl = (name) -> ffi.C.CreateSemaphoreA nil, 1, 1, name
  isOpenImpl = (handle) -> handle != nil
  tryLockImpl = (handle) -> ffi.C.WaitForSingleObject(handle, 0) == WAIT_OBJECT_0
  lockImpl = (handle) -> ffi.C.WaitForSingleObject handle, INFINITE
  unlockImpl = (handle) -> ffi.C.ReleaseSemaphore handle, 1, nil
  closeImpl = (name, handle, unlink) -> ffiWin.kernel32.CloseHandle handle

else
  -- POSIX named semaphore. Unlike Windows, the name persists in the kernel namespace
  -- until it is unlinked or reboot, so it does not self-heal after a holder process dies.

  ffiPosix = require "l0.DependencyControl.helpers.ffi-posix"
  -- sem_open is variadic, so its mode and value are passed as typed cdata. LuaJIT converts a bare-number
  -- vararg to a double, and the Apple-Silicon ABI passes varargs on the stack, so an untyped initial value
  -- arrives as garbage (often 0) and makes the first sem_wait block forever.
  -- Aegisub runs per-user, so semaphores don't need to be shared with others.
  SEMAPHORE_FILE_MODE = ffi.new "int", ffiPosix.getFileMode "rw"
  BINARY_SEMAPHORE_INITIAL_VALUE = ffi.new "unsigned int", 1
  SEM_FAILED = ffi.cast "void *", -1 -- sem_open's failure sentinel ((void*)-1)

  pcall ffi.cdef, [[
    int getpid(void);
    void *sem_open(const char *name, int oflag, ...);
    int sem_wait(void *sem);
    int sem_trywait(void *sem);
    int sem_post(void *sem);
    int sem_close(void *sem);
    int sem_unlink(const char *name);
  ]]

  okPid, p = pcall -> tonumber ffi.C.getpid!
  pid = okPid and p or 0
  isAvailable = true

  -- Names to unlink when this Lua state tears down (≈ process exit for the main state). We deliberately
  -- do NOT unlink when an individual instance is collected: unlinking a name another holder still owns
  -- would let a later sem_open create a *separate* semaphore, so two holders could each believe they hold
  -- the lock. Deferring the unlink to teardown keeps the name valid for the whole process while still
  -- cleaning it up (so a reused pid can't inherit a stale semaphore) on a clean exit. The finalizer must be
  -- kept alive for the module's lifetime (anchored on the class below) or it would be collected — and fire
  -- the unlink — early.
  namesToUnlink = {}
  unlinkAtExit = Finalizer.create -> pcall(-> ffi.C.sem_unlink name) for name in pairs namesToUnlink

  -- Darwin caps the whole name at 31 chars (including the NULL terminator) and fails with ENAMETOOLONG beyond it.
  -- Linux allows 251 (255 - the length of the leading 'sem.'). If the token exceeds the OS-specific limit, we use a
  -- truncated SHA-1 digest of it instead to ensure deterministic uniqueness.
  PSEMNAMLEN = 31
  MAX_POSIX_SEM_NAME = ffi.os == "OSX" and PSEMNAMLEN - 1 or 251

  formatName = (token) ->
    name = "/#{token}" -- POSIX names start with a single '/' and contain no other slashes.
    return name if #name <= MAX_POSIX_SEM_NAME
    -- required lazily to break a load cycle (Logger → NamedSemaphore → hash → Enum → Logger).
    Hash or= require "l0.DependencyControl.hash"
    return "/#{Hash.getDigest(Hash.HashType.Sha1, token)\sub(1, MAX_POSIX_SEM_NAME - 1)}"

  openImpl = (name) -> ffi.C.sem_open name, ffiPosix.FileCreationFlags.Create, SEMAPHORE_FILE_MODE, BINARY_SEMAPHORE_INITIAL_VALUE
  isOpenImpl = (handle) -> handle != nil and handle != SEM_FAILED
  tryLockImpl = (handle) -> ffi.C.sem_trywait(handle) == 0
  lockImpl = (handle) -> ffi.C.sem_wait handle
  unlockImpl = (handle) -> ffi.C.sem_post handle
  closeImpl = (name, handle, unlink) ->
    ffi.C.sem_close handle
    namesToUnlink[name] = true if unlink -- unlinked at state teardown, not now (see above)


---A non-reentrant binary semaphore identified by a name.
---Usable as a per-process or cross-process lock primitive.
---@class NamedSemaphore
class NamedSemaphore
  -- whether the OS semaphore FFI is available at all on this platform/build
  ---@type boolean
  @isAvailable = isAvailable

  -- this process's id, exposed so callers can build process-scoped names and holder records
  ---@type integer
  @pid = pid

  -- anchor the teardown-unlink finalizer to the class so it lives as long as the module (nil on Windows)
  ---@type userdata?
  @__unlinkFinalizer = unlinkAtExit

  ---Gets a handle to the named semaphore for the given token, creating it if it doesn't exist.
  ---@param token string A name token restricted to [A-Za-z0-9_].
  ---@param unlinkOnClose? boolean POSIX-only: remove the OS name when this Lua state tears down (not when
  ---this instance is collected — that would break a name another holder still owns).
  ---Use true for process-private names so a reused PID can't inherit a stale semaphore.
  ---Use false for cross-process usage to prevent an exiting process from removing a name others still hold.
  ---No effect on Windows, where names are cleaned up automatically when the last handle closes.
  new: (token, unlinkOnClose = false) =>
    assert isAvailable, msgs.noImplementation

    @name = formatName token
    @handle = openImpl @name
    @isOpen = isOpenImpl @handle
    return unless @isOpen

    -- close the OS handle when this object is garbage-collected.
    name, handle, unlink = @name, @handle, unlinkOnClose
    Finalizer.guard @, -> closeImpl name, handle, unlink

  ---Attempts to acquire without blocking.
  ---@return boolean acquired True if the semaphore was acquired.
  tryLock: => @isOpen and tryLockImpl(@handle) or false

  ---Blocks until the semaphore is acquired.
  ---@return boolean issued True if a wait was issued (false only when unavailable).
  lock: =>
    return false unless @isOpen
    lockImpl @handle
    true

  ---Releases one unit of the semaphore. Safe to call only by the current holder.
  ---@return boolean issued True if a release was issued.
  unlock: =>
    return false unless @isOpen
    unlockImpl @handle
    true

return NamedSemaphore
