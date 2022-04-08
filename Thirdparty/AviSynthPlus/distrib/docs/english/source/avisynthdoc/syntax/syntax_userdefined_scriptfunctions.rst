
AviSynth Syntax - User defined script functions
===============================================


Definition and Structure
------------------------

You can define and call your own functions in AviSynth scripts as shown
below. The function can return any clip or variable type. An user defined
script function is an independent block of script code that is executed each
time a call to the function is made in the script. An example of a simple
user defined script function (here a custom filter) immediately follows:

::

    function MuteRange(clip c, int fstart, int fend)
    {
        before = c.Trim(0, -fstart)
        current = c.Trim(fstart, fend)
        after = c.Trim(fend + 1, 0)
        audio = Dissolve(before, current.BlankClip, 3)
        return AudioDub(c, audio)
    }

User defined script functions start with the keyword ``function`` followed by
the function name. The name of a script function follows the same naming
rules as :doc:`script variables <syntax_script_variables>`.

Immediately after the name, the function's argument list follows. The list
(which can be empty) consists of (expected argument's type - argument's name)
pairs. Each argument's :doc:`type <syntax_script_variables>` may be any of those supported by the scripting
language.

::

    function MuteRange(clip c, int fstart, int fend)

Then comes the function body, ie the code that is executed each time the
function is called. The arguments are accessed within the function body by
their names. The function body is contained within an opening and closing
brace pair ``{ ... }``.

::

    {
        before = c.Trim(0, -fstart)
        current = c.Trim(fstart, fend)
        after = c.Trim(fend + 1, 0)
        audio = Dissolve(before, current.BlankClip, 3)
        return AudioDub(c, audio)
    }

(For simplicity, this code assumes the function will only be called with
fstart > 0 and fend < c.Framecount-1. A more complete version would also
handle the special cases fstart=0 and fend=c.Framecount-1.)

At the end of the function body a ``return`` statement which returns the
final value calculated from the arguments and the function's body code is
placed.

::

    return AudioDub(c, audio)

It should be noted that unlike other languages where multiple return
statements are allowed inside the function body, in AviSynth functions
contain a *single* return statement. This is because the language does not
support branching (i.e. compound block statements).


Facts about user defined script functions
-----------------------------------------

-   Functions can take up to sixty arguments and the return value can be
    of any type supported by the scripting language (clip, int, float, bool,
    string).

-   Although not recommended practice, an argument type may be omitted,
    and will default to ``val``, the generic type.

-   If the function expects a video clip as its first argument, and that
    argument is not supplied, then the clip in the special ``last`` variable
    will be used.

-   Functions support *named arguments*. Simply enclose an argument's
    name inside double quotes to make it a named argument. Note that after
    doing so the following apply:

    -   1.  All subsequent arguments in the argument list must be made named also.
    -   2.  **A named argument is an optional argument**, that is, it need not be supplied by the caller.
    -   3.  When a function is called, any optional argument which has *not*
            been provided is set to a value which has the void ('undefined') type.
            This does not mean that its value is random garbage - simply that its
            type is neither clip, int, float, bool or string and so it has *no*
            usable value.
    -   4.  Normally, you should use the :doc:`Defined <syntax_internal_functions_boolean>` function to test if an
            optional argument has an explicit value, or the :doc:`Default <syntax_internal_functions_control>` function,
            which combines the Defined test with the delivery of a default value if
            appropriate.
    -   5.  A void ('undefined') value can be passed on to another function
            as one of its optional arguments. This is useful when you want to write a
            wrapper function that calls another function, preserving the same
            defaults.

-   Functions always produce a new value and never modify an existing
    one. What that means is that all arguments to a function are passed "by
    value" and not "by reference"; in order to alter a variable's value in
    AviSynth script language you must assign to it a new value.

-   Functions can call other functions, *including theirselves*. The
    latter is known as recursion and is a very useful technique for creating
    functions that can accomplish complex tasks.

-   Local function variables mask global ones with the same name inside
    the function body. For example, if you define in a function a local
    variable ``myvar`` by assigning to it a value, then you cannot read the
    global ``myvar`` anymore inside this function.

-   The above is also true for arguments, since from the perspective of a
    function arguments are initialized local variables.

-   Each function has its own local version of the special variable
    *last*. On entry to a function, *last* is set to a void ('undefined')
    value.


Related Links
-------------

-   `Shared functions`_. An ever growing collection of shared script
    functions created by the members of the AviSynth community.

{TEMP: http://www.avisynth.org/ShareFunctions}

-   `Conditional filters and script functions`_. A collection of highly
    useful conditional filters implemented as user defined script functions.

| {TEMP:
| http://www.avisynth.org/ExportingSingleImage,
| http://www.avisynth.org/HowToApplyFilterToManySingleFrames,
| Perhaps make decent functions from the last two?}

$Date: 2013/03/19 18:10:27 $

.. _Shared functions: http://avisynth.org/mediawiki/Shared_functions
.. _Conditional filters and script functions: http://avisynth.org/mediawiki/index.php?title=Shared_functions/Conditional&action=edit
