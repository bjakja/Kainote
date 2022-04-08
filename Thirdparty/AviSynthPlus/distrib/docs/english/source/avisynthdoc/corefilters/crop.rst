
Crop / CropBottom
=================

``Crop`` (clip, int left, int top, int width, int height, bool "align")
``Crop`` (clip, int left, int top, int -right, int -bottom, bool "align")
``CropBottom`` (clip, int count, bool align)

``Crop`` crops excess pixels off of each frame.

If your source video has 720x480 resolution, and you want to reduce it to
352x240 for VideoCD, here's the correct way to do it:
::

    # Convert CCIR601 to VCD, preserving the correct aspect ratio
    ReduceBy2
    Crop(4,0,352,240)

See :doc:`colorspace conversion filters <convert>` for limitations when using different color formats.

If a negative value is entered in the width and height these are also treated
as offsets. For example:
::

    # Crop 16 pixels all the way around the picture, regardless of image size:
    Crop(16,16,-16,-16)

In *v2.53* an option *align* (false by default) is added:

Cropping an YUY2/RGB32 image is always mod4 (four bytes). However, when
reading x bytes (an int), it is faster when the read is aligned to a modx
placement in memory. MMX/SSE likes 8-byte alignment and SSE2 likes 16-byte
alignment. If the data is NOT aligned, each read/write operation will be
delayed at least 4 cycles. So images are always aligned to mod16 when they
are created by AviSynth.

If an image has been cropped, they will sometimes be placed unaligned in
memory - "align = true" will copy the entire frame from the unaligned memory
placement to an aligned one. So if the penalty of the following filter is
larger than the penalty of a complete image copy, using "align = true" will
be faster. Especially when it is followed by smoothers.

The alternative ``CropBottom`` syntax is useful for cropping garbage off the
bottom of a clip captured from VHS tape. It removes count lines from the
bottom of each frame.


Memory alignment
----------------

In v2.53 an option *align* (false by default) is added:

Cropping an YUY2/RGB32 image is always mod4 (four bytes). However, when
reading x bytes (an int), it is faster when the read is aligned to a modx
placement in memory. MMX/SSE likes 8-byte alignment and SSE2 likes 16-byte
alignment. If the data is NOT aligned, each read/write operation will be
delayed at least 4 cycles. So images are always aligned to mod16 when they
are created by AviSynth.

If an image has been cropped, they will sometimes be placed unaligned in
memory - "align = true" will copy the entire frame from the unaligned memory
placement to an aligned one. So if the penalty of the following filter is
larger than the penalty of a complete image copy, using "align=true" will be
faster. Especially when it is followed by smoothers.


Crop restrictions
-----------------

In order to preserve the data structure of the different colorspaces, the
following mods should be used. You will not get an error message if they are
not obeyed, but it may create strange artifacts. For a complete discussion on
this, see DataStorageInAviSynth ...


+----------------+------------------+--------------------------------------+
| **Colorspace** | **Width**        | **Height**                           |
+----------------+------------------+-------------------+------------------+
|                |                  | progressive video | interlaced video |
+================+==================+===================+==================+
| RGB            | *no restriction* | *no restriction*  | mod-2            |
+----------------+------------------+-------------------+------------------+
| YUY2           | mod-2            | *no restriction*  | mod-2            |
+----------------+------------------+-------------------+------------------+
| YV12           | mod-2            | mod-2             | mod-4            |
+----------------+------------------+-------------------+------------------+

NOTE: The :doc:`resize functions <resize>` optionally allow fractional pixel cropping of
the input frame, this results in a weighting being applied to the edge pixels
being resized.  These options may be used if the mod-n format dimension
restriction of crop are inconvienient. In sum -- "For cropping off hard
artifacts like VHS head noise or leterbox borders always use Crop. For
extracting a portion of an image and to maintain accurate edge resampling use
the resize cropping parameters." (`Doom9 thread`_)

$Date: 2009/09/12 15:10:22 $

.. _Doom9 thread: http://forum.doom9.org/showthread.php?s=&threadid=91630
