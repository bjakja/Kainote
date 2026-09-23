return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    checkValue: {
      mustBePositive: "%s must be a positive number, got %d."
      mustBeInteger: "%s must be an integer, got %s."
      mustBeInRange: "%s must be in range %d - %d, got %s."
    }
    cmp: {
      badOperand: "operand #%d must be a number or an object of (or based on) the %s class, got a %s."
    }
  }

  Number = createASSClass "Number", ASS.Tag.Base, {"value"}, {"number"}, {base: 10, precision: 3, scale: 1}

  Number.new = (args) =>
    if type(args) == "number"
      @value = args
    else
      @value = @getArgs(args, 0, true)[1]
      @readProps args
      @checkValue!
      @value %= @__tag.mod if @__tag.mod
      @value *= @__tag.scale if @__tag.scale != 1

    return @

  Number.checkValue = =>
    @typeCheck{@value}
    tag = @__tag

    if tag.range and (@value < tag.range[1] or @value > tag.range[2])
      logger\error msgs.checkValue.mustBeInRange, @typeName, tag.range[1], tag.range[2], @value
    logger\error msgs.checkValue.mustBePositive, @typeName, @value if tag.positive and @value < 0
    logger\error msgs.checkValue.mustBeInteger, @typeName, @value if tag.integer and not math.isInt @value

  Number.getTagParams = (_, precision = @__tag.precision) =>
    val = @value
    @checkValue!

    val %= @__tag.mod if @__tag.mod
    val /= @__tag.scale if @__tag.scale != 1

    return math.round val, precision

  Number.cmp = (a, mode, b) ->
    aType, bType = type(a), type b

    if aType == "table" and (a.compatible[Number] or a.baseClasses[Number])
      a = a.value
    elseif aType !="number"
      logger\error msgs.cmp.badOperand, 1, Number.typeName, ASS\instanceOf(a) and a.typeName or type a

    if bType == "table" and (b.compatible[Number] or b.baseClasses[Number])
      b = b.value
    elseif bType !="number"
      logger\error msgs.cmp.badOperand, 2, Number.typeName, ASS\instanceOf(b) and b.typeName or type b

    return switch mode
      when "<" then a < b
      when ">" then a > b
      when "<=" then a <= b
      when ">=" then a >= b

  Number.lerp = (a, b, t) ->
    c = a\copy!
    c.value = a.value + (b.value - a.value) * t
    return c

  Number.modEq = (val, div) => (@%div)\equal val

  Number.__lt = (a, b) -> Number.cmp a, "<", b
  Number.__le = (a, b) -> Number.cmp a, "<=", b
  Number.__add = (a, b) -> type(a) == "table" and a\copy!\add(b) or b\copy!\add a
  Number.__sub = (a, b) -> type(a) == "table" and a\copy!\sub(b) or Number(a)\sub b
  Number.__mul = (a, b) -> type(a) == "table" and a\copy!\mul(b) or b\copy!\mul a
  Number.__div = (a, b) -> type(a) == "table" and a\copy!\div(b) or Number(a)\div b
  Number.__mod = (a, b) -> type(a) == "table" and a\copy!\mod(b) or Number(a)\mod b
  Number.__pow = (a, b) -> type(a) == "table" and a\copy!\pow(b) or Number(a)\pow b

  return Number
