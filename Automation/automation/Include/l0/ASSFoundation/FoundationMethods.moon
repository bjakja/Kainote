return (createASSClass, Functional, LineCollection, Line, logger) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    createLine: {
      badRef: "argument #2 (ref) must be a Line, LineCollection or %s object or nil; got a %s."
      badLine: "argument #1 (contents) must be a Line or %s object, a section or a table of sections, a raw line or line string, or nil; got a %s."
      missingCollection: "can only create a Line with a reference to a LineCollection, but none could be found."
    }

    getScriptInfo: {
      badSub: "can't get script info because no valid subtitles object was supplied or cached."
    }

    addStyle: {
      badSub: "no valid subtitles object was supplied or cached."
    }

    createTag: {
      badType: "argument #1 must be a string, got a %s."
      noSuchTag: "can't find tag with name '%s'"
    }

  }

  ASS = createASSClass "ASSFoundation"

  ASS.new = =>
    @cache = {}
    return @

  ASS.getTagNames = (ovrNames) =>
    if "string" == type ovrNames
      if name = @tagMap[ovrNames]
        return name
      ovrNames = {ovrNames}

    tagNames, t = {}, 1
    for name in *ovrNames
      ovrToTag = @tagNames[name]
      if ovrToTag and ovrToTag.n == 1
        tagNames[t] = ovrToTag[1]
      elseif ovrToTag
        tagNames, t = list.joinInto tagNames, ovrToTag
      elseif @tagMap[name]
        tagNames[t] = name
      t += 1

    return tagNames

  ASS.addStyle = (tagList, name, styleRef, sub) =>
    style = tagList\getStyleTable styleRef, name
    sub = assert type(sub) == "userdata" and sub.insert or tagList.contentRef.line.parentCollection.sub or @cache.lastSub,
      msgs.addStyle.badSub

    styles, s = {}
    for i, line in ipairs sub
      if line.class == "style"
        styles[line.name], s = line, i
      elseif s then break

    sub.insert s + 1, style
    styles[style.name], @cache.lastStyles = style, styles

  ASS.createTag = (name, ...) =>
    type_ = type name
    logger\assert type_ == "string", msgs.createTag.badType, type_

    tag = @tagMap[name]
    logger\assert tag, msgs.createTag.noSuchTag, name
    return tag.type {tagProps: tag.props, ...}

  local assSectionTypes
  ASS.createLine = (args) =>
    defaults, newLine = @defaults.line
    cnts, ref, useLineProps = args[1], args[2], args[3]

    ref = if "table" == type ref
      if ref.__class == Line
        ref.parentCollection
      elseif ref.class == @LineContents
        ref.line.parentCollection
      elseif ref.__class != LineCollection
        error msgs.createLine.badRef\format @LineContents.typeName, ref.typeName or "table"
    elseif ref != nil
      error msgs.createLine.badRef\format @LineContents.typeName, type ref

    if not cnts
      error msgs.createLine.badLine unless ref
      newLine = Line {}, ref, table.union defaults, args
      newLine\parse!

    elseif type(cnts) == "string"
      error msgs.createLine.missingCollection unless ref
      p, s, num = {}, {cnts\match "^Dialogue: (%d+),(.-),(.-),(.-),(.-),(%d*),(%d*),(%d*),(.-),(.-)$"}, tonumber

      if #s == 0
        p = util.copy defaults
        p.text = cnts
      else
        p.layer, p.start_time, p.end_time, p.style = num(s[1]), util.assTimecode2ms(s[2]), util.assTimecode2ms(s[3]), s[4]
        p.actor, p.margin_l, p.margin_r, p.margin_t, p.effect, p.text = s[5], num(s[6]), num(s[7]), num(s[8]), s[9], s[10]

      newLine = Line {}, ref, table.union args, p, defaults
      @parse newLine

    elseif type(cnts) != "table"
      error msgs.createLine.badLine\format @LineContents.typeName, type cnts

    elseif cnts.__class == Line
      -- Line objects will be copied and the ASSFoundation stuff committed and reparsed (full copy)
      ref = assert ref or cnts.parentCollection, msgs.createLine.missingCollection
      text = cnts.ASS and cnts.ASS\getString! or cnts.text
      newLine = Line cnts, ref, args
      newLine.text = text
      @parse newLine

    elseif cnts.class == @LineContents
      -- ASSLineContents object will be attached to the new line
      -- line properties other than the text will be taken either from the defaults or the current previous line
      ref = assert ref or cnts.parentCollection, msgs.createLine.missingCollection
      newLine = useLineProps and Line(cnts.line, ref, args) or Line {}, ref, table.union defaults, args
      newLine.ASS, cnts.ASS.line = cnts.ASS, newLine
      newLine\commit!

    else
      -- A new ASSLineContents object is created from the supplied sections and attached to a new Line
      cnts = {cnts} if cnts.class
      newLine = Line {}, ref, table.union defaults, args
      assSectionTypes = table.values @Section unless assSectionTypes  -- cache Section types list for performance

      for cnt in *cnts
        -- TODO: move into ASSLineContents:new()
        unless @instanceOf cnt, assSectionTypes
          error msgs.createLine.badLine\format @LineContents.typeName, cnt.typeName or type cnt
        ref or= if lc = @getParentLineContents!
          lc.line.parentCollection

      error msgs.createLine.missingCollection unless ref
      newLine.ASS = @.LineContents newLine, cnts
      newLine.ASS\commit!

    newLine\createRaw!
    return newLine

  ASS.getParentLineContents = (obj) =>
    return nil unless type(obj) == "table" and obj.class
    while obj
      return obj if obj.class == @LineContents
      obj = obj.parent

  ASS.getScriptInfo = (obj = @cache.lastSub) =>
    if type(obj) == "table" and obj.class
      lineContents = @getParentLineContents obj
      return lineContents and lineContents.scriptInfo, lineContents

    assert obj and type(obj) == "userdata" and obj.insert, msgs.getScriptInfo.badSub
    @cache.lastSub = obj
    return util.getScriptInfo obj

  ASS.getTagFromString = (str) =>
    -- all tags with starting brackets must have at least one closing bracket
    -- for us to be able to freely move them around
    str = str\gsub "%(([^)]*)%)?", "(%1)"

    for _, tag in pairs @tagMap
      for name, sig in pairs tag.signatures
        if sig.pattern -- TODO: use continue here once Aegisub is on Moonscript v0.4.0+
          res = {str\find sig.pattern}
          if #res > 0
            start, end_ = table.remove(res,1), table.remove(res,1)  -- TODO: optimize the removes away
            tag.props.signature = name
            return tag.type({raw: res, tagProps: tag.props}), start, end_

    tagType = @tagMap[str\sub(1,1)=="\\" and "unknown" or "junk"]
    return @.Tag.Unknown({str, tagProps: tagType.props}), 1, #str

  ASS.getTagsNamesFromProps = (props) =>
    names, n = {}, 1
    for name, tag in pairs @tagMap
      continue unless tag.props
      propMatch = true

      for k, v in pairs props
        if tag.props[k] != v
          propMatch = false
          break

      if propMatch
        names[n], n = name, n+1

    return names

  ASS.instanceOf = (val, classes, filter, includeCompatible) =>
    return false if type(val) != "table" or not val.class -- not an ASSFoundation class

    if classes == nil
      -- return class name and compatible classes if requested
      return val.class, includeCompatible and table.keys val.compatible

    classes = {classes} if type(classes) != "table" or classes.instanceOf

    if type(filter) == "table"
      filter = if filter.instanceOf
        {[filter]: true}
      elseif #filter > 0
        list.makeSet filter

    for cls in *classes
      return cls if (val.class == cls or includeCompatible and val.compatible[cls]) and (not filter or filter[cls])

    return false

  ASS.parse = (line) =>
    line.ASS = ASS.Parser.LineText\getLineContents line
    return line.ASS

  -- DEPRECATED, do not use
  ASS.mapTag = (name) =>
    type_ = type name
    logger\assert type_ == "string", msgs.createTag.badType, type_

    tag = @tagMap[name]
    logger\assert tag, msgs.createTag.noSuchTag, name
    return tag

  return ASS
