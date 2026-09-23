-- Finalizer tests: runs a cleanup action when an anchored value is garbage-collected.
-- Called from test.moon as: (controls\requireTest "Finalizer")!
->
  Finalizer = require "l0.DependencyControl.Finalizer"

  -- Calls `alloc flag` (which builds the finalizer-anchored value and must not return it), then forces GC
  -- until the flag flips. `alloc` runs in its own call frame, which LuaJIT reliably reclaims on return, so
  -- its allocation becomes unreachable; a value held by a local in the test body could keep its register
  -- slot live across the collection loop and never be collected.
  collectAfter = (alloc) ->
    flag = {done: false}
    alloc flag
    for _ = 1, 50
      collectgarbage "collect"
      break if flag.done
    flag.done

  {
    _description: "Finalizer: runs a cleanup action when an anchored value is garbage-collected."

    -- create: the cleanup runs when the returned finalizer is collected
    create_runsCleanupOnGC: (ut) ->
      alloc = (flag) ->
        Finalizer.create -> flag.done = true
        return
      ut\assertTrue collectAfter alloc

    -- create: an error raised by the cleanup is swallowed, so it can't escape during collection
    create_swallowsCleanupErrors: (ut) ->
      alloc = (flag) ->
        Finalizer.create ->
          flag.done = true
          error "cleanup boom"
        return
      ut\assertTrue collectAfter alloc

    -- guard: pins the finalizer to the anchor, so the cleanup runs when the anchor is collected
    guard_runsCleanupWhenAnchorCollected: (ut) ->
      alloc = (flag) ->
        Finalizer.guard {}, -> flag.done = true
        return
      ut\assertTrue collectAfter alloc

    -- guard: returns the anchor for chaining
    guard_returnsAnchor: (ut) ->
      anchor = {}
      ut\assertEquals Finalizer.guard(anchor, ->), anchor

    _order: {
      "create_runsCleanupOnGC", "create_swallowsCleanupErrors",
      "guard_runsCleanupWhenAnchorCollected", "guard_returnsAnchor"
    }
  }
