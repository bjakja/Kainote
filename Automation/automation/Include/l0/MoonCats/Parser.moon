utils = require "l0.DependencyControl.utils"
Enum = require "l0.DependencyControl.Enum"
Diagnostics = require "l0.MoonCats.Diagnostics"
moonParse = require "moonscript.parse"
moonUtil = require "moonscript.util"

FindingCode = Diagnostics.FindingCode

---@alias MoonCatsMemberKind
---| "constructor" # Constructor: the class's `new`
---| "method" # Method: a fat-arrow instance method
---| "staticMethodFat" # StaticMethodFat: a class-level function called with the class as self
---| "staticMethodThin" # StaticMethodThin: a class-level plain function
---| "staticData" # StaticData: a class-level data field
---| "instanceDefault" # InstanceDefault: an instance-level default value declared in the class body
---| "accessorProperty" # AccessorProperty: a computed property declared via Accessors.property
---| "enumExport" # EnumExport: a class-level field holding a DependencyControl Enum
---| "metamethod" # Metamethod: a Lua metamethod
MemberKind = Enum "MoonCatsMemberKind", {
  Constructor: "constructor"
  Method: "method"
  StaticMethodFat: "staticMethodFat"
  StaticMethodThin: "staticMethodThin"
  StaticData: "staticData"
  InstanceDefault: "instanceDefault"
  AccessorProperty: "accessorProperty"
  EnumExport: "enumExport"
  Metamethod: "metamethod"
}

---@alias MoonCatsExportKind
---| "class" # Class: the module returns a class
---| "table" # Table: the module returns a plain table
---| "function" # Function: the module returns a bare function
---| "unknown" # Unknown: the module's export could not be determined
ExportKind = Enum "MoonCatsExportKind", {
  Class: "class"
  Table: "table"
  Function: "function"
  Unknown: "unknown"
}

---@alias MoonCatsSymbolKind
---| "require" # Require: a local bound to require "..."
---| "import" # Import: a name destructured or imported from a required module
---| "class" # Class: a local bound to a class statement
---| "enum" # Enum: a local bound to a DependencyControl Enum definition
---| "function" # Function: a local bound to a function literal
---| "table" # Table: a local bound to a table literal
---| "literal" # Literal: a local bound to a scalar literal
---| "reference" # Reference: a local aliasing another local
---| "other" # Other: any binding the parser does not model
SymbolKind = Enum "MoonCatsSymbolKind", {
  Require: "require"
  Import: "import"
  Class: "class"
  Enum: "enum"
  Function: "function"
  Table: "table"
  Literal: "literal"
  Reference: "reference"
  Other: "other"
}

---@alias MoonCatsSegmentKind
---| "alias" # Alias: an @alias declaration with its variant lines
---| "class" # Class: an annotation-only @class declaration with its @field lines
---| "rest" # Rest: the member contract part of a block (params, returns, prose)
SegmentKind = Enum "MoonCatsSegmentKind", {
  Alias: "alias"
  Class: "class"
  Rest: "rest"
}

---@alias MoonCatsValueKind
---| "literal" # Literal: a scalar literal with a reconstructible source token
---| "reference" # Reference: a bare identifier
---| "call" # Call: a call chain (constructor or function invocation)
---| "table" # Table: a table literal
---| "function" # Function: a function literal
---| "expression" # Expression: an operator expression
---| "other" # Other: anything else
ValueKind = Enum "MoonCatsValueKind", {
  Literal: "literal"
  Reference: "reference"
  Call: "call"
  Table: "table"
  Function: "function"
  Expression: "expression"
  Other: "other"
}

metamethodNames = utils.makeSet {
  "__tostring", "__eq", "__lt", "__le", "__call", "__index", "__newindex",
  "__pairs", "__ipairs", "__gc", "__len", "__concat", "__add", "__sub",
  "__mul", "__div", "__mod", "__pow", "__unm", "__metatable", "__mode", "__close"
}

---A run of contiguous doc-comment lines.
---@class MoonCatsAnnotationBlock
---@field startLine integer 1-based line of the block's first doc line.
---@field endLine integer 1-based line of the block's last doc line.
---@field lines string[] The doc lines, verbatim from the leading `---` on.

---A slice of an annotation block — a standalone alias/class declaration or the member contract.
---@class MoonCatsSegment
---@field kind MoonCatsSegmentKind
---@field name? string Declared alias/class name; nil for rest segments.
---@field lines string[] The segment's doc lines, verbatim.
---@field line? integer Source line the segment is anchored to, for output ordering.

---One parameter of a function signature.
---@class MoonCatsParam
---@field name string Parameter name with any `@`/`@@` prefix stripped.
---@field hasDefault boolean Whether the signature declares a default value.
---@field isVararg boolean Whether this is the `...` parameter.

---Classification of a member's or field's value expression.
---@class MoonCatsValueInfo
---@field kind MoonCatsValueKind
---@field token? string Reconstructed source token for literal values.
---@field literalKind? string Lua type name of a literal value ("number", "string", "boolean", "nil").
---@field refName? string Identifier name for reference values.
---@field baseName? string Chain base identifier for call values.
---@field params? MoonCatsParam[] Signature for function values.
---@field arrow? string "fat" or "slim" for function values.
---@field node? table The raw AST node for table/call/expression/other values.

