return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  min, max = math.min, math.max
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    new: {
      refreshingSubCache: "Cached new subtitle object %s."
      badLine: "argument 1 to %s() must be a Line or %s object, got %s."
    }

    getString: {
      invalidSection: "invalid %s section #%d. Expected {%s}, got a %s."
    }

    callback: {
      noMixedStartEndSigns: "arguments 'start' and 'end' to callback() must be either both >0 or both <0, got %d and %d."
      noSwappedStartEnd: "condition 'start' <= 'end' not met, got %d <= %d"
    }

    getEffectiveTags: {
      badIndex: "argument #1 (index) to getEffectiveTags() must be an integer != 0, got '%s' of type %s."
    }

    getLineBounds: {
      inspectorError: "SubInspector Error: %s."
      inspectorRefreshingHeaders: "Sending headers to SubInspector for new subtitle object %s..."
      inspectorNoSub: "Can't initialize SubInspector because the Line isn't linked to a subtitle object."
    }

    getPosition: {
      badAlign: "argument #1 (align) must be of type number or %s, got a %s."
    }

    getStyleRef: {
      badStyle: "invalid argument #1 (style): expected a style name or a styleRef, got a %s."
      noSuchStyle: "couldn't find style with name '%s'."
    }

    insertTags: {
      badIndex: "argument #2 (index) to insertTags() must be an integer != 0, got '%s' of type %s"
      badTarget: "can't insert tag in section #%d of type %s."
    }

    removeSections: {
      badArg: "Error: invalid parameter #1. Expected a range, an ASSObject or a table of ASSObjects, got a %s."
    }

    insertSections: {
      invalidSectionType: "can only insert sections of type {%s}, got %s."
    }

    replaceTags: {
      badTagList: "argument #1 must be a tag object, a table of tag objects, an %s or an ASSTagList; got a %s."
    }

    splitAtIndexes: {
      badIndices: "argument #1 must be either a single index or table of indices got a %s."
    }

    splitAtIntervals: {
      badCallback: "argument #1 must be either a number or a callback function, got a %s."
      badCallbackReturnType: "callback must return a number, got a %s"
      invalidCallbackReturnIndex: "index returned by callback function must increase with every iteration, got %d<=%d."
    }
  }

  LineContents = createASSClass "LineContents", ASS.Base, {"sections"}, {"table"}, nil, nil, (tbl, key) ->
    if key == "textLength"
      chars = 0
      tbl\callback ((sect) -> chars += sect.len), ASS.Section.Text
      return chars

    else return getmetatable(tbl)[key]

  LineContents.new = (line, sections, copyAndCheckSections = true) =>
    sections = @getArgs({sections})[1] if sections
    logger\assert line and line.__class == Line, msgs.new.badLine, @typeName, @typeName, type line

    sections = if not sections
      ASS.Parser.LineText\getSections line
    elseif copyAndCheckSections
      @typeCheck {sections}
      util.copy sections

    -- TODO: check if typeCheck works correctly with compatible classes and doesn't do useless busy work
    if line.parentCollection
      @sub, @styles = line.parentCollection.sub, line.parentCollection.styles
      if @sub and ASSFInst.cache.lastSub != @sub
        ASSFInst.cache.lastSub = @sub
        logger\trace msgs.new.refreshingSubCache, @sub

      @scriptInfo = line.parentCollection.meta
      ASSFInst.cache.lastParentCollection = line.parentCollection
      ASSFInst.cache.lastStyles = @styles
    else
      @scriptInfo = @sub and ASS\getScriptInfo @sub

    @line, @sections = line, sections
    @updateRefs!
    return @

  LineContents.updateRefs = (prevCnt) =>
    if prevCnt != #@sections -- TODO: is is really safe to do this?
      for i, section in ipairs @sections
        section.prevSection = @sections[i-1]
        section.parent = @
        section.index = i

      return true
    else return false

  LineContents.getString = (includeEmptySections = true, currDrawingState, predicate, predicateLookAhead = true, a1, a2, a3) =>
    defDrawingState = ASS\createTag "drawing", 0
    sections, sectCnt, currSectType = @sections, #@sections
    currDrawingState or= defDrawingState
    str, s = {}, 1

    if type(predicate) == "table"
      _classes = predicate
      predicate = (section) -> ASS\instanceOf section, ASS.Section, _classes

    for i, currSect in ipairs sections
      currSectType = sections[i].class
      if predicate and not predicate currSect, i, sections, currSectType, a1, a2, a3
        continue
      elseif currSectType == ASS.Section.Text or currSectType == ASS.Section.Drawing
        -- insert a new tag section with a drawing tag if drawing state wasn't synced, yet
        nextDrawingState = currSectType == ASS.Section.Drawing and currSect.scale or defDrawingState
        if currDrawingState != nextDrawingState
          str[s], str[s+1], str[s+2], s = "{", nextDrawingState\getTagString!, "}", s+3
          currDrawingState = nextDrawingState

        str[s], s = currSect\getString!, s + 1

      elseif currSectType == ASS.Section.Tag
        sectionWritten = false
        if includeEmptySections or #currSect.tags > 0
          str[s], str[s+1] = "{", currSect\getString!
          sectionWritten = true

        if i < sectCnt
          -- add a drawing tag to this section if drawing mode changes in the next section
          nextSectType = sections[i+1].class
          if not predicate or not predicateLookAhead or predicate sections[i+1], i+1, sections, nextSectType, a1, a2, a3 -- TODO: don't run predicate on drawing section twice
            nextDrawingState = nextSectType == ASS.Section.Drawing and sections[i+1].scale or defDrawingState

            if (nextSectType == ASS.Section.Drawing or nextSectType == ASS.Section.Text) and currDrawingState != nextDrawingState
              unless sectionWritten
                str[s], str[s+1] = "{", currSect\getString!
                sectionWritten = true
              str[s+2], str[s+3], s = nextDrawingState\getTagString!, "}", s+4
              currDrawingState = nextDrawingState
              continue

        if sectionWritten
          str[s+2], s = "}", s+3

      elseif currSectType == ASS.Section.Comment
        str[s], str[s+1], str[s+2], s =  "{", currSect\getString!, "}", s+3

      else
        logger\error msgs.getString.invalidSection, @typeName, i, table.concat(table.pluck(ASS.Section, "typeName"), ", "),
          type(currSect)=="table" and currSect.typeName or type currSect

    return table.concat(str), currDrawingState

  LineContents.get = (sectionClasses, start, end_, relative) =>
    result, j = {}, 1
    cb = (section, sections, i) ->
      result[j], j = section\copy!, j + 1
    @callback cb, sectionClasses, start, end_, relative
    return result

  LineContents.callback = (callback, sectionClasses, start = 1, end_ , relative, reverse) =>
    prevCnt = #@sections
    end_ or= start < 1 and -1 or max prevCnt, 1
    reverse = relative and start<0 or reverse

    -- logger\assert (math.isInt(start) and math.isInt(end_),
    --         "arguments 'start' and 'end' to callback() must be integers, got %s and %s.", type(start), type(end_))

    logger\assert (start>0)==(end_>0) and start != 0 and end_ != 0, msgs.callback.noMixedStartEndSigns, start, end_
    logger\assert start <= end_, msgs.callback.noSwappedStartEnd, start, end_

    j, numRun, sects = 0, 0, @sections
    if start < 0
      start, end_ = relative and math.abs(end_) or prevCnt+start+1, relative and math.abs(start) or prevCnt+end_+1

    for i = reverse and prevCnt or 1, reverse and 1 or prevCnt, reverse and -1 or 1
      if sectionClasses == nil or ASS\instanceOf sects[i], sectionClasses
        j += 1
        if (relative and j >= start and j <= end_) or (not relative and i >= start and i <= end_)
          numRun += 1
          result = callback sects[i], @sections, i, j
          if result == false
            sects[i] = nil
          elseif result != nil and result != true
            sects[i] = result
            prevCnt = -1

    @sections = table.continuous @sections
    @updateRefs prevCnt
    return numRun > 0 and numRun or false

  local assSectionTypes
  LineContents.insertSections = (sections, index) =>
    index = index or #@sections+1
    if type(sections) != "table" or sections.instanceOf
      sections = {sections}

    assSectionTypes = table.values ASS.Section unless assSectionTypes  -- cache Section types list for performance
    for i, section in ipairs sections
      unless ASS\instanceOf section, assSectionTypes
        logger\error msgs.insertSections.invalidSectionType,
          table.concat(table.pluck(ASS.Section, "typeName"), ", "),
          type(section) == "table" and section.typeName or type section

      table.insert @sections, index+i-1, section

    @updateRefs!
    return sections

  LineContents.removeSections = (start, end_ = start) =>
    removed = {}
    switch type start
      when nil -- purge all sections
        @sections, removed = {}, @sections
      when "number"
        removed = list.removeRange @sections, start, end_
      when "table"
        toRemove = start.instanceOf and {[start]: true} or list.makeSet start
        j = 1
        for i, section in ipairs @sections
          if toRemove[section]
            removed[i-j+1], @sections[i] = section, nil
            section.parent, section.index, section.prevSection = nil, nil, nil
          elseif j != i
            @sections[j], j = section, j+1
          else j = i + 1

      else logger\error msgs.removeSections.badArg, type start
    @updateRefs!
    return removed

  LineContents.modTags = (tagNames, callback, start = 1, end_, relative) =>
    end_ or= start < 0 and -1 or max @getTagCount!, 1
    -- TODO: validation for start and end_
    modCnt, reverse = 0, start < 0

    cb = (section) ->
      if (reverse and modCnt < -start) or (modCnt < end_)
        sectStart = reverse and start+modCnt or max start-modCnt, 1
        sectEnd = reverse and min(end_ + modCnt, -1) or end_ - modCnt
        sectModCnt = section\modTags tagNames, callback, relative and sectStart or nil, relative and sectEnd or nil, true
        modCnt += sectModCnt or 0

    @callback cb, ASS.Section.Tag, not relative and start or nil, not relative and end_ or nil, true, reverse
    return modCnt > 0 and modCnt

  LineContents.getTags = (tagNames, start, end_, relative) =>
    tags, i = {}, 1
    @modTags tagNames, ((tag) -> tags[i], i = tag, i+1), start, end_, relative
    return tags

  LineContents.replaceTags = (tagList, start, end_, relative, insertRemaining = true) =>  -- TODO: transform and reset support
    tagList = switch type tagList
      when nil then return
      when "table"
        switch tagList.class
          when ASS.Section.Tag then ASS.TagList tagList
          when nil, ASS.TagList then ASS.TagList ASS.Section.Tag tagList
          else -- assume Tag, TODO: maybe not assume things
            singleTagList = ASS.TagList nil, @
            singleTagList.tags[tagList.__tag.name] = tagList
            singleTagList
      else logger\error msgs.replaceTags.badTagList, ASS.Section.Tag.typeName, ASS.TagList.typeName, type tagList

    toInsert = ASS.TagList tagList
    -- search for tags in line, replace them if found
    -- remove all matching global tags that are not in the first section
    cb = (section, _, i) ->
      section\callback (tag) ->
        props = tag.__tag
        if tagList.tags[props.name]
          return false if props.global and i > 1
          toInsert.tags[props.name] = nil
          return tagList.tags[props.name]\copy!
      return true

    @callback cb, ASS.Section.Tag, start, end_, relative

    globalToInsert, toInsert = toInsert\filterTags nil, {global: true}
    firstIsTagSection = #@sections > 0 and @sections[1].instanceOf[ASS.Section.Tag]
    globalSection = firstIsTagSection and @sections[1] or ASS.Section.Tag!
    -- Insert the global tag section at the beginning of the line
    -- in case it doesn't exist and we have global tags to insert.
    -- Always insert the global tags into the first section.
    if 0 < table.length globalToInsert.tags -- XXX
      @insertSections globalSection, 1 unless firstIsTagSection
      globalSection\insertTags globalToInsert

    -- insert remaining tags (not replaced) into the first processed section
    -- to easily allow for the function to be used to insert tags (overwriting existing ones)
    -- create tag section at index 1 when it was requested as the start of the replacement
    -- but doesn't yet exist
    if insertRemaining
      start or= 1
      if start == 1 and not relative and not ASS\instanceOf @sections[1], ASS.Section.Tag
        @insertSections ASS.Section.Tag(toInsert), 1
      else @insertTags toInsert, start, nil, not relative


  LineContents.removeTags = (tags, start = 1, end_, relative) =>
    end_ or= start < 0 and -1 or @getTagCount! if relative
    -- TODO: validation for start and end_
    removed, matchCnt, reverse  = {}, 0, start < 0
    cb = (section) ->
      if not relative
        list.joinInto removed, (section\removeTags tags) -- extra brackets because we only want the first return value
        return
      elseif (reverse and matchCnt < -start) or (matchCnt < end_)
        sectStart = reverse and start+matchCnt or max start-matchCnt, 1
        sectEnd = reverse and min(end_+matchCnt, -1) or end_-matchCnt
        sectRemoved, matched = section\removeTags tags, sectStart, sectEnd, true
        list.joinInto removed, sectRemoved
        matchCnt += matched

    @callback cb, ASS.Section.Tag, not relative and start or nil, not relative and end_ or nil, true, reverse
    return removed

  LineContents.insertTags = (tags, index = 1, sectionPosition, direct) =>
    if index == 0 or not math.isInt index
      logger\error msgs.insertTags.badIndex, tostring(index), type(index)

    if direct
      section = @sections[index > 0 and index or #@sections-index+1]
      logger\assert type(section) == "table" and section.class == ASS.Section.Tag,
        msgs.insertTags.badTarget, index, section and section.typeName or "<no section>"
      return section\insertTags tags, sectionPosition

    else
      local inserted
      sectFound = @callback ((section) -> inserted = section\insertTags tags, sectionPosition),
        ASS.Section.Tag, index, index, true
      if not sectFound and index == 1
        inserted = @insertSections(ASS.Section.Tag!, 1)[1]\insertTags tags
      return inserted

  LineContents.insertDefaultTags = (tagNames, index, sectionPosition, direct) =>
    defaultTags = @getDefaultTags!\filterTags tagNames
    return @insertTags defaultTags, index, sectionPosition, direct

  LineContents.getEffectiveTags = (index = 1, includeDefault, includePrevious, copyTags = true) =>
    if index == 0 or not math.isInt index
      logger\error msgs.getEffectiveTags.badIndex, tostring(index), type index

    index += #@sections+1 if index < 0
    return @sections[index]\getEffectiveTags includeDefault, includePrevious, copyTags if @sections[index]
    return @getDefaultTags nil, copyTags if includeDefault
    return ASS.TagList nil, @

  LineContents.getTagCount = =>
    cnt = 0
    cnt += #sect.tags for sect in *@sections when sect.tags
    return cnt

  LineContents.stripTags = =>
    @callback (-> false), ASS.Section.Tag
    return @

  LineContents.stripText = =>
    @callback (-> false), ASS.Section.Text
    return @

  LineContents.stripComments = =>
    @callback (-> false), ASS.Section.Comment
    return @

  LineContents.stripDrawings = =>
    @callback (-> false), ASS.Section.Drawing
    return @

  LineContents.commit = (line = @line, includeEmptySections = true, text = @getString includeEmptySections) =>
    line.text, line.undoText = text, line.text
    line\createRaw!
    return text

  LineContents.undoCommit = (line = @line) =>
    if line.undoText
      line.text, line.undoText = line.undoText
      line\createRaw!
      return true
    return false

  LineContents.cleanTags = (level = 3, mergeConsecutiveSections = true, defaultToKeep, tagSortOrder) =>
    -- Merge consecutive sections
    if mergeConsecutiveSections
      predicate = mergeConsecutiveSections if "function" == type mergeConsecutiveSections
      targetIndex, mergedCount = -1, 0
      local targetSection
      cb = (sourceSection, sections, i) ->
        if i == targetIndex + mergedCount + 1 and (not predicate or predicate sourceSection, targetSection, sections, i)
          list.joinInto targetSection.tags, sourceSection.tags
          mergedCount += 1
          return false
        else
          targetIndex, mergedCount = i, 0
          targetSection = sections[targetIndex]

      @callback cb, ASS.Section.Tag

    -- 1: remove empty sections, 2: dedup tags locally, 3: dedup tags globally
    -- 4: remove tags matching style default and not changing state, end: remove empty sections
    local tagListPrev

    if level > 3
      tagListPrev = @getDefaultTags!
      if not defaultToKeep or #defaultToKeep == 1 and defaultToKeep[1] == "position"
        -- speed up the default mode a little by using a precomputed tag name table
        tagListPrev\filterTags ASS.tagNames.noPos
      else tagListPrev\filterTags defaultToKeep, nil, false, true

    if level >= 1
      cb = (section, sections, i) ->
        return #section.tags > 0 if level < 2
        isLastSection = i == #sections

        tagList = section\getEffectiveTags false, false, false
        if level == 3 and tagListPrev
          -- strip any tags that don't change the current state excluding defaults
          tagList\diff tagListPrev
        if level >= 4
          -- strip any non-global, non-clip tags from last section (they don't have any effect)
          tagList\filterTags nil, {globalOrRectClip: true} if isLastSection

          -- strip any tags that don't change the current state including defaults
          tagList\diff tagListPrev, false, true

        return if isLastSection

        if tagListPrev
          -- update the tag state with this current section
          tagListPrev\merge tagList, false, false, false, true
        else tagListPrev = tagList

        return false if tagList\isEmpty!
        return ASS.Section.Tag tagList, false, tagSortOrder
      @callback cb, ASS.Section.Tag
    return @


  LineContents.splitAtTags = (cleanLevel = 3, reposition, writeOrigin) =>
    splitLines = {}
    cb = (section, _, i, j) ->
      splitLine = Line @line, @line.parentCollection, {ASS: {}}
      splitSections = @get ASS.Section.Tag, 1, i
      splitSections[#splitSections+1] = section
      splitLine.ASS = ASS.LineContents splitLine, splitSections
      splitLine.ASS\cleanTags cleanLevel if cleanLevel > 0
      splitLine.ASS\commit!
      splitLines[j] = splitLine

    @callback cb, ASS.Section.Text
    @repositionSplitLines splitLines, writeOrigin if reposition
    return splitLines

  LineContents.splitAtIntervals = (callback, cleanLevel = 3, reposition = true, writeOrigin) =>
    cType = type callback
    if cType == "number"
      step = callback
      callback = (idx, len) -> idx + step
    else logger\assert cType == "function", msgs.splitAtIntervals.badCallback, cType

    len = unicode.len (@copy!\stripTags!\getString!)
    idx, sectEndIdx, nextIdx, lastI = 1, 0, 0
    splitLines, splitCnt = {}, 1

    cb = (section, _, i) ->
      text, off = section.value, sectEndIdx
      sectEndIdx += section.len

      -- process unfinished line carried over from previous section
      if nextIdx > idx
        -- carried over part may span over more than this entire section
        skip = nextIdx > sectEndIdx + 1
        idx = skip and sectEndIdx + 1 or nextIdx
        addTextSection = skip and section\copy! or ASS.Section.Text text\sub 1, nextIdx - off - 1
        lastContents = splitLines[#splitLines].ASS
        lastContents\insertSections @get ASS.Section.Tag, lastI+1, i
        lastContents\insertSections addTextSection

      while idx <= sectEndIdx
        nextIdx = callback idx, len
        nextIdxType = type nextIdx
        logger\assert nextIdxType == "number", msgs.splitAtIntervals.badCallbackReturnType, nextIdxType
        nextIdx = math.ceil nextIdx
        logger\assert nextIdx > idx, msgs.splitAtIntervals.invalidCallbackReturnIndex, nextIdx, idx
        -- create a new line
        splitLine = Line @line, @line.parentCollection
        splitLine.ASS = LineContents splitLine, @get ASS.Section.Tag, 1, i
        splitLine.ASS\insertSections ASS.Section.Text unicode.sub text, idx-off, nextIdx-off-1
        splitLines[splitCnt], splitCnt = splitLine, splitCnt+1
        -- check if this section is long enough to fill the new line
        idx = sectEndIdx >= nextIdx-1 and nextIdx or sectEndIdx+1
      lastI = i

    @callback cb, ASS.Section.Text

    for splitLine in *splitLines
      splitLine.ASS\cleanTags cleanLevel if cleanLevel > 0
      splitLine.ASS\commit!

    @repositionSplitLines splitLines, writeOrigin if reposition
    return splitLines

  LineContents.splitAtIndexes = (indices, cleanLevel, reposition, writeOrigin) =>
    iType = type indices
    if iType == "number"
      indices = {indices}
    else logger\assert iType == "table", msgs.splitAtIndexes.badIndices, iType

    i = 1
    intervalGenerator = (_, len) ->
      textIndex = indices[i] or len + 1
      i += 1
      return textIndex

    return @splitAtIntervals intervalGenerator, cleanLevel, reposition, writeOrigin

  getAlignOffset = {
      [0]: (wSec, wLine) -> wSec-wLine,      -- right
      [1]: -> 0,                             -- left
      [2]: (wSec, wLine) -> wSec/2 - wLine/2 -- center
    }

  LineContents.repositionSplitLines = (splitLines, writeOrigin = true) =>
    lineWidth, xOff = @getTextExtents!, 0
    pos, _, org = @getPosition!

    for splitLine in *splitLines
      data = splitLine.ASS
      -- get tag state at last line section, if you use more than one \pos, \org or \an in a single line,
      -- you deserve things breaking around you
      effTags = data\getEffectiveTags -1, true, true, false
      sectWidth = data\getTextExtents!

      -- now trim zero-bounds characters
      -- TODO: right now this only trims known whitespace characters, but depending on the font there might be more
      -- so we actually need to check the bounds for each character in the future
      data\trim!

      -- calculate new position
      alignOffset = getAlignOffset[effTags.tags.align\get!%3] data\getTextExtents!, lineWidth
      pos = effTags.tags.position\copy!
      pos\add alignOffset + xOff, 0
      -- write new position tag to first tag section
      data\replaceTags pos

      -- if desired, write a new origin to the line if the style or the override tags contain any angle
      -- this doesn't work with \move since there's no single \org so getPosition() doesn't return one
      if writeOrigin and org
        needOrg = effTags.tags.angle\get! != 0 or 0 < #data\getTags {"angle", "angle_x", "angle_y"}
        data\replaceTags org\copy! if needOrg

      xOff += sectWidth
      data\commit!

    return splitLines

  LineContents.trim = =>
    textSects, t = {}, 0
    @callback (section, sections, i) ->
        t += 1
        textSects[t] = section,
      ASS.Section.Text

    return if t == 0
    if t == 1
      textSects[1]\trim!
    else
      textSects[1]\trimLeft!
      textSects[t]\trimRight!

  LineContents.getStyleRef = (style) =>
    if ASS\instanceOf style, ASS.Tag.String -- can use a reset tag as argument
      style = style.value

    if style == nil or style == ""
      style = @line.styleRef

    else
      sType = type style
      if sType == "string"
        style = @line.parentCollection.styles[style] or style
        logger\assert type(style) == "table", msgs.getStyleRef.noSuchStyle, style
      elseif sType != "table" or style.class != "style"
        logger\error msgs.getStyleRef.badStyle, style.typeName or style.class or sType

    return style

  LineContents.getPosition = (style, align, forceDefault) =>
    @line\extraMetrics!
    effTags = not (forceDefault and align) and @getEffectiveTags(-1, false, true, false).tags
    style = @getStyleRef style
    align or= effTags.align or style.align

    if "number" == type align
      align = ASS\createTag "align", align
    elseif align.class != ASS.Tag.Align
      logger\error msgs.getPosition.badAlign, ASS.Tag.Align.typeName, ASS\instanceOf(align) or type align

    pos = effTags.position or effTags.move
    -- default origin moves with \move which can't be expressed in a single \org tag, so none is returned
    org = effTags.origin or pos and pos.class == ASS.Point and ASS\createTag("origin", pos) or nil

    return pos, align, org if pos and not forceDefault

    -- (re)calculate position
    scriptInfo = @scriptInfo or ASS\getScriptInfo @sub

    vMargin = @line.margin_t == 0 and style.margin_t or @line.margin_t
    lMargin = @line.margin_l == 0 and style.margin_l or @line.margin_l
    rMargin = @line.margin_r == 0 and style.margin_r or @line.margin_r

    an = align\get!
    pos = ASS\createTag "position", @line.defaultXPosition[an%3+1](scriptInfo.PlayResX, lMargin, rMargin),
                    @line.defaultYPosition[math.ceil(an/3)](scriptInfo.PlayResY, vMargin)

    return pos, align, org or ASS\createTag "origin", pos

  -- TODO: make all caches members of ASSFoundation
  styleDefaultCache = {}
  decomposeAlpha = (tag, style) -> style[tag]\sub 3,4
  decomposeColor = (tag, style) -> style[tag]\sub(5,6), style[tag]\sub(7,8), style[tag]\sub(9,10)

  LineContents.getDefaultTags = (style, copyTags = true, useOvrAlign = true) =>
    style = @getStyleRef style

    -- alignment override tag may affect the default position so we'll have to retrieve it
    position, align = @getPosition style, not useOvrAlign and style.align, true
    raw = (useOvrAlign and style.align != align.value) and "#{style.raw}_#{align.value}" or style.raw

    if styleDefaultCache[raw]
      -- always return at least a fresh ASSTagList object to prevent the cached one from being overwritten
      return copyTags and styleDefaultCache[raw]\copy! or ASS.TagList styleDefaultCache[raw]

    scriptInfo = @scriptInfo or ASS\getScriptInfo @sub
    resX, resY = tonumber(scriptInfo.PlayResX), tonumber(scriptInfo.PlayResY)

    tagList = ASS.TagList nil, @
    tagList.tags = {
      scale_x:    ASS\createTag "scale_x",   style.scale_x
      scale_y:    ASS\createTag "scale_y",   style.scale_y
      align:      ASS\createTag "align",     style.align
      angle:      ASS\createTag "angle",     style.angle
      outline:    ASS\createTag "outline",   style.outline
      outline_x:  ASS\createTag "outline_x", style.outline
      outline_y:  ASS\createTag "outline_y", style.outline
      shadow:     ASS\createTag "shadow",    style.shadow
      shadow_x:   ASS\createTag "shadow_x",  style.shadow
      shadow_y:   ASS\createTag "shadow_y",  style.shadow
      bold:       ASS\createTag "bold",      style.bold
      italic:     ASS\createTag "italic",    style.italic
      underline:  ASS\createTag "underline", style.underline
      strikeout:  ASS\createTag "strikeout", style.strikeout
      spacing:    ASS\createTag "spacing",   style.spacing
      fontsize:   ASS\createTag "fontsize",  style.fontsize
      fontname:   ASS\createTag "fontname",  style.fontname

      alpha1:     ASS\createTag "alpha1", decomposeAlpha "color1", style
      alpha2:     ASS\createTag "alpha2", decomposeAlpha "color2", style
      alpha3:     ASS\createTag "alpha3", decomposeAlpha "color3", style
      alpha4:     ASS\createTag "alpha4", decomposeAlpha "color4", style
      color1:     ASS\createTag "color1", decomposeColor "color1", style
      color2:     ASS\createTag "color2", decomposeColor "color2", style
      color3:     ASS\createTag "color3", decomposeColor "color3", style
      color4:     ASS\createTag "color4", decomposeColor "color4", style

      alpha: ASS\createTag "alpha", 0
      clip_vect:  ASS\createTag "clip_vect",  {ASS.Draw.Move(0,0), ASS.Draw.Line(resX,0), ASS.Draw.Line(resX,resY), ASS.Draw.Line(0,resY), ASS.Draw.Line(0,0)}
      iclip_vect: ASS\createTag "iclip_vect", {ASS.Draw.Move(0,0), ASS.Draw.Line(0,0), ASS.Draw.Line(0,0), ASS.Draw.Line(0,0), ASS.Draw.Line(0,0)}
      clip_rect:  ASS\createTag "clip_rect",  0, 0, resX, resY,
      iclip_rect: ASS\createTag "iclip_rect", 0, 0, 0, 0,

      position:   position,
      move:       ASS\createTag "move",   position, position
      origin:     ASS\createTag "origin", position
    }

    for name, tag in pairs ASS.tagMap
      -- defaults always use default signature
      tag.props.signature = "default"
      if tag.default and not tagList.tags[name]
        tagList.tags[name] = tag.type {raw: tag.default, tagProps: tag.props}

    styleDefaultCache[style.raw] = tagList
    return copyTags and tagList\copy! or ASS.TagList tagList

  LineContents.getTextExtents = (coerce) =>   -- TODO: account for linebreaks
    width, other = 0, {0,0,0}
    cb = (section) ->
      extents = {section\getTextExtents coerce}
      width += table.remove extents, 1
      list.compareLeft other, extents, (val1, val2) -> max val1, val2
      return

    @callback cb, ASS.Section.Text
    return width, unpack other

  LineContents.getLineBounds = (noCommit, keepRawText) =>
    -- TODO: throw error if no video is open
    @commit! unless noCommit

    subInspector = ASSFInst.cache.SubInspector
    if not subInspector
      logger\assert @sub, msgs.getLineBounds.inspectorNoSub
      subInspector, msg = SubInspector @sub
      logger\error msgs.getLineBounds.inspectorError, tostring msg unless subInspector
      ASSFInst.cache.SubInspector = subInspector
      ASSFInst.cache.lastInspectorSub = @sub

    elseif @sub and @sub != ASSFInst.cache.lastInspectorSub
      logger\trace msgs.getLineBounds.inspectorRefreshingHeaders, @sub
      subInspector\updateHeader @sub
      ASSFInst.cache.lastInspectorSub = @sub

    @line.si_exhaustive = @isAnimated!

    bounds, times = subInspector\getBounds {@line}
    if bounds == nil
      logger\error getLineBounds.inspectorError, tostring times

    lineBounds = ASS.LineBounds @line, bounds, times.frames, nil, nil, nil, keepRawText
    @undoCommit! unless noCommit
    return lineBounds

  LineContents.getTextMetrics = (calculateBounds, coerce) =>
    metr = {ascent: 0, descent: 0, internal_leading: 0, external_leading: 0, height: 0, width: 0}
    bounds = calculateBounds and {0, 0, 0, 0}

    cb = (section, sections, i, j) ->
      sectMetr = section\getTextMetrics calculateBounds, coerce
      -- combine type bounding boxes
      if calculateBounds
        if j == 1
          bounds[1], bounds[2] = sectMetr.bounds[1] or 0, sectMetr.bounds[2] or 0
        bounds[2] = min bounds[2],sectMetr.bounds[2] or 0
        bounds[3] = bounds[1] + sectMetr.bounds.w
        bounds[4] = max bounds[4],sectMetr.bounds[4] or 0

      -- add all section widths
      metr.width += sectMetr.width
      -- get maximum encountered section values for all other metrics (does that make sense?)
      metr.ascent = max sectMetr.ascent, metr.ascent
      metr.descent = max sectMetr.descent, metr.descent
      metr.internal_leading = max sectMetr.internal_leading, metr.internal_leading
      metr.external_leading = max sectMetr.external_leading, metr.external_leading
      metr.height = max sectMetr.height, metr.height

    @callback cb, ASS.Section.Text

    if calculateBounds
      bounds.w, bounds.h = bounds[3] - bounds[1], bounds[4] - bounds[2]
      metr.bounds = bounds

    return metr

  LineContents.getSectionCount = (classes) =>
    if classes
      cnt = 0
      @callback ((section, _, _, j) -> cnt = j), classes, nil, nil, true
      return cnt

    cnt = {}
    @callback (section) ->
      cls = section.class
      cnt[cls] = cnt[cls] and cnt[cls] + 1 or 1

    return cnt, #@sections

  LineContents.getTextLength = =>
    len = 0
    @callback ((section) -> len += section.len), ASS.Section.Text
    return len

  LineContents.isAnimated = =>
    line, xres, effTags = @line, aegisub.video_size!, @getEffectiveTags -1, false, true, false
    frameCount = xres and aegisub.frame_from_ms(line.end_time) - aegisub.frame_from_ms(line.start_time)
    t = effTags.tags
    -- single frame lines are by definition not animated
    return false if xres and frameCount < 2

    -- any karaoke tag
    for karaTag in *ASS.tagNames.karaoke
      if t[karaTag] and t[karaTag].value * t[karaTag].__tag.scale < line.duration
        -- TODO: this is broken right now due to incorrect handling of kara tags in getEffectiveTags
        return true

    -- any transform
    return true if #effTags.transforms > 0

    -- moves of positive, non-zero move duration that actually change the text position
    if t.move and not t.move.startPos\equal t.move.endPos
      return true if t.move.startTime < t.move.endTime or t.move.endTime\equal 0

    -- complex fade-outs or fade-ins of positive, non-zero fade duration that actually change the opacity
    if t.fade
      return true if t.fade.inDuration > 0 and not t.fade.inAlpha\equal t.fade.midAlpha
      return true if t.fade.outDuration > 0 and not t.fade.midAlpha\equal t.fade.outAlpha

    -- -- simple fade-outs or fade-ins of positive, non-zero fade duration that actually change the opacity
    if t.fade_simple
      return true if t.fade_simple.inDuration > 0 and not t.fade_simple.inAlpha\equal t.fade_simple.midAlpha
      return true if t.fade_simple.outDuration > 0 and not t.fade_simple.midAlpha\equal t.fade_simple.outAlpha

    return false

  LineContents.reverse = =>
    reversed = {}

    cb = (section, _, _, j) ->
      reversed[j*2-1] = ASS.Section.Tag section\getEffectiveTags true, true
      reversed[j*2] = section\reverse!

    @callback cb, ASS.Section.Text, nil, nil, nil, true
    @sections = reversed
    @updateRefs!
    return @cleanTags 4

  return LineContents
