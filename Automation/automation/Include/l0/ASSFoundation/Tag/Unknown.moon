return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  Unknown = createASSClass "Tag.Unknown", ASS.Tag.String, {"value"}, {"string"}
  Unknown.add, Unknown.sub, Unknown.mul, Unknown.div, Unknown.pow, Unknown.mod = nil

  Unknown.new = (args) =>
    -- all tags with starting brackets must have at least one closing bracket
    -- for us to be able to freely move them around
    @value = @getArgs(args, "", true)[1]\gsub "%(([^)]*)%)?", "(%1)"
    @readProps args
    return @

  return Unknown
