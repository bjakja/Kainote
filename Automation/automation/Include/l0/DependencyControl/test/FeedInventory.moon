-- FeedInventory tests: gathering reachable feeds from config + official trust lists, with per-feed
-- provenance (source tags) and trust status. Network-free; feedTrust and config are stubbed.
-- Called from test.moon as: (controls\requireTest "FeedInventory")!
() ->
  FeedInventory = require "l0.DependencyControl.FeedInventory"
  FeedTrust = require "l0.DependencyControl.FeedTrust"
  constants = require "l0.DependencyControl.Constants"
  UpdateTask = require "l0.DependencyControl.UpdateTask"

  Provenance = FeedInventory.Provenance
  TrustStatus = FeedTrust.TrustStatus
  CrawlLimit = FeedInventory.CrawlLimit
  SourceFeedKind = UpdateTask.SourceFeedKind

  -- a fake FeedTrust; opts: official (set), trusted (set), blocked (set), status (url -> FeedTrustStatus),
  -- blockEntries (url -> entry), fetchUntrustedFeeds (policy string), prompter (url -> bool for the prompt
  -- policy). getTrustStatus returns (status, blockEntry). isTrusted/isBlocked/shouldFetch back the crawl's
  -- follow decision; shouldFetch mirrors the real gate (Allow -> true, Deny -> false, Prompt -> prompter).
  makeFeedTrust = (opts = {}) ->
    {
      getOfficialTrustedFeeds: => opts.official or {}
      isTrusted: (url) => (opts.trusted or {})[url] and true or false
      isBlocked: (url) => (opts.blocked or {})[url] and true or false
      getTrustStatus: (url) =>
        return TrustStatus.Blocked, (opts.blockEntries or {})[url] if (opts.blocked or {})[url]
        (opts.status or {})[url] or TrustStatus.Untrusted
      getFetchDecision: (url) =>
        return FeedTrust.FetchDecision.Deny if (opts.blocked or {})[url]
        return FeedTrust.FetchDecision.Allow if (opts.trusted or {})[url]
        switch opts.fetchUntrustedFeeds
          when "never" then FeedTrust.FetchDecision.Deny
          when "prompt" then FeedTrust.FetchDecision.Prompt
          else FeedTrust.FetchDecision.Allow
      shouldFetch: (url) =>
        switch @getFetchDecision url
          when FeedTrust.FetchDecision.Allow then true
          when FeedTrust.FetchDecision.Deny then false
          else opts.prompter and opts.prompter(url) and true or false
    }

  -- a fake feed loader: `.cache\getMeta` reports last-fetch metadata and `\load url` returns a fake feed.
  -- Both default to "nothing known"; override per test via opts.meta (url -> meta) and opts.load (url -> feed).
  makeFeedLoader = (opts = {}) ->
    {
      cache: {getMeta: (_, key) -> opts.meta and opts.meta[key]}
      load: (_, url) -> opts.load and opts.load url
    }

  -- section a flat test config into the live sectioned layout (feeds/updates/paths + the macros/modules registries)
  makeInventory = (configC = {}, feedTrustOpts, feedLoader) ->
    sectioned = {
      feeds: {extraFeeds: configC.extraFeeds, trustedFeeds: configC.trustedFeeds, crawlLimits: configC.feedCrawlLimits}
      updates: {blockPrivateHosts: configC.updaterBlockPrivateHosts}
      paths: {cache: "?user/cache"}
      macros: configC.macros
      modules: configC.modules
    }
    FeedInventory {c: sectioned}, (makeFeedTrust feedTrustOpts), (feedLoader or makeFeedLoader!)
  gatherFeeds = (configC, feedTrustOpts) -> makeInventory(configC, feedTrustOpts)\gather!

  -- a fake FeedLoader over a fixed {url -> knownFeeds list} map: load(url) yields a feed exposing those
  -- knownFeeds; an unmapped url yields a feed with no `.data`, i.e. unreachable (like a failed fetch)
  crawlLoader = (map) ->
    makeFeedLoader {
      load: (url) ->
        known = map[url]
        {data: known, knownFeeds: known or {}}
    }

  -- index a gathered feed list by url for assertions
  byUrl = (feeds) -> {f.url, f for f in *feeds}

  -- the first crawl truncation recorded against a given feed
  truncationFor = (stats, feedUrl) ->
    matches = [t for t in *stats.truncations when t.feed == feedUrl]
    matches[1]

  {
    _description: "FeedInventory: reachable-feed gathering with provenance and trust status."

    -- each config/official/package source contributes a feed tagged with the matching provenance
    gather_tagsEachSource: (ut) ->
      m = byUrl gatherFeeds {
        extraFeeds: {"feed://extra"}
        trustedFeeds: {"feed://trusted"}
        macros: {
          "a.b": {
            feed: "feed://declared"
            userFeed: "feed://override"
            requiredModules: {{moduleName: "x", feed: "feed://advertised"}}
          }
        }
        modules: {}
      }, official: {[constants.DEPCTRL_FEED_URL]: true, "feed://known": true}
      ut\assertEquals m[constants.DEPCTRL_FEED_URL].provenance, {Provenance.OfficialDepCtrl}
      ut\assertEquals m["feed://known"].provenance, {Provenance.OfficialKnown}
      ut\assertEquals m["feed://extra"].provenance, {Provenance.UserExtra}
      -- a trustedFeeds-only feed isn't a discovery source: no provenance, just the inTrustedFeeds trust flag
      ut\assertEquals m["feed://trusted"].provenance, {}
      ut\assertTrue m["feed://trusted"].inTrustedFeeds
      ut\assertEquals m["feed://declared"].provenance, {Provenance.PackageDeclared}
      ut\assertEquals m["feed://override"].provenance, {Provenance.PackageOverride}
      ut\assertEquals m["feed://advertised"].provenance, {Provenance.DependencyAdvertised}
      -- package-sourced feeds record the contributing namespace; others have none
      ut\assertEquals m["feed://declared"].packages, {"a.b"}
      ut\assertEquals m["feed://override"].packages, {"a.b"}
      ut\assertEquals m["feed://advertised"].packages, {"a.b"}
      ut\assertEquals m["feed://extra"].packages, {}

    -- a feed reached via several sources collects every provenance, in enum order; trustedFeeds membership
    -- adds no provenance but sets the inTrustedFeeds flag
    gather_mergesProvenance: (ut) ->
      m = byUrl gatherFeeds {
        extraFeeds: {"feed://x"}
        trustedFeeds: {"feed://x"}
        macros: {"a.b": {feed: "feed://x"}}
      }, official: {"feed://x": true}
      ut\assertEquals m["feed://x"].provenance,
        {Provenance.OfficialKnown, Provenance.UserExtra, Provenance.PackageDeclared}
      ut\assertTrue m["feed://x"].inTrustedFeeds

    -- gather surfaces the status FeedTrust reports for each feed (the classification itself lives in the
    -- FeedTrust suite); a non-blocked feed has no blockedBy
    gather_surfacesTrustStatus: (ut) ->
      m = byUrl gatherFeeds {extraFeeds: {"feed://o", "feed://u"}},
        status: {"feed://o": TrustStatus.TrustedOfficial, "feed://u": TrustStatus.TrustedUser}
      ut\assertEquals m["feed://o"].trustStatus, TrustStatus.TrustedOfficial
      ut\assertEquals m["feed://u"].trustStatus, TrustStatus.TrustedUser
      ut\assertNil m["feed://o"].blockedBy

    -- a blocked feed carries the block entry that matched it (the second value from getTrustStatus)
    gather_blockedByEntry: (ut) ->
      blockEntry = {url: "feed://bad", matchMode: "prefix", reason: "malware", isOfficial: true}
      m = byUrl gatherFeeds {extraFeeds: {"feed://bad"}},
        blocked: {"feed://bad": true}, blockEntries: {"feed://bad": blockEntry}
      ut\assertEquals m["feed://bad"].trustStatus, TrustStatus.Blocked
      ut\assertEquals m["feed://bad"].blockedBy, blockEntry

    -- a feed is marked inUse when it's a package's effective source (override if set, else declared feed)
    gather_marksInUse: (ut) ->
      m = byUrl gatherFeeds {
        macros: {
          "a.plain": {feed: "feed://plain"}
          "a.overridden": {feed: "feed://declared", userFeed: "feed://override"}
        }
      }, {}
      ut\assertTrue m["feed://plain"].inUse -- declared with no override -> effective source
      ut\assertTrue m["feed://override"].inUse -- the override is the effective source
      ut\assertNil m["feed://declared"].inUse -- declared but overridden -> not the effective source

    -- getPackagesSourcedFrom / getEffectiveSource: a package's remembered currentSource (resolved per kind), else its
    -- override, else its declared feed
    -- getEffectiveSource's second return names the rung the URL came from, so a caller showing or
    -- seeding a source kind reads the same fallback order the URL followed instead of restating it
    getEffectiveSource_reportsKind: (ut) ->
      recorded = {feed: 'feed://d', userFeed: 'feed://u', currentSource: {feedSource: SourceFeedKind.UserFeed}}
      url, kind = FeedInventory.getEffectiveSource recorded, {}
      ut\assertEquals url, 'feed://u'
      ut\assertEquals kind, SourceFeedKind.UserFeed
      overridden = {feed: 'feed://d', userFeed: 'feed://u'}
      url, kind = FeedInventory.getEffectiveSource overridden, {}
      ut\assertEquals url, 'feed://u'
      ut\assertEquals kind, SourceFeedKind.UserFeed -- no record: the override is the first pick
      plain = {feed: 'feed://d'}
      url, kind = FeedInventory.getEffectiveSource plain, {}
      ut\assertEquals url, 'feed://d'
      ut\assertEquals kind, SourceFeedKind.SelfDeclared
      url, kind = FeedInventory.getEffectiveSource {}, {}
      ut\assertNil url
      ut\assertNil kind

    getPackagesSourcedFrom_getEffectiveSource: (ut) ->
      inv = makeInventory {
        macros: {
          "a.self": {feed: "feed://self", currentSource: {feedSource: SourceFeedKind.SelfDeclared}}
          "a.user": {feed: "feed://declared", userFeed: "feed://user", currentSource: {feedSource: SourceFeedKind.UserFeed}}
          "a.other": {feed: "feed://x", currentSource: {feedSource: SourceFeedKind.Other, feedUrl: "feed://literal"}}
          "a.plain": {feed: "feed://plain"} -- no currentSource -> falls back to feed
          "a.override": {feed: "feed://d2", userFeed: "feed://o2"} -- no currentSource -> falls back to userFeed
        }
        modules: {
          "m.prov": {feed: "feed://provider"}
          "m.viaProv": {feed: "feed://y", currentSource: {feedSource: SourceFeedKind.Provider, provider: {namespace: "m.prov"}}}
        }
      }, {}
      ut\assertEquals inv\getPackagesSourcedFrom("feed://self"), {"a.self"} -- self-declared -> own feed
      ut\assertEquals inv\getPackagesSourcedFrom("feed://user"), {"a.user"} -- user-feed kind -> the override
      ut\assertEquals inv\getPackagesSourcedFrom("feed://declared"), {} -- declared, but sourced elsewhere
      ut\assertEquals inv\getPackagesSourcedFrom("feed://literal"), {"a.other"} -- other -> literal feedUrl
      ut\assertEquals inv\getPackagesSourcedFrom("feed://plain"), {"a.plain"} -- fallback: no currentSource -> feed
      ut\assertEquals inv\getPackagesSourcedFrom("feed://o2"), {"a.override"} -- fallback: no currentSource -> userFeed
      ut\assertEquals inv\getPackagesSourcedFrom("feed://d2"), {} -- declared but overridden -> not effective
      -- provider resolves to m.prov's feed; m.prov itself (no currentSource) also falls back to that same feed
      ut\assertEquals inv\getPackagesSourcedFrom("feed://provider"), {"m.prov", "m.viaProv"}

    -- an empty config with no official feeds yields no entries
    gather_empty: (ut) ->
      ut\assertEquals #gatherFeeds({}, {}), 0

    -- crawl discovers transitively-advertised feeds, tagging them TransitiveKnown and recording the advertiser
    crawl_discoversTransitively: (ut) ->
      loader = crawlLoader {"root": {"mid", "leaf"}, "mid": {"deep"}}
      inv = makeInventory {extraFeeds: {"root"}}, {trusted: {"root": true, "mid": true}}, loader
      feeds, stats = inv\crawl!
      m = byUrl feeds
      ut\assertEquals m["mid"].provenance, {Provenance.TransitiveKnown}
      ut\assertEquals m["leaf"].provenance, {Provenance.TransitiveKnown}
      ut\assertEquals m["deep"].provenance, {Provenance.TransitiveKnown}
      ut\assertEquals m["mid"].advertisedBy, {"root"}
      ut\assertEquals m["deep"].advertisedBy, {"mid"}
      ut\assertFalse stats.truncated -- nothing was capped

    -- with fetchUntrustedFeeds="never", an untrusted feed is recorded (advertised) but not fetched/expanded
    crawl_neverDoesNotFetchUntrusted: (ut) ->
      loader = crawlLoader {"root": {"untrusted"}, "untrusted": {"deep"}}
      inv = makeInventory {extraFeeds: {"root"}}, {trusted: {"root": true}, fetchUntrustedFeeds: "never"}, loader
      m = byUrl inv\crawl!
      ut\assertEquals m["untrusted"].provenance, {Provenance.TransitiveKnown} -- recorded (advertised)
      ut\assertNil m["deep"] -- not fetched -> not discovered

    -- a blocked root is gated like any other feed: recorded but never fetched, so what it advertises
    -- isn't discovered (a blocked feed is always denied, even when it's a configured discovery root)
    crawl_blockedRootNotFetched: (ut) ->
      loader = crawlLoader {"blockedRoot": {"child"}}
      inv = makeInventory {extraFeeds: {"blockedRoot"}}, {blocked: {"blockedRoot": true}}, loader
      m = byUrl inv\crawl!
      ut\assertNotNil m["blockedRoot"] -- still listed (user extra feed)
      ut\assertNil m["child"] -- root not fetched -> its advertised feed not discovered

    -- with fetchUntrustedFeeds="prompt", an untrusted feed is followed only when the prompter confirms it
    crawl_promptFollowsOnlyConfirmedUntrusted: (ut) ->
      asked = {}
      prompter = (url) ->
        asked[#asked + 1] = url
        url == "yes"
      -- deepYes is trusted so following the confirmed "yes" doesn't itself prompt again
      loader = crawlLoader {
        "root": {"yes", "no"}
        "yes": {"deepYes"}, "no": {"deepNo"}
      }
      inv = makeInventory {extraFeeds: {"root"}}, {trusted: {"root": true, "deepYes": true}, fetchUntrustedFeeds: "prompt", :prompter}, loader
      m = byUrl inv\crawl!
      ut\assertNotNil m["yes"] -- both untrusted feeds are recorded (advertised)
      ut\assertNotNil m["no"]
      ut\assertNotNil m["deepYes"] -- confirmed -> followed -> its child is discovered
      ut\assertNil m["deepNo"] -- declined -> not followed
      ut\assertEquals #asked, 2 -- only the two untrusted feeds were asked, once each

    -- the per-subtree budget caps how many untrusted feeds are fetched from one root
    crawl_boundsUntrustedPerRoot: (ut) ->
      loader = crawlLoader {
        "root": {"u1", "u2", "u3"}
        "u1": {"d1"}, "u2": {"d2"}, "u3": {"d3"}
      }
      inv = makeInventory {extraFeeds: {"root"}, feedCrawlLimits: {[CrawlLimit.PerRoot]: 2}}, {trusted: {"root": true}}, loader
      feeds, stats = inv\crawl!
      m = byUrl feeds
      -- all three untrusted feeds are recorded; only two are fetched, so only two children are discovered
      reached = 0
      for d in *{"d1", "d2", "d3"}
        reached += 1 if m[d]
      ut\assertEquals reached, 2
      -- the third untrusted feed the root advertised was dropped by the per-root budget
      rootDrop = truncationFor stats, "root"
      ut\assertEquals rootDrop.limit, CrawlLimit.PerRoot
      ut\assertEquals rootDrop.limitValue, 2
      ut\assertEquals rootDrop.droppedUrls, {"u3"}

    -- the per-feed cap bounds how many untrusted feeds a single feed contributes
    crawl_boundsUntrustedPerFeed: (ut) ->
      loader = crawlLoader {"root": {"u1", "u2"}}
      inv = makeInventory {extraFeeds: {"root"}, feedCrawlLimits: {[CrawlLimit.PerFeed]: 1}}, {trusted: {"root": true}}, loader
      feeds, stats = inv\crawl!
      m = byUrl feeds
      ut\assertNotNil m["u1"]
      ut\assertNil m["u2"]
      ut\assertEquals #stats.truncations, 1
      t = stats.truncations[1]
      ut\assertEquals t.limit, CrawlLimit.PerFeed
      ut\assertEquals t.feed, "root"
      ut\assertEquals t.route, {"root"}
      ut\assertEquals t.dropped, 1
      ut\assertEquals t.droppedUrls, {"u2"}

    -- a feed at the depth cap is left unfetched and reported with its route from the root
    crawl_reportsDepthTruncation: (ut) ->
      loader = crawlLoader {"root": {"a"}, "a": {"b"}}
      inv = makeInventory {extraFeeds: {"root"}, feedCrawlLimits: {[CrawlLimit.Depth]: 1}}, {trusted: {"root": true, "a": true}}, loader
      feeds, stats = inv\crawl!
      m = byUrl feeds
      ut\assertNil m["b"] -- "a" sat at the depth cap, so its knownFeeds were never read
      t = truncationFor stats, "a"
      ut\assertEquals t.limit, CrawlLimit.Depth
      ut\assertEquals t.route, {"root", "a"}
      ut\assertEquals t.dropped, 0

    -- crawl marks each feed it successfully fetched; an advertised feed it can't load stays unmarked
    crawl_marksFetched: (ut) ->
      loader = crawlLoader {"root": {"mid", "dead"}, "mid": {}}
      inv = makeInventory {extraFeeds: {"root"}}, {trusted: {"root": true, "mid": true}}, loader
      m = byUrl inv\crawl!
      ut\assertTrue m["root"].fetched -- config root, loaded
      ut\assertTrue m["mid"].fetched -- trusted, loaded (even with empty knownFeeds)
      ut\assertNil m["dead"].fetched -- advertised but the loader returns nil -> unreachable

    -- a feed known only through trustedFeeds is trust-only: visible in the inventory but not a crawl root,
    -- so its knownFeeds are never expanded
    crawl_trustedFeedsNotCrawlRoot: (ut) ->
      loader = crawlLoader {"tf": {"child"}}
      inv = makeInventory {trustedFeeds: {"tf"}}, {trusted: {"tf": true}}, loader
      m = byUrl inv\crawl!
      ut\assertTrue m["tf"].inTrustedFeeds
      ut\assertEquals m["tf"].provenance, {} -- not a discovery source
      ut\assertNil m["tf"].fetched -- never fetched (not a root)
      ut\assertNil m["child"] -- so its advertised feed isn't discovered

    -- gather stamps each feed with its last-fetch time from the persistent cache; an uncached feed has none
    gather_stampsLastFetchedFromCache: (ut) ->
      loader = makeFeedLoader {meta: {"feed://cached": {key: "feed://cached", cachedAt: 1700000000, latestFile: "x.json"}}}
      m = byUrl (makeInventory {extraFeeds: {"feed://cached", "feed://uncached"}}, {}, loader)\gather!
      ut\assertEquals m["feed://cached"].lastFetchedAt, 1700000000
      ut\assertNil m["feed://uncached"].lastFetchedAt

    -- a feed advertised by DepCtrl's own feed is official-known, not transitive-known
    crawl_depCtrlFeedTagsOfficialKnown: (ut) ->
      loader = crawlLoader {[constants.DEPCTRL_FEED_URL]: {"known"}}
      inv = makeInventory {}, {
        official: {[constants.DEPCTRL_FEED_URL]: true}
        trusted: {[constants.DEPCTRL_FEED_URL]: true, "known": true}
      }, loader
      m = byUrl inv\crawl!
      ut\assertEquals m["known"].provenance, {Provenance.OfficialKnown}

    _order: {
      "gather_tagsEachSource", "gather_mergesProvenance", "gather_surfacesTrustStatus", "gather_blockedByEntry"
      "gather_marksInUse", "getEffectiveSource_reportsKind", "getPackagesSourcedFrom_getEffectiveSource", "gather_empty"
      "crawl_discoversTransitively", "crawl_neverDoesNotFetchUntrusted", "crawl_blockedRootNotFetched"
      "crawl_promptFollowsOnlyConfirmedUntrusted"
      "crawl_boundsUntrustedPerRoot", "crawl_boundsUntrustedPerFeed", "crawl_reportsDepthTruncation"
      "crawl_marksFetched", "crawl_trustedFeedsNotCrawlRoot"
      "gather_stampsLastFetchedFromCache", "crawl_depCtrlFeedTagsOfficialKnown"
    }
  }
