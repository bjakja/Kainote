| Download | Help Us Translate! | Join Discord Server |
|-------|----------|---------|
| [![release](https://img.shields.io/github/v/release/bjakja/Kainote.svg?maxAge=3600&label=download)](https://github.com/bjakja/Kainote/releases) | [![Translation status](https://hosted.weblate.org/widgets/kainote/-/svg-badge.svg)](https://hosted.weblate.org/engage/kainote/?utm_source=widget) | [Discord](https://discord.gg/8kNAxDFgwj) |


# Kainote
Subtitles editor that can play video using FFMS2 (for typesetting, timing and more advanced edition) or DirectShow (for playback or minor subtitles edition).

## Features

* Supported formats: ASS, SSA (loaded convert to ASS), SRT, MPL2, MDVD, TMP. 

* Conversion from one format to another.

* Traslation mode: keeps original text in second text field till translation is ended. 

* Seeking not translated and not committed lines.

* Buttons Putting ASS tags in multiple lines.

* Video visual tools for basic tags and position shifter to move multiple positions of tags: \pos, \move, \org, \clip, \iclip and \p (ASS drawings).

* Video zoom to use with visual tools. Works also on fullscreen that makes possible to create very accurate vector clips and drawings.

* Shift times docked in subtitles grid with moving to autio/video time.

* Audio spectrum / waveform with karaoke autosplitting tool good for japanese lirycs.

* Automation 4 with Dependency control.

* Subtitles comparison for compare two subtitles from different tabs.

* Subtitles filtering for hide unneeded lines.

## Beta Build

You can download Kainote bata version [here.](https://drive.google.com/uc?id=1ECqsrLo5d1jPoz-FKvJrS0279YeTKrmS&export=download)
Plsease Note: Bata is a unstable versions, and if you have any question you can reach out to us on [Discord](https://discord.gg/8kNAxDFgwj).

## FAQ

You can reach out to us on [Discord](https://discord.gg/8kNAxDFgwj).

## Building

To build You need install:
* Visual Studio 2019 (it builds Icu with x64 compiler but uses of 16gb of RAM).
* DirectX SDK https://www.microsoft.com/en-us/download/details.aspx?id=6812
* Windows SDK 10 install it with Visual studio 2019 installer

You need to put following into Thirdparty folder that source not have main one folder:
* Boost https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.7z
* Icu https://github.com/unicode-org/icu/releases/download/release-60-3/icu4c-60_3-src.zip
* (For libass install nasm C:/Nasm)

Next:
* Change in project properties paths of installed Windows SDK's 10 and Direct X if are not installed on C disk. (Sometimes Visual Studio 2017 will not find paths to Windows SDK's 10.)
* Then you have to add it manually to projects c/c++ -> general -> additional include directories and Linker -> general -> additional include directories (It's one of bug of Visual Studio 2019.)

* Build FFMS2:
    - Download MSYS2, msys2-x86_64-{date}.exe from https://www.msys2.org/
    - Install into c:/msys64
    - Edit c:/msys64/msys2_shell.cmd and remove rem from the line with rem set MSYS2_PATH_TYPE=inherit
    - Open a x64 Native Tools Command Prompt for VS 2019
    - Run c:/msys64/msys2_shell.cmd
    - Use the MSYS2 shell for the next steps and enter:
    ```pacman -Syu
    pacman -S make
    pacman -S diffutils
    pacman -S yasm
    pacman -S nasm

    mv /usr/bin/link.exe /usr/bin/link.exe.bak
    ```
    - Get code of ffmpeg from https://github.com/FFmpeg/FFmpeg/archive/refs/heads/master.zip
    - Onpack it somewhere on c disc with short path for example c:/ffmpeg
    - Set path to ffmpeg cd /c/path_to_ffmpeg/
    - Type in msys console *(for x64 remove --arch=x86 --arch=x86_64)*:
        ```
        ./configure --toolchain=msvc --enable-gpl --enable-version3 --disable-encoders --disable-programs --disable-filters \ --disable-network --disable-doc --disable-avdevice --disable-postproc --disable-avfilter
        ```
    - When shows list of codecs type **make**
    - After build type **make install**
- Done, You just build FFMS2 in Visual Studio 19.
