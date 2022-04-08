
SeparateFields / SeparateColumns / SeparateRows
===============================================

``SeparateFields`` (clip)

NTSC and PAL video signals are sequences of fields, but all capture cards
that I'm aware of capture two fields at a time and interlace (or "weave")
them into frames. So frame 0 in the capture file contains fields 0 and 1;
frame 1 contains fields 2 and 3; and so on. ``SeparateFields`` takes a frame-
based clip and splits each frame into its component fields, producing a new
half height clip with twice the frame rate and twice the frame count. This is
useful if you would like to use :doc:`Trim <trim>` and similar filters with single-
field accuracy.

``SeparateFields`` uses the field-dominance information in the source clip to
decide which of each pair of fields to place first in the output. If it gets
it wrong, use :ref:`ComplementParity`, :ref:`AssumeTFF <AssumeFieldFirst>` or :ref:`AssumeBFF <AssumeFieldFirst>`
before ``SeparateFields``.

From version 2.5.6 this filter raises an exception if the clip is already
field-based. You may want to use :ref:`AssumeFrameBased <AssumeFrameField>` to force separate a
second time. Prior versions did a no-op for materials that was already field-
based.

See also: :doc:`Weave <weave>`.

| ``SeparateColumns`` (clip, "interval")
| ``SeparateRows`` (clip, "interval")

``SeparateColumns`` separates the columns of each frame into interval
frames. The number of frames of the new clip is interval times the number of
frames of the old clip. The width of the frame must be a multiple of
interval, otherwise an error is thrown. SeparateColumns is a relatively slow
filter due to the sparse pixel picking required by the algorithm. In some
applications it may be faster to use :doc:`TurnLeft/Right <turn>` with SeparateRows.

``SeparateRows`` separates the rows of each frame into interval frames. The
number of frames of the new clip is interval times the number of frames of
the old clip. The height of the frame must be a multiple of interval,
otherwise an error is thrown. SeparateRows like SeparateFields is very fast
as it uses zero cost subframing to perform it's magic.

``SeparateRows(2)`` is the same as ``SeparateFields()`` except the output is
frame-based instead of field-based.

**Examples:**

::

    # returns the original clip:
    AviSource("c:\file.avi")
    SeparateColumns(1)

    # returns a clip where the columns are separated:
    # frame 0 consists of the columns 0,3,6,... of the original frame 0
    # frame 1 consists of the columns 1,4,7,... of the original frame 0
    # frame 2 consists of the columns 2,5,8,... of the original frame 0
    # frame 3 consists of the columns 0,3,6,... of the original frame 1
    # etc ...
    AviSource("c:\file.avi")
    SeparateColumns(3)

    # returns a clip where the rows are separated:
    # frame 0 consists of the rows 0,2,4,... of the original frame 0
    # frame 1 consists of the rows 1,3,5,... of the original frame 0
    # frame 2 consists of the rows 0,2,4,... of the original frame 1
    # frame 3 consists of the rows 1,3,5,... of the original frame 1
    # etc ...
    AviSource("c:\file.avi")
    SeparateRows(2)

+---------+-----------------------------------------+
| Changes |                                         |
+=========+=========================================+
| v2.60   | Added SeparateColumns and SeparateRows. |
+---------+-----------------------------------------+

$Date: 2012/04/15 14:59:41 $
