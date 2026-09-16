# Automation 4 library

This is Kainote's copy of the Aegisub automation library, tracked here rather
than fetched at build time: the upstream repositories do not publish several of
these files (the karaoke template helpers, the `json/` library, `effector`,
`karahelper`, Yutils, ASSFoundation's `Common.lua`), and the files they do
publish differ from what Kainote ships.

It was assembled from these revisions, then Kainote's own files were laid over
them; `git log` on this directory is the history from there on:

| Source | Revision |
|---|---|
| `TypesettingTools/Aegisub` (`automation/include`, `automation/autoload`) | `ce97a367c611` |
| `TypesettingTools/DependencyControl` (`modules/l0`, `macros`) | `8cd732eae9cb` |
| `TypesettingTools/ASSFoundation` | `8fbb4f4261ef` |
| `TypesettingTools/Functional` | `5a28339d6d8b` |
| `TypesettingTools/Yutils` | `364f8fa5a9d9` |
| `TypesettingTools/Aegisub-Motion` | `a98331082f80` |
| `TypesettingTools/SubInspector` | `a346959b24ec` |
| `TypesettingTools/lyger-Aegisub-Scripts` (`modules/LibLyger.moon`) | `f07033106376` |
| `torque/ffi-experiments` (`DMv0.4.0-rqffiv0.1.2`: BadMutex, PreciseTimer, DownloadManager, requireffi) | `691b128703ba` |

Files the released Kainote package carries over upstream, and the changes it
makes to the ones upstream has too, came from that package; the noteworthy ones:

- `automation/include/aegisub/lfs.moon` wraps each `err_arg_to_multiple_return`
  implementation individually, because the `pairs` loop upstream uses does not
  survive on LuaJIT.
- `automation/include/aegisub/re.moon` frees the match result buffer with
  `ffi.C.free`.
- `automation/include/lyger/LibLyger.moon` declares `has_script_info` local
  before use.
- The `a-mo/*.moon` modules carry the version numbers DependencyControl
  substitutes into `##__X_VERSION__##` placeholders.
- `BM/`, `DM/`, `PT/` are Kainote's DependencyControl modules; their `.lua` is
  what the released package ships and their DLLs are built from this repository
  (`Thirdparty/DependencyControl/`) and installed into those directories by
  `.github/scripts/package.py`.

The modules we build ourselves (`BM/BadMutex.dll`, `PT/PreciseTimer.dll`,
`DM/DownloadManager.dll`) are not tracked here; packaging takes them from the
build output. `SubInspector.dll` and `packages/karahelper.dll` are, because this
solution does not build them.

## Refreshing from upstream

Bump a revision, copy the files in, and commit: there is no build-time fetching
or patching to keep in step. Dictionaries and the VSFiltermod renderer are still
fetched at build time, pinned in `.github/scripts/runtime-assets.json`.
