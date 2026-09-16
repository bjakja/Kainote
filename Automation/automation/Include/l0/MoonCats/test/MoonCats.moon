-- MoonCats orchestrator tests: the two-pass package extraction, cross-module symbol
-- resolution, and the parse-failure path.
-- Receives the module under test from the suite root (a direct require would recurse
-- into l0.MoonCats' own load, which registers this suite).
(MoonCats) ->
  Diagnostics = require "l0.MoonCats.Diagnostics"
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  {:buildPackageSymbols} = UnitTestSuite\getTestExports MoonCats
  FindingCode = Diagnostics.FindingCode

  definitionFor = (result, requireId) ->
    for def in *result.definitions
      return def.text if def.requireId == requireId

  {
    _description: "MoonCats: two-pass type-definition extraction over module sets."

    extractPackage_crossModuleTyping: (ut) ->
      -- the re-exported Common static resolves to the other module's annotated class name
      result = MoonCats!\extractPackage {
        {requireId: "l0.Test.Common", source: "---@class TestCommon\nclass Common\n  ---validates\n  ---@param ns string\n  ---@return boolean ok\n  @validate = (ns) -> true\nreturn Common"}
        {requireId: "l0.Test.Main", source: 'Common = require "l0.Test.Common"\nclass Main\n  @Common = Common\nreturn Main'}
      }
      ut\assertEquals #result.definitions, 2
      mainDef = definitionFor result, "l0.Test.Main"
      ut\assertMatches mainDef, "%-%-%-@field Common TestCommon"
      ut\assertFalse result.diagnostics\hasCheckFailures!

    extractPackage_aliasesSharedAcrossModules: (ut) ->
      -- an alias declared in one module types enum fields in another
      result = MoonCats!\extractPackage {
        {requireId: "l0.Test.Types", source: "---@alias Mode\n---| \"fast\" # Fast: quick\nx = 1\n\nf = -> 1\nreturn {f: f}"}
        {requireId: "l0.Test.Owner", source: 'Enum = require "l0.DependencyControl.Enum"\nclass Owner\n  @Mode = Enum "Mode", {Fast: "fast"}\nreturn Owner'}
      }
      ownerDef = definitionFor result, "l0.Test.Owner"
      ut\assertMatches ownerDef, "%-%-%-@field Fast Mode"

    extractPackage_parseFailureIsolated: (ut) ->
      -- a broken module yields a finding; the healthy one still emits
      result = MoonCats!\extractPackage {
        {requireId: "l0.Test.Broken", source: "class {{{"}
        {requireId: "l0.Test.Fine", source: "class Fine\n  ---documented\n  go: => 1\nreturn Fine"}
      }
      ut\assertEquals #result.definitions, 1
      ut\assertEquals result.definitions[1].requireId, "l0.Test.Fine"
      found = false
      for finding in *result.diagnostics.findings
        found = true if finding.code == FindingCode.ParseFailure and finding.requireId == "l0.Test.Broken"
      ut\assertTrue found
      ut\assertTrue result.diagnostics\hasCheckFailures!

    extractModule_matchesSingleModulePackage: (ut) ->
      source = "class Solo\n  ---documented\n  go: => 1\nreturn Solo"
      definitionText, diagnostics = MoonCats!\extractModule source, "l0.Test.Solo"
      result = MoonCats!\extractPackage {{requireId: "l0.Test.Solo", source: source}}
      ut\assertEquals definitionText, result.definitions[1].text
      ut\assertFalse diagnostics\hasCheckFailures!

    extractModule_nilOnParseFailure: (ut) ->
      definitionText, diagnostics = MoonCats!\extractModule "class {{{", "l0.Test.Broken"
      ut\assertNil definitionText
      ut\assertTrue diagnostics\hasCheckFailures!

    renderDocs_producesPagesAndScaffold: (ut) ->
      result, diagnostics = MoonCats!\renderDocs {
        {requireId: "l0.Test.Solo", source: "class Solo\n  ---documented\n  go: => 1\nreturn Solo"}
      }, {siteName: "Smoke API"}
      ut\assertEquals #result.pages, 1
      ut\assertMatches result.pages[1].text, "# l0%.Test%.Solo"
      ut\assertMatches result.indexPage.text, "# Smoke API"
      ut\assertEquals result.scaffold[1].path, "mkdocs.yml"
      ut\assertFalse diagnostics\hasCheckFailures!

    buildPackageSymbols_collectsTypeNamesAndAliases: (ut) ->
      parser = MoonCats.Parser!
      irA = parser\parse "---@class RealA\nclass A\n  go: => 1\nreturn A", "l0.Test.A"
      irB = parser\parse "{\n  X: 1\n}", "l0.Test.B"
      packageSymbols = buildPackageSymbols {irA, irB}
      ut\assertEquals packageSymbols.typeNameByRequireId["l0.Test.A"], "RealA"
      -- table modules type as their require identifier
      ut\assertEquals packageSymbols.typeNameByRequireId["l0.Test.B"], "l0.Test.B"
  }
