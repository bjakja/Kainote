Parser = require "l0.MoonCats.Parser"
annotations = require "l0.MoonCats.annotations"

{:MemberKind, :ExportKind, :SymbolKind, :ValueKind, :SegmentKind} = Parser
{:collectBlockTags, :parseFieldTag, :parseAliasVariants, :proseLines, :blockTagText
  :blockDocumentsFunction, :blockOwnType, :flattenBlockProse
  :inferLiteralType, :inferExpressionType} = annotations

---Options controlling a documentation render.
---@class MoonCatsDocOptions
---@field includePrivate? boolean Render private members with a badge instead of omitting them (default false).
---@field siteName? string Site title used by the index page and scaffolding (default "API Documentation").
---@field site? string Scaffold flavor to emit: "mkdocs" (default), "mdbook", or "none".
---@field packages? table<string, {name: string, description?: string, version?: string, modules: string[]}> Feed package info keyed by namespace, used to group the index page.

---One rendered documentation file.
---@class MoonCatsDocPage
---@field path string File path relative to the output root; module and index pages live under docs/.
---@field title string
---@field text string The page's markdown.

---The outcome of a documentation render.
---@class MoonCatsDocResult
---@field pages MoonCatsDocPage[] One page per module, sorted by require identifier.
---@field indexPage MoonCatsDocPage
---@field scaffold MoonCatsDocPage[] Site-generator scaffolding files; empty for site "none".

luaKeywords = {
  "and": true, "break": true, "do": true, "else": true, "elseif": true, "end": true
  "false": true, "for": true, "function": true, "if": true, "in": true, "local": true
  "nil": true, "not": true, "or": true, "repeat": true, "return": true, "then": true
  "true": true, "until": true, "while": true
  -- MoonScript additions
  "class": true, "export": true, "import": true, "switch": true, "when": true
  "unless": true, "using": true, "with": true, "extends": true, "super": true
}

builtinTypes = {
  "any": true, "boolean": true, "string": true, "number": true, "integer": true
  "table": true, "function": true, "nil": true, "true": true, "false": true
  "self": true, "fun": true, "userdata": true, "lightuserdata": true, "thread": true
}

---Derives the receiver variable used in instance-call examples from a class name.
---@param typeName string
---@return string receiver
receiverName = (typeName) ->
  leaf = typeName\match("([%w_]+)$") or typeName
  name = leaf\sub(1, 1)\lower! .. leaf\sub 2
  luaKeywords[name] and "the" .. leaf or name

---Escapes a description for use inside a markdown table cell.
---@param text? string
---@return string cellText
cellText = (text) ->
  return "" unless text
  (text\gsub("|", "\\|")\gsub "\n", " ")

---Sanitizes a require identifier's leaf into an identifier for module-value examples.
---@param requireId string
---@return string name
moduleVarName = (requireId) ->
  leaf = requireId\match("([^%.]+)$") or requireId
  name = leaf\gsub "[^%w_]", "_"
  name = "_" .. name if name\match "^%d"
  name

---Renders a type expression with cross-links to known type anchors. Complex expressions
---(function types, inline tables, generics) stay as a single code span.
---@param typeText? string
---@param state table
---@return string rendered The markdown for the type, with known names linked; empty for a nil or empty input.
linkifyType = (typeText, state) ->
  return "" unless typeText and #typeText > 0
  if typeText\find("fun(", 1, true) or typeText\find("{", 1, true) or typeText\find("<", 1, true)
    return "`#{typeText}`"
  out = {}
  pos = 1
  while pos <= #typeText
    startPos, endPos = typeText\find "[%w_%.]+", pos
    unless startPos
      table.insert out, (cellText typeText\sub pos)
      break
    if startPos > pos
      table.insert out, (cellText typeText\sub pos, startPos - 1)
    token = typeText\sub startPos, endPos
    target = state.linkIndex[token]
    if target
      href = target.page == state.currentPage and "#" .. target.anchor or "#{target.page}##{target.anchor}"
      table.insert out, "[#{token}](#{href})"
    else
      table.insert out, "`#{token}`"
    pos = endPos + 1
  table.concat out

