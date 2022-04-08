
SpatialSoften / TemporalSoften
==============================

| ``SpatialSoften`` (clip, int radius, int luma_threshold, int
  chroma_threshold)
| ``TemporalSoften`` (clip, int radius, int luma_threshold, int
  chroma_threshold, int "scenechange", int "mode")

The ``SpatialSoften`` and ``TemporalSoften`` filters remove noise from a
video clip by selectively blending pixels. These filters can work miracles,
and I highly encourage you to try them. But they can also wipe out fine
detail if set too high, so don't go overboard. And they are very slow,
especially with a large value of radius, so don't turn them on until you've
got everything else ready.

``SpatialSoften`` replaces each sample in a frame with the average of all
nearby samples with differ from the central sample by no more than a certain
threshold value. "Nearby" means no more than radius pixels away in the x and
y directions. The threshold used is luma_threshold for the Y (intensity)
samples, and chroma_threshold for the U and V (color) samples.

``TemporalSoften`` is similar, except that it looks at the same pixel in
nearby frames, instead of nearby pixels in the same frame. All frames no more
than radius away are examined. This filter doesn't seem to be as effective as
``SpatialSoften``.

I encourage you to play around with the parameters for these filters to get
an idea of what they do--for example, try setting one of the three parameters
to a very high value while leaving the others low, and see what happens. Note
that setting any of the three parameters to zero will cause the filter to
become a very slow no-op.

``TemporalSoften`` smoothes luma and chroma separately, but ``SpatialSoften``
smoothes only if both luma and chroma have passed the threshold.

The ``SpatialSoften`` filter work only with YUY2 input. You can use the
:doc:`ConvertToYUY2 <convert>` filter if your input is not in YUY2 format.

Note that if you use AviSynth *v2.04* or above, you don't need the
TemporalSoften2 plugin anymore, the built-in TemporalSoften is replaced with
that implementation.

Starting from *v2.50*, two options are added to ``TemporalSoften``:

-   An optional second mode parameter (mode=2): It has a new and better way of
    blending frame and provides better quality. It is also much faster.
    Requires ISSE. mode=1 is default operation, and works as always.
-   An optional scenechange=n parameter: Using this parameter will avoid
    blending across scene changes. 'n' defines the maximum average pixel
    change between frames. Good values for 'n' are between 5 and 30. Requires
    ISSE.

Good initial values: ``TemporalSoften(4,4,8,15,2)``

+---------+----------------------------------------------------------------------+
| Changes |                                                                      |
+=========+======================================================================+
| v2.56   | TemporalSoften working also with RGB32 input (as well as YV12, YUY2) |
+---------+----------------------------------------------------------------------+

$Date: 2007/07/14 18:06:23 $
