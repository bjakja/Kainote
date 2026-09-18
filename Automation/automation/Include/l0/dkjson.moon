---DependencyControl wrapper around David Kolf's dkjson: a thin overlay on the vendored upstream module,
---adding a DependencyControl version record and a Prettier-flavored `indentMode` encode option while
---deferring every other call to upstream dkjson.
---
---The bare specifiers this module `provides` are "json" and "dkjson", resolved by DependencyControl's
---module searcher. A locally installed dkjson, luajson, or other JSON module imported by bare specifier
---takes precedence over this one.

-- The vendored upstream library is kept pristine at `modules/l0/dkjson/vendor/dkjson.lua`, so a new
-- release can be dropped in as a straight copy.
dkjson = require "l0.dkjson.vendor.dkjson"

DEFAULT_PRETTIER_PRINT_WIDTH = 80

-- Mirrors dkjson's own error so a reference cycle raises the same message in every encode mode.
msgs = {
  prettyEncode: {
    referenceCycle: "reference cycle"
  }
}

---Detects a reference cycle: a table reachable from itself through nested tables. A `__tojson` value
---is treated as an opaque leaf, since dkjson performs its own cycle check on the contents it renders.
---@param val any The value to inspect.
---@param ancestors? table Tables already on the current path, tracked internally by the recursion.
---@return boolean cyclic True when val reaches itself through nested tables.
hasReferenceCycle = (val, ancestors = {}) ->
  meta = getmetatable val
  return false if type(val) != "table" or (meta and meta.__tojson)
  return true if ancestors[val]
  ancestors[val] = true
  for _, child in pairs val
    return true if hasReferenceCycle child, ancestors
  ancestors[val] = nil
  false

---Serializes a Lua value to Prettier-flavored JSON: two-space indents, a space after every colon, one
---property per line for objects, arrays kept on one line when they fit the print width (otherwise one
---element per line), and a trailing newline. Keys in `state.keyorder` are emitted first in that order.
---Remaining keys follow `state.defaultKeyOrder`, case-insensitive alphabetical by default, so output is
---deterministic. Scalars and the null sentinel are delegated to upstream dkjson for correct escaping.
---@param value any The value to serialize.
---@param state? DkJsonEncodeState The Prettier formatting options.
---@return string json The Prettier-formatted JSON, ending in a newline.
prettyEncode = (value, state = {}) ->
  keyorder = state.keyorder or {}
  printWidth = state.indentPrintWidth or DEFAULT_PRETTIER_PRINT_WIDTH
  ---Default object-key comparator: case-insensitive, breaking ties on the raw key so keys that fold to
  ---the same lowercase string keep a stable total order and byte-identical output.
  ---@param a any The first key.
  ---@param b any The second key.
  ---@return boolean before True when a should sort before b.
  defaultKeySorter = (a, b) ->
    la, lb = string.lower(tostring a), string.lower tostring b
    return la < lb unless la == lb
    tostring(a) < tostring b
  defaultKeySorter = state.defaultKeyOrder if type(state.defaultKeyOrder) == "function"

  rank = {k, i for i, k in ipairs keyorder}
  ---Renders an indent level as two-space indentation.
  ---@param level integer The indent depth.
  ---@return string indent The indentation string.
  indentStr = (level) -> ("  ")\rep level
  ---Encodes an object key as a JSON string, coercing non-string keys with tostring first, since a raw
  ---non-string key would emit unquoted and produce invalid JSON.
  ---@param k any The key to encode.
  ---@return string json The key as a quoted JSON string.
  encodeKey = (k) -> dkjson.encode tostring k

  ---Classifies a table as a JSON "array" or "object", honoring dkjson's decode-time `__jsontype` tag and
  ---otherwise falling back to a key-shape heuristic. Empty tables classify as objects.
  ---@param tbl table The table to classify.
  ---@param meta? table The table's metatable, or nil when it has none.
  ---@return "array"|"object" jsontype The JSON container kind.
  classify = (tbl, meta) ->
    return meta.__jsontype if meta and meta.__jsontype
    len, count = #tbl, 0
    count += 1 for _ in pairs tbl
    return len > 0 and len == count and "array" or "object"

  ---Returns a table's keys ordered by `keyorder` rank first, then by the default sorter.
  ---@param tbl table The object table whose keys to order.
  ---@return any[] keys The keys in emission order.
  orderedKeys = (tbl) ->
    keys = [k for k in pairs tbl]
    table.sort keys, (a, b) ->
      ra, rb = rank[a], rank[b]
      return ra < rb if ra and rb
      return ra != nil if ra or rb
      defaultKeySorter a, b
    keys

  local compact, forcesBreak, render

  ---Renders a value on a single line, used only to measure whether an array fits the current line.
  ---@param val any The value to render.
  ---@return string json The value serialized without line breaks.
  compact = (val) ->
    meta = getmetatable val
    return dkjson.encode val if type(val) != "table" or (meta and meta.__tojson)
    if classify(val, meta) == "array"
      "[#{table.concat [compact v for v in *val], ", "}]"
    else
      "{#{table.concat ["#{encodeKey k}: #{compact val[k]}" for k in *orderedKeys val], ", "}}"

  ---Reports whether a value must span multiple lines regardless of width: a non-empty object always
  ---breaks, and an array breaks when any of its elements does.
  ---@param val any The value to test.
  ---@return boolean breaks True when the value cannot render on a single line.
  forcesBreak = (val) ->
    meta = getmetatable val
    return false if type(val) != "table" or (meta and meta.__tojson)
    if classify(val, meta) == "array"
      for v in *val
        return true if forcesBreak v
      false
    else next(val) != nil

  ---Renders a value as Prettier-formatted JSON, breaking objects and over-width arrays across lines.
  ---@param val any The value to render.
  ---@param col integer The column the value begins at, used to decide whether an array still fits the line.
  ---@param level integer The current indent depth.
  ---@return string json The formatted JSON fragment.
  render = (val, col, level) ->
    meta = getmetatable val
    return dkjson.encode val if type(val) != "table" or (meta and meta.__tojson)

    if classify(val, meta) == "array"
      return "[]" if #val == 0
      unless forcesBreak val
        inline = compact val
        return inline if col + #inline <= printWidth
      inner = indentStr level + 1
      "[\n#{table.concat ["#{inner}#{render v, (level + 1) * 2, level + 1}" for v in *val], ",\n"}\n#{indentStr level}]"
    else
      keys = orderedKeys val
      return "{}" if #keys == 0
      inner = indentStr level + 1
      parts = for k in *keys
        key = encodeKey k
        "#{inner}#{key}: #{render val[k], #inner + #key + 2, level + 1}"
      "{\n#{table.concat parts, ",\n"}\n#{indentStr level}}"

  -- encode tail-calls this, so its frame is elided and level 2 lands on encode's caller. That is the
  -- frame upstream dkjson blames, so a cycle raises the same message whether or not prettier is used.
  error msgs.prettyEncode.referenceCycle, 2 if hasReferenceCycle value
  "#{render value, 0, 0}\n"

