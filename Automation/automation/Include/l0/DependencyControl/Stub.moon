
utils = require "l0.DependencyControl.utils"
Logger = require "l0.DependencyControl.Logger"
constants = require "l0.DependencyControl.Constants"
Finalizer = require "l0.DependencyControl.Finalizer"

defaultLogger = Logger fileBaseName: "#{constants.DEPCTRL_SHORT_NAME}.Stub"

msgs = {
  notCalled: "Expected stub to have been called, but it was never called."
  wasCalled: "Expected stub not to have been called, but it was called %d time(s)."
  wrongCallCount: "Expected stub to have been called %d time(s), but it was called %d time(s)."
  notCalledWith: "No call matched the expected arguments (stub was called %d time(s)).\n  Expected: %s"
  noNthCall: "Expected at least %d call(s), but stub was only called %d time(s)."
  wrongCall: "Call #%d arguments did not match.\n  Expected: %s\n    Actual: %s"
  calledAfterRestore: "Stub for '%s' was called after being restored."
  finalizer: {
    notRestored: "Stub for '%s' was not restored before being garbage collected."
  }
}

_stubMatch = (call, expected) ->
  for i = 1, expected.n
    return false unless utils.equals call[i], expected[i]
  return true

---A callable stub that records invocations and supports fluent configuration and assertions.
---Can be used standalone or via UnitTest:stub for automatic lifecycle management.
---@class Stub
class Stub
  ---Creates a spy on a method, recording calls while still invoking the original method.
  ---@param table table|string The table to spy into, or a module name (looked up in the module cache).
  ---@param key string The field name to spy on.
  ---@param logger? Logger Logger to use; a default logger is used when nil.
  ---@param unitTest? UnitTest Unit test instance used to report assertion failures.
  ---@return Stub
  @spy = (table, key, logger, unitTest) =>
    s = @ table, key, logger, unitTest
    return s\calls (...) -> s._originalMethod ...

  ---Creates a stub, optionally replacing a key in a table.
  ---@param table? table|string The table to stub into, or a module name (looked up in the module cache).
  ---@param key? string The field name to replace; no table is modified when nil.
  ---@param logger? Logger Logger to use; a default logger is used when nil.
  ---@param unitTest? UnitTest Unit test instance to report assertion failures to; failures throw when nil.
  new: (table, key, logger, unitTest) =>
    @_calls = {}
    @_replacement = ->
    @unitTest = unitTest
    restored = {false}
    @_restored = restored
    @logger = logger

    if type(table) == "string"
      table = package.loaded[table]

    if table != nil and key != nil
      @_targetTable = table
      @_targetMethodKey = key
      @_originalMethod = table[key]
      table[key] = @

      -- warn if this stub is garbage-collected without restore() having been called
      keyRef, logger = key, @logger or defaultLogger
      Finalizer.guard @, ->
        unless restored[1]
          pcall logger.warn, logger, msgs.finalizer.notRestored, keyRef

  __call: (...) =>
    @__fail msgs.calledAfterRestore, @_targetMethodKey if @_restored[1]
    @_calls[#@_calls + 1] = table.pack ...
    repl = @_replacement
    return repl ...

  ---Sets the function to invoke when the stub is called.
  ---@param impl function
  ---@return Stub self
  calls: (impl) =>
    @_replacement = impl
    return @

  ---Sets the stub to return fixed values on every call.
  ---@param ... any Values to return from every call.
  ---@return Stub self
  returns: (...) =>
    vals = table.pack ...
    @_replacement = -> unpack vals, 1, vals.n
    return @

  ---Restores the original value that was replaced by this stub.
  restore: =>
    if @_targetTable != nil
      @_targetTable[@_targetMethodKey] = @_originalMethod
      @_restored[1] = true

  ---Reports an assertion failure, routing it through the unit test when one is set or throwing otherwise.
  ---@private
  ---@param msg string A printf-style format string describing the failure.
  ---@param ... any Values interpolated into the format string.
  __fail: (msg, ...) =>
    if @unitTest
      @unitTest\assert false, msg, ...
    else
      error string.format(msg, ...), 2

  ---Formats a value for inclusion in a failure message.
  ---@private
  ---@param val any The value to stringify.
  ---@return string dump The value's string representation.
  __dump: (val) =>
    return @unitTest.logger\dumpToString val if @unitTest
    return tostring val

  ---Asserts the stub was called at least once.
  assertCalled: =>
    @__fail msgs.notCalled unless #@_calls > 0

  ---Asserts the stub was never called.
  assertNotCalled: =>
    @__fail msgs.wasCalled, #@_calls unless #@_calls == 0

  ---Asserts the stub was called exactly the given number of times.
  ---@param n integer The expected call count.
  assertCalledTimes: (n) =>
    @__fail msgs.wrongCallCount, n, #@_calls unless #@_calls == n

  ---Asserts the stub was called exactly once.
  assertCalledOnce: =>
    @__fail msgs.wrongCallCount, 1, #@_calls unless #@_calls == 1

  ---Asserts the stub was called exactly once and with the given arguments.
  ---@param ... any The arguments expected on the single call.
  assertCalledOnceWith: (...) =>
    @__fail msgs.wrongCallCount, 1, #@_calls unless #@_calls == 1
    expected = table.pack ...
    @__fail msgs.wrongCall, 1, @__dump(expected), @__dump(@_calls[1]) unless _stubMatch @_calls[1], expected

  ---Asserts at least one recorded call was made with the given arguments.
  ---@param ... any The arguments expected on some call.
  assertCalledWith: (...) =>
    expected = table.pack ...
    for call in *@_calls
      return if _stubMatch call, expected
    @__fail msgs.notCalledWith, #@_calls, @__dump expected

  ---Asserts the most recent call was made with the given arguments.
  ---@param ... any The arguments expected on the last call.
  assertLastCalledWith: (...) =>
    expected = table.pack ...
    last = @_calls[#@_calls]
    @__fail msgs.notCalled unless last != nil
    @__fail msgs.wrongCall, #@_calls, @__dump(expected), @__dump last unless _stubMatch last, expected

  ---Asserts the nth recorded call was made with the given arguments.
  ---@param n integer The 1-based index of the call to check.
  ---@param ... any The arguments expected on that call.
  assertNthCalledWith: (n, ...) =>
    expected = table.pack ...
    call = @_calls[n]
    @__fail msgs.noNthCall, n, #@_calls unless call != nil
    @__fail msgs.wrongCall, n, @__dump(expected), @__dump call unless _stubMatch call, expected

return Stub
