Enum = require "l0.DependencyControl.Enum"

---@alias MoonCatsSeverity
---| "error" # Error: fails a check run; the source or the extractor needs fixing
---| "warning" # Warning: a construct the extractor degraded gracefully; the definition may be imprecise
---| "info" # Info: a benign note about a fallback taken during emission
Severity = Enum "MoonCatsSeverity", {
  Error: "error"
  Warning: "warning"
  Info: "info"
}

---@alias MoonCatsFindingCode
---| "E-PARSE" # ParseFailure: the MoonScript source failed to parse
---| "E-MISSING-DOC" # MissingDoc: a public function member carries no annotation block
---| "E-PARAM-NAME" # ParamNameMismatch: a documented parameter name differs from the signature
---| "E-PARAM-COUNT" # ParamCountMismatch: the @param tags do not match the signature's parameter count
---| "E-MISSING-RETURN" # MissingReturn: the body explicitly returns a value but no @return is documented
---| "E-EMIT" # EmitFailure: the emitted definition is not loadable Lua
---| "W-COMPUTED-KEY" # ComputedKeySkipped: a member with a computed table key was skipped
---| "W-UNRESOLVED" # UnresolvedReference: an identifier could not be resolved and was typed as any
---| "W-ACCESSOR-FIELD" # AccessorFieldMissing: a computed property has no matching @field on its class
---| "W-CLASS-NAME" # ClassNameMismatch: the @class annotation names a different class than the statement
---| "W-NO-EXPORT" # NoExport: the module's export could not be determined
---| "I-EXTERNAL-REQUIRE" # ExternalRequire: a required module is outside the extraction run and was typed as any
---| "I-NESTED-TABLE" # NestedTableFlattened: a nested table value was flattened to the plain table type
---| "I-CTOR-FALLBACK" # CtorOverloadFallback: a subclass without its own constructor got a fun(...) overload
---| "I-DUPLICATE-MEMBER" # DuplicateMemberFolded: a same-name member was folded into the primary declaration's overloads
FindingCode = Enum "MoonCatsFindingCode", {
  ParseFailure: "E-PARSE"
  MissingDoc: "E-MISSING-DOC"
  ParamNameMismatch: "E-PARAM-NAME"
  ParamCountMismatch: "E-PARAM-COUNT"
  MissingReturn: "E-MISSING-RETURN"
  EmitFailure: "E-EMIT"
  ComputedKeySkipped: "W-COMPUTED-KEY"
  UnresolvedReference: "W-UNRESOLVED"
  AccessorFieldMissing: "W-ACCESSOR-FIELD"
  ClassNameMismatch: "W-CLASS-NAME"
  NoExport: "W-NO-EXPORT"
  ExternalRequire: "I-EXTERNAL-REQUIRE"
  NestedTableFlattened: "I-NESTED-TABLE"
  CtorOverloadFallback: "I-CTOR-FALLBACK"
  DuplicateMemberFolded: "I-DUPLICATE-MEMBER"
}

msgs = {
  add: {
    badCode: "Invalid finding code '%s'."
    templates: {
      ["E-PARSE"]: "failed to parse MoonScript source: %s"
      ["E-MISSING-DOC"]: "public member '%s' has no annotation block"
      ["E-PARAM-NAME"]: "@param %d is named '%s' but the signature says '%s'"
      ["E-PARAM-COUNT"]: "%d @param tag(s) document a %d-parameter signature of '%s'"
      ["E-MISSING-RETURN"]: "'%s' explicitly returns a value but documents no @return"
      ["E-EMIT"]: "emitted definition is not valid Lua: %s"
      ["W-COMPUTED-KEY"]: "computed key skipped in '%s'"
      ["W-UNRESOLVED"]: "cannot resolve '%s'; typed as any"
      ["W-ACCESSOR-FIELD"]: "computed property '%s' has no matching @field on class '%s'"
      ["W-CLASS-NAME"]: "@class annotation names '%s' but the class statement says '%s'"
      ["W-NO-EXPORT"]: "cannot determine the module's export"
      ["I-EXTERNAL-REQUIRE"]: "'%s' resolves outside this extraction run; typed as any"
      ["I-NESTED-TABLE"]: "nested table '%s' flattened to the plain table type"
      ["I-CTOR-FALLBACK"]: "'%s' extends a parent without a constructor of its own; overload emitted as fun(...)"
      ["I-DUPLICATE-MEMBER"]: "'%s' is defined more than once; secondary definitions folded into overloads"
    }
  }
}

severityByCodePrefix = {
  E: Severity.Error
  W: Severity.Warning
  I: Severity.Info
}

---A single finding produced while extracting definitions.
---@class MoonCatsFinding
---@field severity MoonCatsSeverity
---@field code MoonCatsFindingCode
---@field requireId string Require identifier of the module the finding is about.
---@field line? integer 1-based source line the finding anchors to.
---@field message string Fully formatted human-readable description.

---Collects, classifies, and formats the findings produced while extracting LuaCATS definitions.
---@class MoonCatsDiagnostics
---@field findings MoonCatsFinding[] All findings recorded so far, in insertion order.
class MoonCatsDiagnostics
  ---@type Enum
  @Severity = Severity
  ---@type Enum
  @FindingCode = FindingCode

  ---Creates an empty findings collection.
  new: =>
    @findings = {}

  ---Records a finding against a module; its severity derives from the finding code.
  ---@param code MoonCatsFindingCode
  ---@param requireId string Require identifier of the module the finding is about.
  ---@param line? integer 1-based source line the finding anchors to.
  ---@param ... any Values for the finding message's format string.
  ---@return MoonCatsFinding finding The recorded finding.
  add: (code, requireId, line, ...) =>
    template = msgs.add.templates[code]
    error msgs.add.badCode\format tostring code unless template
    finding = {
      severity: severityByCodePrefix[code\sub 1, 1]
      :code, :requireId, :line
      message: template\format ...
    }
    table.insert @findings, finding
    finding

  ---Reports whether any finding is severe enough to fail a check run.
  ---@return boolean failed True when at least one error-severity finding was recorded.
  hasCheckFailures: =>
    for finding in *@findings
      return true if finding.severity == Severity.Error
    false

  ---Returns the number of findings recorded per severity.
  ---@return table<MoonCatsSeverity, integer> counts Keyed by severity value; zero for severities without findings.
  getCounts: =>
    counts = {severity, 0 for severity in *Severity.values}
    counts[finding.severity] += 1 for finding in *@findings
    counts

  ---Formats all findings as one line each, sorted by module, line, and code.
  ---@return string report The formatted findings joined with newlines; empty when there are none.
  format: =>
    sorted = [finding for finding in *@findings]
    table.sort sorted, (a, b) ->
      return a.requireId < b.requireId if a.requireId != b.requireId
      lineA, lineB = a.line or 0, b.line or 0
      return lineA < lineB if lineA != lineB
      a.code < b.code
    lines = for finding in *sorted
      location = finding.line and "#{finding.requireId}:#{finding.line}" or finding.requireId
      "#{finding.severity} #{location} [#{finding.code}] #{finding.message}"
    table.concat lines, "\n"
