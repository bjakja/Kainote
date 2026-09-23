-- POSIX-only tests for helpers/ffi-posix: validates the open(2) flag/mode values against
-- the real kernel by actually opening files. Skipped on Windows (see _condition), where
-- the values are never used. Run it via the WSL test runner.
-- Called from test.moon as: (controls\requireTest "FfiPosix")!
() ->
  ffi = require "ffi"
  ffiPosix = require "l0.DependencyControl.helpers.ffi-posix"
  lfs = require "lfs"
  constants = require "l0.DependencyControl.Constants"

  pcall ffi.cdef, [[
    unsigned int umask(unsigned int mask);
  ]]

  Access = ffiPosix.FileAccessMode
  Create = ffiPosix.FileCreationFlags.Create
  Exclusive = ffiPosix.FileCreationFlags.Exclusive

  tmpPath = -> aegisub.decode_path "?temp/#{constants.DEPCTRL_SHORT_NAME}_ffiPosix_#{'%08X'\format math.random 0, 16^8-1}"

  {
    _description: "POSIX open(2) flag/mode values (helpers/ffi-posix), validated against the real kernel."
    -- only meaningful on POSIX; the flag values are never exercised on Windows
    _condition: -> ffi.os != "Windows", "POSIX-only"

    -- O_WRONLY | O_CREAT actually creates a file (proves the access mode + Create values)
    create_makesFile: (ut) ->
      path = tmpPath!
      fd = ffiPosix.open path, bit.bor(Access.Write, Create), ffiPosix.getFileMode "rw"
      ut\assertGreaterThanOrEquals fd, 0
      ffiPosix.close fd if fd >= 0
      ut\assertEquals lfs.attributes(path, "mode"), "file"
      os.remove path

    -- O_CREAT | O_EXCL fails when the file already exists (proves the Exclusive value)
    exclusive_failsOnExisting: (ut) ->
      path = tmpPath!
      fd1 = ffiPosix.open path, bit.bor(Access.Write, Create), ffiPosix.getFileMode "rw"
      ffiPosix.close fd1 if fd1 >= 0
      fd2 = ffiPosix.open path, bit.bor(Access.Write, Create, Exclusive), ffiPosix.getFileMode "rw"
      ut\assertTrue fd2 < 0 -- EEXIST
      ffiPosix.close fd2 if fd2 >= 0
      os.remove path

    -- getFileMode's bits become the real on-disk permissions (with the umask cleared)
    getFileMode_setsPermissions: (ut) ->
      oldMask = ffi.C.umask 0
      path = tmpPath!
      fd = ffiPosix.open path, bit.bor(Access.Write, Create), ffiPosix.getFileMode "rw", "r", "r"
      ffiPosix.close fd if fd >= 0
      ffi.C.umask oldMask
      perms = lfs.attributes path, "permissions"
      os.remove path
      ut\assertEquals perms, "rw-r--r--" -- 0o644
  }
