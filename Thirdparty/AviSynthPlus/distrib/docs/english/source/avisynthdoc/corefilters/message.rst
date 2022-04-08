
MessageClip
===========

``MessageClip`` (string message, int "width", int "height", bool "shrink",
int "text_color", int "halo_color", int "bg_color")

``MessageClip`` produces a clip containing a text message; used internally
for error reporting.
Arial font is used, size between 24 points and 9 points chosen to fit, if
possible, in the width by height clip.
If shrink is true, the clip resolution is then reduced, if necessary, to fit
the text.

$Date: 2004/03/09 21:28:07 $
