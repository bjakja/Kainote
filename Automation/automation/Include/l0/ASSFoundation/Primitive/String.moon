return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  String = createASSClass "String", ASS.Tag.Base, {"value"}, {"string"}

  String.new = (args) =>
    @value = @getArgs(args, "", true)[1]
    @readProps args
    return @

  String.append = (str, sep) =>
    if type(str) == "table"
      return @ if #str == 0
      str = table.concat str, sep

    sep = #@value > 0 and sep or ""
    return @commonOp "append", ((val, str) -> val .. sep ..str), "", str

  String.prepend = (str) =>
    return @commonOp "prepend", ((val,str) -> str .. val), "", str

  String.replace = (pattern, rep, plainMatch, useRegex) =>
    if plainMatch
      useRegex, pattern = false, string.escLuaExp pattern

    @value = useRegex and re.sub(@value, pattern, rep) or @value\gsub pattern, rep
    return @

  String.reverse = =>
    @value = unicode.reverse @value
    return @

  return String
