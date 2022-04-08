
Subtitle
========

``Subtitle`` (clip, string text, float "x", float "y", int "first_frame", int
"last_frame", string "font", float "size", int "text_color", int
"halo_color", int "align", int "spc", int "lsp", float "font_width", float
"font_angle", bool "interlaced")

``Subtitle`` (clip, string text)

The ``Subtitle`` filter adds anti-aliased text to a range of frames. All
parameters after text are optional and can be omitted or specified out of
order using the *name=value* syntax.


Parameters
----------

*text* is the text which will be overlayed on the clip starting from frame
first_frame and ending with frame last_frame.

*(x,y)* is the position of the text. The parameters x and y can be set to -1 to
automatically calculate and use the horizontal or vertical center coordinate.
Other negative values of ''x'' and ''y'' can be used to give subtitles
partially off the screen. **Caution:** If your script uses Subtitle with
:doc:`Animate <animate>` and negative ''x'' or ''y'' values, ''x'' or ''y'' might
momentarily become -1, causing a glitch in the video. Starting from *v2.60*
they can be float.

*font* is the font of the text (all installed fonts on the current machine are
available, they are located in your ``'windows\fonts'`` folder).

*size* is the height of the text in pixels, and is rounded to the nearest 0.125
pixel.

The *text_color* and *halo_color* should be given as hexadecimal $aarrggbb
values, similar to HTML--except that they start with $ instead of # and the
4th octet specifies the alpha transparency. $00rrggbb is completely opaque,
$FF000000 is fully transparent. You can disable the halo by selecting this
color. See :doc:`here <../syntax/syntax_colors>` for more information on specifying colors.

The *align* parameter allows you to set where the text is placed relative to
the (x,y) location and is based on the numeric keypad as follows:

+----------------------------+-----------------------------+--------------------------+-----------------------------+
| <left> 7 <top>             | <center> 8 <top>            | <right> 9 <top>          | top of text aligned to      |
|                            |                             |                          | y-location for align=7,8,9  |
+----------------------------+-----------------------------+--------------------------+-----------------------------+
| <left> 4 <baseline>        | <center> 5 <baseline>       | <right> 6 <baseline>     | baseline of text aligned to |
|                            |                             |                          | y-location for align=4,5,6  |
+----------------------------+-----------------------------+--------------------------+-----------------------------+
| <left> 1 <bottom>          | <center> 2 <bottom>         | <right> 3 <bottom>       | bottom of text aligned to   |
|                            |                             |                          | y-location for align=1,2,3  |
+----------------------------+-----------------------------+--------------------------+-----------------------------+
| start at x for align=1,4,7 | center on x for align=2,5,8 | end at x for align=3,6,9 |                             |
+----------------------------+-----------------------------+--------------------------+-----------------------------+

Note: There is no vertical center alignment setting.

The *spc* parameter allows you to modify the character spacing (0=unchanged).
The value can be positive or negative to widen or narrow the text. Per the
Visual C++ documentation of the function SetTextCharacterExtra(), that
performs this task, this value is in logical units and rounded to the nearest
0.125 pixel. This is helpful for trying to match typical fonts on the PC to
fonts used in film and television credits which are usually wider for the
same height or to just fit or fill in a space with a fixed per-character
adjustment.

Multi-line text using "\n" is added in *v2.57* and it is used if the lsp
(line spacing) parameter is set. It sets the additional line space between
two lines in 0.125 pixel units.

The *font_width* parameter allows you to modify, in 0.125 units, the aspect
ratio of character glyphs per the Visual C++ documentation of the function
CreateFont(). It is related to the size parameter by the default GDI aspect
ratio and the natural aspect ratio of the chosen font.

The *font_angle* parameter allows you to modify the baseline angle of text in
0.1 degree increments anti-clockwise.

The *interlaced* parameter when enabled reduces flicker from sharp fine
vertical transitions on interlaced displays. It does this by increasing the
window for the anti-aliaser to include 0.5 of the pixel weight from the lines
above and below, it effectivly applies a mild vertical blur.

