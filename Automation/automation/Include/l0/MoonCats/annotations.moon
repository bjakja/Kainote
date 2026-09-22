-- Shared LuaCATS tag-line parsing helpers used by both the definition emitter and the
-- documentation renderer.

---A LuaCATS @param tag's parsed contract.
---@class MoonCatsParamTag
---@field name string Parameter name with any trailing ? stripped.
---@field optional boolean
---@field type string The parameter's type expression.

---A LuaCATS @return tag's parsed contract.
---@class MoonCatsReturnTag
---@field type string The return's type expression.
---@field name? string The return's name, when documented.
---@field description? string Description text following the name.

---A LuaCATS @field tag's parsed contract.
---@class MoonCatsFieldTag
---@field name string Field name with any trailing ? stripped.
---@field optional boolean
---@field visibility? string "private", "protected", "public", or "package" when tagged.
---@field type string The field's type expression.
---@field description? string Description text following the type.

---One variant line of an @alias declaration.
---@class MoonCatsAliasVariant
---@field value string The variant's literal value, verbatim.
---@field key? string The member name from a "# Key: description" comment.
---@field description? string The comment's description text.

visibilityKeywords = {private: true, protected: true, public: true, package: true}

---Extracts the leading type expression from a tag line remainder, balancing brackets so
---function types, inline tables, and generics survive; a token ending in ":" or "," pulls
---in the next token (fun return lists).
---@param text string Everything after the tag's name field.
---@return string typeText The type expression, or "any" when none could be read.
---@return string restText The remainder following the type expression, whitespace-trimmed.
extractTypeText = (text) ->
  pos, len = 1, #text
  depthRound, depthCurly, depthAngle = 0, 0, 0
  readToken = ->
    start = pos
    while pos <= len
      char = text\sub pos, pos
      switch char
        when "(" then depthRound += 1
        when ")" then depthRound -= 1
        when "{" then depthCurly += 1
        when "}" then depthCurly -= 1
        when "<" then depthAngle += 1
        when ">" then depthAngle -= 1
        when " ", "\t"
          break if depthRound <= 0 and depthCurly <= 0 and depthAngle <= 0
      pos += 1
    text\sub start, pos - 1

  skipSpace = ->
    while pos <= len and text\sub(pos, pos)\match "%s"
      pos += 1

  skipSpace!
  token = readToken!
  return "any", "" if token == ""
  -- a token ending in ":" (fun return list) or "," (multi-return continuation) continues
  while token\sub(-1) == ":" or token\sub(-1) == ","
    skipSpace!
    nextToken = readToken!
    break if nextToken == ""
    token ..= " " .. nextToken
  rest = text\sub(pos)\gsub("^%s+", "")\gsub "%s+$", ""
  token, rest

