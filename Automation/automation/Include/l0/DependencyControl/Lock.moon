constants = require "l0.DependencyControl.Constants"
NamedSemaphore = require "l0.DependencyControl.NamedSemaphore"
FileLock = require "l0.DependencyControl.FileLock"
Timer = require "l0.DependencyControl.Timer"
Logger = require "l0.DependencyControl.Logger"
utils = require "l0.DependencyControl.utils"
Enum = require "l0.DependencyControl.Enum"
Hash = require "l0.DependencyControl.hash"
fileOps = require "l0.DependencyControl.file-ops"
Accessors = require "l0.DependencyControl.Accessors"
Finalizer = require "l0.DependencyControl.Finalizer"
json = require "json"

defaultLogger = Logger fileBaseName: "#{constants.DEPCTRL_SHORT_NAME}.Lock"

DEFAULT_LOCK_WAIT_INTERVAL = 250
DEFAULT_EXPIRY_DURATION = 5 * 60
DEFAULT_HOLDER_NAME = "unknown"

-- default lower bound on remaining lease before renew refreshes, so a renewal still lands
-- ahead of expiry despite system latency/hangs.
RENEW_SAFETY_MARGIN_MS = 2000

-- separates namespace from resource when hashing them into a single name token
NAMESPACE_RESOURCE_SEPARATOR = "\31"

---@class LockArgs
---@field namespace? string Logical namespace component of the locked resource (default "").
---@field resource? string Resource component within the namespace (default "").
---@field holderName? string Human-readable holder name recorded for diagnostics (default "unknown").
---@field logger? Logger
---@field expiresAfter? number Lease duration in seconds before a holder is considered stale (default 300).
---@field scope? LockScope Scope selecting the primitive and reach of exclusion (default "process").
---@field recordHolder? boolean Write a holder side file while held (default true).
---@field overrideExpiry? boolean Judge foreign holders against this instance's expiresAfter rather than their recorded lease (default false).

---@class GuardArgs: LockArgs
---@field timeout? number Acquire timeout in milliseconds (default math.huge).
---@field lockWaitInterval? number Poll interval in milliseconds while waiting (default 250).

msgs = {
  new: {
    lockNotReleased: "Lock holder '%s' (%s) did not release its lock on resource '%s.%s' before discarding it, cleaning up..."
  }
  lock: {
    trying: "Trying to get a lock on resource '%s.%s' for holder '%s' (%s); timeout: %s..."
    failed: "Could not attain lock on resource '%s.%s' for holder '%s' (%s): %s"
    heldByOther: "Lock on resource '%s.%s' is currently held by %s, retrying in %ims..."
    staleHolder: "Lock on resource '%s.%s' is held by %s whose lease lapsed %ds ago; the holder may have crashed or stalled without releasing it."
    alreadyHeld: "'%s' (%s) is already holding the lock on resource '%s.%s'."
    attained: "'%s' (%s) attained the lock on resource '%s.%s'."
    timeout: "Gave up trying to attain a lock on resource '%s.%s' for holder '%s' (%s) after timeout was reached."
    unavailable: "OS lock unavailable for resource '%s.%s'; '%s' (%s) is proceeding with a process-local lock only (no cross-process exclusion)."
  }
  release: {
    failed: "Could not release lock on resource '%s.%s' for '%s' (%s): %s"
    notHeld: "lock is not currently held by this instance"
    released: "'%s' (%s) released its lock on resource '%s.%s'."
  }
  renew: {
    notHeld: "cannot renew a lock that is not currently held by this instance"
  }
  guard: {
    notAcquired: "Could not acquire lock on resource '%s.%s' for holder '%s' (%s): lock state %s."
  }
}

