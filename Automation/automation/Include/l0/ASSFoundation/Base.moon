return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  msgs = {
    checkPositive: {
      notPositiveNumber: "%s tagProps do not permit numbers < 0, got %d."
    }
    coerce: {
      cantCastTable: "can't cast a table to a %s. Table contents: %s"
      noNumber: "failed coercing value '%s' of type %s to a base-%d number on creation of %s object."
      badTargetType: "unsupported conversion target type '%s'"
    }

    getArgs: {
      badArgs: "first argument to getArgs must be a table of arguments, got a %s."
      incompatibleObject: "object of class %s does not accept instances of class %s as argument."
      typeMismatch: "%s: type mismatch in argument #%d (%s). Expected a %s or a compatible object, but got a %s."
    }

    typeCheck: {
      badType: "%s: bad type for argument #%d (%s). Expected %s, got %s."
    }
  }
  Base = createASSClass "Base"

  -- TODO: remove
  Base.checkPositive = (...) =>
    for i = 1, select '#', ...
      val = select i, ...
      if type(val) != "number" or val < 0
        error msgs.checkPositive.notPositiveNumber\format  @typeName, val

  Base.coerceNumber = (num, default = 0) =>
    num = tonumber(num) or default
    if @__tag.positive
      num = math.max num, 0
    elseif range = @__tag.range
      num = util.clamp num, range[1], range[2]
    return num

  Base.coerce = (value, targetType) =>
    valType = type value
    if "table" == valType
      error msgs.coerce.cantCastTable\format targetType, logger\dumpToString value

    return value if valType == targetType
    tagProps = @__tag or @__defProps

    return switch targetType
      when valType == "boolean" and "number"
        value and 1 or 0
      when "number"
        base = tagProps.base or 10
        tonumber(value, base) or error msgs.coerce.noNumber\format tostring(value), valType, base, @typeName
      when "string"
        tostring value
      when "boolean"
        value != 0 and value != "0" and value != false
      when "table"
        {value}
      else error msgs.coerce.badTargetType\format targetType

  Base.getArgs = (args = {}, defaults, coerce, first = 1, last, defaultIdx = 1) =>
    -- TODO: make getArgs automatically create objects
    error msgs.getArgs.badArgs\format type(args) if "table" != type args
    propTypes = @__meta__.types
    last or= #args
    defaultsIsTable = type(defaults) == "table"

    obj = if args.class
      args
    elseif first == last and type(args[first]) == "table" and args[first].class
      args[first]

    -- process a single passed object if it's compatible
    if obj and @compatible[obj.class]
      if obj.deepCopy
        args = obj\get!
      else
        outArgs = [obj[field] or defaultsIsTable and defaults[f] or defaults for f, field in ipairs @__meta__.order]
        return outArgs

    -- process "raw" property that holds all tag parameters when parsed from a string
    elseif type(args.raw) == "table"
      args = args.raw

    -- TODO: what is this useful for?
    elseif args.raw
      args = {args.raw}

    elseif args.class
      args = {args}

    -- decompose all arguments into primitive types, then coerce if requested
    sliceLast, outArgs, o = first, {}, 1

    for i, propName in ipairs @__meta__.order
      propType = propTypes[i]

      -- ASSFoundation class members consume a known number of arguments
      if type(propType) == "table" and propType.class
        rawArgCnt, propRawArgCnt, defSlice = 0, propTypes[i].__meta__.rawArgCnt
        while rawArgCnt < propRawArgCnt
          arg = args[sliceLast]
          rawArgCnt += type(arg) == "table" and arg.class and arg.__meta__.rawArgCnt or 1
          sliceLast += 1

        sliceArgs = propType\getArgs args, defaults, coerce, first, sliceLast-1, defaultIdx
        outArgs[o], o = sliceArg, o + 1 for sliceArg in *sliceArgs
        defaultIdx += propRawArgCnt

      -- primitive class members consume 1 argument
      else
        arg, argType = args[sliceLast], type args[sliceLast]

        outArgs[o] = if arg == nil -- write defaults
          if defaultsIsTable
            defaults[defaultIdx]
          else defaults
        elseif argType == "table" and arg.class
          if arg.__meta__.rawArgCnt != 1
            logger\error msgs.getArgs.typeMismatch, @typeName, i, propName, propType, arg.typeName
          arg\get!
        elseif coerce and argType != propType
          @coerce arg, propType
        else arg

        sliceLast, defaultIdx, o = sliceLast + 1, defaultIdx + 1, o + 1

      first = sliceLast
    return outArgs

  Base.copy = =>
    newObj, meta = {}, getmetatable @
    setmetatable newObj, meta
    keys = list.makeSet @__meta__.order if @__meta__

    for k, v in pairs @
      newObj[k] = if (k == "__tag" or not keys or keys[k]) and "table" == type v
        v.class and v\copy! or Base.copy v
      else v
    return newObj

  Base.typeCheck = (args, first = 1) =>
    valTypes, valNames = @__meta__.types, @__meta__.order
    for i, valName in ipairs valNames
      valType = valTypes[i]
      if type(valType) == "table" and valType.class
        if type(args[first]) == "table" and args[first].class
          -- argument and expected type are both ASSFoundation object
          -- defer type checking to object
          @[valNames]\typeCheck {args[first]}
        else
          -- collect expected number of arguments for target ASSObject
          subCnt = #valType.__meta__.order
          valType\typeCheck args, first, first+subCnt-1
          first += subCnt - 1

      elseif args[first] != nil and valType != "nil" and type(args[first]) != valType
        error msgs.typeCheck.badType\format @typeName, i, valName, valType, type args[first]
      first += 1

  Base.get = (packed) =>
    vals, valCnt = {}, 1
    for name in *@__meta__.order
      if type(@[name]) == "table" and @[name].class
        vals[valCnt], valCnt = subVal, valCnt+1 for j, subVal in pairs {@[name]\get!}

      else
        vals[valCnt], valCnt = @[name], valCnt + 1

    return if packed
      vals
    else unpack vals

  return Base
