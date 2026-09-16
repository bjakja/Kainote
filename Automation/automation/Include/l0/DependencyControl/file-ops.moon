ffi = require "ffi"
lfs = require "lfs"
constants = require "l0.DependencyControl.Constants"
Logger = require "l0.DependencyControl.Logger"
domain = require "l0.DependencyControl.domain"
utils = require "l0.DependencyControl.utils"
Hash = require "l0.DependencyControl.hash"

ENOENT = 2 -- POSIX error code for "No such file or directory"
ENOTDIR = 20 -- POSIX error code for "Not a directory"
ERROR_PATH_NOT_FOUND = 3 -- Windows error code for "The system cannot find the path specified"

local ConfigView, FileOps

-- Filesystem path length limits.
WINDOWS_MAX_PATH = 260 -- Windows with long path support disabled
WINDOWS_LONG_PATH_MAX = 32767 -- Windows with long path support enabled
MAX_PATH_COMPONENT = 255 -- per-segment limit on NTFS and common POSIX filesystems
POSIX_PATH_MAX = 4096 -- typical full-path limit on modern POSIX systems

---Reports whether the current process can use paths beyond the legacy MAX_PATH limit.
---@return boolean enabled True when this process may use long paths.
detectProcessLongPathsEnabled = ->
  -- ntdll!RtlAreLongPathsEnabled gives the effective per-process answer, folding in both the
  -- system registry policy and the process's manifest opt-in. A process whose executable manifest
  -- lacks the `longPathAware` setting stays capped at MAX_PATH even when the registry enables long
  -- paths. The symbol arrived in Windows 10 1607, when long paths were introduced. On older systems
  -- it is absent and long paths are unsupported, so they read as disabled.
  okLib, ntdll = pcall ffi.load, "ntdll"
  return false unless okLib
  pcall ffi.cdef, "unsigned char RtlAreLongPathsEnabled(void);"
  ok, enabled = pcall -> ntdll.RtlAreLongPathsEnabled! != 0
  return ok and enabled

---Reads the system-wide LongPathsEnabled policy from the Windows registry (HKLM\…\Control\FileSystem).
---@return boolean enabled True when the value is present and set to 1; false when it is missing, zero, or unreadable.
detectRegistryLongPathsEnabled = ->
  -- This reflects the system policy only, not the per-process manifest. It exists just to tailor
  -- the diagnostic when a path is rejected — telling "long paths are off system-wide" apart from
  -- "they're on, but this application isn't long-path-aware".
  okLib, advapi = pcall ffi.load, "advapi32"
  return false unless okLib
  pcall ffi.cdef, [[
    long RegOpenKeyExA(uintptr_t hKey, const char* subKey, unsigned long options, unsigned long samDesired, uintptr_t* result);
    long RegQueryValueExA(uintptr_t hKey, const char* valueName, unsigned long* reserved, unsigned long* type, unsigned char* data, unsigned long* dataSize);
    long RegCloseKey(uintptr_t hKey);
  ]]
  -- HKEY_LOCAL_MACHINE is (HKEY)(LONG)0x80000002; the int32->uintptr cast reproduces
  -- the sign-extended pointer value the API expects on both 32- and 64-bit builds.
  HKEY_LOCAL_MACHINE = ffi.cast "uintptr_t", ffi.cast "int32_t", 0x80000002
  KEY_READ, ERROR_CODE_SUCCESS = 0x20019, 0
  hKey = ffi.new "uintptr_t[1]"
  return false unless ERROR_CODE_SUCCESS == advapi.RegOpenKeyExA HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Control\\FileSystem", 0, KEY_READ, hKey
  value = ffi.new "unsigned long[1]"
  size = ffi.new "unsigned long[1]", ffi.sizeof "unsigned long"
  status = advapi.RegQueryValueExA hKey[0], "LongPathsEnabled", nil, nil,
    ffi.cast("unsigned char*", value), size
  advapi.RegCloseKey hKey[0]
  return status == ERROR_CODE_SUCCESS and value[0] == 1

windowsProcessLongPathsEnabled, windowsRegistryLongPathsEnabled = false, false
if ffi.os == "Windows"
  ok, res = pcall detectProcessLongPathsEnabled
  windowsProcessLongPathsEnabled = ok and res
  -- only needed to explain *why* long paths are unavailable
  unless windowsProcessLongPathsEnabled
    ok, res = pcall detectRegistryLongPathsEnabled
    windowsRegistryLongPathsEnabled = ok and res

defaultLogger = Logger!

