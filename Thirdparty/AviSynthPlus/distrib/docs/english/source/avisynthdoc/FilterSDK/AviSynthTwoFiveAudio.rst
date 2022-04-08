
`AviSynthTwoFiveAudio`_
=======================

Most sample handling has been completely rewritten. And most notably, the way
of fetching samples has changed slightly. The old way of fetching:

``virtual void __stdcall GetAudio(void* buf, int start, int count,
IScriptEnvironment* env);``

has been replaced by:

``virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count,
IScriptEnvironment* env);``

This has been done to support longer samples. If you extend PClip instead of
GenericVideoFilter, you will have to change this. If you extend
GenericVideoFilter, and don't change it, samples will go through your filter
untouched (as if it wasn't there).

An automatic audio type converter has been added, and is available to all
filters. This enables you to recieve samples only in supported formats. Use
like this:

``PClip ok_clip = ConvertAudio::Create(_clip, SAMPLE_INT16|SAMPLE_FLOAT,
SAMPLE_FLOAT);``

It accepts two parameters besides the original clip - they are accepted
formats and prefered format. If the samples are already in one of the
accepted formats, they will be returned untouched. If they are not, all
samples will be converted to the prefered format. This makes it possible for
your filter to only support a subrange of sample-types without having to
worry about conversion or error messages.

The currently supported sampletypes are:

-   SAMPLE_INT8
-   SAMPLE_INT16
-   SAMPLE_INT24
-   SAMPLE_INT32
-   SAMPLE_FLOAT

vi.sample_type contains one of the types above. The sample type should be
read by calling vi.SampleType(). You should only directly refer to
sample_type, when writing to it (which should only be necessary on sources,
since all convertion is can be done by AviSynth internally).

You can no longer refer to vi.stereo or vi.sixteen_bit. The number of
channels can be retrieved by using vi.AudioChannels(), and it can now contain
any number. The channels are still interleaved, so if vi.AudioChannels()is 4
the samples will be delivered like this:

``[ch1-s1][ch2-s1][ch3-s1][ch4-s1][ch1-s2][ch2-s2][ch3-s2][ch4-s2][ch1-s3][ch
2-s3][ch3-s3][ch4-s3]``

ch is channel and s sample.

For more info, refer to the new audio.cpp and avisynth.h


Back to :doc:`AviSynthTwoFiveSDK <AviSynthTwoFiveSDK>`

$Date: 2006/11/24 18:21:25 $

.. _AviSynthTwoFiveAudio:
    http://www.avisynth.org/AviSynthTwoFiveAudio
