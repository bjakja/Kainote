-- The DependencyControl global config: the cross-cutting sectioned defaults and the one-time migration that
-- lifts a flat pre-sectioned config into those sections. A setting read by a single class keeps its default
-- on that class (e.g. Updater.defaultCheckInterval, FileCache.defaultMaxAge). Only settings shared across
-- subsystems default here.

domain = require "l0.DependencyControl.domain"
utils = require "l0.DependencyControl.utils"
SemanticVersion = require "l0.DependencyControl.SemanticVersion"

CONFIG_SCHEMA_ID_V0_7_0 = "https://raw.githubusercontent.com/TypesettingTools/DependencyControl/publish/schemas/config/v0.7.0.json"
CONFIG_SCHEMA_ID_CURRENT = "https://raw.githubusercontent.com/TypesettingTools/DependencyControl/publish/schemas/config/v0.9.0.json"
-- the version a config written before the `$schema` key existed is measured as
PRE_SCHEMA_VERSION = "0.0.0"

-- Per-section defaults. Each section under `sections` is loaded as its own ConfigView with these defaults
-- and handed to the classes of that domain. A section key is kept even when the section has no shared
-- default (e.g. `feeds`), so a view on a section the user hasn't touched still resolves to a table.
sections = {
  updates: {
    blockPrivateHosts: true
  }
  feeds: {}
  logging: {
    defaultLevel: 3
    toFile: true
  }
  paths: {
    config: "?user/config"
    log: "?user/log"
    cache: "?user/cache"
  }
}

-- Old flat config key -> {section, newKey, transform?}, for the one-time lift from the flat pre-sectioned
-- layout into topic sections; a transform maps the old value onto the new key's value domain. Only v0.6.3
-- keys appear here: v0.7.0 was never released, so keys added since need no migration and, if present in a
-- dev config, are simply left in place.
keyMap = {
  updaterEnabled: {"updates", "mode", (enabled) -> enabled and "auto-update" or "off"}
  updateInterval: {"updates", "checkInterval"}
  updateWaitTimeout: {"updates", "waitTimeout"}
  updateOrphanTimeout: {"updates", "orphanTimeout"}
  extraFeeds: {"feeds", "extraFeeds"}
  traceLevel: {"logging", "defaultLevel"}
  writeLogs: {"logging", "toFile"}
  logMaxFiles: {"logging", "maxFiles"}
  logMaxAge: {"logging", "maxAge"}
  logMaxSize: {"logging", "maxSize"}
  configDir: {"paths", "config"}
  logDir: {"paths", "log"}
}

-- v0.6.3 keys removed outright on migration: settings dropped in v0.7.0 with no sectioned replacement.
droppedKeys = {"tryAllFeeds", "dumpFeeds"}

-- DepCtrl pre-0.7 published these packages on an `alpha` channel only, pinning every install to it; v0.7.0
-- makes `stable` their default. `main` is the feed maintenance process's own channel, published as a second
-- default by a CI mishap, so installs ended up tracking it — through the 0.7.0 schema as well, where an
-- `alpha` pin is one the Toolbox offers and so stays.
ownPackages = {"l0.DependencyControl", "l0.DependencyControl.Toolbox", "l0.dkjson", "l0.MoonCats"}

---A channel the migration retires, and the schema step over which it does so.
---@class ConfigChannelRename
---@field fromSchemaVersion string Range the config's own schema version must satisfy.
---@field toSchemaVersion string Range the schema version being migrated to must satisfy.
---@field packages string[] The namespaces whose records are rewritten.
---@field from string[] The channel names to rewrite.
---@field to string The channel name they are rewritten to.

---@type ConfigChannelRename[]
channelsToRename = {
  {
    fromSchemaVersion: "< 0.7.0"
    toSchemaVersion: ">= 0.7.0"
    packages: ownPackages
    from: {"alpha", "main"}
    to: "stable"
  }
  {
    fromSchemaVersion: ">= 0.7.0 < 0.9.0"
    toSchemaVersion: ">= 0.9.0"
    packages: ownPackages
    from: {"main"}
    to: "stable"
  }
}

---Reports whether a list holds the given string.
---@param list string[] The list to search.
---@param value? string The value to look for; nil is never held.
---@return boolean holds Whether the value is in the list.
holds = (list, value) ->
  return false unless value
  for entry in *list
    return true if entry == value
  false

---The schema version a config `$schema` id names, read from the `vX.Y.Z.json` its URL ends in.
---@param schemaId? string A config schema id, or nil for a config written before there were any.
---@return string version The version named, or `0.0.0` where there is no id to read one from.
schemaVersion = (schemaId) ->
  type(schemaId) == "string" and schemaId\match("v(%d+%.%d+%.%d+)%.json$") or PRE_SCHEMA_VERSION

