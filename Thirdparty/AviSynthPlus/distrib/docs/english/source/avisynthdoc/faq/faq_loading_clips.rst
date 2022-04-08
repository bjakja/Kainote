
AviSynth FAQ - Loading clips (video, audio and images) into AviSynth
====================================================================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Which media formats can be loaded into AviSynth?
------------------------------------------------

Most video/audio formats can be loaded into AviSynth, but there are some
exceptions like flv4 (VP6) and dvr-ms. If it is not possible to load a clip
into AviSynth, you will have to convert it into some other format which can
be loaded. Remember to choose a format for which you will have a minimal
downgrade in quality as a result of the conversion.


Which possibilities exist to load my clip into AviSynth?
--------------------------------------------------------

In general there are two ways to load your video into AviSynth:

1.  using an AviSynth internal filter or plugin which is designed to open
    some specific format.
2.  using the :doc:`DirectShowSource <../corefilters/directshowsource>` plugin.

Make sure that your clip contains maximal one video and/or one audio stream
(thus remove the subtitles and remove other video/audio streams). If you want
to load a clip which contains both video and audio, you have two options:

-   Demux the audio stream and load the streams separately in AviSynth.
-   Try to load the clip in AviSynth. This might or might not work. For
    AVIs, make sure you have a good AVI splitter installed, e.g. `Gabest's
    AVI splitter`_. (Yes, Windows comes with an own AVI splitter, which will
    work in most cases.)

When loading a clip into AviSynth it is advised to follow the following
guidelines:

-   When it is possible to load your clip into AviSynth using either
    AviSource or a specific plugin then do so, since this is more reliable
    than the alternatives which are listed below.
-   If the above fails, load your clip using the DirectShowSource plugin.
-   If the above fails, convert your clip into a different format (into
    one which is supported by AviSynth).


What are the advantages and disadvantages of using DirectShowSource to load your media files?
---------------------------------------------------------------------------------------------

*advantages of DirectShowSource:*

-   Many video and audio formats are supported through DirectShowSource
    (have a look at ffdshow for example).

*disadvantages of DirectShowSource:*

-   It's less reliable than AviSource and specific video/audio input
    plugins.
-   Seeking problems.
-   It might be much trouble to get specific DirectShow filter doing the
    decoding for you. In many cases you will have multiple decoders that can
    decode the same specific format. The one which will be used is the one
    with the highest merit. It might be difficult to ensure that a specific
    decoder is doing the decoding. The document
    ":doc:`Importing media into AviSynth <../advancedtopics/importing_media>`"
    contains some more information about this.


Has AviSynth a direct stream copy mode like VirtualDub?
-------------------------------------------------------

No, the video and the audio are decompressed when opening them into AviSynth.

There is a modification of AviSynth v2.55 which supports 'direct stream copy'
for both video and audio. This modification is called DSynth and can be
downloaded `here`_. Perhaps it will be updated and merged into the official
AviSynth builds one day.


How do I load AVI files into AviSynth?
--------------------------------------

Use :doc:`AviSource <../corefilters/avisource>` to load your AVI files in AviSynth. Example:

::

    AviSource("d:\filename.avi")

or without the audio:

::

    AviSource("d:\filename.avi", false)

If AviSynth is complaining about not being able to load your avi (couldn't
decompress ...) you need to install an appropriate codec. `GSpot`_, for
example, will tell you what codec you need to install in order to be able to
open your avi.

Forcing a decoder being used for loading the clip into AviSynth:

::

    # load your avi using the XviD codec:
    AviSource("d:\filename.avi", fourCC="XVID")
    # opens an avi (for example encoded with DivX3) using the XviD Codec

::

    # load your dv-avi using the Canopus DV Codec:
    AviSource("d:\filename.avi", fourCC="CDVC")

Can I load video with audio from AVI into AviSynth?
---------------------------------------------------

It is always possible to demux your audio from the AVI file and load it
separately in AviSynth using an audio decoder, but in some cases (for
example: AVI with MP2/MP3/AC3/DTS audio) it is possible to load it directly
in AviSynth.

For loading your AVI with audio you need (1) a VfW (Video for Windows) codec
to open (that is decode) your video in AviSynth and an ACM (Audio Compression
Manager) codec to open your audio in AviSynth. For many video and audio
format such codecs are available, but certainly not for all of them.

You can find those codecs in the document ":doc:`Importing media into AviSynth <../advancedtopics/importing_media>`" .


How do I load MPEG-1/MPEG-2/DVD VOB/TS/PVA into AviSynth?
---------------------------------------------------------

DGDecode is an external plugin and supports MPEG-1, MPEG-2 / VOB, TS (with
MPEG-4 ASP video) and PVA streams. Open them into DGIndex first and create a
d2v script which can be opened in AviSynth (note that it will only open the
video into AviSynth):

A few examples:

::

    # DGDecode:
    LoadPlugin("d:\dgdecode.dll")
    mpeg2source("d:\filename.d2v")

If your transport stream (``*.TS``) contains MPEG-4 AVC video you need to demux
the raw video stream from it and use `DGAVCDec`_ to open it in AviSynth. See
`here`_ for its usage.


