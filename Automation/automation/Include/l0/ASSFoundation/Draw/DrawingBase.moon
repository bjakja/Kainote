return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  msgs = {
    new: {
      badInput: "argument #%d is not a valid drawing object, contour or table, got a %s."
      invalidObject: "argument #%d is not a valid drawing object or contour, got a %s."
      badCommandsInTable: "argument #%d to %s contains invalid drawing objects (#%d is a %s)."
    }

    modCommands: {
      badCommandType: "argument #2 must be either a table of command classes or names, a single command class or a single command name, got a %s."
    }

    insertContours: {
      badContours: "argument #1 (contours) must be either a single contour object or a table of contours, got a %s."
      badContour: "can only insert objects of class %s, got a %s."
    }

    insertCommand: {
      badIndex: "argument #2 (index) must be an integer != 0, got '%s' of type %s."
      badCommands: "argument #1 (cmds) must be either a drawing command object or a table of drawing commands, got a %s."
      noContourAtIndex: "can't insert command: no contour at index %d"
      badCommand: "command #%d must be a drawing command object, got a %s"
    }

    getCommandAtLength: {
      missingLength: "Unexpected Error: command at length not found in target contour."
    }

    removeContours: {
      badContours: "argument #1 must be either an %s object, an index, nil or a table of contours/indexes; got a %s."
    }

    rotate: {
      badAngle: "argument #1 (angle) must be either a number or a %s object, got a %s."
    }
  }

  DrawingBase = createASSClass "Draw.DrawingBase", ASS.Tag.Base, {"contours"}, {"table"}
  -- TODO: check if these can be remapped/implemented in a way that makes sense, maybe work on strings
  DrawingBase.set, DrawingBase.mod = nil, nil

  local cmdMap, cmdSet, Close, Draw, Move, Contour, Bezier

  DrawingBase.new = (args = {}) =>
    unless cmdMap -- import often accessed information into local scope
      cmdMap, cmdSet = ASS.Draw.commandMapping, ASS.Draw.commands
      Close, Move, Contour, Bezier = ASS.Draw.Close, ASS.Draw.Move, ASS.Draw.Contour, ASS.Draw.Bezier

    -- construct from a raw string of drawing commands
    if args.raw or args.str
      scale = args.scale or 1

      str = if args.raw and args.tagProps.signature == "scale"
        scale = args.raw[1]
        args.raw[2]
      elseif args.raw
        args.raw[1]
      else args.str

      @contours, @junk = ASS.Parser.Drawing\getContours str, @, args.splitContours
      @scale = ASS\createTag "drawing", scale

      if @scale > 1
        @div 2^(@scale-1), 2^(@scale-1)

    -- construct by deep-copying a compatible object
    elseif ASS\instanceOf args[1], DrawingBase, nil, true
      copy = args[1]\copy!
      @contours, @scale = copy.contours, copy.scale
      @__tag.inverse = copy.__tag.inverse

    -- construct from valid drawing commands, contours and tables of drawing commands
    else
      @contours, c = {}, 1
      @scale = ASS\createTag "drawing", args.scale or 1
      contour, d = {}, 1

      for i, arg in ipairs args
        argType = type arg
        logger\assert argType == "table", msgs.new.badInput, i, argType

        -- a new contour causes the already open contour to be closed
        if arg.class == Contour
          if #contour > 0
            @contours[c] = Contour contour
            @contours[c].parent, c = @, c+1

          @contours[c], c = arg, c+1
          contour, d = {}, 1

        -- a move creates a new contour, unless otherwise requested
        elseif arg.class == Move and i > 1 and args.splitContours != false
          @contours[c], c = Contour(contour), c+1
          contour, d = {arg}, 2

        -- any other valid command is added to the currently open contour
        elseif ASS\instanceOf arg, cmdSet
          contour[d] = arg
          d += 1

        else
          logger\assert not arg.class, msgs.new.invalidObject, i, arg.typeName

          -- a table of drawing commands is also supported
          for j, cmd in ipairs arg
            logger\assert ASS\instanceOf(cmd, cmdSet), msgs.new.badCommandsInTable,
                          i, @typeName, j, cmd.class or type cmd

            if cmd.instanceOf[Move]
              @contours[c] = Contour contour
              @contours[c].parent = @
              contour, d = {cmd}, 2
              c += 1
            else
              contour[d] = cmd
              d += 1

      -- commit last open contour if it contains any drawing commands
      if #contour > 0
        @contours[c] = Contour contour
        @contours[c].parent = @

    @readProps args
    return @

  DrawingBase.callback = (callback, first = 1, last = #@contours, includeCW = true, includeCCW = true, a1, a2, a3, a4, a5, a6) =>
    j, rmCnt, removed = 1, 0

    for i = first, last
      cnt = @contours[i]
      if (includeCW or not cnt.isCW) and (includeCCW or cnt.isCW)
        res = callback cnt, @contours, i, j, @toRemove, a1, a2, a3, a4, a5, a6
        j += 1

        if res == false
          @toRemove or= {}
          @toRemove[cnt], @toRemove[rmCnt+1] = true, i
          rmCnt += 1
        elseif res != nil and res != true
          @contours[i], @length = res


    -- delay removal of contours until the all contours have been processed
    if rmCnt > 0
      removed = list.removeIndices @contours, @toRemove
      @length = nil

    @toRemove = nil
    return removed, rmCnt

  mapCommandTypes = (cmdType, cmdSet = {}) ->
    switch type cmdType
      when "string"
        if cmdClass = cmdMap[cmdType]
          cmdSet[cmdClass] = true
      when "table"
        if cmdType.class
          cmdSet[cmdType.class] = true
        else
          for cmd in *cmdType
            res, err = mapCommandTypes cmd, cmdSet
            return nil, err unless res
      else return nil, cmdType

    return true

  DrawingBase.modCommands = (callback, commandTypes, start, end_, includeCW = true, includeCCW = true, args) =>
    cmdSet, err = mapCommandTypes commandTypes if commandTypes
    logger\error msgs.modCommands.badCommandType, err if err

    matchedCmdCnt, matchedCntsCnt, rmCntsCnt, rmCmdsCnt = 1, 1, 0, 0

    for i, cnt in @contours
      if (includeCW or not cnt.isCW) and (includeCCW or cnt.isCW)
        rmCmdsCnt = 0
        for j, cmd in cnt.commands
          if not cmdSet or cmdSet[cmd.class]
            res = callback cmd, cnt.commands, j, matchedCmdCnt, i, matchedCntsCnt, cnt.toRemove, @toRemove, args
            matchedCmdCnt += 1
            if res == false
              cnt.toRemove or= {}
              cnt.toRemove[cmd], cnt.toRemove[rmCmdsCnt+1] = true, j
              rmCmdsCnt += 1

            elseif res != nil and res != true
                cnt.commands[j] = res
                cnt.length, cnt.isCW, @length = nil

        matchedCntsCnt += 1
        if rmCmdsCnt > 0
            list.removeIndices cnt.commands, cnt.toRemove
            cnt.length, cnt.isCW, @length, cnt.toRemove = nil
            if #cnt.commands == 0
              @toRemove or= {}
              @toRemove[cnt], @toRemove[rmCntsCnt+1] = true, i
              rmCntsCnt += 1

    -- delay removal of contours until the all contours have been processed
    if rmCntsCnt > 0
        list.removeIndices @contours, @toRemove
        @length, @toRemove = nil

  DrawingBase.insertCommands = (cmds, index) =>
    prevCnt = #@contours
    index or= math.max prevCnt, 1

    unless math.isInt(index) and index != 0
      logger\error msgs.insertCommands.badIndex, tostring(index), type index
    if type(cmds) != "table"
      logger\error msgs.insertCommands.badCommands, type cmds

    index += prevCnt + 1 if index < 0
    currentContour = @contours[index]
    unless currentContour
      logger\assert index == 1 and prevCnt == 0, msgs.insertCommands.noContourAtIndex, index
      @contours[1] = Contour! -- create a contour if none exists and index is 1
      currentContour = @contours[1]

    c = #currentContour.commands + 1

    -- insert a single drawing command
    if cmds.class
      unless cmdSet[cmds.class]
        logger\error msgs.insertCommands.badCommand, cmds.typeName
      if cmds.class == Move
        @insertContours Contour({cmds}), index
      else
        currentContour.commands[c] = cmds
        currentContour.length, currentContour.isCW = nil

      @length = nil
      return

    -- insert any number of drawing commands
    for i, cmd in ipairs cmds
      if type(cmd) != "table" or not cmdSet[cmd.class]
        logger\error msgs.insertCommands.badCommand, type(cmd) == "table" and cmd.typeName or type cmd

      if cmd.class == Move
        unless currentContour.class
          @insertContours Contour(currentContour), index
        currentContour, c = {cmd}, 1
        index += 1

      elseif currentContour.class
        currentContour.commands[c] = cmd
        currentContour.length, currentContour.isCW = nil

      else currentContour[c] = cmd
      c += 1

    @insertContours Contour(currentContour), index unless currentContour.class
    @length = nil

  DrawingBase.insertContours = (cnts, index = #@contours + 1) =>
    if type(cnts) != "table"
      logger\error msgs.insertContours.badContours, type cnts

    if cnts.class
      if cnts.compatible[DrawingBase]
        -- copy all contours from another drawing
        cnts = cnts\copy!.contours
      elseif cnts.class == Contour
        -- insert a single contour
        table.insert @contours, index, cnts
        @length = nil
        return cnts
      else logger\error msgs.insertContours.badContour, Contour.typeName, cnts.typeName

    -- insert a table of contours
    for i, cnt in ipairs cnts
      if type(cnt) != "table" or cnt.class != Contour
        logger\error msgs.insertContours.badContour, Contour.typeName,
          type(cnt) == "table" and cnt.typeName or type cnt

      table.insert @contours, index + i-1, cnt
      cnt.parent = @

    @length = nil if #cnts > 0
    return cnts

  DrawingBase.toString = =>
    cmds, c = {}, 1
    for i, cnt in ipairs @contours
      unless cnt.disabled or @toRemove and @toRemove[cnt]
        -- make disabled contours and contours subject to delayed deletion disappear
        cmds[c] = cnt\getTagParams @scale
        c += 1
    return table.concat cmds, " "

  DrawingBase.__tostring = DrawingBase.__tostring

  DrawingBase.getTagParams = =>
    if @scale\equal 1
      return DrawingBase.toString @
    return @scale\getTagParams!, DrawingBase.toString @

  DrawingBase.commonOp = (method, callback, default, x, y) => -- drawing commands only have x and y in common
    for cnt in *@contours
      cnt\commonOp method, callback, default, x, y
    return @

  DrawingBase.drawRect = (tl, br) => -- TODO: contour direction
    rect = ASS.Draw.Contour{ASS.Draw.Move(tl), ASS.Draw.Line(br.x, tl.y),
                            ASS.Draw.Line(br), ASS.Draw.Line(tl.x, br.y)}
    @insertContours rect
    return @, rect

  DrawingBase.expand = (x, y) =>
    holes, others, covered = @getHoles!
    @removeContours covered
    hole\expand -x, -y for hole in *holes
    other\expand x, y for other in *others
    return @

  DrawingBase.flatten = =>
    flatStr = {}
    for i, cnt in *@contours
      _, flatStr[i] = cnt\flatten!

    return @, table.concat flatStr, " "

  DrawingBase.getLength = (includeContourLengths) =>
    totalLen = 0
    lens = includeContourLengths and {} or nil
    for cnt in *@contours
      len, lenParts = cnt\getLength!
      table.joinInto(lens, lenParts) if includeContourLengths
      totalLen += len

    @length = totalLen
    return totalLen, lens

  DrawingBase.getCommandAtLength = (len, useCachedLength) =>
    @getLength! unless useCachedLength and @length

    currTotalLen, contourCnt = 0, #@contours

    for i, contour in ipairs @contours
      if currTotalLen+contour.length-len > -0.001 and contour.length > 0
        cmd, remLen = contour\getCommandAtLength len, useCachedLength
        if i != contourCnt and not cmd
          logger\error msgs.getCommandAtLength.missingLength
        return cmd, remLen, contour, i
      else currTotalLen += contour.length - len

    return false
    -- error(string.format("Error: length requested (%02f) is exceeding the total length of the shape (%02f)",len,currTotalLen))

  DrawingBase.getPositionAtLength = (len, useCachedLength, useCurveTime) =>
    @getLength! unless useCachedLength and @length
    cmd, remLen, cnt  = @getCommandAtLength len, true
    return false unless cmd
    return cmd\getPositionAtLength(remLen, true, useCurveTime), cmd, cnt

  DrawingBase.getAngleAtLength = (len, useCachedLength) =>
    @getLength! unless useCachedLength and @length
    cmd, remLen, cnt = @getCommandAtLength len, true
    return false unless cmd

    fCmd = cmd.class == Bezier and cmd.flattened\getCommandAtLength(remLen, true) or cmd
    return fCmd\getAngle(nil, false, true), cmd, cnt

  DrawingBase.getExtremePoints = (allowCompatible) =>
    return {w: 0, h: 0} if #@contours == 0
    ext = @contours[1]\getExtremePoints allowCompatible

    for i=2, #@contours
      pts = @contours[i]\getExtremePoints allowCompatible
      ext.top = pts.top if ext.top.y > pts.top.y
      ext.left = pts.left if ext.left.x > pts.left.x
      ext.bottom = pts.bottom if ext.bottom.y < pts.bottom.y
      ext.right=pts.right if ext.right.x < pts.right.x

    ext.w, ext.h = ext.right.x - ext.left.x, ext.bottom.y - ext.top.y
    return ext

  DrawingBase.getBounds = =>
    logger\assert Yutils, yutilsMissingMsg
    l, t, r, b = Yutils.shape.bounding Yutils.shape.flatten DrawingBase.toString @
    return {ASS.Point(l, t), ASS.Point(r, b), w: r-l, h: b-t}

  DrawingBase.outline = (x, y, mode) =>
    @contours = @getOutline(x, y, mode).contours
    @length = nil

  DrawingBase.getOutline = (x, y = x, mode = "round") =>
    logger\assert Yutils, yutilsMissingMsg
    outline = Yutils.shape.to_outline Yutils.shape.flatten(DrawingBase.toString @), x, y, mode
    return @class {str: outline}

  DrawingBase.removeContours = (cnts, first = 1, last, includeCW = true, includeCCW = true) =>
    -- calling this without any arguments removes all contours
    if not cnts and first == 1 and not last and includeCW and includeCCW
      removed, @contours, @length = @contours, {}
      cnt.parent = nil for cnt in *removed
      return removed

    cntsType = type cnts
    -- remove a single contour by index or reference
    if not cnts or cntsType == "number" or cntsType == "table" and cnts.class == Contour
      return @callback ((cnt, _, i) -> cnts and cnts != i and cnts != cnt), first, last, includeCW, includeCCW

    if cntsType == "table" and cnts.class or cntsType != "table"
      logger\error msgs.removeContours.badContours, Contour.typeName, cntsType == "table" and cnts.typeName or cntsType

    -- remove a list of contours or indices
    cntsSet = list.makeSet cnts
    @callback((cnt,_,i) -> not cntsSet[cnt] and not cntsSet[i]), first, last, includeCW, includeCCW

  DrawingBase.getFullyCoveredContours = =>
    scriptInfo, parentContents = ASS\getScriptInfo @
    parentCollection = parentContents and parentContents.line.parentCollection or LineCollection ASSFInst.cache.lastSub
    covCnts, c = {}, 0

    @callback (cnt, cnts, i) ->
      return if covCnts[cnt]
      for j = i+1, #cnts
        if not (covCnts[cnt] or covCnts[cnts[j]]) and cnts[j].isCW == cnt.isCW
          if covered = cnt\getFullyCovered cnts[j], scriptInfo, parentCollection
            covCnts[covered], c = true, c+1
            covCnts[c] = covered == cnt and i or j

    return covCnts

  DrawingBase.getHoles = =>
    scriptInfo, parentContents = ASS\getScriptInfo @
    parentCollection = parentContents and parentContents.line.parentCollection or LineCollection ASSFInst.cache.lastSub

    bounds, safe = @getExtremePoints!, 1.05
    scaleFac = math.max bounds.w*safe / scriptInfo.PlayResX, bounds.h*safe / scriptInfo.PlayResY

    testDrawing = ASS.Section.Drawing{@}
    testDrawing\modCommands (cmd) ->
        cmd\sub bounds.left.x, bounds.top.y
        cmd\div scaleFac, scaleFac if scaleFac > 1
        -- snap drawing commands to the pixel grid to avoid false positives
        -- when using the absence of opaque pixels in the clipped drawing to determine
        -- whether the contour is a hole
        cmd\ceil 0, 0

    testLineCnts = ASS\createLine({{ASS.Section.Tag(ASS.defaults.drawingTestTags), testDrawing}, parentCollection}).ASS
    testTagCnt = #testLineCnts.sections[1].tags

    covered, holes, other, h, o = @getFullyCoveredContours!, {}, {}, 1, 1
    coveredSet = list.makeSet covered
    testDrawing\callback (cnt, _, i) ->
      unless coveredSet[@contours[i]]
        testLineCnts.sections[1].tags[testTagCnt+1] = ASS\createTag "clip_vect", cnt
        if not testLineCnts\getLineBounds!.firstFrameIsSolid
          -- clipping the drawing to the contour produced no solid pixels (only subpixel residue)
          -- most likely means the contour is a hole
          holes[h] = @contours[i]
          h += 1
        else
          other[o] = @contours[i]
          o += 1

    return holes, other, covered


  DrawingBase.rotate = (angle = 0) =>
    if ASS\instanceOf angle, ASS.Number
      angle = angle\getTagParams!
    else logger\assert type(angle) == "number", msgs.rotate.badAngle,ASS.Number.typeName,
      type(angle) == "table" and angle.typeName or type angle

    if angle % 360 != 0
      logger\assert Yutils, yutilsMissingMsg
      shape = DrawingBase.toString @
      bnd = {Yutils.shape.bounding shape }

      rotMatrix = with Yutils.math.create_matrix!
        .translate((bnd[3]-bnd[1])/2,(bnd[4]-bnd[2])/2,0)
        .rotate("z",angle)
        .translate(-bnd[3]+bnd[1]/2,(-bnd[4]+bnd[2])/2,0)

      shape = Yutils.shape.transform shape, rotMatrix
      @contours = DrawingBase({raw: shape}).contours

    return @

  DrawingBase.get = =>
    if #@contours == 1
      return @contours[1]\get!, @scale\get!

    commands, c = {}, 1
    for cnt in *@contours
      commands[c], c = cmd, c + 1 for cmd in *cnt\get!

    return commands, @scale\get!

  DrawingBase.getSection = =>
    section = ASS.Section.Drawing!
    section.contours, section.scale = @contours, @scale
    return section

  return DrawingBase
