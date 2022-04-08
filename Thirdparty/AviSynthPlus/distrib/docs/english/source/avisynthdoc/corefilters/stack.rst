
StackHorizontal / StackVertical
===============================

| ``StackHorizontal`` (clip1, clip2 [, ...])
| ``StackVertical`` (clip1, clip2 [, ...])

``StackHorizontal`` takes two or more video clips and displays them together
in left-to-right order. The heights of the images and their color formats
must be the same. Most other information (sound track, frame rate, etc) is
taken from the first clip - see :ref:`here <multiclip>` for the resulting clip properties.
``StackVertical`` does the same, except from top to bottom.



::

    # Compare frames with and without noise reduction
    StackVertical(last, last.SpatialSoften(2,3,6))

    #                                                      a b
    # Show clips in variables a,b,c,d in a box like this:  c d
    StackVertical(StackHorizontal(a,b),StackHorizontal(c,d))

$Date: 2004/03/09 21:28:07 $