msgs = {
  generic: {
    deletionRescheduled: "Another deletion attempt has been rescheduled for the next restart."
  }
  attributes: {
    badPath: "Path failed verification: %s."
    genericError: "Can't retrieve attributes: %s."
    noAttribute: "Can't find attribute with name '%s'."
  }

  createConfig: {
    handlerFailed: "Couldn't create ConfigHandler for the FileOps configuration file: %s"
  },
  createTempDir: {
    failedCreate: "Failed to create temporary directory: %s"
  }
  mkdir: {
    createError: "Couldn't create directory: %s."
    otherExists: "Couldn't create directory because a %s of the same name is already present."
  }
  copy: {
    genericError: "An error occurred while copying file '%s' to '%s':\n%s"
    dirCopyUnsupported: "Copying directories is currently not supported."
    missingSource: "Couldn't find source file '%s'."
    openError: "Couldn't open %s file '%s' for reading: \n%s"
  },
  exists: {
    notFound: "No such file or directory: '%s'."
    wrongType: "Expected %s to be a %s but found a %s."
  }
  listDir: {
    notADirectory: "Can only list directories but supplied path '%s' points to a %s."
  },
  joinPath: {
    invalidSegment: "Invalid path segment type: expected a string or pure array table, got '%s'."
  }
  move: {
    inUseTryingRename: "Target file '%s' already exists and appears to be in use. Trying to rename and delete existing file..."
    renamedDeletionFailed: "The existing file was successfully renamed to '%s', but couldn't be deleted (%s).\n%s"
    overwritingFile: "File '%s' already exists, overwriting..."
    createdDir: "Created target directory '%s'."
    exists: "Couldn't move file '%s' to '%s' because a %s of the same name is already present."
    genericError: "An error occurred while moving file '%s' to '%s':\n%s"
    createDirError: "Could not create target directory for '%s': %s"
    cantRemove: "Couldn't overwrite file '%s': %s. Attempts at renaming the existing target file failed."
    cantRenameTryingCopy: "Move operation failed to rename '%s' to '%s' (%s), trying copy+remove instead..."
    removeFilesFailed: "Move operation succeeded in copying the file(s) to the target location, but some of the source files couldn't be removed:\n%s\n%s"
    cantCopy: "Move operation failed to copy '%s' to '%s' (%s) after a failed rename attempt (%s)."
  }
  readFile: {
    cantOpen: "Couldn't open file '%s' for reading: %s"
    cantRead: "An error occurred while trying to read from file '%s': %s"
    notAFile: "Can only read files but supplied path '%s' points to a %s."
  }
  writeFile: {
    cantOpen: "Couldn't open file '%s' for writing: %s"
    failedWrite: "An error occurred while trying to write to file '%s': %s",
    notAFile: "Can only write to files but supplied path '%s' points to a %s.",
    targetExists: "Target file '%s' already exists."
  }
  remove: {
    noConfigReschedule: "Couldn't load the FileOps config file (%s) - deletions of %s cannot be rescheduled!"
  }
  rmdir: {
    emptyPath: "Argument #1 (path) must not be an empty string."
    removeFilesFailed: "Some of the files and folders in the specified directory couldn't be removed:\n%s"
    removeDirFailed: "Couldn't remove empty directory: %s.",
    notFound: "No such file or directory: '%s'."
    notDir: "Expected '%s' to be a directory but found a %s."
  }
  runScheduledRemoval: {
    noConfigReschedule: "Couldn't load the FileOps config file (%s) - rescheduled deletions will not be performed!"
  }
  getNamespacedPath: {
    badBasePath: "Provided base path '%s' is not a valid full path (%s)."
    badPath: "Could not generate a valid full path from base path '%s' and namespaced sub-path '%s': %s."
  }
  validateFullPath: {
    badType: "Argument #%s (%s) had the wrong type. Expected 'string', got '%s'."
    tooLong: "The specified path exceeded the maximum length limit (%d > %d)."
    tooLongRegistryDisabled: "The specified path exceeded the Windows MAX_PATH limit (%d > %d characters) and long path support is disabled on this system.\nEnable it by setting the registry value 'HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\FileSystem\\LongPathsEnabled' (DWORD) to 1 and restarting, e.g. by running this in an elevated PowerShell:\n  Set-ItemProperty -Path 'HKLM:\\SYSTEM\\CurrentControlSet\\Control\\FileSystem' -Name 'LongPathsEnabled' -Value 1 -Type DWord"
    tooLongProcessUnaware: "The specified path exceeded the Windows MAX_PATH limit (%d > %d characters). Long path support is enabled system-wide, but the host application is not long-path-aware (its executable manifest lacks the 'longPathAware' setting), so paths remain capped at %d characters in this process."
    segmentTooLong: "A path component exceeded the maximum length limit (%d > %d): '%s'."
    invalidChars: "The specified path contains one or more invalid characters: '%s'."
    reservedNames: "The specified path contains reserved path or file names: '%s'."
    parentPath: "Accessing parent directories is not allowed."
    notFullPath: "The specified path is not a valid full path."
    missingExt: "The specified path is missing a file extension."
  }
}

