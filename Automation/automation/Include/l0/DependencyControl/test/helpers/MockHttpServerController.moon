ffi = require "ffi"
moonbase = require "moonscript.base"
fileOps = require "l0.DependencyControl.file-ops"
Timer = require "l0.DependencyControl.Timer"

MOCK_SERVER_FILE_BASENAME = "mock-http-server"

isWindows = ffi.os == "Windows"
interpreter = (arg and arg[-1]) or "luajit"

-- Under CLI, luasocket loads in our own process, so we pick the port and poll for a real HTTP response.
-- In Aegisub, the internal LuaJIT is incompatible with the luasocket rock, so the server picks an ephemeral port,
-- reports it through a ready-file, and stops when a quit-file appears, keeping luasocket out of our process.
canDriveWithSocket = pcall require, "socket"

-- mock-http-server.moon sits next to this file; locate it from our own source path.
_, device, dir = fileOps.validateFullPath debug.getinfo(1, "S").source\gsub("^@", ""), true
serverSourcePath = fileOps.joinPath "#{device}#{dir}", "#{MOCK_SERVER_FILE_BASENAME}.moon"

quote = (s) -> "\"#{tostring(s)}\""

spawnDetached = (cmd) ->
  os.execute isWindows and "start \"\" /b #{cmd}" or "#{cmd} >/dev/null 2>&1 &"

-- os.execute on Windows mis-parses a command that begins with a quote unless the whole command
-- is wrapped in one more pair of quotes (cmd /c then strips the outer pair).
runBlocking = (cmd) -> os.execute isWindows and quote(cmd) or cmd

---Controls a mock HTTP server subprocess for the Downloader integration tests.
---@class MockHttpServerController
class MockHttpServerController
  -- Compile mock-http-server.moon to a throwaway .lua once, so we can spawn it with a bare interpreter.
  @compileServer = =>
    return @compiledServerPath if @compiledServerPath

    serverSource = assert fileOps.readFile serverSourcePath
    compiledServerLua = assert moonbase.to_lua serverSource
    tempDir = assert fileOps.createTempDir!
    path = fileOps.joinPath tempDir, "#{MOCK_SERVER_FILE_BASENAME}.lua"
    assert fileOps.writeFile path, compiledServerLua
    @compiledServerPath, @compiledServerTempDir = path, tempDir
    return @compiledServerPath

  ---Whether the server can be launched here: its subprocess must load luasocket, copas and pegasus.
  ---Checked by spawning it with --check, so this needs no luasocket in our own process.
  @isReady: =>
    success, errMsg = pcall @compileServer, @
    return false, "mock server compilation failed: #{errMsg}" unless success
    return true if runBlocking("#{quote interpreter} #{quote @compiledServerPath} --check")
    return false, "mock server dependencies (luasocket/copas/pegasus) not available"

  ---@param opts? table Options: serveDir (directory to serve), maxLifetime (server self-destruct
  ---timeout in seconds), timeout (readiness wait in seconds).
  new: (opts = {}) =>
    @serveDir = opts.serveDir or "."
    @maxLifetime = opts.maxLifetime or 120
    @timeout = opts.timeout or 10

  ---Starts the server and blocks until it answers on its loopback port.
  ---@return MockHttpServerController self
  start: =>
    return @__startViaSocket! if canDriveWithSocket
    return @__startViaControlFiles!

  ---Signals the server to stop, best-effort. Its max-lifetime cleans it up regardless.
  stop: =>
    return unless @port
    if @quitFile
      fileOps.writeFile @quitFile, "", true
    else
      @__quitViaSocket!
    @port, @baseUrl, @quitFile = nil, nil, nil

  ---Assembles the launch command shared by both modes and spawns it detached.
  ---@private
  ---@param portArgs string[] The port-related flags for the chosen mode.
  __spawn: (portArgs) =>
    args = { quote(interpreter), quote(@@compileServer!), "--dir", quote(@serveDir), "--max-lifetime", tostring(@maxLifetime) }
    args[#args + 1] = flag for flag in *portArgs
    spawnDetached table.concat args, " "

  ---Starts the server and blocks until it answers on its loopback port, using a socket probe to detect readiness.
  ---@private
  ---@return MockHttpServerController self
  __startViaSocket: =>
    socket = require "socket"
    -- Tiny race between closing the probe and the server re-binding the port, but it's loopback and
    -- a throwaway server.
    probe = assert socket.bind "127.0.0.1", 0
    _, port = probe\getsockname!
    probe\close!
    @port, @baseUrl = port, "http://127.0.0.1:#{port}"
    @__spawn { "--port", tostring(port) }

    -- Ready only once the server answers HTTP: a bare TCP connect succeeds as soon as the kernel
    -- accepts into the listen backlog, which can happen before copas starts dispatching.
    isServing = ->
      conn = socket.tcp!
      conn\settimeout 0.5
      unless conn\connect "127.0.0.1", port
        conn\close!
        return false
      conn\send "GET /status/200 HTTP/1.0\r\nHost: 127.0.0.1\r\n\r\n"
      statusLine = conn\receive "*l"
      conn\close!
      statusLine != nil and statusLine\match("^HTTP/") != nil

    deadline = socket.gettime! + @timeout
    while socket.gettime! < deadline
      return @ if isServing!
      socket.sleep 0.05
    error "mock HTTP server didn't answer on port #{port} within #{@timeout}s"

  ---Starts the server and blocks until it writes its ephemeral port to a ready-file or the timeout expires.
  ---@private
  ---@return MockHttpServerController self
  __startViaControlFiles: =>
    controlDir = assert fileOps.createTempDir!
    readyFile = fileOps.joinPath controlDir, "ready"
    @quitFile = fileOps.joinPath controlDir, "quit"
    @__spawn { "--port", "0", "--ready-file", quote(readyFile), "--quit-file", quote(@quitFile) }

    deadline = Timer.getTime! + @timeout * 1000
    while Timer.getTime! < deadline
      data = fileOps.readFile readyFile
      if port = data and tonumber data
        @port, @baseUrl = port, "http://127.0.0.1:#{port}"
        return @
      Timer.sleep 50
    error "mock HTTP server didn't report a port within #{@timeout}s"

  ---Signals the server to stop via a TCP connection to its loopback port.
  ---@private
  __quitViaSocket: =>
    pcall ->
      socket = require "socket"
      conn = socket.tcp!
      conn\settimeout 2
      if conn\connect "127.0.0.1", @port
        conn\send "GET /__quit HTTP/1.0\r\nHost: 127.0.0.1\r\n\r\n"
        conn\receive "*a" -- wait for the response / server exit
      conn\close!

return MockHttpServerController
