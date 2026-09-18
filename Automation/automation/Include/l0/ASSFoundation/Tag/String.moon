return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  String = createASSClass "Tag.String", {ASS.Tag.Base, ASS.String}, {"value"}, {"string"}
  String.add, String.mul, String.div, String.pow, String.mod = String.append, nil, nil, nil, nil

  String.getTagParams = =>
    @typeCheck {@value}
    return @value

  return String
