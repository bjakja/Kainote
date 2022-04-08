
Trim / AudioTrim
================

| ``Trim`` (clip, int first_frame, int last_frame [, bool "pad"])
| ``Trim`` (clip, int first_frame, int -num_frames [, bool "pad"])
| ``Trim`` (clip, int first_frame, int "end" [, bool "pad"])
| ``Trim`` (clip, int first_frame, int "length" [, bool "pad"])

| ``AudioTrim`` (clip, float start_time, float end_time)
| ``AudioTrim`` (clip, float start_time, float -duration)
| ``AudioTrim`` (clip, float start_time, float "end")
| ``AudioTrim`` (clip, float start_time, float "length")

``Trim`` trims a video clip so that it includes only the frames first_frame
up to last_frame (first_frame and last_frame are included). The audio is
similarly trimmed so that it stays synchronized. Remember AviSynth starts
counting at frame 0.

Prior to v2.60, to trim an audio-only clip, you may not just set a fake frame
rate with :doc:`AssumeFPS <assumerate>`. Instead, you must make a :doc:`BlankClip <blankclip>`, use
:doc:`AudioDub <audiodub>`, trim *that*, and then :doc:`KillVideo <killaudio>`. Otherwise, AviSynth returns
an error message "cannot trim if there is no video".

Since v2.60 you can trim the audio using ``AudioTrim``. The start_time,
end_time and duration need to be specified in seconds (but can be float).
Like ``Trim`` it keeps only the audio samples corresponding to start_time up
to end_time. The target source clip does not need to have a video track. If
present the video is similarly trimmed so that it stays synchronized within 1
frame duration.

*pad* (default true) causes the audio stream to be padded to align with the
video stream. Otherwise the tail of a short audio stream is left so. When
last_frame=0 and pad=false the end of the two streams remains independent.

Since v2.60 you can also use AudioTrim/Trim(3, end=7) instead of
AudioTrim/Trim(3, 7) and AudioTrim/Trim(3, length=7) instead of
AudioTrim/Trim(3, -7). Note, the End and Length explicitly named parameters
have no discontinuous boundary values. End=0 means end at frame 0. Length=0
means return a zero length clip. These are most useful in avoiding unexpected
boundary conditions in your user functions.

**Examples:**

::

    Trim(100,0)             # delete the first 100 frames, audio padded
                            # or trimmed to match the video length.
    Trim(100,0,false)       # delete the first 100 frames of audio and video,
                            # the resulting stream lengths remain independent.
    Trim(100,-100)          # is the same as trim(100,199)
    Trim(100,199,false)     # audio will be trimmed if longer but not
                            # padded if shorter to frame 199
    Trim(0,-1)              # returns only the first frame
    Trim(0,End=0)           #
    Trim(0,Length=1)        #
    AudioTrim(1,5.5)        # keeps the audio samples between 1 and 5.5 seconds
    AudioTrim(1,End=5.5)    #
    AudioTrim(1,-5.5)       # cuts the first second and keeps the following 5.5 seconds
    AudioTrim(1,Length=5.5) #

+-----------+--------------------------------------------+
| Changelog |                                            |
+===========+============================================+
| v2.60     || Added AudioTrim.                          |
|           || Added explicit length and end parameters. |
+-----------+--------------------------------------------+
| v2.56     | added pad audio                            |
+-----------+--------------------------------------------+

$Date: 2012/04/15 14:59:42 $
