
MonoToStereo
============

``MonoToStereo`` (left_channel_clip, right_channel_clip)

Converts two mono signals to one stereo signal. This can be used, if one or
both channels has been modified seperately, and then has to be recombined.

The two clips has to have audio at the same sample rates (Use
:doc:`ResampleAudio <resampleaudio>` if this is a problem.) If either of the sources is in
stereo, the signal will be taken from the corresponding channel (left channel
from the left_channel_clip, and vice versa for right channel).
Before merging audio is converted to the sample type of the
left_channel_clip.

In v2.5 this function is simply mapped to :doc:`MergeChannels <mergechannels>`.

::

    # Combines two seperate wave sources to a stereo signal:
    left_channel = WavSource("c:\left_channel.wav")
    right_channel = WavSource("c:\right_channel.wav")
    return MonoToStereo(left_channel, right_channel)


$Date: 2004/03/09 21:28:07 $
