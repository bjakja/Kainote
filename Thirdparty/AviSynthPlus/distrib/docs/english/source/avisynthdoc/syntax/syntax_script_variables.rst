
AviSynth Syntax - Script variables
==================================

This page shows how to use *variables* to store intermediate values for
further processing in a script. It also describes the types of data that
scripts can manipulate, and how *literals* (constants) of those types are
written.

A *variable name* can be a character string of practically any length (more
than 4000 characters in Avisynth 2.56 and later) that contains (English)
letters, digits, and underscores (_), but no other characters. The name
cannot start with a digit.

You may use characters from your language system codepage (locale) in strings
and file names (ANSI 8 bit only, not Unicode).

A variable's placement in an *expression* is determined by the
:doc:`AviSynth Syntax <syntax_ref>`.

Variables can have a value of one of the following *types*:

-   clip

A video clip containing video and / or audio. A script must return a value of
this type.

-   string

A sequence of characters representing text. String literals are written as
text surrounded either by "quotation marks" or by """three quotes""". The
text can contain any characters except the terminating quotation mark or
triple-quote sequence. The manual used to mention *TeX-style quotes*, but it
has been confirmed that AviSynth doesn't work this way since v1.03. If you
need to put a quotation mark inside a string, you need to use `Python-style`_
"""three quotes""". For example:
::

    Subtitle("""AVISynth is as they say "l33t".""")

Alternatively, you can use Windows extended-ASCII curly-quotes
inside the string instead of straight quotes to get around this limitation.

-   int

An integer (32 bits, signed). An integer literal is entered as a sequence of
digits, optionally with a + or - at the beginning. The value can be given in
*hexadecimal* by preceding them with a "$" character. For example ``$FF`` as
well as ``$ff`` (case does not matter) are equal to 255.

-   float

A single-precision, `floating-point`_ number. Literals are entered as a
sequence of digits with a decimal point (.) somewhere in it and an optional +
or -. For example, +1. is treated as a floating-point number. Note that
exponent-style notation is **not** supported.

-   bool

Boolean values must be either *true* or *false*. In addition they can be
written as ''yes'' or ''no'', but you should avoid using these in your
scripts (they remain for compatibility purposes only).

-   val

A generic type name. It is applicable only inside a
:doc:`user defined script functions <syntax_userdefined_scriptfunctions>` argument list,
in order to be able to declare an argument variable to be of *any* type (int, float, bool, string, or clip). You must
then explicitly test for its type (using the :doc:`boolean functions <syntax_internal_functions_boolean>`) and take
appropriate actions.

There is another type which is used internally by Avisynth - the void or
'undefined' type. Its principal use is in conjunction with optional function
arguments. See the :doc:`Defined() <syntax_internal_functions_boolean>` function.

Variables can be either local (bound to the local scope of the executing
script block) or global. Global variables are bound to the global script
environment's scope and can be accessed by all :doc:`Internal functions <syntax_internal_functions>`,
:doc:`User defined script functions <syntax_userdefined_scriptfunctions>`, :doc:`runtime environment <syntax_runtime_environment>` scripts and the main
script also.

To define and / or assign a value to a global variable you must precede its
name with the keyword ``global`` at the left side of the assignment. The
keyword is not needed (actually it is not allowed) in order to read the value
of a global variable. Examples:

::

    global canvas = BlankClip(length=200, pixel_type="yv12")
    global stroke_intensity = 0.7
    ...
    global canvas = Overlay(canvas, pen, opacity=stroke_intensity, mask=brush)

To declare a variable, simply type the variable name, followed by '=' (an
equals sign), followed by its initial value. The type must not be declared;
it is inferred by the value assigned to it (and can actually be changed by
subsequent assignments). The only place where it is allowed (though not
strictly required) to declare a variable's type is in
:doc:`user defined script functions <syntax_userdefined_scriptfunctions>` argument lists. Examples:

::

    b = false      # this declares a variable named 'b' of type 'bool' and initializes it to 'false'
    x = $100       # type int (initial value is in hexadecimal)
    y = 256        # type int (initial value is in decimal)
    global f = 0.0 # type float declared globally
    ...
    function my_recolor_filter(clip c, int new_color, float amount, val
    "userdata") { ... }

$Date: 2011/12/04 15:27:59 $

.. _Python-style: http://forum.doom9.org/showthread.php?s=&threadid=71597
.. _floating-point: http://en.wikipedia.org/wiki/Floating_point
