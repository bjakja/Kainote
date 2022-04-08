
DirectShowSource
================

``DirectShowSource`` (string filename, float "fps", bool "seek", bool
"audio", bool "video", bool "convertfps", bool "seekzero", int "timeout",
string "pixel_type", int "framecount", string "logfile", int "logmask")

``DirectShowSource`` reads the file *filename* using DirectShow, the same
multimedia playback system which Windows Media Player uses. It can read most
formats which Media Player can play, including MPEG, MP3, and QuickTime, as
well as AVI files that ``AVISource`` doesn't support (like DV type 1, or
files using DirectShow-only codecs). Try reading AVI files with ``AVISource``
first, and if that doesn't work then try this filter instead.

There are some caveats:

-   Some decoders (notably MS MPEG-4) will produce upside-down video.
    You'll have to use :doc:`FlipVertical <flip>`.
-   DirectShow video decoders are not required to support frame-accurate
    seeking. In most cases seeking will work, but on some it might not.
-   DirectShow video decoders are not even required to tell you the frame
    rate of the incoming video. Most do, but the ASF decoder doesn't. You
    have to specify the frame rate using the fps parameter, like this:
    ``DirectShowSource`` ("video.asf", fps=15).
-   This version automatically detects the Microsoft DV codec and sets it
    to decode at full (instead of half) resolution. I guess this isn't a
    caveat. :-)
-   Also this version attempts to disable any decoder based
    deinterlacing.

*fps*: This is sometimes needed to specify the framerate of the video. If the
framerate or the number of frames is incorrect (this can happen with asf or
mov clips), use this option to force the correct framerate.

*seek* = true (in *v2.53*): There is full seeking support (available on most
file formats). If problems occur try enabling the seekzero option first, if
seeking still cause problems completely disable seeking. With seeking
disabled the audio stream returns silence and the video stream the last
rendered frame when trying to seek backwards. Note the AviSynth cache may
provide limited access to the previous few frames, beyond that the last frame
rendered will be returned.

*audio* = true (in *v2.53*): There is audio support in DirectShowSource.
DirectShowSource is able to open formats like WAV/DTS/AC3/MP3, provided you
can play them in WMP for example (more exact: provided they are rendered
correctly in graphedit). The channel ordering is the same as in the
[`wave-format-extensible format`_], because the input is always decompressed to WAV.
For more information, see also GetChannel. AviSynth loads 8, 16, 24 and 32
bit int PCM samples, and float PCM format, and any number of channels.

*video* = true (in *v2.52*): When setting it to false, it lets you open the
audio only.

*convertfps* = false (in *v2.56*): When setting it to true, it turns variable
framerate video (vfr) into constant framerate video (cfr) by duplicating or
skipping frames. This is useful when you want to open vfr video (for example
mkv, rmvb, mp4, asf or wmv with hybrid video) in AviSynth. It is most useful
when the fps parameter is set to the least common multiple of the component
vfr rates, e.g. 120 or 119.880.

*seekzero* = false (in *v2.56*): An option to restrict seeking only back to the
beginning. It allows limited seeking with files like unindexed ASF. Seeking
forwards is of course done the hard way (by reading all samples).

*timeout* = 60000 (in milliseconds; 60000 ms = 1 min) (in *v2.56*): To set time
to wait when DirectShow refuses to render. Positive values cause the return
of blank frames for video and silence for audio streams. Negative values
cause a runtime Avisynth exception to be thrown.

*pixel_type* (in *v2.56*): The mnemonic for the videomedia subtype negotiated
for the IPin connection. It can be "YV24", "YV16", "YV12", "YUY2", "AYUV",
"Y41P", "Y411", "ARGB", "RGB32", "RGB24", "YUV", "YUVex", "RGB", "AUTO" or
"FULL". By default, upstream DirectShow filters are free to bid all of their
supported media types in the order of their choice. A few DirectShow filters
get this wrong. The **pixel_type** argument limits the acceptable video
stream subformats for the IPin negotiation. Note the graph builder may add a
format converter to satisfy your request, so make sure the codec in use can
actually decode to your chosen format. The M$ format converter is just
adequate. The "YUV" and "RGB" pseudo-types restrict the negotiation to all
official supported YUV or RGB formats respectively. The "YUVex" also includes
YV24 and YV16 non-standard pixel types. The "AUTO" pseudo-type permits the
negotiation to use all relevant official formats, YUV plus RGB. The "FULL"
pseudo-type includes the YV24 and YV16 non-standard pixel types in addition
to those supported by "AUTO". The full order of preference is YV24, YV16,
YV12, YUY2, AYUV, Y41P, Y411, ARGB, RGB32, RGB24. Many DirectShow filters get
this wrong, which is why it is not enabled by default. The option exists so
you have enough control to encourage the maximum range of filters to serve
your media. (See `discussion`_.)

