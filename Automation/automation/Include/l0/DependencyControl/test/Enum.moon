-- Enum tests: immutable enumeration types with reverse lookup.
-- Called from Tests.moon as: (require "...test.Enum")!
->
  Enum = require "l0.DependencyControl.Enum"

  {
    _description: "Tests for the Enum class providing immutable enumeration types with reverse lookup."

    -- construction

    new_table: (ut) ->
      e = Enum "MyEnum", {Foo: 1, Bar: 2}
      ut\assertEquals e.Foo, 1
      ut\assertEquals e.Bar, 2

    new_list: (ut) ->
      e = Enum "MyEnum", {"Foo", "Bar"}
      found = e\test "Foo"
      ut\assertTrue found

    new_badName: (ut) ->
      ok, err = pcall -> Enum 42, {Foo: 1}
      ut\assertFalse ok
      ut\assertString err

    new_reservedKey: (ut) ->
      ok, err = pcall -> Enum "MyEnum", {keys: 1}
      ut\assertFalse ok
      ut\assertString err

    new_duplicateValue: (ut) ->
      ok, err = pcall -> Enum "MyEnum", {Foo: 1, Bar: 1}
      ut\assertFalse ok
      ut\assertString err

    -- test

    test_found: (ut) ->
      e = Enum "MyEnum", {Foo: 1, Bar: 2}
      found, val = e\test "Foo"
      ut\assertTrue found
      ut\assertEquals val, 1

    test_notFound: (ut) ->
      e = Enum "MyEnum", {Foo: 1}
      found, val = e\test "Baz"
      ut\assertFalse found
      ut\assertNil val

    -- describe

    describe_single: (ut) ->
      e = Enum "MyEnum", {Foo: 1, Bar: 2}
      result = e\describe 1
      ut\assertEquals result, "1 (Foo)"

    describe_list: (ut) ->
      e = Enum "MyEnum", {Foo: 1, Bar: 2}
      result = e\describe {1, 2}, nil, false
      ut\assertTable result
      ut\assertEquals #result, 2

    describe_join: (ut) ->
      e = Enum "MyEnum", {Foo: 1, Bar: 2}
      result = e\describe {1, 2}, (k) -> k
      ut\assertString result
      ut\assertContains result, "Foo"
      ut\assertContains result, "Bar"

    describe_unknown: (ut) ->
      e = Enum "MyEnum", {Foo: 1}
      result, err = pcall e\describe, 99
      ut\assertFalse result
      ut\assertContains err, "MyEnum"
      ut\assertContains err, "99"

    -- validate

    validate_valid: (ut) ->
      e = Enum "MyEnum", {Foo: 1, Bar: 2}
      result, err = e\validate 1
      ut\assertTrue result
      ut\assertNil err

    validate_invalid: (ut) ->
      e = Enum "MyEnum", {Foo: 1}
      result, err = e\validate 99
      ut\assertNil result
      ut\assertString err

    validate_withArgName: (ut) ->
      e = Enum "MyEnum", {Foo: 1}
      result, err = e\validate 99, "myArg"
      ut\assertNil result
      ut\assertContains err, "myArg"

    -- immutability

    immutable_read: (ut) ->
      e = Enum "MyEnum", {Foo: 1}
      ok, err = pcall -> e.Bar
      ut\assertFalse ok
      ut\assertString err

    immutable_write: (ut) ->
      e = Enum "MyEnum", {Foo: 1}
      ok, err = pcall -> e.Foo = 99
      ut\assertFalse ok
      ut\assertString err

    _order: {
      "new_table", "new_list", "new_badName", "new_reservedKey", "new_duplicateValue",
      "test_found", "test_notFound",
      "describe_single", "describe_list", "describe_join", "describe_unknown",
      "validate_valid", "validate_invalid", "validate_withArgName",
      "immutable_read", "immutable_write"
    }
  }
