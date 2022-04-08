
ConvertBackToYUY2 / ConvertToRGB / ConvertToRGB24 / ConvertToRGB32 / ConvertToY8 / ConvertToYUY2 / ConvertToYV12 / ConvertToYV16 / ConvertToYV24 / ConvertToYV411
=================================================================================================================================================================

| ``ConvertToRGB`` (clip [, string "matrix"] [, bool "interlaced"] [, string
  "ChromaInPlacement"] [, string "chromaresample"])
| ``ConvertToRGB24`` (clip [, string "matrix"] [, bool "interlaced"] [, string
  "ChromaInPlacement"] [, string "chromaresample"])
| ``ConvertToRGB32`` (clip [, string "matrix"] [, bool "interlaced"] [, string
  "ChromaInPlacement"] [, string "chromaresample"])
| ``ConvertToYUY2`` (clip [, bool "interlaced"] [, string "matrix"] [, string
  "ChromaInPlacement"] [, string "chromaresample"])``
| ``ConvertBackToYUY2`` (clip [, string "matrix"])
| ``ConvertToY8`` (clip [, string "matrix"])
| ``ConvertToYV411`` (clip [, bool "interlaced"] [, string "matrix"] [, string
  "ChromaInPlacement"] [, string "chromaresample"])
| ``ConvertToYV12`` (clip [, bool "interlaced"] [, string "matrix"] [, string
  "ChromaInPlacement"] [, string "chromaresample"] [, string
  "ChromaOutPlacement"])
| ``ConvertToYV16`` (clip [, bool "interlaced"] [, string "matrix"] [, string
  "ChromaInPlacement"] [, string "chromaresample"])
| ``ConvertToYV24`` (clip [, bool "interlaced"] [, string "matrix"] [, string
  "ChromaInPlacement"] [, string "chromaresample"])


Colorformats
------------

The following formats can be converted to and from.

+--------------+--------------------+------------------------------------------+
| colorformats | planar/interleaved | chroma resolution                        |
+==============+====================+==========================================+
| RGB          | interleaved        | full chroma - 4:4:4                      |
+--------------+--------------------+------------------------------------------+
| RGB24        | interleaved        | full chroma - 4:4:4                      |
+--------------+--------------------+------------------------------------------+
| RGB32        | interleaved        | full chroma - 4:4:4                      |
+--------------+--------------------+------------------------------------------+
| YUY2         | planar             | chroma shared between 2 pixels - 4:2:2   |
+--------------+--------------------+------------------------------------------+
| Y8           | planar/interleaved | no chroma - 4:0:0                        |
+--------------+--------------------+------------------------------------------+
| YV411        | planar             | chroma shared between 4 pixels - 4:1:1   |
+--------------+--------------------+------------------------------------------+
| YV12         | planar             | chroma shared between 2x2 pixels - 4:2:0 |
+--------------+--------------------+------------------------------------------+
| YV16         | planar             | chroma shared between 2 pixels - 4:2:2   |
+--------------+--------------------+------------------------------------------+
| YV24         | planar             | full chroma - 4:4:4                      |
+--------------+--------------------+------------------------------------------+

Options
-------

*matrix*: Default Rec601. Controls the colour coefficients and scaling factors
used in RGB - YUV conversions.

-   "Rec601" : Uses Rec.601 coefficients, scaled to TV range [16,235].
-   "PC.601" : Uses Rec.601 coefficients, keep full range [0,255].
-   "Rec709" : Uses Rec.709 coefficients, scaled to TV range.
-   "PC.709" : Uses Rec.709 coefficients, keep full range.
-   "AVERAGE" : Uses averaged coefficients, keep full range (added in
    *v2.60*). (So the luma becomes the average of the RGB channels.)

*interlaced*: Default false. Use interlaced layout for YV12 chroma conversions.

*ChromaInPlacement* (added in *v2.60*): This determines the chroma placement
when converting from YV12. It can be "MPEG2" (default), "MPEG1" and "DV".

*chromaresample* (added in *v2.60*): This determines which resizer is used in
the conversion. It is used when the chroma resolution of the source and
target is different. It can be all resamplers, default is "bicubic".

*ChromaOutPlacement* (added in *v2.60*): This determines the chroma placement
when converting to YV12. It can be "MPEG2" (default), "MPEG1" and "DV".

AviSynth prior to *v2.50* can deal internally with two color formats, RGB and
YUY2. Starting from v2.50 AviSynth can also deal with a third color format,
YV12. These filters convert between them. If the video is already in the
specified format, it will be passed through unchanged. RGB is assumed
throughout this doc to mean RGBA = RGB32. ``ConvertToRGB`` converts to RGB32
unless your clip is RGB24. If you need 24-bit RGB for some reason, use
``ConvertToRGB24`` explicitly and ``ConvertToRGB32`` to do the reverse.

In *v2.60* the following additional formats are supported:

- Y8 greyscale (it is both planar and interleaved since it contains no chroma; 4:0:0)
- YV411 (planar; YUV 4:1:1)
- YV16 (a planar version of YUY2; 4:2:2) and
- YV24 (planar; YUV 4:4:4).

Syntax and operation of ``ConvertToRGB24`` is identical to ``ConvertToRGB``,
except that the output format is 24-bit; if the source is RGB32, the alpha
channel will be stripped.

Since v2.51/v2.52 an optional interlaced parameter is added
(interlaced*=*false is the default operation). When set to false it is
assumed that clip is progressive, when set to true it is assumed that clip is
interlaced. This option is added because for example (assuming clip is
interlaced YV12):
::

    SeparateFields(clip)
    ConvertToYV12

