-- Logger tests: message formatting, dump serialization, and log dispatch.
-- Called from Tests.moon as: (require "...test.Logger")!
->
  Logger = require "l0.DependencyControl.Logger"

  {
    _description: "Tests for the Logger class covering message formatting, dump serialization, and log dispatch."

    -- format: pure computation, no stubs needed

    format_string: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result = logger\format "hello world", 0
      ut\assertEquals result, "hello world"

    format_printf: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result = logger\format "value: %d", 0, 42
      ut\assertEquals result, "value: 42"

    format_table: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result = logger\format {"line1", "line2"}, 0
      ut\assertEquals result, "line1\nline2"

    format_indent: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result = logger\format "line1\nline2", 1
      ut\assertContains result, "— line2"

    -- dumpToString: pure computation, no stubs needed

    dumpToString_scalar: (ut) ->
      logger = Logger toFile: false, toWindow: false
      ut\assertEquals logger\dumpToString("hello"), "hello"
      ut\assertEquals logger\dumpToString(42), "42"
      ut\assertEquals logger\dumpToString(true), "true"

    dumpToString_flatTable: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result = logger\dumpToString {key: "val"}
      ut\assertContains result, "key:"
      ut\assertContains result, "val"

    dumpToString_ignoreKey: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result = logger\dumpToString {keep: "yes", skip: "no"}, "skip"
      ut\assertContains result, "keep:"
      ut\assertNil result\find "skip:", 1, true

    dumpToString_maxDepth: (ut) ->
      logger = Logger toFile: false, toWindow: false
      nested = {inner: {deep: "value"}}
      result = logger\dumpToString nested, nil, 0
      ut\assertContains result, "<...>"

    dumpToString_circular: (ut) ->
      logger = Logger toFile: false, toWindow: false
      t = {}
      t.self = t
      result = logger\dumpToString t
      ut\assertContains result, "self: @1"

    -- log/dispatch: stubs aegisub.log

    log_dispatches: (ut) ->
      logger = Logger toFile: false, toWindow: true
      logStub = ut\stub aegisub, "log"
      result = logger\log 2, "hello"
      ut\assertTrue result
      logStub\assertCalledOnce!

    log_emptyMsg: (ut) ->
      logger = Logger toFile: false, toWindow: true
      logStub = ut\stub aegisub, "log"
      result = logger\log 2, ""
      ut\assertFalse result
      logStub\assertNotCalled!

    log_nonNumberLevel: (ut) ->
      logger = Logger toFile: false, toWindow: true
      logStub = ut\stub aegisub, "log"
      result = logger\log "hello"
      ut\assertTrue result
      logStub\assertCalledOnce!

    -- a log file that can't be opened disables file logging instead of crashing on every subsequent call
    log_fileOpenFailureDisablesToFile: (ut) ->
      logger = Logger toFile: true, toWindow: false, logDir: "?temp"
      (ut\stub io, "open")\calls -> nil -- simulate the log file failing to open
      ut\stub aegisub, "log" -- swallow the one-time warning
      ok = pcall -> logger\log 4, "hello"
      ut\assertTrue ok -- no crash
      ut\assertFalse logger.toFile -- file logging disabled after the failure

    -- the usePrefix shorthand configures both sinks (regression: a multi-assign from the
    -- single value left the window flag on its default)
    usePrefix_setsBothSinks: (ut) ->
      logger = Logger toFile: false, toWindow: false, usePrefix: false
      ut\assertFalse logger.usePrefixFile
      ut\assertFalse logger.usePrefixWindow

    -- a progress bar's fill chunks continue the logger's own open window line, so they must
    -- not repeat the indent prefix inside the bar
    progress_noIndentInsideBar: (ut) ->
      captured = {}
      (ut\stub aegisub, "log")\calls (level, msg) -> captured[#captured + 1] = msg
      logger = Logger toFile: false, toWindow: true, indent: 2
      logger\progress 10, "Downloading..."
      logger\progress 50
      logger\progress!
      out = table.concat captured
      ut\assertContains out, "—— Downloading" -- the opening chunk keeps its indent
      bar = out\match "%[(.-)%]"
      ut\assertEquals bar, "■"\rep 10

    -- assert/assertNotNil: success path returns values, failure path throws

    assert_truthy: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result, extra = logger\assert true, "should not log"
      ut\assertTrue result
      ut\assertEquals extra, "should not log"

    assert_falsy: (ut) ->
      logger = Logger toFile: false, toWindow: false
      ok, err = pcall -> logger\assert false, "boom"
      ut\assertFalse ok
      ut\assertString err

    assertNotNil_value: (ut) ->
      logger = Logger toFile: false, toWindow: false
      result = logger\assertNotNil 0, "should not log"
      ut\assertEquals result, 0

    assertNotNil_nil: (ut) ->
      logger = Logger toFile: false, toWindow: false
      ok, err = pcall -> logger\assertNotNil nil, "boom"
      ut\assertFalse ok
      ut\assertString err

    _order: {
      "format_string", "format_printf", "format_table", "format_indent",
      "dumpToString_scalar", "dumpToString_flatTable", "dumpToString_ignoreKey",
      "dumpToString_maxDepth", "dumpToString_circular",
      "log_dispatches", "log_emptyMsg", "log_nonNumberLevel", "log_fileOpenFailureDisablesToFile",
      "usePrefix_setsBothSinks", "progress_noIndentInsideBar",
      "assert_truthy", "assert_falsy",
      "assertNotNil_value", "assertNotNil_nil"
    }
  }