---Parses a ---@param tag line into its contract.
---@param line string
---@return MoonCatsParamTag? tag Nil when the line is not a @param tag.
parseParamTag = (line) ->
  name, remainder = line\match "^%-%-%-@param%s+(%S+)%s*(.*)$"
  return nil unless name
  optional = name\sub(-1) == "?"
  name = name\sub 1, -2 if optional
  typeText, description = extractTypeText remainder
  {:name, :optional, type: typeText, description: #description > 0 and description or nil}

---Parses a ---@return tag line into its contract. The first word after the type is taken
---as the return's name, matching how the language server reads the tag.
---@param line string
---@return MoonCatsReturnTag? tag Nil when the line is not a @return tag.
parseReturnTag = (line) ->
  remainder = line\match "^%-%-%-@return%s+(.*)$"
  return nil unless remainder
  typeText, rest = extractTypeText remainder
  name, description = rest\match "^([%w_%.]+)%s*(.*)$"
  if name
    {type: typeText, :name, description: #description > 0 and description or nil}
  else
    {type: typeText, description: #rest > 0 and rest or nil}

---Parses a ---@field tag line into its contract, honoring a leading visibility keyword.
---@param line string
---@return MoonCatsFieldTag? tag Nil when the line is not a @field tag.
parseFieldTag = (line) ->
  rest = line\match "^%-%-%-@field%s+(.*)$"
  return nil unless rest
  visibility = nil
  firstWord = rest\match "^(%l+)%s"
  if firstWord and visibilityKeywords[firstWord]
    visibility = firstWord
    rest = rest\gsub "^%l+%s+", ""
  name, remainder = rest\match "^(%S+)%s*(.*)$"
  return nil unless name
  optional = name\sub(-1) == "?"
  name = name\sub 1, -2 if optional
  typeText, description = extractTypeText remainder
  {:name, :optional, :visibility, type: typeText, description: #description > 0 and description or nil}

---Parses an @alias declaration's ---| variant lines.
---@param blockLines string[] The alias segment's lines (non-variant lines are skipped).
---@return MoonCatsAliasVariant[] variants In declaration order.
parseAliasVariants = (blockLines) ->
  variants = {}
  for line in *blockLines
    value, comment = line\match "^%-%-%-|%s*(.-)%s*#%s*(.*)$"
    value = line\match "^%-%-%-|%s*(.-)%s*$" unless value
    continue unless value and #value > 0
    variant = {:value}
    if comment
      key, description = comment\match "^([%w_]+):%s*(.*)$"
      if key
        variant.key = key
        variant.description = #description > 0 and description or nil
      else
        variant.description = #comment > 0 and comment or nil
    table.insert variants, variant
  variants

---Collects the @param and @return tags of an annotation block.
---@param blockLines? string[]
---@return MoonCatsParamTag[] params In tag order.
---@return MoonCatsReturnTag[] returns In tag order.
collectBlockTags = (blockLines) ->
  params, returns = {}, {}
  if blockLines
    for line in *blockLines
      tag = parseParamTag line
      if tag
        table.insert params, tag
      else
        returnTag = parseReturnTag line
        table.insert returns, returnTag if returnTag
  params, returns

---Reports whether an annotation block documents a function (has @param or @return tags).
---@param blockLines? string[]
---@return boolean documentsFunction
blockDocumentsFunction = (blockLines) ->
  return false unless blockLines
  for line in *blockLines
    return true if line\match("^%-%-%-@param%s") or line\match "^%-%-%-@return%s"
  false

---Reports whether a block already carries an @private tag.
---@param blockLines? string[]
---@return boolean tagged
blockContainsPrivateTag = (blockLines) ->
  return false unless blockLines
  for line in *blockLines
    return true if line\match "^%-%-%-@private"
  false

---Collects a block's prose lines (untagged, non-variant), stripped of the comment lead but
---otherwise verbatim: indentation and blank separator lines survive, so markdown constructs
---like indented code examples keep working. Boundary blank lines are trimmed.
---@param blockLines? string[]
---@return string[] lines In block order; empty when the block has no prose.
proseLines = (blockLines) ->
  lines = {}
  if blockLines
    for line in *blockLines
      remainder = line\match "^%-%-%-(.*)$"
      continue unless remainder
      -- only an immediately following @ or | marks a tag/variant line; an indented
      -- one is content (e.g. a @{template} example inside an indented code block)
      head = remainder\sub 1, 1
      continue if head == "@" or head == "|"
      table.insert lines, remainder
  while #lines > 0 and lines[1]\match "^%s*$"
    table.remove lines, 1
  while #lines > 0 and lines[#lines]\match "^%s*$"
    table.remove lines
  lines

---Joins a block's prose lines into a single-line description for a @field tag.
---@param blockLines? string[]
---@return string prose Empty when the block has no prose.
flattenBlockProse = (blockLines) ->
  return "" unless blockLines
  parts = {}
  for line in *blockLines
    prose = line\match "^%-%-%-%s*([^@|%s].*)$"
    table.insert parts, prose if prose
  table.concat parts, " "

---Extracts the type expression of a block's own @type tag.
---@param blockLines? string[]
---@return string? typeText Nil when the block carries no @type tag.
blockOwnType = (blockLines) ->
  return nil unless blockLines
  for line in *blockLines
    remainder = line\match "^%-%-%-@type%s+(.*)$"
    if remainder
      typeText = extractTypeText remainder
      return typeText
  nil

---Extracts the text of a block's first occurrence of the given tag.
---@param blockLines? string[]
---@param tag string Tag name without the @ (e.g. "deprecated", "see").
---@return string? text The tag's trailing text, empty for a bare tag, or nil when the tag is absent.
blockTagText = (blockLines, tag) ->
  return nil unless blockLines
  for line in *blockLines
    text = line\match "^%-%-%-@#{tag}%s*(.*)$"
    return text if text
  nil

---Parenthesizes a type expression containing a fun type, whose return list would otherwise
---swallow the comma before the next parameter or return in an assembled signature.
---@param typeText string
---@return string safeText
guardFunType = (typeText) ->
  return typeText unless typeText\find "fun(", 1, true
  return typeText if typeText\match "^%b()$"
  "(#{typeText})"

---Infers a LuaCATS type for a literal token.
---@param token string
---@param literalKind string Lua type name from the parser.
---@return string typeText
inferLiteralType = (token, literalKind) ->
  switch literalKind
    when "number"
      token\match("^%-?%d+$") and "integer" or "number"
    when "string" then "string"
    when "boolean" then "boolean"
    else "any"

---Guesses a numeric or string result type for an operator expression from its leaf nodes.
---@param node table
---@return string typeText "number" or "string" from the leaf nodes, or "any" when neither appears.
inferExpressionType = (node) ->
  sawNumber, sawString = false, false
  walk = (child) ->
    return unless "table" == type child
    sawNumber = true if child[1] == "number"
    sawString = true if child[1] == "string"
    for _, grandchild in pairs child
      walk grandchild if "table" == type grandchild
  walk node
  if sawString then "string"
  elseif sawNumber then "number"
  else "any"

{
  :extractTypeText, :parseParamTag, :parseReturnTag, :parseFieldTag, :parseAliasVariants
  :collectBlockTags, :blockDocumentsFunction, :blockContainsPrivateTag
  :proseLines, :flattenBlockProse, :blockOwnType, :blockTagText, :guardFunType
  :inferLiteralType, :inferExpressionType
}
