-- General-purpose Lua helpers: deep equality, table copying, set and list operations, string
-- trimming and pattern escaping, array flattening, UUID generation, and rng seeding. Loads at the
-- bottom of the dependency graph with no DependencyControl requires; seedRandom and getRandomSeed
-- lazily pull Timer and NamedSemaphore when first called.

local Timer, NamedSemaphore

-- Compares two values for deep equality. Tables are compared recursively;
-- other types use == except that two identical values always compare equal.
-- Circular references are handled.
_equals = (a, b, aType, bType) ->
  treeA, treeB, depth = {}, {}, 0

  recurse = (a, b, aType = type a, bType) ->
    return true if a == b
    bType or= type b
    return false if aType != bType or aType != "table"

    return false if #a != #b

    aFieldCnt, bFieldCnt = 0, 0
    local tablesSeenAtKeys

    depth += 1
    treeA[depth], treeB[depth] = a, b

    for k, v in pairs a
      vType = type v
      if vType == "table"
        tablesSeenAtKeys or= {}
        tablesSeenAtKeys[k] = true

      -- this key's value cycles back to a table already being compared up the stack; treat the key
      -- as matched and move on to the remaining keys (returning true here would wrongly declare the
      -- whole tables equal and would also skip the depth decrement)
      cyclic = false
      for i = 1, depth
        if v == treeA[i] and b[k] == treeB[i]
          cyclic = true
          break

      unless cyclic or recurse v, b[k], vType
        depth -= 1
        return false

      aFieldCnt += 1

    for k, v in pairs b
      continue if tablesSeenAtKeys and tablesSeenAtKeys[k]
      if bFieldCnt == aFieldCnt or not recurse v, a[k]
        depth -= 1
        return false
      bFieldCnt += 1

    res = recurse getmetatable(a), getmetatable b
    depth -= 1
    return res

  return recurse a, b, aType, bType

-- Compares table items for equality ignoring keys.
-- Delegates table-vs-table comparisons to _equals.
_itemsEqual = (a, b, onlyNumKeys = true, ignoreExtraAItems, requireIdenticalItems) ->
  seen, aTbls = {}, {}
  aCnt, aTblCnt, bCnt = 0, 0, 0

  findEqualTable = (bTbl) ->
    for i, aTbl in ipairs aTbls
      if _equals aTbl, bTbl
        table.remove aTbls, i
        seen[aTbl] -= 1
        return true
    return false

  if onlyNumKeys
    aCnt, bCnt = #a, #b
    return false if not ignoreExtraAItems and aCnt != bCnt

    for v in *a
      seen[v] = (seen[v] or 0) + 1 -- multiset: track occurrences so duplicate items match by count
      if "table" == type v
        aTblCnt += 1
        aTbls[aTblCnt] = v

    for v in *b
      if (seen[v] or 0) > 0
        seen[v] -= 1
        continue

      if type(v) != "table" or requireIdenticalItems or not findEqualTable v
        return false

  else
    for _, v in pairs a
      aCnt += 1
      seen[v] = (seen[v] or 0) + 1 -- multiset: track occurrences so duplicate items match by count
      if "table" == type v
        aTblCnt += 1
        aTbls[aTblCnt] = v

    for _, v in pairs b
      bCnt += 1
      if (seen[v] or 0) > 0
        seen[v] -= 1
        continue

      if type(v) != "table" or requireIdenticalItems or not findEqualTable v
        return false

    return false if not ignoreExtraAItems and aCnt != bCnt

  return true


getTableLength = (tbl) ->
  n = 0
  n += 1 for _, _ in pairs tbl
  return n

isPureArrayTable = (tbl) ->
  typ = type tbl
  return false, nil, typ if typ != "table"
  len = getTableLength tbl
  return #tbl == len, len, typ

flatten = (value, depth = 1, toArrayTable) ->
  flattened, f = {}, 0

  recurse = (v, d) ->
    isArray, _, typ = isPureArrayTable v
    if toArrayTable and not isArray
      v, isArray = toArrayTable v, typ
      isArray = isPureArrayTable(v) if isArray == nil
    if isArray and d > 0
      recurse nestedVal, d - 1 for nestedVal in *v
    else
      f += 1
      flattened[f] = v

  recurse value, depth
  return flattened, f

