---Renders feed changelog entries into release notes, grouping them by a conventional-commit-style
---marker at the head of each entry:
---
---    <type>[(<scope>)][!]: <message>
---
---`type` is `feat`, `fix`, or `change` (case-insensitive); an unrecognized leading token leaves the
---entry unmarked. `(scope)` is an optional area tag such as `(Updater)`. A trailing `!` flags a
---breaking change: it stays in its type's section but is tagged and floated to the top of it. A
---version with nothing marked renders as a flat list, no headings.
---
---`renderMarkdown` produces a GitHub release body (cross-package, bold scopes); `renderLog` produces
---the plain-text in-app changelog block (single package, glyph-headed sections). Framing a record's
---multi-version changelog around `renderLog` is the caller's job.

-- Marks an individual breaking entry within its section.
breakingMark = "⚠️"

-- Recognized types and their section, in render order. `glyph` heads the section in plain-text
-- output; the log window has no markdown, so the glyph carries the visual grouping there.
categories = {
  {key: "feat", heading: "New Features", glyph: "✨"}
  {key: "fix", heading: "Bug Fixes", glyph: "🐛"}
  {key: "change", heading: "Changes", glyph: "🔧"}
  {key: "other", heading: "Other Changes", glyph: "•"}
}

types = {feat: true, fix: true, change: true}

---A fresh set of empty per-section buckets, keyed by `categories` key.
---@return table<string, table[]> buckets
makeBuckets = -> {cat.key, {} for cat in *categories}

---Splits a changelog entry into its marker parts.
---@param entry string A changelog entry, optionally prefixed with a `<type>[(<scope>)][!]:` marker.
---@return {type: string?, scope: string?, breaking: boolean, message: string} `type` is nil and `message` holds the whole entry when no recognized marker is present.
parseEntry = (entry) ->
  marker, message = entry\match "^(%S-):%s+(.+)$"
  if marker
    breaking = marker\sub(-1) == "!"
    core = breaking and marker\sub(1, -2) or marker
    scope = core\match "%((.-)%)"
    typ = core\match "^(%a+)"
    typ and= typ\lower!
    if typ and types[typ]
      return {type: typ, :scope, :breaking, :message}
  {type: nil, scope: nil, breaking: false, message: entry}

---Section a parsed entry belongs to (its type, or "other" when unmarked).
---@param parsed table A `parseEntry` result.
---@return string key One of the `categories` keys.
bucketKeyFor = (parsed) -> parsed.type or "other"

---Turns per-section buckets of `{breaking, text}` into the non-empty sections in render order, each
---with its final lines (breaking entries tagged and floated to the top).
---@param buckets table<string, {breaking: boolean, text: string}[]> One list per `categories` key.
---@return table[] sections `{category, lines}` per non-empty section, in `categories` order.
---@return boolean flat True when the only non-empty section is `other`, so callers drop the headings.
getSections = (buckets) ->
  sections = {}
  for cat in *categories
    bucket = buckets[cat.key]
    continue if #bucket == 0
    ordered = ["#{breakingMark} #{e.text}" for e in *bucket when e.breaking]
    ordered[#ordered + 1] = e.text for e in *bucket when not e.breaking
    sections[#sections + 1] = {:cat, lines: ordered}
  sections, #sections == 1 and sections[1].cat.key == "other"

---Renders a GitHub-flavored markdown release body from one version's changelog across packages.
---@param packages {name: string, scope: string, primary: boolean?, entries: string[]}[] One item per package with entries for the version; `scope` is the fallback area tag for that package's entries that carry no marker scope. The `primary` package's unscoped entries render with no lead-in at all.
---@param opts? {title: string?} `title` prepends a top-level `# ` heading.
---@return string markdown Grouped notes in the fixed category order (a flat list when nothing is marked); empty when no entries.
renderMarkdown = (packages, opts = {}) ->
  buckets = makeBuckets!
  for pkg in *packages
    for entry in *pkg.entries
      parsed = parseEntry entry
      scope = parsed.scope
      unless parsed.scope or pkg.primary
        scope = pkg.scope
      text = scope and "**#{scope}:** #{parsed.message}" or parsed.message
      table.insert buckets[bucketKeyFor parsed], {breaking: parsed.breaking, :text}
  sections, flat = getSections buckets

  lines = {}
  if opts.title
    lines[#lines + 1] = "# #{opts.title}"
    lines[#lines + 1] = ""
  for s in *sections
    lines[#lines + 1] = "## #{s.cat.heading}" unless flat
    lines[#lines + 1] = "- #{item}" for item in *s.lines
    lines[#lines + 1] = "" unless flat
  lines[#lines] = nil if #lines > 0 and lines[#lines] == ""
  table.concat lines, "\n"

---Renders one package-version's changelog as the plain-text in-app changelog block. Each entry is
---reconstructed to its readable form (`scope: message`), dropping the machine `type` token so the
---marker convention never leaks into what users read; sections are headed by their glyph, and a
---version with nothing marked lists flat with no headings.
---@param entries string[] The raw changelog entries of a single package version.
---@return string block The rendered lines joined by newlines; empty when no entries.
renderLog = (entries) ->
  buckets = makeBuckets!
  for entry in *entries
    parsed = parseEntry entry
    text = parsed.scope and "#{parsed.scope}: #{parsed.message}" or parsed.message
    table.insert buckets[bucketKeyFor parsed], {breaking: parsed.breaking, :text}
  sections, flat = getSections buckets

  lines = {}
  for s in *sections
    lines[#lines + 1] = "#{s.cat.glyph} #{s.cat.heading}" unless flat
    lines[#lines + 1] = "  • #{item}" for item in *s.lines
  table.concat lines, "\n"

return {:parseEntry, :renderMarkdown, :renderLog}
