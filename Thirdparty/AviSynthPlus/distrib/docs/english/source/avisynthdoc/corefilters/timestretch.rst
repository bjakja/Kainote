
TimeStretch
===========

``TimeStretch`` (clip, float "tempo", float "rate", float "pitch", int
"sequence", int "seekwindow", int "overlap", bool "quickseek", int "aa")

``TimeStretch`` allows changing the sound tempo, pitch and playback rate
parameters independently from each other, i.e.:

-   Sound tempo can be increased or decreased while maintaining the
    original pitch.
-   Sound pitch can be increased or decreased while maintaining the
    original tempo.
-   Change playback rate that affects both tempo and pitch at the same
    time.
-   Choose any combination of tempo/pitch/rate.

**Parameters:**

The speed parameters are percentages, and defaults to 100. If tempo is 200 it
will play twice as fast, if it is 50, it will play at half the speed.
Adjusting rate is equivalent to using :doc:`AssumeSampleRate <assumerate>` and
:doc:`ResampleAudio <resampleaudio>`.

The time-stretch algorithm has a few parameters that can be tuned to optimize
sound quality for certain applications. The current default parameters have
been chosen by iterative if-then analysis (read: "trial and error") to obtain
the best subjective sound quality in pop/rock music processing, but in
applications processing different kind of sound the default parameter set may
result in a sub-optimal result.

The time-stretch algorithm default parameter values are

::

    Sequence     82
    SeekWindow   28
    Overlap      12

These parameters affect the time-stretch algorithm as follows:

-   *Sequence*: This is the length of a single processing sequence in
    milliseconds which determines how the original sound is chopped in the
    time-stretch algorithm. Larger values mean fewer sequences are used in
    processing. In principle a larger value sounds better when slowing down
    the tempo, but worse when increasing the tempo and vice versa.

-   *SeekWindow*: The seeking window length in milliseconds is for the
    algorithm that searches for the best possible overlap location. This
    determines from how wide a sample "window" the algorithm can use to find
    an optimal mixing location when the sound sequences are to be linked back
    together.

The bigger this window setting is, the higher the possibility of finding a
better mixing position becomes, but at the same time large values may cause a
"drifting" sound artifact because neighboring sequences may be chosen at more
uneven intervals. If there's a disturbing artifact that sounds as if a
constant frequency was drifting around, try reducing this setting.

-   *Overlap*: The overlap length in milliseconds. When the sound sequences
    are mixed back together to form a continuous sound stream again, this
    parameter defines how much of the ends of the consecutive sequences will
    be overlapped.

This shouldn't be that critical parameter. If you reduce the Sequence setting
by a large amount, you might wish to try a smaller value on this.

-   *QuickSeek*: The time-stretch routine has a 'quick' mode that
    substantially speeds up the algorithm but may degrade the sound quality.
-   *aa*: Controls the number of tap the Anti-alias filter uses for the
    rate changer. Set to 0 to disable the filter. The value must be a
    multiple of 4.

The table below summarizes how the parameters can be adjusted for different
applications:

+----------------+--------------------------+---------------------------+---------------------------+---------------+-----------------+-----------------------------+
| Parameter name | Default value magnitude  | Larger value affects...   | Smaller value affects...  | Music         | Speech          | Effect in CPU burden        |
+================+==========================+===========================+===========================+===============+=================+=============================+
| Sequence       | Default value is         | Larger value is usually   | Smaller value might be    | Default value | A smaller value | Increasing the parameter    |
|                | relatively large, chosen | better for slowing down   | better for speeding up    | usually good  | than default    | value reduces computation   |
|                | for slowing down music   | tempo. Growing the value  | tempo. Reducing the value |               | might be better | burden                      |
|                | tempo                    | decelerates the "echoing" | accelerates the "echoing" |               |                 |                             |
|                |                          | artifact when slowing     | artifact when slowing     |               |                 |                             |
|                |                          | down the tempo.           | down the tempo            |               |                 |                             |
+----------------+--------------------------+---------------------------+---------------------------+---------------+-----------------+-----------------------------+
| SeekWindow     | Default value is         | Larger value eases        | Smaller reduce            | Default value | Default value   | Increasing the parameter    |
|                | relatively large, chosen | finding a good mixing     | possibility to find a     | usually good, | usually good    | value increases computation |
|                | for slowing down music   | position, but may cause a | good mixing position, but | unless a      |                 | burden                      |
|                | tempo                    | "drifting" artifact       | reduce the "drifting"     | "drifting"    |                 |                             |
|                |                          |                           | artifact.                 | artifact is   |                 |                             |
|                |                          |                           |                           | disturbing.   |                 |                             |
+----------------+--------------------------+---------------------------+---------------------------+---------------+-----------------+-----------------------------+
| Overlap        | Default value is         |                           | If you reduce the         |               |                 | Increasing the parameter    |
|                | relatively large, chosen |                           | "sequence ms" setting,    |               |                 | value increases computation |
|                | to suit with above       |                           | you might wish to try a   |               |                 | burden                      |
|                | parameters.              |                           | smaller value.            |               |                 |                             |
+----------------+--------------------------+---------------------------+---------------------------+---------------+-----------------+-----------------------------+

**Notes:**

-   This is NOT a sample exact plugin. If you use it, slight inaccuracies
    might occur. Since we are dealing with float values rounding errors might
    occur, especially on large samples. In general however inaccuracies
    should not exceed a few 10's of milliseconds for movielength samples.


-   Currently the SoundTouch library only supports 1 and 2 channels. When
    used with more than 2 channels, each channel is processed individually in
    1 channel mode. This will destroy the phase relationship between the
    channels. See this thread for details :- `TimeStretch in AVISynth 2.5.5
    Alpha - Strange stereo effects ?`_


-   SoundTouch is used in float sample mode.

**Examples:**

::

    TimeStretch(pitch = 200)

This will raise the pitch one octave, while preserving the length of the
original sample.

::

    TimeStretch(pitch = 100.0*pow(2.0, 1.0/12.0))

This will raise the pitch one semi-tone, while preserving the length of the
original sample.

::

    TimeStretch(tempo = 25.0/(24000.0/1001.0)*100.0)

This will change the tempo from Film speed to PAL speed without changing the
pitch.

**Credits:**

This function uses:

SoundTouch library Copyright (c) Olli Parviainen 2002-2006

| `<http://www.iki.fi/oparviai/soundtouch>`_
| `<http://www.surina.net/soundtouch>`_

+-----------+------------------------------+
| Changelog |                              |
+===========+==============================+
| v2.55     | Initial Release              |
+-----------+------------------------------+
| v2.57     | Expose soundtouch parameters |
+-----------+------------------------------+

$Date: 2010/04/04 16:46:19 $

.. _TimeStretch in AVISynth 2.5.5 Alpha - Strange stereo effects ?:
    http://forum.doom9.org/showthread.php?t=71632
