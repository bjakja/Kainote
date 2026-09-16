constants = require "l0.DependencyControl.Constants"
utils = require "l0.DependencyControl.utils"
Hash = require "l0.DependencyControl.hash"
Enum = require "l0.DependencyControl.Enum"
FeedInventory = require "l0.DependencyControl.FeedInventory"
FeedTrust = require "l0.DependencyControl.FeedTrust"

FeedAction = Enum "FeedAction", {
  Trust: "trust"
  Block: "block"
  Unblock: "unblock"
  Remove: "remove"
  OpenBrowser: "open-browser"
}

-- The DepCtrl Browser pre-renders one page per crawled feed under this path, slugged by the feed URL's hash.
browserFeedBase = "https://typesettingtools.github.io/depctrl-browser/feeds/"

---An action the Manage Feeds UI can offer for a feed.
---@alias FeedAction
---| "trust" # Trust: add the feed to the user's `trustedFeeds`
---| "block" # Block: add a block entry for the feed
---| "unblock" # Unblock: remove the user's exact block on the feed
---| "remove" # Remove: drop the user's `extraFeeds`/`trustedFeeds` contribution
---| "open-browser" # OpenBrowser: open the feed in the DepCtrl Browser

---A Manage Feeds list row: a feed's inventory fields plus what the UI needs to render and act on it.
---@class FeedManagerRow
---@field url string The feed URL.
---@field trustStatus FeedTrustStatus The feed's trust state.
---@field inTrustedFeeds? boolean True when the feed is in the user's `trustedFeeds` (a trust-only listing).
---@field provenance FeedProvenance[] The sources the feed was found through.
---@field packages string[] Namespaces of installed packages tied to this feed.
---@field advertisedBy string[] Feeds that advertise this feed (crawl only).
---@field blockedBy? BlockedFeedEntry The matching block entry when the feed is blocked.
---@field reachable? boolean Whether a crawl fetched the feed (nil when not crawled).
---@field lastFetchedAt? integer Unix time the feed was last fetched into the persistent cache (nil if never cached).
---@field inUse? boolean Whether the feed is the effective update source of an installed package.
---@field actions FeedAction[] The actions offered for this feed.
---@field browserUrl string The feed's DepCtrl Browser deep-link.
---@field removable boolean Whether the feed has a user contribution that Remove can drop.

---Turns reachable-feed data into what the Manage Feeds UI shows and offers: the actions available for a feed
---and its DepCtrl Browser link. Trust reads/writes go through `FeedTrust`.
---@class FeedManager
class FeedManager
  @FeedAction = FeedAction

  ---@param feedTrust FeedTrust The trust model that action execution delegates to.
  new: (@feedTrust) =>

  ---The DepCtrl Browser deep-link for a feed. Best-effort: it resolves only for feeds in the browser's crawl
  ---graph (official/known feeds).
  ---@param feedUrl string The exact feed URL, as stored (the hash is sensitive to casing and trailing slash).
  ---@return string url The Browser page URL.
  @getBrowserUrl = (feedUrl) ->
    "#{browserFeedBase}#{Hash.getDigest(Hash.HashType.Sha1, feedUrl)\sub 1, 7}/"

  ---The actions the Manage Feeds UI offers for a feed, from its trust state and provenance.
  ---@param entry FeedInventoryEntry The feed to compute the offered actions for.
  ---@return FeedAction[] actions In a stable order, state-changing actions first and Open Browser last.
  @getAvailableActions = (entry) ->
    {:Provenance} = FeedInventory
    {:TrustStatus} = FeedTrust
    actions = {}
    add = (action) -> actions[#actions + 1] = action

    add FeedAction.Trust if entry.trustStatus == TrustStatus.Untrusted

    add FeedAction.Block if entry.url != constants.DEPCTRL_FEED_URL and entry.trustStatus != TrustStatus.Blocked

    blk = entry.blockedBy
    add FeedAction.Unblock if entry.trustStatus == TrustStatus.Blocked and blk and not blk.isOfficial and
      blk.matchMode == FeedTrust.BlockMatchMode.Exact

    add FeedAction.Remove if utils.listIncludes(entry.provenance, Provenance.UserExtra) or entry.inTrustedFeeds

    add FeedAction.OpenBrowser
    actions

  ---Turns feed entries into dialog rows — each feed with its offered actions, Browser link, and `removable`
  ---flag. Sorted by URL.
  ---@param entries FeedInventoryEntry[] The reachable-feed entries to render (from `FeedInventory.gather`/`crawl`).
  ---@return FeedManagerRow[] rows The URL-sorted display rows.
  @buildRows = (entries) ->
    rows = {}
    for entry in *entries
      acts = FeedManager.getAvailableActions entry
      rows[#rows + 1] = {
        url: entry.url
        trustStatus: entry.trustStatus
        inTrustedFeeds: entry.inTrustedFeeds
        provenance: entry.provenance
        packages: entry.packages
        advertisedBy: entry.advertisedBy
        blockedBy: entry.blockedBy
        reachable: entry.fetched
        lastFetchedAt: entry.lastFetchedAt
        inUse: entry.inUse
        actions: acts
        browserUrl: FeedManager.getBrowserUrl entry.url
        removable: utils.listIncludes acts, FeedAction.Remove
      }
    table.sort rows, (a, b) -> a.url < b.url
    rows

  ---Applies a trust action to a feed, delegating to `FeedTrust`. Re-checks that the action is one the feed
  ---actually offers, so a guarded action (e.g. blocking the bootstrap feed) is refused even if asked for.
  ---Open Browser and any action the feed doesn't offer mutate nothing.
  ---@param action FeedAction The action to apply (one offered by `getAvailableActions`).
  ---@param entry FeedInventoryEntry The feed to act on.
  ---@param opts? { reason?: string } `reason` annotates a Block entry.
  ---@return boolean applied Whether trust state was changed.
  applyAction: (action, entry, opts = {}) =>
    return false unless utils.listIncludes @@.getAvailableActions(entry), action
    switch action
      when FeedAction.Trust
        @feedTrust\trust entry.url
      when FeedAction.Block
        @feedTrust\block entry.url, {matchMode: FeedTrust.BlockMatchMode.Exact, reason: opts.reason}
      when FeedAction.Unblock
        @feedTrust\unblock entry.url
      when FeedAction.Remove
        @feedTrust\removeExtraFeed entry.url
        @feedTrust\untrust entry.url
      else
        return false
    true

return FeedManager
