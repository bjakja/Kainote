
AudioDub / AudioDubEx
=====================

``AudioDub`` (video_clip, audio_clip)
``AudioDubEx`` (video_clip, audio_clip)

``AudioDub`` takes the video stream from the first argument and the audio
stream from the second argument and combines them into a single clip. If
either track isn't available, it tries it the other way around, and if that
doesn't work it returns an error.

::

    # Load capture segments from patched AVICAP32 which puts
    # video in multiple AVI segments and audio in a WAV file
    video = AVISource("capture1.avi") + AVISource("capture2.avi")
    audio = WAVSource("capture.wav")
    AudioDub(video, audio)

``AudioDubEx`` takes the video stream from the first argument if present, the
audio stream from the second argument if present and combines them into a
single clip. Thus if you feed it with two video clips, and the second one has
no audio, the resulting clip will have the video of the first clip and no
audio. If you feed it with two audio clips, the resulting clip will have the
audio of the second clip and no video.

$Date: 2005/11/08 12:37:33 $
