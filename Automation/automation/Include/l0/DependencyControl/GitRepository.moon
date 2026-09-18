---Interface to a local git repository for running git commands.
---@class GitRepository
class GitRepository
  ---Creates an interface to the git repository rooted at `dir`.
  ---@param dir string Absolute path to the repository root.
  new: (@dir) =>

  ---Runs a git command and returns trimmed stdout+stderr, or nil on failure or empty output.
  ---@param args string Command and flags passed verbatim after `git -C <dir>`.
  ---@return string? output
  run: (args) =>
    h = io.popen ('git -C "%s" %s 2>&1')\format @dir, args
    return nil unless h
    out = (h\read("*a") or "")\gsub "%s+$", ""
    h\close! and out != "" and out or nil


  ---Returns the branch name the given ref resolves to.
  ---@param ref? string Git ref to resolve (defaults to HEAD).
  ---@return string? branch Short branch name, or nil when the command fails.
  getBranch: (ref = "HEAD") => @run "rev-parse --abbrev-ref #{ref}"
  ---Returns the abbreviated commit hash of the given ref.
  ---@param ref? string Git ref to resolve (defaults to HEAD).
  ---@return string? hash Seven-character commit hash, or nil when the command fails.
  getCommitHash: (ref = "HEAD") => @run "rev-parse --short=7 #{ref}"
  ---Reports whether the given ref sits exactly on a tag.
  ---@param ref? string Git ref to test (defaults to HEAD).
  ---@return boolean atTag True when the ref points exactly at a tag.
  isAtTag: (ref = "HEAD") => not not @run "describe --exact-match --tags #{ref}"

  ---Returns a git describe-style version suffix for the given ref.
  ---Returns "" when the ref is exactly on a tag, "-<branch>-g<hash>" otherwise.
  ---@param ref? string Git ref to describe (defaults to HEAD).
  ---@return string suffix
  getVersionSuffix: (ref = "HEAD") =>
    return "" if @isAtTag ref
    branch = @getBranch(ref) or "unknown"
    hash = @getCommitHash(ref) or "0000000"
    "-#{branch}-g#{hash}"

return GitRepository
