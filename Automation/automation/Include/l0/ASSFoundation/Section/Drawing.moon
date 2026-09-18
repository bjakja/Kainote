return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  DrawingSection = createASSClass "Section.Drawing", ASS.Draw.DrawingBase, {"contours","scale"},
    {"table", ASS.Number}, {}, {ASS.Draw.DrawingBase, ASS.Tag.ClipVect}

  DrawingSection.getStyleTable = ASS.Section.Text.getStyleTable
  DrawingSection.getEffectiveTags = ASS.Section.Text.getEffectiveTags
  DrawingSection.getTagString = nil

  getTagParams = DrawingSection.getTagParams
  DrawingSection.getTagParams = () =>
    scale, commands = getTagParams @
    return commands or scale
  DrawingSection.getString = DrawingSection.getTagParams -- TODO: remove in favor of toString
  DrawingSection.toString = DrawingSection.getTagParams

  DrawingSection.alignToOrigin = (mode) =>
    mode = ASS.Tag.Align {mode or 7}
    ex = @getExtremePoints true
    cmdOff = ASS.Point ex.left.x, ex.top.y
    posOff = mode\getPositionOffset(ex.w, ex.h)\add cmdOff
    @sub cmdOff
    return posOff, ex

  DrawingSection.getClip = (inverse) =>
    -- TODO: scale support
    effTags, ex = @parent\getEffectiveTags(-1, true, true, false).tags, @getExtremePoints!
    clip = ASS\createTag ASS.tagNames[ASS.Tag.ClipVect][inverse and 2 or 1], self
    anOff = effTags.align\getPositionOffset ex.w, ex.h
    return clip\add(effTags.position)\sub anOff

  return DrawingSection
