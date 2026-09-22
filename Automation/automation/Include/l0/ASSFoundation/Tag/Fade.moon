return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re} = Functional

  Fade = createASSClass "Tag.Fade", ASS.Tag.Base,
    {"inDuration", "outDuration", "inStartTime", "outStartTime", "inAlpha", "midAlpha", "outAlpha"},
    {ASS.Duration, ASS.Duration, ASS.Time, ASS.Time, ASS.Hex, ASS.Hex, ASS.Hex}

  Fade.new = (args) =>
    @readProps args
    if args.raw and @__tag.name == "fade" -- \fade(<a1>,<a2>,<a3>,<t1>,<t2>,<t3>,<t4>)
      a, r, num = {}, args.raw, tonumber
      a[1], a[2], a[3], a[4] = num(r[5])-num(r[4]), num(r[7])-num(r[6]), r[4], r[6]
      -- avoid having alpha values automatically parsed as hex strings
      a[5], a[6], a[7] = num(r[1]), num(r[2]), num(r[3])
      args.raw = a

    inDuration, outDuration, inStartTime, outStartTime, inAlpha, midAlpha, outAlpha = unpack @getArgs args,
      {0, 0, math.nan, math.nan, 255, 0, 255}, true

    @inDuration = ASS.Duration {inDuration}
    @outDuration = ASS.Duration {outDuration}
    @inStartTime = ASS.Time {inStartTime}
    @outStartTime = ASS.Time {outStartTime}
    @inAlpha = ASS.Hex {inAlpha}
    @midAlpha = ASS.Hex {midAlpha}
    @outAlpha = ASS.Hex {outAlpha}

    return @

  Fade.getTagParams = =>
    if @__tag.name == "fade_simple"
      return @inDuration\getTagParams!, @outDuration\getTagParams!

    t1, t3 = @inStartTime\getTagParams!, @outStartTime\getTagParams!
    inDuration, outDuration = @inDuration\getTagParams!, @outDuration\getTagParams!
    t2 = t1 + inDuration
    t4 = t3 + outDuration
    @checkPositive inDuration, outDuration
    return @inAlpha\getTagParams!, @midAlpha\getTagParams!, @outAlpha\getTagParams!, t1, math.max(t2, t1), t3, math.max(t4, t3)

  -- TODO: add method to convert between fades by supplying a line duration

  return Fade
