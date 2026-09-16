return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  defProps = {name: "c", ords: 0}

  DrawClose = createASSClass "Draw.Close", ASS.Draw.CommandBase, {}, {}, defProps

  DrawClose.getPoints = =>
    return {}

  -- optimized superfast constructor for internal use
  DrawClose.__defNew = ->
    close = setmetatable {__tag: defProps}, DrawClose
    return close

  return DrawClose
