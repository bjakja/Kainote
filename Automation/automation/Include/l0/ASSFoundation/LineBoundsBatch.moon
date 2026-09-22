return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  time, frame = aegisub.ms_from_frame, aegisub.frame_from_ms

  msgs = {
    add: {
      badLineContents: "argument #1 (lineContents) must be an object of type %s, got a %s."
    }
    run: {
      inspectorError: "SubInspector Error: %s."
      inspectorRefreshingHeaders: "Sending headers to SubInspector for new subtitle object %s..."
      inspectorNoSub: "Can't initialize SubInspector because the Line isn't linked to a subtitle object."
    }
  }

  LineBoundsBatch = createASSClass "LineBoundsBatch", ASS.Base, {"lines", "lineCount", "virtualStartFrame", "bounds"},
    {"table", "number", "number", "table"}


  LineBoundsBatch.new = =>
    @lines = {}
    @lineCount = 0
    @virtualStartFrame = 1
    return @

  LineBoundsBatch.add = (cnts, custText, foreignKey = cnts, isAnimated = cnts\isAnimated!) =>
    if cnts.class != ASS.LineContents
      logger\error msgs.add.badLineContents, ASS.LineContents.typeName, ASS\instanceOf(cnts) or type cnts

    line = cnts.line
    @sub or= line.sub
    @lineCount += 1

    startFrame, endFrame = frame(line.start_time), frame line.end_time
    line = Line line, nil, {
      text: custText == true and line.text or custText or cnts\getString!
      start_time: time @virtualStartFrame
      end_time: time @virtualStartFrame + endFrame - startFrame
      extra: false
    }

    line.si_exhaustive = isAnimated
    @lines[foreignKey] = {id: @lineCount, :line, :foreignKey, startFrame: @virtualStartFrame,
                offset: startFrame - @virtualStartFrame }
    @virtualStartFrame += 1 + endFrame - startFrame

  LineBoundsBatch.run = (purgeLines, outBounds = {}) =>
    return {} if @lineCount == 0

    subInspector = ASSFInst.cache.SubInspector
    if not subInspector
      logger\assert @sub, msgs.run.inspectorNoSub
      subInspector, msg = SubInspector @sub
      logger\error msgs.run.inspectorError, tostring msg unless subInspector
      ASSFInst.cache.SubInspector = subInspector
      ASSFInst.cache.lastInspectorSub = @sub

    elseif @sub and @sub != ASSFInst.cache.lastInspectorSub
      logger\trace msgs.run.inspectorRefreshingHeaders, @sub
      subInspector\updateHeader @sub
      ASSFInst.cache.lastInspectorSub = @sub

    linesById = {line.id, line for _, line in pairs @lines}
    bounds, times = subInspector\getBounds [line.line for line in *linesById]
    frames = times.frames

    fStart, lineId, line = 1, 1, linesById[1]
    for f, frame in ipairs frames
      if frame == line.startFrame
        outBounds[line.foreignKey] = ASS.LineBounds line.line, bounds, frames, fStart, f, line.offset
        if purgeLines
          @lines[line.foreignKey] = nil
        lineId += 1
        line = linesById[lineId]
        fStart = f + 1
        break unless line

    if purgeLines
      @lines, @lineCount, @virtualStartFrame = {}, 0, 1

    return outBounds
  return LineBoundsBatch
