Kainote: 
Subtitles editor that can play video using FFMS2 (for typesetting, 
timing and more advanced edition) or DirectShow (for playback 
or minor subtitles edition).

Features:

- Supported formats: ASS, SSA (loaded convert to ASS), SRT, 
MPL2, MDVD, TMP. 

- Conversion from one format to another.

- Traslation mode: keeps original text in second text field till
translation is ended. 

- Seeking not translated and not committed lines.

- Buttons Putting ASS tags in multiple lines.

- Video visual tools for basic tags and position shifter to move
multiple positions of tags: \pos, \move, \org, \clip, \iclip and
\p (ASS drawings).

- Video zoom to use with visual tools. Works also on fullscreen
that makes possible to create very accurate vector clips and drawings.

- Shift times docked in subtitles grid with moving to autio/video 
time.

- Audio spectrum / waveform with karaoke autosplitting tool good 
for japanese lirycs.

- Automation 4 with Dependency control.

- Subtitles comparison for compare two subtitles from different tabs.

- Subtitles filtering for hide unneeded lines.



Newest beta build:

https://drive.google.com/uc?id=1ECqsrLo5d1jPoz-FKvJrS0279YeTKrmS&export=download

Official releases:

https://github.com/bjakja/Kainote/releases


To build You need install:

Visual Studio 2019 (it builds Icu with x64 compiler but uses of 16gb of RAM).

DirectX SDK
https://www.microsoft.com/en-us/download/details.aspx?id=6812

Windows SDK 10 install it with Visual studio 2019 installer

Put into Thirdparty folder that source not have main one folder:

Boost
https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.7z

Icu
https://github.com/unicode-org/icu/releases/download/release-60-3/icu4c-60_3-src.zip

For libass nstall nasm C:/Nasm

Change in project properties paths of installed Windows SDK's 10
and Direct X if are not installed on C disk.
Sometimes Visual Studio 2017 will not find paths to Windows SDK's 10
then you have to add it manually to projects
c/c++ -> general -> additional include directories
and Linker -> general -> additional include directories
It's one of bug of Visual Studio 2019.

Build FFMS2
- Download MSYS2, msys2-x86_64-{date}.exe from https://www.msys2.org/
- Install into c:/msys64
- Edit c:/msys64/msys2_shell.cmd and remove rem from the line with rem set MSYS2_PATH_TYPE=inherit
- Open a x64 Native Tools Command Prompt for VS 2019
- Run c:/msys64/msys2_shell.cmd
- Use the MSYS2 shell for the next steps and enter:
pacman -Syu
pacman -S make
pacman -S diffutils
pacman -S yasm
pacman -S nasm
 
mv /usr/bin/link.exe /usr/bin/link.exe.bak

- get code of ffmpeg from https://github.com/FFmpeg/FFmpeg/archive/refs/heads/master.zip
- unpack it somewhere on c disc with short path for example c:/ffmpeg
- set path to ffmpeg cd /c/path_to_ffmpeg/
- type in msys console (for x64 remove --arch=x86): ./configure --toolchain=msvc --arch=x86 --enable-gpl --enable-version3 --disable-encoders --disable-programs --disable-filters \
 --disable-network --disable-doc --disable-avdevice --disable-postproc --disable-avfilter
 
- when shows list of codecs type make
- after build type make install

- download avisynth from https://github.com/AviSynth/AviSynthPlus/archive/refs/heads/master.zip

- build FFMS2 in Visual Studio 19.

--------------------- Polski - Polish --------------------------

Kainote:
Edytor napisów, który odtwrza wideo zarówno przez FFMS2 (dobre
do typesettingu, timingu i inych zaawansowanych edycji) a także 
DirectShow (przeznaczone do odtwarzania i prostej edycji napisów).

Funkcje:

- Obsługa formatów: ASS, SSA (wczytuje i konwertuje na ASS), SRT, 
MPL2, MDVD, TMP.

- Konwersja między tymi formatami.

- Tryb tłumaczenia: przechowuje tekst oryginalny tak długo póki
tłumaczenie nie zostanie ukończone.

- Szukanie niepewnych i nieprzetłumaczonych linijek.

- Przyciski umieszczające tagi ASS w wielu linijkach równocześnie.

- Narzędzia wizualne wideo do podstawowych tagów i zmieniacz pozycji
pozwalający zmieniać równocześnie pozycje tagów: \pos, \move, \org, 
\clip, \iclip i \p (rysunki ASS).

- Powiększenie wideo można używać z narzędziami wizualnymi wideo.
Działają również na pełnym ekranie, co pozwala na tworzenie bardzo 
dokładnych rysunków i wycinków wektorowych.

- Przesuwanie czasów umiejscowione za polem napisów pozwalający na
przesuwanie do czasów wideo / audio.

