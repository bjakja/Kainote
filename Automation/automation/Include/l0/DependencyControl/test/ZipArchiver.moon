-- ZipArchiver tests: entry collection and the per-platform archive writers.
-- The platform-specific writers (__writeWindows/__writeUnix) are exercised by calling
-- them directly with all filesystem and shell-out calls stubbed, so both paths run
-- regardless of the host OS. A real archive round-trip lives in test/integration/ZipArchiver.
-- Called from Tests.moon as: (require "...test.ZipArchiver") basePath
(basePath) ->
  lfs = require "lfs"
  fileOps = require "l0.DependencyControl.file-ops"
  ZipArchiver = require "l0.DependencyControl.ZipArchiver"

  FILEOPS_MODULE_NAME = "l0.DependencyControl.file-ops"
  JSON_MODULE_NAME = "l0.dkjson"
  pathSep = fileOps.pathSep

  -- Fake io handle supporting the `h\write(data)\close!` chain the writers use.
  makeHandle = -> {write: ((self, data) -> self), close: ((self) -> nil)}

  -- Stubs lfs.attributes + lfs.dir over a fake tree so addDirectory can be driven without
  -- touching disk. `dirs` is a set of directory paths; `children` maps a dir path to its
  -- entry names (callers include "."/".." to prove they're skipped).
  stubTree = (ut, dirs, children) ->
    (ut\stub lfs, "attributes")\calls (path, key) -> dirs[path] and "directory" or "file"
    (ut\stub lfs, "dir")\calls (path) ->
      names, i = children[path] or {}, 0
      ->
        i += 1
        names[i]

  {
    _description: "Tests for ZipArchiver entry collection and the per-platform archive writers."

    -- new

    new_initializes: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      ut\assertEquals arch.outputPath, "#{basePath}/out.zip"
      ut\assertTable arch.entries
      ut\assertEquals #arch.entries, 0
      ut\assertNotNil arch.logger

    -- addFile

    addFile_appendsAndChains: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      ret = arch\addFile "/src/a.txt", "a.txt"
      ut\assertEquals #arch.entries, 1
      ut\assertEquals arch.entries[1].source, "/src/a.txt"
      ut\assertEquals arch.entries[1].name, "a.txt"
      ut\assertIs ret, arch -- returns self for chaining

    -- addDirectory

    addDirectory_missingDir: (ut) ->
      (ut\stub lfs, "attributes")\calls (path, key) -> false
      arch = ZipArchiver "#{basePath}/out.zip"
      ret = arch\addDirectory "#{basePath}/nope"
      ut\assertEquals #arch.entries, 0
      ut\assertIs ret, arch

    addDirectory_flat: (ut) ->
      root = "#{basePath}#{pathSep}src"
      stubTree ut, {[root]: true}, {[root]: {".", "..", "a.txt", "b.txt"}}
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addDirectory root
      ut\assertEquals #arch.entries, 2
      ut\assertEquals arch.entries[1].name, "a.txt"
      ut\assertEquals arch.entries[2].name, "b.txt"

    addDirectory_nestedUsesForwardSlash: (ut) ->
      root = "#{basePath}#{pathSep}src"
      sub = "#{root}#{pathSep}sub"
      stubTree ut, {[root]: true, [sub]: true},
        {[root]: {"a.txt", "sub"}, [sub]: {"c.txt"}}
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addDirectory root
      ut\assertEquals #arch.entries, 2
      byName = {entry.name, entry for entry in *arch.entries}
      ut\assertNotNil byName["a.txt"]
      ut\assertNotNil byName["sub/c.txt"] -- always forward-slash, even on Windows

    addDirectory_prefixNormalizesTrailingSlash: (ut) ->
      root = "#{basePath}#{pathSep}src"
      stubTree ut, {[root]: true}, {[root]: {"a.txt"}}
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addDirectory root, "pre/" -- trailing slash must not double up
      ut\assertEquals arch.entries[1].name, "pre/a.txt"

    -- write: dispatch + guards

    write_noEntries: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      result, err = arch\write!
      ut\assertNil result
      ut\assertString err

    write_removesTargetThenDispatches: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      removeStub = (ut\stub FILEOPS_MODULE_NAME, "remove")\returns true
      -- stub both writers; only the host platform's runs, but this keeps the test OS-agnostic
      (ut\stub arch, "__writeWindows")\returns true
      (ut\stub arch, "__writeUnix")\returns true
      result = arch\write!
      ut\assertTrue result
      removeStub\assertCalledOnceWith "#{basePath}/out.zip" -- Create mode needs the target absent

    -- __writeWindows: manifest + helper script + PowerShell shell-out (all stubbed)

    writeWindows_success: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      (ut\stub io, "open")\returns makeHandle!
      (ut\stub JSON_MODULE_NAME, "encode")\returns "[]"
      (ut\stub os, "execute")\returns 0 -- exit code 0 (Lua 5.1 numeric return)
      (ut\stub os, "remove")\returns true
      ut\assertTrue arch\__writeWindows!

    writeWindows_manifestWriteFailure: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      (ut\stub io, "open")\returns nil, "disk full"
      result, err = arch\__writeWindows!
      ut\assertNil result
      ut\assertContains err, "disk full"

    writeWindows_scriptWriteFailureCleansUp: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      idx = 0
      (ut\stub io, "open")\calls (path, mode) ->
        idx += 1
        return makeHandle! if idx == 1 -- manifest write succeeds
        nil, "boom" -- helper script write fails
      (ut\stub JSON_MODULE_NAME, "encode")\returns "[]"
      removeStub = (ut\stub os, "remove")\returns true
      result, err = arch\__writeWindows!
      ut\assertNil result
      ut\assertContains err, "boom"
      removeStub\assertCalledTimes 2 -- cleanup removes both temp files

    writeWindows_toolFailure: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      (ut\stub io, "open")\returns makeHandle!
      (ut\stub JSON_MODULE_NAME, "encode")\returns "[]"
      (ut\stub os, "execute")\returns 1 -- non-zero exit
      (ut\stub os, "remove")\returns true
      result, err = arch\__writeWindows!
      ut\assertNil result
      ut\assertContains err, "PowerShell"

    -- __writeUnix: stage into a temp tree, then run the `zip` CLI (all stubbed)

    writeUnix_success: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      (ut\stub FILEOPS_MODULE_NAME, "mkdir")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "copy")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "remove")\returns true
      (ut\stub lfs, "currentdir")\returns "#{basePath}/prev"
      (ut\stub lfs, "chdir")\returns true
      (ut\stub lfs, "dir")\calls (path) ->
        names, i = {".", "..", "a.txt"}, 0
        ->
          i += 1
          names[i]
      (ut\stub os, "execute")\returns true -- boolean success (Lua 5.2+/LUA52COMPAT return)
      ut\assertTrue arch\__writeUnix!

    writeUnix_stageFailureCleansUp: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      (ut\stub FILEOPS_MODULE_NAME, "mkdir")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "copy")\returns nil, "no space"
      removeStub = (ut\stub FILEOPS_MODULE_NAME, "remove")\returns true
      result, err = arch\__writeUnix!
      ut\assertNil result
      ut\assertContains err, "/src/a.txt" -- names the file that couldn't be staged
      removeStub\assertCalledOnce! -- staging dir torn down

    writeUnix_enterStageFailureCleansUp: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      (ut\stub FILEOPS_MODULE_NAME, "mkdir")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "copy")\returns true
      removeStub = (ut\stub FILEOPS_MODULE_NAME, "remove")\returns true
      (ut\stub lfs, "currentdir")\returns "#{basePath}/prev"
      (ut\stub lfs, "chdir")\returns false -- can't cd into the staging dir
      result, err = arch\__writeUnix!
      ut\assertNil result
      ut\assertContains err, "staging directory"
      removeStub\assertCalledOnce!

    writeUnix_toolFailure: (ut) ->
      arch = ZipArchiver "#{basePath}/out.zip"
      arch\addFile "/src/a.txt", "a.txt"
      (ut\stub aegisub, "decode_path")\returns basePath
      (ut\stub FILEOPS_MODULE_NAME, "mkdir")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "copy")\returns true
      (ut\stub FILEOPS_MODULE_NAME, "remove")\returns true
      (ut\stub lfs, "currentdir")\returns "#{basePath}/prev"
      (ut\stub lfs, "chdir")\returns true
      (ut\stub lfs, "dir")\calls (path) ->
        names, i = {".", "..", "a.txt"}, 0
        ->
          i += 1
          names[i]
      (ut\stub os, "execute")\returns 1 -- zip exits non-zero
      result, err = arch\__writeUnix!
      ut\assertNil result
      ut\assertContains err, "zip"

    _order: {
      "new_initializes",
      "addFile_appendsAndChains",
      "addDirectory_missingDir", "addDirectory_flat",
      "addDirectory_nestedUsesForwardSlash", "addDirectory_prefixNormalizesTrailingSlash",
      "write_noEntries", "write_removesTargetThenDispatches",
      "writeWindows_success", "writeWindows_manifestWriteFailure",
      "writeWindows_scriptWriteFailureCleansUp", "writeWindows_toolFailure",
      "writeUnix_success", "writeUnix_stageFailureCleansUp",
      "writeUnix_enterStageFailureCleansUp", "writeUnix_toolFailure"
    }
  }