| The two non-standard pixel types, YV24 and YV16 use the following GUID's
  respectivly :-
| MEDIASUBTYPE_YV24 = {'42VY', 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00,
  0x38, 0x9b, 0x71};
| MEDIASUBTYPE_YV16 = {'61VY', 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00,
  0x38, 0x9b, 0x71};

*framecount* (in *v2.57*): This is sometimes needed to specify the framecount
of the video. If the framerate or the number of frames is incorrect (this can
happen with asf or mov clips), use this option to force the correct number of
frames. If fps is also specified the length of the audio stream is also
adjusted.

*logfile* (in *v2.57*): Use this option to specify the name of a debugging
logfile.

*logmask* = 35 (in *v2.57*): When a logfile is specified, use this option to
select which information is logged.

+-------+-------------------------+
| Value | Data                    |
+=======+=========================+
| 1     | Format Negotiation      |
+-------+-------------------------+
| 2     | Receive samples         |
+-------+-------------------------+
| 4     | GetFrame/GetAudio calls |
+-------+-------------------------+
| 8     | Directshow callbacks    |
+-------+-------------------------+
| 16    | Requests to Directshow  |
+-------+-------------------------+
| 32    | Errors                  |
+-------+-------------------------+
| 64    | COM object use count    |
+-------+-------------------------+
| 128   | New objects             |
+-------+-------------------------+
| 256   | Extra info              |
+-------+-------------------------+
| 512   | Wait events             |
+-------+-------------------------+

Add the values together of the data you need logged. Specify -1 to log
everything. The default, 35, logs Format Negotiation, Received samples and
Errors. i.e 1+2+32

Examples
--------

Opens an avi with the first available RGB format (without audio):

::

    DirectShowSource("F:\TestStreams\xvid.avi",
    \       fps=25, audio=false, pixel_type="RGB")

Opens a DV clip with the MS DV decoder:

::

    DirectShowSource("F:\DVCodecs\Analysis\Ced_dv.avi") # MS-DV

Opens a variable framerate mkv as 119.88 by adding frames (ensuring sync):

::

    DirectShowSource("F:\Guides\Hybrid\vfr_startrek.mkv",
    \       fps=119.88, convertfps=true)

Opens a realmedia (``*.rmvb``) clip:

::

    DirectShowSource("F:\test.rmvb", fps=24, convertfps=true)

Opens a GraphEdit file:

::

    V=DirectShowSource("F:\vid_graph.grf", audio=False) # video only (audio renderer removed)
    A=DirectShowSource("F:\aud_graph.grf", video=False) # audio only
    (video renderer removed)
    AudioDub(V, A)

See below for some audio examples.


Troubleshooting video and audio problems
----------------------------------------

AviSynth will by default try to open only the media it can open without any
problems. If one component cannot be opened it will simply not be added to
the output. This will also mean that if there is a problem, you will not see
the error. To get the error message to the missing component, use audio=false
or video=false and disable the component that is actually working. This way
AviSynth will print out the error message of the component that doesn't work.


RenderFile, the filter graph manager won't talk to me
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is a common error that occurs when DirectShow isn't able to deliver any
format that is readable to AviSynth. Try creating a filter graph manually and
see if you are able to construct a filter graph that delivers any output
AviSynth can open. If not, you might need to download additional DirectShow
filters that can deliver correct material.


The samplerate is wrong
~~~~~~~~~~~~~~~~~~~~~~~

Some filters might have problems reporting the right samplerate, and then
correct this when the file is actually playing. Unfortunately there is no way
for AviSynth to correct this once the file has been opened. Use
:doc:`AssumeSampleRate <assumerate>` and set the correct samplerate to fix this problem.


My sound is choppy
~~~~~~~~~~~~~~~~~~

Unfortunately Directshow is not required to support sample exact seeking.
Open the sound another way, or demux your video file and serve it to AviSynth
another way. Otherwise you can specify "seekzero = true" or "seek = false" as
parameters or use the :doc:`EnsureVBRMP3Sync <ensuresync>` filter to enforce linear access to
the Directshow audio stream.


My sound is out of sync
~~~~~~~~~~~~~~~~~~~~~~~

This can happen especially with WMV, apparently due to variable frame rate
video being returned. Determine what the fps should be and set it explicitly,
and also "ConvertFPS" to force it to remain constant. And :doc:`EnsureVBRMP3Sync <ensuresync>`
reduces problems with variable rate audio.

::

    DirectShowSource("video.wmv", fps=25, ConvertFPS=True)
    EnsureVBRMP3Sync()

