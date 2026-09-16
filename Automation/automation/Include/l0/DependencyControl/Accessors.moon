constants = require "l0.DependencyControl.Constants"

-- marks an AccessorSpec so install can pick specs out of a class's __base entries
ACCESSOR_TAG = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}AccessorSpec"

msgs = {
  property: {
    badSpec: "Accessors.property expects a spec table, got a %s."
    emptySpec: "Accessors.property needs a `get` and/or `set` function; got neither."
    badFunction: "Accessors.property `%s` must be a function, got a %s."
  }
  install: {
    badClass: "Accessors.install expects a class with a `__base` table, got a %s."
    readOnly: "Can't set read-only property '%s'."
  }
}

---An opaque computed-property spec produced by `property` and consumed by `install`. Not indexed directly.
---@class AccessorSpec

---Per-property capability flags recorded on a class's `__accessors` registry after install.
---@alias AccessorInfo { get: boolean, set: boolean }

---Declares metatable-backed get/set computed properties on a class through one standardized transform:
---`property` marks a property in a class body; `install`, called once after the body, wires the instance
---metatable to dispatch those keys to the getter/setter and records each on the class's `__accessors`.
---@class Accessors
class Accessors
  ---Declares a computed property to place at a key in a class body; wire the class up with install.
  ---Omitting `set` makes the property read-only, so a write raises at runtime.
  ---@param spec { get?: (fun(self: any): any), set?: (fun(self: any, value: any)) } Getter and/or setter (at least one).
  ---@return AccessorSpec spec Assign this to the property's key in the class body.
  @property = (spec) ->
    error msgs.property.badSpec\format type spec unless type(spec) == "table"
    error msgs.property.emptySpec unless spec.get or spec.set
    error msgs.property.badFunction\format("get", type spec.get) if spec.get and type(spec.get) != "function"
    error msgs.property.badFunction\format("set", type spec.set) if spec.set and type(spec.set) != "function"
    {[ACCESSOR_TAG]: true, get: spec.get, set: spec.set}

  ---Installs standardized get/set dispatch for every property declared with `property` in a class's
  ---body, plus any inherited from a parent class that ran install, recording each on the class's
  ---`__accessors` registry. Call once, immediately after the class body (and after the parent's install
  ---for a subclass). Reading a getter-less property falls through to normal lookup; writing a setter-less
  ---one raises.
  ---@param cls table The class whose `__base` carries the property specs.
  ---@return table cls The same class, for chaining.
  @install = (cls) ->
    base = type(cls) == "table" and cls.__base
    error msgs.install.badClass\format type cls unless type(base) == "table"

    -- a subclass's __base has the parent __base as its metatable, so plain pairs here would run the inherited
    -- __pairs against the base table, whose getters have no instance to read. next avoids that
    own, specKeys = {}, {}
    for key, value in next, base
      if type(value) == "table" and value[ACCESSOR_TAG]
        own[key] = {get: value.get, set: value.set}
        specKeys[#specKeys + 1] = key
    base[key] = nil for key in *specKeys -- drop the sentinels so they aren't served as raw fields

    -- a function __index doesn't reach a child instance with the right self through the parent chain, so a
    -- subclass must dispatch its inherited accessors from its own __base
    accessors = {}
    if parent = cls.__parent
      if inherited = parent.__accessorSpecs
        accessors[name] = spec for name, spec in pairs inherited
    accessors[name] = spec for name, spec in pairs own

    cls.__accessorSpecs = accessors -- for a subclass's install to inherit
    cls.__accessors = {name, {get: spec.get != nil, set: spec.set != nil} for name, spec in pairs accessors}
    return cls unless next accessors

    fallback = base.__index
    base.__index = (self, key) ->
      accessor = accessors[key]
      return accessor.get self if accessor and accessor.get
      return fallback self, key if type(fallback) == "function"
      fallback[key]
    base.__newindex = (self, key, value) ->
      accessor = accessors[key]
      if accessor
        error msgs.install.readOnly\format(key), 2 unless accessor.set
        return accessor.set self, value
      rawset self, key, value

    -- makes computed properties appear in pairs(instance), which requires LUAJIT_ENABLE_LUA52COMPAT
    readable = [name for name, spec in pairs accessors when spec.get]
    base.__pairs = (self) ->
      i, key, rawDone = 0, nil, false
      ->
        unless rawDone
          key, value = next self, key
          return key, value if key != nil
          rawDone = true
        i += 1
        name = readable[i]
        return name, accessors[name].get self if name
    cls

return Accessors
