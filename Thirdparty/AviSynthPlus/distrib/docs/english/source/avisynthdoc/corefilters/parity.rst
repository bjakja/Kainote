
Parity
======

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



.. _AssumeFrameField:

AssumeFrameBased / AssumeFieldBased
-----------------------------------

``AssumeFrameBased`` (clip)
``AssumeFieldBased`` (clip)

AviSynth keeps track of whether a given clip is field-based or frame-based.
If the clip is field-based it also keeps track of the parity of each field
(that is, whether it's the top or the bottom field of a frame). If the clip
is frame-based it keeps track of the dominant field in each frame (that is,
which field in the frame comes first when they're separated).

However, this information isn't necessarily correct, because field
information usually isn't stored in video files and Avisynth's source filters
just guess at it. ``AssumeFrameBased`` and ``AssumeFieldBased`` let you tell
AviSynth the correct type of a clip.

``AssumeFrameBased`` throws away the existing information and assumes that
the clip is frame-based, with the bottom (even) field dominant in each frame.
(This happens to be what the source filters guess.) If you want the top field
dominant, use ``ComplementParity`` afterwards.

``AssumeFieldBased`` throws away the existing information and assumes that
the clip is field-based, with the even-numbered fields being bottom fields
and the odd-numbered fields being top fields. If you want it the other way
around, use ``ComplementParity`` afterwards.


.. _AssumeFieldFirst:

AssumeTFF / AssumeBFF
---------------------

``AssumeTFF`` (clip)
``AssumeBFF`` (clip)

Forcing the field order regardless of current value.


.. _ComplementParity:

ComplementParity
----------------

``ComplementParity`` (clip)

If the input clip is field-based, ``ComplementParity`` changes top fields to
bottom fields and vice-versa. If the input clip is frame-based, it changes
each frame's dominant field (bottom-dominant to top-dominant and vice-versa).

$Date: 2005/10/13 21:41:11 $
