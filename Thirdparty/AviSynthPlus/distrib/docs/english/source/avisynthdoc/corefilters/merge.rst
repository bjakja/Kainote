
Merge / MergeChroma / MergeLuma
===============================

| ``Merge`` (clip1, clip2, float weight=0.5)
| ``MergeChroma`` (clip1, clip2, float weight=1.0)
| ``MergeLuma`` (clip1, clip2, float weight=1.0)

These filters make it possible to merge pixels (or, for YUV clips only, just
the luma or chroma) from one videoclip into another. There is an optional
weighting, so a percentage between the two clips can be specified. ``Merge``
is present in *v2.56*.

*clip1* is the clip that has the appropriate data merged INTO (based on which
filter you use). In the case of ``MergeLuma`` or ``MergeChroma``, that
means that the OTHER channel (chroma or luma respectively) is completely
untouched.

*clip2* is the one from which the pixel data must be taken. In ``MergeChroma``,
this is where the Chroma will be taken from, and vice-versa for
``MergeLuma``. It must be the same colorspace as clip1; i.e. you cannot merge
from a YV12 clip into a YUY2 clip.

The *weight* defines how much influence the new clip should have. Range is 0.0
to 1.0, where 0.0 is no influence and 1.0 will completely overwrite the
specified channel. The default is 0.5 for ``Merge`` and 1.0 for ``MergeChroma``
and ``MergeLuma``. The filter will be slightly slower when a weight other than
0.0, 0.5 or 1.0 is specified.

Also see :ref:`here <multiclip>` for the resulting clip properties.
::

    # Will only blur the Luma channel.
    mpeg2source("c:\apps\avisynth\main.d2v")
    lumvid = Blur(1.0)
    MergeLuma(lumvid)

    # This will do a Spatial Smooth on the Chroma channel
    # that will be mixed 50/50 with the original image.
    mpeg2source("c:\apps\avisynth\main.d2v")
    chromavid = SpatialSmoother(2,3)
    MergeChroma(chromavid,0.5)

    # This will run a temporalsmoother and a soft spatial
    # smoother on the Luma channel, and a more agressive
    # spatial smoother on the Chroma channel.
    # The original luma channel is then added with the
    # smoothed version at 75%. The chroma channel is
    # fully replaced with the blurred version.
    mpeg2source("c:\apps\avisynth\main.d2v")
    luma = TemporalSmoother(2,3)
    luma = luma.Spatialsmoother(3,10,10)
    chroma = Spatialsmoother(3,40,40)
    MergeLuma(luma,0.75)
    MergeChroma(chroma)

    # This will average two video sources.
    avisource("c:\apps\avisynth\main.avi")
    vid2 = avisource("c:\apps\avisynth\main2.avi")
    Merge(vid2)

+-----------+-------------+
| Changelog |             |
+===========+=============+
| v2.56     | added Merge |
+-----------+-------------+

$Date: 2010/04/04 16:46:19 $
