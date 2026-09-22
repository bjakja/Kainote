Diagnostics = require "l0.MoonCats.Diagnostics"
Parser = require "l0.MoonCats.Parser"
annotations = require "l0.MoonCats.annotations"

FindingCode = Diagnostics.FindingCode
{:MemberKind, :ExportKind, :SymbolKind, :ValueKind} = Parser
{:extractTypeText, :parseParamTag, :parseReturnTag, :collectBlockTags, :blockDocumentsFunction
  :blockContainsPrivateTag, :flattenBlockProse, :blockOwnType, :guardFunType
  :inferLiteralType, :inferExpressionType} = annotations

---The cross-module context the emitter resolves identifier references against.
---@class MoonCatsPackageSymbols
---@field typeNameByRequireId table<string, string> LuaCATS type name each require identifier's export declares.
---@field aliases table<string, boolean> Names of @alias declarations anywhere in the extraction run.

---Appends one line to the emission buffer.
---@param state table
---@param line string
push = (state, line) ->
  table.insert state.out, line

---Appends a block of annotation lines verbatim.
---@param state table
---@param blockLines? string[]
pushBlock = (state, blockLines) ->
  if blockLines
    for line in *blockLines
      push state, line

---Records a finding when a diagnostics collection is attached.
---@param state table
---@param code MoonCatsFindingCode
---@param line? integer
---@param ... any Message format values.
report = (state, code, line, ...) ->
  state.diagnostics\add code, state.ir.requireId, line, ... if state.diagnostics

---Collects the field names a class annotation block already declares.
---@param blockLines? string[]
---@return table<string, boolean> fieldNames
collectFieldNames = (blockLines) ->
  names = {}
  if blockLines
    for line in *blockLines
      name = line\match "^%-%-%-@field%s+([%w_]+)"
      names[name] = true if name
  names

---Reports whether a member, field, or augmentation holds plain data — neither a function
---literal nor a block documenting a callable contract.
---@param member MoonCatsMemberIR|MoonCatsFieldIR|MoonCatsAugmentationIR
---@return boolean plainData
isPlainData = (member) ->
  return false if member.valueInfo and member.valueInfo.kind == ValueKind.Function
  not blockDocumentsFunction member.block

---Sanitizes a require identifier's leaf into a Lua identifier for the module local.
---@param requireId string
---@return string name
moduleVarName = (requireId) ->
  leaf = requireId\match("([^%.]+)$") or requireId
  name = leaf\gsub "[^%w_]", "_"
  name = "_" .. name if name\match "^%d"
  name

---Resolves an identifier to the LuaCATS type name of the value it holds.
---@param name string
---@param state table
---@param cls? MoonCatsClassIR Class scope to search before module scope.
---@param depth? integer Reference-chase depth guard.
---@return string? typeName Nil when the identifier cannot be resolved.
resolveRefType = (name, state, cls, depth = 0) ->
  return nil if depth > 4 or name == nil
  sym = (cls and cls.localSymbols[name]) or state.ir.symbols[name]
  return nil unless sym
  switch sym.kind
    when SymbolKind.Class
      sym.class.typeName
    when SymbolKind.Require
      typeName = state.packageSymbols.typeNameByRequireId[sym.requireId]
      unless typeName
        report state, FindingCode.ExternalRequire, nil, sym.requireId
      typeName
    when SymbolKind.Enum
      -- An exported enum resolves to its synthesized <Name>Enum class. An unexported module-local,
      -- reached only through the returned table, resolves to the base Enum type.
      sym.enum.exportedAs and "#{sym.enum.name}Enum" or "Enum"
    when SymbolKind.Reference
      resolveRefType sym.target, state, cls, depth + 1
    when SymbolKind.Literal
      sym.literalKind != "nil" and inferLiteralType(sym.token, sym.literalKind) or nil
    when SymbolKind.Table
      "table"
    else
      nil

