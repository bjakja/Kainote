
AviSynth Syntax - Boolean functions
===================================

Boolean functions return true or false, if the condition that they test holds
or not, respectively.

-   IsBool   |     |   IsBool(var)

Tests if *var* is of the bool type. *var* can be any expression allowed by
the :doc:`AviSynth Syntax <syntax>`.

*Examples:*
::

    b = false
    IsBool(b) = true
    IsBool(1 < 2 && 0 == 1) = true
    IsBool(123) = false

-   IsClip   |     |   IsClip(var)

Tests if *var* is of the clip type. *var* can be any expression allowed by
the :doc:`AviSynth Syntax <syntax>`.

*Examples:*
::

    c = AviSource(...)
    IsClip(c) = true
    IsClip("c") = false

-   IsFloat   |     |   IsFloat(var)

Tests if *var* is of the float type. *var* can be any expression allowed by
the :doc:`AviSynth Syntax <syntax>`.

*Examples:*
::

    f = Sqrt(2)
    IsFloat(f) = true
    IsFloat(2) = true   # ints are considered to be floats by this function
    IsFloat(true) = false

-   IsInt   |     |   IsInt(var)

Tests if *var* is of the int type. *var* can be any expression allowed by the
:doc:`AviSynth Syntax <syntax>`.

*Examples:*
::

    IsInt(2) = true
    IsInt(2.1) = false
    IsInt(true) = false

-   IsString   |     |   IsString(var)

Tests if *var* is of the string type. *var* can be any expression allowed by
the :doc:`AviSynth Syntax <syntax>`.

*Examples:*
::

    IsString("test") = true
    IsString(2.3) = false
    IsString(String(2.3)) = true

-   Exist   |   v2.07   |   Exist(filename)

Tests if the file specified by *filename* exists.

*Examples:*
::

    filename = ...
    clp = Exist(filename) ? AviSource(filename) : Assert(false,
    "file: " + filename + " does not exist")

-   Defined   |     |   Defined(var)

Tests if *var* is defined. Can be used inside :doc:`Script functions <syntax_userdefined_scriptfunctions>` to test if
an optional argument has been given an explicit value. More formally, the
function returns false if its argument (normally a function argument or
variable) has the void ('undefined') type, otherwise it returns true.

*Examples:*
::

    b_arg_supplied = Defined(arg)
    myvar = b_arg_supplied ? ... : ...

--------

Back to :doc:`Internal functions <syntax_internal_functions>`.

$Date: 2008/12/07 15:46:17 $