---A class-body member.
---@class MoonCatsMemberIR
---@field kind MoonCatsMemberKind
---@field name string
---@field line integer Source line of the member's definition.
---@field block? string[] The member's bound annotation lines (rest segment), verbatim.
---@field params? MoonCatsParam[] Signature for function members.
---@field arrow? string "fat" or "slim" for function members.
---@field hasExplicitValueReturn? boolean Whether the body contains an explicit valued return.
---@field isPrivate boolean From a `__` name prefix or an @private tag in the block.
---@field valueInfo? MoonCatsValueInfo For data members.
---@field enum? MoonCatsEnumIR For enum-export members.

---A DependencyControl Enum definition found in the module.
---@class MoonCatsEnumIR
---@field name string The Enum's declared runtime name.
---@field line integer
---@field members {key: string, literal: string, display?: string}[] Literal-valued members in definition order; display holds a computed key's source `Enum.Member` expression.
---@field computedKeyCount integer Number of members skipped for non-literal keys or values.
---@field exportedAs? string Member/field name the Enum is exported under, when it is.

---A real class found in the module.
---@class MoonCatsClassIR
---@field name string The moon class statement's name.
---@field line integer
---@field hasParent boolean
---@field parentName? string The extends expression's identifier when it is a simple reference.
---@field block? string[] The class's bound annotation block, verbatim (prose + @class + @field lines).
---@field typeName string LuaCATS type name: the block's @class name when present, else the moon name.
---@field annotatedParentName? string Parent named in the block's @class line, when present.
---@field members MoonCatsMemberIR[] In source order.
---@field localSymbols table<string, MoonCatsSymbol> Class-body locals.

---A module-level dotted assignment augmenting a class or module table.
---@class MoonCatsAugmentationIR
---@field targetName string Identifier being augmented.
---@field name string Field name assigned.
---@field line integer
---@field block? string[]
---@field valueInfo MoonCatsValueInfo
---@field params? MoonCatsParam[] Signature when the value is a function literal.
---@field arrow? string
---@field hasExplicitValueReturn? boolean

---A field of a table-module export.
---@class MoonCatsFieldIR
---@field name string
---@field line integer
---@field block? string[]
---@field valueInfo MoonCatsValueInfo
---@field enum? MoonCatsEnumIR Set when the field holds an Enum, mirroring a class-static enum export.
---@field params? MoonCatsParam[] Signature when the value is a function literal.
---@field arrow? string
---@field hasExplicitValueReturn? boolean

---What a module returns to requirers.
---@class MoonCatsExportIR
---@field kind MoonCatsExportKind
---@field name? string The exported local/class name, when the export is a named value.
---@field class? MoonCatsClassIR For class exports.
---@field fields? MoonCatsFieldIR[] For table exports.
---@field params? MoonCatsParam[] For function exports.
---@field block? string[] For function exports: the function's bound annotation lines.
---@field hasExplicitValueReturn? boolean For function exports.

---A resolved module-scope or class-scope name binding.
---@class MoonCatsSymbol
---@field kind MoonCatsSymbolKind
---@field requireId? string For require/import symbols.
---@field enum? MoonCatsEnumIR For enum symbols.
---@field params? MoonCatsParam[] For function symbols.
---@field arrow? string For function symbols.
---@field hasExplicitValueReturn? boolean For function symbols.
---@field node? table Table node for table symbols.
---@field target? string For reference symbols.
---@field token? string For literal symbols.
---@field literalKind? string For literal symbols.
---@field block? string[] Annotation lines bound to the symbol's definition.
---@field class? MoonCatsClassIR For class symbols.

---Everything the emitter needs to know about one parsed module.
---@class MoonCatsModuleIR
---@field requireId string
---@field lines string[] The module's source lines.
---@field segments MoonCatsSegment[] Standalone alias/class declarations, in source order.
---@field classes MoonCatsClassIR[] Real classes, in source order.
---@field classesByName table<string, MoonCatsClassIR>
---@field enums MoonCatsEnumIR[] All Enum definitions found.
---@field aliases table<string, boolean> Names of @alias declarations found in this module.
---@field symbols table<string, MoonCatsSymbol> Module-scope name bindings.
---@field export MoonCatsExportIR
---@field augmentations MoonCatsAugmentationIR[] Module-level dotted assignments.
---@field moduleDoc? string[] A leading doc block detached from any declaration, documenting the module itself.

---Returns a node's AST type tag, or "value" for scalars.
---@param node any
---@return string tag
nodeType = (node) ->
  return "value" unless "table" == type node
  node[1]

---Splits source text into lines with trailing carriage returns stripped.
---@param source string
---@return string[] lines
splitLines = (source) ->
  lines = {}
  pos = 1
  while true
    nl = source\find "\n", pos, true
    unless nl
      last = source\sub(pos)\gsub "\r$", ""
      table.insert lines, last
      break
    line = source\sub(pos, nl - 1)\gsub "\r$", ""
    table.insert lines, line
    pos = nl + 1
  lines

---Returns the smallest source position attached to a node or any of its descendants.
---@param node any
---@return integer? pos Nil when the node is not a table or carries no positions.
minDescendantPos = (node) ->
  return nil unless "table" == type node
  best = node[-1]
  for _, child in pairs node
    if "table" == type child
      pos = minDescendantPos child
      best = pos if pos and (not best or pos < best)
  best

---Returns the 1-based source line a node is defined on, falling back to its earliest descendant.
---@param node any
---@param source string
---@return integer? line Nil when no position can be resolved from the node.
nodeLine = (node, source) ->
  return nil unless "table" == type node
  pos = node[-1] or minDescendantPos node
  pos and moonUtil.pos_to_line source, pos

