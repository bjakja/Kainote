return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  min, max = math.min, math.max
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  LineBounds = createASSClass "LineBounds", ASS.Base, {1, 2, "w", "h", "fbf", "animated", "rawText"},
    {ASS.Point, ASS.Point, "number", "number", "table", "boolean", "string"}

  LineBounds.new = (line, bounds, frames, first, last, offset, keepRawText) =>
    first, last, offset = first or 1, last or #bounds, offset or 0
    @animated = line.si_exhaustive

    if bounds[first] != false or @animated
      x2Max, y2Max, x1Min, y1Min = 0, 0

      @fbf = {off: frames[first]+offset, n: last - first + 1}
      for i = first, last
        bound = bounds[i]
        if bound
          x1, y1, w, h = bound.x, bound.y, bound.w, bound.h
          x2, y2 = x1+w, y1+h
          @fbf[frames[i]+offset] = {ASS.Point(x1,y1), ASS.Point(x2,y2), :w, :h, hash: bound.hash, solid: bound.solid}
          x1Min, y1Min = min(x1, x1Min or x1), min(y1, y1Min or y1)
          x2Max, y2Max = max(x2, x2Max),       max(y2, y2Max)
        else @fbf[frames[i]+offset] = {w: 0, h: 0, hash: false}

      if x1Min
        @[1], @[2], @w, @h = ASS.Point(x1Min, y1Min), ASS.Point(x2Max, y2Max), x2Max-x1Min, y2Max-y1Min
        @firstHash = @fbf[@fbf.off].hash
        @firstFrameIsSolid = @fbf[@fbf.off].solid
      else @w, @h = 0, 0

    else @w, @h, @fbf = 0, 0, {n: 0}

    @rawText = line.text if keepRawText
    return @

  LineBounds.equal = (other) =>
    if other.class != LineBounds
      -- TODO: replace w/ logger.error
      error string.format "argument #1 must be an object of type %s, got a %s.",
        LineBounds.typeName, ASS\instanceOf(other) or type(other)

    return true if @w == other.w == 0
    if @w != other.w or @h != other.h or @animated != other.animated or @fbf.n != other.fbf.n or @fbf.off != other.fbf.off
      return false

    for i = 0, @fbf.n-1
      return false if @fbf[@fbf.off+i].hash != other.fbf[other.fbf.off+i].hash

    return true

  return LineBounds
