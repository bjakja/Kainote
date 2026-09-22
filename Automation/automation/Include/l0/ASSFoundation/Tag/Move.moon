return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  Move = createASSClass "Tag.Move", ASS.Tag.Base, {"startPos", "endPos", "startTime", "endTime"}, {ASS.Point, ASS.Point, ASS.Time, ASS.Time}

  msgs = {
    getTagParams: {
      endsBeforeStart: "move times must evaluate to t1 <= t2, got %d<=%d."
    }
  }

  Move.new = (args) =>
    startX, startY, endX, endY, startTime, endTime = unpack @getArgs args, 0, true
    if startTime > endTime
      startTime, endTime = endTime, startTime

    @readProps args
    @startPos = ASS.Point {startX, startY}
    @endPos = ASS.Point {endX, endY}
    @startTime = ASS.Time {startTime}
    @endTime = ASS.Time {endTime}

    return @

  Move.getSignature = =>
    @__tag.signature = if @startTime\equal(0) and @endTime\equal(0) -- TODO: remove legacy property
      "simple"
    else "default"
    return @__tag.signature

  Move.getTagParams = =>
    startX, startY = @startPos\getTagParams!
    endX, endY = @endPos\getTagParams!

    if @__tag.signature == "simple"
      return startX, startY, endX, endY

    t1, t2 = @startTime\getTagParams!, @endTime\getTagParams!
    logger\assert t1 <= t2, msgs.getTagParams.endsBeforeStart, t1, t2
    return startX, startY, endX, endY, t1, t2

  return Move
