return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  class Sections
    tagMatchPattern = re.compile "\\\\[^\\\\\\(]+(?:\\([^\\)]+\\)?[^\\\\]*)?"

    @getTagOrCommentSection = (rawTags) =>
      tags = @parseTags rawTags
      return ASS.Section.Comment rawTags if #tags == 0 and #rawTags > 0

      tagSection = ASS.Section.Tag tags
      return tagSection

    @parseTags = (rawTags) =>
      tags, t = {}, 1
      return tags if #rawTags == 0

      for match in tagMatchPattern\gfind rawTags
        tag, _, last = ASS\getTagFromString match
        tags[t] = tag
        t += 1

        -- comments inside tag sections are read into ASS.Tag.Unknowns
        if last < #match
          afterStr = match\sub last + 1
          tags[t] = ASS\createTag afterStr\sub(1,1)=="\\" and "unknown" or "junk", afterStr
          t += 1

      return tags
