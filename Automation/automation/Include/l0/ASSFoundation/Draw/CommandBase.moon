return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  round = math.round

  CommandBase = createASSClass "Draw.CommandBase", ASS.Tag.Base, {}, {}, {precision: 3}
  CommandBase.new = (args, y) =>
    -- most drawing commands (l, m, n) represent a single point
    if @compatible[ASS.Point]
      argType = type args
      @x, @y = if argType == "number"
        args, y or 0
      elseif argType == "table" and args.__raw
        args[1] or 0, args[2] or args[1] or 0
      else
        args = y == nil and args or {args, y}
        @readProps args
        @getArgs args, 0, true
    else
      -- any other drawing command either consist of multiple (b, s) or no (c) points
      -- do note that this constructor is fairly slow in the context of drawings
      -- which is why faster overrides are specified for common drawing commands
      for i = 1, #args, 2
        j = (i+1)/2
        @[@__meta__.order[j]] = @__meta__.types[j]{args[i], args[i+1] or args[i]}

    return @

  CommandBase.getTagParams = (precision = @__tag.precision) =>
    return round(@x, precision), round(@y, precision) if @compatible[ASS.Point]

    params, parts = @__meta__.order, {}
    i, j = 1, 1
    while i <= @__meta__.rawArgCnt
      parts[i], parts[i+1] = @[params[j]]\getTagParams!
      i += @[params[j]].__meta__.rawArgCnt
      j += 1

    return unpack parts


  CommandBase.getLength = (prevCmd) =>
    assert Yutils, yutilsMissingMsg
    -- get end coordinates (cursor) of previous command
    x0, y0 = if prevCmd
      if prevCmd.class == ASS.Draw.Bezier
        prevCmd.p3.x, prevCmd.p3.y
      elseif prevCmd.compatible[ASS.Point]
        prevCmd.x, prevCmd.y
      else prevCmd\get!
    else 0, 0

    -- save cursor for further processing
    @cursor = ASS.Point x0, y0

    name = @__tag.name

    len = switch name
      when "b"
        -- TODO: make less grievously slow, drop Yutils
        shapeSection = ASS.Draw.DrawingBase{ASS.Draw.Move(@cursor.x, @cursor.y), @}
        --save flattened shape for further processing
        @flattened = ASS.Draw.DrawingBase{str: Yutils.shape.flatten shapeSection\getTagParams!}
        @flattened\getLength!
      when "l"
        x, y = @get!
        math.vector2.distance x0, y0, x, y
      else 0 -- m, n

    -- save length for further processing
    @length = len
    return len


  CommandBase.getPositionAtLength = (len, useCachedLengths, useCurveTime) =>
    assert Yutils, yutilsMissingMsg
    @parent\getLength! unless @length and @cursor and useCachedLengths

    pos = switch @__tag.name
      when useCurveTime and "b"
        px, py = Yutils.math.bezier math.min(len/@length, 1), {{@cursor.x, @cursor.y}, {@p1.x, @p1.y},
          {@p2.x, @p2.y}, {@p3.x, @p3.y}}
        ASS.Point px, py
      when "b"
        -- we already know this data is up-to-date because @parent\getLength! was run
        @getFlattened(true)\getPositionAtLength len, true
      when "l"
        ASS.Point @copy!\scaleToLength len, true
      when "m"
        ASS.Point @

    pos.__tag.name = "position"
    return pos


  CommandBase.getPoints = (allowCompatible) =>
    return allowCompatible and {@} or {ASS.Point{@}}

  return CommandBase
