-- Note: this is a private API intended to be exclusively for internal DependencyControl use
-- Everything in this class can and will change without any prior notice
-- and calling any method is guaranteed to interfere with DependencyControl operation

constants = require "l0.DependencyControl.Constants"
SemanticVersion = require "l0.DependencyControl.SemanticVersion"
ModuleProvider = require "l0.DependencyControl.ModuleProvider"
domain = require "l0.DependencyControl.domain"
utils = require "l0.DependencyControl.utils"
-- required lazily because a load-time require of UpdateTask would be circular
local UpdateTask

DEPCTRL_DUMMY_MODULE_MARKER = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}Dummy"

msgs = {
  checkOptionalModules: {
    downloadHint: "Please download the modules in question manually, put them in your %s folder and reload your automation scripts."
    missing: "A %s feature you're trying to use requires additional modules that were not found on your system:\n%s\n%s"
  }
  formatVersionErrorTemplate: {
    missing: "— %s %s%s\n—— Reason: %s"
    outdated: "— %s (Installed: v%s; Required: v%s)%s\n—— Reason: %s"
  }
  loadModules: {
    missing: "One or more of the modules required by %s could not be found on your system:\n%s\n%s"
    missingRecord: "Module '%s' is missing a version record."
    moduleError: "Error in required module %s:\n%s"
    outdated: [[One or more of the modules required by %s are outdated on your system:
%s\nPlease update the modules in question manually and reload your automation scripts.]]
  }
}