---Marks every line whose content lies inside a string literal, so the comment scanner skips them.
---@param tree table Parsed AST root.
---@param source string
---@return table<integer, boolean> mask
computeStringMask = (tree, source) ->
  mask = {}

  maskString = (node) ->
    startPos = node[-1]
    return unless startPos
    startLine = moonUtil.pos_to_line source, startPos
    delim = node[2]
    endLine = startLine
    if "string" == type(delim) and delim\sub(1, 1) == "["
      level = #(delim\match("^%[(=*)%[") or "")
      closer = "]" .. ("=")\rep(level) .. "]"
      closePos = source\find closer, startPos + #delim, true
      endLine = moonUtil.pos_to_line source, closePos if closePos
    else
      newlines = 0
      for i = 3, #node
        chunk = node[i]
        if "string" == type chunk
          newlines += select 2, chunk\gsub("\n", "")
      endLine = startLine + newlines
    for l = startLine, endLine
      mask[l] = true

  walk = (node) ->
    return unless "table" == type node
    maskString node if node[1] == "string"
    for _, child in pairs node
      walk child if "table" == type child

  walk tree
  mask

---Collects maximal runs of doc-comment lines outside string literals.
---A plain -- line (or a 4+-dash divider) terminates a block and is never part of one.
---@param lines string[]
---@param mask table<integer, boolean> String-interior lines to skip.
---@return MoonCatsAnnotationBlock[] blocks In source order.
scanCommentBlocks = (lines, mask) ->
  blocks = {}
  current = nil
  for i, line in ipairs lines
    docText = not mask[i] and line\match "^%s*(%-%-%-.*)$"
    docText = nil if docText and docText\match "^%-%-%-%-"
    if docText
      current or= {startLine: i, lines: {}}
      table.insert current.lines, docText
      current.endLine = i
    elseif current
      table.insert blocks, current
      current = nil
  table.insert blocks, current if current
  blocks

---Splits a block into standalone @alias/@class segments and the remaining member contract.
---Prose directly above an @alias/@class line belongs to that segment; prose after variant
---lines falls through to the next segment, so a member doc following an alias stays with the member.
---@param block MoonCatsAnnotationBlock
---@return MoonCatsSegment[] segments In block order, with at most one rest segment.
segmentBlock = (block) ->
  segments = {}
  pendingProse = {}
  current, rest = nil, nil
  lastContext = nil

  flushCurrent = ->
    if current
      table.insert segments, current
      current = nil

  appendRest = (line) ->
    unless rest
      rest = {kind: SegmentKind.Rest, lines: {}}
      table.insert segments, rest
    for prose in *pendingProse
      table.insert rest.lines, prose
    pendingProse = {}
    table.insert rest.lines, line if line

  for line in *block.lines
    tag = line\match "^%-%-%-@(%w+)"
    isVariant = not tag and line\match("^%-%-%-|") != nil
    if tag == "alias" or tag == "class"
      flushCurrent!
      name = line\match("^%-%-%-@%w+%s+%(%w+%)%s+([%w_%.]+)") or line\match "^%-%-%-@%w+%s+([%w_%.]+)"
      current = {kind: tag == "alias" and SegmentKind.Alias or SegmentKind.Class, :name, lines: {}}
      for prose in *pendingProse
        table.insert current.lines, prose
      pendingProse = {}
      table.insert current.lines, line
      lastContext = tag
    elseif isVariant
      if current and current.kind == SegmentKind.Alias
        table.insert current.lines, line
      else
        appendRest line
      lastContext = "variant"
    elseif tag == "field" or tag == "operator"
      if current and current.kind == SegmentKind.Class
        table.insert current.lines, line
      else
        appendRest line
      lastContext = tag
    elseif tag
      flushCurrent!
      appendRest line
      lastContext = "rest"
    else
      -- prose with no tag continues the preceding tag line, else it leads the next declaration
      if current and (lastContext == "alias" or lastContext == "class" or lastContext == "field" or lastContext == "operator")
        table.insert current.lines, line
      elseif rest and lastContext == "rest"
        table.insert rest.lines, line
      else
        table.insert pendingProse, line
  flushCurrent!
  appendRest nil if #pendingProse > 0
  segments

---Extracts the declared type name and parent from a class annotation block.
---@param blockLines string[]
---@return string? name The @class type name, or nil when the block has no @class line.
---@return string? parent The parent type from a `Name: Parent` line, or nil when unparented.
classAnnotationInfo = (blockLines) ->
  for line in *blockLines
    name, parent = line\match "^%-%-%-@class%s+([%w_%.]+)%s*:%s*([%w_%.]+)"
    name or= line\match "^%-%-%-@class%s+([%w_%.]+)"
    return name, parent if name
  nil

---Reports whether an annotation block contains an @private tag.
---@param blockLines? string[]
---@return boolean private
blockContainsPrivate = (blockLines) ->
  return false unless blockLines
  for line in *blockLines
    return true if line\match "^%-%-%-@private"
  false

