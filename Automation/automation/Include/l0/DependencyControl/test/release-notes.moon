-- release-notes tests: changelog marker parsing and the markdown / log renderings.
-- Called from test.moon as: (require "...test.release-notes")!
->
  ReleaseNotes = require "l0.DependencyControl.release-notes"
  {:parseEntry, :renderMarkdown, :renderLog} = ReleaseNotes

  {
    _description: "Tests for changelog marker parsing and release-note rendering."

    parseEntry_marked: (ut) ->
      p = parseEntry "fix(Updater): a bug"
      ut\assertEquals p.type, "fix"
      ut\assertEquals p.scope, "Updater"
      ut\assertFalsy p.breaking
      ut\assertEquals p.message, "a bug"

    parseEntry_breaking: (ut) ->
      p = parseEntry "change!: gone"
      ut\assertEquals p.type, "change"
      ut\assertNil p.scope
      ut\assertTrue p.breaking
      ut\assertEquals p.message, "gone"

    parseEntry_breakingWithScope: (ut) ->
      p = parseEntry "feat(API)!: new thing"
      ut\assertEquals p.type, "feat"
      ut\assertEquals p.scope, "API"
      ut\assertTrue p.breaking
      ut\assertEquals p.message, "new thing"

    parseEntry_caseInsensitive: (ut) ->
      ut\assertEquals (parseEntry "FIX: x").type, "fix"

    -- a leading component label that isn't a recognized type is not a marker
    parseEntry_unknownTypeStaysUnmarked: (ut) ->
      p = parseEntry "Updater: something happened"
      ut\assertNil p.type
      ut\assertEquals p.message, "Updater: something happened"

    parseEntry_plainUnmarked: (ut) ->
      p = parseEntry "Just some prose without a marker"
      ut\assertNil p.type
      ut\assertFalsy p.breaking
      ut\assertEquals p.message, "Just some prose without a marker"

    renderMarkdown_groupsInFixedOrder: (ut) ->
      pkgs = {{name: "P", scope: "P", primary: true, entries: {
        "feat: added it", "fix: fixed it", "change: changed it", "a loose note"}}}
      md = renderMarkdown pkgs
      f = md\find "## New Features", 1, true
      x = md\find "## Bug Fixes", 1, true
      c = md\find "## Changes", 1, true
      o = md\find "## Other Changes", 1, true
      ut\assertNotNil f
      ut\assertNotNil x
      ut\assertNotNil c
      ut\assertNotNil o
      ut\assertTrue f < x and x < c and c < o
      ut\assertFalsy md\find "Breaking Changes", 1, true -- breaking is not a section of its own

    renderMarkdown_breakingMarkedAtTopOfSection: (ut) ->
      pkgs = {{name: "P", scope: "P", primary: true, entries: {
        "change: regular change", "change(API)!: breaking change"}}}
      md = renderMarkdown pkgs
      ut\assertContains md, "## Changes"
      ut\assertContains md, "⚠️ **API:** breaking change" -- breaking tagged, kept in its type section
      mark = md\find "⚠️", 1, true
      reg = md\find "regular change", 1, true
      ut\assertNotNil mark
      ut\assertTrue mark < reg -- floated above the non-breaking entry

    renderMarkdown_barePrimaryBoldOthers: (ut) ->
      pkgs = {
        {name: "Main", scope: "Main", primary: true, entries: {"feat: primary bare"}}
        {name: "Side", scope: "Side", entries: {"feat: side attributed"}}
      }
      md = renderMarkdown pkgs
      ut\assertContains md, "- primary bare" -- primary unscoped entry: no lead-in
      ut\assertContains md, "**Side:** side attributed" -- non-primary: package fallback scope
      ut\assertFalsy md\find "**Main:**", 1, true -- the primary never gets a package lead-in

    renderMarkdown_markerScopeOverridesFallback: (ut) ->
      md = renderMarkdown {{name: "Side", scope: "Side", entries: {"fix(Net): a fix"}}}
      ut\assertContains md, "**Net:** a fix"
      ut\assertFalsy md\find "**Side:**", 1, true

    renderMarkdown_emptyWhenNoEntries: (ut) ->
      ut\assertEquals (renderMarkdown {}), ""

    renderMarkdown_allUnmarkedIsFlat: (ut) ->
      md = renderMarkdown {{name: "P", scope: "P", primary: true, entries: {"loose one", "loose two"}}}
      ut\assertContains md, "- loose one"
      ut\assertContains md, "- loose two"
      ut\assertFalsy md\find "#", 1, true -- no headings at all when nothing is marked

    renderLog_reconstructsAndGroups: (ut) ->
      out = renderLog {"fix(Updater): a bug", "feat: a feature"}
      ut\assertContains out, "New Features"
      ut\assertContains out, "Bug Fixes"
      ut\assertContains out, "  • Updater: a bug" -- scope kept, machine type token dropped
      ut\assertContains out, "  • a feature"
      ut\assertFalsy out\find "fix(Updater):", 1, true

    renderLog_allUnmarkedIsFlat: (ut) ->
      out = renderLog {"plain one", "plain two"}
      ut\assertContains out, "  • plain one"
      ut\assertContains out, "  • plain two"
      ut\assertFalsy out\find "Other Changes", 1, true -- no heading when nothing is marked

    _order: {
      "parseEntry_marked", "parseEntry_breaking", "parseEntry_breakingWithScope",
      "parseEntry_caseInsensitive", "parseEntry_unknownTypeStaysUnmarked", "parseEntry_plainUnmarked",
      "renderMarkdown_groupsInFixedOrder", "renderMarkdown_breakingMarkedAtTopOfSection",
      "renderMarkdown_barePrimaryBoldOthers",
      "renderMarkdown_markerScopeOverridesFallback", "renderMarkdown_emptyWhenNoEntries",
      "renderMarkdown_allUnmarkedIsFlat",
      "renderLog_reconstructsAndGroups", "renderLog_allUnmarkedIsFlat"
    }
  }
