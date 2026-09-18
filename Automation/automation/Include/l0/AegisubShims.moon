aegisub = require "l0.AegisubShims.aegisub"

-- Re-expose the shim's configuration hooks (see AegisubShims.aegisub) so callers can
-- relocate path tokens without reaching into the faux `aegisub` global.
return {
  :aegisub
  setPathToken: aegisub.__depCtrl.setPathToken
  getPathToken: aegisub.__depCtrl.getPathToken
}
