
Info
====

``Info`` (clip)

Present in *v2.5*. It gives info of a clip printed in the left corner of the
clip. The info consists of the duration, colorspace, size, fps, whether it is
field (you applied :doc:`SeparateFields <separatefields>`) or frame based (you didn't apply
``SeparateFields``), whether AviSynth thinks it is bottom (the default in
case of :doc:`AviSource <avisource>`) or top field first, whether there is audio present,
the number of channels, sample type, number of samples and the samplerate. In
* v2.55* a CPU flag is added with supported optimizations.

Example:

::

    AviSource("C:\filename.avi").Info

Results in a video with information in the top left corner:

::

    Frame: 0 of 6035
    Time: 00:00:00:000 of 00:04:01:400
    ColorSpace: YUY2
    Width: 720 pixels, Height: 576 pixels.
    Frames per second: 25.0000 (25/1)
    FieldBased (Separated) Video: NO
    Parity: Bottom Field First
    Video Pitch: 1440 bytes.
    Has Audio: YES
    Audio Channels: 2
    Sample Type: Integer 16 bit
    Samples Per Second: 44100
    Audio length: 10650150 samples. 00:04:01:500
    CPU deteced: x87 MMX ISSE SSE 3DNOW 3DNOW_EXT

+-----------+-----------------------------------------------------------------------+
| Changelog |                                                                       |
+===========+=======================================================================+
| v2.57     | Added time of current frame, total time, numerator and denominator of |
|           | the framerate and audio length.                                       |
+-----------+-----------------------------------------------------------------------+
| v2.55     | Added supported CPU optimizations                                     |
+-----------+-----------------------------------------------------------------------+
| v2.50     | Initial Release                                                       |
+-----------+-----------------------------------------------------------------------+

$Date: 2009/09/12 15:10:22 $
