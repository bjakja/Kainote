$revNum = (git rev-list --reverse HEAD | measure).count
$isTagged = (git name-rev --name-only --tags HEAD) -ne "undefined"
$branch = git rev-parse --abbrev-ref HEAD
$shortHash = git rev-parse --short HEAD

echo "Version matching in progress"
$x = 0
do {
    $closestTag = git describe --abbrev=0 --tags HEAD~$x
    $versionMatch = $closestTag | sls "^v?(\d+\.\d+\.\d+(?:\-.*)?$)"
    $x++
} until ($versionMatch -or -not $closestTag)
echo "Version matching done"

@"
[Version]
Branch=$branch
Revision=g$shortHash
RevisionNumber=$revNum
IsTagged=$isTagged
IsRelease=$($isTagged -and $versionMatch)
Version=$(if($versionMatch) {$versionMatch.Matches.Groups[1].Value} else {"0.1.0"} )
"@>git_rev.ini