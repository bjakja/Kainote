return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  CommentSection = createASSClass "Section.Comment", ASS.Section.Text, {"value"}, {"string"}

  CommentSection.new = (value) =>
    ASS.Section.Text.new @, value
    -- there's no way to escape a } in an ASS tag/comment section so just go ahead and nuke it
    @value = @value\gsub("}", "")\gsub "\\{", "{"
    return @

  CommentSection.getString = =>
    @typeCheck {@value}
    -- there's no way to escape a } in an ASS tag/comment section so just go ahead and nuke it
    return @value\gsub("}", "")\gsub "{", "\\{"
  return CommentSection
