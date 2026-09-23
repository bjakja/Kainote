return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  Color = createASSClass "Tag.Color", ASS.Tag.Base, {"r","g","b"}, {ASS.Hex, ASS.Hex, ASS.Hex}

  Color.new = (args) =>
    b, g, r = unpack @getArgs args, nil, true
    @readProps args
    @r, @g, @b = ASS.Hex(r), ASS.Hex(g), ASS.Hex(b)

  Color.addHSV = (h, s, v) =>
    ho, so, vo = @getHSV!
    return @set util.HSV_to_RGB ho+h, util.clamp(so+s, 0, 1), util.clamp vo+v, 0, 1

  Color.applyHSV = (filter) =>
    ho, so, vo = @getHSV!
    h, s, v = filter ho, so, vo, @
    return @set util.HSV_to_RGB h, util.clamp(s, 0, 1), util.clamp v, 0, 1

  Color.fromHSV = (h, s, v) ->
    color = Color!
    color\addHSV h, s, v
    return color

  Color.getHSV = => util.RGB_to_HSV @r\get!, @g\get!, @b\get!

  Color.getTagParams = => @b\getTagParams!, @g\getTagParams!, @r\getTagParams!

  Color.lerp = (a, b, t) ->
    c = a\copy!
    return c\applyHSV (ha, sa, va) ->
      hb, sb, vb = b\getHSV!
      return ha + (hb - ha)*t, sa + (sb - sa)*t, va + (vb - va)*t

  return Color
