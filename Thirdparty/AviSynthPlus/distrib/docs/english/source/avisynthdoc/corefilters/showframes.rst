
ShowFrames
==========

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



ShowFrameNumber
---------------

``ShowFrameNumber`` (clip, bool "scroll", int "offset", float "x", float "y",
string "font", float "size", int "text_color", int "halo_color", float
"font_width", float "font_angle")

``ShowFrameNumber`` draws text on every frame indicating what number AviSynth
thinks it is. This is sometimes useful when writing scripts. If you apply
additional filters to the clip produced by ``ShowFrameNumber``, they will
treat the text on the frame just as they would treat an image, so the numbers
may be distorted by the time you see them.

If scroll (default: false) is set to true the framenumber will be drawn only
once on the video and scroll from top to bottom, else it will be drawn on the
right side as often as it fits. For top field first material the framenumber
will be drawn on the left side of the clip, for bottom field first material
on the right side and for field-based material it will be drawn alternating
on the left side and right side of the clip (depending whether the field is
top or bottom).

Starting from v2.56 other options ..., x, y, font, size, text_color,
halo_color, font_width, font_angle) are present, see :doc:`Subtitle <subtitle>` for an
explanation of these options.

offset enables the user to add an offset to the shown framenumber.


ShowSMPTE
---------

``ShowSMPTE`` (clip, float "fps", string "offset", int "offset_f", float "x",
float "y", string "font", float "size", int "text_color", int "halo_color",
float "font_width", float "font_angle")

``ShowSMPTE`` is similar to ``ShowFrameNumber`` but displays SMPTE timecode
(hours:minutes:seconds:frame). Starting from v2.53 the fps argument is not
required, unless the current fps can't be used. Otherwise, the fps argument
is required and must be 23.976, 24, 25, 29.97, or 30.

Starting from v2.56 other options (x, y, font, size, text_color, halo_color,
font_width, font_angle) are present, see :doc:`Subtitle <subtitle>` for an explanation of
these options.

offset enables the user to add an offset to the timecode, while offset_f
enables the user to add an offset to the timecode specifying the number of
frames (offset takes precedence over offset_f).

**drop-frame versus non-drop-frame timecode**

For some framerates the `drop-frame timecode`_ is enabled. If for example the
framerate of the clip is between 29.969 and 29.971 it is enabled. Originally,
when the signal of the TV was black and white, NTSC run at 60 Hz (30 fps).
When they added color, they changed it to 59.94 Hz (29.97 fps) due to
technical reasons. They run 1000 frames, but count 1001 (they never actually
drop a frame, just a frame number). The first two frames are dropped of every
minute except the tenth, ie 00:00:00:00, 00:00:00:01, 00:00:00:02, ...,
00:00:59:29, 00:01:00:02, 00:01:00:03, ..., 00:01:59:29, 00:02:00:02,
00:02:00:03, ..., 00:08:59:29, 00:09:00:02, 00:09:00:03, ..., 00:09:59:29,
00:10:00:00, 00:10:00:01, etc ... Counting the dropped frames implies that
00:10:00:00 in drop-frame matches 00:10:00:00 in real time.

Something similar holds also for the following framerates:

+--------------------+-----------+
| framerate range    | base-rate |
+====================+===========+
| [29.969, 29.971]   | 30        |
+--------------------+-----------+
| [47.951, 47.953]   | 48        |
+--------------------+-----------+
| [59.939, 59.941]   | 60        |
+--------------------+-----------+
| [119.879, 119.881] | 120       |
+--------------------+-----------+

**Examples**
::

    ShowSMPTE(offset="00:00:59:29", x=360, y=576, font="georgia",
      /       size=24, text_color=$ff0000)

    Mpeg2Source("clip.d2v") # is always top field first
    # will draw the framenumber on the left side of the clip using
    # an offset of 9 frames, scrolling from top to bottom
    ShowFrameNumber(scroll=true, offset=9, text_color=$ff0000)

ShowTime
--------

``ShowTime`` (clip, int "offset_f", float "x", float "y", string "font",
float "size", int "text_color", int "halo_color", float "font_width", float
"font_angle")

``ShowTime`` is similar to ``ShowSMPTE`` but it displays the time duration
(hours:minutes:seconds.ms). See ShowSMPTE for an explanation of the options.

Take care: these filters are quite slow due to the text-drawing.

+---------+-----------------------------------------------------------------------------+
| Changes |                                                                             |
+=========+=============================================================================+
| v2.60   | All functions: position (x,y) can be float (previously int) (with 0.125     |
|         | pixel granularity). ShowSMPTE: added drop-frame for other framerates (other |
|         | than 30).                                                                   |
+---------+-----------------------------------------------------------------------------+
| v2.58   || Added ShowTime function.                                                   |
|         || Added font_width, font_angle args.                                         |
+---------+-----------------------------------------------------------------------------+
| v2.56   | Added offset and other options.                                             |
+---------+-----------------------------------------------------------------------------+

$Date: 2012/03/16 20:53:18 $

.. _drop-frame timecode:
   http://teched.vt.edu/gcc/HTML/VirtualTextbook/PDFs/AdobeTutorialsPDFs/Premiere/PremiereTimecode.pdf