---Derives the emitted type for a data member's value expression.
---@param valueInfo? MoonCatsValueInfo
---@param state table
---@param cls? MoonCatsClassIR
---@param name string Member name, for findings.
---@param line? integer
---@return string typeText Falls back to "any" with a finding when unresolvable.
typeFromValueInfo = (valueInfo, state, cls, name, line) ->
  return "any" unless valueInfo
  switch valueInfo.kind
    when ValueKind.Literal
      if valueInfo.literalKind == "nil"
        "any"
      else
        inferLiteralType valueInfo.token, valueInfo.literalKind
    when ValueKind.Reference
      typeName = resolveRefType valueInfo.refName, state, cls
      unless typeName
        report state, FindingCode.UnresolvedReference, line, valueInfo.refName
      typeName or "any"
    when ValueKind.Call
      -- setmetatable(t, mt) returns t; the common `setmetatable {}, …` form is a table
      if valueInfo.baseName == "setmetatable"
        "table"
      else
        typeName = resolveRefType valueInfo.baseName, state, cls
        unless typeName
          report state, FindingCode.UnresolvedReference, line, name
        typeName or "any"
    when ValueKind.Table
      "table"
    when ValueKind.Expression
      inferExpressionType valueInfo.node
    else
      report state, FindingCode.UnresolvedReference, line, name
      "any"

---Emits a standalone alias or annotation-only class declaration verbatim.
---@param state table
---@param segment MoonCatsSegment
emitSegment = (state, segment) ->
  pushBlock state, segment.lines
  push state, ""

---Builds the ---@overload line for a class from its constructor's signature and documented types.
---@param cls MoonCatsClassIR
---@param ctor? MoonCatsMemberIR
---@param state table
---@return string overloadLine
synthesizeOverload = (cls, ctor, state) ->
  unless ctor
    if cls.hasParent
      report state, FindingCode.CtorOverloadFallback, cls.line, cls.typeName
      return "---@overload fun(...): #{cls.typeName}"
    return "---@overload fun(): #{cls.typeName}"
  tags, _ = collectBlockTags ctor.block
  typesByName = {tag.name, tag for tag in *tags}
  parts = for param in *ctor.params
    tag = typesByName[param.name]
    paramType = guardFunType tag and tag.type or "any"
    if param.isVararg
      "...: #{paramType}"
    else
      optional = (tag and tag.optional or param.hasDefault) and "?" or ""
      "#{param.name}#{optional}: #{paramType}"
  "---@overload fun(#{table.concat parts, ", "}): #{cls.typeName}"

---Synthesizes the @field line documenting a data member on its owner's class header.
---@param state table
---@param cls? MoonCatsClassIR Class scope for reference resolution.
---@param member MoonCatsMemberIR|MoonCatsFieldIR|MoonCatsAugmentationIR
---@return string fieldLine
synthesizeDataField = (state, cls, member) ->
  fieldType = blockOwnType member.block
  unless fieldType
    fieldType = if member.enum
      "#{member.enum.name}Enum"
    else
      typeFromValueInfo member.valueInfo, state, cls, member.name, member.line
  visibility = (member.isPrivate or member.name\match("^__") != nil) and "private " or ""
  description = flattenBlockProse member.block
  description = " " .. description if #description > 0
  "---@field #{visibility}#{member.name} #{fieldType}#{description}"

---Resolves the exported Enum an augmentation re-exports, when it does.
---@param state table
---@param aug MoonCatsAugmentationIR
---@return MoonCatsEnumIR? enum The re-exported enum, or nil when the augmentation is not an enum re-export.
augmentationEnum = (state, aug) ->
  return aug.enum if aug.enum
  return nil unless aug.valueInfo.kind == ValueKind.Reference
  sym = state.ir.symbols[aug.valueInfo.refName]
  sym and sym.kind == SymbolKind.Enum and sym.enum.exportedAs == aug.name and sym.enum or nil

---Resolves a member holding a bare reference to a local function into an emittable function
---member carrying the local's signature and annotation block. Members with their own callable
---contract block are left alone — the block stays authoritative for them.
---@param state table
---@param cls? MoonCatsClassIR Class scope to search before module scope.
---@param member MoonCatsMemberIR|MoonCatsFieldIR|MoonCatsAugmentationIR
---@return MoonCatsMemberIR? fnMember Nil when the member is not an undocumented function reference.
resolveFunctionReference = (state, cls, member) ->
  valueInfo = member.valueInfo
  return nil unless valueInfo and valueInfo.kind == ValueKind.Reference
  return nil if blockDocumentsFunction member.block
  sym = (cls and cls.localSymbols[valueInfo.refName]) or state.ir.symbols[valueInfo.refName]
  return nil unless sym and sym.kind == SymbolKind.Function
  {
    kind: sym.arrow == "fat" and MemberKind.StaticMethodFat or MemberKind.StaticMethodThin
    name: member.name, line: member.line
    block: member.block or sym.block
    params: sym.params, hasExplicitValueReturn: sym.hasExplicitValueReturn
    isPrivate: member.isPrivate or member.name\match("^__") != nil
    arrow: sym.arrow
  }

