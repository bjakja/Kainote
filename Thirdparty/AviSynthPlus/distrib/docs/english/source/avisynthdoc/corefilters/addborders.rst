
AddBorders
==========

``AddBorders`` (clip, int left, int top, int right, int bottom, int "color")

``AddBorders`` adds black borders around the image, with the specified widths
(in pixels).

See :doc:`colorspace conversion filters <convert>` for constraints when using the different
color formats.

The color parameter is optional (added in v2.07), default=0 <black>, and is
specified as an RGB value regardless of whether the clip format is RGB or YUV
based. Color presets can be found in the file colors_rgb.avsi, which should
be present in your plugin folder. See :doc:`here <../syntax/syntax_colors>` for more information on
specifying colors.

Be aware that many lossy compression algorithms don't deal well with solid-
color borders, unless the border width happens to be a multiple of the block
size (16 pixels for MPEG).

You can use this filter in combination with :doc:`Crop <crop>` to shift an image
around without changing the frame size. For example:
::

    # Shift a 352x240 image 2 pixels to the right
    Crop(0,0,350,240).AddBorders(2,0,0,0)

    # Add letterbox borders that are Black making a 720x308 image 720x480
    # For YUV colorspace it will be correct (for CCIR-601) color (luma=16)
    AddBorders(0, 86, 0, 86, $000000)

$Date: 2008/06/06 11:37:04 $
