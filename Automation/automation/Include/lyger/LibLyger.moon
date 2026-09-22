[[
README

This file is a library of commonly used functions across all my automation
scripts. This way, if there are errors or updates for any of these functions,
I'll only need to update one file.

The filename is a bit vain, perhaps, but I couldn't come up with anything else.

]]

DependencyControl = require("l0.DependencyControl")
version = DependencyControl{
    name: "LibLyger",
    version: "2.0.3",
    description: "Library of commonly used functions across all of lyger's automation scripts.",
    author: "lyger",
    url: "http://github.com/TypesettingTools/lyger-Aegisub-Scripts",
    moduleName: "lyger.LibLyger",
    feed: "https://raw.githubusercontent.com/TypesettingTools/lyger-Aegisub-Scripts/master/DependencyControl.json",
    {
        "aegisub.util", "karaskel"
    }
}
util = version\requireModules!

class LibLyger
    msgs = {
        preproc_lines: {
            bad_type: "Error: argument #1 must be either a line object, an index into the subtitle object or a table of indexes; got a %s."
        }
    }
    new: (sub, sel, generate_furigana) =>
        @set_sub sub, sel, generate_furigana if sub

    set_sub: (@sub, @sel = {}, generate_furigana = false) =>
        local has_script_info

        @script_info, @lines, @dialogue, @dlg_cnt = {}, {}, {}, 0
        for i, line in ipairs sub
            @lines[i] = line
            switch line.class
                when "info" then @script_info[line.key] = line.value
                when "dialogue"
                    @dlg_cnt += 1
                    @dialogue[@dlg_cnt], line.i = line, i

        @meta, @styles = karaskel.collect_head @sub, generate_furigana
        @preproc_lines @sel

    insert_line: (line, i = #@lines + 1) =>
        table.insert(@lines, i, line)
        @sub.insert(i, line)

    preproc_lines: (lines) =>
        val_type = type lines
        -- indexes into the subtitles object
        if val_type == "number"
            lines, val_type = {@lines[lines]}, "table"
        assert val_type == "table", msgs.preproc_lines.bad_type\format val_type

        -- line objects
        if lines.raw and lines.section and not lines.duration
            karaskel.preproc_line @sub, @meta, @styles, lines
        -- tables of line numbers/objects such as the selection
        else @preproc_lines line for line in *lines

    -- returns a "Lua" portable version of the string
    exportstring: (s) -> string.format "%q", s

    --Lookup table for the nature of each kind of parameter
    param_type: {
        alpha: "alpha"
        "1a":  "alpha"
        "2a":  "alpha"
        "3a":  "alpha"
        "4a":  "alpha"
        c:     "color"
        "1c":  "color"
        "2c":  "color"
        "3c":  "color"
        "4c":  "color"
        fscx:  "number"
        fscy:  "number"
        frz:   "angle"
        frx:   "angle"
        fry:   "angle"
        shad:  "number"
        bord:  "number"
        fsp:   "number"
        fs:    "number"
        fax:   "number"
        fay:   "number"
        blur:  "number"
        be:    "number"
        xbord: "number"
        ybord: "number"
        xshad: "number"
        yshad: "number"
        pos: "point"
        org: "point"
        clip: "clip"
        }

    --Convert float to neatly formatted string
    float2str: (f) -> "%.3f"\format(f)\gsub("%.(%d-)0+$","%.%1")\gsub "%.$", ""

    --Escapes string for use in gsub
    esc: (str) -> str\gsub "([%%%(%)%[%]%.%*%-%+%?%$%^])","%%%1"

    [[
    Tags that can have any character after the tag declaration: \r, \fn
    Otherwise, the first character after the tag declaration must be:
    a number, decimal point, open parentheses, minus sign, or ampersand
    ]]

    -- Remove listed tags from the given text
    line_exclude: (text, exclude) ->
        remove_t = false
        new_text = text\gsub "\\([^\\{}]*)", (a) ->
            if a\match "^r"
                for val in *exclude
                    return "" if val == "r"
            elseif a\match "^fn"
                for val in *exclude
                    return "" if val == "fn"
            else
                tag = a\match "^[1-4]?%a+"
                for val in *exclude
                    if val == tag
                        --Hacky exception handling for \t statements
                        if val == "t"
                            remove_t = true
                            return "\\#{a}"
                        elseif a\match "%)$"
                            return a\match("%b()") and "" or ")"
                        else
                            return ""
            return "\\"..a

        if remove_t
            new_text = new_text\gsub "\\t%b()", ""

        return new_text\gsub "{}", ""

    -- Remove all tags except the given ones
    line_exclude_except: (text, exclude) ->
        remove_t = true
        new_text = text\gsub "\\([^\\{}]*)", (a) ->
            if a\match "^r"
                for val in *exclude
                    return "\\#{a}" if val == "r"
            elseif a\match "^fn"
                for val in *exclude
                    return "\\#{a}" if val == "fn"
            else
                tag = a\match "^[1-4]?%a+"
                for val in *exclude
                    if val == tag
                        remove_t = false if val == "t"
                        return "\\#{a}"

            if a\match "^t"
                return "\\#{a}"
            elseif a\match "%)$"
                return a\match("%b()") and "" or ")"
            else return ""

        if remove_t
            new_text = new_text\gsub "\\t%b()", ""

        return new_text

    -- Returns the position of a line
    get_default_pos: (line, align_x, align_y) =>
        @preproc_lines line
        x = {
            @script_info.PlayResX - line.eff_margin_r,
            line.eff_margin_l,
            line.eff_margin_l + (@script_info.PlayResX - line.eff_margin_l - line.eff_margin_r) / 2
        }
        y = {
            @script_info.PlayResY - line.eff_margin_b,
            @script_info.PlayResY / 2
            line.eff_margin_t
        }
        return x[align_x], y[align_y]

    get_pos: (line) =>
        posx, posy = line.text\match "\\pos%(([%d%.%-]*),([%d%.%-]*)%)"
        unless posx
            posx, posy = line.text\match "\\move%(([%d%.%-]*),([%d%.%-]*),"
        return tonumber(posx), tonumber(posy) if posx

        -- \an alignment
        if align = tonumber line.text\match "\\an([%d%.%-]+)"
            return @get_default_pos line, align%3 + 1, math.ceil align/3
        -- \a alignment
        elseif align = tonumber line.text\match "\\a([%d%.%-]+)"
            return @get_default_pos line, align%4,
                                   align > 8 and 2 or align> 4 and 3 or 1
        -- no alignment tags (take karaskel values)
        else return line.x, line.y

    -- Returns the origin of a line
    get_org: (line) =>
        orgx, orgy = line.text\match "\\org%(([%d%.%-]*),([%d%.%-]*)%)"
        if orgx
            return orgx, orgy
        else return @get_pos line

    -- Returns a table of default values
    style_lookup: (line) =>
        @preproc_lines line
        return {
            alpha: "&H00&"
            "1a":  util.alpha_from_style line.styleref.color1
            "2a":  util.alpha_from_style line.styleref.color2
            "3a":  util.alpha_from_style line.styleref.color3
            "4a":  util.alpha_from_style line.styleref.color4
            c:     util.color_from_style line.styleref.color1
            "1c":  util.color_from_style line.styleref.color1
            "2c":  util.color_from_style line.styleref.color2
            "3c":  util.color_from_style line.styleref.color3
            "4c":  util.color_from_style line.styleref.color4
            fscx:  line.styleref.scale_x
            fscy:  line.styleref.scale_y
            frz:   line.styleref.angle
            frx:   0
            fry:   0
            shad:  line.styleref.shadow
            bord:  line.styleref.outline
            fsp:   line.styleref.spacing
            fs:    line.styleref.fontsize
            fax:   0
            fay:   0
            xbord: line.styleref.outline
            ybord: line.styleref.outline
            xshad: line.styleref.shadow
            yshad: line.styleref.shadow
            blur:  0
            be:    0
        }

    -- Modify the line tables so they are split at the same locations
    match_splits: (line_table1, line_table2) ->
        for i=1, #line_table1
            text1 = line_table1[i].text
            text2 = line_table2[i].text

            insert = (target, text, i) ->
                for j = #target, i+1, -1
                    target[j+1] = target[j]

                target[i+1] = tag: "{}", text: target[i].text\match "#{LibLyger.esc(text)}(.*)"
                target[i] = tag: target[i].tag, :text

            if #text1 > #text2
                -- If the table1 item has longer text, break it in two based on the text of table2
                insert line_table1, text2, i
            elseif #text2 > #text1
                -- If the table2 item has longer text, break it in two based on the text of table1
                insert line_table2, text1, i

        return line_table1, line_table2

    -- Remove listed tags from any \t functions in the text
    time_exclude: (text, exclude) ->
        text = text\gsub "(\\t%b())", (a) ->
            b = a
            for tag in *exclude
                if a\match "\\#{tag}"
                    b = b\gsub(tag == "clip" and "\\#{tag}%b()" or "\\#{tag}[^\\%)]*", "")
            return b

        -- get rid of empty blocks
        return text\gsub "\\t%([%-%.%d,]*%)", ""

    -- Returns a state table, restricted by the tags given in "tag_table"
    -- WILL NOT WORK FOR \fn AND \r
    make_state_table: (line_table, tag_table) ->
        this_state_table = {}
        for i, val in ipairs line_table
            temp_line_table = {}
            pstate = LibLyger.line_exclude_except val.tag, tag_table
            for j, ctag in ipairs tag_table
                -- param MUST start in a non-alpha character, because ctag will never be \r or \fn
                -- If it is, you fucked up
                param = pstate\match "\\#{ctag}(%A[^\\{}]*)"
                temp_line_table[ctag] = param if param

            this_state_table[i] = temp_line_table
        return this_state_table
    interpolate: (this_table, start_state_table, end_state_table, factor, preset) ->
        this_current_state = {}

        rebuilt_text = for k, val in ipairs this_table
            temp_tag = val.tag
            -- Cycle through all the tag blocks and interpolate
            for ctag, param in pairs start_state_table[k]
                temp_tag = "{}" if #temp_tag == 0
                temp_tag = temp_tag\gsub "}", ->
                    tval_start, tval_end = start_state_table[k][ctag], end_state_table[k][ctag]
                    tag_type = LibLyger.param_type[ctag]
                    ivalue = switch tag_type
                        when "alpha"
                            util.interpolate_alpha factor, tval_start, tval_end
                        when "color"
                            util.interpolate_color factor, tval_start, tval_end
                        when "number", "angle"
                            nstart, nend = tonumber(tval_start), tonumber(tval_end)
                            if tag_type == "angle" and preset.c.flip_rot
                                nstart %= 360
                                nend %= 360
                                ndelta = nend - nstart
                                if 180 < math.abs ndelta
                                    nstart += ndelta * 360 / math.abs ndelta

                            nvalue = util.interpolate factor, nstart, nend
                            nvalue += 360 if tag_type == "angle" and nvalue < 0

                            LibLyger.float2str nvalue
                        when "point", "clip" then nil -- not touched by this function
                        else ""

                    -- check for redundancy
                    if this_current_state[ctag] == ivalue
                        return "}"
                    this_current_state[ctag] = ivalue
                    return "\\#{ctag..ivalue}}"
            temp_tag .. val.text

        return table.concat(rebuilt_text)\gsub "{}", ""

    write_table: (my_table, file, indent) ->
        indent or= ""
        charS, charE = "   ", "\n"

        --Opening brace of the table
        file\write "#{indent}{#{charE}"

        for key,val in pairs my_table
            file\write switch type key
                when "number" then indent..charS
                when "string" then table.concat {indent, charS, "[", LibLyger.exportstring(key), "]="}
                else "#{indent}#{charS}#{key}="

            switch type val
                when "table"
                    file\write charE
                    LibLyger.write_table val, file, indent..charS
                    file\write indent..charS
                when "string" then file\write LibLyger.exportstring val
                when "number" then file\write tostring val
                when "boolean" then file\write val and "true" or "false"

            file\write ","..charE

        -- Closing brace of the table
        file\write "#{indent}}#{charE}"

    :version

return version\register LibLyger