- Spektrum / wykres falowy audio z narzędziem do autopodziału karaoke
dobrym do japońskich tekstów piosenek.

- Automatyzacja 4 z Dependency Control.

- Porównywarka napisów z dwóch różnych zakładek.

- Filtrowanie napisów pozwalające ukryć niepotrzebne linijki.


Najnowszy beta build:

https://drive.google.com/uc?id=1ECqsrLo5d1jPoz-FKvJrS0279YeTKrmS&export=download

Oficjalne wydania:

https://github.com/bjakja/Kainote/releases

By zbudować program należy zainstalować:

Visual Studio 2019 (które builduje Icu po zmianie kompilatora na x64, 
ale wymaga do tego 16gb ramu, można też ustawić plik stronicowania na duże wartości)

DirectX SDK
https://www.microsoft.com/en-us/download/details.aspx?id=6812

Windows SDK 10 należy zainstalować używając instalatora Visual Studio 2019

Skopiować do folderów Thirdpardy, pomijając folder zbiorczy:

Boost
https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.7z

Icu
https://github.com/unicode-org/icu/releases/download/release-60-3/icu4c-60_3-src.zip

Zmienić w projekcie Kainote ścieżki do zainstalowanych windows SDK i Direct X, 
jeśli nie są zaintalowane na dysku C.
Gdyby nie widziało ścieżek do SDK 10 to niestety należy je dodać 
c/c++ -> general -> additional include directories
i Linker -> general -> additional include directories
To wada Visual Sudio 2017, nie moja wina, raz je widzi raz nie.


-Thai - ไทย-

Kainote:
เครื่องมือแก้ไขคำบรรยายที่สามารถเล่นวิดีโอโดยใช้ FFMS2 (สำหรับการเรียงพิมพ์,
การจับเวลาและขั้นสูงอื่นๆ ) หรือ DirectShow (สำหรับการเล่น
หรือรุ่นคำบรรยายย่อย)

คุณสมบัติ:

- รูปแบบที่รองรับ: ASS, SSA (โหลดและแปลงเป็น ASS), SRT,
MPL2, MDVD, TMP

- แปลงจากรูปแบบหนึ่งเป็นอีกรูปแบบหนึ่ง

- โหมดผู้แปล: เก็บข้อความต้นฉบับในฟิลด์ข้อความที่สองจนกว่าจะสิ้นสุดการแปล

- ค้นหาบรรทัดที่แปลหรือยังไม่ได้แปลโดยเฉพาะ

- ปุ่มที่ใส่แท็ก ASS ในหลายบรรทัด

- เครื่องมือแสดงภาพวิดีโอสำหรับแท็กพื้นฐานและตัวเปลี่ยนตำแหน่ง
แท็กหลายตำแหน่งเช่น: \pos, \move, \org, \clip, \iclip และ \p (ภาพวาด ASS)

- เลื่อนเวลาคำบรรยายตามเวลา วิดีโอหรือเสียง

- สเปกตรัมเสียง / แผนผังคลื่นเสียง พร้อมเครื่องมือปรับพยางค์คาราโอเกะอัตโนมัติสำหรับเนื้อเพลงญี่ปุ่น

- สคริปต์อัตโนมัติ 4 กับ Despendency control


รับรุ่นเบต้าใหม่ล่าสุด:

https://drive.google.com/uc?id=1ECqsrLo5d1jPoz-FKvJrS0279YeTKrmS&export=download

รับรุ่นเผยแพร่อย่างเป็นทางการ:

https://github.com/bjakja/Kainote/releases


ในการบิวด์คุณต้องติดตั้ง:

Visual Studio 2017 (บิวด์ Icu พร้อมคอมไพเลอร์ x64 และแรมขนาด 16gb)

DirectX SDK
https://www.microsoft.com/en-us/download/details.aspx?id=6812

Windows SDK 10 พร้อมตัวติดตั้ง Visual studio 2019

ใส่ลงในโฟลเดอร์ Thirdparty โดยไม่ต้องใส่โฟลเดอร์:

Boost
https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.7z

Icu
https://github.com/unicode-org/icu/releases/download/release-60-3/icu4c-60_3-src.zip

เปลี่ยนเส้นทางคุณสมบัติโครงการของ Windows SDK 10 ที่ติดตั้ง และ Direct X หากไม่ได้ติดตั้งไว้ในดิสก์ C 
บางครั้ง Visual Studio 2017 จะไม่พบเส้นทางไปยัง Windows SDK 10 จากนั้นคุณต้องเพิ่มมันเข้าไปในโครงการด้วยตนเอง c/c++ -> General -> ไดเร็กเทอรี่เพิ่มเติมและ Linker -> General -> ไดเร็กเทอรี่เพิ่มเติม 
มันเป็นหนึ่งในข้อบกพร่องของ Visual Studio 2019
