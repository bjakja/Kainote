Timer = require "l0.DependencyControl.Timer"

-- The native PT.PreciseTimer exposes the path of the loaded shared library. This pure-Lua
-- shim has no library, so expose this module's own source path as the closest equivalent.
selfPath = debug.getinfo(1, "S").source\gsub "^@", ""

---A monotonic stopwatch replicating the native PT.PreciseTimer API on top of the
---DependencyControl Timer engine.
---@class PreciseTimer
class PreciseTimer
  -- mirrors the native PT.PreciseTimer version this shim is API-compatible with
  @version = 0x000106
  @version_string = "0.1.6"
  ---@type string
  @loadedLibraryPath = selfPath

  ---Starts a new timer, capturing the current time as its start point.
  new: =>
    @timer = Timer!

  ---Returns the seconds elapsed since the timer was created.
  ---@return number seconds
  timeElapsed: => @timer.elapsed / 1000

  ---Sleeps for the given number of milliseconds.
  ---@param ms? number milliseconds to sleep (defaults to 100)
  sleep: (ms = 100) -> Timer.sleep ms

return PreciseTimer
