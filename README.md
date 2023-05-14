| Download | Help Us Translate! | Join Discord Server |
|-------|----------|---------|
| [![release](https://img.shields.io/github/v/release/bjakja/Kainote.svg?maxAge=3600&label=download)](https://github.com/bjakja/Kainote/releases) | [![Translation status](https://hosted.weblate.org/widgets/kainote/-/svg-badge.svg)](https://hosted.weblate.org/engage/kainote/?utm_source=widget) | [![Discord](https://img.shields.io/discord/961361569269293077.svg?label=discord&labelColor=7289da&color=2c2f33&style=flat)](https://discord.gg/9WacFTtK6q) |


# Kainote
Subtitle editor that can play videos using FFMS2 (for typesetting, timing, and more advanced editing) or DirectShow (for playback or minor subtitle editing).

## Features

* Supported formats: ASS, SSA (loaded convert to ASS), SRT, MPL2, MDVD, TMP. 

* Conversion from one format to another.

* Translation mode: keeps the original text in the second text field until the translation is complete. 

* Seeking not translated and not committed lines.

* Buttons for putting ASS tags in multiple lines.

* Video visual tools for basic tags and a position shifter to move the positions of multiple tags: \pos, \move, \org, \clip, \iclip and \p (ASS drawings).

* Video zoom to use with visual tools. Works also on fullscreen that makes possible to create very accurate vector clips and drawings.

* Shift times docked in subtitles grid with moving to audio/video time.

* Audio spectrum/waveform with karaoke auto-splitting tool good for Japanese lyrics.

* Automation 4 with Dependency control.

* Subtitles comparison to compare two subtitles from different tabs.

* Subtitles filtering to hide unneeded lines.

## Beta Build

You can download Kainote beta version [here.](https://drive.google.com/uc?id=1ECqsrLo5d1jPoz-FKvJrS0279YeTKrmS&export=download)

Please Note: Beta is an unstable version, and if you have any questions, you can reach out to us on [Discord](https://discord.gg/8kNAxDFgwj).

## Contributing a translations
Want to help translate Kainote to your language? You can easily help by utilizing a service we use called **Weblate**.

Visit our translation project [here](https://hosted.weblate.org/engage/kainote/?utm_source=widget).

## FAQ

You can reach out to us on [Discord](https://discord.gg/8kNAxDFgwj).

## Building

To build, you need to install:
* Visual Studio 2019 (it builds Icu with x64 compiler but uses 16gb of RAM).
* [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
* Windows SDK 10 - install it with the Visual Studio 2019 installer

You need to put the following libraries into the Thirdparty folder so that the source code does not have one main folder:
* [Boost](https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.7z)
* [Icu](https://github.com/unicode-org/icu/releases/download/release-60-3/icu4c-60_3-src.zip)
* (For libass, install NASM in `C:/Nasm`)

Next:
* Change the paths of installed Windows SDK 10 and Direct X in project properties if they are not installed on the C drive. (Sometimes Visual Studio 2017 may not find the paths to Windows SDK 10.)
* Then manually add them to project properties c/c++ -> general -> additional include directories and Linker -> general -> additional include directories (This is one of the bugs of Visual Studio 2019.)

* Build FFMS2:
    - Download **MSYS2**, msys2-x86_64-{date}.exe from https://www.msys2.org/
    - Install it into `c:/msys64`
    - Edit `c:/msys64/msys2_shell.cmd` and replace `rem` from the line with `rem set MSYS2_PATH_TYPE=inherit`
    - Open a **x64 Native Tools Command Prompt for VS 2019**
    - Run `c:/msys64/msys2_shell.cmd`
    - Use the MSYS2 shell for the next steps and enter:
    ```pacman -Syu
    pacman -S make
    pacman -S diffutils
    pacman -S yasm
    pacman -S nasm

    mv /usr/bin/link.exe /usr/bin/link.exe.bak
    ```
    - Get the code of FFmpeg from https://github.com/FFmpeg/FFmpeg/archive/refs/heads/master.zip
    - Unpack it somewhere on the C drive with a short path, for example: `c:/ffmpeg`
    - Set the path to FFmpeg `cd c:/path_to_ffmpeg/`
    - Type in **msys** console *(for x64 remove `--arch=x86 --arch=x86_64`)*:
        ```
        ./configure --toolchain=msvc --enable-gpl --enable-version3 --disable-encoders --disable-programs --disable-filters --disable-network --disable-doc --disable-avdevice --disable-postproc --disable-avfilter --enable-dxva2 --enable-d3d11va
        ```
    - When the list of codecs is displayed, type `make`
    - After the build is complete, type `make install`
- Done, you can build FFMS2 in the Kainote solution.
