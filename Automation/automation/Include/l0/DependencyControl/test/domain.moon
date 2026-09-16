-- Domain tests: namespace validation, shared terms, and install/test directory resolution.
-- Called from test.moon as: (controls\requireTest "domain")!
->
  domain = require "l0.DependencyControl.domain"

  {
    _description: "Tests for the DependencyControl domain vocabulary: namespace validation, terms, and install locations."

    capitalizeTerms: (ut) ->
      ut\assertEquals domain.terms.capitalize("hello world"), "Hello world"

    -- validateNamespace: pure computation, no stubs needed

    validateNamespace_valid: (ut) ->
      result, err = domain.validateNamespace "l0.DependencyControl"
      ut\assertTrue result
      ut\assertNil err

    validateNamespace_multiPart: (ut) ->
      result, err = domain.validateNamespace "a.b.c"
      ut\assertTrue result
      ut\assertNil err

    validateNamespace_noDot: (ut) ->
      result, err = domain.validateNamespace "no-dot"
      ut\assertFalse result
      ut\assertString err

    validateNamespace_leadingDot: (ut) ->
      result, err = domain.validateNamespace ".foo.bar"
      ut\assertFalse result
      ut\assertString err

    validateNamespace_trailingDot: (ut) ->
      result, err = domain.validateNamespace "foo.bar."
      ut\assertFalse result
      ut\assertString err

    validateNamespace_invalidChars: (ut) ->
      result, err = domain.validateNamespace "foo bar.baz"
      ut\assertFalse result
      ut\assertString err

    validateNamespace_consecutiveDots: (ut) ->
      result, err = domain.validateNamespace "foo..bar"
      ut\assertFalse result
      ut\assertString err

    -- getAutomationDir

    getAutomationDir_automation: (ut) ->
      result = domain.getAutomationDir domain.ScriptType.Automation
      ut\assertString result
      ut\assertContains result, "autoload"

    getAutomationDir_module: (ut) ->
      result = domain.getAutomationDir domain.ScriptType.Module
      ut\assertString result
      ut\assertContains result, "include"

    getAutomationDir_customRoot: (ut) ->
      (ut\stub aegisub, "decode_path")\calls (path) -> path
      result = domain.getAutomationDir domain.ScriptType.Automation, "myRoot"
      ut\assertString result
      ut\assertContains result, "myRoot"
      ut\assertContains result, "autoload"

    getAutomationDir_unknown: (ut) ->
      result = domain.getAutomationDir 99
      ut\assertNil result

    -- getTestDir

    getTestDir_automation: (ut) ->
      result = domain.getTestDir domain.ScriptType.Automation
      ut\assertString result
      ut\assertContains result, "macros"

    getTestDir_module: (ut) ->
      result = domain.getTestDir domain.ScriptType.Module
      ut\assertString result
      ut\assertContains result, "modules"

    getTestDir_customRoot: (ut) ->
      (ut\stub aegisub, "decode_path")\calls (path) -> path
      result = domain.getTestDir domain.ScriptType.Module, "myRoot"
      ut\assertString result
      ut\assertContains result, "myRoot"
      ut\assertContains result, "DepUnit"

    _order: {
      "capitalizeTerms",
      "validateNamespace_valid", "validateNamespace_multiPart",
      "validateNamespace_noDot", "validateNamespace_leadingDot",
      "validateNamespace_trailingDot", "validateNamespace_invalidChars",
      "validateNamespace_consecutiveDots",
      "getAutomationDir_automation", "getAutomationDir_module",
      "getAutomationDir_customRoot", "getAutomationDir_unknown",
      "getTestDir_automation", "getTestDir_module", "getTestDir_customRoot"
    }
  }
