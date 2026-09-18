-- open-url tests: the http(s)-only validation gate and the POSIX shell-safe command building. The actual
-- open() is a thin, platform-dependent I/O wrapper and isn't exercised here.
() ->
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  openUrl = require "l0.DependencyControl.helpers.open-url"
  {:isSafeUrl, :buildPosixOpenCommand} = UnitTestSuite\getTestExports openUrl

  {
    _description: "open-url: URL validation gate and POSIX shell-safe command building."

    -- ordinary http(s) URLs are accepted
    isSafeUrl_acceptsHttp: (ut) ->
      ut\assertTrue isSafeUrl "https://raw.githubusercontent.com/x/y/DependencyControl.json"
      ut\assertTrue isSafeUrl "http://example.com/feed"

    -- non-http(s) schemes are rejected (no file://, javascript:, ftp://)
    isSafeUrl_rejectsOtherSchemes: (ut) ->
      ut\assertFalse isSafeUrl "file:///etc/passwd"
      ut\assertFalse isSafeUrl "javascript:alert(1)"
      ut\assertFalse isSafeUrl "ftp://x/y"

    -- whitespace and control characters (the levers for command injection) are rejected
    isSafeUrl_rejectsWhitespaceAndControl: (ut) ->
      ut\assertFalse isSafeUrl "https://x/a b"
      ut\assertFalse isSafeUrl "https://x/a\nrm -rf ~"
      ut\assertFalse isSafeUrl "https://x/a\tb"

    -- non-string input is rejected
    isSafeUrl_rejectsNonString: (ut) ->
      ut\assertFalse isSafeUrl nil
      ut\assertFalse isSafeUrl 42

    -- the URL is single-quoted, with embedded quotes closed/escaped/reopened, so /bin/sh sees one argument
    buildPosixOpenCommand_escapesForShell: (ut) ->
      ut\assertEquals buildPosixOpenCommand("xdg-open", "https://x/a"),
        "xdg-open 'https://x/a' >/dev/null 2>&1"
      ut\assertEquals buildPosixOpenCommand("open", "https://x/a'b"),
        "open 'https://x/a'\\''b' >/dev/null 2>&1"

    -- the public open() enforces the gate: an unsafe URL is refused (nil + error) before any opener runs
    open_refusesUnsafeUrl: (ut) ->
      ok, err = openUrl.open "file:///etc/passwd"
      ut\assertNil ok
      ut\assertNotNil err

    _order: {
      "isSafeUrl_acceptsHttp", "isSafeUrl_rejectsOtherSchemes", "isSafeUrl_rejectsWhitespaceAndControl"
      "isSafeUrl_rejectsNonString", "buildPosixOpenCommand_escapesForShell", "open_refusesUnsafeUrl"
    }
  }
