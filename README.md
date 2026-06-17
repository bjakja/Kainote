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

The Linux build uses CMake and system packages. It has been verified on an Ubuntu/Debian-style environment with GCC, wxGTK 3.2, Lua 5.1, FFMS2, FFmpeg, libass, Hunspell, uchardet, libcurl, ICU, Boost, and OpenGL development packages.

#### 1. Install dependencies on Ubuntu/Debian

```bash
sudo apt update
sudo apt install --no-install-recommends -y \
  build-essential \
  cmake \
  git \
  pkg-config \
  libwxgtk3.2-dev \
  libwxgtk-gl3.2-dev \
  libass-dev \
  libffms2-dev \
  liblua5.1-0-dev \
  libhunspell-dev \
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
  libgtk-3-dev
```

Optional but useful for headless smoke tests:

```bash
sudo apt install --no-install-recommends -y xvfb
```

#### 2. Package names on other Linux distributions

The exact package names vary by distribution. Install the equivalent development packages for:

- C and C++ compiler toolchain (`gcc`, `g++`, `make`)
- CMake
- pkg-config
- wxWidgets/wxGTK 3.x with core, base, adv, aui, html, xml, gl, and stc components
- libass
- FFMS2
- Lua 5.1 development headers and library
- Hunspell
- uchardet
- libcurl
- ICU (`icu-uc` and `icu-i18n` pkg-config modules)
- Boost filesystem, locale, regex, and system
- FFmpeg development libraries: libavformat, libavcodec, libavutil
- OpenGL/Mesa development headers
- GTK 3 development headers

For Fedora-like systems, the package set is approximately:

```bash
sudo dnf install \
  gcc gcc-c++ make cmake git pkgconf-pkg-config \
  wxGTK-devel wxGTK-gl wxGTK-media \
  libass-devel ffms2-devel lua-devel hunspell-devel uchardet-devel \
  libcurl-devel libicu-devel boost-devel ffmpeg-devel mesa-libGL-devel gtk3-devel
```

For Arch-like systems, the package set is approximately:

```bash
sudo pacman -S --needed \
  base-devel cmake git pkgconf wxwidgets-gtk3 libass ffms2 lua51 \
  hunspell uchardet curl icu boost ffmpeg mesa gtk3
```

If your distribution only provides Lua 5.4 as `lua`, install the separate Lua 5.1 development package. The CMake file intentionally checks for `lua5.1` because Kainote uses Lua 5.1 APIs such as `lua_getfenv`, `lua_objlen`, and `luaL_register`.

#### 3. Verify dependency discovery

Before configuring Kainote, confirm that pkg-config can find the required libraries:

```bash
pkg-config --modversion \
  libass \
  ffms2 \
  lua5.1 \
  hunspell \
  uchardet \
  libcurl \
  icu-uc \
  icu-i18n \
  libavformat \
  libavcodec \
  libavutil
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

The Linux build is compile/link-capable and can start under wxGTK. Current smoke coverage includes launching the GUI under Xvfb and verifying that the main window, subtitle grid, options controls, and time-shifting side panel render without wxSizer consistency asserts or GTK runtime warnings.

Some Windows-only subsystems are still compatibility layers or partial ports:

- DirectShow-specific playback paths are not native Linux backends; prefer FFMS2-backed media paths while porting and testing.
- DirectSound-specific audio paths are not a native Linux audio backend.
- Direct3D 9/D3DX compatibility headers are present so the project can compile, but Linux rendering should continue moving toward wxGraphics/OpenGL/Vulkan/native backends.
- Windows taskbar/COM/Shell features are mapped only where a practical Linux equivalent exists.

For development, run both a headless Xvfb smoke test and a real desktop-session check, especially after changing wxGTK layout code or custom controls.
