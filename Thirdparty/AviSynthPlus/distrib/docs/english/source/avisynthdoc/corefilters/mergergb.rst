
MergeARGB / MergeRGB
====================

``MergeARGB`` (clipA, clipR, clipG, clipB)
``MergeRGB`` (clipR, clipG, clipB [, string "pixel_type"])

These filters makes it possible to merge the alpha and color channels from
the source videoclips into the output videoclip.

*ClipA* is the clip that provided the alpha data to merge into the output clip.
For a YUV format input clip the data is taken from the Luma channel. For a
RGB32 format input clip the data is taken from the Alpha channel. It may not
be in RGB24 format.

*ClipR*, *ClipG* and *ClipB* are the clips that provided the R, G and B data
respectively to merge into the output clip. For YUV format input clips the
data is taken from the Luma channel. For RGB format input clips the data is
taken from the respective source channel. i.e. R to R, G to G, B to B. The
unused chroma or color channels of the input clips are ignored.

All YUV luma pixel data is assumed to be pc-range, [0..255], there is no tv-
range, [16..235], scaling. Chroma data from YUV clips is ignored. Input clips
may be a mixture of all formats. YV12 is the most efficient format for
transporting single channels thru any required filter chains.

*pixel_type* default RGB32, optionally RGB24, specifies the output pixel
format.

Also see :ref:`here <multiclip>` for the resulting clip properties.

**Examples:**
::

    # This will only blur the Green channel.
    mpeg2source("c:\apps\avisynth\main.d2v")
    ConvertToRGB24()
    MergeRGB(Last, Blur(0.5), Last)


    # This will swap the red and blue channels and
    # load the alpha from a second video sources.
    vid1 = avisource("c:\apps\avisynth\main.avi")
    vid2 = avisource("c:\apps\avisynth\alpha.avi")
    MergeARGB(vid2, vid1.ShowBlue("YV12"), vid1, vid1.ShowRed("YV12"))
    AudioDub(vid1)

+-----------+------------------------------+
| Changelog |                              |
+===========+==============================+
| v2.56     | added MergeARGB and MergeRGB |
+-----------+------------------------------+

$Date: 2008/05/28 21:24:49 $
