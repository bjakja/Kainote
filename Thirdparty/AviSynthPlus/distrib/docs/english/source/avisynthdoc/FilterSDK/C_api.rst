
C API
=====

The header, avisynth_c.h, declares all the classes, structures and
miscellaneous constants that you might need when writing a plugin. All
external plugins should #include it:
::

    #include "avisynth_c.h"

source: http://forum.doom9.org/showthread.php?p=1464911#post1464911

compiling plugins: http://forum.doom9.org/showthread.php?p=1092380#post1092380

example: http://forum.doom9.org/showthread.php?p=1001260#post1001260

| For now the api is described here:
| http://www.kevina.org/avisynth_c/api.html.

| An example is given here:
| http://www.kevina.org/avisynth_c/example.html.

In v2.60 (AVISYNTH_INTERFACE_VERSION = 6) the following functions are
added to the C interface:
::

    avs_is_yv24
    avs_is_yv16
    avs_is_yv12
    avs_is_yv411
    avs_is_y8
    avs_is_color_space
    avs_get_plane_width_subsampling
    avs_get_plane_height_subsampling
    avs_bits_per_pixel
    avs_bytes_from_pixels
    avs_row_size
    avs_bmp_size
    avs_get_row_size_p
    avs_get_height_p

____

Back to :doc:`FilterSDK`

$Date: 2015/01/13 00:24:50 $
