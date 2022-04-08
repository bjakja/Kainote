
Compare
=======

``Compare`` (clip_filtered, clip_original, string "channels",
string "logfile", bool "show_graph")

This filter compares the original clip clip_original and its filtered version
clip_filtered. The filtered version will be returned with the results of the
comparison. Per frame the Mean Absolute Difference, Mean Difference and Peak
signal-to-noise ratio (PSNR) is given, as well as the min (minimum), avg
(average) and max (maximum) PSNR up to that frame (calculated frame-wise).
Starting from *v2.53*, the 'Overall PSNR' (calculated over all pixels in all
frames) is also shown on the output clip.

The channels (default "") string is a combination of R,G,B [,A] or Y,U,V,
depending on the source clips. If it is empty, it means either "YUV" when the
input clips are YCbCr or "RGB" when in the input clips are RGB.

If show_graph (default false) is true then Marc's PSNR graph is also drawn on
it.

If a logfile is specified, the results will be written to a file by this name
and not drawn on the clip. It is much faster if you need to compare a lot of
frames.

**Examples:**
::

    # Displays differences on screen
    Compare(clip1, clip2)
    # for creating a log file:
    Compare(clip1, clip2, "", "compare.log")
    # will only compare chroma channels of YUY2 clips.
    Compare(clip1, clip2, "UV")

The `PSNR`_ is measured in decibels. It's defined as

PSNR(I,K) = 20 * log_10 ( 255/sqrt(MSE(I,K)) )

with

MSE(I,K) = 1/M * sum_{j,k} | I(j,k) - K(j,k) | ^2

and (j,k) runs over all pixels in a frame, and M is the number of pixels in a
frame.

+----------+---------------+
| Changes: |               |
+==========+===============+
| v2.58    | YV12 support. |
+----------+---------------+

$Date: 2008/06/16 19:42:53 $

.. _PSNR: http://en.wikipedia.org/wiki/PSNR
