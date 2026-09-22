return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  class LineText
    insertContentSections = (str, sections, sectCnt, drawingState) ->
      if drawingState.value == 0
        sections[sectCnt+1], sectCnt = ASS.Section.Text(str), sectCnt + 1
      else
        sections[sectCnt+1] = ASS.Section.Drawing{:str, scale: drawingState}
        if sections[sectCnt+1].junk
          sections[sectCnt+2], sectCnt = sections[sectCnt+1].junk, sectCnt + 2
        else sectCnt = sectCnt + 1
      return sectCnt

    @getSections = (line) =>
      sections = {}
      i, sectCnt, drawingState, ovrStart, ovrEnd = 1, 0, ASS\createTag "drawing", 0
      while i <= #line.text
        ovrStart, ovrEnd = line.text\find "{.-}", i
        if ovrStart
          if ovrStart > i
            substr = line.text\sub i, ovrStart-1
            sectCnt = insertContentSections substr, sections, sectCnt, drawingState

          tagSection = ASS.Parser.Sections\getTagOrCommentSection line.text\sub ovrStart+1, ovrEnd-1
          -- remove drawing tags from tag sections so we don't have to keep state in sync with ASSSection.Drawing
          if tagSection.class == ASS.Section.Tag
            drawingTags, drawingTagCnt = tagSection\removeTags "drawing"
            if #tagSection.tags == 0 and drawingTagCnt > 0
              tagSection = nil

            drawingState = drawingTags[drawingTagCnt] or drawingState

          if tagSection
            sections[sectCnt+1], sectCnt = tagSection, sectCnt + 1

          i = ovrEnd +1
        else
          insertContentSections line.text\sub(i), sections, sectCnt, drawingState
          break

      return sections

    @getLineContents = (line) => ASS.LineContents line, @getSections(line), false
