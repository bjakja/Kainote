# What Kainote's package carries over the upstream automation library

The `Automation/` and `Themes/` folders of Kainote's package are not stock
upstream. `.github/scripts/package.py` fetches the automation library from the
pinned upstream repositories (see `../scripts/runtime-assets.json`) and then
installs the files Kainote ships from here:

* `kainote-runtime-files.tar.gz` — the automation library and the four
  `Themes/*.txt` exactly as the released package has them, with `FILES.sha256`
  as a manifest. Extraction is checksum-verified, so a partial or corrupt bundle
  fails loudly instead of producing half a library. `--prefer-upstream` skips it.
* `bjakja/*.patch` — one unified diff per file against the upstream revision the
  build fetches. These are the readable record of what Kainote changed; the
  bundle above is what gets installed.

Three kinds of difference, marked by `# tier:` in each patch:

* **carried** — the upstream repositories we fetch do not publish the file at
  all: `BezierToText.lua`, `gradient-factory.lua`, `skew gradient.lua`, the
  `json/` library, `effector-auto4.lua`, `bakukara.lua`, `karahelper.lua`,
  Yutils, ASSFoundation's `Common.lua`, DependencyControl's `FileOps.moon`,
  `BP`/`DM`/`PT` scripts and `requireffi`.
* **fix** — a targeted change upstream does not have. `aegisub/lfs.moon` wraps
  each `err_arg_to_multiple_return` implementation individually, because the
  `pairs` loop does not survive on LuaJIT; `aegisub/re.moon` frees the match
  result buffer with `ffi.C.free`; `lyger/LibLyger.moon` declares
  `has_script_info` local before use; the `a-mo/*.moon` modules carry the version
  numbers DependencyControl substitutes into `##__X_VERSION__##` placeholders.
* **vendor** — the maintainer ships a different build or version rather than a
  patch: `moonscript.lua` (a bundle whose `package.path` handling knows about
  `.moon` files) and the `l0/DependencyControl*` library.

## Regenerating

Download the released asset, then rebuild both artefacts from it:

```sh
gh release download 1.0.0.1537 --repo bjakja/Kainote --pattern Kainote.x64.zip --dir /tmp/rel
python3 - <<'PY'
import zipfile, io, tarfile, hashlib
z = zipfile.ZipFile('/tmp/rel/Kainote.x64.zip')
rel = {n.replace('\\', '/')[len('Kainote_x64/'):]: z.read(n) for n in z.namelist()
       if n.startswith('Kainote_x64/') and not n.endswith('/')}
# keep the DLLs this repository builds itself out of the bundle
skip = {f'Automation/automation/Include/{m}'
        for m in ('BM/BadMutex.dll', 'DM/DownloadManager.dll', 'PT/PreciseTimer.dll')}
keep = [k for k in sorted(rel) if k.startswith(('Automation/', 'Themes/')) and k not in skip]
with tarfile.open('.github/patches/kainote-runtime-files.tar.gz', 'w:gz', compresslevel=9) as tar:
    for k in keep:
        info = tarfile.TarInfo(k); info.size = len(rel[k]); info.mtime = 0
        tar.addfile(info, io.BytesIO(rel[k]))
    manifest = '\n'.join(f'{hashlib.sha256(rel[k]).hexdigest()}  {k}' for k in keep) + '\n'
    info = tarfile.TarInfo('FILES.sha256'); info.size = len(manifest); info.mtime = 0
    tar.addfile(info, io.BytesIO(manifest.encode()))
PY
```

The `.patch` files come from diffing each kept file against the revision the
build fetches for that path (or `/dev/null` when nothing is fetched); regenerate
them the same way after bumping a pin in `runtime-assets.json`.
