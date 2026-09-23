resolveHost = require "l0.DependencyControl.helpers.resolve-host"

-- Parses one inet_aton numeric component: 0x-prefixed hex, 0-prefixed octal, or decimal. nil if invalid.
parseAtonPart = (s) ->
  if s\match "^0[xX]%x+$"
    tonumber s\sub(3), 16
  elseif s\match "^0[0-7]+$"
    tonumber s, 8
  elseif s\match "^%d+$"
    tonumber s, 10
  else
    nil

-- Whether a 4-byte IPv4 address falls in a non-public range (loopback, private, link-local, CGNAT, ...).
isPrivateIPv4 = (b) ->
  b0, b1 = b[1], b[2]
  return true if b0 == 0 -- 0.0.0.0/8 (this host)
  return true if b0 == 10 -- 10.0.0.0/8 (private)
  return true if b0 == 127 -- 127.0.0.0/8 (loopback)
  return true if b0 == 169 and b1 == 254 -- 169.254.0.0/16 (link-local, incl. cloud metadata)
  return true if b0 == 172 and b1 >= 16 and b1 <= 31 -- 172.16.0.0/12 (private)
  return true if b0 == 192 and b1 == 168 -- 192.168.0.0/16 (private)
  return true if b0 == 100 and b1 >= 64 and b1 <= 127 -- 100.64.0.0/10 (carrier-grade NAT)
  return true if b0 >= 240 -- 240.0.0.0/4 (reserved, incl. 255.255.255.255)
  return false

-- Whether a 16-byte IPv6 address falls in a non-public range; IPv4-mapped addresses defer to isPrivateIPv4.
isPrivateIPv6 = (b) ->
  -- ::ffff:a.b.c.d — classify the embedded IPv4
  mapped = b[11] == 0xff and b[12] == 0xff
  if mapped
    for i = 1, 10
      if b[i] != 0
        mapped = false
        break
  return isPrivateIPv4 {b[13], b[14], b[15], b[16]} if mapped

  -- :: (unspecified) and ::1 (loopback)
  highAllZero = true
  for i = 1, 15
    if b[i] != 0
      highAllZero = false
      break
  return true if highAllZero and (b[16] == 0 or b[16] == 1)

  b0, b1 = b[1], b[2]
  return true if b0 == 0xfe and b1 >= 0x80 and b1 <= 0xbf -- fe80::/10 (link-local)
  return true if b0 == 0xfc or b0 == 0xfd -- fc00::/7 (unique local)
  return false

