json = require "json"
lfs = require "lfs"

constants = require "l0.DependencyControl.Constants"
domain = require "l0.DependencyControl.domain"
utils = require "l0.DependencyControl.utils"
Logger = require "l0.DependencyControl.Logger"
ConfigView = require "l0.DependencyControl.ConfigView"
fileOps = require "l0.DependencyControl.file-ops"
Updater = require "l0.DependencyControl.Updater"
ModuleLoader = require "l0.DependencyControl.ModuleLoader"
ModuleProvider = require "l0.DependencyControl.ModuleProvider"
SemanticVersion = require "l0.DependencyControl.SemanticVersion"
Accessors = require "l0.DependencyControl.Accessors"
UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
FileCache = require "l0.DependencyControl.FileCache"
configSchema = require "l0.DependencyControl.config-schema"

-- Global registry of live DepCtrl version records keyed by namespace, backed by a global table
--  so it survives DepCtrl self-update reloads. Required to reach the DepCtrl version records
-- of automation scripts/macros, which don't expose it globally (only a few script_* globals)
DEPCTRL_RECORDS_GLOBAL_KEY = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}Records"
recordsByNamespace = _G[DEPCTRL_RECORDS_GLOBAL_KEY]
unless recordsByNamespace
  recordsByNamespace = {}
  _G[DEPCTRL_RECORDS_GLOBAL_KEY] = recordsByNamespace

---Registers a record in the global registry under its namespace. Latest call wins.
---@param record PackageRecord
---@return PackageRecord record The record passed in.
registerRecord = (record) ->
  recordsByNamespace[record.namespace] = record if record.namespace
  return record

---Removes a namespace's record from the registry (e.g. on uninstall).
---@param namespace string
unregisterRecord = (namespace) -> recordsByNamespace[namespace] = nil