How do I load QuickTime files into AviSynth?
--------------------------------------------

There are two ways to load your quicktime movies into AviSynth (and also
RawSource for uncompressed movs): QTSource and QTReader. The former one is
very recent and able to open many quicktime formats (with the possibility to
open them as YUY2), but you need to install QuickTime player in order to be
able to use this plugin. The latter one is very old, no installation of a
player is required in order to be able to open quicktime formats in AviSynth.

QTSource:

You will need Quicktime 6 for getting video only or Quicktime 7 for getting
audio and video.

::

    # YUY2 (default):
    QTInput("FileName.mov", color=2)

    # with audio (in many cases possible with QuickTime 7)
    QTInput("FileName.mov", color=2, audio=1)

    # raw (with for example a YUYV format):
    QTInput("FileName.mov", color=2, mode=1, raw="yuyv")

    # dither = 1; converts raw 10bit to 8bit video (v210 = 10bit uyvy):
    QTInput("FileName.mov", color=2, dither=1, raw="v210")

QTReader:

If that doesn't work, or you don't have QuickTime, download the QTReader
plugin (can be found in Dooms download section):

::

    LoadVFAPIPlugin("C:\QTReader\QTReader.vfp", "QTReader")
    QTReader("C:\quicktime.mov")

How do I load raw source video files into AviSynth?
---------------------------------------------------

The external plugin RawSource supports all kinds of raw video files with the
YUV4MPEG2 header and without header (video files which contains YUV2, YV16,
YV12, RGB or Y8 video data).

Examples:

::

    # This assumes there is a valid YUV4MPEG2-header inside:
    RawSource("d:\yuv4mpeg.yuv")

    # A raw file with RGBA data:
    RawSource("d:\src6_625.raw",720,576,"BGRA")

    # You can enter the byte positions of the video frames directly (which can be found with yuvscan.exe).
    # This is useful if it's not really raw video, but e.g. uncompressed MOV files or a file with some kind of header:
    RawSource("d:\yuv.mov", 720, 576, "UYVY", index="0:192512 1:1021952 25:21120512 50:42048512 75:62976512")

How do I load RealMedia files into AviSynth?
--------------------------------------------

RM/RMVB (RealMedia / RealMedia Variable Bitrate usually containing Real
Video/Audio): install the `rmvb splitter`_ and the Real codecs by installing
RealPlayer/`RealAlternative`_. Create the script:

::

    DirectShowSource("d:\clip.rmvb", fps=25, convertfps=true) # adjust fps if necessary

How do I load Windows Media Video files into AviSynth?
------------------------------------------------------

WMV/ASF (Windows Media Video / Advanced Systems Format; usually containing
WMV/WMA) is not fully supported by ffdshow, so you will have to install wmv
codecs. Get `WMF SDK v9 for W2K or later for XP/Vista`_ which contains the
codecs (and the DMO wrappers necessary to use DMO filters in DirectShow). You
can also get these codecs from Windows Media Player 9 Series or later,
Windows Media Format runtime (WMFDist.exe), Codec Installation Package
(WM9Codecs.exe) from Microsoft site or other place. (Note that Microsoft's
own VC1 codec is not supported in W2K since you need WMF SDK v11.) Create the
script:

::

    DirectShowSource("d:\clip.wmv", fps=25, convertfps=true) # adjust fps if necessary


.. _How do I load MP4/MKV/M2TS/EVO into AviSynth:

How do I load MP4/MKV/M2TS/EVO into AviSynth?
---------------------------------------------

If your media file contains MPEG-4 ASP video, then there are two
possibilities of opening them in AviSynth:

1) Using the plugin `FFmpegSource`_. Some examples:

::

    # loading the video from MKV and returning a timecodes file:
    FFmpegSource("D:\file.mkv", vtrack = -1, atrack = -2, timecodes="timecodes_file.txt")

    # loading the video and audio from a MP4 and returning a timecodes file:
    FFmpegSource("D:\file_aac.mp4", vtrack = -1, atrack = -1, timecodes="timecodes_file.txt")

It's important to generate a timecode file to check whether the video has a
constant framerate. If this the case you don't need to use the timecode file
and you can process the video in any way you want. However, many non-AVI
files contain video with a variable framerate (AVI files always have a
constant framerate though), and in that case you need to make sure of the
following two things:

1.  *Don't change the framerate and the number of frames in AviSynth.* If
    you do this (and you don't change the timecodes file manually) your video
    and audio in your final encoding will be out of sync.
2.  *:ref:`Use the timecodes file again <create-vfr-mkv>` when muxing your encoded video and
    audio.* If you don't do this your video and audio in your final encoding
    will be out of sync.

The main reason for this is that FFmpegSource opens the video as it is. It
doesn't add or remove frames to convert it to constant framerate video to
ensure sync.

2) Get `ffdshow`_ and open the MP4/MKV file with DirectShowSource, thus
   for example

::

    DirectShowSource("D:\file.mkv", convertfps=true)
    # convertfps=true ensures sync if your video has a variable framerate