---A host — a hostname or IP literal — that reports whether it targets a non-public IP address.
---@class Host
class Host
  -- Default resolver for non-literal hosts, mapping a hostname to a list of address byte arrays. Each
  -- instance copies it at construction, and tests override it via the constructor.
  ---@private
  ---@type fun(host: string): integer[][]?
  @__resolver = resolveHost

  ---@param host string A hostname or IP-literal string.
  ---@param resolver? fun(host: string): integer[][]? Address resolver (default: the class-level resolver).
  new: (@host, @__resolver = @@__resolver) =>
    @__literal = Host.parseIPv4Literal @host

  ---Resolves this host to a list of address byte arrays, cached across calls. A literal host yields its own
  ---bytes without resolving; a non-literal host is looked up via the resolver (nil when it can't be resolved).
  ---@return integer[][]? addresses
  addresses: =>
    return {@__literal} if @__literal
    if @__resolved == nil
      resolve = @__resolver
      @__resolved = resolve and resolve(@host) or false
    @__resolved or nil

  ---Reports whether this host targets a non-public IP address — private, loopback, link-local, or
  ---reserved. Resolves a non-literal host on first call; an unresolvable host reads as false.
  ---@return boolean private True when the host targets a non-public address.
  resolvesToPrivate: =>
    if @__private == nil
      if @__literal
        @__private = Host.isPrivateAddress @__literal
      else
        @__private = false
        if addresses = @addresses!
          for address in *addresses
            if Host.isPrivateAddress address
              @__private = true
              break
    @__private

  ---Builds a Host from a URL's host component, or nil when the URL carries no host.
  ---@param url string The URL to extract the host from.
  ---@param resolver? fun(host: string): integer[][]? Address resolver (default: the class-level resolver).
  ---@return Host? host
  @fromUrl = (url, resolver) ->
    host = Host.getUrlHostPart url
    host and Host(host, resolver) or nil

  ---Reports whether a resolved IP address is in a non-public range.
  ---@param bytes integer[] The address bytes (length 4 for IPv4, 16 for IPv6).
  ---@return boolean private True when the address is private/loopback/link-local/reserved.
  @isPrivateAddress = (bytes) ->
    switch #bytes
      when 4 then isPrivateIPv4 bytes
      when 16 then isPrivateIPv6 bytes
      else false

  ---Extracts the host from a URL, dropping protocol, auth, port and unwrapping a bracketed `[IPv6]`.
  ---@param url string The URL to extract the host from.
  ---@return string? host The host, or nil when none could be extracted.
  @getUrlHostPart = (url) ->
    return nil unless type(url) == "string"
    urlWithoutProtocol = url\gsub "^%a[%w+.-]*://", ""
    originWithoutProtocol = urlWithoutProtocol\match "^([^/?#]*)"
    return nil unless originWithoutProtocol
    -- drop userinfo, matching the fetcher: curl/WinINet split the authority at the LAST '@', so strip
    -- greedily. A first-'@' strip would leave `http://a@b@127.0.0.1/` as host `b@127.0.0.1` (unresolvable
    -- → treated as public) while the fetcher connects to 127.0.0.1, defeating the private-host block.
    host = originWithoutProtocol\gsub "^.*@", ""
    return host\match "^%[([^%]]*)%]" if host\sub(1, 1) == "[" -- bracketed IPv6
    hostWithoutPort = host\match "^([^:]*)"
    hostWithoutPort != "" and hostWithoutPort or nil

  ---Parses an IPv4 literal in inet_aton form (dotted/decimal/octal/hex, 1-4 parts) to a 4-byte array.
  ---Hand-rolled to catch the legacy encodings the fetcher (curl/WinINet) accepts but a resolver's
  ---inet_pton rejects — e.g. decimal `2130706433` (== 127.0.0.1), which could be used to bypass SSRF protection.
  ---IPv6 literals have a single canonical form the resolver parses identically, so they don't need this.
  ---@param host string The host string to parse.
  ---@return integer[]? bytes The 4 address bytes, or nil when `host` isn't an IPv4 literal.
  @parseIPv4Literal = (host) ->
    return nil unless type(host) == "string" and #host > 0
    -- empty components (leading/trailing/double dots) aren't a valid literal
    return nil if host\match("%.%.") or host\sub(1, 1) == "." or host\sub(-1) == "."
    parts = [p for p in host\gmatch "[^.]+"]
    n = #parts
    return nil if n < 1 or n > 4
    values = {}
    for i = 1, n
      v = parseAtonPart parts[i]
      return nil unless v
      values[i] = v
    byte = (v, shift) -> math.floor(v / shift) % 0x100
    switch n
      when 1
        v = values[1]
        return nil if v > 0xFFFFFFFF
        return {byte(v, 0x1000000), byte(v, 0x10000), byte(v, 0x100), v % 0x100}
      when 2 -- a.b : b fills the low 24 bits
        return nil if values[1] > 0xFF or values[2] > 0xFFFFFF
        return {values[1], byte(values[2], 0x10000), byte(values[2], 0x100), values[2] % 0x100}
      when 3 -- a.b.c : c fills the low 16 bits
        return nil if values[1] > 0xFF or values[2] > 0xFF or values[3] > 0xFFFF
        return {values[1], values[2], byte(values[3], 0x100), values[3] % 0x100}
      when 4
        for v in *values
          return nil if v > 0xFF
        return values

return Host
