Downloader = require "l0.DependencyControl.Downloader"
fileOps = require "l0.DependencyControl.file-ops"
Hash = require "l0.DependencyControl.hash"

msgs = {
  checkMissingArgs: "Required arguments had the wrong type. Expected string, got '%s' and '%s'."
  hashMismatch: "Hash mismatch. Got %s, expected %s."
}

---A download manager replicating the DM.DownloadManager API on top of the
---DependencyControl Downloader engine.
---@class DownloadManager
class DownloadManager
  -- Matches the DM.DownloadManager dependency version declared in DependencyControl.moon
  -- so DependencyControl accepts this implementation without a full managed record.
  @version = "0.3.1"

  ---Creates a download manager.
  ---@param etagCacheDir? string Accepted for API compatibility; ETag caching is not implemented.
  new: (etagCacheDir) =>
    @downloader = Downloader!
    -- the native API exposes .downloads directly; Downloader.clear empties it in
    -- place, so this reference stays valid. .failedDownloads is rebuilt per run.
    @downloads = @downloader.downloads
    @failedDownloads = {}

  ---Queues a download, optionally verifying its SHA-1 once complete.
  ---@param url string
  ---@param outfile string Full output path.
  ---@param sha1? string Expected SHA-1 hash.
  ---@param etag? string Accepted for API compatibility; ignored.
  ---@return Download? download
  ---@return string? err
  addDownload: (url, outfile, sha1, etag) =>
    @downloader\addDownload url, outfile, sha1

  ---Performs all queued downloads (DM.DownloadManager-compatible).
  ---@param callback? fun(progress: number): any Called with 0-100; returning a falsy value cancels remaining downloads. Bridged to the engine's Progress event.
  waitForFinish: (callback) =>
    if callback
      -- bridge the DM-style cancel-capable callback onto the Progress event
      @downloader\await (_, percent) -> @downloader\cancel! unless callback percent
      callback 100 unless @downloader.cancelled
    else
      @downloader\await!
    -- rebuild the native-style failedDownloads list from each download's status
    failed = Downloader.Download.Status.Failed
    @failedDownloads = [dl for dl in *@downloads when dl.status == failed]
    return

  ---@return number progress Current aggregate progress (0-100).
  progress: => @downloader.progress

  ---Cancels all remaining queued downloads.
  cancel: => @downloader\cancel!
  ---Removes all queued downloads and resets state; existing `.downloads` references stay valid.
  clear: => @downloader\clear!

  ---@return boolean connected Whether an internet connection appears to be available.
  isInternetConnected: => @downloader\isInternetConnected!

  ---Computes the SHA-1 of a file's contents.
  ---@param filename string
  ---@return string? hexDigest
  ---@return string? err
  getFileSHA1: (filename) => fileOps.getHash filename, "sha1"

  ---Verifies a file against an expected SHA-1 hash.
  ---@param filename string
  ---@param expected string Expected SHA-1 hex digest.
  ---@return boolean? match
  ---@return string? err
  checkFileSHA1: (filename, expected) => fileOps.verifyHash filename, expected, "sha1"

  ---Verifies a string against an expected SHA-1 hash.
  ---@param str string
  ---@param expected string Expected SHA-1 hex digest.
  ---@return boolean? match
  ---@return string? err
  checkStringSHA1: (str, expected) =>
    return nil, msgs.checkMissingArgs\format type(str), type(expected) unless type(expected) == "string"
    actual, err = Hash.getDigest Hash.HashType.Sha1, str -- Hash.getDigest validates the payload type
    return actual, err unless actual
    return true if actual == expected\lower!
    false, msgs.hashMismatch\format actual, expected

return DownloadManager
