-- Headless shim for the Aegisub automation Lua API.
-- Installs `aegisub` as a global before any module that requires Aegisub-specific APIs.
--
-- Configurable via environment variables:
--   DEPCTRL_USER_DIR  — base for ?user / ?local  (default: %APPDATA%\Aegisub / ~/.aegisub)
--   DEPCTRL_DATA_DIR  — base for ?data            (default: same as ?user; real Aegisub uses exe dir)
--   DEPCTRL_TEMP_DIR  — base for ?temp            (default: %TEMP% / /tmp)

ffi = require "ffi"
utils = require "l0.DependencyControl.utils"

isWindows = ffi.os == "Windows"
pathSep = isWindows and "\\" or "/"

tempDir = os.getenv("DEPCTRL_TEMP_DIR") or (isWindows and (os.getenv("TEMP")) or "/tmp")
userDir = os.getenv("DEPCTRL_USER_DIR") or
  (isWindows and "#{os.getenv 'APPDATA'}\\Aegisub" or "#{os.getenv 'HOME'}/.aegisub")
dataDir = os.getenv("DEPCTRL_DATA_DIR") or userDir

userPathsAddedToPackagePathLua = {}
userPathsAddedToPackagePathMoon = {}

makePackagePaths = (dir, ext) -> {"#{dir}/?.#{ext}", "#{dir}/?/init.#{ext}"}

-- Canonical token table matching libaegisub/path.cpp.
-- Empty string means "unset" — decode_path returns the path unchanged (same as real Aegisub).
-- ?audio, ?script, ?video are empty because no file is loaded headlessly.
pathTokens = {
  "?audio": ""
  "?data": dataDir
  "?dictionary": dataDir .. pathSep .. "dictionaries"
  "?local": userDir
  "?script": ""
  "?temp": tempDir
  "?user": userDir
  "?video": ""
}

-- Sorted longest-first so ?dictionary matches before ?data. Rebuilt whenever a token
-- changes; decodePath closes over the `sortedTokens` upvalue, so reassigning it here is
-- enough to update the resolver.
local sortedTokens
rebuildSortedTokens = ->
  sortedTokens = [{spec, dir} for spec, dir in pairs pathTokens]
  table.sort sortedTokens, (a, b) -> #a[1] > #b[1]
rebuildSortedTokens!

-- Normalize a token name to its canonical "?name" form so callers may pass either
-- "user" or "?user".
normalizeToken = (spec) ->
  "string" == type(spec) and (spec\sub(1, 1) == "?" and spec or "?#{spec}") or spec

---Points an Aegisub path token (e.g. "?user", "?temp") at a different directory.
---Lets headless callers relocate where DepCtrl reads/writes without environment variables.
---@param spec string The token to set, with or without the leading "?" ("user" or "?user").
---@param dir? string The directory to resolve the token to; nil/"" marks it unset.
---@return string? dir The value the token now resolves to.
setPathToken = (spec, dir) ->
  normalizedToken = normalizeToken spec
  previousDir = pathTokens[normalizedToken]
  return dir if previousDir == dir

  pathTokens[normalizedToken] = dir or ""
  rebuildSortedTokens!

  if normalizedToken == "?user"
    -- Re-point the module search paths at the new ?user dir, dropping our prior additions so they
    -- don't pile up. Order of the surviving entries is preserved, so module shadowing (first match
    -- wins) holds.
    modulesDir = "#{dir}/automation/modules"
    package.path, userPathsAddedToPackagePathLua = utils.mergeSearchPath(
      package.path, makePackagePaths(modulesDir, "lua"), userPathsAddedToPackagePathLua)
    package.moonpath, userPathsAddedToPackagePathMoon = utils.mergeSearchPath(
      package.moonpath, makePackagePaths(modulesDir, "moon"), userPathsAddedToPackagePathMoon)
  return dir

---Returns the directory an Aegisub path token currently resolves to.
---@param spec string The token to query, with or without the leading "?".
---@return string? dir The configured directory, or nil if the token is unknown.
getPathToken = (spec) ->
  dir = pathTokens[normalizeToken spec]
  return dir if dir and dir != ""

decodePath = (path) ->
  for {spec, dir} in *sortedTokens
    if path\sub(1, #spec) == spec
      -- Empty dir means token is unset — return path as-is (Aegisub behavior).
      return path if dir == ""
      suffix = path\sub #spec + 1
      -- Consume the separator that follows the token, if any.
      suffix = suffix\sub 2 if suffix\sub(1, 1) == "/" or suffix\sub(1, 1) == "\\"
      return suffix == "" and dir or dir .. pathSep .. suffix
  return path -- no token: return as-is

---Writes a log message to stderr, interpolating printf-style arguments like real Aegisub. Accepts both
---call forms: `(msg, ...)` and `(level, msg, ...)` with a numeric level, which is ignored here.
---The message is written verbatim with no trailing newline: callers append their own, so adding one
---would break same-line output.
---@param level number|string The numeric log level, or the message when the level is omitted.
---@param msg? any The message (or its first format argument in the level-less form).
---@param ... any Arguments interpolated into the message with string.format.
writeLog = (level, msg, ...) ->
  local text
  if type(level) == "string"
    text = if msg != nil then level\format msg, ... else level
  else
    text = if select("#", ...) > 0 then msg\format ... else msg
  io.stderr\write tostring text or ""

aegisub = {
  lua_automation_version: 4

  decode_path: decodePath

  -- Always-nil stubs for context-dependent queries.
  frame_from_ms: -> nil -- nil when no video loaded
  ms_from_frame: -> nil
  video_size: -> nil
  keyframes: -> nil
  get_audio_selection: -> nil
  project_properties: -> nil
  file_name: -> nil

  -- No-ops.
  register_macro: -> nil
  register_filter: -> nil
  set_undo_point: -> nil
  set_status_text: -> nil

  -- text_extents needs font rendering; error loudly rather than returning garbage.
  text_extents: -> error "aegisub.text_extents is not available in headless mode", 2

  gettext: (s) -> s

  cancel: -> error "aegisub.cancel", 2

  -- These are normally injected by LuaProgressSink during macro execution.
  -- We provide static stubs so scripts that call them at module load time don't crash.
  log: writeLog

  debug: {
    out: writeLog
  }

  progress: {
    set: -> nil
    task: -> nil
    title: -> nil
    is_cancelled: -> false
  }

  dialog: {
    -- every headless dialog reports a cancel: button false, plus an empty result-values table
    display: -> false, {}
    open: -> nil
    save: -> nil
  }

  clipboard: {
    get: -> ""
    set: -> true
  }
}

-- Shim-only configuration hooks, namespaced so they can't collide with the real
-- Aegisub API surface. Surfaced through l0.AegisubShims for callers to use.
aegisub.__depCtrl = {
  :setPathToken
  :getPathToken
}

_G.aegisub = aegisub

return aegisub
