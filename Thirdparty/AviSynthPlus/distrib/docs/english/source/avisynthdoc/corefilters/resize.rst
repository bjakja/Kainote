
BicubicResize / BilinearResize / BlackmanResize / GaussResize / LanczosResize / Lanczos4Resize / PointResize / SincResize / Spline16Resize / Spline36Resize / Spline64Resize
============================================================================================================================================================================

| ``BicubicResize`` (clip, int target_width, int target_height, float
  "b=1./3.", float "c=1./3.", float "src_left", float "src_top", float
  "src_width", float "src_height")
| ``BilinearResize`` (clip, int target_width, int target_height, float
  "src_left", float "src_top", float "src_width", float "src_height")
| ``BlackmanResize`` (clip, int target_width, int target_height, float
  "src_left", float "src_top", float "src_width", float "src_height", int
  "taps=4")
| ``GaussResize`` (clip, int target_width, int target_height, float "src_left",
  float "src_top", float "src_width", float "src_height", float "p=30.0")
| ``LanczosResize`` (clip, int target_width, int target_height, float
  "src_left", float "src_top", float "src_width", float "src_height", int
  "taps=3")
| ``Lanczos4Resize`` (clip, int target_width, int target_height, float
  "src_left", float "src_top", float "src_width", float "src_height")
| ``PointResize`` (clip, int target_width, int target_height, float "src_left",
  float "src_top", float "src_width", float "src_height")
| ``SincResize`` (clip, int target_width, int target_height, float "src_left",
  float "src_top", float "src_width", float "src_height", int "taps=4")
| ``Spline16Resize`` (clip, int target_width, int target_height, float
  "src_left", float "src_top", float "src_width", float "src_height")
| ``Spline36Resize`` (clip, int target_width, int target_height, float
  "src_left", float "src_top", float "src_width", float "src_height")
| ``Spline64Resize`` (clip, int target_width, int target_height, float
  "src_left", float "src_top", float "src_width", float "src_height")


General information
-------------------

From *v2.56* you can use offsets (as in :doc:`Crop <crop>`) for all resizers:

``GaussResize`` (clip, int target_width, int target_height, float "src_left",
float "src_top", float -"src_right", float -"src_top")

For all resizers you can use an expanded syntax which crops before resizing.
The same operations are performed as if you put a Crop before the Resize,
there can be a slight speed difference.

Note the edge semantics are slightly different, Crop gives a hard absolute
boundary, the Resizer filter lobes may extend into the cropped region but not
beyond the physical edge of the image.

Use :doc:`Crop <crop>` to remove any hard borders or VHS head switching noise, using the
Resizer cropping may propagate the noise into the adjacent output pixels. Use
the Resizer cropping to maintain accurate edge rendering when excising a part
of a complete image.

::

    Crop(10,10,200,300).BilinearResize(100,150)

    # which is nearly the same as
    BilinearResize(100,150,10,10,200,300)

Important: AviSynth has completely separate vertical and horizontal resizers.
If the input is the same as the output on one axis, that resizer will be
skipped. Which one is called first, is determined by which one has the
smallest downscale ratio. This is done to preserve maximum quality, so the
second resizer has the best possible picture to work with. Data storing will
have an impact on what modulos that ''should'' be used for sizes when
resizing and cropping, see the :doc:`Crop <crop>` page.


BilinearResize
--------------

The ``BilinearResize`` filter rescales the input video frames to an arbitrary
new resolution. If you supply the optional source arguments, the result is
the same as if you had applied :doc:`Crop <crop>` with those arguments to the clip
before ``BilinearResize``.

``BilinearResize`` uses standard bilinear filtering and is almost identical
to VirtualDub's "precise bilinear" resizing option. It's only "almost"
because VirtualDub's filter seems to get the scaling factor slightly wrong,
with the result that pixels at the top and right of the image get either
clipped or duplicated. (This error is noticeable when expanding the frame
size by a factor or two or more, but insignificant otherwise, so I wouldn't
worry too much about it.)

