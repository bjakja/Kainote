
Levels
======

``Levels`` (clip, int input_low, float gamma, int input_high, int output_low,
int output_high, bool "coring", bool "dither")

The ``Levels`` filter adjusts brightness, contrast, and gamma (which must be
> 0). The input_low and input_high parameters determine what input pixel
values are treated as pure black and pure white, the output_low and
output_high parameters determine the output values corresponding to pure
black and white and the gamma parameter controls the degree of nonlinearity
in the conversion. To be precise, the conversion function is:


> output = [(input - input_low) / (input_high - input_low)]1/gamma
(output_high - output_low) + output_low

This is one of those filters for which it would really be nice to have a GUI.
Since I can't offer a GUI (at least not in AviSynth's current form), I
decided I could at least make this filter compatible with VirtualDub's when
the clip is RGB. In that case you should be able to take the numbers from
VirtualDub's Levels dialog and pass them as parameters to the ``Levels``
filter and get the same results. However, the input and output parameters can
be larger than 255.

When processing data in YUV mode, ``Levels`` only gamma-corrects the luma
information, not the chroma. Gamma correction is really an RGB concept, and I
don't know how to do it properly in YUV. However, if gamma = 1.0, the filter
should have the same effect in RGB and YUV modes. For adjusting brightness or
contrast it is better to use :doc:`Tweak <tweak>` or :doc:`ColorYUV <coloryuv>`, because ``Levels`` also
changes the chroma of the clip.

In *v2.53* an optional coring = true/false (true by default, which reflects
the behaviour in older versions) is added.

*coring* = true/false (true by default): When set to true: input luma is
clamped to [16,235] (and the chroma to [16,240]), this result is *scaled*
from [16,235] to [0,255], the conversion takes place according to the formula
above, and output is *scaled* back from [0,255] to [16,235]. When set to
false, the conversion takes place according to the formula above.

*dither* = true/false (false by default): When set to true, `ordered
dithering`_ is applied when doing the adjustment.

::

    # does nothing on a [16,235] clip, but it clamps (or rounds) a [0,255] clip to [16,235]:
    Levels(0, 1, 255, 0, 255)

    # the input is scaled from [16,235] to [0,255],
    # the conversion [0,255]->[16,235] takes place (accordingly to the formula),
    # and the output is scaled back from [0,255] to [16,235]:
    # (for example: the luma values in [0,16] are all converted to 30)
    Levels(0, 1, 255, 16, 235)

    # gamma-correct image for display in a brighter environment:
    # example: luma of 16 stays 16, 59 is converted to 79, etc.
    Levels(0, 1.3, 255, 0, 255)

    # invert the image (make a photo-negative):
    # example: luma of 16 is converted to 235
    Levels(0, 1, 255, 255, 0)

    # does nothing on a [0,255] clip; does nothing on a [16,235]:
    Levels(0, 1, 255, 0, 255, coring=false)

    # scales a [0,255] clip to [16,235]:
    Levels(0, 1, 255, 16, 235, coring=false)
    # note both luma and chroma components are scaled by the same
    # amount, so it's not exactly the same as ColorYUV(levels="PC->TV")

    # scales a [16,235] clip to [0,255]:
    Levels(16, 1, 235, 0, 255, coring=false)
    # note both luma and chroma components are scaled by the same
    # amount, so it's not exactly the same as ColorYUV(levels="TV->PC")

    # makes a clip 100% black
    Levels(0, 1.0, 255, 0, 0)

+---------+--------------+
| Changes |              |
+=========+==============+
| v2.53   | added coring |
+---------+--------------+
| v2.60   | added dither |
+---------+--------------+

$Date: 2012/04/09 08:19:32 $

.. _ordered dithering: http://avisynth.org/mediawiki/Ordered_dithering
