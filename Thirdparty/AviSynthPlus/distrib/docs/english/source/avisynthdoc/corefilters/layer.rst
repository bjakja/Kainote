
Layer
=====


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Layer
-----

This filter works with *yuy2, rgb32*

``Layer`` (base_clip, overlay_clip, string "op", int "level", int "x", int
"y", int "threshold", bool "use_chroma")

This filter can overlay two clips of different sizes (but with the same color
format) using different operation modes.
For pixel-wise transparency information the 4th color channel of RGB32 (A- or
alpha-channel) is used as a mask.

*Base_clip*: the underlying clip which determines the size and all other video
and audio properties of the result.

*Overlay_clip*: the clip which is merged onto clip. This clip can contain an
alpha layer.

*op*: the performed merge operation, which can be: "add", "subtract",
"lighten", "darken", "fast", "mul"

*level*: 0-257, the strength of the performed operation. 0: the base_clip is
returned unchanged, 257 (256 for YUY2): the maximal strength is used

*x*, *y*: offset position of the overlay_clip

*threshold*: only implemented for "lighten" and "darken"

*use_chroma*: use chroma of the overlay_clip, default=true. When false only
luma is used.

There are some differences in the behaviour and the allowed parameter
depending on the color format and the operation, here are the details:

-   There is no mask (alpha-channel) in YUY2, so the alpha-channel is
    assumed to be fully opaque everywhere.

-   in RGB32 the alpha-channel of the overlay_clip is multiplied with
    level, so the resulting alpha = (alpha_mask * level + 1) / 256. This
    means for full strength of operation alpha has to be 255 and level has to
    be 257.

These operators behave equally for RGB32 or YUY2:

::

    "fast": *use_chroma* must be TRUE, *level* and *threshold* is not used.
            The result is simply the average of *base_clip* and
            *overlay_clip*.

    "add":  *threshold* is not used. The difference between base_clip and
            overlay_clip is multiplied with alpha and added to
            the base_clip.
              alpha=0 -> only base_clip visible,
              alpha=128 -> base and overlay equally blended,
              alpha=255 -> only overlay visible.
        Formula used :-
          RGB32 :: base += ((overlay-base)*(alpha*level+1)>>8)>>8
          YUY2  :: base += ((overlay-base)*level)>>8

    "subtract": the same as add, but the overlay_clip is inverted before.

These operators seem to work correctly only in YUY2:

::

    "mul": *threshold* is not used. The base_clip is colored
           as overlay_clip, so *use_chroma* should be TRUE.
           alpha=0 -> only base_clip visible, alpha=255 -> approx.
           the same Luminance as Base but with the colors of
           Overlay

    "lighten": *use_chroma* must be TRUE. Performs the same operation
               as "add", but only when the result is BRIGHTER
               than the base the new values are used. With a higher
               *threshold* the operation is more likely, so with
               *threshold*=255 it's the same as "add", with threshold=0
               the base_clip is more likely passed unchanged, depending
               on the difference between base_clip and overlay_clip.

    "darken": the same as "lighten", but it is performed only
              when the result is DARKER than the base.

Also see :ref:`here <multiclip>` for the resulting clip properties.


Examples
~~~~~~~~

This can be used to combine two captures of different broadcasts for reducing
noise. A discussion of this idea can be found `in this thread`_. A sample script (of
course you have to ensure that the frames of the two clips match exactly --
use :doc:`DeleteFrame <deleteframe>` if necessary):

::

    clip1 = AviSource("F:\shakira-underneath_your_clothes.avi").ConvertToYUY2
    clip2 = AviSource("F:\shakira-
    underneath_your_clothes2.avi").ConvertToYUY2
    return Layer(clip1, clip2, "fast")

Mask
----

This filter works with *rgb32*

``Mask`` (clip, mask_clip)

Applies a defined alpha-mask to clip, for use with ``Layer``, by converting
mask_clip to greyscale and using that for the mask (the alpha-channel) of
RGB32. In this channel "black" means completely transparent and white means
completely opaque.

For those of you who familiar with Photoshop masks, the concept is the same.
In fact you can create a black and white photo in Photoshop, load it in your
script and use it as a mask.

Here is an example; ss.jpg is derived from a snapshot from a video clip,
which served as a guideline to create the mask just using Paint. We use
Imagesource to load the image in the script and Mask to apply it.

::

    bg = AviSource("01gray.avi").ConvertToRGB32()       # here is the background clip
    mk = Imagesource("ss.jpg").ConvertToRGB32()         # load the image
    top = AviSource("k3.avi").ConvertToRGB32().Mask(mk) # load the top layer clip and apply the mask to it
    Layer(bg, top)                                      # layer the background and the top layer clip with the mask

ResetMask
---------

This filter works with *rgb32*

``ResetMask`` (clip)

Applies an "all-opaque" (that is white) alpha-mask to *clip*, for use with
``Layer``.

The alpha-channel of a RGB32-clip is not always well-defined (depending on
the source), this filter is the faster way to apply an all white mask:

::

    clip = ResetMask(clip)

ColorKeyMask
------------

This filter works with *rgb32*

``ColorKeyMask`` (clip, int color[, int tolB, int tolG, int tolR])

Clears pixels in the alpha-channel by comparing the color (default black).
Each pixel with a color differing less than (tolB, tolR, tolG) (default 10)
is set to transparent (that is black), otherwise it is left unchanged i.e. It
is NOT set to opaque (that it is not set to white, that's why you might need
ResetMask before applying this filter), this allows a aggregate mask to be
constructed with multiple calls. When tolR or tolG are not set, they use the
value from tolB (which reflects the old behaviour). Normally you start with a
ResetMask, then chain a few calls to ColorKeyMask to cause transparent holes
where each color of interest occurs. **See :doc:`Overlay <overlay>` for examples.**

For AviSynth versions older than v2.58, there were no separate tolerance
levels for blue, green and red. There was only one tolerance level called
tolerance and was used for blue, green and red simultaneously.

$Date: 2012/03/18 15:25:39 $

.. _in this thread: http://forum.doom9.org/showthread.php?s=&threadid=28438
