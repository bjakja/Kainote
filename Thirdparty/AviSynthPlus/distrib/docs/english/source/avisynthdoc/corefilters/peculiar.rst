
PeculiarBlend
=============

``PeculiarBlend`` (clip, int cutoff)

This filter blends each frame with the following frame in a peculiar way. The
portion of the frame below the *(cutoff)*-th scanline is unchanged. The
portion above the *(cutoff-30)*-th scanline is replaced with the corresponding
portion of the following frame. The 30 scan lines in between are blended
incrementally to disguise the switchover.

You're probably wondering why anyone would use this filter. Well, it's like
this. Most videos which were originally shot on film use the 3:2 pulldown
technique which is described in the description of the :doc:`Pulldown <pulldown>`
filter. But some use a much nastier system in which the crossover to the next
frame occurs in the middle of a field--in other words, individual fields look
like one movie frame on the top, and another on the bottom. This filter
partially undoes this peculiar effect. It should be used after th before the
:doc:`Pulldown <pulldown>` filter. To determine *cutoff*, examine a frame which is
blended in this way and set *cutoff* to the number of the first scanline in
which you notice a blend.

This filter works only with YUY2 input.

$Date: 2004/03/09 21:28:07 $
