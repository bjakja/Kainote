-- Downloader engine tests: round-robin scheduling and per-download callbacks,
-- driven by a fake transfer driver so the suite stays fully offline.
-- Called from Tests.moon as: (require "...test.Downloader") basePath
(basePath) ->
  Downloader = require "l0.DependencyControl.Downloader"
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  {:resolveRedirect} = UnitTestSuite\getTestExports Downloader

  -- Fake transfer driver for Downloader.multiplex: each download completes after
  -- `steps` step() calls (1 byte each), recording the order step() is called so
  -- tests can assert round-robin fairness without any real network I/O.
  makeFakeDriver = (steps, order) ->
    {
      start: (dl) ->
        dl.totalBytes = steps
        dl.bytesReceived = 0
        true
      step: (dl) ->
        order[#order + 1] = dl.id
        dl.bytesReceived += 1
        return "done" if dl.bytesReceived >= steps
        "more"
      finish: (dl) -> nil
    }

  -- builds a downloader whose runner drives multiplex with the given fake driver
  fakeManager = (driver) ->
    Downloader (mgr) -> Downloader.multiplex mgr, driver

  Status = Downloader.Download.Status

  {
    _description: "Tests for the Downloader engine: round-robin scheduling and per-download callbacks (via a fake driver). (Offline — no network.)"

    -- round-robin scheduling: the scheduler must step every active transfer once
    -- per pass, so two downloads interleave rather than running one to completion first

    roundRobin_interleaves: (ut) ->
      order = {}
      dm = fakeManager makeFakeDriver 3, order
      dm\addDownload "http://x/1", "#{basePath}_rr1"
      dm\addDownload "http://x/2", "#{basePath}_rr2"
      dm\await!
      -- 2 downloads × 3 steps; each pass touches both before re-stepping either
      ut\assertEquals #order, 6
      ut\assertNotEquals order[1], order[2] -- first pass touched both
      ut\assertNotEquals order[3], order[4] -- second pass too
      ut\assertEquals dl.status, Status.Finished for dl in *dm.downloads

    -- the user-described scenario: start two slow downloads, detect (via the
    -- progress callback) that both are in flight simultaneously, then abort early

    roundRobin_detectsConcurrencyThenCancels: (ut) ->
      order = {}
      dm = fakeManager makeFakeDriver 1000, order -- "slow": many steps to finish
      dm\addDownload "http://x/1", "#{basePath}_c1"
      dm\addDownload "http://x/2", "#{basePath}_c2"

      maxConcurrent = 0
      dm\on Downloader.Event.Progress, (downloader, percent) ->
        inFlight = 0
        for dl in *dm.downloads
          inFlight += 1 if dl.status == Status.Active and (dl.bytesReceived or 0) > 0
        maxConcurrent = math.max maxConcurrent, inFlight
        dm\cancel! if maxConcurrent >= 2 -- proven concurrent → abort
      dm\await!

      ut\assertGreaterThanOrEquals maxConcurrent, 2
      -- aborted after the first pass: neither 1000-step download finished
      ut\assertEquals dl.status, Status.Cancelled for dl in *dm.downloads

    -- Finish event listeners fire on completion and may mark the download failed
    -- (the mechanism SHA-1 verification rides on)

    finishEvent_canMarkFailed: (ut) ->
      dm = fakeManager makeFakeDriver 1, {}
      dl = dm\addDownload "http://x/1", "#{basePath}_fin"
      fired = false
      dl\on Downloader.Download.Event.Finish, (d) ->
        fired = true
        d\markFailed "verification failed"
      dm\await!
      ut\assertTrue fired
      ut\assertEquals dl.error, "verification failed"
      ut\assertEquals dl.status, Status.Failed

    -- on/off: a removed listener no longer fires

    on_off: (ut) ->
      dl = Downloader.Download "http://x/1", "#{basePath}_o", 1
      count = 0
      cb = (d) -> count += 1
      dl\on Downloader.Download.Event.Progress, cb
      dl\_notifyProgress!
      dl\off Downloader.Download.Event.Progress, cb
      dl\_notifyProgress!
      ut\assertEquals count, 1

    on_rejectsUnknownEvent: (ut) ->
      dl = Downloader.Download "http://x/1", "#{basePath}_u", 1
      ut\assertError -> dl\on "notAnEvent", ->

    -- addDownload sha1: a matching hash leaves no error; a mismatch records one

    addDownload_sha1Verifies: (ut) ->
      path = "#{basePath}_sha1ok.txt"
      handle = io.open path, "wb"
      handle\write "abc"
      handle\close!
      dm = fakeManager makeFakeDriver 1, {}
      dl = dm\addDownload "http://x/1", path, "a9993e364706816aba3e25717850c26c9cd0d89d"
      dm\await!
      os.remove path
      ut\assertNil dl.error
      ut\assertEquals dl.status, Status.Finished

    addDownload_sha1Mismatch: (ut) ->
      path = "#{basePath}_sha1bad.txt"
      handle = io.open path, "wb"
      handle\write "abc"
      handle\close!
      dm = fakeManager makeFakeDriver 1, {}
      dl = dm\addDownload "http://x/1", path, ("0")\rep 40
      dm\await!
      os.remove path
      ut\assertString dl.error
      ut\assertEquals dl.status, Status.Failed

    -- Downloader-level events: Progress fires during, Finished fires after await

    downloaderEvents: (ut) ->
      dm = fakeManager makeFakeDriver 2, {}
      dm\addDownload "http://x/1", "#{basePath}_de"
      progressCount, finished = 0, false
      dm\on Downloader.Event.Progress, (d, percent) -> progressCount += 1
      dm\on Downloader.Event.Finished, (d) -> finished = true
      dm\await!
      ut\assertGreaterThan progressCount, 0
      ut\assertTrue finished

    -- await(onProgress) registers the callback for that run only and removes it before
    -- returning, so it never leaks into a later await. It follows the EventEmitter convention
    -- of receiving the emitter (the downloader) ahead of the percent.

    await_onProgressAutoBinds: (ut) ->
      dm = fakeManager makeFakeDriver 2, {}
      dm\addDownload "http://x/1", "#{basePath}_apb1"
      seen = {}
      dm\await (downloader, percent) ->
        ut\assertEquals downloader, dm -- the emitter is passed through per the convention
        seen[#seen + 1] = percent
      ut\assertGreaterThan #seen, 0
      ut\assertEquals type(seen[1]), "number"

      -- a second await without a callback must not re-invoke the first run's listener
      priorCount = #seen
      dm\clear!
      dm\addDownload "http://x/2", "#{basePath}_apb2"
      dm\await!
      ut\assertEquals #seen, priorCount

    -- a failed start marks the download Failed with the start error

    runner_recordsStartFailure: (ut) ->
      failingDriver = {
        start: (dl) -> false, "boom"
        step: (dl) -> "done"
        finish: (dl) -> nil
      }
      dm = Downloader (mgr) -> Downloader.multiplex mgr, failingDriver
      dm\addDownload "http://x/1", "#{basePath}_f1"
      dm\await!
      ut\assertEquals dm.downloads[1].error, "boom"
      ut\assertEquals dm.downloads[1].status, Status.Failed

    -- a single download can be cancelled mid-flight without affecting the others

    individualCancel: (ut) ->
      order = {}
      dm = fakeManager makeFakeDriver 3, order
      dl1 = dm\addDownload "http://x/1", "#{basePath}_ic1"
      dl2 = dm\addDownload "http://x/2", "#{basePath}_ic2"
      dm\on Downloader.Event.Progress, -> dl1\cancel! -- cancel dl1 once it's underway
      dm\await!
      ut\assertEquals dl1.status, Status.Cancelled
      ut\assertEquals dl2.status, Status.Finished

    -- addDownload queueing and validation

    addDownload_queues: (ut) ->
      dm = Downloader!
      dl = dm\addDownload "https://example.com/x", "#{basePath}_dl.txt"
      ut\assertEquals dl.url, "https://example.com/x"
      ut\assertEquals #dm.downloads, 1

    addDownload_badArgs: (ut) ->
      dl, err = Downloader!\addDownload nil, nil
      ut\assertNil dl
      ut\assertString err

    -- clear empties the arrays in place (external references stay valid)

    clear_emptiesInPlace: (ut) ->
      dm = Downloader!
      downloadsRef = dm.downloads
      dm\addDownload "http://x/1", "#{basePath}_cl"
      dm\clear!
      ut\assertEquals #dm.downloads, 0
      ut\assertIs dm.downloads, downloadsRef -- same table, emptied in place

    -- the private-host SSRF guard (enabled per-instance) refuses literal private/loopback addresses,
    -- lets public addresses through, and is skippable via the option (all with literal IPs, so offline)

    blockPrivateHosts_refusesPrivateLiteral: (ut) ->
      dm = Downloader nil, {blockPrivateHosts: true}
      dl, err = dm\addDownload "http://127.0.0.1/x", "#{basePath}_blk"
      ut\assertNil dl
      ut\assertString err

    blockPrivateHosts_allowsPublicLiteral: (ut) ->
      dm = Downloader nil, {blockPrivateHosts: true}
      dl = dm\addDownload "http://93.184.216.34/x", "#{basePath}_pub"
      ut\assertNotNil dl

    blockPrivateHosts_optOutAllowsPrivate: (ut) ->
      dm = Downloader nil, {blockPrivateHosts: false}
      dl = dm\addDownload "http://127.0.0.1/x", "#{basePath}_optOut"
      ut\assertNotNil dl

    -- with the guard on, a non-http(s) URL is refused even though it has no host for the private-IP check
    blockPrivateHosts_refusesNonHttpScheme: (ut) ->
      dm = Downloader nil, {blockPrivateHosts: true}
      dl, err = dm\addDownload "file:///etc/passwd", "#{basePath}_file"
      ut\assertNil dl
      ut\assertString err

    -- fetch caps are stored on the downloader for the backend to enforce
    new_storesFetchCaps: (ut) ->
      dm = Downloader nil, {maxFileSize: 1000, timeout: 5}
      ut\assertEquals dm.maxFileSize, 1000
      ut\assertEquals dm.timeout, 5

    -- resolveRedirect: a Location header may be absolute, protocol-relative, root-relative, or path-relative
    resolveRedirect_resolvesAllForms: (ut) ->
      ut\assertEquals resolveRedirect("http://a/b/c", "https://x/y"), "https://x/y" -- absolute wins
      ut\assertEquals resolveRedirect("http://a.com/b/c", "/new"), "http://a.com/new" -- root-relative
      ut\assertEquals resolveRedirect("https://a.com/b", "//o.com/z"), "https://o.com/z" -- protocol-relative
      ut\assertEquals resolveRedirect("https://a.com/d/p.json", "s.json"), "https://a.com/d/s.json" -- path-relative

    _order: {
      "roundRobin_interleaves", "roundRobin_detectsConcurrencyThenCancels",
      "finishEvent_canMarkFailed", "on_off", "on_rejectsUnknownEvent",
      "addDownload_sha1Verifies", "addDownload_sha1Mismatch",
      "downloaderEvents", "await_onProgressAutoBinds",
      "runner_recordsStartFailure", "individualCancel",
      "addDownload_queues", "addDownload_badArgs",
      "clear_emptiesInPlace",
      "blockPrivateHosts_refusesPrivateLiteral", "blockPrivateHosts_allowsPublicLiteral",
      "blockPrivateHosts_optOutAllowsPrivate", "blockPrivateHosts_refusesNonHttpScheme",
      "new_storesFetchCaps", "resolveRedirect_resolvesAllForms"
    }
  }
