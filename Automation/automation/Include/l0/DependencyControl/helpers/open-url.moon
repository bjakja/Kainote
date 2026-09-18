ffi = require "ffi"
UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"

msgs = {
  unsafeUrl: "Refusing to open '%s': only http(s) URLs without whitespace or control characters are allowed."
  unsupported: "Don't know how to open a URL on this platform (%s)."
  openFailed: "The system opener failed for '%s'."
}

-- Accepts only an http(s) URL free of whitespace and control characters. Those characters have no place in a
-- URL and are the main levers of shell-command injection; rejecting them keeps hostile input from reaching the
-- OS opener at all — even though the openers below never build a shell command line out of the URL.
isSafeUrl = (url) ->
  type(url) == "string" and url\match("^https?://") != nil and url\match("[%s%c]") == nil

-- Builds the POSIX open command, single-quoting the URL so /bin/sh treats it as one literal argument: each
-- embedded ' is closed, escaped and reopened ('\''). Output is discarded.
buildPosixOpenCommand = (launcher, url) ->
  quoted = "'" .. url\gsub("'", "'\\''") .. "'"
  "#{launcher} #{quoted} >/dev/null 2>&1"

-- os.execute returns the exit code (Lua 5.1) or a boolean (5.2+/LUA52COMPAT).
execOk = (cmd) ->
  r = os.execute cmd
  (type(r) == "number" and r == 0) or r == true

-- The platform opener: (url) -> boolean success; url is pre-validated by isSafeUrl.
opener = switch ffi.os
  when "Windows"
    ffi.cdef [[
      uintptr_t ShellExecuteA(void*, const char*, const char*, const char*, const char*, int);
    ]]
    shell32 = ffi.load "shell32"
    (url) ->
      -- ShellExecuteA hands the URL straight to the OS protocol handler, so no cmd.exe ever parses it.
      -- The return is HINSTANCE-like; a value > 32 indicates success.
      tonumber(shell32.ShellExecuteA nil, "open", url, nil, nil, 1) > 32 -- 1 = SW_SHOWNORMAL
  when "OSX", "Linux", "BSD"
    launcher = ffi.os == "OSX" and "open" or "xdg-open"
    (url) -> execOk buildPosixOpenCommand launcher, url

---Opens an http(s) URL in the user's default browser. The URL is validated and never passed through a shell
---command line, so an attacker-controlled feed URL can't inject a command.
---@param url string The URL to open.
---@return true? ok True on success; nil if the URL was rejected, the platform is unsupported, or the open failed.
---@return string? err A human-readable reason on failure; nil on success.
openUrl = (url) ->
  return nil, msgs.unsafeUrl\format tostring url unless isSafeUrl url
  return nil, msgs.unsupported\format ffi.os unless opener
  return nil, msgs.openFailed\format url unless opener url
  true

return UnitTestSuite\withTestExports {open: openUrl}, {:isSafeUrl, :buildPosixOpenCommand}
