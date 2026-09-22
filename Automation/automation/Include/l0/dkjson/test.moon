-- dkjson wrapper tests: the Prettier-flavored encoder (used to persist the config and write feeds
-- back to disk) must emit valid JSON *and* a stable, Prettier-shaped layout — two-space indents, a
-- space after each colon, one property per line, arrays collapsed only when they fit the print width,
-- a deterministic key order, and a single trailing newline. Most tests pin the exact output string,
-- since round-trips alone would pass even if the formatting were completely broken. A non-string
-- object key — e.g. a stray integer left on a malformed required-module spec — has to be quoted, or
-- the file it writes can no longer be parsed.
-- The l0.dkjson package's own test suite; run via `depctrl test` or the DepUnit test menu.
UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"

return UnitTestSuite "l0.dkjson", (dkjson) ->
  prettier = (value, extra) ->
    state = {indentMode: "prettier"}
    if extra then state[k] = v for k, v in pairs extra
    dkjson.encode value, state

  -- assemble an expected block: the given lines joined by newlines, plus the trailing newline every
  -- Prettier document ends with
  lines = (...) -> "#{table.concat {...}, '\n'}\n"

  encoderTests = {
    _description: "Tests for the DependencyControl dkjson wrapper's Prettier encoder."

    -- layout: indentation, colon spacing, one property per line

    encode_prettierSingleKeyObjectLayout: (ut) ->
      ut\assertEquals prettier({k: 1}), lines '{', '  "k": 1', '}'

    encode_prettierEndsWithSingleTrailingNewline: (ut) ->
      -- exactly one trailing newline (a non-newline byte followed by the final "\n")
      ut\assertMatches prettier({k: 1}), "[^\n]\n$"

    encode_prettierObjectOnePropertyPerLine: (ut) ->
      ut\assertEquals prettier({name: "x", version: "1.2.3"}),
        lines '{', '  "name": "x",', '  "version": "1.2.3"', '}'

    encode_prettierNestedObjectIndents: (ut) ->
      ut\assertEquals prettier({outer: {inner: 1}}),
        lines '{', '  "outer": {', '    "inner": 1', '  }', '}'

    -- arrays: collapsed when they fit, one element per line otherwise

    encode_prettierArrayInlineWhenItFits: (ut) ->
      ut\assertEquals prettier({list: {1, 2, 3}}), lines '{', '  "list": [1, 2, 3]', '}'

    encode_prettierArrayBreaksWhenTooWide: (ut) ->
      ut\assertEquals prettier({numbers: {1, 2, 3, 4, 5}}, {indentPrintWidth: 12}),
        lines '{', '  "numbers": [', '    1,', '    2,', '    3,', '    4,', '    5', '  ]', '}'

    encode_prettierNestedArraysInlineWhenTheyFit: (ut) ->
      ut\assertEquals prettier({rows: {{1, 2, 3}, {4, 5, 6}}}),
        lines '{', '  "rows": [[1, 2, 3], [4, 5, 6]]', '}'

    encode_prettierArrayOfObjectsAlwaysBreaks: (ut) ->
      -- a non-empty object forces its containing array onto multiple lines even when it would fit
      ut\assertEquals prettier({items: {{a: 1}, {b: 2}}}),
        lines '{', '  "items": [', '    {', '      "a": 1', '    },', '    {',
              '      "b": 2', '    }', '  ]', '}'

    encode_prettierArrayOfEmptyObjectsStaysInline: (ut) ->
      -- empty objects don't force a break, so the array can still collapse
      ut\assertEquals prettier({arr: {{}, {}}}), lines '{', '  "arr": [{}, {}]', '}'

    -- empty containers

    encode_prettierEmptyObject: (ut) ->
      ut\assertEquals prettier({}), "{}\n"

    encode_prettierEmptyTaggedArray: (ut) ->
      arr = setmetatable {}, __jsontype: "array"
      ut\assertEquals prettier(arr), "[]\n"

    -- container classification honors dkjson's decode-time __jsontype tag

    encode_prettierJsontypeTagOverridesShapeHeuristic: (ut) ->
      -- an empty table would default to an object; the tag makes it an array
      tagged = setmetatable {}, __jsontype: "array"
      ut\assertEquals prettier({payload: tagged}), lines '{', '  "payload": []', '}'

    -- key ordering

    encode_prettierKeyorderPrecedesDefaultSort: (ut) ->
      ut\assertEquals prettier({c: 3, a: 1, b: 2}, {keyorder: {"b", "a"}}),
        lines '{', '  "b": 2,', '  "a": 1,', '  "c": 3', '}'

    encode_prettierDefaultSortIsCaseInsensitiveAlphabetical: (ut) ->
      ut\assertEquals prettier({Banana: 1, apple: 2, Cherry: 3}),
        lines '{', '  "apple": 2,', '  "Banana": 1,', '  "Cherry": 3', '}'

    encode_prettierCustomDefaultKeyOrder: (ut) ->
      ut\assertEquals prettier({a: 1, b: 2, c: 3}, {defaultKeyOrder: (x, y) -> x > y}),
        lines '{', '  "c": 3,', '  "b": 2,', '  "a": 1', '}'

    encode_prettierDeterministicOnCaseCollidingKeys: (ut) ->
      -- keys that fold to the same lowercase string must serialize identically no matter the
      -- order they were inserted in, so re-persisting a config never churns the file
      a = {}
      a.name = 1
      a.Name = 2
      a.beta = 3
      a.Beta = 4
      b = {}
      b.Beta = 4
      b.beta = 3
      b.Name = 2
      b.name = 1
      ut\assertEquals prettier(a), prettier b

    -- scalar and sentinel values

    encode_prettierNullSentinel: (ut) ->
      ut\assertEquals prettier({value: dkjson.null}), lines '{', '  "value": null', '}'

    encode_prettierBooleanAndNumberValues: (ut) ->
      ut\assertEquals prettier({count: 0, flag: false}),
        lines '{', '  "count": 0,', '  "flag": false', '}'

    encode_prettierTopLevelScalarString: (ut) ->
      ut\assertEquals prettier("hello"), '"hello"\n'

    encode_prettierTopLevelScalarNumber: (ut) ->
      ut\assertEquals prettier(42), "42\n"

    -- non-string keys quote (regression: a stray integer key must not emit unquoted)

    encode_prettierQuotesIntegerKey: (ut) ->
      ut\assertEquals prettier({[2]: "json"}), lines '{', '  "2": "json"', '}'

    encode_prettierQuotesMixedKeys: (ut) ->
      decoded = dkjson.decode prettier {moduleName: "ffi", [2]: "json"}
      ut\assertNotNil decoded
      ut\assertEquals decoded.moduleName, "ffi"
      ut\assertEquals decoded["2"], "json"

    encode_prettierKeepsSequenceAsArray: (ut) ->
      -- a plain sequence stays a JSON array, not an object with stringified indices
      decoded = dkjson.decode prettier {"a", "b", "c"}
      ut\assertNotNil decoded
      ut\assertEquals #decoded, 3
      ut\assertEquals decoded[1], "a"
      ut\assertEquals decoded[3], "c"

    -- values with escapes survive the round-trip

    encode_prettierEscapesRoundTrip: (ut) ->
      decoded = dkjson.decode prettier {path: 'a\nb\t"c"'}
      ut\assertNotNil decoded
      ut\assertEquals decoded.path, 'a\nb\t"c"'

    -- indentPrintWidth boundary: the fit check counts the full line, key and colon included

    encode_prettierIndentPrintWidthBoundaryFits: (ut) ->
      -- `  "k": [1, 2, 3]` is exactly 16 columns
      ut\assertEquals prettier({k: {1, 2, 3}}, {indentPrintWidth: 16}),
        lines '{', '  "k": [1, 2, 3]', '}'

    encode_prettierIndentPrintWidthBoundaryBreaks: (ut) ->
      ut\assertEquals prettier({k: {1, 2, 3}}, {indentPrintWidth: 15}),
        lines '{', '  "k": [', '    1,', '    2,', '    3', '  ]', '}'

    -- any other indentMode (or none) defers entirely to upstream dkjson

    encode_nonPrettierDefersToUpstream: (ut) ->
      compact = dkjson.encode {k: 1}
      ut\assertNil compact\find "\n", 1, true
      ut\assertEquals dkjson.decode(compact).k, 1

    -- reference cycles raise like upstream dkjson, rather than overflowing the stack

    encode_prettierRaisesReferenceCycleOnObjectCycle: (ut) ->
      o = {name: "x"}
      o.self = o
      ut\assertErrorMsgMatches prettier, {o}, "reference cycle"

    encode_prettierRaisesReferenceCycleOnArrayCycle: (ut) ->
      a = {1, 2}
      a[3] = a
      ut\assertErrorMsgMatches prettier, {a}, "reference cycle"

    encode_prettierEncodesSharedAcyclicReferences: (ut) ->
      -- the same table in sibling positions is a DAG, not a cycle, and must still encode
      shared = {x: 1}
      decoded = dkjson.decode prettier {a: shared, b: shared}
      ut\assertNotNil decoded
      ut\assertEquals decoded.a.x, 1
      ut\assertEquals decoded.b.x, 1
  }

  {Encoder: encoderTests}