---Builds the moon- and Lua-form call lines shown in a signature block.
---@param callee string The receiver-and-name prefix, e.g. "timer\\start" and "timer:start".
---@param calleeLua string
---@param paramNames string[]
---@param assignTo? string Left-hand side for constructor examples.
---@return string moonLine
---@return string luaLine
callForms = (callee, calleeLua, paramNames, assignTo) ->
  args = table.concat paramNames, ", "
  moonLine = #paramNames > 0 and "#{callee} #{args}" or "#{callee}!"
  luaLine = "#{calleeLua}(#{args})"
  if assignTo
    moonLine = "#{assignTo} = #{moonLine}"
    luaLine = "local #{assignTo} = #{luaLine}"
  moonLine, luaLine

---Emits a block's prose, upgrading markdown indented code blocks into fenced moonscript blocks
---so the site highlights them. An indented run is a code block only when a blank line precedes
---it, so hanging-indent continuation lines inside a paragraph are left as prose. A trailing blank
---line is appended when anything was emitted.
---@param out string[]
---@param blockLines? string[]
---@return boolean emitted Whether any prose was written.
pushProse = (out, blockLines) ->
  lines = proseLines blockLines
  return false if #lines == 0
  i = 1
  while i <= #lines
    line = lines[i]
    blankBefore = i == 1 or lines[i - 1]\match("^%s*$") != nil
    if blankBefore and line\match "^    "
      -- collect the indented run (interior blank lines belong to the example)
      last = i
      probe = i
      while probe <= #lines and (lines[probe]\match("^    ") or lines[probe]\match "^%s*$")
        last = probe if not lines[probe]\match "^%s*$"
        probe += 1
      table.insert out, "```moonscript"
      for exampleIndex = i, last
        table.insert out, (lines[exampleIndex]\gsub "^    ", "")
      table.insert out, "```"
      i = last + 1
    else
      table.insert out, line
      i += 1
  table.insert out, ""
  true

---Emits a construct's MoonScript and Lua forms as linked content tabs, falling back to one aligned
---block where the host lacks tab support (mdbook and the standalone mkdocs scaffold).
---@param out string[]
---@param tabbed boolean
---@param moonLine string
---@param luaLine string
pushLanguageForms = (out, tabbed, moonLine, luaLine) ->
  if tabbed
    table.insert out, "=== \"MoonScript\""
    table.insert out, ""
    table.insert out, "    ```moonscript"
    table.insert out, "    #{moonLine}"
    table.insert out, "    ```"
    table.insert out, ""
    table.insert out, "=== \"Lua\""
    table.insert out, ""
    table.insert out, "    ```lua"
    table.insert out, "    #{luaLine}"
    table.insert out, "    ```"
  else
    width = math.max #moonLine, #luaLine
    pad = (line) -> line .. (" ")\rep width - #line
    table.insert out, "```lua"
    table.insert out, "#{pad moonLine}  -- MoonScript"
    table.insert out, "#{pad luaLine}  -- Lua"
    table.insert out, "```"
  table.insert out, ""

---Resolves the documented type of a plain-data member, chasing identifier references
---through the module's and run's symbol tables.
---@param state table
---@param ir MoonCatsModuleIR
---@param cls? MoonCatsClassIR
---@param member table
---@return string typeText Falls back to "any" when the type cannot be resolved.
resolveDataType = (state, ir, cls, member) ->
  ownType = blockOwnType member.block
  return ownType if ownType
  return "#{member.enum.name}Enum" if member.enum
  valueInfo = member.valueInfo
  return "any" unless valueInfo

  chaseRef = (name, depth = 0) ->
    return nil if depth > 4 or name == nil
    sym = (cls and cls.localSymbols[name]) or ir.symbols[name]
    return nil unless sym
    switch sym.kind
      when SymbolKind.Class then sym.class.typeName
      when SymbolKind.Require then state.typeNameByRequireId[sym.requireId]
      when SymbolKind.Enum
        -- unexported module-local enums have no synthesized <Name>Enum class, so they use base Enum
        sym.enum.exportedAs and "#{sym.enum.name}Enum" or "Enum"
      when SymbolKind.Reference then chaseRef sym.target, depth + 1
      when SymbolKind.Literal
        sym.literalKind != "nil" and inferLiteralType(sym.token, sym.literalKind) or nil
      else nil

  switch valueInfo.kind
    when ValueKind.Literal
      valueInfo.literalKind == "nil" and "any" or inferLiteralType valueInfo.token, valueInfo.literalKind
    when ValueKind.Reference
      chaseRef(valueInfo.refName) or "any"
    when ValueKind.Call
      -- setmetatable(t, mt) returns t; the common `setmetatable {}, …` form is a table
      valueInfo.baseName == "setmetatable" and "table" or chaseRef(valueInfo.baseName) or "any"
    when ValueKind.Table then "table"
    when ValueKind.Expression then inferExpressionType valueInfo.node
    else "any"

