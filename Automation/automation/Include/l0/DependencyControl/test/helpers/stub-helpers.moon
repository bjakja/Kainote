FeedTrust = require "l0.DependencyControl.FeedTrust"

---Builds a stub `self` for driving a class's methods in isolation: installs `Cls.__base` as the metatable
---index so method lookups (and internal `@`/`@@` calls) resolve, while `fields` supplies the instance state
---the code under test reads. The caller sets `__class` in `fields` when the code reads it.
---@param cls table The class whose method table (`__base`) the stub inherits.
---@param fields? table The stub's own fields (default empty).
---@return table stub The `fields` table with `cls.__base` installed as its metatable index.
stubSelf = (cls, fields = {}) -> setmetatable fields, cls.__base

---Builds a logger stub whose every method is a no-op, with a live `indent` field the code under test can
---adjust. Its `assert` returns the condition and never raises, so assert-guarded code under test proceeds.
---@return table logger A logger stub: `indent` is 0, `assert` returns its first argument, and any other method call does nothing and returns nil.
makeNullLogger = -> setmetatable {indent: 0, assert: ((cond) => cond)}, __index: -> ->

---Builds a stub FeedTrust `self` seeded with the given trust state and a null logger, for driving FeedTrust
---methods (and code that consults trust) in isolation.
---@param opts? { config?: table, official?: { trusted: table, blocked: table }, feedLoader?: table } The config view, the merged official trusted/blocked sets (stored as the private `__official`), and a feed-loader stub; each optional.
---@return table feedTrust A FeedTrust stub-self carrying the seeded fields.
makeSeededFeedTrust = (opts = {}) ->
  stubSelf FeedTrust, {config: opts.config, __official: opts.official, feedLoader: opts.feedLoader, logger: makeNullLogger!}

{:stubSelf, :makeNullLogger, :makeSeededFeedTrust}
