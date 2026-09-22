lfs = require "lfs"
Downloader = require "l0.DependencyControl.Downloader"
UpdateFeed = require "l0.DependencyControl.UpdateFeed"
FeedTrust = require "l0.DependencyControl.FeedTrust"
fileOps = require "l0.DependencyControl.file-ops"
domain = require "l0.DependencyControl.domain"
environment = require "l0.DependencyControl.environment"
Enum = require "l0.DependencyControl.Enum"
ModuleLoader = require "l0.DependencyControl.ModuleLoader"
ScriptUpdateRecord = require "l0.DependencyControl.ScriptUpdateRecord"
SemanticVersion = require "l0.DependencyControl.SemanticVersion"
UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
utils = require "l0.DependencyControl.utils"

---The "installation"/"update" term for a record's task. domain.terms.isInstall is keyed by
---true/false, while an installed record leaves `virtual` nil.
---@param record PackageRecord
---@return string term
getInstallTerm = (record) -> domain.terms.isInstall[record.virtual or false]

-- How preferred a candidate package source is, in highest-to-lowest trust order.
---@alias UpdaterTrustBand
---| 1 # DeclaredDirect: the declared/own feed, trusted, offering the module by name
---| 2 # TrustedDirect: another trusted feed, offering the module by name
---| 3 # TrustedProvider: a trusted feed, offering a module that provides it
---| 4 # UntrustedDirect: an untrusted feed, offering the module by name
---| 5 # UntrustedProvider: an untrusted feed, offering a provider
TrustBand = Enum "UpdaterTrustBand", {
  DeclaredDirect: 1
  TrustedDirect: 2
  TrustedProvider: 3
  UntrustedDirect: 4
  UntrustedProvider: 5
}

---A potential package source pooled during resolution to collect a feed's update record, where it came from
---and its trust status.
---@class CandidatePackageSource
---@field updateRecord ScriptUpdateRecord The feed's update record for this candidate, channel already selected.
---@field feedUrl string URL of the feed the candidate was found in.
---@field isDirect boolean True when the feed offers the required package by name; false when via a provider.
---@field trustBand UpdaterTrustBand The candidate's trust band.
---@field providesVersion? string For a provider, the alias version range it declares (nil = any version).

---The outcome of resolving a package source: either a source to install or a status to return.
---@class UpdaterResolution
---@field installRequired boolean Whether the caller must perform an install/update.
---@field statusCode? UpdateStatus The status to return when no install is required (installRequired false).
---@field statusDetailMessage? any Detail accompanying statusCode (e.g. an untrusted feed URL or an error string).
---@field selectedSource? CandidatePackageSource The source to install; set when installRequired is true.
---@field stickiness? SourceChoiceStickiness The source-choice stickiness to persist; set when installRequired is true.
---@field maxVersion? number The highest candidate version found during resolution; set when installRequired is true.

-- Why a given update is running. The values double as the rungs of the update-context ladder,
-- ordered by autonomy (see UpdateContextCeiling).
---@alias UpdateReason
---| "user-requested" # UserRequested: an explicit user/UI request (e.g. the Toolbox)
---| "dependency-resolution" # DependencyResolution: installing/updating a module as a dependency of another
---| "auto-update" # AutoUpdate: a background scheduled update check
UpdateReason = Enum "UpdateReason", {
  UserRequested: "user-requested"
  DependencyResolution: "dependency-resolution"
  AutoUpdate: "auto-update"
}

---A ceiling on the update-context ladder: names the most autonomous context for which a gated behavior —
---running at all (`updates.mode`) or prompting the user (the prompt thresholds) — still applies.
---Every value includes all less autonomous contexts; `off` includes none.
---@alias UpdateContextCeiling
---| "off" # Off: no context at all
---| "user-requested" # UserRequested: only actions the user starts themselves (e.g. via the Toolbox)
---| "dependency-resolution" # DependencyResolution: also installing/updating a module as a dependency
---| "auto-update" # AutoUpdate: also background scheduled update checks
ContextCeiling = Enum "UpdateContextCeiling", {
  Off: "off"
  -- the three shared rungs carry UpdateReason's own values, so a reason ranks directly against a ceiling
  UserRequested: UpdateReason.UserRequested
  DependencyResolution: UpdateReason.DependencyResolution
  AutoUpdate: UpdateReason.AutoUpdate
}

-- Each context's rank on the ladder, for ceiling comparisons (`off` ranks below every context).
contextRank = {
  [ContextCeiling.Off]: 0
  [ContextCeiling.UserRequested]: 1
  [ContextCeiling.DependencyResolution]: 2
  [ContextCeiling.AutoUpdate]: 3
}

-- The user's decision when asked to trust a candidate from an untrusted feed (a cancelled prompt is nil).
---@alias FeedTrustDecision
---| "once" # Once: use the untrusted feed for this install only
---| "always" # Always: add the feed to the user's trusted feeds
---| "never" # Never: add the feed to the user's blocked feeds
FeedTrustDecision = Enum "FeedTrustDecision", {
  Once: "once"
  Always: "always"
  Never: "never"
}

SourceChoiceStickiness = domain.SourceChoiceStickiness

-- Where a remembered package source came from.
---@alias SourceFeedKind
---| "self-declared" # SelfDeclared: the feed declared in the record
---| "user-feed" # UserFeed: the per-package user override feed
---| "provider" # Provider: another module that provides the required one (URL taken from the provider's own source)
---| "other" # Other: a third-party trusted/extra feed; stores a literal feedUrl
SourceFeedKind = Enum "SourceFeedKind", {
  SelfDeclared: "self-declared"
  UserFeed: "user-feed"
  Provider: "provider"
  Other: "other"
}

---A package source and how sticky the choice of it is. Persisted per-package twice: `configuredSource`
---holds the source the next update should use, `currentSource` the one the installed copy came from.
---The two diverge while a source change hasn't been applied by a completed update yet.
---@class SourceChoiceRecord
---@field feedSource SourceFeedKind Where the source came from.
---@field feedUrl? string The literal feed URL. A `configuredSource` stores it only for the `other` kind, and otherwise defers to the kind-specific properties elsewhere in the config. A `currentSource` always stores it, so provenance stays true when those fields change.
---@field channel string The update channel the source was resolved on.
---@field provider? { namespace: string, version?: string } The provider that satisfied the requirement, when resolved indirectly.
---@field stickiness SourceChoiceStickiness How sticky the choice is.

