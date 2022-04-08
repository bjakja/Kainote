
AVISource / OpenDMLSource / AVIFileSource / WAVSource
=====================================================

| ``AVISource`` (string filename [, ...], bool "audio" = true, string
  "pixel_type" = "FULL", [string fourCC])
| ``OpenDMLSource`` (string filename [, ...], bool "audio" = true, string
  "pixel_type" = "FULL", [string fourCC])
| ``AVIFileSource`` (string filename [, ...], bool "audio" = true, string
  "pixel_type" = "FULL", [string fourCC])
| ``WAVSource`` (string filename [, ...])

``AVISource`` takes as argument one or more file name in quotes, and reads in
the file(s) using either the Video-for-Windows "AVIFile" interface, or
AviSynth's built-in OpenDML code (taken from VirtualDub). This filter can
read any file for which there's an AVIFile handler. This includes not only
AVI files but also WAV files, AVS (AviSynth script) files, and VDR
(VirtualDub frameserver) files. If you give multiples filenames as arguments,
the clips will be spliced together with :doc:`UnalignedSplice <splice>`. The bool
argument is optional and defaults to ``true``.

The ``AVISource`` filter examines the file to determine its type and passes
it to either the AVIFile handler or the OpenDML handler as appropriate. In
case you have trouble with one or the other handler, you can also use the
``OpenDMLSource`` and ``AVIFileSource`` filters, which force the use of one
or the other handler. Either handler can read ordinary (< 2GB) AVI files, but
only the OpenDML handler can read larger AVI files, and only the AVIFile
handler can read other file types like WAV, VDR and AVS. There is built-in
support for ACM (Audio Compression Manager) audio (e.g. mp3-AVIs).

``WAVSource`` can be used to open a WAV file, or the audio stream from an AVI
file. This can be used, for example, if your video stream is damaged or its
compression method is not supported on your system.

The pixel_type parameter (default "YV12" allows you to choose the output
format of the decompressor. Valid values are "YV12", "YV411", "YV16", "YV24",
"YUY2", "Y8", "RGB32" and "RGB24". If omitted, AviSynth will use the first
format supported by the decompressor (in the following order: YV12, YV411,
YV16, YV24, YUY2, Y8, RGB32 and RGB24). This parameter has no effect if the
video is in an uncompressed format, because no decompressor will be used in
that case. To put it in different words: if you don't specify something it
will try to output the AVI as YV12, if that isn't possible it tries YV411 and
if that isn't possible it tries YV16, etc ...

The pixel_type parameter allows you to choose the output format of the
decompressor. Valid values are "YV24", "YV16", "YV12", "YV411", "YUY2",
"RGB32", "RGB24", "Y8", "AUTO" and "FULL" (default value). If omitted or set
to "FULL", AviSynth will use the first format supported by the decompressor
(in the following order: YV24, YV16, YV12, YV411, YUY2, RGB32, RGB24 and Y8).
If set to "AUTO", AviSynth will use the old ordering: YV12, YUY2, RGB32,
RGB24 and Y8. This parameter has no effect if the video is in an uncompressed
format, because no decompressor will be used in that case. To put it in
different words: if you don't specify something it will try to output the AVI
as YV24, if that isn't possible it tries YV16 and if that isn't possible it
tries YV12, etc ...

Sometimes the colors will be distorted when loading a DivX clip in AviSynth
v2.5 (the chroma channels U and V are swapped), due to a bug in DivX (5.02
and older). You can use :doc:`SwapUV <swap>` to correct it.

From *v2.53* ``AVISource`` can also open DV type 1 video input (only video,
not audio).

From *v2.55*, an option  fourCC is added. FourCC, is a FOUR Character Code in
the beginning of media file, mostly associated with avi, that tells what
codec your system should use for decoding the file. You can use this to force
AviSource to open the avi file using a different codec. A list of FOURCCs can
be found `here`_. By default, the fourCC of the avi is used.

Some MJPEG/DV codecs do not give correct CCIR 601 compliant output when using
``AVISource``. The problem could arise if the input and output colorformat of
the codec are different. For example if the input colorformat is YUY2, while
the output colorformat is RGB, or vice versa. There are two ways to resolve
it:

1) Force the same output as the input colorformat. Thus for example (if
   the input is RGB):

::

    AVISource("file.avi", pixel_type="RGB32")

2) Correct it with the filter :doc:`ColorYUV <coloryuv>`:

::

    AVISource("file.avi").ColorYUV(levels="PC->TV")

Some reference threads:
`MJPEG codecs`_
`DV codecs`_

**Examples:**

::

    # C programmers note: backslashes are not doubled; forward slashes work too
    AVISource("d:\capture.avi")
    AVISource("c:/capture/00.avi")
    WAVSource("f:\soundtrack.wav")
    WAVSource("f:/soundtrack.wav")

    # the following is the same as AVISource("cap1.avi") +
    AVISource("cap2.avi"):
    AVISource("cap1.avi", "cap2.avi")

    # disables audio and request RGB32 decompression
    AVISource("cap.avi", false, "RGB32")

    # opens a DV using the Canopus DV Codec
    AviSource("cap.avi", false, fourCC="CDVC")

    # opens an avi (for example DivX3) using the XviD Codec
    AviSource("cap.avi", false, fourCC="XVID")

    # splicing two clips where one of them contains no audio.
    # when splicing the clips must be compatible (have the same video and
    audio properties):
    A = AviSource("FileA.avi")
    B = AviSource("FileB.avi") # No audio stream
    A ++ AudioDub(B, BlankClip(A))

Some compression formats impose a limit to the number of AviSource() calls
that can be placed in a script. Some people have experienced this limit with
fewer than 50 AviSource() statements. See `discussion`_.

+----------+---------------------------------------------+
| Changes: |                                             |
+==========+=============================================+
| v2.60    | Added new color formats, "AUTO" and "FULL". |
+----------+---------------------------------------------+
| v2.55    | Added fourCC option.                        |
+----------+---------------------------------------------+

$Date: 2012/05/03 07:32:20 $

.. _here: http://www.fourcc.org/index.php?http%3A//www.fourcc.org/codecs.php
.. _MJPEG codecs: http://forum.doom9.org/showthread.php?s=&postid=330657
.. _DV codecs: http://forum.doom9.org/showthread.php?s=&threadid=58110
.. _discussion: http://forum.doom9.org/showthread.php?t=131687