---Converts an fndef's argument list into parameter records.
---@param args table The fndef node's argument tuples.
---@return MoonCatsParam[] params Caller-facing parameters; the leading underscores of an
---`@field`/`@@field` argument (a field-visibility marker on the field, not the argument) are dropped.
paramsFromFndef = (args) ->
  return {} unless "table" == type args
  params = {}
  for tuple in *args
    nameNode = tuple[1]
    param = if "table" == type nameNode
      {name: (nameNode[2]\gsub "^_+", ""), hasDefault: tuple[2] != nil, isVararg: false}
    elseif nameNode == "..."
      {name: "...", hasDefault: false, isVararg: true}
    else
      {name: nameNode, hasDefault: tuple[2] != nil, isVararg: false}
    table.insert params, param
  params

---Reconstructs the source token of a scalar literal node.
---@param node any
---@return string? token Reconstructed literal, or nil when the node is not a scalar literal.
---@return string? literalKind Lua type name of the literal, nil in the same non-literal case.
literalTokenFromNode = (node) ->
  switch nodeType node
    when "number"
      node[2], "number"
    when "string"
      -- only plain single-chunk strings reconstruct; interpolated ones don't
      if #node == 3 and "string" == type node[3]
        "%q"\format(node[3]), "string"
    when "ref"
      switch node[2]
        when "true" then "true", "boolean"
        when "false" then "false", "boolean"
        when "nil" then "nil", "nil"
    when "minus"
      token, kind = literalTokenFromNode node[2]
      if token and kind == "number"
        "-" .. token, "number"

---Returns the content of a plain, non-interpolated string node.
---@param node any
---@return string? content Nil when the node is not a plain single-chunk string.
plainStringFromNode = (node) ->
  return nil unless nodeType(node) == "string"
  return nil unless #node == 3 and "string" == type node[3]
  node[3]

---Extracts the require identifier from a require "..." chain.
---@param node any
---@return string? requireId Nil when the chain is not a `require "..."` call.
requireIdFromChain = (node) ->
  return nil unless nodeType(node) == "chain"
  base = node[2]
  return nil unless nodeType(base) == "ref" and base[2] == "require"
  callItem = node[3]
  return nil unless "table" == type(callItem) and callItem[1] == "call" and node[4] == nil
  plainStringFromNode callItem[2][1]

---Reduces a chain node to a flat summary for pattern-matching.
---@param node any
---@return {baseName?: string, accessor?: string, accessorKind?: string, callArgs?: table}? info Base identifier, last dot/colon accessor, and last call arguments; nil when the node is not a chain.
describeChain = (node) ->
  return nil unless nodeType(node) == "chain"
  base = node[2]
  baseName = nodeType(base) == "ref" and base[2] or nil
  accessor, accessorKind, callArgs = nil, nil, nil
  for i = 3, #node
    item = node[i]
    continue unless "table" == type item
    switch item[1]
      when "dot", "colon"
        accessor, accessorKind = item[2], item[1]
      when "call"
        callArgs = item[2]
  {:baseName, :accessor, :accessorKind, :callArgs}

---Reports whether an fndef body contains an explicit return with values, without descending
---into nested function literals.
---@param fndefNode table
---@return boolean returns
fndefHasValueReturn = (fndefNode) ->
  found = false
  search = (node) ->
    return if found or "table" != type node
    return if node[1] == "fndef"
    if node[1] == "return"
      expList = node[2]
      found = true if "table" == type(expList) and expList[2] != nil
      return
    for _, child in pairs node
      search child if "table" == type child
  body = fndefNode[5]
  return false unless "table" == type body
  for stm in *body
    search stm
  found

---Resolves an `SomeEnum.Member` reference, in key or value position, against an already-parsed enum.
---@param node any
---@param resolveSymbol fun(name: string): MoonCatsSymbol?
---@return string? literal The referenced member's literal token.
---@return string? expr The source `Enum.Member` expression, for display.
resolveEnumMemberRef = (node, resolveSymbol) ->
  info = describeChain node
  return nil unless info and info.baseName and info.accessor and info.accessorKind == "dot"
  sym = resolveSymbol info.baseName
  return nil unless sym and sym.kind == SymbolKind.Enum and sym.enum
  for member in *sym.enum.members
    return member.literal, "#{info.baseName}.#{info.accessor}" if member.key == info.accessor
  nil

---Recognizes an `Enum "Name", {...}` definition chain.
---@param node any
---@param resolveSymbol fun(name: string): MoonCatsSymbol?
---@return {name: string, members: table[], computedKeyCount: integer}? spec Nil when the chain is not an Enum definition.
enumSpecFromChain = (node, resolveSymbol) ->
  info = describeChain node
  return nil unless info and info.baseName and not info.accessor and info.callArgs
  sym = resolveSymbol info.baseName
  return nil unless sym and sym.kind == SymbolKind.Require and sym.requireId\match "%.Enum$"
  name = plainStringFromNode info.callArgs[1]
  tableNode = info.callArgs[2]
  return nil unless name and nodeType(tableNode) == "table"
  members, computedKeyCount = {}, 0
  for pair in *tableNode[2]
    key, value = pair[1], pair[2]
    -- a value referencing another enum's member carries that member's literal
    valueToken = value != nil and (literalTokenFromNode(value) or resolveEnumMemberRef(value, resolveSymbol))
    if not valueToken
      computedKeyCount += 1
    elseif nodeType(key) == "key_literal"
      table.insert members, {key: key[2], literal: valueToken}
    else
      -- a `[SomeEnum.Member]` key is that member's value at runtime, so key the field by the resolved literal
      keyToken, keyExpr = resolveEnumMemberRef key, resolveSymbol
      if keyToken
        table.insert members, {key: "[#{keyToken}]", display: keyExpr, literal: valueToken}
      else
        computedKeyCount += 1
  {:name, :members, :computedKeyCount}

