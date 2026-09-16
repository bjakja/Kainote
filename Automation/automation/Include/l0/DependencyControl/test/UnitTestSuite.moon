-- UnitTestSuite tests: the unit-test framework's own assertions and run loop.
-- Called from test.moon as: (controls\requireTest "UnitTestSuite")!
->
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  UnitTest = UnitTestSuite.UnitTest

  -- A probe `self` for driving a single UnitTest assert method directly: its logger records the
  -- failure message/args and raises (mirroring the real logger's level-1 assert) so a failing assert
  -- is observable via pcall. `__class` lets `@@msgs`/`@format` resolve through UnitTest.__base.
  makeProbe = ->
    cap = {}
    setmetatable {
      __class: UnitTest
      assertFailed: false
      captured: cap
      logger: {
        -- checkArgTypes routes through logger.assert: valid args pass through, while a failed
        -- check records the message and raises like the real logger's level-1 assert
        assert: (cond, msg, ...) =>
          unless cond
            cap.msg, cap.args = msg, table.pack ...
            error "PROBE_ARG_CHECK_FAILED"
          cond
        logEx: (level, msg, _insertLineFeed, _prefix, _indent, ...) =>
          cap.msg, cap.args = msg, table.pack ...
          error "PROBE_ASSERT_FAILED"
        dumpToString: (value) => tostring value -- assert methods dump their values eagerly
      }
    }, __index: UnitTest.__base

  -- Replaces every logger a constructed suite holds (suite + each class/test/setup/teardown) with a
  -- no-op so a nested run() doesn't write to the live stream or a log file. Loggers are captured at
  -- construction, so this must run after the suite is built.
  silence = (suite) ->
    -- logEx must keep raising on level 1: that raise is what makes a failing assertion fail
    -- its test (see Logger.logEx). dumpToString is needed even by passing asserts, which
    -- format their failure arguments eagerly.
    noop = {indent: 0, log: (->), warn: (->), trace: (->), assert: ((cond) => cond),
      logEx: ((level) => error "SILENCED_ASSERT_FAILED" if level == 1),
      dumpToString: ((value) => tostring value)}
    suite.logger = noop
    for cls in *suite.classes
      cls.logger, cls.setup.logger, cls.teardown.logger = noop, noop, noop
      test.logger = noop for test in *cls.tests
    suite

  {
    _description: "Tests for the UnitTestSuite framework's own assertions and run loop."

    -- assertContains: regression for the inverted case-insensitivity and malformed failure message.

    assertContains_caseInsensitiveMatches: (ut) ->
      probe = makeProbe!
      ok = pcall UnitTest.assertContains, probe, "Hello World", "WORLD", false
      ut\assertTrue ok
      ut\assertFalse probe.assertFailed

    assertContains_caseSensitiveRejectsWrongCase: (ut) ->
      probe = makeProbe!
      ok = pcall UnitTest.assertContains, probe, "Hello World", "world" -- default: case-sensitive
      ut\assertFalse ok
      ut\assertTrue probe.assertFailed

    assertContains_failureUsesContainsMessage: (ut) ->
      probe = makeProbe!
      pcall UnitTest.assertContains, probe, "abc", "xyz"
      ut\assertEquals probe.captured.msg, UnitTest.msgs.assert.contains

    -- assertNegative: regression for the wrong "positive" word and the missing value arg (the message
    -- template's %d crashed on format because `actual` wasn't passed).

    assertNegative_failureMessage: (ut) ->
      probe = makeProbe!
      pcall UnitTest.assertNegative, probe, 5 -- 5 isn't negative → the assertion fails
      ut\assertEquals probe.captured.args[1], "negative"
      ut\assertEquals probe.captured.args[3], 5 -- the value, now passed for the template's %d

    -- run(): regression for the crash when a test class's setup fails (the -1 sentinel was iterated).

    setupFailureDoesNotCrashRun: (ut) ->
      suite = UnitTestSuite "test.regression.setupCrash", {
        Failing: {
          _setup: -> error "intentional setup failure"
          neverRuns: (t) -> t\assertTrue true
        }
      }
      silence suite
      ok, success = pcall -> suite\run!
      ut\assertTrue ok -- previously raised on `for … in *(-1)`
      ut\assertFalse success -- the failed setup is reported as a suite failure

    -- ut\skip: a test that skips itself is marked skipped (not failed), aborts the rest of its body,
    -- and is reported as skipped with its reason; the suite run still succeeds.
    skip_marksSkippedAbortsAndReports: (ut) ->
      suite = UnitTestSuite "test.regression.skip", {
        Skipping: {
          skips: (t) ->
            t\skip "unmet precondition"
            t\assertTrue false -- unreachable: skip aborts the body before this would fail
          passes: (t) -> t\assertTrue true
        }
      }
      silence suite
      ok, success = pcall -> suite\run!
      ut\assertTrue ok
      ut\assertTrue success -- a skip is not a failure

      byName = {t.name, t for t in *suite.classes[1].tests}
      ut\assertTrue byName.skips.skipped
      ut\assertEquals byName.skips.skipReason, "unmet precondition"
      ut\assertFalse byName.passes.skipped

      summary = suite\toCtrf!.results.summary
      ut\assertEquals summary.skipped, 1
      ut\assertEquals summary.passed, 1
      ut\assertEquals summary.failed, 0

    -- a test absent from a class's _order still runs, appended (name-sorted) after the listed ones:
    -- _order sets the run order, not which tests run.
    runsTestsMissingFromOrder: (ut) ->
      ran = {}
      suite = UnitTestSuite "test.regression.orderMembership", {
        Partial: {
          _order: {"bbb", "aaa"} -- non-alphabetical, and deliberately omits "ccc"
          aaa: (t) -> ran[#ran+1] = "aaa"
          bbb: (t) -> ran[#ran+1] = "bbb"
          ccc: (t) -> ran[#ran+1] = "ccc"
        }
      }
      silence suite
      suite\run!
      ut\assertEquals ran, {"bbb", "aaa", "ccc"} -- listed order first, then the unlisted test appended

    -- assertItemsEqual/assertItemsAre: regressions for the failure message passing the keys
    -- sub-phrase as the whole template, and the expected arg being type-checked against `actual`

    assertItemsEqual_passesAndFailureMessage: (ut) ->
      probe = makeProbe!
      ok = pcall UnitTest.assertItemsEqual, probe, {1, 2, 3}, {3, 2, 1}
      ut\assertTrue ok
      probe = makeProbe!
      ok = pcall UnitTest.assertItemsEqual, probe, {1, 2}, {1, 3}
      ut\assertFalse ok
      ut\assertEquals probe.captured.msg, UnitTest.msgs.assert.itemsEqual
      ut\assertEquals probe.captured.args[1], "equal"
      ut\assertEquals probe.captured.args[2], UnitTest.msgs.assert.itemsEqualNumericKeys

    assertItemsEqual_typeChecksExpectedArg: (ut) ->
      probe = makeProbe!
      ok = pcall UnitTest.assertItemsEqual, probe, {1}, "not a table"
      ut\assertFalse ok
      ut\assertEquals probe.captured.msg, UnitTest.msgs.assert.checkArgTypes

    assertItemsAre_comparesByReference: (ut) ->
      shared = {}
      probe = makeProbe!
      ok = pcall UnitTest.assertItemsAre, probe, {shared}, {shared}
      ut\assertTrue ok
      probe = makeProbe!
      ok = pcall UnitTest.assertItemsAre, probe, {{}}, {{}} -- equal items, but not identical
      ut\assertFalse ok
      ut\assertEquals probe.captured.args[1], "identical"

    -- assertContinuous: regression for counting integer values instead of keys

    assertContinuous_checksKeysNotValues: (ut) ->
      probe = makeProbe!
      ok = pcall UnitTest.assertContinuous, probe, {"a", "b", "c"} -- continuous keys, no numeric values
      ut\assertTrue ok
      sparse = {"a", "b"}
      sparse[4] = "d"
      probe = makeProbe!
      ok = pcall UnitTest.assertContinuous, probe, sparse
      ut\assertFalse ok

    -- assertNotAlmostEquals: regression for printing the almostEquals message

    assertNotAlmostEquals_failureMessage: (ut) ->
      probe = makeProbe!
      ok = pcall UnitTest.assertNotAlmostEquals, probe, 1.0, 1.0 + 1e-10
      ut\assertFalse ok
      ut\assertEquals probe.captured.msg, UnitTest.msgs.assert.notAlmostEquals

    -- assertError: regression for over-counting a non-raising function's returned values

    assertError_countsReturnedValues: (ut) ->
      returnsTwo = -> 1, 2
      probe = makeProbe!
      ok = pcall UnitTest.assertError, probe, returnsTwo
      ut\assertFalse ok
      ut\assertEquals probe.captured.args[1], 2
      probe = makeProbe!
      err = UnitTest.assertError probe, -> error "boom"
      ut\assertContains err, "boom"

    -- ut\stub in a _setup: regression for UnitTestSetup never initializing @stubs; setup stubs
    -- are class-scoped and restored once the class (including teardown) has finished

    setupCanStubAndRestoresAfterClass: (ut) ->
      target = {value: -> "real"}
      sawStubInTeardown = nil
      suite = UnitTestSuite "test.regression.setupStub", {
        Stubbing: {
          _setup: (s) ->
            (s\stub target, "value")\returns "stubbed"
            true
          _teardown: -> sawStubInTeardown = target.value! == "stubbed"
          usesStub: (t) -> t\assertEquals target.value!, "stubbed"
        }
      }
      silence suite
      ok, success = pcall -> suite\run!
      ut\assertTrue ok
      ut\assertTrue success
      ut\assertTrue sawStubInTeardown -- still stubbed while the teardown ran
      ut\assertEquals target.value!, "real" -- restored once the class finished

    -- suite run(true): regression for the abort path leaving endTime/success unset (breaking the
    -- CTRF summary) and referencing a nonexistent message key

    abortOnFail_setsEndTimeAndSuccess: (ut) ->
      suite = UnitTestSuite "test.regression.abort", {
        Failing: {
          fails: (t) -> t\assertTrue false
        }
      }
      silence suite
      ok, success = pcall -> suite\run true
      ut\assertTrue ok
      ut\assertFalse success
      ut\assertNotNil suite.endTime
      ut\assertFalse suite.success
  }
