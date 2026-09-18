Logger = require "l0.DependencyControl.Logger"
constants = require "l0.DependencyControl.Constants"
domain = require "l0.DependencyControl.domain"
environment = require "l0.DependencyControl.environment"
utils = require "l0.DependencyControl.utils"
SemanticVersion = require "l0.DependencyControl.SemanticVersion"
ReleaseNotes = require "l0.DependencyControl.release-notes"

defaultLogger = Logger fileBaseName: "#{constants.DEPCTRL_SHORT_NAME}.ScriptUpdateRecord"

---@class FeedFileData
---@field name string Filename relative to the base URL.
---@field url? string Absolute download URL after template variable expansion.
---@field platform? string Target platform filter (e.g. "Windows-x64"); absent means all platforms.

---@class FeedChannelData
---@field version string Semantic version string of this release.
---@field files? FeedFileData[] Files provided by this release.
---@field platforms? string[] Platforms supported by this channel; absent means all platforms.
---@field default? boolean Whether this is the default channel.
---@field released? string ISO 8601 release date string (e.g. "2024-01-31" or "2024-01-31T23:59:00Z")
---@field fileBaseUrl? string Base URL prepended to file names during template expansion.

---@class FeedScriptData
---@field name string Display name of the script.
---@field channels table<string, FeedChannelData> Available update channels keyed by channel name.
---@field changelog? table<string, string|string[]> Version-keyed changelog entries; values are a single string or a list of strings.
---@field author? string Script author.
---@field url? string Project or homepage URL.
---@field feed? string URL of the script's primary update feed.

---@class FeedData
---@field name? string Display name of the feed.
---@field baseUrl? string Base URL used for template variable expansion across all entries.
---@field knownFeeds? table<string, string> Named registry of other feed URLs for cross-feed references.
---@field macros table<string, FeedScriptData> Automation scripts indexed by namespace.
---@field modules table<string, FeedScriptData> Modules indexed by namespace.

msgs = {
  errors: {
    noActiveChannel: "No active channel."
  }
  getChannels: {
    ambiguousDefault: "The feed marks several channels of '%s' as the default (%s); using '%s'."
  }
  setChannel: {
    channelGone: "The feed no longer offers channel '%s' for '%s'; switching to '%s'."
  }
  changelog: {
    header: "Changelog for %s v%s (released %s):"
    verTemplate: "v %s:"
  }
}

