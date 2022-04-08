
ResampleAudio
=============

``ResampleAudio`` (clip, int new_rate_numerator[, int new_rate_denominator])

``ResampleAudio`` performs a high-quality change of audio samplerate. The
conversion is skipped if the samplerate is already at the given rate.

When using fractional resampling the output audio samplerate is given by :
::

    int(new_rate_numerator / new_rate_denominator + 0.5)

However the internally the resampling factor used is:
::

    new_rate_numerator / (new_rate_denominator * old_sample_rate)

This causes the audio duration to vary slightly (which is generally what is desired).

::

    # resamples audio to 48 kHz
    source = AviSource("c:\audio.wav")
    return ResampleAudio(source, 48000)

    # Exact 4% speed up for Pal telecine
    Global Nfr_num=25
    Global Nfr_den=1
    AviSource("C:\Film.avi") # 23.976 fps, 44100Hz
    Ar=Audiorate()
    ResampleAudio(Ar*FramerateNumerator()*Nfr_den,
    FramerateDenominator()*Nfr_num)
    AssumeSampleRate(Ar)
    AssumeFPS(Nfr_num, Nfr_den, False)

For exact resampling the intermediate samplerate needs to be 42293.706293
which if rounded to 42294 would causes about 30ms per hour variation.

+-----------+----------------------------------------------------+
| Changelog |                                                    |
+===========+====================================================+
| v2.53     | ``ResampleAudio`` accepts any number of channels.  |
+-----------+----------------------------------------------------+
| v2.56     || ``ResampleAudio`` process float samples directly. |
|           || Support fractional resampling.                    |
+-----------+----------------------------------------------------+

$Date: 2005/01/18 11:10:51 $
