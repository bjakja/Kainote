
Normalize
=========

``Normalize`` (clip, float "volume", bool "show")

Amplifies the entire waveform as much as possible, without clipping.

By default the clip is amplified to 1.0, that is maximum without clipping -
higher values are sure to clip, and create distortions. If one volume is
supplied, the other channel will be amplified the same amount.

The calculation of the peak value is done the first time the audio is
requested, so there will be some seconds until AviSynth continues.

Starting from *v2.08* there is an optional argument show, if set to ``true``,
it will show the maximum amplification possible without distortions.

Multichannels are never amplified separately by the filter, even if the level
between them is very different. The volume is applied AFTER the maximum peak
has been found, and works in effect as a separate :doc:`Amplify <amplify>`. That means
if you have two channels that are very different the loudest channel will
also be the peak for the lowest. If you want to normalize each channel
separately, you must use :doc:`GetChannel <getchannel>` to split up the stereo source.

The audio sample type is converted to float or is left untouched if it is 16
bits.

Examples:
::

    # normalizes signal to 98%
    video = AviSource("C:\video.avi")
    audio = WavSource("c:\autechre.wav")
    audio = Normalize(audio, 0.98)
    return AudioDub(video, audio)

    # normalizes each channel seperately
    video = AviSource("C:\video.avi")
    audio = WavSource("C:\bjoer7000.wav")
    left_ch = GetLeftChannel(audio).Normalize
    right_ch = GetRightChannel(audio).Normalize
    audio = MonoToStereo(left_ch, right_ch)
    return AudioDub(video, audio)

    # normalizes each channel seperately
    clip = AviSource("D:\Video\rawstuff\stereo-test file_left(-6db).avi")
    left_ch = GetChannel(clip,1).Normalize
    right_ch = GetChannel(clip,2).Normalize
    audio = MergeChannels(left_ch, right_ch)
    AudioDub(clip, audio)

$Date: 2009/09/12 15:10:22 $
