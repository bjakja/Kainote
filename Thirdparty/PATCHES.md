# The two forks

Two dependencies carry Kainote's own changes. Rather than sitting as
unattributed copies in this tree, each is a fork repository whose `kainote`
branch is *upstream at a known commit, plus commits that are the changes*. The
submodule pins one of those commits.

That makes the delta reviewable with ordinary git, and makes rebasing onto a
newer upstream an ordinary rebase.

## ffms2 — [altqx/ffms2](https://github.com/altqx/ffms2), branch `kainote`

- **Upstream:** <https://github.com/FFMS/ffms2>
- **Base:** tag `5.0`, plus upstream commit `9417465` ("Add layered decoding
  support") cherry-picked from master
- **Kainote's own delta:** 61 lines across 3 files, in one commit

| Commit | What |
|---|---|
| `Add layered decoding support` | Upstream cherry-pick, not ours |
| `Add the Kainote metadata/attachment API and linear no-rewind init fix` | The track name/language, chapter, attachment and subtitle-demux API, exported from `ffms.h`, plus an `FFMS_ColorSpaces` enum. `FFMS_VERSION` becomes 5.1.0.0 so a patched ffms2 is distinguishable from stock 5.0. Also skips the `INITIALIZE_SOURCE` stage when `SeekMode < 0`, since that stage seeks and linear no-rewind mode must not. |

The *definitions* for that API are not in the fork at all — they live in this
repository, in `Thirdparty/Build/FFMS2/indexing_additional.cpp`, which both
builds compile.

> **Known hazard.** On Linux, `indexing_additional.cpp` defines out-of-line
> members of `FFMS_Indexer` against the submodule's private header
> `Thirdparty/ffms2/src/core/indexing.h`, then links against the *distribution's*
> `libffms2`. That is only correct while the distribution ships an ffms2 whose
> `FFMS_Indexer` layout matches this header. It matches ffms2 5.0, and nothing
> currently checks. `CMakeLists.txt` should pin a version range for `ffms2`, or
> the API should be upstreamed so the private header is not needed.

## xy-VSFilter — [altqx/xy-VSFilter](https://github.com/altqx/xy-VSFilter), branch `kainote`

- **Upstream:** <https://github.com/Cyberbeing/xy-VSFilter>, branch
  `xy_sub_filter_rc5`
- **Base:** commit `532b756` (2023-02-20, the branch tip; upstream has been
  dormant since)
- **Kainote's delta:** 58 modified files (4248 lines) and 12 added, plus the
  removal of components Kainote never builds

| Commit | What |
|---|---|
| `Remove upstream components Kainote does not build` | Kainote builds seven of this repo's projects; the bundled gtest and boost_lib, the test fixtures, unrar, log4cplus's tests and the DX7 subpic backend are never compiled. Boost comes from Kainote's own hydrated copy. |
| `Kainote's source changes` | CSRI entry points extended with extra colour formats and a direct `SimpleSubPicProvider` path, the subtitle/subpic fixes accumulated since, and the AviSynth interface headers under `include/avisynth/`. |
| `Build inside Kainote.sln` | Project and property files retargeted: toolset, output paths, include paths pointing at Kainote's hydrated boost. |

The base was identified by comparing the vendored tree against every commit on
that branch and taking the one that maximised exact file matches — 940 of 998.

### Follow-up

The submodule path is still `xy-VSFilter-xy_sub_filter_rc5`, which is just how
GitHub's archive happened to unpack. Renaming it means touching `Kainote.sln`
and every project that references it, so it is left for a separate change.

## Rebasing a fork onto newer upstream

```sh
cd Thirdparty/ffms2
git remote add upstream https://github.com/FFMS/ffms2.git
git fetch upstream --tags
git rebase --onto <new-tag> <old-base> kainote
# resolve, then:
git push --force-with-lease origin kainote
cd ../.. && git add Thirdparty/ffms2 && git commit   # move the submodule pin
```

To see just the delta at any time:

```sh
git -C Thirdparty/ffms2 log --oneline 5.0..kainote
git -C Thirdparty/ffms2 diff 5.0..kainote
```

## wxWidgets

No longer a fork. `Thirdparty/wxWidgets` used to be a modified wxWidgets 2.9.4 —
a 2012 development snapshot of a series upstream never stabilised — whose delta
was not a set of intentional patches: a tree-wide find-and-replace had commented
out 4152 `wxCHECK`/`wxASSERT`/`wxFAIL` lines across 658 files, including the
macro definitions in `include/wx/debug.h`, where `wxCHECK(cond, rc)` expanded to
nothing while 59 call sites still used it.

It is now a plain submodule at `v3.3.3` with no local changes, built by
wxWidgets' own `wx_vc17.sln`. Those guards are upstream's again, so there is
nothing here to patch.
