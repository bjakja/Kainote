
Invert
======

``Invert`` (clip, string "channels")

Inverts one or many color channels of a clip, available in *v2.53*.

**Parameters:**

+----------+-------------------------------------------------------------------------------+
| channels | Defines which channels should be inverted. The default is all                 |
|          | channels of the current colorspace. Valid channels are R,G,B,A for RGB clips, |
|          | and Y,U & V for YUY2 and YV12 clips.                                          |
+----------+-------------------------------------------------------------------------------+

**Example:**

::

    AviSource("clip.avi")
    ConvertToRGB32()
    Invert("BG")  # inverts the blue and green channels

+-----------+----------------------------------+
| Changelog |                                  |
+===========+==================================+
| v2.53     | Initial Release                  |
+-----------+----------------------------------+
| v2.55     | Added RGB24, YUY2 and YV12 mode. |
+-----------+----------------------------------+

$Date: 2004/04/09 16:58:20 $