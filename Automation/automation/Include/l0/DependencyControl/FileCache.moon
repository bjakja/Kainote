Hash = require "l0.DependencyControl.hash"
fileOps = require "l0.DependencyControl.file-ops"
Logger = require "l0.DependencyControl.Logger"
constants = require "l0.DependencyControl.Constants"
Lock = require "l0.DependencyControl.Lock"
dkjson = require "l0.dkjson"

defaultLogger = Logger fileBaseName: "#{constants.DEPCTRL_SHORT_NAME}.FileCache"

LOCK_NAMESPACE = "l0.DependencyControl.FileCache" -- Global-lock namespace for serializing cache writes
LOCK_TIMEOUT = 5000 -- ms to wait for the write lock before skipping the write

-- Replaces filesystem-hostile characters so a cache entry's label is safe in a file name, and clamps its
-- length. Falls back to "entry" for an empty or missing label.
sanitizeLabel = (label) ->
  safe = tostring(label or "entry")\gsub "[^%w%._-]", "_"
  #safe > 0 and safe\sub(1, 64) or "entry"

-- The 7-hex-char SHA-1 slug of a cache key. Deterministic per key (sensitive to its exact bytes), matching
-- the DepCtrl Browser's feed-URL slug convention.
keySlug = (key) -> Hash.getDigest(Hash.HashType.Sha1, key)\sub 1, 7

-- The embedded UTC timestamp of a snapshot file name, for chronological ordering ("" when absent).
snapshotStamp = (fileName) -> fileName\match "(%d+T%d+Z)" or ""

-- An instance's on-disk directory: the configured base, namespaced and named. The single place the layout
-- is defined, shared by the constructor and the `get` factory's registry key.
resolveDir = (basePath, namespace, name) -> "#{aegisub.decode_path basePath}/#{namespace}/#{name}"

---The per-key index entry FileCache persists next to each snapshot; returned by getMeta/getFile/get/put.
---@class FileCacheMeta
---@field key string The cache key this entry indexes.
---@field cachedAt integer Unix time the latest snapshot was written.
---@field expiresAt integer Unix time the entry turns stale, fixed at write time.
---@field latestFile string File name of the latest snapshot, relative to the cache directory.

---Construction options for FileCache.
---@class FileCacheOptions
---@field maxAge? integer Default entry lifetime in seconds, used when a put doesn't set its own (default 3600).
---@field maxFiles? integer Snapshot files retained per cache before the oldest are trimmed (default 50).
---@field logger? Logger Logger for cache operations.
---@field now? fun(): integer Clock override returning Unix time; defaults to os.time (injected in tests).
---@field deserialize? fun(content: string): any Codec turning stored content into the value get returns and memoizes; its presence enables the in-memory L1 layer.