---Appends @field lines for every plain-data member and augmentation an owner carries, skipping
---names its block already declares.
---@param state table
---@param cls? MoonCatsClassIR Class scope for reference resolution; nil for table modules.
---@param members table[] Data-bearing members/fields in source order.
---@param augmentations table[] Module-level augmentations owned by this class or table.
---@param fieldNames table<string, boolean> Declared field names; updated in place.
---@param fieldLines string[] Receives the synthesized lines.
collectDataFields = (state, cls, members, augmentations, fieldNames, fieldLines) ->
  dataKinds = {
    [MemberKind.StaticData]: true
    [MemberKind.InstanceDefault]: true
    [MemberKind.EnumExport]: true
  }
  for member in *members
    -- table-module fields carry no member kind; class members filter to the data kinds
    continue unless member.kind == nil or dataKinds[member.kind]
    continue if fieldNames[member.name] or not isPlainData member
    -- references to local functions emit as functions at their position, not as fields
    continue if resolveFunctionReference state, cls, member
    table.insert fieldLines, synthesizeDataField state, cls, member
    fieldNames[member.name] = true
  for aug in *augmentations
    continue if fieldNames[aug.name] or not isPlainData aug
    continue if resolveFunctionReference state, cls, aug
    aug.enum = augmentationEnum state, aug
    table.insert fieldLines, synthesizeDataField state, cls, aug
    fieldNames[aug.name] = true

---Checks a documented function member's tags against its real signature.
---@param state table
---@param member MoonCatsMemberIR
validateFunctionContract = (state, member) ->
  unless member.block
    unless member.isPrivate
      report state, FindingCode.MissingDoc, member.line, member.name
    return
  return unless member.params
  tags, returns = collectBlockTags member.block
  if #tags != #member.params
    report state, FindingCode.ParamCountMismatch, member.line, #tags, #member.params, member.name
  else
    for i, param in ipairs member.params
      if tags[i].name != param.name
        report state, FindingCode.ParamNameMismatch, member.line, i, tags[i].name, param.name
        break
  if member.hasExplicitValueReturn and #returns == 0
    report state, FindingCode.MissingReturn, member.line, member.name

---Builds an ---@overload line describing a folded duplicate member's contract.
---@param state table
---@param cls MoonCatsClassIR
---@param member MoonCatsMemberIR
---@return string? overloadLine Nil when the member documents no callable contract.
buildDuplicateOverload = (state, cls, member) ->
  return nil unless blockDocumentsFunction member.block
  tags, returns = collectBlockTags member.block
  parts = {}
  -- fat members take the class or an instance as self
  isFat = member.kind == MemberKind.Method or member.kind == MemberKind.StaticMethodFat or
    member.kind == MemberKind.Constructor
  table.insert parts, "self: #{cls.typeName}" if isFat
  for tag in *tags
    optional = tag.optional and "?" or ""
    if tag.name == "..."
      table.insert parts, "...: #{guardFunType tag.type}"
    else
      table.insert parts, "#{tag.name}#{optional}: #{guardFunType tag.type}"
  guardedReturns = [guardFunType returnTag.type for returnTag in *returns]
  returnsText = #guardedReturns > 0 and ": #{table.concat guardedReturns, ", "}" or ""
  "---@overload fun(#{table.concat parts, ", "})#{returnsText}"