---dkjson encode state, extended with the DependencyControl wrapper's Prettier options.
---@class DkJsonEncodeState
---@field indentMode? string Set to `"prettier"` for Prettier-flavored formatting; any other value defers to upstream dkjson.
---@field indentPrintWidth? integer Target line width for the prettier indent mode. Defaults to 80.
---@field defaultKeyOrder? fun(a: any, b: any): boolean Sorts object keys absent from `keyorder`, returning true when the first sorts ahead. Defaults to case-insensitive alphabetical, and applies only in the prettier indent mode.
---@field keyorder? any[] Object keys emitted first, in this order, ahead of `defaultKeyOrder`.

---David Kolf's dkjson, wrapped for DependencyControl.
---@class DkJson
---@field version DependencyControl The DependencyControl record, populated on initialization.
wrapper = setmetatable {}, __index: dkjson

---Encodes a Lua value as JSON. With `state.indentMode` set to `"prettier"`, output is Prettier-flavored:
---two-space indents, one property per line, arrays inlined when they fit `state.indentPrintWidth`, and a
---trailing newline. Any other indent mode (or none) defers entirely to upstream dkjson.
---A reference cycle raises `"reference cycle"`, as upstream dkjson does in every other encode mode.
---@param value any The value to encode.
---@param state? DkJsonEncodeState Encode state carrying the Prettier options plus any upstream dkjson state.
---@return string|boolean json The JSON string, or `true` when `state.buffer` is supplied (upstream dkjson's native return).
wrapper.encode = (value, state) ->
  return prettyEncode value, state if state and state.indentMode == "prettier"
  return dkjson.encode value, state

---Initializes this module's DependencyControl version record, invoked once by the module loader.
---@private
---@param DependencyControl table The DependencyControl class handed to the initializer.
wrapper.__depCtrlInit = (DependencyControl) ->
  wrapper.version = DependencyControl {
    name: "dkjson"
    version: "0.8.0" -- @{l0.dkjson:version}
    description: "David Kolf's JSON module for Lua."
    author: "David Kolf"
    moduleName: "l0.dkjson"
    url: "http://dkolf.de/dkjson-lua/"
    feed: "https://raw.githubusercontent.com/TypesettingTools/DependencyControl/publish/DependencyControl.json"
    provides: {"json", "dkjson"}
  }
  -- prevent update failure when coming from DependencyControl v0.6.x, which breaks when modules register
  -- themselves from the initializer hook by checking for presence of a field that only exists in v0.7.0+.
  wrapper.version\register wrapper if wrapper.version.semanticVersion

return wrapper
