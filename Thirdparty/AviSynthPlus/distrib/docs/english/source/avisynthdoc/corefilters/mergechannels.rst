
MergeChannels
=============

``MergeChannels`` (clip1 , clip2 [, clip3, ...])

Starting from v2.5 ``MergeChannels`` replaces :doc:`MonoToStereo <monotostereo>`, and can be
used to merge the audio channels of two or more clips.
Don't confuse this with mixing of channels (:doc:`MixAudio <mixaudio>` and
:doc:`ConvertToMono <converttomono>` do this) - the sound of each channel is left untouched,
the channels are only put into the new clip.
Before merging audio is converted to the sample type of clip1.

::

    # Example, converts "uncompressed wav" audio to a 44.1 kHz stereo signal:
    video = AviSource("c:\divx_wav.avi")
    audio = WavSource("c:\divx_wav.avi")
    l_ch = GetChannel(audio, 1)
    r_ch = GetChannel(audio, 2)
    stereo = MergeChannels(l_ch, r_ch).ResampleAudio(44100)
    return AudioDub(video, stereo)

    # This is similar to:
    video = AviSource("c:\divx_wav.avi")
    audio = WavSource("c:\divx_wav.avi")
    stereo = GetChannel(audio, 1, 2).ResampleAudio(44100)
    return AudioDub(video, stereo)

$Date: 2004/03/09 21:28:07 $
