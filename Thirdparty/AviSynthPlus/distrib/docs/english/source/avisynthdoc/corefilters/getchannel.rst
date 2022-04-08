
GetChannel
==========

| ``GetChannel`` (clip, int ch1 [, int ch2, ...])
| ``GetChannels`` (clip, int ch1 [, int ch2, ...])

Prior to v2.5 ``GetLeftChannel`` returns the left and ``GetRightChannel`` the
right channel from a stereo signal. ``GetChannel`` is present starting
from v2.5 and it returns one or more channels of a multichannel signal.
``GetChannels`` is an alias to ``GetChannel``.

The ordering of the channels is determined by the ordering of the input file,
because AviSynth doesn't assume any ordering. In case of stereo 2.0 WAV and
5.1 WAV files the ordering should be as follows:

+-------------------+---------------+
| WAV 2 ch (stereo) |               |
+===================+===============+
| 1                 | left channel  |
+-------------------+---------------+
| 2                 | right channel |
+-------------------+---------------+

+------------+----------------------+
| WAV 5.1 ch |                      |
+============+======================+
| 1          | front left channel   |
+------------+----------------------+
| 2          | front right channel  |
+------------+----------------------+
| 3          | front center channel |
+------------+----------------------+
| 4          | LFE (Subwoofer)      |
+------------+----------------------+
| 5          | rear left channel    |
+------------+----------------------+
| 6          | rear right channel   |
+------------+----------------------+



::

    # Removes right channel information, and return as mono clip with only left channel:
    video = AviSource("c:\filename.avi")
    stereo = WavSource("c:\afx-ab3_t4.wav")
    mono = GetLeftChannel(stereo)
    return AudioDub(video, mono)

    # Using v2.5 this becomes:
    video = AviSource("c:\filename.avi")
    stereo = WavSource("c:\afx-ab3_t4.wav")
    mono = GetChannel(stereo, 1)
    return AudioDub(video, mono)

    # You could also obtain the channels from the avi file itself:
    video = AviSource("c:\filename.avi")
    return GetChannel(video, 1)

    # Converts avi with "uncompressed 5.1 wav" audio to a stereo signal:
    video = AviSource("c:\divx_wav.avi")
    audio = WavSource(c:\divx_wav.avi)
    stereo = GetChannel(audio, 1, 2)
    return AudioDub(video, stereo)

Remark1
-------

Every file format has a different internal channel ordering. The following
table gives this internal ordering for some formats (useful for plugin
writers), but it is a decoder task supply to AviSynth the expected channel
order (like WAV), if you use decoders like NicAudio/BassAudio or
ffdshow/AC3_filter you don't need to worry about this:

+-------------+----------------------+----------------------+----------------------+---------------------+--------------------+--------------------+
| reference:  | channel 1:           | channel 2:           | channel 3:           | channel 4:          | channel 5:         | channel 6:         |
+=============+======================+======================+======================+=====================+====================+====================+
| `5.1 WAV`_  | front left channel   | front right channel  | front center channel | LFE                 | rear left channel  | rear right channel |
+-------------+----------------------+----------------------+----------------------+---------------------+--------------------+--------------------+
| `5.1 AC3`_  | front left channel   | front center channel | front right channel  | rear left channel   | rear right channel | LFE                |
+-------------+----------------------+----------------------+----------------------+---------------------+--------------------+--------------------+
| `5.1 DTS`_  | front center channel | front left channel   | front right channel  | rear left channel   | rear right channel | LFE                |
+-------------+----------------------+----------------------+----------------------+---------------------+--------------------+--------------------+
| `5.1 AAC`_  | front center channel | front left channel   | front right channel  | rear left channel   | rear right channel | LFE                |
+-------------+----------------------+----------------------+----------------------+---------------------+--------------------+--------------------+
| `5.1 AIFF`_ | front left channel   | rear left channel    | front center channel | front right channel | rear right channel | LFE                |
+-------------+----------------------+----------------------+----------------------+---------------------+--------------------+--------------------+

* 5.1 DTS: the LFE is on a separate stream (much like on multichannel MPEG2).
* AAC specifications are unavailable on the internet (a free version)?

Remark2
-------

At the time of writing, Besweet still has the `2GB barrier`_. So make sure
that the size of the 5.1 WAV is below 2GB, otherwise encode to six separate
wavs or use HeadAC3he. But now there are encoders than also support WAV files
greater than 4GB with the appropriate parameters.

$Date: 2010/11/28 18:47:07 $

.. _5.1 WAV: http://www.cs.bath.ac.uk/~jpff/NOS-DREAM/researchdev/wave-ex/wave_ex.html
.. _5.1 AC3: http://www.atsc.org/standards/a_52a.pdf
.. _5.1 DTS: http://webapp.etsi.org/action%5CPU/20020827/ts_102114v010101p.pdf
.. _5.1 AAC: http://www.hydrogenaudio.org/index.php?showtopic=10986
.. _5.1 AIFF: http://preserve.harvard.edu/standards/Audio%20IFF%20Specification%201%203.pdf
.. _2GB barrier: http://forum.doom9.org/showthread.php?s=&postid=305084
