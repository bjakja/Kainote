<#
.SYNOPSIS
    Generates Kainote/gitparams.h with the current git commit and branch.

.DESCRIPTION
    config.cpp consumes GIT_CUR_COMMIT / GIT_BRANCH (guarded by #ifdef
    GIT_CUR_COMMIT) and stringifies them for the title-bar version. The file is
    gitignored and regenerated at build time.

    This is the Windows counterpart of cmake/GenGitParams.cmake and must keep
    producing byte-identical output to it. The two exist separately so the
    Windows build needs no CMake installation.

    Like the CMake version, it only rewrites the header when the content
    actually changes, so config.cpp is not recompiled on every build.
#>
[CmdletBinding()]
param(
    [string] $SourceRoot = (Split-Path -Parent $PSScriptRoot)
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-GitValue([string[]] $GitArgs) {
    try {
        $value = & git -C $SourceRoot @GitArgs 2>$null
        if ($LASTEXITCODE -ne 0) { return '' }
        return ($value | Select-Object -First 1).Trim()
    } catch {
        # git is not installed, or this is not a checkout: not an error.
        return ''
    }
}

$commit = Get-GitValue @('rev-parse', 'HEAD')
$branch = Get-GitValue @('rev-parse', '--abbrev-ref', 'HEAD')

if ([string]::IsNullOrEmpty($commit)) {
    # No git / not a checkout: emit an empty header (config.cpp omits the suffix).
    $content = "#pragma once`n"
} else {
    if ([string]::IsNullOrEmpty($branch)) { $branch = 'unknown' }
    $content = "#pragma once`n#define GIT_BRANCH $branch`n#define GIT_CUR_COMMIT $commit`n"
}

$out = Join-Path $SourceRoot 'Kainote\gitparams.h'
$existing = ''
if (Test-Path -LiteralPath $out) {
    $existing = [System.IO.File]::ReadAllText($out)
}

if ($existing -ne $content) {
    [System.IO.File]::WriteAllText($out, $content, (New-Object System.Text.UTF8Encoding $false))
    Write-Host "gitparams.h updated ($commit)"
} else {
    Write-Host "gitparams.h already current"
}
