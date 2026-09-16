-- FeedManager tests: the actions the Manage Feeds UI offers per feed, action execution (via a stub FeedTrust),
-- the DepCtrl Browser deep-link, and row assembly. No network.
() ->
  FeedManager = require "l0.DependencyControl.FeedManager"
  FeedInventory = require "l0.DependencyControl.FeedInventory"
  FeedTrust = require "l0.DependencyControl.FeedTrust"
  constants = require "l0.DependencyControl.Constants"

  Provenance = FeedInventory.Provenance
  TrustStatus = FeedTrust.TrustStatus
  FeedAction = FeedManager.FeedAction
  BlockMatchMode = FeedTrust.BlockMatchMode

  actionsFor = (entry) -> FeedManager.getAvailableActions entry

  -- a fake FeedTrust that records the mutator calls the manager delegates, in order
  makeManager = ->
    calls = {}
    feedTrust = {
      trust: (url) => calls[#calls + 1] = {"trust", url}
      untrust: (url) => calls[#calls + 1] = {"untrust", url}
      unblock: (url) => calls[#calls + 1] = {"unblock", url}
      removeExtraFeed: (url) => calls[#calls + 1] = {"removeExtraFeed", url}
      block: (url, opts) => calls[#calls + 1] = {"block", url, opts}
    }
    FeedManager(feedTrust), calls

  {
    _description: "FeedManager: per-feed available actions, the DepCtrl Browser link, and action execution."

    -- an untrusted feed can be trusted, blocked, or viewed
    availableActions_untrusted: (ut) ->
      entry = {url: "feed://x", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Untrusted}
      ut\assertEquals actionsFor(entry), {FeedAction.Trust, FeedAction.Block, FeedAction.OpenBrowser}

    -- a feed in a user list (here extraFeeds) can be blocked, removed, or viewed (not re-trusted)
    availableActions_trustedUser: (ut) ->
      entry = {url: "feed://x", provenance: {Provenance.UserExtra}, trustStatus: TrustStatus.TrustedUser}
      ut\assertEquals actionsFor(entry), {FeedAction.Block, FeedAction.Remove, FeedAction.OpenBrowser}

    -- a trustedFeeds-only feed carries no provenance but is still removable via the inTrustedFeeds flag
    availableActions_trustedFeedsOnly: (ut) ->
      entry = {url: "feed://t", provenance: {}, inTrustedFeeds: true, trustStatus: TrustStatus.TrustedUser}
      ut\assertEquals actionsFor(entry), {FeedAction.Block, FeedAction.Remove, FeedAction.OpenBrowser}

    -- the DepCtrl bootstrap feed can only be viewed: blocking it would collapse trust, and it isn't user-owned
    availableActions_bootstrapFeed: (ut) ->
      entry = {url: constants.DEPCTRL_FEED_URL, provenance: {Provenance.OfficialDepCtrl}, trustStatus: TrustStatus.TrustedOfficial}
      ut\assertEquals actionsFor(entry), {FeedAction.OpenBrowser}

    -- an official-known trusted feed can be blocked (the only way to stop trusting it) or viewed, not removed
    availableActions_trustedOfficial: (ut) ->
      entry = {url: "feed://known", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.TrustedOfficial}
      ut\assertEquals actionsFor(entry), {FeedAction.Block, FeedAction.OpenBrowser}

    -- a user's exact block can be lifted from the list view
    availableActions_blockedUserExact: (ut) ->
      entry = {url: "feed://b", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Blocked, blockedBy: {url: "feed://b", matchMode: BlockMatchMode.Exact, isOfficial: false}}
      ut\assertEquals actionsFor(entry), {FeedAction.Unblock, FeedAction.OpenBrowser}

    -- a prefix (or official) block isn't liftable per-feed from the list view -> only Open Browser
    availableActions_blockedPrefix: (ut) ->
      entry = {url: "feed://b", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Blocked, blockedBy: {url: "feed://", matchMode: BlockMatchMode.Prefix, isOfficial: true}}
      ut\assertEquals actionsFor(entry), {FeedAction.OpenBrowser}

    -- a feed that's both user-listed and under the user's exact block offers Unblock and Remove
    availableActions_blockedAndUserListed: (ut) ->
      entry = {url: "feed://b", provenance: {Provenance.UserExtra}, trustStatus: TrustStatus.Blocked, blockedBy: {url: "feed://b", matchMode: BlockMatchMode.Exact, isOfficial: false}}
      ut\assertEquals actionsFor(entry), {FeedAction.Unblock, FeedAction.Remove, FeedAction.OpenBrowser}

    -- the browser link slugs the feed URL by the first 7 hex chars of its SHA-1
    getBrowserUrl_slug: (ut) ->
      ut\assertEquals FeedManager.getBrowserUrl("abc"),
        "https://typesettingtools.github.io/depctrl-browser/feeds/a9993e3/"

    -- Trust delegates to feedTrust\trust and reports a mutation
    applyAction_trust: (ut) ->
      mgr, calls = makeManager!
      entry = {url: "feed://x", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Untrusted}
      ut\assertTrue mgr\applyAction FeedAction.Trust, entry
      ut\assertEquals calls, {{"trust", "feed://x"}}

    -- Block adds an exact block carrying the given reason
    applyAction_block: (ut) ->
      mgr, calls = makeManager!
      entry = {url: "feed://x", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Untrusted}
      mgr\applyAction FeedAction.Block, entry, {reason: "malware"}
      ut\assertEquals calls[1][1], "block"
      ut\assertEquals calls[1][2], "feed://x"
      ut\assertEquals calls[1][3].matchMode, BlockMatchMode.Exact
      ut\assertEquals calls[1][3].reason, "malware"

    -- Remove clears both user lists (offered here via the inTrustedFeeds flag alone)
    applyAction_removeClearsBothLists: (ut) ->
      mgr, calls = makeManager!
      entry = {url: "feed://x", provenance: {}, inTrustedFeeds: true, trustStatus: TrustStatus.TrustedUser}
      mgr\applyAction FeedAction.Remove, entry
      ut\assertEquals calls, {{"removeExtraFeed", "feed://x"}, {"untrust", "feed://x"}}

    -- Unblock lifts the user's exact block
    applyAction_unblock: (ut) ->
      mgr, calls = makeManager!
      entry = {url: "feed://b", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Blocked, blockedBy: {url: "feed://b", matchMode: BlockMatchMode.Exact, isOfficial: false}}
      mgr\applyAction FeedAction.Unblock, entry
      ut\assertEquals calls, {{"unblock", "feed://b"}}

    -- an action the feed doesn't offer (blocking the bootstrap feed) is refused and mutates nothing
    applyAction_refusesGuarded: (ut) ->
      mgr, calls = makeManager!
      entry = {url: constants.DEPCTRL_FEED_URL, provenance: {Provenance.OfficialDepCtrl}, trustStatus: TrustStatus.TrustedOfficial}
      ut\assertFalse mgr\applyAction FeedAction.Block, entry
      ut\assertEquals #calls, 0

    -- Open Browser performs no mutation
    applyAction_openBrowserNoOp: (ut) ->
      mgr, calls = makeManager!
      entry = {url: "feed://x", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Untrusted}
      ut\assertFalse mgr\applyAction FeedAction.OpenBrowser, entry
      ut\assertEquals #calls, 0

    -- rows attach actions, the browser link, removability, reachability and last-fetch time, sorted by URL
    buildRows_assemblesAndSorts: (ut) ->
      entries = {
        {url: "feed://b", provenance: {Provenance.OfficialKnown}, trustStatus: TrustStatus.Untrusted, fetched: true, lastFetchedAt: 1700000000}
        {url: "feed://a", provenance: {Provenance.UserExtra}, trustStatus: TrustStatus.TrustedUser, inUse: true}
      }
      rows = FeedManager.buildRows entries
      ut\assertEquals rows[1].url, "feed://a" -- URL-sorted, independent of input order
      ut\assertEquals rows[2].url, "feed://b"
      ut\assertEquals rows[1].actions, {FeedAction.Block, FeedAction.Remove, FeedAction.OpenBrowser}
      ut\assertTrue rows[1].removable
      ut\assertEquals rows[1].browserUrl, FeedManager.getBrowserUrl "feed://a"
      ut\assertTrue rows[2].reachable -- surfaced from entry.fetched
      ut\assertNil rows[1].reachable
      ut\assertTrue rows[1].inUse -- surfaced from entry.inUse
      ut\assertNil rows[2].inUse
      ut\assertEquals rows[2].lastFetchedAt, 1700000000 -- surfaced from entry.lastFetchedAt
      ut\assertNil rows[1].lastFetchedAt

    -- regression: a trust-only feed (no provenance) keeps inTrustedFeeds on its row, so the row round-trips
    -- back through applyAction and Remove actually fires — previously the row dropped it and Remove no-op'd
    buildRows_trustOnlyFeedRoundTripsRemove: (ut) ->
      mgr, calls = makeManager!
      rows = FeedManager.buildRows {
        {url: "feed://t", provenance: {}, inTrustedFeeds: true, trustStatus: TrustStatus.TrustedUser}
      }
      ut\assertTrue rows[1].removable
      ut\assertTrue rows[1].inTrustedFeeds
      ut\assertTrue mgr\applyAction FeedAction.Remove, rows[1]
      ut\assertEquals calls, {{"removeExtraFeed", "feed://t"}, {"untrust", "feed://t"}}

    _order: {
      "availableActions_untrusted", "availableActions_trustedUser", "availableActions_trustedFeedsOnly"
      "availableActions_bootstrapFeed"
      "availableActions_trustedOfficial", "availableActions_blockedUserExact", "availableActions_blockedPrefix"
      "availableActions_blockedAndUserListed", "getBrowserUrl_slug"
      "applyAction_trust", "applyAction_block", "applyAction_removeClearsBothLists", "applyAction_unblock"
      "applyAction_refusesGuarded", "applyAction_openBrowserNoOp"
      "buildRows_assemblesAndSorts", "buildRows_trustOnlyFeedRoundTripsRemove"
    }
  }
