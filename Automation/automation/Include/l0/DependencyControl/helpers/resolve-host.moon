-- Resolves a hostname (or IPv6 literal) to its IP addresses via the platform getaddrinfo(3), returning
-- each address as a byte array (4 bytes for IPv4, 16 bytes for IPv6) or nil when it can't resolve.

ffi = require "ffi"

-- sockaddr layouts are standardized, so the address bytes sit at the same offsets on every platform.
sockaddrDefs = [[
  struct dc_sockaddr_in {
      short          sin_family;
      unsigned short sin_port;
      unsigned char  sin_addr[4];
      char           sin_zero[8];
  };
  struct dc_sockaddr_in6 {
      short          sin6_family;
      unsigned short sin6_port;
      unsigned int   sin6_flowinfo;
      unsigned char  sin6_addr[16];
      unsigned int   sin6_scope_id;
  };
]]

-- The `addrinfo` layout is platform-specific
addrinfoDef = switch ffi.os
  when "Windows"
    [[ struct dc_addrinfo { int ai_flags; int ai_family; int ai_socktype; int ai_protocol;
            size_t ai_addrlen; void* ai_canonname; void* ai_addr; struct dc_addrinfo* ai_next; }; ]]
  when "OSX", "BSD"
    [[ struct dc_addrinfo { int ai_flags; int ai_family; int ai_socktype; int ai_protocol;
            unsigned int ai_addrlen; void* ai_canonname; void* ai_addr; struct dc_addrinfo* ai_next; }; ]]
  else
    [[ struct dc_addrinfo { int ai_flags; int ai_family; int ai_socktype; int ai_protocol;
            unsigned int ai_addrlen; void* ai_addr; void* ai_canonname; struct dc_addrinfo* ai_next; }; ]]

pcall ffi.cdef, sockaddrDefs
pcall ffi.cdef, addrinfoDef
pcall ffi.cdef, [[
  int getaddrinfo(const char* node, const char* service, const void* hints, struct dc_addrinfo** res);
  void freeaddrinfo(struct dc_addrinfo* res);
]]

local lib
if ffi.os == "Windows"
  pcall ffi.cdef, "int WSAStartup(unsigned short version, void* data);"
  ok, winsock2 = pcall ffi.load, "ws2_32"
  lib = ok and winsock2 or nil
else
  lib = ffi.C

-- AF_INET is 2 on every platform, but AF_INET6 has a different value on each platform.
-- Since host name lookups can only yield IPv4 or IPv6 addresses, we can just treat
-- anything that isn't AF_INET as AF_INET6 and avoid the platform-specific constant.
AF_INET = 2

-- Initializes Winsock once on Windows, which is required before calling `getaddrinfo()`.
wsaStarted = false
ensureWinsock2 = ->
  return true unless ffi.os == "Windows"
  return true if wsaStarted
  return false unless lib
  -- WSAStartup is ref-counted, so no need to check if it's already started
  wsaStarted = pcall -> lib.WSAStartup 0x0202, ffi.new "char[512]" -- request Winsock 2.2
  return wsaStarted

---Resolves a hostname or IPv6 literal to a list of address byte arrays, or nil when it can't be resolved.
---@param host string The hostname or IPv6 literal to resolve.
---@return integer[][]? addresses One byte array per resolved address (4 bytes IPv4, 16 bytes IPv6).
resolveHost = (host) ->
  return nil unless lib and type(host) == "string" and #host > 0
  return nil if ensureWinsock2 and not ensureWinsock2!

  res = ffi.new "struct dc_addrinfo*[1]"
  ok, rc = pcall lib.getaddrinfo, host, nil, nil, res
  return nil unless ok and rc == 0

  addresses = {}
  node = res[0]
  while node != nil
    addr = node.ai_addr
    if addr != nil
      if node.ai_family == AF_INET
        sa = ffi.cast "struct dc_sockaddr_in*", addr
        addresses[#addresses + 1] = [sa.sin_addr[i] for i = 0, 3]
      else
        sa = ffi.cast "struct dc_sockaddr_in6*", addr
        addresses[#addresses + 1] = [sa.sin6_addr[i] for i = 0, 15]
    node = node.ai_next
  pcall lib.freeaddrinfo, res[0]

  #addresses > 0 and addresses or nil

return resolveHost
