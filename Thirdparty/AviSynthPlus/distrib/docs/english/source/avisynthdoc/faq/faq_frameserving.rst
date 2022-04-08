
AviSynth FAQ - Opening scripts in encoder and player applications
=================================================================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



What is frameserving and what is it good for?
---------------------------------------------

An excellent description is found on `Lukes homepage`_:

"Frameserving is a process by which you directly transfer video data from one
program on your computer to another. No intermediate or temporary files are
created. The program that opens the source file(s) and outputs the video data
is called the frameserver. The program that receives the data could be any
type of video application.

There are two main reasons that you would want to frameserve a video:

1.  **Save Disk Space**: Depending on the frameserving application, you
    can usually edit/process your video as it is being frameserved. Because
    frameserving produces no intermediate files, you can use a frameserver to
    alter your videos without requiring any additional disk space. For
    example, if you wanted to join two video files, resize them, and feed
    them to another video application, frameserving would allow you to do
    this without creating a large intermediate file.
2.  **Increased Compatibility**: To the video application that's
    receiving the frameserved video, the input looks like a relatively small,
    uncompressed video file. However, the source file that the frameserver is
    transferring could actually be, for example, a highly compressed MPEG-1
    video. If your video application doesn't support MPEG-1 files, it's not a
    problem because the application is just receiving standard uncompressed
    video from the frameserver. This feature of frameserving enables you to
    open certain types of files in an application that wouldn't normally
    support them.

Furthermore, because the video application is being fed the source video one
frame at a time, it doesn't know anything about the file size of the source
video. Therefore, if your application has 2 GB or 4 GB limit on input file
size, it won't have any effect on your frameserved video. You could feed 100
GB of video via a frameserver to an application limited to 2 GB and it
wouldn't cause a problem."


How do I use AviSynth as a frameserver?
---------------------------------------

