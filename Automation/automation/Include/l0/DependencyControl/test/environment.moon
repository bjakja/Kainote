-- Environment tests: platform identity and the headless check.
-- Called from test.moon as: (controls\requireTest "environment")!
->
  constants = require "l0.DependencyControl.Constants"
  environment = require "l0.DependencyControl.environment"

  {
    _description: "Tests for the runtime environment helpers."

    platform_isOsArchString: (ut) ->
      ut\assertString environment.platform
      ut\assertMatches environment.platform, "^%w+%-%w+$" -- e.g. "Windows-x64"

    -- isHeadless mirrors the presence of DepCtrl's private global; toggle it and restore under pcall
    -- so a failed assertion can't leak the mutated global into later tests
    isHeadless_reflectsPrivateGlobal: (ut) ->
      key = constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX
      orig = aegisub[key]
      ok, err = pcall ->
        aegisub[key] = {}
        ut\assertTrue environment.isHeadless!
        aegisub[key] = nil
        ut\assertFalse environment.isHeadless!
      aegisub[key] = orig
      error err unless ok

    _order: {
      "platform_isOsArchString", "isHeadless_reflectsPrivateGlobal"
    }
  }