If your media file contains MPEG-4 AVC video, then there are two
possibilities of opening them in AviSynth:

1) Using the plugin `FFmpegSource`_. See above for its usage. At the
   moment, the supported containers are: AVI, MKV and MP4.

2) Get `DGAVCDec`_. At the moment you need to extract the raw stream
   (``*.264``)  from the container first (using MKVExtract, MPlayer, TSRemux or
   whatever program can extract those streams). Open the raw stream file in
   DGAVCIndex to create an index file (say track1.dga). Open the index file
   in AviSynth:

::

    # raw video demuxed from M2TS (Blu-ray BDAV MPEG-2 transport streams)
    LoadPlugin("C:\Program Files\AviSynth\plugins\DGAVCDecode.dll")
    AVCSource("D:\track1.dga")

How do I load WAVE PCM files into AviSynth?
-------------------------------------------

Use WavSource to open your WAVE PCM files (assuming that they are smaller
than 4GB):

::

    WavSource("D:\file.wav")

Use the plugin RaWav to open your WAVE PCM files that are larger than 4GB
(`Sonic Foundry Video Editor Wave64 Files or W64`_):

::

    RaWavSource("D:\file.w64", SampleRate=96000, SampleBits=24, Channels=6)

    # or when a W64 header is present
    RaWavSource("D:\file.w64", SampleRate=6) # assumes the presence of a
    W64 header and reads the needed info from it

How do I load MP1/MP2/MP3/MPA/AC3/DTS/LPCM into AviSynth?
---------------------------------------------------------

Use NicAudio for loading your MP1/MP2/MP3/MPA/AC3/DTS/LPCM in AviSynth:

Some examples:

::

    LoadPlugin("C:\Program Files\AviSynth25\plugins\NicAudio.dll")

    # AC3 audio:
    V = BlankClip(height=576, width=720, fps=25)
    A = NicAC3Source("D:\audio.AC3")
    # A = NicAC3Source("D:\audio.AC3", downmix=2) # downmix to stereo
    AudioDub(V, A) ::# LPCM audio (48 kHz, 16 bit and stereo):
    V = BlankClip(height=576, width=720, fps=25)
    A = NicLPCMSource("D:\audio.lpcm", 48000, 16, 2)
    AudioDub(V, A)

How do I load aac/flac/ogg files into AviSynth?
-----------------------------------------------

Use ffdshow (set AAC to libfaad or realaac), and use

::

    DirectShowSource("d:\audio.aac")

For WAVE_FORMAT_EXTENSIBLE, ogg, flac, wma, and other formats, `BassAudio and
the correspoding libraries and Add-Ons`_ can be used. Note that
BassAudioSource can decode stereo aac/mp4, but it can't decode multichannel
aac.

Some examples:

::

    bassAudioSource("C:\ab\Dido\001 Here With Me.m4a")

    or

    bassAudioSource("C:\ab\Dido\001 Here With Me.aac")


How do I load pictures into AviSynth?
-------------------------------------

1) Use :doc:`ImageReader <../corefilters/imagesource>` or :doc:`ImageSource <../corefilters/imagesource>` to load your pictures into
   AviSynth (can load the most popular formats, except GIF and animated
   formats). See internal documentation for information.

2) Use the Immaavs plugin for GIF, animated formats and other type of
   pictures.

::

    # single picture:
    immareadpic("x:\path\pic.bmp")

    # animation:
    immareadanim("x:\path\anim.gif")

    # image sequence:
    immareadseq("x:\path\seq%3.3d.png", start=5, stop=89, fps=25,
    textmode=2, posx=50, posy=50)

| :doc:`Main Page <faq_sections>` | :doc:`General Info <faq_general_info>` | **Loading Clips** | :doc:`Loading Scripts <faq_frameserving>` | :doc:`Common Error Messages <faq_common_errors>` | :doc:`Processing Different Content <faq_different_types_content>` | :doc:`Dealing with YV12 <faq_yv12>` | :doc:`Processing with Virtualdub Plugins <faq_using_virtualdub_plugins>` |

$Date: 2011/12/04 15:27:59 $

.. _Gabest's AVI splitter:
    http://sourceforge.net/project/showfiles.php?group_id=205650
.. _here: http://esby.free.fr/
.. _GSpot: http://www.headbands.com/gspot/
.. _DGAVCDec: http://forum.doom9.org/showthread.php?p=959013
.. _rmvb splitter: http://sourceforge.net/projects/guliverkli/
.. _RealAlternative: http://www.free-codecs.com/download/Real_Alternative.htm
.. _WMF SDK v9 for W2K or later for XP/Vista:
    http://msdn.microsoft.com/windowsmedia/downloads/default.aspx
.. _FFmpegSource: http://forum.doom9.org/showthread.php?t=127037
.. _ffdshow: http://ffdshow-tryout.sourceforge.net/
.. _Sonic Foundry Video Editor Wave64 Files or W64:
    http://dotwhat.net/w64/9033/
.. _BassAudio and the correspoding libraries and Add-Ons:
    http://forum.doom9.org/showthread.php?t=108254
