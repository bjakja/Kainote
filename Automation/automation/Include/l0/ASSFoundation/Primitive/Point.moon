return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  round = math.round
  msgs = {
    getAngle: {
      badRef: "Argument #1 (ref) must be a Drawing Command, a %s (or compatible) or a coordinate table, got a %s."
      badTuple: "Table with reference coordinates must be of format {x, y}, got {%s, %s}."
    }
  }

  Point = createASSClass "Point", ASS.Tag.Base, {"x","y"}, {"number", "number"}, {precision: 3, scale: 1}

  Point.new = (x, y) =>
    xType = type x
    @x, @y = if xType == "number"
      x, y or 0
    elseif xType == "table" and x.__raw
      x[1] or 0, x[2] or x[1] or 0
    else
      args = y == nil and x or {x, y}
      @readProps args
      unpack @getArgs args, 0, true

    return @

  Point.getTagParams = (precision = @__tag.precision) =>
    x, y = @x, @y
    x, y %= @__tag.mod if @__tag.mod

    return round(x, precision), round(y, precision)

  Point.getAngle = (ref, vectAngle) =>
    if "table" != type ref
      logger\error msgs.getAngle.badRef, Point.typeName, type ref

    rx, ry = if ref.class == ASS.Draw.Bezier
      ref.p3.x, ref.p3.y
    elseif not ref.class
      logger\assert type(ref[1]) == "number" and type(ref[2]) == "number", msgs.getAngle.badTuple,
        tostring(ref[1]), tostring ref[2]
      ref[1], ref[2]
    elseif ref.compatible[Point]
      ref.x, ref.y
    else logger\error msgs.getAngle.badRef, Point.typeName, type ref


    sx, sy, cw = @x, @y
    deg = if vectAngle
      cw = (sx*ry - sy*rx) < 0
      a = (sx*rx + sy*ry) / math.sqrt(sx^2 + sy^2) / math.sqrt(rx^2 + ry^2)

      if a >= 1 or a <= -1 or a != a
        0 -- math.acos(x) only defined for -1<x<1, a may be 1/0
      else math.deg math.acos(a) * (cw and 1 or -1)
    else
      math.deg -math.atan2 sy-ry, sx-rx

    return ASS\createTag("angle", deg), cw

  Point.lerp = (a, b, t) ->
    c = a\copy!
    c.x = a.x + (b.x - a.x) * t
    c.y = a.y + (b.y - a.y) * t
    return c

  return Point
