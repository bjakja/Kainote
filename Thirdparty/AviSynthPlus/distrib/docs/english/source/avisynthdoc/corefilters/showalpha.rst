
ShowAlpha, ShowRed, ShowGreen, ShowBlue
=======================================

``ShowAlpha`` (clip, string pixel_type)
``ShowBlue`` (clip, string pixel_type)
``ShowGreen`` (clip, string pixel_type)
``ShowRed`` (clip, string pixel_type)

``ShowAlpha`` shows the alpha channel of a RGB32 clip, available in *v2.53*.
``ShowBlue`` / ``ShowGreen`` / ``ShowRed`` shows the selected channel of a
RGB clip, available in *v2.56*.

In *v2.54* ``ShowAlpha`` now returns RGB, YUY2, or YV12 via the pixel_type
argument. The latter two can be used to layer an RGB clip with alpha
transparency data onto a YUV clip using the 3-argument form of ``Overlay``,
because when setting pixel_type to YUY2 or YV12 the luma range is [0,255].

In *v2.56* ``ShowAlpha/Red/Green/Blue`` now returns RGB24, RGB32, YUY2, or
YV12 via the pixel_type argument. For RGB32 output the selected channel is
copied to all R, G and B channels, but not the Alpha channel which is left
untouched. For YUV output the selected channel is copied to the Luma channel,
the chroma channels are set to grey (0x80).

**Examples:**
::

    # shows alpha channels of clip
    AviSource("clip.avi")
    ShowAlpha()

    # swaps red and blue channels:
    AviSource("clip.avi")
    MergeRGB(ShowBlue("YV12"), Last, ShowRed("YV12"))

+-----------+---------------------------------------+
| Changelog |                                       |
+===========+=======================================+
| v2.56     | added ShowBlue, ShowGreen and ShowRed |
+-----------+---------------------------------------+

$Date: 2005/07/08 22:53:16 $
