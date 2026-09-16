# Kainote's changes to the automation library

The automation library in Kainote's package is not stock upstream. Comparing the
released `Kainote_x64\Automation\` against the pinned upstream revisions in
`../scripts/runtime-assets.json` leaves 30 files different, and the differences
are not version drift alone: several are fixes upstream does not carry. The
patches here are those differences, generated as `upstream -> released file`
unified diffs.

`../scripts/package.py` fetches the pinned upstream files and applies these
patches; a hunk that no longer matches logs the file instead of dropping the fix
quietly. Two tiers:

* **fixes** — small, targeted changes. Examples: `aegisub/lfs.moon` wraps each
  `err_arg_to_multiple_return` implementation explicitly instead of looping over
  `pairs` (the loop does not survive on LuaJIT), `aegisub/re.moon` frees the
  match result buffer with `ffi.C.free`, `lyger/LibLyger.moon` declares
  `has_script_info` local before use, and the `a-mo/*.moon` modules carry the
  version numbers that DependencyControl's build substitutes into
  `##__X_VERSION__##` placeholders.
* **vendor** — the maintainer ships a different build or version of the file
  rather than a patch, so the diff is effectively a whole-file replacement:
  `moonscript.lua` (a bundled build whose `package.path` handling knows about
  `.moon` files) and the `l0/DependencyControl*` library. Pass
  `--skip-vendor-patches` to `package.py` to keep the newest upstream versions of
  these instead.

Files that are older than upstream but carry no local change (for example the
1.301 cleantags, the pre-`argcheck` `aegisub/util.moon`) are patched too, because
that is what Kainote's package contains; drop the individual patch file if you
would rather follow upstream.
