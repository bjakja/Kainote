<#
.SYNOPSIS
    Installs Inno Setup from the official jrsoftware/issrc release.

.DESCRIPTION
    Pinned by tag and verified by SHA-256, the same way Thirdparty/hydrate.ps1
    treats every runtime dependency. Chocolatey would be a second, unpinned
    intermediary between this repository and the compiler that builds its
    installer.

.EXAMPLE
    pwsh -File packaging\windows\install-innosetup.ps1
#>
[CmdletBinding()]
param(
    [string] $Version = '7.1.0',
    [string] $Tag     = 'is-7_1_0',
    [string] $Sha256  = '0362a383ed217d4c4239b5933866dd96d3eb2102737da92f80f6057a4b40df2f'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ProgressPreference    = 'SilentlyContinue'

$url = "https://github.com/jrsoftware/issrc/releases/download/$Tag/innosetup-$Version-x64.exe"
$cache = Join-Path ([IO.Path]::GetTempPath()) "innosetup-$Version-x64.exe"

if (-not (Test-Path -LiteralPath $cache)) {
    Write-Host "Downloading $url"
    Invoke-WebRequest -Uri $url -OutFile $cache -UseBasicParsing -TimeoutSec 900
}

$actual = (Get-FileHash -LiteralPath $cache -Algorithm SHA256).Hash
if ($actual -ine $Sha256) {
    Remove-Item -LiteralPath $cache -Force
    throw "SHA-256 mismatch for Inno Setup $Version`n  expected $Sha256`n  actual   $actual"
}
Write-Host "sha256 ok"

# Inno's own installer accepts the switches it gives everything it builds.
& $cache /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP- | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw "Inno Setup installer failed ($LASTEXITCODE)"
}

$iscc = Get-ChildItem -Path @($env:ProgramFiles, ${env:ProgramFiles(x86)}) `
    -Filter 'ISCC.exe' -Recurse -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not $iscc) {
    throw "Inno Setup installed but ISCC.exe was not found"
}
Write-Host "ISCC: $($iscc.FullName)"
