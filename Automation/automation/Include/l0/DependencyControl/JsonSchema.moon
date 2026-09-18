json = require "json"
Logger = require "l0.DependencyControl.Logger"
constants = require "l0.DependencyControl.Constants"
fileOps = require "l0.DependencyControl.file-ops"
SemanticVersion = require "l0.DependencyControl.SemanticVersion"

JSON_SCHEMA_ID_KEYWORD = "$schema"
defaultLogger = Logger fileBaseName: "#{constants.DEPCTRL_SHORT_NAME}.JsonSchema"

-- Lazily resolve lua-schema because it's only available on luarocks, not DepCtrl
luaSchema = nil
patternKeywordShadowed = nil

patternNoFallbackError = "Cannot validate the `pattern` keyword '%s': rex_pcre2 is unavailable and " ..
  "the schema node provides no `lpegPattern` fallback to validate it with."

-- When rex_pcre2 is absent, lua-schema's built-in `pattern` keyword errors out on use. Shadow it
-- so that validation instead relies on a sibling `lpegPattern` keyword (checked via LPeg.re).
-- If an `lpegPattern` sibling field is missing for any `pattern`, an error is raised at schema build time.
shadowPatternKeyword = (luaSchemaLib) ->
  havePcre2 = pcall require, "rex_pcre2"
  if havePcre2
    patternKeywordShadowed = false
    return

  -- custom_keyword takes priority over the built-in keyword of the same name
  luaSchemaLib.custom_keyword["pattern"] = (patternValue, schemaNode) ->
    error patternNoFallbackError\format patternValue unless schemaNode.schema.lpegPattern
    -- passing the `pattern` check leaves the real validation to `lpegPattern`
    (schema, _, dataPtr) -> schema\_mk_output true, nil, "pattern", dataPtr, schema.validation
  patternKeywordShadowed = true

---Loads and memoizes the lua-schema library, installing the `pattern`-keyword shadow on first successful load.
---@return table|false luaSchemaLib The lua-schema library, or false when it isn't installed.
loadLuaSchemaLib = ->
  return luaSchema unless luaSchema == nil
  ok, lib = pcall require, "schema"
  luaSchema = ok and lib or false
  shadowPatternKeyword luaSchema if luaSchema
  return luaSchema

