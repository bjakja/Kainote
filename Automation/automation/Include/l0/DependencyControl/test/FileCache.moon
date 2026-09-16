-- FileCache tests: key-addressed persistent JSON snapshots indexed by a deterministic meta file, with a
-- freshness window, human-readable snapshot names, stale-fallback lookup, and retention trimming. Instances
-- live under <basePath>/<namespace>/<name>. Uses a temp base + an injected clock; no network.
-- Called from test.moon as: (require "…test.FileCache") basePath
(basePath) ->
  FileCache = require "l0.DependencyControl.FileCache"
  fileOps = require "l0.DependencyControl.file-ops"

  -- a fresh cache in its own base subdir with a controllable clock; returns (cache, clock)
  makeCache = (name, opts = {}) ->
    clock = {t: opts.t or 1000}
    opts.now = -> clock.t
    FileCache(fileOps.joinPath(basePath, "filecache", name), "testNamespace", "test", opts), clock

  readFile = fileOps.readFile

  {
    _description: "FileCache: persistent key-addressed JSON snapshots with freshness, readable names, and trimming."

    -- a stored blob round-trips: readable snapshot on disk, index points at it, content preserved
    put_roundTrip: (ut) ->
      cache = makeCache "roundtrip"
      meta = cache\put "https://example.com/DependencyControl.json", '{"name":"DepCtrl"}', "DependencyControl"
      ut\assertNotNil meta
      ut\assertEquals meta.key, "https://example.com/DependencyControl.json"
      ut\assertMatches meta.latestFile, "^%x%x%x%x%x%x%x%-DependencyControl%-%d+T%d+Z%-%x%x%x%x%.json$"

      path, gotMeta = cache\getFile "https://example.com/DependencyControl.json"
      ut\assertNotNil path
      ut\assertEquals readFile(path), '{"name":"DepCtrl"}'
      ut\assertEquals gotMeta.latestFile, meta.latestFile

    -- the default (real) clock path works end to end — put/get use os.time/os.date, not an injected stub
    put_worksWithDefaultClock: (ut) ->
      cache = FileCache fileOps.joinPath(basePath, "filecache", "defaultClock"), "testNamespace", "test"
      meta = cache\put "u://real", '{"name":"Real"}', "Real"
      ut\assertNotNil meta
      ut\assertEquals readFile(cache\getFile "u://real"), '{"name":"Real"}'

    -- an unknown key is a miss
    getFile_uncached: (ut) ->
      cache = makeCache "uncached"
      ut\assertNil (cache\getFile "https://example.com/none.json")

    -- freshness is bounded by maxAge, measured from cache time
    isFresh_window: (ut) ->
      cache, clock = makeCache "fresh", {maxAge: 100, t: 1000}
      meta = cache\put "u://f", "{}", "f"
      ut\assertTrue cache\isFresh meta
      clock.t = 1099
      ut\assertTrue cache\isFresh meta
      clock.t = 1100
      ut\assertFalse cache\isFresh meta

    -- a second put repoints the index at the newer snapshot
    put_updatesLatest: (ut) ->
      cache, clock = makeCache "update", {t: 1000}
      first = cache\put "u://f", '{"v":1}', "f"
      clock.t = 1002
      second = cache\put "u://f", '{"v":2}', "f"
      ut\assertNotEquals first.latestFile, second.latestFile
      path = cache\getFile "u://f"
      ut\assertEquals readFile(path), '{"v":2}'

    -- a stale entry is still resolvable, so callers can fall back to it (with a warning) when offline
    getFile_staleStillResolves: (ut) ->
      cache, clock = makeCache "stale", {maxAge: 100, t: 1000}
      cache\put "u://f", '{"cached":true}', "f"
      clock.t = 5000
      path, meta = cache\getFile "u://f"
      ut\assertNotNil path
      ut\assertFalse cache\isFresh meta
      ut\assertEquals readFile(path), '{"cached":true}'

    -- filesystem-hostile characters in the label are replaced (label follows the key slug)
    put_sanitizesLabel: (ut) ->
      cache = makeCache "sanitize"
      meta = cache\put "u://f", "{}", "My Feed/../x"
      ut\assertMatches meta.latestFile, "^%x%x%x%x%x%x%x%-My_Feed_.._"

    -- .get reuses one instance per resolved directory, and constructs distinct ones for a different name
    get_sharesInstancePerDir: (ut) ->
      base = fileOps.joinPath basePath, "filecache", "shared"
      a1 = FileCache.get base, "ns", "one"
      a2 = FileCache.get base, "ns", "one"
      b = FileCache.get base, "ns", "two"
      ut\assertTrue a1 == a2 -- same base/namespace/name → shared instance
      ut\assertFalse a1 == b -- different name → distinct instance

    -- expiry is fixed at write time, so bumping the instance's maxAge afterwards can't retroactively prolong it
    put_expiryFixedAtWriteTime: (ut) ->
      cache, clock = makeCache "fixed-expiry", {maxAge: 100, t: 1000}
      meta = cache\put "u://f", "{}", "f" -- expiresAt fixed at 1000 + 100 = 1100
      cache.maxAge = 100000 -- a later, longer default lifetime
      clock.t = 1200
      ut\assertFalse cache\isFresh meta -- still stale: honors the baked 1100, not the new maxAge

    -- a per-resource expiresAfter on put overrides the cache's default lifetime for that one entry
    put_perResourceExpiry: (ut) ->
      cache, clock = makeCache "per-resource", {maxAge: 100, t: 1000}
      short = cache\put "u://short", "{}", "s", 10 -- expiresAt 1010
      long = cache\put "u://long", "{}", "l", 5000 -- expiresAt 6000
      clock.t = 1050
      ut\assertFalse cache\isFresh short -- 1050 >= 1010
      ut\assertTrue cache\isFresh long -- 1050 < 6000

    -- trimming caps retained snapshots but never deletes an entry's current one
    trim_keepsLatestOverCap: (ut) ->
      cache, clock = makeCache "trim", {maxFiles: 2, t: 1000}
      first = cache\put "u://f", '{"v":1}', "f"
      clock.t = 1001
      cache\put "u://f", '{"v":2}', "f"
      clock.t = 1002
      third = cache\put "u://f", '{"v":3}', "f"

      -- the oldest, unprotected snapshot is gone; the newest (the index's target) survives
      ut\assertFalsy fileOps.getAttributes(fileOps.joinPath(cache.cacheDir, first.latestFile), "mode").attr
      ut\assertEquals "file", fileOps.getAttributes(fileOps.joinPath(cache.cacheDir, third.latestFile), "mode").attr
      path = cache\getFile "u://f"
      ut\assertEquals readFile(path), '{"v":3}'

    -- get materializes the snapshot through the codec and memoizes it: a second get returns the same object
    get_materializesAndMemoizes: (ut) ->
      cache = makeCache "get-memo", {deserialize: (content) -> {:content}}
      cache\put "u://f", '{"v":1}', "f"
      v1, _, fresh = cache\get "u://f"
      ut\assertEquals v1.content, '{"v":1}'
      ut\assertTrue fresh
      v2 = cache\get "u://f"
      ut\assertIs v1, v2 -- same object → served from L1, not re-deserialized

    -- a memo is keyed to its snapshot's cache time, so another writer's newer put supersedes it: get re-reads L2
    get_memoSupersededByNewerSnapshot: (ut) ->
      dir = fileOps.joinPath basePath, "filecache", "get-super"
      codec = (content) -> {:content}
      a = FileCache dir, "ns", "s", {deserialize: codec, now: -> 1000}
      a\put "u://f", '{"v":1}', "f"
      a\get "u://f" -- a's L1 memoizes v1 @ cachedAt 1000
      b = FileCache dir, "ns", "s", {deserialize: codec, now: -> 1002}
      b\put "u://f", '{"v":2}', "f" -- L2 snapshot is now v2 @ cachedAt 1002
      value = a\get "u://f" -- a's memo no longer matches the on-disk cachedAt → re-reads L2
      ut\assertEquals value.content, '{"v":2}'

    -- get reports staleness without withholding the value, so callers can use it as an offline fallback
    get_staleReturnsValueWithFreshFalse: (ut) ->
      cache, clock = makeCache "get-stale", {maxAge: 100, t: 1000, deserialize: (content) -> {:content}}
      cache\put "u://f", '{"cached":true}', "f"
      clock.t = 1200
      value, meta, fresh = cache\get "u://f"
      ut\assertEquals value.content, '{"cached":true}'
      ut\assertFalse fresh
      ut\assertNotNil meta

    -- an uncached key is a plain miss
    get_missReturnsNil: (ut) ->
      cache = makeCache "get-miss", {deserialize: (content) -> {:content}}
      ut\assertNil (cache\get "u://none")

    -- soft expireAll marks entries cached before the cut-off stale but keeps the snapshot; a later put refreshes
    expireAll_marksOlderStaleKeepingSnapshot: (ut) ->
      cache, clock = makeCache "expire-soft", {maxAge: 100000, t: 1000}
      meta = cache\put "u://f", '{"v":1}', "f"
      ut\assertTrue cache\isFresh meta -- within the (large) window
      cache\expireAll 1500 -- everything cached before 1500 is now stale
      ut\assertFalse cache\isFresh meta
      ut\assertNotNil (cache\getFile "u://f") -- snapshot retained as an offline fallback
      clock.t = 1600
      refreshed = cache\put "u://f", '{"v":2}', "f" -- re-put after the cut-off
      ut\assertTrue cache\isFresh refreshed -- fresh again

    -- purging expireAll deletes the affected entries' L1 memo, snapshot, and index, so the key becomes a miss
    expireAll_purgeDeletesEntries: (ut) ->
      cache = makeCache "expire-purge", {t: 1000, deserialize: (content) -> {:content}}
      meta = cache\put "u://f", '{"v":1}', "f"
      cache\get "u://f" -- prime the L1 memo
      cache\expireAll 2000, true -- purge everything cached before 2000
      ut\assertNil (cache\get "u://f") -- memo dropped and L2 gone → full miss
      ut\assertFalsy fileOps.getAttributes(fileOps.joinPath(cache.cacheDir, meta.latestFile), "mode").attr

    _order: {
      "put_roundTrip", "put_worksWithDefaultClock", "getFile_uncached", "isFresh_window", "put_updatesLatest"
      "getFile_staleStillResolves", "put_sanitizesLabel"
      "get_sharesInstancePerDir", "put_expiryFixedAtWriteTime", "put_perResourceExpiry", "trim_keepsLatestOverCap"
      "get_materializesAndMemoizes", "get_memoSupersededByNewerSnapshot"
      "get_staleReturnsValueWithFreshFalse", "get_missReturnsNil"
      "expireAll_marksOlderStaleKeepingSnapshot", "expireAll_purgeDeletesEntries"
    }
  }
