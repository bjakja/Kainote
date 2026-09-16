# Vendored forks and their patches

Two libraries stay in the tree because Kainote genuinely modifies them. For each
one, the upstream base is recorded here and the local delta is extracted into
`Thirdparty/patches/<name>/`, so the changes are reviewable on their own and the
fork can be rebased later.

The patches are documentation of what is already in the tree — **the build does
not apply them.** The source under `Thirdparty/<name>/` is the real thing; the
patch series exists so you can see the delta, re-derive it, or replay it onto a
newer upstream.

## ffms2

- **Upstream:** <https://github.com/FFMS/ffms2>
- **Base:** tag `5.0`, plus upstream commit `9417465` ("Add layered decoding
  support") cherry-picked from master.
- **Local delta:** 61 lines across 3 files.

| Patch | What it does |
|---|---|
| `0001-kainote-metadata-and-attachment-api.patch` | Adds the track name/language, chapter, attachment and subtitle-demux API, plus an `FFMS_ColorSpaces` enum. Bumps `FFMS_VERSION` to 5.1.0.0 so a patched ffms2 is distinguishable from stock 5.0. |
| `0002-skip-seek-during-init-in-linear-no-rewind-mode.patch` | With `SeekMode < 0` the source must not seek, but init still ran `INITIALIZE_SOURCE`, which does. |

The *definitions* for the API added by patch 1 are not in the upstream tree at
all — they live in `Thirdparty/Build/FFMS2/indexing_additional.cpp`, which both
builds compile.

> **Known hazard.** On Linux, `indexing_additional.cpp` defines out-of-line
> members of `FFMS_Indexer` against the vendored private header
> `Thirdparty/ffms2/src/core/indexing.h`, then links against the distribution's
> `libffms2`. That is only correct while the distribution ships an ffms2 whose
> `FFMS_Indexer` layout matches this header. It matches ffms2 5.0, and nothing
> currently checks. `CMakeLists.txt` should pin a version range for `ffms2`, or
> the API should be upstreamed so the private header is not needed.

### Rebasing onto a newer ffms2

```sh
git clone https://github.com/FFMS/ffms2 && cd ffms2
git checkout <new-tag>
git apply ../Kainote/Thirdparty/patches/ffms2/*.patch
```

Then copy the result over `Thirdparty/ffms2/` and regenerate the series.

## xy-VSFilter

- **Upstream:** <https://github.com/Cyberbeing/xy-VSFilter>, branch
  `xy_sub_filter_rc5`
- **Base:** commit `591a14c` (2018-09-04, "Kill some warnings")
- **Local delta:** 3447 lines across 65 files.

| Patch | What it does |
|---|---|
| `0001-kainote-vsfilter-source-changes.patch` | CSRI entry points extended with extra colour formats and a direct `SimpleSubPicProvider` path, plus the subtitle/subpic fixes made since 2018. |
| `0002-kainote-vsfilter-msvc-integration.patch` | Project and property files retargeted to build inside `Kainote.sln`. |

The base was identified by diffing the vendored tree against every commit
reachable on that branch and taking the closest match, so it is the best
available estimate rather than a recorded fact. Upstream has been dormant since
February 2023, and the branch has roughly 700 lines of drift past this base that
the fork never picked up.

### Follow-ups

- The directory is still named `xy-VSFilter-xy_sub_filter_rc5`, which is just
  how GitHub's archive happened to unpack. Renaming it to `xy-VSFilter` means
  touching `Kainote.sln` and every project that references it, so it is left for
  a separate change.

## Regenerating a series

Both series were produced by diffing a clean upstream checkout against the
vendored tree with CR stripped from both sides:

```sh
diff -u --label a/<path> --label b/<path> \
    <(tr -d '\r' < upstream/<path>) <(tr -d '\r' < Thirdparty/<name>/<path>)
```

Both were verified by applying them to a fresh checkout of the recorded base and
confirming the result matches the vendored tree byte-for-byte once line endings
are normalised.

## wxWidgets

`Thirdparty/wxWidgets` is a modified 2.9.4 and is **not** covered by a patch
series here, because its delta is not a set of intentional patches: a
tree-wide find-and-replace commented out 4152 `wxCHECK`/`wxASSERT`/`wxFAIL`
lines across 658 files, including the macro definitions in
`include/wx/debug.h`. `wxCHECK(cond, rc)` there now expands to nothing while 59
call sites still use it, 42 of them in code the MSW build compiles, so those
guards no longer return. Untangling that is its own piece of work — either move
to wxWidgets 3.3.x (which the Linux build already uses) or restore `debug.h`
and set `wxDEBUG_LEVEL=0`, which is the supported way to silence assertions
while keeping the `wxCHECK_*` guards.
