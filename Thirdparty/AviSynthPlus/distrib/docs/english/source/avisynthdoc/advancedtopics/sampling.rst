
Sampling
========

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Color format conversions and the Chroma Upsampling Error
--------------------------------------------------------

The following figures show errors, which are examples of **the Chroma
Upsampling Error**, called this way because the video is upsampled
incorrectly (interlaced YV12 upsampled as progressive or vice versa). As a
result, you will often see gaps on the top and bottom of colored objects and
"ghost" lines floating above or below the objects.

+---------------------------------------------------------------------------------------------------------------------------+
| .. image:: pictures/badchroma.jpg                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------+
| figure 1a: example of interlaced source (YV12) being upsampled as progressive video (YUY2) (from http://zenaria.com/gfx/) |
+---------------------------------------------------------------------------------------------------------------------------+
| .. image:: pictures/goodchroma.jpg                                                                                        |
+---------------------------------------------------------------------------------------------------------------------------+
| figure 1b: the same image with correct chroma upsampling (from http://zenaria.com/gfx/)                                   |
+---------------------------------------------------------------------------------------------------------------------------+
| .. image:: pictures/badchroma2.jpg                                                                                        |
+---------------------------------------------------------------------------------------------------------------------------+
| figure 2a: example of progressive source (YV12) being upsampled as interlaced video (YUY2)                                |
+---------------------------------------------------------------------------------------------------------------------------+
| .. image:: pictures/goodchroma2.jpg                                                                                       |
+---------------------------------------------------------------------------------------------------------------------------+
| figure 2b: the same image with correct chroma upsampling                                                                  |
+---------------------------------------------------------------------------------------------------------------------------+

In this section, it will be shown what causes it, and how to fix it. Where
fixing mean making it less visible, because it's not possible to correct it
completely.

| References:
| [`The Chroma Upsampling Error`_]
| [`The Chroma Upsampling Error - Television and Video Advice`_]


Chroma Upsampling Error (or CUE)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As previously stated, the Chroma Upsampling Error occurs when you convert
from (trully) interlaced YV12 to mostly any other format and the converter
thinks the video is progressive. Or, the other way around, if material is
progressive (or interlaced encoded as progressive), and upsampled as
interlaced. This is however not as bad as the other way around.

When VDub previews your video, it will need to convert it to RGB. Since
AviSynth delivers YV12, it asks the codec (for example XviD or DivX) to
convert YV12 to RGB. The codec however ALWAYS upsamples progressively. Hence
you will get artifacts in VDub preview on interlaced YV12 material. This is
however not present in the YV12 video (or in the resulting encoding). To
confirm this, let AviSynth do the conversion by adding
ConvertToRGB(interlaced=true) at the end of your script.


Correcting video having the Chroma Upsampling Error
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You will have to blur the chroma in some way (leaving the luma intact).

For example (using tomsmocomp.dll):

::

    AviSource(...)
    MergeChroma(TomsMoComp(-1,5,0))

Theoretical Aspects
-------------------

In this section, the chroma placement will be explained, how this is related
to subsampling (RGB -> YUY2 -> YV12) and how the upsampling is done in
AviSynth.

It should also explain in detail why the CUE occurs. To summarize the latter,
the problem is that there is a difference between YV12 progressive and YV12
interlaced, because the chroma is shared vertically between neighboring
pixels.

See also `<http://forum.doom9.org/showthread.php?s=&threadid=52151&highlight=upsampling>`_.


The color formats: RGB, YUY2 and YV12
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to be able to understand how YV12 <-> YUY2 sampling works and why it
matters whether your source is interlaced or progressive, the YV12/YUY2 color
formats will be discussed first. It's not important here how they are stored
in your memory. Information about that can be found in :ref:`Section 2.4 References <References>`.


YUV 4:4:4 color format
::::::::::::::::::::::

The term 4:4:4 denotes that for every four samples of the luminance (Y),
there are four samples each of U and V. Thus each pixel has a luminance value
(Y), a U value (blue difference sample or Cb) and a V value (red difference
sample or Cr). Note, "C" is just a chroma sample (UV-sample).

The layout of a 4:4:4 encoded image looks as follows

+-------------+--------+
| frame       | line   |
+=============+========+
| YC YC YC YC | line 1 |
+-------------+--------+
| YC YC YC YC | line 2 |
+-------------+--------+
| YC YC YC YC | line 3 |
+-------------+--------+
| YC YC YC YC | line 4 |
+-------------+--------+

YUY2 color format
:::::::::::::::::

YUY2 (or YUYV) is a 4:2:2 format. The term 4:2:2 denotes that for every four
samples of the luminance (Y), there are two samples each of U and V, giving
less chrominance (color) bandwidth in relation to luminance. So for each
pixel, it is horizontally sharing UV (chroma) with a neighboring pixel.

The layout of a 4:2:2 encoded image looks as follows

+-------------+--------+
| frame       | line   |
+=============+========+
| YC Y YC Y   | line 1 |
+-------------+--------+
| YC Y YC Y   | line 2 |
+-------------+--------+
| YC Y YC Y   | line 3 |
+-------------+--------+
| YC Y YC Y   | line 4 |
+-------------+--------+

YV12 color format
:::::::::::::::::

For the YV12 color format, there's a difference between progressive and
interlaced. The cause is that chroma values are also shared vertically
between two neighboring lines.

YV12 is a 4:2:0 format. The term 4:2:0 denotes that for every four samples
(two horizontal and two vertical) of the luminance (Y), there is one sample
each of U and V, giving less chrominance (color) bandwidth in relation to
luminance.

**YV12 progressive**

For each pixel, it is horizontally sharing UV (chroma or C) with a
neighboring pixel and vertically sharing UV with the neighboring line (thus
line 1 with line 2, line 3 with 4, etc).

The layout of a progressive 4:2:0 encoded image looks as follows (MPEG 2
scheme - see below)

+-------------+--------+
| frame       | line   |
+=============+========+
| Y_Y_Y_Y     | line 1 |
+-------------+        +
| C___C__     |        |
+-------------+--------+
| Y_Y_Y_Y     | line 2 |
+-------------+        +
|             |        |
+-------------+--------+
| Y_Y_Y_Y     | line 3 |
+-------------+        +
| C___C__     |        |
+-------------+--------+
| Y_Y_Y_Y     | line 4 |
+-------------+        +
|             |        |
+-------------+--------+


**YV12 interlaced**

For each pixel, it is horizontally sharing UV (chroma or C) with a
neighboring pixel and vertically sharing UV with the next to neighboring line
(thus line 1t with line 3t, line 2b with 4b, etc).

The layout of a interlaced 4:2:0 encoded image looks as follows (MPEG 2
scheme - see below)

+-------------+---------+
| frame       | line    |
+=============+=========+
| Y_Y_Y_Y     | line 1t |
+-------------+         +
| C___C__     |         |
+-------------+---------+
| Y_Y_Y_Y     | line 2b |
+-------------+         +
|             |         |
+-------------+---------+
| Y_Y_Y_Y     | line 3t |
+-------------+         +
| C___C__     |         |
+-------------+---------+
| Y_Y_Y_Y     | line 4b |
+-------------+         +
|             |         |
+-------------+---------+


or

+-----------+---------+---------+
| field 1   | field 2 | line    |
+===========+=========+=========+
| Y_Y_Y_Y   |         | line 1t |
+-----------+---------+---------+
| C___C__   |         |         |
+-----------+---------+---------+
|           | Y_Y_Y_Y | line 2b |
+-----------+---------+---------+
|           |         |         |
+-----------+---------+---------+
| Y_Y_Y_Y   |         | line 3t |
+-----------+---------+---------+
|           | C___C__ |         |
+-----------+---------+---------+
|           | Y_Y_Y_Y | line 4b |
+-----------+---------+---------+
|           |         |         |
+-----------+---------+---------+


Subsampling
~~~~~~~~~~~

Subsampling is used to reduce the storage and broadcast bandwidth
requirements for digital video. This is effective for a !YCbCr signal because
the human eye is more sensitive for changes in black and white than for
changes in color. So drastically reducing the color info shows very little
difference. YUY2 and YV12 are examples of reduced color formats.


RGB -> YUY2 conversion
::::::::::::::::::::::

| More about RGB -> YUV color conversions can be found here: :doc:`ColorConversions <color_conversions>`.
| Recall the layout of a 4:4:4 encoded image

+-----------------+--------+
| frame           | line   |
+=================+========+
| YC1 YC2 YC3 YC4 | line 1 |
+-----------------+--------+
| YC1 YC2 YC3 YC4 | line 2 |
+-----------------+--------+
| YC1 YC2 YC3 YC4 | line 3 |
+-----------------+--------+
| YC1 YC2 YC3 YC4 | line 4 |
+-----------------+--------+

In AviSynth, the default mode is using a 1-2-1 kernel to interpolate chroma,
that is

| C1x = (C1+C1+C1+C2)/4 (C1 is used three times, since this is the border)
| C3x = (C2+C3+C3+C4)/4
| C5x = (C4+C5+C5+C6)/4

The 4:2:2 encoded image becomes

+-------------------+--------+
| frame             | line   |
+===================+========+
| Y1C1x Y2 Y3C3x Y4 | line 1 |
+-------------------+--------+
| Y1C1x Y2 Y3C3x Y4 | line 2 |
+-------------------+--------+
| Y1C1x Y2 Y3C3x Y4 | line 3 |
+-------------------+--------+
| Y1C1x Y2 Y3C3x Y4 | line 4 |
+-------------------+--------+

The other mode :doc:`ConvertBackToYUY2 <../corefilters/convert>` uses chroma from the left pixel, thus

+-------------------+--------+
| frame             | line   |
+===================+========+
| Y1C1 Y2 Y3C3 Y4   | line 1 |
+-------------------+--------+
| Y1C1 Y2 Y3C3 Y4   | line 2 |
+-------------------+--------+
| Y1C1 Y2 Y3C3 Y4   | line 3 |
+-------------------+--------+
| Y1C1 Y2 Y3C3 Y4   | line 4 |
+-------------------+--------+

*Note (as with the layout of other formats) the position of the chroma
values, represent the WEIGHT result of the subsampling.*

**YUY2 -> YV12 interlaced conversion**

Recall the layout of a interlaced 4:2:0 encoded image, but with the weights
included:

+---------+---------+------------------------+
| frame   | line    | weights                |
+=========+=========+========================+
| Y_Y_Y_Y | line 1t |                        |
+---------+         +------------------------+
| C___C__ |         || chroma of YUY2_lines  |
|         |         || (0.75)*1t + (0.25)*3t |
+---------+---------+------------------------+
| Y_Y_Y_Y | line 2b |                        |
+---------+         +------------------------+
|         |         |                        |
+---------+---------+------------------------+
| Y_Y_Y_Y | line 3t |                        |
+---------+         +------------------------+
| C___C__ |         || chroma of YUY2_lines  |
|         |         || (0.25)*2b + (0.75)*4b |
+---------+---------+------------------------+
| Y_Y_Y_Y | line 4b |                        |
+---------+         +------------------------+
|         |         |                        |
+---------+---------+------------------------+

or

+---------+---------+---------+------------------------+
| field 1 | field 2 | line    | weights                |
+=========+=========+=========+========================+
| Y_Y_Y_Y |         | line 1t |                        |
+---------+---------+         +------------------------+
| C___C__ |         |         || chroma of YUY2_lines  |
|         |         |         || (0.75)*1t + (0.25)*3t |
+---------+---------+---------+------------------------+
|         | Y_Y_Y_Y | line 2b |                        |
+---------+---------+         +------------------------+
|         |         |         |                        |
+---------+---------+---------+------------------------+
| Y_Y_Y_Y |         | line 3t |                        |
+---------+---------+         +------------------------+
|         | C___C__ |         || chroma of YUY2_lines  |
|         |         |         || (0.25)*2b + (0.75)*4b |
+---------+---------+---------+------------------------+
|         | Y_Y_Y_Y | line 4b |                        |
+---------+---------+         +------------------------+
|         |         |         |                        |
+---------+---------+---------+------------------------+


*Note (as with the layout of other formats) the position of the chroma
values, represent the WEIGHT as a result of the subsampling.*

Thus the chroma is stretched across two luma lines in the same field!

**YUY2 -> YV12 progressive conversion**

Recall the layout of a 4:2:0 encoded image

+---------+--------+-----------------------+
| frame   | line   | weights               |
+=========+========+=======================+
| Y_Y_Y_Y | line 1 |                       |
+---------+        +-----------------------+
| C___C__ |        || chroma of YUY2_lines |
|         |        || (0.5)*1 + (0.5)*2    |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 2 |                       |
+---------+        +-----------------------+
|         |        |                       |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 3 |                       |
+---------+        +-----------------------+
| C___C__ |        || chroma of YUY2_lines |
|         |        || (0.5)*3 + (0.5)*4    |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 4 |                       |
+---------+        +-----------------------+
|         |        |                       |
+---------+--------+-----------------------+


*Note (as with the layout of other formats) the position of the chroma
values, represent the WEIGHT result of the subsampling.*

Thus the chroma is stretched across two luma lines in the same frame!


Upsampling
~~~~~~~~~~


YUY2 conversion -> RGB
::::::::::::::::::::::

Recall the layout of a 4:2:2 encoded image

+-------------------+--------+
| frame             | line   |
+===================+========+
| Y1C1 Y2 Y3C3 Y4   | line 1 |
+-------------------+--------+
| Y1C1 Y2 Y3C3 Y4   | line 2 |
+-------------------+--------+
| Y1C1 Y2 Y3C3 Y4   | line 3 |
+-------------------+--------+
| Y1C1 Y2 Y3C3 Y4   | line 4 |
+-------------------+--------+

The C++ and the ASM code use different sampling methods.

For the C++ code ConvertToRGB uses the same chroma for two RGB pixels using
left point upsampling. Thus the 4:4:4 encoded image becomes

+---------------------+--------+
| frame               | line   |
+=====================+========+
| Y1C1 Y2C1 Y3C3 Y4C3 | line 1 |
+---------------------+--------+
| Y1C1 Y2C1 Y3C3 Y4C3 | line 2 |
+---------------------+--------+
| Y1C1 Y2C1 Y3C3 Y4C3 | line 3 |
+---------------------+--------+
| Y1C1 Y2C1 Y3C3 Y4C3 | line 4 |
+---------------------+--------+

For the ASM code ConvertToRGB the missing chroma samples are interpolated
(using a [1 1] kernel), that is

| C2x = (C1+C3)/2
| C4x = (C3+C5)/2

and the existing chroma samples are just copied.

The 4:4:4 encoded image becomes

+-----------------------+--------+
| frame                 | line   |
+=======================+========+
| Y1C1 Y2C2x Y3C3 Y4C4x | line 1 |
+-----------------------+--------+
| Y1C1 Y2C2x Y3C3 Y4C4x | line 2 |
+-----------------------+--------+
| Y1C1 Y2C2x Y3C3 Y4C4x | line 3 |
+-----------------------+--------+
| Y1C1 Y2C2x Y3C3 Y4C4x | line 4 |
+-----------------------+--------+

The ASM code is the one which is actually used in AviSynth.


YV12 interlaced conversion -> YUY2
::::::::::::::::::::::::::::::::::

In AviSynth, the missing chroma samples are interpolated as follows

+---------+---------+-------------------------+
| frame   | line    | weights                 |
+=========+=========+=========================+
| Y_Y_Y_Y | line 1t | chroma of YV12_lines 1t |
+---------+         +-------------------------+
| C___C__ |         |                         |
+---------+---------+-------------------------+
| Y_Y_Y_Y | line 2b | chroma of YV12_lines 4b |
+---------+         +-------------------------+
|         |         |                         |
+---------+---------+-------------------------+
| Y_Y_Y_Y | line 3t || chroma of YV12_lines   |
|         |         || (0.75)*1t + (0.25)*5t  |
+---------+         +-------------------------+
| C___C__ |         |                         |
+---------+---------+-------------------------+
| Y_Y_Y_Y | line 4b || chroma of YV12_lines   |
|         |         || (0.75)*4b + (0.25)*8b  |
+---------+         +-------------------------+
|         |         |                         |
+---------+---------+-------------------------+
| Y_Y_Y_Y | line 5t || chroma of YV12_lines   |
|         |         || (0.25)*1t + (0.75)*5t  |
+---------+         +-------------------------+
| C___C__ |         |                         |
+---------+---------+-------------------------+
| Y_Y_Y_Y | line 6b || chroma of YV12_lines   |
|         |         || (0.25)*4b + (0.75)*8b  |
+---------+         +-------------------------+
|         |         |                         |
+---------+---------+-------------------------+
| Y_Y_Y_Y | line 7t || chroma of YV12_lines   |
|         |         || (0.75)*5t + (0.25)*9t  |
+---------+         +-------------------------+
| C___C__ |         |                         |
+---------+---------+-------------------------+
| Y_Y_Y_Y | line 8b || chroma of YV12_lines   |
|         |         || (0.75)*8b + (0.25)*12b |
+---------+         +-------------------------+
|         |         |                         |
+---------+---------+-------------------------+

or

+---------+---------+---------+-------------------------+
| field 1 | field 2 | line    | weights                 |
+=========+=========+=========+=========================+
| Y_Y_Y_Y |         | line 1t | chroma of YV12_lines 1t |
+---------+---------+         +-------------------------+
| C___C__ |         |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 2b | chroma of YV12_lines 4b |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+
| Y_Y_Y_Y |         | line 3t || chroma of YV12_lines   |
|         |         |         || (0.75)*1t + (0.25)*5t  |
+---------+---------+         +-------------------------+
|         | C___C__ |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 4b || chroma of YV12_lines   |
|         |         |         || (0.75)*4b + (0.25)*8b  |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+
| Y_Y_Y_Y |         | line 5t || chroma of YV12_lines   |
|         |         |         || (0.25)*1t + (0.75)*5t  |
+---------+---------+         +-------------------------+
| C___C__ |         |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 6b || chroma of YV12_lines   |
|         |         |         || (0.25)*4b + (0.75)*8b  |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+
| Y_Y_Y_Y |         | line 7t || chroma of YV12_lines   |
|         |         |         || (0.75)*5t + (0.25)*9t  |
+---------+---------+         +-------------------------+
|         | C___C__ |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 8b || chroma of YV12_lines   |
|         |         |         || (0.75)*8b + (0.25)*12b |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+

This implementation results in a `chroma shift`_. AviSynth uses a different
interpolation as the one suggested by the mpeg2 specs (perhaps due to speed
issues). The latter is

+---------+---------+---------+-------------------------+
| field 1 | field 2 | line    | weights                 |
+=========+=========+=========+=========================+
| Y_Y_Y_Y |         | line 1t | chroma of YV12_lines 1t |
+---------+---------+         +-------------------------+
| C___C__ |         |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 2b | chroma of YV12_lines 4b |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+
| Y_Y_Y_Y |         | line 3t || chroma of YV12_lines   |
|         |         |         || (5/8)*1t + (3/8)*5t    |
+---------+---------+         +-------------------------+
|         | C___C__ |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 4b || chroma of YV12_lines   |
|         |         |         || (7/8)*4b + (1/8)*8b    |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+
| Y_Y_Y_Y |         | line 5t || chroma of YV12_lines   |
|         |         |         || (1/8)*1t + (7/8)*5t    |
+---------+---------+         +-------------------------+
| C___C__ |         |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 6b || chroma of YV12_lines   |
|         |         |         || (3/8)*4b + (5/8)*8b    |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+
| Y_Y_Y_Y |         | line 7t || chroma of YV12_lines   |
|         |         |         || (5/8)*5t + (3/8)*9t    |
+---------+---------+         +-------------------------+
|         | C___C__ |         |                         |
+---------+---------+---------+-------------------------+
|         | Y_Y_Y_Y | line 8b || chroma of YV12_lines   |
|         |         |         || (7/8)*8b + (1/8)*12b   |
+---------+---------+         +-------------------------+
|         |         |         |                         |
+---------+---------+---------+-------------------------+

**YV12 progressive conversion -> YUY2**

The missing chroma samples are interpolated as follows

+---------+--------+------------------------+
| frame   | line   | weights                |
+=========+========+========================+
| Y_Y_Y_Y | line 1 | chroma of YV12_lines 1 |
+---------+        +------------------------+
| C___C__ |        |                        |
+---------+--------+------------------------+
| Y_Y_Y_Y | line 2 || chroma of YV12_lines  |
|         |        || (0.75)*1 + (0.25)*3   |
+---------+        +------------------------+
|         |        |                        |
+---------+--------+------------------------+
| Y_Y_Y_Y | line 3 || chroma of YV12_lines  |
|         |        || (0.25)*1 + (0.75)*3   |
+---------+        +------------------------+
| C___C__ |        |                        |
+---------+--------+------------------------+
| Y_Y_Y_Y | line 4 || chroma of YV12_lines  |
|         |        || (0.75)*3 + (0.25)*5   |
+---------+        +------------------------+
|         |        |                        |
+---------+--------+------------------------+
| Y_Y_Y_Y | line 5 || chroma of YV12_lines  |
|         |        || (0.25)*3 + (0.75)*5   |
+---------+        +------------------------+
| C___C__ |        |                        |
+---------+--------+------------------------+
| Y_Y_Y_Y | line 6 || chroma of YV12_lines  |
|         |        || (0.75)*5 + (0.25)*7   |
+---------+        +------------------------+
|         |        |                        |
+---------+--------+------------------------+

.. _References:

References
~~~~~~~~~~

| :doc:`ColorSpaces <../FilterSDK/ColorSpaces>`
| [`4:4:4`_] sampling
| [`4:2:2`_] sampling
| [`4:2:0`_] sampling
| [`Chroma Upsampling`_]
| [`Chroma Subsampling Standards`_]

Format-specific sampling
------------------------

MPEG-1 versus MPEG-2 sampling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are two common variants of 4:2:0 sampling. One of these is used in
MPEG-2 (and CCIR-601) video, and the other is used in MPEG-1. **The MPEG-2
scheme is how AviSynth samples 4:2:0 video**, because it completely avoids
horizontal resampling in 4:2:0 <-> 4:2:2 conversions.

The layout of a progressive MPEG-1 4:2:0 encoded image

+---------+--------+-----------------------+
| frame   | line   | weights               |
+=========+========+=======================+
| Y_Y_Y_Y | line 1 |                       |
+---------+        +-----------------------+
| _C__C_  |        || chroma of YUY2_lines |
|         |        || (0.5)*1 + (0.5)*2    |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 2 |                       |
+---------+        +-----------------------+
|         |        |                       |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 3 |                       |
+---------+        +-----------------------+
| _C__C_  |        || chroma of YUY2_lines |
|         |        || (0.5)*3 + (0.5)*4    |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 4 |                       |
+---------+        +-----------------------+
|         |        |                       |
+---------+--------+-----------------------+

The layout of a MPEG-2 4:2:0 encoded image

+---------+--------+-----------------------+
| frame   | line   | weights               |
+=========+========+=======================+
| Y_Y_Y_Y | line 1 |                       |
+---------+        +-----------------------+
| C___C__ |        || chroma of YUY2_lines |
|         |        || (0.5)*1 + (0.5)*2    |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 2 |                       |
+---------+        +-----------------------+
|         |        |                       |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 3 |                       |
+---------+        +-----------------------+
| C___C__ |        || chroma of YUY2_lines |
|         |        || (0.5)*3 + (0.5)*4    |
+---------+--------+-----------------------+
| Y_Y_Y_Y | line 4 |                       |
+---------+        +-----------------------+
|         |        |                       |
+---------+--------+-----------------------+


DV sampling
~~~~~~~~~~~

For completeness, we will mention DV sampling. DV is 4:2:0 (PAL) and 4:1:1
(NTSC). Note, that the sample positioning of the former is different from the
4:2:0 chroma in MPEG-1/MPEG-2!

The layout of a 4:2:0 encoded image (field-based)

+---------------------+--------+
| field               | line   |
+=====================+========+
| YV Y YV Y YV Y YV Y | line 1 |
+---------------------+--------+
| YU Y YU Y YU Y YU Y | line 2 |
+---------------------+--------+
| YV Y YV Y YV Y YV Y | line 3 |
+---------------------+--------+
| YU Y YU Y YU Y YU Y | line 4 |
+---------------------+--------+

The layout of a 4:1:1 encoded image (field-based)

+-------------------+--------+
| field             | line   |
+===================+========+
| YC Y Y Y YC Y Y Y | line 1 |
+-------------------+--------+
| YC Y Y Y YC Y Y Y | line 2 |
+-------------------+--------+
| YC Y Y Y YC Y Y Y | line 3 |
+-------------------+--------+
| YC Y Y Y YC Y Y Y | line 4 |
+-------------------+--------+

Some comments about this formats:

- 4:1:1 is supported natively in AviSynth v2.6.
- DV decoders all output YUY2 or RGB (with the exception of ffdshow when YV12
  is enabled).
- When outputting YUY2/RGB (NTSC), the MainConcept codec duplicates the
  chroma samples instead of interpolating. The [`ReInterpolate411 plugin`_] can
  be used to correct for this, resulting in better quality.


References
~~~~~~~~~~

| [`MSDN: YUV sampling <http://http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwmt/html/YUVFormats.asp>`_] Describes the most common YUV sampling techniques.
| [`DV sampling`_]


4:2:0 Interlaced Chroma Problem (or ICP)
----------------------------------------

In general interlaced content will have static parts. If it is upsampled
correctly using interlaced upsampling, it will still have *chroma problems on
diagonal edges of bright-colored objects in static parts of a frame*. The
reason is that "When the two fields are put back together later by a
deinterlacer (or by your eye and brain, if you watch it on an interlaced TV),
the relatively smooth gradations and contours of each field are broken up by
a slightly different set of gradations and contours from the other field."
(quote from first reference). This is called **the Interlaced Chroma
Problem**. The "solution" is a motion-adaptive upsampler, but such an
AviSynth/VDub filter which attempts to do this doesn't exist yet.

| References:
| [`The 4:2:0 Interlaced Chroma Problem <http://www.hometheaterhifi.com/volume_8_2/dvd-benchmark-special-report-chroma-bug-4-2001.html>`_]
| [`The 4:2:0 Interlaced Chroma Problem - Television and Video Advice <http://members.aol.com/ajaynejr/vidbug2.htm>`_]

$Date: 2010/06/05 14:47:46 $

.. _The Chroma Upsampling Error:
    http://www.hometheaterhifi.com/volume_8_2/dvd-benchmark-special-report-chroma-bug-4-2001.html
.. _Chroma Upsampling:
    http://www.hometheaterhifi.com/volume_8_2/dvd-benchmark-special-report-chroma-bug-4-2001.html
.. _The Chroma Upsampling Error - Television and Video Advice:
    http://members.aol.com/ajaynejr/vidbug2.htm
.. _chroma shift:
    http://forum.doom9.org/showthread.php?p=1294886#post1294886
.. _4:4:4:
    http://www.quantel.com/domisphere/infopool.nsf/HTML/dfb444?OpenDocument
.. _4:2:2:
    http://www.quantel.com/domisphere/infopool.nsf/HTML/dfb422?OpenDocument
.. _4:2:0:
    http://www.quantel.com/domisphere/infopool.nsf/HTML/dfb420?OpenDocument
.. _Chroma Subsampling Standards: http://www.mir.com/DMG/chroma.html
.. _ReInterpolate411 plugin:
    http://mywebpages.comcast.net/trbarry/downloads.htm
.. _DV sampling: http://www.adamwilt.com/pix-sampling.html
