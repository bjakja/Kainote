
AviSynth Syntax - Conversion functions
======================================

Conversion functions convert between different types. There are also some
:doc:`numeric functions <syntax_internal_functions_numeric>` that can be classified in this category, namely: ``Ceil,
Floor, Float, Int`` and ``Round``.

-   Value   |   v2.07   |   Value(string)

Converts a decimal string to its associated numeric value.

*Examples:*
::

    Value ("-2.7") = -2.7

-   HexValue   |   v2.07   |   HexValue(string)

Converts a hexadecimal string to its associated numeric value.

*Examples:*
::

    HexValue ("FF00") = 65280

-   Hex   |   v2.60   |   Hex(int)

Converts a numerical value to its hexadecimal value string. See `Colors`_ for
more information on specifying colors.

*Examples:*
::

    Hex (10824234) = "A52A2A"

-   String   |   v2.07   |   String(float / int [, string format_string])

Converts a variable to a string. If the variable is float or integer, it
first converts it to a float and then uses format_string to convert the float
to a string. The syntax of format_string is as follows:

- ``%[flags][width][.precision]f``

  - *width*: the minimum width (the string is never truncated)
  - *precision*: the number of digits printed
  - *flags*:

    - ``-`` left align (instead right align)
    - ``+`` always print the +/- sign
    - ``0`` padding with leading zeros
    - ``' '`` print a blank instead of a "+"
    - ``#`` always print the decimal point

You can also put arbitrary text around the format_string as defined above, similar to the C-language *printf* function.

*Examples:*
::

    Subtitle( "Clip height is " + String(last.height) )
    Subtitle( String(x, "Value of x is %.3f after AR calc") )
    Subtitle( "Value of x is " + String(x, "%.3f") + " after AR calc") )
    # same as above
    String(1.23, "%f") = '1.23'
    String(1.23, "%5.1f") = ' 1.2'
    String(1.23, "%1.3f") = '1.230'
    String(24, "%05.0f") = '00024'

--------

Back to :doc:`Internal functions <syntax_internal_functions>`.

$Date: 2012/02/08 08:50:12 $

.. _Colors: http://avisynth.org/mediawiki/Colors