My ASF renders start fast and finish slow
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Microsoft in their infinite wisdom chose to implement ASF stream timing in
the ASF demuxer. As a result it is not possible to strip ASF format files any
faster than realtime. This is most apparent when you first start to process
the streams, usually after opening the Avisynth script it takes you a while
to configure your video editor, all this time the muxer is accumulating
*credit* time. When you then start to process your stream it races away at
maximum speed until you catch up to realtime at which point it slows down to
the realtime rate of the source material. This feature makes it impossible to
use Avisynth to reclock 24fps ASF material upto 25fps for direct PAL
playback.


Common tasks
------------

This section will describe various tasks that might not be 100% obvious. :)


Opening GRF files
~~~~~~~~~~~~~~~~~

GraphEdit GRF-files are automatically detected by a .grf filename extension
and directly loaded by DirectShowSource. For AviSynth to be able to connect
to it, you must leave a pin open in GraphEdit of a media types that AviSynth
is able to connect to. AviSynth will not attempt to disconnect any filters,
so it is important that the output type is correct. DirectShowSource only
accepts YV24, YV16, YV12, YUY2, AYUV, Y41P, Y411, ARGB, RGB32 and RGB24 video
formats and 32, 24, 16 and 8 bit PCM and IEEE FLOAT audio formats.

A given GRF-file should only target one of an audio or video stream to avoid
confusion when directshowsource attempts the connection to your open pin(s).
From version 2.57 this single stream restriction is enforced.


Downmixing AC3 to stereo
~~~~~~~~~~~~~~~~~~~~~~~~

There are essentially two ways to do this. The first is to set the downmixing
in the configuration of your AC3 decoder itself, and the second one is to use
the external downmixer of "Trombettworks":

1) Install AC3filter.

a) Open **AC3Filter Config**
On tab "Main" in section "Output format" select "2/0 - stereo".
[Nothing else is needed.]

***-OR-***

b) Open the AC3 file in WMP6.4 and select the file properties. Set the output
of AC3Filter on **2/0 - stereo**. If you want the best possible quality,
select PCM Float as Sample format.

.. image:: pictures/ac3downmix1a.jpg
.. image:: pictures/ac3downmix1b.jpg


Make the following script:
::

    v = Mpeg2Source("e:\movie.d2v")
    a = DirectShowSource("e:\Temp\Test2\test.ac3")
    AudioDub(v,a)

Finally, open the script in vdub and convert the audio stream to MP3
(of course you can also demux the downmixed WAV stream if needed).

2) Register the directshow filter `Channel Downmixer by Trombettworks`_
   (under start -> run):

    *regsvr32 ChannelDownmixer.ax*

Open the AC3 file in WMP6.4 and select the file properties. Set the output of
AC3Filter on **3/2+SW 5.1 channels** (this downmixer can't handle PCM Float,
thus PCM 16 bit is selected here). In the properties of the downmixer, the
number of input and output channels should be detected automatically. Check
whether this is indeed correct.

.. image:: pictures/ac3downmix2a.jpg
.. image:: pictures/ac3downmix2b.jpg


.. image:: pictures/ac3downmix2c.jpg


Make the following script:
::

    v = Mpeg2Source("e:\movie.d2v")
    a = DirectShowSource("e:\Temp\Test2\test.ac3")
    AudioDub(v,a)

Finally, open the script in vdub and convert the audio stream to MP3
(of course you can also demux the downmixed WAV stream if needed).

For some reason, I can't get this to work with DTS streams :(


Windows 7 users
---------------

Windows 7 forces its own DirectShow filters for decoding several audio and
video formats. Changing their merits or physically removing those filters
doesn't help. clsid made the tool "`Win7DSFilterTweaker`_" to change the
preferred filters. However new decoders need to be added each time so it's
not the perfect solution.

+---------+-----------------------------------------------------------+
| Changes |                                                           |
+=========+===========================================================+
| v2.60   | Added pixel_types "YV24", "YV16", "AYUV", "Y41P", "Y411". |
+---------+-----------------------------------------------------------+
| v2.57   || framecount overrides the length of the streams.          |
|         || logfile and logmask specify debug logging.               |
+---------+-----------------------------------------------------------+
| v2.56   || convertfps turns vfr into constant cfr by adding frames  |
|         || seekzero restricts seeking to begining only              |
|         || timeout controls response to recalcitrant graphs         |
|         || pixel_type specifies/restricts output video pixel format |
+---------+-----------------------------------------------------------+

$Date: 2013/01/25 02:45:53 $

.. _wave-format-extensible format:
    http://www.cs.bath.ac.uk/~jpff/NOS-DREAM/researchdev/wave-ex/wave_ex.html
.. _discussion: http://forum.doom9.org/showthread.php?t=143321
.. _Channel Downmixer by Trombettworks:
    http://www.trombettworks.com/directshow.php
.. _Win7DSFilterTweaker: http://forum.doom9.org/showthread.php?t=146910