deepCopy = (tbl) ->
  seen = {}
  recurse = (value) ->
    return value if type(value) != "table"
    return seen[value] if seen[value]

    -- create the copy and register it before recursing into its values, so circular references resolve to the copy
    copy = {}
    seen[value] = copy
    copy[k] = recurse v for k, v in pairs value
    return copy

  return recurse tbl

mergeSearchPath = (pathStr, add, remove) ->
  removed = remove and {p, true for p in *remove} or {}
  seen, ordered = {}, {}
  for path in pathStr\gmatch "[^;]+"
    continue if removed[path] or seen[path]
    seen[path] = true
    ordered[#ordered + 1] = path

  added = {}
  for path in *add
    continue if seen[path]
    seen[path] = true
    ordered[#ordered + 1] = path
    added[#added + 1] = path

  return table.concat(ordered, ";"), added

-- Aegisub loads automation scripts concurrently, each into its own Lua state that seeds its rng as
-- DependencyControl loads. Two states can seed at the same instant and share a process id, so the
-- clock and PID don't tell them apart. Each live Lua state has a distinct address, read here from the
-- running state's main thread, whose tostring yields "thread: 0x<address>".
getStateToken = ->
  addr = tostring(coroutine.running!)\match "0x(%x+)"
  addr and tonumber(addr, 16) or 0

__deriveSeed = (token, clockMs, pid) -> clockMs * 1000 + pid + token

getRandomSeed = ->
  Timer or= require "l0.DependencyControl.Timer"
  NamedSemaphore or= require "l0.DependencyControl.NamedSemaphore"
  __deriveSeed getStateToken!, Timer.getTime!, NamedSemaphore.pid

seedRandom = ->
  seed = getRandomSeed!
  math.randomseed seed
  return seed

---@class Utils
Utils = {
  ---Deep equality comparison. Tables compared recursively; other types use ==.
  ---Circular references are handled. Metatables are included in the comparison.
  ---@param a any
  ---@param b any
  ---@return boolean equal
  equals: _equals

  ---Compares table items for equality, ignoring keys.
  ---By default only numerical indexes are compared.
  ---@param a table
  ---@param b table
  ---@param onlyNumKeys? boolean Compare only sequential numeric indices (default true).
  ---@param ignoreExtraAItems? boolean Allow `a` to contain items absent from `b` (default false).
  ---@param requireIdenticalItems? boolean Require identical (not merely equal) table items (default false).
  ---@return boolean equal
  itemsEqual: _itemsEqual

  ---Shallow-copies a table (no metatable).
  ---@param tbl table The table to copy.
  ---@return table copy The copied table.
  copy: (tbl) -> {k, v for k, v in pairs tbl}

  ---Deep-copies a table recursively (no metatables). Circular references are preserved as cycles in the copy.
  ---@param tbl table The table to deep-copy.
  ---@return table copy The deep-copied table. Keys are carried over as-is; only values are copied.
  deepCopy: deepCopy

  ---Deep-copies a table, and returns any other value as it is, for copying a mix of tables and scalars.
  ---@param value any The value to copy.
  ---@return any copy A deep copy of a table, or the value itself.
  copyValue: (value) -> type(value) == "table" and deepCopy(value) or value

  ---Builds (or extends) a set from an array's values: each value becomes a key mapped to `value`.
  ---@param source any[] Array whose values become the set's keys.
  ---@param target? table Table to populate (default a new table).
  ---@param overwrite? boolean Overwrite keys already present in `target` (default true).
  ---@param value? any Value to map each key to (default true).
  ---@return table set The populated `target`.
  makeSet: (source, target = {}, overwrite = true, value = true) ->
    target[v] = value for v in *source when overwrite or not target[v]
    return target

  ---Reports whether an array contains a value (compared with `==`).
  ---@param list any[] The array to search.
  ---@param value any The value to look for.
  ---@return boolean included Whether `value` is an element of `list`.
  listIncludes: (list, value) ->
    for entry in *list
      return true if entry == value
    return false

  ---Fills in missing entries of `tbl` from `defaults`, mutating `tbl` in place.
  ---@param tbl table The table to fill in.
  ---@param defaults table Default key/value pairs.
  ---@param predicate? fun(value: any, key: any, tbl: table): boolean Per-key test for whether to apply the default; when omitted, defaults apply wherever `tbl[key]` is nil.
  ---@return number addedCount The number of defaults applied.
  addDefaults: (tbl, defaults, predicate) ->
    addedCnt = 0
    for k, v in pairs defaults
      if not predicate and tbl[k] == nil or predicate and predicate tbl[k], k, tbl
        addedCnt += 1
        tbl[k] = v
    return addedCnt

  ---Strips leading and trailing whitespace from a string.
  ---@param str string The string to trim.
  ---@return string trimmed The trimmed string.
  trim: (str) -> (str\gsub "^%s*(.-)%s*$", "%1")

  ---Escapes all Lua pattern magic characters in a string so it matches literally.
  ---@param str string The string to escape.
  ---@return string escaped The escaped string.
  escapePattern: (str) -> (str\gsub "[%^%$%(%)%%%.%[%]%*%+%-%?]", "%%%1")

  ---Flattens nested array tables into a single array up to the specified depth. Values that are not (or not converted to) pure array tables are included as-is.
  ---@param value any The value to flatten.
  ---@param depth? number Maximum depth to flatten (default 1).
  ---@param toArrayTable? fun(value: any, valueType: string): table?, boolean? Converts a non-array value to an array table.
  ---@return table flattened A flattened array table containing the flattened values.
  ---@return number flattenedCount The number of elements in the flattened array.
  flatten: flatten

  ---Merges new entries into a semicolon-separated Lua search path, appending only the ones not
  ---already present and dropping any listed for removal, with the order of kept entries preserved.
  ---@param pathStr string The existing search path, e.g. `package.path`.
  ---@param add string[] Path entries to append, each only when not already present.
  ---@param remove? string[] Path entries to drop before merging, e.g. to undo an earlier addition.
  ---@return string pathStr The merged search-path string.
  ---@return string[] added The entries actually appended, empty when all were already present.
  mergeSearchPath: mergeSearchPath

  ---Extends one of package's search-path fields in place, folding in the entries of a semicolon-
  ---separated string and appending only those not already present. Does nothing when there is nothing to add.
  ---@param field "path"|"cpath"|"moonpath" Which package search path to extend.
  ---@param entries? string A semicolon-separated path string to fold in, such as an env var's value.
  ---@return string[] added The entries actually added, empty when all were present or nothing was passed.
  extendPackagePath: (field, entries) ->
    return {} unless entries and entries != ""
    package[field], added = mergeSearchPath (package[field] or ""), [entry for entry in entries\gmatch "[^;]+"]
    return added

  ---Returns a random-number seed unique to this script's Lua state, differing from one launch to the next.
  ---@return number seed
  getRandomSeed: getRandomSeed

  ---Reseeds this Lua state's random number generator so `math.random` yields a stream unique to the
  ---script and to this launch. DependencyControl seeds it on load; call this to reseed it yourself.
  ---@return number seed The applied seed.
  seedRandom: seedRandom

  ---Pure (token, clockMs, pid) seed combiner, reachable for the seed-divergence test.
  ---@param token number Distinct per-Lua-state identity token.
  ---@param clockMs number Clock reading in milliseconds.
  ---@param pid number The current process id.
  ---@return number seed
  ---@private
  __deriveSeed: __deriveSeed

  ---Generates a random RFC-4122 version-4 UUID string.
  ---@return string uuid
  uuid: ->
    -- https://gist.github.com/jrus/3197011
    -- cspell:ignore yxxx
    "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"\gsub "[xy]", (c) ->
      v = c == "x" and math.random(0, 0xf) or math.random 8, 0xb
      return "%x"\format v
}

return Utils