-- The outcome of an install/update operation. A non-negative value is a success/skip outcome; a
-- negative value is a failure whose message template is `updateError[value]`.
---@alias UpdateStatus
---| 0 # UpToDate: the installed version already satisfies the target
---| 1 # Installed: the install or update succeeded
---| 2 # AlreadyUpdated: another in-flight update already brought the package to the target version
---| 3 # SkippedOptional: an optional dependency couldn't be satisfied and was skipped
---| 4 # UpdateInProgress: this package's update is already under way and will install a satisfying version
---| -1 # UpdaterDisabled: the updater is disabled in the config
---| -2 # InvalidNamespace: the record's namespace doesn't conform to the rules
---| -3 # Unmanaged: the record is virtual or unmanaged, so it isn't updated
---| -5 # AnotherUpdateRunning: another script or process holds the updater lock
---| -6 # NoSuitablePackage: no feed offered a package satisfying the requirement
---| -7 # NoInternet: no internet connection is available
---| -8 # InvalidVersion: the requested version string couldn't be parsed
---| -9 # ProtectedInstall: the entry point is in Aegisub's ?data automation directory
---| -10 # TaskAlreadyRunning: this update task is already running
---| -15 # RequirementsUnmet: the package's own required modules couldn't be satisfied
---| -16 # UntrustedFeed: the only suitable package is in an untrusted feed
---| -17 # PinnedUnavailable: the pinned package source is no longer available
---| -18 # UserAborted: the user aborted the update
---| -19 # BlockedFeed: the only suitable package is in a feed the user blocked
---| -30 # TempDirFailed: the temporary download directory couldn't be created
---| -33 # PathTraversal: a feed file tried to deploy outside its namespaced path
---| -35 # BadHash: a feed file carried a missing or malformed SHA-1 hash
---| -50 # MoveFailed: some downloaded files couldn't be moved into place
---| -55 # ModuleNotFound: the install succeeded but the module loader couldn't find the module
---| -56 # ModuleLoadFailed: the install succeeded but the module raised while loading
---| -57 # MissingVersionRecord: the installed module exposes no version record
---| -58 # RecordCreateFailed: creating an unmanaged record for the installed module failed
---| -140 # DownloadAddFailed: a file download couldn't be queued
---| -245 # DownloadFailed: one or more file downloads failed
UpdateStatus = Enum "UpdateStatus", {
  UpToDate: 0
  Installed: 1
  AlreadyUpdated: 2
  SkippedOptional: 3
  UpdateInProgress: 4
  UpdaterDisabled: -1
  InvalidNamespace: -2
  Unmanaged: -3
  AnotherUpdateRunning: -5
  NoSuitablePackage: -6
  NoInternet: -7
  InvalidVersion: -8
  ProtectedInstall: -9
  TaskAlreadyRunning: -10
  RequirementsUnmet: -15
  UntrustedFeed: -16
  PinnedUnavailable: -17
  UserAborted: -18
  BlockedFeed: -19
  TempDirFailed: -30
  PathTraversal: -33
  BadHash: -35
  MoveFailed: -50
  ModuleNotFound: -55
  ModuleLoadFailed: -56
  MissingVersionRecord: -57
  RecordCreateFailed: -58
  DownloadAddFailed: -140
  DownloadFailed: -245
}

msgs = {
  new: {
    badRecord: "First parameter must be a %s object."
    badTargetVersion: "Second parameter must be a semantic version number in integer format."
  }
  updateError: {
    [UpdateStatus.UpToDate]: "Couldn't complete the %s of %s '%s' because of a paradox: module not found but updater says up-to-date (%s)"
    [UpdateStatus.UpdaterDisabled]: "Couldn't complete the %s of %s '%s' because the updater is disabled."
    [UpdateStatus.InvalidNamespace]: "Skipping %s of %s '%s': namespace '%s' doesn't conform to rules."
    [UpdateStatus.Unmanaged]: "Skipping %s of unmanaged %s '%s'."
    [UpdateStatus.AnotherUpdateRunning]: "Skipped %s of %s '%s': another update initiated by %s is already running."
    [UpdateStatus.NoSuitablePackage]: "The %s of %s '%s' failed because no suitable package could be found %s."
    [UpdateStatus.NoInternet]: "Skipped %s of %s '%s': an internet connection is currently not available."
    [UpdateStatus.InvalidVersion]: "Couldn't complete the %s of %s '%s' because the requested version is invalid: %s"
    [UpdateStatus.ProtectedInstall]: "Skipped %s of %s '%s' because its entry point (%s) is in Aegisub's data automation directory. If it's managed by a system package manager, please update it through that instead."
    [UpdateStatus.TaskAlreadyRunning]: "Skipped %s of %s '%s': the update task is already running."
    [UpdateStatus.RequirementsUnmet]: "Couldn't complete the %s of %s '%s' because its requirements could not be satisfied:\n%s"
    [UpdateStatus.UntrustedFeed]: "Couldn't complete the %s of %s '%s' because a suitable package was only found in an untrusted feed (%s). Add it to your trusted feeds to proceed."
    [UpdateStatus.PinnedUnavailable]: "Couldn't complete the %s of %s '%s' because its pinned package source is no longer available. Update or clear the pin to proceed."
    [UpdateStatus.UserAborted]: "Aborted the %s of %s '%s' at your request."
    [UpdateStatus.BlockedFeed]: "Couldn't complete the %s of %s '%s' because you blocked the feed (%s) it would be installed from."
    [UpdateStatus.TempDirFailed]: "Couldn't complete the %s of %s '%s': failed to create temporary download directory %s"
    [UpdateStatus.PathTraversal]: "Aborted the %s of %s '%s' because it attempted to deploy a file (%s) outside of its namespaced path."
    [UpdateStatus.BadHash]: "Aborted the %s of %s '%s' because the feed contained a missing or malformed SHA-1 hash for file %s."
    [UpdateStatus.MoveFailed]: "Couldn't finish the %s of %s '%s' because some files couldn't be moved to their target location:\n"
    [UpdateStatus.ModuleNotFound]: "The %s of %s '%s' succeeded, but the module couldn't be located by the module loader."
    [UpdateStatus.ModuleLoadFailed]: "The %s of %s '%s' succeeded, but an error occurred while loading the module:\n%s"
    [UpdateStatus.MissingVersionRecord]: "The %s of %s '%s' succeeded, but it's missing a version record."
    [UpdateStatus.RecordCreateFailed]: "The %s of unmanaged %s '%s' succeeded, but an error occurred while creating a DependencyControl record: %s"
    -- shared template for component-encoded statuses (value <= -100, e.g. DownloadAddFailed/DownloadFailed)
    component: "Error (%d) in component %s during the %s of %s '%s':\n— %s"
    -- fallback for a nil or unmapped status code, so error reporting can't itself fail
    unknown: "Couldn't complete the %s of %s '%s' (unrecognized updater status: %s)."
  }
  updaterErrorComponent: {"DownloadManager (adding download)", "DownloadManager"}
  checkFeed: {
    fetchDenied: "Skipped feed %s: it's blocked, or untrusted while fetchUntrustedFeeds is 'never'. Trust the feed to update from it."
    downloadFailed: "Failed to download feed: %s"
    badChannel: "The specified update channel '%s' wasn't present in the feed."
    invalidVersion: "The feed contains an invalid version record for %s '%s' (channel: %s): %s."
  }
  run: {
    starting: "Starting %s of %s '%s'... "
    -- %s fills to "" or "re", composing "fetch"/"refetch"
    -- cspell:ignore sfetch
    fetching: "Trying to %sfetch missing %s '%s'..."
    feedChecking: "Checking feed %s..."
    upToDate: "The %s '%s' is up-to-date (v%s)."
    alreadyUpdated: "%s v%s has already been installed."
    noFeedAvailableExt: "(required: %s; installed: %s; available: %s)"
    noPlatformAvailable: "for your platform (%s); a build is available only for %s"
    skippedOptional: "Skipped %s of optional dependency '%s': %s"
    optionalNoUpdate: "No suitable download could be found %s."
    optionalUntrusted: "a suitable package was only found in an untrusted feed (%s)."
    optionalBlocked: "you blocked the feed (%s) it would be installed from."
    optionalPinnedUnavailable: "its pinned package source is no longer available."
    optionalAborted: "aborted at your request."
    providerAmbiguous: "Multiple modules provide '%s'; selected '%s' (candidates: %s)."
    providerResolved: "Satisfying required module '%s' with provider %s '%s' (v%s)."
    providerInstallFailed: "Found a provider for '%s' (%s) but it couldn't be installed: %s"
    untrustedPrompt: "The %s of %s '%s' can only be satisfied from a feed DependencyControl doesn't trust:\n%s\n\nTrust this feed and proceed?"
    choosePrompt: "More than one source can supply '%s'. Choose which one to install:"
    choiceUnavailable: "Your remembered source for '%s' is no longer available. Please choose again:"
    choiceUntrustedFlag: " [untrusted]"
  }

  performUpdate: {
    updateReqs: "Checking requirements..."
    updateReady: "Update ready. Using temporary directory '%s'."
    fileUnchanged: "Skipped unchanged file '%s'."
    fileAddDownload: "Added Download %s ==> '%s'."
    filesDownloading: "Downloading %d files..."
    movingFiles: "Downloads complete. Now moving files to Aegisub automation directory '%s'..."
    movedFile: "Moved '%s' ==> '%s'."
    moveFileFailed: "Failed to move '%s' ==> '%s': %s"
    updSuccess: "%s of %s '%s' (v%s) complete."
    reloadNotice: "Please rescan your autoload directory for the changes to take effect."
    unknownType: "Skipping file '%s': unknown type '%s'."
  }
  refreshRecord: {
    unsetVirtual: "Update initiated by another macro already fetched %s '%s', switching to update mode."
    otherUpdate: "Update initiated by another macro already updated %s '%s' to v%s."
  }
  __promptTrustFeed: {
    trustOnce: "Trust this time"
    trustAlways: "Always trust this feed"
    trustNever: "Never (block this feed)"
  }
  promptSelectSource: {
    once: "Just This Once"
    retain: "Remember"
    pinned: "Pin/Lock"
    auto: "Auto-Pick"
    abort: "Cancel"
  }
  dialogCommon: {
    ok: "OK"
    cancel: "Cancel"
  }
}

