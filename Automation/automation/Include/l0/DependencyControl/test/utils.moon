-- Utils tests: flattening, list membership, deep copying, deep equality, item equality, and pattern escaping.
-- Called from test.moon as: (controls\requireTest "utils")!
->
  utils = require "l0.DependencyControl.utils"

  {
    _description: "Tests for the general-purpose utility helpers."

    -- flatten

    flatten_depth2Array: (ut) ->
      flat, n = utils.flatten {{"a", "b"}, {"c"}}, 2
      ut\assertEquals n, 3
      ut\assertEquals flat[1], "a"
      ut\assertEquals flat[2], "b"
      ut\assertEquals flat[3], "c"

    flatten_depth1StopsEarly: (ut) ->
      flat, n = utils.flatten {{"a", "b"}, "c"}, 1
      ut\assertEquals n, 2
      ut\assertTable flat[1]
      ut\assertEquals flat[2], "c"

    flatten_depth0NoFlatten: (ut) ->
      flat, n = utils.flatten {{"a"}, "b"}, 0
      ut\assertEquals n, 1
      ut\assertTable flat[1]

    flatten_scalar: (ut) ->
      flat, n = utils.flatten "hello"
      ut\assertEquals n, 1
      ut\assertEquals flat[1], "hello"

    flatten_returnsCount: (ut) ->
      _, n = utils.flatten {"x", "y", "z"}, 2
      ut\assertEquals n, 3

    flatten_toArrayTable: (ut) ->
      input = {42, "x"}
      converter = (v, typ) ->
        return {"a", "b"} if typ == "number"
        v
      flat, n = utils.flatten input, 2, converter
      ut\assertEquals n, 3
      ut\assertEquals flat[1], "a"
      ut\assertEquals flat[2], "b"
      ut\assertEquals flat[3], "x"

    -- listIncludes: exact (==) membership in an array

    listIncludes_found: (ut) ->
      ut\assertTrue utils.listIncludes {"a", "b", "c"}, "b"

    listIncludes_notFoundAndEmpty: (ut) ->
      ut\assertFalse utils.listIncludes {"a", "b"}, "z"
      ut\assertFalse utils.listIncludes {}, "a"

    -- deepCopy

    deepCopy_nestedTablesDetached: (ut) ->
      keyTbl = {}
      source = {n: 1, nested: {deep: {"x"}}, [keyTbl]: "keyed"}
      copy = utils.deepCopy source
      ut\assertEquals copy, source
      ut\assertIsNot copy.nested, source.nested
      ut\assertIsNot copy.nested.deep, source.nested.deep
      ut\assertEquals copy[keyTbl], "keyed" -- table keys are carried over by identity, not copied
      source.nested.deep[1] = "mutated"
      ut\assertEquals copy.nested.deep[1], "x"

    -- a table reaching back into one already being copied resolves to that copy, so the walk terminates
    -- and the result is a cycle of the same shape rather than a chain of ever-deeper copies
    deepCopy_cyclicRefsPreserved: (ut) ->
      source = {name: "root"}
      source.self = source
      source.child = {parent: source}

      copy = utils.deepCopy source
      ut\assertIsNot copy, source
      ut\assertIs copy.self, copy
      ut\assertIs copy.child.parent, copy
      ut\assertEquals copy.name, "root"

    -- one source table reached through two keys stays one table in the copy
    deepCopy_sharedTableCopiedOnce: (ut) ->
      shared = {"x"}
      copy = utils.deepCopy {first: shared, second: shared}
      ut\assertIsNot copy.first, shared
      ut\assertIs copy.first, copy.second

    -- equals: a cyclic value is treated as matched at that key, not as making the whole tables equal
    equals_cyclicRefs: (ut) ->
      cyc = (x) ->
        t = {:x}
        t.self = t
        t
      ut\assertFalse utils.equals cyc(1), cyc(2) -- a differing sibling key still makes them unequal
      ut\assertTrue utils.equals cyc(1), cyc(1)

    -- itemsEqual counts occurrences, so duplicate scalar items match by multiplicity
    itemsEqual_duplicateScalars: (ut) ->
      ut\assertTrue utils.itemsEqual {1, 1}, {1, 1}
      ut\assertTrue utils.itemsEqual {1, 2}, {1, 2}
      ut\assertFalse utils.itemsEqual {1, 1}, {1, 2}
      ut\assertFalse utils.itemsEqual {1, 1, 2}, {1, 2, 2}

    escapePattern_matchesLiterally: (ut) ->
      s = "a-mo.Line[1] (v2)+?*^$%"
      escaped = utils.escapePattern s
      ut\assertEquals (s\find escaped), 1
      ut\assertNil ("amo.Line[1] (v2)+?*^$%")\find "^" .. escaped -- hyphen no longer quantifies

    escapePattern_plainStringUnchanged: (ut) ->
      ut\assertEquals utils.escapePattern("l0_Functional"), "l0_Functional"

    -- Aegisub loads each automation script concurrently, each into its own Lua state, and those states
    -- seed their rng at the same instant while sharing one pid, so the clock and pid don't tell them
    -- apart. Hold both fixed and give each the identity of a state at a neighbouring address; the seeds
    -- and their first draws must still come out distinct, or per-script log file names and temp paths
    -- would collide.
    seedRandom_divergesAcrossSimultaneousStates: (ut) ->
      ut\assertNumber utils.seedRandom!
      seeds, heads = {}, {}
      base = 0x1a2b00000000 -- a plausible Lua state address; neighbours differ by an allocation offset
      for i = 0, 199
        seed = utils.__deriveSeed base + i * 0x10, 1234.5, 4242 -- frozen clock and pid
        ut\assertNil seeds[seed]
        seeds[seed] = true
        math.randomseed seed
        heads[math.random 0, 16^4 - 1] = true
      distinctHeads = 0
      distinctHeads += 1 for _ in pairs heads
      -- the head is a 16-bit draw, so a few of the 200 may collide by birthday chance; requiring the
      -- vast majority distinct still fails hard for a repeating seed, which collapses to a single head
      ut\assertGreaterThan distinctHeads, 190

    _order: {
      "flatten_depth2Array", "flatten_depth1StopsEarly", "flatten_depth0NoFlatten",
      "flatten_scalar", "flatten_returnsCount", "flatten_toArrayTable",
      "listIncludes_found", "listIncludes_notFoundAndEmpty",
      "deepCopy_nestedTablesDetached", "deepCopy_cyclicRefsPreserved", "deepCopy_sharedTableCopiedOnce",
      "equals_cyclicRefs", "itemsEqual_duplicateScalars",
      "escapePattern_matchesLiterally", "escapePattern_plainStringUnchanged",
      "seedRandom_divergesAcrossSimultaneousStates"
    }
  }