---Resolves a member holding a bare reference to a local function into a renderable function
---member carrying the local's signature and annotation block.
---@param ir MoonCatsModuleIR
---@param cls? MoonCatsClassIR
---@param member table
---@return table? fnMember Nil when the member is not such a reference.
resolveFunctionMember = (ir, cls, member) ->
  valueInfo = member.valueInfo
  return nil unless valueInfo and valueInfo.kind == ValueKind.Reference
  return nil if blockDocumentsFunction member.block
  sym = (cls and cls.localSymbols[valueInfo.refName]) or ir.symbols[valueInfo.refName]
  return nil unless sym and sym.kind == SymbolKind.Function
  {
    name: member.name, line: member.line
    block: member.block or sym.block
    params: sym.params
    isPrivate: member.isPrivate or member.name\match("^__") != nil
  }

---Reports whether a member should be hidden as private under the given options.
---@param member table
---@param state table
---@return boolean hidden
isHiddenPrivate = (member, state) ->
  return false if state.includePrivate
  (member.isPrivate or (member.name and member.name\match("^__") != nil)) and true or false

---Formats a member heading, badging private members when they are rendered at all.
---@param name string
---@param anchor string
---@param level string Heading marker, e.g. "####".
---@param isPrivate? boolean
---@return string heading
memberHeading = (name, anchor, level, isPrivate) ->
  badge = isPrivate and " 🔒" or ""
  "#{level} #{name}#{badge} <a name=\"#{anchor}\"></a>"

---Renders the full documentation section for one function-like member.
---@param out string[]
---@param state table
---@param member table
---@param moonCallee string
---@param luaCallee string
---@param anchorPrefix string
renderFunctionMember = (out, state, member, moonCallee, luaCallee, anchorPrefix) ->
  table.insert out, memberHeading member.name, "#{anchorPrefix}.#{member.name}", "####", member.isPrivate
  table.insert out, ""

  deprecated = blockTagText member.block, "deprecated"
  if deprecated
    table.insert out, "> **Deprecated**#{#deprecated > 0 and " — " .. deprecated or ""}"
    table.insert out, ""

  tags, returns = collectBlockTags member.block
  paramNames = if member.params
    [param.name for param in *member.params]
  else
    [tag.name for tag in *tags]
  moonLine, luaLine = callForms moonCallee, luaCallee, paramNames
  pushLanguageForms out, state.tabbedForms, moonLine, luaLine

  pushProse out, member.block

  if #tags > 0
    table.insert out, "| param | type | description |"
    table.insert out, "|-------|------|-------------|"
    for tag in *tags
      optional = tag.optional and "?" or ""
      table.insert out, "| #{tag.name}#{optional} | #{linkifyType tag.type, state} | #{cellText tag.description} |"
    table.insert out, ""

  if #returns > 0
    table.insert out, "**Returns:**"
    table.insert out, ""
    for returnTag in *returns
      label = returnTag.name and "`#{returnTag.name}` " or ""
      description = returnTag.description and " — #{cellText returnTag.description}" or ""
      table.insert out, "- #{label}#{linkifyType returnTag.type, state}#{description}"
    table.insert out, ""

  see = blockTagText member.block, "see"
  if see and #see > 0
    table.insert out, "See also: `#{see}`"
    table.insert out, ""

---Renders a fields table from prepared row records.
---@param out string[]
---@param state table
---@param rows {name: string, typeText: string, description?: string, isPrivate?: boolean}[]
renderFieldsTable = (out, state, rows) ->
  return if #rows == 0
  table.insert out, "| field | type | description |"
  table.insert out, "|-------|------|-------------|"
  for row in *rows
    badge = row.isPrivate and " 🔒" or ""
    table.insert out, "| #{row.name}#{badge} | #{linkifyType row.typeText, state} | #{cellText row.description} |"
  table.insert out, ""

