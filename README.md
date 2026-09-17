| Download | Help Us Translate! | Join Discord Server |
| :---: | :---: | :---: |
| [![release](https://img.shields.io/github/v/release/bjakja/Kainote.svg?maxAge=3600&label=download)](https://github.com/bjakja/Kainote/releases) | [![Translation status](https://hosted.weblate.org/widgets/kainote/-/svg-badge.svg)](https://hosted.weblate.org/engage/kainote/?utm_source=widget) | [![Discord](https://img.shields.io/discord/961361569269293077.svg?label=discord&labelColor=7289da&color=2c2f33&style=flat)](https://discord.gg/9WacFTtK6q) |

# Kainote

Kainote is a powerful subtitle editor designed for a wide range of tasks. It utilizes **FFMS2** for high-precision work like typesetting, timing, and advanced editing, and **DirectShow** for general video playback and minor subtitle adjustments.

## Features

* **Comprehensive Format Support**: Natively handles ASS, SRT, MPL2, MDVD, and TMP formats. SSA files are automatically converted to ASS upon loading.
* **Versatile Format Conversion**: Easily convert subtitles between any of the supported formats.
* **Translation Mode**: A dedicated mode that displays the original text alongside the translation field, streamlining the localization process.
* **Efficient Navigation**: Quickly seek to lines that have not yet been translated or committed to final.
* **Bulk Tagging**: Apply ASS tags to multiple selected lines simultaneously.
* **Visual Tools**: Visually adjust tags like `\pos`, `\move`, `\org`, `\clip`, `\iclip`, and vector drawings (`\p`) directly on the video frame.
* **Precision Zoom**: Zoom in on the video, even in fullscreen mode, to create highly accurate vector clips and drawings.
* **Integrated Time Shifting**: Adjust subtitle timing directly within the main grid and sync changes with the current audio/video position.
* **Advanced Audio Tools**: Visualize audio as a spectrum or waveform display. Includes an auto-splitting tool perfect for timing karaoke lyrics.
* **Automation 4 Support**: Supported Automation 4 scripts with [DependencyControl](https://github.com/TypesettingTools/DependencyControl).
* **Subtitle Comparison**: Compare two different subtitle files side-by-side in separate tabs.
* **Advanced Subtitle Filtering**: Filter the subtitle view to hide unnecessary lines and focus on your work.

## Beta Builds

You can download the latest beta version of Kainote from the link below.

[**Download Kainote Beta**](https://github.com/bjakja/Kainote/actions/workflows/build.yml?query=branch%3Amaster)

The Automation 4 library and the themes are tracked in this repository; everything else the package needs is either built here or fetched from pinned sources at build time. Builds are produced by CI on every push to `master`. Open the newest run and download the package for your platform — **`kainote-windows-x64`** (a zip holding `Kainote_x64\` with the executables, the Automation 4 library, dictionaries, translations, the CSRI renderers and the runtimes Windows does not always have) or **`kainote-linux-x86_64`** (the same layout, with the `kainote` binary in place of the Windows executables). GitHub asks you to sign in before it hands over an artifact, and keeps artifacts for 90 days.

**Please Note**: Beta builds are unstable and intended for testing purposes. Features may be incomplete or contain bugs. If you encounter issues or have feedback, please join our Discord server.

## Contributing

### Translations

Want to see Kainote in your native language? You can help us by contributing translations on Weblate, a user-friendly platform for localization.

[Help Translate Kainote on Weblate](https://hosted.weblate.org/engage/kainote/?utm_source=widget)

[![Translation status](https://hosted.weblate.org/widget/kainote/287x66-grey.png)](https://hosted.weblate.org/engage/kainote/)

### Support & Community

For questions, help, or to join the community, find us on Discord!

[**Join the Kainote Discord Server**](https://discord.gg/8kNAxDFgwj)

## Building from Source

Kainote currently has two supported source-build paths:

- **Windows**: the upstream Visual Studio solution (`Kainote.sln`). This is the full-featured build that uses DirectShow, DirectSound, Direct3D 9/D3DX9, and the Windows COM/Shell APIs.
- **Linux**: This build uses wxGTK and system packages where possible. Some Windows-only runtime backends are still compatibility layers or partial ports, but the project can be configured, compiled, linked, and smoke-tested on Linux.

Both start from a recursive clone -- most third-party code is a git submodule,
and wxWidgets keeps its own dependencies in nested submodules:

```bash
git clone --recurse-submodules https://github.com/bjakja/Kainote.git
cd Kainote
```

---

### Windows build

```powershell
pwsh -File Thirdparty\bootstrap.ps1
msbuild Kainote.sln /m /p:Configuration=Release /p:Platform=x64
```

`Kainote.exe` is written to `x64\Release`.

If the clone was not recursive, run `git submodule update --init --recursive`
first -- `bootstrap.ps1` does this too, but a non-recursive checkout otherwise
fails inside the wxWidgets build on empty directories.

#### Required tools

| Tool | Why |
|---|---|
| **Visual Studio 2022** | Desktop development with C++, MSVC v143, Windows 10/11 SDK |
| **NASM** | Assembly in libass and LuaJIT. Must be on `PATH` (`nasm -v`) |
| **Git** | Submodules, and the commit recorded in the title bar |
| **DirectX SDK (June 2010)** | D3DX9. Expected at `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)` |

Optionally **gettext** for `msgfmt` and **Python 3**, which together compile
`Locale\*.po` into the `Locale\<lang>\LC_MESSAGES\kainote.mo` files the program
loads. Without either the step prints a notice and is skipped, and the build
has no translations.

#### What bootstrap.ps1 does

1. `git submodule update --init --recursive`
2. `hydrate.ps1` — downloads the dependencies that are archives rather than
   submodules (FFmpeg, Boost, ICU, fribidi, zlib, curl), verifying each
   archive's SHA-256 against [`Thirdparty/dependencies.json`](Thirdparty/dependencies.json)
   and refusing to continue on a mismatch
3. `build-wxwidgets.ps1` — builds wxWidgets 3.3.3 using `wx_vc17.sln`, the
   solution wxWidgets itself ships
4. `gen-gitparams.ps1` — writes `Kainote/gitparams.h`

Every step is re-runnable: hydrate skips what is already extracted, and the
wxWidgets build is incremental. Pass `-Force` to re-extract, or
`-SkipSubmodules` / `-SkipWxWidgets` to skip a step.

For a Debug build, wxWidgets needs its Debug libraries too:

```powershell
pwsh -File Thirdparty\bootstrap.ps1 -Configuration Debug
msbuild Kainote.sln /m /p:Configuration=Debug /p:Platform=x64
```

#### Where the dependencies come from

Most are git submodules pinned to a release tag; the rest are hash-pinned
archives. [`Thirdparty/README.md`](Thirdparty/README.md) has the full table and
explains why each one is where it is.

FFmpeg is a prebuilt developer package — headers, MSVC import libraries and
runtime DLLs. Building it from source previously required MSYS2 and a
from-scratch FFmpeg compile; that is no longer part of the build. The DLLs are
copied next to `Kainote.exe` by a post-build step, so they must ship with the
application.

> **32-bit.** `Release|Win32` exists but is not exercised. It needs a 32-bit
> FFmpeg developer package placed at `Thirdparty\ffmpegx32` (same `include\`
> and `lib\` layout); `hydrate.ps1` does not fetch one. x64 is the supported
> target.

#### If the build cannot find something

Check that `bootstrap.ps1` completed — most failures are a step that was
skipped or a submodule that was not checked out. Beyond that, the include and
library directories live in `Kainote\Kainote.vcxproj` under
**C/C++ > General > Additional Include Directories** and
**Linker > General > Additional Library Directories**.

---

### Linux build

The Linux build uses CMake and system packages. It has been verified on an Ubuntu/Debian-style environment with GCC, wxGTK 3.2, LuaJIT 2.1 (Lua 5.1-compatible), FFMS2, FFmpeg, libass, Hunspell, uchardet, libcurl, ICU, Boost, GStreamer 1.x, and OpenGL development packages.

#### 1. Install dependencies on Ubuntu/Debian

```bash
sudo apt update
sudo apt install --no-install-recommends -y \
  build-essential \
  cmake \
  gzip \
  tar \
  git \
  pkg-config \
  libwxgtk3.2-dev \
  libwxgtk-gl3.2-dev \
  libass-dev \
  libffms2-dev \
  libluajit-5.1-dev \
  libhunspell-dev \
  hunspell-en-us \
  libuchardet-dev \
  libcurl4-openssl-dev \
  libicu-dev \
  libboost-filesystem-dev \
  libboost-locale-dev \
  libboost-regex-dev \
  libboost-system-dev \
  libavformat-dev \
  libavcodec-dev \
  libavutil-dev \
  libgl1-mesa-dev \
  libgtk-3-dev \
  libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev \
  gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good \
  gstreamer1.0-pulseaudio \
  gettext
```

GStreamer backs both video and audio playback on Linux, so its runtime plugins
must be present, not just the `-dev` headers. The `-base` plugins provide
`appsrc`, `audioconvert`, `audioresample` and `playbin`; the `-good` plugins
provide `autoaudiosink`; and an audio sink such as `gstreamer1.0-pulseaudio`
(or `gstreamer1.0-pipewire` / `gstreamer1.0-alsa`) is needed to actually output
sound. Without these plugins the audio/video pipeline cannot be created.

Optional but useful for headless smoke tests:

```bash
sudo apt install --no-install-recommends -y xvfb
```

#### 2. Package names on other Linux distributions

The exact package names vary by distribution. Install the equivalent development packages for:

- C and C++ compiler toolchain (`gcc`, `g++`, `make`)
- CMake 3.21 or newer
- GNU tar and gzip (used by the reproducible Linux archive target)
- pkg-config
- wxWidgets/wxGTK 3.x with core, base, adv, aui, html, xml, gl, and stc components
- libass
- FFMS2
- LuaJIT 2.1 development headers and library (the `luajit` pkg-config module)
- Hunspell
- A Hunspell dictionary (the runtime copy step looks for `en_US.aff` and `en_US.dic`)
- GNU gettext (`msgfmt`) for compiling translations
- Python 3, which drives the translation compile step (`tools/compile_catalogs.py`)
- uchardet
- libcurl
- ICU (`icu-uc` and `icu-i18n` pkg-config modules)
- Boost filesystem, locale, regex, and system
- FFmpeg development libraries: libavformat, libavcodec, libavutil
- OpenGL/Mesa development headers
- GTK 3 development headers
- GStreamer 1.x: the core (`gstreamer-1.0`) plus the `-base` libraries
  (`gstreamer-app-1.0`, `gstreamer-audio-1.0`, `gstreamer-video-1.0`), and at
  runtime the base and good plugin sets plus an audio sink (pulse/pipewire/alsa)

For Fedora-like systems, the package set is approximately:

```bash
sudo dnf install \
  gcc gcc-c++ make cmake gzip tar git pkgconf-pkg-config \
  wxGTK-devel wxGTK-gl wxGTK-media \
  libass-devel ffms2-devel luajit-devel hunspell-devel hunspell-en-US uchardet-devel \
  libcurl-devel libicu-devel boost-devel ffmpeg-devel mesa-libGL-devel gtk3-devel \
  gstreamer1-devel gstreamer1-plugins-base-devel \
  gstreamer1-plugins-base gstreamer1-plugins-good gettext
```

For Arch-like systems, the package set is approximately:

```bash
sudo pacman -S --needed \
  base-devel cmake gzip tar git pkgconf wxwidgets-gtk3 libass ffms2 luajit \
  hunspell hunspell-en_us uchardet curl icu boost ffmpeg mesa gtk3 \
  gstreamer gst-plugins-base gst-plugins-good gettext
```

Kainote requires LuaJIT rather than the standard Lua interpreter: its Automation subsystem uses LuaJIT's FFI as well as Lua 5.1 APIs. The CMake configuration therefore checks for the `luajit` pkg-config module.

#### 3. Verify dependency discovery

Before configuring Kainote, confirm that pkg-config can find the required libraries:

```bash
pkg-config --modversion \
  libass \
  ffms2 \
  luajit \
  hunspell \
  uchardet \
  libcurl \
  icu-uc \
  icu-i18n \
  libavformat \
  libavcodec \
  libavutil \
  gstreamer-1.0 \
  gstreamer-video-1.0 \
  gstreamer-audio-1.0 \
  gstreamer-app-1.0
```

Also verify wxWidgets:

```bash
wx-config --version
wx-config --libs core,base,adv,aui,html,xml,gl,stc
```

If any command fails, install the missing `-dev`/`-devel` package or adjust `PKG_CONFIG_PATH` so that pkg-config can locate the corresponding `.pc` file.

#### 4. Configure and build

```bash
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux -j$(nproc)
```

The executable is created at:

```text
build-linux/kainote
```

To create a clean Linux runtime archive after building:

```bash
cmake --build build-linux --target kainote_linux_package
```

The archive and its SHA-256 file are written under `build-linux/dist/`. The
package target uses an explicit allowlist: it includes the executable,
manifest-recorded runtime libraries, freshly compiled translations, bitmap
resources, the project license, and the configured English dictionary when one
is available. It deliberately excludes CMake internals and any Config,
Automation, or Themes files created by local runs. Host graphics/windowing
libraries, codec libraries excluded for licensing reasons, and GStreamer
plugins remain system requirements; this archive is not a universal AppImage.

#### 5. Run Kainote

On a normal desktop session:

```bash
./build-linux/kainote
```

For a headless smoke test, use Xvfb and a timeout:

```bash
timeout 8s xvfb-run -a ./build-linux/kainote
```

Exit code `124` from the command above is expected when `timeout` stops an otherwise running GUI application after 8 seconds.

#### 6. Clean or rebuild

To rebuild incrementally:

```bash
cmake --build build-linux -j$(nproc)
```

To force a clean reconfigure:

```bash
rm -rf build-linux
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux -j$(nproc)
```

#### 7. Current Linux runtime notes

On Linux the build behaves the same as the Windows build except for the following.

Media backends (Windows uses DirectShow / DirectSound / Direct3D):

- General video playback uses **GStreamer** in place of DirectShow: `playbin`
  decodes the file and an `appsink` hands BGRA frames to the app, which
  composites the libass subtitles and progress bar and presents them through the
  shared wxWidgets paint path. The frame-accurate FFMS2 path used for
  typesetting, timing, and visual editing is unchanged.
- Audio plays through **GStreamer** (`appsrc → audioconvert → audioresample →
  autoaudiosink`); the playback position is tracked on a wall-clock model. There
  is no DirectSound path.
- GStreamer discovers plugins from the runtime system registry and standard
  plugin paths. The base and good plugin sets (and an audio sink) must be
  installed on the machine that runs Kainote.

Other Linux differences:

- Move-to-trash uses freedesktop `gio trash`, so the `gio` tool (glib2) must be present at runtime; if it is missing, deleting a loaded video is a no-op instead of an unrecoverable hard delete.
- The file-association `("Skojarzenia")` options tab is not shown. On Linux associations
  are declared by the `.desktop` file rather than set per extension at runtime. Run
  `./install-desktop-integration.sh` from the extracted archive to register the menu
  entry, icons and file types under `~/.local/share`; `--uninstall` removes them. A
  prefix install (`cmake --install`) installs the same data, but not the executable:
  Kainote still locates its resources relative to the binary, so it has to run from its
  own directory.

Under Wayland specifically (these work normally on X11):

- Cursor-anchored dialogs (colour picker, font/style/hotkey dialogs, etc.) open centred instead of at the pointer a Wayland client cannot position its own windows.
- "Fullscreen on a specific monitor" falls back to fullscreen on the current output.
- Auto-pause-on-minimize and the B-key minimize do nothing a Wayland client cannot minimize itself.
- The screen-pixel colour eyedropper is unavailable Wayland forbids reading pixels outside the app's own surface.
