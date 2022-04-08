
FadeIn / FadeIn0 / FadeIn2 / FadeIO0 / FadeIO / FadeIO2 / FadeOut / FadeOut0 / FadeOut2
=======================================================================================

| ``FadeIn`` (clip clip, int num_frames, int "color", float "fps")
| ``FadeIO`` (clip clip, int num_frames, int "color", float "fps")
| ``FadeOut`` (clip clip, int num_frames, int "color", float "fps")

| ``FadeIn0`` (clip clip, int num_frames, int "color", float "fps")
| ``FadeIO0`` (clip clip, int num_frames, int "color", float "fps")
| ``FadeOut0`` (clip clip, int num_frames, int "color", float "fps")

| ``FadeIn2`` (clip clip, int num_frames, int "color", float "fps")
| ``FadeIO2`` (clip clip, int num_frames, int "color", float "fps")
| ``FadeOut2`` (clip clip, int num_frames, int "color", float "fps")

``FadeOut`` cause the video stream to fade linearly to black or the specified
RGB color at the end. Similarly ``FadeIn`` cause the video stream to fade
linearly from black or the specified RGB color at the start. ``FadeIO``
is a combination of the respective ``FadeIn`` and ``FadeOut`` functions. The
sound track (if present) also fades linearly to or from silence. The fading
affects only the last num_frames frames of the video. The last frame of the
video becomes almost-but-not-quite black (or the specified color). An
additional perfectly black (or the specified color) frame is added at the
end, thus increasing the total frame count by one.

``FadeIn0`` / ``FadeOut0`` do not include the extra frame. It is useful when
processing Audio only clips or chaining two or more fades to get a square law
or a cube law fading effects. e.g Clip.FadeOut0(60).FadeOut0(60).FadeOut(60)
gives a much sharper attack and gentler tailoff. The 50% point is at frame 12
of the fade, at frame 30 the fade is 12.5%, at frame 45, 1.6% the
effectiveness is more pronounced with audio.

``FadeIn2`` / ``FadeOut2`` works similarly, except that two black (or color)
frames are added at the end instead of one. The main purpose of this is to
work around a bug in Windows Media Player. All the WMP versions that I've
tested fail to play the last frame of an MPEG file - instead, they stop on
the next-to-last frame when playback ends. This leaves an unsightly almost-
but-not-quite-black frame showing on the screen when the movie ends if you
use ``FadeOut``. ``FadeOut2`` avoids this problem.

The *color* parameter is optional, default=0 <black>, and is specified as an
RGB value regardless of whether the clip format is RGB or YUV based. See
:doc:`here <../syntax/syntax_colors>` for more information on specifying colors.

The *fps* parameter is optional, default=24.0, and provides a reference for
num_frames in audio only clips. It is ignored if a video stream is present.
Set fps=AudioRate() if sample exact audio positioning is required.

``FadeOut`` (clip, n) is just a shorthand for
::

    Dissolve(clip, Blackness(clip, n+1, color=$000000), n)
    # (or instead of n+1, n+2 for FadeOut2 and n for FadeOut0).

+---------------+---------------------------------------------------------------------+
| Changelog:    |                                                                     |
+===============+=====================================================================+
| Until *v2.06* || The ``FadeIn`` / ``FadeIn2`` commands do not exist, but you        |
|               |  can get the same effect by reversing the arguments to Dissolve:    |
|               || Dissolve(Blackness(clip, n+1, color=$000000), clip, n).            |
+---------------+---------------------------------------------------------------------+
| *v2.07*       | ``FadeIO`` / ``FadeIO2`` commands are added and the color parameter |
|               | is added to all fade functions.                                     |
+---------------+---------------------------------------------------------------------+
| *v2.56*       | ``FadeIn0`` / ``FadeIO0`` / ``FadeOut0`` commands are added and     |
|               | the fps parameter is added to all fade functions.                   |
+---------------+---------------------------------------------------------------------+

$Date: 2009/10/11 11:43:32 $
