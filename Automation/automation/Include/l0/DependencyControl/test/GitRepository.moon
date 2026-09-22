-- GitRepository tests: git command execution and version suffix derivation.
-- Called from Tests.moon as: (require "...test.GitRepository")!
() ->
  GitRepository = require "l0.DependencyControl.GitRepository"

  {
    _description: "Tests for GitRepository: git command execution and version suffix derivation."

    -- run

    run_returnsOutput: (ut) ->
      git = GitRepository "/some/dir"
      mockHandle = {
        read: (h, f) -> "main\n"
        close: (h) -> true
      }
      (ut\stub io, "popen")\returns mockHandle
      ut\assertEquals git\run("rev-parse --abbrev-ref HEAD"), "main"

    run_nilOnEmptyOutput: (ut) ->
      git = GitRepository "/some/dir"
      mockHandle = {
        read: (h, f) -> "   \n"
        close: (h) -> true
      }
      (ut\stub io, "popen")\returns mockHandle
      ut\assertNil git\run "status"

    run_nilOnPopenFailure: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub io, "popen")\returns nil
      ut\assertNil git\run "status"

    -- getBranch / getCommitHash / isAtTag delegate to run

    getBranch_returnsRef: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub git, "run")\returns "feature/x"
      ut\assertEquals git\getBranch!, "feature/x"

    getCommitHash_returnsHash: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub git, "run")\returns "a1b2c3d"
      ut\assertEquals git\getCommitHash!, "a1b2c3d"

    isAtTag_true: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub git, "run")\returns "v1.0.0"
      ut\assertTrue git\isAtTag!

    isAtTag_false: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub git, "run")\returns nil
      ut\assertFalse git\isAtTag!

    -- getVersionSuffix

    getVersionSuffix_atTag: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub git, "isAtTag")\returns true
      ut\assertEquals git\getVersionSuffix!, ""

    getVersionSuffix_notAtTag: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub git, "isAtTag")\returns false
      (ut\stub git, "getBranch")\returns "main"
      (ut\stub git, "getCommitHash")\returns "abc1234"
      -- getVersionSuffix prefixes the short hash with git's "g"
      -- cspell:ignore gabc
      ut\assertEquals git\getVersionSuffix!, "-main-gabc1234"

    getVersionSuffix_unknownFallbacks: (ut) ->
      git = GitRepository "/some/dir"
      (ut\stub git, "isAtTag")\returns false
      (ut\stub git, "getBranch")\returns nil
      (ut\stub git, "getCommitHash")\returns nil
      ut\assertEquals git\getVersionSuffix!, "-unknown-g0000000"

    _order: {
      "run_returnsOutput", "run_nilOnEmptyOutput", "run_nilOnPopenFailure",
      "getBranch_returnsRef", "getCommitHash_returnsHash",
      "isAtTag_true", "isAtTag_false",
      "getVersionSuffix_atTag", "getVersionSuffix_notAtTag",
      "getVersionSuffix_unknownFallbacks"
    }
  }
