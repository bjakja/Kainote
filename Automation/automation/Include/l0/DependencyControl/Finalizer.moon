constants = require "l0.DependencyControl.Constants"

-- namespaced key under which `guard` pins its finalizer; collision-free across the shared DepCtrl namespace
FINALIZER_KEY = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}Finalizer"

---Runs a cleanup action when a value is garbage-collected — a last-resort release for OS handles, locks, and
---the like when a caller forgets to release explicitly.
---@class Finalizer
class Finalizer
  ---Creates a finalizer that runs the given cleanup once, when it is collected. Store it on the object whose
  ---lifetime should trigger the cleanup. The cleanup must capture the resources it frees, not the owning
  ---object — capturing the owner keeps it reachable, so the finalizer never runs.
  ---@param cleanup fun() Runs once when the finalizer is collected; its errors are swallowed.
  ---@return userdata finalizer An opaque value to anchor; it exposes no usable fields.
  @create = (cleanup) ->
    finalizer = newproxy true
    (getmetatable finalizer).__gc = -> pcall cleanup
    finalizer

  ---Creates a finalizer and pins it to the anchor under a private key, so the cleanup runs when the anchor is
  ---collected. Pass an instance to release its resources on GC, or a class to tie the cleanup to the module's
  ---lifetime. The cleanup must capture the resources it frees, not the anchor.
  ---@param anchor table The instance or class whose collection triggers the cleanup.
  ---@param cleanup fun() Runs once when the anchor is collected; its errors are swallowed.
  ---@return table anchor The same anchor, for chaining.
  @guard = (anchor, cleanup) ->
    anchor[FINALIZER_KEY] = Finalizer.create cleanup
    anchor

return Finalizer