---Internal module loading helpers for DependencyControl-managed module dependencies.
---@class ModuleLoader
class ModuleLoader

  ---Formats a single module's version-error line for a load-error summary.
  ---@param name string The module's display name.
  ---@param reqVersion? string|integer The required version, shown in the message when present.
  ---@param url? string The module's feed/download URL, appended when present.
  ---@param reason? string The failure reason shown for the module.
  ---@param ref? table The installed module's version record; when given, formats the outdated template instead of the missing one.
  ---@return string errorLine The formatted one-line error entry.
  @formatVersionErrorTemplate = (name, reqVersion, url, reason, ref) =>
    url = url and ": #{url}" or ""
    if ref
      -- unmanaged records have refs whose .version is a string instead of a DepCtrl record
      version = SemanticVersion\toString type(ref.version) == "table" and ref.version.version or ref.version
      return msgs.formatVersionErrorTemplate.outdated\format name, version, reqVersion, url, reason
    else
      reqVersion = reqVersion and " (v#{reqVersion})" or ""
      return msgs.formatVersionErrorTemplate.missing\format name, reqVersion, url, reason

  ---Registers a placeholder entry for this module in the global module registry so a circular
  ---dependency can resolve this module while it is still loading. No-op for non-module scripts.
  ---@return boolean|nil registered True if a dummy was registered, false if one already existed, nil if this isn't a module.
  @createDummyRef = =>
    return nil if @scriptType != domain.ScriptType.Module
    -- global module registry allows for circular dependencies:
    -- set a dummy reference to this module since this module is not ready
    -- when the other one tries to load it (and vice versa)
    export LOADED_MODULES = {} unless LOADED_MODULES
    unless LOADED_MODULES[@namespace]
      @ref = {}
      LOADED_MODULES[@namespace] = setmetatable {[DEPCTRL_DUMMY_MODULE_MARKER]: true, version: @}, @ref
      return true
    return false

  ---Removes this module's placeholder registry entry if it is still a dummy (not yet replaced by the
  ---real module). No-op for non-module scripts.
  ---@return boolean|nil removed True if a dummy entry was removed, false if none was present, nil if this isn't a module.
  @removeDummyRef = =>
    return nil if @scriptType != domain.ScriptType.Module
    if LOADED_MODULES[@namespace] and LOADED_MODULES[@namespace][DEPCTRL_DUMMY_MODULE_MARKER]
      LOADED_MODULES[@namespace] = nil
      return true
    return false

  ---Loads a single required module, storing the result in `mdl._ref` and running its
  ---DependencyControl initializer. On failure, sets `mdl._missing` when the module wasn't found or
  ---`mdl._error` with the load error.
  ---@param mdl table The module descriptor to load; mutated in place with the result and error flags.
  ---@param usePrivate? boolean Load this script's private copy (namespaced under its own name) instead of the shared module.
  ---@param reload? boolean Discard cached references to the module and its submodules and load afresh.
  ---@return table? ref The loaded module reference, or nil on failure.
  @loadModule = (mdl, usePrivate, reload) =>
    with mdl
      ._missing, ._error = nil

      moduleName = usePrivate and "#{@namespace}.#{mdl.moduleName}" or .moduleName
      name = "#{mdl.name or mdl.moduleName}#{usePrivate and ' (Private Copy)' or ''}"

      if .outdated or reload
        -- Submodules cached under the module's namespace belong to the version being replaced,
        -- so the fresh load must not pick them up from the require cache.
        submodulePrefix = "#{moduleName}."
        for cachedName in pairs package.loaded
          if type(cachedName) == "string" and cachedName\sub(1, #submodulePrefix) == submodulePrefix
            package.loaded[cachedName] = nil
        package.loaded[moduleName], LOADED_MODULES[moduleName] = nil

      elseif ._ref = LOADED_MODULES[moduleName]
        -- module is already loaded, however it may or may not have been loaded by DepCtrl
        -- so we have to call any DepCtrl initializer if it hasn't been called yet
        ModuleProvider.runInitializer ._ref, @@
        return ._ref

      loaded, res = xpcall require, ModuleProvider.fullTraceback, moduleName
      unless loaded
        LOADED_MODULES[moduleName] = nil
        res or= "unknown error"
        ._missing = nil != res\find "module '#{moduleName}' not found:", nil, true
        ._error = res unless ._missing
        return nil

      -- set new references
      if reload and ._ref and ._ref[DEPCTRL_DUMMY_MODULE_MARKER]
        setmetatable ._ref, res
      ._ref, LOADED_MODULES[moduleName] = res, res

      -- run DepCtrl initializer if one was specified
      ModuleProvider.runInitializer res, @@

    return mdl._ref -- having this in the with block breaks moonscript

  ---Loads required modules, updates missing/outdated ones, and validates version constraints.
  ---@param modules table[]
  ---@param addFeeds? string[] Extra feed URLs to search when fetching missing modules (default: this script's feed).
  ---@param skip? table<string, boolean> Module names to skip, keyed by name (default: this module itself).
  ---@return boolean success
  ---@return string err Combined error message (empty on success).
  @loadModules = (modules, addFeeds = {@feed}, skip = @moduleName and {[@moduleName]: true} or {}) =>
    UpdateTask or= require "l0.DependencyControl.UpdateTask"
    for mdl in *modules
      continue if skip[mdl.moduleName]
      with mdl
        ._ref, ._updated, ._missing, ._outdated, ._reason, ._error = nil

        -- try to load private copies of required modules first
        ModuleLoader.loadModule @, mdl, true
        ModuleLoader.loadModule @, mdl unless ._ref

        -- try to fetch and load a missing module from the web
        if ._missing
          record = @@{moduleName:.moduleName, name:.name or .moduleName,
            version:-1, url:.url, feed:.feed, virtual:true}
          ._ref, code, extErr = @@updater\require record, .version, addFeeds, .optional
          if ._ref
            ._updated, ._missing = true, false
          else
            unless code == UpdateTask.UpdateStatus.SkippedOptional
              ._reason = UpdateTask.getUpdaterErrorMsg code, .name or .moduleName, domain.ScriptType.Module, true, extErr
            -- nuke dummy reference for circular dependencies
            LOADED_MODULES[.moduleName] = nil

        -- check if the version requirements are satisfied
        -- which is guaranteed for modules updated with \require, so we don't need to check again
        if .version and ._ref and not ._updated
          record = ._ref.version
          unless record
            ._error = msgs.loadModules.missingRecord\format .moduleName
            continue

          if not ModuleProvider.isDepCtrlVersionRecord record
            record = @@ moduleName: .moduleName, version: record, recordType: domain.RecordType.Unmanaged

          -- force an update for outdated modules
          if not record\checkVersion .version
            ref, code, extErr = @@updater\require record, .version, addFeeds
            if ref
              ._ref = ref
            elseif code != UpdateTask.UpdateStatus.UpdateInProgress and not .optional
              ._outdated = true
              ._reason = UpdateTask.getUpdaterErrorMsg code, .name or .moduleName, domain.ScriptType.Module, false, extErr

    missing, outdated, moduleError = {}, {}, {}
    for mdl in *modules
      with mdl
        name = .name or .moduleName
        if ._missing and not .optional
          missing[#missing+1] = ModuleLoader.formatVersionErrorTemplate @, name, .version, .url, ._reason
        elseif ._outdated
          outdated[#outdated+1] = ModuleLoader.formatVersionErrorTemplate @, name, .version, .url, ._reason, ._ref
        elseif ._error
          moduleError[#moduleError+1] = msgs.loadModules.moduleError\format name, ._error

    errorMsg = {}
    if #moduleError > 0
      errorMsg[1] = table.concat moduleError, "\n"
    if #outdated > 0
      errorMsg[#errorMsg+1] = msgs.loadModules.outdated\format @name, table.concat outdated, "\n"
    if #missing > 0
      downloadHint = msgs.checkOptionalModules.downloadHint\format domain.getAutomationDir domain.ScriptType.Module
      errorMsg[#errorMsg+1] = msgs.loadModules.missing\format @name, table.concat(missing, "\n"), downloadHint

    return #errorMsg == 0, table.concat(errorMsg, "\n\n")

  ---Validates optional module availability for the requested feature set.
  ---@param modules string|string[] Feature name(s) whose optional modules to check.
  ---@return boolean available
  ---@return string? err Error message listing missing modules.
  @checkOptionalModules = (modules) =>
    modules = type(modules)=="string" and {[modules]:true} or utils.makeSet modules
    missing = [ModuleLoader.formatVersionErrorTemplate @, mdl.moduleName, mdl.version, mdl.url,
      mdl._reason for mdl in *@requiredModules when mdl.optional and mdl._missing and modules[mdl.name]]

    if #missing>0
      downloadHint = msgs.checkOptionalModules.downloadHint\format domain.getAutomationDir domain.ScriptType.Module
      errorMsg = msgs.checkOptionalModules.missing\format @name, table.concat(missing, "\n"), downloadHint
      return false, errorMsg
    return true

return ModuleLoader