windowsReservedNameSet = {n, true for n in *{
  "CON", "PRN", "AUX", "NUL",
  "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
  "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
}}

-- effective full-path limit; on Windows this depends on whether *this process*
-- can use long paths (see detectProcessLongPathsEnabled)
pathMaxLength = if ffi.os == "Windows"
  windowsProcessLongPathsEnabled and WINDOWS_LONG_PATH_MAX or WINDOWS_MAX_PATH
else POSIX_PATH_MAX

---Lazily creates and caches the FileOps deletion-tracking config on the module table.
---@param noLoad? boolean Don't read the file from disk when the handler is created.
---@param configDir? string Directory holding the config; sets or overrides the cached location.
---@return ConfigView? config The cached config view, or nil on failure.
---@return string? err
createConfig = (noLoad, configDir) ->
  FileOps.configDir = configDir if configDir
  ConfigView or= require "#{constants.DEPCTRL_NAMESPACE}.ConfigView"
  unless FileOps.config
    FileOps.config = ConfigView\get "#{FileOps.configDir}/#{constants.DEPCTRL_NAMESPACE}.json",
      nil, {toRemove: {}}, defaultLogger, noLoad
    return nil, msgs.createConfig.handlerFailed\format "constructor returned nil" unless FileOps.config
  return FileOps.config

---Creates `dir` along with any missing parent directories, building the path up one
---segment at a time. Idempotent: levels that already exist are left untouched.
---@param dir string A validated, absolute directory path.
---@return boolean? success True on success, or nil on error.
---@return string dirPathOrError The directory path on success, or an error message.
mkdirRecursive = (dir) ->
  -- preserve a leading separator so POSIX absolute paths keep their root
  accumulator, first = dir\match("^[/\\]") and FileOps.pathSep or "", true
  for segment in FileOps.pathSegments dir
    accumulator = first and accumulator .. segment or "#{accumulator}#{FileOps.pathSep}#{segment}"
    first = false
    continue if accumulator\match "^%a:$" -- skip bare drive letters like "C:"
    unless lfs.attributes accumulator, "mode"
      _, err = lfs.mkdir accumulator
      -- tolerate races and pre-existing levels; only fail if it's still absent
      if err and not lfs.attributes accumulator, "mode"
        return nil, msgs.mkdir.createError\format err
  return true, dir

---@class FileOpsAttributesInfo
---@field attr table|string|number|false The requested attribute(s), or false when the entry doesn't exist.
---@field path string The validated full path.
---@field dev string The device component of the path.
---@field dir string The directory component of the path.
---@field file string The file name component of the path.

