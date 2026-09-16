return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  Weight = createASSClass "Tag.Weight", ASS.Tag.Base, {"weightClass","bold"}, {ASS.Number, ASS.Tag.Toggle}
  Weight.new = (args) =>
    weight, bold = unpack @getArgs args, {0, false}, true
    -- also support signature Weight{bold} without weight
    if args.raw or (#args == 1 and (type(args[1]) != "table" or args[1].class != Weight))
      weight, bold = weight != 1 and weight or 0, weight == 1

    @readProps args
    @bold = ASS.Tag.Toggle {bold}
    @weightClass = ASS.Number {weight, tagProps: {positive: true, precision: 0}}
    return @

  Weight.getTagParams = =>
    return if @weightClass.value > 0
      @weightClass\getTagParams!
    else @bold\getTagParams!

  Weight.setBold = (state) =>
    @bold\set type(state) == "nil" and true or state
    @weightClass.value = 0

  Weight.toggle = =>
    @bold\toggle!

  Weight.setWeight = (weightClass) =>
    @bold\set false
    @weightClass\set weightClass or 400

  return Weight
