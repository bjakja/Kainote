
SSRC
====

``SSRC`` (int samplerate, bool "fast")

``SSRC`` Shibata Sample Rate Converter is a resampler. Audio is always
converted to float. This filter will result in better audio quality than
:doc:`ResampleAudio <resampleaudio>`.
It uses SSRC by `Naoki Shibata`_, which offers the best resample quality
available.

Sampling rates of 44.1kHz and 48kHz are populary used, but the ratio of these
two frequencies is 147:160, and they are not small numbers. Therefore,
sampling rate conversion without degradation of sound quality requires
filters with very large order. It is difficult to achieve both quality and
speed. This program achieved relatively fast and high quality with two
different kinds of filters combined skillfully.

**Parameters:**

+------------+-----------------------------------------------------------------------------------+
| samplerate | Samplerate must be an integer.                                                    |
+------------+-----------------------------------------------------------------------------------+
| fast       || This will enable faster processing at slightly lower quality. Set this           |
|            |  to false when you are doing large samplerate conversions (more than a factor 2). |
|            || Default: True.                                                                   |
+------------+-----------------------------------------------------------------------------------+

SSRC doesn't work for arbitrary ratios of the samplerate of the source and
target clip. The following ratios are allowed (see SSRC.cpp):

::

    sfrq = samplerate of source clip
      dfrq = samplerate of destination clip
      frqgcd = gcd(sfrq,dfrq) # Greatest Common Divisor
      fs1 = (dfrq > sfrq) ? sfrq / frqgcd : dfrq / frggcd
    Resampling is possible if: (fs1 == 1) or (fs1 % 2 == 0) or (fs1 % 3 == 0)

    example for which resampling is possible:
      sfrq = 44.1 kHz
      dfrq = 48 kHz
      frqgcd = gcd(44100,48000) = 300
    dfrq > sfrq, hence
      fs1 = sfrq / frqgcd = sfrq / gcd(sfrq,dfrq) = 44100/300 = 147
    and 147 % 3 = 0 since 147 = 3 * 49

The samplerate of your source clip can be found as follows

::

    AviSource(...)
    Subtitle(string(AssumeFPS(23.976,sync_audio=true).AudioRate))

**Example:**

::

    # Downsampling to 44,1 kHz:
    AviSource("c:\file.avi") # Has 48000 audio
    SSRC(44100)

+-----------+-----------------+
| Changelog |                 |
+===========+=================+
| v2.54     | Initial Release |
+-----------+-----------------+

Some parts of SSRC is: Copyright © 2001-2003, Peter Pawlowski. All rights
reserved.

$Date: 2012/04/15 14:59:42 $

.. _Naoki Shibata: http://shibatch.sourceforge.net/
