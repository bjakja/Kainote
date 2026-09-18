<#
.SYNOPSIS
    Downloads the pinned third-party sources the Windows build needs.

.DESCRIPTION
    Reads Thirdparty/dependencies.json and, for every entry, downloads the pinned
    archive, verifies its SHA-256 and extracts it into Thirdparty/<directory>.

    An entry whose marker file is already present is skipped, so re-running this
    is cheap. Use -Force to re-extract anyway, or -Only to work on a subset.

    Only the Windows build needs this. The Linux build resolves the same
    libraries through pkg-config and never reads dependencies.json.

.EXAMPLE
    pwsh -File Thirdparty/hydrate.ps1

.EXAMPLE
    pwsh -File Thirdparty/hydrate.ps1 -Only ffmpeg,boost -Force
#>
[CmdletBinding()]
param(
    [string]   $ManifestPath = (Join-Path $PSScriptRoot 'dependencies.json'),
    [string]   $Destination  = $PSScriptRoot,
    [string[]] $Only,
    [switch]   $Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ProgressPreference    = 'SilentlyContinue'

# How long any single extractor gets before it is killed and the next one is
# tried.  Overridable so the fallback path can be exercised in a test.
$script:ExtractTimeoutSec = if ($env:KAINOTE_EXTRACT_TIMEOUT_SEC) {
    [int] $env:KAINOTE_EXTRACT_TIMEOUT_SEC
} else { 300 }

function Write-Section([string] $Message) {
    Write-Host ''
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Invoke-WithRetry {
    param([scriptblock] $Action, [int] $Attempts = 3)
    for ($i = 1; $i -le $Attempts; $i++) {
        try { return & $Action }
        catch {
            if ($i -eq $Attempts) { throw }
            Write-Warning "Attempt $i failed: $($_.Exception.Message). Retrying..."
            Start-Sleep -Seconds ([Math]::Min(30, 5 * $i))
        }
    }
}

# Runs a native tool with a hard timeout and captures its output.
#
# A timeout is not paranoia here: Windows' System32 bsdtar is built without
# liblzma, so for .xz it falls back to spawning an external `xz` helper, and
# when that helper is absent it can block forever instead of failing.  Without
# a timeout the first bad candidate hangs the whole build and the fallback
# below never gets a turn.
function Invoke-Native {
    param(
        [Parameter(Mandatory)] [string]   $FilePath,
        [Parameter(Mandatory)] [string[]] $Arguments,
        [int] $TimeoutSec = $script:ExtractTimeoutSec
    )

    $outFile = [IO.Path]::GetTempFileName()
    $errFile = [IO.Path]::GetTempFileName()
    try {
        # Start-Process with redirected streams also detaches stdin, so a tool
        # that decides to prompt cannot block on a console that is not there.
        $proc = Start-Process -FilePath $FilePath -ArgumentList $Arguments `
            -NoNewWindow -PassThru `
            -RedirectStandardOutput $outFile -RedirectStandardError $errFile

        if (-not $proc.WaitForExit($TimeoutSec * 1000)) {
            try { $proc.Kill() } catch { }
            return [pscustomobject]@{ ExitCode = -1; Output = "timed out after ${TimeoutSec}s"; TimedOut = $true }
        }

        $text = @(
            (Get-Content -LiteralPath $outFile -Raw -ErrorAction SilentlyContinue),
            (Get-Content -LiteralPath $errFile -Raw -ErrorAction SilentlyContinue)
        ) -join ''
        return [pscustomobject]@{ ExitCode = $proc.ExitCode; Output = $text.Trim(); TimedOut = $false }
    }
    finally {
        Remove-Item -LiteralPath $outFile, $errFile -Force -ErrorAction SilentlyContinue
    }
}

# 7-Zip reads .tar.xz reliably on Windows and is preinstalled on the GitHub
# runners, so it is tried before tar.
function Get-SevenZipCandidate {
    $ordered = @(
        (Get-Command 7z  -CommandType Application -ErrorAction SilentlyContinue | ForEach-Object { $_.Source })
        (Get-Command 7za -CommandType Application -ErrorAction SilentlyContinue | ForEach-Object { $_.Source })
    )
    foreach ($env in @($env:ProgramFiles, ${env:ProgramFiles(x86)})) {
        if ($env) {
            $candidate = Join-Path $env '7-Zip\7z.exe'
            if (Test-Path -LiteralPath $candidate) { $ordered += $candidate }
        }
    }
    $seen = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
    foreach ($path in $ordered) { if ($path -and $seen.Add($path)) { $path } }
}

# Windows commonly has more than one `tar` on PATH: the bsdtar in System32 and
# the GNU tar from Git for Windows.  Get-Command returns all of them, so taking
# .Source blindly yields an array that stringifies into one nonsense command
# name.  Git's GNU tar is listed first because it ships its own xz; System32
# bsdtar is the one that can hang on .xz.
function Get-TarCandidate {
    $fromPath = @(
        Get-Command tar -CommandType Application -ErrorAction SilentlyContinue |
            ForEach-Object { $_.Source }
    )
    $system32 = if ($env:SystemRoot) { Join-Path $env:SystemRoot 'system32\tar.exe' } else { $null }

    $ordered  = @($fromPath | Where-Object { $_ -ne $system32 })
    $ordered += @($fromPath | Where-Object { $_ -eq $system32 })
    if ($system32 -and (Test-Path -LiteralPath $system32) -and $ordered -notcontains $system32) {
        $ordered += $system32
    }

    $seen = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
    foreach ($path in $ordered) { if ($path -and $seen.Add($path)) { $path } }
}

# Unpacks a tarball into $Staging, trying every extractor this machine has
# until one produces files.  Returns the tool that worked.
function Expand-Tarball {
    param(
        [Parameter(Mandatory)] [string] $Archive,
        [Parameter(Mandatory)] [string] $Staging
    )

    $attempts = @()

    function Test-Extracted { @(Get-ChildItem -LiteralPath $Staging -Force).Count -gt 0 }
    function Clear-Staging {
        Get-ChildItem -LiteralPath $Staging -Force |
            Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
    }

    # 7-Zip needs two passes for .tar.xz: decompress, then untar.
    foreach ($sevenZip in (Get-SevenZipCandidate)) {
        $mid = Join-Path ([IO.Path]::GetTempPath()) ("kainote-xz-" + [Guid]::NewGuid().ToString('N'))
        New-Item -ItemType Directory -Force -Path $mid | Out-Null
        try {
            $first = Invoke-Native -FilePath $sevenZip -Arguments @('x', '-y', "-o$mid", $Archive)
            if ($first.ExitCode -eq 0) {
                $inner = Get-ChildItem -LiteralPath $mid -Filter *.tar -File | Select-Object -First 1
                if ($inner) {
                    $second = Invoke-Native -FilePath $sevenZip -Arguments @('x', '-y', "-o$Staging", $inner.FullName)
                    if ($second.ExitCode -eq 0 -and (Test-Extracted)) { return $sevenZip }
                    $attempts += "      $sevenZip (untar) -> exit $($second.ExitCode) $($second.Output)"
                } else {
                    # Already a plain .tar, or 7-Zip unpacked it in one pass.
                    Copy-Item -Path (Join-Path $mid '*') -Destination $Staging -Recurse -Force
                    if (Test-Extracted) { return $sevenZip }
                }
            } else {
                $attempts += "      $sevenZip -> exit $($first.ExitCode) $($first.Output)"
            }
        }
        finally {
            Remove-Item -LiteralPath $mid -Recurse -Force -ErrorAction SilentlyContinue
        }
        Clear-Staging
    }

    foreach ($tar in (Get-TarCandidate)) {
        $result = Invoke-Native -FilePath $tar -Arguments @('-xf', $Archive, '-C', $Staging)
        if ($result.ExitCode -eq 0 -and (Test-Extracted)) { return $tar }
        $attempts += "      $tar -> exit $($result.ExitCode) $($result.Output)"
        Clear-Staging
    }

    $name  = [IO.Path]::GetFileName($Archive)
    $tried = if ($attempts) { $attempts -join [Environment]::NewLine } else { '      (no extractor found)' }
    throw "Nothing on this machine could extract ${name}:$([Environment]::NewLine)$tried$([Environment]::NewLine)      Install 7-Zip, or make sure a tar with xz support is on PATH."
}

# Extracts $Archive into $Target, discarding $Strip leading path components.
function Expand-Any {
    param(
        [Parameter(Mandatory)] [string] $Archive,
        [Parameter(Mandatory)] [string] $Target,
        [int] $Strip = 0
    )

    $staging = Join-Path ([IO.Path]::GetTempPath()) ("kainote-hydrate-" + [Guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Force -Path $staging | Out-Null
    try {
        if ($Archive -like '*.zip') {
            Expand-Archive -LiteralPath $Archive -DestinationPath $staging -Force
        }
        else {
            $used = Expand-Tarball -Archive $Archive -Staging $staging
            Write-Host "    extracted with $used"
        }

        $root = $staging
        for ($i = 0; $i -lt $Strip; $i++) {
            $children = @(Get-ChildItem -LiteralPath $root -Force)
            $dirs     = @($children | Where-Object { $_.PSIsContainer })
            if ($dirs.Count -ne 1) {
                throw "Cannot strip $Strip component(s) from $Archive : expected exactly one directory at depth $i, found $($dirs.Count)"
            }
            $root = $dirs[0].FullName
        }

        New-Item -ItemType Directory -Force -Path $Target | Out-Null
        Copy-Item -Path (Join-Path $root '*') -Destination $Target -Recurse -Force
    }
    finally {
        Remove-Item -LiteralPath $staging -Recurse -Force -ErrorAction SilentlyContinue
    }
}

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$selected = $manifest.dependencies
if ($Only) {
    $selected = $selected | Where-Object { $Only -contains $_.name }
    $missing  = $Only | Where-Object { $manifest.dependencies.name -notcontains $_ }
    if ($missing) { throw "Unknown dependency name(s): $($missing -join ', ')" }
}

$cache = Join-Path $Destination '.cache'
New-Item -ItemType Directory -Force -Path $cache | Out-Null

$hydrated = 0
$skipped  = 0

foreach ($dep in $selected) {
    $target = Join-Path $Destination $dep.directory
    # marker is written with '/' in the manifest; split so it works on any host.
    $marker = $target
    foreach ($part in ($dep.marker -split '/')) { $marker = Join-Path $marker $part }

    if ((Test-Path -LiteralPath $marker) -and -not $Force) {
        Write-Host ("  {0,-10} {1,-9} already present" -f $dep.name, $dep.version)
        $skipped++
        continue
    }

    Write-Section ("{0} {1}" -f $dep.name, $dep.version)

    $archive = Join-Path $cache ([IO.Path]::GetFileName(([Uri] $dep.url).AbsolutePath))

    $needsDownload = $true
    if (Test-Path -LiteralPath $archive) {
        if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ieq $dep.sha256) {
            Write-Host "    using cached archive"
            $needsDownload = $false
        }
        else {
            Remove-Item -LiteralPath $archive -Force
        }
    }

    if ($needsDownload) {
        Write-Host "    downloading $($dep.url)"
        # -TimeoutSec matters: PowerShell 7 defaults it to 0, meaning no timeout
        # at all, so a connection that stalls mid-transfer hangs the build
        # forever and Invoke-WithRetry never fires because nothing ever throws.
        # 15 minutes is far more than the largest archive here needs (boost is
        # 327 MB) while still bounding the failure.
        Invoke-WithRetry {
            Invoke-WebRequest -Uri $dep.url -OutFile $archive -UseBasicParsing -TimeoutSec 900
        }
        $mb = [Math]::Round((Get-Item -LiteralPath $archive).Length / 1MB, 1)
        Write-Host "    downloaded $mb MB"
    }

    $actual = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
    if ($actual -ine $dep.sha256) {
        Remove-Item -LiteralPath $archive -Force -ErrorAction SilentlyContinue
        throw ("SHA-256 mismatch for {0} {1}`n  expected {2}`n  actual   {3}" -f $dep.name, $dep.version, $dep.sha256, $actual)
    }
    Write-Host "    sha256 ok"

    # The wipe below would take Thirdparty\<dir>\.gitignore with it, and that file
    # is what keeps the extracted tree out of git. Carry it across, or write it.
    $ignorePath = Join-Path $target '.gitignore'
    $ignoreKept = if (Test-Path -LiteralPath $ignorePath) {
        [IO.File]::ReadAllBytes($ignorePath)
    } else { $null }

    if (Test-Path -LiteralPath $target) {
        Remove-Item -LiteralPath $target -Recurse -Force
    }

    $strip = if ($dep.PSObject.Properties.Name -contains 'strip') { [int] $dep.strip } else { 0 }
    Expand-Any -Archive $archive -Target $target -Strip $strip

    if ($null -ne $ignoreKept) {
        [IO.File]::WriteAllBytes($ignorePath, $ignoreKept)
    }
    else {
        Set-Content -LiteralPath $ignorePath -Value @('*', '!.gitignore') -Encoding utf8NoBOM
    }

    if (-not (Test-Path -LiteralPath $marker)) {
        throw "$($dep.name) did not produce its marker file: $marker"
    }
    Write-Host "    extracted to Thirdparty\$($dep.directory)"
    $hydrated++
}

Write-Host ''
Write-Host "Hydrated $hydrated, already present $skipped." -ForegroundColor Green
