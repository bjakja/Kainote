<#
.SYNOPSIS
    Builds the Kainote Windows installer.

.DESCRIPTION
    Takes a payload staged by

        python .github/scripts/package.py --platform windows --flavor installer

    generates the association entries from Kainote/FileTypes.h, and compiles
    packaging/windows/kainote.iss with Inno Setup.

.EXAMPLE
    pwsh -File packaging\windows\build-installer.ps1 -PayloadDir build\installer-payload -OutputDir dist
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $PayloadDir,
    [Parameter(Mandatory)] [string] $OutputDir,
    [string] $Iscc
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

if (-not (Test-Path -LiteralPath $PayloadDir)) {
    throw "payload directory not found: $PayloadDir"
}
if (-not (Test-Path -LiteralPath (Join-Path $PayloadDir 'Kainote.exe'))) {
    throw "no Kainote.exe in $PayloadDir; run package.py --flavor installer first"
}

# The version is in exactly one place; do not re-derive it anywhere else.
$versionHeader = Join-Path $repo 'Kainote\VersionKainote.h'
$match = Select-String -LiteralPath $versionHeader -Pattern '#define\s+VersionKainote\s+"([0-9.]+)"'
if (-not $match) {
    throw "could not read VersionKainote from $versionHeader"
}
$appVersion = $match.Matches[0].Groups[1].Value
Write-Host "Kainote version: $appVersion"

& python (Join-Path $PSScriptRoot 'gen_associations.py') `
    --repo $repo --out (Join-Path $PSScriptRoot 'associations.iss')
if ($LASTEXITCODE -ne 0) { throw "gen_associations.py failed ($LASTEXITCODE)" }

if (-not $Iscc) {
    $candidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
        (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe')
    )
    $Iscc = $candidates | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -First 1
}
if (-not $Iscc) {
    throw "ISCC.exe not found. Install Inno Setup, or pass -Iscc <path>."
}
Write-Host "Using $Iscc"

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$payloadFull = (Resolve-Path -LiteralPath $PayloadDir).Path
$outputFull = (Resolve-Path -LiteralPath $OutputDir).Path

& $Iscc `
    "/DAppVersion=$appVersion" `
    "/DPayloadDir=$payloadFull" `
    "/DOutputDir=$outputFull" `
    (Join-Path $PSScriptRoot 'kainote.iss')

# ISCC's exit code is easy to lose in a pipeline; this is the only check the
# .iss ever gets, since it cannot be compiled anywhere but Windows.
if ($LASTEXITCODE -ne 0) {
    throw "ISCC failed ($LASTEXITCODE)"
}

$setup = Get-ChildItem -LiteralPath $outputFull -Filter 'Kainote-*-x64-setup.exe' |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $setup) {
    throw "ISCC reported success but produced no installer in $outputFull"
}
Write-Host ("Built {0} ({1:N1} MB)" -f $setup.Name, ($setup.Length / 1MB)) -ForegroundColor Green