---Groups a class's same-name members, choosing a primary declaration and folding documented
---secondaries into overload lines on it.
---@param state table
---@param cls MoonCatsClassIR
---@return MoonCatsMemberIR[] members The class's members with duplicates removed.
dedupeMembers = (state, cls) ->
  groups = {}
  for member in *cls.members
    continue if member.kind == MemberKind.Metamethod
    groups[member.name] or= {}
    table.insert groups[member.name], member

  deduped = {}
  for member in *cls.members
    continue if member.kind == MemberKind.Metamethod
    group = groups[member.name]
    if #group == 1
      table.insert deduped, member
      continue
    -- pick the documented declaration, else the one with more parameters, else the earliest
    primary = group[1]
    for candidate in *group
      continue if candidate == primary
      better = if (candidate.block != nil) != (primary.block != nil)
        candidate.block != nil
      elseif #(candidate.params or {}) != #(primary.params or {})
        #(candidate.params or {}) > #(primary.params or {})
      else
        candidate.line < primary.line
      primary = candidate if better
    if member == primary
      primary.__overloads = {}
      for candidate in *group
        continue if candidate == primary or not candidate.block
        overload = buildDuplicateOverload state, cls, candidate
        if overload
          table.insert primary.__overloads, overload
          report state, FindingCode.DuplicateMemberFolded, candidate.line, candidate.name
      table.insert deduped, primary
  deduped

---Emits a function member's annotation block and `function` statement, and records any contract findings.
---@param state table
---@param owner string
---@param member MoonCatsMemberIR
---@param sep string ":" for self-taking functions, "." for plain ones.
emitFunction = (state, owner, member, sep) ->
  validateFunctionContract state, member
  push state, "---@private" if member.isPrivate and not blockContainsPrivateTag member.block
  pushBlock state, member.block
  pushBlock state, member.__overloads
  paramNames = [param.name for param in *member.params or {}]
  push state, "function #{owner}#{sep}#{member.name}(#{table.concat paramNames, ", "}) end"
  push state, ""

---Emits a member that resolves to a function — a function literal, or a data member whose block
---documents a callable contract — as a `function` statement.
---@param state table
---@param owner string
---@param member MoonCatsMemberIR|MoonCatsFieldIR|MoonCatsAugmentationIR
---@param defaultSep string Separator to use when the block alone defines the contract.
emitFunctionValued = (state, owner, member, defaultSep) ->
  isPrivate = member.isPrivate or member.name\match("^__") != nil
  if member.valueInfo and member.valueInfo.kind == ValueKind.Function
    sep = member.valueInfo.arrow == "fat" and ":" or "."
    fnMember = {
      kind: member.valueInfo.arrow == "fat" and MemberKind.StaticMethodFat or MemberKind.StaticMethodThin
      name: member.name, line: member.line, block: member.block
      params: member.params, hasExplicitValueReturn: member.hasExplicitValueReturn
      :isPrivate
    }
    emitFunction state, owner, fnMember, sep
    return
  tags, _ = collectBlockTags member.block
  push state, "---@private" if isPrivate and not blockContainsPrivateTag member.block
  pushBlock state, member.block
  paramNames = [tag.name for tag in *tags]
  push state, "function #{owner}#{defaultSep}#{member.name}(#{table.concat paramNames, ", "}) end"
  push state, ""

---Emits the synthesized class describing an exported Enum's members, once per enum.
---@param state table
---@param enum MoonCatsEnumIR
emitEnumClass = (state, enum) ->
  enumClassName = "#{enum.name}Enum"
  return if state.synthesizedEnums[enumClassName]
  state.synthesizedEnums[enumClassName] = true
  memberType = state.packageSymbols.aliases[enum.name] and enum.name or nil
  push state, "---@class #{enumClassName}: Enum"
  for enumMember in *enum.members
    fieldType = memberType or enumMember.literal
    -- a bare negative literal is not a valid LuaCATS type; fall back to integer
    fieldType = "integer" if not memberType and fieldType\sub(1, 1) == "-"
    push state, "---@field #{enumMember.key} #{fieldType}"
  push state, ""
  if enum.computedKeyCount > 0
    report state, FindingCode.ComputedKeySkipped, enum.line, enum.name

