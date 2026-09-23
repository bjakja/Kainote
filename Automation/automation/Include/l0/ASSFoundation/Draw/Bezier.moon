return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  round = math.round
  defProps, ptsDefProps = {name: "b", ords: 6, precision: 3}, {precision: 3, scale: 1}

  DrawBezier = createASSClass "Draw.Bezier", ASS.Draw.CommandBase, {"p1","p2","p3"},
    {ASS.Point, ASS.Point, ASS.Point}, defProps
  Point = ASS.Point

  DrawBezier.new = (args) =>
    -- this whole constructor is an optimization of the generic CommandBase constructor
    -- creating beziers works without this constructor, albeit slower
    args = @getArgs args, nil, true unless args.__raw
    args[1] or= 0

    @p1 = Point args[1], args[2] or args[1]
    @p2 = Point args[3] or @p1.x, args[4] or @p1.y
    @p3 = Point args[5] or @p2.x, args[6] or @p2.y

    return @

  -- optimized superfast constructor for internal use
  DrawBezier.__defNew = (args) ->
    p1 = setmetatable {__tag: ptsDefProps, x: args[1], y: args[2] or args[1]}, Point
    p2 = setmetatable {__tag: ptsDefProps, x: args[3] or p1.x, y: args[4] or p1.y}, Point
    p3 = setmetatable {__tag: ptsDefProps, x: args[5] or p2.x, y: args[6] or p2.y}, Point

    bezier = setmetatable {__tag: defProps, :p1, :p2, :p3}, DrawBezier
    return bezier


  DrawBezier.commonOp = (method, callback, default, ...) =>
    args, j = {...}, 1
    if #args <= 2 -- special case to allow common operation on all x an y values of a vector drawing
      args[3], args[4], args[5], args[6] = args[1], args[2], args[1], args[2]
      if type(default)=="table" and #default <= 2
        default = {default[1], default[2], default[1], default[2]}

    args = @getArgs args, default, false

    for valName in *@__meta__.order
      subCnt = #@[valName].__meta__.order
      @[valName][method] @[valName], unpack args, j, j+subCnt-1
      j += subCnt

    return @

  DrawBezier.getTagParams = (precision = @__tag.precision) =>
    x1, y1, x2, y2, x3, y3 = @p1.x, @p1.y, @p2.x, @p2.y, @p3.x, @p3.y
    x1, x2, x3, y1, y2, y3 %= @__tag.mod if @__tag.mod

    return round(x1, precision), round(y1, precision), round(x2, precision), round(y2, precision), round(x3, precision), round(y3, precision)

  DrawBezier.getFlattened = (noUpdate) =>
    assert Yutils, yutilsMissingMsg
    unless noUpdate and @flattened
      @parent\getLength! unless noUpdate and @cursor

      -- TODO: check
      shapeSection = ASS.Draw.DrawingBase{ASS.Draw.Move(@cursor.x, @cursor.y), @}
      @flattened = ASS.Draw.DrawingBase{str: Yutils.shape.flatten shapeSection\getTagParams!}

    return @flattened


  DrawBezier.getPoints = =>
    return {@p1, @p2, @p3}

  return DrawBezier
