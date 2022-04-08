
SelectRangeEvery
================

``SelectRangeEvery`` (clip, int "every", int "length", int "offset", bool
"audio")

This filter is available starting from v2.5. The filter selects length number
of frames every n frames, starting from frame offset. Default values are:
Select 50 frames every 1500 frames, starting with first selection at frame 0.
(every = 1500, length = 50, offset = 0).

Starting from v2.55, ``SelectRangeEvery`` will also process audio. To keep
the original audio, use  audio = false.

Examples:
::

    # Selects the frames 0 to 13, 280 to 293, 560 to 573, etc.
    SelectRangeEvery(clip, 280, 14)

    # Selects the frames 2 to 15, 282 to 295, 562 to 575, etc.
    SelectRangeEvery(clip, 280, 14, 2)

$Date: 2004/07/21 18:54:28 $