---Emits a class's annotation block and the `local` table its members attach to.
---@param state table
---@param cls MoonCatsClassIR
emitClassHeader = (state, cls) ->
  ctor = nil
  for member in *cls.members
    ctor = member if member.kind == MemberKind.Constructor
  cls.__fieldNames = collectFieldNames cls.block

  headerLines = {}
  if cls.block
    -- fold the constructor's prose into the class doc, ahead of the @class line
    firstTagAt = nil
    for i, line in ipairs cls.block
      if line\match "^%-%-%-@"
        firstTagAt = i
        break
    firstTagAt or= #cls.block + 1
    for i = 1, firstTagAt - 1
      table.insert headerLines, cls.block[i]
    if ctor and ctor.block
      for line in *ctor.block
        table.insert headerLines, line if line\match("^%-%-%-[^@|]") and not line\match "^%-%-%-%s*$"
    for i = firstTagAt, #cls.block
      line = cls.block[i]
      -- reattach a parent the annotation omits but the class statement declares
      if cls.hasParent and not cls.annotatedParentName and line\match "^%-%-%-@class%s"
        parentType = resolveRefType(cls.parentName, state, nil) or cls.parentName
        line = "#{line}: #{parentType}" if parentType
      table.insert headerLines, line
  else
    if ctor and ctor.block
      for line in *ctor.block
        table.insert headerLines, line if line\match("^%-%-%-[^@|]") and not line\match "^%-%-%-%s*$"
    parentType = cls.hasParent and (resolveRefType(cls.parentName, state, nil) or cls.parentName) or nil
    classLine = parentType and "---@class #{cls.typeName}: #{parentType}" or "---@class #{cls.typeName}"
    table.insert headerLines, classLine

  ownAugmentations = [aug for aug in *state.ir.augmentations when aug.targetName == cls.name]
  collectDataFields state, cls, cls.__dedupedMembers or cls.members, ownAugmentations, cls.__fieldNames, headerLines
  table.insert headerLines, synthesizeOverload cls, ctor, state

  pushBlock state, headerLines
  push state, "local #{cls.name} = {}"
  push state, ""

  validateFunctionContract state, ctor if ctor

---Emits one class member at its source position.
---@param state table
---@param owner string The local the member is emitted onto.
---@param member MoonCatsMemberIR
---@param cls MoonCatsClassIR
emitMember = (state, owner, member, cls) ->
  switch member.kind
    when MemberKind.Constructor
      nil -- folded into the class header
    when MemberKind.AccessorProperty
      unless cls.__fieldNames and cls.__fieldNames[member.name]
        report state, FindingCode.AccessorFieldMissing, member.line, member.name, cls.typeName
    when MemberKind.EnumExport
      -- the exporting @field lives on the class header; only the enum's class is emitted here
      emitEnumClass state, member.enum
    when MemberKind.Method, MemberKind.StaticMethodFat
      emitFunction state, owner, member, ":"
    when MemberKind.StaticMethodThin
      emitFunction state, owner, member, "."
    when MemberKind.InstanceDefault, MemberKind.StaticData
      -- plain data became @field lines on the class header; callables emit here
      return if cls.__fieldNames and cls.__fieldNames[member.name]
      fnMember = resolveFunctionReference state, cls, member
      if fnMember
        emitFunction state, owner, fnMember, fnMember.arrow == "fat" and ":" or "."
      elseif not isPlainData member
        emitFunctionValued state, owner, member, "."

---Emits a table module's annotation block and the `local` table its members attach to.
---@param state table
---@param varName string
emitTableModuleHeader = (state, varName) ->
  exportIR = state.ir.export
  headerLines = {"---@class #{state.ir.requireId}"}
  fieldNames = {}
  ownAugmentations = [aug for aug in *state.ir.augmentations when aug.targetName == exportIR.name or aug.targetName == varName]
  collectDataFields state, nil, exportIR.fields or {}, ownAugmentations, fieldNames, headerLines
  pushBlock state, headerLines
  push state, "local #{varName} = {}"
  push state, ""

---Emits a module-level augmentation onto its owner — an enum re-export's synthesized class, or a
---function's statement. Plain-data augmentations are already declared on the owner's header, so
---they emit nothing here.
---@param state table
---@param owner string
---@param aug MoonCatsAugmentationIR
emitAugmentation = (state, owner, aug) ->
  enum = augmentationEnum state, aug
  if enum
    emitEnumClass state, enum
    return
  fnRef = resolveFunctionReference state, nil, aug
  if fnRef
    emitFunction state, owner, fnRef, fnRef.arrow == "fat" and ":" or "."
  elseif not isPlainData aug
    emitFunctionValued state, owner, aug, "."

---Emits a function module's exported function as a documented `local function` statement.
---@param state table
---@param exportIR MoonCatsExportIR
emitFunctionModule = (state, exportIR) ->
  member = {
    kind: MemberKind.StaticMethodThin
    name: exportIR.name, line: 0
    block: exportIR.block, params: exportIR.params
    hasExplicitValueReturn: exportIR.hasExplicitValueReturn
    isPrivate: false
  }
  validateFunctionContract state, member
  pushBlock state, member.block
  paramNames = [param.name for param in *member.params or {}]
  push state, "local function #{exportIR.name}(#{table.concat paramNames, ", "}) end"
  push state, ""

