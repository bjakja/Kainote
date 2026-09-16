-- Hashing utilities: content digests used for integrity and identity (currently SHA-1).
-- Uses a fast native SHA-1 when one is available (CommonCrypto on macOS, libcrypto on
-- Linux, the Windows CryptoAPI), and falls back to a pure-Lua implementation otherwise —
-- so it always works, even headless / on platforms without the libs.

ffi = require "ffi"
bit = require "bit"
Enum = require "l0.DependencyControl.Enum"
band, bor, bxor, bnot = bit.band, bit.bor, bit.bxor, bit.bnot
lshift, rol, tobit, tohex = bit.lshift, bit.rol, bit.tobit, bit.tohex

msgs = {
  badData: "Expected a string payload to hash, got a '%s'."
  badExpected: "Expected a string hash to compare against, got a '%s'."
  mismatch: "Hash mismatch. Got %s, expected %s."
}

-- Formats a 20-byte digest buffer as a 40-character lowercase hex string.
digestToHex = (buf) -> table.concat ["%02x"\format buf[i] for i = 0, 19]

-- Pure-Lua SHA-1 (reference / fallback). Assumes a string input.
sha1Lua = (msg) ->
  h0, h1, h2, h3, h4 = 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
  bytes = #msg

  -- append 0x80, pad with zeros until length ≡ 56 (mod 64)
  msg ..= "\128"
  while #msg % 64 != 56
    msg ..= "\0"

  -- append the original length in bits as a 64-bit big-endian integer
  lenHi = math.floor bytes / 0x20000000
  lenLo = bytes * 8 % 0x100000000
  beBytes = (v) -> string.char(
    band(math.floor(v / 0x1000000), 0xFF), band(math.floor(v / 0x10000), 0xFF),
    band(math.floor(v / 0x100), 0xFF), band(v, 0xFF))
  msg ..= beBytes(lenHi) .. beBytes(lenLo)

  W = {}
  for chunk = 1, #msg, 64
    for i = 0, 15
      b0, b1, b2, b3 = string.byte msg, chunk + i * 4, chunk + i * 4 + 3
      W[i] = bor lshift(b0, 24), lshift(b1, 16), lshift(b2, 8), b3
    for i = 16, 79
      W[i] = rol bxor(W[i - 3], W[i - 8], W[i - 14], W[i - 16]), 1

    a, b, c, d, e = h0, h1, h2, h3, h4
    for i = 0, 79
      local f, k
      if i < 20
        f, k = bor(band(b, c), band(bnot(b), d)), 0x5A827999
      elseif i < 40
        f, k = bxor(b, c, d), 0x6ED9EBA1
      elseif i < 60
        f, k = bor(band(b, c), bor(band(b, d), band(c, d))), 0x8F1BBCDC
      else
        f, k = bxor(b, c, d), 0xCA62C1D6
      temp = tobit rol(a, 5) + f + e + k + W[i]
      e, d, c, b, a = d, c, rol(b, 30), a, temp

    h0 = tobit h0 + a
    h1 = tobit h1 + b
    h2 = tobit h2 + c
    h3 = tobit h3 + d
    h4 = tobit h4 + e

  tohex(h0) .. tohex(h1) .. tohex(h2) .. tohex(h3) .. tohex(h4)

