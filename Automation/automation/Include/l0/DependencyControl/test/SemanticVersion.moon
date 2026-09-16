-- SemanticVersion tests: the static utilities (toPacked, toString, check, ranges) and the version
-- value instance (construction, comparison, bumping, and interop with the static layer).
-- Called from Tests.moon as: (require "...test.SemanticVersion")!
->
  SemanticVersion = require "l0.DependencyControl.SemanticVersion"

  {
    _description: "Tests for SemanticVersion: static version/range utilities plus the version value instance."

    -- toPacked

    toPacked_string: (ut) ->
      result, err = SemanticVersion\toPacked "1.2.3"
      ut\assertEquals result, 66051
      ut\assertNil err

    toPacked_zero: (ut) ->
      result, err = SemanticVersion\toPacked "0.0.0"
      ut\assertEquals result, 0
      ut\assertNil err

    toPacked_number: (ut) ->
      result = SemanticVersion\toPacked 66051
      ut\assertEquals result, 66051

    toPacked_nil: (ut) ->
      result = SemanticVersion\toPacked nil
      ut\assertEquals result, 0

    toPacked_badString: (ut) ->
      result, err = SemanticVersion\toPacked "1.2"
      ut\assertFalse result
      ut\assertString err

    toPacked_overflow: (ut) ->
      result, err = SemanticVersion\toPacked "1.256.0"
      ut\assertFalse result
      ut\assertString err

    toPacked_badType: (ut) ->
      result, err = SemanticVersion\toPacked {}
      ut\assertFalse result
      ut\assertString err

    -- toString

    toString_fromNumber: (ut) ->
      result, err = SemanticVersion\toString 66051
      ut\assertEquals result, "1.2.3"
      ut\assertNil err

    toString_roundtrip: (ut) ->
      result, err = SemanticVersion\toString "1.2.3"
      ut\assertEquals result, "1.2.3"
      ut\assertNil err

    toString_majorPrecision: (ut) ->
      result = SemanticVersion\toString 66051, "major"
      ut\assertEquals result, "1.0.0"

    toString_nilRendersZero: (ut) ->
      result, err = SemanticVersion\toString nil
      ut\assertEquals result, "0.0.0"
      ut\assertNil err

    toString_fromInstance: (ut) ->
      result = SemanticVersion\toString SemanticVersion "1.2.3"
      ut\assertEquals result, "1.2.3"

    toString_badString: (ut) ->
      result, err = SemanticVersion\toString "not-a-version"
      ut\assertNil result
      ut\assertString err

    -- check

    check_equal: (ut) ->
      result, b = SemanticVersion\check "1.2.3", "1.2.3"
      ut\assertTrue result

    check_greater: (ut) ->
      result = SemanticVersion\check "2.0.0", "1.0.0"
      ut\assertTrue result

    check_less: (ut) ->
      result = SemanticVersion\check "1.0.0", "2.0.0"
      ut\assertFalse result

    check_majorPrecision: (ut) ->
      result = SemanticVersion\check "2.0.0", "1.9.9", "major"
      ut\assertTrue result

    check_badArg: (ut) ->
      result, err = SemanticVersion\check "bad", "1.0.0"
      ut\assertNil result
      ut\assertString err

    -- check with precision "range": b is an npm-style range string

    check_rangeMode: (ut) ->
      ut\assertTrue SemanticVersion\check("1.2.9", "~1.2.3", "range")
      ut\assertFalse SemanticVersion\check("1.3.0", "~1.2.3", "range")

    check_rangeBadRange: (ut) ->
      result, err = SemanticVersion\check "1.2.3", "garbage", "range"
      ut\assertNil result
      ut\assertString err

    -- satisfiesRange: npm semver range syntax

    satisfiesRange_tilde: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.3", "~1.2.3")
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.9", "~1.2.3")
      ut\assertFalse SemanticVersion\satisfiesRange("1.3.0", "~1.2.3")
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.0", "~1.2") -- ~1.2 => >=1.2.0 <1.3.0
      ut\assertFalse SemanticVersion\satisfiesRange("2.0.0", "~1") -- ~1 => >=1.0.0 <2.0.0

    satisfiesRange_caret: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("1.9.9", "^1.2.3")
      ut\assertFalse SemanticVersion\satisfiesRange("2.0.0", "^1.2.3")
      ut\assertTrue SemanticVersion\satisfiesRange("0.2.9", "^0.2.3") -- 0.x: minor is left-most non-zero
      ut\assertFalse SemanticVersion\satisfiesRange("0.3.0", "^0.2.3")
      ut\assertTrue SemanticVersion\satisfiesRange("0.0.3", "^0.0.3") -- 0.0.x: patch is left-most non-zero
      ut\assertFalse SemanticVersion\satisfiesRange("0.0.4", "^0.0.3")

    satisfiesRange_xRangeAndAny: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("1.5.0", "1.x")
      ut\assertFalse SemanticVersion\satisfiesRange("2.0.0", "1.x")
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.7", "1.2.x")
      ut\assertTrue SemanticVersion\satisfiesRange("5.0.0", "*")
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.3", "") -- empty range => any

    satisfiesRange_exact: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.3", "1.2.3")
      ut\assertFalse SemanticVersion\satisfiesRange("1.2.4", "1.2.3")

    satisfiesRange_comparators: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("1.5.0", ">=1.2.3 <2.0.0")
      ut\assertFalse SemanticVersion\satisfiesRange("2.0.0", ">=1.2.3 <2.0.0")
      ut\assertTrue SemanticVersion\satisfiesRange("2.0.0", ">1") -- >1 => >=2.0.0
      ut\assertFalse SemanticVersion\satisfiesRange("1.5.0", ">1")
      ut\assertTrue SemanticVersion\satisfiesRange("1.3.0", ">1.2") -- >1.2 => >=1.3.0
      ut\assertTrue SemanticVersion\satisfiesRange("1.0.0", "<=1") -- <=1 => <2.0.0

    satisfiesRange_orUnion: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("0.9.0", "<1.0.0 || >=2.0.0")
      ut\assertTrue SemanticVersion\satisfiesRange("2.1.0", "<1.0.0 || >=2.0.0")
      ut\assertFalse SemanticVersion\satisfiesRange("1.5.0", "<1.0.0 || >=2.0.0")

    satisfiesRange_hyphen: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.3", "1.2.3 - 2.3.4")
      ut\assertTrue SemanticVersion\satisfiesRange("2.3.4", "1.2.3 - 2.3.4") -- inclusive upper
      ut\assertFalse SemanticVersion\satisfiesRange("2.4.0", "1.2.3 - 2.3.4")
      ut\assertTrue SemanticVersion\satisfiesRange("2.3.9", "1.2.3 - 2.3") -- partial upper => <2.4.0
      ut\assertFalse SemanticVersion\satisfiesRange("2.4.0", "1.2.3 - 2.3")

    -- npm allows an operator to stand apart from its version, and a range written that way must not
    -- quietly reduce to one nothing satisfies
    satisfiesRange_spacedOperator: (ut) ->
      ut\assertTrue SemanticVersion\satisfiesRange("0.6.4", "< 0.7.0")
      ut\assertFalse SemanticVersion\satisfiesRange("0.7.0", "< 0.7.0")
      ut\assertTrue SemanticVersion\satisfiesRange("0.8.2", ">= 0.7.0 < 0.9.0")
      ut\assertFalse SemanticVersion\satisfiesRange("0.9.0", ">= 0.7.0 < 0.9.0")
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.9", "~ 1.2.3")
      ut\assertTrue SemanticVersion\satisfiesRange("1.2.3", "1.2.3 - 2.3.4") -- a hyphen range still parses

    satisfiesRange_errors: (ut) ->
      r1, e1 = SemanticVersion\satisfiesRange "1.2.3", "garbage"
      ut\assertNil r1
      ut\assertString e1
      r2, e2 = SemanticVersion\satisfiesRange "bad", "1.2.3"
      ut\assertNil r2
      ut\assertString e2
      r3, e3 = SemanticVersion\satisfiesRange "1.2.3", nil
      ut\assertNil r3
      ut\assertString e3

    -- parseRange: range string -> set of half-open [min, max) intervals

    parseRange_intervals: (ut) ->
      intervals = SemanticVersion\parseRange "~1.2.3"
      ut\assertEquals #intervals, 1
      ut\assertEquals intervals[1].min, SemanticVersion\toPacked "1.2.3"
      ut\assertEquals intervals[1].max, SemanticVersion\toPacked "1.3.0" -- exclusive upper

    parseRange_unsatisfiableIsEmpty: (ut) ->
      intervals = SemanticVersion\parseRange ">2.0.0 <1.0.0"
      ut\assertEquals #intervals, 0

    parseRange_badType: (ut) ->
      result, err = SemanticVersion\parseRange nil
      ut\assertNil result
      ut\assertString err

    -- rangesIntersect: do two ranges share any version?

    rangesIntersect_overlap: (ut) ->
      ut\assertTrue SemanticVersion\rangesIntersect("~1.2.3", "~1.2")
      ut\assertTrue SemanticVersion\rangesIntersect(">=1.0.0 <2.0.0", "^1.5.0")
      ut\assertTrue SemanticVersion\rangesIntersect("1.x", "1.5.x")

    rangesIntersect_disjoint: (ut) ->
      ut\assertFalse SemanticVersion\rangesIntersect("~1.2", "~1.3") -- [1.2,1.3) vs [1.3,1.4) adjacent
      ut\assertFalse SemanticVersion\rangesIntersect("<1.0.0", ">=1.0.0")
      ut\assertFalse SemanticVersion\rangesIntersect("1.x", "2.x")

    rangesIntersect_unionGroups: (ut) ->
      ut\assertTrue SemanticVersion\rangesIntersect("^1.0.0 || ^2.0.0", "~2.0")
      ut\assertFalse SemanticVersion\rangesIntersect("^1.0.0 || ^3.0.0", "~2.0")

    rangesIntersect_emptyAndExactBounds: (ut) ->
      ut\assertFalse SemanticVersion\rangesIntersect(">2.0.0 <1.0.0", "*") -- contradictory range matches nothing
      ut\assertTrue SemanticVersion\rangesIntersect("1.2.3 - 2.0.0", ">=2.0.0") -- inclusive upper meets >=
      ut\assertFalse SemanticVersion\rangesIntersect("1.2.3 - 1.9.9", ">=2.0.0")
      ut\assertFalse SemanticVersion\rangesIntersect("=1.2.3", "=1.2.4")

    rangesIntersect_error: (ut) ->
      r, e = SemanticVersion\rangesIntersect "garbage", "1.2.3"
      ut\assertNil r
      ut\assertString e

    -- getRangeMaxVersion: the highest version a range can supply (for ranking competing ranges)

    getRangeMaxVersion_bounds: (ut) ->
      ut\assertEquals SemanticVersion\getRangeMaxVersion("^1"), SemanticVersion\toPacked "1.255.255"
      ut\assertEquals SemanticVersion\getRangeMaxVersion("~1.2"), SemanticVersion\toPacked "1.2.255"
      ut\assertEquals SemanticVersion\getRangeMaxVersion("*"), SemanticVersion\toPacked "255.255.255"

    getRangeMaxVersion_unionTakesHighest: (ut) ->
      ut\assertEquals SemanticVersion\getRangeMaxVersion("^1.0.0 || ^2.0.0"), SemanticVersion\toPacked "2.255.255"

    getRangeMaxVersion_emptyAndError: (ut) ->
      ut\assertNil SemanticVersion\getRangeMaxVersion ">2.0.0 <1.0.0" -- empty range supplies nothing
      r, e = SemanticVersion\getRangeMaxVersion "garbage"
      ut\assertNil r
      ut\assertString e

    -- instance API: construction, comparison, bumping, and interop with the static layer

    new_fromString: (ut) ->
      v = SemanticVersion "1.2.3"
      ut\assertEquals {v.major, v.minor, v.patch}, {1, 2, 3}
      ut\assertEquals tostring(v), "1.2.3"

    new_fromComponents: (ut) ->
      full = SemanticVersion 1, 2, 3
      ut\assertEquals {full.major, full.minor, full.patch}, {1, 2, 3}
      ut\assertEquals tostring(SemanticVersion(1, 2)), "1.2.0" -- patch defaults to 0
      ut\assertEquals tostring(SemanticVersion(1)), "1.0.0" -- minor and patch default to 0

    new_raisesOnInvalid: (ut) ->
      ut\assertFalse (pcall -> SemanticVersion "nope") -- unparseable string
      ut\assertFalse (pcall -> SemanticVersion 1, 2, 999) -- component out of range
      ut\assertFalse (pcall -> SemanticVersion 1, -1, 0) -- negative component

    fromPacked_roundTripsAndValidates: (ut) ->
      v = SemanticVersion "3.4.5"
      ut\assertEquals tostring(SemanticVersion.fromPacked v\toPacked!), "3.4.5"
      ut\assertFalse (pcall SemanticVersion.fromPacked, -1)
      ut\assertFalse (pcall SemanticVersion.fromPacked, 0x1000000)

    parse_returnsInstanceOrError: (ut) ->
      ut\assertEquals tostring(SemanticVersion.parse "1.2.3"), "1.2.3"
      r, err = SemanticVersion.parse "garbage"
      ut\assertNil r
      ut\assertString err
      r2, err2 = SemanticVersion.parse 5 -- non-string input
      ut\assertNil r2
      ut\assertString err2

    toPacked_andStaticUnwrap: (ut) ->
      v = SemanticVersion "1.2.3"
      ut\assertEquals v\toPacked!, SemanticVersion\toPacked "1.2.3"
      ut\assertEquals SemanticVersion\toPacked(v), v\toPacked! -- static toPacked unwraps an instance
      ut\assertTrue SemanticVersion.isHigher SemanticVersion("2.0.0"), v -- statics accept instances too

    compare_operators: (ut) ->
      a = SemanticVersion "1.2.3"
      ut\assertTrue a < SemanticVersion "1.2.4"
      ut\assertTrue a <= SemanticVersion "1.2.3"
      ut\assertTrue a == SemanticVersion "1.2.3"
      ut\assertTrue a != SemanticVersion "2.0.0"
      ut\assertFalse a > SemanticVersion "1.2.4"

    compare_mixedOperands: (ut) ->
      a = SemanticVersion "1.2.3"
      ut\assertTrue a < "1.2.4" -- against a string
      ut\assertTrue a < SemanticVersion("1.2.4")\toPacked! -- against a packed number
      ut\assertTrue (SemanticVersion("1.2.2")\toPacked!) < a -- number on the left operand

    bump_immutableAndResets: (ut) ->
      a = SemanticVersion "1.2.3"
      ut\assertEquals tostring(a\bumpMajor!), "2.0.0" -- minor/patch reset
      ut\assertEquals tostring(a\bumpMinor!), "1.3.0" -- patch reset
      ut\assertEquals tostring(a\bumpPatch!), "1.2.4"
      ut\assertEquals tostring(a), "1.2.3" -- original is untouched

    satisfies_delegatesToRange: (ut) ->
      v = SemanticVersion "1.2.3"
      ut\assertTrue v\satisfies "^1.2.0"
      ut\assertFalse v\satisfies "^2.0.0"
      r, err = v\satisfies "garbage"
      ut\assertNil r
      ut\assertString err

    _order: {
      "toPacked_string", "toPacked_zero", "toPacked_number", "toPacked_nil",
      "toPacked_badString", "toPacked_overflow", "toPacked_badType",
      "toString_fromNumber", "toString_roundtrip", "toString_majorPrecision",
      "toString_nilRendersZero", "toString_fromInstance", "toString_badString",
      "check_equal", "check_greater", "check_less", "check_majorPrecision", "check_badArg",
      "check_rangeMode", "check_rangeBadRange",
      "satisfiesRange_tilde", "satisfiesRange_caret", "satisfiesRange_xRangeAndAny",
      "satisfiesRange_exact", "satisfiesRange_comparators", "satisfiesRange_orUnion",
      "satisfiesRange_hyphen", "satisfiesRange_spacedOperator", "satisfiesRange_errors",
      "parseRange_intervals", "parseRange_unsatisfiableIsEmpty", "parseRange_badType",
      "rangesIntersect_overlap", "rangesIntersect_disjoint", "rangesIntersect_unionGroups",
      "rangesIntersect_emptyAndExactBounds", "rangesIntersect_error",
      "getRangeMaxVersion_bounds", "getRangeMaxVersion_unionTakesHighest", "getRangeMaxVersion_emptyAndError",
      "new_fromString", "new_fromComponents", "new_raisesOnInvalid",
      "fromPacked_roundTripsAndValidates", "parse_returnsInstanceOrError", "toPacked_andStaticUnwrap",
      "compare_operators", "compare_mixedOperands", "bump_immutableAndResets", "satisfies_delegatesToRange"
    }
  }
