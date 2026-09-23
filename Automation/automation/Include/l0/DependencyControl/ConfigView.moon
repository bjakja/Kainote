utils = require "l0.DependencyControl.utils"
local ConfigHandler

---A read/write view over one section key of a view's user config. A read serves the user-set value,
---falling through to the section default when unset. A write lands only in the user section (created on
---first write), so defaults are never materialized into stored config. The view's user config is read
---live, so a held section view survives a refresh or load replacing that table.
---@param view ConfigView The view whose user config backs the section.
---@param sectionKey string The section's top-level key within the view's hive.
---@param defaultSection table The section's defaults, read for keys the user section doesn't set.
---@return table sectionView The merged read/write section view; pairs yields the defaults overlaid with the user-set keys.
mergeSection = (view, sectionKey, defaultSection) ->
  setmetatable {}, {
    __index: (_, k) ->
      userSection = view.userConfig[sectionKey]
      v = userSection and userSection[k]
      if v != nil then v else defaultSection[k]
    __newindex: (_, k, v) ->
      userSection = view.userConfig[sectionKey]
      unless userSection
        userSection = {}
        view.userConfig[sectionKey] = userSection
      userSection[k] = v
    __len: -> 0
    __ipairs: -> error "numerically indexed config hive keys are not supported"
    __pairs: ->
      merged = {}
      merged[k] = v for k, v in pairs defaultSection
      if userSection = view.userConfig[sectionKey]
        merged[k] = v for k, v in pairs userSection
      return next, merged
  }

msgs = {
  new: {
    failedRetrieveHive: "Failed to retrieve hive %s from ConfigHandler: %s"
  }
  isOverlappingView: {
    differentHandler: "Other view on config file '%s' does not belong to the same config handler as this view on config file '%s'."
  }
}

