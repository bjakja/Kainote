ffi = require "ffi"
Accessors = require "l0.DependencyControl.Accessors"

local getTime, sleep

if ffi.os == "Windows"
  -- each cdef gets its own pcall so a Sleep redeclaration conflict can't block the QPC/QPF definitions
  pcall ffi.cdef, "int QueryPerformanceCounter(long long *lpPerformanceCount);"
  pcall ffi.cdef, "int QueryPerformanceFrequency(long long *lpFrequency);"
  pcall ffi.cdef, "unsigned int Sleep(unsigned int dwMilliseconds);"

  freq = ffi.new "long long[1]"
  ffi.C.QueryPerformanceFrequency freq
  freq = tonumber freq[0]

  counter = ffi.new "long long[1]"
  getTime = ->
    ffi.C.QueryPerformanceCounter counter
    tonumber(counter[0]) / freq * 1000

  sleep = (ms) -> ffi.C.Sleep ms

else
  CLOCK_MONOTONIC = ffi.os == "OSX" and 6 or 1

  pcall ffi.cdef, [[
    struct timespec { long tv_sec; long tv_nsec; };
    int clock_gettime(int clk_id, struct timespec *tp);
    int poll(struct pollfd *fds, unsigned long nfds, int timeout);
  ]]

  ts = ffi.new "struct timespec"
  getTime = ->
    ffi.C.clock_gettime CLOCK_MONOTONIC, ts
    tonumber(ts.tv_sec) * 1000 + tonumber(ts.tv_nsec) * 1e-6

  sleep = (ms) -> ffi.C.poll nil, 0, ms

---Timer with monotonic clock readings and millisecond sleep.
---Not affected by system clock changes.
---@class Timer
---@field elapsed number Milliseconds measured so far, excluding any intervals during which the timer was stopped. Read-only.
class Timer
  ---Creates a new timer, running from the current time.
  new: =>
    @accumulated = 0
    @start!

  elapsed: Accessors.property
    get: => @accumulated + (@running and getTime! - @startTime or 0)

  ---Resumes measurement from the current time. No-op if already running, so a prior
  ---stop/start round trip never discards accumulated time.
  ---@return Timer self for chaining
  start: =>
    unless @running
      @startTime = getTime!
      @running = true
    return @

  ---Pauses measurement, folding the elapsed interval into the accumulated total.
  ---No-op if already stopped.
  ---@return Timer self for chaining
  stop: =>
    if @running
      @accumulated += getTime! - @startTime
      @running = false
    return @

  ---Clears the accumulated time and restarts measuring from the current time,
  ---preserving the running/stopped state.
  ---@return Timer self for chaining
  reset: =>
    @accumulated = 0
    @startTime = getTime!
    return @

  ---Sleeps for the given number of milliseconds.
  ---@param ms number
  sleep: sleep

  @sleep = sleep

  ---Returns the current value of the process's monotonic clock, in milliseconds, at
  ---sub-millisecond resolution. Only differences between readings are meaningful.
  ---@return number milliseconds
  @getTime = getTime

Accessors.install Timer

return Timer
