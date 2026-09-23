constants = require "l0.DependencyControl.Constants"
domain = require "l0.DependencyControl.domain"
Enum = require "l0.DependencyControl.Enum"
ScriptUpdateRecord = require "l0.DependencyControl.ScriptUpdateRecord"

local UpdateTask

---How a feed was discovered — the source it came from. A feed can have several.
---@alias FeedProvenance
---| "official-depctrl" # OfficialDepCtrl: DependencyControl's own feed
---| "official-known" # OfficialKnown: a feed advertised in DependencyControl's own feed's knownFeeds
---| "user-extra" # UserExtra: a feed in the user's extraFeeds (a discovery root)
---| "package-declared" # PackageDeclared: the feed an installed package declares for its own updates
---| "package-override" # PackageOverride: an installed package's per-package userFeed override
---| "dependency-advertised" # DependencyAdvertised: a feed advertised in an installed package's requiredModules
---| "transitive-known" # TransitiveKnown: a feed advertised in another fetched feed's knownFeeds (crawl only)
Provenance = Enum "FeedProvenance", {
  OfficialDepCtrl: "official-depctrl"
  OfficialKnown: "official-known"
  UserExtra: "user-extra"
  PackageDeclared: "package-declared"
  PackageOverride: "package-override"
  DependencyAdvertised: "dependency-advertised"
  TransitiveKnown: "transitive-known"
}

-- Display order for a feed's provenance, most authoritative origin first. Pinned here because Enum.values
-- follows hash order, not definition order.
provenanceOrder = {
  Provenance.OfficialDepCtrl
  Provenance.OfficialKnown
  Provenance.UserExtra
  Provenance.PackageDeclared
  Provenance.PackageOverride
  Provenance.DependencyAdvertised
  Provenance.TransitiveKnown
}

---Which crawl budget a truncation hit.
---@alias FeedCrawlLimit
---| "per-feed" # PerFeed: the cap on how many untrusted feeds one feed may contribute
---| "per-root" # PerRoot: the per-subtree budget for untrusted expansion
---| "depth" # Depth: the crawl-depth limit; a feed at this depth is left unfetched
CrawlLimit = Enum "FeedCrawlLimit", {
  PerFeed: "per-feed"
  PerRoot: "per-root"
  Depth: "depth"
}

-- Ensures inventoryEntriesByUrl has a (possibly blank) entry for url; returns it, or nil for an invalid url.
ensureInventoryEntry = (inventoryEntriesByUrl, url) ->
  return nil unless type(url) == "string" and #url > 0
  entry = inventoryEntriesByUrl[url]
  unless entry
    entry = {:url, provenance: {}, packages: {}, advertisedBy: {}}
    inventoryEntriesByUrl[url] = entry
  entry

-- Ensures inventoryEntriesByUrl has an entry for url and tags it with the given provenance source; returns the entry (nil for
-- an invalid url). The `packages`/`advertisedBy` sets are filled by the caller.
addSource = (inventoryEntriesByUrl, url, source) ->
  entry = ensureInventoryEntry inventoryEntriesByUrl, url
  entry.provenance[source] = true if entry
  entry

-- The sorted keys of a set, as a list.
getSortedKeys = (set) ->
  keys = [k for k in pairs set]
  table.sort keys
  keys

