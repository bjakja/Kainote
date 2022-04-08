
AviSynth Clip properties
========================

You can access clip properties in AVS scripts. For example, if the variable
*clip* holds a video clip, then *clip.height* is its height in pixels,
*clip.framecount* is its length in frames, and so on. Clip properties can be
manipulated just like :doc:`script variables <syntax_script_variables>` (see the :doc:`AviSynth Syntax <syntax_ref>` for
more), except that they cannot be l-values in C-terminology.

The full list of properties:

-   Width (clip)

Returns the width of the clip in pixels (type: int).

-   Height (clip)

Returns the height of the clip in pixels (type: int).

-   FrameCount (clip)

Returns the number of frames of the clip (type: int).

-   FrameRate (clip)

Returns the number of frames per seconds of the clip (type: float). The
framerate is internally stored as a ratio though and more about it can be
read :doc:`here[1] <../corefilters/fps>`.

-   FrameRateNumerator (clip) (v2.55)

Returns the numerator of the number of frames per seconds of the clip
(type: int).

-   FrameRateDenominator (clip) (v2.55)

Returns the denominator of the number of frames per seconds of the clip
(type: int).

-   AudioRate (clip)

Returns the sample rate of the audio of the clip (type: int).

-   AudioLength (clip) (v2.51)

Returns the number of samples of the audio of the clip (type: int). Be aware
of possible overflow on very long clips (2^31 samples limit).

-   AudioLengthLo (clip [, int]) (v2.60)

Returns the number of samples of the audio of the clip modulo int. int is
1,000,000,000 by default (type: int).

-   AudioLengthHi (clip [, int]) (v2.60)

Returns the number of samples of the audio of the clip divided by int
(truncated to nearest integer). int is 1,000,000,000 by default (type: int).

-   AudioLengthS (clip) (v2.60)

Returns a string formated with the total number of samples of the audio of
the clip (type: string).

-   AudioLengthF (clip) (v2.55)

Returns the number of samples of the audio of the clip (type: float).

-   AudioDuration (clip) (v2.60)

Returns the duration in seconds of the audio of the clip (type: float).

-   AudioChannels (clip)

Returns the number of audio channels of the clip (type: int).

-   AudioBits (clip)

Returns the audio bit depth of the clip (type: int).

-   IsAudioFloat (clip) (v2.55)

Returns true if the audio format of the clip is float (type: bool).

-   IsAudioInt (clip) (v2.55)

Returns true if the audio format of the clip is an integer type (type: bool).

-   IsRGB (clip)

Returns true if the clip is `RGB`_, false otherwise (type: bool).

-   IsRGB24 (clip) (v2.07)

Returns true if the clip is `RGB24`_, false otherwise (type: bool).

-   IsRGB32 (clip) (v2.07)

Returns true if the clip is `RGB32`_, false otherwise (type: bool).

-   IsYUV (clip) (v2.54)

Returns true if the clip is `YUV`_, false otherwise (type: bool).

-   IsY8 (clip) (2.60)

Returns true if the clip is `Y8`_, false otherwise (type: bool).

-   IsYUY2 (clip)

Returns true if the clip is `YUY2`_, false otherwise (type: bool).

-   IsYV12 (clip) (v2.52)

Returns true if the clip is `YV12`_, false otherwise (type: bool).

-   IsYV16 (clip) (v2.60)

Returns true if the clip is `YV16`_, false otherwise (type: bool).

-   IsYV24 (clip) (v2.60)

Returns true if the clip is `YV24`_, false otherwise (type: bool).

-   IsYV411 (clip) (v2.60)

Returns true if the clip is `YV411`_, false otherwise (type: bool).

-   PixelType (clip) (v2.60)

Returns the name of the pixel format (type: string).

-   IsFieldBased (clip)

Returns true if the clip is field-based (type: bool). What this means is
explained :doc:`here[2] <../advancedtopics/interlaced_fieldbased>`.

-   IsFrameBased (clip)

Returns true if the clip is frame-based (type: bool). What this means is
explained :doc:`here[2] <../advancedtopics/interlaced_fieldbased>`.

-   IsPlanar (clip) (v2.52)

Returns true if the clip is `planar`_, false otherwise (type: bool).

-   IsInterleaved (clip) (v2.52)

Returns true if the clip color format is Interleaved, false otherwise (type:
bool).

-   GetParity (clip, int n)

Returns true if frame n (default 0) is top field of field-based clip, or it
is full frame with top field first of frame-based clip (type: bool).

-   HasAudio (clip) (v2.56)

Returns true if the clip has audio, false otherwise (type: bool).

-   HasVideo (clip) (v2.56)

Returns true if the clip has video, false otherwise (type: bool).

--------

Back to :doc:`AviSynth Syntax <syntax_ref>`.

$Date: 2013/01/06 13:38:34 $

.. _rgb: http://avisynth.org/mediawiki/RGB
.. _rgb24: http://avisynth.org/mediawiki/RGB24
.. _rgb32: http://avisynth.org/mediawiki/RGB32
.. _yuv: http://avisynth.org/mediawiki/YUV
.. _y8: http://avisynth.org/mediawiki/Y8
.. _yuy2: http://avisynth.org/mediawiki/YUY2
.. _yv12: http://avisynth.org/mediawiki/YV12
.. _yv16: http://avisynth.org/mediawiki/YV16
.. _yv24: http://avisynth.org/mediawiki/YV24
.. _yv411: http://avisynth.org/mediawiki/YV411
.. _planar: http://avisynth.org/mediawiki/Planar
