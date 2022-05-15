#### FFI Experiments

C++ + luajit FFI = horrible cross-platform messes.

#### BadMutex

A global mutex.

#### PreciseTimer

Measure times down to the nanosecond. Except not really.

#### DownloadManager

Download things with libcurl without blocking Lua.

#### requireffi

Load C libraries using `package.path`.

#### Building

If you are on an operating system not managed by a hulking corporate
abomination (if you are, [download a release][binary]), type `make` in
the root of the source. This assumes you have, at the bare minimum:

- Some kind of make program (may need to be GNU Make compatible).
- A working c99 compiler.
- A working c++11 compiler.
- libcurl installed in some place your linker will think to look for it.
- Hopefully all the other things which I've forgotten.

If you want to build a single library, the makefile has targets
`BadMutex`, `DownloadManager`, and `PreciseTimer`, which, shockingly,
correspond to exactly what you might expect (capitalization is
important).

The companion lua scripts are not built by default and must be built
separately with `make lua`. This is because they aren't platform
dependent, so you can just [download the release versions][binary] like
everyone else. Also, in order to build them, you must have:

- bash or some bash-compatible shell.
- `moonc`, the moonscript compiler.
- Some variant of perl 5.
- The C preprocessor, `cpp`.

in addition to some of the earlier requirements.

All of the lua scripts must be built simultaneously, for reasons
completely unrelated to my indolence.

[binary]: ../../releases/tag/r3
