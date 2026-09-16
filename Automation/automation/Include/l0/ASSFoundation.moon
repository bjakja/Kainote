DependencyControl = require "l0.DependencyControl"
version = DependencyControl{
  name: "ASSFoundation",
  version: "0.5.0",
  description: "General purpose ASS processing library",
  author: "line0",
  url: "http://github.com/TypesettingTools/ASSFoundation",
  moduleName: "l0.ASSFoundation",
  feed: "https://raw.githubusercontent.com/TypesettingTools/ASSFoundation/master/DependencyControl.json",
  {
    "l0.ASSFoundation.ClassFactory",
    {"a-mo.LineCollection", version: "1.3.0", url: "https://github.com/TypesettingTools/Aegisub-Motion",
      feed: "https://raw.githubusercontent.com/TypesettingTools/Aegisub-Motion/DepCtrl/DependencyControl.json"},
    {"a-mo.Line", version: "1.5.3", url: "https://github.com/TypesettingTools/Aegisub-Motion",
      feed: "https://raw.githubusercontent.com/TypesettingTools/Aegisub-Motion/DepCtrl/DependencyControl.json"},
    {"SubInspector.Inspector", version: "0.7.2", url: "https://github.com/TypesettingTools/SubInspector",
      feed: "https://raw.githubusercontent.com/TypesettingTools/SubInspector/master/DependencyControl.json"},
    {"l0.Functional", version: "0.5.0", url: "https://github.com/TypesettingTools/Functional",
      feed: "https://raw.githubusercontent.com/TypesettingTools/Functional/master/DependencyControl.json"},
    {"Yutils", optional: true}
  }
}

modules, logger = {version\requireModules!}, version\getLogger!
createASSClass, LineCollection, Line, SubInspector, Functional, Yutils = unpack modules
{:list, :math, :string, :table, :unicode, :util, :re } = Functional

ASS = require("l0.ASSFoundation.FoundationMethods") createASSClass, Functional, LineCollection, Line, logger
ASSFInstMeta = __index: ASS
ASSFInstProxy = setmetatable {}, ASSFInstMeta
_, yutilsMissingMsg = version\checkOptionalModules "Yutils"

loadClass = (name) ->
  require("l0.ASSFoundation."..name) ASS, ASSFInstProxy, yutilsMissingMsg, createASSClass, Functional,
    LineCollection, Line, logger, SubInspector, Yutils

-- Base Classes
ASS.Base = loadClass "Base"
ASS.Tag, ASS.Draw = {}, {}
ASS.Tag.Base = loadClass "Tag.Base"
ASS.Draw.DrawingBase = loadClass "Draw.DrawingBase"
ASS.Draw.CommandBase = loadClass "Draw.CommandBase"

-- Primitives
ASS.Number = loadClass "Primitive.Number"
ASS.String = loadClass "Primitive.String"
ASS.Point = loadClass "Primitive.Point"
ASS.Time = loadClass "Primitive.Time"
ASS.Duration = createASSClass "Duration", ASS.Time, {"value"}, {"number"}, {positive: true}
ASS.Hex = createASSClass "Hex", ASS.Number, {"value"}, {"number"}, {range: {0,255}, base: 16, precision:0}

ASS.LineContents = loadClass "LineContents"
ASS.LineBounds = loadClass "LineBounds"
ASS.LineBoundsBatch = loadClass "LineBoundsBatch"
ASS.TagList = loadClass "TagList"

-- Sections
ASS.Section = {}
ASS.Section.Text = loadClass "Section.Text"
ASS.Section.Tag = loadClass "Section.Tag"
ASS.Section.Comment = loadClass "Section.Comment"
ASS.Section.Drawing = loadClass "Section.Drawing"

-- Tags
ASS.Tag.ClipRect = loadClass "Tag.ClipRect"
ASS.Tag.ClipVect = loadClass "Tag.ClipVect"
ASS.Tag.Color = loadClass "Tag.Color"
ASS.Tag.Fade = loadClass "Tag.Fade"
ASS.Tag.Indexed = loadClass "Tag.Indexed"
ASS.Tag.Align = loadClass "Tag.Align"
ASS.Tag.Move = loadClass "Tag.Move"
ASS.Tag.String = loadClass "Tag.String"
ASS.Tag.Transform = loadClass "Tag.Transform"
ASS.Tag.Toggle = loadClass "Tag.Toggle"
ASS.Tag.Weight = loadClass "Tag.Weight"
ASS.Tag.WrapStyle = createASSClass "Tag.WrapStyle", ASS.Tag.Indexed, {"value"}, {"number"}, {range: {0,3}, default: 0}
ASS.Tag.Unknown = loadClass "Tag.Unknown"