---Renders an enum's member table, joining descriptions from its alias's variant lines.
---@param out string[]
---@param state table
---@param enum MoonCatsEnumIR
---@param block? string[] The exporting member's annotation block, for lead prose.
---@param anchorPrefix string
renderEnum = (out, state, enum, block, anchorPrefix) ->
  -- a bare-name anchor so a `param: Name` value-type link resolves here; the heading keeps the
  -- prefixed anchor used by the enum-object type (<Name>Enum)
  table.insert out, "<a name=\"#{enum.name}\"></a>"
  table.insert out, memberHeading enum.name, "#{anchorPrefix}.#{enum.name}", "####"
  table.insert out, ""
  pushProse out, block

  variantsByKey, variantsByValue = {}, {}
  aliasSegment = state.aliasSegmentsByName[enum.name]
  if aliasSegment
    for variant in *parseAliasVariants aliasSegment.lines
      variantsByKey[variant.key] = variant if variant.key
      variantsByValue[variant.value] = variant

  table.insert out, "| member | value | description |"
  table.insert out, "|--------|-------|-------------|"
  for enumMember in *enum.members
    variant = variantsByKey[enumMember.key] or variantsByValue[enumMember.literal]
    description = variant and variant.description or nil
    table.insert out, "| #{enumMember.display or enumMember.key} | `#{enumMember.literal}` | #{cellText description} |"
  table.insert out, ""

---Renders a standalone alias or annotation-only class segment in a module's Types section.
---@param out string[]
---@param state table
---@param segment MoonCatsSegment
renderSegment = (out, state, segment) ->
  return unless segment.name
  table.insert out, memberHeading segment.name, segment.name, "###"
  table.insert out, ""
  pushProse out, segment.lines

  if segment.kind == SegmentKind.Alias
    variants = parseAliasVariants segment.lines
    if #variants > 0
      table.insert out, "| value | description |"
      table.insert out, "|-------|-------------|"
      for variant in *variants
        label = variant.key and "#{variant.key}: " or ""
        table.insert out, "| `#{variant.value}` | #{label}#{cellText variant.description} |"
      table.insert out, ""
  else
    rows = {}
    for line in *segment.lines
      fieldTag = parseFieldTag line
      continue unless fieldTag
      isPrivate = fieldTag.visibility == "private" or fieldTag.name\match("^__") != nil
      continue if isPrivate and not state.includePrivate
      optional = fieldTag.optional and "?" or ""
      table.insert rows, {
        name: fieldTag.name .. optional, typeText: fieldTag.type
        description: fieldTag.description, :isPrivate
      }
    renderFieldsTable out, state, rows