---Cooperative, named lock with per-resource granularity. Each distinct
---(scope, namespace, resource) maps to its own OS lock, so unrelated resources lock
---independently. A lock is mutually exclusive across every Lock instance -- and, for
---Global scope, across every process -- that targets the same tuple.
---
---Scope (see Lock.Scope) selects the primitive and reach of exclusion:
---  Process: a named semaphore whose name embeds the pid; only Lua states within this
---           process contend.
---  Global:  an OS advisory file lock (FileLock) shared by every process in the session --
---           use for resources shared between Aegisub instances (e.g. a config file). The
---           kernel releases it if the holder crashes, so it never stays stuck; it cannot,
---           however, be taken from a holder that is alive but hung.
---
---While held, the holder's identity and lease are recorded in a per-resource side file for
---diagnostics. Long operations should call renew! periodically to extend the recorded lease
---so waiters don't mistake a busy holder for a crashed one.
---@class Lock
---@field state LockState Held when this instance holds the lock, otherwise Unknown (a foreign holder's state can't be told without acquiring). Read-only.
class Lock

  ---@alias LockState
  ---| -1 # Unknown: the state can't be determined without trying to acquire (e.g. a foreign holder)
  ---| 0 # Unavailable: the lock is held elsewhere and couldn't be acquired
  ---| 1 # Available: the lock is free to acquire
  ---| 2 # Held: this instance holds the lock
  @LockState = Enum "LockState", {
    Unknown: -1
    Unavailable: 0
    Available: 1
    Held: 2
  }, defaultLogger
  LockState or= @LockState

  ---@alias LockScope
  ---| "process" # Process: only Lua states within this process contend
  ---| "global" # Global: every process in the session contends via an advisory file lock
  @Scope = Enum "LockScope", {
    Process: "process"
    Global: "global"
  }, defaultLogger
  Scope = @Scope

  ---Builds the OS lock primitive backing a lock: a named semaphore for Process scope, an
  ---advisory file lock for Global scope.
  ---@param scope LockScope
  ---@param token string OS-safe semaphore name token (Process scope).
  ---@param lockFile string Full path to the lock file (Global scope).
  ---@return table? primitive A primitive exposing isOpen, tryLock and unlock, or nil on invalid scope.
  ---@return string? err
  ---@private
  @__createPrimitive = (scope, token, lockFile) =>
    scopeIsValid, errMsg = Scope\validate scope, "scope"
    return nil, errMsg unless scopeIsValid

    if scope == Scope.Global
      FileLock lockFile
    else
      -- Process-scoped names embed our pid, so unlink them on close: a future process
      -- that reuses this pid must not inherit a stuck name.
      NamedSemaphore token, true

  -- Derives the OS-safe semaphore name token, holder-file path and Global lock-file path
  -- for a tuple.
  deriveNames = (scope, namespace, resource) ->
    hash = Hash.getDigest Hash.HashType.Sha1, "#{namespace}#{NAMESPACE_RESOURCE_SEPARATOR}#{resource}"
    token = scope == Scope.Global and "#{constants.DEPCTRL_SHORT_NAME}_global_#{hash}" or "#{constants.DEPCTRL_SHORT_NAME}_p#{NamedSemaphore.pid}_#{hash}"
    holderFilePath = aegisub.decode_path "?temp/depctrl_lock_#{token}.json"
    lockFilePath = aegisub.decode_path "?temp/depctrl_lock_#{token}.lock"
    return token, holderFilePath, lockFilePath

  ---Creates a lock for the given resource.
  ---@param args LockArgs
  new: (args) =>
    {namespace: @namespace, resource: @resource, holderName: @holderName, logger: @logger,
      expiresAfter: @expiresAfter, scope: @scope, recordHolder: @recordHolder,
      overrideExpiry: @overrideExpiry} = args

    @scope or= Scope.Process
    assert Scope\validate @scope, 'scope'

    @logger or= defaultLogger
    @expiresAfter or= DEFAULT_EXPIRY_DURATION
    @holderName or= DEFAULT_HOLDER_NAME
    @namespace or= ""
    @resource or= ""
    @recordHolder = true if @recordHolder == nil
    @instanceId = utils.uuid!

    token, holderFilePath, lockFilePath = deriveNames @scope, @namespace, @resource
    @_holderFilePath = holderFilePath
    @_primitive = @@__createPrimitive @scope, token, lockFilePath

    -- mutable held-state shared with the GC finalizer (avoids capturing self)
    state = {held: false}
    @_state = state

    -- release any still-held lock when this object is garbage collected.
    holderName, instanceId, namespace, resource, logger = @holderName, @instanceId, @namespace, @resource, @logger
    primitive, recordHolder = @_primitive, @recordHolder
    Finalizer.guard @, ->
      return unless state.held
      pcall logger.warn, logger, msgs.new.lockNotReleased, holderName, instanceId, namespace, resource
      pcall ->
        primitive\unlock!
        fileOps.remove holderFilePath if recordHolder
        state.held = false

  ---Reads the holder record written by the current lock holder, or nil if none is
  ---present/parseable. Read lock-free, so the record may be stale or briefly absent.
  ---@return table? record
  ---@private
  __readHolder: =>
    return nil unless @recordHolder
    data = fileOps.readFile @_holderFilePath
    return nil unless data
    ok, record = pcall json.decode, data
    return ok and type(record) == "table" and record or nil

  -- Records this instance as the current holder in the side file, stamping the lease
  -- (expiresAt = now + expiresAfter) so waiters honor the holder's own expiry. Keeps the
  -- original @acquiredAt across renewals. Also tracks the lease end on the monotonic clock
  -- for this instance's own renew decisions. os.time is shared across processes but coarse
  -- and can jump, whereas the monotonic clock is local but fine-grained and steady. No-op
  -- when holder recording is disabled, and best-effort so failures don't affect the lock.
  ---@private
  __writeHolder: =>
    return unless @recordHolder
    @expiresAt = os.time! + @expiresAfter
    @_leaseExpiresMono = Timer.getTime! + @expiresAfter * 1000
    record = {
      holderName: @holderName, instanceId: @instanceId, pid: NamedSemaphore.pid
      scope: @scope, namespace: @namespace, resource: @resource
      acquiredAt: @acquiredAt, expiresAt: @expiresAt
    }
    ok, data = pcall json.encode, record
    fileOps.writeFile @_holderFilePath, data, true if ok

  -- Removes the holder side file. No-op when holder recording is disabled.
  ---@private
  __clearHolder: =>
    fileOps.remove @_holderFilePath if @recordHolder

  -- Human-readable description of the current foreign holder for log messages, e.g.
  -- "'ConfigHandler' (pid 1234)" or "another instance" when no record is available.
  ---@private
  ---@param record table? A holder record, or nil when none is available.
  ---@return string description Human-readable holder description; "another instance" when the record is nil.
  __describeHolder: (record) =>
    return "another instance" unless record
    "'#{record.holderName or DEFAULT_HOLDER_NAME}' (pid #{record.pid or "?"})"

  -- Timestamp at which a foreign holder's lease lapses, or nil if it can't be determined.
  -- Honors the holder's recorded expiresAt unless overrideExpiry is set, in which case
  -- this instance's expiresAfter is applied to the holder's acquiredAt instead.
  ---@private
  ---@param record table? A holder record, or nil.
  ---@return number? deadline Unix timestamp when the holder's lease lapses, or nil when the record is nil or carries no usable timestamp.
  __holderDeadline: (record) =>
    return nil unless record
    if @overrideExpiry and record.acquiredAt
      return record.acquiredAt + @expiresAfter
    return record.expiresAt or (record.acquiredAt and record.acquiredAt + @expiresAfter)

  ---Returns the holder currently believed to hold this lock, or nil if it appears free.
  ---Reads the side file lock-free, so the result is advisory: a holder whose lease has
  ---lapsed (likely crashed) is reported as free, and a brand-new holder may not be visible
  ---yet. Reports this instance too when it holds the lock. Requires holder recording.
  ---@return table? record The holder record (holderName, pid, namespace, resource, ...).
  getActiveHolder: =>
    record = @__readHolder!
    return nil unless record
    deadline = @__holderDeadline record
    return nil if deadline and os.time! > deadline
    return record

  state: Accessors.property
    get: =>
      return @@LockState.Held if @_state.held
      @@LockState.Unknown

  ---Attempts to acquire the lock, waiting up to timeout milliseconds.
  ---@param timeout? number Maximum time to wait in milliseconds (default math.huge).
  ---@param lockWaitInterval? number Poll interval in milliseconds while waiting (default 250).
  ---@return LockState state Held on success, Unavailable on timeout.
  ---@return number timePassed Milliseconds spent waiting.
  lock: (timeout = math.huge, lockWaitInterval = DEFAULT_LOCK_WAIT_INTERVAL) =>
    -- Without a working OS primitive we can't coordinate across states/processes;
    -- degrade to a process-local grant so DepCtrl keeps functioning, and warn once.
    unless @_primitive.isOpen
      unless @_state.held
        @logger\warn msgs.lock.unavailable, @namespace, @resource, @holderName, @instanceId
        @_state.held = true
        @acquiredAt = os.time!
        @__writeHolder!
      return @@LockState.Held, 0

    -- traced once per acquisition: repeating it on every poll drowns the log during a long wait
    @logger\trace msgs.lock.trying, @namespace, @resource, @holderName, @instanceId,
      timeout == math.huge and "none" or "#{math.floor timeout}ms"

    timePassed = 0
    staleHolderWarned = nil
    while timeout == math.huge or timeout >= timePassed
      state = @state
      switch state
        when @@LockState.Held
          @logger\trace msgs.lock.alreadyHeld, @holderName, @instanceId, @namespace, @resource
          return @@LockState.Held, timePassed

        else -- Unknown: attempt to acquire
          if @_primitive\tryLock!
            @_state.held = true
            @acquiredAt = os.time!
            @__writeHolder!
            @logger\trace msgs.lock.attained, @holderName, @instanceId, @namespace, @resource
            return @@LockState.Held, timePassed

          -- the lock is held by someone else, so surface who holds it and warn — once
          -- per holder — when the holder's lease has lapsed (likely crashed or stalled).
          -- Informational only -- a Global file lock self-heals on crash, and a live
          -- holder's lock is never force-stolen.
          record = @__readHolder!
          holderDesc = @__describeHolder record
          deadline = @__holderDeadline record
          if deadline and os.time! > deadline and holderDesc != staleHolderWarned
            @logger\warn msgs.lock.staleHolder, @namespace, @resource, holderDesc, os.time! - deadline
            staleHolderWarned = holderDesc

          break if timeout == 0
          @logger\trace msgs.lock.heldByOther, @namespace, @resource, holderDesc, lockWaitInterval
          Timer.sleep lockWaitInterval
          timePassed += lockWaitInterval

    @logger\trace msgs.lock.timeout, @namespace, @resource, @holderName, @instanceId
    return @@LockState.Unavailable, timePassed

  ---Attempts to acquire the lock without waiting.
  ---@return LockState state Held if the lock was acquired, Unavailable if it's held elsewhere.
  ---@return number timePassed Milliseconds spent waiting (always 0).
  tryLock: =>
    return @lock 0

  ---Releases the lock held by this instance.
  ---@return boolean? released True on success, nil if the lock wasn't held by this instance.
  ---@return LockState|string statusOrErr Available on success, or an error message when the lock wasn't held.
  release: =>
    unless @_state.held
      return nil, msgs.release.failed\format @namespace, @resource, @holderName, @instanceId, msgs.release.notHeld
    @_primitive\unlock!
    @__clearHolder!
    @_state.held = false
    @acquiredAt = nil
    @_leaseExpiresMono = nil
    @logger\trace msgs.release.released, @holderName, @instanceId, @namespace, @resource
    return true, @@LockState.Available

  ---Refreshes the held lock's lease when it is close to expiring, re-stamping its recorded
  ---expiry to @expiresAfter from now. This only affects the metadata on this Lock instance
  ---and the side file, so waiters don't mistake a busy holder for a crashed one. The underlying
  ---OS lock remains held until explicitly released. To avoid unnecessary writes, the side file
  ---is only updated when the remaining lease is approaching expiry.
  ---@param expiryThreshold? number Renew only if the remaining lease is <= this many milliseconds; -1 forces an unconditional refresh. Defaults to the larger of half the lease or the safety margin, capped at the full lease.
  ---@return boolean? renewed True if refreshed, false if still fresh, nil if the lock isn't held.
  ---@return string? err Set (with nil renewed) only when the lock isn't held.
  renew: (expiryThreshold) =>
    return nil, msgs.renew.notHeld unless @_state.held
    return false unless @recordHolder and @_leaseExpiresMono
    validForMs = @expiresAfter * 1000
    threshold = expiryThreshold or math.min math.max(validForMs / 2, RENEW_SAFETY_MARGIN_MS), validForMs
    unless threshold < 0 -- negative forces a refresh
      remainingMs = @_leaseExpiresMono - Timer.getTime!
      return false if remainingMs > threshold
    @__writeHolder!
    return true

  ---Acquires a lock for the given args and runs the provided body function.
  ---Releases the lock when the body completes or throws. The held Lock is passed to the body
  ---so it can call renew on it if needed.
  ---@param args GuardArgs Lock constructor args plus optional timeout/lockWaitInterval for the acquire.
  ---@param body fun(lock: Lock): ... Called with the held Lock; its return values are passed through.
  ---@return any ... The body's return values on success, or nil + err if the lock couldn't be acquired.
  @guard = (args, body) =>
    lock = @ args
    state = lock\lock args.timeout or math.huge, args.lockWaitInterval or DEFAULT_LOCK_WAIT_INTERVAL
    unless state == @LockState.Held
      return nil, msgs.guard.notAcquired\format lock.namespace, lock.resource, lock.holderName, lock.instanceId, state
    results = table.pack pcall body, lock
    lock\release!
    error results[2], 0 unless results[1]
    return unpack results, 2, results.n

Accessors.install Lock

UnitTestSuite = require "l0.DependencyControl.UnitTestSuite"
return UnitTestSuite\withTestExports Lock, {:defaultLogger}
