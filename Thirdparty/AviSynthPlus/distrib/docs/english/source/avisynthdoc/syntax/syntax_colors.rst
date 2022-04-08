
AviSynth - Colors
=================

In some filters (`BlankClip`_, `Letterbox`_, `AddBorders`_ and `FadeXXX`_) a
color argument can be specified. In all cases the color should be specified
in `RGB`_ format even if the colorformat of the input clip is `YUV`_. This
can be done in hexadecimal or decimal notation.

In **hexadecimal notation** the number is composed as follows: the first two
digits denote the red channel, the next two the green channel and the last
two the blue channel. The hexadecimal number must be preceded with a $.

In **decimal notation** the number is as follows: the red channel value is
multiplied by 65536, the green channel value is multiplied by 256 and the two
resulting products are both added to the blue channel value.

Let's consider an example. Brown is given by R=$A5 (165), G=$2A (42), B=$2A
(42). Thus

::

    BlankClip(color=$A52A2A)

gives a brown frame. Converting each channel to decimal (remember that A=10,
B=11, C=12, D=14, E=14, F=15) gives

::

    R = $A5 = 10*16^1 +  5*16^0 = 165
    G = $2A =  2*16^1 + 10*16^0 =  42
    B = $2A =  2*16^1 + 10*16^0 =  42

    165*65536 + 42*256 + 42 = 10824234

Thus creating a brown frame specifying the color in decimal notation gives

::

    BlankClip(color=10824234)

Common color presets can be found in the file colors_rgb.avsi, which should
be present in your plugin autoload folder (look into the file for list of
presets). Thus BlankClip(color=color_brown) gives the same brown frames.

Note that black RGB=$000000 will be converted to Y=16, U=V=128 if the
colorformat of the input clip is YUV, since the default color conversion RGB
[0,255] -> YUV [16,235] is used.

$Date: 2010/02/27 14:44:45 $

.. _BlankClip: http://avisynth.org/mediawiki/BlankClip
.. _Letterbox: http://avisynth.org/mediawiki/Letterbox
.. _AddBorders: http://avisynth.org/mediawiki/AddBorders
.. _FadeXXX: http://avisynth.org/mediawiki/Fade
.. _RGB: http://avisynth.org/mediawiki/RGB
.. _YUV: http://avisynth.org/mediawiki/YUV