---Recognizes an `Accessors.property {...}` computed-property value.
---@param node any
---@param resolveSymbol fun(name: string): MoonCatsSymbol?
---@return boolean isProperty
isAccessorsProperty = (node, resolveSymbol) ->
  info = describeChain node
  return false unless info and info.baseName and info.accessor == "property" and info.accessorKind == "dot"
  sym = resolveSymbol info.baseName
  (sym and sym.kind == SymbolKind.Require and sym.requireId\match "%.Accessors$") != nil

---Classifies a member's or field's value expression.
---@param value any
---@return MoonCatsValueInfo info
valueInfoFromNode = (value) ->
  return {kind: ValueKind.Other} unless "table" == type value
  token, literalKind = literalTokenFromNode value
  return {kind: ValueKind.Literal, :token, :literalKind} if token
  switch nodeType value
    when "string"
      -- an empty or interpolated string still types as string, even when its token can't be reconstructed
      {kind: ValueKind.Literal, literalKind: "string"}
    when "ref"
      {kind: ValueKind.Reference, refName: value[2]}
    when "table"
      {kind: ValueKind.Table, node: value}
    when "chain"
      -- only an actual invocation types as its base (a constructor/factory result);
      -- a bare dotted access is a field alias whose type the base name doesn't carry
      info = describeChain value
      hasCall = info and info.callArgs != nil
      {kind: ValueKind.Call, baseName: hasCall and info.baseName or nil, node: value}
    when "fndef"
      {kind: ValueKind.Function, params: paramsFromFndef(value[2]), arrow: value[4], node: value}
    when "exp"
      {kind: ValueKind.Expression, node: value}
    else
      {kind: ValueKind.Other, node: value}

---Records a finding when a diagnostics collection is attached to the parse.
---@param ctx table
---@param code MoonCatsFindingCode
---@param line? integer
---@param ... any Message format values.
report = (ctx, code, line, ...) ->
  ctx.diagnostics\add code, ctx.requireId, line, ... if ctx.diagnostics

---Registers the item a given source line defines; the first item on a line wins.
---@param ctx table
---@param line? integer
---@param item table
registerLineItem = (ctx, line, item) ->
  return unless line
  ctx.lineItems[line] or= item

---Creates an EnumIR from a recognized Enum definition and registers it.
---@param ctx table
---@param spec table The recognized definition (name, members, computedKeyCount).
---@param line integer
---@return MoonCatsEnumIR enum
recordEnum = (ctx, spec, line) ->
  enum = {name: spec.name, members: spec.members, computedKeyCount: spec.computedKeyCount, :line}
  table.insert ctx.enums, enum
  enum

---Builds a symbol record from a local's value expression, registering any Enum definition found.
---@param value any
---@param ctx table
---@param line? integer
---@return MoonCatsSymbol symbol
symbolFromValue = (value, ctx, line) ->
  return {kind: SymbolKind.Other} unless "table" == type value
  token, literalKind = literalTokenFromNode value
  return {kind: SymbolKind.Literal, :token, :literalKind} if token
  requireId = requireIdFromChain value
  return {kind: SymbolKind.Require, :requireId} if requireId
  spec = enumSpecFromChain value, (name) -> ctx.symbols[name]
  if spec
    return {kind: SymbolKind.Enum, enum: recordEnum ctx, spec, line or 0}
  switch nodeType value
    when "fndef"
      {
        kind: SymbolKind.Function
        params: paramsFromFndef value[2]
        arrow: value[4]
        hasExplicitValueReturn: fndefHasValueReturn value
      }
    when "table"
      {kind: SymbolKind.Table, node: value}
    when "ref"
      {kind: SymbolKind.Reference, target: value[2]}
    when "chain"
      -- setmetatable({...}, ...) modules expose the table argument's fields
      info = describeChain value
      if info and info.baseName == "setmetatable" and info.callArgs and nodeType(info.callArgs[1]) == "table"
        {kind: SymbolKind.Table, node: info.callArgs[1]}
      else
        {kind: SymbolKind.Other}
    else
      {kind: SymbolKind.Other}

---Classifies one class-body member declared as a props key/value pair.
---@param name string
---@param keyType string "key_literal", "self", or "self_class".
---@param value any
---@param line integer
---@param resolveSymbol fun(name: string): MoonCatsSymbol?
---@param ctx table
---@return MoonCatsMemberIR member
classifyPropsMember = (name, keyType, value, line, resolveSymbol, ctx) ->
  member = {:name, :line, isPrivate: name\match("^__") != nil}
  if nodeType(value) == "fndef"
    member.params = paramsFromFndef value[2]
    member.arrow = value[4]
    member.hasExplicitValueReturn = fndefHasValueReturn value
    member.kind = if name == "new"
      MemberKind.Constructor
    elseif metamethodNames[name]
      member.isPrivate = false
      MemberKind.Metamethod
    elseif member.arrow == "slim"
      MemberKind.StaticMethodThin
    elseif keyType == "self" or keyType == "self_class"
      MemberKind.StaticMethodFat
    else
      MemberKind.Method
    return member
  if isAccessorsProperty value, resolveSymbol
    member.kind = MemberKind.AccessorProperty
    return member
  spec = enumSpecFromChain value, resolveSymbol
  if spec
    enum = recordEnum ctx, spec, line
    enum.exportedAs = name
    member.kind = MemberKind.EnumExport
    member.enum = enum
    return member
  member.valueInfo = valueInfoFromNode value
  if member.valueInfo.kind == ValueKind.Reference
    sym = resolveSymbol member.valueInfo.refName
    if sym and sym.kind == SymbolKind.Enum
      sym.enum.exportedAs or= name
      member.kind = MemberKind.EnumExport
      member.enum = sym.enum
      return member
  member.kind = keyType == "key_literal" and MemberKind.InstanceDefault or MemberKind.StaticData
  member

