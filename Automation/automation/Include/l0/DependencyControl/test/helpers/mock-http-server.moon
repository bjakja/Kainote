-- Standalone HTTP mock server for DependencyControl's Downloader integration tests.
--
-- Built on pegasus (HTTP) + copas (concurrency). pegasus' own `server\start` serves one
-- connection at a time; running it under copas (as below) is what makes it handle many
-- connections concurrently — the whole point here, since the Downloader's scheduling can only
-- be stressed with several transfers genuinely in flight at once.
--
-- This is MoonScript, but it's launched in a *fresh* interpreter, so MockHttpServerController
-- compiles it to plain Lua first (the spawned process needs no MoonScript). Endpoints serve
-- files from --dir:
--   GET /fast/<name>                      full-speed response (Content-Length)
--   GET /slow/<name>?delay=<ms>&chunk=<n>  chunked response, <n> bytes every <ms> ms
--   GET /status/<code>                    respond with the given HTTP status
--   GET /redirect-to/<path>               302 with a relative Location of /<path>
--   GET /__quit                           stop the server (the clean shutdown route)
--
-- Flags: --dir <d>, --port <n> (loopback port; 0 or omitted binds an ephemeral one), --ready-file
-- <p> (the bound port is written here once the server is dispatching), --quit-file <p> (the server
-- exits when this file appears), --max-lifetime <s> (orphan safety, default 120), --check (verify
-- deps load, then exit 0/1 without starting a server).

-- "--flag value" / "--flag" parser (a flag with no following value is a boolean)

CONTENT_TYPE_HEADER_NAME = "Content-Type"
MIME_TYPE_BINARY = "application/octet-stream"
LOCALHOST_IP = "127.0.0.1"

parseArgs = (argv) ->
  opts, i = {}, 1
  while argv[i]
    flag = argv[i]\match "^%-%-(.+)$"
    if flag
      value = argv[i + 1]
      if value and not value\match "^%-%-"
        opts[flag], i = value, i + 2
      else
        opts[flag], i = true, i + 1
    else
      i += 1
  opts

opts = parseArgs arg or {}

-- --check: confirm the server's dependencies are installed, then exit. Lets the integration
-- tests gate themselves on "can we actually launch the server here?" without an env var.
depsOk = pcall ->
  require "socket"
  require "copas"
  require "pegasus.handler"

if opts.check
  os.exit depsOk and 0 or 1
assert depsOk, "mock server dependencies (luasocket, copas, pegasus) are not available"

socket = require "socket"
copas = require "copas"
Handler = require "pegasus.handler"

requestedPort = tonumber(opts.port) or 0
serveDir = opts.dir or "."
maxLifetime = tonumber(opts["max-lifetime"]) or 120
readyFile = opts["ready-file"]
quitFile = opts["quit-file"]

readFile = (path) ->
  f = io.open path, "rb"
  return nil unless f
  data = f\read "*a"
  f\close!
  data

fileExists = (path) ->
  return false unless path
  f = io.open path, "r"
  return false unless f
  f\close!
  true

-- map a request name to a file inside serveDir, rejecting traversal
resolve = (name) ->
  return nil if not name or name == "" or name\find "%.%."
  "#{serveDir}/#{name}"

quitRequested = false

handleRequest = (req, res) ->
  path = req\path!

  if path == "/__quit"
    res\statusCode 200
    res\write "bye"
    quitRequested = true
    return res\close!

  if code = path\match "^/status/(%d+)$"
    res\statusCode tonumber code
    res\write "status #{code}"
    return res\close!

  if target = path\match "^/redirect%-to/(.+)$"
    res\statusCode 302
    res\addHeader "Location", "/#{target}" -- relative, so it also exercises redirect resolution
    res\write "redirecting to /#{target}"
    return res\close!

  if name = path\match "^/fast/(.+)$"
    data = readFile resolve name
    return res\statusCode(404)\write("not found") unless data
    res\statusCode 200
    res\addHeader CONTENT_TYPE_HEADER_NAME, MIME_TYPE_BINARY
    res\write data -- non-streaming: Content-Length, single send
    return res\close!

  if name = path\match "^/slow/(.+)$"
    data = readFile resolve name
    return res\statusCode(404)\write("not found") unless data
    delayMs = tonumber(req.querystring.delay) or 50
    chunk = tonumber(req.querystring.chunk) or 1024
    res\statusCode 200
    res\addHeader CONTENT_TYPE_HEADER_NAME, MIME_TYPE_BINARY
    for i = 1, #data, chunk
      res\write data\sub(i, i + chunk - 1), true -- stayOpen => chunked
      copas.sleep delayMs / 1000 -- yields, so other transfers flow
    return res\close!

  res\statusCode 404
  res\write "unknown endpoint"
  res\close!

-- bind the loopback port the controller chose, or an ephemeral one when it passed 0
server = assert socket.bind LOCALHOST_IP, requestedPort
_, listenPort = server\getsockname!
listenPort = assert tonumber(listenPort), "couldn't determine the bound port"

handler = Handler\new handleRequest, serveDir, {}, nil
copas.addserver server, copas.handler (client) -> handler\processRequest listenPort, client

-- announce readiness from inside the copas loop: the ready-file appears once the scheduler is
-- dispatching, so the controller never reads a port before the server can serve
if readyFile
  copas.addthread ->
    f = assert io.open readyFile, "w"
    f\write tostring listenPort
    f\close!

-- shut down on the /__quit route or the controller's quit-file, or after max-lifetime so we never orphan
startedAt = os.time!
copas.addthread ->
  while true
    copas.sleep 0.1
    os.exit 0 if quitRequested or (quitFile and fileExists quitFile) or os.time! - startedAt > maxLifetime

copas.loop!
