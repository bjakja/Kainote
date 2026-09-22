return (ASS, ASSFInst, yutilsMissingMsg, createASSClass, Functional, LineCollection, Line, logger, SubInspector, Yutils) ->
  {:list, :math, :string, :table, :unicode, :util, :re } = Functional

  class Drawing
    msgs = {
      parse: {
        invalidQuirksMode: "Invalid quirks mode '%s'"
        badOrdinate: "expected ordinate, got '%s'"
        wrongOrdinateCount: "incorrect number of ordinates for command '%s': expected %d, got %d"
        cantParse: "Error: failed to parse drawing near '%s' (%s)."
        unsupportedCommand: "Error: Unsupported drawing Command '%s'."
      }
    }

    local cmdMap, cmdSet, Close, Draw, Move, Contour, Bezier

    @getContours = (str, parent, splitContours = true) =>
      unless cmdMap -- import often accessed information into local scope
        cmdMap, cmdSet = ASS.Draw.commandMapping, ASS.Draw.commands
        Close, Move, Contour, Bezier = ASS.Draw.Close, ASS.Draw.Move, ASS.Draw.Contour, ASS.Draw.Bezier

      -- split drawing command string
      cmdParts = string.split string.trim(str), "%s+", nil, false -- TODO: check possible optimization by using simple split and no trimming here and skipping empty stuff later on instead
      cmdPartCnt = #cmdParts

      i, junk, prevCmdType = 1
      contours, c = {}, 1
      contour, d = {}, 1

      while i <= cmdPartCnt
        cmd, cmdType = cmdParts[i], cmdMap[cmdParts[i]]

        -- move command creates a new contour
        if cmdType == Move and i > 1 and splitContours
          contours[c] = Contour contour
          contours[c].parent = parent
          contour, d, c = {}, 1, c+1

        -- close command closes a contour
        if cmdType == Close
          contour[d] = Close {}

        -- There are two ways for a new drawing command to start
        -- (a) a new command is explicitely denoted by its identifier m, l, b, etc
        -- (b) a new command of the same type as the previous command implicitely starts
        --     once coordinates in addition to those consumed by the previous command are encountered
        elseif cmdType or prevCmdType and cmd\find "^[%-%d%.]+$"
          if cmdType
            prevCmdType = cmdType
          else i -= 1

          prmCnt = prevCmdType.__defProps.ords
          p, prms, skippedOrdCnt = 1, {}, 0

          while p <= prmCnt
            prm = cmdParts[p+i]
            if prms[p-skippedOrdCnt] = tonumber prm
              p += 1
              continue

            -- process ordinates that failed to cast to a number

            -- either the command or the the drawing is truncated
            if not prm or prm\match "^[bclmnps]$"
              unless ASS.config.fixDrawings
                reason = msgs.parse.wrongOrdinateCount\format prevCmdType.typeName, prmCnt, p-1
                near = table.concat cmdParts, " ", math.max(i+p-10,1), i+p
                logger\error msgs.parse.cantParse, near, reason

              -- we can fix this if required, but need to
              -- give back any subsequent drawing commands
              prmCnt = prm and p - 1 or p
              break

            -- the ordinate is either malformed or has trailing junk
            elseif not ASS.config.fixDrawings
              near = table.concat cmdParts, " ", math.max(i+p-10,1), i+p
              logger\error msgs.parse.cantParse, near, msgs.parse.badOrdinate\format prm

            ord, fragment = prm\match "^([%-%d%.]*)(.*)$"

            switch ASS.config.quirks
              when ASS.Quirks.VSFilter
                -- xy-vsfilter accepts and draws an ordinate with junk at the end,
                -- but will skip any following drawing commands in the drawing section.
                if ord
                  prms[p-skippedOrdCnt] = tonumber ord
                  fragment = prm unless prms[p-skippedOrdCnt]

                -- move the junk into a comment section and stop parsing further drawing commands
                junk or= ASS.Section.Comment fragment
                junk\append list.slice(cmdParts, i+p+1), " "
                i = cmdPartCnt + 1 -- fast forward to end of the outer loop
                break

              when ASS.Quirks.libass
                -- libass only draws y ordinates with trailing junk but discards x ordinates.
                -- However, it will recover once it hits valid commands again.
                junk or= ASS.Section.Comment!

                if not ord or p % 2 == 1
                  -- move broken x parameter into a comment section
                  junk\append prm, " "
                  -- make sure to read another parameter for this command
                  -- to replace the one we just gave up
                  skippedOrdCnt += 1
                  prmCnt += 1
                else
                  -- move junk trailing the y ordinate into a comment section
                  prms[p-skippedOrdCnt] = tonumber ord
                  if not prms[p-skippedOrdCnt]
                    junk\append prm, " "
                    skippedOrdCnt += 1
                    prmCnt += 1
                  else junk\append fragment, " "
                p += 1

              else logger\error msgs.parse.invalidQuirksMode, tostring ASS.config.quirks

          -- set the marker for skipping input/type checking in the drawing command constructors
          prms.__raw = true

          -- only create the command if the required number of ordinates have been specified
          if #prms == prevCmdType.__defProps.ords
            -- use the superfast internal constructor if available
            contour[d] = if prevCmdType.__defNew
              prevCmdType.__defNew prms
            else prevCmdType prms

          i += prmCnt

        -- TODO: also check for ordinates ending w/ junk here and apply quirks algo
        else logger\error msgs.parse.unsupportedCommand, cmdParts[i]
        i += 1
        d += 1

      if d > 0
        contours[c] = Contour contour
        contours[c].parent = parent

      return contours, junk