---Filesystem utility helpers used by DependencyControl.
---@class FileOps
FileOps = {
  pathSep: ffi.os == "Windows" and "\\" or "/"
  pathMatch: {
    sep: ffi.os == "Windows" and "\\" or "/"
    sepAll: ffi.os == "Windows" and "[\\/]" or "/"
    invalidChars: '[<>:"|%?%*%z%c;]'
  }
  ---@type integer
  pathMaxLength: pathMaxLength
  pathMaxSegmentLength: MAX_PATH_COMPONENT
  -- true when running on Windows but capped at the legacy MAX_PATH limit because this process
  -- can't use long paths. Drives the descriptive error below, and is always false off Windows.
  longPathsDisabled: ffi.os == "Windows" and not windowsProcessLongPathsEnabled
  -- when capped, whether the system registry policy enables long paths -- lets the error
  -- tell a system-wide opt-out apart from an app that isn't long-path-aware
  windowsRegistryLongPathsEnabled: windowsRegistryLongPathsEnabled

  ---Creates a unique temporary directory and returns its path.
  ---@return string? tempDirPath Absolute path to the created temporary directory, or nil if it couldn't be created.
  ---@return string? err Error message if the directory couldn't be created.
  createTempDir: () ->
    tempDir = FileOps.getTempDir()
    res, dir = FileOps.mkdir tempDir
    return tempDir if res
    return nil, msgs.createTempDir.failedCreate\format dir

  ---Generates a unique temporary directory path that does not exist yet.
  ---@return string tempDirPath Absolute path to a unique, not-yet-existing temporary directory.
  getTempDir: () ->
    return aegisub.decode_path "?temp/#{constants.DEPCTRL_NAMESPACE}_#{'%04X'\format math.random 0, 16^4-1}"

  ---Removes one or more files/directories and optionally reschedules failed removals.
  ---@param paths string|(string|string[])[] Path, or list of paths (each a string or an array of path segments).
  ---@param recurse? boolean Recurse into directories (default false, so a non-empty directory is not removed).
  ---@param reSchedule? boolean Reschedule failed removals for the next restart.
  ---@return boolean? overallSuccess True if all succeeded, false if any were rescheduled, nil on hard failure.
  ---@return table details Per-path result tables keyed by path.
  ---@return string? firstErr The first error encountered.
  remove: (paths, recurse = false, reSchedule) ->
    config, configLoaded, overallSuccess, details, firstErr = nil, false, true, {}
    paths = {paths} unless type(paths) == "table"

    for path in *paths
      info, attrErr = FileOps.getAttributes path, "mode"
      unless info
        -- report a hard failure to resolve or stat the path, which is distinct from an absent target
        firstErr or= attrErr
        details[path] = {nil, attrErr}
        overallSuccess = nil
        continue

      path = info.path
      if info.attr
        rmFunc = info.attr == "file" and os.remove or FileOps.rmdir
        res, err = rmFunc path, recurse
        unless res
          firstErr or= err
          unless reSchedule -- delete operation failed entirely
            details[path] = {nil, err}
            overallSuccess = nil
            continue

          -- load the FileOps configuration file and reschedule deletions
          unless configLoaded
            config, msg = createConfig true
            if config
              FileOps.config\load!
              configLoaded = true
            else
              defaultLogger\warn msgs.remove.noConfigReschedule, msg, defaultLogger\dumpToString paths
              details[path] = {nil, err}
              overallSuccess = nil
              continue

          config.c.toRemove[path] = os.time!
          -- mark the operations as failed "for now", indicating a second attempt has been scheduled
          details[path] = {false, err}
          overallSuccess = false

        -- delete operation succeeded
        else details[path] = {true}
      -- file not found or permission issue
      else details[path] = {nil, path}

    config\save! if configLoaded
    return overallSuccess, details, firstErr

  ---Replays removals previously scheduled by remove().
  ---@param configDir? string Directory holding the FileOps config (defaults to the configured dir).
  ---@return boolean? success
  ---@return string? err
  runScheduledRemoval: (configDir) ->
    config, msg = createConfig false, configDir
    unless config
      msg = msgs.runScheduledRemoval.noConfigReschedule\format msg
      defaultLogger\warn msg
      return nil, msg
    paths = [path for path, _ in pairs config.c.toRemove]
    if #paths > 0
      -- rescheduled removals will not be rescheduled another time
      FileOps.remove paths, true
      config.c.toRemove = {}
      config\save!
    return true

  ---Copies a file to a target path.
  ---@param source string
  ---@param target string
  ---@param clobber? boolean Overwrite an existing target file.
  ---@return boolean success
  ---@return string? err
  copy: ( source, target, clobber ) ->
    -- source check
    info, err = FileOps.getAttributes source, "mode"
    return false, msgs.copy.genericError\format source, target, err unless info
    {attr: mode, path: sourceFullPath, file: fileName} = info
    switch mode
      when "directory"
        return false, msgs.copy.dirCopyUnsupported
      when false
        return false, msgs.copy.missingSource\format source

    -- target check
    checkTarget = (target) ->
      info, err = FileOps.getAttributes target, "mode"
      return false, msgs.copy.genericError\format source, target, err unless info
      switch info.attr
        when "file"
          return false, msgs.writeFile.targetExists\format target unless clobber
        when "directory"
          target ..= "/#{fileName}"
          return checkTarget target
      return true, info.path

    success, targetFullPath = checkTarget target
    return false, targetFullPath unless success

    input, msg = io.open sourceFullPath, "rb"
    unless input
      return false, msgs.copy.openError\format "source", sourceFullPath, msg

    output, msg = io.open targetFullPath, "wb"
    unless output
      input\close!
      return false, msgs.copy.openError\format "target", targetFullPath, msg

    success, msg = output\write input\read "*a"
    input\close!
    output\close!

    if success
      return true
    else
      return false, msgs.copy.genericError\format sourceFullPath, targetFullPath, msg

  ---Lists the names of a directory's entries, excluding `.` and `..`.
  ---@param dirPath string|string[] Path or path segments of the directory.
  ---@return string[]? entries The entry names, or nil when the path isn't a directory.
  ---@return string? err
  listDir: (dirPath) ->
    info, err = FileOps.getAttributes dirPath, "mode"
    return nil, err unless info
    return nil, msgs.listDir.notADirectory\format info.path, info.attr if info.attr != "directory"
    return [entry for entry in lfs.dir(info.path) when entry != "." and entry != ".."]

  ---Recursively collects all files below a directory.
  ---@param dirPath string|string[] Path or path segments of the directory to walk.
  ---@return string[]? files Full paths of every file below the directory (joined onto the given path), or nil when it can't be listed.
  ---@return string? err
  listFilesRecursive: (dirPath) ->
    entries, err = FileOps.listDir dirPath
    return nil, err unless entries
    files = {}
    for entry in *entries
      fullPath = FileOps.joinPath dirPath, entry
      info = FileOps.getAttributes fullPath, "mode"
      mode = info and info.attr
      if mode == "directory"
        for file in *(FileOps.listFilesRecursive(fullPath) or {})
          files[#files + 1] = file
      elseif mode == "file"
        files[#files + 1] = fullPath
    return files

  ---Joins and resolves multiple path segments into a single path string.
  ---@param ... string|string[] One or more path segments, or arrays of path segments.
  ---@return string? joinedPath The path segments joined by OS-specific separators, or nil on error.
  ---@return string? err
  joinPath: (...) ->
    args = {...}
    -- detect root from the first string before splitting consumes separators
    firstStr = type(args[1]) == "table" and args[1][1] or args[1]
    return nil, msgs.joinPath.invalidSegment\format type firstStr if type(firstStr) ~= "string"
    absolutePathRoot = type(firstStr) == "string" and FileOps.__getPathRoot firstStr

    invalidPathSegmentType = nil
    flatPathSegments = utils.flatten args, 3, (value, typ) ->
      if typ != "string"
        invalidPathSegmentType = typ
        return {}, true -- error is raised below via invalidPathSegmentType; contribute nothing here

      firstSegment, moreSegments = nil, nil
      for segment in FileOps.pathSegments value
        if firstSegment
          moreSegments or= {firstSegment}
          table.insert moreSegments, segment
        else firstSegment = segment
      -- an empty or separator-only segment has no components: return {} so it adds nothing, rather
      -- than a nil that would leave a hole and stop the ipairs walk over the flattened segments
      return {}, true unless firstSegment
      return moreSegments or firstSegment, moreSegments
    return nil, msgs.joinPath.invalidSegment\format invalidPathSegmentType if invalidPathSegmentType

    -- filter extraneous '.', resolve '..', and clamp path traversal at root
    segments = {}
    for i, segment in ipairs flatPathSegments
      switch segment
        when "." then segments[#segments + 1] = segment if i == 1 and not absolutePathRoot
        when ".."
          if #segments > (absolutePathRoot and 1 or 0) and segments[#segments] != ".."
            segments[#segments] = nil
          elseif not absolutePathRoot
            segments[#segments + 1] = segment
        else segments[#segments + 1] = segment
    -- re-add root separator for absolute paths on POSIX systems removed by splitting
    return "#{absolutePathRoot and ffi.os != "Windows" and FileOps.pathSep or ""}#{table.concat segments, FileOps.pathSep}"

  ---Returns an iterator over the non-empty components of a path, split on any separator.
  ---@param path string
  ---@return fun(): string? iterator
  pathSegments: (path) -> path\gmatch "[^/\\]+"

  ---Moves a file to a target path, optionally replacing existing targets.
  ---@param source string
  ---@param target string
  ---@param overwrite? boolean Replace an existing target file.
  ---@return boolean success
  ---@return string? err
  move: (source, target, overwrite) ->
    info, err = FileOps.getAttributes target, "mode"
    return false, msgs.move.genericError\format source, target, err unless info
    mode = info.attr
    if mode == "file"
      unless overwrite
        return false, msgs.move.exists\format source, target, mode
      defaultLogger\trace msgs.move.overwritingFile, target
      res, _, err = FileOps.remove target
      unless res
        -- can't remove old target file, probably in use or lack of permissions
        -- try to rename and then delete it
        defaultLogger\debug msgs.move.inUseTryingRename, target
        junkName = "#{target}.depCtrlRemoved"
        -- There might be an old removed file we couldn't delete before
        FileOps.remove junkName
        res = os.rename target, junkName
        unless res
          return false, msgs.move.cantRemove\format target, err
        -- rename succeeded, now clean up after ourselves
        res, _, err = FileOps.remove junkName, false, true
        unless res
          defaultLogger\debug msgs.move.renamedDeletionFailed, junkName, err, msgs.generic.deletionRescheduled

    elseif mode -- a directory (or something else) of the same name as the target file is already present
      return false, msgs.move.exists\format source, target, mode

    else -- target file not found, check directory
      res, dirOrErr = FileOps.mkdir target, true, true
      if res == nil
        return false, msgs.move.createDirError\format source, target, dirOrErr
      elseif res
        defaultLogger\trace msgs.move.createdDir, dirOrErr

    -- at this point the target directory exists and the target file doesn't, move the file
    res, err = os.rename source, target
    unless res
      -- renaming the file failed, could be because of a permission issue
      -- but me might a well be trying to rename over file system boundaries on *nix
      -- so we should try copy + remove before giving up
      defaultLogger\debug msgs.move.cantRenameTryingCopy, source, target, err
      renErr, res, err = err, FileOps.copy source, target
      unless res
        return false, msgs.move.cantCopy\format source, target, err, renErr
      res, details = FileOps.remove source, false, true -- TODO: also support directories/recursion, but also require copy to support it

      unless res
        fileList = table.concat ["#{path}: #{res[2]}" for path, res in pairs details when not res[1]], "\n"
        defaultLogger\debug msgs.move.removeFilesFailed, fileList, msgs.generic.deletionRescheduled

    return true

  ---Reads and returns the full contents of a file.
  ---@param path string|string[] Path or path segments to the file to read.
  ---@return string? data The contents of the file, or nil if an error occurred.
  ---@return string? err An error message if an error occurred.
  readFile: (path) ->
    info, err = FileOps.getAttributes path, "mode"
    return nil, err unless info
    return nil, msgs.readFile.cantOpen\format path, info.path unless info.attr
    return nil, msgs.readFile.notAFile\format path, info.attr if info.attr != "file"

    handle, msg = io.open info.path, "rb"
    return nil, msgs.readFile.cantOpen\format fullPath, msg unless handle

    data, msg = handle\read "*a"
    handle\close!

    if data
      return data
    else return nil, msgs.readFile.cantRead\format path, msg

  ---Writes data to a file, creating the file if it doesn't exist and optionally overwriting existing files.
  ---@param path string|string[] Path or path segments to the file to write.
  ---@param data string The data to write to the file.
  ---@param clobber? boolean Overwrite the file if it already exists (default false).
  ---@return boolean success True if the file was written successfully.
  ---@return string? err
  writeFile: (path, data, clobber = false) ->
    info, err = FileOps.getAttributes path, "mode"
    return false, err unless info
    return false, msgs.writeFile.notAFile\format path, info.attr if info.attr and info.attr != "file"
    return false, msgs.writeFile.targetExists\format path if info.attr == "file" and not clobber

    handle, msg = io.open info.path, "wb"
    return false, msgs.writeFile.cantOpen\format fullPath, msg unless handle

    success, msg = handle\write data
    handle\close!
    return true if success
    return false, msgs.writeFile.failedWrite\format fullPath, msg

  ---Reads a file and computes the hash of its contents.
  ---@param fileName string|string[] Path or path segments to the file to hash.
  ---@param hashType? HashType The hash algorithm to use (default Sha1).
  ---@return string? hexDigest The lowercase hex digest, or nil if an error occurred.
  ---@return string? err An error message if an error occurred.
  getHash: (fileName, hashType = Hash.HashType.Sha1) ->
    data, readErr = FileOps.readFile fileName
    return nil, readErr unless data
    Hash.getDigest hashType, data

  ---Reads a file and verifies its contents match an expected hash.
  ---@param fileName string|string[] Path or path segments to the file to verify.
  ---@param hash string The expected hex digest (compared case-insensitively).
  ---@param hashType? HashType The hash algorithm to use (default Sha1).
  ---@return boolean? match True on match, false on mismatch, or nil on error.
  ---@return string? err The mismatch detail or error message.
  verifyHash: (fileName, hash, hashType = Hash.HashType.Sha1) ->
    data, readErr = FileOps.readFile fileName
    return nil, readErr unless data
    Hash.verify hashType, data, hash

  ---Removes a directory, by default together with everything it contains.
  ---@param path string|string[] Path or path segments to the directory to remove.
  ---@param recurse? boolean Remove the directory's contents first (default true); when false, only an already-empty directory is removed.
  ---@return boolean? success True on success, or nil on error.
  ---@return string? err An error message when the path is empty, doesn't exist, isn't a directory, or something couldn't be removed.
  rmdir: (path, recurse = true) ->
    return nil, msgs.rmdir.emptyPath if path == ""
    info, err = FileOps.getAttributes path, "mode"
    return nil, err unless info
    path = info.path
    return nil, msgs.rmdir.notFound\format path if info.attr == false
    return nil, msgs.rmdir.notDir\format path, info.attr unless info.attr == "directory"

    if recurse
      -- recursively remove contained files and directories
      toRemove = [FileOps.joinPath(path, file) for file in *FileOps.listDir path]
      res, details = FileOps.remove toRemove, true
      unless res
        fileList = table.concat ["#{path}: #{res[2]}" for path, res in pairs details when not res[1]], "\n"
        return nil, msgs.rmdir.removeFilesFailed\format fileList

    -- remove empty directory
    success, err = lfs.rmdir path
    -- lfs implementations disagree on the success value (LuaFileSystem returns true,
    -- Aegisub's lfs returns nothing), so only an explicit error or a directory that
    -- still exists counts as failure
    unless not err and (success or not lfs.attributes(path, "mode"))
      return nil, msgs.rmdir.removeDirFailed\format(err or "unknown error")

    return true

  ---Creates a directory.
  ---@param path string|string[] Path or path segments to the directory to create.
  ---@param isFile boolean Whether the path is a file path (discards the last segment when checking/creating the directory).
  ---@param recurse? boolean Also create any missing parent directories (default false).
  ---@return boolean? created True if created, false if it already existed, nil if an error occurred.
  ---@return string dirPathOrError The existing/created directory path, or an error message.
  mkdir: (path, isFile, recurse) ->
    info, err = FileOps.getAttributes path, "mode"
    return nil, err unless info
    {attr: mode, path: fullPath, :dev, :dir, :file} = info
    dir = isFile and table.concat({dev, dir or file}) or fullPath

    if not mode
      return mkdirRecursive dir if recurse
      res, err = lfs.mkdir dir
      -- lfs implementations disagree on the success value (LuaFileSystem returns true,
      -- Aegisub's lfs returns nothing), so only an explicit error or a directory that
      -- is still missing counts as failure
      unless not err and (res or "directory" == lfs.attributes(dir, "mode"))
        return nil, msgs.mkdir.createError\format(err or "unknown error")
      return true, dir
    elseif isFile and mode == "file" -- if the file already exists, so does the directory
      return false, dir
    elseif mode != "directory" -- a file of the same name as the target directory is already present
      return nil, msgs.mkdir.otherExists\format mode
    return false, dir

  ---Retrieves file or directory attributes along with the parsed components of its path.
  ---@param path string|string[] Either a path or an array of path segments.
  ---@param key? string Attribute name to retrieve (e.g. "mode", "size", "modification"), or nil for the full attribute table.
  ---@return FileOpsAttributesInfo? info The attributes and path components, or nil on a hard error (an invalid path or an lfs failure). A path that simply doesn't exist is not an error: `info.attr` is then false.
  ---@return string? err An error message, present only when info is nil.
  getAttributes: (path, key) ->
    fullPath, dev, dir, file = FileOps.validateFullPath path, false, lfs.currentdir!
    unless fullPath
      return nil, msgs.attributes.badPath\format dev

    attr, err, errCode = lfs.attributes fullPath, key
    if attr
      return {:attr, path: fullPath, :dev, :dir, :file}
    -- Aegisub's lfs implementation signals a non-existent file/dir with a bare nil,
    -- while the stock library (https://lunarmodules.github.io/luafilesystem/; v1.7.0+)
    -- returns an error code alongside an error message
    elseif err == nil or errCode == ENOENT or errCode == ERROR_PATH_NOT_FOUND or errCode == ENOTDIR
      return {attr: false, path: fullPath, :dev, :dir, :file}
    else
      return nil, msgs.attributes.genericError\format err

  ---Retrieves file or directory attributes.
  ---@deprecated Use `getAttributes`, which returns a single info table plus an error message.
  ---@param path string|string[] Either a path or an array of path segments.
  ---@param key? string Attribute name to retrieve (e.g. "mode", "size", "modification"), or nil for the full attribute table.
  ---@return table|string|number|boolean|nil attr The requested attribute(s), false if absent, or nil on error.
  ---@return string fullPath The validated full path, or an error message if the path was invalid.
  ---@return string? device The device component of the path.
  ---@return string? dir The directory component of the path.
  ---@return string? file The file name component of the path.
  attributes: (path, key) ->
    info, err = FileOps.getAttributes path, key
    return nil, err unless info
    return info.attr, info.path, info.dev, info.dir, info.file

  ---Checks whether a file or directory exists and optionally verifies its type.
  ---@param path string|string[] Either a path or an array of path segments.
  ---@param expectedMode? string If specified, the required type of the filesystem entry.
  ---@return boolean? exists True if it exists and matches the expected type, false if not, nil on error.
  ---@return string? err An error message if the file doesn't exist or is of the wrong type.
  exists: (path, expectedMode) ->
    info, err = FileOps.getAttributes path, "mode"
    return nil, err unless info
    return false, msgs.exists.notFound\format info.path unless info.attr
    return true if not expectedMode or info.attr == expectedMode
    return false, msgs.exists.wrongType\format info.path, expectedMode, info.attr

  ---Extracts the root anchor of an absolute path.
  ---@private
  ---@param absolutePath string The absolute path to inspect.
  ---@return string? root On Windows the drive prefix with its separator (e.g. "C:\"), on POSIX the leading slash plus first segment (e.g. "/usr"), or nil when the path has no such root.
  __getPathRoot: (absolutePath) ->
    return absolutePath\match "^[A-Za-z]:[/\\]" if ffi.os == "Windows"
    return absolutePath\match "^/[^/\\]+"

  ---Validates and normalizes an absolute filesystem path.
  ---@param path string|string[] Either a path or an array of path segments.
  ---@param checkFileExt? boolean Require the path to have a file extension.
  ---@param basePath? string|string[] Base path to resolve relative paths against; relative paths are rejected without it.
  ---@return string|false|nil normalizedPath The normalized path, or false/nil on error.
  ---@return string? deviceOrErr The device/root component on success, or an error message on failure.
  ---@return string? dir The directory component (success only).
  ---@return string? file The file name component (success only).
  validateFullPath: (path, checkFileExt, basePath) ->
    if "table" == type path
      path, errMsg = FileOps.joinPath path
      return nil, errMsg if not path
    elseif "string" != type path
      return nil, msgs.validateFullPath.badType\format 1, "path", type(path)

    if "table" == type basePath
      basePath, errMsg = FileOps.joinPath basePath
      return nil, errMsg if not basePath
    elseif basePath and "string" != type basePath
      return nil, msgs.validateFullPath.badType\format 3, "basePath", type(basePath)

    -- expand aegisub path specifiers
    path = aegisub.decode_path path
    -- expand home directory on linux
    homeDir = os.getenv "HOME"
    path = path\gsub "^~", "#{homeDir}/" if homeDir
    -- use single native path separators
    path = path\gsub "[\\/]+", FileOps.pathSep
    -- check length
    if #path > FileOps.pathMaxLength
      if FileOps.longPathsDisabled
        -- distinguish a system-wide opt-out from an app that isn't long-path-aware
        if FileOps.windowsRegistryLongPathsEnabled
          return nil, msgs.validateFullPath.tooLongProcessUnaware\format #path, FileOps.pathMaxLength, FileOps.pathMaxLength
        return nil, msgs.validateFullPath.tooLongRegistryDisabled\format #path, FileOps.pathMaxLength
      return nil, msgs.validateFullPath.tooLong\format #path, FileOps.pathMaxLength
    -- check for invalid characters
    invChar = path\match FileOps.pathMatch.invalidChars, ffi.os == "Windows" and 3 or nil
    if invChar
      return nil, msgs.validateFullPath.invalidChars\format invChar
    -- check if path is absolute
    dev = FileOps.__getPathRoot path
    unless dev
      -- make relative paths absolute if base path is provided
      if basePath
        path, errMsg = FileOps.joinPath basePath, path
        return nil, errMsg if not path
        dev = FileOps.__getPathRoot path
      else return false, msgs.validateFullPath.notFullPath
    -- parse path structure
    rest = path\sub #dev + 1
    dir, file = rest\match "^(.*)[/\\]([^/\\]*)$"
    unless dir
      return false, msgs.validateFullPath.notFullPath
    for segment in FileOps.pathSegments rest
      if #segment > FileOps.pathMaxSegmentLength
        return nil, msgs.validateFullPath.segmentTooLong\format #segment, FileOps.pathMaxSegmentLength, segment
      if ffi.os == "Windows"
        segmentWithoutExt = segment\match("^[^%.]+") or segment
        if windowsReservedNameSet[segmentWithoutExt\upper!]
          return nil, msgs.validateFullPath.reservedNames\format segmentWithoutExt
      unless segment\match "[^%.%s]$"
        return nil, msgs.validateFullPath.notFullPath
    file = file != "" and file or nil
    if checkFileExt and not (file and file\match ".+%.+")
      return false, msgs.validateFullPath.missingExt

    path = table.concat {dev, dir, file and FileOps.pathSep, file}
    return path, dev, dir, file

  ---Converts a base path and namespace into a namespaced filesystem path.
  ---Dots in the namespace are converted to path separators when nested is true.
  ---@param basePath string|string[] Base path (or segments) the namespaced path is created under.
  ---@param namespace string
  ---@param ext string File extension (including the dot).
  ---@param nested? boolean Convert namespace dots to path separators (default true).
  ---@return string? path
  ---@return string? err
  getNamespacedPath: (basePath, namespace, ext, nested = true) ->
    res, msg = domain.validateNamespace namespace
    return nil, msg unless res

    fullBasePath, msg = FileOps.validateFullPath basePath
    return nil, msgs.getNamespacedPath.badBasePath\format basePath, msg unless fullBasePath

    namespacePath = "#{nested and namespace\gsub("%.", FileOps.pathSep) or namespace}#{ext}"
    normalizedFullPath, msg = FileOps.validateFullPath namespacePath, false, fullBasePath
    return nil, msgs.getNamespacedPath.badPath\format fullBasePath, namespacePath, msg unless normalizedFullPath

    return normalizedFullPath
}

return FileOps
