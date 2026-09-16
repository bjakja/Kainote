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

# Windows commonly has more than one `tar` on PATH: the bsdtar that ships in
# System32 (Windows 10 1803+) and the GNU tar from Git for Windows.  They are
# not interchangeable for .tar.xz -- bsdtar links liblzma directly, while GNU
# tar shells out to an `xz` binary that may not be installed -- and
# Get-Command returns *all* of them, so taking .Source blindly yields an array
# that stringifies into one nonsense command name.
#
# Return them in preference order, System32 first, without duplicates.
function Get-TarCandidate {
    $ordered = @()
    if ($env:SystemRoot) {
        $system32 = Join-Path $env:SystemRoot 'system32\tar.exe'
        if (Test-Path -LiteralPath $system32) { $ordered += $system32 }
    }
    $ordered += @(
        Get-Command tar -CommandType Application -ErrorAction SilentlyContinue |
            ForEach-Object { $_.Source }
    )

    $seen = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
    foreach ($path in $ordered) {
        if ($path -and $seen.Add($path)) { $path }
    }
}

# Extracts $Archive into $Target, discarding $Strip leading path components.
# .zip goes through Expand-Archive; tarballs go through whichever tar on this
# machine can actually read them.
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
            $candidates = @(Get-TarCandidate)
            if ($candidates.Count -eq 0) {
                throw "No tar executable was found on PATH; it is required to extract $([IO.Path]::GetFileName($Archive))"
            }

            $extracted = $false
            $attempts  = @()
            foreach ($tar in $candidates) {
                $output = & $tar -xf $Archive -C $staging 2>&1
                $code   = $LASTEXITCODE
                if ($code -eq 0 -and @(Get-ChildItem -LiteralPath $staging -Force).Count -gt 0) {
                    Write-Host "    extracted with $tar"
                    $extracted = $true
                    break
                }
                $attempts += "      $tar -> exit $code $(($output | Out-String).Trim())"
                # Leave a clean staging directory for the next candidate.
                Get-ChildItem -LiteralPath $staging -Force |
                    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
            }
            if (-not $extracted) {
                $name = [IO.Path]::GetFileName($Archive)
                $tried = $attempts -join [Environment]::NewLine
                throw "No tar on this machine could extract ${name}:$([Environment]::NewLine)$tried"
            }
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
        Invoke-WithRetry { Invoke-WebRequest -Uri $dep.url -OutFile $archive -UseBasicParsing }
    }

    $actual = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
    if ($actual -ine $dep.sha256) {
        Remove-Item -LiteralPath $archive -Force -ErrorAction SilentlyContinue
        throw ("SHA-256 mismatch for {0} {1}`n  expected {2}`n  actual   {3}" -f $dep.name, $dep.version, $dep.sha256, $actual)
    }
    Write-Host "    sha256 ok"

    if (Test-Path -LiteralPath $target) {
        Remove-Item -LiteralPath $target -Recurse -Force
    }

    $strip = if ($dep.PSObject.Properties.Name -contains 'strip') { [int] $dep.strip } else { 0 }
    Expand-Any -Archive $archive -Target $target -Strip $strip

    if (-not (Test-Path -LiteralPath $marker)) {
        throw "$($dep.name) did not produce its marker file: $marker"
    }
    Write-Host "    extracted to Thirdparty\$($dep.directory)"
    $hydrated++
}

Write-Host ''
Write-Host "Hydrated $hydrated, already present $skipped." -ForegroundColor Green
