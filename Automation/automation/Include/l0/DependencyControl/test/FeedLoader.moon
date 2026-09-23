-- FeedLoader tests: builds the shared feed cache from config and hands out UpdateFeed instances wired to
-- it. Network-free — feeds are constructed with autoLoad off, so nothing is fetched.
-- Called from test.moon as: (require "…test.FeedLoader") basePath, DepCtrl
(basePath, DepCtrl) ->
  FeedLoader = require "l0.DependencyControl.FeedLoader"
  fileOps = require "l0.DependencyControl.file-ops"
  constants = require "l0.DependencyControl.Constants"

  url = "https://example.com/feed.json"

  -- a FeedLoader over a temp cache base; opts override the config's feed settings
  make = (opts = {}) ->
    config = {c: {
      paths: {cache: fileOps.joinPath basePath, "feedLoader"}
      feeds: {cacheMaxAge: opts.cacheMaxAge or 4242, maxFeedSize: opts.maxFeedSize, feedFetchTimeout: opts.feedFetchTimeout}
      updates: {blockPrivateHosts: opts.blockPrivateHosts}
    }}
    FeedLoader config, DepCtrl.logger

  {
    _description: "FeedLoader: shared feed cache construction and UpdateFeed wiring."

    -- the cache lives under DepCtrl's namespace in a `feeds` subdir of the configured cache base
    new_opensFeedCacheUnderNamespace: (ut) ->
      loader = make!
      ut\assertNotNil loader.cache
      ut\assertContains loader.cache.cacheDir, constants.DEPCTRL_NAMESPACE
      ut\assertMatches loader.cache.cacheDir, "feeds$"

    -- load injects the shared cache and the configured fetch policy
    load_wiresSharedCacheAndPolicy: (ut) ->
      loader = make {blockPrivateHosts: true}
      feed = loader\load url, {autoLoad: false}
      ut\assertIs feed.config.cache, loader.cache
      ut\assertTrue feed.config.blockPrivateHosts
      ut\assertNil feed.data -- autoLoad off: constructed but not fetched

    -- feed-fetch caps fall back to FeedLoader's defaults when the config leaves them unset
    load_appliesFeedCapDefaults: (ut) ->
      loader = make!
      feed = loader\load url, {autoLoad: false}
      ut\assertEquals feed.config.maxFeedSize, FeedLoader.defaultMaxFeedSize
      ut\assertEquals feed.config.feedFetchTimeout, FeedLoader.defaultFeedFetchTimeout

    -- load reads the fetch policy from the config at call time, so a settings change applies to the next fetch
    load_readsFetchPolicyAtCallTime: (ut) ->
      loader = make {blockPrivateHosts: true}
      loader.config.c.updates.blockPrivateHosts = false
      loader.config.c.feeds.maxFeedSize = 1234
      feed = loader\load url, {autoLoad: false}
      ut\assertFalse feed.config.blockPrivateHosts
      ut\assertEquals feed.config.maxFeedSize, 1234

    -- a configured feed-fetch cap overrides the default
    load_usesConfiguredFeedCaps: (ut) ->
      loader = make {maxFeedSize: 1000, feedFetchTimeout: 5}
      feed = loader\load url, {autoLoad: false}
      ut\assertEquals feed.config.maxFeedSize, 1000
      ut\assertEquals feed.config.feedFetchTimeout, 5

    _order: {
      "new_opensFeedCacheUnderNamespace", "load_wiresSharedCacheAndPolicy",
      "load_appliesFeedCapDefaults", "load_readsFetchPolicyAtCallTime", "load_usesConfiguredFeedCaps"
    }
  }
