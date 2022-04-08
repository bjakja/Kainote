
ColorYUV
========

``ColorYUV`` (clip, float "gain_y", float "off_y", float "gamma_y",
float "cont_y", float "gain_u", float "off_u", float "gamma_u", float "cont_u",
float "gain_v", float "off_v", float "gamma_v", float "cont_v", string "levels",
string "opt", boolean "showyuv", boolean "analyze", boolean "autowhite",
boolean "autogain", boolean "conditional")

Description
-----------

``ColorYUV`` allows many different methods of changing the color and
luminance of your images. ``ColorYUV`` is present in AviSynth v2.5. All
settings for this filter are optional. All values are defaulting to "0".

gain,  off,  gamma and cont can be set independent on each channel.

*gain* is a multiplier for the value, and it stretches the signal up from the
bottom. In order to confuse you, in the filter :doc:`Tweak <tweak>` this setting is
called *contrast*. That means that if the gain is set to 0, it preserves the
values as they are. When gain is 256 all values are multiplied by 2 (twice as
bright). If the gain is 512 all values are multiplied by 3. Thus if gain =
k*256 for some integer k then Y becomes (k+1)*Y (idem for the chroma).
Although it is possible, it doesn't make sense to apply this setting to the
chroma of the signal.

*off (offset)* adds a value to the luma or chroma values. An offset set to 16
will add 16 to the pixel values. An offset of -32 will subtract 32 from all
pixel values.

*gamma* adjusts gamma of the specified channel. A gamma value of 0 is the same
as gamma 1.0. When gamma is set to 256 it is the same as gamma 2.0. Gamma is
valid down to -256, where it is the same as gamma 0.0. Note: gamma for chroma
is not implemented (gamma_u and gamma_v are dummy parameters).

*cont (contrast)* is also multiplier for the value, and it stretches the signal
out from the center. That means that if the contrast is set to 0, it
preserves the values as they are. When the contrast is 256 all values are
multiplied by 2 (twice as bright). If the contrast is 512 all values are
multiplied by 3. Thus if cont = k*256 for some integer k (and zero gain) then
Y becomes Y + k*(Y-128) (idem for the chroma). Although it is possible, it
doesn't make sense to apply this setting to the luma of the signal.

*levels* can be set to either "TV->PC" or "PC->TV". This will perform a range
conversion. Normally YUV values are not mapped from 0 to 255 (PC range), but
a limited range (TV range). This performs conversion between the two formats.
If nothing is entered as parameter, no conversion will be made (default
operation).

*opt* can be either "coring" or "" (nothing, default setting). Specifying
"coring" will clip your YUV values to the valid TV-ranges. Otherwise "invalid
results" will be accepted.

*showYUV* can be true or false. This will overwrite your image with an image
showing all chroma values along the two axes. This can be useful if you need
to adjust the color of your image, but need to know how the colors are
arranged. At the topleft of the image, the chroma values are '16'. At the
right side of the image, U is at maximum. At the bottom of the screen V is at
its maximum. In the middle both chroma is 128 (or grey).

*analyze* can be true or false. This will print out color statistics on the
screen. There are maximum and minimum values for all channels. There is an
average for all channels. There is a "loose maximum" and "loose minimum". The
"loose" values are made to filter out very bright or very dark noise creating
an artificially low or high minimum / maximum.

*autowhite* can be true or false. This setting will use the information from
the analyzer, and attempt to center the color offsets. If you have recorded
some material, where the colors are shifted toward one color, this filter may
help. But be careful - it isn't very intelligent - if your material is a
clear blue sky, autowhite will make it completely grey! If you add "off_u" or
"off_v" parameters at the same time as autowhite, they will not be used!

*autogain* can be true or false. This setting will use the information from the
analyzer, and attempt to create as good contrast as possible. That means, it
will scale up the luma (y) values to match the minimum and maximum values.
This will make it act much as an "autogain" setting on cameras, amplifying
dark scenes very much, while leaving scenes with good contrast alone. Some
places this is also refered to as "autolevels".

*conditional* can be true or false (false by default). When set to false, it
will make ColorYUV ignore any given conditional variables. See the
"Conditional Variables" section for an overview of the conditional variables.

The quantities *saturation*, *contrast* and *brightness* (as in :doc:`Tweak <tweak>` for
example) are connected with quantities in this filter by the following
equations:

| cont_u = cont_v = (sat-1) * 256
| gain_y = (cont-1) * 256
| off_y = bright

A saturation of 0.8 gives for example: cont_u = cont_v = - 0.2 * 256 = -
51.2. Note that in Tweak your YUV values will always be clipped to valid TV-
ranges, but here you have to specify opt="coring".

Conditional Variables
---------------------

The global variables "coloryuv_xxx" with xxx = gain_y, gain_u, gain_v,
bright_y, bright_u, bright_v, gamma_y, contrast_y, contrast_u or contrast_v
are read each frame, and applied. It is possible to modify these variables
using :doc:`FrameEvaluate <conditionalfilter>` or :doc:`ConditionalReader <conditionalreader>`.

For example:

coloryuvoffset.txt:

::

    Type float
    Default 0.0

    I 25 50 0.0 255.0
    R 75 225 128.0
    I 250 275 255.0 0.0

the script:

::

    Colorbars(512,256).ConvertToYV12.Trim(0,299)
    ColorYUV(cont_y=10, conditional=true)
    ConditionalReader("coloryuvoffset.txt", "coloryuv_gain_y", false)
    ShowFrameNumber()

So up to frame 25 gain_y is equal to the default (which is 0.0), for frame 25
up to 50 the gain_y is increased from 0.0 to 255.0, etc ...

There are more examples of conditional modification at the
:doc:`ConditionalReader <conditionalreader>` page.


Examples
--------

::

    # This will adjust gamma for luma, while making luma smaller and chroma U greater:
    ColorYUV(gamma_y=128, off_y=-16, off_u=5)

    #  Shows all colors. Frame 0 luma is 16, frame 1 luma is 17 and so on.
    ColorYUV(showyuv=true)

    #  Recovers visibility on very bad recordings.
    ColorYUV(autogain=true, autowhite=true)

+-----------+--------------------------+
| Changelog |                          |
+===========+==========================+
| v2.6      | Added conditional option |
+-----------+--------------------------+

$Date: 2011/04/29 20:09:50 $
