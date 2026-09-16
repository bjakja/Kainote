Logger = require "l0.DependencyControl.Logger"
constants = require "l0.DependencyControl.Constants"

defaultLogger = Logger fileBaseName: "#{constants.DEPCTRL_SHORT_NAME}.Enum"

reservedKeys = {
  "describe",
  "elements"
  "keys",
  "name",
  "test",
  "values"
}

reservedKeySet = {v, true for v in *reservedKeys}

msgs = {
  __index: {
    invalidKeyAccess: "Cannot access invalid key '%s' on Enum '%s'"
  }
  __newindex: {
    immutableError: "Cannot assign field '%s' to '%s' on immutable Enum '%s'."
  }
  new: {
    valueAlreadyTaken: "Could not define '%s' in enum '%s': value %s is already taken by '%s'."
    keyAlreadyDefined: "Cannot redefine key '%s' in enum '%s'."
    noReservedKeys: "Key may not be any of the reserved words [#{table.concat reservedKeys, ', '}] or start with '__' (was '%s')."
    missingOrInvalidName: "Missing or invalid Enum name (expected a string, got a '%s')."
  }
  describe: {
    valueNotDefined: "Value '%s' is not defined in enum '%s'."
  }
  validate: {
    argPrefix: "Argument %s: "
    invalidValue: "%sInvalid value '%s' for enum '%s'."
  }
}

---An immutable enumeration type with value/key reverse lookup.
---@class Enum
class Enum
  ---Reports whether `k` is reserved as an enum key — a built-in member name or `__`-prefixed.
  ---@param k string
  ---@return boolean reserved
  @isReservedKey = (k) =>
    return type(k) == "string" and (k\sub(1,2) == "__" or reservedKeySet[k]) or false


  ---Creates an enum from a table of key/value pairs or a list of names.
  ---@param name string
  ---@param values table Key/value pairs, or a list of names whose value defaults to their position.
  ---@param logger? Logger Logger for enum error messages (default: a shared logger).
  new: (@name, values, @__logger = defaultLogger) =>
    @__logger\assert type(@name) == "string", msgs.new.missingOrInvalidName, Logger\describeType @name
    @elements, @__keysByValue, @values, @keys = {}, {}, {}, {}

    for k, v in pairs values
      -- we support lists as input, but we do not support numerical keys, which is sane
      if "number" == type k
        k, v = v, k

      @__logger\assert not @@isReservedKey(k), msgs.new.noReservedKeys, k
      @__logger\assert @elements[k] == nil, msgs.new.keyAlreadyDefined, k, @name
      @__logger\assert @__keysByValue[v] == nil, msgs.new.valueAlreadyTaken, k, @name, v, @__keysByValue[v]

      @elements[k], @__keysByValue[v] = v, k
      table.insert @values, v
      table.insert @keys, k

    meta = getmetatable @
    clsIdx = meta.__index

    setmetatable @, setmetatable {
      __index: (k) =>
        if @elements[k] != nil
          return @elements[k]

        v = switch type clsIdx
          when "function" then clsIdx @, k
          when "table" then clsIdx[k]
        return v if v != nil

        @__logger\error msgs.__index.invalidKeyAccess, k, @name

      __newindex: (k, v) =>
        @__logger\error msgs.__newindex.immutableError, k, v, @name
    }, clsIdx


  ---Returns whether the given key is defined in this enum.
  ---@param key string
  ---@return boolean defined
  ---@return any value The value mapped to the key, or nil if undefined.
  test: (key) =>
    val = @elements[key]
    return val != nil and true or false, val


  ---Returns a human-readable description of the given value(s) in this enum.
  ---@param values? any A single value, or a list of values to look up. If not provided, returns all keys.
  ---@param pattern? fun(key: string, value: any): string A function to format the key/value pair for display (default "<value> (<key>)").
  ---@param join? string|boolean Separator string for joining multiple keys, true for ", ", or false to return a list (default true).
  ---@return string|string[] result A single string when joining, or a list of the formatted keys when join is false.
  describe: (values = @values, pattern = ((key, value) -> "#{value} (#{key})"), join = true) =>
    values = {values} if "table" != type values

    keys = for v in *values
      key = @__keysByValue[v]
      @__logger\assert key != nil, msgs.describe.valueNotDefined, v, @name
      pattern key, v

    return join and table.concat(keys, join == true and ', ' or join) or keys

  ---Validates that a value is a member of this enum.
  ---@param value any
  ---@param argName? string Argument name to include in the error message.
  ---@return boolean? valid True when the value is a member, nil otherwise.
  ---@return string? err Validation error message when invalid.
  validate: (value, argName) =>
    if value == nil or @__keysByValue[value] == nil
      prefix = argName != nil and msgs.validate.argPrefix\format(argName) or ""
      return nil, msgs.validate.invalidValue\format prefix, value, @name

    return true

return Enum