-- Attempts to set up a native SHA-1. Returns (fn, backendName) or nil.
-- Each fn takes a string and returns the 40-char hex digest.
setupNativeSha1 = ->
  switch ffi.os
    when "OSX"
      -- CommonCrypto's CC_SHA1 is exported from libSystem (always loaded).
      pcall ffi.cdef, "unsigned char* CC_SHA1(const void* data, uint32_t len, unsigned char* md);"
      return unless pcall -> ffi.C.CC_SHA1
      digest = ffi.new "unsigned char[20]"
      impl = (msg) ->
        ffi.C.CC_SHA1 msg, #msg, digest
        digestToHex digest
      return impl, "CommonCrypto"

    when "Windows"
      okLib, advapi = pcall ffi.load, "advapi32"
      return unless okLib
      pcall ffi.cdef, [[
        int CryptAcquireContextW(uintptr_t* phProv, const wchar_t* container, const wchar_t* provider, unsigned long provType, unsigned long flags);
        int CryptCreateHash(uintptr_t hProv, unsigned int algId, uintptr_t hKey, unsigned long flags, uintptr_t* phHash);
        int CryptHashData(uintptr_t hHash, const unsigned char* data, unsigned long len, unsigned long flags);
        int CryptGetHashParam(uintptr_t hHash, unsigned long param, unsigned char* data, unsigned long* len, unsigned long flags);
        int CryptDestroyHash(uintptr_t hHash);
      ]]
      PROV_RSA_FULL, CRYPT_VERIFYCONTEXT = 1, 0xF0000000
      CALG_SHA1, HP_HASHVAL = 0x8004, 2
      prov = ffi.new "uintptr_t[1]"
      return if 0 == advapi.CryptAcquireContextW prov, nil, nil, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT
      hProv = prov[0]
      digest = ffi.new "unsigned char[20]"
      digestLen = ffi.new "unsigned long[1]"
      impl = (msg) ->
        hashPtr = ffi.new "uintptr_t[1]"
        return sha1Lua msg if 0 == advapi.CryptCreateHash hProv, CALG_SHA1, 0, 0, hashPtr
        hHash = hashPtr[0]
        advapi.CryptHashData hHash, msg, #msg, 0
        digestLen[0] = 20
        advapi.CryptGetHashParam hHash, HP_HASHVAL, digest, digestLen, 0
        advapi.CryptDestroyHash hHash
        digestToHex digest
      return impl, "CryptoAPI"

    else
      -- on Linux and other Unix, use OpenSSL's libcrypto
      local libcrypto
      for name in *{"libcrypto.so.3", "libcrypto.so.1.1", "libcrypto.so", "crypto"}
        okLib, lib = pcall ffi.load, name
        if okLib
          libcrypto = lib
          break
      return unless libcrypto
      digest = ffi.new "unsigned char[20]"

      -- use the non-deprecated EVP interface where available (OpenSSL 1.1+/3.0)
      pcall ffi.cdef, [[
        const void* EVP_sha1(void);
        void* EVP_MD_CTX_new(void);
        void EVP_MD_CTX_free(void* ctx);
        int EVP_DigestInit_ex(void* ctx, const void* type, void* engine);
        int EVP_DigestUpdate(void* ctx, const void* data, size_t count);
        int EVP_DigestFinal_ex(void* ctx, unsigned char* md, unsigned int* size);
      ]]
      if pcall -> libcrypto.EVP_MD_CTX_new
        md = libcrypto.EVP_sha1!
        impl = (msg) ->
          ctx = libcrypto.EVP_MD_CTX_new!
          return sha1Lua msg if ctx == nil
          libcrypto.EVP_DigestInit_ex ctx, md, nil
          libcrypto.EVP_DigestUpdate ctx, msg, #msg
          libcrypto.EVP_DigestFinal_ex ctx, digest, nil
          libcrypto.EVP_MD_CTX_free ctx
          digestToHex digest
        return impl, "OpenSSL (EVP)"

      -- on very old libcrypto, fall back to the legacy one-shot SHA1, deprecated in 3.0
      -- but still exported and resolvable by FFI at runtime
      pcall ffi.cdef, "unsigned char* SHA1(const unsigned char* d, size_t n, unsigned char* md);"
      return unless pcall -> libcrypto.SHA1
      impl = (msg) ->
        libcrypto.SHA1 msg, #msg, digest
        digestToHex digest
      return impl, "OpenSSL (SHA1)"

-- Resolve the SHA-1 backend, but only trust a native one if it reproduces the
-- reference digest (guards against a mis-bound symbol or wrong digest length).
sha1Impl, sha1Backend = sha1Lua, "lua"
ok, native, backendName = pcall setupNativeSha1
if ok and native
  verified, digest = pcall native, "abc"
  if verified and digest == sha1Lua "abc"
    sha1Impl, sha1Backend = native, backendName

