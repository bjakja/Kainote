
HorizontalReduceBy2 / VerticalReduceBy2 / ReduceBy2
===================================================

| ``HorizontalReduceBy2`` (clip)
| ``VerticalReduceBy2`` (clip)
| ``ReduceBy2`` (clip)

``HorizontalReduceBy2`` reduces the horizontal size of each frame by half,
and ``VerticalReduceBy2`` reduces the vertical size by half. Chain them
together (in either order) to reduce the whole image by half. You can also
use the shorthand ``ReduceBy2``, which is the same as ``HorizontalReduceBy2``
followed by ``VerticalReduceBy2``.

The filter kernel used is (1/4,1/2,1/4), which is the same as in VirtualDub's
"2:1 reduction (high quality)" filter. This avoids the aliasing problems that
occur with a (1/2,1/2) kernel. VirtualDub's "resize" filter uses a third,
fancier kernel for 2:1 reduction, but I experimented with it and found that
it actually produced slightly worse-looking MPEG files--presumably because it
sharpens edges slightly, and most codecs don't like sharp edges.

If your source video is interlaced, the ``VerticalReduceBy2`` filter will
deinterlace it (by field blending) as a side-effect. If you plan to produce
output video in a size like 320x240, I recommend that you capture at full
interlaced vertical resolution (320x480) and use ``VerticalReduceBy2``. You
will get much better-looking output. My Huffyuv utility will compress
captured video about 2:1, losslessly, so you can capture 320x480 in about the
same space as it used to take to capture 320x240. (If your disk has the
capacity and throughput to support it, you can even capture at 640x480 and
use both ``HorizontalReduceBy2`` and ``VerticalReduceBy2``. But this won't
improve the quality as much, and if you have to go to MotionJPEG to achieve
640x480, you're probably better off with Huffyuv at 320x480.)

Note that, it's a quick and dirty filter (performance related compromise).
Unlike the standard :doc:`resize <resize>` filters, the ``ReduceBy2`` filters do not
preserve the position of the image center. It shifts color planes by half of
pixel. In fact, ``ReduceBy2()`` is equivalent to:

``BilinearResize(Width/2, Height/2, 0.5, -0.5)`` for RGB,

``MergeChroma(BilinearResize(Width/2,Height/2,0.5,0.5),BilinearResize(Width/2
,Height/2,1.0,1.0))`` for YV12,

``MergeChroma(BilinearResize(Width/2,Height/2,0.5,0.5),BilinearResize(Width/2
,Height/2,1.0,0.5))`` for YUY2.

$Date: 2008/12/24 19:19:07 $
