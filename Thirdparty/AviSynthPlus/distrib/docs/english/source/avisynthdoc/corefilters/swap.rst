
SwapUV / UToY / VToY / YToUV / UToY8 / VToY8
============================================

| ``SwapUV`` (clip)
| ``UToY`` (clip)
| ``VToY`` (clip)
| ``YToUV`` (clipU, clipV [, clipY])
| ``UToY8`` (clip)
| ``VToY8`` (clip)

These four filters are available starting from *v2.5*. The last two from
*v2.60*.

``SwapUV`` swaps chroma channels (U and V) of a clip. Sometimes the colors
are distorted (faces blue instead of red, etc) when loading a `DivX`_ or
`MJPEG`_ clip in AviSynth v2.5, due to a bug in the decoders. You can use
this filter to correct it.

``UToY`` copies chroma U plane to Y plane. All color (chroma) information is
removed, so the image is now greyscale. Depending on the YUV format, the
image resolution can be changed.

``VToY`` copies chroma V plane to Y plane. All color (chroma) information is
removed, so the image is now greyscale. Depending on the YUV format, the
image resolution can be changed.

``YToUV`` puts the luma channels of the two clips as U and V channels. Luma
is now 50% grey. Starting from v2.51 there is an optional argument clipY
which puts the luma channel of this clip as the Y channel. Depending on the
YUV format, the image resolution can be changed.

Starting from *v2.53* they also work in YUY2.

| ``UToY8`` (added in v2.60) is a shorthand for UToY.ConvertToY8, and
| ``VToY8`` (added in v2.60) is a shorthand for VToY.ConvertToY8, but faster.

| Starting from v2.53, YUY2 is supported.
| Starting from v2.60, Y8, YV411, YV16, YV24 are supported.

**Example:**
::

    # Blurs the U chroma channel
    video = Colorbars(512, 512).ConvertToYV12
    u_chroma = UToY(video).blur(1.5)
    YtoUV(u_chroma, video.VToY)
    MergeLuma(video)

$Date: 2012/04/15 14:12:41 $

.. _DivX: http://avisynth.org/mediawiki/DivX
.. _MJPEG: http://avisynth.org/mediawiki/MJPEG
