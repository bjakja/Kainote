UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"

UnitTestSuite "l0.MoonCats", (MoonCats, ...) ->
  -- The suite controls object is appended by UnitTestSuite\import as the final argument.
  nArgs = select "#", ...
  controls = select nArgs, ...

  {
    Annotations: (controls\requireTest "annotations")!
    Parser: (controls\requireTest "Parser")!
    Emitter: (controls\requireTest "Emitter")!
    DocRenderer: (controls\requireTest "DocRenderer")!
    -- the module under test is injected: requiring l0.MoonCats here would recurse into its own load
    MoonCats: (controls\requireTest "MoonCats") MoonCats
  }