Write a script using a text editor. Load your clip in AviSynth (see
:doc:`FAQ loading clips <faq_loading_clips>`), do the necessary filtering and load the AVS-file in
encoder/application X (must be an encoder or application which can read AVI-
files (see also `here`_).


How do I frameserve my AVS-file to encoder/application X?
---------------------------------------------------------

There is simple way for many applications, and tricky ways for many others.


Direct frameserving to compatible applications
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Simply open your AVS file in coder/application with menu, command line or
drag-and-drop AVS file to it (working ways are dependent on the application).
Some programs have ``"AviSynth *.avs"`` in "Open" menu, for others try select
"All files *.*" or type AVS file name instead of "AVI".

Players: Media Player Classic, Windows Media Player 6.4, 9 and others.

Encoders: QuEnc, Mencoder, HC Encoder, CCE SP 2.50 and 2.66, Canopus Procoder
1.5 and above, MainConcept MPEG Encoder, TMPGEnc, TMPGEncXpress 3/4, Elecard
Converter Studio, xvid_encraw, FFMpeg (new versions), Nero 6, Nero 7 (drag-
and-drop only) and others.

Editors: VirtualDub, AviDemux (through its avs proxy option)

But some applications work fine only with some specific video or audio
formats, have a look at the next section.


Direct frameserving to applications using additional plugins
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   For frameserving to Premiere there exists an import plugin "IM-
    Avisynth.prm".

The original version was located at `Bens site`_, see `mirror`_. A much
improved version can be downloaded from the `Video Editors Kit sourceforge
page`_. This works for Premiere 5.x, 6.x and Pro at present. Version 1.5 also
works for Premier CS3. To install the import plugin move the IM-Avisynth.prm
file into your Premiere "Plug-ins" directory.


Direct frameserving to special or modified versions of encoders
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some programs initially could not open AviSynth scripts, but updated or
alternative programs can do it:

-   Mencoder
-   FFMpeg: Versions older then SVN-r6129 use the "AVSredirect.dll" for
    communication with Avisynth. From SVN-r6129 up, AVS redirect code is
    integrated in the FFmpeg executable (as option at compiling). Use builds
    at `http://ffdshow.faireal.net/mirror/ffmpeg/`_
-   Windows Media 9 Encoder: Download Nic's Windows Media 9 Encoder and
    make sure you also installed the Windows Media 9 codec. Both can be found
    `here`_.


Frameserving to applications via fake AVI files and proxy utilities
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Many "new" programs do NOT use the Windows functions to read the AVI-files.
If they use own read functions the AviSynth-script files will not work. There
are utilities that can create small fake AVI file with special type (FOURCC),
and provide correspondent system codec to "decode" these dummy compressed
files.

Select your AVS file in utility menu, set options and create fake AVI file
with some name. Then you can open this fake AVI in your application/encoder,
that will be receive frames from the codec that wiil be receive frames from
AviSynth.

Several such utilities are different by supported modes (formats) of output
video (with or without conversion) and audio (unpacked audio is most
compatible but filesize is larger), by user interface (window, command line)
and number of bugs.

-   `VFAPI reader codec`_ with DGVfapi (as a client) from `DGDMPGDec`_.

Features - output RGB24 only, unpacked audio, multiple files support, good
compatibility, but a bit slow.

-   MakeAVIS is included in ffvfw and `FFDShow`_.

Features - ouput to any color format. Uncompressed audio works properly in
old ffvfw and recent (13 november 2007) ffdshow (8 and 16 bit only, use
:doc:`ConvertAudioTo16bit <../corefilters/convertaudio>` when necessary).

-   `Proxy-codec AVS2AVI`_. (Note that the same-name utility by Moitah
    and others is an encoder and not an AVI-wrapper.)

Features - video output same as input format, no audio.

Known programs that will not open AVS scrips without these utilities:
CCE SP v2.62-2.64, Windows Media Encoder vx.x. (older than v9), Ulead
VideoStudio 5-11, MediaStudio 6-8, Pinnacle Studio, Sony Vegas, Nero 8,
ImageMixer and others.


Frameserving via pipe from auxiliary programs to application-encoders
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`Avs2YUV`_ is a command-line program, intended for use under Wine, to
interface between AviSynth and Linux-based video tools.

Programs: Mpeg2enc, Mencoder, FFMpeg.

avs2yuv out.avs -o - | mpeg2enc - options...

This way is obsolete since these programs have native AviSynth support now.


How do I solve problems when opening/reading scripts in encoders and players?
-----------------------------------------------------------------------------

1.  TMPGEnc doesn't read my AVS files (this happens in old versions of
    TMPGEnc), what to do?

    -   Install the VFAPI plugin for TMPGEnc.
    -   Disable the direct show filters within TMPGEnc and turn off the
        VirtualDub proxy before frameserving.
    -   Add ":doc:`ConvertToRGB24 <../corefilters/convert>`" at the end of your AVS-file.
    -   Install `Huffyuv`_/`DivX`_ codec so that it can do the
        decompression for you when loading an AVI in TMPGEnc.
    -   Install the `ReadAVS plugin`_. Just copy ReadAVS.dll to the VFAPI
        reader directory and open the reg-file ReadAVS.reg in notepad and change
        the corresponding path. Save it, and doubleclick on it to merge it with
        your registry-file.

2.  CCE SP crashes when reading an AVS-file, what to do?

    -   If you're using Win2k then run CCE in WinNT4-SP5 compatibility mode.
    -   Put addaudio.avsi in your AviSynth plugin folder and add
        ``AddAudio(44100)`` in your script, if you don't have any audio in your
        AVS-file.
    -   Some versions (like CCE SP v2.62/v2.64) don't read AVS files. Get
        CCE SP v2.66 or a more recent version.

3.  My encoder or player doesn't open AviSynth scripts, what should I do?

    -   In this case you may try other way, for example an AVI wrapper,
        like `vfapi`_ or `makeAVIS`_.

4.  When opening my clip in an encoder or player, the colors are messed
    up, what to do?

    -   If you have such problems, some external (or internal) codec is
        messing up the used colorspace conversion. If you have such problems add
        :doc:`ConvertToRGB24 <../corefilters/convert>` as the last line of your script (for Procoder and CCE
        use ConvertToYUY2(interlaced=true) or =false) and have a look at the
        thread (and the suggested solutions) `colorspace conversion errors`_.

