return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  Toggle = createASSClass "Tag.Toggle", ASS.Tag.Base, {"value"}, {"boolean"}

  msgs = {
    toggle: {
      badState: "argument #1 (state) must be true, false or nil, got a %s."
    }
  }

  Toggle.new = (args) =>
    @value = @getArgs(args, false, true)[1]
    @readProps args
    @typeCheck {@value}
    return @

  Toggle.toggle = (state) =>
    logger\assert type(state) == "boolean" or state == nil, msgs.toggle.badState, type state
    @value = state == nil and not @value or state
    return @value

  Toggle.getTagParams = =>
    @typeCheck {@value}
    return @value and 1 or 0

  return Toggle
