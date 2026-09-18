constants = require "l0.DependencyControl.Constants"

-- Resolves provided module aliases (e.g. "json") to their provider module
-- (e.g. "l0.dkjson") through a custom package searcher.
--
-- A module declares the aliases it can satisfy via its record's `provides` field;
-- DependencyControl registers those here, and a single searcher — appended last so
-- stock searchers and any real user-supplied module always win first — lazily loads
-- the provider when an otherwise-unresolved alias is required.
--
-- State lives in a global table so registrations and the installed searcher survive
-- DependencyControl self-update reloads.

DEPCTRL_MODULE_INIT_HOOK_NAME = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}Init"
GLOBAL_KEY = "#{constants.DEPCTRL_PRIVATE_GLOBAL_VAR_PREFIX}ModuleProvider"

state = _G[GLOBAL_KEY]
unless state
  state = { providers: {}, installed: false }
  _G[GLOBAL_KEY] = state

msgs = {
  runInitializer: {
    initializerError: "Module '%s' initializer error: %s"
  }
}

-- debug.traceback truncates chunk names to LUA_IDSIZE (60 chars); using debug.getinfo
-- directly lets us read the full untruncated source path from info.source.
fullTraceback = (msg) ->
  parts = {}
  parts[#parts+1] = msg if msg
  parts[#parts+1] = "stack traceback:"
  pathPrefixes = {}
  if type(aegisub) == "table"
    for alias in *{"?user", "?data", "?temp"}
      ok, resolved = pcall aegisub.decode_path, alias
      pathPrefixes[#pathPrefixes+1] = {resolved, alias} if ok and resolved
  i = 2
  while true
    info = debug.getinfo i, "Sln"
    break unless info
    src = info.source
    if src\sub(1, 1) == "@"
      src = src\sub 2
    elseif src\sub(1, 1) == "="
      -- "=[C]" → "[C]", "=name" → "name"
      src = src\sub 2
    for {prefix, alias} in *pathPrefixes
      if src\sub(1, #prefix) == prefix
        src = alias .. src\sub #prefix + 1
        break
    if src == "[C]"
      -- anonymous C frames carry no useful location; named ones don't need a line number
      parts[#parts+1] = "\t[C] in #{info.name}()" if info.name
    else
      entry = "#{src}:#{info.currentline}"
      entry ..= " in #{info.name}()" if info.name
      parts[#parts+1] = "\t#{entry}"
    i += 1
  table.concat parts, "\n"

-- Returns true when value is a live DependencyControl PackageRecord instance. Uses class name and the
-- presence of checkVersion rather than class identity so the test passes across self-update
-- reloads and other classes accidentally named "DependencyControl".
isDepCtrlVersionRecord = (value) ->
  type(value) == "table" and
    value.__class and value.__class.__name == constants.DEPCTRL_NAME and
    type(value.checkVersion) == "function"

-- Runs a freshly-loaded module's DependencyControl initializer (`__depCtrlInit`) if it has one and
-- hasn't been initialized yet, so the module exposes a proper DependencyControl record. The guard
-- avoids re-initializing modules that mutate their exported state on first init (e.g. BadMutex).
runInitializer = (ref, DependencyControl) ->
  return ref unless type(ref) == "table"
  initializer = ref[DEPCTRL_MODULE_INIT_HOOK_NAME]
  return false unless initializer
  return false if isDepCtrlVersionRecord ref.version

  success, errMsg = xpcall initializer, fullTraceback, DependencyControl
  return true if success
  return nil, msgs.runInitializer.initializerError\format ref.moduleName or "?", errMsg

-- Resolves DependencyControl from package.loaded rather than require()ing it, because an alias can be
-- pulled in during DepCtrl's own bootstrap where a require-back would cycle, and the type check also
-- rejects the mid-bootstrap "loading" sentinel. Until the real class is loaded there's nothing to init
-- against, so the module is returned as-is.
initProvidedModule = (mod) ->
  DependencyControl = package.loaded[constants.DEPCTRL_NAMESPACE]
  return mod unless type(DependencyControl) == "table"

  initialized, errMsg = runInitializer mod, DependencyControl
  error errMsg if initialized == nil
  return mod

-- Returns a loader for a registered alias, or nil for an unregistered name.
-- Kept to a single hash lookup since it runs for every otherwise-unresolved require.
search = (name) ->
  providerName = state.providers[name]
  return unless providerName
  -> initProvidedModule require providerName

---Resolves provided module aliases to their provider module through a custom package searcher.
---@class ModuleProvider
class ModuleProvider
  ---Returns true when value is a live DependencyControl PackageRecord instance, regardless of which class object created it.
  ---@param value any
  ---@return boolean isRecord
  @isDepCtrlVersionRecord = isDepCtrlVersionRecord

  ---Runs a freshly-loaded module reference's DependencyControl initializer (`__depCtrlInit`), if
  ---it has one and hasn't been initialized yet, so the module exposes a proper DependencyControl
  ---record. The guard avoids re-initializing modules that mutate state on first init (e.g. BadMutex).
  ---@param ref any The loaded module reference.
  ---@param DependencyControl table The DependencyControl class handed to the initializer.
  ---@return boolean|nil ran True if the initializer ran, false if there was nothing to run, nil on initializer error.
  ---@return string? err Error message when the initializer raised.
  @runInitializer = runInitializer

  ---Registers a provider for an alias name. First registration wins.
  ---@param alias string The (possibly bare) module name to provide.
  ---@param providerName string The namespaced module that provides it.
  ---@return boolean registered Whether the registration was applied.
  @register = (alias, providerName) =>
    return false unless type(alias) == "string" and type(providerName) == "string"
    return false if state.providers[alias]
    state.providers[alias] = providerName
    return true

  ---Registers every alias declared in a record's `provides` field.
  ---@param record table A record with .moduleName and an optional .provides array.
  @registerRecord = (record) =>
    return unless record.provides and record.moduleName
    for alias in *record.provides
      name = type(alias) == "table" and alias.name or alias
      @register name, record.moduleName if name

  ---Builds a stack traceback string with full, untruncated source paths, rewriting known Aegisub
  ---directory prefixes back to their `?user`/`?data`/`?temp` aliases. Suitable as an xpcall handler.
  ---@param msg? string A message, typically the error, prepended to the traceback.
  ---@return string traceback The formatted traceback, one frame per line.
  @fullTraceback = fullTraceback

  ---Gets the provider namespace registered for an alias module name.
  ---@param alias string
  ---@return string? providerName The provider namespace registered for the alias.
  @getProvider = (alias) => state.providers[alias]

  ---Installs the alias searcher. Idempotent across reloads.
  @install = =>
    return if state.installed
    loaders = package.loaders or package.searchers
    loaders[#loaders + 1] = search
    state.installed = true

return ModuleProvider
