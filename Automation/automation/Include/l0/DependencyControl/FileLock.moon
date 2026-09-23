ffi = require "ffi"
fileOps = require "l0.DependencyControl.file-ops"
Finalizer = require "l0.DependencyControl.Finalizer"

local openImpl, tryLockImpl, unlockImpl, closeImpl, isAvailable

msgs = {
  noImplementation: "No file lock implementation is available on this platform/build configuration."
}

if ffi.os == "Windows"
  ffiWin = require "l0.DependencyControl.helpers.ffi-windows"

  -- LockFileEx on a one-byte range on Windows
  pcall ffi.cdef, [[
    void* CreateFileW(const wchar_t* name, unsigned long access, unsigned long share, void* sec, unsigned long disposition, unsigned long flags, void* template);
    int LockFileEx(void* hFile, unsigned long flags, unsigned long reserved, unsigned long countLow, unsigned long countHigh, void* overlapped);
    int UnlockFileEx(void* hFile, unsigned long reserved, unsigned long countLow, unsigned long countHigh, void* overlapped);
  ]]
  -- mirrors the fields of OVERLAPPED; zeroed, it locks the byte range at offset 0
  pcall ffi.cdef, "typedef struct { uintptr_t Internal; uintptr_t InternalHigh; unsigned long Offset; unsigned long OffsetHigh; void* hEvent; } DepCtrlOverlapped;"

  kernel32, toWide = ffiWin.kernel32, ffiWin.toWide
  isAvailable = ffiWin.haveKernel32

  -- CreateFileW
  GENERIC_READ = 0x80000000
  GENERIC_WRITE = 0x40000000
  GENERIC_READ_WRITE = bit.bor(GENERIC_READ, GENERIC_WRITE)

  FILE_SHARE_READ = 0x1
  FILE_SHARE_WRITE = 0x2
  FILE_SHARE_READ_WRITE = bit.bor(FILE_SHARE_READ, FILE_SHARE_WRITE)

  OPEN_ALWAYS = 4 -- open the file, creating it if it doesn't exist
  FILE_ATTRIBUTE_NORMAL = 0x80 -- a file without any special attributes
  INVALID_HANDLE = ffi.cast "void*", -1

  -- LockFileEx
  LOCKFILE_FAIL_IMMEDIATELY = 1 -- fail instead of waiting when the range is locked
  LOCKFILE_EXCLUSIVE_LOCK = 2 -- request an exclusive lock instead of a shared one
  LOCK_EXCLUSIVE_NONBLOCKING = bit.bor(LOCKFILE_EXCLUSIVE_LOCK, LOCKFILE_FAIL_IMMEDIATELY)

  openImpl = (path) ->
    handle = kernel32.CreateFileW toWide(path), GENERIC_READ_WRITE, FILE_SHARE_READ_WRITE,
      nil, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nil
    return nil if handle == INVALID_HANDLE
    return {handle: handle, overlapped: ffi.new "DepCtrlOverlapped"}
  tryLockImpl = (h) -> 0 != kernel32.LockFileEx h.handle, LOCK_EXCLUSIVE_NONBLOCKING, 0, 1, 0, h.overlapped
  unlockImpl = (h) -> kernel32.UnlockFileEx h.handle, 0, 1, 0, h.overlapped
  closeImpl = (h) -> kernel32.CloseHandle h.handle

else
  ffiPosix = require "l0.DependencyControl.helpers.ffi-posix"

  -- flock(2) advisory lock (per-open-file-description, so two independent opens contend even within
  -- one process). open/close are provided by ffi-posix.
  pcall ffi.cdef, [[
    int flock(int fd, int operation);
  ]]
  isAvailable = true

  -- flock
  LOCK_SH = 1 -- request a shared lock
  LOCK_EX = 2 -- request an exclusive lock
  LOCK_NB = 4 -- fail instead of waiting if the lock is held by another process
  LOCK_UN = 8 -- remove an existing lock held by this process

  LOCK_EXCLUSIVE_NONBLOCKING = bit.bor(LOCK_EX, LOCK_NB)

  openImpl = (path) ->
    fd = ffiPosix.open path, bit.bor(ffiPosix.FileAccessMode.ReadWrite, ffiPosix.FileCreationFlags.Create), ffiPosix.getFileMode 'rw', 'r', 'r'
    return nil if fd < 0
    return {fd: fd}
  tryLockImpl = (h) -> 0 == ffi.C.flock h.fd, LOCK_EXCLUSIVE_NONBLOCKING
  unlockImpl = (h) -> ffi.C.flock h.fd, LOCK_UN
  closeImpl = (h) -> ffiPosix.close h.fd

---A cross-process advisory lock on a file.
---Usable as a cross-process lock primitive.
---Automatically released when the instance is garbage collected or when the process exits.
---However, unlike a semaphore, it cannot be forcibly taken from a process that is alive but hung.
---@class FileLock
class FileLock
  -- whether the OS file-lock FFI is available on this platform/build
  ---@type boolean
  @isAvailable = isAvailable

  ---Opens (creating if absent) the lock file and prepares it for locking.
  ---@param path string Full path to the lock file.
  new: (path) =>
    @isOpen = false
    assert isAvailable, msgs.noImplementation
    normalizedPath, errMsg = fileOps.validateFullPath path, true
    assert normalizedPath, errMsg

    handle = openImpl normalizedPath
    return unless handle
    @_handle = handle
    @path = normalizedPath
    @isOpen = true

    -- close the handle when this object is garbage collected to release the lock in case it's still being held
    handleRef = handle
    Finalizer.guard @, -> closeImpl handleRef

  ---Attempts to acquire the lock without blocking.
  ---@return boolean acquired True if acquired.
  tryLock: => @isOpen and tryLockImpl(@_handle) or false

  ---Releases the lock. Only the current holder should call this.
  ---@return boolean issued True if a release was issued.
  unlock: =>
    return false unless @isOpen
    unlockImpl @_handle
    true

return FileLock