::

    Examples:
    # Load a video file and resize it to 240x180 (from whatever it was before)
    AVISource("video.avi").BilinearResize(240,180)

    # Load a 720x480 (CCIR601) video and resize it to 352x240 (VCD),
    # preserving the correct aspect ratio
    AVISource("dv.avi").BilinearResize(352, 240, 8, 0, 704, 480)

    # or what is the same
    AviSource("dv.avi").BilinearResize(352, 240, 8, 0, -8, -0)

    # Extract the upper-right quadrant of a 320x240 video and zoom it
    # to fill the whole frame
    BilinearResize(320,240,160,0,160,120)

BicubicResize
-------------

``BicubicResize`` is similar to ``BilinearResize``, except that
instead of a linear filtering function it uses the Mitchell-Netravali two-
part cubic. The parameters *b* and *c* can be used to adjust the properties
of the cubic, they are sometimes referred to as ``blurring`` and ``ringing``,
respectively.

With *b* = 0 and *c* = 0.75 the filter is exactly the same as VirtualDub's
"precise bicubic," and the results are identical except for the VirtualDub
scaling problem mentioned above. The default values are *b* = 1./3. and *c* =
1./3., which were the values recommended by Mitchell and Netravali as
yielding the most visually pleasing results in subjective tests of human
beings. Larger values of *b* and *c* can produce interesting op-art effects
-- for example, try *b* = 0 and *c* = -5.

If you are magnifying your video, you will get much better-looking results
with ``BicubicResize`` than with ``BilinearResize``. However, if you
are shrinking it, you are probably just as well off, or even better off, with
``BilinearResize``. Although VirtualDub's bicubic filter does produce
better-looking images than its bilinear filter, this is mainly because the
bicubic filter sharpens the image, not because it samples it better. Sharp
images are nice to look at--until you try to compress them, at which point
they turn nasty on you very quickly. The ``BicubicResize`` default
doesn't sharpen nearly as much as VirtualDub's bicubic, but it still sharpens
more than the bilinear. If you plan to encode your video at a low bitrate, I
wouldn't be at all surprised if ``BilinearResize`` yields a better
overall final result.

For the most numerically accurate filter constrain b and c such that they
satisfy :-
::

    b + 2 * c = 1 This gives maximum value for c = 0.5 when b = 0.

This is the Catmull-Rom spline. Which is a good suggestion for sharpness.

From c > 0.6 the filter starts to "ring". You won't get real sharpness, what
you'll get is crispening like with a TV set sharpness control. Negative
values are not allowed for b, use b = 0 for values of c > 0.5.


BlackmanResize
--------------

``BlackmanResize`` is a modification of ``LanczosResize`` that has better
control of ringing artifacts for high numbers of taps. See ``LanczosResize``
of an explanation for the taps argument (default: taps=4, 1<=taps<=100).
(added in *v2.58*)


GaussResize
-----------

``GaussResize`` uses a gaussian resizer with adjustable sharpness parameter p
(default 30). p has a range from about 1 to 100, with 1 being very blurry and
100 being very sharp. ``GaussResize`` uses 4 taps and has similar speed as
``Lanczos4Resize``. (added in *v2.56*)


LanczosResize / Lanczos4Resize
------------------------------

``LanczosResize`` is an alternative to ``BicubicResize`` with high
values of c about 0.6 ... 0.75 which produces quite strong sharpening. It
usually offers better quality (fewer artifacts) and a sharp image.

Lanczos was created for AviSynth because it retained so much detail, more so
even than BicubicResize(x,y,0,0.75). As you might know, the more detail a
frame has, the more difficult it is to compress it. This means that Lanczos
is NOT suited for low bitrate video, the various Bicubic flavours are much
better for this. If however you have enough bitrate then using Lanczos will
give you a better picture, but in general I do not recommend using it for 1
CD rips because the bitrate is usually too low (there are exceptions of
course).

