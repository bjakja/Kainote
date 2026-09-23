return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  Align = createASSClass "Tag.Align", ASS.Tag.Indexed, {"value"}, {"number"}, {range: {1,9}, default: 5}

  Align.up = => @value < 7 and @add(3) or false
  Align.down = => @value > 3 and @sub(3) or false
  Align.left = => @value %3 != 1 and @sub(1) or false
  Align.right = => @value %3 != 0 and @add(1) or false

  Align.centerV = =>
    if @value <= 3 then @up!
    elseif @value >= 7 then @down!

  Align.centerH = =>
    if @value%3 == 1 then @right!
    elseif @value%3 == 0 then @left!

  Align.getSet = (pos) =>
    val = @value
    set = {
      top: val >=7
      centerV: val > 3 and val < 7
      bottom: val <= 3
      left: val%3 == 1
      centerH: val%3 == 2
      right: val%3 == 0
    }
    return pos == nil and set or set[pos]

  Align.isTop = => @getSet "top"
  Align.isCenterV = => @getSet "centerV"
  Align.isBottom = => @getSet "bottom"
  Align.isLeft = => @getSet "left"
  Align.isCenterH = => @getSet "centerH"
  Align.isRight = => @getSet "right"

  Align.getPositionOffset = (w, h, refAlign = Align{7}) =>
    if ASS\instanceOf w, ASS.Point, nil, true
      w, h = w\get!

    x, y = {w, 0, w/2}, {h, h/2, 0}
    off = ASS.Point{x[@value%3 + 1], y[math.ceil @value/3]}
    off\sub x[refAlign.value%3 + 1], y[math.ceil refAlign.value/3]

    return off

  Align.lerp = nil

  return Align
