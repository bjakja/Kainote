
BlankClip / Blackness
=====================

``BlankClip`` (clip clip, int "length", int "width", int "height",
string "pixel_type", float "fps", int "fps_denominator", int "audio_rate",
int "channels", string "sample_type", int "color", int "color_yuv")


``BlankClip`` (clip clip, int "length", int "width", int "height",
string "pixel_type", float "fps", int "fps_denominator", int "audio_rate",
bool "stereo", bool "sixteen_bit", int "color", int "color_yuv")


``Blackness`` ()

The ``BlankClip`` filter produces a solid color, silent video clip of the
specified length (in frames). The clip passed as an argument is used as a
template for frame rate, image size, and so on, but you can specify all clip
properties without having to provide a template. color should be given as
hexadecimal RGB values. Without any argument, ``BlankClip`` will produce a
pitch-black 10 seconds clip (RGB32), 640x480, 24 fps, 16 bit 44100 Hz mono.

clip: if present, the resulting clip will have the clip-properties of the
template, except for the properties you define explicitly.

length: length of the resulting clip (in frames).

width, height: width and height of the resulting clip.

pixel_type: pixel type of the resulting clip, it can be "RGB24", "RGB32",
"YUY2" or "YV12".

fps: the framerate of the resulting clip.

fps_denominator: you can use this option if "fps" is not accurate enough. For
example: fps = 30000, fps_denominator = 1001 (ratio = 29.97) or fps = 24000,
fps_denominator = 1001 (ratio = 23.976). It is 1 by default.

audio_rate: samplerate of the silent audio of the clip.

channels: specifies the number of audio channels of silent audio added to the
blank clip ( added in *v2.58*).

stereo: (boolean) when set to true the silent audio is in stereo, when set to
false a silent mono track is added. Deprecated! Use should the channels
parameter instead.

sample_type: specifies the audio sample type of the resulting clip. It can be
"8bit", "16bit", "24bit", "32bit" or "float" ( added in *v2.58*).

sixteen_bit: (boolean) true give 16 bit, false gives ieee float. Deprecated!
Use the sample_type parameter instead.

color: specifies the color of the clip, black (= $000000) by default. See
ColorPresets for preset colors. See :doc:`here <../syntax/syntax_colors>` for more information on
specifying colors.

color_yuv: is added in *v2.55*, and it lets you specify the color of the clip
using YUV values. It requires setting  pixel_type to "YUY2" or "YV12",
otherwise it doesn't do anything.

``Blackness`` is an alias for ``BlankClip``, provided for backward
compatibility.

**Examples:**
::

    # produces a black clip (3000 frames, width 720, height 576, framerate 25),
    # with a silent audio track (16 bit 44.1 kHz stereo):
    BlankClip(length=3000, width=720, height=576, fps=25, color=$000000)

    # produces a black clip (3000 frames) with the remaining clip properties of the avi:
    video = AviSource("E:\pdwork\DO-Heaven.AVI")
    BlankClip(video, length=3000, color=$000000)

    # adds a silent audio stream (with a samplerate of 48 kHz) to a video clip:
    video = AviSource("E:\pdwork\DO-Heaven.AVI")
    audio = BlankClip(video, audio_rate=48000)
    AudioDub(video, audio)

+----------+--------------------------------+
| Changes: |                                |
+==========+================================+
| v2.55    | added color_yuv                |
+----------+--------------------------------+
| v2.58    | added channels and sample_type |
+----------+--------------------------------+

$Date: 2011/04/29 20:09:50 $
