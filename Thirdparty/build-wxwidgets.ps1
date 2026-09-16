<#
.SYNOPSIS
    Builds the bundled wxWidgets submodule with the solution wxWidgets ships.

.DESCRIPTION
    Kainote links wxWidgets statically. Rather than maintaining our own project
    files for it, this builds Thirdparty/wxWidgets/build/msw/wx_vc17.sln, which
    is upstream's own solution, and leaves the libraries where Kainote.vcxproj
    expects them (lib\vc_x64_lib for x64, lib\vc_lib for Win32).

    Run this once after checking out the submodules, and again after the
    wxWidgets submodule is moved to a new tag.

.EXAMPLE
    pwsh -File Thirdparty\build-wxwidgets.ps1

.EXAMPLE
    pwsh -File Thirdparty\build-wxwidgets.ps1 -Platform Win32 -Configuration Debug
#>
[CmdletBinding()]
param(
    [ValidateSet('x64', 'Win32')]         [string[]] $Platform      = @('x64'),
    [ValidateSet('Release', 'Debug')]     [string[]] $Configuration = @('Release'),
    [string] $Solution = (Join-Path $PSScriptRoot 'wxWidgets\build\msw\wx_vc17.sln')
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Solution)) {
    throw @"
wxWidgets solution not found at:
  $Solution
The wxWidgets submodule has probably not been checked out. Run:
  git submodule update --init --recursive
"@
}

# wxWidgets keeps several of its third-party libraries (zlib, png, pcre,
# scintilla, lexilla, expat...) in nested submodules, so a non-recursive
# checkout silently produces empty directories and a confusing build failure.
$zlib = Join-Path $PSScriptRoot 'wxWidgets\src\zlib\zlib.h'
if (-not (Test-Path -LiteralPath $zlib)) {
    throw @"
wxWidgets' own submodules are missing (src/zlib is empty). Run:
  git submodule update --init --recursive
"@
}

function Find-MSBuild {
    $msbuild = Get-Command msbuild -CommandType Application -ErrorAction SilentlyContinue
    if ($msbuild) { return $msbuild.Source }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw 'msbuild is not on PATH and vswhere.exe was not found; run this from a Developer PowerShell.'
    }
    $found = & $vswhere -latest -products * `
        -requires Microsoft.Component.MSBuild `
        -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $found) { throw 'MSBuild was not found by vswhere.' }
    return $found
}

$msbuild = Find-MSBuild
Write-Host "Using MSBuild: $msbuild"

foreach ($p in $Platform) {
    foreach ($c in $Configuration) {
        Write-Host ''
        Write-Host "==> wxWidgets $c|$p" -ForegroundColor Cyan
        & $msbuild $Solution /m /v:minimal /p:Configuration=$c /p:Platform=$p
        if ($LASTEXITCODE -ne 0) {
            throw "wxWidgets build failed for $c|$p (exit $LASTEXITCODE)"
        }
    }
}

$libDir = if ($Platform -contains 'x64') { 'lib\vc_x64_lib' } else { 'lib\vc_lib' }
Write-Host ''
Write-Host "wxWidgets built. Libraries are in Thirdparty\wxWidgets\$libDir." -ForegroundColor Green
