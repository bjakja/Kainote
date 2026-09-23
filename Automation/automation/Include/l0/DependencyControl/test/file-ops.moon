-- FileOps tests: path validation and filesystem utilities.
-- Called from Tests.moon as: (require "...test.FileOps") basePath, isWindows
(basePath, isWindows) ->
  ffi = require "ffi"
  lfs = require "lfs"
  fileOps = require "l0.DependencyControl.file-ops"
  pathSep = isWindows and "\\" or "/"

  FILEOPS_MODULE_NAME = "l0.DependencyControl.file-ops"

  -- Runs fn with FileOps' path-length detection results overridden, restoring them
  -- afterwards (even if fn raises) so the platform-derived values don't leak between
  -- tests. Lets us exercise every "path too long" diagnostic branch on any OS.
  withPathLimits = (maxLength, longPathsDisabled, registryEnabled, fn) ->
    saved = {fileOps.pathMaxLength, fileOps.longPathsDisabled, fileOps.windowsRegistryLongPathsEnabled}
    fileOps.pathMaxLength = maxLength
    fileOps.longPathsDisabled = longPathsDisabled
    fileOps.windowsRegistryLongPathsEnabled = registryEnabled
    results = table.pack pcall fn
    fileOps.pathMaxLength, fileOps.longPathsDisabled, fileOps.windowsRegistryLongPathsEnabled = saved[1], saved[2], saved[3]
    error results[2] unless results[1]
    return unpack results, 2, results.n

  {
    _description: "Tests for fileOps path validation and filesystem utilities."

    -- validateFullPath: pure computation, no stubs needed

    validateFullPath_nonString: (ut) ->
      result, err = fileOps.validateFullPath 42
      ut\assertNil result
      ut\assertString err

    validateFullPath_parentDir: (ut) ->
      -- ".." is resolved rather than rejected
      result = fileOps.validateFullPath {basePath, "..", "escape.txt"}
      ut\assertString result -- resolves to parent dir + escape.txt

    validateFullPath_tooLong: (ut) ->
      -- exceed the full-path limit on every platform/config (well past the ~32k
      -- long-path-enabled Windows limit) while keeping each component within bounds
      segments = [string.rep "a", 200 for _ = 1, 200]
      result = fileOps.validateFullPath {basePath, segments}
      ut\assertNil result

    validateFullPath_segmentTooLong: (ut) ->
      -- a single component over the per-segment limit is rejected even when the overall
      -- path fits the length limit (raise the length cap so the segment check is reached)
      result, err = withPathLimits 32767, false, false, ->
        fileOps.validateFullPath {basePath, "#{string.rep 'a', 300}.txt"}
      ut\assertNil result
      ut\assertContains err, "path component"

    -- detected, platform-specific path limits
    pathLimits_detected: (ut) ->
      ut\assertEquals fileOps.pathMaxSegmentLength, 255
      if isWindows
        -- 260 (capped) or 32767 (long paths available to this process)
        ut\assertTrue fileOps.pathMaxLength == 260 or fileOps.pathMaxLength == 32767
        ut\assertBoolean fileOps.longPathsDisabled
      else
        ut\assertEquals fileOps.pathMaxLength, 4096
        ut\assertFalse fileOps.longPathsDisabled

    -- "path too long" diagnostic selection (field-driven via withPathLimits, runs on any OS)
    validateFullPath_tooLong_generic: (ut) ->
      -- non-Windows / long paths available: plain limit message, no Windows-specific guidance
      result, err = withPathLimits 260, false, false, ->
        fileOps.validateFullPath {basePath, [string.rep "a", 200 for _ = 1, 3]}
      ut\assertNil result
      ut\assertContains err, "maximum length limit"

    validateFullPath_tooLong_registryDisabled: (ut) ->
      -- Windows, long paths off system-wide: error explains how to enable the registry key
      result, err = withPathLimits 260, true, false, ->
        fileOps.validateFullPath {basePath, [string.rep "a", 200 for _ = 1, 3]}
      ut\assertNil result
      ut\assertContains err, "LongPathsEnabled"

    validateFullPath_tooLong_processUnaware: (ut) ->
      -- Windows, registry on but app not long-path-aware: error explains the manifest cap
      result, err = withPathLimits 260, true, true, ->
        fileOps.validateFullPath {basePath, [string.rep "a", 200 for _ = 1, 3]}
      ut\assertNil result
      ut\assertContains err, "long-path-aware"

    validateFullPath_invalidChars: (ut) ->
      return unless isWindows
      result = fileOps.validateFullPath {basePath, "with<invalid>.txt"}
      ut\assertNil result

    validateFullPath_reservedNames: (ut) ->
      return unless isWindows
      result = fileOps.validateFullPath {basePath, "CON", "file.txt"}
      ut\assertNil result

    validateFullPath_reservedNameWithExt: (ut) ->
      return unless isWindows
      result = fileOps.validateFullPath {basePath, "NUL.txt"}
      ut\assertNil result

    validateFullPath_trailingDotSegment: (ut) ->
      result = fileOps.validateFullPath {basePath, "trailingDot.", "file.txt"}
      ut\assertNil result

    validateFullPath_valid: (ut) ->
      path, dev, dir, file = fileOps.validateFullPath {basePath, "file.txt"}
      ut\assertString path
      ut\assertString dev
      ut\assertEquals file, "file.txt"

    validateFullPath_noExt_rejected: (ut) ->
      result = fileOps.validateFullPath {basePath, "no-ext"}, true
      ut\assertFalse result

    validateFullPath_withExt_accepted: (ut) ->
      result = fileOps.validateFullPath {basePath, "file.txt"}, true
      ut\assertString result

    validateFullPath_homeDirExpansion: (ut) ->
      return if isWindows
      home = os.getenv "HOME"
      return unless home
      result = fileOps.validateFullPath {"~", "subdir", "file.txt"}
      ut\assertString result
      ut\assertContains result, home

    validateFullPath_reservedNameNonWindows: (ut) ->
      return if isWindows
      result = fileOps.validateFullPath {basePath, "NUL", "file.txt"}
      ut\assertString result

    -- getNamespacedPath: pure computation, no stubs needed

    getNamespacedPath_nested: (ut) ->
      path, err = fileOps.getNamespacedPath basePath, "l0.DependencyControl.Test", ".lua"
      ut\assertNil err
      ut\assertString path
      ut\assertContains path, fileOps.joinPath "l0", "DependencyControl", "Test.lua"

    getNamespacedPath_flat: (ut) ->
      path, err = fileOps.getNamespacedPath basePath, "l0.DependencyControl", ".lua", false
      ut\assertNil err
      ut\assertString path
      ut\assertContains path, "l0.DependencyControl.lua"

    getNamespacedPath_badNamespace: (ut) ->
      path, err = fileOps.getNamespacedPath basePath, "not-a-namespace", ".lua"
      ut\assertNil path
      ut\assertString err

    getNamespacedPath_badBasePath: (ut) ->
      path, err = fileOps.getNamespacedPath {"relative", "path"}, "l0.DependencyControl", ".lua"
      ut\assertNil path
      ut\assertString err

    -- getAttributes: stubs lfs.attributes
    -- lfs.attributes(path, key) returns (value) on success, (nil) when not found, or
    -- (nil, errMsg, errCode) on error. getAttributes maps these to an info table whose
    -- attr is the value or false, or to nil plus an error message on a hard failure.

    getAttributes_file: (ut) ->
      attrStub = (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      info, err = fileOps.getAttributes {basePath, "file.txt"}, "mode"
      ut\assertEquals info.attr, "file"
      ut\assertString info.path
      ut\assertNil err
      attrStub\assertCalledOnceWith fileOps.joinPath(basePath, "file.txt"), "mode"

    getAttributes_notFound: (ut) ->
      attrStub = (ut\stub lfs, "attributes")\calls (path, key) -> nil
      info, err = fileOps.getAttributes {basePath, "missing.txt"}, "mode"
      ut\assertFalse info.attr
      ut\assertString info.path
      ut\assertNil err
      attrStub\assertCalledOnceWith fileOps.joinPath(basePath, "missing.txt"), "mode"

    -- a genuine lfs failure (an error code other than not-found) surfaces as nil plus a
    -- message, keeping the error out of the value channel
    getAttributes_error: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> nil, "permission denied", 13
      info, err = fileOps.getAttributes {basePath, "denied.txt"}, "mode"
      ut\assertNil info
      ut\assertString err

    -- the deprecated attributes() shim still exposes the legacy five-value contract
    attributes_deprecatedShim: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      mode, fullPath = fileOps.attributes {basePath, "file.txt"}, "mode"
      ut\assertEquals mode, "file"
      ut\assertString fullPath

    -- joinPath: pure computation, no stubs needed

    joinPath_segmentsArray: (ut) ->
      result = fileOps.joinPath {"path", "to", "file.txt"}
      ut\assertEquals result, "path#{pathSep}to#{pathSep}file.txt"

    joinPath_segmentsVarargs: (ut) ->
      result = fileOps.joinPath "path", "to", "file.txt"
      ut\assertEquals result, "path#{pathSep}to#{pathSep}file.txt"

    joinPath_segmentsMixed: (ut) ->
      result = fileOps.joinPath {"path", "to"}, "file.txt"
      ut\assertEquals result, "path#{pathSep}to#{pathSep}file.txt"

    -- an empty or separator-only segment contributes nothing and must not truncate later segments
    joinPath_skipsEmptySegments: (ut) ->
      ut\assertEquals fileOps.joinPath("path", "", "file.txt"), "path#{pathSep}file.txt"
      ut\assertEquals fileOps.joinPath("path", {}, "file.txt"), "path#{pathSep}file.txt"
      ut\assertEquals fileOps.joinPath("a", "b/c", "d"), "a#{pathSep}b#{pathSep}c#{pathSep}d"

    -- mkdir: stubs lfs.attributes + lfs.mkdir

    mkdir_new: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> nil
      mkdirStub = (ut\stub lfs, "mkdir")\calls (path) -> true
      result, path = fileOps.mkdir {basePath, "newdir"}
      ut\assertTrue result
      ut\assertString path
      mkdirStub\assertCalledOnce!

    mkdir_exists: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "directory"
      result, dir = fileOps.mkdir {basePath, "existing"}
      ut\assertFalse result
      ut\assertString dir

    -- Aegisub's lfs.mkdir returns nothing on success; only an explicit error or a directory
    -- that is still missing counts as failure
    mkdir_acceptsSilentLfsSuccess: (ut) ->
      created = false
      (ut\stub lfs, "attributes")\calls (path, key) -> created and "directory" or nil
      (ut\stub lfs, "mkdir")\calls (path) ->
        created = true
        nil
      result, dir = fileOps.mkdir {basePath, "silent-new"}
      ut\assertTrue result
      ut\assertString dir

    mkdir_silentLfsFailure: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> nil
      (ut\stub lfs, "mkdir")\calls (path) -> nil
      result, err = fileOps.mkdir {basePath, "silent-fail"}
      ut\assertNil result
      ut\assertString err

    rmdir_acceptsSilentLfsSuccess: (ut) ->
      removed = false
      (ut\stub lfs, "attributes")\calls (path, key) ->
        return nil if removed
        "directory"
      (ut\stub lfs, "rmdir")\calls (path) ->
        removed = true
        nil
      result, err = fileOps.rmdir {basePath, "silent-rm"}, false
      ut\assertTrue result
      ut\assertNil err

    -- readFile: stubs lfs.attributes + io.open

    readFile_success: (ut) ->
      filePath = fileOps.joinPath basePath, "file.txt"
      content = "hello, DependencyControl"
      mockHandle = {
        read: (handle, fmt) -> content
        close: (handle) ->
      }
      (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      openStub = (ut\stub io, "open")\calls (path, mode) -> mockHandle
      data, err = fileOps.readFile filePath
      ut\assertEquals data, content
      ut\assertNil err
      openStub\assertCalledOnceWith filePath, "rb"

    readFile_isDirectory: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "directory"
      data, err = fileOps.readFile {basePath, "dir"}
      ut\assertNil data
      ut\assertString err

    -- getHash / verifyHash: stub readFile so the hash is computed over known content

    getHash_sha1: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "readFile")\returns "abc"
      ut\assertEquals fileOps.getHash("/path/file", "sha1"),
        "a9993e364706816aba3e25717850c26c9cd0d89d"

    getHash_defaultsToSha1: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "readFile")\returns "abc"
      ut\assertEquals fileOps.getHash("/path/file"),
        "a9993e364706816aba3e25717850c26c9cd0d89d"

    getHash_unsupportedType: (ut) ->
      hash, err = fileOps.getHash "/path/file", "md5"
      ut\assertNil hash
      ut\assertString err

    verifyHash_match: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "readFile")\returns "abc"
      ut\assertTrue fileOps.verifyHash "/path/file", "A9993E364706816ABA3E25717850C26C9CD0D89D", "sha1"

    verifyHash_mismatch: (ut) ->
      (ut\stub FILEOPS_MODULE_NAME, "readFile")\returns "abc"
      ok, err = fileOps.verifyHash "/path/file", ("0")\rep(40), "sha1"
      ut\assertFalse ok
      ut\assertString err

    verifyHash_badArg: (ut) ->
      ok, err = fileOps.verifyHash "/path/file", nil
      ut\assertNil ok
      ut\assertString err

    -- copy: stubs lfs.attributes + io.open

    copy_success: (ut) ->
      srcPath = fileOps.joinPath basePath, "src.txt"
      dstPath = fileOps.joinPath basePath, "dst.txt"
      mockIn = {
        read: (handle, fmt) -> "content"
        close: (handle) ->
      }
      mockOut = {
        write: (handle, data) -> true
        close: (handle) ->
      }
      (ut\stub lfs, "attributes")\calls (path, key) ->
        if path == srcPath then "file" else nil
      ioStub = (ut\stub io, "open")\calls (path, mode) ->
        if mode == "rb" then mockIn else mockOut
      result, err = fileOps.copy srcPath, dstPath
      ioStub\assertCalledTimes 2
      ioStub\assertNthCalledWith 1, srcPath, "rb"
      ioStub\assertNthCalledWith 2, dstPath, "wb"
      ut\assertTrue result
      ut\assertNil err

    copy_targetExists: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      result, err = fileOps.copy {basePath, "src.txt"}, {basePath, "dst.txt"}
      ut\assertFalse result
      ut\assertString err

    -- move: stubs lfs.attributes + os.remove + os.rename

    move_overwrite: (ut) ->
      srcPath = fileOps.joinPath basePath, "src.txt"
      dstPath = fileOps.joinPath basePath, "dst.txt"
      (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      removeStub = (ut\stub os, "remove")\returns true
      renameStub = (ut\stub os, "rename")\returns true
      result, err = fileOps.move srcPath, dstPath, true
      ut\assertTrue result
      ut\assertNil err
      removeStub\assertCalledOnceWith dstPath
      renameStub\assertCalledOnceWith srcPath, dstPath

    -- remove: stubs lfs.attributes + os.remove

    remove_success: (ut) ->
      filePath = fileOps.joinPath basePath, "file.txt"
      (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      removeStub = (ut\stub os, "remove")\returns true
      result, details = fileOps.remove filePath
      ut\assertTrue result
      removeStub\assertCalledOnceWith filePath

    remove_notFound: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> nil
      result, details = fileOps.remove fileOps.joinPath basePath, "missing.txt"
      ut\assertTrue result
      ut\assertTable details

    -- a hard stat failure (an lfs error other than not-found) is a failure, not an absent
    -- target: remove reports nil plus the error rather than a misleading overall success
    remove_hardFailureReported: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> nil, "permission denied", 13
      result, details, firstErr = fileOps.remove fileOps.joinPath basePath, "denied.txt"
      ut\assertNil result
      ut\assertTable details
      ut\assertString firstErr

    -- a directory is removed non-recursively unless recurse is passed, so a stray directory path can't
    -- silently delete a whole tree (the recurse flag forwarded to rmdir must be false by default)
    remove_dirNonRecursiveByDefault: (ut) ->
      recurseArgs = {}
      (ut\stub lfs, "attributes")\calls (path, key) -> "directory"
      (ut\stub fileOps, "rmdir")\calls (path, recurse) -> recurseArgs[#recurseArgs + 1] = recurse; true
      fileOps.remove fileOps.joinPath basePath, "d"
      fileOps.remove fileOps.joinPath(basePath, "d"), true
      ut\assertEquals recurseArgs, {false, true}

    -- validateFullPath with basePath

    validateFullPath_withBasePath: (ut) ->
      result = fileOps.validateFullPath "file.txt", false, basePath
      ut\assertString result
      ut\assertContains result, "file.txt"

    -- __getPathRoot

    getPathRoot_windowsPath: (ut) ->
      return unless isWindows
      result = fileOps.__getPathRoot "C:\\Users\\foo"
      ut\assertEquals result, "C:\\"

    getPathRoot_posixPath: (ut) ->
      return if isWindows
      result = fileOps.__getPathRoot "/usr/local"
      ut\assertEquals result, "/usr"

    getPathRoot_relative: (ut) ->
      result = fileOps.__getPathRoot "relative/path"
      ut\assertNil result

    -- joinPath: dot/dot-dot resolution

    joinPath_resolvesDotDot: (ut) ->
      result = fileOps.joinPath "a", "b", "..", "c"
      ut\assertEquals result, "a#{pathSep}c"

    joinPath_invalidSegment: (ut) ->
      result, err = fileOps.joinPath 42
      ut\assertNil result
      ut\assertString err

    -- exists

    exists_fileFound: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      result = fileOps.exists {basePath, "file.txt"}, "file"
      ut\assertTrue result

    exists_notFound: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> nil, "No such file or directory", 2
      result, err = fileOps.exists {basePath, "missing.txt"}, "file"
      ut\assertFalse result
      ut\assertString err

    exists_wrongType: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "directory"
      result, err = fileOps.exists {basePath, "dir"}, "file"
      ut\assertFalse result
      ut\assertString err

    exists_noTypeCheck: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "directory"
      result = fileOps.exists {basePath, "dir"}
      ut\assertTrue result

    -- listDir

    listDir_success: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "directory"
      entries = {"a.txt", ".", "b.lua", ".."}
      idx = 0
      makeIter = ->
        i = 0
        ->
          i += 1
          entries[i]
      (ut\stub lfs, "dir")\calls (path) -> makeIter!
      result = fileOps.listDir basePath
      ut\assertTable result
      ut\assertEquals #result, 2

    listDir_notDirectory: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> "file"
      result, err = fileOps.listDir basePath
      ut\assertNil result
      ut\assertString err

    -- listFilesRecursive

    listFilesRecursive_collectsNestedFiles: (ut) ->
      root = fileOps.joinPath basePath, "walk"
      fileOps.mkdir fileOps.joinPath(root, "sub", "subSub"), false, true
      fileOps.writeFile fileOps.joinPath(root, "a.txt"), "a", true
      fileOps.writeFile fileOps.joinPath(root, "sub", "b.txt"), "b", true
      fileOps.writeFile fileOps.joinPath(root, "sub", "subSub", "c.txt"), "c", true
      files = fileOps.listFilesRecursive root
      ut\assertTable files
      ut\assertEquals #files, 3
      found = {file\match("[^/\\]+$"), true for file in *files}
      ut\assertTrue found["a.txt"]
      ut\assertTrue found["b.txt"]
      ut\assertTrue found["c.txt"]

    listFilesRecursive_notDirectory: (ut) ->
      filePath = fileOps.joinPath basePath, "walk-file.txt"
      fileOps.writeFile filePath, "x", true
      result, err = fileOps.listFilesRecursive filePath
      ut\assertNil result
      ut\assertString err

    _order: {
      "validateFullPath_nonString", "validateFullPath_parentDir", "validateFullPath_tooLong",
      "validateFullPath_segmentTooLong", "pathLimits_detected",
      "validateFullPath_tooLong_generic", "validateFullPath_tooLong_registryDisabled",
      "validateFullPath_tooLong_processUnaware",
      "validateFullPath_invalidChars", "validateFullPath_reservedNames",
      "validateFullPath_reservedNameWithExt", "validateFullPath_trailingDotSegment",
      "validateFullPath_valid", "validateFullPath_noExt_rejected", "validateFullPath_withExt_accepted",
      "validateFullPath_homeDirExpansion", "validateFullPath_reservedNameNonWindows",
      "getNamespacedPath_nested", "getNamespacedPath_flat",
      "getNamespacedPath_badNamespace", "getNamespacedPath_badBasePath",
      "getAttributes_file", "getAttributes_notFound", "getAttributes_error", "attributes_deprecatedShim",
      "mkdir_new", "mkdir_exists",
      "mkdir_acceptsSilentLfsSuccess", "mkdir_silentLfsFailure", "rmdir_acceptsSilentLfsSuccess",
      "readFile_success", "readFile_isDirectory",
      "getHash_sha1", "getHash_defaultsToSha1", "getHash_unsupportedType",
      "verifyHash_match", "verifyHash_mismatch", "verifyHash_badArg",
      "copy_success", "copy_targetExists",
      "move_overwrite",
      "remove_success", "remove_notFound", "remove_hardFailureReported", "remove_dirNonRecursiveByDefault",
      "validateFullPath_withBasePath",
      "getPathRoot_windowsPath", "getPathRoot_posixPath", "getPathRoot_relative",
      "joinPath_segmentsArray", "joinPath_segmentsVarargs", "joinPath_segmentsMixed",
      "joinPath_skipsEmptySegments", "joinPath_resolvesDotDot", "joinPath_invalidSegment",
      "exists_fileFound", "exists_notFound", "exists_wrongType", "exists_noTypeCheck",
      "listDir_success", "listDir_notDirectory",
      "listFilesRecursive_collectsNestedFiles", "listFilesRecursive_notDirectory"
    }
  }
