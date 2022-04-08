
ColorBars / ColorBarsHD
=======================

``ColorBars`` (int "width", int "height", string "pixel_type")

The ``ColorBars`` filter produces a video clip containing SMPTE color bars
(Rec. ITU-R BT.801-1) scaled to any image size. By default, a clip is
produced of 640x480, RGB32 [16,235], 29.97 fps, 1 hour long, alpha channel
opaque.
The ``ColorBarsHD`` filter (added in *v2.60*) produces a video clip
containing SMPTE color bars (Rec. ITU-R BT.709 / `arib std b28 v1.0`_) scaled
to any image size. By default, a clip is produced of 1288x720, YV24, 29.97
fps, 1 hour long.

For both filters, a test tone is also generated. Test tone is a 440Hz sine at
48KHz, 16 bit, stereo. The tone pulses in the RIGHT speaker, being turned on
and off once every second.

For ColorBars the following pixel types are supported: "YUY2" (*v2.56*),
"YV12" (*v2.56*), "YV24" (*v2.60*), or (default) "RGB32". For ColorsBarsHD
only "YV24" is supported.

Note, that for example

::

    ColorBars(pixel_type="YUY2")

is almost equivalent with (the -I and +Q patterns have Y increased to the
minimum legal value for RGB)

::

    ColorBars(pixel_type="RGB32")
    ConvertToYUY2(matrix="PC.601")  # doesn't scale the luma range

When directly generating YUV format data the color transitions are arranged
to occur on a chroma aligned boundary.

The lower part of ColorBars is called the pluge. From left to right it
consists of: -I, white, +Q, black, -4/0/+4 IRE levels and black. The -4/0/4
IRE levels can be used to set the brightness correctly. The -4 IRE and 0 IRE
should have the same brightness, and the +4 IRE should be a little brighter
than -4/0 IRE. The -I/+Q levels are not really interesting, since they are
not used anymore for NTSC (analog TV), but they were used to set the
chrominance correctly. More information about the colorbars and the pluge can
be found `here`_.

+----------+----------------------------------------+
| Changes: |                                        |
+==========+========================================+
| v2.56    | Added pixel_type="YUY2"/"YV12".        |
+----------+----------------------------------------+
| v2.60    || Added pixel_type="YV24" to ColorBars. |
|          || Initial release of ColorBarsHD.       |
+----------+----------------------------------------+

$Date: 2011/04/17 03:58:18 $

.. _arib std b28 v1.0: http://www.arib.or.jp/english/html/overview/img
    /arib_std-b28v1.0_e.pdf
.. _here: http://avisynth.org/mediawiki/ColorBars_theory
