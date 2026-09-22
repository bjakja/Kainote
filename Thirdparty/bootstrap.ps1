<#
.SYNOPSIS
    Prepares a fresh checkout for the Windows build.

.DESCRIPTION
    Runs, in order:

      1. git submodule update --init --recursive  (unless -SkipSubmodules)
      2. hydrate.ps1        -- the dependencies that are archives, not submodules
      3. build-wxwidgets.ps1 -- wxWidgets, via the solution wxWidgets itself ships
      4. gen-gitparams.ps1  -- Kainote/gitparams.h

    After this, Kainote.sln builds without further setup:

      msbuild Kainote.sln /m /p:Configuration=Release /p:Platform=x64

    Every step is safe to re-run: hydrate skips what is already extracted and
    the wxWidgets build is incremental.

.EXAMPLE
    pwsh -File Thirdparty\bootstrap.ps1

.EXAMPLE
    pwsh -File Thirdparty\bootstrap.ps1 -Configuration Release,Debug
#>
[CmdletBinding()]
param(
    [ValidateSet('x64', 'Win32')]     [string[]] $Platform      = @('x64'),
    [ValidateSet('Release', 'Debug')] [string[]] $Configuration = @('Release'),
    [switch] $SkipSubmodules,
    [switch] $SkipWxWidgets,
    [switch] $Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repo = Split-Path -Parent $PSScriptRoot
$step = 0

function Step([string] $Title) {
    $script:step++
    Write-Host ''
    Write-Host ("[{0}] {1}" -f $script:step, $Title) -ForegroundColor Cyan
}

Step 'Submodules'
if ($SkipSubmodules) {
    Write-Host '    skipped (-SkipSubmodules)'
} else {
    & git -C $repo submodule update --init --recursive
    if ($LASTEXITCODE -ne 0) { throw "git submodule update failed ($LASTEXITCODE)" }
}

Step 'Archive dependencies'
$hydrateArgs = @{}
if ($Force) { $hydrateArgs['Force'] = $true }
& (Join-Path $PSScriptRoot 'hydrate.ps1') @hydrateArgs

Step 'wxWidgets'
if ($SkipWxWidgets) {
    Write-Host '    skipped (-SkipWxWidgets)'
} else {
    & (Join-Path $PSScriptRoot 'build-wxwidgets.ps1') `
        -Platform $Platform -Configuration $Configuration
}

Step 'Version header'
& (Join-Path $PSScriptRoot 'gen-gitparams.ps1')

Write-Host ''
Write-Host 'Ready. Build with:' -ForegroundColor Green
foreach ($p in $Platform) {
    foreach ($c in $Configuration) {
        Write-Host ("  msbuild Kainote.sln /m /p:Configuration={0} /p:Platform={1}" -f $c, $p)
    }
}