---Classifies one class-body static declared as an assignment to @name.
---@param name string
---@param value any
---@param line integer
---@param resolveSymbol fun(name: string): MoonCatsSymbol?
---@param ctx table
---@return MoonCatsMemberIR member
classifyStaticMember = (name, value, line, resolveSymbol, ctx) ->
  member = {:name, :line, isPrivate: name\match("^__") != nil}
  if nodeType(value) == "fndef"
    member.params = paramsFromFndef value[2]
    member.arrow = value[4]
    member.hasExplicitValueReturn = fndefHasValueReturn value
    member.kind = member.arrow == "slim" and MemberKind.StaticMethodThin or MemberKind.StaticMethodFat
    return member
  spec = enumSpecFromChain value, resolveSymbol
  if spec
    enum = recordEnum ctx, spec, line
    enum.exportedAs = name
    member.kind = MemberKind.EnumExport
    member.enum = enum
    return member
  member.valueInfo = valueInfoFromNode value
  if member.valueInfo.kind == ValueKind.Reference
    sym = resolveSymbol member.valueInfo.refName
    if sym and sym.kind == SymbolKind.Enum
      sym.enum.exportedAs or= name
      member.kind = MemberKind.EnumExport
      member.enum = sym.enum
      return member
  member.kind = MemberKind.StaticData
  member

---Walks a class statement's body into a ClassIR, registering members and class locals.
---@param classNode table
---@param ctx table
---@return MoonCatsClassIR class
walkClass = (classNode, ctx) ->
  cls = {
    name: classNode[2]
    line: nodeLine(classNode, ctx.source) or 0
    hasParent: "table" == type classNode[3]
    members: {}
    localSymbols: {}
  }
  cls.typeName = cls.name
  if cls.hasParent and nodeType(classNode[3]) == "ref"
    cls.parentName = classNode[3][2]
  resolveSymbol = (name) -> cls.localSymbols[name] or ctx.symbols[name]

  body = classNode[4]
  if "table" == type body
    for item in *body
      continue unless "table" == type item
      if item[1] == "props"
        for j = 2, #item
          kv = item[j]
          key, value = kv[1], kv[2]
          keyType = nodeType key
          if keyType == "key_literal" or keyType == "self" or keyType == "self_class"
            line = nodeLine(value, ctx.source) or cls.line
            member = classifyPropsMember key[2], keyType, value, line, resolveSymbol, ctx
            table.insert cls.members, member
            registerLineItem ctx, line, {kind: "member", :member}
          else
            report ctx, FindingCode.ComputedKeySkipped, nodeLine(kv, ctx.source), cls.name
      else
        stm = item[2]
        continue unless "table" == type stm
        continue unless nodeType(stm) == "assign"
        names, values = stm[2], stm[3]
        continue unless "table" == type(names) and "table" == type values
        line = nodeLine(stm, ctx.source) or cls.line
        first = names[1]
        firstType = nodeType first
        if firstType == "self" or firstType == "self_class"
          member = classifyStaticMember first[2], values[1], line, resolveSymbol, ctx
          table.insert cls.members, member
          registerLineItem ctx, line, {kind: "member", :member}
        elseif firstType == "ref"
          for i, nameNode in ipairs names
            continue unless nodeType(nameNode) == "ref"
            cls.localSymbols[nameNode[2]] = symbolFromValue values[i], ctx, line
          registerLineItem ctx, line, {kind: "local", name: first[2], scope: cls.localSymbols}

  table.insert ctx.classes, cls
  ctx.classesByName[cls.name] = cls
  ctx.symbols[cls.name] = {kind: SymbolKind.Class, class: cls}
  registerLineItem ctx, cls.line, {kind: "class", class: cls}
  cls

---Handles one module-level assignment — a local binding, a destructuring import, or a dotted augmentation.
---@param stm table
---@param ctx table
walkModuleAssign = (stm, ctx) ->
  names, values = stm[2], stm[3]
  return unless "table" == type(names) and "table" == type values
  line = nodeLine stm, ctx.source
  first = names[1]
  firstType = nodeType first

  if #names == 1 and firstType == "chain"
    info = describeChain first
    if info and info.baseName and info.accessor and info.accessorKind == "dot" and not info.callArgs
      -- a `Class.__class.field =` path manipulates the runtime class object, not the documented type surface
      for i = 3, #first
        item = first[i]
        return if "table" == type(item) and item[1] == "dot" and item[2] == "__class"
      value = values[1]
      aug = {
        targetName: info.baseName
        name: info.accessor
        line: line or 0
        valueInfo: valueInfoFromNode value
      }
      if aug.valueInfo.kind == ValueKind.Function
        aug.params, aug.arrow = aug.valueInfo.params, aug.valueInfo.arrow
        aug.hasExplicitValueReturn = fndefHasValueReturn value
      table.insert ctx.augmentations, aug
      registerLineItem ctx, aug.line, {kind: "augmentation", augmentation: aug}
    return

  if #names == 1 and firstType == "table"
    requireId = requireIdFromChain values[1]
    for pair in *first[2]
      target = pair[2]
      if "table" == type(target) and nodeType(target) == "ref"
        ctx.symbols[target[2]] = requireId and {kind: SymbolKind.Import, :requireId} or {kind: SymbolKind.Other}
    registerLineItem ctx, line, {kind: "skip"}
    return

  for i, nameNode in ipairs names
    continue unless nodeType(nameNode) == "ref"
    ctx.symbols[nameNode[2]] = symbolFromValue values[i], ctx, line
  if firstType == "ref"
    registerLineItem ctx, line, {kind: "local", name: first[2], scope: ctx.symbols}

