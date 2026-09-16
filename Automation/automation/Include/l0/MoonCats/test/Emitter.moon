-- Emitter tests: definition structure, type-text extraction, overload/enum/field synthesis,
-- duplicate folding, and contract diagnostics — all on in-memory snippets.
() ->
  Parser = require "l0.MoonCats.Parser"
  Emitter = require "l0.MoonCats.Emitter"
  Diagnostics = require "l0.MoonCats.Diagnostics"
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  {:inferLiteralType, :moduleVarName} = UnitTestSuite\getTestExports Emitter
  FindingCode = Diagnostics.FindingCode

  -- parse + emit a snippet, asserting the definition is loadable Lua
  emit = (ut, source, requireId = "test.mod", packageSymbols) ->
    parser, emitter = Parser!, Emitter!
    diagnostics = Diagnostics!
    ir, err = parser\parse source, requireId, diagnostics
    ut\assertNotNil ir, err
    text = emitter\emit ir, packageSymbols, diagnostics
    ut\assertNotNil (loadstring or load)(text), "definition failed to load: #{text}"
    text, diagnostics

  hasFinding = (diagnostics, code) ->
    for finding in *diagnostics.findings
      return true if finding.code == code
    false

  {
    _description: "MoonCatsEmitter: module IR to LuaLS .d.lua definition text."

    -- ── definition skeleton ───────────────────────────────────────────────────────

    skeleton_metaHeaderAndReturn: (ut) ->
      text = emit ut, "class Foo\n  go: => 1\nreturn Foo", "l0.Test.Foo"
      ut\assertMatches text, "^%-%-%-@meta l0%.Test%.Foo\n"
      ut\assertMatches text, "\nreturn Foo\n$"
      ut\assertMatches text, "\nlocal Foo = {}\n"

    skeleton_methodsAndStatics: (ut) ->
      text = emit ut, "class Foo\n  ---does a thing\n  ---@param a string\n  go: (a) => a\n  ---@param b integer\n  @make = (b) -> b\n  ---@param c integer\n  @fat = (c) => c\nreturn Foo"
      ut\assertMatches text, "function Foo:go%(a%) end"
      ut\assertMatches text, "function Foo%.make%(b%) end"
      ut\assertMatches text, "function Foo:fat%(c%) end"

    skeleton_metamethodsSkipped: (ut) ->
      text = emit ut, "class Foo\n  __tostring: => \"x\"\n  go: => 1\nreturn Foo"
      ut\assertFalse text\match("__tostring") != nil

    -- ── type inference (tag parsing itself is covered by the annotations suite) ──

    inferLiteral_types: (ut) ->
      ut\assertEquals inferLiteralType("42", "number"), "integer"
      ut\assertEquals inferLiteralType("-1", "number"), "integer"
      ut\assertEquals inferLiteralType("1.5", "number"), "number"
      ut\assertEquals inferLiteralType("\"x\"", "string"), "string"

    moduleVar_sanitizesLeaf: (ut) ->
      ut\assertEquals moduleVarName("l0.DependencyControl.helpers.ffi-posix"), "ffi_posix"
      ut\assertEquals moduleVarName("l0.DependencyControl.Constants"), "Constants"

    -- ── constructor overload synthesis ──────────────────────────────────────

    overload_typedFromCtorParams: (ut) ->
      text = emit ut, "class Foo\n  ---Creates it.\n  ---@param name string The name.\n  ---@param opts? table Options.\n  new: (@name, opts = {}) =>\nreturn Foo"
      ut\assertMatches text, "%-%-%-@overload fun%(name: string, opts%?: table%): Foo"
      -- constructor prose folds into the class doc; no .new function is emitted
      ut\assertMatches text, "%-%-%-Creates it%."
      ut\assertFalse text\match("function Foo[%.:]new") != nil

    overload_funTypedParamParenthesized: (ut) ->
      -- a fun type's return list would swallow the comma before the next parameter
      text = emit ut, "class Foo\n  ---Creates it.\n  ---@param cb fun(x: string): boolean The callback.\n  ---@param name string\n  new: (cb, name) =>\nreturn Foo"
      ut\assertMatches text, "%-%-%-@overload fun%(cb: %(fun%(x: string%): boolean%), name: string%): Foo"

    overload_varargAndUndocumented: (ut) ->
      text = emit ut, "class Foo\n  ---@param a string\n  ---@param ... any extras\n  new: (a, ...) =>\nreturn Foo"
      ut\assertMatches text, "%-%-%-@overload fun%(a: string, %.%.%.: any%): Foo"

    overload_noCtorPlainClass: (ut) ->
      text = emit ut, "class Foo\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "%-%-%-@overload fun%(%): Foo"

    overload_noCtorSubclassFallsBack: (ut) ->
      text, diagnostics = emit ut, "class Base\n  new: =>\nclass Foo extends Base\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "%-%-%-@overload fun%(%.%.%.%): Foo"
      ut\assertTrue hasFinding diagnostics, FindingCode.CtorOverloadFallback

    -- ── class header ────────────────────────────────────────────────────────

    header_verbatimBlockKept: (ut) ->
      text = emit ut, "---A documented class.\n---@class Foo\n---@field bar string The bar.\nclass Foo\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "%-%-%-A documented class%.\n%-%-%-@class Foo\n%-%-%-@field bar string The bar%."

    header_parentReattachedWhenOmitted: (ut) ->
      text = emit ut, "class Base\n  new: =>\n---@class Foo\nclass Foo extends Base\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "%-%-%-@class Foo: Base"

    header_parentSynthesizedWithoutBlock: (ut) ->
      text = emit ut, "class Base\n  new: =>\nclass Foo extends Base\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "%-%-%-@class Foo: Base"

    header_instanceDefaultFieldsSynthesized: (ut) ->
      text = emit ut, "class Foo\n  toFile: false, maxSize: 200\n  name: \"x\"\nreturn Foo"
      ut\assertMatches text, "%-%-%-@field toFile boolean"
      ut\assertMatches text, "%-%-%-@field maxSize integer"
      ut\assertMatches text, "%-%-%-@field name string"

    header_declaredFieldNotDuplicated: (ut) ->
      text = emit ut, "---@class Foo\n---@field toFile boolean Whether to log to file.\nclass Foo\n  toFile: false\nreturn Foo"
      _, count = text\gsub "%-%-%-@field toFile", ""
      ut\assertEquals count, 1

    -- ── data members ────────────────────────────────────────────────────────

    data_fieldsSynthesizedWithInference: (ut) ->
      text = emit ut, "class Foo\n  @answer = 42\n  @label = \"x\"\n  @factor = 1.5\n  @flag = false\n  @lookup = {a: 1}\nreturn Foo"
      ut\assertMatches text, "%-%-%-@field answer integer"
      ut\assertMatches text, "%-%-%-@field label string"
      ut\assertMatches text, "%-%-%-@field factor number"
      ut\assertMatches text, "%-%-%-@field flag boolean"
      ut\assertMatches text, "%-%-%-@field lookup table"

    data_ownTypeBlockWins: (ut) ->
      text = emit ut, "class Foo\n  ---limits per crawl\n  ---@type table<string, integer>\n  @limits = {depth: 2}\nreturn Foo"
      -- the block's own @type and its prose merge into the synthesized field
      ut\assertMatches text, "%-%-%-@field limits table<string, integer> limits per crawl"

    data_functionContractBecomesFunction: (ut) ->
      text = emit ut, "equalsImpl = (a, b) -> a == b\nclass Foo\n  ---Compares deeply.\n  ---@param a any\n  ---@param b any\n  ---@return boolean equal\n  @equals = equalsImpl\nreturn Foo"
      ut\assertMatches text, "function Foo%.equals%(a, b%) end"

    data_classReExportResolvesAcrossModules: (ut) ->
      packageSymbols = {typeNameByRequireId: {["l0.Other.Common"]: "OtherCommon"}, aliases: {}}
      text = emit ut, 'Common = require "l0.Other.Common"\nclass Foo\n  @Common = Common\nreturn Foo', "test.mod", packageSymbols
      ut\assertMatches text, "%-%-%-@field Common OtherCommon"

    data_callResolvesToClassInstance: (ut) ->
      text = emit ut, "class Logger\n  new: =>\nclass Foo\n  @logger = Logger!\nreturn Foo"
      ut\assertMatches text, "%-%-%-@field logger Logger"

    data_unresolvedTypedAnyWithWarning: (ut) ->
      text, diagnostics = emit ut, "mystery = doSomething!\nclass Foo\n  @thing = mystery\nreturn Foo"
      ut\assertMatches text, "%-%-%-@field thing any"
      ut\assertTrue hasFinding diagnostics, FindingCode.UnresolvedReference

    data_emptyAndInterpolatedStringsTypedString: (ut) ->
      -- their token can't be reconstructed, but they are still strings, not any
      text, diagnostics = emit ut, 'x = "y"\nclass Foo\n  @blank = ""\n  @interpolated = "#{x}z"\nreturn Foo'
      ut\assertMatches text, "%-%-%-@field blank string"
      ut\assertMatches text, "%-%-%-@field interpolated string"
      ut\assertFalse hasFinding diagnostics, FindingCode.UnresolvedReference

    data_setmetatableFieldTypedTable: (ut) ->
      -- a field seeded with setmetatable is a table, not an unresolved call
      text, diagnostics = emit ut, "class Foo\n  @instances = setmetatable {}, {__mode: \"v\"}\nreturn Foo"
      ut\assertMatches text, "%-%-%-@field instances table"
      ut\assertFalse hasFinding diagnostics, FindingCode.UnresolvedReference

    data_privateSynthesizedForDunder: (ut) ->
      text = emit ut, "class Foo\n  @__instances = 42\nreturn Foo"
      ut\assertMatches text, "%-%-%-@field private __instances integer"

    -- ── enum synthesis ──────────────────────────────────────────────────────

    enum_aliasTypedFields: (ut) ->
      text = emit ut, 'Enum = require "l0.DependencyControl.Enum"\n---@alias Mode\n---| "fast" # Fast: quick\n---| "slow" # Slow: careful\nMode = Enum "Mode", {Fast: "fast", Slow: "slow"}\nclass Foo\n  @Mode = Mode\nreturn Foo'
      ut\assertMatches text, "%-%-%-@class ModeEnum: Enum\n%-%-%-@field Fast Mode\n%-%-%-@field Slow Mode"
      ut\assertMatches text, "%-%-%-@field Mode ModeEnum"

    enum_literalFallbackWithoutAlias: (ut) ->
      text = emit ut, 'Enum = require "l0.DependencyControl.Enum"\nclass Foo\n  @Status = Enum "Status", {Ok: 1, Failed: -1, Named: "n"}\nreturn Foo'
      ut\assertMatches text, "%-%-%-@field Ok 1"
      -- negative literals are not valid LuaCATS types; they fall back to integer
      ut\assertMatches text, "%-%-%-@field Failed integer"
      ut\assertMatches text, "%-%-%-@field Named \"n\""

    enum_unexportedNotSynthesized: (ut) ->
      text = emit ut, 'Enum = require "l0.DependencyControl.Enum"\nOp = Enum "Op", {Eq: "="}\nclass Foo\n  go: => 1\nreturn Foo'
      ut\assertFalse text\match("OpEnum") != nil

    enum_tableFieldExportsSynthesizedClass: (ut) ->
      -- a table field holding a module-local enum is an enum export: typed <Name>Enum, class synthesized
      text, diagnostics = emit ut, 'Enum = require "l0.DependencyControl.Enum"\nOp = Enum "Op", {Eq: "="}\n---@class Ops\nOps = {:Op}\nreturn Ops'
      ut\assertMatches text, "%-%-%-@field Op OpEnum"
      ut\assertMatches text, "%-%-%-@class OpEnum: Enum"
      ut\assertFalse hasFinding diagnostics, FindingCode.UnresolvedReference

    enum_computedKeysReported: (ut) ->
      _, diagnostics = emit ut, 'Enum = require "l0.DependencyControl.Enum"\nkey = "K"\nclass Foo\n  @Map = Enum "Map", {[key]: "v", Plain: "p"}\nreturn Foo'
      ut\assertTrue hasFinding diagnostics, FindingCode.ComputedKeySkipped

    enum_computedKeyResolvesViaReferencedEnum: (ut) ->
      -- a `[OtherEnum.Member]` key resolves to that member's value, keying the field by the literal
      src = 'Enum = require "l0.DependencyControl.Enum"\nKind = Enum "Kind", {A: "a", B: "b"}\nclass Foo\n  @Section = Enum "Section", {[Kind.A]: "macros", [Kind.B]: "modules"}\nreturn Foo'
      text, diagnostics = emit ut, src
      ut\assertMatches text, '%-%-%-@field %["a"%] "macros"'
      ut\assertMatches text, '%-%-%-@field %["b"%] "modules"'
      ut\assertFalse hasFinding diagnostics, FindingCode.ComputedKeySkipped

    enum_computedValueResolvesViaReferencedEnum: (ut) ->
      -- a value referencing another enum's member resolves to that member's literal
      src = 'Enum = require "l0.DependencyControl.Enum"\nBase = Enum "Base", {A: "a", B: "b"}\nclass Foo\n  @Derived = Enum "Derived", {X: Base.A, Y: Base.B}\nreturn Foo'
      text, diagnostics = emit ut, src
      ut\assertMatches text, '%-%-%-@field X "a"'
      ut\assertMatches text, '%-%-%-@field Y "b"'
      ut\assertFalse hasFinding diagnostics, FindingCode.ComputedKeySkipped

    enum_augmentationEmitsOntoClass: (ut) ->
      text = emit ut, 'Enum = require "l0.DependencyControl.Enum"\nStatus = Enum "Status", {Ok: 0}\nclass Task\n  go: => 1\nTask.Status = Status\nreturn Task'
      ut\assertMatches text, "%-%-%-@class StatusEnum: Enum"
      ut\assertMatches text, "%-%-%-@field Status StatusEnum"

    augment_classDunderPathSkipped: (ut) ->
      -- a `Class.__class.field =` path manipulates the runtime class object, not the type surface
      text = emit ut, 'class Foo\n  ---does\n  go: => 1\nFoo.__class.tag = 42\nreturn Foo'
      ut\assertFalse text\match("field tag") != nil

    -- ── duplicate members ───────────────────────────────────────────────────

    duplicate_documentedBeatsUndocumented: (ut) ->
      text = emit ut, "sleepImpl = (ms) -> ms\nclass Timer\n  ---Sleeps.\n  ---@param ms number\n  sleep: sleepImpl\n  @sleep = sleepImpl\nreturn Timer"
      _, count = text\gsub "Timer%.sleep", ""
      ut\assertEquals count, 1
      ut\assertMatches text, "function Timer%.sleep%(ms%) end"

    duplicate_documentedPairFoldsToOverload: (ut) ->
      text, diagnostics = emit ut, "class Ver\n  ---Packs this instance.\n  ---@return integer packed\n  toPacked: => 1\n  ---Packs any value.\n  ---@param value number|string\n  ---@return integer packed\n  @toPacked = (value) => 1\nreturn Ver"
      -- the richer static wins; the instance form survives as a self-typed overload
      ut\assertMatches text, "function Ver:toPacked%(value%) end"
      ut\assertMatches text, "%-%-%-@overload fun%(self: Ver%): integer"
      ut\assertTrue hasFinding diagnostics, FindingCode.DuplicateMemberFolded

    -- ── table, function, and multi-class modules ────────────────────────────

    module_tableWithoutAnnotations: (ut) ->
      text = emit ut, '{\n  NAME: "depctrl"\n  COUNT: 42\n}', "l0.Test.Constants"
      ut\assertMatches text, "%-%-%-@class l0%.Test%.Constants\n%-%-%-@field NAME string\n%-%-%-@field COUNT integer\nlocal Constants = {}"
      ut\assertMatches text, "return Constants\n$"

    module_tableLocalReferenceTypedTable: (ut) ->
      -- an export referencing a table-literal local resolves to table, not any
      text, diagnostics = emit ut, 'cfg = {a: 1, b: 2}\nreturn {:cfg}', "l0.Test.Config"
      ut\assertMatches text, "%-%-%-@field cfg table"
      ut\assertFalse hasFinding diagnostics, FindingCode.UnresolvedReference

    module_tableWithFunctionField: (ut) ->
      text = emit ut, 'return {\n  ---Builds a mode string.\n  ---@param user? string\n  ---@return string mode\n  getFileMode: (user = "") -> user\n}', "l0.Test.ffi-posix"
      ut\assertMatches text, "function ffi_posix%.getFileMode%(user%) end"

    module_functionExport: (ut) ->
      text = emit ut, "---Resolves.\n---@param host string\n---@return string resolved\nresolveHost = (host) -> host\nreturn resolveHost"
      ut\assertMatches text, "local function resolveHost%(host%) end"
      ut\assertMatches text, "return resolveHost\n$"

    module_multiClassEmitsBoth: (ut) ->
      text = emit ut, "class Download\n  go: => 1\nclass Downloader\n  run: => 1\nreturn Downloader"
      ut\assertMatches text, "local Download = {}"
      ut\assertMatches text, "local Downloader = {}"
      ut\assertMatches text, "return Downloader\n$"

    module_augmentedTableFunction: (ut) ->
      text = emit ut, 'wrapper = setmetatable {}, {}\n---Encodes a value.\n---@param value any\n---@return string json\nwrapper.encode = (value) -> "{}"\nwrapper.__depCtrlInit = (DepCtrl) -> nil\nreturn wrapper', "l0.Test.dkjson"
      ut\assertMatches text, "function wrapper%.encode%(value%) end"
      -- dunder augmentations get a synthesized @private
      ut\assertMatches text, "%-%-%-@private\nfunction wrapper%.__depCtrlInit%(DepCtrl%) end"

    -- ── tag passthrough ─────────────────────────────────────────────────────

    passthrough_genericAndDeprecated: (ut) ->
      text = emit ut, "class Foo\n  ---@deprecated Use other instead.\n  ---@param x string\n  old: (x) => x\n  ---@generic T\n  ---@param mod T\n  ---@return T mod\n  @wrap = (mod) -> mod\nreturn Foo"
      ut\assertMatches text, "%-%-%-@deprecated Use other instead%."
      ut\assertMatches text, "%-%-%-@generic T"

    -- ── contract diagnostics ────────────────────────────────────────────────

    check_missingDocOnPublicFunction: (ut) ->
      _, diagnostics = emit ut, "class Foo\n  go: (a) => a\nreturn Foo"
      ut\assertTrue hasFinding diagnostics, FindingCode.MissingDoc
      ut\assertTrue diagnostics\hasCheckFailures!

    check_privateExemptFromMissingDoc: (ut) ->
      _, diagnostics = emit ut, "class Foo\n  __helper: (a) => a\n  ---documented\n  go: => 1\nreturn Foo"
      ut\assertFalse hasFinding diagnostics, FindingCode.MissingDoc

    check_paramNameMismatch: (ut) ->
      _, diagnostics = emit ut, "class Foo\n  ---does\n  ---@param wrongName string\n  go: (a) => a\nreturn Foo"
      ut\assertTrue hasFinding diagnostics, FindingCode.ParamNameMismatch

    check_paramCountMismatch: (ut) ->
      _, diagnostics = emit ut, "class Foo\n  ---does\n  ---@param a string\n  go: (a, b) => a\nreturn Foo"
      ut\assertTrue hasFinding diagnostics, FindingCode.ParamCountMismatch

    check_missingReturn: (ut) ->
      _, diagnostics = emit ut, "class Foo\n  ---does\n  ---@param a string\n  go: (a) => return a\nreturn Foo"
      ut\assertTrue hasFinding diagnostics, FindingCode.MissingReturn

    check_cleanContractNoErrors: (ut) ->
      _, diagnostics = emit ut, "class Foo\n  ---does\n  ---@param a string\n  ---@return string a\n  go: (a) => return a\nreturn Foo"
      ut\assertFalse diagnostics\hasCheckFailures!
  }
