
ConvertAudio
============

ConvertAudioTo8bit / ConvertAudioTo16bit / ConvertAudioTo24bit / ConvertAudioTo32bit / ConvertAudioToFloat
----------------------------------------------------------------------------------------------------------

| ``ConvertAudioTo8bit`` (clip)
| ``ConvertAudioTo16bit`` (clip)
| ``ConvertAudioTo24bit`` (clip)
| ``ConvertAudioTo32bit`` (clip)
| ``ConvertAudioToFloat`` (clip)

The first four filters convert the audio samples to 8, 16, 24 and 32 bits,
and ``ConvertAudioToFloat`` converts the audio samples to float.
``ConvertAudioTo8bit``, ``ConvertAudioTo24bit``, ``ConvertAudioTo32bit`` and
``ConvertAudioToFloat`` are available starting from *v2.5*.

Starting from v2.5 the audio samples will be automatically converted if any
filters requires a special type of sample. This means that most filters will
accept several types of input, but if a filter doesn't support the type of
sample it is given, it will automatically convert the samples to something it
supports. The internal formats supported in each filter is listed in the
colorspace column. A specific sample type can be forced by using the
ConvertAudio functions.

$Date: 2004/12/23 22:00:52 $