---Handles an import statement, binding each imported name.
---@param stm table
---@param ctx table
walkImport = (stm, ctx) ->
  names, source = stm[2], stm[3]
  requireId = requireIdFromChain source
  if "table" == type names
    for nameNode in *names
      name = if "table" == type nameNode then nameNode[2] else nameNode
      continue unless "string" == type name
      ctx.symbols[name] = requireId and {kind: SymbolKind.Import, :requireId} or {kind: SymbolKind.Other}
  registerLineItem ctx, nodeLine(stm, ctx.source), {kind: "skip"}

---Resolves an export expression to its exported value.
---@param node any
---@param ctx table
---@return MoonCatsExportIR export
resolveExportTarget = (node, ctx) ->
  switch nodeType node
    when "ref"
      name = node[2]
      sym = ctx.symbols[name]
      if sym
        switch sym.kind
          when SymbolKind.Class
            {kind: ExportKind.Class, :name, class: sym.class}
          when SymbolKind.Function
            {
              kind: ExportKind.Function, :name
              params: sym.params, block: sym.block
              hasExplicitValueReturn: sym.hasExplicitValueReturn
            }
          when SymbolKind.Table
            {kind: ExportKind.Table, :name, tableNode: sym.node}
          else
            {kind: ExportKind.Unknown, :name}
      else
        {kind: ExportKind.Unknown, :name}
    when "table"
      {kind: ExportKind.Table, tableNode: node}
    when "chain"
      info = describeChain node
      if info and info.callArgs and info.callArgs[1] and
        (info.accessor == "withTestExports" or info.accessor == "register" or info.accessor == "install")
          resolveExportTarget info.callArgs[1], ctx
      else
        {kind: ExportKind.Unknown}
    else
      {kind: ExportKind.Unknown}