ASS.Draw.Contour = loadClass "Draw.Contour"
-- Drawing Command Classes
ASS.Draw.Bezier = loadClass "Draw.Bezier"
ASS.Draw.Close = loadClass "Draw.Close"
ASS.Draw.Line = loadClass "Draw.Line"
ASS.Draw.Move = loadClass "Draw.Move"
ASS.Draw.MoveNc = loadClass "Draw.MoveNc"

ASS.Draw.commands = {ASS.Draw.Bezier, ASS.Draw.Close, ASS.Draw.Line, ASS.Draw.Move, ASS.Draw.MoveNc}
list.makeSet ASS.Draw.commands, ASS.Draw.commands
-- Drawing Command -> Class Mappings
ASS.Draw.commandMapping = {}
for i=1, #ASS.Draw.commands
  ASS.Draw.commandMapping[ASS.Draw.commands[i].__defProps.name] = ASS.Draw.commands[i]

-- Parser
ASS.Parser = {
  Drawing: loadClass "Parser.Drawing"
  LineText: loadClass "Parser.LineText"
  Sections: loadClass "Parser.Sections"
}

-- Tag Mapping
ASS.tagMap = {
  scale_x: {
    sort: 6
    overrideName: "\\fscx"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\fscx([%d%.]+)", format: "\\fscx%.3N"
    }
    props: transformable: true
  }

  scale_y: {
    sort: 7
    overrideName: "\\fscy"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\fscy([%d%.]+)", format: "\\fscy%.3N"
    }
    props: transformable: true
  }
  align: {
    sort: 1
    overrideName: "\\an"
    type: ASS.Tag.Align
    signatures: {
      default: pattern: "^\\an([1-9])", format: "\\an%d"
    }
    props: global: true
  },
  angle: {
    sort: 8
    overrideName: "\\frz"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\frz?([%-%d%.]+)", format: "\\frz%.3N"
    }
    props: transformable: true
  }
  angle_y: {
    sort: 9
    overrideName: "\\fry"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\fry([%-%d%.]+)", format: "\\fry%.3N"
    }
    props: transformable: true
    default: {0}
  }
  angle_x: {
    sort: 10
    overrideName: "\\frx"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\frx([%-%d%.]+)", format: "\\frx%.3N"
    }
    props: transformable: true
    default: {0}
  }
  outline: {
    sort: 20
    overrideName:"\\bord"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\bord([%d%.]+)", format: "\\bord%.2N"
    }
    props: positive: true, transformable: true
  }
  outline_x: {
    sort: 21
    overrideName: "\\xbord"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\xbord([%d%.]+)", format: "\\xbord%.2N"
    }
    props: positive: true, transformable: true
  }
  outline_y: {
    sort: 22
    overrideName: "\\ybord"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\ybord([%d%.]+)", format: "\\ybord%.2N"
    }
    props: positive: true, transformable: true
  }
  shadow: {
    sort: 23
    overrideName: "\\shad"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\shad([%-%d%.]+)", format: "\\shad%N"
    }
    props: transformable: true, precision: math.huge
  }
  shadow_x: {
    sort: 24
    overrideName: "\\xshad"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\xshad([%-%d%.]+)", format: "\\xshad%N"
    }
    props: transformable: true, precision: math.huge
  }
  shadow_y: {
    sort: 25
    overrideName: "\\yshad"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\yshad([%-%d%.]+)", format: "\\yshad%N"
    }
    props: transformable: true, precision: math.huge
  }
  reset: {
    overrideName: "\\r"
    type: ASS.Tag.String
    signatures: {
      default: pattern: "^\\r([^\\}]*)", format: "\\r%s"
    }
    props: transformable: true
  }
  alpha: {
    sort: 30
    overrideName: "\\alpha"
    type: ASS.Hex
    signatures: {
      default: pattern: "^\\alpha&H(%x%x?)&?", format: "\\alpha&H%02X&"
    }
    props: transformable: true
    default: {0}
  }
  alpha1: {
    sort: 31
    overrideName: "\\1a"
    type: ASS.Hex
    signatures: {
      default: pattern: "^\\1a&H(%x%x)&", format: "\\1a&H%02X&"
    }
    props: transformable: true, master: "alpha"
  }
  alpha2: {
    sort: 32
    overrideName: "\\2a"
    type: ASS.Hex
    signatures: {
      default: pattern: "^\\2a&H(%x%x)&", format: "\\2a&H%02X&"
    }
    props: transformable: true, master: "alpha"
  }
  alpha3: {
    sort: 33
    overrideName: "\\3a"
    type: ASS.Hex
    signatures: {
      default: pattern: "^\\3a&H(%x%x)&", format: "\\3a&H%02X&"
    }
    props: transformable: true, master: "alpha"
  }
  alpha4: {
    sort: 34
    overrideName: "\\4a"
    type: ASS.Hex
    signatures: {
      default: pattern: "^\\4a&H(%x%x)&", format: "\\4a&H%02X&"
    }
    props: transformable: true, master: "alpha"
  }
  color1: {
    sort: 26
    overrideName: "\\1c"
    type: ASS.Tag.Color
    signatures: {
      default: pattern: "^\\1c&H%x-(%x%x)(%x%x)(%x%x)&?", format: "\\1c&H%02X%02X%02X&"
      short: pattern: "^\\c&H%x-(%x%x)(%x%x)(%x%x)&?", format: "\\c&H%02X%02X%02X&"
    }
    friendlyName: "\\1c  & \\c"
    props: transformable: true
  }
  color2: {
    sort: 27
    overrideName: "\\2c"
    type: ASS.Tag.Color
    signatures: {
      default: pattern: "^\\2c&H%x-(%x%x)(%x%x)(%x%x)&", format: "\\2c&H%02X%02X%02X&"
    }
    props: transformable: true
  }
  color3: {
    sort: 28
    overrideName: "\\3c"
    type: ASS.Tag.Color
    signatures: {
      default: pattern: "^\\3c&H%x-(%x%x)(%x%x)(%x%x)&", format: "\\3c&H%02X%02X%02X&"
    }
    props: transformable: true
  }
  color4: {
    sort: 29
    overrideName: "\\4c"
    type: ASS.Tag.Color
    signatures: {
      default: pattern: "^\\4c&H%x-(%x%x)(%x%x)(%x%x)&", format: "\\4c&H%02X%02X%02X&"
    }
    props: transformable: true
  }
  clip_vect: {
    sort: 41
    overrideName: "\\clip"
    type: ASS.Tag.ClipVect
    signatures: {
      default: pattern: "^\\clip%(([mnlbspc] .-)%)", format: "\\clip(%s)"
      scale: pattern: "^\\clip%((%d+),([mnlbspc] .-)%)", format: "\\clip(%d,%s)"
    }
    friendlyName: "\\clip  (Vector)"
    props: global: true, clip: true
  }
  iclip_vect: {
    sort: 42
    overrideName: "\\iclip"
    type: ASS.Tag.ClipVect
    signatures: {
      default: pattern: "^\\iclip%(([mnlbspc] .-)%)", format: "\\iclip(%s)"
      scale: pattern: "^\\iclip%((%d+),([mnlbspc] .-)%)", format: "\\iclip(%d,%s)"
    }
    friendlyName: "\\iclip  (Vector)",
    props: inverse: true, global: true, clip: true
    default: {"m 0 0 l 0 0 0 0 0 0 0 0"}
  }
  clip_rect: {
    sort: 39
    overrideName: "\\clip"
    type: ASS.Tag.ClipRect
    signatures: {
      default: {
        pattern: "^\\clip%(([%-%d%.]+),([%-%d%.]+),([%-%d%.]+),([%-%d%.]+)%)",
        format: "\\clip(%.2N,%.2N,%.2N,%.2N)"
      }
    }
    friendlyName: "\\clip  (Rectangle)"
    props: transformable: true, global: false, clip: true
  }
  iclip_rect: {
    sort: 40
    overrideName: "\\iclip"
    type: ASS.Tag.ClipRect
    signatures: {
      default: {
        pattern: "^\\iclip%(([%-%d%.]+),([%-%d%.]+),([%-%d%.]+),([%-%d%.]+)%)",
        format: "\\iclip(%.2N,%.2N,%.2N,%.2N)"
      }
    }
    friendlyName: "\\iclip  (Rectangle)",
    props: inverse: true, global: false, transformable: true, clip: true
    default: {0,0,0,0}
  }
  drawing: {
    sort: 44
    overrideName: "\\p"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\p(%d+)", format: "\\p%d"
    }
    props: positive: true, integer: true, precision: 0
    default: {0}
  }
  blur_edges: {
    sort: 36
    overrideName: "\\be"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\be([%d%.]+)", format: "\\be%.2N"
    }
    props: positive: true, transformable: true
    default: {0}
  }
  blur: {
    sort: 35
    overrideName: "\\blur"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\blur([%d%.]+)", format: "\\blur%.2N"
    }
    props: positive: true, transformable: true
    default: {0}
  }
  shear_x: {
    sort: 11
    overrideName: "\\fax"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\fax([%-%d%.]+)", format: "\\fax%.3N"
    }
    props: transformable: true
    default: {0}
  }
  shear_y: {
    sort: 12
    overrideName: "\\fay"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\fay([%-%d%.]+)", format: "\\fay%.3N"
    }
    props: transformable: true
    default: {0}
  }
  bold: {
    sort: 16
    overrideName: "\\b"
    type: ASS.Tag.Weight
    signatures: {
      default: pattern: "^\\b(%d+)", format: "\\b%d"
    }
  }
  italic: {
    sort: 17
    overrideName: "\\i"
    type: ASS.Tag.Toggle
    signatures: {
      default: pattern: "^\\i([10])", format: "\\i%d"
    }
  }
  underline: {
    sort: 18
    overrideName: "\\u"
    type: ASS.Tag.Toggle
    signatures: {
      default: pattern: "^\\u([10])", format: "\\u%d"
    }
  }
  strikeout: {
    sort: 19
    overrideName: "\\s"
    type: ASS.Tag.Toggle
    signatures: {
      default: pattern: "^\\s([10])", format: "\\s%d"
    }
  }
  spacing: {
    sort: 15
    overrideName: "\\fsp"
    type: ASS.Number,
    signatures: {
      default: pattern: "^\\fsp([%-%d%.]+)", format: "\\fsp%.2N"
    }
    props: transformable: true
  }
  fontsize: {
    sort: 14
    overrideName: "\\fs"
    type: ASS.Number
    signatures: {
      default: pattern: "^\\fs([%d%.]+)", format: "\\fs%.2N"
    }
    props: positive: true, transformable: true
  }
  fontname: {
    sort: 13
    overrideName: "\\fn"
    type: ASS.Tag.String
    signatures: {
      default: pattern: "^\\fn([^\\}]*)", format: "\\fn%s"
    }
  }
  k_fill: {
    sort: 45
    overrideName: "\\k"
    type: ASS.Duration
    signatures: {
      default: pattern: "^\\k([%d]+)", format: "\\k%d"
    }
    props: scale: 10, karaoke: true, multi: true
    default: {0}
  }
  k_sweep: {
    sort: 46
    overrideName: "\\kf"
    type: ASS.Duration
    signatures: {
      default: pattern: "^\\kf([%d]+)", format: "\\kf%d"
      short: pattern: "^\\K([%d]+)", format: "\\K%d"
    }
    props: scale: 10, karaoke: true, multi: true
    default: {0}
  }
  k_bord: {
    sort: 48
    overrideName: "\\ko"
    type: ASS.Duration
    signatures: {
      default: pattern: "^\\ko([%d]+)", format: "\\ko%d"
    }
    props: scale: 10, karaoke: true, multi: true
    default: {0}
  }
  position: {
    sort: 2
    overrideName: "\\pos"
    type: ASS.Point
    signatures: {
      default: pattern: "^\\pos%(([%-%d%.]+),([%-%d%.]+)%)", format: "\\pos(%.3N,%.3N)"
    }
    props: global: true, position: true
  }
  move: {
    sort: 3
    overrideName: "\\move"
    type: ASS.Tag.Move
    signatures: {
      default: {
        pattern: "^\\move%(([%-%d%.]+),([%-%d%.]+),([%-%d%.]+),([%-%d%.]+),([%-%d]+),([%-%d]+)%)",
        format: "\\move(%.3N,%.3N,%.3N,%.3N,%.3N,%.3N)"
      }
      simple: {
        pattern: "^\\move%(([%-%d%.]+),([%-%d%.]+),([%-%d%.]+),([%-%d%.]+)%)",
        format: "\\move(%.3N,%.3N,%.3N,%.3N)"
      }
    }
    props: global: true, position: true
  }
  origin: {
    sort: 5
    overrideName: "\\org"
    type: ASS.Point
    signatures: {
      default: pattern: "^\\org%(([%-%d%.]+),([%-%d%.]+)%)", format: "\\org(%.3N,%.3N)"
    }
    props: global: true
  }
  wrapstyle: {
    sort: 43
    overrideName: "\\q"
    type: ASS.Tag.WrapStyle
    signatures: {
      default: pattern: "^\\q(%d)", format: "\\q%d"
    }
    props: global: true
    default: {0}
  }
  fade_simple: {
    sort: 37
    overrideName: "\\fad"
    type: ASS.Tag.Fade
    signatures: {
      default: pattern: "^\\fad%((%d+),(%d+)%)", format: "\\fad(%d,%d)"
    }
    props: global: true
    default: {0,0}
  }
  fade: {
    sort: 38
    overrideName: "\\fade"
    type: ASS.Tag.Fade
    signatures: {
      default: {
        pattern: "^\\fade%((%d+),(%d+),(%d+),([%-%d]+),([%-%d]+),([%-%d]+),([%-%d]+)%)",
        format: "\\fade(%d,%d,%d,%d,%d,%d,%d)"
      }
    }
    props: global: true
    default: {255,0,255,0,0,0,0}
  }
  transform: {
    overrideName: "\\t"
    type: ASS.Tag.Transform,
    signatures: {
      default: pattern: "^\\t%(([%-%d%.]+),([%-%d%.]+),([%d%.]+),(.+)%)", format: "\\t(%.2N,%.2N,%.2N,%s)"
      simple: pattern: "^\\t%((\\.+)%)", format: "\\t(%s)"
      accel: pattern: "^\\t%(([%d%.]+),(\\.+)%)", format: "\\t(%.2N,%s)"
      time: pattern: "^\\t%(([%-%d%.]+),([%-%d%.]+),(\\.+)%)", format: "\\t(%.2N,%.2N,%s)"
    }
  }
  unknown: {
    sort: 98
    type: ASS.Tag.Unknown
    friendlyName: "Unknown Tag"
    signatures: {
      default: format: "%s"
    }
    props: nonOverriding: true, multi: true
  }
  junk: {
    sort: 99
    type: ASS.Tag.Unknown
    friendlyName: "Junk"
    signatures: {
      default: format: "%s"
    }
  }
}

