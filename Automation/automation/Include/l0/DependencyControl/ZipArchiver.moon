lfs = require "lfs"
ffi = require "ffi"
Logger = require "l0.DependencyControl.Logger"
constants = require "l0.DependencyControl.Constants"
fileOps = require "l0.DependencyControl.file-ops"
json = require "l0.dkjson"

defaultLogger = Logger fileBaseName: "#{constants.DEPCTRL_SHORT_NAME}.ZipArchiver"

-- Drives .NET's System.IO.Compression ZipArchive directly on Windows, reading
-- the (source → entry name) mapping from a JSON manifest. Entry names are taken
-- verbatim from the manifest (always forward-slash), which sidesteps both the
-- Compress-Archive cmdlet's backslash bug and the legacy ZipFile.CreateFromDirectory
-- quirk — so it works on stock Windows PowerShell out-of-the-box.
windowsBuilderScript = [[
$ErrorActionPreference = 'Stop'
$manifest = $args[0]
$dest = $args[1]
Add-Type -AssemblyName System.IO.Compression.FileSystem
$entries = Get-Content -LiteralPath $manifest -Raw | ConvertFrom-Json
$zip = [System.IO.Compression.ZipFile]::Open($dest, 'Create')
try {
    foreach ($e in $entries) {
        $entry = $zip.CreateEntry($e.name, [System.IO.Compression.CompressionLevel]::Optimal)
        $out = $entry.Open()
        $in = [System.IO.File]::OpenRead($e.source)
        $in.CopyTo($out)
        $in.Dispose()
        $out.Dispose()
    }
} finally {
    $zip.Dispose()
}
]]

-- Runs a shell command and reports success.
-- os.execute returns the exit code (Lua 5.1) or a boolean (5.2+/LUA52COMPAT).
execOk = (cmd) ->
  r = os.execute cmd
  return (type(r) == "number" and r == 0) or r == true

msgs = {
  errors: {
    noEntries: "No files have been added to the archive."
    helperWrite: "Couldn't write the archive helper file (%s)."
    stageFailed: "Couldn't stage '%s' for archiving (%s)."
    enterStage: "Couldn't enter the staging directory '%s'."
    zipFailed: "Archive creation failed (the '%s' tool reported an error)."
  }
}

---Builds zip archives using each platform's stock tooling — no extra rocks or
---shared libraries to install or locate. Files are added with explicit, forward-slash
---entry names so the resulting archives extract correctly on every platform.
---Compression only for now; reading/extraction can be added when needed.
---
---    archiver = ZipArchiver outputPath
---    archiver\addDirectory distDir
---    archiver\addFile readmePath, "README.md"
---    success, err = archiver\write!
---@class ZipArchiver
class ZipArchiver
  isWindows = ffi.os == "Windows"
  pathSep = fileOps.pathSep


  ---Creates an archiver that will write a zip to `outputPath`.
  ---@param outputPath string Absolute path of the archive to create.
  ---@param logger? Logger
  new: (@outputPath, @logger = defaultLogger) =>
    @entries = {}

  ---Adds a single file under `archiveName` (a forward-slash path within the archive).
  ---@param sourcePath string Absolute path of the file to add.
  ---@param archiveName string Name/path the file should have inside the archive.
  ---@return ZipArchiver self for chaining.
  addFile: (sourcePath, archiveName) =>
    @entries[#@entries + 1] = {source: sourcePath, name: archiveName}
    return @

  ---Adds every file beneath `sourceDir`, naming each entry by its path relative to
  ---`sourceDir` (optionally below `archivePrefix`).
  ---@param sourceDir string Absolute path of the directory to add.
  ---@param archivePrefix? string Optional path prefix for the entries inside the archive.
  ---@return ZipArchiver self for chaining.
  addDirectory: (sourceDir, archivePrefix = "") =>
    return @ unless lfs.attributes sourceDir, "mode"
    prefix = archivePrefix == "" and "" or "#{archivePrefix\gsub '[\\/]+$', ''}/"

    recurse = (dir, rel) ->
      for name in lfs.dir dir
        continue if name == "." or name == ".."
        full = "#{dir}#{pathSep}#{name}"
        entryName = rel == "" and name or "#{rel}/#{name}"
        if lfs.attributes(full, "mode") == "directory"
          recurse full, entryName
        else
          @entries[#@entries + 1] = {source: full, name: prefix .. entryName}

    recurse sourceDir, ""
    return @

  ---Writes the archive to `outputPath`. Returns true on success, or nil plus an
  ---error message.
  ---@return boolean|nil success
  ---@return string|nil err
  write: =>
    return nil, msgs.errors.noEntries if #@entries == 0
    fileOps.remove @outputPath -- ZipArchive 'Create' mode requires the target to be absent
    return @__writeWindows! if isWindows
    return @__writeUnix!

  -- on Windows, write a JSON manifest plus the helper script to the temp dir, then run
  -- it via -File (only path arguments to quote, avoiding cmd.exe quoting pitfalls).
  ---@private
  ---@return boolean|nil success true on success, nil on failure.
  ---@return string|nil err Error message describing the failure, nil on success.
  __writeWindows: =>
    token = "%04X"\format math.random 0, 16^4 - 1
    tmpDir = aegisub.decode_path "?temp"
    manifestPath = "#{tmpDir}#{pathSep}#{constants.DEPCTRL_SHORT_NAME}-#{@@__name}-#{token}.json"
    scriptPath = "#{tmpDir}#{pathSep}#{constants.DEPCTRL_SHORT_NAME}-#{@@__name}-#{token}.ps1"

    cleanup = ->
      os.remove manifestPath
      os.remove scriptPath

    mh, err = io.open manifestPath, "w"
    return nil, msgs.errors.helperWrite\format err unless mh
    mh\write(json.encode @entries)\close!

    sh, err = io.open scriptPath, "w"
    unless sh
      cleanup!
      return nil, msgs.errors.helperWrite\format err
    sh\write(windowsBuilderScript)\close!

    success = execOk ([[powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%s" "%s" "%s"]])\format scriptPath, manifestPath, @outputPath
    cleanup!
    return true if success
    return nil, msgs.errors.zipFailed\format "PowerShell"

  -- on Unix, the `zip` CLI can't rename entries, so stage each file into a temp tree at
  -- its archive name, then archive that tree from the inside.
  ---@private
  ---@return boolean|nil success true on success, nil on failure.
  ---@return string|nil err Error message describing the failure, nil on success.
  __writeUnix: =>
    token = "%04X"\format math.random 0, 16^4 - 1
    stageDir = "#{aegisub.decode_path '?temp'}#{pathSep}#{constants.DEPCTRL_SHORT_NAME}-#{@@__name}-#{token}"
    fileOps.mkdir stageDir, false, true

    for entry in *@entries
      target = "#{stageDir}/#{entry.name}"
      fileOps.mkdir target, true, true -- create the entry's parent directories
      ok, err = fileOps.copy entry.source, target
      unless ok
        fileOps.remove stageDir, true
        return nil, msgs.errors.stageFailed\format entry.source, err

    prevDir = lfs.currentdir!
    unless lfs.chdir stageDir
      fileOps.remove stageDir, true
      return nil, msgs.errors.enterStage\format stageDir

    names = [("'%s'")\format name for name in lfs.dir stageDir when name != "." and name != ".."]
    success = execOk ([[zip -r -q -X "%s" %s]])\format @outputPath, table.concat names, " "
    lfs.chdir prevDir
    fileOps.remove stageDir, true

    return true if success
    return nil, msgs.errors.zipFailed\format "zip"

return ZipArchiver