---Determines what the module returns, from its last top-level statement.
---@param tree table
---@param ctx table
---@return MoonCatsExportIR export
resolveModuleExport = (tree, ctx) ->
  last = tree[#tree]
  return {kind: ExportKind.Unknown} unless "table" == type last
  switch nodeType last
    when "return"
      expList = last[2]
      if "table" == type(expList) and expList[2] != nil
        resolveExportTarget expList[2], ctx
      else
        {kind: ExportKind.Unknown}
    when "class"
      cls = ctx.classesByName[last[2]]
      {kind: ExportKind.Class, name: last[2], class: cls}
    when "chain"
      info = describeChain last
      if info and info.accessor == "install" and info.callArgs and info.callArgs[1]
        resolveExportTarget info.callArgs[1], ctx
      else
        {kind: ExportKind.Unknown}
    when "table"
      {kind: ExportKind.Table, tableNode: last}
    else
      {kind: ExportKind.Unknown}

---Builds field records for a table-module export and registers them for block binding.
---@param ctx table
buildExportFields = (ctx) ->
  exportIR = ctx.export
  return unless exportIR.kind == ExportKind.Table and exportIR.tableNode
  exportIR.fields = {}
  for pair in *exportIR.tableNode[2]
    key, value = pair[1], pair[2]
    continue if value == nil
    unless nodeType(key) == "key_literal"
      report ctx, FindingCode.ComputedKeySkipped, nodeLine(key, ctx.source), ctx.requireId
      continue
    line = nodeLine(value, ctx.source) or 0
    field = {name: key[2], :line, valueInfo: valueInfoFromNode value}
    -- a field holding an Enum (inline or a module-local reference) is an enum export, like a class static
    spec = enumSpecFromChain value, (n) -> ctx.symbols[n]
    if spec
      field.enum = recordEnum ctx, spec, line
      field.enum.exportedAs = field.name
    elseif field.valueInfo.kind == ValueKind.Reference
      sym = ctx.symbols[field.valueInfo.refName]
      if sym and sym.kind == SymbolKind.Enum
        sym.enum.exportedAs or= field.name
        field.enum = sym.enum
    if field.valueInfo.kind == ValueKind.Function
      field.params, field.arrow = field.valueInfo.params, field.valueInfo.arrow
      field.hasExplicitValueReturn = fndefHasValueReturn value
    table.insert exportIR.fields, field
    registerLineItem ctx, line, {kind: "field", :field}

---Hoists a block's standalone alias/class segments into the module IR and returns the member contract.
---@param segments MoonCatsSegment[]
---@param ctx table
---@param line integer Source line the block starts on, for output ordering.
---@return string[]? restLines The rest segment's lines, or nil when the block has none.
hoistSegments = (segments, ctx, line) ->
  restLines = nil
  for segment in *segments
    if segment.kind == SegmentKind.Rest
      restLines = segment.lines
    else
      segment.line = line
      table.insert ctx.segments, segment
      if segment.kind == SegmentKind.Alias and segment.name
        ctx.aliases[segment.name] = true
  restLines

---Binds every comment block to the item defined on the following line, hoisting standalone declarations.
---@param blocks MoonCatsAnnotationBlock[]
---@param ctx table
bindBlocks = (blocks, ctx) ->
  for block in *blocks
    target = ctx.lineItems[block.endLine + 1]
    unless target
      hoistSegments (segmentBlock block), ctx, block.startLine
      continue
    switch target.kind
      when "class"
        -- a real class keeps its whole block verbatim: prose, @class line, and @field list
        target.class.block = block.lines
      when "member"
        rest = hoistSegments (segmentBlock block), ctx, block.startLine
        target.member.block = rest if rest
      when "augmentation"
        rest = hoistSegments (segmentBlock block), ctx, block.startLine
        target.augmentation.block = rest if rest
      when "field"
        rest = hoistSegments (segmentBlock block), ctx, block.startLine
        target.field.block = rest if rest
      when "local"
        rest = hoistSegments (segmentBlock block), ctx, block.startLine
        if rest and target.name
          sym = target.scope[target.name]
          sym.block = rest if sym
      else
        hoistSegments (segmentBlock block), ctx, block.startLine

---Resolves each class's annotated type name and marks its @private members, once blocks are bound.
---@param ctx table
finalizeClasses = (ctx) ->
  for cls in *ctx.classes
    if cls.block
      annotatedName, annotatedParent = classAnnotationInfo cls.block
      if annotatedName
        if annotatedName != cls.name
          report ctx, FindingCode.ClassNameMismatch, cls.line, annotatedName, cls.name
        cls.typeName = annotatedName
        cls.annotatedParentName = annotatedParent
    for member in *cls.members
      member.isPrivate = true if blockContainsPrivate member.block

---Parses annotated MoonScript sources into the intermediate representation consumed by the definition emitter.
---@class MoonCatsParser
class MoonCatsParser
  ---@type Enum
  @MemberKind = MemberKind
  ---@type Enum
  @ExportKind = ExportKind
  ---@type Enum
  @SymbolKind = SymbolKind
  ---@type Enum
  @SegmentKind = SegmentKind
  ---@type Enum
  @ValueKind = ValueKind

  ---Parses one MoonScript module into its definition-extraction IR.
  ---@param source string The module's MoonScript source text.
  ---@param requireId string Require identifier of the module (e.g. "l0.DependencyControl.Timer").
  ---@param diagnostics? MoonCatsDiagnostics Collection that receives findings discovered while parsing.
  ---@return MoonCatsModuleIR? ir The parsed module IR, or nil when the source fails to parse.
  ---@return string? err Parse error message when parsing failed.
  parse: (source, requireId, diagnostics) =>
    tree, err = moonParse.string source
    return nil, tostring err unless tree

    ctx = {
      :source, :requireId, :diagnostics
      lines: splitLines source
      symbols: {}
      enums: {}
      classes: {}
      classesByName: {}
      segments: {}
      aliases: {}
      augmentations: {}
      lineItems: {}
    }

    for stm in *tree
      continue unless "table" == type stm
      switch nodeType stm
        when "assign" then walkModuleAssign stm, ctx
        when "class" then walkClass stm, ctx
        when "import" then walkImport stm, ctx

    ctx.export = resolveModuleExport tree, ctx
    if ctx.export.kind == ExportKind.Unknown
      report ctx, FindingCode.NoExport, #ctx.lines
    buildExportFields ctx

    mask = computeStringMask tree, source
    blocks = scanCommentBlocks ctx.lines, mask
    bindBlocks blocks, ctx
    finalizeClasses ctx

    -- a leading doc block detached from any declaration (nothing bound on the line below it, and
    -- no code above it) documents the module itself
    if #blocks > 0 and not ctx.lineItems[blocks[1].endLine + 1]
      aboveAllCode = true
      for lineNo in pairs ctx.lineItems
        aboveAllCode = false if lineNo < blocks[1].startLine
      ctx.moduleDoc = blocks[1].lines if aboveAllCode

    -- binding runs after export resolution, so late-fill blocks copied off symbols
    if ctx.export.kind == ExportKind.Function and ctx.export.name
      sym = ctx.symbols[ctx.export.name]
      ctx.export.block or= sym and sym.block

    -- enum exports through module-level augmentations (Class.Name = enumLocal)
    for aug in *ctx.augmentations
      if aug.valueInfo.kind == ValueKind.Reference
        sym = ctx.symbols[aug.valueInfo.refName]
        if sym and sym.kind == SymbolKind.Enum
          sym.enum.exportedAs or= aug.name

    {
      requireId: ctx.requireId
      lines: ctx.lines
      segments: ctx.segments
      classes: ctx.classes
      classesByName: ctx.classesByName
      enums: ctx.enums
      aliases: ctx.aliases
      symbols: ctx.symbols
      export: ctx.export
      augmentations: ctx.augmentations
      moduleDoc: ctx.moduleDoc
    }

UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
return UnitTestSuite\withTestExports MoonCatsParser, {
  :splitLines, :computeStringMask, :scanCommentBlocks, :segmentBlock
  :paramsFromFndef, :literalTokenFromNode, :requireIdFromChain, :describeChain
  :fndefHasValueReturn, :classAnnotationInfo
}
