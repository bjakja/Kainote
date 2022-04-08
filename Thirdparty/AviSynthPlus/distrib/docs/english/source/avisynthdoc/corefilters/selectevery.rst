
SelectEvery
===========

``SelectEvery`` (clip, int step_size, int offset1 [, int offset2 [, ...]])

``SelectEvery`` is a generalization of filters like :doc:`SelectEven <select>` and
:doc:`Pulldown <pulldown>`. I think the easiest way to describe it is by example:
::

    SelectEvery(clip,2,0)      # identical to SelectEven(clip)
    SelectEvery(clip,2,1)      # identical to SelectOdd(clip)
    SelectEvery(clip,10,3,6,7) # select frames 3, 6, 7, 13, 16, 17, ... from source clip
    SelectEvery(clip,9,0)      # select frames 0, 9, 18, ... from source
    clip

And how about this:
::

    # Take a 24fps progressive input clip and apply 3:2 pulldown,
    # yielding a 30fps interlaced output clip
    AssumeFrameBased
    SeparateFields
    SelectEvery(8, 0,1, 2,3,2, 5,4, 7,6,7)
    Weave

$Date: 2004/03/09 21:28:07 $