---Splits a class's members into renderable groups, dropping hidden privates and duplicate
---definitions (the first documented one wins).
---@param state table
---@param ir MoonCatsModuleIR
---@param cls MoonCatsClassIR
---@return table groups Lists: ctor, instanceMethods, classMethods, dataFields, enums.
groupClassMembers = (state, ir, cls) ->
  groups = {ctor: nil, instanceMethods: {}, classMethods: {}, dataFields: {}, enums: {}}
  seen = {}
  byName = {}
  for member in *cls.members
    continue if member.kind == MemberKind.Metamethod
    byName[member.name] or= {}
    table.insert byName[member.name], member

  pickPrimary = (name) ->
    group = byName[name]
    return group[1] if #group == 1
    for candidate in *group
      return candidate if candidate.block
    group[1]

  for member in *cls.members
    continue if member.kind == MemberKind.Metamethod
    continue if seen[member.name]
    primary = pickPrimary member.name
    continue if member != primary
    seen[member.name] = true
    continue if isHiddenPrivate primary, state

    switch primary.kind
      when MemberKind.Constructor
        groups.ctor = primary
      when MemberKind.Method
        table.insert groups.instanceMethods, primary
      when MemberKind.StaticMethodFat, MemberKind.StaticMethodThin
        table.insert groups.classMethods, primary
      when MemberKind.EnumExport
        table.insert groups.enums, primary
      when MemberKind.AccessorProperty
        nil -- documented via the class block's @field lines
      when MemberKind.StaticData, MemberKind.InstanceDefault
        fnMember = resolveFunctionMember ir, cls, primary
        if fnMember
          fnMember.kind = MemberKind.StaticMethodThin
          table.insert groups.classMethods, fnMember unless isHiddenPrivate fnMember, state
        elseif blockDocumentsFunction primary.block
          table.insert groups.classMethods, primary
        else
          table.insert groups.dataFields, primary

  for aug in *ir.augmentations
    continue unless aug.targetName == cls.name
    continue if seen[aug.name] or isHiddenPrivate aug, state
    seen[aug.name] = true
    augEnum = aug.enum
    if not augEnum and aug.valueInfo.kind == ValueKind.Reference
      sym = ir.symbols[aug.valueInfo.refName]
      if sym and sym.kind == SymbolKind.Enum and sym.enum.exportedAs == aug.name
        augEnum = sym.enum
    if augEnum
      table.insert groups.enums, {name: aug.name, block: aug.block, enum: augEnum}
    else
      fnMember = resolveFunctionMember ir, cls, aug
      if fnMember
        fnMember.kind = MemberKind.StaticMethodThin
        table.insert groups.classMethods, fnMember
      elseif aug.valueInfo.kind == ValueKind.Function or blockDocumentsFunction aug.block
        table.insert groups.classMethods, {
          name: aug.name, block: aug.block, params: aug.params
          kind: MemberKind.StaticMethodThin
          isPrivate: aug.name\match("^__") != nil
        }
      else
        table.insert groups.dataFields, aug

  groups

---Renders the full documentation section for one class.
---@param out string[]
---@param state table
---@param ir MoonCatsModuleIR
---@param cls MoonCatsClassIR
renderClass = (out, state, ir, cls) ->
  typeName = cls.typeName
  table.insert out, "## #{typeName} <a name=\"#{typeName}\"></a>"
  table.insert out, ""
  pushProse out, cls.block

  groups = groupClassMembers state, ir, cls
  receiver = receiverName typeName

  if groups.ctor
    table.insert out, "### Constructor <a name=\"#{typeName}.new\"></a>"
    table.insert out, ""
    ctor = groups.ctor
    tags, _ = collectBlockTags ctor.block
    paramNames = [param.name for param in *ctor.params or {}]
    moonLine, luaLine = callForms typeName, typeName, paramNames, receiver
    pushLanguageForms out, state.tabbedForms, moonLine, luaLine
    pushProse out, ctor.block
    if #tags > 0
      table.insert out, "| param | type | description |"
      table.insert out, "|-------|------|-------------|"
      for tag in *tags
        optional = tag.optional and "?" or ""
        table.insert out, "| #{tag.name}#{optional} | #{linkifyType tag.type, state} | #{cellText tag.description} |"
      table.insert out, ""

  if #groups.instanceMethods > 0
    table.insert out, "### Instance methods"
    table.insert out, ""
    for member in *groups.instanceMethods
      renderFunctionMember out, state, member,
        "#{receiver}\\#{member.name}", "#{receiver}:#{member.name}", typeName

  if #groups.classMethods > 0
    table.insert out, "### Class methods"
    table.insert out, ""
    for member in *groups.classMethods
      if member.kind == MemberKind.StaticMethodFat
        renderFunctionMember out, state, member,
          "#{typeName}\\#{member.name}", "#{typeName}:#{member.name}", typeName
      else
        renderFunctionMember out, state, member,
          "#{typeName}.#{member.name}", "#{typeName}.#{member.name}", typeName

  fieldRows = {}
  for line in *cls.block or {}
    fieldTag = parseFieldTag line
    continue unless fieldTag
    isPrivate = fieldTag.visibility == "private" or fieldTag.name\match("^__") != nil
    continue if isPrivate and not state.includePrivate
    optional = fieldTag.optional and "?" or ""
    table.insert fieldRows, {
      name: fieldTag.name .. optional, typeText: fieldTag.type
      description: fieldTag.description, :isPrivate
    }
  for member in *groups.dataFields
    table.insert fieldRows, {
      name: member.name
      typeText: resolveDataType state, ir, cls, member
      description: flattenBlockProse member.block
      isPrivate: member.isPrivate
    }
  if #fieldRows > 0
    table.insert out, "### Fields"
    table.insert out, ""
    renderFieldsTable out, state, fieldRows

  if #groups.enums > 0
    table.insert out, "### Enums"
    table.insert out, ""
    for member in *groups.enums
      renderEnum out, state, member.enum, member.block, typeName

