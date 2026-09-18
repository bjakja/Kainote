return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    new: {
      unsupportedTag: "supplied tag %d (a %s with name '%s') is not a supported tag."
    }

    callback: {
      badFirstLastType: "arguments #3 (first) and #4 (last) must both be integers, got %s and %s."
      badRange: "arguments #3 (first) and #4 (last) must be either both >0 or both <0 with first <= last, got %d and %d."
      badTagNames: "argument #2 (tagNames) must be either a table of strings or a single string, got %s."
    }

    insertTags: {
      badIndex: "argument #2 (index) must be an integer != 0, got '%s' of type %s."
      badTag: "argument %d to insertTags! must be a tag object, got a %s"
      unrecognizedTag: "can't insert tag #%d of type %s: no tag with name '%s'."
      badTags: "argument #1 (tags) must be one of the following: a Tag object, a table of Tag objects, a Tag Section or a TagList; got a %s."
      tagNameClassMismatch: "can't insert tag #%d with name '%s': expected type was %s, got %s."
    }

    removeTags: {
      badTag: "argument %d to removeTags() must be either a tag name or a tag object, got a %s." -- TODO: rewrite
    }
  }

  TagSection = createASSClass "Section.Tag", ASS.Base, {"tags"}, {"table"}
  TagSection.getStyleTable = ASS.Section.Text.getStyleTable

  -- TODO: replace transformableOnly with customizable filter
  TagSection.new = (tags, transformableOnly, tagSortOrder = ASS.tagSortOrder) =>
    tagsType = type tags

    -- create a tag section from a TagList
    if tagsType == "table" and tags.class == ASS.TagList
      -- TODO: check if it's a good idea to work with refs instead of copies
      @tags, t = {}, 1
      if tags.reset
        @tags[1], t = tags.reset, 2

      for i = 1, #tagSortOrder
        tag = tags.tags[tagSortOrder[i]]
        if tag and (not transformableOnly or tag.__tag.transformable or tag.instanceOf[ASS.Tag.Unknown])
          @tags[t] = tag
          t += 1

      list.joinInto @tags, tags.transforms, tags.multiTags

    -- create a blank tag section
    elseif tags == nil
      @tags = {}

    -- import tag objects from another tag section
    elseif ASS\instanceOf tags, TagSection
      @parent = tags.parent
      j, otherTags = 1, tags.tags
      @tags = {}
      for tag in *otherTags
        if not transformableOnly or (tag.__tag.transformable or tag.instanceOf[ASS.Tag.Unknown])
          @tags[j] = tag
          @tags[j].parent = @
          j += 1

    -- parse raw tags
    elseif tagsType == "string" or tagsType == "table" and #tags == 1 and type(tags[1]) == "string"
      tags = tags[1] if tagsType == "table"
      @tags = ASS.Parser.Sections\parseTags tags
      for tag in *@tags
        tag.parent = @

    -- create a tag section from a list of tag objects
    elseif type(tags) == "table"
      @tags = {}
      allTags = ASS.tagNames.all
      for i = 1, #tags
        tag = tags[i]

        tagType = type tag
        unless tagType == "table" and tag.__tag and allTags[tag.__tag.name or false]
          logger\error msgs.new.unsupportedTag, i,
            tagType == "table" and tags[i].typeName or tagType,
            tagType == "table" and tag.__tag and tag.__tag.name or "none"

        @tags[i], tag.parent = tag, @

    else
      @tags = @getArgs({tags})[1]
      @typeCheck{@tags}

    return @

  TagSection.callback = (callback, tagNames, first = 1, last, relative, reverse) =>
    tagSet, prevCnt = {}, #@tags
    last or= first >= 1 and math.max(prevCnt,1) or -1
    reverse = relative and first<0 or reverse

    logger\assert math.isInt(first) and math.isInt(last), msgs.callback.badFirstLastType,
      type(first), type(last)
    logger\assert (first>0) == (last>0) and first != 0 and last != 0 and first <= last,
      msgs.callback.badRange, first, last

    if type(tagNames) == "string"
      tagNames = {tagNames}
    if tagNames
      logger\assert type(tagNames) == "table", msgs.callback.badTagNames,
        type(tagNames)
      tagSet[name] = true for name in *tagNames

    j, numRun, tags, rmCnt = 0, 0, @tags, 0
    @toRemove = {}

    if first < 0
      first, last = relative and math.abs(last) or prevCnt+first+1, relative and math.abs(first) or prevCnt+last+1

    for i=reverse and prevCnt or 1, reverse and 1 or prevCnt, reverse and -1 or 1 do
      if not tagNames or tagSet[tags[i].__tag.name]
        j += 1
        if (relative and j>=first and j<=last) or (not relative and i>=first and i<=last)
          result = callback tags[i], @tags, i, j, @toRemove
          numRun += 1
          if result == false
            @toRemove[tags[i]], @toRemove[rmCnt+1], rmCnt = true, i, rmCnt+1
            if tags[i].parent == @ then tags[i].parent=nil
          elseif result != nil and result != true
            tags[i] = result
            tags[i].parent = @


    -- delay removal of tags until the all contours have been processed
    if rmCnt > 0
      list.removeIndices tags, @toRemove

    @toRemove = {}

    return numRun > 0 and numRun or false

  TagSection.modTags = (tagNames, callback, first, last, relative) =>
    @callback callback, tagNames, first, last, relative

  TagSection.getTags = (tagNames, first, last, relative) =>
    tags = {}
    @callback ((tag) -> tags[#tags+1] = tag),
      tagNames, first, last, relative
    return tags

  TagSection.remove = => not @parent and @ or @parent\removeSections @

  TagSection.removeTags = (tags, first, last, relative) =>
    if type(tags) == "number" and relative == nil
      -- when called without tags parameter, delete all tags in range
      tags, first, last, relative = nil, tags, first, last

    if #@tags == 0
      return {}, 0

    elseif not (tags or first or last)
      -- remove all tags if called without parameters
      removed, @tags = @tags, {}
      return removed, #removed

    first or= 1
    last or= first and first<0 and -1 or #@tags
    -- wrap single tags and tag objects
    if tags != nil and (type(tags) != "table" or ASS\instanceOf tags)
      tags = {tags}


    tagNames, tagObjects, removed, reverse = {}, {}, {}, first<0
    -- build sets
    if tags and #tags > 0
      for i, tag in ipairs tags
        if ASS\instanceOf tag
          tagObjects[tag] = true
        elseif type(tag) =="string"
          tagNames[ASS\mapTag(tag).props.name] = true
        else logger\error msgs.remove.badTag, i, type tag

    if reverse and relative
      first, last = math.abs(last), math.abs(first)

    -- remove matching tags
    matched = 0
    callback = (tag) ->
      if tagNames[tag.__tag.name] or tagObjects[tag] or not tags
        matched += 1
        if not relative or (matched >= first and matched <= last)
          removed[#removed+1], tag.parent = tag
          return false

    @callback callback, nil, not relative and first or nil,
      not relative and last or nil, false, reverse

    return removed, matched

  TagSection.insertTags = (tags, index) =>
    prevCnt, inserted = #@tags, {}
    index = math.max prevCnt, 1 if index == nil
    logger\assert math.isInt(index) and index != 0,
      msgs.insertTags.badIndex, tostring(index), type index

    tags = if type(tags) == "table"
      if tags.instanceOf == nil
        tags
      elseif tags.instanceOf[TagSection]
        tags.tags
      elseif tags.instanceOf[ASS.TagList]
        TagSection(tags).tags
      else {tags}
    else logger\error msgs.insertTags.badTags, type tags

    for i = 1, #tags
      cls = tags[i].class
      logger\error msgs.insertTags.badTag, i, type tags[i] unless cls

      tagData = ASS.tagMap[tags[i].__tag.name]
      if not tagData
        logger\error msgs.insertTags.unrecognizedTag,
          i, tags[i].typeName, tags[i].__tag.name
      elseif cls != tagData.type
        logger\error msgs.insertTags.tagNameClassMismatch,
          i, tags[i].__tag.name, tagData.type.typeName, tags[i].typeName

      insertIdx = index<0 and prevCnt+index+i or index+i-1
      table.insert @tags, insertIdx, tags[i]
      tags[i].parent, tags[i].deleted = @, false
      inserted[i] = @tags[insertIdx]

    return #inserted>1 and inserted or inserted[1]

  TagSection.insertDefaultTags = (tagNames, index) =>
    defaultTags = @parent\getDefaultTags!

    tags = if type(tagNames) == "string"
      defaultTags.tags[tagNames]
    else [defaultTags.tags[tagName] for tagName in *tagNames]

    return @insertTags tags, index

  TagSection.getString = =>
    tagStrings = {}
    @callback (tag, _, i) ->
      tagStrings[i] = tag\getTagString @

    return table.concat tagStrings

  -- TODO: properly handle transforms, include forward sections for global tags
  TagSection.getEffectiveTags = (includeDefault, includePrevious = true, copyTags = true) =>
    -- previous and default tag lists
    local effTags
    if includeDefault
      effTags = @parent\getDefaultTags(nil, copyTags)

    if includePrevious and @prevSection
      prevTagList = @prevSection\getEffectiveTags false, true, copyTags
      effTags = includeDefault and effTags\merge(prevTagList, false, false, true) or prevTagList
      includeDefault = false

    -- tag list of this section
    tagList = copyTags and ASS.TagList(@)\copy! or ASS.TagList @
    return effTags and effTags\merge(tagList, false, nil, includeDefault) or tagList

  return TagSection