---A provided-module alias: the `require` name this module satisfies plus optional metadata. In a
---`provides` list a bare string is shorthand for `{name = <string>}`; records and feeds store the
---normalized table form. `version` is the version of the provided module the provider satisfies
---(reserved — not yet consulted during resolution, which uses the provider's own release version).
---@alias ModuleAlias { name: string, version?: string }

---A required-module dependency. A bare `require` name is shorthand for a version-agnostic
---requirement. The table form below adds a version floor and a source to fetch the module from
---when it is missing.
---@class RequiredModuleSpec
---@field [1]? string The module namespace (alternative to `moduleName`).
---@field moduleName? string The module namespace, as used in `require`.
---@field version? string|number Minimum version; the module must carry a compatible DependencyControl version record.
---@field url? string Where the module can be downloaded, shown to the user in error messages.
---@field feed? string Update feed used to fetch the module when it is missing.
---@field optional? boolean When true a missing module is not an error, but a module that is found is still version-checked.
---@field name? string Friendly name used in error messages.

---Constructor arguments for a [PackageRecord](lua://PackageRecord). All fields are optional; unset fields are
---filled from script_* globals (for automation scripts) or sensible defaults.
---@class PackageRecordArgs
---@field [1]? (string|RequiredModuleSpec)[] Required module specs, passed positionally.
---@field moduleName? string Module namespace; its presence marks this record as a module rather than an automation script.
---@field name? string Display name (defaults to the script/module name).
---@field description? string Description (defaults to script_description).
---@field author? string Author (defaults to script_author).
---@field version? number|string Semantic version (defaults to script_version).
---@field namespace? string Unique namespace (defaults to script_namespace).
---@field url? string Project or homepage URL.
---@field feed? string Update feed URL.
---@field configFile? string Config file name (defaults to "<namespace>.json").
---@field virtual? boolean Mark as a not-yet-installed placeholder record.
---@field recordType? RecordType A domain.RecordType value (default Managed).
---@field requiredModules? (string|RequiredModuleSpec)[] Required module specs (alternative to the positional list).
---@field provides? (string|ModuleAlias)[] Module aliases this module satisfies for `require` (bare strings are normalized to ModuleAlias tables).
---@field readGlobalScriptVars? boolean Read script_* globals for unset fields (default true).
---@field saveRecordToConfig? boolean Persist this record to the config file (default true).

msgs = {
  registerTests: {
    initFailed: "Couldn't initialize the test suite for %s '%s': %s"
  }
  registerMacro: {
    badProcess: "Can't register macro '%s': its process callback must be a function, got a %s."
  }
  new: {
    badRecordError: "Bad #{constants.DEPCTRL_NAME} record (%s)."
    strayRequiredModuleNames: "A required-module entry for %s carries extra names that are ignored: %s. Give each required module its own entry."
    badRecord: {
      noUnmanagedMacros: "Creating unmanaged version records for macros is not allowed"
      missingNamespace: "No namespace defined"
      badVersion: "Couldn't parse version number: %s"
      badModuleTable: "Invalid required module table #%d (%s)."
    }
  }
  uninstall: {
    noVirtualOrUnmanaged: "Can't uninstall %s %s '%s'. (Only installed scripts managed by #{constants.DEPCTRL_NAME} can be uninstalled)."
  }
  writeConfig: {
    error: "An error occurred while writing the #{constants.DEPCTRL_NAME} config file: %s"
    writing: "Writing updated %s data to config file..."
  }
}

---DependencyControl record representing one managed or unmanaged script/module.
---@class PackageRecord
---@field semanticVersion SemanticVersion This record's version as a value object (the canonical store).
---@field version integer This record's version as a packed integer; assignable from a string, packed integer, or SemanticVersion.
class PackageRecord

  @depConf = {
    file: aegisub.decode_path "?user/config/#{constants.DEPCTRL_NAMESPACE}.json",
    -- version is a packed integer at runtime but a semver string in the config, so loadConfig and
    -- writeConfig convert it explicitly
    scriptFields: {"author", "configFile", "feed", "moduleName", "name", "namespace", "url",
      "requiredModules", "recordType", "provides"}
    -- identity fields must survive a reload from a corrupt or incomplete hive, so loadConfig never blanks them
    identityFields: {moduleName: true, namespace: true}
  }

  ---Returns the live, installed record registered for a namespace, or nil if none is registered
  ---or the registered one is still a virtual (not-yet-installed) placeholder.
  ---@param namespace string
  ---@return PackageRecord? record
  @getRegisteredRecord = (namespace) =>
    record = recordsByNamespace[namespace]
    record unless record and record.virtual

  ---Returns all currently registered live records keyed by namespace.
  ---Includes virtual (not-yet-installed) placeholders.
  ---@return table<string, PackageRecord> records
  @getAllRegisteredRecords = => {ns, record for ns, record in pairs recordsByNamespace}

  init = =>
    fileOps.mkdir @depConf.file, true
    @loadGlobalConfig!
    {:logging, :paths} = @config.c
    @logger = Logger { fileBaseName: constants.DEPCTRL_SHORT_NAME, fileSubName: script_namespace, prefix: "[#{constants.DEPCTRL_SHORT_NAME}] ",
      toFile: logging.toFile, defaultLevel: logging.defaultLevel,
      maxAge: logging.maxAge, maxSize: logging.maxSize, maxFiles: logging.maxFiles,
      logDir: paths.log }

    @updater = Updater script_namespace, @config, @logger
    @configDir = paths.config

    fileOps.mkdir aegisub.decode_path @configDir
    @logger\trimFiles!
    fileOps.runScheduledRemoval @configDir


  ---Creates a DependencyControl record from explicit arguments and/or script globals.
  ---@param args PackageRecordArgs
  new: (args) =>
    init PackageRecord unless @@logger

    -- createDummyRef below can expose this record before its real version is parsed, so set a valid one now
    @semanticVersion = SemanticVersion.fromPacked 0

    utils.addDefaults args, {
      readGlobalScriptVars: true
      saveRecordToConfig: true
    }

    {@requiredModules, moduleName:@moduleName, configFile:configFile, virtual:@virtual, :name,
      description:@description, url:@url, feed:@feed, recordType:@recordType, :namespace,
      author:@author, :version, configFile:@configFile, :provides,
      :readGlobalScriptVars, :saveRecordToConfig} = args

    @recordType or= domain.RecordType.Managed
    -- {name, description, process, validate, isActive} of each registered macro, keyed by name
    @registeredMacros = {}
    -- also support name key (as used in configuration) for required modules
    @requiredModules or= args.requiredModules

    if @moduleName
      @namespace = @moduleName
      @name = name or @moduleName
      @scriptType = domain.ScriptType.Module
      ModuleLoader.createDummyRef @ unless @virtual or @recordType == domain.RecordType.Unmanaged

    else
      if @virtual or not readGlobalScriptVars
        @name = name or namespace
        @namespace = namespace
        version or= 0
      else
        @name = name or script_name
        @description or= script_description
        @author or= script_author
        version or= script_version

      @namespace = namespace or script_namespace
      @@logger\assert @recordType == domain.RecordType.Managed, msgs.new.badRecordError, msgs.new.badRecord.noUnmanagedMacros
      @@logger\assert @namespace, msgs.new.badRecordError, msgs.new.badRecord.missingNamespace
      @scriptType = domain.ScriptType.Automation

    -- if the hosting macro doesn't have a namespace defined, define it for
    -- the first DepCtrled module loaded by the macro or its required modules
    unless script_namespace
      export script_namespace = @namespace

    -- non-depctrl records don't need to conform to namespace rules; managed ones defer to
    -- domain.validateNamespace (and its message) rather than restating the rules here
    unless @virtual or @recordType == domain.RecordType.Unmanaged
      valid, nsErr = @validateNamespace!
      @@logger\assert valid, msgs.new.badRecordError, nsErr

    @configFile = configFile or "#{@namespace}.json"
    @automationDir = domain.getAutomationDir @scriptType
    @testDir = domain.getTestDir @scriptType
    packed, err = SemanticVersion\toPacked version
    @@logger\assert packed, msgs.new.badRecordError, msgs.new.badRecord.badVersion\format err
    @semanticVersion = SemanticVersion.fromPacked packed

    @requiredModules or= {}
    -- normalize short format module tables
    for i, mdl in pairs @requiredModules
      switch type mdl
        when "table"
          mdl.moduleName or= mdl[1]
          if #mdl > 1
            strayNames = table.concat [tostring mdl[j] for j = 2, #mdl], ", "
            @@logger\warn msgs.new.strayRequiredModuleNames, @name, strayNames
          mdl[j] = nil for j = 1, #mdl
        when "string"
          @requiredModules[i] = {moduleName: mdl}
        else error msgs.new.badRecordError\format msgs.new.badRecord.badModuleTable\format i, tostring mdl

    -- normalize `provides` aliases (bare string -> {name: …}) and register them so
    -- `require`-ing a provided alias resolves to this module (see ModuleProvider)
    if provides
      @provides = [type(alias) == "table" and alias or {name: alias} for alias in *provides]
      ModuleProvider\registerRecord @

    -- publish this record so tooling can look it up by namespace after requiring the script
    registerRecord @

    -- write config file if contents are missing or are out of sync with the script version record
    -- ramp up the random wait time on first initialization (many scripts may want to write configuration data)
    -- we can't really profit from write concerting here because we don't know which module loads last
    shouldWriteConfig = @loadConfig!
    @writeConfig! if shouldWriteConfig and saveRecordToConfig

  ---Validates optional module availability for the requested feature set.
  ---@param modules string|string[] Feature name(s) whose optional modules to check.
  ---@return boolean available
  ---@return string? err Error message listing missing modules.
  checkOptionalModules: ModuleLoader.checkOptionalModules

  ---Loads global DependencyControl configuration.
  ---@return ConfigView config
  @loadGlobalConfig = =>
    if @config
      @config\load!
    else
      @config = ConfigView\get @depConf.file, {"config"}, configSchema.sections, @logger, false,
        {schemaId: configSchema.CONFIG_SCHEMA_ID_CURRENT, migrate: configSchema.migration.migrate}

  ---Loads this record's script/module configuration hive.
  ---@param importRecord? boolean Overwrite this record's fields from the stored config (default false).
  ---@return boolean shouldWriteConfig
  loadConfig: (importRecord = false) =>
    -- virtual modules are not yet present on the user's system and have no persistent configuration
    @config or= ConfigView\get not @virtual and @@depConf.file,
      { domain.ScriptTypeSection[@scriptType], @namespace }, {}, @@logger, true

    -- import and overwrites version record from the configuration
    if importRecord
      -- check if a module that was previously virtual was installed in the meantime
      -- TODO: prevent issues caused by orphaned config entries
      haveConfig = false
      if @virtual
        @config\setFile @@depConf.file
        -- require a persisted record before treating a still-virtual module as installed
        if @config\load! and @config.c.version != nil
          haveConfig, @virtual = true, false
        else @config\unsetFile!
      else
        haveConfig = @config\load!

      -- only need to refresh data if the record was changed by an update
      if haveConfig
        -- a real install's config hive is authoritative, but in case of config file corruption
        -- we at least need to prevent blanking of the records namespace/module name to avoid crashes
        for key in *@@depConf.scriptFields
          value = @config.c[key]
          @[key] = value unless value == nil and @@depConf.identityFields[key]
        -- version isn't in scriptFields, so read it explicitly
        @version = @config.c.version if @config.c.version != nil

    elseif not @virtual
      --  copy script information to the config
      @config\load!
      shouldWriteConfig = @config\import @, @@depConf.scriptFields, false, true
      -- version isn't a scriptField, so the import above can't see a release that bumped only it
      shouldWriteConfig or= @config.c.version != tostring @semanticVersion
      return shouldWriteConfig

    return false

  ---Writes this record's persisted fields to the shared config file.
  writeConfig: =>
    unless @virtual or @config.file
      @config\setFile @@depConf.file

    @@logger\trace msgs.writeConfig.writing, domain.terms.scriptType.singular[@scriptType]
    @config\import @, @@depConf.scriptFields, false, true
    -- version isn't a scriptField, so store it as a semver string
    @config.c.version = tostring @semanticVersion
    success, errMsg = @config\save!

    assert success, msgs.writeConfig.error\format errMsg


  ---@deprecated Use `SemanticVersion.toPacked`.
  ---Converts a version to its packed-integer form, defaulting to this record's version.
  ---@param value? number|string Version to convert (default: this record's version).
  ---@return number? versionNumber nil on an invalid version string.
  ---@return string? err
  getVersionNumber: (value = @version) => SemanticVersion\toPacked value

  ---@deprecated Use `SemanticVersion.toString`.
  ---Converts a version to its string form, defaulting to this record's version.
  ---@param version? number|string Version to convert (default: this record's version).
  ---@return string? versionString nil on an invalid version.
  ---@return string? err
  getVersionString: (version = @version) => SemanticVersion\toString version


  ---Resolves this record's external config file path. Config files share one directory so
  ---they stay discoverable to other scripts through the DependencyControl config file.
  ---@return string path
  getConfigFileName: () =>
    return aegisub.decode_path "#{@@configDir}/#{@configFile}"

  ---Creates a ConfigView for this record's script-specific config file.
  ---@param defaults? table Default values for the config.
  ---@param section? string|string[] Config section path.
  ---@param noLoad? boolean Skip loading the file immediately.
  ---@return ConfigView
  getConfigHandler: (defaults, section, noLoad) =>
    return ConfigView\get @getConfigFileName!, section, defaults, nil, noLoad

  ---Creates a logger preconfigured for this record.
  ---@param args? table Logger options; missing fields are filled from this record's config.
  ---@return Logger
  getLogger: (args = {}) =>
    args.fileBaseName or= @namespace
    args.toFile = @config.c.logToFile if args.toFile == nil
    args.defaultLevel or= @config.c.logLevel
    args.prefix or= @moduleName and "[#{@name}]"

    return Logger args

  ---Returns a shared, persistent on-disk cache for this script, under the user's configured cache location.
  ---It lives at `<the configured cache dir>/<this script's namespace>/<name>`, so each script gets its own
  ---namespaced caches and honors the DependencyControl config. Repeated calls for the same name share one instance.
  ---@param name string A short name for the cache's purpose (e.g. "thumbnails").
  ---@param opts? FileCacheOptions Default cache options; applied only when the cache is first created.
  ---@return FileCache
  getFileCache: (name, opts) =>
    FileCache.get @@config.c.paths.cache, @namespace, name, opts

  ---Checks whether this record's version satisfies a minimum version.
  ---@param value number|string|PackageRecord Version, or record, to compare against.
  ---@param precision? SemverPrecision Precision to compare at (default "patch").
  ---@return boolean? satisfied
  ---@return number|string|nil maskedOrError Masked comparison value on success, or an error message.
  checkVersion: (value, precision = "patch") =>
    if type(value) == "table" and value.__class == @@
      value = value.version
    return SemanticVersion\check @version, value, precision


  ---Retrieves managed submodules registered under this module namespace.
  ---@return string[]? submodules Submodule namespaces, or nil for non-module records.
  ---@return ConfigView? config The module config section handler.
  getSubmodules: =>
    return nil if @virtual or @recordType == domain.RecordType.Unmanaged or @scriptType != domain.ScriptType.Module
    mdlConfig = @@config\getSectionHandler domain.ScriptTypeSection[domain.ScriptType.Module]
    pattern = "^#{utils.escapePattern @namespace}%."
    return [mdl for mdl, _ in pairs mdlConfig.c when mdl\match pattern], mdlConfig

  ---Loads or updates required modules and returns their references.
  ---@param modules? (string|RequiredModuleSpec)[] Module specs to load (default: this record's requiredModules).
  ---@param addFeeds? string[] Extra feed URLs to search (default: this record's feed).
  ---@return any ... The loaded module references, in order; an absent optional module comes back as nil.
  requireModules: (modules = @requiredModules, addFeeds = {@feed}) =>
    success, err = ModuleLoader.loadModules @, modules, addFeeds
    @@updater\releaseLock!
    unless success
      -- if we failed loading our required modules
      -- then that means we also failed to load
      LOADED_MODULES[@namespace] = nil
      @@logger\error err
    return unpack [mdl._ref for mdl in *modules]

  ---Registers DepUnit tests for this record if test modules are available.
  ---@param ... any Extra arguments forwarded to the suite's import function (see UnitTestSuite for its full signature).
  registerTests: (...) =>
    return if @haveTestSuite == false or @testSuiteInitialized

    testSuiteIdentifier = UnitTestSuite\getTestSuiteRequireIdentifier @scriptType, @namespace
    @haveTestSuite, testsOrErrorMsg = xpcall UnitTestSuite\require, ModuleProvider.fullTraceback, testSuiteIdentifier
    if not @haveTestSuite
      @testSuiteLoadError = testsOrErrorMsg unless testsOrErrorMsg\match "module '[^']+' not found"
      return

    @tests = testsOrErrorMsg
    @tests.name = @name

    modules = table.pack @requireModules!
    success, errMsg = nil, nil

    -- The test import receives the subject under test first:
    -- modules hand over their own ref
    -- automation scripts hand over their currently registered macros
    if @moduleName
      success, errMsg = pcall @tests\import, @ref, modules, ...
    else
      success, errMsg = pcall @tests\import, @registeredMacros, modules, ...

    if success
      @testSuiteInitialized = true
    else
      @testSuiteInitializeError = errMsg
      @@logger\warn msgs.registerTests.initFailed, domain.terms.scriptType.singular[@scriptType], @name, errMsg

    -- Automation scripts run in their own isolated environment exactly once, so they register
    -- their own test menu right here. Modules, by contrast, load in every script's environment;
    -- registering from here would create duplicate menu entries, so their test menus are
    -- registered centrally by the Toolbox (which loads each module exactly once).
    @tests\registerMacros! if @testSuiteInitialized and @scriptType == domain.ScriptType.Automation

  ---Finalizes module registration and swaps dummy module refs for real refs. Call it in place of
  ---returning the module. Modules registered this way may depend on each other circularly, provided
  ---they don't use each other during construction.
  ---@param selfRef table The module's real exported table.
  ---@param ... any Forwarded to registerTests().
  ---@return table selfRef
  register: (selfRef, ...) =>
    -- forward  dummy refs to the real module for dependencies that already captured it.
    -- A module that defers creating its DepCtrl record to the __depCtrlInit hook doesn't
    -- get a dummy ref because it already finished loading by the time the hook is called.
    @ref.__index = selfRef if @ref
    @ref, LOADED_MODULES[@moduleName] = selfRef, selfRef
    @registerTests selfRef, ...
    return selfRef

  ---Registers a single Aegisub macro with DependencyControl update hooks.
  ---When the first argument is a function, name and description are taken from the script and the
  ---remaining arguments shift left. A `customMenu` property in the script's config overrides the
  ---macro's menu location; it is a user-owned setting, so scripts must not change it without consent.
  ---@param name? string|function Macro name, or the process function in the short signature.
  ---@param description? string|function Macro description, or the validate function in the short signature.
  ---@param process function Macro processing callback.
  ---@param validate? function Aegisub validation callback.
  ---@param isActive? function Aegisub is-active callback.
  ---@param submenu? string|boolean Submenu name, or true to use the script name.
  registerMacro: (name=@name, description=@description, process, validate, isActive, submenu) =>
    @registerTests!
    -- alternative signature takes name and description from script
    if type(name)=="function"
      process, validate, isActive, submenu = name, description, process, validate
      name, description = @name, @description

    @@logger\assert type(process) == "function", msgs.registerMacro.badProcess, name, type process

    -- use automation script name for submenu by default
    submenu = @name if submenu == true

    menuName = { @config.c.customMenu }
    menuName[#menuName+1] = submenu if submenu
    menuName[#menuName+1] = name

    -- check for updates before running a macro
    processHooked = (sub, sel, act) ->
      @@updater\scheduleUpdate @
      @@updater\releaseLock!
      return process sub, sel, act

    aegisub.register_macro table.concat(menuName, "/"), description, processHooked, validate, isActive

    -- record the unhooked process so this script's test suite can drive the macro without triggering an update check
    @registeredMacros[name] = {:name, :description, :process, :validate, :isActive}

  ---Registers multiple macros declared in table form.
  ---@param macros? table[] Macro definitions, each an argument list for registerMacro.
  ---@param submenuDefault? string|boolean Default submenu value applied when a macro omits it (default true).
  ---@param testExports? table Internals to expose to this record's DepUnit test suite, forwarded to its test import function.
  registerMacros: (macros = {}, submenuDefault = true, testExports) =>
    @registerTests testExports
    for macro in *macros
      -- allow macro table to omit name and description
      submenuIdx = type(macro[1])=="function" and 4 or 6
      macro[submenuIdx] = submenuDefault if macro[submenuIdx] == nil
      @registerMacro unpack(macro, 1, 6)

  version: Accessors.property
    get: => @semanticVersion\toPacked!
    set: (value) =>
      packed, err = SemanticVersion\toPacked value
      error err, 0 unless packed
      @semanticVersion = SemanticVersion.fromPacked packed

  ---Parses and sets this record's semantic version without raising on invalid input.
  ---@param version number|string
  ---@return number? version The parsed integer version, or nil on error.
  ---@return string? err
  setVersion: (version) =>
    packed, err = SemanticVersion\toPacked version
    return nil, err unless packed
    @semanticVersion = SemanticVersion.fromPacked packed
    return packed

  ---Validates this record's namespace, always passing for virtual records.
  ---@return boolean valid
  ---@return string? err
  validateNamespace: =>
    return true if @virtual
    return domain.validateNamespace @namespace

  ---Returns all candidate entry point paths for this record under a given base directory,
  ---covering .moon and .lua extensions and init.* variants for modules.
  ---@param baseDir string Absolute automation base directory.
  ---@return string[] paths
  getPossibleEntryPointPaths: (baseDir) =>
    isModule = @scriptType == domain.ScriptType.Module
    subPath = isModule and @namespace\gsub("%.", "/") or @namespace
    paths = {}
    for ext in *{".moon", ".lua"}
      if path = fileOps.validateFullPath "#{subPath}#{ext}", false, baseDir
        paths[#paths+1] = path
      if isModule
        if path = fileOps.validateFullPath "#{subPath}/init#{ext}", false, baseDir
          paths[#paths+1] = path
    return paths

  ---Finds this record's primary entry point file, checking ?user then ?data automation directories.
  ---@return string? path
  ---@return boolean? isUserPath True when found under ?user, false when found under ?data, nil when not found.
  getEntryPointPath: =>
    userDir = domain.getAutomationDir @scriptType, "?user"
    for path in *@getPossibleEntryPointPaths userDir
      info = fileOps.getAttributes path, "mode"
      return path, true if info and info.attr == "file"

    dataDir = domain.getAutomationDir @scriptType, "?data"
    if dataDir and dataDir != userDir
      for path in *@getPossibleEntryPointPaths dataDir
        info = fileOps.getAttributes path, "mode"
        return path, false if info and info.attr == "file"

    -- TODO: what if a module is available in another package search path?
    return nil, nil

  ---Uninstalls this managed record and removes matching files from automation paths.
  ---@param removeConfig? boolean Also delete the record's config (default true).
  ---@return boolean? success nil when the record can't be uninstalled (virtual/unmanaged).
  ---@return table|string|nil result Per-file removal results, or an error message.
  uninstall: (removeConfig = true) =>
    if @virtual or @recordType == domain.RecordType.Unmanaged
      return nil, msgs.uninstall.noVirtualOrUnmanaged\format @virtual and "virtual" or "unmanaged",
        domain.terms.scriptType.singular[@scriptType],
        @name
    @config\delete!
    subModules, mdlConfig = @getSubmodules!
    -- uninstalling a module also removes all submodules
    if subModules and #subModules > 0
      mdlConfig.c[mdl] = nil for mdl in *subModules
      mdlConfig\write!

    toRemove, pattern, dir = {}
    if @moduleName
      nsp, name = @namespace\match "(.+)%.(.+)"
      pattern = "^#{utils.escapePattern name}"
      dir = "#{@automationDir}/#{nsp\gsub '%.', '/'}"
    else
      pattern = "^#{utils.escapePattern @namespace}"
      dir = @automationDir

    lfs.chdir dir
    for file in lfs.dir dir
      info = fileOps.getAttributes file, "mode"
      mode, path = info and info.attr, info and info.path
      -- a file must be "<stem>.<ext>" and a module directory exactly "<stem>", so a
      -- sibling package sharing the name prefix never falls into the recursive delete
      currPattern = mode == "file" and pattern .. "%." or pattern .. "$"
      -- automation scripts don't use any subdirectories
      if (@moduleName or mode == "file") and file\match currPattern
        toRemove[#toRemove+1] = path

    -- drop the record from the registry so tooling no longer sees the removed script
    unregisterRecord @namespace
    return fileOps.remove toRemove, true, true

-- wire the computed `version` accessor (returns PackageRecord, so the module still yields the class)
Accessors.install PackageRecord
return PackageRecord
