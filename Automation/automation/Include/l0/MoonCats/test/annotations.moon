-- annotations tests: LuaCATS tag-line parsing shared by the definition and doc emitters.
() ->
  {:extractTypeText, :parseParamTag, :parseReturnTag, :parseFieldTag, :parseAliasVariants,
    :collectBlockTags, :proseLines, :blockTagText, :guardFunType} = require "l0.MoonCats.annotations"

  {
    _description: "annotations: LuaCATS tag-line parsing helpers."

    -- ── extractTypeText ─────────────────────────────────────────────────────

    typeText_plainWithDescription: (ut) ->
      typeText, rest = extractTypeText "string The name to use."
      ut\assertEquals typeText, "string"
      ut\assertEquals rest, "The name to use."

    typeText_funWithReturnAndDescription: (ut) ->
      ut\assertEquals extractTypeText("fun(key: string, value: any): string A formatter."),
        "fun(key: string, value: any): string"

    typeText_funMultiReturnContinuation: (ut) ->
      ut\assertEquals extractTypeText("fun(value: any, valueType: string): table?, boolean? A checker."),
        "fun(value: any, valueType: string): table?, boolean?"

    typeText_inlineTable: (ut) ->
      ut\assertEquals extractTypeText("{ get?: fun(self: any): any, set?: fun(self: any, value: any) } The spec."),
        "{ get?: fun(self: any): any, set?: fun(self: any, value: any) }"

    typeText_genericTable: (ut) ->
      ut\assertEquals extractTypeText("table<string, [any, string]> Args by name."),
        "table<string, [any, string]>"

    -- ── param and return tags ───────────────────────────────────────────────

    paramTag_optionalDetection: (ut) ->
      tag = parseParamTag "---@param join? string|boolean Separator or flag."
      ut\assertEquals tag.name, "join"
      ut\assertTrue tag.optional
      ut\assertEquals tag.type, "string|boolean"
      ut\assertEquals tag.description, "Separator or flag."

    returnTag_namedWithDescription: (ut) ->
      tag = parseReturnTag "---@return boolean? valid True when the value is a member."
      ut\assertEquals tag.type, "boolean?"
      ut\assertEquals tag.name, "valid"
      ut\assertEquals tag.description, "True when the value is a member."

    returnTag_typeOnly: (ut) ->
      tag = parseReturnTag "---@return integer"
      ut\assertEquals tag.type, "integer"
      ut\assertNil tag.name
      ut\assertNil tag.description

    collectTags_paramsAndReturns: (ut) ->
      params, returns = collectBlockTags {
        "---Does a thing."
        "---@param a string The input."
        "---@return boolean ok Whether it worked."
        "---@return string? err Error message on failure."
      }
      ut\assertEquals #params, 1
      ut\assertEquals #returns, 2
      ut\assertEquals returns[2].name, "err"

    -- ── field tags ──────────────────────────────────────────────────────────

    fieldTag_plain: (ut) ->
      tag = parseFieldTag "---@field semanticVersion SemanticVersion The canonical store."
      ut\assertEquals tag.name, "semanticVersion"
      ut\assertNil tag.visibility
      ut\assertEquals tag.type, "SemanticVersion"
      ut\assertEquals tag.description, "The canonical store."

    fieldTag_privateVisibility: (ut) ->
      tag = parseFieldTag "---@field private __state table Internal cache."
      ut\assertEquals tag.name, "__state"
      ut\assertEquals tag.visibility, "private"
      ut\assertEquals tag.type, "table"

    fieldTag_optionalComplexTypeNoDescription: (ut) ->
      tag = parseFieldTag "---@field limits? table<string, integer>"
      ut\assertTrue tag.optional
      ut\assertEquals tag.type, "table<string, integer>"
      ut\assertNil tag.description

    -- ── alias variants ──────────────────────────────────────────────────────

    aliasVariants_stringValuesWithKeys: (ut) ->
      variants = parseAliasVariants {
        "---@alias Mode"
        '---| "fast" # Fast: quick and shallow'
        '---| "slow" # Slow: careful and deep'
      }
      ut\assertEquals #variants, 2
      ut\assertEquals variants[1].value, '"fast"'
      ut\assertEquals variants[1].key, "Fast"
      ut\assertEquals variants[1].description, "quick and shallow"

    aliasVariants_numericValues: (ut) ->
      variants = parseAliasVariants {"---| -1 # Unknown: state not read yet", "---| 0 # Free: no holder"}
      ut\assertEquals variants[1].value, "-1"
      ut\assertEquals variants[1].key, "Unknown"

    aliasVariants_plainWithoutComment: (ut) ->
      variants = parseAliasVariants {"---| 'major'", "---| 'minor'"}
      ut\assertEquals #variants, 2
      ut\assertEquals variants[1].value, "'major'"
      ut\assertNil variants[1].key
      ut\assertNil variants[1].description

    -- ── prose and generic tags ──────────────────────────────────────────────

    prose_extractsUntaggedLines: (ut) ->
      lines = proseLines {
        "---First paragraph line."
        "---Second line."
        "---@param a string"
        "---| \"x\" # variant line"
      }
      ut\assertEquals #lines, 2
      ut\assertEquals lines[1], "First paragraph line."

    prose_preservesIndentAndBlankSeparators: (ut) ->
      -- indentation and blank lines carry markdown structure (indented code blocks);
      -- an indented @ is example content, not a tag
      lines = proseLines {
        "---Intro."
        "---"
        "---    example \"@{fileName}\""
        "---@param a string"
      }
      ut\assertEquals #lines, 3
      ut\assertEquals lines[2], ""
      ut\assertEquals lines[3], "    example \"@{fileName}\""

    prose_trimsBoundaryBlanks: (ut) ->
      lines = proseLines {"---", "---Text.", "---"}
      ut\assertEquals #lines, 1
      ut\assertEquals lines[1], "Text."

    tagText_deprecatedReason: (ut) ->
      reason = blockTagText {"---does things", "---@deprecated Use save instead.", "---@param a string"}, "deprecated"
      ut\assertEquals reason, "Use save instead."

    tagText_bareTagYieldsEmpty: (ut) ->
      ut\assertEquals blockTagText({"---@private"}, "private"), ""
      ut\assertNil blockTagText {"---plain prose"}, "deprecated"

    -- ── guardFunType ────────────────────────────────────────────────────────

    guard_wrapsFunTypes: (ut) ->
      ut\assertEquals guardFunType("fun(x: string): boolean"), "(fun(x: string): boolean)"
      ut\assertEquals guardFunType("(fun(x: string): boolean)"), "(fun(x: string): boolean)"
      ut\assertEquals guardFunType("string|integer"), "string|integer"
  }
