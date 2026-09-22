Enum = require "l0.DependencyControl.Enum"

msgs = {
  validateNamespace: {
    badNamespace: "Namespace '%s' failed validation. Namespace rules: must contain 1+ single dots, but not start or end with a dot; all other characters must be in [A-Za-z0-9-_]."
  }
}

---Whether a record is managed (installed and updated) or unmanaged (tracked only).
---@alias RecordType
---| "managed" # Managed: a script/module DependencyControl installs and keeps up to date
---| "unmanaged" # Unmanaged: a record describing a module DependencyControl tracks but does not update
RecordType = Enum "RecordType", {
  Managed: "managed"
  Unmanaged: "unmanaged"
}

---Whether a script is an automation script or a require()-able module.
---@alias ScriptType
---| "automation" # Automation: an automation script (macro / applied filter)
---| "module" # Module: a require()-able module
ScriptType = Enum "ScriptType", {
  Automation: "automation"
  Module: "module"
}

---The config/feed section a script type is stored under.
---@alias ScriptTypeSection
---| "macros" # Automation scripts are stored in the "macros" section
---| "modules" # Modules are stored in the "modules" section
ScriptTypeSection = Enum "ScriptTypeSection", {
  [ScriptType.Automation]: "macros"
  [ScriptType.Module]: "modules"
}

---User policy for fetching feeds that are neither trusted nor blocked.
---`prompt` only applies to feed discovery; dependency resolution always fetches and instead
---gates *installing* from an untrusted feed via the `feedTrustPromptThreshold` setting.
---@alias FetchUntrustedFeeds
---| "always" # Always: fetch untrusted feeds without asking (the default)
---| "never" # Never: never fetch untrusted feeds
---| "prompt" # Prompt: ask before fetching an untrusted feed; falls back to Never where no prompter is available (e.g. headless)
FetchUntrustedFeeds = Enum "FetchUntrustedFeeds", {
  Always: "always"
  Never: "never"
  Prompt: "prompt"
}

-- How sticky a recorded package-source choice is on subsequent resolutions of the same package.
---@alias SourceChoiceStickiness
---| "unset" # Unset: no preference recorded yet; resolve normally and prompt only if interactive
---| "once" # Once: prompt again whenever a choice remains, preselecting the recorded pick
---| "retain" # Retain: reuse the recorded pick whenever it's still eligible, without prompting
---| "pinned" # Pinned: always reuse the recorded pick; if it's gone, abort (required) or skip (optional)
---| "auto" # Auto: never prompt; always resolve via the ranking, refreshing the recorded pick for information
SourceChoiceStickiness = Enum "SourceChoiceStickiness", {
  Unset: "unset"
  Once: "once"
  Retain: "retain"
  Pinned: "pinned"
  Auto: "auto"
}

---Shared vocabulary of DependencyControl's problem domain: the kinds of scripts and records it
---manages, the human-readable terms for them, namespace rules, and install/test locations.
---@class Domain
Domain = {
  :FetchUntrustedFeeds
  :RecordType
  :ScriptType
  :ScriptTypeSection
  :SourceChoiceStickiness

  terms: {
    scriptType: {
      singular: {
        [ScriptType.Automation]: "automation script"
        [ScriptType.Module]: "module"
      }
      plural: {
        [ScriptType.Automation]: "automation scripts"
        [ScriptType.Module]: "modules"
      }
    }

    isInstall: {
      [true]: "installation"
      [false]: "update"
    }

    capitalize: (str) -> (str\sub 1, 1)\upper! .. str\sub 2
  }

  ---Validates a DependencyControl namespace string.
  ---@param namespace string
  ---@return boolean? valid True when the namespace is well-formed.
  ---@return string? err Validation error message when invalid.
  validateNamespace: (namespace) ->
    segments = [seg for seg in namespace\gmatch "[^%.]+"]
    _, dotCount = namespace\gsub "%.", ""
    if #segments >= 2 and dotCount == #segments - 1 and not namespace\match "[^-._%w]"
      return true
    return false, msgs.validateNamespace.badNamespace\format namespace

  ---Returns the Aegisub directory that scripts of the given type are installed to.
  ---@param scriptType ScriptType Whether to resolve the automation-script or module directory.
  ---@param rootDir? string Aegisub path token to resolve against (defaults to "?user").
  ---@return string? dir Absolute directory path, or nil for an unrecognized script type.
  getAutomationDir: (scriptType, rootDir = "?user") ->
    switch scriptType
      when ScriptType.Automation then aegisub.decode_path("#{rootDir}/automation/autoload")
      when ScriptType.Module then aegisub.decode_path("#{rootDir}/automation/include")
      else nil

  ---Returns the DepUnit test directory for scripts of the given type.
  ---@param scriptType ScriptType Whether to resolve the automation-script or module test directory.
  ---@param rootDir? string Aegisub path token to resolve against (defaults to "?user").
  ---@return string? dir Absolute directory path, or nil for an unrecognized script type.
  getTestDir: (scriptType, rootDir = "?user") ->
    switch scriptType
      when ScriptType.Automation then aegisub.decode_path("#{rootDir}/automation/tests/DepUnit/macros")
      when ScriptType.Module then aegisub.decode_path("#{rootDir}/automation/tests/DepUnit/modules")
      else nil
}

return Domain
