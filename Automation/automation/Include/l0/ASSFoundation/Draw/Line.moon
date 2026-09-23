return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  defProps = {name: "l", ords: 2, precision: 3, scale: 1}

  DrawLine = createASSClass "Draw.Line", {ASS.Draw.CommandBase, ASS.Point}, {"x", "y"}, {"number", "number"},
    defProps, {ASS.Point, ASS.Draw.Move, ASS.Draw.MoveNc}

  -- optimized superfast constructor for internal use
  DrawLine.__defNew = (args) ->
    line = setmetatable {x: args[1], y: args[2], __tag: defProps}, DrawLine
    return line

  DrawLine.scaleToLength = (len, useCachedLengths) =>
    @parent\getLength! unless @length and @cursor and useCachedLengths
    @sub @cursor
    @set @cursor\copy!\add math.vector2.normalize @x, @y, len
    return @


  DrawLine.getAngle = (ref, vectAngle, useCachedLengths) =>
    unless ref and @cursor and useCachedLengths
      @parent\getLength!
      ref = @cursor

    return ASS.Point.getAngle @, ref, vectAngle

  return DrawLine
