
Weave / WeaveColumns / WeaveRows
================================

``Weave`` (clip)

``Weave`` is the opposite of :doc:`SeparateFields <separatefields>`: it takes pairs of fields
from the input video clip and combines them together to produce interlaced
frames. The new clip has half the frame rate and frame count. ``Weave`` uses
the frame-parity information in the source clip to decide which field to put
on top. If it gets it wrong, use :ref:`ComplementParity` beforehand or
:doc:`SwapFields <swapfields>` afterwards.

All AviSynth filters keep track of field parity, so ``Weave`` will always
join the fields together in the proper order. If you want the other order,
you'll have to use :ref:`ComplementParity`, :ref:`AssumeTFF <AssumeFieldFirst>` or
:ref:`AssumeBFF <AssumeFieldFirst>` beforehand or :doc:`SwapFields <swapfields>` afterwards.

From verions 2.5.6 this filter raises an exception if the clip is already
frame-based. You may want to use :ref:`AssumeFieldBased <AssumeFrameField>` to force weave a
second time. Prior versions did a no-op for materials that was already frame-
based.

``WeaveColumns`` (clip, int period)
``WeaveRows`` (clip, int period)

``WeaveColumns`` is the opposite of :doc:`SeparateColumns <separatefields>`: it weaves the
columns of period frames into a single output frame. The number of frames of
the new clip is the ceiling of the number of frames of the input clip divided
by period. WeaveColumns is a relatively slow filter due to the sparse pixel
placing required by the algorithm. In some applications it may be faster to
use TurnLeft/Right with WeaveRows.

``WeaveRows`` is the opposite of :doc:`SeparateRows <separatefields>`: it weaves the rows of
period frames into a single output frame. The number of frames of the new
clip is the ceiling of the number of frames of the input clip divided by
period. WeaveRows is a relatively quick filter, typically costing 1 output
frame blit.

$Date: 2013/01/06 13:38:34 $
