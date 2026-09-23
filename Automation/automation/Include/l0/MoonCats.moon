DependencyControl = require "l0.DependencyControl"
constants = require "l0.DependencyControl.Constants"
Parser = require "l0.MoonCats.Parser"
Emitter = require "l0.MoonCats.Emitter"
DocRenderer = require "l0.MoonCats.DocRenderer"
Diagnostics = require "l0.MoonCats.Diagnostics"

FindingCode = Diagnostics.FindingCode

version = DependencyControl {
  name: "MoonCATS"
  version: "0.7.0" -- @{l0.MoonCats:version}
  description: "Extracts LuaCATS annotations from MoonScript sources into LuaLS type definitions."
  author: "line0"
  moduleName: "l0.MoonCats"
  url: "https://github.com/TypesettingTools/DependencyControl"
  feed: constants.DEPCTRL_FEED_URL
}

---One MoonScript module handed to an extraction run.
---@class MoonCatsSource
---@field requireId string Require identifier the module resolves under.
---@field source string The module's MoonScript source text.

---The outcome of an extraction run.
---@class MoonCatsExtractionResult
---@field definitions {requireId: string, text: string}[] One definition file per successfully parsed module, in input order.
---@field diagnostics MoonCatsDiagnostics All findings from parsing and emission.

---Builds the cross-module symbol context the emitter resolves references against.
---@param irs table[] The run's parsed module IRs.
---@return MoonCatsPackageSymbols packageSymbols The run-wide type-name and alias tables, empty when no module resolved a type.
buildPackageSymbols = (irs) ->
  packageSymbols = {typeNameByRequireId: {}, aliases: {}}
  for ir in *irs
    typeName = switch ir.export.kind
      when Parser.ExportKind.Class
        ir.export.class and ir.export.class.typeName
      when Parser.ExportKind.Table
        -- the module's own declared @class name when it has one, else the synthesized require-id class
        declared = nil
        for seg in *ir.segments
          declared = ir.export.name if seg.kind == Parser.SegmentKind.Class and seg.name == ir.export.name
        declared or ir.requireId
    packageSymbols.typeNameByRequireId[ir.requireId] = typeName if typeName
    for name in pairs ir.aliases
      packageSymbols.aliases[name] = true
  packageSymbols

---Extracts LuaCATS annotations from MoonScript sources into LuaLS .d.lua type-definition files
---and rendered API documentation.
---@class MoonCats
---@field private __parser MoonCatsParser
---@field private __emitter MoonCatsEmitter
---@field private __docRenderer MoonCatsDocRenderer
class MoonCats
  ---@type MoonCatsParser
  @Parser = Parser
  ---@type MoonCatsEmitter
  @Emitter = Emitter
  ---@type MoonCatsDocRenderer
  @DocRenderer = DocRenderer
  ---@type MoonCatsDiagnostics
  @Diagnostics = Diagnostics

  ---Creates an extractor.
  new: =>
    @__parser = Parser!
    @__emitter = Emitter!
    @__docRenderer = DocRenderer!

  ---Parses every source into its IR and builds the run-wide symbol context.
  ---@private
  ---@param sources MoonCatsSource[]
  ---@param diagnostics MoonCatsDiagnostics
  ---@return table[] irs The IRs of the modules that parsed, in input order.
  ---@return MoonCatsPackageSymbols packageSymbols The run-wide type-name and alias context those IRs resolve against.
  __parseAll: (sources, diagnostics) =>
    irs = {}
    for moduleSource in *sources
      ir, err = @__parser\parse moduleSource.source, moduleSource.requireId, diagnostics
      if ir
        table.insert irs, ir
      else
        diagnostics\add FindingCode.ParseFailure, moduleSource.requireId, nil, err
    irs, buildPackageSymbols irs

  ---Extracts type-definition files for a set of modules, resolving cross-module references
  ---against every module in the set.
  ---@param sources MoonCatsSource[] The modules to extract, each with its require identifier.
  ---@return MoonCatsExtractionResult result Definitions for the modules that parsed, plus all findings.
  extractPackage: (sources) =>
    diagnostics = Diagnostics!
    irs, packageSymbols = @__parseAll sources, diagnostics
    definitions = for ir in *irs
      {requireId: ir.requireId, text: @__emitter\emit ir, packageSymbols, diagnostics}
    {:definitions, :diagnostics}

  ---Renders API documentation pages for a set of modules, resolving cross-module
  ---references and type links against every module in the set.
  ---@param sources MoonCatsSource[] The modules to document, each with its require identifier.
  ---@param opts? MoonCatsDocOptions
  ---@return MoonCatsDocResult result Pages, index, and site scaffolding.
  ---@return MoonCatsDiagnostics diagnostics All findings from the parse pass.
  renderDocs: (sources, opts) =>
    diagnostics = Diagnostics!
    irs, packageSymbols = @__parseAll sources, diagnostics
    result = @__docRenderer\render irs, packageSymbols, opts
    result, diagnostics

  ---Extracts the type-definition file for a single module without cross-module context.
  ---@param source string The module's MoonScript source text.
  ---@param requireId string Require identifier the module resolves under.
  ---@return string? definition The definition text, or nil when the module fails to parse.
  ---@return MoonCatsDiagnostics diagnostics All findings from the run.
  extractModule: (source, requireId) =>
    result = @extractPackage {{:requireId, :source}}
    definition = result.definitions[1]
    definition and definition.text, result.diagnostics

UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
UnitTestSuite\withTestExports MoonCats, {:buildPackageSymbols}
return version\register MoonCats
