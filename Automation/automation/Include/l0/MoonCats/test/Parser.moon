-- Parser tests: comment-block scanning, string masking, segmentation, block binding,
-- member classification, export unwrapping, and enum recognition — all on in-memory snippets.
() ->
  Parser = require "l0.MoonCats.Parser"
  Diagnostics = require "l0.MoonCats.Diagnostics"
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  {:splitLines, :computeStringMask, :scanCommentBlocks, :segmentBlock,
    :paramsFromFndef, :literalTokenFromNode, :fndefHasValueReturn} = UnitTestSuite\getTestExports Parser
  {:MemberKind, :ExportKind, :SegmentKind, :ValueKind, :SymbolKind} = Parser

  parse = (source, requireId = "test.mod") ->
    parser = Parser!
    diagnostics = Diagnostics!
    ir, err = parser\parse source, requireId, diagnostics
    ir, diagnostics, err

  memberByName = (cls, name) ->
    for member in *cls.members
      return member if member.name == name

  {
    _description: "MoonCatsParser: MoonScript source to definition-extraction IR."

    -- ── block scanning ──────────────────────────────────────────────────────

    scan_boundAndStandaloneBlocks: (ut) ->
      ir = parse "---standalone prose block\n\n---bound to f\nf = -> 1\n\nreturn {f: f}"
      -- the bound block landed on the local; the standalone one has no alias/class segments,
      -- so its rest is dropped and nothing hoists
      ut\assertEquals #ir.segments, 0
      ut\assertTable ir.symbols.f
      ut\assertEquals ir.symbols.f.block[1], "---bound to f"

    scan_plainCommentTerminatesBlock: (ut) ->
      -- a plain -- line between doc lines splits them into two blocks and joins neither
      ir = parse "---@alias Foo\n---| \"a\" # A: first\n-- plain divider\n---bound doc\nf = -> 1\nreturn {f: f}"
      ut\assertEquals #ir.segments, 1
      ut\assertEquals ir.segments[1].name, "Foo"
      ut\assertEquals ir.symbols.f.block[1], "---bound doc"

    scan_dashDividerNotDoc: (ut) ->
      blocks = scanCommentBlocks splitLines("---- divider\n---real doc\nx = 1"), {}
      ut\assertEquals #blocks, 1
      ut\assertEquals blocks[1].lines[1], "---real doc"

    scan_privateOnlyBlockBinds: (ut) ->
      ir = parse "class Foo\n  ---@private\n  __hidden: => 1\nreturn Foo"
      member = memberByName ir.classes[1], "__hidden"
      ut\assertTrue member.isPrivate
      ut\assertEquals member.block[1], "---@private"

    scan_untaggedContinuationKept: (ut) ->
      ir = parse "class Foo\n  ---@param x string a wrapped description\n  ---that continues on the next line.\n  go: (x) => x\nreturn Foo"
      member = memberByName ir.classes[1], "go"
      ut\assertEquals #member.block, 2
      ut\assertMatches member.block[2], "continues"

    -- ── string masking ──────────────────────────────────────────────────────

    mask_longStringNeverScanned: (ut) ->
      ir = parse "x = [==[\n---@class NotReal\n---@alias NotAnAlias\n]==]\n\nreturn {x: x}"
      ut\assertEquals #ir.segments, 0

    mask_coversLongStringSpan: (ut) ->
      source = "x = [[\nline\n---@class Fake\n]]\ny = 1"
      moonParse = require "moonscript.parse"
      tree = moonParse.string source
      mask = computeStringMask tree, source
      ut\assertTrue mask[3]

    -- ── segmentation ────────────────────────────────────────────────────────

    segment_aliasClaimsLeadingProse: (ut) ->
      segments = segmentBlock {startLine: 1, endLine: 3, lines: {
        "---Alias description.", "---@alias Mode", "---| \"a\" # A: first"
      }}
      ut\assertEquals #segments, 1
      ut\assertEquals segments[1].kind, SegmentKind.Alias
      ut\assertEquals segments[1].name, "Mode"
      ut\assertEquals #segments[1].lines, 3

    segment_proseAfterVariantsFallsToRest: (ut) ->
      segments = segmentBlock {startLine: 1, endLine: 4, lines: {
        "---@alias Mode", "---| \"a\" # A: first", "---Member doc that follows the alias.", "---@type string"
      }}
      ut\assertEquals #segments, 2
      ut\assertEquals segments[2].kind, SegmentKind.Rest
      ut\assertEquals segments[2].lines[1], "---Member doc that follows the alias."
      ut\assertEquals segments[2].lines[2], "---@type string"

    segment_annotationOnlyClassKeepsFields: (ut) ->
      segments = segmentBlock {startLine: 1, endLine: 3, lines: {
        "---@class Args", "---@field name string The name.", "---@field count? integer"
      }}
      ut\assertEquals #segments, 1
      ut\assertEquals segments[1].kind, SegmentKind.Class
      ut\assertEquals #segments[1].lines, 3

    segment_detachedClassBlocksHoist: (ut) ->
      ir = parse "---@class PartialVersion\n---@field major? integer\n\n---@class Interval\n---@field min integer\n\nmsgs = {a: 1}\n\nf = -> 1\nreturn {f: f}"
      ut\assertEquals #ir.segments, 2
      ut\assertEquals ir.segments[1].name, "PartialVersion"
      ut\assertEquals ir.segments[2].name, "Interval"

    segment_nestedAnnotationOnlyClassInBody: (ut) ->
      ir = parse "class Foo\n  ---@class Spec\n  ---@field url string\n\n  data = {url: 1}\n\n  go: => 1\nreturn Foo"
      ut\assertEquals #ir.segments, 1
      ut\assertEquals ir.segments[1].name, "Spec"

    segment_aliasAboveEnumHoistsAndRegisters: (ut) ->
      ir = parse 'Enum = require "l0.DependencyControl.Enum"\n---@alias Mode\n---| "fast" # Fast: quick\nMode = Enum "Mode", {Fast: "fast"}\nreturn {Mode: Mode}'
      ut\assertTrue ir.aliases.Mode
      ut\assertEquals #ir.enums, 1
      ut\assertEquals ir.enums[1].name, "Mode"

    -- ── classification ──────────────────────────────────────────────────────

    classify_staticShapes: (ut) ->
      ir = parse "class Foo\n  @fat = (a) => a\n  @thin = (a) -> a\n  @colon: (a) => a\n  @answer = 42\nreturn Foo"
      cls = ir.classes[1]
      ut\assertEquals memberByName(cls, "fat").kind, MemberKind.StaticMethodFat
      ut\assertEquals memberByName(cls, "thin").kind, MemberKind.StaticMethodThin
      ut\assertEquals memberByName(cls, "colon").kind, MemberKind.StaticMethodFat
      ut\assertEquals memberByName(cls, "answer").kind, MemberKind.StaticData

    classify_signatureTraps: (ut) ->
      ir = parse [==[
class Foo
  describe: (values = @values, pattern = ((key, value) -> "#{value} (#{key})"), join = true) =>
    join
  new: (@name, @f = -> , @testClass) =>
return Foo]==]
      cls = ir.classes[1]
      describeMember = memberByName cls, "describe"
      ut\assertEquals #describeMember.params, 3
      ut\assertEquals describeMember.params[2].name, "pattern"
      ut\assertTrue describeMember.params[2].hasDefault
      ctor = memberByName cls, "new"
      ut\assertEquals ctor.kind, MemberKind.Constructor
      ut\assertItemsEqual [p.name for p in *ctor.params], {"name", "f", "testClass"}

    classify_zeroArgNoParens: (ut) ->
      ir = parse "class Foo\n  @make = =>\n  peek: =>\nreturn Foo"
      cls = ir.classes[1]
      ut\assertEquals #memberByName(cls, "make").params, 0
      ut\assertEquals #memberByName(cls, "peek").params, 0

    classify_selfParamsStrippedAndVarargs: (ut) ->
      ir = parse "class Foo\n  go: (@a, @@b, c, ...) => c\nreturn Foo"
      params = memberByName(ir.classes[1], "go").params
      ut\assertItemsEqual [p.name for p in *params], {"a", "b", "c", "..."}
      ut\assertTrue params[4].isVararg

    classify_fieldParamUnderscoresDropped: (ut) ->
      -- an @field param's leading underscores mark the field private, not the argument
      ir = parse "class Foo\n  new: (@__resolver, @_opts, plain) =>\nreturn Foo"
      params = memberByName(ir.classes[1], "new").params
      ut\assertItemsEqual [p.name for p in *params], {"resolver", "opts", "plain"}

    classify_twoPerLineInstanceDefaults: (ut) ->
      ir = parse "class Foo\n  toFile: false, toWindow: true\nreturn Foo"
      cls = ir.classes[1]
      ut\assertEquals memberByName(cls, "toFile").kind, MemberKind.InstanceDefault
      ut\assertEquals memberByName(cls, "toFile").valueInfo.literalKind, "boolean"
      ut\assertEquals memberByName(cls, "toWindow").kind, MemberKind.InstanceDefault

    classify_accessorPropertyRecognized: (ut) ->
      ir = parse 'Accessors = require "l0.DependencyControl.Accessors"\nclass Foo\n  state: Accessors.property\n    get: => 1\nreturn Foo'
      ut\assertEquals memberByName(ir.classes[1], "state").kind, MemberKind.AccessorProperty

    classify_metamethodVsPrivate: (ut) ->
      ir = parse "class Foo\n  __tostring: => \"Foo\"\n  __helper: => 1\nreturn Foo"
      cls = ir.classes[1]
      ut\assertEquals memberByName(cls, "__tostring").kind, MemberKind.Metamethod
      helper = memberByName cls, "__helper"
      ut\assertEquals helper.kind, MemberKind.Method
      ut\assertTrue helper.isPrivate

    classify_classLocalsInvisible: (ut) ->
      ir = parse 'class Foo\n  msgs = {err: "%s"}\n  helper = (a) -> a\n  go: => 1\nreturn Foo'
      cls = ir.classes[1]
      ut\assertNil memberByName cls, "msgs"
      ut\assertNil memberByName cls, "helper"
      ut\assertEquals cls.localSymbols.helper.kind, SymbolKind.Function

    classify_computedKeyReported: (ut) ->
      ir, diagnostics = parse "class Foo\n  [someVar]: 1\n  go: => 1\nreturn Foo"
      ut\assertNil memberByName ir.classes[1], "someVar"
      found = false
      for finding in *diagnostics.findings
        found = true if finding.code == Diagnostics.FindingCode.ComputedKeySkipped
      ut\assertTrue found

    classify_classAnnotationOverridesTypeName: (ut) ->
      ir = parse "---@class RealName\nclass FileName\n  go: => 1\nreturn FileName"
      ut\assertEquals ir.classes[1].typeName, "RealName"
      ut\assertEquals ir.classes[1].name, "FileName"

    classify_privateFromBlockTag: (ut) ->
      ir = parse "class Foo\n  ---does things\n  ---@private\n  helper: => 1\nreturn Foo"
      ut\assertTrue memberByName(ir.classes[1], "helper").isPrivate

    -- ── export unwrapping ───────────────────────────────────────────────────

    export_returnClass: (ut) ->
      ir = parse "class Foo\n  go: => 1\nreturn Foo"
      ut\assertEquals ir.export.kind, ExportKind.Class
      ut\assertEquals ir.export.class.name, "Foo"

    export_implicitTrailingClass: (ut) ->
      ir = parse "class Foo\n  go: => 1"
      ut\assertEquals ir.export.kind, ExportKind.Class

    export_trailingAccessorsInstall: (ut) ->
      ir = parse 'Accessors = require "l0.DependencyControl.Accessors"\nclass Foo\n  go: => 1\nAccessors.install Foo'
      ut\assertEquals ir.export.kind, ExportKind.Class
      ut\assertEquals ir.export.name, "Foo"

    export_withTestExportsUnwrapsAndHidesExports: (ut) ->
      ir = parse 'UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"\nsecret = -> 1\nclass Foo\n  go: => 1\nreturn UnitTestSuite\\withTestExports Foo, {:secret}'
      ut\assertEquals ir.export.kind, ExportKind.Class
      ut\assertEquals ir.export.name, "Foo"
      -- the exports table must not surface anywhere in the export record
      ut\assertNil ir.export.fields

    export_registerUnwraps: (ut) ->
      ir = parse 'version = {}\nclass Foo\n  go: => 1\nreturn version\\register Foo'
      ut\assertEquals ir.export.kind, ExportKind.Class

    export_wholeFileTable: (ut) ->
      ir = parse '{\n  NAME: "depctrl"\n  COUNT: 42\n}'
      ut\assertEquals ir.export.kind, ExportKind.Table
      ut\assertEquals #ir.export.fields, 2

    export_returnedTableLiteralWithFieldBlocks: (ut) ->
      ir = parse '---field doc\n---@param url string\nopenUrl = (url) -> url\nreturn {open: openUrl}'
      ut\assertEquals ir.export.kind, ExportKind.Table
      ut\assertEquals ir.export.fields[1].name, "open"
      ut\assertEquals ir.export.fields[1].valueInfo.kind, ValueKind.Reference

    export_setmetatableTable: (ut) ->
      ir = parse 'wrapper = setmetatable {}, {}\nwrapper.encode = (value) -> value\nreturn wrapper'
      ut\assertEquals ir.export.kind, ExportKind.Table
      ut\assertEquals #ir.augmentations, 1
      ut\assertEquals ir.augmentations[1].name, "encode"

    export_returnFunction: (ut) ->
      ir = parse "---resolves hosts\n---@param host string\nresolveHost = (host) -> host\nreturn resolveHost"
      ut\assertEquals ir.export.kind, ExportKind.Function
      ut\assertEquals ir.export.name, "resolveHost"
      ut\assertEquals #ir.export.params, 1
      ut\assertNotNil ir.export.block

    export_unknownReported: (ut) ->
      ir, diagnostics = parse "x = 1\nprint x"
      ut\assertEquals ir.export.kind, ExportKind.Unknown
      found = false
      for finding in *diagnostics.findings
        found = true if finding.code == Diagnostics.FindingCode.NoExport
      ut\assertTrue found

    -- ── enum recognition ────────────────────────────────────────────────────

    enum_valuesAndTrailingArg: (ut) ->
      ir = parse 'Enum = require "l0.DependencyControl.Enum"\nclass Foo\n  @logger = {}\n  @LockState = Enum "LockState", {\n    Unknown: -1\n    Free: 0\n    Held: "held"\n  }, @logger\nreturn Foo'
      ut\assertEquals #ir.enums, 1
      enum = ir.enums[1]
      ut\assertEquals enum.name, "LockState"
      ut\assertEquals enum.exportedAs, "LockState"
      ut\assertEquals #enum.members, 3
      ut\assertEquals enum.members[1].literal, "-1"
      ut\assertEquals enum.members[3].literal, "\"held\""

    enum_localThenStaticReExport: (ut) ->
      ir = parse 'Enum = require "l0.DependencyControl.Enum"\nclass Foo\n  Mode = Enum "Mode", {Fast: "fast"}\n  @Mode = Mode\nreturn Foo'
      ut\assertEquals #ir.enums, 1
      ut\assertEquals ir.enums[1].exportedAs, "Mode"
      member = memberByName ir.classes[1], "Mode"
      ut\assertEquals member.kind, MemberKind.EnumExport

    enum_augmentationExport: (ut) ->
      ir = parse 'Enum = require "l0.DependencyControl.Enum"\nStatus = Enum "Status", {Ok: 0}\nclass Task\n  go: => 1\nTask.Status = Status\nreturn Task'
      ut\assertEquals ir.enums[1].exportedAs, "Status"
      ut\assertEquals #ir.augmentations, 1

    enum_computedMembersCounted: (ut) ->
      ir = parse 'Enum = require "l0.DependencyControl.Enum"\nkey = "K"\nclass Foo\n  @Map = Enum "Map", {[key]: "v", Plain: "p"}\nreturn Foo'
      ut\assertEquals #ir.enums[1].members, 1
      ut\assertEquals ir.enums[1].computedKeyCount, 1

    -- ── explicit-return detection ───────────────────────────────────────────

    valueReturn_topLevel: (ut) ->
      ir = parse "class Foo\n  go: => return 1\nreturn Foo"
      ut\assertTrue memberByName(ir.classes[1], "go").hasExplicitValueReturn

    valueReturn_insideIf: (ut) ->
      ir = parse "class Foo\n  go: (x) =>\n    if x\n      return 5\n    x\nreturn Foo"
      ut\assertTrue memberByName(ir.classes[1], "go").hasExplicitValueReturn

    valueReturn_implicitNotCounted: (ut) ->
      ir = parse "class Foo\n  go: (x) => x + 1\nreturn Foo"
      ut\assertFalse memberByName(ir.classes[1], "go").hasExplicitValueReturn

    valueReturn_nestedFunctionNotCounted: (ut) ->
      ir = parse "class Foo\n  go: =>\n    helper = -> return 5\n    helper\nreturn Foo"
      ut\assertFalse memberByName(ir.classes[1], "go").hasExplicitValueReturn

    -- ── parse failure ───────────────────────────────────────────────────────

    parse_syntaxErrorReturnsNil: (ut) ->
      parser = Parser!
      ir, err = parser\parse "class {{{", "test.bad"
      ut\assertNil ir
      ut\assertString err
  }