5.  Windows Media Encoder 9 Series does not open AVS files, what to do?

    -   Use an `updated WMCmd.vbs script`_ [`discussion about the fix`_].

    -   In order to use AviSynth source with WME9, you need to set the
        encoder source to "Both device and file" in the Session Properties, see
        `discussion`_ and `WMV faq`_. Or use `Nic's WMV encoder`_.

6.  WMP11 on Vista dos no play AVS, what to do?

    -   You may `edit registry`_ to add .avs as known extension. Copy the
        registry key (and subkeys) for

        ``HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Multimedia\WMPlayer\Extensions\.avi``

        to

        ``HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Multimedia\WMPlayer\Extensions\.avs``

    -   On Vista x64 you have to copy the correct 32-Bit nodes:

        ``HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Multimedia\WMPlayer\Extensions\.avi``

        to

        ``HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Multimedia\WMPlayer\Extensions\.avs``

    -   Do not change anything at the registry if you are not experienced!


How do I frameserve from Premiere/Ulead/Vegas to AviSynth?
----------------------------------------------------------

Install the AviSynth compatible frameserver `PluginPace frameserver (by
Satish Kumar)`_ for frameserving from SonicFoundry Vegas (and earlier Vegas
Video/VideoFactory versions), Adobe Premiere, Ulead MediaStudio Pro or Wax to
AviSynth (`discussion here`_).

| :doc:`Main Page <faq_sections>` | :doc:`General Info <faq_general_info>` | :doc:`Loading Clips <faq_loading_clips>` | **Loading Scripts** | :doc:`Common Error Messages <faq_common_errors>` | :doc:`Processing Different Content <faq_different_types_content>` | :doc:`Dealing with YV12 <faq_yv12>` | :doc:`Processing with Virtualdub Plugins <faq_using_virtualdub_plugins>` |

$Date: 2009/09/12 20:57:20 $

.. _Lukes homepage: http:///neuron2.net/lvg/frameserving.html
.. _Bens site: http://math.berkeley.edu/~benrg/avisynth-premiere.html
.. _mirror: http://neuron2.net/www.math.berkeley.edu/benrg/avisynth-premiere.html
.. _Video Editors Kit sourceforge page:
    http://sourceforge.net/projects/videoeditorskit/
.. _http://ffdshow.faireal.net/mirror/ffmpeg/:
    http://ffdshow.faireal.net/mirror/ffmpeg/
.. _here: http://nic.dnsalias.com/WM9Enc.html
.. _Nic's WMV encoder: http://nic.dnsalias.com/WM9Enc.html
.. _VFAPI reader codec:
    http://www.doom9.org/Soft21/SupportUtils/VFAPIConv-1.05-EN.zip
.. _DGDMPGDec: http://neuron2.net/dgmpgdec/dgmpgdec.html
.. _FFDShow: http://sourceforge.net/project/showfiles.php?group_id=173941
.. _Proxy-codec AVS2AVI: http://hmd.c58.ru/files.html
.. _Avs2YUV: http://akuvian.org/src/avisynth/avs2yuv/
.. _Huffyuv: http://avisynth.org/mediawiki/Huffyuv
.. _DivX: http://avisynth.org/mediawiki/DivX
.. _ReadAVS plugin: http://www.math.berkeley.edu/~benrg/avisynth/tmpgenc-readavs.zip
.. _AddAudio: http://avisynth.org/mediawiki/AddAudio
.. _vfapi: http://avisynth.org/mediawiki/Vfapi
.. _makeAVIS: http://avisynth.org/mediawiki/MakeAVIS
.. _colorspace conversion errors:
    http://forum.doom9.org/showthread.php?s=&threadid=27932
.. _updated WMCmd.vbs script: http://www.citizeninsomniac.com/WMV/#WMCmd
.. _discussion about the fix:
    http://forum.doom9.org/showthread.php?t=65638
.. _discussion: http://forum.doom9.org/showthread.php?t=48477
.. _WMV faq: http://forum.doom9.org/showthread.php?t=112634
.. _edit registry: http://forum.doom9.org/showthread.php?t=121674
.. _PluginPace frameserver (by Satish Kumar):
    http://www.debugmode.com/pluginpac/frameserver.php
.. _discussion here: http://forum.doom9.org/showthread.php?s=&threadid=51242