The input parameter taps (default 3, 1<=taps<=100) is equal to the number of
lobes (ignoring mirroring around the origin).

``Lanczos4Resize`` (added in *v2.55*) is a short hand for
``LanczosResize(taps=4)``. It produces sharper images than LanczosResize with the
default taps=3, especially useful when upsizing a clip.

*Warning: the input argument named taps should really be lobes. When
discussing resizers, taps has a different meaning, as described below (the
first paragraph concerns LanczosResize(taps=2)):*

"For upsampling (making the image larger), the filter is sized such that the
entire equation falls across 4 input samples, making it a 4-tap filter. It
doesn't matter how big the output image is going to be - it's still just 4
taps. For downsampling (making the image smaller), the equation is sized so
it will fall across 4 *destination* samples, which obviously are spaced at
wider intervals than the source samples. So for downsampling by a factor of 2
(making the image half as big), the filter covers 2*4=8 input samples, and
thus 8 taps. For 3x downsampling, you need 3*4=12 taps, and so forth.

Thus the effective number of taps you get for downsampling is the
downsampling ratio times the number of filter input taps (thus ``**T**x``
downsampling and Lanczos**k**Resize results in ``T*2*k`` taps), this is rounded
up to the next even integer. For upsampling, it's always just ``2*k`` taps."
Source: [`avsforum post`_].


PointResize
-----------

``PointResize`` is the simplest resizer possible. It uses a Point Sampler
or Nearest Neighbour algorithm, which usually results in a very blocky image.
So in general this filter should only be used, if you intend to have
inferiour quality, or you need the clear pixel drawings.
It is very useful for magnifying small areas of pixels for close examination.


Spline16Resize/Spline36Resize/Spline64Resize
--------------------------------------------

Three Spline based resizers  (added in *v2.56/v2.58*).

``Spline16Resize``, ``Spline36Resize`` and ``Spline64Resize`` are
three Spline based resizers. They are the (cubic) spline based resizers from
`Panorama tools`_ that fit a spline through the sample points and then
derives the filter kernel from the resulting blending polynomials. See `this
thread`_ for the details.

The rationale for Spline is to be as sharp as possible with less ringing
artefacts than ``LanczosResize`` produces. ``Spline16Resize`` uses sqrt(16)=4
sample points, ``Spline36Resize`` uses 6 sample points, etc ... The more
sample points that are used, the sharper your clip will be. A comparison
between some resizers is given `here`_.


SincResize
----------

``SincResize`` is added in *v2.6* and it uses the truncated sinc function as
resizer. See LanczosResize for an explanation of the taps argument (default:
taps=4; 1<=taps<=20).

+-----------+-------------------------------------------------------------------------+
| Changelog |                                                                         |
+===========+=========================================================================+
| v2.55     | added Lanczos4Resize                                                    |
+-----------+-------------------------------------------------------------------------+
| v2.56     | added Spline16Resize, Spline36Resize, GaussResize and taps parameter in |
|           | LanczosResize; added offsets in Crop part of xxxResize                  |
+-----------+-------------------------------------------------------------------------+
| v2.58     | added BlackmanResize, Spline64Resize                                    |
+-----------+-------------------------------------------------------------------------+
| v2.6      | added SincResize                                                        |
+-----------+-------------------------------------------------------------------------+

$Date: 2009/09/12 15:10:22 $

.. _avsforum post:
    http://archive2.avsforum.com/avs-vb/showthread.php?s=&postid=4760581#post4760581
.. _Panorama tools: http://www.all-in-one.ee/~dersch/
.. _this thread: http://forum.doom9.org/showthread.php?t=147117
.. _here:
    http://web.archive.org/web/20060827184031/http://www.path.unimelb.edu.au/~dersch/interpolator/interpolator.html
