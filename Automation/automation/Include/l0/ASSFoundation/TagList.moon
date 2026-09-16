return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  min, max = math.min, math.max
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    new: {
      badTagSource: "a %s can only be constructed from an %s or %s; got a %s."
    }
    diff: {
      badTagList: "can only diff %s objects, got a %s."
    }
    filterTags: {
      badTagName: "invalid tag name #%d '(%s)'. expected a string, got a %s"
      badTagNames: "argument #1 must be either a single or a table of tag names, got a %s."
    }
    getStyleTable: {
      badStyleRef: "argument #1 must be a style table, got a %s."
    }
    merge: {
      badTagList: "can only merge %s objects, got a %s for argument #%d."
    }
  }

  TagList = createASSClass "TagList", ASS.Base, {"tags", "transforms", "multiTags" ,"reset", "startTime", "endTime", "accel"},
    {"table", "table", "table", ASS.String, ASS.Time, ASS.Time, ASS.Number}

  TagList.new = (tags, contentRef) =>
    if ASS\instanceOf tags, ASS.Section.Tag
      @tags, @transforms, @multiTags = {}, {}, {}
      @contentRef = tags.parent
      transIdx, transforms, ovrTransTags, transTags = 1, {}, {}
      seenVectClip = false
      seenPosTag = false

      tags\callback (tag) ->
        tagProps = tag.__tag

        -- Discard all previous non-global tags when a reset is encountered
        if tagProps.name == "reset"
          @tags, @reset = @getGlobal(true), tag

          -- also filter transforms
          for i = 1, #transforms
            keep = false
            transforms[i].tags\callback (tag) ->
              -- Vectorial clips are not "global" but can't be reset
              -- TODO: make "global" more clear
              if tag.instanceOf[ASS.Tag.ClipRect]
                keep = true
              else return false

            transforms[i] = nil unless keep
          return

        -- Transforms are stored in a separate table because there can be more than one.
        elseif tag.instanceOf[ASS.Tag.Transform]
          -- create a shallow copy of the transform for filtering purposes
          transforms[transIdx] = ASS.Tag.Transform{tag, transformableOnly: true}
          transTags = transforms[transIdx].tags.tags
          transIdx += 1
          return

        elseif tag.__tag.multi
          @multiTags[#@multiTags+1] = tag
          return

        -- Discard all except the first instance of global tags.
        -- This expects all global tags to be non-transformable which is true for ASSv4+
        return if tagProps.global and @tags[tagProps.name]
        return if seenPosTag and tagProps.position
        -- Since there can be only one vectorial clip or iclip at a time, only keep the first one
        return if seenVectClip and tag.instanceOf[ASS.Tag.ClipVect]
        -- discard child tag identical to active master tag
        return if tagProps.master and tag\equal @tags[tagProps.master], true

        @tags[tagProps.name] = tag
        if tagProps.transformable
          -- When the list is converted back into an ASSTagSection, the transforms are written to its end,
          -- so we have to make sure transformed tags are not overridden afterwards.
          -- If a transformable tag is encountered, its entry in the overridden transforms list
          -- is set to the nummber of the last transform(+1), so the tag can be purged from all previous transforms.
          ovrTransTags[tagProps.name] = transIdx
          if tagProps.children
            ovrTransTags[tagProps.children[i]] = transIdx for i = 1, #tagProps.children

        elseif tag.instanceOf[ASS.Tag.ClipVect]
          seenVectClip = true
        elseif tagProps.position
          seenPosTag = true

        -- purge all overriden child tags (such as \1a, \2a, etc in the case of \alpha)
        -- TODO: do not purge children that appear after master transforms
        if tagProps.children
          @tags[tagProps.children[i]] = nil for i = 1 , #tagProps.children

      -- filter tags by overridden transform list, keep transforms that have still tags left at the end
      t = 1
      for i = 1, transIdx - 1
        transform = transforms[i]
        continue unless transform
        transTagCnt = 0
        local tagsToInsert
        transform.tags\callback (tag) ->
          ovrEnd = ovrTransTags[tag.__tag.name] or 0
          -- drop all overridden transforms
          if ovrEnd > i
            return false
          else if tag.__tag.children
            -- check for any master tag transforms later overriden in children
            -- such as {\alpha&HF0&\t(28,279,\alpha&H00&)\1a&HFF&}
            ovrChildren = [child for child in *tag.__tag.children when ovrTransTags[child] and ovrTransTags[child] > i]
            if #ovrChildren == 0
              transTagCnt += 1
              return

            -- add all children except the ones overriding to the transform and remove the master
            childTags = [ASS\createTag childTagName,
              tag\getTagParams! for childTagName in *list.diff tag.__tag.children, ovrChildren]
            if tagsToInsert == nil
              tagsToInsert = childTags
            else list.joinInto tagsToInsert, childTags
            transTagCnt += #childTags
            return false

          else transTagCnt += 1
        transform.tags\insertTags tagsToInsert if tagsToInsert != nil
        -- write final transforms table
        if transTagCnt > 0
          @transforms[t] = transform
          t += 1

    elseif ASS\instanceOf(tags, TagList)
      @tags = util.copy tags.tags
      @transforms = util.copy tags.transforms
      @multiTags = util.copy tags.multiTags

      @reset, @contentRef = tags.reset, tags.contentRef
    elseif tags == nil
      @tags, @transforms, @multiTags = {}, {}, {}
    else logger\error msgs.new.badTagSource, TagList.typeName, ASS.Section.Tag.typeName, TagList.typeName,
      ASS\instanceOf(tags) and tags.typeName or type tags

    @contentRef or= contentRef

  TagList.get = => {name, tag\get! for name, tag in pairs @tags}

  TagList.checkTransformed = (tagName) =>
    set = {}
    for i = 1, #@transforms
      for j = 1, #@transforms[i].tags.tags
        set[@transforms[i].tags.tags[j].__tag.name] = true

    return tagName and set[tagName] or set

  TagList.merge = (tagLists, copyTags = true, returnOnly, overrideGlobalTags, expandResets) =>
    tagLists = {tagLists} if ASS\instanceOf tagLists, TagList

    merged, ovrTransTags, resetIdx = TagList(@), {}, 0
    seenTransform = #@transforms > 0
    seenVectClip = @tags.clip_vect or @tags.iclip_vect

    if expandResets and @reset
      merged.tags = (merged\getDefaultTags(merged.reset)\merge merged.tags, false).tags

    for i = 1, #tagLists
      logger\assert ASS\instanceOf(tagLists[i],TagList), msgs.merge.badTagList,
        TagList.typeName, type(tagLists[i]), i

      -- apply resets
      if tagLists[i].reset
        resetIdx = i

        merged.tags = if expandResets
          expReset = tagLists[i].contentRef\getDefaultTags tagLists[i].reset
          if overrideGlobalTags
            expReset
          else
            (expReset\merge merged\getGlobal(true), false).tags
        else
          -- discard all previous non-global tags when a reset is encountered
          merged.reset = tagLists[i].reset
          merged\getGlobal true

      seenTransform or= #tagLists[i].transforms > 0

      -- merge override tags
      for name, tag in pairs tagLists[i].tags
        tagProps = tag.__tag

        if not overrideGlobalTags
          -- discard all except the first instance of global tags
          continue if merged.tags[name] and tagProps.global
          -- discard all vectorial clips if one (\clip or \iclip) was already seen
          continue if seenVectClip and tag.instanceOf[ASS.Tag.ClipVect]

        -- when overriding tags, make sure vectorial \iclip and \clip overwrite each other
        elseif tag.instanceOf[ASS.Tag.ClipVect]
          merged.tags.clip_vect, merged.tags.iclip_vect = nil, nil

        merged.tags[name] = tag

        -- mark transformable tags in previous transform lists as overridden
        if seenTransform and tagProps.transformable
          ovrTransTags[tagProps.name] = i

        -- master tag overrides its children
        if tagProps.children
          for i = 1, #tagProps.children
            merged.tags[tagProps.children[i]] = nil

      -- Tags w/ multiple possible appearances require special treatment because a single instance doesn't fully
      -- overwrite the state established by the previous tag instance.
      -- Karaoke tags cause karaoke timings to be offset in the following sections, so we have to keep all of them
      -- around in some way. When using more than 1 karaoke tag in a section, the time parameters of first n-1 tags
      -- are added together to make up the timing offset.
      -- We know nothing about unknown tags, so we must assume every one of them contributes to the state and
      -- keep them all around
      -- Conveniently all of the multi tags we know (which are the 3 karaoke tags and possibly every unknown tag)
      -- are not affected by resets so we can ignore any we encounter.
      for tag in *tagLists[i].multiTags
        -- karaoke tags are a special case  that do never override state but potentially all contribute to it
        -- within or between sections making it necessary to keep all of them around when merging.
        -- Such a tag may or may not exist in the wild but (it does not in ASSv4+), so we err on the side of caution
        -- and assume this behavior of all unknown tags we encounter
        logger\assert tag.__tag.karaoke or tag.__tag.nonOverriding, msgs.merge.noMergeStrategyForMultiTag, tag.__tag.name
        merged.multiTags[#merged.multiTags+1] = tag

    -- merge transforms
    merged.transforms = {}
    if seenTransform
      t = 1
      for i = 0, #tagLists
        transforms = i == 0 and @transforms or tagLists[i].transforms
        for j = 1, #transforms
          transform = i == 0 and transforms[j] or ASS.Tag.Transform{transforms[j]}
          transTagCnt = 0

          transform.tags\callback (tag) ->
            ovrEnd = ovrTransTags[tag.__tag.name] or 0
            -- remove transforms overwritten by resets or the override table
            if resetIdx > i and not tag.instanceOf[ASS.Tag.ClipRect] or ovrEnd > i
              return false
            else transTagCnt += 1

          -- fill final transforms table
          if transTagCnt > 0
            merged.transforms[t] = transform
            t += 1

    merged = merged\copy! if copyTags

    if returnOnly
      return merged
    else
      @tags, @reset, @transforms, @multiTags = merged.tags, merged.reset, merged.transforms, merged.multiTags
      return @

  -- gets the change in tag state caused by applying this line state onto a previous line state
  -- returnOnly note: only provided because copying the tag list before diffing may be much slower
  -- TODO: change "returnOnly" -> "mutate" to when breaking API
  TagList.diff = (previous, returnOnly, ignoreGlobalState) =>
    logger\assert ASS\instanceOf(previous,TagList), msgs.diff.badTagList,
      TagList.typeName, type(previous)

    -- resets can only be identical if the previous section didn't modify the state after its reset
    reset = @reset
    reset = if @reset and table.length(previous.tags) == 0
      if previous.reset
        nil if @reset.value == previous.reset.value
      elseif @reset.value == "" or @reset.value == previous.contentRef\getStyleRef().name
        nil

    thisResetDefaults = @contentRef\getDefaultTags reset if reset
    previousResetDefaults = previous.contentRef\getDefaultTags previous.reset if previous.reset
    previousTransSet = previous\checkTransformed!
    diff = TagList nil, @contentRef if returnOnly

    for name, tag in pairs @tags
      isGlobal = tag.__tag.global and not ignoreGlobalState

      -- if this tag list contains a reset, we need to compare its local tags to the default values set by the reset
      -- instead of to the values of the previous tag list
      ref = (reset and not global) and thisResetDefaults or previous

      isDiff = if isGlobal
        -- Since global tags can't be overwritten, only treat global tags
        -- that are not present in the previous tag list as different.
        -- There can be only vector \iclip or \clip at the time, so treat any in this list
        -- only as different if there are neither in the previous list.
        if previous.tags[name] then false
        elseif tag.instanceOf[ASS.Tag.ClipVect] and (previous.tags.clip_vect or previous.tags.iclip_vect)
          false
        else true
      elseif previousTransSet[name]
        -- all local tags transformed in the previous section will change state when used in this section
        -- unless they are nuked by a reset which affects every non-global tag except rectangular clips
        if reset and not tag.instanceOf[ASS.Tag.ClipRect] then false
        else true
      elseif tag.__tag.children
        if ref.tags[name] and not tag\equal ref.tags[name]
          true
        else
          isDifferentFromRefChildren = false
          for childTagName in *tag.__tag.children
            if not ref.tags[childTagName] or not ref.tags[childTagName]\equal tag, false, true
              isDifferentFromRefChildren = true
              break
          isDifferentFromRefChildren
      elseif ref.tags[name]
        -- decimate tags that are both present and equal in this and the previous section
        not tag\equal ref.tags[name]
      elseif previousResetDefaults and tag\equal previousResetDefaults.tags[name]
        -- decimate tags that are identical to the state set by a reset in the previous section
        false
      elseif tag.__tag.master
        -- child tags (such as \1a and \2a in the case of \alpha) change the state
        -- if they differ from a non-overriden master in the previous section
        -- and must also be included if the master tag present in this section
        -- changes state from the previous section
        masterTagName = tag.__tag.master
        if not tag\equal ref.tags[masterTagName] then true
        -- TODO: if the child tag is following a transform tag, then the reference master tag is actually the one in the transform (if present)
        elseif @tags[masterTagName] and not @tags[masterTagName]\equal ref.tags[masterTagName], true
          true
        else false

      else not ref.tags[name] -- TODO: optimize away second lookup into ref.tags[name]

      if isDiff and returnOnly
        diff.tags[name] = tag
      elseif not isDiff and not returnOnly
        @tags[name] = nil

    -- Tags w/ multiple possible appearances require special treatment because a single instance doesn't fully
    -- overwrite the state established by the previous tag instance.
    -- The offset and duration parameters of karaoke tags are relative to those of karaoke tags in previous sections
    -- so they always must be part of the diff even if parameters are identical. We know nothing about the behavior
    -- of unknown tags, so we consider them as non-overriding and keep them in the diff as well.
    -- conveniently all of the multi tags we know (which are the 3 karaoke tags and possibly every unknown tag)
    -- are not affected by resets so we can ignore any we encounter.
    for tag in *previous.multiTags
      logger\assert tag.__tag.karaoke or tag.__tag.nonOverriding, msgs.diff.noDiffStrategyForMultiTag, tag.__tag.name

    if returnOnly
      diff.reset = reset
      -- transforms can't be deduplicated so all of them will be kept in the diff
      diff.transforms = @transforms
      diff.multiTags = @multiTags
      return diff

    else
      @reset = reset
      return @

  TagList.getTagParams = (name, asBool, multiValue) =>
    if @tags[name]
      vals = multiValue and {@tags[name]\getTagParams!} or @tags[name]\getTagParams!
      return if asBool and not multiValue
        vals > 0
      else vals

  TagList.getCombinedColor = (num, styleRef) =>
    alphaName, colorName = "alpha" .. tostring(num), "color" .. tostring(num)
    alpha = @getTagParams alphaName
    color = @getTagParams colorName, false, true

    combined = alpha and "&H%02X"\format(alpha) or styleRef[colorName]\sub 1,4
    combined ..= color and #color == 3 and "%02X%02X%02X&"\format(unpack color) or styleRef[colorName]\sub 5
    return combined

  TagList.getStyleTable = (styleRef, name, coerce) =>
    styleRefType = type styleRef
    logger\assert styleRefType == "table" and styleRef.class == "style",
      msgs.getStyleTable.badStyleRef, styleRefType

    sTbl = table.merge {
      name: name or styleRef.name
      id: util.uuid!
      align: @getTagParams "align"
      angle: @getTagParams "angle"
      bold: @getTagParams "bold", true
      color1: @getCombinedColor 1, styleRef
      color2: @getCombinedColor 2, styleRef
      color3: @getCombinedColor 3, styleRef
      color4: @getCombinedColor 4, styleRef
      encoding: @getTagParams "encoding"
      fontname: @getTagParams "fontname"
      fontsize: @getTagParams "fontsize"
      italic: @getTagParams "italic", true
      outline: @getTagParams "outline"
      underline: @getTagParams "underline", true
      scale_x: @getTagParams "scale_x"
      scale_y: @getTagParams "scale_y"
      shadow: @getTagParams "shadow"
      spacing: @getTagParams "spacing"
      strikeout: @getTagParams "strikeout", true
    }, styleRef, false

    sTbl.raw = string.formatEx "Style: %s,%s,%N,%s,%s,%s,%s,%B,%B,%B,%B,%N,%N,%N,%N,%d,%N,%N,%d,%d,%d,%d,%d",
      sTbl.name, sTbl.fontname, sTbl.fontsize, sTbl.color1, sTbl.color2, sTbl.color3, sTbl.color4,
      sTbl.bold, sTbl.italic, sTbl.underline, sTbl.strikeout, sTbl.scale_x, sTbl.scale_y,
      sTbl.spacing, sTbl.angle, sTbl.borderstyle, sTbl.outline, sTbl.shadow, sTbl.align,
      sTbl.margin_l, sTbl.margin_r, sTbl.margin_t, sTbl.encoding

    return sTbl

  defaultPropCheckExempts = {"reset"}
  defaultPropCheckExemptsSet = reset: true

  TagList.filterTags = (tagNames, tagProps, returnOnly, inverseNameMatch, propCheckExempts = defaultPropCheckExempts) =>
    tagPropCnt = tagProps and table.length(tagProps) or 0
    tagNames = switch type tagNames
      when "string"
        {tagNames}
      when "nil"
        return returnOnly and @copy! or @ if tagPropCnt == 0
        ASS.tagNames.all
      when "table"
        return TagList nil, @contentRef if #tagNames == 0
        inverseNameMatch and list.diff(ASS.tagNames.all, tagNames) or tagNames
      else logger\error msgs.filterTags.badTagNames, type tagNames

    propCheckExemptsSet = if propCheckExempts == defaultPropCheckExempts
      defaultPropCheckExemptsSet
    else list.makeSet propCheckExempts

    included, removed = TagList(nil, @contentRef), TagList nil, @contentRef
    selected, transformTarget = {}, removed

    for i, name in ipairs tagNames
      logger\assert type(name) == "string", msgs.filterTags.badTagName,
                    i, tostring(name), type name
      haveTag = name == "reset" and @reset or @tags[name]

      -- check if tag properties match
      propMatch = if tagPropCnt != 0 and haveTag and not propCheckExemptsSet[name]
        propDiff, propDiffCnt = table.diff tagProps, @tags[name].__tag, false, (l, r, x) ->
          l == false and not r or l == r -- tag props are sparse, so nil implies false
        propDiffCnt == 0
      else true

      target = propMatch and haveTag and included or removed

      if name == "reset"
        target.reset = haveTag
      elseif name == "transform"
        transformTarget = included -- TODO: filter transforms by type
      elseif @tags[name]
        target.tags[name] = haveTag

    transformTarget.transforms = returnOnly and util.copy(@transforms) or @transforms

    if returnOnly
      return included, removed

    @tags, @reset, @transforms = included.tags, included.reset, included.transforms
    return @, removed

  TagList.isEmpty = =>
    table.length(@tags) < 1 and not @reset and #@transforms == 0 and #@multiTags == 0

  TagList.getGlobal = (includeRectClips) =>
    {name, tag for name, tag in pairs @tags when includeRectClips and tag.__tag.globalOrRectClip or tag.__tag.global}

  return TagList
