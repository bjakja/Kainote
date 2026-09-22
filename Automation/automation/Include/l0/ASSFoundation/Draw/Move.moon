return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  defProps = {name: "m", ords: 2, precision: 3, scale: 1}

  DrawMove = createASSClass "Draw.Move", {ASS.Draw.CommandBase, ASS.Point}, {"x", "y"}, {"number", "number"},
    defProps, {ASS.Point}

  -- optimized superfast constructor for internal use
  DrawMove.__defNew = (args) ->
    move = setmetatable {x: args[1], y: args[2], __tag: defProps}, DrawMove
    return move

  return DrawMove
