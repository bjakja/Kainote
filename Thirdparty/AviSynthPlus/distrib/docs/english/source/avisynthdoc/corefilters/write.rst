
WriteFile / WriteFileIf / WriteFileStart / WriteFileEnd
=======================================================

| ``WriteFile`` (clip, string filename, *string expression1 [, string
  expression2 [, ...]], bool "append", bool "flush"*)
| ``WriteFileIf`` (clip, string filename, *string expression1 [, string
  expression2 [, ...]], bool "append", bool "flush"*)
| ``WriteFileStart`` (clip, string filename, *string expression1 [, string
  expression2 [, ...]], bool "append"*)
| ``WriteFileEnd`` (clip, string filename, *string expression1 [, string
  expression2 [, ...]], bool "append"*)

``WriteFile`` evaluates each expressionN, converts the result to a string and
puts the concatenated results into a file, followed by a newline.

The "run-time" variable current_frame is set so that you can use it in an
"expression"
(this works similar as with ScriptClip, look there in the docu for more
infos).
current_frame is set to -1 when the script is loaded and to -2 when the
script is closed.

``WriteFile`` evaluates the ''expression''s and generates output for each
frame rendered by the filter. ``WriteFileIf`` is similar, but generates
output only if the first expression is true. In both cases, there is no
output at script opening or closure. Note that since output is produced only
for ''rendered'' frames, there will be no output at all if the result of the
filter is not used in deriving the final result of the script.

``WriteFileStart`` and ``WriteFileEnd`` generate output only on script
opening and closure respectively, there is no action on each frame. In both
cases, the ''expression''s are evaluated exactly once, at the location of the
filter in the script.

When append = true, the result(s) will be appended to any existing file. The
default for append is always true, except for ``WriteFileStart`` (here it is
false).

When flush = true, the file is closed and reopened after each operation so
you can see the result immediately (this may be slower). The default for
flush (``WriteFile`` and ``WriteFileIf``) is true. For ``WriteFileStart``
and ``WriteFileEnd``, the file is always closed immediately after writing.


Usage is best explained with some simple examples
-------------------------------------------------

::

    filename = "c:\myprojects\output.txt"
    # create a test video to get frames
    Version()

    # the expression here is only a variable, which is evaluated and put in the file
    # you will get a file with the framenumber in each line
    WriteFile(filename, "current_frame")

    # this line is written when the script is opened
    WriteFileStart(filename, """ "This is the header" """)

    # and this when the script is closed
    WriteFileEnd(filename, """ "Now the script was closed" """)

Look how you can use triple-quotes to type a string in a string!

If the expression cannot be evaluated, the error message is written instead.
In case this happens with the If-expression in ``WriteFileIf`` the result is
assumed to be ``true``.

::

    # will result in "I don't know what "this" means"
    WriteFile(filename, "this is nonsense")

--------


There are easier ways to write numbers in a file, BUT
-----------------------------------------------------

... with this example you can see how to use the "runtime function"
AverageLuma:

::

    # create a test video to get different frames
    Version.FadeIn(50).ConvertToYV12

    # this will print the frame number, a ":" and the average luma for that frame
    colon = ": "
    WriteFile("F:\text.log", "current_frame", "colon", "AverageLuma")

Or maybe you want the actual time printed too:

::

    # create a test video to get different frames
    Version.FadeIn(50).ConvertToYV12

    # this will print the frame number, the current time and the average luma for that frame
    # the triple quotes are necessary to put quotes inside a string
    WriteFile(last, filename, "current_frame", """ time(" %H:%M:%S") """, "AverageLuma")

--------


More examples
-------------

In ``WriteFileIf`` the FIRST expression is expected to be boolean (true or
false). Only if it is TRUE the other expressions are evaluated and the line
is printed. (Remember: && is AND, || is OR, == is EQUAL, != is NOT EQUAL)
That way you can omit lines completely from your file.

::

    # create a test video to get different frames
    Version.FadeIn(50).ConvertToYV12

    # this will print the frame number, but only of frames where AverageLuma is between 30 and 60
    WriteFileIf(last, filename, "(AverageLuma>30) && (AverageLuma<60)", "current_frame", """ ":" """, "AverageLuma")

+---------+------------------------------------------------------------+
| Changes |                                                            |
+=========+============================================================+
| v2.60   | Number of expressions changed from 16 to nearly unlimited. |
+---------+------------------------------------------------------------+
| v2.55   | Initial release.                                           |
+---------+------------------------------------------------------------+

$Date: 2010/01/06 16:01:28 $
