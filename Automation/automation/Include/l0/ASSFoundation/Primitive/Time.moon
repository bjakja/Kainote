return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    getTagParams:
    {
      noFloatMs: "%s doesn't support floating point precision."
    }
  }

  Time = createASSClass "Time", ASS.Number, {"value"}, {"number"}, {precision: 0}
  -- TODO: implement adding by framecount

  Time.getTagParams = (precision = 0) =>
    val = @value
    if precision > 0
      logger\error msgs.getTagParams.noFloatMs @typeName

    @checkPositive @value if @__tag.positive
    val /= @.__tag.scale
    return math.round val, precision

  return Time
