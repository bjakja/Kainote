
AlignedSplice / UnalignedSplice
===============================

| ``AlignedSplice`` (clip1, clip2 [, ...])
| ``UnAlignedSplice`` (clip1, clip2 [, ...])

``AlignedSplice`` and ``UnalignedSplice`` join two or more video clips end to
end. The difference between the filters lies in the way they treat the sound
track. ``UnalignedSplice`` simply concatenates the sound tracks without
regard to synchronization with the video. ``AlignedSplice`` cuts off the
first sound track or inserts silence as necessary to ensure that the second
sound track remains synchronized with the video.

You should use ``UnalignedSplice`` when the soundtracks being joined were
originally contiguous - for example, when you're joining files captured with
AVI_IO. Slight timing errors may lead to glitches in the sound if you use
``AlignedSplice`` in these situations.

Avisynth's scripting language provides ``+`` and ``++`` operators as synonyms
for ``UnalignedSplice`` and ``AlignedSplice`` respectively.

Also see :ref:`here <multiclip>` for the resulting clip properties.



::

    # Join segmented capture files to produce a single clip
    UnalignedSplice(AVISource("cap1.avi"),AVISource("cap2.avi"),AVISource("cap3.avi"))

    # or:
    AVISource("cap1.avi") + AVISource("cap2.avi") + AVISource("cap3.avi")

    # Extract three scenes from a clip and join them together in a new order
    AVISource("video.avi")
    edited_video = Trim(2000,2500) ++ Trim(3000,3500) ++ Trim(1000,1500)


$Date: 2004/03/09 21:28:07 $
