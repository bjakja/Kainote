
Dissolve
========

``Dissolve`` (clip1, clip2 [, ...], int overlap, float "fps")

``Dissolve`` is like :doc:`AlignedSplice <splice>`, except that the clips are combined
with some overlap. The last overlap frames of the first video stream are
blended progressively with the first overlap frames of the second video
stream so that the streams fade into each other. The audio streams are
blended similarly.

The *fps* parameter is optional, default=24.0, and provides a reference for
*overlap* in audio only clips. It is ignored if a video stream is present.
Set fps=AudioRate() if sample exact audio positioning is required.

The term "dissolve" is sometimes used for a different effect in which the
transition is pointwise rather than gradated. This filter won't do that.

Also see :ref:`here <multiclip>` for the resulting clip properties.

+----------+------------------------+
| Changes: |                        |
+==========+========================+
| v2.56    | Added *fps* parameter. |
+----------+------------------------+

$Date: 2011/01/16 12:22:43 $