Weave is upsampled incorrectly. Instead it is better to use:
::

    ConvertToYV12(clip, interlaced=true)

Note, the interlaced=true setting only does something if the conversion
YV12 <-> YUY2/RGB is requested, otherwise it's simply ignored. More about
it can be found here ":doc:`Color conversions and interlaced / field-based video <../advancedtopics/interlaced_fieldbased>`".

When the target format is the same as the source format, the original clip
will be returned. Except for the following situation: When both formats are
YV12, the source clip will be processed when ChromaInPlacement and
ChromaOutPlacement differ. So this can be used to change the chroma placement
in a YV12 clip.


Examples
--------

Contrary to what one might expect, there is no unique way of converting YUV
to RGB. In AviSynth the two most common ones are implemented: Rec.601 and
Rec.709 (named after their official specifications). Although it will not be
correct in all cases, the following shoud be correct in most cases:

The first one (Rec.601) should be used when your source is DivX/XviD or some
analogue capture:
::

    ConvertToRGB(clip)

The second one (Rec.709) should be used when your source is DVD or
HDTV:
::

    ConvertToRGB(clip, matrix="rec709")

In *v2.56*, the reverse is also available, that is

::

    ConvertToYUY2(clip, matrix="rec709")

    or

    ConvertToYV12(clip, matrix="rec709")

In *v2.56*, matrix="pc.601" (and matrix="pc.709") enables you to do the RGB
<-> YUV conversion while keeping the luma range, thus RGB [0,255] <-> YUV
[0,255] (instead of the usual/default RGB [0,255] <-> YUV [16,235]).

All VirtualDub filters (loaded with ``LoadVirtualdubPlugin``, see :ref:`Plugins <syntax-rst-plugins>`)
support only RGB32 input.

**RGB24, RGB32:** The colors are stored as values of red, green and blue. In
RGB32 there is an extra alpha channel for opacity. The image dimensions can
have any values.

**YUY2:** The picture is stored as a luma value Y and two color values U, V.
For two horizontal pixels there is only one chroma value and two luma values
(two Y's, one U, one V). Therefore the width has to be a multiple of two.

**YV8:** Greyscale. Thus the same as YV24 without the chroma planes.

**YV411:** Similar as YV12 but with only one chroma value for 4 pixels (a 1x4
square). The horizontal image dimension has to be a multiple of four.

**YV12:** The same as YUY2 but there is only one chroma value for 4 pixels (a
2x2 square). Both image dimensions have to be a multiple of two, if the video
is interlaced the height has to be a multiple of four because the 2x2 square
is taken from a field, not from a frame.

**YV16:** The same as YUY2 but planar instead of interleaved.

**YV24:** The same as YV12/YV16, but with full chroma.

Some functions check for the dimension rules, some round the parameters,
there still can be some where an picture distortion or an error occurs.

Working in YUY2 is faster than in RGB. YV12 is even faster and is the native
MPEG format, so there are fewer colorspace conversions.

Conversion back and forth is not lossless, so use as few conversions as
possible. If multiple conversions are necessary, use ``ConvertBackToYUY2`` to
convert to YUY2, if your source already has already once been YUY2. This will
reduce colorblurring, but there is still some precision lost.

In most cases, the ``ConvertToRGB`` filter should not be necessary. If
Avisynth's output is in YUY2 format and an application expects RGB, the
system will use the installed YUY2 codec to make the conversion. However, if
there's no installed YUY2 codec, or if (as is the case with ATI's and some
other YUY2 codec) the codec converts from YUY2 to RGB incorrectly, you can
use AviSynth's built-in filter to convert instead.

Conversion paths
----------------

-   The *ChromaInPlacement*, *chromaresample* and *ChromaOutPlacement*
    options are only used in the 'planar conversion part' of the conversion
    path, and they process the chroma of the clip.

In v2.60 the following conversion paths occur

-   YUV planar -> RGB via YV24
-   YUV planar -> YUY2 via YV16 (except for YV12 and parameters
    *ChromaInPlacement*/*chromaresample* not explicitly set", in that case
    there is a direct conversion from YV12 to YUY2)
-   RGB -> YUV planar via YV24
-   YUY2 -> YUV planar via YV16 (except for YV12 and parameters
    *ChromaInPlacement*/*chromaresample* not explicitly set", in that case
    there is a direct conversion from YUY2 to YV12)

Suppose you have a YUY2 clip for example and you convert it to YV24. The YUY2
will be converted to YV16 first without applying *ChromaInPlacement*,
*chromaresample* and *ChromaOutPlacement*. Then YV16 will be converted to
YV24 while applying *chromaresample*. *ChromaInPlacement* and
*ChromaOutPlacement* won't be used since YV12 is not involved.


Sampling
--------

:doc:`This part of the documentation <../advancedtopics/sampling>` covers the sampling methods and color formats in more detail.


Color conversions
-----------------

:doc:`This page <../advancedtopics/color_conversions>` covers the color conversions, "YUV <-> RGB", in more detail.

+----------+-------------------------------------------+
| Changes: |                                           |
+==========+===========================================+
| v2.60    | Added: ConvertToY8, ConvertToYV411,       |
|          | ConvertToYV16, ConvertToYV24,             |
|          | ChromaInPlacement, ChromaOutPlacement and |
|          | chromaresample, matrix="AVERAGE"          |
+----------+-------------------------------------------+
| v2.50    | ConvertToYV12                             |
+----------+-------------------------------------------+

$Date: 2011/12/04 15:28:44 $
