
Pulldown
========

``Pulldown`` (clip, int a, int b)


a:b Pulldown
------------

In general, **a:b pulldown** (also called **telecine**) means that the first
frame is represented by "a" fields of video, the second frame is represented
by "b" fields of video, which is repeated till the end of the clip. **inverse
telecine**: means undoing the telecine.

**Example:**

**2:3 or 3:2 pulldown**: 23.976 -> 29.97 by adding duplicate fields in the
following way: Thus the first frame is represented by two fields of video,
the second frame by three fields of video, which is repeated to the end of
the clip:

::

    source: AtAb BtBb CtCb DtDb (four frames)
    2:3 pulldown: AtAb BtBb BtCb CtDb DtDb (five frames)
    3:2 pulldown: AtAb AtBb BtCb CtCb DtDb (five frames)
    etc ...

Pulldown as AviSynth filter
---------------------------

``Pulldown(a,b)`` (with a < b) selects frames a, b, 5+a, 5+b, 2*5+a, 2*5+b,
... So the filter simply selects two out of every five frames of the source
video. The frame rate is reduced to two-fifths of its original value. Note
that a a:b telecine can be undone by ``Pulldown(a,b)`` if a+b=5.

**Example:**

::

    DoubleWeave()
    Pulldown(0,3)

is the same as undoing a 2:3 (or 3:2) pulldown (ie 29.97 -> 23.976).

Suppose you have material on which 2:3 pulldown is applied, that is

::

    AtAb BtBb CtCb DtDb => AtAb BtBb BtCb CtDb DtDb (five frames)

The reason you need to use :doc:`DoubleWeave <doubleweave>` first is that many capture
cards sometimes recombine fields in the "wrong" way. In terms of fields, the
3:2 pulldown sequence is simply "At Ab Bt Bb Bt Cb Ct Db Dt Db ...", where
"A" through "D" represent the original film frames (before the broadcaster
telecined it). But many capture cards the fields into frames with no respect
for the pulldown pattern, and you get this:

::

    AtAb BtAb BtBb BtBb BtCb CtCb CtDb DtDb DtDb

``Pulldown(0,3)`` selects frames 0, 3, 5+0, 5+3, 10+0, 10+3, ...

::

    AtAb BtAb BtBb BtBb BtCb CtCb CtDb DtDb DtDb
     *              *         *              *

thus getting

::

    AtAb BtBb CtCb DtDb

``Pulldown(a,b)`` is implemented internally as
::

    SelectEvery(5,a,b).AssumeFrameBased

$Date: 2010/01/06 16:01:28 $
