-- POSIX open(2) flag/mode constants for FFI callers, with the per-OS numeric values
-- folded in. Linux values are the asm-generic ones used by x86/x86_64/arm/arm64 (the
-- platforms Aegisub ships on); a few historical arches (alpha, mips, parisc, sparc) differ
-- but are not supported. macOS (Darwin) values are taken from <sys/fcntl.h>.

ffi = require "ffi"

isOSX = ffi.os == "OSX"

filePermissionBits = {r: 4, w: 2, x: 1}

-- open(2) is variadic (int open(const char*, int, ...)); the open wrapper below passes the mode as
-- typed cdata so the Apple-Silicon vararg ABI (stack-passed) receives it intact.
pcall ffi.cdef, [[
  int open(const char* path, int flags, ...);
  int close(int fd);
]]

return {
  -- low two bits of the open(2) flags: the access mode (same on Linux and macOS)
  FileAccessMode: {
    Read: 0 -- O_RDONLY (read-only)
    Write: 1 -- O_WRONLY (write-only)
    ReadWrite: 2 -- O_RDWR   (read-write)
  }

  FileCreationFlags: {
    Create: isOSX and 0x200 or 0x40 -- O_CREAT: create the file if it doesn't exist
    Exclusive: isOSX and 0x800 or 0x80 -- O_EXCL: with Create, fail if the file already exists
    Truncate: isOSX and 0x400 or 0x200 -- O_TRUNC: truncate the file to zero length
    -- O_NOCTTY: don't let an opened terminal become the process's controlling terminal
    NoControllingTerminal: isOSX and 0x20000 or 0x100
    Directory: isOSX and 0x100000 or 0x10000 -- O_DIRECTORY: fail if the path isn't a directory
    NoFollow: isOSX and 0x100 or 0x20000 -- O_NOFOLLOW: fail if the final component is a symlink
    -- O_CLOEXEC: set the close-on-exec flag so child processes don't inherit the fd
    CloseOnExec: isOSX and 0x1000000 or 0x80000
    -- O_TMPFILE: create an unnamed temporary file (the Linux value already includes
    -- O_DIRECTORY, as the kernel requires). Not available on macOS, where it is 0 (a
    -- no-op) -- callers needing a temp file there must fall back to another mechanism.
    TmpFile: isOSX and 0 or 0x410000
  }

  ---Builds the numeric file mode for the given symbolic permissions.
  ---@param user? string Any combination of "r", "w" and "x" for the owner, or "" for none.
  ---@param group? string Same, for the owner's group.
  ---@param other? string Same, for all other users.
  ---@return number mode The file mode, e.g. getFileMode("rwx", "r", "r") -> 0o744 (484).
  getFileMode: (user = "", group = "", other = "") ->
    mode = 0
    for perm in user\gmatch "."
      mode += (filePermissionBits[perm] or 0) * 64
    for perm in group\gmatch "."
      mode += (filePermissionBits[perm] or 0) * 8
    for perm in other\gmatch "."
      mode += filePermissionBits[perm] or 0
    return mode

  ---Opens a file, creating it when the flags include Create (O_CREAT), and returns the raw descriptor.
  ---@param path string
  ---@param flags integer open(2) flags, e.g. FileAccessMode.ReadWrite | FileCreationFlags.Create.
  ---@param mode integer Permission bits for a newly created file (from getFileMode).
  ---@return integer fd The open descriptor, or a negative value on failure.
  open: (path, flags, mode) -> ffi.C.open path, flags, ffi.new "int", mode

  ---Closes an open file descriptor.
  ---@param fd integer A descriptor returned by open.
  ---@return integer status Zero on success, or a negative value on failure.
  close: (fd) -> ffi.C.close fd
}