---Feed-specific update information for a single script in a selected channel.
---
---Fields of the underlying [FeedScriptData](lua://FeedScriptData) (name, changelog, etc.)
---are readable directly on the instance, and the active channel's
---[FeedChannelData](lua://FeedChannelData) fields (version, files, platforms, etc.) are
---exposed directly on the instance once a channel is selected.
---@class ScriptUpdateRecord
---@field namespace string Script namespace.
---@field data FeedScriptData Shallow copy of the raw script entry from the feed.
---@field config {c: {configuredSource?: table, currentSource?: table, lastChannel?: string, channels?: string[]}} Per-package config; read for the configured channel and provenance metadata.
---@field moduleName string|false Namespace string for modules; false for automation scripts.
---@field logger Logger
---@field activeChannel? string Name of the currently active update channel.
---@field version? string Release version of the active channel (set by setChannel).
---@field files FeedFileData[] Platform-filtered file list for the active channel (set by setChannel).
---@field platforms? string[] Platforms supported by the active channel (set by setChannel).
class ScriptUpdateRecord

  -- Shared per-class metatable for the @data __index fallback; initialized lazily on first instantiation.
  instanceMetaTable = nil

  ---Creates an update record for a single script entry in a feed.
  ---@param namespace string
  ---@param data FeedScriptData
  ---@param config? {c: {configuredSource?: table, currentSource?: table, lastChannel?: string}}
  ---@param scriptType ScriptType
  ---@param autoChannel? boolean Select the default channel on construction (default true).
  ---@param logger? Logger
  new: (@namespace, data, @config = {c:{}}, scriptType, autoChannel = true, @logger = defaultLogger) =>
    @data = {k, v for k, v in pairs data}
    @moduleName = scriptType == domain.ScriptType.Module and @namespace

    unless instanceMetaTable
      meta = getmetatable @
      instanceMetaTable = {__index: (t, k) ->
        v = meta[k]
        return v if v != nil
        d = rawget t, "data"
        return d and d[k]
      }
    setmetatable @, instanceMetaTable

    @setChannel! if autoChannel


  ---Picks the channel a package's feed entry flags as its default.
  ---The feed format allows only one default channel, but in case there are multiple defaults, this
  ---picks one deterministically and notifies callers of the conflict, so they can warn the user or
  ---otherwise handle it as they see fit.
  ---@param channels? table<string, FeedChannelData> A package's `channels` map.
  ---@return string? name The default channel's name, or nil when none is flagged.
  ---@return string[]? conflicting Every flagged name, sorted, when more than one is flagged; nil otherwise.
  @getDefaultChannel = (channels) ->
    return nil unless type(channels) == "table"
    names = [name for name, channel in pairs channels when channel.default]
    return nil if #names == 0
    table.sort names
    return names[1], #names > 1 and names or nil

  ---The source a package's config records for its next update: the source configured for it, or the one the
  ---installed copy came from where no choice has been made. Configs before v0.9.0 hold only the latter.
  ---@param packageConfig table An installed package's config entry.
  ---@return table? source The recorded source, or nil when the package records none.
  @getRecordedSource = (packageConfig) -> packageConfig.configuredSource or packageConfig.currentSource

  ---The update channel a package's config records, from its recorded source or, for a config written before
  ---v0.7.0, the `lastChannel` key.
  ---@param packageConfig table An installed package's config entry.
  ---@return string? channel The recorded channel, or nil when the package records none.
  @getRecordedChannel = (packageConfig) ->
    source = @.getRecordedSource packageConfig
    source and source.channel or packageConfig.lastChannel

  ---Returns all available channel names for this script and the default channel.
  ---@return string[] channels Channel names, empty when the package declares none.
  ---@return string? defaultChannel
  getChannels: =>
    channels = {}
    return channels unless type(@data.channels) == "table"
    channels[#channels+1] = name for name in pairs @data.channels

    default, conflicting = @@.getDefaultChannel @data.channels
    if conflicting
      @logger\warn msgs.getChannels.ambiguousDefault, @namespace, table.concat(conflicting, ", "), default
    return channels, default

  ---Selects the active update channel and exposes its fields on this instance.
  ---An explicitly requested channel wins over the channel recorded in the configured package source,
  ---which wins over the feed's default.
  ---This only affects the in-memory state of this ScriptUpdateRecord instance; persistence to the DepCtrl
  ---config file is left to the caller.
  ---@param channelName? string Channel to activate; must be offered by the feed.
  ---@return boolean success False when the selected channel isn't offered by the feed.
  ---@return string? activeChannel The selected channel name; nil when the package declares no channels at all.
  setChannel: (channelName) =>
    _, default = @getChannels!
    -- When no source is configured (<0.9.0), the source used for the last install (≥0.7.0) is used, and,
    -- failing that, the `lastChannel` which has been recorded to the config since the pre-v0.7.0 days.
    source = @@.getRecordedSource @config.c
    recorded = @@.getRecordedChannel @config.c
    selected = channelName or recorded or default
    -- When the currently configured/last used channel is no longer offered by the feed, we fall back
    -- to the feed's default channel, unless the package source has been explicitly pinned, in which case
    -- we leave it to the user to rectify.
    pinned = source and source.stickiness == domain.SourceChoiceStickiness.Pinned
    if not pinned and default and not channelName and selected and not @data.channels[selected]
      @logger\warn msgs.setChannel.channelGone, selected, @namespace, default
      selected = default
    @activeChannel = selected
    channelData = selected and @data.channels[selected]
    return false, @activeChannel unless channelData
    @[k] = v for k, v in pairs channelData

    @files = @files and [file for file in *@files when not file.platform or file.platform == environment.platform] or {}
    return true, @activeChannel

  ---Checks whether this script's active channel supports the current platform.
  ---@return boolean supported
  ---@return string platform
  checkPlatform: =>
    @logger\assert @activeChannel, msgs.errors.noActiveChannel
    return not @platforms or (utils.makeSet @platforms)[environment.platform], environment.platform

  ---Formats changelog entries from the current version down to a minimum version, grouping each
  ---version's entries into marker categories (Bug Fixes, New Features, …) with a glyph heading.
  ---@param versionRecord any Unused; present for API compatibility.
  ---@param minVer? number|string Oldest version to include (default 0, i.e. all).
  ---@return string changelog Formatted multi-line string, or "" if nothing to show. A version whose entries carry no markers lists them flat, without category headings.
  getChangelog: (versionRecord, minVer = 0) =>
    return "" unless "table" == type @changelog
    maxVer = SemanticVersion\toPacked @version
    minVer = SemanticVersion\toPacked minVer

    changelog = {}
    for ver, entry in pairs @changelog
      verNum = SemanticVersion\toPacked ver
      -- skip a malformed changelog version key. feed changelog keys aren't schema-validated, so toPacked
      -- returns false for them, and toString(false) or comparing false >= minVer would otherwise raise
      continue unless verNum
      if verNum >= minVer and verNum <= maxVer
        changelog[#changelog+1] = {verNum, SemanticVersion\toString(verNum), entry}

    return "" if #changelog == 0
    table.sort changelog, (a,b) -> a[1]>b[1]

    msg = {msgs.changelog.header\format @name, SemanticVersion\toString(@version), @released or "<no date>"}
    for chg in *changelog
      chg[3] = {chg[3]} if type(chg[3]) ~= "table"
      continue if #chg[3] == 0
      msg[#msg+1] = @logger\format msgs.changelog.verTemplate, 1, chg[2]
      block = ReleaseNotes.renderLog chg[3]
      msg[#msg+1] = block unless block == ""

    return table.concat msg, "\n"

return ScriptUpdateRecord
