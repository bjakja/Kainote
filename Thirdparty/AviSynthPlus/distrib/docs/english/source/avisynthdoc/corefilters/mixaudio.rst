
MixAudio
========

``MixAudio`` (clip1, clip2, float "clip1_factor", float "clip2_factor")

Mixes audio from two clips. A volume for the two clips can be given, but is
optional.

Volume is given as a factor, where 0.0 is no audio from the desired channel,
and 1.0 is 100% from the desired channel. Default is 0.5/0.5 - if only one
factor is given, the other channel will be 1.0-(factor). If factor1 + factor2
is more than 1.0, you risk clipping your signal.

The sample rate of the two clips needs to be the same (use :doc:`ResampleAudio <resampleaudio>`
if necessary). Your clips need also to have the same number of channels
(stereo/mono) - use :doc:`ConvertToMono <converttomono>` or :doc:`MonoToStereo <monotostereo>` / :doc:`MergeChannels <mergechannels>` if
necessary.

**Example:**
::

    # Mixes two sources, with one source slightly lower than the other.
    Soundtrack = WavSource("c:\soundtrack.wav")
    Speak = WavSource("c:\speak.wav")
    return MixAudio(Soundtrack, Speak, 0.75, 0.25)    # The Expert may
    notice that the last 0.25 is actually redundant here.

$Date: 2010/02/27 14:45:27 $
