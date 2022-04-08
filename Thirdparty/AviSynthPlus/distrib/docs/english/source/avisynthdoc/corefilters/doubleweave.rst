
DoubleWeave
===========

``DoubleWeave`` (clip)

If the input clip is field-based, the ``DoubleWeave`` filter operates like
:doc:`Weave <weave>`, except that it produces double the number of frames: instead of
combining fields 0 and 1 into frame 0, fields 2 and 3 into frame 1, and so
on, it combines fields 0 and 1 into frame 0, fields 1 and 2 into frame 1, and
so on. It does not change the frame rate or frame count.

If the input clip is frame-based, this filter acts just as though you'd
separated it into fields with :doc:`SeparateFields <separatefields>` first, only faster!

:doc:`Weave <weave>` is actually just a shorthand for ``DoubleWeave`` followed by
:doc:`SelectEven <select>`.

Most likely you will want to use a filter like :doc:`SelectOdd <select>` or
:doc:`Pulldown <pulldown>` after using this filter, unless you really want a 50fps or
60fps video. It may seem inefficient to interlace every pair of fields only
to immediately throw away half of the resulting frames. But actually, because
Avisynth only generates frames on demand, frames that are not needed will
never be generated in the first place.

If you're processing field-based video, like video-camera footage, you
probably won't need this filter. But if you're processing NTSC video
converted from film and you plan to use the ``Pulldown`` filter, you need to
use ``DoubleWeave`` first. See the ``Pulldown`` filter for an explanation.

If you're processing PAL video converted from film, you don't need
``Pulldown``, but you might want to use ``DoubleWeave`` in the following
situation:

::

    # Duplicate the functionality of the VirtualDub "PAL deinterlace" filter
    DoubleWeave
    SelectOdd


$Date: 2005/01/21 07:47:22 $
