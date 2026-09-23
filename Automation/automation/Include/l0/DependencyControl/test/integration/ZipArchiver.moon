-- Real archive round-trip for ZipArchiver against each platform's stock tooling.
-- Self-gating via _condition: skipped unless the build tool (PowerShell on Windows,
-- `zip` on *nix) and a matching extractor (`Expand-Archive` / `unzip`) are available,
-- so the default offline run never depends on them.
-- Called from Tests.moon as: (require "...test.integration.ZipArchiver") basePath
(basePath) ->
  ffi = require "ffi"
  fileOps = require "l0.DependencyControl.file-ops"
  ZipArchiver = require "l0.DependencyControl.ZipArchiver"

  isWindows = ffi.os == "Windows"

  -- Runs a shell command and reports success across Lua 5.1 (numeric) and 5.2+ (boolean) returns.
  execOk = (cmd) ->
    r = os.execute cmd
    (type(r) == "number" and r == 0) or r == true

  extractCmd = (archivePath, destDir) ->
    if isWindows
      ([[powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "Expand-Archive -LiteralPath '%s' -DestinationPath '%s' -Force"]])\format archivePath, destDir
    else
      ([[unzip -q -o "%s" -d "%s"]])\format archivePath, destDir

  {
    _description: "Real-tooling ZipArchiver round-trip (runs when the platform zip/unzip tools are available)."

    _condition: ->
      if isWindows
        return false, "PowerShell unavailable" unless execOk [[powershell -NoProfile -NonInteractive -Command "exit 0"]]
      else
        return false, "`zip` unavailable" unless execOk "command -v zip > /dev/null 2>&1"
        return false, "`unzip` unavailable" unless execOk "command -v unzip > /dev/null 2>&1"
      return true

    _setup: (ut) ->
      base = "#{basePath}_zipArchiver"
      srcDir = "#{base}/src"
      extractDir = "#{base}/extracted"
      fileOps.mkdir "#{srcDir}/sub", false, true
      fileOps.mkdir extractDir, false, true

      write = (path, data) ->
        f = assert io.open path, "wb"
        f\write data
        f\close!

      write "#{srcDir}/top.txt", "top-level file"
      write "#{srcDir}/sub/c.txt", "nested file contents"

      {:base, :srcDir, :extractDir, archivePath: "#{base}/out.zip"}

    _teardown: (ut, ctx) ->
      fileOps.remove ctx.base, true if ctx and ctx.base

    -- A directory added by ZipArchiver must round-trip through the real tooling with its
    -- nested entry recreated under a forward-slash subpath on every platform.
    roundTrip_preservesNestedEntries: (ut, ctx) ->
      arch = ZipArchiver ctx.archivePath
      arch\addDirectory ctx.srcDir
      ok, err = arch\write!
      ut\assertTrue ok, err
      ut\assertTrue fileOps.exists ctx.archivePath

      ut\assertTrue execOk(extractCmd ctx.archivePath, ctx.extractDir), "extraction failed"
      ut\assertTrue fileOps.exists "#{ctx.extractDir}/top.txt"
      ut\assertTrue fileOps.exists "#{ctx.extractDir}/sub/c.txt" -- nested path materialized

      data = fileOps.readFile "#{ctx.extractDir}/sub/c.txt"
      ut\assertEquals data, "nested file contents"
  }