---Renders a table-module's fields and functions onto its module value.
---@param out string[]
---@param state table
---@param ir MoonCatsModuleIR
renderTableModule = (out, state, ir) ->
  varName = ir.export.name and ir.export.name\match("^[%w_]+$") and ir.export.name or moduleVarName ir.requireId
  -- anchor for the module's own declared type, so a re-export elsewhere links here. Its @class is
  -- dropped from the Types listing, which would otherwise carry the anchor.
  table.insert out, "<a name=\"#{ir.export.name}\"></a>" if ir.export.name
  fieldRows = {}
  fnMembers = {}
  enumMembers = {}

  collect = (entry) ->
    return if isHiddenPrivate entry, state
    if entry.enum
      table.insert enumMembers, entry
      return
    fnMember = resolveFunctionMember ir, nil, entry
    if fnMember
      table.insert fnMembers, fnMember
    elseif (entry.valueInfo and entry.valueInfo.kind == ValueKind.Function) or blockDocumentsFunction entry.block
      table.insert fnMembers, entry
    else
      table.insert fieldRows, {
        name: entry.name
        typeText: resolveDataType state, ir, nil, entry
        description: flattenBlockProse entry.block
        isPrivate: entry.isPrivate
      }

  collect field for field in *ir.export.fields or {}
  for aug in *ir.augmentations
    collect aug if aug.targetName == ir.export.name or aug.targetName == varName

  if #fnMembers > 0
    table.insert out, "## Functions"
    table.insert out, ""
    for member in *fnMembers
      renderFunctionMember out, state, member,
        "#{varName}.#{member.name}", "#{varName}.#{member.name}", varName

  if #fieldRows > 0
    table.insert out, "## Fields"
    table.insert out, ""
    renderFieldsTable out, state, fieldRows

  if #enumMembers > 0
    table.insert out, "## Enums"
    table.insert out, ""
    for entry in *enumMembers
      renderEnum out, state, entry.enum, entry.block, varName

---Renders a function-module's single exported function.
---@param out string[]
---@param state table
---@param ir MoonCatsModuleIR
renderFunctionModule = (out, state, ir) ->
  exportIR = ir.export
  member = {
    name: exportIR.name, block: exportIR.block
    params: exportIR.params, isPrivate: false
  }
  renderFunctionMember out, state, member, exportIR.name, exportIR.name, ir.requireId

---Renders one module's documentation page.
---@param state table
---@param ir MoonCatsModuleIR
---@return MoonCatsDocPage page
renderModulePage = (state, ir) ->
  state.currentPage = "#{ir.requireId}.md"
  out = {}
  table.insert out, "# #{ir.requireId}"
  table.insert out, ""
  varName = if ir.export.kind == ExportKind.Class and ir.export.class
    ir.export.class.name
  elseif ir.export.kind == ExportKind.Function
    ir.export.name
  else
    moduleVarName ir.requireId
  moonRequire = "#{varName} = require \"#{ir.requireId}\""
  luaRequire = "local #{varName} = require(\"#{ir.requireId}\")"
  pushLanguageForms out, state.tabbedForms, moonRequire, luaRequire

  pushProse out, ir.moduleDoc if ir.moduleDoc

  switch ir.export.kind
    when ExportKind.Table
      renderTableModule out, state, ir
    when ExportKind.Function
      renderFunctionModule out, state, ir

  for cls in *ir.classes
    renderClass out, state, ir, cls

  -- an exported enum's @alias is already shown with its members under Enums, and a table module's own
  -- @class describes the module itself — drop both from the Types listing to avoid the duplicate
  enumNames = {}
  for enum in *ir.enums
    enumNames[enum.name] = true if enum.exportedAs
  shouldKeep = (seg) ->
    return false if seg.kind == SegmentKind.Alias and enumNames[seg.name]
    return false if seg.kind == SegmentKind.Class and seg.name == ir.export.name
    true
  namedSegments = [segment for segment in *ir.segments when segment.name and shouldKeep segment]
  if #namedSegments > 0
    table.insert out, "## Types"
    table.insert out, ""
    for segment in *namedSegments
      renderSegment out, state, segment

  -- pages sit in the docs dir (a docs/ subdir for standalone sites, flat for the embeddable
  -- section); cross-page links stay bare filenames because the pages are siblings either way
  {
    path: "#{state.docsPrefix}#{ir.requireId}.md"
    title: ir.requireId
    text: table.concat(out, "\n") .. "\n"
  }

