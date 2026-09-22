return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional
  Transform = createASSClass "Transform", ASS.Tag.Base, {"tags", "startTime", "endTime", "accel"}, {ASS.Section.Tag, ASS.Time, ASS.Time, ASS.Number}

  msgs = {
    changeTagType: {
      invalidTransformSignature: "invalid transform signature '%s'."
    }
    getTagParams: {
      transformStartTimeGreaterEndTime: "transform start time must not be greater than the end time, got %d <= %d."
    }
  }

  Transform.new = (args) =>
    @readProps args
    signature = @__tag.signature
    if args.raw
      r = {}
      switch signature
        when "accel" then r[1], r[4] = args.raw[2], args.raw[1] -- \t(<accel>,<style modifiers>)
        when "default" then r[1], r[2], r[3], r[4] = args.raw[4], args.raw[1], args.raw[2], args.raw[3] -- \t(<t1>,<t2>,<accel>,<style modifiers>)
        when "time" then r[1], r[2], r[3] = args.raw[3], args.raw[1], args.raw[2] -- \t(<t1>,<t2>,<style modifiers>)
        else r = args.raw
      args.raw = r

    tags, startTime, endTime, accel = unpack @getArgs args, {"", 0, 0, 1}, true

    @tags = ASS.Section.Tag tags, args.transformableOnly
    @accel = ASS.Number {accel, tagProps: {positive: true}}
    @startTime = ASS.Time {startTime}
    @endTime = ASS.Time {endTime}
    return @

  Transform.changeTagType = (signature) =>
    if signature
      logger\assert ASS.tagMap.transform.signatures[signature], msgs.changeTagType.invalidTransformSignature, tostring signature
      @__tag.signature, @__tag.typeLocked = signature, true
    else
      noTime = @startTime\equal(0) and @endTime\equal 0
      @__tag.signature = @accel\equal(1) and (noTime and "simple" or "time") or noTime and "accel" or "default"
      @__tag.typeLocked = false

    return @__tag.signature, @__tag.typeLocked

  Transform.getSignature = =>
    unless @__tag.typeLocked
      noTime = @startTime\equal(0) and @endTime\equal 0
      @__tag.signature = @accel\equal(1) and (noTime and "simple" or "time") or noTime and "accel" or "default"

    return @__tag.signature or "default"

  Transform.getTagParams = =>
    signature = @getSignature!
    t1, t2 = @startTime\getTagParams!, @endTime\getTagParams!

    logger\assert t1 <= t2 or t2 == 0, msgs.getTagParams.transformStartTimeGreaterEndTime, t1, t2
    return switch signature
      when "simple" then @tags\getString!
      when "accel" then @accel\getTagParams!, @tags\getString! -- \t(<accel>,<style modifiers>)
      when "time" then t1, t2, @tags\getString! -- \t(<t1>,<t2>,<style modifiers>)
      when "default" then t1, t2, @accel\getTagParams!, @tags\getString! -- \t(<t1>,<t2>,<accel>,<style modifiers>)
      else logger\error msgs.changeTagType.invalidTransformSignature, tostring signature

  return Transform
