
RGBAdjust
=========

``RGBAdjust`` (clip, float "r", float "g", float "b", float "a", float "rb",
float "gb", float "bb", float "ab", float "rg", float "gg", float "bg", float
"ag", bool "analyze", bool "dither")

This filter multiplies each color channel with the given value, adds the
given bias offset then adjusts the relevant gamma, clipping the result at 0
and 255. Note that ``RGBAdjust`` (1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1) leaves
the clip untouched.

r (-255.0 - 255.0; default 1.0): This option determines how much red is to be
scaled. For example, a scale of 3.0 multiplies the red channel of each pixel
by 3. Green and blue work similar.
a (-255.0 - 255.0; default 1.0) specifies the scale of the alpha channel. The
alpha channel represents the transparency information on a per-pixel basis.
An alpha value of zero represents full transparency, and a value of 255
represents a fully opaque pixel.

In *v2.56* the bias offsets rb, gb, bb, ab (default 0.0) add a value to the
red, green, blue or alpha channels. For example, rb = 16 will add 16 to the
red pixel values and -32 will subtract 32 from all red pixel values.

Also in *v2.56* the exponents rg, gg, bg, ag(default 1.0) adjust the gamma of
the red, green, blue or alpha channels. For example, rg = 1.2 will brighten
the red pixel values and gg = 0.8 will darken the green pixel values.

In *v2.56* analyze (can be true or false) will print out color statistics on
the screen. There are maximum and minimum values for all channels. There is
an average and a standard deviation for all channels. There is a "loose
minimum" and "loose maximum". The "loose" values are made to filter out very
bright or very dark noise specs creating an artificially low or high minimum
/ maximum (it just means that the amount of red/green/blue of 255/256 of all
pixels is above (under) the loose minimum (maximum)).

*dither* = true/false (false by default): When set to true, `ordered
dithering`_ is applied when doing the adjustment.

Keep in mind ALL the values are not scaled to accomodate changes to one (for
that you should use levels) so doing something like:

::

    RGBAdjust(2, 1, 1, 1)

will get you a whole lot of clipped red. If you WANT a whole lot of clipped
red, there you go - but if you want MORE red without clipping you should do

::

    Levels(0, 1, 255, 0, 128).RGBAdjust(2, 1, 1, 1)

This would scale all the levels (and average lum) by half, then double the
red. Or more compact

::

    RGBAdjust(1.0, 0.5, 0.5, 1.0)

This leaves the red and halves the green and blue.

To invert the alpha channel

::

    RGBAdjust(a=-1.0, ab=255)

Thus alpha pixels values become a=(255-a)

+------------+--------------------------------+
| Changelog: |                                |
+============+================================+
| v2.56      | Added offsets, gamma, analyze. |
+------------+--------------------------------+
| v2.60      | Added dither.                  |
+------------+--------------------------------+

$Date: 2011/12/04 15:28:44 $

.. _ordered dithering: http://avisynth.org/mediawiki/Ordered_dithering