---Builds the run-wide index mapping type names to their page and anchor.
---@param state table
---@param irs MoonCatsModuleIR[]
buildLinkIndex = (state, irs) ->
  for ir in *irs
    page = "#{ir.requireId}.md"
    for cls in *ir.classes
      state.linkIndex[cls.typeName] = {:page, anchor: cls.typeName}
    for segment in *ir.segments
      continue unless segment.name
      state.linkIndex[segment.name] or= {:page, anchor: segment.name}
      state.aliasSegmentsByName[segment.name] or= segment if segment.kind == SegmentKind.Alias
    for enum in *ir.enums
      continue unless enum.exportedAs
      owner = nil
      for cls in *ir.classes
        for member in *cls.members
          owner = cls.typeName if member.enum == enum
      anchor = owner and "#{owner}.#{enum.name}" or enum.name
      enumClassName = "#{enum.name}Enum"
      state.linkIndex[enumClassName] = {:page, :anchor} unless state.linkIndex[enumClassName]

---Renders the index page grouping module pages by their feed package.
---@param state table
---@param irs MoonCatsModuleIR[]
---@param opts MoonCatsDocOptions
---@return MoonCatsDocPage page
renderIndexPage = (state, irs, opts) ->
  out = {"# #{state.siteName}", ""}
  requireIds = [ir.requireId for ir in *irs]
  table.sort requireIds
  covered = {}

  if opts.packages
    namespaces = [namespace for namespace in pairs opts.packages]
    table.sort namespaces
    for namespace in *namespaces
      pkg = opts.packages[namespace]
      version = pkg.version and " `v#{pkg.version}`" or ""
      table.insert out, "## #{pkg.name or namespace}#{version}"
      table.insert out, ""
      if pkg.description
        table.insert out, pkg.description
        table.insert out, ""
      moduleIds = [id for id in *pkg.modules or {}]
      table.sort moduleIds
      for id in *moduleIds
        covered[id] = true
        table.insert out, "- [#{id}](#{id}.md)"
      table.insert out, ""

  leftovers = [id for id in *requireIds when not covered[id]]
  if #leftovers > 0
    if opts.packages
      table.insert out, "## Other modules"
      table.insert out, ""
    for id in *leftovers
      table.insert out, "- [#{id}](#{id}.md)"
    table.insert out, ""

  {path: "#{state.docsPrefix}index.md", title: state.siteName, text: table.concat(out, "\n") .. "\n"}

---Derives a package-grouped module's sidebar label by dropping the namespace prefix its package
---header already shows.
---@param requireId string
---@param namespace string
---@return string label
navLabel = (requireId, namespace) ->
  prefix = "#{namespace}."
  return requireId\sub #prefix + 1 if requireId\sub(1, #prefix) == prefix
  requireId

---Builds a literate-nav SUMMARY.md for the embeddable reference section, grouping module pages by
---their feed package. A host site includes it with a `- Reference: <dir>/` nav entry and the
---mkdocs-literate-nav plugin. Links are bare filenames, resolved against the section's own dir.
---@param pages MoonCatsDocPage[]
---@param opts MoonCatsDocOptions
---@return string summary
buildLiterateNav = (pages, opts) ->
  requireIds = [page.title for page in *pages]
  table.sort requireIds
  covered = {}
  out = {"* [Overview](index.md)"}
  if opts.packages
    namespaces = [namespace for namespace in pairs opts.packages]
    table.sort namespaces
    for namespace in *namespaces
      pkg = opts.packages[namespace]
      moduleIds = [id for id in *pkg.modules or {}]
      table.sort moduleIds
      table.insert out, "* #{pkg.name or namespace}"
      -- the root module (id == namespace) leads as "Overview"; the rest drop the namespace prefix
      for id in *moduleIds
        covered[id] = true
        label = if id == namespace then "Overview" else navLabel id, namespace
        table.insert out, "    * [#{label}](#{id}.md)"
  leftovers = [id for id in *requireIds when not covered[id]]
  if #leftovers > 0
    table.insert out, "* Other modules" if opts.packages
    indent = opts.packages and "    " or ""
    for id in *leftovers
      table.insert out, "#{indent}* [#{id}](#{id}.md)"
  table.concat(out, "\n") .. "\n"

