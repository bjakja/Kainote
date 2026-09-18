return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  defProps = {name: "n", ords: 2, precision: 3, scale: 1}

  DrawMoveNc = createASSClass "Draw.MoveNc", {ASS.Draw.Move}, {"x", "y"}, {"number", "number"},
    defProps, {ASS.Draw.Move, ASS.Point}

  -- optimized superfast constructor for internal use
  DrawMoveNc.__defNew = (args) ->
    moveNc = setmetatable {x: args[1], y: args[2], __tag: defProps}, DrawMoveNc
    return moveNc

  return DrawMoveNc