---Flattens lua-schema's `detailed` validation output into a list of "<location>: <error>" strings.
---@param result table A lua-schema result node: a leaf carrying `.error` at `.instanceLocation`, or a branch nesting further results under `.errors`.
---@param acc? string[] Accumulator for the recursive descent (callers omit it).
---@return string[] errors The flattened "<location>: <error>" messages, empty when the node carries none.
collectValidationErrors = (result, acc = {}) ->
  if result.errors
    collectValidationErrors sub, acc for sub in *result.errors
  elseif result.error
    acc[#acc + 1] = "#{result.instanceLocation or '?'}: #{result.error}"
  return acc

msgs = {
  load: {
    errors: {
      read: "Couldn't read JSON schema file '%s': %s"
      jsonParse: "Couldn't parse JSON schema file '%s' as JSON."
      notAnObject: "JSON schema file '%s' did not decode to a JSON object (got %s).",
      badArgument: "Invalid schema argument of type %s (expected table or string file path)."
    }
  }
  getSchemasInDirectory: {
    errors: {
      readDir: "Couldn't read schema directory '%s': %s"
      noSchemasFound: "No schema files found in directory '%s' matching pattern '%s'."
    }
  }
  validate: {
    errors: {
      libMissing: "JSON schema validation requires 'lua-schema'. Manually install it via LuaRocks and/or ensure it's on the Lua path to enable validation."
      genericInvalid: "Data did not conform to schema, but no specific error information is available."
    }
    noPcre: "rex_pcre2 not available — using LPeg.re `lpegPattern` fallback for `pattern` validation."
  }
  validateAny: {
    errors: {
      versionNotFound: "No schema available for version '%s'."
      versionLoadFailed: "Failed to load schema for version '%s': %s"
      validateErrored: "An error occurred while validating against schema version '%s': %s"
      invalid: "Data did not validate against schema version '%s': %s"
      allFailed: "Validation failed against all available schemas (feed version was '%s'). Errors by schema version:\n%s"
    }
  }
}

---JSON schema loading and validation utilities.
---Depends on the `lua-schema` library for validation, which must be manually installed
---via LuaRocks and/or otherwise made available on the Lua path by the user.
---@class JsonSchema
class JsonSchema

  -- The JSON Schema keyword whose value declares which schema a document conforms to (`$schema`).
  @JSON_SCHEMA_ID_KEYWORD = JSON_SCHEMA_ID_KEYWORD

  ---Finds the versioned schema files in a directory, mapping each file's declared version to its path.
  ---@param schemaDir string Directory to scan.
  ---@param fileNamePattern? string Lua pattern matched against each filename, capturing the version (default matches `vX.Y.Z.json`).
  ---@return table<string, string>? schemaPathsByVersion Version → full file path, or nil when the directory can't be read or holds no matching file.
  ---@return string? err The failure message accompanying a nil result.
  @getSchemasInDirectory = (schemaDir, fileNamePattern = "^v(%d+%.%d+%.%d+)%.json$") =>
    schemaDirContents, listErr = fileOps.listDir schemaDir
    unless schemaDirContents
      return nil, msgs.getSchemasInDirectory.errors.readDir\format schemaDir, listErr

    -- map each matching file's captured version (e.g. "0.4.0") to its full path
    schemaPathsByVersion, foundAny = {}, false
    for fileName in *schemaDirContents
      version = fileName\match fileNamePattern
      if version
        schemaPathsByVersion[version] = fileOps.joinPath schemaDir, fileName
        foundAny = true
    unless foundAny
      return nil, msgs.getSchemasInDirectory.errors.noSchemasFound\format schemaDir, fileNamePattern
    return schemaPathsByVersion

  ---Validates data against the best-matching schema from the provided versions. A document's own root
  ---`$schema` names its version authoritatively; otherwise the given version hint is tried first,
  ---then each remaining schema highest-version-first.
  ---@param data table The value to validate.
  ---@param schemasByVersion table<string, string|JsonSchema> Version → schema file path or ready instance.
  ---@param dataSchemaVersion? string|fun(data: table): string? Version hint for data with no `$schema`: a fixed version, or a function that derives it from the data (e.g. a feed reading its own `dependencyControlFeedFormatVersion`, a field that predates `$schema`).
  ---@return boolean? valid True/false on a completed validation, nil if none could run.
  ---@return string? version The schema version validated against, or nil when none matched.
  ---@return string? err Validation errors on failure, or a message when validation couldn't run.
  @validateAny: (data, schemasByVersion, dataSchemaVersion) =>
    trySchemaVersion = (version) ->
      entry = schemasByVersion[version]
      unless entry
        return nil, version, msgs.validateAny.errors.versionNotFound\format version

      -- accept either a ready JsonSchema instance or a path/table to construct one from
      schema = entry
      unless type(entry) == "table" and entry.__class == JsonSchema
        loaded, instanceOrErr = pcall JsonSchema, entry
        unless loaded
          return nil, version, msgs.validateAny.errors.versionLoadFailed\format version, instanceOrErr
        schema = instanceOrErr

      valid, err = schema\validate data
      if valid == nil
        return nil, version, msgs.validateAny.errors.validateErrored\format version, err
      if valid == false
        return false, version, msgs.validateAny.errors.invalid\format version, err
      return true, version

    -- a document's own root `$schema` names its version authoritatively; otherwise fall back to the hint
    declaredVersion = type(data) == "table" and type(data[JSON_SCHEMA_ID_KEYWORD]) == "string" and
      data[JSON_SCHEMA_ID_KEYWORD]\match "v(%d+%.%d+%.%d+)%.json"
    unless declaredVersion
      declaredVersion = if type(dataSchemaVersion) == "function" then dataSchemaVersion(data) else dataSchemaVersion
    dataSchemaVersion = declaredVersion

    errors = {}
    if dataSchemaVersion
      -- try exact schema version used by the feed first
      isValid, validationVersion, validationErr = trySchemaVersion dataSchemaVersion
      return isValid, validationVersion, validationErr if isValid != nil
      errors[validationVersion] = validationErr

    -- no exact match for the feed's version: try the other available ones, highest version
    -- first to avoid skipping validation of fields not present in earlier schema versions
    otherVersions = [version for version in pairs schemasByVersion when version != dataSchemaVersion]
    table.sort otherVersions, SemanticVersion.isHigher
    for version in *otherVersions
      isValid, validationVersion, validationErr = trySchemaVersion version
      return isValid, validationVersion if isValid
      errors[validationVersion] = validationErr

    return nil, nil, msgs.validateAny.errors.allFailed\format tostring(dataSchemaVersion), table.concat(
      ["  v#{v}: #{e}" for v, e in pairs errors], "\n")

  ---Loads and parses a JSON schema, ready to validate against.
  ---@param schemaOrSchemaPath table|string The JSON schema, either as a path to the schema file or a pre-parsed table.
  ---@param logger? Logger Logger for load/parse errors (defaults to a shared JsonSchema logger).
  new: (schemaOrSchemaPath, @logger = defaultLogger) =>
    dataType = type schemaOrSchemaPath
    @data = schemaOrSchemaPath

    -- load a schema JSON file from disk
    if dataType == "string"
      @schemaPath = schemaOrSchemaPath
      raw, err = fileOps.readFile schemaOrSchemaPath
      unless raw
        @logger\error msgs.load.errors.read, schemaOrSchemaPath, err

      decoded, @data = pcall json.decode, raw
      unless decoded
        @logger\error msgs.load.errors.jsonParse, schemaOrSchemaPath, @data
      dataType = type @data

    return if dataType == "table"
    @logger\error @schemaPath and
      msgs.load.errors.notAnObject\format(@schemaPath, dataType) or
      msgs.load.errors.badArgument\format dataType

  ---Validates a Lua value against the loaded schema.
  ---Best-effort: returns the validation result rather than raising, so callers can warn and
  ---continue. Returns `nil` (plus a message) when validation couldn't be performed at all.
  ---@param data table The value to validate.
  ---@return boolean? valid True/false on a completed validation, nil if it couldn't run.
  ---@return string? err The validation errors if validation failed, or an error message when validation could not be performed.
  validate: (data) =>
    lib = loadLuaSchemaLib!
    return nil, msgs.validate.errors.libMissing unless lib
    @logger\debug msgs.validate.noPcre if patternKeywordShadowed

    ok, result = pcall -> lib.new(@data)\validate data
    return nil, result unless ok
    return true if result.valid
    errors = collectValidationErrors result
    return false, #errors > 0 and table.concat(errors, "; ") or msgs.validate.errors.genericInvalid

return JsonSchema