ASS.tagNames = {
  all: table.keys ASS.tagMap,
  ASSv4Plus: table.keys ASS.tagMap, {"unknown", "junk"}
  noPos: table.keys ASS.tagMap, "position"
  clips: ASS\getTagsNamesFromProps clip: true
  karaoke: ASS\getTagsNamesFromProps karaoke: true
  position: ASS\getTagsNamesFromProps position: true
}

ASS.toFriendlyName, ASS.toTagName, ASS.tagSortOrder = {}, {}, {}

for name, tag in pairs ASS.tagMap
  -- insert tag name into props
  tag.props or= {}
  tag.props.name or= name

  -- generate properties for treating rectangular clips as global tags
  tag.props.globalOrRectClip = tag.props.global or tag.type==ASS.Tag.ClipRect
  -- fill in missing friendly names
  tag.friendlyName = tag.friendlyName or tag.overrideName
  -- populate friendly name <-> tag name conversion tables
  if tag.friendlyName
    ASS.toFriendlyName[name], ASS.toTagName[tag.friendlyName] = tag.friendlyName, name
  -- fill tag names table
  tagType = ASS.tagNames[tag.type]
  if not tagType
    ASS.tagNames[tag.type] = {name, n: 1}
  else
    tagType[tagType.n+1], tagType.n = name, tagType.n+1

  -- fill override tag name -> internal tag name mapping tables
  if tag.overrideName
    ovrToName = ASS.tagNames[tag.overrideName]
    if ovrToName
      ovrToName[#ovrToName+1] = name
    else ASS.tagNames[tag.overrideName] = {name}

  -- fill sort order table
  if tag.sort
    ASS.tagSortOrder[tag.sort] = name

  -- create master/child relations
  if tag.props.master
    masterTag = ASS.tagMap[tag.props.master]
    masterTag.props or= {}
    masterTag.props.children or= {}
    masterTag.props.children[#masterTag.props.children+1] = name


ASS.tagSortOrder = table.continuous ASS.tagSortOrder

-- make name tables also work as sets
for _,names in pairs ASS.tagNames
  names.n = #names unless names.n
  list.makeSet names, names


ASS.defaults = {
  line: {
    actor: "", class: "dialogue", comment: false, effect: "", start_time: 0, end_time: 5000, layer: 0,
    margin_l: 0, margin_r: 0, margin_t: 0, section: "[Events]", style: "Default", text: "", extra: {}
  }
  drawingTestTags: ASS.Section.Tag {
    ASS\createTag("position",0,0), ASS\createTag("align",7),
    ASS\createTag("outline", 0), ASS\createTag("scale_x", 100), ASS\createTag("scale_y", 100),
    ASS\createTag("alpha", 0), ASS\createTag("angle", 0), ASS\createTag("shadow", 0)
  }
}

ASS.Quirks = {
  VSFilter: 1
  libass: 2
}

ASS.config = {
  fixDrawings: false
  quirks: ASS.Quirks.VSFilter
}

ASS.version = version

ASSFInst = ASS()
ASSFInstMeta.__index = ASSFInst

return version\register(ASSFInst)
