return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  round, Point, Move, Bezier = math.round

  msgs = {
    new: {
      missingFirstMove: "first drawing command of a contour must be of class %s, got a %s."
      badCommand: "argument #%d is not a valid drawing command object (%s)."
    }
    callback: {
      badCmdTypes: "argument #2 (cmdTypes) must be either a table of strings or a single string, got a %s."
    }
    expand: {
      badArgs: "x and y must be a number or nil, got x=%s (%s) and y=%s (%s)."
      noSimultaneousExpandInpand: "cannot expand and inpand at the same time (sign must be the same for x and y); got x=%d, y=%d."
    }
    insertCommand: {
      badIndex: "argument #2 (index) must be an integer != 0, got '%s' of type %s."
      badCmds: "argument #1 (cmds) must be either a drawing command object or a table of drawing commands, got a %s."
      badCmd: "command #%d must be a drawing command object, got a %s"
      noMove: "command #%d must be a drawing command object, but not a %s; got a %s"
    }
  }

  Contour = createASSClass "Draw.Contour", ASS.Base, {"commands"}, {"table"}, nil, nil, (tbl, key) ->
    if key == "isCW"
      return tbl\getDirection!
    else return getmetatable(tbl)[key]


  Contour.add, Contour.sub, Contour.mul = ASS.Tag.Base.add, ASS.Tag.Base.sub, ASS.Tag.Base.mul
  Contour.div, Contour.mod, Contour.pow = ASS.Tag.Base.div, ASS.Tag.Base.mod, ASS.Tag.Base.pow

  Contour.new = (args) =>
    unless Point
      Point, Move, Bezier = ASS.Point, ASS.Draw.Move, ASS.Draw.Bezier

    cmds, clsSet = {}, ASS.Draw.commands
    @commands = cmds
    return @ unless args

    for i, arg in ipairs args
      if i == 1 and arg.class != Move
        logger\error msgs.new.missingFirstMove, Move.typeName, args[1].typeName
      elseif type(arg) != "table" or not clsSet[arg.class or false]
        logger\error msgs.new.badCommand, i, type(arg) == "table" and arg.typeName or type arg
      cmds[i] = arg
      cmds[i].parent = @

    return @


  Contour.callback = (callback, cmdTypes, getPoints) =>
    cmdSet = if cmdTypes
      tCmdTypes = type cmdTypes
      if tCmdTypes == "string" or tCmdTypes == "table" and cmdTypes.class
        cmdTypes = {cmdTypes}

      elseif tCmdTypes != "table"
        logger\error msgs.callback.badCmdTypes, tCmdTypes

      list.makeSet cmdTypes

    j, cmdsDeleted = 1, false
    for i, cmd in ipairs @commands
      continue if cmdSet and not (cmdSet[cmd.__tag.name] or cmdSet[cmd.class])
      if getPoints and not cmd.compatible[Point]
        pointsDeleted = false
        for p, pKey in ipairs cmd.__meta__.order
          res = callback cmd[pKey], @commands, i, j, cmd, p
          j += 1
          if res == false
            -- deleting a single point causes the whole command to be deleted
            cmdsDeleted, pointsDeleted = true, true
          elseif res != nil and res != true
            pClass = cmd.__meta__.types[p]
            cmd[pKey] = res.class and res or pClass res

        if pointsDeleted
          @commands[i] = nil
      else
        res = callback cmd, @commands, i, j
        j += 1
        if res == false
          @commands[i], cmdsDeleted = nil, true
        elseif res != nil and res != true
          @commands[i] = res

    if cmdsDeleted
      @commands = table.continuous @commands

    if j > 1 -- there may have been changes to the commands so we need to clear some caches
      @length, @isCW = nil, nil
      @parent.length = nil if @parent


  Contour.expand = (x = 1, y = x) =>
    xType, yType = type(x), type y
    logger\assert xType  == "number" and yType == "number", msgs.expand.badArgs, tostring(x), xType, tostring(y), yType

    return @ if x == 0 and y == 0
    logger\assert x >= 0 and y >= 0 or x <= 0 and y <=0 , msgs.expand.noSimultaneousExpandInpand, x, y

    sameDir = if x < 0 or y < 0
      x, y = math.abs(x), math.abs(y)
      not @isCW
    else @isCW

    outline = @getOutline x, y

    -- may violate the "one move per contour" principle
    @commands, @length, @isCW = {}, nil, nil
    for i = sameDir and 2 or 1, #outline.contours, 2
      @insertCommands outline.contours[i].commands, -1, true

    return @


  Contour.insertCommands = (cmds, index, acceptMoves) =>
    prevCnt, inserted, clsSet = #@commands, {}, ASS.Draw.commands
    index or= math.max prevCnt, 1

    logger\assert math.isInt(index) and index != 0, msgs.insertCommands.badIndex, tostring(index), type index
    cmdsType = type cmds
    logger\assert cmdsType == "table", msgs.insertCommands.badCmds, cmdsType

    cmds = if cmds.class == Contour
      acceptMoves = true
      cmds.commands
    elseif cmds.class
      {cmds}


    for i, cmd in ipairs cmds
      cmdIsTbl = type(cmds[i]) == "table"
      logger\assert cmdIsTbl and cmd.class, msgs.insertCommands.badCmd, i, cmdIsTbl and cmd.typeName or type cmd
      logger\assert clsSet[cmd.class] and (not cmd.instanceOf[Move] or acceptMoves),
        msgs.insertCommands.noMove, Move.typeName, cmd.typeName

      insertIdx = index < 0 and prevCnt+index+i+1 or index+i-1
      table.insert @commands, insertIdx, cmd
      cmd.parent = @
      inserted[i] = @commands[insertIdx]

    if #cmds > 0
      @length, @isCW = nil, nil
      @parent.length = nil if @parent

    return #cmds > 1 and inserted or inserted[1]


  Contour.flatten = =>
    logger\assert Yutils, yutilsMissingMsg
    flatStr = Yutils.shape.flatten @getTagParams!
    flattened = ASS.Draw.DrawingBase{str: flatStr, tagProps: @__tag}
    @commands = flattened.contours[1].commands
    return @, flatStr -- TODO: fix this


  Contour.get = =>
    commands, j = {}, 1
    for i = 1, #@commands
      commands[j] = @commands[i].__tag.name
      commands[j], j = cmd, j + 1 for cmd in *@commands[i]\get true

    return commands


  Contour.getCommandAtLength = (len, noUpdate) =>
    @getLength! unless noUpdate and @length
    currTotalLen, nextTotalLen = 0
    for i, cmd in ipairs @commands
      nextTotalLen = currTotalLen + cmd.length
      if nextTotalLen-len > -0.001 and cmd.length > 0 and not (cmd.class == Move or cmd.class == ASS.Draw.MoveNc)
        return cmd, math.max len-currTotalLen, 0
      else currTotalLen = nextTotalLen

    return false


  Contour.getDirection = =>
    angle = ASS\createTag "angle", 0
    logger\assert @commands[1].class == Move, msgs.new.missingFirstMove, Move.typeName, @commands[1].typeName

    p0, p1 = @commands[1]
    cb = (point, _, _, j, _, _) ->
      if j == 2
        p1 = point
      elseif j > 2
        vec0, vec1 = p1\copy!\sub(p0), point\copy!\sub p1
        angle\add vec1\getAngle vec0, true
        p0, p1 = p1, point

    @callback cb, nil, true
    @isCW = angle >= 0
    return @isCW


  Contour.getExtremePoints = (allowCompatible) =>
    local top, left, bottom, right
    for i = 1, #@commands
      pts = @commands[i]\getPoints allowCompatible
      for pt in *pts
        top = pt if not top or top.y > pt.y
        left = pt if not left or left.x > pt.x
        bottom = pt if not bottom or bottom.y < pt.y
        right = pt if not right or right.x < pt.x

    return {:top, :left, :bottom, :right, w: right.x-left.x, h: bottom.y-top.y,
        bounds: {left.x, top.y, right.x, bottom.y}}


  Contour.getBounds = =>
    logger\assert Yutils, yutilsMissingMsg
    l, t, r, b = Yutils.shape.bounding Yutils.shape.flatten @getTagParams!
    return {Point(l, t), Point(r, b), w: r-l, h: b-t}


  Contour.getLength = =>
    totalLen, lens = 0, {}
    for i = 1, #@commands
      len = @commands[i]\getLength @commands[i-1]
      lens[i], totalLen = len, totalLen + len

    @length = totalLen
    return totalLen, lens


  Contour.getPositionAtLength = (len, noUpdate, useCurveTime) =>
    @getLength! unless noUpdate and @length
    cmd, remLen  = @getCommandAtLength len, true
    return false unless cmd
    return cmd\getPositionAtLength(remLen, true, useCurveTime), cmd


  Contour.getAngleAtLength = (len, noUpdate) =>
    @getLength! unless noUpdate and @length
    cmd, remLen = @getCommandAtLength len, true
    return false unless cmd

    fCmd = cmd.class == Bezier and cmd.flattened\getCommandAtLength(remLen, true) or cmd
    return fCmd\getAngle(nil, false, true), cmd


  Contour.getTagParams = (scale = @parent and @parent.scale) =>
    scale = scale and scale\get! or 1

    cmdStr, j, lastCmdType = {}, 1
    for i, cmd in ipairs @commands
      if lastCmdType ~= cmd.__tag.name
        lastCmdType = cmd.__tag.name
        cmdStr[j], j = lastCmdType, j+1

      -- optimized inlined versions of the respective \getTagParams methods
      if cmd.class == Bezier
        precision = cmd.__tag.precision
        cmdStr[j] = round(scale>1 and cmd.p1.x*(2^(scale-1)) or cmd.p1.x, precision)
        cmdStr[j+1] = round(scale>1 and cmd.p1.y*(2^(scale-1)) or cmd.p1.y, precision)
        cmdStr[j+2] = round(scale>1 and cmd.p2.x*(2^(scale-1)) or cmd.p2.x, precision)
        cmdStr[j+3] = round(scale>1 and cmd.p2.y*(2^(scale-1)) or cmd.p2.y, precision)
        cmdStr[j+4] = round(scale>1 and cmd.p3.x*(2^(scale-1)) or cmd.p3.x, precision)
        cmdStr[j+5] = round(scale>1 and cmd.p3.y*(2^(scale-1)) or cmd.p3.y, precision)
        j += 6

      elseif cmd.compatible[Point]
        precision = cmd.__tag.precision
        cmdStr[j] = round(scale>1 and cmd.x*(2^(scale-1)) or cmd.x, precision)
        cmdStr[j+1] = round(scale>1 and cmd.y*(2^(scale-1)) or cmd.y, precision)
        j += 2

      -- generic case
      else
        params = {cmd\getTagParams!}
        for param in *params
          cmdStr[j] = scale>1 and param*(2^(scale-1)) or param
          j += 1

    return table.concat cmdStr, " "


  Contour.commonOp = (method, callback, default, x, y) => -- drawing commands only have x and y in common
    if type(x) == "table" and x.class and x.compatible[Point]
      x, y = x.x, x.y

    for cmd in *@commands
      cmd[method] cmd, x, y

    return @


  Contour.getOutline = (x, y = x, mode = "round", splitContours) =>
    logger\assert Yutils, yutilsMissingMsg
    outline = Yutils.shape.to_outline Yutils.shape.flatten(@getTagParams!), x, y, mode
    return (@parent and @parent.class or ASS.Draw.DrawingBase) {str: outline, :splitContours}


  Contour.outline = (x, y, mode) =>
    -- may violate the "one move per contour" principle
    @commands = @getOutline(x, y, mode, false).contours[1].commands
    @length, @isCW = nil, nil


  Contour.rotate = (angle) =>
    ASS.Draw.DrawingBase.rotate(@, angle)
    @commands = @contours[1]  -- rotating a contour should produce no additional contours
    @contours = nil
    return @


  Contour.getFullyCovered = (contour, scriptInfo, parentCollection) =>
    if not scriptInfo and parentCollection
      scriptInfo, parentContents = ASS\getScriptInfo @
      parentCollection = parentContents and parentContents.line.parentCollection or LineCollection ASSFInst.cache.lastSub

    bs, bo = @getExtremePoints!.bounds, contour\getExtremePoints!.bounds
    bounds = {math.min(bs[1], bo[1]), math.min(bs[2], bo[2]), math.max(bs[3], bo[3]), math.max(bs[4], bo[4])}
    w, h, safe = bounds[3]-bounds[1], bounds[4]-bounds[2], 1.05
    sx, sy = w*safe/scriptInfo.PlayResX, h*safe/scriptInfo.PlayResY
    -- move contours as close as possible to point of origin
    -- and proportionally scale it to fully fit the render surface
    a = @copy!\sub bounds[1], bounds[2]
    b = contour\copy!\sub bounds[1], bounds[2]

    if sx > 1 or sy > 1
      fac = math.max sx, sy
      a\div fac, fac
      b\div fac, fac

    -- create line with both contours and get reference line bounds
    section = ASS.Section.Drawing{a, b}
    testLineCnts = ASS\createLine({{ASS.defaults.drawingTestTags, section}, parentCollection}).ASS
    -- create lines with only one of the two contours
    lbAB = testLineCnts\getLineBounds!
    testLineCnts.sections[2].contours[2] = nil
    lbA = testLineCnts\getLineBounds!
    testLineCnts.sections[2].contours[1] = b
    lbB = testLineCnts\getLineBounds!
    -- compare the render results of both single contours to the reference
    -- if one is identical to the reference it is covering up the other one (or the other one is 0-width)
    return lbAB\equal(lbA) and contour or lbAB\equal(lbB) and @ or false


  Contour.reverseDirection = =>
    revCmds, n = {@commands[1]}, #@.commands
    for i = n, 2, -1
      revCmds[n-i+2] = @commands[i]

    @isCW, @commands = nil, revCmds
    return @


  return Contour
