-- DocRenderer tests: page structure, both-form signatures, privacy, cross-links,
-- enum/alias tables, and site scaffolding — all on in-memory snippets.
() ->
  Parser = require "l0.MoonCats.Parser"
  DocRenderer = require "l0.MoonCats.DocRenderer"
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  {:receiverName, :linkifyType} = UnitTestSuite\getTestExports DocRenderer

  -- parse snippets and render; returns the result plus a page lookup by require id
  renderModules = (ut, modules, opts) ->
    parser = Parser!
    irs = {}
    packageSymbols = {typeNameByRequireId: {}, aliases: {}}
    for mod in *modules
      ir, err = parser\parse mod.source, mod.requireId
      ut\assertNotNil ir, err
      table.insert irs, ir
      if ir.export.kind == "class" and ir.export.class
        packageSymbols.typeNameByRequireId[ir.requireId] = ir.export.class.typeName
      for name in pairs ir.aliases
        packageSymbols.aliases[name] = true
    result = DocRenderer!\render irs, packageSymbols, opts
    pageFor = (requireId) ->
      for page in *result.pages
        return page.text if page.requireId == requireId or page.title == requireId
    result, pageFor

  render = (ut, source, requireId = "test.Mod", opts) ->
    result, pageFor = renderModules ut, {{:source, :requireId}}, opts
    (pageFor requireId), result

  {
    _description: "MoonCatsDocRenderer: module IRs to markdown API documentation."

    -- ── page skeleton ───────────────────────────────────────────────────────

    page_headerAndRequireSnippet: (ut) ->
      text = render ut, "class Foo\n  go: => 1\nreturn Foo", "l0.Test.Foo"
      ut\assertMatches text, "^# l0%.Test%.Foo\n"
      ut\assertMatches text, 'Foo = require "l0%.Test%.Foo"'
      ut\assertMatches text, 'local Foo = require%("l0%.Test%.Foo"%)'

    prose_indentedExampleFenced: (ut) ->
      -- a blank-separated indented run (the annotations' inline-example form) becomes a
      -- highlighted fence, dedented
      text = render ut, "---Filters things.\n---\n---    Filter!\\include \"x\"\n---    Filter!\\includeAll!\n---@class Filter\nclass Filter\n  go: => 1\nreturn Filter"
      ut\assertMatches text, "Filters things%.\n\n```moonscript\nFilter!\\include \"x\"\nFilter!\\includeAll!\n```"

    prose_hangingIndentStaysInline: (ut) ->
      -- an indented continuation directly under a paragraph line is prose, not code
      text = render ut, "class Foo\n  ---Compares deeply:\n  ---    tables must match at identical indexes\n  ---@param a any\n  go: (a) => a\nreturn Foo"
      ut\assertFalse text\match("```moonscript") != nil
      ut\assertMatches text, "Compares deeply:\n    tables must match at identical indexes"

    page_classProseRendered: (ut) ->
      text = render ut, "---A documented class.\n---Second prose line.\n---@class Foo\nclass Foo\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "## Foo <a name=\"Foo\"></a>\n\nA documented class%.\nSecond prose line%."

    -- a leading ---doc block detached from any declaration (above all code) documents the module itself
    page_moduleDocRendered: (ut) ->
      text = render ut, "---Overview line.\n---\n---Second paragraph.\n\nfoo = -> 1\nreturn {:foo}", "l0.Test.Mod"
      ut\assertMatches text, "Overview line%."
      ut\assertMatches text, "Second paragraph%."

    -- a leading ---block bound to the first declaration is that declaration's doc, not the module's
    page_boundLeadingDocIsNotModuleDoc: (ut) ->
      text = render ut, "---Makes one.\n---@param x integer\nmake = (x) -> x\nreturn {:make}", "l0.Test.Mod"
      ut\assertMatches text, "Makes one%." -- rendered as the function's doc
      -- and not duplicated as a module description above the Functions heading
      ut\assertMatches text, "```\n\n## Functions"

    -- ── signatures ──────────────────────────────────────────────────────────

    signature_constructorBothForms: (ut) ->
      text = render ut, "class Widget\n  ---Creates it.\n  ---@param name string The name.\n  new: (@name) =>\nreturn Widget"
      ut\assertMatches text, "widget = Widget name"
      ut\assertMatches text, "local widget = Widget%(name%)"
      -- ctor param table carries the description the definitions lose
      ut\assertMatches text, "| name | `string` | The name%. |"

    signature_instanceMethodBothForms: (ut) ->
      text = render ut, "class Widget\n  ---does\n  ---@param x integer\n  poke: (x) => x\nreturn Widget"
      ut\assertMatches text, "widget\\poke x"
      ut\assertMatches text, "widget:poke%(x%)"

    signature_zeroArgUsesBang: (ut) ->
      text = render ut, "class Widget\n  ---does\n  peek: => 1\nreturn Widget"
      ut\assertMatches text, "widget\\peek!"
      ut\assertMatches text, "widget:peek%(%)"

    signature_fatStaticOnClass: (ut) ->
      text = render ut, "class Widget\n  ---does\n  ---@param ns string\n  @find = (ns) => ns\nreturn Widget"
      ut\assertMatches text, "Widget\\find ns"
      ut\assertMatches text, "Widget:find%(ns%)"

    signature_thinStaticDotForm: (ut) ->
      text = render ut, "class Widget\n  ---does\n  ---@param ns string\n  @parse = (ns) -> ns\nreturn Widget"
      ut\assertMatches text, "Widget%.parse ns"
      ut\assertMatches text, "Widget%.parse%(ns%)"

    receiver_lowerCamelAndKeywordSafe: (ut) ->
      ut\assertEquals receiverName("UpdateFeed"), "updateFeed"
      ut\assertEquals receiverName("Timer"), "timer"
      -- a receiver that would be a keyword gets a prefix
      ut\assertEquals receiverName("Local"), "theLocal"

    -- ── member sections ─────────────────────────────────────────────────────

    sections_instanceVsClassMethods: (ut) ->
      text = render ut, "class Foo\n  ---inst\n  go: => 1\n  ---stat\n  @make = -> 1\nreturn Foo"
      instancePos = text\find "### Instance methods"
      classPos = text\find "### Class methods"
      ut\assertNotNil instancePos
      ut\assertNotNil classPos
      goPos = text\find "#### go"
      makePos = text\find "#### make"
      ut\assertTrue instancePos < goPos and goPos < classPos
      ut\assertTrue classPos < makePos

    returns_namedWithDescriptions: (ut) ->
      text = render ut, "class Foo\n  ---does\n  ---@return boolean ok True when it worked.\n  ---@return string? err Failure reason.\n  go: => true\nreturn Foo"
      ut\assertMatches text, "%*%*Returns:%*%*"
      ut\assertMatches text, "%- `ok` `boolean` — True when it worked%."
      ut\assertMatches text, "%- `err` `string`%? — Failure reason%."

    deprecated_blockquoteWithReason: (ut) ->
      text = render ut, "class Foo\n  ---old\n  ---@deprecated Use go2 instead.\n  ---@param a string\n  go: (a) => a\nreturn Foo"
      ut\assertMatches text, "> %*%*Deprecated%*%* — Use go2 instead%."

    params_optionalMarkerAndVararg: (ut) ->
      text = render ut, "class Foo\n  ---does\n  ---@param a? integer Count.\n  ---@param ... any Extras.\n  go: (a, ...) => a\nreturn Foo"
      ut\assertMatches text, "| a%? | `integer` | Count%. |"
      ut\assertMatches text, "| %.%.%. | `any` | Extras%. |"

    -- ── privacy ─────────────────────────────────────────────────────────────

    private_omittedByDefault: (ut) ->
      text = render ut, "class Foo\n  ---@private\n  __hidden: => 1\n  ---documented\n  go: => 1\nreturn Foo"
      ut\assertFalse text\match("__hidden") != nil

    private_badgedWhenIncluded: (ut) ->
      text = render ut, "class Foo\n  ---@private\n  __hidden: => 1\n  go: => 1\nreturn Foo", "test.Mod", {includePrivate: true}
      ut\assertMatches text, "#### __hidden 🔒"

    private_fieldsFiltered: (ut) ->
      text = render ut, "---@class Foo\n---@field name string Public field.\n---@field private __cache table Internal.\nclass Foo\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "| name | `string` | Public field%. |"
      ut\assertFalse text\match("__cache") != nil

    -- ── fields and data ─────────────────────────────────────────────────────

    fields_accessorPropertyFromClassBlock: (ut) ->
      text = render ut, 'Accessors = require "l0.DependencyControl.Accessors"\n---@class Foo\n---@field state integer Read-only view.\nclass Foo\n  state: Accessors.property\n    get: => 1\nreturn Foo'
      ut\assertMatches text, "### Fields"
      ut\assertMatches text, "| state | `integer` | Read%-only view%. |"

    fields_dataStaticsTyped: (ut) ->
      text = render ut, "class Logger\n  new: =>\nclass Foo\n  @logger = Logger!\n  @maxSize = 200\nreturn Foo"
      ut\assertMatches text, "| logger | %[Logger%]%(#Logger%) |"
      ut\assertMatches text, "| maxSize | `integer` |"

    fields_setmetatableTypedTable: (ut) ->
      text = render ut, "class Foo\n  @instances = setmetatable {}, {__mode: \"v\"}\nreturn Foo"
      ut\assertMatches text, "| instances | `table` |"

    -- ── enums and aliases ───────────────────────────────────────────────────

    enum_tableWithAliasDescriptions: (ut) ->
      text = render ut, 'Enum = require "l0.DependencyControl.Enum"\nclass Foo\n  ---@alias Mode\n  ---| "fast" # Fast: quick and shallow\n  ---| "slow" # Slow: careful\n  Mode = Enum "Mode", {Fast: "fast", Slow: "slow"}\n  @Mode = Mode\nreturn Foo'
      ut\assertMatches text, "### Enums"
      ut\assertMatches text, "#### Mode"
      ut\assertMatches text, "| Fast | `\"fast\"` | quick and shallow |"
      ut\assertMatches text, "| Slow | `\"slow\"` | careful |"

    enum_plainValuesWithoutAlias: (ut) ->
      text = render ut, 'Enum = require "l0.DependencyControl.Enum"\nclass Foo\n  @Status = Enum "Status", {Ok: 1, Failed: -1}\nreturn Foo'
      ut\assertMatches text, "| Ok | `1` |"
      ut\assertMatches text, "| Failed | `%-1` |"

    enum_computedKeyShowsSourceExpression: (ut) ->
      -- a key referencing another enum's member shows the source Enum.Member expression, value joined from the alias
      src = 'Enum = require "l0.DependencyControl.Enum"\nKind = Enum "Kind", {A: "a", B: "b"}\n---@alias Section\n---| "macros" # automation scripts\n---| "modules" # modules\nSection = Enum "Section", {[Kind.A]: "macros", [Kind.B]: "modules"}\n---@class Domain\nreturn {:Kind, :Section}'
      text = render ut, src
      ut\assertMatches text, "| Kind%.A | `\"macros\"` | automation scripts |"
      ut\assertMatches text, "| Kind%.B | `\"modules\"` | modules |"

    types_aliasVariantTable: (ut) ->
      text = render ut, "---Precision selector.\n---@alias Precision\n---| 'major' # Major: whole releases\n---| 'minor' # Minor: feature releases\n\nx = 1\n\nf = -> 1\nreturn {f: f}"
      ut\assertMatches text, "## Types"
      ut\assertMatches text, "### Precision"
      ut\assertMatches text, "Precision selector%."
      ut\assertMatches text, "| `'major'` | Major: whole releases |"

    types_annotationOnlyClassTable: (ut) ->
      text = render ut, "---Constructor arguments.\n---@class FooArgs\n---@field name string The name.\n---@field count? integer How many.\n\nclass Foo\n  go: => 1\nreturn Foo"
      ut\assertMatches text, "### FooArgs"
      ut\assertMatches text, "| name | `string` | The name%. |"
      ut\assertMatches text, "| count%? | `integer` | How many%. |"

    -- ── cross-links ─────────────────────────────────────────────────────────

    links_crossModuleTypeResolves: (ut) ->
      _, pageFor = renderModules ut, {
        {requireId: "l0.Test.Logger", source: "---@class Logger\nclass Logger\n  go: => 1\nreturn Logger"}
        {requireId: "l0.Test.Main", source: 'Logger = require "l0.Test.Logger"\nclass Main\n  ---does\n  ---@param logger Logger The logger.\n  run: (logger) => 1\nreturn Main'}
      }
      mainText = pageFor "l0.Test.Main"
      ut\assertMatches mainText, "%[Logger%]%(l0%.Test%.Logger%.md#Logger%)"

    links_unknownTypeStaysCode: (ut) ->
      -- the union pipe is escaped because linkified types land in table cells
      ut\assertEquals linkifyType("string|integer", {linkIndex: {}, currentPage: "x.md"}), "`string`\\|`integer`"

    links_complexTypeSingleSpan: (ut) ->
      rendered = linkifyType "fun(a: string): boolean", {linkIndex: {}, currentPage: "x.md"}
      ut\assertEquals rendered, "`fun(a: string): boolean`"

    -- ── duplicates ──────────────────────────────────────────────────────────

    duplicates_documentedWins: (ut) ->
      text = render ut, "sleepImpl = (ms) -> ms\nclass Timer\n  ---Sleeps.\n  ---@param ms number Milliseconds.\n  sleep: sleepImpl\n  @sleep = sleepImpl\nreturn Timer"
      _, count = text\gsub "#### sleep", ""
      ut\assertEquals count, 1
      ut\assertMatches text, "| ms | `number` | Milliseconds%. |"

    -- ── module shapes ───────────────────────────────────────────────────────

    module_tableWithFunctionsAndFields: (ut) ->
      text = render ut, '{\n  NAME: "depctrl"\n  ---Greets.\n  ---@param who string\n  greet: (who) -> who\n}', "l0.Test.Constants"
      ut\assertMatches text, "## Functions"
      ut\assertMatches text, "Constants%.greet who"
      ut\assertMatches text, "## Fields"
      ut\assertMatches text, "| NAME | `string` |"

    module_tableEnumFieldUnderEnumsSection: (ut) ->
      -- a table field holding a module-local enum renders under its own Enums section, like a class enum
      text = render ut, 'Enum = require "l0.DependencyControl.Enum"\nOp = Enum "Op", {Eq: "="}\nreturn {:Op}', "l0.Test.ops"
      ut\assertMatches text, "## Enums"
      ut\assertFalse text\match("| Op |") != nil

    module_enumAliasAndOwnClassNotUnderTypes: (ut) ->
      -- an exported enum's alias shows under Enums, not duplicated under Types, and the table's own
      -- @class is dropped from Types too — leaving no Types section here, while the value type still links
      src = 'Enum = require "l0.DependencyControl.Enum"\n---@alias Mode\n---| "a" # A: first\nMode = Enum "Mode", {A: "a"}\n---@class Ops\nOps = {:Mode}\nreturn Ops'
      text = render ut, src, "l0.Test.ops"
      ut\assertMatches text, "## Enums"
      ut\assertFalse text\match("## Types") != nil
      ut\assertMatches text, '<a name="Mode"></a>'

    module_ownTypeGetsAnchor: (ut) ->
      -- a table module's own declared type gets an anchor so a re-export elsewhere can link to it
      text = render ut, '---@class Ops\nOps = {NAME: "x"}\nreturn Ops', "l0.Test.ops"
      ut\assertMatches text, '<a name="Ops"></a>'

    module_functionExport: (ut) ->
      text = render ut, "---Resolves a host.\n---@param host string The host.\n---@return string resolved The resolution.\nresolveHost = (host) -> host\nreturn resolveHost", "l0.Test.resolve-host"
      ut\assertMatches text, "#### resolveHost"
      ut\assertMatches text, "resolveHost host"
      ut\assertMatches text, "resolveHost%(host%)"

    -- ── index and scaffolding ───────────────────────────────────────────────

    index_groupsByPackage: (ut) ->
      result = nil
      do
        parser = Parser!
        ir = parser\parse "class Foo\n  go: => 1\nreturn Foo", "l0.Test.Foo"
        result = DocRenderer!\render {ir}, nil, {
          siteName: "My API"
          packages: {
            "l0.Test": {name: "Test Package", version: "1.0.0", description: "A test.", modules: {"l0.Test.Foo"}}
          }
        }
      ut\assertMatches result.indexPage.text, "# My API"
      ut\assertMatches result.indexPage.text, "## Test Package `v1%.0%.0`"
      ut\assertMatches result.indexPage.text, "%- %[l0%.Test%.Foo%]%(l0%.Test%.Foo%.md%)"

    scaffold_mkdocsDefault: (ut) ->
      _, result = render ut, "class Foo\n  go: => 1\nreturn Foo", "l0.Test.Foo"
      ut\assertEquals result.pages[1].path, "docs/l0.Test.Foo.md" -- standalone site nests pages under docs/
      ut\assertEquals #result.scaffold, 1
      ut\assertEquals result.scaffold[1].path, "mkdocs.yml"
      ut\assertMatches result.scaffold[1].text, "site_name: API Documentation"
      ut\assertMatches result.scaffold[1].text, "%- l0%.Test%.Foo: l0%.Test%.Foo%.md"
      -- the mike version-provider block powers the published site's version picker
      ut\assertMatches result.scaffold[1].text, "provider: mike"

    scaffold_mdbook: (ut) ->
      _, result = render ut, "class Foo\n  go: => 1\nreturn Foo", "l0.Test.Foo", {site: "mdbook", siteName: "Book"}
      paths = [file.path for file in *result.scaffold]
      -- SUMMARY.md must live inside the source dir; book.toml beside it at the output root
      ut\assertItemsEqual paths, {"docs/SUMMARY.md", "book.toml"}
      for file in *result.scaffold
        if file.path == "docs/SUMMARY.md"
          ut\assertMatches file.text, "%- %[l0%.Test%.Foo%]%(l0%.Test%.Foo%.md%)"
        else
          ut\assertMatches file.text, 'title = "Book"'
          ut\assertMatches file.text, 'src = "docs"'

    -- "none" is the embeddable section: pages sit flat (to drop into a host site's docs dir) and the
    -- only scaffold file is a literate-nav SUMMARY.md
    scaffold_noneEmbedsFlatWithLiterateNav: (ut) ->
      _, result = render ut, "class Foo\n  go: => 1\nreturn Foo", "l0.Test.Foo", {site: "none"}
      ut\assertEquals result.pages[1].path, "l0.Test.Foo.md" -- flat, no docs/ prefix
      ut\assertEquals result.indexPage.path, "index.md"
      ut\assertEquals #result.scaffold, 1
      ut\assertEquals result.scaffold[1].path, "SUMMARY.md"
      ut\assertMatches result.scaffold[1].text, "%* %[Overview%]%(index%.md%)"
      ut\assertMatches result.scaffold[1].text, "%* %[l0%.Test%.Foo%]%(l0%.Test%.Foo%.md%)"

    -- Under a plain section header, the root module leads as "Overview" and the rest drop the
    -- namespace. Ungrouped leftovers stay fully qualified.
    scaffold_literateNavRootAsOverview: (ut) ->
      result = renderModules ut, {
        {source: "class Root\n  go: => 1\nreturn Root", requireId: "l0.Test"}
        {source: "class Foo\n  go: => 1\nreturn Foo", requireId: "l0.Test.Foo"}
        {source: "class Bar\n  go: => 1\nreturn Bar", requireId: "l0.Other.Bar"}
      }, {
        site: "none"
        packages: {"l0.Test": {name: "Test Package", modules: {"l0.Test", "l0.Test.Foo"}}}
      }
      summary = result.scaffold[1].text
      ut\assertMatches summary, "%* %[Overview%]%(l0%.Test%.md%)"
      ut\assertMatches summary, "%* %[Foo%]%(l0%.Test%.Foo%.md%)"
      ut\assertMatches summary, "%* %[l0%.Other%.Bar%]%(l0%.Other%.Bar%.md%)"

    -- In the embeddable "none" mode the require snippet and signatures render as linked MoonScript/Lua
    -- content tabs. Other modes keep one inline block.
    page_noneModeRendersLanguageTabs: (ut) ->
      tabbed = render ut, "class Foo\n  go: (x) => x\nreturn Foo", "l0.Test.Foo", {site: "none"}
      ut\assertMatches tabbed, '=== "MoonScript"'
      ut\assertMatches tabbed, '=== "Lua"'
      inline = render ut, "class Foo\n  go: (x) => x\nreturn Foo", "l0.Test.Foo", {site: "mkdocs"}
      ut\assertMatches inline, "%-%- MoonScript"
      ut\assertFalse inline\match('=== "MoonScript"') != nil
  }
