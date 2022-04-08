
SwapFields
==========

``SwapFields`` (clip)

The ``SwapFields`` filter swaps image line 0 with line 1, line 2 with line 3,
and so on, thus effectively swapping the two fields in an interlaced frame.
It's the same as ``SeparateFields().ComplementParity().Weave()`` (and it's
implemented that way).

$Date: 2005/03/24 22:07:09 $
