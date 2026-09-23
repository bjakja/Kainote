return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  ClipRect = createASSClass "Tag.ClipRect", ASS.Tag.Base, {"topLeft", "bottomRight"}, {ASS.Point, ASS.Point}

  ClipRect.new = (args) =>
    left, top, right, bottom = unpack @getArgs args, 0, true
    @readProps args

    @topLeft = ASS.Point{left, top}
    @bottomRight = ASS.Point{right, bottom}
    @setInverse @__tag.inverse or false

  ClipRect.getTagParams = =>
    @setInverse @__tag.inverse or false
    xTopLeft, yTopLeft = @topLeft\getTagParams!
    xBottomRight, yBottomRight = @bottomRight\getTagParams!
    return xTopLeft, yTopLeft, xBottomRight, yBottomRight

  ClipRect.getVect = =>
    vect = ASSFInst\createTag ASS.tagNames[ASS.Tag.ClipVect][@__tag.inverse and 2 or 1]
    return vect\drawRect @topLeft, @bottomRight

  ClipRect.getDrawing = (trimDrawing, pos, an) =>
    if ASS\instanceOf pos, ASS.TagList
      pos, an = pos.tags.position, pos.tags.align

    unless pos and an
      if @parent and @parent.parent
        effTags = @parent.parent\getEffectiveTags(-1, true, true, false).tags
        pos, an = pos or effTags.position, an or effTags.align

    return @getVect!\getDrawing trimDrawing, pos, an

  ClipRect.lerp = (a, b, t) ->
    c = a\copy!
    c.topLeft = a.topLeft\lerp b.topLeft, t
    c.bottomRight = a.bottomRight\lerp b.bottomRight, t

  ClipRect.setInverse = (state = true) =>
    @__tag.inverse = state
    @__tag.name = state and "iclip_rect" or "clip_rect"
    return state

  ClipRect.toggleInverse = => @setInverse not self.__tag.inverse -- TODO: rename to invert()

  return ClipRect