---Mutable execution state for one install/update operation.
---@class UpdateTask
---@field private __installingVersion? string The version this task's running update will install. Set only while it resolves its requirements and used to break module dependency cycles.
class UpdateTask
  ---@private
  @__downloader = Downloader!
  ---DependencyControl's own class, required lazily to break the circular dependency.
  ---@private
  @__DependencyControl = nil

  -- Defaults for the prompt-threshold `updates` settings this class owns, applied when the config key is unset.
  ---@type UpdateContextCeiling
  @defaultFeedTrustPromptThreshold = ContextCeiling.AutoUpdate
  ---@type UpdateContextCeiling
  @defaultPackageChoicePromptThreshold = ContextCeiling.UserRequested

  ---Reports whether an update context is allowed under a ceiling on the context ladder.
  ---@param reason UpdateReason The context asking to act.
  ---@param ceiling? UpdateContextCeiling The most autonomous context still allowed.
  ---@return boolean within True when the context sits at or below the ceiling; false when the ceiling is `off`, unset, or unrecognized.
  @isWithinContextCeiling = (reason, ceiling) ->
    ceilingRank = contextRank[ceiling]
    ceilingRank != nil and (contextRank[reason] or math.huge) <= ceilingRank

  ---Converts updater status/error codes into user-facing error messages.
  ---@param code? UpdateStatus A nil or unmapped code yields a generic message naming the code.
  ---@param name string
  ---@param scriptType ScriptType A domain.ScriptType value.
  ---@param isInstall boolean
  ---@param detailMsg? string
  ---@return string message The user-facing error text for the status code.
  @getUpdaterErrorMsg = (code, name, scriptType, isInstall, detailMsg) ->
    isInstall or= false -- terms.isInstall is keyed by true/false; tolerate a nil argument
    detailMsg or= "" -- a template's trailing %s must never format a nil detail
    if code and code <= -100
      -- a component-encoded status packs its component id as floor(-code / 100)
      return msgs.updateError.component\format -code, msgs.updaterErrorComponent[math.floor(-code/100)],
        domain.terms.isInstall[isInstall], domain.terms.scriptType.singular[scriptType], name, detailMsg
    template = msgs.updateError[code]
    unless template
      return msgs.updateError.unknown\format domain.terms.isInstall[isInstall],
        domain.terms.scriptType.singular[scriptType], name, tostring code
    return template\format domain.terms.isInstall[isInstall],
      domain.terms.scriptType.singular[scriptType],
      name, detailMsg

  ---Creates an update task for one record.
  ---@param record PackageRecord
  ---@param targetVersionNumber? number Minimum version to install (default 0, i.e. any).
  ---@param addFeeds? string[]
  ---@param optional? boolean Treat this as an optional dependency.
  ---@param channel? string Update channel to use.
  ---@param reason? UpdateReason Why this task runs; a prompt is allowed only when this reason is permitted by that prompt kind's configured threshold.
  ---@param updater Updater
  new: (@record, targetVersionNumber = 0, @addFeeds, @optional, @channel, @reason, @updater) =>
    @@__DependencyControl or= require "l0.DependencyControl"
    assert @record.__class == @@__DependencyControl, msgs.new.badRecord\format @@__DependencyControl.__name
    assert type(targetVersionNumber) == "number", msgs.new.badTargetVersion

    @logger = @updater.logger
    @status = nil
    @targetVersion = targetVersionNumber

  ---Loads a candidate feed, downloading it if necessary.
  ---@param feedUrl string
  ---@return UpdateFeed? feed The loaded feed, or nil on download failure.
  ---@return string? err Error message on failure.
  ---@private
  __loadFeed: (feedUrl) =>
    -- Refuse a blocked feed outright, and an untrusted one while fetchUntrustedFeeds is 'never'; the
    -- surfaced reason keeps a skipped update from being silent. Untrusted feeds under always/prompt are
    -- still fetched — the install-trust prompt (feedTrustPromptThreshold) gates acting on them.
    feedTrust = @updater.feedTrust
    return nil, msgs.checkFeed.fetchDenied\format feedUrl if feedTrust and feedTrust\getFetchDecision(feedUrl) == FeedTrust.FetchDecision.Deny

    feed = @updater.feedLoader\load feedUrl, {autoLoad: false}
    -- ensureLoaded reuses an in-memory/on-disk copy where valid, otherwise downloads
    data, err = feed\ensureLoaded!
    return nil, msgs.checkFeed.downloadFailed\format err unless data
    return feed

  ---Looks up this task's module in an already-loaded feed (matched by namespace), returning its
  ---update record on the task's channel along with the parsed release version.
  ---@param feed UpdateFeed A feed loaded via __loadFeed.
  ---@return ScriptUpdateRecord|nil record The module's update record, or nil if absent/unusable.
  ---@return string? err Error message worth reporting (nil when the module simply isn't in this feed).
  ---@return number? version The candidate's parsed version number.
  ---@private
  checkFeed: (feed) =>
    updateRecord, err = feed\getScript @record.namespace, @record.scriptType, @record.config, false
    return nil, err unless updateRecord -- err is nil for "not in this feed", set for a real error

    success, currentChannel = updateRecord\setChannel @channel
    return nil, msgs.checkFeed.badChannel\format currentChannel unless success

    version = SemanticVersion\toPacked updateRecord.version
    unless version
      return nil, msgs.checkFeed.invalidVersion\format domain.terms.scriptType.singular[@record.scriptType],
        @record.name, currentChannel, tostring updateRecord.version
    return updateRecord, nil, version

  ---Resolves the feed URL a persisted source record maps to: its own `feedUrl` when it stores one,
  ---otherwise derived from the owning package's feed fields by source kind.
  ---@param source SourceChoiceRecord The persisted source record.
  ---@param selfFeed? string The package's declared feed (used for a self-declared source).
  ---@param userFeed? string The package's per-package override feed (used for a user-feed source).
  ---@param modulesSection? table<string, table> The modules config section (used to resolve a provider source).
  ---@return string? url The resolved feed URL, or nil if it can't be determined.
  @resolveSourceUrl = (source, selfFeed, userFeed, modulesSection) ->
    -- Source records used for `configuredSource` omit the feed URL when that is also stored
    -- elsewhere in the config to maintain a single source of truth. In contrast, `currentSource`
    -- always stores it to preserve provenance when those fields change.
    return source.feedUrl if source.feedUrl
    switch source.feedSource
      when SourceFeedKind.SelfDeclared then selfFeed
      when SourceFeedKind.UserFeed then userFeed
      when SourceFeedKind.Other then source.feedUrl
      when SourceFeedKind.Provider
        return nil unless source.provider and source.provider.namespace
        provider = modulesSection and modulesSection[source.provider.namespace]
        provider and provider.feed

  ---Resolves the feed URL a remembered source maps to. Every kind but `Other` derives its URL from
  ---current state, so a remembered choice survives a feed-URL migration.
  ---@param previousSource SourceChoiceRecord A persisted `currentSource` table.
  ---@return string? feedUrl The derived feed URL, or nil if it can't be determined.
  ---@private
  __resolveRememberedFeedUrl: (previousSource) =>
    view = @updater.config\getSectionHandler domain.ScriptTypeSection[domain.ScriptType.Module]
    @@.resolveSourceUrl previousSource, @record.feed, @record.config.c.userFeed, view and view.c

  ---Finds the candidate corresponding to a remembered source that is still eligible to satisfy this task.
  ---@param candidates CandidatePackageSource[] Pooled candidates.
  ---@param previousSource SourceChoiceRecord A persisted `currentSource` table.
  ---@return CandidatePackageSource? candidate The matching, eligible candidate, or nil if none matches or it's no longer eligible.
  ---@private
  __matchRememberedCandidate: (candidates, previousSource) =>
    wantUrl = @__resolveRememberedFeedUrl previousSource
    return nil unless wantUrl
    viaProvider = previousSource.feedSource == SourceFeedKind.Provider
    for candidate in *candidates
      continue unless candidate.feedUrl == wantUrl
      if viaProvider
        continue if candidate.isDirect
        continue unless previousSource.provider and candidate.updateRecord.namespace == previousSource.provider.namespace
      else
        continue unless candidate.isDirect
      return candidate if @__getCandidateRankVersion candidate
    return nil

  ---Classifies which kind of source a provided candidate came from.
  ---@param candidate CandidatePackageSource The candidate to classify.
  ---@return SourceFeedKind kind The candidate's source kind (self-declared, user-feed, provider, or other).
  ---@private
  __feedSourceOf: (candidate) =>
    return SourceFeedKind.Provider unless candidate.isDirect
    return SourceFeedKind.SelfDeclared if candidate.feedUrl == @record.feed
    return SourceFeedKind.UserFeed if candidate.feedUrl == @record.config.c.userFeed
    return SourceFeedKind.Other

  ---Records which source satisfied this task and how sticky the choice is, in the per-package
  ---`configuredSource` config, so later resolutions can honor it. The feed's channel lineup is
  ---refreshed alongside. Writes only when something changed.
  ---@param selectedCandidate CandidatePackageSource The chosen candidate.
  ---@param stickiness? SourceChoiceStickiness The stickiness to record (defaults to the existing one, else `unset`).
  ---@private
  __persistSource: (selectedCandidate, stickiness) =>
    return unless @record.config
    existing = ScriptUpdateRecord.getRecordedSource @record.config.c
    feedSource = @__feedSourceOf selectedCandidate
    configuredSource = {
      :feedSource
      channel: selectedCandidate.updateRecord.activeChannel or @channel
      stickiness: stickiness or (existing and existing.stickiness) or SourceChoiceStickiness.Unset
    }
    configuredSource.feedUrl = selectedCandidate.feedUrl if feedSource == SourceFeedKind.Other
    unless selectedCandidate.isDirect
      configuredSource.provider = {namespace: selectedCandidate.updateRecord.namespace, version: selectedCandidate.providesVersion}

    -- sorted, so the persisted lineup doesn't ride on table order and rewrite the config every session
    channelLineup = selectedCandidate.updateRecord\getChannels!
    table.sort channelLineup
    lineupUnchanged = utils.itemsEqual channelLineup, @record.config.c.channels or {}

    unchanged = lineupUnchanged and @record.config.c.configuredSource and existing and
      existing.feedSource == configuredSource.feedSource and
      existing.channel == configuredSource.channel and existing.stickiness == configuredSource.stickiness and
      existing.feedUrl == configuredSource.feedUrl and
      (existing.provider and existing.provider.namespace) == (configuredSource.provider and configuredSource.provider.namespace) and
      (existing.provider and existing.provider.version) == (configuredSource.provider and configuredSource.provider.version)
    return if unchanged

    @record.config.c.channels = channelLineup
    @record.config.c.configuredSource = configuredSource
    @record.config\save!

  ---Persists the provenance of the package just installed or updated to the per-package `currentSource`
  ---property in the DepCtrl config file. `lastChannel` kept in step as its pre-0.7 single-field form.
  ---@private
  __recordInstalledSource: =>
    return unless @record.config
    source = ScriptUpdateRecord.getRecordedSource @record.config.c
    return unless source
    installed = utils.deepCopy source
    -- the URL is stamped rather than left to be derived later, so changing a userFeed or a declared
    -- feed afterwards doesn't rewrite where this copy is recorded as having come from
    installed.feedUrl or= @__resolveRememberedFeedUrl source
    with @record.config
      .c.currentSource = installed
      .c.lastChannel = installed.channel
      \save!

  ---The version this candidate is ranked by, or nil when it can't satisfy the task. A direct candidate ranks by
  ---its release version, a provider by the highest version its declared alias range covers (any version if none).
  ---@param candidate CandidatePackageSource
  ---@return number? rankVersion The ranking version, or nil if the candidate is ineligible.
  ---@private
  __getCandidateRankVersion: (candidate) =>
    local rankVersion
    if candidate.isDirect
      versionNumber = SemanticVersion\toPacked candidate.updateRecord.version
      return nil unless versionNumber and SemanticVersion\check versionNumber, @targetVersion
      rankVersion = versionNumber
    else
      -- a provider is matched and ranked by its declared alias range, defaulting to any version
      range = candidate.providesVersion or "*"
      return nil unless SemanticVersion\rangesIntersect range, ">=#{SemanticVersion\toString @targetVersion}"
      rankVersion = SemanticVersion\getRangeMaxVersion range
      return nil unless rankVersion
    return nil unless candidate.updateRecord\checkPlatform!
    return nil unless candidate.updateRecord.files and #candidate.updateRecord.files > 0
    rankVersion

  ---Selects the best candidate to satisfy this task's requirement from a pooled set, ranking by trust band, then
  ---version, then a deterministic tie-break.
  ---@param candidates CandidatePackageSource[] Pooled candidates, channel already selected on each record.
  ---@return CandidatePackageSource? selected The chosen candidate, or nil when none is eligible.
  ---@return CandidatePackageSource[]? tied The selected candidate plus any others tied with it on band and version (for an interactive chooser); nil when none is eligible.
  ---@return CandidatePackageSource[]? eligible All eligible candidates, sorted best-first (for the offer-all-sources chooser); nil when none is eligible.
  ---@private
  __selectCandidate: (candidates) =>
    eligibleCandidates = {}
    for candidate in *candidates
      rankVersion = @__getCandidateRankVersion candidate
      eligibleCandidates[#eligibleCandidates + 1] = {:candidate, :rankVersion} if rankVersion
    return nil if #eligibleCandidates == 0

    declaredFeed = @record.feed
    table.sort eligibleCandidates, (a, b) ->
      return a.candidate.trustBand < b.candidate.trustBand if a.candidate.trustBand != b.candidate.trustBand
      return a.rankVersion > b.rankVersion if a.rankVersion != b.rankVersion
      aDeclared, bDeclared = a.candidate.feedUrl == declaredFeed, b.candidate.feedUrl == declaredFeed
      return aDeclared if aDeclared != bDeclared
      return a.candidate.updateRecord.namespace < b.candidate.updateRecord.namespace

    winner, topVersion = eligibleCandidates[1].candidate, eligibleCandidates[1].rankVersion
    tied = [e.candidate for e in *eligibleCandidates when e.candidate.trustBand == winner.trustBand and e.rankVersion == topVersion]
    if #tied > 1
      @logger\log msgs.run.providerAmbiguous, @record.namespace, winner.updateRecord.namespace,
        table.concat [c.updateRecord.namespace for c in *tied], ", "
    return winner, tied, [e.candidate for e in *eligibleCandidates]

  ---Installs a module that `provides` this task's required module, satisfying the requirement
  ---indirectly with that provider.
  ---@param provider ScriptUpdateRecord The selected provider's feed record.
  ---@param feedUrl string The feed the provider was found in, used as its primary feed.
  ---@return any ref The loaded provider module reference, or nil on failure.
  ---@return UpdateStatus? code Status of the provider's update run. Always present when the ref is nil.
  ---@return string? detail Error detail on failure.
  ---@private
  __installProvider: (provider, feedUrl) =>
    @@__DependencyControl or= require "l0.DependencyControl"
    providerRecord = @@.__DependencyControl {
      moduleName: provider.namespace, name: provider.name or provider.namespace,
      version: -1, virtual: true, feed: feedUrl, url: provider.url
    }
    addFeeds = [feed for feed in *@addFeeds]
    addFeeds[#addFeeds + 1] = feedUrl
    @updater\require providerRecord, @targetVersion, addFeeds, @optional, nil, @reason

  ---Reports whether this task may prompt the user for a given kind of prompt,
  ---e.g. to approve an untrusted feed or choose among multiple eligible candidates.
  ---@param threshold? UpdateContextCeiling The configured ceiling for this prompt kind.
  ---@return boolean allowed True when a prompt of this kind is permitted for this task's reason.
  ---@private
  __shouldPrompt: (threshold) =>
    return false unless @reason
    @@.isWithinContextCeiling @reason, threshold

  ---Asks the user whether to proceed with a candidate from an untrusted feed. Depending on the user's choice,
  ---the feed may be added to the trusted or blocked lists.
  ---@param selectedCandidate CandidatePackageSource A candidate source from an untrusted feed.
  ---@return FeedTrustDecision? decision The user's decision, or nil if they cancelled.
  ---@private
  __promptTrustFeed: (selectedCandidate) =>
    msg = msgs.run.untrustedPrompt\format getInstallTerm(@record),
      domain.terms.scriptType.singular[@record.scriptType],
      @record.name, selectedCandidate.feedUrl
    dlg = {{class: "label", label: msg, x: 0, y: 0, width: 1, height: 1}}
    buttons = {msgs.dialogCommon.cancel, msgs.__promptTrustFeed.trustOnce, msgs.__promptTrustFeed.trustAlways,
      msgs.__promptTrustFeed.trustNever}

    btn = aegisub.dialog.display dlg, buttons, {cancel: msgs.dialogCommon.cancel}
    return nil if not btn or btn == msgs.dialogCommon.cancel

    switch btn
      when msgs.__promptTrustFeed.trustAlways
        @updater.feedTrust\trust selectedCandidate.feedUrl
        return FeedTrustDecision.Always
      when msgs.__promptTrustFeed.trustNever
        @updater.feedTrust\block selectedCandidate.feedUrl
        return FeedTrustDecision.Never
    FeedTrustDecision.Once

  ---Lets the user pick a package source among the eligible candidates and how sticky that pick should be.
  ---Untrusted candidates are flagged in the list. Choosing "Auto-Pick" keeps the algorithm's pre-selected candidate.
  ---@param candidates CandidatePackageSource[] The candidates to choose from.
  ---@param selectedCandidate? CandidatePackageSource The candidate to pre-select (defaults to the first).
  ---@param noLongerAvailable? boolean Show the "remembered source unavailable" prompt instead of the default one.
  ---@return CandidatePackageSource? chosen The picked candidate, or nil if the user aborted.
  ---@return SourceChoiceStickiness? stickiness How sticky the pick should be, or nil if the user aborted.
  ---@private
  __promptSelectPackageSource: (candidates, selectedCandidate = candidates[1], noLongerAvailable) =>
    labelFor = (candidate) ->
      label = "#{candidate.updateRecord.name or candidate.updateRecord.namespace} (#{candidate.feedUrl})"
      label ..= msgs.run.choiceUntrustedFlag if candidate.trustBand and candidate.trustBand >= TrustBand.UntrustedDirect
      label
    candidatesByLabel = {labelFor(candidate), candidate for candidate in *candidates}

    prompt = noLongerAvailable and msgs.run.choiceUnavailable or msgs.run.choosePrompt
    dlg = {
      {class: "label", label: prompt\format(@record.namespace), x: 0, y: 0, width: 2, height: 1}
      {class: "dropdown", name: "choice", items: [labelFor c for c in *candidates], value: labelFor(selectedCandidate),
        x: 0, y: 1, width: 2, height: 1}
    }
    buttons = {msgs.promptSelectSource.once, msgs.promptSelectSource.retain, msgs.promptSelectSource.pinned,
      msgs.promptSelectSource.auto, msgs.promptSelectSource.abort}
    btn, res = aegisub.dialog.display dlg, buttons, {cancel: msgs.promptSelectSource.abort}

    switch btn
      when msgs.promptSelectSource.auto then return selectedCandidate, SourceChoiceStickiness.Auto
      when msgs.promptSelectSource.once then return (candidatesByLabel[res.choice] or selectedCandidate), SourceChoiceStickiness.Once
      when msgs.promptSelectSource.retain then return (candidatesByLabel[res.choice] or selectedCandidate), SourceChoiceStickiness.Retain
      when msgs.promptSelectSource.pinned then return (candidatesByLabel[res.choice] or selectedCandidate), SourceChoiceStickiness.Pinned
    return nil, nil

  ---Runs the full update/install flow for this task.
  ---Acquires the global updater lock but does not release it — the caller (a PackageRecord.requireModules /
  ---macro-hook / Toolbox entry point) must call `updater\releaseLock` once its whole operation is done,
  ---or the lock is held until its lease lapses (orphanTimeout), blocking other scripts' updates.
  ---@param waitLock? boolean Wait for a concurrent update to finish instead of bailing.
  ---@return UpdateStatus statusCode
  ---@return any detail
  run: (waitLock) =>
    with @record do @logger\log msgs.run.starting, getInstallTerm(@record),
      domain.terms.scriptType.singular[.scriptType], .name

    -- The field is only set while this task's own update resolves its requirements, so re-entering here
    -- means we must be dealing with a dependency cycle. This short-circuits the requirement check if the
    -- requested version is satisfied by the version being installed, so the cycle can be broken.
    if @__installingVersion
      satisfied = not @targetVersion or SemanticVersion\check @__installingVersion, @targetVersion
      return UpdateStatus.UpdateInProgress, @__installingVersion if satisfied
      return @__logUpdateError UpdateStatus.TaskAlreadyRunning

    -- don't perform update of a script when another one is already running for the same script
    return @__logUpdateError UpdateStatus.TaskAlreadyRunning if @running

    -- don't shadow scripts installed to the ?data automation dir with a ?user copy
    entryPath, isUserPath = @record\getEntryPointPath!
    if isUserPath == false
      return @__logUpdateError UpdateStatus.ProtectedInstall, entryPath

    -- check if the script was already updated
    if @updated and @record\checkVersion @targetVersion
      @logger\log msgs.run.alreadyUpdated, @record.name, SemanticVersion\toString @record.version
      return UpdateStatus.AlreadyUpdated

    -- check internet connection
    return @__logUpdateError UpdateStatus.NoInternet unless @@__downloader\isInternetConnected!

    -- get a lock on the updater
    success, otherHost = @updater\acquireLock waitLock
    return @__logUpdateError UpdateStatus.AnotherUpdateRunning, otherHost unless success

    resolution = @__resolve!
    return resolution.statusCode, resolution.statusDetailMessage unless resolution.installRequired
    selectedSource, stickiness, maxVersion = resolution.selectedSource, resolution.stickiness, resolution.maxVersion

    -- remember which source satisfied this package and how sticky the choice is, for next time
    @__persistSource selectedSource, stickiness

    -- an installed module already satisfies the chosen (trusted) version
    if selectedSource.isDirect and not @record.virtual and @record\checkVersion selectedSource.updateRecord.version
      @logger\log msgs.run.upToDate, domain.terms.scriptType.singular[@record.scriptType],
        @record.name, SemanticVersion\toString @record.version
      return UpdateStatus.UpToDate

    wasVirtual = @record.virtual
    if selectedSource.isDirect
      code, res = @performUpdate selectedSource.updateRecord
      -- A fresh install resolves on a virtual record, whose config view has no file behind it, so
      -- everything recorded about the source during the install went nowhere durable. The install
      -- adopted the real record, whose config can hold it, so record the source and provenance again.
      if wasVirtual and not @record.virtual
        @__persistSource selectedSource, stickiness
        @__recordInstalledSource!
      return @__logUpdateError code, res, wasVirtual

    -- for an indirect source, install the chosen provider in place of the required module
    ref, code, extErr = @__installProvider selectedSource.updateRecord, selectedSource.feedUrl
    unless ref
      @logger\trace msgs.run.providerInstallFailed, @record.namespace, selectedSource.updateRecord.namespace, tostring extErr
      code, detail = @__reportNoSuitablePackage maxVersion
      return code, detail
    @ref, @updated = ref, true
    @__recordInstalledSource!
    @logger\log msgs.run.providerResolved, @record.namespace, domain.terms.scriptType.singular[domain.ScriptType.Module],
      selectedSource.updateRecord.name or selectedSource.updateRecord.namespace, selectedSource.updateRecord.version
    return UpdateStatus.Installed, selectedSource.updateRecord.version

  ---Logs the error message for a negative status code and returns the code and detail unchanged.
  ---Non-negative (success/skip) codes are returned without logging.
  ---@param statusCode UpdateStatus The updater status code.
  ---@param statusDetailMessage? string a message with further explanation of the outcome, if any.
  ---@param virtual? boolean Whether this is a fresh install (default: the record's current virtual flag).
  ---@return UpdateStatus statusCode the same code passed in.
  ---@return string? statusDetailMessage  the same status detail message passed in, if any.
  ---@private
  __logUpdateError: (statusCode, statusDetailMessage, virtual = @record.virtual) =>
    if statusCode < 0
      @logger\log UpdateTask.getUpdaterErrorMsg statusCode, @record.name, @record.scriptType, virtual, statusDetailMessage
    return statusCode, statusDetailMessage

  ---The platforms a version-satisfying build is offered for when none covers the current platform, so a
  ---"no suitable package" failure can be attributed to a platform mismatch.
  ---@param candidates CandidatePackageSource[] The candidates pooled during resolution.
  ---@return string[] platforms Offered platforms, sorted and de-duplicated; empty when the shortfall isn't a platform mismatch.
  ---@private
  __getOfferedBuildPlatforms: (candidates) =>
    offered = {}
    for candidate in *candidates
      continue unless candidate.isDirect
      versionNumber = SemanticVersion\toPacked candidate.updateRecord.version
      continue unless versionNumber and SemanticVersion\check versionNumber, @targetVersion
      continue if candidate.updateRecord\checkPlatform!
      offered[platform] = true for platform in *(candidate.updateRecord.platforms or {})
    platforms = [platform for platform in pairs offered]
    table.sort platforms
    return platforms

  ---Logs and returns this task's "no suitable package" status — a skip if optional, else a failure.
  ---@param maxVersion number The highest candidate version seen during resolution (0 when none was found).
  ---@param offeredPlatforms? string[] The platforms a version-satisfying build is offered for; when non-empty, the failure is reported as a platform mismatch rather than a version shortfall.
  ---@return UpdateStatus statusCode A negative failure code for a required dependency, or the skip code (3) for an optional one.
  ---@return string? statusDetailMessage The availability summary for a failure; nil for an optional skip.
  ---@private
  __reportNoSuitablePackage: (maxVersion, offeredPlatforms) =>
    detail = if offeredPlatforms and #offeredPlatforms > 0
      msgs.run.noPlatformAvailable\format environment.platform, table.concat offeredPlatforms, ", "
    else
      msgs.run.noFeedAvailableExt\format @targetVersion == 0 and "any" or SemanticVersion\toString(@targetVersion),
        @record.virtual and "no" or SemanticVersion\toString(@record.version),
        maxVersion < 1 and "none" or SemanticVersion\toString maxVersion
    if @optional
      @logger\log msgs.run.skippedOptional, getInstallTerm(@record), @record.name,
        msgs.run.optionalNoUpdate\format detail
      return UpdateStatus.SkippedOptional
    return @__logUpdateError UpdateStatus.NoSuitablePackage, detail

  ---Resolves which package source should satisfy this task, without installing anything. May fetch feeds
  ---and prompt the user (to choose a package source or to approve an untrusted feed). The updater lock
  ---must already be held.
  ---@return UpdaterResolution resolution The source to install, or a status code to return when no install is needed.
  ---@private
  __resolve: =>
    withoutInstall = (statusCode, statusDetailMessage) -> {installRequired: false, :statusCode, :statusDetailMessage}

    -- Candidates are ranked by trust band. Feeds are fetched lazily per band, so we only reach for
    -- less-trusted feeds when no closer source can satisfy.
    config, userFeed, declaredFeed = @updater.config.c.updates, @record.config.c.userFeed, @record.feed
    feedTrust = @updater.feedTrust
    isBlocked = (url) -> feedTrust\isBlocked url
    -- a user override feed counts as trusted for this resolution (unless block-listed), without polluting the shared set
    userFeedTrusted = userFeed and not isBlocked userFeed
    isTrusted = (url) -> feedTrust\isTrusted(url) or (userFeedTrusted and url == userFeed)

    remembered = ScriptUpdateRecord.getRecordedSource @record.config.c
    stickiness = remembered and remembered.stickiness or SourceChoiceStickiness.Unset
    -- a remembered provider stays pinned to its band so a version bump updates it in place instead of switching providers
    stickyProvider = remembered and remembered.provider and remembered.provider.namespace

    bandOf = (feedUrl, direct) ->
      if isTrusted feedUrl
        return direct and (feedUrl == declaredFeed and TrustBand.DeclaredDirect or TrustBand.TrustedDirect) or TrustBand.TrustedProvider
      return direct and TrustBand.UntrustedDirect or TrustBand.UntrustedProvider

    maxVer, candidates, triedFeeds = 0, {}, {}
    moduleConfigView = @updater.config\getSectionHandler domain.ScriptTypeSection[domain.ScriptType.Module]
    -- Gather candidates from a list of feed URLs, skipping any that are blocked or already tried.
    gather = (feedUrls) ->
      for feedUrl in *(feedUrls or {})
        continue if not feedUrl or triedFeeds[feedUrl] or isBlocked feedUrl
        triedFeeds[feedUrl] = true
        @updater\renewLock!
        @logger\trace msgs.run.feedChecking, feedUrl
        feed, errMsg = @__loadFeed feedUrl
        unless feed
          @logger\log errMsg
          continue
        rec, errMsg, version = @checkFeed feed

        if rec
          maxVer = math.max maxVer, version
          candidates[#candidates + 1] = {updateRecord: rec, feedUrl: feedUrl, isDirect: true, trustBand: bandOf(feedUrl, true)}
        elseif errMsg
          @logger\log errMsg
        if @record.virtual
          for provider in *feed\getProviders @record.namespace, (moduleConfigView and moduleConfigView.c) or {}
            -- the version range this provider declares for the required alias, if any
            providesVersions = [e.version for e in *(provider.provides or {}) when type(e) == "table" and e.name == @record.namespace]
            -- a trusted candidate from the sticky (remembered/installed) provider stays pinned (declared-direct band)
            trustBand = (provider.namespace == stickyProvider and isTrusted feedUrl) and TrustBand.DeclaredDirect or bandOf(feedUrl, false)
            candidates[#candidates + 1] = {updateRecord: provider, feedUrl: feedUrl, isDirect: false, :trustBand, providesVersion: providesVersions[1]}

    @logger.indent += 1

    local selected, tied, eligible
    -- A hard pin or soft-remember reuses the remembered source directly: load its feed first (it may
    -- sit in a lower or untrusted tier the cascade would never reach) and reuse it if still eligible.
    -- An exclusive userFeed still constrains it, so a remembered source outside userFeed counts as gone.
    reuse = nil
    if stickiness == SourceChoiceStickiness.Pinned or stickiness == SourceChoiceStickiness.Retain
      rememberedUrl = @__resolveRememberedFeedUrl remembered
      if rememberedUrl and (not userFeed or rememberedUrl == userFeed)
        gather {rememberedUrl}
        reuse = @__matchRememberedCandidate candidates, remembered

    if reuse
      selected = reuse
    elseif stickiness == SourceChoiceStickiness.Pinned
      -- pinned, but the remembered source is gone: don't silently switch to another one
      selected = nil
    elseif userFeed
      gather {userFeed}
      selected, tied, eligible = @__selectCandidate candidates
    else
      -- tier 1: the feed declared by / advertised in the record
      gather {declaredFeed}
      selected, tied, eligible = @__selectCandidate candidates

      unless selected and selected.trustBand == TrustBand.DeclaredDirect
        -- tier 2: the trusted discovery feeds — the user's extra feeds, trusted add-feeds, and the
        -- official set. (extraFeeds lives in the `feeds` section, not `updates`.)
        gather @updater.config.c.feeds.extraFeeds
        gather [url for url in *@addFeeds when isTrusted url]
        gather [url for url in pairs(feedTrust\getOfficialTrustedFeeds!)] unless @optional -- don't trigger a registry-wide crawl for a nice-to-have
        selected, tied, eligible = @__selectCandidate candidates

        unless selected and selected.trustBand <= TrustBand.TrustedProvider
          -- tier 3: untrusted feeds
          gather [url for url in *@addFeeds when not isTrusted url]
          selected, tied, eligible = @__selectCandidate candidates
    @logger.indent -= 1

    abortResolution = ->
      if @optional
        @logger\log msgs.run.skippedOptional, getInstallTerm(@record), @record.name,
          msgs.run.optionalAborted
        return UpdateStatus.SkippedOptional
      return @__logUpdateError UpdateStatus.UserAborted

    -- a hard pin whose remembered source vanished aborts (required) or skips (optional)
    if stickiness == SourceChoiceStickiness.Pinned and not reuse
      if @optional
        @logger\log msgs.run.skippedOptional, getInstallTerm(@record), @record.name,
          msgs.run.optionalPinnedUnavailable
        return withoutInstall UpdateStatus.SkippedOptional
      code, detail = @__logUpdateError UpdateStatus.PinnedUnavailable
      return withoutInstall code, detail

    unless selected
      if maxVer > 0 and not @record.virtual and @targetVersion <= @record.version
        -- dependency is already up-to-date, so no matter we don't have a candidate to install
        @logger\log msgs.run.upToDate, domain.terms.scriptType.singular[@record.scriptType],
          @record.name, SemanticVersion\toString @record.version
        return withoutInstall UpdateStatus.UpToDate

      code, detail = @__reportNoSuitablePackage maxVer, @__getOfferedBuildPlatforms candidates
      return withoutInstall code, detail

    -- consult the remembered source choice to decide whether to let the user pick a package source.
    -- `auto` never asks and the reuse path already settled the pick, so neither prompts here.
    unless reuse or stickiness == SourceChoiceStickiness.Auto
      -- present every eligible candidate when configured to, otherwise only an exact band/version tie
      choices = config.offerAllSources and eligible or tied
      allowPrompt = @__shouldPrompt(config.packageChoicePromptThreshold or @@defaultPackageChoicePromptThreshold)
      remPick = remembered and @__matchRememberedCandidate candidates, remembered

      if stickiness == SourceChoiceStickiness.Retain
        -- the soft-remembered pick is gone: re-ask in interactive mode, downgrade to `once` otherwise
        if allowPrompt
          pick, pickType = @__promptSelectPackageSource choices, selected, true
          unless pick
            code, detail = abortResolution!
            return withoutInstall code, detail
          selected, stickiness = pick, pickType
        else
          stickiness = SourceChoiceStickiness.Once
      elseif choices and #choices > 1 and allowPrompt
        pick, pickType = @__promptSelectPackageSource choices, (remPick or selected)
        unless pick
          code, detail = abortResolution!
          return withoutInstall code, detail
        selected, stickiness = pick, pickType

    -- the chosen candidate is from an untrusted feed: install it only if the user is asked and approves
    if selected.trustBand >= TrustBand.UntrustedDirect
      trustDecision = @__shouldPrompt(@updater.config.c.updates.feedTrustPromptThreshold or @@defaultFeedTrustPromptThreshold) and @__promptTrustFeed selected
      unless trustDecision == FeedTrustDecision.Once or trustDecision == FeedTrustDecision.Always
        userBlockedFeed = trustDecision == FeedTrustDecision.Never
        if @optional
          reason = (userBlockedFeed and msgs.run.optionalBlocked or msgs.run.optionalUntrusted)\format selected.feedUrl
          @logger\log msgs.run.skippedOptional, getInstallTerm(@record), @record.name, reason
          return withoutInstall UpdateStatus.SkippedOptional
        code, detail = @__logUpdateError (userBlockedFeed and UpdateStatus.BlockedFeed or UpdateStatus.UntrustedFeed), selected.feedUrl
        return withoutInstall code, detail

    return {installRequired: true, selectedSource: selected, :stickiness, maxVersion: maxVer}

  ---Downloads and installs files for a selected update entry.
  ---@param update ScriptUpdateRecord
  ---@return UpdateStatus statusCode
  ---@return table|string|nil detail
  ---@private
  performUpdate: (update) =>
    finish = (...) ->
      @running = false
      if @record.virtual or @record.recordType == domain.RecordType.Unmanaged
        ModuleLoader.removeDummyRef @record
      return ...

    @running = true

    -- set a dummy ref (which hasn't yet been set for virtual and unmanaged modules)
    -- and record version to allow resolving circular dependencies
    if @record.virtual or @record.updateRecordType == domain.RecordType.Unmanaged
      ModuleLoader.createDummyRef @record
      @record\setVersion update.version

    -- try to load required modules first to see if all dependencies are satisfied
    -- this may trigger more updates
    reqs = update.requiredModules
    if reqs and #reqs > 0
      @logger\log msgs.performUpdate.updateReqs
      @logger.indent += 1
      -- Remember the version being installed to break dependency cycles, but prevent it from becoming stale, e.g.,
      -- in case of an update error, as it could otherwise corrupt subsequent update runs in an unpredictable way.
      @__installingVersion = update.version
      loaded, success, err = pcall ModuleLoader.loadModules, @record, reqs, {@record.feed}
      @__installingVersion = nil
      @logger.indent -= 1
      unless loaded
        @running = false
        error success, 0
      unless success
        @logger.indent += 1
        @logger\log err
        @logger.indent -= 1
        return finish UpdateStatus.RequirementsUnmet, err

      -- since circular dependencies are possible, our task may have completed in the meantime
      -- so check again if we still need to update
      return finish UpdateStatus.AlreadyUpdated if @updated and @record\checkVersion update.version


    -- download updated scripts to temp directory
    -- check hashes before download, only update changed files

    tmpDir = fileOps.getTempDir!
    res, dir = fileOps.mkdir tmpDir

    return finish UpdateStatus.TempDirFailed, "#{tmpDir} (#{dir})" if res == nil

    @logger\log msgs.performUpdate.updateReady, tmpDir

    scriptSubDir = @record.namespace
    scriptSubDir = scriptSubDir\gsub "%.","/" if @record.scriptType == domain.ScriptType.Module

    @@__downloader.blockPrivateHosts = @updater.config.c.updates.blockPrivateHosts
    @@__downloader\clear!
    for file in *update.files
      file.type or= "script"

      baseName = scriptSubDir .. file.name
      tmpName, prettyName = "#{tmpDir}/#{file.type}/#{baseName}", baseName
      switch file.type
        when "script", "test"
          return finish UpdateStatus.PathTraversal, file.name if file.name\match "%.%."
          file.fullName = UpdateFeed\getFileDeployPath @record.namespace, @record.scriptType, file.name, file.type

          prettyName ..= " (Unit Test)" if file.type == "test"
        else
          file.unknown = true
          @logger\log msgs.performUpdate.unknownType, file.name, file.type
          continue
      continue if file.delete

      unless type(file.sha1)=="string" and #file.sha1 == 40 and tonumber(file.sha1, 16)
        return finish UpdateStatus.BadHash, "#{prettyName} (#{tostring(file.sha1)\lower!})"

      if fileOps.verifyHash file.fullName, file.sha1
        @logger\trace msgs.performUpdate.fileUnchanged, prettyName
        continue

      dl, err = @@__downloader\addDownload file.url, tmpName, file.sha1
      return finish UpdateStatus.DownloadAddFailed, err unless dl
      dl.targetFile = file.fullName
      @logger\trace msgs.performUpdate.fileAddDownload, file.url, prettyName

    @@__downloader\await (_, progress) ->
      @updater\renewLock!
      @logger\progress progress, msgs.performUpdate.filesDownloading, #@@__downloader.downloads
    @logger\progress!

    failedDownloads = [dl for dl in *@@__downloader.downloads when dl.status == Downloader.Download.Status.Failed]
    if #failedDownloads>0
      err = @logger\format ["#{dl.url}: #{dl.error}" for dl in *failedDownloads], 1
      return finish UpdateStatus.DownloadFailed, err


    -- move files to their destination directory and clean up

    @logger\log msgs.performUpdate.movingFiles, @record.automationDir
    moveErrors = {}
    @logger.indent += 1
    for dl in *@@__downloader.downloads
      res, err = fileOps.move dl.outfile, dl.targetFile, true
      -- don't immediately error out if moving of a single file failed
      -- try to move as many files as possible and let the user handle the rest
      if res
        @logger\trace msgs.performUpdate.movedFile, dl.outfile, dl.targetFile
      else
        @logger\log msgs.performUpdate.moveFileFailed, dl.outfile, dl.targetFile, err
        moveErrors[#moveErrors+1] = err
    @logger.indent -= 1

    if #moveErrors>0
      return finish UpdateStatus.MoveFailed, @logger\format moveErrors, 1
    else fileOps.rmdir tmpDir -- recurses by default: the temp dir still holds the per-type subdirectories
    os.remove file.fullName for file in *update.files when file.delete and not file.unknown

    @__recordInstalledSource!

    -- Nuke old module refs and reload
    oldVer, wasVirtual = @record.version, @record.virtual

    -- Update complete, refresh module information/configuration
    if @record.scriptType == domain.ScriptType.Module
      ref = ModuleLoader.loadModule @record, @record, false, true
      unless ref
        if @record._error
          return finish UpdateStatus.ModuleLoadFailed, @logger\format @record._error, 1
        else return finish UpdateStatus.ModuleNotFound

      -- get a fresh version record
      if type(ref.version) == "table" and ref.version.__class.__name == @@__DependencyControl.__name
        @record = ref.version
      elseif registered = @@__DependencyControl\getRegisteredRecord @record.namespace
        -- the module registered a DependencyControl record during load but didn't expose it as `.version`;
        -- adopt its self-declared record rather than demoting a managed module to unmanaged
        @record = registered
      else
        -- look for any compatible non-DepCtrl version records and create an unmanaged record
        return finish UpdateStatus.MissingVersionRecord unless ref.version
        success, rec = pcall @@__DependencyControl, { moduleName: @record.moduleName, version: ref.version,
          recordType: domain.RecordType.Unmanaged, name: @record.name }
        return finish UpdateStatus.RecordCreateFailed, rec unless success
        @record = rec
      @ref = ref

    else with @record
      .name = @record.name
      .virtual = false
      .version = SemanticVersion\toPacked update.version
      @record\writeConfig!

    @updated = true
    @logger\log msgs.performUpdate.updSuccess, domain.terms.capitalize(domain.terms.isInstall[wasVirtual or false]),
      domain.terms.scriptType.singular[@record.scriptType],
      @record.name, SemanticVersion\toString @record.version

    -- Display changelog
    @logger\log update\getChangelog @record, (SemanticVersion\toPacked oldVer) + 1
    @logger\log msgs.performUpdate.reloadNotice

    -- TODO: check handling of private module copies (need extra return value?)
    return finish UpdateStatus.Installed, SemanticVersion\toString @record.version


  ---Reloads this task's record from its config file to pick up an install/update another updater performed
  ---concurrently: on a version bump (or a virtual record becoming real) it reloads the module and marks the
  ---task updated. Called after waiting on the updater lock, so a task doesn't redo work already done.
  refreshRecord: =>
    with @record
      wasVirtual, oldVersion = .virtual, .version
      \loadConfig true
      if wasVirtual and not .virtual or .version > oldVersion
        @updated = true
        @ref = ModuleLoader.loadModule @record, @record, false, true if .scriptType == domain.ScriptType.Module
        if wasVirtual
          @logger\log msgs.refreshRecord.unsetVirtual, domain.terms.scriptType.singular[.scriptType], .name
        else
          @logger\log msgs.refreshRecord.otherUpdate, domain.terms.scriptType.singular[.scriptType], .name,
            SemanticVersion\toString @record.version

UpdateTask.UpdateStatus = UpdateStatus
UpdateTask.ContextCeiling = ContextCeiling
UpdateTask.UpdateReason = UpdateReason
UpdateTask.SourceChoiceStickiness = SourceChoiceStickiness
UpdateTask.SourceFeedKind = SourceFeedKind
UpdateTask.FeedTrustDecision = FeedTrustDecision

-- Reveal the private dialog button labels to the unit tests without putting them on the public API.
return UnitTestSuite\withTestExports UpdateTask, {:msgs}
