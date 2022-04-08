
Amplify / AmplifydB
===================

``Amplify`` (clip, float amount1 [, ...])
``AmplifydB`` (clip, float amount1 [, ...])

``Amplify`` multiplies the audio samples by amount. You can specify different
factors for each channel.
If there are more volumes than there are channels, they are ignored. If there
are fewer volumes than channels, the last volume is applied to the rest of
the channels.

``AmplifydB`` is the same except values are in decibels (dB).
You can use negative dB values (or scale factor between 0 and 1) for reducing
volume. Negative scale factors will shift the phase by 180 degrees (i.e.
invert the samples).

8bit and 24bit Audio samples are converted to float in the process, the
other audio formats are kept.

::

    # Amplifies left channel with 3 dB (adds 3 dB):
    video = AviSource("c:\filename.avi")
    stereo = WavSource("c:\audio.wav")
    stereo_amp = AmplifydB(stereo, 3, 0)
    return AudioDub(video, stereo_amp)

    # Amplifies front channels with 3 dB (adds 3 dB):
    video = AviSource("c:\divx_6ch_wav.avi")
    audio = WavSource(c:\divx_6ch_wav.avi)
    multichannel_amp = AmplifydB(audio, 3, 3, 3)
    return AudioDub(video, multichannel_amp)

How the multichannels are mapped can be found in the description of
:doc:`GetChannel <getchannel>`.

$Date: 2009/09/12 15:10:22 $
