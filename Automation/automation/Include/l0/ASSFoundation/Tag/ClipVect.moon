return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  ClipVect = createASSClass "Tag.ClipVect", ASS.Draw.DrawingBase, {"commands", "scale"},
    {"table", ASS.Number}, {}, {ASS.Draw.DrawingBase}

  msgs = {
    getDrawing: {
      badPosition: "argument position must be an %d or a compatible object, got a %s."
      badAlign: "argument align must be an %d or a compatible object, got a %s."
    }
  }

  ClipVect.toString = ASS.Tag.Base.toString
  ClipVect.__toString = ASS.Tag.Base.toString

  ClipVect.setInverse = (state = true) =>
    @__tag.inverse = state
    @__tag.name = state and "iclip_vect" or "clip_vect"
    return state

  ClipVect.toggleInverse = => @setInverse not @__tag.inverse

  ClipVect.getSignature = =>
    @__tag.signature = if @scale\equal 1 -- TODO: remove legacy property
      "default"
    else "scale"
    return @__tag.signature

  ClipVect.getDrawing = (trimDrawing, pos, an) =>
    if ASS\instanceOf pos, ASS.TagList
      pos, an = pos.tags.position, pos.tags.align

    unless pos and an
      if @parent and @parent.parent
        effTags = @parent.parent\getEffectiveTags(-1, true, true, false).tags
        pos, an = pos or effTags.position, an or effTags.align
      elseif not an
        an = ASS.Tag.Align{7}

    posType = type pos
    logger\assert not pos or ASS\instanceOf(pos, ASS.Point, nil, true), msgs.getDrawing.badPosition,
      ASS.Point.typeName, posType == "table" and pos.typeName or posType
    logger\assert ASS\instanceOf(an, ASS.Tag.Align), msgs.getDrawing.badAlign,
      ASS.Tag.Align.typeName, posType =="table" and an.typeName or type an

    drawing = ASS.Section.Drawing{@}
    extremePoints = @getExtremePoints!
    -- width and height of a drawing are always determined by the extreme points
    -- in the drawing, not the actual bounding box
    anOffset = an\getPositionOffset extremePoints.w, extremePoints.h

    if trimDrawing or not pos
      topLeft = ASS\createTag "position", @getBounds![1]
      drawing\sub topLeft
      return drawing, topLeft\add anOffset
    else return drawing\add(anOffset)\sub pos

  return ClipVect