---Emits LuaLS .d.lua type definitions from parsed module IRs.
---@class MoonCatsEmitter
class MoonCatsEmitter
  ---Emits the definition definition for one parsed module.
  ---@param ir MoonCatsModuleIR The module IR produced by the parser.
  ---@param packageSymbols? MoonCatsPackageSymbols Cross-module symbol context; omit for single-module runs.
  ---@param diagnostics? MoonCatsDiagnostics Collection that receives findings discovered while emitting.
  ---@return string definition The definition file's text.
  emit: (ir, packageSymbols, diagnostics) =>
    state = {
      :ir
      packageSymbols: packageSymbols or {typeNameByRequireId: {}, aliases: {}}
      :diagnostics
      out: {}
      synthesizedEnums: {}
    }
    -- module-local aliases count like run-wide ones for enum field typing
    for name in pairs ir.aliases
      state.packageSymbols.aliases[name] = true

    push state, "---@meta #{ir.requireId}"
    push state, ""

    items = @@__collectItems state
    table.sort items, (a, b) ->
      return a.line < b.line if a.line != b.line
      a.ordinal < b.ordinal
    for item in *items
      item.emit!

    returnName = @@__resolveReturnName state
    push state, "return #{returnName}" if returnName

    text = table.concat state.out, "\n"
    text ..= "\n" unless text\sub(-1) == "\n"
    chunk, loadErr = (loadstring or load) text
    unless chunk
      report state, FindingCode.EmitFailure, nil, tostring loadErr
    text

  ---Builds the line-ordered emission worklist: standalone segments, classes and their members,
  ---table-module fields, and augmentations.
  ---@private
  ---@param state table
  ---@return {line: integer, ordinal: integer, emit: fun()}[] items
  @__collectItems = (state) =>
    {:ir} = state
    items = {}
    ordinal = 0
    add = (line, emit) ->
      ordinal += 1
      table.insert items, {line: line or 0, :ordinal, :emit}

    for segment in *ir.segments
      add segment.line, -> emitSegment state, segment

    for cls in *ir.classes
      cls.__dedupedMembers = dedupeMembers state, cls
      add cls.line, -> emitClassHeader state, cls
      for member in *cls.__dedupedMembers
        add member.line, -> emitMember state, cls.name, member, cls

    exportIR = ir.export
    if exportIR.kind == ExportKind.Table
      varName = exportIR.name and exportIR.name\match("^[%w_]+$") and exportIR.name or moduleVarName ir.requireId
      state.tableVarName = varName
      -- the module table must precede its fields; plain-data fields live on its header
      add -1, -> emitTableModuleHeader state, varName
      for field in *exportIR.fields or {}
        if field.enum
          add field.line, -> emitEnumClass state, field.enum
          continue
        fnRef = resolveFunctionReference state, nil, field
        if fnRef
          add field.line, -> emitFunction state, varName, fnRef, fnRef.arrow == "fat" and ":" or "."
        elseif not isPlainData field
          add field.line, -> emitFunctionValued state, varName, field, "."
    elseif exportIR.kind == ExportKind.Function
      add #ir.lines, -> emitFunctionModule state, exportIR

    for aug in *ir.augmentations
      cls = ir.classesByName[aug.targetName]
      owner = if cls
        cls.name
      elseif state.tableVarName and (aug.targetName == exportIR.name or aug.targetName == state.tableVarName)
        state.tableVarName
      if owner
        add aug.line, -> emitAugmentation state, owner, aug

    items

  ---Answers the local name the module's trailing return should reference.
  ---@private
  ---@param state table
  ---@return string? returnName Nil when the module's export is unknown.
  @__resolveReturnName = (state) =>
    exportIR = state.ir.export
    switch exportIR.kind
      when ExportKind.Class
        exportIR.class and exportIR.class.name or exportIR.name
      when ExportKind.Table
        state.tableVarName
      when ExportKind.Function
        exportIR.name
      else
        nil

UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
return UnitTestSuite\withTestExports MoonCatsEmitter, {
  :inferLiteralType, :inferExpressionType, :moduleVarName, :synthesizeOverload
  :dedupeMembers, :buildDuplicateOverload
}
