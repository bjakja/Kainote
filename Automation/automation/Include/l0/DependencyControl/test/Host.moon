-- Host tests: URL host extraction, IPv4/IPv6 range classification, inet_aton literal parsing, and the
-- instance layer (resolution via an injected resolver, cached, plus fromUrl). Called from test.moon as:
-- (controls\requireTest "Host")!
() ->
  Host = require "l0.DependencyControl.Host"

  -- A resolver over a fixed {host -> {address, ...}} map; an unknown host resolves to nil (lookup failed).
  resolverOver = (map) -> (host) -> map[host]

  {
    _description: "Host: SSRF address classification — URL parsing, IP ranges, inet_aton literals, resolution."

    -- isPrivateAddress: IPv4 non-public ranges
    isPrivateAddress_ipv4Private: (ut) ->
      ut\assertTrue Host.isPrivateAddress {127, 0, 0, 1}
      ut\assertTrue Host.isPrivateAddress {10, 0, 0, 5}
      ut\assertTrue Host.isPrivateAddress {192, 168, 1, 1}
      ut\assertTrue Host.isPrivateAddress {172, 16, 0, 1}
      ut\assertTrue Host.isPrivateAddress {172, 31, 255, 255}
      ut\assertTrue Host.isPrivateAddress {169, 254, 169, 254} -- cloud metadata
      ut\assertTrue Host.isPrivateAddress {100, 64, 0, 1} -- CGNAT
      ut\assertTrue Host.isPrivateAddress {0, 0, 0, 0}
      ut\assertTrue Host.isPrivateAddress {255, 255, 255, 255}

    -- isPrivateAddress: IPv4 public ranges (incl. just outside the private blocks)
    isPrivateAddress_ipv4Public: (ut) ->
      ut\assertFalse Host.isPrivateAddress {8, 8, 8, 8}
      ut\assertFalse Host.isPrivateAddress {1, 1, 1, 1}
      ut\assertFalse Host.isPrivateAddress {172, 15, 0, 1} -- just below 172.16/12
      ut\assertFalse Host.isPrivateAddress {172, 32, 0, 1} -- just above 172.16/12
      ut\assertFalse Host.isPrivateAddress {100, 128, 0, 1} -- just above 100.64/10
      ut\assertFalse Host.isPrivateAddress {192, 167, 1, 1}

    -- isPrivateAddress: IPv6 ranges, including IPv4-mapped
    isPrivateAddress_ipv6: (ut) ->
      loopback = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
      unspecified = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
      linkLocal = {0xfe, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
      uniqueLocal = {0xfc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
      mappedPrivate = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 127, 0, 0, 1}
      mappedPublic = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 8, 8, 8, 8}
      publicV6 = {0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
      ut\assertTrue Host.isPrivateAddress loopback
      ut\assertTrue Host.isPrivateAddress unspecified
      ut\assertTrue Host.isPrivateAddress linkLocal
      ut\assertTrue Host.isPrivateAddress uniqueLocal
      ut\assertTrue Host.isPrivateAddress mappedPrivate
      ut\assertFalse Host.isPrivateAddress mappedPublic
      ut\assertFalse Host.isPrivateAddress publicV6

    -- getUrlHostPart: scheme, userinfo, port, and bracketed IPv6 stripping
    getUrlHostPart_variants: (ut) ->
      ut\assertEquals Host.getUrlHostPart("https://example.com/feed.json"), "example.com"
      ut\assertEquals Host.getUrlHostPart("http://user:pass@10.0.0.1:8080/x"), "10.0.0.1"
      ut\assertEquals Host.getUrlHostPart("https://[::1]:443/x"), "::1"
      ut\assertEquals Host.getUrlHostPart("http://127.0.0.1"), "127.0.0.1"
      ut\assertEquals Host.getUrlHostPart("https://Example.COM/"), "Example.COM"
      -- userinfo containing '@' must strip to the LAST '@', as the fetcher does, so the real host is checked
      ut\assertEquals Host.getUrlHostPart("http://a@b@127.0.0.1/x"), "127.0.0.1"

    getUrlHostPart_noHost: (ut) ->
      ut\assertNil Host.getUrlHostPart "file:///etc/passwd"
      ut\assertNil Host.getUrlHostPart 42

    -- parseIPv4Literal: the inet_aton encodings that would otherwise bypass a naive host check
    parseIPv4Literal_encodings: (ut) ->
      ut\assertEquals Host.parseIPv4Literal("127.0.0.1"), {127, 0, 0, 1}
      ut\assertEquals Host.parseIPv4Literal("2130706433"), {127, 0, 0, 1} -- decimal
      ut\assertEquals Host.parseIPv4Literal("0x7f000001"), {127, 0, 0, 1} -- hex
      ut\assertEquals Host.parseIPv4Literal("0177.0.0.1"), {127, 0, 0, 1} -- octal first octet
      ut\assertEquals Host.parseIPv4Literal("127.1"), {127, 0, 0, 1} -- 2-part fill
      ut\assertEquals Host.parseIPv4Literal("192.168.1"), {192, 168, 0, 1} -- 3-part fill

    parseIPv4Literal_notLiterals: (ut) ->
      ut\assertNil Host.parseIPv4Literal "example.com"
      ut\assertNil Host.parseIPv4Literal "256.1.1.1" -- octet out of range
      ut\assertNil Host.parseIPv4Literal "1.2.3.4.5" -- too many parts
      ut\assertNil Host.parseIPv4Literal ".1.2.3" -- leading dot
      ut\assertNil Host.parseIPv4Literal "1.2..3" -- empty component
      ut\assertNil Host.parseIPv4Literal "::1" -- not IPv4

    -- isPrivate: literal hosts classified without the resolver
    isPrivate_literalHost: (ut) ->
      neverResolve = (host) -> error "resolver must not be called for a literal host"
      ut\assertTrue (Host "127.0.0.1", neverResolve)\resolvesToPrivate!
      ut\assertTrue (Host "2130706433", neverResolve)\resolvesToPrivate! -- decimal-encoded loopback
      ut\assertFalse (Host "93.184.216.34", neverResolve)\resolvesToPrivate!

    -- isPrivate: named hosts resolved through the injected resolver; any private address counts
    isPrivate_resolvedHost: (ut) ->
      resolve = resolverOver {
        "internal.example": {{10, 0, 0, 5}}
        "evil.example": {{93, 184, 216, 34}}
        "rebind.example": {{93, 184, 216, 34}, {127, 0, 0, 1}} -- one public, one loopback
        "::1": {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}}
      }
      ut\assertTrue (Host "internal.example", resolve)\resolvesToPrivate!
      ut\assertFalse (Host "evil.example", resolve)\resolvesToPrivate!
      ut\assertTrue (Host "rebind.example", resolve)\resolvesToPrivate!
      ut\assertTrue (Host "::1", resolve)\resolvesToPrivate!

    -- a failed lookup (resolver returns nil) leaves a non-literal host not-private
    isPrivate_unresolved: (ut) ->
      ut\assertFalse (Host "unknown.example", resolverOver {})\resolvesToPrivate!
      ut\assertFalse (Host "unknown.example", (-> nil))\resolvesToPrivate!

    -- resolution is cached: the resolver runs once across repeated queries
    addresses_cachesResolution: (ut) ->
      calls = 0
      resolve = (host) ->
        calls += 1
        {{10, 0, 0, 1}}
      host = Host "internal.example", resolve
      host\resolvesToPrivate!
      host\resolvesToPrivate!
      ut\assertEquals calls, 1

    -- fromUrl builds a Host from a URL's host, or nil when there is none
    fromUrl_extractsHost: (ut) ->
      resolve = resolverOver {"internal.example": {{10, 0, 0, 5}}}
      ut\assertTrue (Host.fromUrl "https://internal.example/feed", resolve)\resolvesToPrivate!
      ut\assertTrue (Host.fromUrl "http://127.0.0.1/x")\resolvesToPrivate!
      ut\assertNil Host.fromUrl "file:///etc/passwd"

    _order: {
      "isPrivateAddress_ipv4Private", "isPrivateAddress_ipv4Public", "isPrivateAddress_ipv6"
      "getUrlHostPart_variants", "getUrlHostPart_noHost"
      "parseIPv4Literal_encodings", "parseIPv4Literal_notLiterals"
      "isPrivate_literalHost", "isPrivate_resolvedHost", "isPrivate_unresolved"
      "addresses_cachesResolution", "fromUrl_extractsHost"
    }
  }
