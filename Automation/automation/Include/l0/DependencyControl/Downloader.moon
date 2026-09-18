-- Non-blocking download manager with SHA-1 verification.
-- Pure FFI implementation inspired by torque's DM.DownloadManager.
--
--   macOS/Linux: libcurl multi interface — parallel, scheduled by libcurl
--   Windows:     WinINet driver multiplexed by our round-robin scheduler (parallel)
--
-- Single-underscore members form the internal contract between a download and its transfer runner and driver.
-- They are not part of the public API, but runner and driver implementations call and maintain them (a
-- Download's _complete/_notifyProgress/_cancel and its transfer scratch, the Downloader's _reportProgress).
-- Double-underscore members are class-private.

ffi = require "ffi"
lfs = require "lfs"
Enum = require "l0.DependencyControl.Enum"
fileOps = require "l0.DependencyControl.file-ops"
EventEmitter = require "l0.DependencyControl.EventEmitter"
Host = require "l0.DependencyControl.Host"
Accessors = require "l0.DependencyControl.Accessors"

msgs = {
  addMissingArgs: "Required arguments #1 (url) and #2 (outfile) had the wrong type. Expected string, got '%s' and '%s'."
  failedToOpen: "Could not open file '%s'."
  noBackend: "No download backend available."
  httpStatus: "Server returned HTTP status %d."
  readFailed: "Connection error while reading response."
  openUrlFailed: "Could not open URL '%s'."
  curlInit: "Failed to initialize curl."
  stalled: "Download stalled: no data received for %d seconds."
  privateHostBlocked: "Refusing to download from a private, loopback, or link-local address as an SSRF safeguard (a feed or package could otherwise point DependencyControl at an internal host): %s. To allow this for local development/testing, set updates.blockPrivateHosts = false in the DependencyControl config."
  nonHttpScheme: "Refusing to download from a non-http(s) URL as an SSRF safeguard (a feed or package could otherwise point DependencyControl at a local file or other scheme): %s."
  tooManyRedirects: "Gave up following redirects while downloading '%s' (too many hops)."
  redirectNoLocation: "Redirect response from '%s' had no Location header."
  tooLarge: "Aborting download: response exceeded the %d-byte size limit."
  timedOut: "Aborting download: exceeded the %d-second fetch timeout."
}

-- Resolves a redirect Location (which may be relative) against the URL it came from. An absolute URL is
-- returned unchanged; "//host/path" inherits the base's scheme; "/path" keeps the base authority; anything
-- else is resolved against the base's directory. Used by the WinINet backend, which follows redirects by
-- hand (libcurl resolves them internally).
resolveRedirect = (base, location) ->
  return location if location\match "^%a[%w+.%-]*://"
  proto, authority, path = base\match "^(%a[%w+.%-]*)://([^/]*)(.*)$"
  return location unless proto
  return "#{proto}:#{location}" if location\sub(1, 2) == "//"
  return "#{proto}://#{authority}#{location}" if location\sub(1, 1) == "/"
  dir = path\match("^(.*/)") or "/"
  "#{proto}://#{authority}#{dir}#{location}"

---Lifecycle state of a single download.
---@alias DownloadStatus
---| "queued" # Queued: created, not yet started
---| "active" # Active: transfer in progress
---| "finished" # Finished: completed successfully
---| "failed" # Failed: completed with an error
---| "cancelled" # Cancelled: cancelled before completion
DownloadStatus = Enum "DownloadStatus", {
  Queued: "queued"
  Active: "active"
  Finished: "finished"
  Failed: "failed"
  Cancelled: "cancelled"
}

-- statuses representing a download that is no longer in flight
isTerminalStatus = {
  [DownloadStatus.Finished]: true
  [DownloadStatus.Failed]: true
  [DownloadStatus.Cancelled]: true
}

-- Reports progress by emitting the downloader's Progress event, then returns
-- whether to keep going (a Progress listener may call cancel! to stop).
report = (manager, progress) ->
  manager\_reportProgress progress
  not manager.cancelled

