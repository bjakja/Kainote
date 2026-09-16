# Thirdparty

How Kainote gets its third-party code, and why each library is where it is.

The two builds source dependencies very differently:

- **Linux** (`CMakeLists.txt`) resolves everything through `pkg-config` against the
  distribution's packages. It reads nothing in this directory except `luabins`,
  `DependencyControl` and the `ffms2` headers.
- **Windows** (`Kainote.sln`) builds its dependencies from source, using the MSVC
  project files under `Thirdparty/Build/`.

## Hydrated (not in git)

These are unmodified upstream releases, so they are downloaded on demand instead
of being committed. Versions and SHA-256 hashes are pinned in
[`dependencies.json`](dependencies.json).

Before opening `Kainote.sln` for the first time:

```powershell
pwsh -File Thirdparty\hydrate.ps1
```

| Directory    | Version | Notes |
|--------------|---------|-------|
| `freetype2`  | 2.14.3  | |
| `harfbuzz`   | 14.4.0  | Built from the `src/harfbuzz.cc` amalgamation |
| `fribidi`    | 1.0.16  | Release tarball only — it ships the generated `lib/*.tab.i` tables |
| `libass`     | 0.17.5  | |
| `Hunspell`   | 1.7.3   | |
| `boost`      | 1.91.0  | |
| `icu`        | 78.3    | |
| `zlib`       | 1.3.2   | |
| `curl`       | 8.20.0  | |

`hydrate.ps1` verifies each archive's hash before extracting and refuses to
continue on a mismatch. It skips libraries that are already present; pass
`-Force` to re-extract, or `-Only <names>` to work on a subset.

To bump one of these, edit its `version`, `url` and `sha256` in
`dependencies.json` and re-run the script. The MSVC project that compiles it
lives in `Thirdparty/Build/<Name>/` and **is** tracked, because those project
files are Kainote's own work rather than upstream's.

A few of those projects carry configuration that upstream would normally
generate:

- `Build/Fribidi/fribidi-config.h` — stands in for the `configure`/meson output.
  It also defines `HAVE_STDLIB_H`, `HAVE_STRING_H`, `STDC_HEADERS` and
  `HAVE_STRINGIZE`, which fribidi 1.x expects from `config.h`. They live here and
  not in a `config.h` on purpose: `Libass.vcxproj` puts `Build\Fribidi` on its
  include path *ahead of* `Build\libass`, so a `config.h` in this directory would
  shadow libass's own and silently disable `CONFIG_ASM`.
- `Build/Libass/config.h` — libass's `CONFIG_*` switches.
- `Build/Libass/Libass.vcxproj` — passes the nasm flags upstream's meson build
  uses (`-Dprivate_prefix=ass -DPIC=1 -DARCH_X86_64=…`, plus `-DPREFIX` on
  32-bit) and an `-I` pointing at the libass source root so `%include
  "x86/x86inc.asm"` resolves.

## Vendored, carrying Kainote changes

These stay in git because they are not stock upstream. Base revisions and the
extracted patch series are recorded in [`PATCHES.md`](PATCHES.md).

| Directory | Upstream | Why it is still here |
|---|---|---|
| `ffms2` | [FFMS/ffms2](https://github.com/FFMS/ffms2) 5.0 | Adds a Kainote-only API (track names/languages, chapters, attachments, subtitle demuxing, colorspace enum) |
| `xy-VSFilter-xy_sub_filter_rc5` | [Cyberbeing/xy-VSFilter](https://github.com/Cyberbeing/xy-VSFilter) | Hard fork with CSRI extensions and extra colour formats; upstream is dormant |
| `wxWidgets` | 2.9.4 | Locally modified; see the audit notes in `PATCHES.md` |
| `luabins` | [agladysh/luabins](https://github.com/agladysh/luabins) | Three small fixes for modern LuaJIT (`luaL_reg` → `luaL_Reg`, `LUA_LIB`, `LUAI_BITSINT`); upstream is unmaintained |
| `uchardet` | [uchardet](https://gitlab.freedesktop.org/uchardet/uchardet) | Tracks upstream *master*, which is ahead of the 0.0.8 release by seven language models Kainote uses; hydrating the release would lose them |
| `BaseClasses` | DirectShow base classes | Windows SDK sample code, locally patched |
| `DirectX9`, `karahelper`, `DependencyControl`, `LuaJIT`/`luajit` | — | Windows-only support code |

> `Thirdparty/luajit` and `Thirdparty/LuaJIT` are two directories that differ only
> in case. They hold disjoint file sets and merge into one directory on Windows
> and macOS, which is how the build has been finding all of LuaJIT. This is
> fragile and should be collapsed into a single directory.
