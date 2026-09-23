-- Hash tests: SHA-1 against known vectors, the active backend against the pure-Lua reference,
-- and the verify helper. Called from test.moon as: (require "...test.hash")!
->
  Hash = require "l0.DependencyControl.hash"
  ht = Hash.HashType

  {
    _description: "Tests for the Hash utilities (SHA-1 digests and verification) against known vectors."

    sha1_abc: (ut) ->
      ut\assertEquals Hash.getDigest(ht.Sha1, "abc"), "a9993e364706816aba3e25717850c26c9cd0d89d"

    sha1_empty: (ut) ->
      ut\assertEquals Hash.getDigest(ht.Sha1, ""), "da39a3ee5e6b4b0d3255bfef95601890afd80709"

    -- exercises multi-block padding (>55 bytes)
    sha1_quickBrownFox: (ut) ->
      ut\assertEquals Hash.getDigest(ht.Sha1, "The quick brown fox jumps over the lazy dog"),
        "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"

    -- binary payloads (embedded NUL and high bytes) hash without error
    sha1_binaryData: (ut) ->
      digest = Hash.getDigest ht.Sha1, "\0\1\2\254\255"
      ut\assertMatches digest, "^%x+$"
      ut\assertEquals #digest, 40

    sha1_rejectsNonString: (ut) ->
      result, err = Hash.getDigest ht.Sha1, 42
      ut\assertNil result
      ut\assertString err

    -- whichever backend is active (native or lua) must match the reference impl
    sha1_backendMatchesReference: (ut) ->
      for input in *{"", "abc", "The quick brown fox jumps over the lazy dog", "\0\1\2\254\255"}
        ut\assertEquals Hash.getDigest(ht.Sha1, input), Hash.__sha1Lua input

    verify_matchIsCaseInsensitive: (ut) ->
      digest = "a9993e364706816aba3e25717850c26c9cd0d89d"
      ut\assertTrue Hash.verify ht.Sha1, "abc", digest
      ut\assertTrue Hash.verify ht.Sha1, "abc", digest\upper!

    verify_mismatchReturnsFalseAndErr: (ut) ->
      match, err = Hash.verify ht.Sha1, "abc", "0000000000000000000000000000000000000000"
      ut\assertEquals match, false
      ut\assertString err

    verify_rejectsNonStringExpected: (ut) ->
      result, err = Hash.verify ht.Sha1, "abc", 42
      ut\assertNil result
      ut\assertString err

    -- getObjectHash: deterministic, order-independent SHA-1 of a (nested) value

    getObjectHash_isHexString: (ut) ->
      hash = Hash.getObjectHash {a: 1, b: "two"}
      ut\assertString hash
      ut\assertMatches hash, "^%x+$"

    getObjectHash_deterministic: (ut) ->
      ut\assertEquals Hash.getObjectHash({a: 1, b: 2}), Hash.getObjectHash {a: 1, b: 2}

    getObjectHash_ignoresKeyOrder: (ut) ->
      ut\assertEquals Hash.getObjectHash({a: 1, b: 2, c: 3}), Hash.getObjectHash {c: 3, a: 1, b: 2}

    getObjectHash_nestedOrderIndependent: (ut) ->
      a = {x: {p: 1, q: 2}, y: 3}
      b = {y: 3, x: {q: 2, p: 1}}
      ut\assertEquals Hash.getObjectHash(a), Hash.getObjectHash b

    getObjectHash_distinguishesContent: (ut) ->
      ut\assertNotEquals Hash.getObjectHash({v: "1"}), Hash.getObjectHash {v: "2"}

    -- type tagging keeps the number 1 and the string "1" from colliding
    getObjectHash_typeTagged: (ut) ->
      ut\assertNotEquals Hash.getObjectHash({v: 1}), Hash.getObjectHash {v: "1"}

    _order: {
      "sha1_abc", "sha1_empty", "sha1_quickBrownFox",
      "sha1_binaryData", "sha1_rejectsNonString", "sha1_backendMatchesReference",
      "verify_matchIsCaseInsensitive", "verify_mismatchReturnsFalseAndErr", "verify_rejectsNonStringExpected",
      "getObjectHash_isHexString", "getObjectHash_deterministic", "getObjectHash_ignoresKeyOrder",
      "getObjectHash_nestedOrderIndependent", "getObjectHash_distinguishesContent", "getObjectHash_typeTagged"
    }
  }
