return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  Indexed = createASSClass "Tag.Indexed", ASS.Number, {"value"}, {"number"}, {precision: 0, positive: true}
  Indexed.cycle = (down) =>
    min, max = @__tag.range[1], @__tag.range[2]
    if down then
      return @value <= min and @set(max) or @add -1
    else
      return @value >= max and @set(min) or @add 1

  Indexed.lerp = nil

  return Indexed
