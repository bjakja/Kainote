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

[**Download Kainote Beta**](https://drive.google.com/uc?id=1ECqsrLo5d1jPoz-FKvJrS0279YeTKrmS&export=download)

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

The commands below assume a fresh clone:

```bash
git clone https://github.com/bjakja/Kainote.git
cd Kainote
```

---

### Windows build

#### 1. Required tools

Install the following tools before opening the solution:

1. **Visual Studio 2022**
   - Install the **Desktop development with C++** workload.
   - Include the **MSVC v143 x64/x86 build tools**.
   - Include the **Windows 10/11 SDK**.
   - The solution is normally built as **Release | x64**.

2. **DirectX SDK (June 2010)**
   - Download from Microsoft: <https://www.microsoft.com/en-us/download/details.aspx?id=6812>
   - The project expects the default install location:
     `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)`
   - If it is installed somewhere else, update the include/library directories in Visual Studio project properties.

3. **NASM**
   - Download from <https://www.nasm.us/>.
   - Add `nasm.exe` to `PATH` during installation, or add it manually afterwards.
   - Verify from a new terminal:

   ```bat
   nasm -v
   ```

4. **Git**
   - Required for cloning dependency source trees.
   - Download from <https://git-scm.com/download/win>.

5. **CMake**
   - Required for some third-party libraries such as AOM and VVENC.
   - Download from <https://cmake.org/download/> and add it to `PATH`.

6. **MSYS2**
   - Required for building FFmpeg with the MSVC toolchain.
   - Download from <https://www.msys2.org/> and install to `C:\msys64`.

#### 2. Third-party source layout

The Visual Studio project expects most third-party source trees under `Thirdparty`.
After downloading/extracting dependencies, the layout should look like this:

```text
Kainote/
  Kainote.sln
  Kainote/
    Kainote.vcxproj
  Thirdparty/
    boost/
    icu/
    wxWidgets/
    ffms2/
    luajit/
    luabins/
    libass/
    Hunspell/
    uchardet/
    BaseClasses/
```

At minimum, download/extract the following libraries if they are not already present in the repository checkout:

- **Boost**: <https://www.boost.org/releases/latest/>
  - Extract or rename the directory to `Thirdparty/boost`.
- **ICU4C source**: <https://github.com/unicode-org/icu/releases/>
  - Download the `icu4c-*-src.zip` archive.
  - Extract or rename the directory to `Thirdparty/icu`.
- **FFMS2**: <https://github.com/FFMS/ffms2>
  - Kainote uses additional FFMS2 API functions; see the FFMS2 patching step below.

> Note: building ICU from source can require a large amount of RAM. On machines with limited memory, configure a large Windows page file before building.

#### 3. Build AOM and VVENC pkg-config files

These optional codec libraries are used by the FFmpeg build configuration below.
They are built with Visual Studio, but their `.pc` files are consumed from MSYS2.

##### AOM / AV1

Open **x64 Native Tools Command Prompt for VS 2022** and run:

```bat
git clone https://aomedia.googlesource.com/aom C:\src\aom
mkdir C:\build\aom
cd /d C:\build\aom
cmake C:\src\aom -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DAOM_TARGET_CPU=generic ^
  -DBUILD_SHARED_LIBS=0 ^
  -DENABLE_DOCS=0 ^
  -DENABLE_TESTS=0 ^
  -DENABLE_TOOLS=0 ^
  -DENABLE_CCACHE=1 ^
  -DCONFIG_AV1_ENCODER=0
cmake --build . --config Release
```

Edit the generated `aom.pc` so that:

- `includedir` points to a directory containing both the AOM source `aom` headers and the generated `config` headers.
- `libdir` points to the Release library output directory.

Copy the edited file to:

```text
C:\msys64\usr\lib\pkgconfig\aom.pc
```

##### VVENC / H.266

Open **x64 Native Tools Command Prompt for VS 2022** and run:

```bat
git clone https://github.com/fraunhoferhhi/vvenc C:\src\vvenc
mkdir C:\build\vvenc
cd /d C:\build\vvenc
cmake C:\src\vvenc -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_SHARED_LIBS=0 ^
  -DVVENC_LIBRARY_ONLY=1
cmake --build . --config Release
```

Edit the generated `libvvenc.pc` so that:

- `includedir` points to the directory containing the `vvenc` headers.
- `libdir` points to the Release library output directory.

Copy the edited file to:

```text
C:\msys64\usr\lib\pkgconfig\libvvenc.pc
```

#### 4. Configure MSYS2 for MSVC builds

1. Edit:

   ```text
   C:\msys64\msys2_shell.cmd
   ```

2. Find this line:

   ```bat
   rem set MSYS2_PATH_TYPE=inherit
   ```

3. Uncomment it:

   ```bat
   set MSYS2_PATH_TYPE=inherit
   ```

4. Open **x64 Native Tools Command Prompt for VS 2022**.

5. Start the MSYS2 shell from inside that prompt:

   ```bat
   C:\msys64\msys2_shell.cmd
   ```

6. In the MSYS2 shell, install the build tools:

   ```bash
   pacman -Syu
   pacman -S --needed make diffutils yasm nasm pkg-config git
   ```

7. Avoid a linker-name conflict with MSYS2's `link.exe`:

   ```bash
   if [ -f /usr/bin/link.exe ]; then mv /usr/bin/link.exe /usr/bin/link.exe.bak; fi
   ```

8. Confirm MSVC tools are visible inside MSYS2:

   ```bash
   which cl
   which link
   cl
   ```

`which link` should resolve to the Visual Studio linker, not `/usr/bin/link.exe`.

#### 5. Build FFmpeg for FFMS2

From the same MSYS2 shell that inherited the Visual Studio environment:

```bash
cd /c
curl -L -o ffmpeg-n7.1.1.zip https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n7.1.1.zip
unzip ffmpeg-n7.1.1.zip
mv FFmpeg-n7.1.1 ffmpeg
cd /c/ffmpeg

./configure \
  --toolchain=msvc \
  --enable-gpl \
  --enable-version3 \
  --disable-programs \
  --disable-doc \
  --disable-avdevice \
  --disable-postproc \
  --disable-avfilter \
  --enable-dxva2 \
  --enable-d3d11va

make -j$(nproc)
make install
```

This installs FFmpeg headers, libraries, and pkg-config files into:

```text
C:\msys64\usr\local
```

Make sure MSYS2 can see the installed packages:

```bash
pkg-config --modversion libavformat libavcodec libavutil
```

#### 6. Patch/update FFMS2 for Kainote

Kainote requires FFMS2 functions that are not part of the stock public API.
If you replace `Thirdparty/ffms2` with a fresh upstream checkout, apply the Kainote additions below.

Add the following declarations near the end of `Thirdparty/ffms2/include/ffms.h`, before the final `#endif`:

```c
// Kainote functions
FFMS_API(const char*) FFMS_GetTrackName(FFMS_Indexer* Indexer, int Track);
FFMS_API(const char*) FFMS_GetTrackLanguage(FFMS_Indexer* Indexer, int Track);

typedef struct FFMS_Chapter {
    const char* Title;
    int64_t Start;
    int64_t End;
} FFMS_Chapter;

typedef struct FFMS_Chapters {
    FFMS_Chapter* Chapters;
    int NumOfChapters;
} FFMS_Chapters;

FFMS_API(FFMS_Chapters*) FFMS_GetChapters(FFMS_Indexer* Indexer);
FFMS_API(void) FFMS_FreeChapters(FFMS_Chapters** Chapters);

typedef struct FFMS_Attachment {
    const char* Filename;
    const char* Mimetype;
    const uint8_t* Data;
    int DataSize;
} FFMS_Attachment;

FFMS_API(FFMS_Attachment*) FFMS_GetAttachment(FFMS_Indexer* Indexer, int Track);
FFMS_API(void) FFMS_FreeAttachment(FFMS_Attachment** Attachment);

typedef int (FFMS_CC* GetSubtitlesCallback)(int64_t Start, int64_t Duration, int64_t Total, const char* Line, void* ICPrivate);
FFMS_API(void) FFMS_GetSubtitles(FFMS_Indexer* Indexer, int Track, GetSubtitlesCallback IC, void* ICPrivate);
FFMS_API(const char*) FFMS_GetSubtitleExtradata(FFMS_Indexer* Indexer, int Track);
FFMS_API(const char*) FFMS_GetSubtitleFormat(FFMS_Indexer* Indexer, int Track);
```

Add the following members to the end of the `FFMS_Indexer` class/struct declaration in the FFMS2 indexing header (`Indexing/Indexing.h` or `src/core/indexing.h`, depending on the FFMS2 version):

```cpp
// Kainote functions
const char* GetTrackName(int Track);
const char* GetTrackLanguage(int Track);
FFMS_Chapters* GetChapters();
FFMS_Attachment* GetAttachment(int Track);
void GetSubtitles(int Track, GetSubtitlesCallback IC, void* ICPrivate);
const char* GetSubtitleExtradata(int Track);
const char* GetSubtitleFormat(int Track);
```

Then build FFMS2 according to the FFMS2 build system you are using, making sure it links against the FFmpeg libraries built above. The resulting `ffms2.lib` must be available in one of the Visual Studio library directories used by `Kainote.vcxproj`.

#### 7. Build the Windows solution

1. Open `Kainote.sln` in Visual Studio 2022.
2. Select:
   - Configuration: `Release`
   - Platform: `x64`
3. If Visual Studio cannot find SDKs or third-party headers/libraries, check:
   - `Kainote/Kainote.vcxproj`
   - `Project Properties > C/C++ > General > Additional Include Directories`
   - `Project Properties > Linker > General > Additional Library Directories`
4. Build the solution from Visual Studio, or from **x64 Native Tools Command Prompt for VS 2022**:

```bat
msbuild Kainote.sln /m /p:Configuration=Release /p:Platform=x64
```

The executable is written under the repository's Visual Studio output folders, typically `x64\Release` or `bin\x64\Release`, depending on the active project configuration.

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
- The file-association `("Skojarzenia")` options tab is not shown there is no `xdg-mime` backend yet.

Under Wayland specifically (these work normally on X11):

- Cursor-anchored dialogs (colour picker, font/style/hotkey dialogs, etc.) open centred instead of at the pointer a Wayland client cannot position its own windows.
- "Fullscreen on a specific monitor" falls back to fullscreen on the current output.
- Auto-pause-on-minimize and the B-key minimize do nothing a Wayland client cannot minimize itself.
- The screen-pixel colour eyedropper is unavailable Wayland forbids reading pixels outside the app's own surface.
