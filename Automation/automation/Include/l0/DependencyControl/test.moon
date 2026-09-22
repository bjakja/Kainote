constants = require "l0.DependencyControl.Constants"
UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"

return UnitTestSuite constants.DEPCTRL_NAMESPACE, (DepCtrl, ...) ->
  -- The suite controls object is appended by UnitTestSuite\import as the final argument.
  -- Its index varies by loader (CLI vs Aegisub pass different arg counts), so grab the last one.
  nArgs = select "#", ...
  controls = select nArgs, ...
  ffi = require "ffi"

  isWindows = ffi.os == "Windows"
  basePath = aegisub.decode_path "?temp/l0.#{DepCtrl.__name}.#{UnitTestSuite.__name}_#{'%04X'\format math.random 0, 16^4-1}"

  -- Shared test helpers are themselves sibling test modules, so they load through requireTest
  -- and resolve under both the Aegisub-default and custom (CI) suite roots. requireHelper stays
  -- lazy for the optional integration deps, which must fail soft when their rocks aren't installed.
  requireHelper = (name) -> controls\requireTest "helpers.#{name}"
  stubHelpers = requireHelper "stub-helpers"

  -- Each test class lives in its own sibling module under `test/`, loaded via the suite's
  -- requireTest helper so the same call resolves in both the Aegisub-default and custom (CI)
  -- test locations. Classes needing shared fixtures receive them as arguments (basePath for
  -- temp-file paths, DepCtrl for the live record/logger, isWindows for platform branches,
  -- stubHelpers for the shared stub-self/null-logger/feed-trust builders).
  {
    Timer: (controls\requireTest "Timer")!
    BadMutex: (controls\requireTest "BadMutex")!
    Hash: (controls\requireTest "hash")!
    ModuleProvider: (controls\requireTest "ModuleProvider") basePath, DepCtrl
    Downloader: (controls\requireTest "Downloader") basePath
    Domain: (controls\requireTest "domain")!
    Environment: (controls\requireTest "environment")!
    Utils: (controls\requireTest "utils")!
    FileOps: (controls\requireTest "file-ops") basePath, isWindows
    Logger: (controls\requireTest "Logger")!
    UnitTestSuite: (controls\requireTest "UnitTestSuite")!
    Enum: (controls\requireTest "Enum")!
    Accessors: (controls\requireTest "Accessors")!
    Finalizer: (controls\requireTest "Finalizer")!
    SemanticVersion: (controls\requireTest "SemanticVersion")!
    Lock: (controls\requireTest "Lock")!
    FileLock: (controls\requireTest "FileLock")!
    NamedSemaphore: (controls\requireTest "NamedSemaphore")!
    ConfigHandler: (controls\requireTest "ConfigHandler")!
    ConfigView: (controls\requireTest "ConfigView")!
    ConfigSchema: (controls\requireTest "config-schema")!
    ModuleLoader: (controls\requireTest "ModuleLoader")!
    PackageRecord: (controls\requireTest "PackageRecord") basePath, stubHelpers
    UpdateTask: (controls\requireTest "UpdateTask") stubHelpers
    Updater: (controls\requireTest "Updater") stubHelpers
    FeedTrust: (controls\requireTest "FeedTrust") stubHelpers
    FeedLoader: (controls\requireTest "FeedLoader") basePath, DepCtrl
    Host: (controls\requireTest "Host")!
    FeedInventory: (controls\requireTest "FeedInventory")!
    FeedManager: (controls\requireTest "FeedManager")!
    FileCache: (controls\requireTest "FileCache") basePath
    ScriptUpdateRecord: (controls\requireTest "ScriptUpdateRecord")!
    ReleaseNotes: (controls\requireTest "release-notes")!
    UpdateFeed: (controls\requireTest "UpdateFeed") basePath, DepCtrl, stubHelpers
    GitRepository: (controls\requireTest "GitRepository")!
    ScriptTargetFilter: (controls\requireTest "ScriptTargetFilter")!
    ZipArchiver: (controls\requireTest "ZipArchiver") basePath
    JsonSchema: (controls\requireTest "JsonSchema") basePath
    FfiPosix: (controls\requireTest "ffi-posix")!
    OpenUrl: (controls\requireTest "open-url")!
    DownloaderIntegration: (controls\requireTest "integration.Downloader") basePath, requireHelper
    ZipArchiverIntegration: (controls\requireTest "integration.ZipArchiver") basePath
  }