---Builds the site scaffolding for the chosen flavor: a standalone mkdocs/mdbook project, or (for
---"none") the literate-nav SUMMARY.md that lets a host site embed the reference section.
---@param state table
---@param pages MoonCatsDocPage[]
---@param opts MoonCatsDocOptions
---@return MoonCatsDocPage[] scaffold The config/nav files; a lone SUMMARY.md for the embeddable "none" mode.
buildScaffold = (state, pages, opts) ->
  site = opts.site or "mkdocs"
  -- nav and summary entries are relative to the docs dir, so they use bare filenames
  fileName = (page) -> page.path\match "([^/]+)$"
  switch site
    when "mkdocs"
      out = {
        "site_name: #{state.siteName}"
        "theme:"
        "  name: material"
        -- mike maintains versions.json on the published site; Material shows its
        -- version picker when present and hides it in unversioned/local serves
        "extra:"
        "  version:"
        "    provider: mike"
        "nav:"
        "  - Overview: index.md"
      }
      for page in *pages
        table.insert out, "  - #{page.title}: #{fileName page}"
      {{path: "mkdocs.yml", title: "mkdocs.yml", text: table.concat(out, "\n") .. "\n"}}
    when "mdbook"
      summary = {"# Summary", "", "[Overview](index.md)", ""}
      for page in *pages
        table.insert summary, "- [#{page.title}](#{fileName page})"
      book = {
        "[book]"
        "title = \"#{state.siteName}\""
        "src = \"docs\""
      }
      {
        {path: "docs/SUMMARY.md", title: "Summary", text: table.concat(summary, "\n") .. "\n"}
        {path: "book.toml", title: "book.toml", text: table.concat(book, "\n") .. "\n"}
      }
    when "none"
      {{path: "SUMMARY.md", title: "SUMMARY.md", text: buildLiterateNav(pages, opts)}}
    else
      {}

---Renders API documentation pages from parsed module IRs.
---@class MoonCatsDocRenderer
class MoonCatsDocRenderer
  ---Renders one documentation page per module, plus an index page and site scaffolding.
  ---@param irs MoonCatsModuleIR[] The run's parsed module IRs.
  ---@param packageSymbols? MoonCatsPackageSymbols Cross-module symbol context from the parse pass.
  ---@param opts? MoonCatsDocOptions
  ---@return MoonCatsDocResult result
  render: (irs, packageSymbols, opts = {}) =>
    state = {
      includePrivate: opts.includePrivate or false
      siteName: opts.siteName or "API Documentation"
      typeNameByRequireId: packageSymbols and packageSymbols.typeNameByRequireId or {}
      linkIndex: {}
      aliasSegmentsByName: {}
      -- standalone sites (mkdocs/mdbook) keep pages in a docs/ subdir beside their config;
      -- the embeddable "none" section drops them flat, to sit inside a host site's docs dir
      docsPrefix: (opts.site or "mkdocs") == "none" and "" or "docs/"
      -- content tabs need pymdownx.tabbed, configured only on the embeddable "none" host;
      -- the standalone scaffolds fall back to one inline block
      tabbedForms: (opts.site or "mkdocs") == "none"
    }
    buildLinkIndex state, irs

    sorted = [ir for ir in *irs]
    table.sort sorted, (a, b) -> a.requireId < b.requireId
    pages = [renderModulePage state, ir for ir in *sorted]
    indexPage = renderIndexPage state, sorted, opts
    scaffold = buildScaffold state, pages, opts
    {:pages, :indexPage, :scaffold}

UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
return UnitTestSuite\withTestExports MoonCatsDocRenderer, {
  :receiverName, :linkifyType, :moduleVarName, :callForms, :resolveDataType
}