-- A shallow copy of `list` with `item` appended, leaving the original untouched (for per-node crawl routes).
appended = (list, item) ->
  copy = [x for x in *list]
  copy[#copy + 1] = item
  copy

-- Cap on how many dropped feed URLs one truncation event records; its `dropped` count stays exact.
maxDropSample = 50

---A reachable feed with the sources it was discovered through and its trust status.
---@class FeedInventoryEntry
---@field url string The feed URL.
---@field provenance FeedProvenance[] The sources this feed was found through, in a stable order (empty for a feed present only in `trustedFeeds`).
---@field packages string[] Sorted namespaces of installed packages that declare, override to, or advertise this feed (empty when none).
---@field advertisedBy string[] Sorted URLs of fetched feeds that list this feed in their knownFeeds (crawl only).
---@field trustStatus FeedTrustStatus The feed's trust status.
---@field blockedBy? BlockedFeedEntry The block entry matching this feed when `trustStatus` is blocked (carries the reason and official/user flag).
---@field fetched? boolean True when `crawl` successfully read this feed; nil for `gather` (offline) or a feed the crawl couldn't reach.
---@field lastFetchedAt? integer Unix time this feed was last successfully fetched into the persistent cache; nil if it was never cached.
---@field inUse? boolean True when this feed is the effective update source (override else declared feed) of an installed package.
---@field inTrustedFeeds? boolean True when the feed is in the user's `trustedFeeds` — a trust-only listing that grants trust without acting as a discovery source.

---The crawl budgets, keyed by `FeedCrawlLimit`; a missing key falls back to a built-in default.
---@alias FeedCrawlLimits table<FeedCrawlLimit, integer>

---A budget the crawl hit, with enough context to act on it (block a feed, file a report).
---@class FeedCrawlTruncation
---@field limit FeedCrawlLimit Which budget was hit.
---@field limitValue integer The configured value of that budget.
---@field feed string The feed being processed when the limit hit (the advertiser for per-feed/per-root, the unfetched feed itself for depth).
---@field root string The config-derived feed whose subtree this occurred in.
---@field depth integer The crawl depth of `feed` (0 for a config-derived root).
---@field route string[] The feed URLs from `root` to `feed` inclusive — how the crawl reached it.
---@field dropped integer How many advertised feeds this event left unexplored (0 for depth, whose single unfetched feed is `feed`).
---@field droppedUrls string[] The dropped feed URLs, capped to a sample when `dropped` exceeds it; empty for depth.

---What a crawl explored and where it stopped short.
---@class FeedCrawlStats
---@field fetched integer How many feeds had their `knownFeeds` read.
---@field truncated boolean Whether any budget was hit (a quick "results incomplete?" check).
---@field truncations FeedCrawlTruncation[] Each budget hit, in crawl order, for surfacing or acting on.

---Enumerates the feeds DependencyControl knows about — from the user config, installed packages, and its
---official trust lists — each tagged with the sources it was found through and its trust status, for the Manage
---Feeds UI. `gather` is network-free; `crawl` additionally discovers transitively-advertised feeds through an
---injected feed loader, bounding untrusted expansion.
---@class FeedInventory
class FeedInventory
  @Provenance = Provenance
  @CrawlLimit = CrawlLimit

  ---The built-in crawl budgets, used for any budget the config doesn't set (and the config's own default).
  ---@type FeedCrawlLimits
  @defaultCrawlLimits = {
    [CrawlLimit.Depth]: 7
    [CrawlLimit.PerRoot]: 50
    [CrawlLimit.PerFeed]: 25
  }

  ---@param config ConfigView The global config view; its `c` holds `extraFeeds`/`trustedFeeds`/`macros`/`modules`, `fetchUntrustedFeeds`, and `feedCrawlLimits`.
  ---@param feedTrust FeedTrust The trust model, for the official feeds and trust/block queries.
  ---@param feedLoader FeedLoader Loads feeds during a crawl and holds the feed cache read for last-fetch times.
  new: (@config, @feedTrust, @feedLoader) =>

  ---The feed a package's configured update source resolves to, via UpdateTask's shared resolver.
  ---Answers where the next update is configured to come from, where `getEffectiveSource` answers which
  ---feed it will actually use once the fallbacks are applied.
  ---@param pkg table An installed package's config entry.
  ---@param modulesSection? table<string, table> The modules config section, for resolving a provider source.
  ---@return string? url The resolved feed URL, or nil when the package records no source or it can't be resolved.
  @resolveConfiguredSource = (pkg, modulesSection) ->
    src = ScriptUpdateRecord.getRecordedSource pkg
    return nil unless type(src) == "table"

    UpdateTask or= require "l0.DependencyControl.UpdateTask"
    UpdateTask.resolveSourceUrl src, pkg.feed, pkg.userFeed, modulesSection

  ---Returns the feed a package will update from: either a previously configured or used source,
  --- a user-set override (`userFeed`) or the package's own declared `feed`.
  ---@param pkg table An installed package's config entry.
  ---@param modulesSection table The modules config section, for resolving a provider source.
  ---@return string? url The feed the package updates from, or nil when it declares none.
  ---@return SourceFeedKind? kind What the URL was taken from: the configured source's own kind, or the kind the fallback stands for.
  @getEffectiveSource = (pkg, modulesSection) ->
    url = @.resolveConfiguredSource pkg, modulesSection
    if url
      source = ScriptUpdateRecord.getRecordedSource pkg
      return url, source.feedSource
    UpdateTask or= require "l0.DependencyControl.UpdateTask"
    return pkg.userFeed, UpdateTask.SourceFeedKind.UserFeed if pkg.userFeed
    return pkg.feed, UpdateTask.SourceFeedKind.SelfDeclared if pkg.feed
    nil

  ---Collects the feeds reachable from config, installed packages, and the official trust lists into a
  ---`url -> raw entry` map (provenance/packages/advertisedBy still as sets). Network-free.
  ---@private
  ---@return table<string, FeedInventoryEntry> inventoryEntriesByUrl
  __collectConfigFeeds: =>
    inventoryEntriesByUrl = {}
    tagPackage = (url, source, namespace) ->
      entry = addSource inventoryEntriesByUrl, url, source
      entry.packages[namespace] = true if entry

    for url in pairs @feedTrust\getOfficialTrustedFeeds!
      addSource inventoryEntriesByUrl, url, url == constants.DEPCTRL_FEED_URL and Provenance.OfficialDepCtrl or Provenance.OfficialKnown

    c = @config.c
    addSource inventoryEntriesByUrl, url, Provenance.UserExtra for url in *(c.feeds.extraFeeds or {})
    -- trustedFeeds grant trust but aren't a discovery source, so they get no provenance — just the flag
    for url in *(c.feeds.trustedFeeds or {})
      entry = ensureInventoryEntry inventoryEntriesByUrl, url
      entry.inTrustedFeeds = true if entry

    modulesSection = c[domain.ScriptTypeSection[domain.ScriptType.Module]] or {}
    for scriptType in *domain.ScriptType.values
      for namespace, pkg in pairs (c[domain.ScriptTypeSection[scriptType]] or {})
        continue unless type(pkg) == "table"
        tagPackage pkg.feed, Provenance.PackageDeclared, namespace
        tagPackage pkg.userFeed, Provenance.PackageOverride, namespace
        for dep in *(pkg.requiredModules or {})
          tagPackage dep.feed, Provenance.DependencyAdvertised, namespace if type(dep) == "table"
        effective = @@.getEffectiveSource pkg, modulesSection
        inventoryEntriesByUrl[effective].inUse = true if type(effective) == "string" and inventoryEntriesByUrl[effective]

    return inventoryEntriesByUrl

  ---Collapses each raw entry's provenance/package/advertisedBy sets to stable-ordered lists, attaches the
  ---trust status (and, for a blocked feed, the block entry that matches it), and stamps the feed's last
  ---fetch time from the persistent cache.
  ---@private
  ---@param inventoryEntriesByUrl table<string, FeedInventoryEntry> The raw entries to finalize.
  ---@return FeedInventoryEntry[] feeds
  __finalize: (inventoryEntriesByUrl) =>
    cache = @feedLoader.cache
    feeds = {}
    for url, entry in pairs inventoryEntriesByUrl
      entry.provenance = [p for p in *provenanceOrder when entry.provenance[p]]
      entry.packages = getSortedKeys entry.packages
      entry.advertisedBy = getSortedKeys entry.advertisedBy
      entry.trustStatus, entry.blockedBy = @feedTrust\getTrustStatus url
      meta = cache\getMeta url
      entry.lastFetchedAt = meta.cachedAt if meta and meta.cachedAt
      feeds[#feeds + 1] = entry
    feeds

  ---Gathers the known feeds from config, installed packages, and the official trust lists. Fetches nothing.
  ---@return FeedInventoryEntry[] feeds
  gather: => @__finalize @__collectConfigFeeds!

  ---Returns the namespaces of installed packages that effectively update from the given feed URL.
  ---@param feedUrl string The feed URL to check.
  ---@return string[] namespaces Sorted namespaces whose effective source is that feed.
  getPackagesSourcedFrom: (feedUrl) =>
    c = @config.c
    modulesSection = c[domain.ScriptTypeSection[domain.ScriptType.Module]] or {}
    matched = {}
    for scriptType in *domain.ScriptType.values
      for namespace, pkg in pairs (c[domain.ScriptTypeSection[scriptType]] or {})
        continue unless type(pkg) == "table"
        matched[#matched + 1] = namespace if @@.getEffectiveSource(pkg, modulesSection) == feedUrl
    table.sort matched
    matched

  ---Loads a feed's `knownFeeds` URLs through the shared feed loader, or nil when the feed can't be loaded.
  ---@private
  ---@param url string The feed URL to read.
  ---@return string[]? knownFeeds
  __loadKnownFeeds: (url) =>
    ok, feed = pcall @feedLoader.load, @feedLoader, url
    ok and feed and feed.data and feed.knownFeeds or nil

  ---Fetches feeds and discovers transitively-advertised ones by crawling the `knownFeeds` graph out from the
  ---config-derived feeds; untrusted expansion is bounded, so check `stats.truncated` for incomplete results.
  ---@return FeedInventoryEntry[] feeds The known feeds, enriched with what the crawl discovered.
  ---@return FeedCrawlStats stats What the crawl explored and where it stopped short.
  crawl: =>
    inventoryEntriesByUrl = @__collectConfigFeeds!
    stats = @__crawlKnownFeeds inventoryEntriesByUrl
    return @__finalize(inventoryEntriesByUrl), stats

  ---Breadth-first crawl of the `knownFeeds` graph, extending inventoryEntriesByUrl in place with the transitively-discovered
  ---feeds under the untrusted-expansion bounds. Each config-derived feed is its own budget subtree, so a
  ---malicious subtree can't starve the others.
  ---@private
  ---@param inventoryEntriesByUrl table<string, FeedInventoryEntry> The config-derived feeds to start from; extended in place.
  ---@return FeedCrawlStats stats
  __crawlKnownFeeds: (inventoryEntriesByUrl) =>
    c = @config.c
    limits = c.feeds.crawlLimits or {}
    defaults = @@defaultCrawlLimits
    maxDepth = limits[CrawlLimit.Depth] or defaults[CrawlLimit.Depth]
    maxPerRoot = limits[CrawlLimit.PerRoot] or defaults[CrawlLimit.PerRoot]
    maxPerFeed = limits[CrawlLimit.PerFeed] or defaults[CrawlLimit.PerFeed]
    stats = {fetched: 0, truncated: false, truncations: {}}

    record = (limit, feedUrl, root, depth, route, limitValue, drops) ->
      stats.truncated = true
      stats.truncations[#stats.truncations + 1] = {
        :limit, feed: feedUrl, :root, :depth, :route, :limitValue
        dropped: drops and drops.count or 0
        droppedUrls: drops and drops.sample or {}
      }

    -- accumulate dropped feed URLs into a bounded sample while keeping an exact count
    newDrops = -> {count: 0, sample: {}}
    drop = (drops, url) ->
      drops.count += 1
      drops.sample[#drops.sample + 1] = url if #drops.sample < maxDropSample

    -- BFS frontier seeded with the config-derived feeds; each is its own subtree root for budgeting
    queue, visited, untrustedPerRoot = {}, {}, {}
    for url, entry in pairs inventoryEntriesByUrl
      -- don't crawl feeds with no discovery provenance (e.g. orphaned `trustedFeeds` entries)
      continue unless next entry.provenance
      -- gate the root itself the same way its advertised children are: a blocked root is never
      -- fetched, and an untrusted root honors the fetch policy (recorded above, just not crawled)
      continue unless @feedTrust\shouldFetch url
      queue[#queue + 1] = {:url, root: url, depth: 0, route: {url}}
      visited[url] = true

    head = 1
    while head <= #queue
      {url: feedUrl, :root, :depth, :route} = queue[head]
      head += 1
      if depth >= maxDepth
        record CrawlLimit.Depth, feedUrl, root, depth, route, maxDepth
        continue
      knownFeeds = @__loadKnownFeeds feedUrl
      continue unless knownFeeds
      stats.fetched += 1
      inventoryEntriesByUrl[feedUrl].fetched = true

      perFeedUntrusted = 0
      perFeedDrops, perRootDrops = newDrops!, newDrops!
      for knownUrl in *knownFeeds
        continue unless type(knownUrl) == "string" and #knownUrl > 0
        continue if @feedTrust\isBlocked knownUrl
        trusted = @feedTrust\isTrusted knownUrl
        unless trusted
          -- bound how many untrusted feeds a single feed may contribute
          if perFeedUntrusted >= maxPerFeed
            drop perFeedDrops, knownUrl
            continue
          perFeedUntrusted += 1

        -- a feed advertised by DepCtrl's own feed is official-known, not merely transitively known
        prov = feedUrl == constants.DEPCTRL_FEED_URL and Provenance.OfficialKnown or Provenance.TransitiveKnown
        entry = addSource inventoryEntriesByUrl, knownUrl, prov
        entry.advertisedBy[feedUrl] = true
        continue if visited[knownUrl]

        if trusted
          visited[knownUrl] = true
          queue[#queue + 1] = {url: knownUrl, :root, depth: depth + 1, route: appended(route, knownUrl)}
        -- for an untrusted feed, within the per-root budget, ask the fetch policy before crawling in. shouldFetch
        -- prompts under the `prompt` policy (session-cached), and denies under `never` or with no
        -- prompter (headless), leaving the feed recorded above but not followed.
        else
          if (untrustedPerRoot[root] or 0) >= maxPerRoot
            drop perRootDrops, knownUrl
          elseif @feedTrust\shouldFetch knownUrl
            untrustedPerRoot[root] = (untrustedPerRoot[root] or 0) + 1
            visited[knownUrl] = true
            queue[#queue + 1] = {url: knownUrl, :root, depth: depth + 1, route: appended(route, knownUrl)}

      record CrawlLimit.PerFeed, feedUrl, root, depth, route, maxPerFeed, perFeedDrops if perFeedDrops.count > 0
      record CrawlLimit.PerRoot, feedUrl, root, depth, route, maxPerRoot, perRootDrops if perRootDrops.count > 0
    stats

return FeedInventory
