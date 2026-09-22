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
    [string] $Iscc,
    # A full signtool command line, with $f where the file name goes, e.g.
    #   'signtool.exe sign /fd sha256 /tr http://timestamp.digicert.com /td sha256 /f cert.pfx /p ... $f'
    # Left empty the installer is simply unsigned; nothing else changes.
    [string] $SignCommand
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
    $roots = @($env:ProgramFiles, ${env:ProgramFiles(x86)}) | Where-Object { $_ }
    $Iscc = Get-ChildItem -Path $roots -Filter 'ISCC.exe' -Recurse -ErrorAction SilentlyContinue |
        Sort-Object FullName -Descending | Select-Object -First 1 |
        ForEach-Object { $_.FullName }
}
if (-not $Iscc) {
    throw "ISCC.exe not found. Install Inno Setup, or pass -Iscc <path>."
}
Write-Host "Using $Iscc"

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$payloadFull = (Resolve-Path -LiteralPath $PayloadDir).Path
$outputFull = (Resolve-Path -LiteralPath $OutputDir).Path

$isccArgs = @(
    "/DAppVersion=$appVersion"
    "/DPayloadDir=$payloadFull"
    "/DOutputDir=$outputFull"
)
if ($SignCommand) {
    # /S defines a named tool; SignTool=kainote in the .iss then uses it, and
    # SignedUninstaller makes it cover the uninstaller too.
    $isccArgs += "/Skainote=$SignCommand"
    $isccArgs += "/DSignInstaller=1"
    Write-Host "Signing enabled"
}
else {
    Write-Warning "No -SignCommand given: the installer will be unsigned, and SmartScreen will warn on it."
}
$isccArgs += (Join-Path $PSScriptRoot 'kainote.iss')

& $Iscc @isccArgs

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
