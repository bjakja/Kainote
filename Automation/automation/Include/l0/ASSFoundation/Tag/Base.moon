return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    toString: {
      failedFormat: "Failed to convert tag '%s' to string using params (%s) and format '%s': %s"
    }
  }

  TagBase = createASSClass "TagBase", ASS.Base

  TagBase.commonOp = (method, callback, default, ...) =>
    args = @getArgs {...}, default, false
    a = 1

    for valName in *@__meta__.order
      val = @[valName]
      if type(val) == "table" and val.class
        subArgCount = val.__meta__.rawArgCnt
        val[method] val, unpack args, a, a+subArgCount-1
        a += subArgCount
      else
        @[valName] = callback val, args[a]
        a += 1

    return @

  TagBase.add = (...) =>
    @commonOp "add", ((a,b) -> a + b), 0, ...

  TagBase.sub = (...) =>
    @commonOp "sub", ((a,b) -> a - b), 0, ...

  TagBase.mul = (...) =>
    @commonOp "mul", ((a,b) -> a * b), 1, ...

  TagBase.div = (...) =>
    @commonOp "div", ((a,b) -> a / b), 1, ...

  TagBase.pow = (...) =>
    @commonOp "pow", ((a,b) -> a ^ b), 1, ...

  TagBase.mod = (...) =>
    @commonOp "mod", ((a,b) -> a % b), 1, ...

  TagBase.set = (...) =>
    @commonOp "set", ((_,b) -> b), nil, ...

  TagBase.round = (...) =>
    @commonOp "round", ((a,b) -> math.round(a,b)), nil, ...

  TagBase.ceil = =>
    @commonOp "ceil", ((a) -> math.ceil a), nil

  TagBase.floor = =>
    @commonOp "floor", ((a) -> math.floor a), nil

  TagBase.modify = (callback, ...) =>
    @set callback @get ...

  TagBase.readProps = (args) =>
    if args.tagProps
      @__tag[k] = v for k, v in pairs args.tagProps

    elseif type(args[1]) == "table" and args[1].instanceOf and args[1].instanceOf[@class]
      @__tag[k] = v for k, v in pairs args[1].__tag

  TagBase.getSignature = =>
    return @__tag.signature or "default"

  TagBase.toString = () =>
    format = ASS.tagMap[@__tag.name].signatures[@getSignature!].format
    tagString, errMsg = string.formatEx format, @getTagParams!
    unless tagString
      logger\error msgs.toString.failedFormat,
        @__tag.name, table.concat({@getTagParams!}, ', '), format, errMsg
    return tagString

  TagBase.__tostring = TagBase.toString

  -- legacy TODO: move the disable magic out and replace w/ toString
  TagBase.getTagString = (caller) =>
    -- disabled tags or tags marked for deletion are not emitted
    return "" if @disabled or caller and caller.toRemove and caller.toRemove[@]
    return @toString!

  -- checks equality only of the relevant properties
  TagBase.equal = (a, b, acceptCompatible, ignoreTagNames) ->
    bVals = if type(b) != "table"
      {b}
    elseif not b.instanceOf
      b
    elseif acceptCompatible and a.compatible[b.class]
      {b\get!}
    elseif b.class == a.class and (a.__tag.name == b.__tag.name or ignoreTagNames)
      {b\get!}
    else return false

    aVals = {a\get!}
    return false if #aVals != #bVals

    for i, aVal in ipairs aVals
      if type(aVal) == "table"
        return false unless util.equals aVal, bVals[i], "table"
      else return false if aVal != bVals[i]

    return true

  return TagBase