The short form (with all default parameters) form is useful when you don't
really care what the subtitle looks like as long as you can see it--for
example, when you're using :doc:`StackVertical <stack>` and its ilk to display several
versions of a frame at once, and you want to label them to remember which is
which.

This filter is used internally by AviSynth for the :doc:`Version <version>` command and for
reporting error messages, and the subtitling apparatus is also used by
:doc:`ShowFrameNumber <showframes>` and friends.

+-------------+----------------------------------------------------------------------------------+
| Parameter   | Default                                                                          |
+=============+==================================================================================+
| clip        | last                                                                             |
+-------------+----------------------------------------------------------------------------------+
| text        | no default, must be specified                                                    |
+-------------+----------------------------------------------------------------------------------+
| x           | 8 if align=1,4,7 or none; -1 if align=2,5,8; or width-8 if align=3,6,9           |
+-------------+----------------------------------------------------------------------------------+
| y           | height-1 if align=1,2,3; size if align=4,5,6 or none; or 0 if align=7,8,9        |
+-------------+----------------------------------------------------------------------------------+
| first_frame | 0                                                                                |
+-------------+----------------------------------------------------------------------------------+
| last_frame  | framecount(clip)-1                                                               |
+-------------+----------------------------------------------------------------------------------+
| font        | "Arial"                                                                          |
+-------------+----------------------------------------------------------------------------------+
| size        | 18.0                                                                             |
+-------------+----------------------------------------------------------------------------------+
| text_color  | $00FFFF00 <full opaque yellow>                                                   |
+-------------+----------------------------------------------------------------------------------+
| halo_color  | 0 <full opaque black>                                                            |
+-------------+----------------------------------------------------------------------------------+
| align       | Normally 4 <left and baseline>; if x=-1, then 5 <horizontal center and baseline> |
+-------------+----------------------------------------------------------------------------------+
| spc         | 0 <font spacing unchanged>                                                       |
+-------------+----------------------------------------------------------------------------------+
| lsp         | <multiline is not enabled>                                                       |
+-------------+----------------------------------------------------------------------------------+
| font_width  | 0 <system default>                                                               |
+-------------+----------------------------------------------------------------------------------+
| font_angle  | 0.0 degrees                                                                      |
+-------------+----------------------------------------------------------------------------------+
| interlaced  | false                                                                            |
+-------------+----------------------------------------------------------------------------------+

**Examples**
::

    # Some text in the center of the clip:
    AviSource("D:\clip.avi")
    Subtitle("Hello world!", align=5)

    # Some text in the upper right corner of the
    # clip with specified font, size and color red:
    AviSource("D:\clip.avi")
    Subtitle("Hello world!", font="georgia", size=24, \
             text_color=$ff0000, align=9)

    # Prints text on multiple lines
    # without any text halo border.
    BlankClip()
    Subtitle( \
      "Some text on line 1\\nMore text on line 1\n" + \
      "Some text on line 2", \
             lsp=10, halo_color=$ff000000)

    # It results in:
    Some text on line 1\nMore text on line 1
    Some text on line 2

    # Use String() to display values of functions.
    AviSource("D:\clip.avi")
    Subtitle("Width=" + String(Width()))

+-----------+-------------------------------------------------------------------------+
| Changelog |                                                                         |
+===========+=========================================================================+
| v2.60     | position (x,y) can be float (previously int) (with 0.125 pixel          |
|           | granularity).                                                           |
+-----------+-------------------------------------------------------------------------+
| v2.58     | Added font_width, font_angle, interlaced and alpha color blending.      |
+-----------+-------------------------------------------------------------------------+
| v2.57     | Added multi-line text and line spacing parameter.                       |
+-----------+-------------------------------------------------------------------------+
| v2.07     || Added align and spc parameters.                                        |
|           || Setting y=-1 calculates vertical center (alignment unaffected)         |
|           || Default x and y values dependent on alignment (previously x=8, y=size) |
+-----------+-------------------------------------------------------------------------+
| v1.00     || Setting x=-1 uses horizontal center and center alignment               |
|           || (undocumented prior to v2.07)                                          |
+-----------+-------------------------------------------------------------------------+

$Date: 2011/12/04 15:28:44 $
