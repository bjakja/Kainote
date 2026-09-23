UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
constants = require "l0.DependencyControl.Constants"
domain = require "l0.DependencyControl.domain"
DepCtrl = require "l0.DependencyControl"

-- Suite for the DependencyControl Toolbox macro. The Plumbing class proves the automation-script test
-- wiring (DependencyControl discovers the suite and hands it the script's registered macros + testExports);
-- the Urls/Age/InstalledList/UntrustedPrompt classes cover the pure and near-pure helpers; the remaining
-- classes drive the dialog flows end to end, stubbing aegisub.dialog.display and the Toolbox's collaborators
-- (the updater, config handler, feed inventory/manager, and the injected feed trust model).
UnitTestSuite "l0.DependencyControl.Toolbox", (macros, dependencies, testExports, controls) ->
  {:shortenUrl, :expandUrl, :formatAge, :buildInstalledDlgList, :promptUntrustedFeed,
    :confirmDialog, :manageExtraFeeds, :manageBlockList, :buttons, :feedActionLabels,
    :sourceKindLabels, :stickinessLabels, :buildLabelChoices, :buildChoiceList, :scanCachedFeeds,
    :addInstalledProviders,
    :scheduleUpdatesAndRegisterTests} = testExports

  -- The UninstallFlow seam: its _setup swaps the DepCtrl class's __call for a constructor returning
  -- `uninstallSeam.record`, so the flow's `DepCtrl(script)\uninstall!` never builds (and registers) a real
  -- record. Held at suite scope because tests don't receive their class's setup context.
  uninstallSeam = {}

  -- The GlobalConfig seam: swaps the class-level DepCtrl.config for an in-memory fake config view so the macro's
  -- reads/writes hit a plain table, restored in the class _teardown. Held at suite scope like uninstallSeam.
  globalConfigSeam = {}
  -- Installs a fresh fake global config for one GlobalConfig test — the class-level _setup runs once, so each test
  -- reinstalls to start from an empty config with its own save spy. Captures the real config once for _teardown.
  setupGlobalConfigFake = ->
    globalConfigSeam.original or= DepCtrl.config
    saved = {}
    fake = {
      c: {updates: {}, feeds: {extraFeeds: {"https://keep.me/feed.json"}}, logging: {}, paths: {}}
      save: => saved.yes = true
    }
    globalConfigSeam.saved = saved
    globalConfigSeam.fake = fake
    DepCtrl.config = fake

  -- a fake config the way buildInstalledDlgList reads it: config.c[section] is the installed-package map
  makeConfig = (section, entries) -> {c: {[section]: entries}}
  -- the set of script display names present in a buildInstalledDlgList map
  namesIn = (map) -> {v.name, true for _, v in pairs map}

  -- Stub aegisub.dialog.display to return a queued sequence of results, one per call: each entry is a
  -- {button, values} pair (or {false} to model a cancel). Exhausting the queue returns nil, which the
  -- redisplay loops read as "close". Returns the stub for call assertions.
  queueDialog = (ut, responses) ->
    i = 0
    ut\stub(aegisub.dialog, "display")\calls (...) ->
      i += 1
      r = responses[i] or {}
      unpack r, 1, #r

  -- Runs one Uninstall Script pass selecting "X v1.0.0", with the constructed record's uninstall returning
  -- the given values; returns how often uninstall ran. Requires the UninstallFlow construction seam.
  runUninstall = (ut, ...) ->
    results = table.pack ...
    ran = 0
    uninstallSeam.record = {
      uninstall: =>
        ran += 1
        unpack results, 1, results.n
    }
    ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {macros: {"a.x": {name: "X", version: "1.0.0"}}, modules: {}}}
    queueDialog ut, {{"OK", {macro: "X v1.0.0", module: ""}}}
    macros["Uninstall Script"].process!
    ran

  -- a fake FeedTrust recording the mutations the trust panels delegate to it; getBlockedFeeds hands back a
  -- fresh copy each call so a panel's in-place sort can't corrupt the source list across redisplay cycles
  makeFeedTrust = (blocked) ->
    calls = {block: {}, unblock: {}, trust: {}, addExtraFeed: {}, removeExtraFeed: {}}
    {
      :calls
      getBlockedFeeds: => [b for b in *(blocked or {})]
      block: (url, opts) => calls.block[#calls.block + 1] = {url, opts}
      unblock: (url) => calls.unblock[#calls.unblock + 1] = url
      trust: (url) => calls.trust[#calls.trust + 1] = url
      addExtraFeed: (url) => calls.addExtraFeed[#calls.addExtraFeed + 1] = url
      removeExtraFeed: (url) => calls.removeExtraFeed[#calls.removeExtraFeed + 1] = url
    }

  -- a minimal fake UpdateFeed exposing one installable automation script, enough for install discovery
  makeFakeFeed = ->
    scriptData = {
      name: "X"
      data: {channels: {main: {version: "1.0.0"}}}
      getChannels: => {"main"}, "main"
    }
    {
      url: "feed://x"
      data: {macros: {"a.x": {channels: {main: {version: "1.0.0"}}}}, modules: {}}
      getScript: (ns, st) => scriptData, nil
      -- mirrors UpdateFeed\walkPackages: yields the package (keyed by .namespace), its script type
      -- and its section, having already skipped the section's non-package template keys
      walkPackages: => coroutine.wrap ->
        coroutine.yield {namespace: "a.x"}, domain.ScriptType.Automation, "macros"
    }

  {
    Plumbing: {
      _description: "Automation-script test wiring: registered macros and testExports reach the suite."

      -- every macro registered through registerMacros is exposed by name, carrying its unhooked process
      receivesRegisteredMacros: (ut) ->
        for name in *{"Install Script", "Update Script", "Uninstall Script", "Manage Feeds", "Macro Configuration"}
          ut\assertNotNil macros[name]
          ut\assertFunction macros[name].process

      -- the script's own internal helpers, passed straight through as testExports
      receivesTestExports: (ut) ->
        ut\assertNotNil testExports
        ut\assertFunction testExports.shortenUrl
        ut\assertFunction testExports.expandUrl
    }

    Urls: {
      _description: "shortenUrl/expandUrl: raw.githubusercontent.com <-> ghuc:// abbreviation."

      -- the raw.githubusercontent.com host collapses to the ghuc:// scheme
      shorten_abbreviatesHost: (ut) ->
        ut\assertEquals shortenUrl("https://raw.githubusercontent.com/TT/DepCtrl/master/x.json"),
          "ghuc://TT/DepCtrl/master/x.json"

      -- a URL on any other host is left untouched
      shorten_passesThroughOther: (ut) ->
        ut\assertEquals shortenUrl("https://example.com/feed.json"), "https://example.com/feed.json"

      -- ghuc:// expands back to the full raw.githubusercontent.com URL
      expand_restoresHost: (ut) ->
        ut\assertEquals expandUrl("ghuc://TT/DepCtrl/master/x.json"),
          "https://raw.githubusercontent.com/TT/DepCtrl/master/x.json"

      -- shorten then expand round-trips a raw.githubusercontent.com URL unchanged
      roundTrips: (ut) ->
        url = "https://raw.githubusercontent.com/TT/DepCtrl/master/x.json"
        ut\assertEquals expandUrl(shortenUrl url), url

      _order: {"shorten_abbreviatesHost", "shorten_passesThroughOther", "expand_restoresHost", "roundTrips"}
    }

    Age: {
      _description: "formatAge: compact 'time since' label for a feed's last-fetch timestamp."

      -- an absent timestamp (never fetched) renders as an em dash
      never_isDash: (ut) -> ut\assertEquals formatAge(nil), "—"

      -- under a minute reads as 'now'; larger spans round down into the coarsest unit that fits
      picksCoarsestUnit: (ut) ->
        now = os.time!
        ut\assertEquals formatAge(now), "now"
        ut\assertEquals formatAge(now - 120), "2m"
        ut\assertEquals formatAge(now - 7200), "2h"
        ut\assertEquals formatAge(now - 172800), "2d"
        ut\assertEquals formatAge(now - 1209600), "2w"
    }

    InstalledList: {
      _description: "buildInstalledDlgList: installed-package picker rows, with the uninstall protected-module guard."

      -- uninstall must not offer DependencyControl's own stack (its namespace is protected)
      uninstall_excludesProtected: (ut) ->
        config = makeConfig "modules", {
          [constants.DEPCTRL_NAMESPACE]: {name: "DepCtrl", version: "0.7.0"}
          "l0.Other": {name: "Other", version: "1.2.3"}
        }
        list, map = buildInstalledDlgList "modules", config, true
        names = namesIn map
        ut\assertNil names["DepCtrl"] -- protected on uninstall
        ut\assertTrue names["Other"]
        ut\assertEquals #list, 1

      -- install/update (not an uninstall) protects nothing, so every installed package is offered
      install_includesAll: (ut) ->
        config = makeConfig "modules", {
          [constants.DEPCTRL_NAMESPACE]: {name: "DepCtrl", version: "0.7.0"}
          "l0.Other": {name: "Other", version: "1.2.3"}
        }
        list, map = buildInstalledDlgList "modules", config, false
        names = namesIn map
        ut\assertTrue names["DepCtrl"]
        ut\assertTrue names["Other"]
        ut\assertEquals #list, 2

      -- rows are sorted case-insensitively by display name
      sortsByNameCaseInsensitively: (ut) ->
        config = makeConfig "macros", {
          "a.z": {name: "Zebra", version: "1.0.0"}
          "a.a": {name: "apple", version: "1.0.0"}
          "a.m": {name: "Mango", version: "1.0.0"}
        }
        list = buildInstalledDlgList "macros", config, false
        ut\assertEquals list, {"apple v1.0.0", "Mango v1.0.0", "Zebra v1.0.0"}

      -- a package with a recorded channel shows the channel in brackets; the package source's channel
      -- wins over the pre-0.7 lastChannel, which stands in alone on an install upgraded from v0.6.3
      formatsRecordedChannel: (ut) ->
        config = makeConfig "macros", {
          "a.x": {name: "X", version: "2.0.0", lastChannel: "beta", currentSource: {channel: "stable"}}
          "a.y": {name: "Y", version: "1.0.0", lastChannel: "beta"}
        }
        list = buildInstalledDlgList "macros", config, false
        ut\assertEquals list[1], "X v2.0.0 [stable]"
        ut\assertEquals list[2], "Y v1.0.0 [beta]"

      -- an entry lacking name/version (e.g. an orphaned unmanaged record) still gets a row
      toleratesMissingVersionAndName: (ut) ->
        config = makeConfig "modules", {"a.orphan": {}}
        list = buildInstalledDlgList "modules", config, false
        ut\assertEquals list[1], "a.orphan v0.0.0"

      _order: {"uninstall_excludesProtected", "install_includesAll", "sortsByNameCaseInsensitively", "formatsRecordedChannel", "toleratesMissingVersionAndName"}
    }

    SourceConfig: {
      _description: "Package Source Configuration helpers: label/value choice lists, the cached-feed scan, and the installed-provider merge."

      -- buildLabelChoices: pairs each enum value with its label, so the dialog offers labels and dispatch
      -- branches on the value behind the chosen one
      buildLabelChoices_pairsLabelsWithValues: (ut) ->
        items, byLabel = buildLabelChoices domain.SourceChoiceStickiness.values, stickinessLabels
        ut\assertEquals #items, #domain.SourceChoiceStickiness.values
        ut\assertEquals byLabel[stickinessLabels[domain.SourceChoiceStickiness.Pinned]],
          domain.SourceChoiceStickiness.Pinned

      -- every source kind and stickiness value carries a label, so no dropdown entry can come out blank
      labels_coverEveryEnumValue: (ut) ->
        ut\assertNotNil sourceKindLabels[kind], "no label for source kind '#{kind}'" for kind in *DepCtrl.UpdateTask.SourceFeedKind.values
        ut\assertNotNil stickinessLabels[sticky], "no label for stickiness '#{sticky}'" for sticky in *domain.SourceChoiceStickiness.values

      -- buildChoiceList: the current value survives even when nothing on offer matches it, flagged so the
      -- user can see their selection no longer holds rather than having it silently swapped
      buildChoiceList_keepsUnofferedCurrentFlagged: (ut) ->
        entries = {{value: "feed://b", label: "b"}, {value: "feed://a", label: "a"}}
        items, byLabel = buildChoiceList entries, "feed://gone", "gone"
        ut\assertEquals #items, 3
        ut\assertContains items[1], "gone" -- the stale entry leads, so it stays visible
        ut\assertEquals byLabel[items[1]], "feed://gone"
        ut\assertEquals items[2], "a" -- the rest stay sorted
        ut\assertEquals items[3], "b"

      -- a current value that is on offer isn't duplicated as a stale entry
      buildChoiceList_offeredCurrentNotDuplicated: (ut) ->
        items = buildChoiceList {{value: "feed://a", label: "a"}}, "feed://a", "a"
        ut\assertEquals #items, 1
        ut\assertEquals items[1], "a"

      -- an Aegisub dropdown can't be empty, so an empty list still yields one entry
      buildChoiceList_emptyGetsPlaceholder: (ut) ->
        items = buildChoiceList {}, nil
        ut\assertEquals #items, 1
        ut\assertString items[1]

      -- scanCachedFeeds: reads only what the cache holds, mapping each feed that offers the package to the
      -- channels it offers it on, and collecting the modules that provide it
      scanCachedFeeds_mapsChannelsAndProviders: (ut) ->
        cached = {
          "feed://one": {
            modules: {
              "l0.Pkg": {channels: {stable: {version: "1.0.0"}, alpha: {version: "1.1.0"}}}
              "l0.Provider": {name: "Provider", provides: {{name: "l0.Pkg"}}}
            }
          }
          "feed://two": {
            modules: {"l0.Other": {name: "Other", provides: {"l0.Pkg"}}}
          }
          "feed://nopkg": {modules: {"l0.Unrelated": {channels: {stable: {}}}}}
        }
        cache = {get: ((url) => cached[url])}
        feedUrls = {"feed://one", "feed://two", "feed://nopkg", "feed://uncached"}
        channelsByFeed, providers = scanCachedFeeds "l0.Pkg", domain.ScriptType.Module, cache, feedUrls
        ut\assertNotNil channelsByFeed["feed://one"]
        ut\assertEquals table.concat(channelsByFeed["feed://one"], ","), "alpha,stable" -- sorted
        ut\assertNil channelsByFeed["feed://nopkg"] -- doesn't offer the package
        ut\assertEquals providers["l0.Provider"].feedUrl, "feed://one"
        ut\assertEquals providers["l0.Other"].feedUrl, "feed://two" -- the bare-string alias form
        ut\assertNil providers["l0.Pkg"] -- a package never provides for itself

      -- addInstalledProviders: folds in installed modules that provide the package, without displacing one
      -- already found in a feed
      addInstalledProviders_mergesWithoutDisplacing: (ut) ->
        providers = {"l0.FromFeed": {name: "FromFeed", feedUrl: "feed://one"}}
        modulesSection = {
          "l0.FromFeed": {name: "Renamed", feed: "feed://local", provides: {"l0.Pkg"}}
          "l0.Installed": {name: "Installed", feed: "feed://local", provides: {{name: "l0.Pkg"}}}
          "l0.Unrelated": {name: "Unrelated", provides: {"yaml"}}
        }
        addInstalledProviders providers, "l0.Pkg", modulesSection
        ut\assertEquals providers["l0.FromFeed"].feedUrl, "feed://one" -- the feed entry wins
        ut\assertEquals providers["l0.Installed"].feedUrl, "feed://local"
        ut\assertNil providers["l0.Unrelated"]

      _order: {
        "buildLabelChoices_pairsLabelsWithValues", "labels_coverEveryEnumValue",
        "buildChoiceList_keepsUnofferedCurrentFlagged", "buildChoiceList_offeredCurrentNotDuplicated",
        "buildChoiceList_emptyGetsPlaceholder",
        "scanCachedFeeds_mapsChannelsAndProviders", "addInstalledProviders_mergesWithoutDisplacing"
      }
    }

    UntrustedPrompt: {
      _description: "promptUntrustedFeed: maps the untrusted-feed dialog choice to a fetch decision + trust side effect."

      -- Trust: remembers the feed and fetches it
      trust_remembersAndFetches: (ut) ->
        ft = makeFeedTrust!
        ut\stub(aegisub.dialog, "display")\calls -> buttons.trust
        ut\assertTrue promptUntrustedFeed "feed://x", ft
        ut\assertEquals ft.calls.trust, {"feed://x"}
        ut\assertEquals #ft.calls.block, 0

      -- Fetch once: fetches without remembering
      fetchOnce_fetchesWithoutTrusting: (ut) ->
        ft = makeFeedTrust!
        ut\stub(aegisub.dialog, "display")\calls -> buttons.fetchOnce
        ut\assertTrue promptUntrustedFeed "feed://x", ft
        ut\assertEquals #ft.calls.trust, 0
        ut\assertEquals #ft.calls.block, 0

      -- Block: blocks the feed and refuses the fetch
      block_blocksAndRefuses: (ut) ->
        ft = makeFeedTrust!
        ut\stub(aegisub.dialog, "display")\calls -> buttons.block
        ut\assertFalse promptUntrustedFeed "feed://x", ft
        ut\assertEquals ft.calls.block[1][1], "feed://x"
        ut\assertEquals #ft.calls.trust, 0

      -- Skip (and dialog cancel): refuses the fetch, changing no trust state
      skip_refusesQuietly: (ut) ->
        ft = makeFeedTrust!
        ut\stub(aegisub.dialog, "display")\calls -> buttons.skip
        ut\assertFalse promptUntrustedFeed "feed://x", ft
        ut\assertEquals #ft.calls.trust, 0
        ut\assertEquals #ft.calls.block, 0

      _order: {"trust_remembersAndFetches", "fetchOnce_fetchesWithoutTrusting", "block_blocksAndRefuses", "skip_refusesQuietly"}
    }

    Confirm: {
      _description: "confirmDialog: a yes/no gate that is true only on Yes."

      yes_isTrue: (ut) ->
        ut\stub(aegisub.dialog, "display")\calls -> buttons.yes
        ut\assertTrue confirmDialog "proceed?"

      no_isFalse: (ut) ->
        ut\stub(aegisub.dialog, "display")\calls -> buttons.no
        ut\assertFalse confirmDialog "proceed?"

      _order: {"yes_isTrue", "no_isFalse"}
    }

    BlockList: {
      _description: "manageBlockList: the block-list panel — remove user blocks, add a block, refuse blocking the bootstrap feed."

      -- Close on the first display makes no changes
      close_makesNoChanges: (ut) ->
        ft = makeFeedTrust {{url: "feed://x", isOfficial: false, matchMode: DepCtrl.FeedTrust.BlockMatchMode.Prefix}}
        queueDialog ut, {{buttons.close}}
        manageBlockList ft
        ut\assertEquals #ft.calls.unblock, 0
        ut\assertEquals #ft.calls.block, 0

      -- a checked user block is unblocked; a checked official block is left alone (it has no remove control)
      removesUserBlockNotOfficial: (ut) ->
        ft = makeFeedTrust {
          {url: "feed://official", isOfficial: true, matchMode: DepCtrl.FeedTrust.BlockMatchMode.Prefix}
          {url: "feed://user", isOfficial: false, matchMode: DepCtrl.FeedTrust.BlockMatchMode.Prefix}
        }
        -- sorted by URL: [feed://official, feed://user] -> remove1 targets the official (ignored), remove2 the user
        queueDialog ut, {{buttons.apply, {remove1: true, remove2: true}}, {buttons.close}}
        manageBlockList ft
        ut\assertEquals ft.calls.unblock, {"feed://user"}

      -- a typed URL is added as a block carrying the chosen mode and reason
      addsBlockWithReason: (ut) ->
        ft = makeFeedTrust {}
        queueDialog ut, {
          {buttons.apply, {newUrl: "ghuc://bad/feed", newMode: DepCtrl.FeedTrust.BlockMatchMode.Exact, newReason: "malware"}}
          {buttons.close}
        }
        manageBlockList ft
        ut\assertEquals ft.calls.block[1][1], "https://raw.githubusercontent.com/bad/feed"
        ut\assertEquals ft.calls.block[1][2].matchMode, DepCtrl.FeedTrust.BlockMatchMode.Exact
        ut\assertEquals ft.calls.block[1][2].reason, "malware"

      -- blocking DependencyControl's own bootstrap feed is refused (it would collapse the trust root)
      refusesBootstrapBlock: (ut) ->
        ft = makeFeedTrust {}
        queueDialog ut, {
          {buttons.apply, {newUrl: constants.DEPCTRL_FEED_URL, newMode: DepCtrl.FeedTrust.BlockMatchMode.Exact}}
          {buttons.close}
        }
        manageBlockList ft
        ut\assertEquals #ft.calls.block, 0

      _order: {"close_makesNoChanges", "removesUserBlockNotOfficial", "addsBlockWithReason", "refusesBootstrapBlock"}
    }

    ExtraFeeds: {
      _description: "manageExtraFeeds: the discovery-roots panel — add a typed feed (expanding ghuc://), close cleanly."

      close_makesNoChanges: (ut) ->
        ft = makeFeedTrust!
        queueDialog ut, {{buttons.close}}
        manageExtraFeeds ft
        ut\assertEquals #ft.calls.addExtraFeed, 0
        ut\assertEquals #ft.calls.removeExtraFeed, 0

      -- a typed ghuc:// shorthand is trimmed and expanded before being added as a discovery root
      addsTypedFeedExpanded: (ut) ->
        ft = makeFeedTrust!
        queueDialog ut, {{buttons.apply, {newFeed: "  ghuc://x/y  "}}, {buttons.close}}
        manageExtraFeeds ft
        ut\assertEquals ft.calls.addExtraFeed, {"https://raw.githubusercontent.com/x/y"}
        ut\assertEquals #ft.calls.removeExtraFeed, 0

      _order: {"close_makesNoChanges", "addsTypedFeedExpanded"}
    }

    Update: {
      _description: "Update Script: picks installed packages and runs update tasks for the selection."

      -- cancelling the picker runs no update task
      cancel_runsNoTask: (ut) ->
        ran = {}
        ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {macros: {"a.x": {name: "X", version: "1.0.0"}}, modules: {}}}
        ut\stub(DepCtrl.updater, "addTask")\calls (self, sd) -> ran[#ran + 1] = sd
        queueDialog ut, {{false}}
        macros["Update Script"].process!
        ut\assertEquals #ran, 0

      -- selecting a macro runs an update task for exactly that package
      selectsMacro_runsItsTask: (ut) ->
        macroEntry = {name: "X", version: "1.0.0"}
        ran = {}
        ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {macros: {"a.x": macroEntry}, modules: {}}}
        ut\stub(DepCtrl.updater, "addTask")\calls (self, sd) ->
          ran[#ran + 1] = sd
          {run: => true}
        queueDialog ut, {{"OK", {macro: "X v1.0.0", module: ""}}}
        macros["Update Script"].process!
        ut\assertEquals #ran, 1
        ut\assertEquals ran[1], macroEntry
    }

    Uninstall: {
      _description: "Uninstall Script: builds the (protected-guarded) installed list and cancels cleanly."

      -- cancelling the picker uninstalls nothing (and never constructs a record to uninstall)
      cancel_uninstallsNothing: (ut) ->
        ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {macros: {"a.x": {name: "X", version: "1.0.0"}}, modules: {}}}
        dlg = queueDialog ut, {{false}}
        macros["Uninstall Script"].process!
        dlg\assertCalledOnce!
    }

    UninstallFlow: {
      _description: "Uninstall Script: a picker selection reaches PackageRecord uninstall, and every result shape is reported without erroring."

      -- A Stub can't stand in for a metamethod (LuaJIT requires __call to be a plain function), so the
      -- construction seam is swapped by hand and restored in the teardown, which runs even when a test fails.
      _setup: (ut) ->
        meta = getmetatable DepCtrl
        uninstallSeam.original = meta.__call
        meta.__call = (cls, script) -> uninstallSeam.record

      _teardown: (ut) ->
        (getmetatable DepCtrl).__call = uninstallSeam.original

      success_reportsAndRuns: (ut) ->
        ut\assertEquals runUninstall(ut, true, {}), 1

      errorWithStringDetail_reported: (ut) ->
        ut\assertEquals runUninstall(ut, nil, "record construction failed"), 1

      errorWithFileTableDetail_formatted: (ut) ->
        ut\assertEquals runUninstall(ut, nil, {["auto/x.moon"]: {nil, "access denied"}}), 1

      lockedFiles_formatted: (ut) ->
        ut\assertEquals runUninstall(ut, false, {["auto/x.moon"]: {false, "file in use"}}), 1

      _order: {"success_reportsAndRuns", "errorWithStringDetail_reported",
        "errorWithFileTableDetail_formatted", "lockedFiles_formatted"}
    }

    Install: {
      _description: "Install Script: crawls feeds for installable packages and runs install tasks for the selection."

      -- an unreachable (unfetched) feed contributes nothing; cancelling installs nothing
      skipsUnfetchedAndCancels: (ut) ->
        ran = {}
        ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {macros: {}, modules: {}}}
        crawl = ut\stub(DepCtrl.FeedInventory.__base, "crawl")\calls -> {{url: "feed://x", fetched: false}}
        ut\stub(DepCtrl.updater, "addTask")\calls (self, sd) -> ran[#ran + 1] = sd
        queueDialog ut, {{false}}
        macros["Install Script"].process!
        crawl\assertCalled!
        ut\assertEquals #ran, 0

      -- a fetched feed's available script is offered, and selecting it runs an install task for it
      selectsAvailableScript: (ut) ->
        ran = {}
        ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {macros: {}, modules: {}}}
        ut\stub(DepCtrl.FeedInventory.__base, "crawl")\calls -> {{url: "feed://x", fetched: true}}
        ut\stub(DepCtrl.FeedLoader.__base, "load")\calls -> makeFakeFeed!
        ut\stub(DepCtrl.updater, "addTask")\calls (self, sd) ->
          ran[#ran + 1] = sd
          {run: => true}
        queueDialog ut, {{"OK", {macro: "X v1.0.0", module: ""}}}
        macros["Install Script"].process!
        ut\assertEquals #ran, 1
        ut\assertEquals ran[1].namespace, "a.x"
        ut\assertEquals ran[1].feed, "feed://x"

      _order: {"skipsUnfetchedAndCancels", "selectsAvailableScript"}
    }

    MacroConfig: {
      _description: "Macro Configuration: writes non-empty edits into the per-macro config and saves."

      appliesEditsAndSaves: (ut) ->
        saved = {}
        cfg = {
          userConfig: {"a.x": {name: "X"}}
          c: {"a.x": {}}
          save: => saved.yes = true
        }
        ut\stub(DepCtrl.config, "getSectionHandler")\returns cfg
        dlg = queueDialog ut, {{"OK", {"a.x.customMenu": "Tools/Mine", "a.x.userFeed": ""}}}
        macros["Macro Configuration"].process!
        ut\assertEquals cfg.c["a.x"].customMenu, "Tools/Mine" -- non-empty edit written
        ut\assertNil cfg.c["a.x"].userFeed -- empty edit left unset
        ut\assertTrue saved.yes
        -- the built dialog is a gap-free array whose first row sits at y=0 (no wasted leading row)
        built = dlg._calls[1][1]
        ut\assertNotNil built[1]
        ut\assertEquals built[1].y, 0
    }

    GlobalConfig: {
      _description: "Global Configuration: seeds effective defaults, writes changes, unsets values equal to default, and never touches feed lists."

      _teardown: (ut) -> DepCtrl.config = globalConfigSeam.original

      -- with nothing configured, every field's widget is present and seeded with its effective default
      buildForm_seedsEveryEffectiveDefault: (ut) ->
        setupGlobalConfigFake!
        dlg = queueDialog ut, {{buttons.cancel}}
        macros["Global Configuration"].process!
        byName = {ctl.name, ctl for ctl in *dlg._calls[1][1] when ctl.name}
        for f in *testExports.configFields
          name = f.crawlLimit and "feeds.crawlLimits.#{f.key}" or "#{f.section}.#{f.key}"
          ctl = byName[name]
          ut\assertNotNil ctl
          seeded = ctl.class == "edit" and ctl.text or ctl.value
          ut\assertEquals seeded, f.default

      -- a stored value overrides the default; widgets use the type-appropriate control class
      buildForm_seedsStoredValueAndTypes: (ut) ->
        setupGlobalConfigFake!
        globalConfigSeam.fake.c.updates.mode = "off"
        dlg = queueDialog ut, {{buttons.cancel}}
        macros["Global Configuration"].process!
        byName = {ctl.name, ctl for ctl in *dlg._calls[1][1] when ctl.name}
        ut\assertEquals byName["updates.mode"].value, "off" -- stored user value, not the default
        ut\assertEquals byName["updates.mode"].class, "dropdown"
        ut\assertEquals byName["updates.checkInterval"].class, "intedit"
        ut\assertEquals byName["updates.blockPrivateHosts"].class, "checkbox"
        ut\assertEquals byName["paths.cache"].class, "edit"

      -- Save writes changed scalar and crawl-budget values and leaves the feed lists untouched
      save_writesChangesKeepsFeeds: (ut) ->
        setupGlobalConfigFake!
        queueDialog ut, {{buttons.save, {"updates.mode": "off", "logging.maxFiles": 50, "feeds.crawlLimits.depth": 3}}}
        macros["Global Configuration"].process!
        cfg = globalConfigSeam.fake.c
        ut\assertEquals cfg.updates.mode, "off"
        ut\assertEquals cfg.logging.maxFiles, 50
        ut\assertEquals cfg.feeds.crawlLimits.depth, 3
        ut\assertEquals cfg.feeds.extraFeeds[1], "https://keep.me/feed.json"
        ut\assertTrue globalConfigSeam.saved.yes

      -- a value left at its effective default is unset, and an all-default crawlLimits object drops out entirely
      save_unsetsValuesEqualToDefault: (ut) ->
        setupGlobalConfigFake!
        globalConfigSeam.fake.c.updates.checkInterval = 999
        queueDialog ut, {{buttons.save, {"updates.checkInterval": DepCtrl.Updater.defaultCheckInterval,
          "feeds.crawlLimits.depth": DepCtrl.FeedInventory.defaultCrawlLimits.depth}}}
        macros["Global Configuration"].process!
        cfg = globalConfigSeam.fake.c
        ut\assertNil cfg.updates.checkInterval
        ut\assertNil cfg.feeds.crawlLimits

      -- Restore Defaults (once confirmed) clears every managed key but preserves the feed lists
      restore_clearsManagedKeepsFeeds: (ut) ->
        setupGlobalConfigFake!
        globalConfigSeam.fake.c.updates.mode = "off"
        globalConfigSeam.fake.c.feeds.crawlLimits = {depth: 2}
        queueDialog ut, {{buttons.restoreDefaults}, {buttons.yes}, {false}}
        macros["Global Configuration"].process!
        cfg = globalConfigSeam.fake.c
        ut\assertNil cfg.updates.mode
        ut\assertNil cfg.feeds.crawlLimits
        ut\assertEquals cfg.feeds.extraFeeds[1], "https://keep.me/feed.json"
        ut\assertTrue globalConfigSeam.saved.yes

      -- Cancel writes nothing
      cancel_writesNothing: (ut) ->
        setupGlobalConfigFake!
        queueDialog ut, {{buttons.cancel}}
        macros["Global Configuration"].process!
        ut\assertNil globalConfigSeam.saved.yes

      _order: {"buildForm_seedsEveryEffectiveDefault", "buildForm_seedsStoredValueAndTypes",
        "save_writesChangesKeepsFeeds", "save_unsetsValuesEqualToDefault",
        "restore_clearsManagedKeepsFeeds", "cancel_writesNothing"}
    }

    ManageFeeds: {
      _description: "Manage Feeds: the redisplay loop dispatches Apply to feed actions and Discover to the crawl."

      -- The offline gather consults the live trust singleton, whose official-set load fetches the DepCtrl
      -- feed over the network on a cold cache; seed both accessors so no test run leaves the process.
      _setup: (ut) ->
        ut\stub(DepCtrl.updater.feedTrust, "getOfficialTrustedFeeds")\returns {}
        ut\stub(DepCtrl.updater.feedTrust, "getOfficialBlockedFeeds")\returns {}

      -- Close on the first render applies no feed action
      close_appliesNothing: (ut) ->
        rows = {{url: "feed://x", provenance: {}, trustStatus: DepCtrl.FeedTrust.TrustStatus.Untrusted, actions: {}, browserUrl: "http://b"}}
        ut\stub(DepCtrl.FeedManager, "buildRows")\calls -> rows
        applyAction = ut\stub DepCtrl.FeedManager.__base, "applyAction"
        queueDialog ut, {{buttons.close}}
        macros["Manage Feeds"].process!
        applyAction\assertNotCalled!

      -- Apply dispatches each row's chosen action to the feed manager
      apply_dispatchesSelectedAction: (ut) ->
        Trust = DepCtrl.FeedManager.FeedAction.Trust
        row = {url: "feed://x", provenance: {}, trustStatus: DepCtrl.FeedTrust.TrustStatus.Untrusted, actions: {Trust}, browserUrl: "http://b"}
        applied = {}
        ut\stub(DepCtrl.FeedManager, "buildRows")\calls -> {row}
        ut\stub(DepCtrl.FeedManager.__base, "applyAction")\calls (self, action, entry) ->
          applied[#applied + 1] = {action, entry}
          true
        queueDialog ut, {{buttons.apply, {action1: feedActionLabels[Trust]}}, {buttons.close}}
        macros["Manage Feeds"].process!
        ut\assertEquals #applied, 1
        ut\assertEquals applied[1][1], Trust
        ut\assertEquals applied[1][2], row

      -- Fetch/Discover switches the list source from the offline gather to the (here stubbed) network crawl
      discover_usesCrawl: (ut) ->
        rows = {{url: "feed://x", provenance: {}, trustStatus: DepCtrl.FeedTrust.TrustStatus.Untrusted, actions: {}, browserUrl: "http://b"}}
        ut\stub(DepCtrl.FeedManager, "buildRows")\calls -> rows
        crawl = ut\stub(DepCtrl.FeedInventory.__base, "crawl")\calls -> {}
        queueDialog ut, {{buttons.discover}, {buttons.close}}
        macros["Manage Feeds"].process!
        crawl\assertCalled!

      -- Block asks for a reason before dispatching, and the reason travels in the action options
      apply_blockPromptsForReason: (ut) ->
        Block = DepCtrl.FeedManager.FeedAction.Block
        row = {url: "feed://x", provenance: {}, trustStatus: DepCtrl.FeedTrust.TrustStatus.Untrusted, actions: {Block}, browserUrl: "http://b"}
        applied = {}
        ut\stub(DepCtrl.FeedManager, "buildRows")\calls -> {row}
        ut\stub(DepCtrl.FeedManager.__base, "applyAction")\calls (self, action, entry, opts) ->
          applied[#applied + 1] = {action, entry, opts}
          true
        ut\stub(DepCtrl.FeedInventory.__base, "getPackagesSourcedFrom")\returns {}
        queueDialog ut, {
          {buttons.apply, {action1: feedActionLabels[Block]}}
          {buttons.block, {reason: "compromised"}}
          {buttons.close}
        }
        macros["Manage Feeds"].process!
        ut\assertEquals #applied, 1
        ut\assertEquals applied[1][1], Block
        ut\assertEquals applied[1][3].reason, "compromised"

      -- declining the sourced-packages warning abandons the block before any prompt or dispatch
      apply_blockDeclinedOnSourcedWarning: (ut) ->
        Block = DepCtrl.FeedManager.FeedAction.Block
        row = {url: "feed://x", provenance: {}, trustStatus: DepCtrl.FeedTrust.TrustStatus.Untrusted, actions: {Block}, browserUrl: "http://b"}
        ut\stub(DepCtrl.FeedManager, "buildRows")\calls -> {row}
        applyAction = ut\stub DepCtrl.FeedManager.__base, "applyAction"
        ut\stub(DepCtrl.FeedInventory.__base, "getPackagesSourcedFrom")\returns {"a.pkg"}
        queueDialog ut, {
          {buttons.apply, {action1: feedActionLabels[Block]}}
          {false} -- the sourced warning, declined
          {buttons.close}
        }
        macros["Manage Feeds"].process!
        applyAction\assertNotCalled!

      _order: {"close_appliesNothing", "apply_dispatchesSelectedAction", "discover_usesCrawl",
        "apply_blockPromptsForReason", "apply_blockDeclinedOnSourcedWarning"}
    }

    StartupSweep: {
      _description: "scheduleUpdatesAndRegisterTests: the startup sweep schedules every record, registers module test menus, and survives per-record failures."

      -- Every registered record gets a schedule attempt. A module whose test suite initialized gets its
      -- test menu registered, while an uninitialized suite and automation scripts (which register their
      -- own) are skipped. An installed module that fails to load is tolerated, and the lock is released.
      sweepsRecordsAndRegistersModuleTests: (ut) ->
        ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {modules: {"toolbox.test.notARealModule": {}}}}
        registered = {}
        records = {
          {name: "Mod", namespace: "a.mod", scriptType: domain.ScriptType.Module, testSuiteInitialized: true, tests: {registerMacros: => registered.mdl = true}}
          {name: "Broken", namespace: "a.broken", scriptType: domain.ScriptType.Module, tests: {registerMacros: => registered.broken = true}}
          {name: "Mac", namespace: "a.mac", scriptType: domain.ScriptType.Automation, tests: {registerMacros: => registered.macro = true}}
        }
        ut\stub(DepCtrl, "getAllRegisteredRecords")\returns records
        scheduled = {}
        ut\stub(DepCtrl.updater, "scheduleUpdate")\calls (self, record) ->
          scheduled[#scheduled + 1] = record.namespace
          0
        released = ut\stub DepCtrl.updater, "releaseLock"
        scheduleUpdatesAndRegisterTests!
        ut\assertEquals #scheduled, 3
        ut\assertTrue registered.mdl -- an initialized module suite is registered by the sweep
        ut\assertNil registered.broken -- a module whose suite failed to initialize is skipped
        ut\assertNil registered.macro -- automation scripts register their own
        released\assertCalledOnce!

      -- one record's raising scheduleUpdate (or a negative status) doesn't end the sweep for the rest
      toleratesPerRecordFailures: (ut) ->
        ut\stub(DepCtrl.config, "getSectionHandler")\returns {c: {modules: {}}}
        records = {
          {name: "Boom", namespace: "a.boom", scriptType: domain.ScriptType.Module}
          {name: "Denied", namespace: "a.denied", scriptType: domain.ScriptType.Module}
          {name: "Fine", namespace: "a.fine", scriptType: domain.ScriptType.Automation}
        }
        ut\stub(DepCtrl, "getAllRegisteredRecords")\returns records
        calls = 0
        ut\stub(DepCtrl.updater, "scheduleUpdate")\calls (self, record) ->
          calls += 1
          error "scheduler exploded" if record.name == "Boom"
          record.name == "Denied" and -1 or 0
        ut\stub DepCtrl.updater, "releaseLock"
        scheduleUpdatesAndRegisterTests!
        ut\assertEquals calls, 3

      _order: {"sweepsRecordsAndRegistersModuleTests", "toleratesPerRecordFailures"}
    }
  }
