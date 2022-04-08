
AviSynth Syntax - String functions
==================================

String functions provide common operations on string variables.

-   LCase   |   v2.07   |   LCase(string)

Returns lower case of string.

*Examples:*
::

    LCase("AviSynth") = "avisynth"

-   UCase   |   v2.07   |   UCase(string)

Returns upper case of string.

*Examples:*
::

    UCase("AviSynth") = "AVISYNTH"

-   StrLen   |   v2.07   |   StrLen(string)

Returns length of string.

*Examples:*
::

    StrLen("AviSynth") = 8

-   RevStr   |   v2.07   |   RevStr(string)

Returns string backwards.

*Examples:*
::

    RevStr("AviSynth") = "htnySivA"

-   LeftStr   |   v2.07   |   LeftStr(string, int)

Returns first int number of characters.

*Examples:*
::

    LeftStr("AviSynth", 3) = "Avi"

-   RightStr   |   v2.07   |   RightStr(string, int)

Returns last int number of characters.

*Examples:*
::

    RightStr("AviSynth", 5) = "Synth"

-   MidStr   |   v2.07   |   MidStr(string, int pos [, int length])

Returns substring starting at *pos* for optional *length* or to end. ``pos=1``
specifies start.

*Examples:*
::

    MidStr("AviSynth", 3, 2) = "iS"

-   FindStr   |   v2.07   |   FindStr(string, substring)

Returns position of substring within string. Returns 0 if substring is not
found.

*Examples:*
::

    Findstr("AviSynth", "Syn") = 4

-   FillStr   |   v2.60   |   FillStr(int [, string]) Fills a string.
    When int>1 it concatenates the string int times. String is space by
    default.

*Examples:*
::

    FillStr(1, "AviSynth") = "AviSynth"
    FillStr(2, "AviSynth") = "AviSynthAviSynth"

-   StrCmp   |   v2.60   |   StrCmp(string, string)

Compares two character strings. The comparison is case-sensitive. If the
first string is less than the second string, the return value is negative. If
it's greater, the return value is positive. If they are equal, the return
value is zero. (The actual value seems to be host operating system dependent
so it can't be relied upon.)

*Examples:*
::

    StrCmp("AviSynth", "AviSynth") = 0 # strings are equal.
    StrCmp("AviSynth", "Avisynth") != 0 # strings are not equal.

-   StrCmpi   |   v2.60   |   StrCmpi(string, string)

Compares two character strings. The comparison is not case-sensitive. If the
first string is less than the second string, the return value is negative. If
it's greater, the return value is positive. If they are equal, the return
value is zero. (The actual value seems to be host operating system dependent
so it can't be relied upon.)

*Examples:*
::

    StrCmpi("AviSynth", "AviSynth") = 0 # strings are equal.
    StrCmpi("AviSynth", "Avisynth") = 0 # strings are equal.
    StrCmpi("abcz", "abcdefg") != 0 # returns the difference betweeen "z"
    and "d" (which is positive).

-   Chr   |   v2.51   |   Chr(int)

Returns the ASCII character. Note that characters above the ASCII character
set (ie above 127) are code page dependent and may render different (visual)
results in different systems. This has an importance only for user-supplied
localised text messages.

*Examples:*
::

    Chr(34) returns the quote character
    Chr(9) returns the tab  character

-   Ord   |   v2.60   |   Ord(string) Gives the ordinal number of the
    first character of a string.

*Examples:*
::

    Ord("a") = 97
    Ord("AviSynth") = Ord("A") = 65
    Ord("§") = 167

-   Time   |   v2.51   |   Time(string)

Returns a string with the current system time formatted as defined by the
string. The string may contain any of the codes for output formatting
presented below:

+--------+---------------------------------------------------+
| Code   | Description                                       |
+========+===================================================+
| %a     | Abbreviated weekday name                          |
|        |                                                   |
| %A     | Full weekday name                                 |
+--------+---------------------------------------------------+
| %b     | Abbreviated month name                            |
|        |                                                   |
| %B     | Full month name                                   |
+--------+---------------------------------------------------+
| %c     | Date and time representation                      |
|        | appropriate for locale                            |
+--------+---------------------------------------------------+
| %d     | Day of month as decimal number (01 ? 31)          |
+--------+---------------------------------------------------+
| %H     | Hour in 24-hour format (00 ? 23)                  |
|        |                                                   |
| %I     | Hour in 12-hour format (01 ? 12)                  |
+--------+---------------------------------------------------+
| %j     | Day of year as decimal number (001 ? 366)         |
+--------+---------------------------------------------------+
| %m     | Month as decimal number (01 ? 12)                 |
+--------+---------------------------------------------------+
| %M     | Minute as decimal number (00 ? 59)                |
+--------+---------------------------------------------------+
| %p     | Current locale's A.M./P.M.                        |
|        | indicator for 12-hour clock                       |
+--------+---------------------------------------------------+
| %S     | Second as decimal number (00 ? 59)                |
+--------+---------------------------------------------------+
| %U     | Week of year as decimal number,                   |
|        | with Sunday as first day of week (00 ? 53)        |
+--------+---------------------------------------------------+
| %w     | Weekday as decimal number (0 ? 6; Sunday is 0)    |
+--------+---------------------------------------------------+
| %W     | Week of year as decimal number,                   |
|        | with Monday as first day of week (00 ? 53)        |
+--------+---------------------------------------------------+
| %x     | Date representation for current locale            |
+--------+---------------------------------------------------+
| %X     | Time representation for current locale            |
+--------+---------------------------------------------------+
| %y     | Year without century, as decimal number (00 ? 99) |
|        |                                                   |
| %Y     | Year *with* century, as decimal number            |
+--------+---------------------------------------------------+
| %z, %Z | Time-zone name or abbreviation;                   |
|        | no characters if time zone is unknown             |
+--------+---------------------------------------------------+
| %%     | Percent sign                                      |
+--------+---------------------------------------------------+

The # flag may prefix any formatting code. In that case, the meaning of the
format code is changed as follows:

+-------------------------------+---------------------------------------------------------------------------------+
| Code with # flag              | Change in meaning                                                               |
+===============================+=================================================================================+
| %#a, %#A, %#b, %#B,           |                                                                                 |
|                               | No change; # flag is ignored.                                                   |
| %#p, %#X, %#z, %#Z, %#%       |                                                                                 |
+-------------------------------+---------------------------------------------------------------------------------+
| %#c                           || Long date and time representation, appropriate for current locale. For example:|
|                               || ``?Tuesday, March 14, 1995, 12:41:29?.``                                       |
+-------------------------------+---------------------------------------------------------------------------------+
| %#x                           || Long date representation, appropriate to current locale. For example:          |
|                               || ``?Tuesday, March 14, 1995?.``                                                 |
+-------------------------------+---------------------------------------------------------------------------------+
| %#d, %#H, %#I, %#j, %#m, %#M, |                                                                                 |
|                               | Remove leading zeros (if any).                                                  |
| %#S, %#U, %#w, %#W, %#y, %#Y  |                                                                                 |
+-------------------------------+---------------------------------------------------------------------------------+

--------

Back to :doc:`Internal functions <syntax_internal_functions>`.

$Date: 2012/10/10 13:41:51 $
