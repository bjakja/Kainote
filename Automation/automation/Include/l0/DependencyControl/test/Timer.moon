-- Timer tests: extracted from the main test suite.
-- Called from test.moon as: (controls\requireTest "Timer")!
() ->
  Timer = require "l0.DependencyControl.Timer"

  {
    _description: "Tests for the FFI-based Timer: monotonic millisecond timing and sleep."

    -- elapsed

    elapsed_nonNegative: (ut) ->
      t = Timer!
      ut\assertGreaterThanOrEquals t.elapsed, 0

    elapsed_monotonic: (ut) ->
      t = Timer!
      a = t.elapsed
      b = t.elapsed
      ut\assertGreaterThanOrEquals b, a

    elapsed_advancesAfterSleep: (ut) ->
      t = Timer!
      Timer.sleep 20 -- 20 ms
      -- Require at least 10 ms to pass; allows 50% margin for CI jitter.
      ut\assertGreaterThan t.elapsed, 10

    elapsed_isReadOnly: (ut) ->
      t = Timer!
      ok = pcall -> t.elapsed = 5
      ut\assertEquals ok, false

    -- stopwatch: start / stop / reset

    stop_freezesElapsed: (ut) ->
      t = Timer!
      Timer.sleep 20
      t\stop!
      a = t.elapsed
      Timer.sleep 20
      -- while stopped, the elapsed total must not advance
      ut\assertEquals t.elapsed, a

    start_resumesAfterStop: (ut) ->
      t = Timer!
      Timer.sleep 20
      t\stop!
      frozen = t.elapsed
      t\start!
      Timer.sleep 20
      -- resuming measurement adds to the time accumulated before the stop
      ut\assertGreaterThan t.elapsed, frozen

    reset_clearsAccumulated: (ut) ->
      t = Timer!
      Timer.sleep 20
      before = t.elapsed
      t\reset!
      -- reset drops back to (near) zero, below the pre-reset total
      ut\assertLessThan t.elapsed, before

    -- getTime: shared monotonic clock

    getTime_isCallable: (ut) ->
      ut\assertFunction Timer.getTime

    getTime_monotonic: (ut) ->
      a = Timer.getTime!
      Timer.sleep 5
      b = Timer.getTime!
      ut\assertGreaterThanOrEquals b, a

    -- readings are milliseconds, so a 20 ms sleep must advance the clock by ~20, not ~0.02
    getTime_readsMilliseconds: (ut) ->
      a = Timer.getTime!
      Timer.sleep 20
      ut\assertGreaterThan Timer.getTime! - a, 10

    -- sleep

    sleep_isCallable: (ut) ->
      -- Smoke test: sleep(0) must not error and must return.
      Timer.sleep 0
      ut\assertTrue true

    sleep_onClass: (ut) ->
      -- sleep is a static method accessible directly on the class.
      ut\assertFunction Timer.sleep

    sleep_onInstance: (ut) ->
      -- sleep is also accessible through an instance (class method inheritance).
      t = Timer!
      ut\assertFunction t.sleep

    _order: {
      "elapsed_nonNegative", "elapsed_monotonic",
      "elapsed_advancesAfterSleep", "elapsed_isReadOnly",
      "stop_freezesElapsed", "start_resumesAfterStop", "reset_clearsAccumulated",
      "getTime_isCallable", "getTime_monotonic", "getTime_readsMilliseconds",
      "sleep_isCallable", "sleep_onClass", "sleep_onInstance"
    }
  }