-- Backend-agnostic aggregate progress (0-100) from per-download state.
-- Relies on dl.bytesReceived / dl.totalBytes / dl.status, which every runner maintains.
computeProgress = (downloads) ->
  total, now, allKnown, done = 0, 0, true, 0
  for dl in *downloads
    if isTerminalStatus[dl.status]
      done += 1
      total += dl.bytesReceived or 0
      now += dl.bytesReceived or 0
    else
      if dl.totalBytes and dl.totalBytes > 0
        total += dl.totalBytes
        now += dl.bytesReceived or 0
      else
        allKnown = false
  if total > 0 and allKnown
    math.floor 100 * now / total
  else
    math.floor 100 * done / math.max #downloads, 1

-- Generic round-robin scheduler over a driver. This is the core scheduling logic
-- (the Windows production path, and the unit-tested path via a fake driver).
-- driver = {
--   start(dl)  -> true | (false, errString)  -- begin one transfer; set dl.totalBytes if known
--   step(dl)   -> "more" | "done" | errString -- advance one chunk; update dl.bytesReceived
--   finish(dl) ->                             -- release one transfer's resources (idempotent)
--   shutdown() ->                             -- optional: release shared resources
-- }
multiplex = (manager, driver) ->
  downloads = manager.downloads
  queue = downloads
  maxConnections = manager.maxConnections or 8
  stallTimeout = manager.stallTimeout

  active, queueIndex = {}, 1

  -- Start the next queued download into an active slot. Returns the started download, or nil
  -- when the queue is exhausted. A download that fails to start is finalized and skipped so
  -- the slot stays filled.
  startNext = ->
    return nil if queueIndex > #queue
    dl = queue[queueIndex]
    queueIndex += 1
    dl.bytesReceived = 0
    ok, err = driver.start dl
    unless ok
      dl\_complete err or "failed to start download"
      return startNext!
    dl.status = DownloadStatus.Active
    dl._lastProgressBytesReceived, dl._lastProgressAt = 0, os.time!
    active[#active + 1] = dl
    dl

  fillSlots = ->
    while #active < maxConnections and queueIndex <= #queue and not manager.cancelled
      break unless startNext!

  fillSlots!

  -- one pass per loop iteration steps every still-active transfer exactly once
  while #active > 0 and not manager.cancelled
    now = os.time!
    remaining = {}
    for dl in *active
      if dl._cancelRequested
        driver.finish dl
        dl\_cancel!
      else
        status = driver.step dl
        if status == "more"
          dl\_notifyProgress!
          if dl.bytesReceived > dl._lastProgressBytesReceived
            -- reset the stall timer
            dl._lastProgressBytesReceived, dl._lastProgressAt = dl.bytesReceived, now
            remaining[#remaining + 1] = dl
          elseif stallTimeout and stallTimeout > 0 and now - dl._lastProgressAt >= stallTimeout
            driver.finish dl
            dl\_complete msgs.stalled\format stallTimeout
          else
            -- no new bytes yet, but not stalled long enough to give up
            remaining[#remaining + 1] = dl
        elseif status == "done"
          driver.finish dl
          dl\_complete!
        else
          driver.finish dl
          dl\_complete status
    active = remaining
    fillSlots!
    -- report progress and allow cancellation between each round of steps
    break unless report manager, computeProgress downloads

  -- cancel remaining individual downloads if the whole downloader is cancelled
  for dl in *active
    driver.finish dl
    dl\_cancel!
  for i = queueIndex, #queue
    queue[i]\_cancel!

  driver.shutdown! if driver.shutdown

-- Platform backend selection: sets defaultRunner(manager) and isInternetConnected().
local defaultRunner, isInternetConnected

if ffi.os != "Windows"
  pcall ffi.cdef, "void* fopen(const char* path, const char* mode);"
  pcall ffi.cdef, "int fclose(void* stream);"
  pcall ffi.cdef, "int usleep(unsigned int usec);"
  pcall ffi.cdef, [[
    void* curl_easy_init(void);
    int curl_easy_setopt(void* handle, int option, ...);
    void curl_easy_cleanup(void* handle);
    int curl_easy_getinfo(void* handle, int info, ...);
    const char* curl_easy_strerror(int errornum);
    void* curl_multi_init(void);
    int curl_multi_setopt(void* multi, int option, long value);
    int curl_multi_add_handle(void* multi, void* easy);
    int curl_multi_remove_handle(void* multi, void* easy);
    int curl_multi_perform(void* multi, int* running);
    int curl_multi_wait(void* multi, void* extra_fds, unsigned int extra_nfds, int timeout_ms, int* numfds);
    void curl_multi_cleanup(void* multi);
    typedef struct CURLMsg {
        int msg;
        void* easy_handle;
        union { void* whatever; int result; } data;
    } CURLMsg;
    CURLMsg* curl_multi_info_read(void* multi, int* msgs_in_queue);
  ]]

  curlNames = ffi.os == "OSX" and {"libcurl.4.dylib", "libcurl.dylib", "curl"} or
    {"libcurl.so.4", "libcurl.so", "curl"}
  local curl
  for name in *curlNames
    loaded, lib = pcall ffi.load, name
    if loaded
      curl = lib
      break

  if curl
    CURLOPT_WRITEDATA = 10001 -- write the response data to the file passed as a pointer
    CURLOPT_URL = 10002 -- set the URL to fetch
    CURLOPT_USERAGENT = 10018 -- set the User-Agent header
    CURLOPT_FOLLOWLOCATION = 52 -- follow HTTP redirects
    CURLOPT_MAXREDIRS = 68 -- cap how many redirects are followed
    CURLOPT_PROTOCOLS = 181 -- bitmask of protocols the transfer may use
    CURLOPT_REDIR_PROTOCOLS = 182 -- bitmask of protocols a redirect may switch to
    CURLPROTO_HTTP = 1
    CURLPROTO_HTTPS = 2
    CURLOPT_PREREQFUNCTION = 20312 -- called before each connection (incl. every redirect hop); may abort it
    CURL_PREREQFUNC_OK, CURL_PREREQFUNC_ABORT = 0, 1
    CURLOPT_FAILONERROR = 45 -- treat HTTP 4xx/5xx responses as errors
    CURLOPT_NOPROGRESS = 43 -- disable curl's built-in progress meter
    CURLOPT_CONNECTTIMEOUT = 78 -- abort if connecting takes longer than the specified number of seconds
    CURLOPT_LOW_SPEED_LIMIT = 19 -- abort if the transfer speed is below this (in bytes/sec) for too long (see LOW_SPEED_TIME)
    CURLOPT_LOW_SPEED_TIME = 20 -- the time (in seconds) the transfer speed should be below the limit before aborting
    CURLINFO_SIZE_DOWNLOAD = 0x300008 -- total bytes downloaded so far
    CURLOPT_MAXFILESIZE = 114 -- abort a transfer curl finds to exceed this many bytes
    CURLOPT_TIMEOUT = 13 -- abort if the whole transfer takes longer than this many seconds
    CURLINFO_CONTENT_LENGTH_DOWNLOAD = 0x30000F -- total expected size of the download, or -1 if unknown
    CURLMSG_DONE = 1 -- a transfer completed (with either success or error)
    CURLMOPT_MAX_TOTAL_CONNECTIONS = 13 -- max simultaneous connections of any kind
    CURLMOPT_MAX_HOST_CONNECTIONS = 7 -- max simultaneous connections to the same host

    -- libcurl's varargs expect a C long for integer options; a bare Lua number
    -- would be passed as a double, so cast explicitly.
    setLong = (h, opt, v) -> curl.curl_easy_setopt h, opt, ffi.cast "long", v
    -- cdata pointers can't be table keys reliably; key by address string instead.
    key = (h) -> tostring ffi.cast "void *", h

    -- Fires before each connection (including every redirect hop), so a redirect to a private/internal
    -- host is refused even though libcurl follows redirects itself. Requires curl >= 7.80, otherwise ignored.
    pcall ffi.cdef, "typedef int (*dc_curl_prereq_cb)(void* clientp, char* conn_primary_ip, char* conn_local_ip, int conn_primary_port, int conn_local_port);"
    prereqBlockPrivate = ffi.cast "dc_curl_prereq_cb", (_, primaryIp) ->
      ok, private = pcall -> Host(ffi.string primaryIp)\resolvesToPrivate!
      ok and private and CURL_PREREQFUNC_ABORT or CURL_PREREQFUNC_OK

    getDouble = (h, info) ->
      out = ffi.new "double[1]"
      curl.curl_easy_getinfo h, info, out
      tonumber out[0]

    -- Unix uses curl's own multi scheduler rather than our round-robin loop.
    defaultRunner = (manager) ->
      downloads = manager.downloads
      -- libcurl keeps excess transfers queued internally
      multi = curl.curl_multi_init!
      maxConnections = manager.maxConnections or 8
      curl.curl_multi_setopt multi, CURLMOPT_MAX_HOST_CONNECTIONS, ffi.cast "long", maxConnections
      curl.curl_multi_setopt multi, CURLMOPT_MAX_TOTAL_CONNECTIONS, ffi.cast "long", maxConnections
      handleMap = {}

      for dl in *downloads
        dl.bytesReceived = 0
        file = ffi.C.fopen dl.outfile, "wb"
        if file == nil
          dl\_complete msgs.failedToOpen\format dl.outfile
          continue
        handle = curl.curl_easy_init!
        if handle == nil
          ffi.C.fclose file
          dl\_complete msgs.curlInit
          continue
        curl.curl_easy_setopt handle, CURLOPT_URL, dl.url
        curl.curl_easy_setopt handle, CURLOPT_USERAGENT, "DependencyControl"
        curl.curl_easy_setopt handle, CURLOPT_WRITEDATA, file
        setLong handle, CURLOPT_FOLLOWLOCATION, 1
        setLong handle, CURLOPT_MAXREDIRS, 10 -- bound redirect fan-out
        -- confine the transfer and any redirect to http(s), so a redirect can't reach file:// etc.
        if manager.blockPrivateHosts
          httpOnly = bit.bor CURLPROTO_HTTP, CURLPROTO_HTTPS
          setLong handle, CURLOPT_PROTOCOLS, httpOnly
          setLong handle, CURLOPT_REDIR_PROTOCOLS, httpOnly
          curl.curl_easy_setopt handle, CURLOPT_PREREQFUNCTION, prereqBlockPrivate
        setLong handle, CURLOPT_FAILONERROR, 1
        setLong handle, CURLOPT_NOPROGRESS, 1
        setLong handle, CURLOPT_CONNECTTIMEOUT, 30
        -- abort a transfer that drops below 1 byte/sec for stallTimeout seconds
        if manager.stallTimeout and manager.stallTimeout > 0
          setLong handle, CURLOPT_LOW_SPEED_LIMIT, 1
          setLong handle, CURLOPT_LOW_SPEED_TIME, manager.stallTimeout
        setLong handle, CURLOPT_MAXFILESIZE, manager.maxFileSize if manager.maxFileSize and manager.maxFileSize > 0
        setLong handle, CURLOPT_TIMEOUT, manager.timeout if manager.timeout and manager.timeout > 0
        dl._handle, dl._file = handle, file
        dl.status = DownloadStatus.Active
        handleMap[key handle] = dl
        curl.curl_multi_add_handle multi, handle

      drain = ->
        pending = ffi.new "int[1]"
        while true
          multiStackInfo = curl.curl_multi_info_read multi, pending
          break if multiStackInfo == nil
          continue unless multiStackInfo.msg == CURLMSG_DONE
          dl = handleMap[key multiStackInfo.easy_handle]
          continue unless dl
          res = multiStackInfo.data.result
          dl.bytesReceived = getDouble dl._handle, CURLINFO_SIZE_DOWNLOAD
          ffi.C.fclose dl._file
          curl.curl_multi_remove_handle multi, dl._handle
          curl.curl_easy_cleanup dl._handle
          dl._file, dl._handle = nil
          transportError = res != 0 and ffi.string(curl.curl_easy_strerror res) or nil
          dl\_complete transportError -- fires finish callbacks (e.g. hash verification)

      -- releases an easy handle + its output file (idempotent)
      releaseHandle = (dl) ->
        ffi.C.fclose dl._file if dl._file
        curl.curl_multi_remove_handle multi, dl._handle
        curl.curl_easy_cleanup dl._handle
        dl._file, dl._handle = nil

      running = ffi.new "int[1]"
      running[0] = 1
      numfds = ffi.new "int[1]"
      while running[0] > 0
        curl.curl_multi_perform multi, running
        drain!
        for dl in *downloads
          continue unless dl._handle
          if dl._cancelRequested
            releaseHandle dl
            dl\_cancel!
          else
            dl.bytesReceived = getDouble dl._handle, CURLINFO_SIZE_DOWNLOAD
            contentLen = getDouble dl._handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD
            dl.totalBytes = contentLen if contentLen > 0
            dl\_notifyProgress!
        break unless report manager, computeProgress downloads
        if running[0] > 0
          curl.curl_multi_wait multi, nil, 0, 100, numfds
          ffi.C.usleep 10000 if numfds[0] == 0
      drain!

      -- finalize any survivors as cancelled (whole-downloader cancellation)
      for dl in *downloads
        if dl._handle
          releaseHandle dl
          dl\_cancel!
      curl.curl_multi_cleanup multi

  else
    defaultRunner = (manager) ->
      dl\_complete msgs.noBackend for dl in *manager.downloads

  isInternetConnected = -> true -- best-effort: assume connected, let downloads report real errors

else
  ffiWin = require "l0.DependencyControl.helpers.ffi-windows"

  pcall ffi.cdef, [[
    void* InternetOpenW(const wchar_t* agent, unsigned long accessType, const wchar_t* proxy, const wchar_t* proxyBypass, unsigned long flags);
    void* InternetOpenUrlW(void* session, const wchar_t* url, const wchar_t* headers, unsigned long headersLen, unsigned long flags, uintptr_t context);
    int InternetReadFile(void* hFile, void* buffer, unsigned long toRead, unsigned long* read);
    int InternetCloseHandle(void* h);
    int InternetSetOptionW(void* hInternet, unsigned long option, void* buffer, unsigned long bufferLen);
    int HttpQueryInfoW(void* hRequest, unsigned long infoLevel, void* buffer, unsigned long* bufferLen, unsigned long* index);
    int HttpQueryInfoA(void* hRequest, unsigned long infoLevel, void* buffer, unsigned long* bufferLen, unsigned long* index);
    int InternetGetConnectedState(unsigned long* flags, unsigned long reserved);
  ]]

  haveKernel32 = ffiWin.haveKernel32
  haveWinInet, winInet = pcall ffi.load, "winInet"

  toWide = ffiWin.toWide

  INTERNET_FLAG_RELOAD = 0x80000000 -- force a reload from the server even if the content is cached
  INTERNET_FLAG_NO_CACHE_WRITE = 0x04000000 -- don't commit this download to the cache
  INTERNET_FLAG_NO_AUTO_REDIRECT = 0x00200000 -- don't auto-follow redirects; we validate each hop ourselves
  INTERNET_OPTION_MAX_CONNS_PER_SERVER = 73 -- max simultaneous connections to the same HTTP/1.1 server
  INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER = 74 -- max simultaneous connections to the same HTTP/1.0 server
  INTERNET_OPTION_CONNECT_TIMEOUT = 2 -- milliseconds to wait for a connection before failing
  INTERNET_OPTION_RECEIVE_TIMEOUT = 6 -- milliseconds to wait for a single read to return before failing
  HTTP_QUERY_STATUS_CODE = 19 -- HTTP response status code (e.g. 200)
  HTTP_QUERY_CONTENT_LENGTH = 5 -- total expected size of the download, or -1 if unknown
  HTTP_QUERY_LOCATION = 33 -- the Location response header (a redirect's target URL)
  HTTP_QUERY_FLAG_NUMBER = 0x20000000 -- return the queried information as a number instead of a string (e.g. for status code or content length)
  MAX_REDIRECTS = 10 -- cap on redirect hops we follow before giving up
  CHUNK_SIZE = 16384 -- bytes to read for each running download per iteration of the scheduler loop (max WinINet buffer size)

  queryNumber = (request, info) ->
    out = ffi.new "unsigned long[1]"
    len = ffi.new "unsigned long[1]"
    len[0] = 4
    ok = winInet.HttpQueryInfoW request, bit.bor(info, HTTP_QUERY_FLAG_NUMBER), out, len, nil
    ok != 0 and tonumber(out[0]) or nil

  -- Reads a response header as a string (ANSI). A fixed buffer is plenty for a Location URL; the
  -- private-host check tolerates a truncated value (it just fails to parse and refuses/loads elsewhere).
  queryString = (request, info) ->
    buf = ffi.new "char[2048]"
    len = ffi.new "unsigned long[1]", 2048
    ok = winInet.HttpQueryInfoA request, info, buf, len, nil
    ok != 0 and ffi.string(buf, len[0]) or nil

  if haveKernel32 and haveWinInet
    -- A WinINet driver for `multiplex`: one request + output file per download,
    -- advanced one chunk per step. The scheduler round-robins across them.
    makeWinINetDriver = (manager, maxConnectionsPerServer = 8) ->
      do
        -- Lift the Windows-default 2-connections-per-server cap so all queued transfers can run at once;
        -- otherwise a 3rd concurrent InternetOpenUrlW to the same host blocks and times out.
        optVal = ffi.new "unsigned long[1]", maxConnectionsPerServer
        winInet.InternetSetOptionW nil, INTERNET_OPTION_MAX_CONNS_PER_SERVER, optVal, 4
        winInet.InternetSetOptionW nil, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, optVal, 4
      session = winInet.InternetOpenW toWide("DependencyControl"), 0, nil, nil, 0
      -- bound each connect/read so a slow host can't block the automation thread indefinitely
      if manager.timeout and manager.timeout > 0
        timeoutMs = ffi.new "unsigned long[1]", manager.timeout * 1000
        winInet.InternetSetOptionW session, INTERNET_OPTION_CONNECT_TIMEOUT, timeoutMs, 4
        winInet.InternetSetOptionW session, INTERNET_OPTION_RECEIVE_TIMEOUT, timeoutMs, 4
      buffer = ffi.new "char[?]", CHUNK_SIZE
      read = ffi.new "unsigned long[1]"
      {
        start: (dl) ->
          outFileHandle, err = io.open dl.outfile, "wb"
          return false, (err or msgs.failedToOpen\format dl.outfile) unless outFileHandle
          dl._deadline = os.time! + manager.timeout if manager.timeout and manager.timeout > 0

          -- Follow redirects by hand (NO_AUTO_REDIRECT) so we can validate each hop's host: WinINet
          -- would otherwise re-resolve and connect to a redirect target with no hook to inspect it.
          fail = (msg) ->
            outFileHandle\close!
            false, msg
          url, redirectsLeft, request = dl.url, MAX_REDIRECTS, nil
          while true
            if manager.blockPrivateHosts
              return fail msgs.nonHttpScheme\format url unless url\match "^https?://"
              host = Host.fromUrl url
              return fail msgs.privateHostBlocked\format url if host and host\resolvesToPrivate!

            request = winInet.InternetOpenUrlW session, toWide(url), nil, 0,
              bit.bor(INTERNET_FLAG_RELOAD, INTERNET_FLAG_NO_CACHE_WRITE, INTERNET_FLAG_NO_AUTO_REDIRECT), 0
            return fail msgs.openUrlFailed\format url if request == nil

            status = queryNumber request, HTTP_QUERY_STATUS_CODE
            if status and status >= 300 and status < 400
              location = queryString request, HTTP_QUERY_LOCATION
              winInet.InternetCloseHandle request
              request = nil
              return fail msgs.tooManyRedirects\format dl.url if redirectsLeft <= 0
              return fail msgs.redirectNoLocation\format url unless location
              url, redirectsLeft = resolveRedirect(url, location), redirectsLeft - 1
            elseif status and status >= 400
              winInet.InternetCloseHandle request
              return fail msgs.httpStatus\format status
            else
              break

          dl._request, dl._outFileHandle = request, outFileHandle
          dl.totalBytes = queryNumber request, HTTP_QUERY_CONTENT_LENGTH
          true

        step: (dl) ->
          return msgs.timedOut\format manager.timeout if dl._deadline and os.time! > dl._deadline
          return msgs.readFailed if 0 == winInet.InternetReadFile dl._request, buffer, CHUNK_SIZE, read
          n = tonumber read[0]
          return "done" if n == 0
          dl._outFileHandle\write ffi.string buffer, n
          dl.bytesReceived += n
          return msgs.tooLarge\format manager.maxFileSize if manager.maxFileSize and manager.maxFileSize > 0 and dl.bytesReceived > manager.maxFileSize
          "more"

        finish: (dl) ->
          winInet.InternetCloseHandle dl._request if dl._request
          dl._outFileHandle\close! if dl._outFileHandle
          dl._request, dl._outFileHandle = nil

        shutdown: ->
          winInet.InternetCloseHandle session
      }

    defaultRunner = (manager) ->
      multiplex manager, makeWinINetDriver manager, manager.maxConnections

  else
    defaultRunner = (manager) ->
      dl\_complete msgs.noBackend for dl in *manager.downloads

  isInternetConnected = ->
    return true unless haveWinInet
    flags = ffi.new "unsigned long[1]"
    winInet.InternetGetConnectedState(flags, 0) != 0

---A single download: its URL, output path, transfer state, and event callbacks.
---Events (see Download.Event): Progress (data arrived), Finish (reached a terminal
---status). A Finish listener may downgrade the status via markFailed (e.g. for a
---failed hash verification). The current state is exposed via @status (Download.Status).
---@class Download: EventEmitter
---@field status DownloadStatus Current lifecycle state of this download.
class Download extends EventEmitter
  @Status = DownloadStatus

  ---Events emitted by a Download.
  ---@alias DownloadEvent
  ---| "progress" # Progress: transfer data arrived
  ---| "finish" # Finish: the download reached a terminal status
  @Event = Enum "DownloadEvent", { Progress: "progress", Finish: "finish" }

  ---Creates a single download in the Queued state.
  ---@param url string
  ---@param outfile string Full output path.
  ---@param id? number An identifier assigned by the Downloader.
  new: (@url, @outfile, @id) =>
    super!
    @bytesReceived = 0
    @totalBytes = nil
    @status = DownloadStatus.Queued
    @error = nil

  ---Requests cancellation of this download. The downloader releases its
  ---resources and sets the status to Cancelled on its next scheduling pass.
  cancel: => @_cancelRequested = true

  ---Marks the download as failed (e.g. from a Finish listener performing
  ---hash verification).
  ---@param err string The failure reason.
  markFailed: (err) =>
    @error = err
    @status = @@Status.Failed

  ---Part of the download-runner callback contract: a runner calls this to fire this download's
  ---Progress listeners as bytes arrive.
  _notifyProgress: => @_emit @@Event.Progress

  ---Part of the download-runner callback contract: a runner calls this when the transfer ends to
  ---finalize it (success, or a transport-level error) and fire Finish listeners (which may downgrade
  ---the status via `markFailed`). Idempotent — only the first call takes effect.
  ---@param transportError? string A transport-level error, if any.
  _complete: (transportError) =>
    return if @__finalized
    @__finalized = true
    if transportError
      @error = transportError
      @status = @@Status.Failed
    else
      @status = @@Status.Finished
    @_emit @@Event.Finish

  ---Part of the download-runner callback contract: a runner calls this to finalize the download as
  ---cancelled and fire Finish listeners. Idempotent.
  _cancel: =>
    return if @__finalized
    @__finalized = true
    @status = @@Status.Cancelled
    @_emit @@Event.Finish


---Options accepted by the Downloader constructor.
---@class DownloaderOptions
---@field stallTimeout? number Seconds a transfer may receive no data before it's aborted; 0/false disables stall detection.
---@field maxConnections? integer Maximum simultaneous transfers (also the per-server connection limit); excess transfers queue.
---@field blockPrivateHosts? boolean Refuse URLs whose host is a private/loopback/link-local address (an SSRF guard).
---@field maxFileSize? number Maximum response size in bytes; a transfer found to exceed it is aborted (0/nil = unlimited).
---@field timeout? number Maximum total seconds a transfer may take before it's aborted (0/nil = unlimited).

---Manages a set of concurrent downloads. This is DepCtrl's own engine; the
---DM.DownloadManager-compatible API lives in l0.DependencyControl.DownloadManager.
---Events (see Downloader.Event): Progress (overall %), Finished (await completed).
---@class Downloader: EventEmitter
---@field progress number Current aggregate download progress across all queued transfers, 0-100 (read-only).
class Downloader extends EventEmitter
  @Download = Download

  ---Events emitted by a Downloader.
  ---@alias DownloaderEvent
  ---| "progress" # Progress: aggregate progress across queued transfers changed
  ---| "finished" # Finished: await completed
  @Event = Enum "DownloaderEvent", { Progress: "progress", Finished: "finished" }
  ---Drives every queued download of the given downloader to completion with a round-robin
  ---scheduler, keeping up to its `maxConnections` transfers active and honoring its cancellation
  ---and stall-timeout settings. Blocks until all transfers finish or are cancelled.
  ---@param manager Downloader The downloader whose queued downloads are transferred.
  ---@param driver table The transfer backend, providing `start(dl)`, `step(dl)`, `finish(dl)`, and an optional `shutdown()`.
  @multiplex = multiplex

  -- Maximum simultaneous transfers (also applied as the per-server connection limit on each
  -- backend). Excess downloads are queued and started as slots free.
  maxConnections: 8

  -- The number of seconds a transfer can go without receiving any data before we consider
  -- it stalled and abort it. Set to 0 or false to disable stall detection.
  stallTimeout: 30

  ---Creates a downloader.
  ---@param runner? fun(downloader: Downloader) Overrides the transfer implementation (defaults to the platform backend).
  ---@param options? DownloaderOptions Optional downloader settings.
  new: (runner, options = {}) =>
    super!

    @stallTimeout = options.stallTimeout if options.stallTimeout != nil
    @maxConnections = options.maxConnections if options.maxConnections != nil
    @blockPrivateHosts = options.blockPrivateHosts if options.blockPrivateHosts != nil
    @maxFileSize = options.maxFileSize if options.maxFileSize != nil
    @timeout = options.timeout if options.timeout != nil

    @downloads = {}
    @cancelled = false
    @__runner = runner or defaultRunner

  ---Queues a download. Transfers happen later, in await.
  ---Register progress/finish listeners on the returned Download as needed.
  ---@param url string
  ---@param outfile string Full output path (relative paths unsupported).
  ---@param sha1? string Expected SHA-1 hash; verified automatically on finish.
  ---@return Download? download
  ---@return string? err
  addDownload: (url, outfile, sha1) =>
    unless type(url) == "string" and type(outfile) == "string"
      return nil, msgs.addMissingArgs\format type(url), type(outfile)

    -- as an opt-in per-instance SSRF guard, confine fetches to http(s) and reject private/internal hosts
    -- The scheme check also covers URLs with no host (file://, ...) that the private-IP check can't see.
    if @blockPrivateHosts
      return nil, msgs.nonHttpScheme\format url unless url\match "^https?://"
      host = Host.fromUrl url
      return nil, msgs.privateHostBlocked\format url if host and host\resolvesToPrivate!

    fileOps.mkdir outfile, true, true

    @_lastId = (@_lastId or 0) + 1
    download = Download url, outfile, @_lastId

    if type(sha1) == "string"
      expected = sha1
      -- piggyback on the finish event to verify the downloaded file's hash (case-insensitive)
      download\on Download.Event.Finish, (dl) ->
        return unless dl.status == Download.Status.Finished -- only verify successful transfers
        ok, msg = fileOps.verifyHash dl.outfile, expected
        dl\markFailed msg unless ok

    @downloads[#@downloads + 1] = download
    download

  ---Performs all queued downloads, blocking until they finish or are cancelled.
  ---Subscribe to Progress/Finished via on; a Progress listener may call cancel!.
  ---Inspect each download's final state via its @status (Download.Status).
  ---@param onProgress? fun(downloader: Downloader, percent: number) Called with this downloader and the aggregate progress (0-100) on each Progress event, for the duration of this call only.
  ---@return Downloader self for chaining
  await: (onProgress) =>
    @on @@Event.Progress, onProgress if onProgress
    @__runner @
    @off @@Event.Progress, onProgress if onProgress
    @_emit @@Event.Finished
    return @

  progress: Accessors.property
    get: => computeProgress @downloads

  ---Part of the download-runner callback contract: a runner calls this to report the manager's
  ---aggregate progress and fire the Downloader's Progress listeners.
  ---@param percent number Aggregate progress, 0-100.
  _reportProgress: (percent) => @_emit @@Event.Progress, percent

  ---Cancels all remaining downloads (e.g. from within a Progress listener).
  cancel: => @cancelled = true

  ---Removes all downloads and resets state.
  ---Empties the array in place so external references stay valid.
  clear: =>
    @downloads[i] = nil for i = #@downloads, 1, -1
    @cancelled = false

  ---@return boolean connected Whether an internet connection appears to be available.
  isInternetConnected: => isInternetConnected!

Accessors.install Downloader
UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
return UnitTestSuite\withTestExports Downloader, {:resolveRedirect}
