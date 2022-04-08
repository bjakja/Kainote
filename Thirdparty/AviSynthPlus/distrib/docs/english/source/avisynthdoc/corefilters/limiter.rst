
Limiter
=======

``Limiter`` (clip, int "min_luma", int "max_luma", int "min_chroma", int
"max_chroma", string "show")

This filter is present in *v2.5*. The standard known as CCIR-601 defines the
range of pixel values considered legal for presenting on a TV. These ranges
are 16-235 for the luma component and 16-240 for the chroma component.

Pixels outside this range are known to cause problems with some TV sets, and
thus it is best to remove them before encoding if that is your intended
display device. By default this filter clips (or "clamps") pixels under 16 to
16 and over 235 (or 240) to 235 (or 240).

Prior to *v2.53* the (incorrect) default value was 236. Use Limiter(16, 235,
16, 240) for CCIR-601 compliant digital video.

In  *v2.56*, an option *show* is added. If set, it colors the pixels outside
the specified [min_luma,max_luma] or [min_chroma,max_chroma] range.

*show* can be "luma" (shows out of bounds luma in red/green), "luma_grey"
(shows out of bounds luma and makes the remaining pixels greyscale), "chroma"
(shows out of bounds chroma in yellow), "chroma_grey" (shows out of bounds
chroma and makes the remaining pixels greyscale). The coloring is done as
follows:

| YUY2 (chroma shared between two horizontal pixels p1 and p2: Y1UY2V): j,k=1,2
| YV24 (no chroma shared): j,k=1
| YV12 (chroma shared between 2x2 pixels Y11UY12V; Y21UY22V): j,k=11,12,21,22

+-----------------------------------+--------------+-----------------------+
|                                   | luma         | luma_grey             |
+===================================+==============+=======================+
| Yj < min_luma                     | red (pj)     | red (pj)              |
+-----------------------------------+--------------+-----------------------+
| Yj > max_luma                     | green (pj)   | green (pj)            |
+-----------------------------------+--------------+-----------------------+
| Yj < min_luma and Yk > max_luma   | yellow (all) | puke (pj), olive (pk) |
+-----------------------------------+--------------+-----------------------+

+-----------------------------------+--------------+-----------------------+
|                                   | chroma       | chroma_grey           |
+===================================+==============+=======================+
| U < min_chroma                    | yellow       | yellow                |
+-----------------------------------+--------------+-----------------------+
| U > max_chroma                    | yellow       | blue                  |
+-----------------------------------+--------------+-----------------------+
| V < min_chroma                    | yellow       | cyan                  |
+-----------------------------------+--------------+-----------------------+
| V > max_chroma                    | yellow       | red                   |
+-----------------------------------+--------------+-----------------------+
| U < min_chroma and V < min_chroma | yellow       | green                 |
+-----------------------------------+--------------+-----------------------+
| U > max_chroma and V < min_chroma | yellow       | teal                  |
+-----------------------------------+--------------+-----------------------+
| U < min_chroma and V > max_chroma | yellow       | orange                |
+-----------------------------------+--------------+-----------------------+
| U > max_chroma and V > max_chroma | yellow       | magenta               |
+-----------------------------------+--------------+-----------------------+

+-----------+----------------------------------------------+
| Changelog |                                              |
+===========+==============================================+
| v2.60     | Added show for YV24.                         |
+-----------+----------------------------------------------+
| v2.56     | added show to show out of bounds luma/chroma |
+-----------+----------------------------------------------+

$Date: 2012/04/09 08:19:32 $
