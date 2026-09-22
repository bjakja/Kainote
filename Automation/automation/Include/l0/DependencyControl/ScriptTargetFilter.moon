domain = require "l0.DependencyControl.domain"
Accessors = require "l0.DependencyControl.Accessors"

-- fresh copy of all script-type values, so the in-place sort below can't mutate the Enum's own list
scriptTypeList = [v for v in *domain.ScriptType.values]
table.sort scriptTypeList

---Selects which packages a feed operation should process, by script type and namespace.
---Construct it from a spec table, or empty and build it up fluently — every builder method
---returns self so calls can be chained. Because modules and automation scripts aren't required
---to have unique namespaces, rules are keyed by script type first.
---
---    ScriptTargetFilter!\include(domain.ScriptType.Module, "l0.DependencyControl")
---    ScriptTargetFilter!\includeAll domain.ScriptType.Module   -- every module
---    ScriptTargetFilter!\includeAll!                            -- everything
---    ScriptTargetFilter {[domain.ScriptType.Module]: {include: {"l0.DependencyControl"}}}
---@class ScriptTargetFilter
---@field scriptTypes ScriptType[] The script types this filter would process (those carrying any rule), sorted. Read-only.
class ScriptTargetFilter
  ---@type ScriptType[]
  @scriptTypeList = scriptTypeList

  ---@param spec? table<ScriptType, true | { include?: string[], exclude?: string[] }> Initial rules keyed by script type.
  new: (spec) =>
    @rules = {} -- [scriptType] = {all: bool, include: {ns -> true}, exclude: {ns -> true}}
    if spec
      for scriptType, rule in pairs spec
        if rule == true
          @includeAll scriptType
        else
          @include scriptType, ns for ns in *(rule.include or {})
          @exclude scriptType, ns for ns in *(rule.exclude or {})

  ---Lazily creates and returns the rule table for a script type.
  ---@private
  ---@param scriptType ScriptType
  ---@return table rule
  ruleFor: (scriptType) =>
    @rules[scriptType] or= {include: {}, exclude: {}}
    @rules[scriptType]

  ---Includes a single namespace of the given script type.
  ---@param scriptType ScriptType
  ---@param namespace string
  ---@return ScriptTargetFilter self
  include: (scriptType, namespace) =>
    @ruleFor(scriptType).include[namespace] = true
    @

  ---Includes every namespace of the given script type, or — when called without an
  ---argument — every namespace of every script type.
  ---@param scriptType? ScriptType Script type to include all of; omit to include everything.
  ---@return ScriptTargetFilter self
  includeAll: (scriptType) =>
    if scriptType
      @ruleFor(scriptType).all = true
    else
      @includeAll t for t in *@@scriptTypeList
    @

  ---Excludes a single namespace of the given script type (takes precedence over includes).
  ---@param scriptType ScriptType
  ---@param namespace string
  ---@return ScriptTargetFilter self
  exclude: (scriptType, namespace) =>
    @ruleFor(scriptType).exclude[namespace] = true
    @

  scriptTypes: Accessors.property
    get: => [t for t in *@@scriptTypeList when @rules[t]]

  ---Tests whether a script of the given type and namespace should be processed.
  ---@param scriptType ScriptType
  ---@param namespace string
  ---@return boolean
  matches: (scriptType, namespace) =>
    rule = @rules[scriptType]
    return false unless rule
    return false if rule.exclude[namespace]
    return true if rule.all
    rule.include[namespace] or false

Accessors.install ScriptTargetFilter

return ScriptTargetFilter
