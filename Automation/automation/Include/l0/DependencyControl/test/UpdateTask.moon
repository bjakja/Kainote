-- UpdateTask tests: extracted from the main test suite.
-- Called from test.moon as: (controls\requireTest "UpdateTask") stubHelpers
(stubHelpers) ->
  domain = require "l0.DependencyControl.domain"
  environment = require "l0.DependencyControl.environment"
  UpdateTask = require "l0.DependencyControl.UpdateTask"
  UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
  SemanticVersion = require "l0.DependencyControl.SemanticVersion"
  fileOps = require "l0.DependencyControl.file-ops"
  UpdateFeed = require "l0.DependencyControl.UpdateFeed"
  Downloader = require "l0.DependencyControl.Downloader"
  ModuleLoader = require "l0.DependencyControl.ModuleLoader"
  ScriptUpdateRecord = require "l0.DependencyControl.ScriptUpdateRecord"
  FeedTrust = require "l0.DependencyControl.FeedTrust"
  {:stubSelf, :makeNullLogger, :makeSeededFeedTrust} = stubHelpers

  UpdateStatus = UpdateTask.UpdateStatus
  ContextCeiling = UpdateTask.ContextCeiling
  UpdateReason = UpdateTask.UpdateReason
  SourceChoiceStickiness = UpdateTask.SourceChoiceStickiness
  SourceFeedKind = UpdateTask.SourceFeedKind
  FeedTrustDecision = UpdateTask.FeedTrustDecision
  -- dialog button labels are module-private; the test suite reads them through the test-export seam
  msgs = UnitTestSuite\getTestExports(UpdateTask).msgs

  -- A candidate as pooled in run(): a feed record (with the fields __selectCandidate reads — version
  -- string, namespace, checkPlatform predicate, files) plus its trust band and feed URL.
  -- opts: namespace, feedUrl, platform (false to fail platform), platforms (offered-platform list), files (set {} to exclude).
  makeCandidate = (band, version, opts = {}) ->
    {
      trustBand: band
      feedUrl: opts.feedUrl or "feed://test"
      isDirect: opts.isDirect != false
      providesVersion: opts.providesVersion
      updateRecord: {
        namespace: opts.namespace or "l0.candidate"
        :version
        files: opts.files == nil and {{}} or opts.files
        checkPlatform: -> opts.platform != false
        platforms: opts.platforms
      }
    }

  -- A stub task self for __selectCandidate: it consults targetVersion, logger, record.feed (the
  -- declared-feed tie-break) and record.namespace (the ambiguity log). The metatable lets __selectCandidate
  -- resolve its self-call to __getCandidateRankVersion.
  makeSelectTask = (targetVersion, opts = {}) ->
    stubSelf UpdateTask, {
      :targetVersion
      record: {namespace: "json", feed: opts.declaredFeed}
      logger: {log: (_, ...) -> opts.logged[#opts.logged + 1] = {...} if opts.logged}
      __class: UpdateTask
    }

  -- A stub task self for the currentSource helpers (__resolveRememberedFeedUrl, __matchRememberedCandidate,
  -- __feedSourceOf, __persistSource). opts: feed (declared feed URL), userFeed, channel, targetVersion,
  -- currentSource (an existing persisted record), modules (installed-module config for provider feed
  -- lookup), onSave (called by record.config\save!). The metatable resolves the helpers' self-calls.
  makeSourceTask = (opts = {}) ->
    stubSelf UpdateTask, {
      targetVersion: opts.targetVersion or 0
      channel: opts.channel
      record: {
        feed: opts.feed
        config: {
          c: {userFeed: opts.userFeed, currentSource: opts.currentSource}
          save: (=> opts.onSave! if opts.onSave)
        }
      }
      updater: {config: {getSectionHandler: (_, section) -> {c: opts.modules or {}}}}
      __class: UpdateTask
    }

  -- A stub task self for __shouldPrompt and the prompt methods. opts: reason (the task's UpdateReason,
  -- for __shouldPrompt), trustedFeeds, blockedFeeds, onSave (called by config\save!). __promptTrustFeed routes
  -- its trust/block through @updater.feedTrust, so the stub carries a real FeedTrust over the same config.
  makeInteractiveTask = (opts = {}) ->
    config = {c: {feeds: {trustedFeeds: opts.trustedFeeds, blockedFeeds: opts.blockedFeeds}, updates: {}}, save: (=> opts.onSave! if opts.onSave)}
    feedTrust = makeSeededFeedTrust {:config}
    stubSelf UpdateTask, {
      reason: opts.reason
      record: {name: "TestMod", namespace: "l0.testMod", virtual: true, scriptType: domain.ScriptType.Module}
      logger: makeNullLogger!
      updater: {:config, :feedTrust}
      __class: UpdateTask
    }

  -- resolve() drives all feed I/O through @__loadFeed/@checkFeed and all prompting through @__shouldPrompt
  -- /@__promptSelectPackageSource/@__promptTrustFeed; makeResolveTask stubs those so a test can place exactly
  -- the candidates each cascade tier should see and script the prompt outcomes. `feeds` maps a feedUrl to
  -- { direct?: <directRec>, providers?: {<providerRec>,...} }; a feed absent from the map fails to load.
  directRec = (spec) -> {
    version: spec.version, namespace: spec.namespace or "json", activeChannel: spec.channel or "release"
    files: spec.files == nil and {{}} or spec.files, checkPlatform: -> spec.platform != false
    platforms: spec.platforms
  }
  providerRec = (spec) -> {
    namespace: spec.namespace, provides: {{name: "json", version: spec.providesVersion}}
    files: spec.files == nil and {{}} or spec.files, checkPlatform: -> spec.platform != false
  }

  -- opts: declaredFeed, feeds, currentSource, userFeed, addFeeds, optional, virtual (default true),
  -- targetVersion, version (installed), modules (for provider feed lookup), officialTrusted/officialBlocked,
  -- config {extraFeeds, trustedFeeds, blockedFeeds, offerAllSources, *PromptThreshold}, allowPrompt (gates
  -- both prompts), selectReturn {pick, stickiness} (nil pick = abort), trustReturn (a FeedTrustDecision).
  makeResolveTask = (opts = {}) ->
    cfg = opts.config or {}
    calls = {select: 0, trust: 0, loaded: {}}
    updaterConfig = {
      c: {
        feeds: {extraFeeds: cfg.extraFeeds, trustedFeeds: cfg.trustedFeeds, blockedFeeds: cfg.blockedFeeds}
        updates: {
          offerAllSources: cfg.offerAllSources
          packageChoicePromptThreshold: cfg.packageChoicePromptThreshold or ContextCeiling.UserRequested
          feedTrustPromptThreshold: cfg.feedTrustPromptThreshold or ContextCeiling.UserRequested
        }
      }
      getSectionHandler: (_, section) -> {c: opts.modules or {}}
    }
    -- a real FeedTrust seeded with the official sets (so it never loads the live DepCtrl feed) over the
    -- updater config, exactly as Updater wires it
    feedTrust = makeSeededFeedTrust {
      config: updaterConfig
      official: {trusted: opts.officialTrusted or {}, blocked: opts.officialBlocked or {}}
    }
    task = stubSelf UpdateTask, {
      __class: UpdateTask
      :calls
      targetVersion: opts.targetVersion or 0
      optional: opts.optional
      reason: opts.reason
      channel: opts.channel
      addFeeds: opts.addFeeds or {}
      _feeds: opts.feeds or {}
      record: {
        feed: opts.declaredFeed
        namespace: "json"
        name: "json"
        version: opts.version or 0
        virtual: opts.virtual != false
        scriptType: domain.ScriptType.Module
        config: {c: {userFeed: opts.userFeed, currentSource: opts.currentSource}}
      }
      updater: {renewLock: ->, :feedTrust, config: updaterConfig}
      logger: makeNullLogger!
      __loadFeed: (url) =>
        calls.loaded[url] = true
        return nil, "feed not found: #{url}" unless @_feeds[url]
        providers = @_feeds[url].providers or {}
        {__url: url, getProviders: (=> providers)}
      checkFeed: (feed) =>
        d = @_feeds[feed.__url].direct
        return nil, nil unless d
        return d, nil, SemanticVersion\toPacked d.version
      __shouldPrompt: (threshold) => opts.allowPrompt and true or false
      __promptSelectPackageSource: (choices, preselect, noLongerAvail) =>
        calls.select += 1
        unpack(opts.selectReturn or {})
      __promptTrustFeed: (selected) =>
        calls.trust += 1
        opts.trustReturn
    }
    task

  -- A stub task self for run() itself. Its fake __class carries the mockable download engine for
  -- the internet check; __resolve returns a scripted resolution and the dispatch targets record their
  -- calls in `calls`. The metatable resolves run()'s own __base methods (e.g. __logUpdateError).
  -- opts: resolution; running, updated, virtual (default true), isUserPath, online (default true),
  -- lockOk (default true), installedSatisfies (drives record\checkVersion), targetVersion;
  -- performUpdateReturn {code, detail}, installProviderReturn {ref, code, detail}.
  makeRunTask = (opts = {}) ->
    calls = {}
    stubSelf UpdateTask, {
      __class: {__downloader: {isInternetConnected: (=> opts.online != false)}}
      :calls
      running: opts.running
      updated: opts.updated
      targetVersion: opts.targetVersion or 0
      record: {
        name: "TestMod", namespace: "l0.TestMod", scriptType: domain.ScriptType.Module, version: 0
        virtual: opts.virtual != false
        getEntryPointPath: (=> "entry/path", opts.isUserPath)
        checkVersion: (=> opts.installedSatisfies and true or false)
      }
      updater: {acquireLock: (=> opts.lockOk != false, "otherHost")}
      logger: makeNullLogger!
      __resolve: =>
        calls.resolved = true
        opts.resolution
      __persistSource: (sel, st) => calls.persisted = {selected: sel, stickiness: st}
      performUpdate: (update) =>
        calls.performUpdate = update
        unpack(opts.performUpdateReturn or {1, "performed"})
      __installProvider: (rec, url) =>
        calls.installProvider = {:rec, :url}
        unpack(opts.installProviderReturn or {})
    }

  -- A stub task self for __installProvider: its fake __class carries a mockable DependencyControl
  -- constructor (here one that wraps its ctor args for inspection), and updater.require records its call
  -- in `opts.capture` and returns opts.requireReturn.
  makeProviderTask = (opts = {}) ->
    cap = opts.capture or {}
    stubSelf UpdateTask, {
      __class: {__DependencyControl: opts.depCtrl}
      addFeeds: opts.addFeeds or {}
      targetVersion: opts.targetVersion or 0
      optional: opts.optional
      reason: opts.reason
      updater: {
        require: (record, targetVersion, addFeeds, optional, channel, reason) =>
          cap.record, cap.addFeeds = record, addFeeds
          cap.targetVersion, cap.optional, cap.reason = targetVersion, optional, reason
          unpack(opts.requireReturn or {})
      }
    }

  -- A stub task self for performUpdate: a non-virtual record (so the dummy-ref + requiredModules preamble
  -- is skipped) and a fake __class carrying a stub download engine. FileOps/UpdateFeed statics are
  -- stubbed per-test. opts: downloadStatus (the status each created download ends up in), addDownloadFails.
  makePerformTask = (opts = {}) ->
    engine = {downloads: {}}
    engine.clear = (self) -> self.downloads = {}
    engine.addDownload = (self, url, outfile, sha1) ->
      return nil, "add failed: #{url}" if opts.addDownloadFails
      d = {:url, :outfile, :sha1, error: "download boom"
        status: opts.downloadStatus or Downloader.Download.Status.Finished}
      self.downloads[#self.downloads + 1] = d
      d
    engine.await = (self, cb) -> cb nil, 1
    stubSelf UpdateTask, {
      __class: {__downloader: engine, __DependencyControl: {__name: "DependencyControl"}}
      running: false
      updated: false
      targetVersion: 0
      record: {
        name: "TestMod", namespace: "l0.test", automationDir: "auto", version: 0
        scriptType: domain.ScriptType.Module, virtual: false
      }
      updater: {renewLock: (=>), config: {c: {updates: {}}}}
      logger: makeNullLogger!
    }

  {
    _description: "Tests for UpdateTask: candidate selection/ranking, interactivity gating, the trust/choice prompts, feed-prefix matching, and installed-provider lookup."

    -- UpdateTask.__selectCandidate: ranks the pooled candidates by trust band, then version, then a
    -- deterministic tie-break (declared feed, then namespace); returns the winner or nil if none is eligible.

    -- eligibility: release version must meet the target version
    selectCandidate_filtersByVersion: (ut) ->
      task = makeSelectTask SemanticVersion\toPacked "1.0.0"
      chosen = UpdateTask.__selectCandidate task, {makeCandidate(2, "0.9.0", namespace: "l0.old"), makeCandidate(2, "1.5.0", namespace: "l0.ok")}
      ut\assertNotNil chosen
      ut\assertEquals chosen.updateRecord.namespace, "l0.ok"

    selectCandidate_noneSatisfiesVersion: (ut) ->
      task = makeSelectTask SemanticVersion\toPacked "3.0.0"
      ut\assertNil UpdateTask.__selectCandidate task, {makeCandidate(2, "1.0.0"), makeCandidate(2, "2.9.0")}

    -- eligibility: a candidate whose channel can't run on the current platform is skipped
    selectCandidate_skipsUnsupportedPlatform: (ut) ->
      task = makeSelectTask 0
      chosen = UpdateTask.__selectCandidate task,
        {makeCandidate(2, "2.0.0", namespace: "l0.win", platform: false), makeCandidate(2, "1.0.0", namespace: "l0.any")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.any"

    -- eligibility: a candidate with no files to install is skipped
    selectCandidate_skipsEmptyFiles: (ut) ->
      task = makeSelectTask 0
      chosen = UpdateTask.__selectCandidate task,
        {makeCandidate(2, "2.0.0", namespace: "l0.noFiles", files: {}), makeCandidate(2, "1.0.0", namespace: "l0.ok")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.ok"

    selectCandidate_noneEligible: (ut) ->
      task = makeSelectTask 0
      ut\assertNil UpdateTask.__selectCandidate task, {makeCandidate(2, "2.0.0", platform: false)}

    -- a lower (more trusted) band wins even against a higher-version candidate in a higher band
    selectCandidate_lowerBandBeatsHigherVersion: (ut) ->
      task = makeSelectTask 0
      chosen = UpdateTask.__selectCandidate task,
        {makeCandidate(4, "9.9.9", namespace: "l0.untrusted"), makeCandidate(1, "1.0.0", namespace: "l0.trusted")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.trusted"
      ut\assertEquals chosen.trustBand, 1

    selectCandidate_highestVersionWithinBand: (ut) ->
      task = makeSelectTask 0
      chosen = UpdateTask.__selectCandidate task, {makeCandidate(2, "1.0.0", namespace: "l0.a"), makeCandidate(2, "2.0.0", namespace: "l0.b")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.b"

    -- same band + version: the candidate from the declared feed wins (even with a higher namespace)
    selectCandidate_declaredFeedTiebreak: (ut) ->
      task = makeSelectTask 0, declaredFeed: "feed://declared"
      chosen = UpdateTask.__selectCandidate task,
        {makeCandidate(2, "2.0.0", namespace: "l0.a", feedUrl: "feed://other"), makeCandidate(2, "2.0.0", namespace: "l0.z", feedUrl: "feed://declared")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.z"

    -- same band + version, neither declared: lexicographically lowest namespace wins; the tie is logged
    selectCandidate_namespaceTiebreakLogged: (ut) ->
      logged = {}
      task = makeSelectTask 0, logged: logged
      chosen = UpdateTask.__selectCandidate task,
        {makeCandidate(3, "2.0.0", namespace: "l0.b"), makeCandidate(3, "2.0.0", namespace: "l0.a")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.a"
      ut\assertTrue #logged > 0

    selectCandidate_unambiguousNoLog: (ut) ->
      logged = {}
      task = makeSelectTask 0, logged: logged
      chosen = UpdateTask.__selectCandidate task, {makeCandidate(1, "1.0.0", namespace: "l0.only")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.only"
      ut\assertEquals #logged, 0

    -- a provider is judged by the alias range it declares, not its own release version: the unrelated
    -- release 9.9.9 is ignored, and ~1.2 still covers the 1.2.4 target
    selectCandidate_providesVersionRangeSatisfies: (ut) ->
      task = makeSelectTask SemanticVersion\toPacked "1.2.4"
      chosen = UpdateTask.__selectCandidate task, {makeCandidate(3, "9.9.9", namespace: "l0.prov", isDirect: false, providesVersion: "~1.2")}
      ut\assertNotNil chosen
      ut\assertEquals chosen.updateRecord.namespace, "l0.prov"

    -- conversely, a high release version can't rescue a provider once its declared range no longer reaches the target
    selectCandidate_providesVersionRangeRejects: (ut) ->
      task = makeSelectTask SemanticVersion\toPacked "1.5.0"
      ut\assertNil UpdateTask.__selectCandidate task, {makeCandidate(3, "9.9.9", namespace: "l0.prov", isDirect: false, providesVersion: "~1.2")}

    -- a target below the declared range stays satisfiable: the provider can still supply a version >= target
    selectCandidate_providesVersionRangeAboveTarget: (ut) ->
      task = makeSelectTask SemanticVersion\toPacked "1.0.0"
      chosen = UpdateTask.__selectCandidate task, {makeCandidate(3, "1.0.0", namespace: "l0.prov", isDirect: false, providesVersion: "~1.2")}
      ut\assertNotNil chosen
      ut\assertEquals chosen.updateRecord.namespace, "l0.prov"

    -- a provider that declares no range stands in for any version
    selectCandidate_providerNoRangeMatchesAny: (ut) ->
      task = makeSelectTask SemanticVersion\toPacked "5.0.0"
      chosen = UpdateTask.__selectCandidate task, {makeCandidate(3, "1.0.0", namespace: "l0.prov", isDirect: false)}
      ut\assertNotNil chosen
      ut\assertEquals chosen.updateRecord.namespace, "l0.prov"

    -- among providers, the release version doesn't drive rank: the wider declared range (higher covered
    -- version) wins even though its provider has the lower release version
    selectCandidate_providerRankedByRangeMaxNotRelease: (ut) ->
      task = makeSelectTask SemanticVersion\toPacked "1.0.0"
      chosen = UpdateTask.__selectCandidate task,
        {makeCandidate(3, "9.9.9", namespace: "l0.a", isDirect: false, providesVersion: "^1"),
          makeCandidate(3, "1.0.0", namespace: "l0.b", isDirect: false, providesVersion: "^2")}
      ut\assertEquals chosen.updateRecord.namespace, "l0.b"

    -- __selectCandidate's 2nd return is the set of candidates tied with the winner (for the chooser)
    selectCandidate_returnsTiedSet: (ut) ->
      task = makeSelectTask 0
      _, tied = UpdateTask.__selectCandidate task,
        {makeCandidate(3, "1.0.0", namespace: "l0.a"), makeCandidate(3, "1.0.0", namespace: "l0.b"),
          makeCandidate(3, "1.0.0", namespace: "l0.c", platform: false)}
      ut\assertEquals #tied, 2 -- the platform-ineligible one is excluded

    -- UpdateTask.isWithinContextCeiling: the shared context-ladder gate — each ceiling admits exactly the
    -- contexts at or below its rung; off, unset, and unrecognized ceilings admit none
    isWithinContextCeiling_ranksLadder: (ut) ->
      ut\assertTrue UpdateTask.isWithinContextCeiling UpdateReason.UserRequested, ContextCeiling.UserRequested
      ut\assertFalse UpdateTask.isWithinContextCeiling UpdateReason.DependencyResolution, ContextCeiling.UserRequested
      ut\assertTrue UpdateTask.isWithinContextCeiling UpdateReason.AutoUpdate, ContextCeiling.AutoUpdate
      ut\assertFalse UpdateTask.isWithinContextCeiling UpdateReason.UserRequested, ContextCeiling.Off
      ut\assertFalse UpdateTask.isWithinContextCeiling UpdateReason.UserRequested, nil
      ut\assertFalse UpdateTask.isWithinContextCeiling UpdateReason.UserRequested, "garbage"

    -- UpdateTask.__shouldPrompt: a task may prompt only when its reason is permitted by the given threshold

    shouldPrompt_withinThreshold: (ut) ->
      ut\assertTrue UpdateTask.__shouldPrompt makeInteractiveTask({reason: UpdateReason.DependencyResolution}), ContextCeiling.DependencyResolution
      ut\assertTrue UpdateTask.__shouldPrompt makeInteractiveTask({reason: UpdateReason.UserRequested}), ContextCeiling.AutoUpdate

    shouldPrompt_aboveThreshold: (ut) ->
      ut\assertFalse UpdateTask.__shouldPrompt makeInteractiveTask({reason: UpdateReason.AutoUpdate}), ContextCeiling.UserRequested

    shouldPrompt_offNeverPrompts: (ut) ->
      ut\assertFalse UpdateTask.__shouldPrompt makeInteractiveTask({reason: UpdateReason.UserRequested}), ContextCeiling.Off

    shouldPrompt_noReason: (ut) ->
      ut\assertFalse UpdateTask.__shouldPrompt makeInteractiveTask({reason: nil}), ContextCeiling.AutoUpdate

    -- feedTrustPromptThreshold defaults to the auto-update ceiling, so an untrusted-feed prompt still shows
    -- during a background (auto-update) install; a user-requested ceiling (see shouldPrompt_aboveThreshold)
    -- would suppress it.
    shouldPrompt_feedTrustDefaultAllowsAutoUpdate: (ut) ->
      ut\assertEquals UpdateTask.defaultFeedTrustPromptThreshold, ContextCeiling.AutoUpdate
      ut\assertTrue UpdateTask.__shouldPrompt makeInteractiveTask({reason: UpdateReason.AutoUpdate}), UpdateTask.defaultFeedTrustPromptThreshold

    -- UpdateTask.__promptTrustFeed: shows the dialog (callers gate it); "always" trusts the feed, "never"
    -- blocks it, and a cancelled prompt returns nil.

    promptTrustFeed_trustOnce: (ut) ->
      task = makeInteractiveTask!
      ut\stub(aegisub.dialog, "display")\calls -> msgs.__promptTrustFeed.trustOnce
      ut\assertEquals UpdateTask.__promptTrustFeed(task, {feedUrl: "feed://x"}), FeedTrustDecision.Once

    promptTrustFeed_cancelReturnsNil: (ut) ->
      task = makeInteractiveTask!
      ut\stub(aegisub.dialog, "display")\calls -> msgs.dialogCommon.cancel
      ut\assertNil UpdateTask.__promptTrustFeed(task, {feedUrl: "feed://x"})

    promptTrustFeed_trustAlwaysPersists: (ut) ->
      saved = {}
      task = makeInteractiveTask trustedFeeds: {}, onSave: -> saved[1] = true
      ut\stub(aegisub.dialog, "display")\calls -> msgs.__promptTrustFeed.trustAlways
      ut\assertEquals UpdateTask.__promptTrustFeed(task, {feedUrl: "feed://new"}), FeedTrustDecision.Always
      ut\assertEquals task.updater.config.c.feeds.trustedFeeds[1], "feed://new"
      ut\assertTrue saved[1]

    promptTrustFeed_neverBlocks: (ut) ->
      saved = {}
      task = makeInteractiveTask blockedFeeds: {}, onSave: -> saved[1] = true
      ut\stub(aegisub.dialog, "display")\calls -> msgs.__promptTrustFeed.trustNever
      ut\assertEquals UpdateTask.__promptTrustFeed(task, {feedUrl: "feed://bad"}), FeedTrustDecision.Never
      ut\assertEquals task.updater.config.c.feeds.blockedFeeds[1], {url: "feed://bad", matchMode: "prefix"}
      ut\assertTrue saved[1]

    -- UpdateTask.__promptSelectPackageSource: shows the dialog (callers gate it), returning the picked
    -- candidate and the chosen stickiness; an "auto" pick keeps the winner and an "abort" returns nil.

    promptSelectPackageSource_picksSelection: (ut) ->
      task = makeInteractiveTask!
      winner = {feedUrl: "feed://a", updateRecord: {namespace: "l0.a", name: "A"}}
      other = {feedUrl: "feed://b", updateRecord: {namespace: "l0.b", name: "B"}}
      ut\stub(aegisub.dialog, "display")\calls -> msgs.promptSelectSource.retain, {choice: "B (feed://b)"}
      chosen, stickiness = UpdateTask.__promptSelectPackageSource task, {winner, other}, winner
      ut\assertEquals chosen, other
      ut\assertEquals stickiness, SourceChoiceStickiness.Retain

    promptSelectPackageSource_autoKeepsWinner: (ut) ->
      task = makeInteractiveTask!
      winner = {feedUrl: "feed://a", updateRecord: {namespace: "l0.a", name: "A"}}
      other = {feedUrl: "feed://b", updateRecord: {namespace: "l0.b", name: "B"}}
      -- "Let DepCtrl Decide" ignores the dropdown and keeps the algorithm's pick
      ut\stub(aegisub.dialog, "display")\calls -> msgs.promptSelectSource.auto, {choice: "B (feed://b)"}
      chosen, stickiness = UpdateTask.__promptSelectPackageSource task, {winner, other}, winner
      ut\assertEquals chosen, winner
      ut\assertEquals stickiness, SourceChoiceStickiness.Auto

    promptSelectPackageSource_abortReturnsNil: (ut) ->
      task = makeInteractiveTask!
      winner = {feedUrl: "feed://a", updateRecord: {namespace: "l0.a", name: "A"}}
      other = {feedUrl: "feed://b", updateRecord: {namespace: "l0.b", name: "B"}}
      ut\stub(aegisub.dialog, "display")\calls -> msgs.promptSelectSource.abort, {}
      ut\assertNil UpdateTask.__promptSelectPackageSource task, {winner, other}, winner

    -- UpdateTask.__resolveRememberedFeedUrl: derives the feed URL of a remembered source for every kind
    -- but `other` (which stores it), so a remembered choice survives a feed-URL migration.

    resolveRememberedFeedUrl_selfDeclared: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      ut\assertEquals UpdateTask.__resolveRememberedFeedUrl(task, {feedSource: SourceFeedKind.SelfDeclared}), "feed://declared"

    resolveRememberedFeedUrl_userFeed: (ut) ->
      task = makeSourceTask userFeed: "feed://user"
      ut\assertEquals UpdateTask.__resolveRememberedFeedUrl(task, {feedSource: SourceFeedKind.UserFeed}), "feed://user"

    resolveRememberedFeedUrl_other: (ut) ->
      task = makeSourceTask!
      ut\assertEquals UpdateTask.__resolveRememberedFeedUrl(task, {feedSource: SourceFeedKind.Other, feedUrl: "feed://third"}), "feed://third"

    resolveRememberedFeedUrl_provider: (ut) ->
      task = makeSourceTask modules: {"l0.prov": {feed: "feed://prov"}}
      ut\assertEquals UpdateTask.__resolveRememberedFeedUrl(task, {feedSource: SourceFeedKind.Provider, provider: {namespace: "l0.prov"}}), "feed://prov"

    resolveRememberedFeedUrl_providerMissing: (ut) ->
      task = makeSourceTask modules: {}
      ut\assertNil UpdateTask.__resolveRememberedFeedUrl task, {feedSource: SourceFeedKind.Provider, provider: {namespace: "l0.gone"}}

    -- UpdateTask.__matchRememberedCandidate: finds the pooled candidate corresponding to the remembered
    -- source, but only when it's still eligible to satisfy the task.

    matchRememberedCandidate_direct: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      candidates = {
        makeCandidate(2, "1.0.0", feedUrl: "feed://other", namespace: "l0.x")
        makeCandidate(1, "1.0.0", feedUrl: "feed://declared", namespace: "l0.x")
      }
      m = UpdateTask.__matchRememberedCandidate task, candidates, {feedSource: SourceFeedKind.SelfDeclared}
      ut\assertNotNil m
      ut\assertEquals m.feedUrl, "feed://declared"

    matchRememberedCandidate_provider: (ut) ->
      task = makeSourceTask modules: {"l0.prov": {feed: "feed://prov"}}
      candidates = {makeCandidate(3, "0.1.0", feedUrl: "feed://prov", isDirect: false, namespace: "l0.prov", providesVersion: "*")}
      m = UpdateTask.__matchRememberedCandidate task, candidates, {feedSource: SourceFeedKind.Provider, provider: {namespace: "l0.prov"}}
      ut\assertNotNil m
      ut\assertFalse m.isDirect

    matchRememberedCandidate_ineligibleVersion: (ut) ->
      task = makeSourceTask feed: "feed://declared", targetVersion: SemanticVersion\toPacked "5.0.0"
      candidates = {makeCandidate(1, "1.0.0", feedUrl: "feed://declared")}
      ut\assertNil UpdateTask.__matchRememberedCandidate task, candidates, {feedSource: SourceFeedKind.SelfDeclared}

    matchRememberedCandidate_noUrlMatch: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      candidates = {makeCandidate(1, "1.0.0", feedUrl: "feed://elsewhere")}
      ut\assertNil UpdateTask.__matchRememberedCandidate task, candidates, {feedSource: SourceFeedKind.SelfDeclared}

    -- UpdateTask.__feedSourceOf: classifies a chosen candidate's source kind for persistence.

    feedSourceOf_provider: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      ut\assertEquals UpdateTask.__feedSourceOf(task, {isDirect: false, feedUrl: "feed://prov"}), SourceFeedKind.Provider

    feedSourceOf_selfDeclared: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      ut\assertEquals UpdateTask.__feedSourceOf(task, {isDirect: true, feedUrl: "feed://declared"}), SourceFeedKind.SelfDeclared

    feedSourceOf_userFeed: (ut) ->
      task = makeSourceTask feed: "feed://declared", userFeed: "feed://user"
      ut\assertEquals UpdateTask.__feedSourceOf(task, {isDirect: true, feedUrl: "feed://user"}), SourceFeedKind.UserFeed

    feedSourceOf_other: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      ut\assertEquals UpdateTask.__feedSourceOf(task, {isDirect: true, feedUrl: "feed://third"}), SourceFeedKind.Other

    -- UpdateTask.__persistSource: records the resolved source and stickiness, writing only when changed.

    persistSource_writesDirect: (ut) ->
      saved = {}
      task = makeSourceTask feed: "feed://declared", onSave: -> saved[1] = true
      selected = {isDirect: true, feedUrl: "feed://declared", updateRecord: {namespace: "l0.x", activeChannel: "main", getChannels: => {"main"}}}
      UpdateTask.__persistSource task, selected, SourceChoiceStickiness.Retain
      cs = task.record.config.c.configuredSource
      ut\assertNotNil cs
      ut\assertEquals cs.feedSource, SourceFeedKind.SelfDeclared
      ut\assertEquals cs.stickiness, SourceChoiceStickiness.Retain
      ut\assertEquals cs.channel, "main"
      ut\assertTrue saved[1]

    persistSource_recordsProvider: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      selected = {isDirect: false, feedUrl: "feed://prov", providesVersion: "~1.2", updateRecord: {namespace: "l0.prov", activeChannel: "main", getChannels: => {"main"}}}
      UpdateTask.__persistSource task, selected, SourceChoiceStickiness.Pinned
      cs = task.record.config.c.configuredSource
      ut\assertEquals cs.feedSource, SourceFeedKind.Provider
      ut\assertEquals cs.provider.namespace, "l0.prov"
      ut\assertEquals cs.provider.version, "~1.2"

    persistSource_storesFeedUrlForOther: (ut) ->
      task = makeSourceTask feed: "feed://declared"
      selected = {isDirect: true, feedUrl: "feed://third", updateRecord: {namespace: "l0.x", activeChannel: "main", getChannels: => {"main"}}}
      UpdateTask.__persistSource task, selected, SourceChoiceStickiness.Once
      ut\assertEquals task.record.config.c.configuredSource.feedUrl, "feed://third"

    persistSource_skipsUnchanged: (ut) ->
      saves = {n: 0}
      existing = {feedSource: SourceFeedKind.SelfDeclared, channel: "main", stickiness: SourceChoiceStickiness.Retain}
      task = makeSourceTask feed: "feed://declared", onSave: -> saves.n += 1
      task.record.config.c.configuredSource, task.record.config.c.channels = existing, {"main"}
      selected = {isDirect: true, feedUrl: "feed://declared", updateRecord: {namespace: "l0.x", activeChannel: "main", getChannels: => {"main"}}}
      UpdateTask.__persistSource task, selected, SourceChoiceStickiness.Retain
      ut\assertEquals saves.n, 0

    -- a source recorded by a pre-0.9.0 version is honored as the configured source, and the write moves
    -- it under its new key without touching what `currentSource` now means
    persistSource_readsLegacyIntentAndWritesNewKey: (ut) ->
      saves = {n: 0}
      legacy = {feedSource: SourceFeedKind.SelfDeclared, channel: "main", stickiness: SourceChoiceStickiness.Pinned}
      task = makeSourceTask feed: "feed://declared", currentSource: legacy, onSave: -> saves.n += 1
      selected = {isDirect: true, feedUrl: "feed://declared", updateRecord: {namespace: "l0.x", activeChannel: "main", getChannels: => {"main"}}}
      UpdateTask.__persistSource task, selected
      ut\assertEquals task.record.config.c.configuredSource.stickiness, SourceChoiceStickiness.Pinned -- carried over
      ut\assertEquals task.record.config.c.currentSource, legacy -- untouched by the intent write
      ut\assertEquals saves.n, 1

    -- the feed's channel lineup is refreshed at resolution (sorted, so the persisted value doesn't ride
    -- on table order), and an unchanged repeat writes nothing
    persistSource_refreshesChannelLineup: (ut) ->
      saves = {n: 0}
      task = makeSourceTask feed: "feed://declared", onSave: -> saves.n += 1
      task.record.config.c.channels = {"alpha", "main"}
      selected = {isDirect: true, feedUrl: "feed://declared", updateRecord: {namespace: "l0.x", activeChannel: "stable", getChannels: => {"stable", "alpha"}}}
      UpdateTask.__persistSource task, selected, SourceChoiceStickiness.Retain
      ut\assertEquals table.concat(task.record.config.c.channels, ","), "alpha,stable"
      ut\assertEquals saves.n, 1
      -- a second identical resolution finds everything already recorded and leaves the config alone
      UpdateTask.__persistSource task, selected, SourceChoiceStickiness.Retain
      ut\assertEquals saves.n, 1

    -- __recordInstalledSource: once the files are in place, the configured source that drove the install
    -- becomes the installed copy's provenance, with lastChannel kept in step; a later change to the
    -- configured source must not reach through into the recorded provenance
    recordInstalledSource_copiesConfiguredToProvenance: (ut) ->
      saves = {n: 0}
      task = makeSourceTask feed: "feed://declared", onSave: -> saves.n += 1
      task.record.config.c.configuredSource = {feedSource: SourceFeedKind.SelfDeclared, channel: "stable", stickiness: SourceChoiceStickiness.Retain}
      UpdateTask.__recordInstalledSource task
      cs = task.record.config.c.currentSource
      ut\assertEquals cs.channel, "stable"
      ut\assertEquals cs.stickiness, SourceChoiceStickiness.Retain -- the full record, for troubleshooting
      ut\assertEquals cs.feedUrl, "feed://declared" -- stamped, though a self-declared choice stores none
      ut\assertEquals task.record.config.c.lastChannel, "stable"
      ut\assertEquals saves.n, 1
      task.record.config.c.configuredSource.channel = "alpha"
      ut\assertEquals cs.channel, "stable" -- a copy, not a shared reference

    -- __recordInstalledSource: the stamped URL is what the record keeps reporting even after the field
    -- it was derived from changes, which is the whole point of recording provenance separately
    recordInstalledSource_stampedUrlSurvivesFeedChange: (ut) ->
      task = makeSourceTask feed: "feed://declared", userFeed: "feed://override"
      task.record.config.c.configuredSource = {feedSource: SourceFeedKind.UserFeed, channel: "stable", stickiness: SourceChoiceStickiness.Unset}
      UpdateTask.__recordInstalledSource task
      ut\assertEquals task.record.config.c.currentSource.feedUrl, "feed://override"
      task.record.config.c.userFeed = "feed://somewhere-else"
      installed = task.record.config.c.currentSource
      ut\assertEquals UpdateTask.resolveSourceUrl(installed, task.record.feed, task.record.config.c.userFeed), "feed://override"
    -- __recordInstalledSource: with no source recorded at all (nothing resolved yet), nothing is written
    recordInstalledSource_noopWithoutSource: (ut) ->
      saves = {n: 0}
      task = makeSourceTask feed: "feed://declared", onSave: -> saves.n += 1
      UpdateTask.__recordInstalledSource task
      ut\assertNil task.record.config.c.currentSource
      ut\assertEquals saves.n, 0

    -- UpdateTask.__resolve: walks the lazy trust-ranked feed cascade and the currentSource stickiness tree,
    -- running prompts inline, and returns either a candidate to install or a terminal status code. It
    -- performs no installs, so it's tested directly with feed I/O and prompts stubbed (see makeResolveTask).

    -- cascade: a DeclaredDirect (band 1) winner short-circuits the rest of the cascade — the trusted tier
    -- is never even fetched (a higher-version candidate there is irrelevant).
    resolve_cascadeShortCircuitsOnDeclaredDirect: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}
        config: {extraFeeds: {"feed://extra"}}
        feeds: {"feed://decl": {direct: directRec version: "1.0.0"}, "feed://extra": {direct: directRec version: "9.9.9"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://decl"
      ut\assertNil task.calls.loaded["feed://extra"] -- tier 2 was never reached

    -- cascade: an empty declared feed falls through to a user extra feed (trusted discovery, tier 2).
    -- Guards that extraFeeds is read from the `feeds` config section, not `updates`.
    resolve_fallsThroughToExtraFeed: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl"
        config: {extraFeeds: {"feed://extra"}}
        feeds: {"feed://decl": {}, "feed://extra": {direct: directRec version: "2.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://extra"

    -- cascade: an empty declared feed falls through to a trusted feed (TrustedDirect, band 2)
    resolve_fallsThroughToTrustedDirect: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://trusted": true}
        feeds: {"feed://decl": {}, "feed://trusted": {direct: directRec version: "2.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://trusted"
      ut\assertEquals d.selectedSource.trustBand, 2

    -- trust gate: a required dependency whose only source is an untrusted feed (band 4) fails with -16
    -- when no prompt is permitted
    resolve_untrustedRequiredFailsWithoutPrompt: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", addFeeds: {"feed://un"}
        feeds: {"feed://decl": {}, "feed://un": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.UntrustedFeed

    -- trust gate: the same untrusted winner proceeds once the user approves it (Trust this time)
    resolve_untrustedApprovedProceeds: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", addFeeds: {"feed://un"}, allowPrompt: true, trustReturn: FeedTrustDecision.Once
        feeds: {"feed://decl": {}, "feed://un": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://un"
      ut\assertEquals task.calls.trust, 1

    -- trust gate: an optional dependency from an untrusted feed is skipped (3) rather than failed
    resolve_untrustedOptionalSkips: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", addFeeds: {"feed://un"}, optional: true
        feeds: {"feed://decl": {}, "feed://un": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.SkippedOptional

    -- no candidate: a required install with no source anywhere fails with -6
    resolve_noCandidateRequiredFails: (ut) ->
      task = makeResolveTask {declaredFeed: "feed://decl", feeds: {"feed://decl": {}}}
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.NoSuitablePackage

    -- no candidate: an optional install with no source is skipped (3)
    resolve_noCandidateOptionalSkips: (ut) ->
      task = makeResolveTask {declaredFeed: "feed://decl", optional: true, feeds: {"feed://decl": {}}}
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.SkippedOptional

    -- platform shortfall: a candidate matches the required version but offers no build for the current
    -- platform, so the failure detail names the offered platforms and the current one instead of implying
    -- the version is installable
    resolve_noPlatformBuildReportsPlatforms: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}
        feeds: {"feed://decl": {direct: directRec version: "1.0.0", platform: false, platforms: {"Windows-x64", "OSX-x64"}}}
      }
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.NoSuitablePackage
      ut\assertContains d.statusDetailMessage, "Windows-x64"
      ut\assertContains d.statusDetailMessage, "OSX-x64"
      ut\assertContains d.statusDetailMessage, environment.platform

    -- pinned: the remembered source is reused directly and the pin is preserved, no prompt
    resolve_pinnedReuseProceeds: (ut) ->
      cs = {feedSource: SourceFeedKind.SelfDeclared, channel: "release", stickiness: SourceChoiceStickiness.Pinned}
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}, currentSource: cs
        feeds: {"feed://decl": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://decl"
      ut\assertEquals d.stickiness, SourceChoiceStickiness.Pinned
      ut\assertEquals task.calls.select, 0

    -- pinned: a required dependency whose pinned source has vanished aborts with -17 (no silent switch)
    resolve_pinnedMissingRequiredAborts: (ut) ->
      cs = {feedSource: SourceFeedKind.SelfDeclared, channel: "release", stickiness: SourceChoiceStickiness.Pinned}
      task = makeResolveTask {declaredFeed: "feed://decl", currentSource: cs, feeds: {"feed://decl": {}}}
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.PinnedUnavailable

    -- pinned: the same vanished pin only skips (3) for an optional dependency
    resolve_pinnedMissingOptionalSkips: (ut) ->
      cs = {feedSource: SourceFeedKind.SelfDeclared, channel: "release", stickiness: SourceChoiceStickiness.Pinned}
      task = makeResolveTask {declaredFeed: "feed://decl", optional: true, currentSource: cs, feeds: {"feed://decl": {}}}
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.SkippedOptional

    -- retain: a still-eligible remembered source is reused without prompting
    resolve_retainReuseProceeds: (ut) ->
      cs = {feedSource: SourceFeedKind.SelfDeclared, channel: "release", stickiness: SourceChoiceStickiness.Retain}
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}, currentSource: cs
        feeds: {"feed://decl": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://decl"
      ut\assertEquals task.calls.select, 0

    -- retain (soft): when the remembered pick is gone and the run is non-interactive, fall back to the
    -- cascade's pick and downgrade the stickiness to Once
    resolve_retainMissingNonInteractiveDowngrades: (ut) ->
      cs = {feedSource: SourceFeedKind.Other, feedUrl: "feed://gone", channel: "release", stickiness: SourceChoiceStickiness.Retain}
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}, currentSource: cs
        feeds: {"feed://decl": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://decl"
      ut\assertEquals d.stickiness, SourceChoiceStickiness.Once
      ut\assertEquals task.calls.select, 0

    -- retain (soft): when the remembered pick is gone and the run is interactive, re-prompt and take the
    -- user's pick and stickiness
    resolve_retainMissingInteractivePicks: (ut) ->
      cs = {feedSource: SourceFeedKind.Other, feedUrl: "feed://gone", channel: "release", stickiness: SourceChoiceStickiness.Retain}
      pick = {feedUrl: "feed://decl", trustBand: 1, isDirect: true, updateRecord: {namespace: "json", version: "1.0.0"}}
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}, currentSource: cs
        allowPrompt: true, selectReturn: {pick, SourceChoiceStickiness.Retain}
        feeds: {"feed://decl": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource, pick
      ut\assertEquals d.stickiness, SourceChoiceStickiness.Retain
      ut\assertEquals task.calls.select, 1

    -- the chooser aborts the update: a required dependency fails with -18
    resolve_choiceAbortRequiredFails: (ut) ->
      cs = {feedSource: SourceFeedKind.Other, feedUrl: "feed://gone", channel: "release", stickiness: SourceChoiceStickiness.Retain}
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}, currentSource: cs
        allowPrompt: true, selectReturn: {}
        feeds: {"feed://decl": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.UserAborted

    -- auto: never prompts even when several equally-ranked candidates tie; takes the algorithm's pick
    resolve_autoNeverPrompts: (ut) ->
      cs = {feedSource: SourceFeedKind.SelfDeclared, channel: "release", stickiness: SourceChoiceStickiness.Auto}
      task = makeResolveTask {
        currentSource: cs, allowPrompt: true, officialTrusted: {"feed://a": true, "feed://b": true}
        feeds: {"feed://a": {direct: directRec version: "1.0.0"}, "feed://b": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals task.calls.select, 0

    -- offer-all-sources: with the global toggle on, the chooser fires whenever ≥2 candidates are eligible
    resolve_offerAllSourcesPromptsOnMultiple: (ut) ->
      pick = {feedUrl: "feed://b", trustBand: 2, isDirect: true, updateRecord: {namespace: "json", version: "1.0.0"}}
      task = makeResolveTask {
        allowPrompt: true, config: {offerAllSources: true}
        officialTrusted: {"feed://a": true, "feed://b": true}, selectReturn: {pick, SourceChoiceStickiness.Once}
        feeds: {"feed://a": {direct: directRec version: "1.0.0"}, "feed://b": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource, pick
      ut\assertEquals task.calls.select, 1

    -- blocked feed: a block-listed feed is skipped entirely (not even fetched, and despite being
    -- trusted); with no other source the required install fails
    -- checkFeed: a feed that has retired the channel a package is recorded on moves it to the default,
    -- except where the user pinned their source choice — that one fails so the pin isn't quietly broken
    checkFeed_pinnedKeepsChannelOthersFollowDefault: (ut) ->
      data = {name: "Pkg", channels: {stable: {default: true, version: "2.0.0", files: {}}}}
      check = (stickiness) ->
        config = {c: {lastChannel: "main", currentSource: {:stickiness}}}
        task = stubSelf UpdateTask, {
          logger: makeNullLogger!
          record: {namespace: "l0.Pkg", name: "Pkg", scriptType: domain.ScriptType.Module, :config}
        }
        feed = {getScript: ((ns, st, cfg) => ScriptUpdateRecord ns, data, cfg, st, false, makeNullLogger!)}
        UpdateTask.checkFeed task, feed
      record = check SourceChoiceStickiness.Retain
      ut\assertNotNil record
      ut\assertEquals record.activeChannel, "stable" -- followed the feed to the channel it still offers
      ut\assertNil check SourceChoiceStickiness.Pinned -- no candidate, so the pin surfaces as unavailable

    -- A task is cached per package for the whole session, so the feeds one run consulted must not be
    -- skipped by the next — the declared feed first among them, which would leave nothing to install from.
    resolve_reconsultsFeedsAfterEarlierRun: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", officialTrusted: {"feed://decl": true}
        feeds: {"feed://decl": {direct: directRec version: "2.0.0"}}
      }
      UpdateTask.__resolve task
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://decl"

    resolve_blockedFeedSkipped: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://blocked", officialTrusted: {"feed://blocked": true}
        config: {blockedFeeds: {{url: "feed://blocked"}}}
        feeds: {"feed://blocked": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertFalse d.installRequired
      ut\assertEquals d.statusCode, UpdateStatus.NoSuitablePackage
      ut\assertNil task.calls.loaded["feed://blocked"] -- skipped before being fetched

    -- userFeed: an exclusive override feed is consulted in place of the declared-feed cascade
    resolve_userFeedUsedExclusively: (ut) ->
      task = makeResolveTask {
        declaredFeed: "feed://decl", userFeed: "feed://user"
        feeds: {"feed://decl": {direct: directRec version: "9.9.9"}, "feed://user": {direct: directRec version: "1.0.0"}}
      }
      d = UpdateTask.__resolve task
      ut\assertTrue d.installRequired
      ut\assertEquals d.selectedSource.feedUrl, "feed://user"
      ut\assertNil task.calls.loaded["feed://decl"] -- the declared feed is never consulted

    -- run(): a direct-install resolution is persisted, then dispatched to performUpdate
    run_dispatchesDirectInstall: (ut) ->
      task = makeRunTask {
        virtual: false -- so the post-resolve up-to-date check runs
        resolution: {
          installRequired: true, stickiness: SourceChoiceStickiness.Once, maxVersion: 0
          selectedSource: {isDirect: true, updateRecord: {version: "1.0.0"}}
        }
      }
      code = UpdateTask.run task
      ut\assertEquals code, UpdateStatus.Installed
      ut\assertNotNil task.calls.persisted
      ut\assertNotNil task.calls.performUpdate
      ut\assertNil task.calls.installProvider

    -- run(): a fresh install records source and provenance again once the real record is adopted —
    -- the resolution-time writes landed on the virtual record's fileless config view and were lost
    run_freshInstallRecordsSourceOnAdoptedRecord: (ut) ->
      task = makeRunTask {
        resolution: {
          installRequired: true, stickiness: SourceChoiceStickiness.Once, maxVersion: 0
          selectedSource: {isDirect: true, updateRecord: {version: "1.0.0"}}
        }
      }
      persists, provenances = 0, 0
      task.__persistSource = (sel, st) => persists += 1
      task.__recordInstalledSource = => provenances += 1
      task.performUpdate = (update) =>
        @record.virtual = false -- the install adopts the real record
        UpdateStatus.Installed
      ut\assertEquals UpdateTask.run(task), UpdateStatus.Installed
      ut\assertEquals persists, 2 -- at resolution, and again once a config file backs the record
      ut\assertEquals provenances, 1

    -- run(): an update of an installed package doesn't repeat the writes — they were durable the first time
    run_installedUpdateSkipsRepersist: (ut) ->
      task = makeRunTask {
        virtual: false
        resolution: {
          installRequired: true, stickiness: SourceChoiceStickiness.Once, maxVersion: 0
          selectedSource: {isDirect: true, updateRecord: {version: "1.0.0"}}
        }
      }
      persists, provenances = 0, 0
      task.__persistSource = (sel, st) => persists += 1
      task.__recordInstalledSource = => provenances += 1
      task.performUpdate = (update) => UpdateStatus.Installed
      ut\assertEquals UpdateTask.run(task), UpdateStatus.Installed
      ut\assertEquals persists, 1
      ut\assertEquals provenances, 0 -- performUpdate's own in-place write covers it

    -- run(): a provider (indirect) resolution is dispatched to installProvider, not performUpdate
    run_dispatchesProviderInstall: (ut) ->
      providerRef = {ref: true}
      task = makeRunTask {
        installProviderReturn: {providerRef}
        resolution: {
          installRequired: true, stickiness: SourceChoiceStickiness.Once, maxVersion: 0
          selectedSource: {
            isDirect: false, feedUrl: "feed://p"
            updateRecord: {namespace: "l0.p", name: "P", version: "2.0.0"}
          }
        }
      }
      code, detail = UpdateTask.run task
      ut\assertEquals code, UpdateStatus.Installed
      ut\assertEquals detail, "2.0.0"
      ut\assertNotNil task.calls.installProvider
      ut\assertNil task.calls.performUpdate

    -- run(): when the chosen direct source already satisfies the installed version, short-circuit to 0
    run_upToDateShortCircuits: (ut) ->
      task = makeRunTask {
        virtual: false, installedSatisfies: true
        resolution: {
          installRequired: true, stickiness: SourceChoiceStickiness.Once, maxVersion: 0
          selectedSource: {isDirect: true, updateRecord: {version: "1.0.0"}}
        }
      }
      code = UpdateTask.run task
      ut\assertEquals code, UpdateStatus.UpToDate
      ut\assertNil task.calls.performUpdate -- no install performed
      ut\assertNotNil task.calls.persisted -- but the source choice is still recorded

    -- run(): a dependency cycle re-enters the task whose update covers the requirement, so it reports the
    -- update in progress and leaves the install to the run already under way
    run_reentrantInFlightSatisfiesRequirement: (ut) ->
      task = makeRunTask targetVersion: SemanticVersion\toPacked "1.3.0"
      task.__installingVersion = "1.3.6"
      code, detail = UpdateTask.run task
      ut\assertEquals code, UpdateStatus.UpdateInProgress
      ut\assertEquals detail, "1.3.6"
      ut\assertNil task.calls.resolved

    -- run(): the same cycle, but the version being installed sits below the requirement, which finishing
    -- that update won't change, so it stays an error
    run_reentrantInFlightBelowRequirementFails: (ut) ->
      task = makeRunTask targetVersion: SemanticVersion\toPacked "2.0.0"
      task.__installingVersion = "1.3.6"
      ut\assertEquals UpdateTask.run(task), UpdateStatus.TaskAlreadyRunning

    -- run(): with no version recorded there is no cycle to break, so a task still flagged running is rejected
    run_reentrantWithoutClaimStillGuardsRunning: (ut) ->
      task = makeRunTask running: true
      ut\assertEquals UpdateTask.run(task), UpdateStatus.TaskAlreadyRunning

    -- run(): a terminal resolution (no install required) returns its status without dispatching
    run_terminalResolutionReturnsStatus: (ut) ->
      task = makeRunTask {
        resolution: {installRequired: false, statusCode: UpdateStatus.UntrustedFeed, statusDetailMessage: "feed://x"}
      }
      code, detail = UpdateTask.run task
      ut\assertEquals code, UpdateStatus.UntrustedFeed
      ut\assertEquals detail, "feed://x"
      ut\assertNil task.calls.persisted
      ut\assertNil task.calls.performUpdate

    -- run(): the internet-connectivity guard fails (-7) before resolution is even attempted
    run_noInternetGuard: (ut) ->
      task = makeRunTask {online: false, resolution: {installRequired: false, statusCode: UpdateStatus.UpToDate}}
      code = UpdateTask.run task
      ut\assertEquals code, UpdateStatus.NoInternet
      ut\assertNil task.calls.resolved -- the guard returns before resolve()

    -- __installProvider: builds a virtual provider record from the feed entry, appends the provider's
    -- feed to addFeeds, and installs it through the updater (forwarding targetVersion/optional/reason)
    installProvider_constructsRecordAndRequires: (ut) ->
      cap = {}
      fakeRef = {ref: true}
      task = makeProviderTask {
        depCtrl: ((args) -> {ctorArgs: args})
        addFeeds: {"feed://a"}, targetVersion: 0x10000, optional: true, reason: UpdateReason.UserRequested
        requireReturn: {fakeRef}, capture: cap
      }
      provider = {namespace: "l0.prov", name: "Prov", url: "http://prov/x.zip"}
      ref = UpdateTask.__installProvider task, provider, "feed://prov"
      ut\assertEquals ref, fakeRef
      ctor = cap.record.ctorArgs
      ut\assertEquals ctor.moduleName, "l0.prov"
      ut\assertEquals ctor.version, -1
      ut\assertTrue ctor.virtual
      ut\assertEquals ctor.feed, "feed://prov"
      ut\assertEquals ctor.url, "http://prov/x.zip"
      ut\assertEquals cap.addFeeds[1], "feed://a"
      ut\assertEquals cap.addFeeds[2], "feed://prov" -- the provider's feed is appended
      ut\assertEquals cap.targetVersion, 0x10000
      ut\assertTrue cap.optional
      ut\assertEquals cap.reason, UpdateReason.UserRequested

    -- performUpdate: an error thrown while resolving requirements leaves no version behind and clears the
    -- running flag, so it can't corrupt a later update run
    performUpdate_clearsInFlightClaimOnThrow: (ut) ->
      task = makePerformTask!
      (ut\stub ModuleLoader, "loadModules")\calls -> error "requirement boom", 0
      update = {version: "1.3.6", files: {}, requiredModules: {{moduleName: "l0.dep"}}}
      completed, err = pcall UpdateTask.performUpdate, task, update
      ut\assertFalse completed
      ut\assertEquals err, "requirement boom"
      ut\assertNil task.__installingVersion
      ut\assertFalse task.running

    -- performUpdate: the version being installed is readable while the requirements resolve, which is what
    -- a cycle breaks against, and gone once they finish
    performUpdate_publishesInFlightVersionWhileResolving: (ut) ->
      task = makePerformTask!
      seen = {}
      (ut\stub ModuleLoader, "loadModules")\calls ->
        seen.claim = task.__installingVersion
        return false, "unmet"
      update = {version: "1.3.6", files: {}, requiredModules: {{moduleName: "l0.dep"}}}
      code = UpdateTask.performUpdate task, update
      ut\assertEquals code, UpdateStatus.RequirementsUnmet
      ut\assertEquals seen.claim, "1.3.6"
      ut\assertNil task.__installingVersion -- and it doesn't outlive the resolution

    -- performUpdate: a temp-directory creation failure aborts with -30
    performUpdate_tempDirFailure: (ut) ->
      ut\stub(fileOps, "getTempDir")\returns "tmp"
      ut\stub(fileOps, "mkdir")\returns nil, "denied"
      code = UpdateTask.performUpdate makePerformTask!, {version: 0x10000, files: {}}
      ut\assertEquals code, UpdateStatus.TempDirFailed

    -- performUpdate: a file name containing ".." is rejected as a path-traversal attempt (-33)
    performUpdate_rejectsPathTraversal: (ut) ->
      ut\stub(fileOps, "getTempDir")\returns "tmp"
      ut\stub(fileOps, "mkdir")\returns true, "tmp"
      update = {version: 0x10000, files: {{name: "../evil.moon", type: "script"}}}
      code, detail = UpdateTask.performUpdate makePerformTask!, update
      ut\assertEquals code, UpdateStatus.PathTraversal
      ut\assertEquals detail, "../evil.moon"

    -- performUpdate: a file with a malformed sha1 hash is rejected (-35)
    performUpdate_rejectsBadSha1: (ut) ->
      ut\stub(fileOps, "getTempDir")\returns "tmp"
      ut\stub(fileOps, "mkdir")\returns true, "tmp"
      ut\stub(UpdateFeed, "getFileDeployPath")\returns "deploy/x.moon"
      update = {version: 0x10000, files: {{name: "x.moon", type: "script", sha1: "abc"}}} -- not 40 hex chars
      code = UpdateTask.performUpdate makePerformTask!, update
      ut\assertEquals code, UpdateStatus.BadHash

    -- performUpdate: a download that ends in a Failed status is reported (-245)
    performUpdate_reportsFailedDownloads: (ut) ->
      ut\stub(fileOps, "getTempDir")\returns "tmp"
      ut\stub(fileOps, "mkdir")\returns true, "tmp"
      ut\stub(UpdateFeed, "getFileDeployPath")\returns "deploy/x.moon"
      ut\stub(fileOps, "verifyHash")\returns false -- not already on disk → it gets downloaded
      task = makePerformTask {downloadStatus: Downloader.Download.Status.Failed}
      update = {version: 0x10000, files: {{name: "x.moon", type: "script", url: "http://x", sha1: string.rep "a", 40}}}
      code = UpdateTask.performUpdate task, update
      ut\assertEquals code, UpdateStatus.DownloadFailed

    -- performUpdate: a failed file move (after a successful download) is reported (-50)
    performUpdate_reportsMoveFailures: (ut) ->
      ut\stub(fileOps, "getTempDir")\returns "tmp"
      ut\stub(fileOps, "mkdir")\returns true, "tmp"
      ut\stub(UpdateFeed, "getFileDeployPath")\returns "deploy/x.moon"
      ut\stub(fileOps, "verifyHash")\returns false
      ut\stub(fileOps, "move")\returns nil, "move denied"
      task = makePerformTask {downloadStatus: Downloader.Download.Status.Finished}
      update = {version: 0x10000, files: {{name: "x.moon", type: "script", url: "http://x", sha1: string.rep "a", 40}}}
      code = UpdateTask.performUpdate task, update
      ut\assertEquals code, UpdateStatus.MoveFailed

    -- performUpdate happy path (module): after a successful download+move, the module is reloaded and the
    -- task's record is swapped to the fresh DependencyControl version record; returns 1 and the new version
    performUpdate_reloadsModuleAndRefreshesRecord: (ut) ->
      ut\stub(fileOps, "getTempDir")\returns "tmp"
      ut\stub(fileOps, "mkdir")\returns true, "tmp"
      ut\stub(UpdateFeed, "getFileDeployPath")\returns "deploy/x.moon"
      ut\stub(fileOps, "verifyHash")\returns false
      ut\stub(fileOps, "move")\returns true
      ut\stub(fileOps, "rmdir")\returns true
      newRecord = {__class: {__name: "DependencyControl"}, version: SemanticVersion\toPacked "1.0.0"}
      ut\stub(ModuleLoader, "loadModule")\returns {version: newRecord}
      task = makePerformTask {downloadStatus: Downloader.Download.Status.Finished}
      update = {
        version: "1.0.0", getChangelog: ((rec, ver) => "")
        files: {{name: "x.moon", type: "script", url: "http://x", sha1: string.rep "a", 40}}
      }
      code, detail = UpdateTask.performUpdate task, update
      ut\assertEquals code, UpdateStatus.Installed
      ut\assertEquals detail, "1.0.0"
      ut\assertTrue task.updated
      ut\assertIs task.record, newRecord -- record swapped to the freshly-loaded version record

    -- a module that registered a DependencyControl record but exposes a plain string as its .version is
    -- recovered as managed from the record registry, not demoted to a fresh unmanaged record
    performUpdate_recoversManagedRecordFromRegistry: (ut) ->
      ut\stub(fileOps, "getTempDir")\returns "tmp"
      ut\stub(fileOps, "mkdir")\returns true, "tmp"
      ut\stub(UpdateFeed, "getFileDeployPath")\returns "deploy/x.moon"
      ut\stub(fileOps, "verifyHash")\returns false
      ut\stub(fileOps, "move")\returns true
      ut\stub(fileOps, "rmdir")\returns true
      registered = {
        name: "TestMod", scriptType: domain.ScriptType.Module, recordType: domain.RecordType.Managed
        version: SemanticVersion\toPacked "1.8.5", __class: {__name: "DependencyControl"}
      }
      ut\stub(ModuleLoader, "loadModule")\returns {version: "1.8.5"} -- a bare string, not a DepCtrl record
      task = makePerformTask {downloadStatus: Downloader.Download.Status.Finished}
      task.__class.__DependencyControl.getRegisteredRecord = (self, ns) -> ns == task.record.namespace and registered or nil
      update = {
        version: "1.8.5", getChangelog: ((rec, ver) => "")
        files: {{name: "x.moon", type: "script", url: "http://x", sha1: string.rep "a", 40}}
      }
      code = UpdateTask.performUpdate task, update
      ut\assertEquals code, UpdateStatus.Installed
      ut\assertIs task.record, registered -- adopted the self-registered managed record, not a new unmanaged one

    -- refreshRecord: another updater installed the module while we waited for the lock → adopt the result
    refreshRecord_detectsExternalInstall: (ut) ->
      loadedRef = {fresh: true}
      ut\stub(ModuleLoader, "loadModule")\returns loadedRef
      record = {
        virtual: true, version: 0, scriptType: domain.ScriptType.Module, name: "Dep"
        loadConfig: (force) =>
          @virtual = false
          @version = SemanticVersion\toPacked "1.0.0"
      }
      task = stubSelf UpdateTask, {updated: false, logger: makeNullLogger!, :record}
      UpdateTask.refreshRecord task
      ut\assertTrue task.updated
      ut\assertIs task.ref, loadedRef

    -- refreshRecord: nothing changed since the last check → the task is left untouched
    refreshRecord_noChangeStaysUntouched: (ut) ->
      record = {
        virtual: false, version: SemanticVersion\toPacked "1.0.0"
        scriptType: domain.ScriptType.Module, name: "Dep", loadConfig: (force) => nil
      }
      task = stubSelf UpdateTask, {updated: false, logger: makeNullLogger!, :record}
      UpdateTask.refreshRecord task
      ut\assertFalse task.updated

    -- __loadFeed refuses a blocked/never feed before any network fetch, surfacing a reason
    loadFeed_refusesDeniedFeed: (ut) ->
      feedTrust = {getFetchDecision: (url) => FeedTrust.FetchDecision.Deny}
      task = stubSelf UpdateTask, {
        updater: {feedTrust: feedTrust}
        logger: makeNullLogger!
      }
      feed, err = UpdateTask.__loadFeed task, "feed://untrusted"
      ut\assertNil feed
      ut\assertString err

    -- getUpdaterErrorMsg: the noun install/update terms read grammatically in every template,
    -- a nil isInstall renders as an update, and a nil or unmapped code falls back to the generic message
    getUpdaterErrorMsg_grammarAndNilInstall: (ut) ->
      msg = UpdateTask.getUpdaterErrorMsg UpdateStatus.TempDirFailed, "X", domain.ScriptType.Module, true, "C:/tmp"
      ut\assertContains msg, "Couldn't complete the installation of module 'X'"
      msg = UpdateTask.getUpdaterErrorMsg UpdateStatus.Unmanaged, "Y", domain.ScriptType.Module, nil
      ut\assertContains msg, "Skipping update of unmanaged module 'Y'"
      msg = UpdateTask.getUpdaterErrorMsg nil, "Z", domain.ScriptType.Module, true
      ut\assertContains msg, "unrecognized updater status: nil"
      msg = UpdateTask.getUpdaterErrorMsg UpdateStatus.SkippedOptional, "Z", domain.ScriptType.Module, true
      ut\assertContains msg, "unrecognized updater status: 3"

    -- getUpdaterErrorMsg: a RequirementsUnmet message carries the nested requirement-failure detail
    -- (e.g. which required module couldn't be installed), rather than dropping it
    getUpdaterErrorMsg_requirementsUnmetKeepsDetail: (ut) ->
      msg = UpdateTask.getUpdaterErrorMsg UpdateStatus.RequirementsUnmet, "l0.ASSFoundation",
        domain.ScriptType.Module, true, "— SubInspector.Inspector: no build for your platform"
      ut\assertContains msg, "requirements could not be satisfied"
      ut\assertContains msg, "SubInspector.Inspector: no build for your platform"

    -- __getOfferedBuildPlatforms: collects the platforms a version-satisfying build is offered for when
    -- none covers the current one (de-duped, sorted), ignoring platform-supporting, too-old, or indirect
    -- candidates
    getOfferedBuildPlatforms_collectsVersionMatchingRejects: (ut) ->
      task = stubSelf UpdateTask, {targetVersion: SemanticVersion\toPacked "1.0.0"}
      candidates = {
        makeCandidate 1, "1.2.0", {platform: false, platforms: {"Windows-x64", "OSX-x64"}} -- version ok, wrong platform → collect
        makeCandidate 1, "1.5.0", {platform: false, platforms: {"Windows-x64"}} -- duplicate platform → de-dup
        makeCandidate 1, "1.0.0", {platform: true, platforms: {"Linux-x64"}} -- supports platform → ignore
        makeCandidate 1, "0.9.0", {platform: false, platforms: {"SomethingElse"}} -- too old → ignore
        makeCandidate 1, "1.0.0", {isDirect: false, platform: false, platforms: {"Provider"}} -- indirect → ignore
      }
      platforms = UpdateTask.__getOfferedBuildPlatforms task, candidates
      ut\assertItemsEqual platforms, {"OSX-x64", "Windows-x64"}

    _order: {
      "loadFeed_refusesDeniedFeed",
      "getUpdaterErrorMsg_grammarAndNilInstall", "getUpdaterErrorMsg_requirementsUnmetKeepsDetail",
      "getOfferedBuildPlatforms_collectsVersionMatchingRejects",
      "selectCandidate_filtersByVersion", "selectCandidate_noneSatisfiesVersion",
      "selectCandidate_skipsUnsupportedPlatform", "selectCandidate_skipsEmptyFiles", "selectCandidate_noneEligible",
      "selectCandidate_lowerBandBeatsHigherVersion", "selectCandidate_highestVersionWithinBand",
      "selectCandidate_declaredFeedTiebreak", "selectCandidate_namespaceTiebreakLogged", "selectCandidate_unambiguousNoLog",
      "selectCandidate_providesVersionRangeSatisfies", "selectCandidate_providesVersionRangeRejects",
      "selectCandidate_providesVersionRangeAboveTarget", "selectCandidate_providerNoRangeMatchesAny",
      "selectCandidate_providerRankedByRangeMaxNotRelease", "selectCandidate_returnsTiedSet",
      "isWithinContextCeiling_ranksLadder",
      "shouldPrompt_withinThreshold", "shouldPrompt_aboveThreshold", "shouldPrompt_offNeverPrompts",
      "shouldPrompt_noReason", "shouldPrompt_feedTrustDefaultAllowsAutoUpdate",
      "promptTrustFeed_trustOnce", "promptTrustFeed_cancelReturnsNil", "promptTrustFeed_trustAlwaysPersists",
      "promptTrustFeed_neverBlocks",
      "promptSelectPackageSource_picksSelection", "promptSelectPackageSource_autoKeepsWinner",
      "promptSelectPackageSource_abortReturnsNil",
      "resolveRememberedFeedUrl_selfDeclared", "resolveRememberedFeedUrl_userFeed", "resolveRememberedFeedUrl_other",
      "resolveRememberedFeedUrl_provider", "resolveRememberedFeedUrl_providerMissing",
      "matchRememberedCandidate_direct", "matchRememberedCandidate_provider",
      "matchRememberedCandidate_ineligibleVersion", "matchRememberedCandidate_noUrlMatch",
      "feedSourceOf_provider", "feedSourceOf_selfDeclared", "feedSourceOf_userFeed", "feedSourceOf_other",
      "persistSource_writesDirect", "persistSource_recordsProvider", "persistSource_storesFeedUrlForOther",
      "persistSource_skipsUnchanged", "persistSource_readsLegacyIntentAndWritesNewKey",
      "persistSource_refreshesChannelLineup",
      "recordInstalledSource_copiesConfiguredToProvenance", "recordInstalledSource_stampedUrlSurvivesFeedChange",
      "recordInstalledSource_noopWithoutSource",
      "resolve_cascadeShortCircuitsOnDeclaredDirect", "resolve_fallsThroughToExtraFeed",
      "resolve_fallsThroughToTrustedDirect",
      "resolve_untrustedRequiredFailsWithoutPrompt", "resolve_untrustedApprovedProceeds",
      "resolve_untrustedOptionalSkips", "resolve_noCandidateRequiredFails", "resolve_noCandidateOptionalSkips",
      "resolve_noPlatformBuildReportsPlatforms",
      "resolve_pinnedReuseProceeds", "resolve_pinnedMissingRequiredAborts", "resolve_pinnedMissingOptionalSkips",
      "resolve_retainReuseProceeds", "resolve_retainMissingNonInteractiveDowngrades",
      "resolve_retainMissingInteractivePicks", "resolve_choiceAbortRequiredFails",
      "resolve_autoNeverPrompts", "resolve_offerAllSourcesPromptsOnMultiple",
      "checkFeed_pinnedKeepsChannelOthersFollowDefault", "resolve_reconsultsFeedsAfterEarlierRun",
      "resolve_blockedFeedSkipped", "resolve_userFeedUsedExclusively",
      "run_dispatchesDirectInstall", "run_freshInstallRecordsSourceOnAdoptedRecord",
      "run_installedUpdateSkipsRepersist", "run_dispatchesProviderInstall", "run_upToDateShortCircuits",
      "run_reentrantInFlightSatisfiesRequirement", "run_reentrantInFlightBelowRequirementFails",
      "run_reentrantWithoutClaimStillGuardsRunning",
      "run_terminalResolutionReturnsStatus", "run_noInternetGuard",
      "installProvider_constructsRecordAndRequires",
      "performUpdate_clearsInFlightClaimOnThrow", "performUpdate_publishesInFlightVersionWhileResolving",
      "performUpdate_tempDirFailure", "performUpdate_rejectsPathTraversal", "performUpdate_rejectsBadSha1",
      "performUpdate_reportsFailedDownloads", "performUpdate_reportsMoveFailures",
      "performUpdate_reloadsModuleAndRefreshesRecord", "performUpdate_recoversManagedRecordFromRegistry",
      "refreshRecord_detectsExternalInstall", "refreshRecord_noChangeStaysUntouched"
    }
  }