---A view into a hive (nested path) of a ConfigHandler's JSON config file.
---Holds the defaults-fallthrough machinery and exposes @c / @config / @userConfig.
---Multiple views on the same file are coordinated through their shared ConfigHandler.
---@class ConfigView
class ConfigView

  ---Returns a ConfigView for the given file and hive path, creating a handler if needed.
  ---@param filePath string|boolean Config file path, or false for an in-memory (orphan) view.
  ---@param hivePath string|string[] The hive (nested key path) this view targets.
  ---@param defaults? table Default values for the hive.
  ---@param logger? Logger
  ---@param noLoad? boolean Don't load the file immediately (default false).
  ---@param schemaOpts? { schemaId: string, migrate: fun(config: table, current?: string, target: string): boolean } Schema id/migration for the backing handler (see ConfigHandler.get); applied only when the handler is first created for this file.
  ---@return ConfigView? view
  ---@return string? err
  @get = (filePath, hivePath, defaults, logger, noLoad = false, schemaOpts) =>
    ConfigHandler or= require "l0.DependencyControl.ConfigHandler"

    if filePath
      handler, msg = ConfigHandler\get filePath, logger, noLoad, schemaOpts
      return nil, msg unless handler
      return handler\getView hivePath, defaults
    else
      -- in-memory view with no file backing, used for virtual modules
      handler = ConfigHandler nil, logger
      return ConfigView handler, hivePath, defaults


  ---Creates a view into a hive of the given ConfigHandler.
  ---@param configHandler ConfigHandler|nil Backing handler, or nil for an in-memory (orphan) view.
  ---@param hivePath string|string[] The hive (nested key path) this view targets.
  ---@param defaults? table Default values for the hive.
  new: (configHandler, hivePath, defaults) =>
    ConfigHandler or= require "l0.DependencyControl.ConfigHandler"
    @__hivePath = "table" == type(hivePath) and hivePath or {hivePath}
    @__configHandler = configHandler

    -- deprecated, provided for compatibility with DepCtrl < 0.7
    @section = @__hivePath
    -- compatibility alias exposing the config file path on the view
    @file = configHandler and configHandler.filePath

    if configHandler
      success, msg = @refresh!
      configHandler.logger\assert @userConfig, msgs.new.failedRetrieveHive, hivePath, msg
    else
      @userConfig = {} -- orphan view: no file backing

    @defaults = defaults and utils.deepCopy(defaults) or {}
    @config = setmetatable {}, {
      __index: (_, k) ->
        uc = @userConfig[k]
        def = @defaults[k]
        return mergeSection @, k, def if type(def) == "table" and (uc == nil or type(uc) == "table")
        return def if uc == nil
        return uc
      __newindex: (_, k, v) ->
        @userConfig[k] = v
      __len: (tbl) -> return 0
      __ipairs: (tbl) -> error "numerically indexed config hive keys are not supported"
      __pairs: (tbl) ->
        merged = utils.copy @defaults
        merged[k] = v for k, v in pairs @userConfig
        return next, merged
    }
    @c = @config -- shortcut


  ---Removes this view's hive from the config file.
  ---@param waitLockTime? number Seconds to wait for the config lock.
  ---@return boolean? success
  ---@return string? err
  delete: (waitLockTime) =>
    @userConfig, msg = @__configHandler\purgeHive @
    return nil, msg unless @userConfig
    return @save waitLockTime


  ---Copies values from a table or ConfigView into this view's user config.
  ---@param tbl? table|ConfigView Source values.
  ---@param keys? string[] Restrict the copy to these keys.
  ---@param updateOnly? boolean Only overwrite keys already present in this view.
  ---@param skipSameLengthTables? boolean Skip array values whose length matches the existing one.
  ---@return boolean changesMade
  import: (tbl, keys, updateOnly, skipSameLengthTables) =>
    tbl = tbl.userConfig if tbl.__class == @@
    changesMade = false
    keySet = utils.makeSet keys if keys

    for k, v in pairs tbl
      continue if keys and not keySet[k] or @userConfig[k] == v
      continue if updateOnly and @config[k] == nil
      isTable = type(v) == "table"
      if isTable and skipSameLengthTables and type(@userConfig[k]) == "table" and #v == #@userConfig[k]
        continue
      continue if type(k) == "string" and k\sub(1,1) == "_"
      @userConfig[k] = ConfigHandler\getSerializableCopy v
      changesMade = true

    return changesMade


  ---Returns whether this view's hive overlaps with another view on the same handler.
  ---@param otherView ConfigView
  ---@return boolean? overlapping nil when the views belong to different handlers.
  ---@return string? err
  isOverlappingView: (otherView) =>
    if @__configHandler != otherView.__configHandler
      return nil, msgs.isOverlappingView.differentHandler\format otherView.__configHandler.filePath,
        @__configHandler.filePath

    thisViewHivePathDepth, otherViewHivePathDepth = #@__hivePath, #otherView.__hivePath

    return true if thisViewHivePathDepth == 0 or otherViewHivePathDepth == 0

    for i, key in ipairs @__hivePath
      return false if key != otherView.__hivePath[i]
      return true if i == thisViewHivePathDepth or i == otherViewHivePathDepth


  ---Reloads only this view's hive from the config file.
  ---@param waitLockTime? number Seconds to wait for the config lock.
  ---@return boolean? success
  ---@return string? err
  load: (waitLockTime) =>
    return false unless @__configHandler and @__configHandler.filePath
    @__configHandler\load @, waitLockTime


  ---Refreshes this view's userConfig from the handler's in-memory config.
  ---@return boolean? success
  ---@return string? err
  refresh: =>
    @userConfig, msg = @__configHandler\getHive @__hivePath
    return if @userConfig
      true
    else nil, msg


  ---Writes this view's hive to the config file.
  ---@param waitLockTime? number Seconds to wait for the config lock.
  ---@return boolean? success
  ---@return string? err
  save: (waitLockTime) =>
    return false unless @__configHandler and @__configHandler.filePath
    @__configHandler\save @, waitLockTime


  ---@deprecated Use `save`. Retained as an alias for callers written against DepCtrl < 0.7.
  ---@param waitLockTime? number Seconds to wait for the config lock.
  ---@return boolean? success
  ---@return string? err
  write: (waitLockTime) => @save waitLockTime

  ---Attaches this view to a different config file, moving its hive to that file's handler, so the view
  ---holds what that handler holds for this hive path.
  ---@param filePath string Full path to the config file.
  ---@param noLoad? boolean Don't read the file when this call creates its handler (default false). A file already loaded is not read again either way.
  ---@return boolean? success
  ---@return string? err
  setFile: (filePath, noLoad = false) =>
    ConfigHandler or= require "l0.DependencyControl.ConfigHandler"
    oldHandler = @__configHandler
    handler, msg = ConfigHandler\get filePath, (oldHandler and oldHandler.logger), noLoad
    return nil, msg unless handler
    oldHandler.views[@] = nil if oldHandler -- detach from the previous handler
    @__configHandler = handler
    handler.views[@] = true -- register so the handler's whole-file refreshes reach this view
    @file = handler.filePath
    moved, msg = @refresh!
    return nil, msg unless moved
    return true

  ---Detaches this view from its config file, reverting to an in-memory (orphan) state.
  ---@return boolean success
  unsetFile: =>
    ConfigHandler or= require "l0.DependencyControl.ConfigHandler"
    oldHandler = @__configHandler
    oldHandler.views[@] = nil if oldHandler -- detach from the previous handler
    @__configHandler = ConfigHandler nil, oldHandler and oldHandler.logger
    @__configHandler.views[@] = true -- register with the fresh in-memory handler
    @file = nil
    @userConfig = {}
    return true

  ---Returns a ConfigView for another hive of the same config file, sharing this view's handler.
  ---@param hivePath string|string[] The hive path within the config file (from the file root, not relative to this view).
  ---@param defaults? table Default values for the returned view's hive.
  ---@param noLoad? boolean Skip loading the returned view (the caller loads it separately).
  ---@return ConfigView? view
  ---@return string? err
  getSectionHandler: (hivePath, defaults, noLoad) =>
    view, msg = @__configHandler\getView hivePath, defaults
    return nil, msg unless view
    view\load! unless noLoad
    return view

return ConfigView
