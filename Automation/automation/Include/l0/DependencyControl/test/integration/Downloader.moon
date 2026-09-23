-- Real-HTTP Downloader integration tests against a local pegasus/copas server
-- (test/helpers/mock-http-server). Self-gating via _condition: skipped unless the
-- server's Lua deps are installed, so the default offline run never needs
-- luasocket/copas/pegasus.
-- Called from test.moon as: (controls\requireTest "integration.Downloader") basePath, requireHelper
(basePath, requireHelper) ->
  Downloader = require "l0.DependencyControl.Downloader"
  fileOps = require "l0.DependencyControl.file-ops"

  {
    _description: "Real-HTTP Downloader tests against a local test server (runs when launchable)."

    -- The controller is required lazily and pcall-guarded, so this is harmless where the test
    -- helpers aren't reachable (e.g. a stripped-down install) — it just skips.
    _condition: ->
      ok, MockServerController = pcall requireHelper, "MockHttpServerController"
      return false, "mock server helper unavailable (#{MockServerController})" unless ok
      isReady, err = MockServerController\isReady!
      return false, "mock server is not ready to start: #{err}" unless isReady
      return true

    _setup: (ut) ->
      MockServerController = requireHelper "MockHttpServerController"
      base = "#{basePath}_downloader"
      serveDir, downloadDir = "#{base}/fixtures", "#{base}/out"
      fileOps.mkdir d, false, true for d in *{base, serveDir, downloadDir}

      -- deterministic pseudo-random bytes (reproducible, no rng seeding dependency)
      makeBytes = (n) ->
        t, x = {}, 0x1234567
        for i = 1, n
          x = (x * 1103515245 + 12345) % 0x80000000
          t[i] = string.char x % 256
        table.concat t

      fixtures = {}
      for spec in *{ {"small.bin", 2048}, {"medium.bin", 64 * 1024}, {"large.bin", 256 * 1024} }
        name, size = spec[1], spec[2]
        path = "#{serveDir}/#{name}"
        f = assert io.open path, "wb"
        f\write makeBytes size
        f\close!
        sha1 = assert fileOps.getHash path
        fixtures[#fixtures + 1] = {:name, :sha1}

      server = MockServerController :serveDir
      server\start!
      {:server, :fixtures, :downloadDir}

    _teardown: (ut, ctx) ->
      ctx.server\stop! if ctx and ctx.server

    -- all transfers at full speed, fired together: every file must arrive and verify (sha1)
    concurrentFast: (ut, ctx) ->
      dm, dls = Downloader!, {}
      for f in *ctx.fixtures
        dls[f.name] = dm\addDownload "#{ctx.server.baseUrl}/fast/#{f.name}", "#{ctx.downloadDir}/#{f.name}", f.sha1
      dm\await!
      ut\assertEquals dls[f.name].status, Downloader.Download.Status.Finished for f in *ctx.fixtures

    -- chunked, throttled transfers kept in flight at once: the real concurrency stress
    concurrentSlow: (ut, ctx) ->
      dm, dls = Downloader!, {}
      for f in *ctx.fixtures
        dls[f.name] = dm\addDownload "#{ctx.server.baseUrl}/slow/#{f.name}?delay=20&chunk=4096", "#{ctx.downloadDir}/slow_#{f.name}", f.sha1
      dm\await!
      ut\assertEquals dls[f.name].status, Downloader.Download.Status.Finished for f in *ctx.fixtures

    -- more downloads than connection slots: all must still complete (windowed scheduler)
    queuedBeyondLimit: (ut, ctx) ->
      f = ctx.fixtures[1]
      dm, dls = Downloader(nil, {maxConnectionsPerServer: 2}), {}
      for i = 1, 5
        dls[i] = dm\addDownload "#{ctx.server.baseUrl}/slow/#{f.name}?delay=20&chunk=1024", "#{ctx.downloadDir}/q#{i}.bin", f.sha1
      dm\await!
      ut\assertEquals dls[i].status, Downloader.Download.Status.Finished for i = 1, 5

    -- a non-2xx response must fail the transfer, not hang or report success
    httpError: (ut, ctx) ->
      dm = Downloader!
      dl = dm\addDownload "#{ctx.server.baseUrl}/status/404", "#{ctx.downloadDir}/missing.bin"
      dm\await!
      ut\assertEquals dl.status, Downloader.Download.Status.Failed

    -- a 302 (with a relative Location) is followed to the target and the file arrives intact. Exercises
    -- curl's own redirect following on Unix and the WinINet backend's hand-rolled redirect loop on Windows.
    followsRedirect: (ut, ctx) ->
      f = ctx.fixtures[1]
      dm = Downloader!
      dl = dm\addDownload "#{ctx.server.baseUrl}/redirect-to/fast/#{f.name}", "#{ctx.downloadDir}/redir_#{f.name}", f.sha1
      dm\await!
      ut\assertEquals dl.status, Downloader.Download.Status.Finished

    -- a response larger than the configured byte cap is aborted, not written out whole
    -- (curl's CURLOPT_MAXFILESIZE on Unix, the WinINet backend's read-loop byte cap on Windows)
    capsFileSize: (ut, ctx) ->
      f = ctx.fixtures[3] -- large.bin (256 KiB), well over the 64 KiB cap below
      dm = Downloader nil, {maxFileSize: 64 * 1024}
      dl = dm\addDownload "#{ctx.server.baseUrl}/fast/#{f.name}", "#{ctx.downloadDir}/capped_#{f.name}"
      dm\await!
      ut\assertEquals dl.status, Downloader.Download.Status.Failed

    -- a fetch that outlasts the configured timeout is aborted rather than completing. The file is
    -- dripped slowly (many delayed chunks) so the transfer can only finish long after the deadline.
    capsTimeout: (ut, ctx) ->
      f = ctx.fixtures[3]
      dm = Downloader nil, {timeout: 1}
      dl = dm\addDownload "#{ctx.server.baseUrl}/slow/#{f.name}?delay=200&chunk=8192", "#{ctx.downloadDir}/timeout_#{f.name}"
      dm\await!
      ut\assertEquals dl.status, Downloader.Download.Status.Failed

    _order: { "concurrentFast", "concurrentSlow", "queuedBeyondLimit", "httpError", "followsRedirect", "capsFileSize", "capsTimeout" }
  }