---@alias HashType
---| "sha1" # Sha1: SHA-1, a 40-character lowercase hex digest
HashType = Enum "HashType", { Sha1: "sha1" }

-- each supported algorithm's string hasher, keyed by HashType value
algorithms = { [HashType.Sha1]: sha1Impl }

-- Computes the digest without input validation; shared by getDigest and verify.
getDigest = (hashType, data) ->
  valid, err = HashType\validate hashType, "hashType"
  return nil, err unless valid
  return nil, msgs.badData\format type(data) unless type(data) == "string"
  algorithms[hashType] data

---Serializes a value into a canonical string for hashing: table keys are emitted in sorted
---order so field ordering never affects the result, and every value is tagged with its type
---so distinct types can't collide (e.g. the number 1 vs. the string "1").
---@param value any The value to canonicalize.
---@return string canonical The canonicalized string.
canonicalize = (value) ->
  switch type value
    when "table"
      entries = {}
      entries[#entries + 1] = "#{canonicalize k}=#{canonicalize v}" for k, v in pairs value
      table.sort entries
      "{#{table.concat entries, ","}}"
    when "string" then "s:#{value}"
    when "number" then "n:#{string.format "%.17g", value}"
    when "boolean" then "b:#{value and 1 or 0}"
    when "nil" then "nil"
    else "#{type value}:#{tostring value}"

---@class Hash
---@field HashType Enum The hash algorithms `getDigest`/`verify` accept (currently `Sha1`).
---@field sha1Backend string Name of the active SHA-1 backend: "CommonCrypto", "OpenSSL (EVP)", "OpenSSL (SHA1)", "CryptoAPI", or "lua".
Hash = {
  :HashType
  sha1Backend: sha1Backend

  ---Computes the digest of a string under the given algorithm.
  ---Accepts arbitrary binary data: Lua strings are byte-safe, so any byte sequence (e.g. a file read
  ---in binary mode) hashes correctly. A raw FFI buffer must be converted with ffi.string(buf, len) first.
  ---Suitable for file integrity verification; not for security-sensitive use.
  ---@param hashType HashType The hash algorithm to use.
  ---@param data string The input bytes (may be binary).
  ---@return string? digest A lowercase hex digest, or nil on invalid input.
  ---@return string? err
  getDigest: getDigest

  ---Checks whether a string hashes to an expected digest under the given algorithm.
  ---@param hashType HashType The hash algorithm to use.
  ---@param data string The input bytes to hash.
  ---@param expected string The expected hex digest (compared case-insensitively).
  ---@return boolean? match True on match, false on mismatch, or nil on error.
  ---@return string? err The mismatch detail or error message.
  verify: (hashType, data, expected) ->
    return nil, msgs.badExpected\format type(expected) unless type(expected) == "string"
    actual, err = getDigest hashType, data
    return actual, err unless actual
    return true if actual == expected\lower!
    false, msgs.mismatch\format actual, expected

  ---Produces a deterministic SHA-1 hash of a (possibly nested) Lua value.
  ---Table keys are sorted before hashing, so field ordering never affects the result; pass an
  ---object pruned to just the fields you care about to obtain a stable content signature that
  ---ignores irrelevant differences. Useful for cheaply detecting whether semantic content changed.
  ---@param value any The value to hash.
  ---@return string hash A 40-character lowercase SHA-1 hex digest.
  getObjectHash: (value) -> getDigest HashType.Sha1, canonicalize value

  ---Computes a SHA-1 digest entirely in Lua; the fallback backend used when no native SHA-1 is
  ---available, and the reference the active backend is checked against. Tests read it directly;
  ---callers should use `get`/`verify`.
  ---@private
  ---@param msg string The input bytes to hash (assumed to be a string; not validated).
  ---@return string digest A 40-character lowercase hex digest.
  __sha1Lua: sha1Lua
}

return Hash
