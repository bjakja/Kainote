
AssumeSampleRate
================

``AssumeSampleRate`` (clip, int samplerate)

``AssumeSampleRate`` (exists starting from *v2.07*) changes the sample rate
(playback speed) of the current sample.
If used alone, it will cause desync with the video, because the playback time
is not changed.

**Examples:**
::

    # Let's say that this video is 25fps, 44100hz video clip.
    AviSource("video_audio.avi")

    # Play audio at half speed:
    AssumeSampleRate(22050)

    # Play video at half speed:
    AssumeFPS(12.5)

    # Video and audio is now in sync, and plays in slow-motion.

$Date: 2011/12/04 15:28:44 $