---Reports whether a version satisfies a range, throwing on a range stated wrongly above.
---@param version string The version to test.
---@param range string The npm-style range it must satisfy.
---@return boolean satisfies Whether the version falls in the range.
satisfiesRange = (version, range) ->
  satisfies, err = SemanticVersion\satisfiesRange version, range
  assert satisfies != nil, err
  satisfies

---Rewrites the channel a record has recorded, and the one its configured source names, for every package
---a rename covers that spans the schema versions being migrated between.
---@param config table The whole config-file table, mutated in place.
---@param currentSchemaId? string The `$schema` the config holds, or nil for a pre-`$schema` config.
---@param targetSchemaId string The `$schema` being migrated to.
renameChannels = (config, currentSchemaId, targetSchemaId) ->
  fromVersion = schemaVersion currentSchemaId
  toVersion = schemaVersion targetSchemaId
  for rename in *channelsToRename
    applies = satisfiesRange(fromVersion, rename.fromSchemaVersion) and satisfiesRange toVersion, rename.toSchemaVersion
    continue unless applies
    for section in *domain.ScriptTypeSection.values
      records = config[section]
      continue unless type(records) == "table"
      for namespace, record in pairs records
        continue unless type(record) == "table" and holds rename.packages, namespace
        record.lastChannel = rename.to if holds rename.from, record.lastChannel
        configured = record.configuredSource
        configured.channel = rename.to if type(configured) == "table" and holds rename.from, configured.channel

---Migrates a whole config-file table up to the current schema, in place, when its root `$schema` predates it.
---For a pre-`$schema` config it lifts flat `config`-hive keys into topic sections and rewrites each record's
---pre-0.7 `unmanaged` flag into its recordType and its packed-integer version into a semver string. For a
---v0.7.0 config it seeds each record's `configuredSource` from its `currentSource`, which v0.9.0 splits into
---intent (`configuredSource`, the source the next update should use) and provenance (`currentSource`, the
---source the installed copy came from). Either step also rewrites our own packages' recorded update
---channel where the schema versions it spans retire one.
---A config already on the current schema, and any keys none of these cover, are left untouched.
---Shaped as ConfigHandler's migration callback: the handler stamps the new `$schema` when this returns true.
---@param config table The whole config-file table, mutated in place (the `config` hive and the record sections).
---@param currentSchemaId? string The `$schema` found in the file, or nil for a pre-`$schema` (flat) config.
---@param targetSchemaId string The schema being migrated to.
---@return boolean migrated Whether a migration was applied.
migrate = (config, currentSchemaId, targetSchemaId) ->
  -- ≥0.7.0/<0.9.0 to ≥0.9.0 migration
  if currentSchemaId == CONFIG_SCHEMA_ID_V0_7_0
    for section in *domain.ScriptTypeSection.values
      records = config[section]
      continue unless type(records) == "table"
      for _, record in pairs records
        continue unless type(record) == "table"
        if type(record.currentSource) == "table"
          record.configuredSource or= utils.deepCopy record.currentSource
    renameChannels config, currentSchemaId, targetSchemaId
    return true
  return false if currentSchemaId

  -- ≤0.6.4 to ≥0.7.0 migration
  configHive = config.config
  if type(configHive) == "table"
    for oldKey, dest in pairs keyMap
      value = configHive[oldKey]
      continue if value == nil
      section, newKey, transform = dest[1], dest[2], dest[3]
      value = transform value if transform
      configHive[section] = {} unless type(configHive[section]) == "table"
      configHive[section][newKey] = value
      configHive[oldKey] = nil

    configHive[k] = nil for k in *droppedKeys
    configHive.formatVersion = nil -- obsolete pre-`$schema` marker, superseded by the root `$schema`

  -- pre-0.7 stored a record's type as a boolean `unmanaged` flag and its version as a packed integer.
  -- rewrite the flag as a recordType and the version as a semver string
  for section in *domain.ScriptTypeSection.values
    records = config[section]
    continue unless type(records) == "table"
    for namespace, record in pairs records
      continue unless type(record) == "table"
      record.recordType = domain.RecordType.Unmanaged if record.unmanaged
      record.unmanaged = nil
      record.version = SemanticVersion\toString record.version if type(record.version) == "number"
  renameChannels config, currentSchemaId, targetSchemaId
  return true

return {
  :CONFIG_SCHEMA_ID_CURRENT
  :sections
  migration: {:migrate, :keyMap, :droppedKeys}
}
