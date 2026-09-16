ffi = require "ffi"
constants = require "l0.DependencyControl.Constants"

---Runtime facts about the DependencyControl host: the platform identity and whether it is
---running outside a real Aegisub session.
---@class Environment
---@field platform string OS-arch identifier, e.g. "Windows-x64".
Environment = {
  platform: "#{ffi.os}-#{ffi.arch}"

  ---Whether DependencyControl is running headless — outside a real Aegisub session, on the Aegisub
  ---shims (the CLI and unit test runner). Lets a script skip Aegisub-session-only startup work.
  ---@return boolean headless
  isHeadless: -> aegisub[constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX] != nil
}

return Environment