---A persistent, key-addressed cache of JSON blobs on disk. Each `put` is stored as a human-readable,
---timestamped snapshot file (which doubles as a history), indexed by a deterministic per-key meta file that
---names the latest snapshot and records when it was cached. An instance lives under
---`<basePath>/<namespace>/<name>`, so distinct caches (feeds, and others later) share one configurable cache
---base without colliding. Prefer the `get` factory over the constructor so instances are shared per directory.
---With a `deserialize` codec, `get` adds an in-memory L1 layer over the on-disk L2: it returns the
---parsed snapshot and memoizes it, keyed to the snapshot's cache time so a newer `put` (here or in another
---process) transparently supersedes the memo.
---@class FileCache
class FileCache
  @defaultMaxAge = 3600 -- default entry lifetime (seconds), fixed into each entry's index at write time
  @defaultMaxFiles = 50 -- how many snapshot files to retain before trimming the oldest

  -- Shared instances keyed by resolved directory, so callers for the same cache get one instance (and thus
  -- share its L1 memo). Weak values let an unreferenced cache be collected while a live one stays shared.
  @__instances = setmetatable {}, __mode: "v"

  ---Returns the shared cache for a base/namespace/name, reusing the existing instance for that resolved
  ---directory rather than constructing a duplicate. Options apply only when the instance is first created.
  ---@param basePath string The cache root (the `paths.cache` setting, e.g. "?user/cache"); path-decoded.
  ---@param namespace string The owning script namespace (`constants.DEPCTRL_NAMESPACE` for DepCtrl's own caches).
  ---@param name string A short name for this cache's purpose (e.g. "feeds").
  ---@param opts? FileCacheOptions See new.
  ---@return FileCache
  @get = (basePath, namespace, name, opts) ->
    dir = resolveDir basePath, namespace, name
    cache = FileCache.__instances[dir]
    unless cache
      cache = FileCache basePath, namespace, name, opts
      FileCache.__instances[dir] = cache
    return cache

  ---@param basePath string The cache root (the `paths.cache` setting, e.g. "?user/cache"); path-decoded here.
  ---@param namespace string The owning script namespace (`constants.DEPCTRL_NAMESPACE` for DepCtrl's own caches).
  ---@param name string A short subdirectory naming this cache's purpose (e.g. "feeds").
  ---@param opts? FileCacheOptions Defaults for entry lifetime, retention, logging, clock, and the L1 codec.
  new: (basePath, namespace, name, opts = {}) =>
    @cacheDir = resolveDir basePath, namespace, name
    @maxAge = opts.maxAge or @@defaultMaxAge
    @maxFiles = opts.maxFiles or @@defaultMaxFiles
    @logger = opts.logger or defaultLogger
    @now = opts.now or os.time
    @__deserialize = opts.deserialize
    -- L1: key -> {value, cachedAt}; each memo mirrors one L2 snapshot and is superseded when its cachedAt does
    @__l1 = {}

  ---@private
  ---@param key string The cache key.
  ---@return string path Filesystem path of the key's cache-index (meta) JSON file.
  __metaPath: (key) => fileOps.joinPath @cacheDir, "#{keySlug key}.meta.json"

  ---Reads and decodes a cache index (meta) JSON file.
  ---@private
  ---@param path string
  ---@return FileCacheMeta? meta
  __readMeta: (path) =>
    content = fileOps.readFile path
    return nil unless content
    -- a torn read from a concurrent write fails to decode and is treated as a cache miss
    ok, meta = pcall dkjson.decode, content
    ok and type(meta) == "table" and meta or nil

  ---Reads the cache index entry for a key.
  ---@param key string
  ---@return FileCacheMeta? meta nil when the key isn't cached.
  getMeta: (key) => @__readMeta @__metaPath key

  ---Whether a cache entry is still fresh, i.e. its expiry (fixed when it was written) hasn't passed
  ---and it hasn't been invalidated by expireAll. The expiry is intrinsic to the entry, so freshness is the
  ---same regardless of which instance asks or what its current `maxAge` is.
  ---@param meta FileCacheMeta? A meta returned by getMeta.
  ---@return boolean fresh
  isFresh: (meta) =>
    return false unless meta and meta.expiresAt
    return false if @__staleBefore and meta.cachedAt and meta.cachedAt < @__staleBefore
    @.now! < meta.expiresAt

  ---Resolves the latest cached snapshot for a key. The snapshot may be stale; callers use isFresh on the
  ---returned meta to decide whether to serve it directly or only as an offline fallback.
  ---@param key string
  ---@return string? path The snapshot file's path, or nil when the key isn't cached (or its file is gone).
  ---@return FileCacheMeta? meta The cache index entry (returned even when the snapshot file is missing).
  getFile: (key) =>
    meta = @getMeta key
    return nil unless meta and meta.latestFile
    path = fileOps.joinPath @cacheDir, meta.latestFile
    info = fileOps.getAttributes path, "mode"
    return nil, meta unless info and info.attr == "file"
    return path, meta

  ---Returns the deserialized latest snapshot for a key, served from the in-memory L1 memo when it still
  ---mirrors the on-disk snapshot, otherwise read and deserialized from L2 and memoized. The value may be
  ---stale — freshness is reported separately so the caller can serve it or treat it as an offline fallback.
  ---Requires a `deserialize` codec; without one this always misses.
  ---@param key string
  ---@return any? value The deserialized snapshot (the freshest available, even when stale), or nil if not cached.
  ---@return FileCacheMeta? meta The snapshot's index entry, returned even when stale.
  ---@return boolean fresh Whether the returned snapshot is still within its expiry.
  get: (key) =>
    meta = @getMeta key
    return nil unless meta and meta.cachedAt
    fresh = @isFresh meta
    memo = @__l1[key]
    return memo.value, meta, fresh if memo and memo.cachedAt == meta.cachedAt
    return nil, meta, fresh unless @__deserialize and meta.latestFile

    path = fileOps.joinPath @cacheDir, meta.latestFile
    info = fileOps.getAttributes path, "mode"
    content = info and info.attr == "file" and fileOps.readFile path
    return nil, meta, fresh unless content

    value = @.__deserialize content
    @__l1[key] = {:value, cachedAt: meta.cachedAt}
    return value, meta, fresh

  ---Marks every entry cached before the cut-off as stale, so the next get/isFresh reports it not fresh. The
  ---snapshot is kept as an offline fallback, and a later put makes the entry fresh again. When purging, the
  ---affected entries are instead dropped from the L1 memo and their on-disk snapshots and index deleted, to
  ---reclaim space or hard-reset.
  ---@param before? integer Cut-off Unix time; entries cached strictly before it are affected (default: now).
  ---@param purge? boolean Delete the affected entries (L1 + L2) instead of only marking them stale (default false).
  expireAll: (before = @.now!, purge = false) =>
    @__staleBefore = before
    return unless purge

    files = fileOps.listDir @cacheDir
    return unless files

    -- collect the key slugs whose index predates the cut-off, dropping their memos as we go
    expiredSlugs = {}
    for file in *files
      continue unless file\match "%.meta%.json$"
      meta = @__readMeta fileOps.joinPath @cacheDir, file
      continue unless meta and meta.cachedAt and meta.cachedAt < before
      expiredSlugs[keySlug meta.key] = true
      @__l1[meta.key] = nil

    -- every file (snapshot or index) is named with its key's 7-hex slug prefix, so one pass removes both
    for file in *files
      slug = file\match "^(%x%x%x%x%x%x%x)"
      fileOps.remove fileOps.joinPath @cacheDir, file if slug and expiredSlugs[slug]

  ---Stores a blob under a readable, timestamped snapshot and repoints the index at it, then trims old
  ---snapshots. The snapshot name is `<slug>-<label>-<utcTimestamp>-<rand>.json` (slug first so a key's
  ---snapshots sort together).
  ---@param key string The cache key.
  ---@param content string The blob to persist verbatim.
  ---@param label? string A human-readable name, used to make the snapshot file recognizable.
  ---@param expiresAfter? integer Seconds until this entry expires, fixed into the index now (default: this cache's `maxAge`). Pass a per-resource lifetime to honor, e.g., an HTTP `max-age`.
  ---@return FileCacheMeta? meta The written index entry, or nil on a filesystem error or a lock timeout.
  ---@return string? err
  put: (key, content, label, expiresAfter = @maxAge) =>
    -- The meta index is a single mutable file; concurrent DepCtrl instances/processes could interleave
    -- writes and corrupt it, so serialize under a Global lock. A failed acquire returns (nil, err) and
    -- the caller just leaves the data uncached this round.
    meta, err = Lock\guard {
      namespace: LOCK_NAMESPACE
      resource: @cacheDir
      scope: Lock.Scope.Global
      holderName: "FileCache"
      logger: @logger
      timeout: LOCK_TIMEOUT
    }, -> @__write key, content, label, expiresAfter
    -- refresh the L1 memo to the snapshot just written, so get() serves it without re-reading L2
    -- (@.__deserialize, not @__deserialize, so the stored codec is called plainly, without self)
    @__l1[key] = {value: @.__deserialize(content), cachedAt: meta.cachedAt} if meta and @__deserialize
    return meta, err

  ---Writes a snapshot, points the index at it, and trims. Runs under the cache write lock held by put.
  ---@private
  ---@param key string The cache key.
  ---@param content string The blob to persist verbatim.
  ---@param label? string A human-readable name, used to make the snapshot file recognizable.
  ---@param expiresAfter integer Seconds until expiry, from the write time; stamped into the index as an absolute `expiresAt`.
  ---@return FileCacheMeta? meta The written index entry, or nil on a filesystem error.
  ---@return string? err Error message when a write failed, nil on success.
  __write: (key, content, label, expiresAfter) =>
    dirRes, dirErr = fileOps.mkdir @cacheDir, false, true
    return nil, dirErr if dirRes == nil

    now = @.now!
    stamp = os.date "!%Y%m%dT%H%M%SZ", now
    rand = "%04X"\format math.random 0, 16^4 - 1
    fileName = "#{keySlug key}-#{sanitizeLabel label}-#{stamp}-#{rand}.json"

    ok, err = fileOps.writeFile {@cacheDir, fileName}, content, true
    return nil, err unless ok

    meta = {:key, cachedAt: now, expiresAt: now + expiresAfter, latestFile: fileName}
    ok, err = fileOps.writeFile @__metaPath(key), (dkjson.encode meta, {indent: true, indentMode: "prettier"}), true
    return nil, err unless ok

    @__trim!
    return meta

  ---Caps the number of retained snapshot files at `maxFiles`, deleting the oldest first but never one an
  ---index still points at (an entry's current snapshot survives regardless of the cap).
  ---@private
  __trim: =>
    files = fileOps.listDir @cacheDir
    return unless files

    protectedFiles, snapshots = {}, {}
    for file in *files
      if file\match "%.meta%.json$"
        meta = @__readMeta fileOps.joinPath @cacheDir, file
        protectedFiles[meta.latestFile] = true if meta and meta.latestFile
      elseif file\match "%.json$"
        snapshots[#snapshots + 1] = file

    -- newest first, so everything past the cap is the oldest
    table.sort snapshots, (a, b) -> snapshotStamp(a) > snapshotStamp(b)
    for i = @maxFiles + 1, #snapshots
      continue if protectedFiles[snapshots[i]]
      fileOps.remove fileOps.joinPath @cacheDir, snapshots[i]

return FileCache
