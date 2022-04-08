
User functions
==============

Having read the basics about :doc:`user-defined script functions <../syntax/syntax_userdefined_scriptfunctions>`, we can now
step forward to examine in detail each function building block and identify
rules for effective code development.

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



The function declaration (header)
---------------------------------

The function declaration consists of the keyword ``function`` followed by the
function's name and a (possibly empty) list of parameters (arguments)
enclosed in a pair of parentheses. Its purpose is to declare the function,
that is both make its name visible to the running script and state the number
and type of the arguments that it expects in subsequent invocations (function
calls).

Note that in AviSynth script language the declaration is also a definition;
the function body (the code that is executed every time the function is
called) must be supplied immediately after.


The function name
~~~~~~~~~~~~~~~~~

To name your user function you can pick any name that appropriately describes
the purpose of it. You should however avoid naming a function with an already
widely used name; the AviSynth script language namespace is flat and thus any
such name collision means you (and others) cannot use both functions
together. Note also that function names (as everything in AviSynth script
language) are case insensitive.


The argument list
~~~~~~~~~~~~~~~~~

Regarding the possible different kinds of arguments a function can declare,
there are two orthogonal to each other categorical divisions:

1.  **Typed** vs **variable** arguments.
2.  **Required** vs **optional** arguments.


Typed and variable (``val``) arguments
::::::::::::::::::::::::::::::::::::::

Typed arguments have a fixed type, decided by the specific type prefix (clip,
int, float, bool, string) used during function declaration. Whenever a script
is calling a function, AviSynth checks the supplied values for all typed
arguments to ensure that they are of the proper type; if a discrepancy is
found an error condition is triggered. Therefore, typed arguments can always
be assumed of being the correct type (but not 'value'!) inside the body of
the function, simplifying coding.

Variable arguments can accept *any* AviSynth type (clip, int, float, bool,
string) when the function is called. You can declare a function argument as
being variable with either of two ways:

-   specify ``val`` as the type of the argument, for example:

::

    function myfunc(clip c, val effect) { ... }
    function myfunc2(clip c, **val "action"**) { ... }

-   do *not* specify a type for the argument, for example:

::

    function myfunc(clip c, effect) { ... }
    function myfunc2(clip c, **"action"**) { ... }

As a side effect, whenever you neglet to provide the type of an argument you
will get a variable argument. Keep this in mind when you are debugging your
scripts.

Variable arguments can also be optional. To do so, you simply enclose the
argument in double quotes, as for typed arguments.

Variable arguments are useful in some situations because they provide
flexibility and reduce the size of the argument list. However, they have the
drawback that your function code has to check the type of each variable
argument in order to ensure its validity for the intended operation (for
typed arguments, the type check is performed by AviSynth).


Required and optional arguments
:::::::::::::::::::::::::::::::

Required arguments must always be supplied when you are calling the function

Optional arguments need not be supplied; they default (if the function is
coded correctly) to reasonable initial values.


The function body
-----------------

The function body contains the bulk of the code that makes up your function.
Since they strongly depend on the tasks-on-hand, the contents of the function
body are quite arbitrary. However, there are some frequently occuring coding
patterns that together form a more or less "standard" recipe for constructing
the function body. These are in the usual order of appearance the following:

-   Argument validation and setup of local variables.
-   Performance of intermediate computations.
-   Return of final computation outcome to the caller of the function.

We will now look closer on each one in the paragraphs that follow.


Argument validation and setup of local variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Performance of intermediate computations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Return of final computation outcome to the caller
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Designing and developing user functions
---------------------------------------


Defining goals
~~~~~~~~~~~~~~


Manipulating globals
~~~~~~~~~~~~~~~~~~~~

how to use effectively and safely


Recursion
~~~~~~~~~

the only tool to act upon collections


Tuning performance
------------------


Design and coding-style considerations
--------------------------------------


Organising user defined functions
---------------------------------


Back to :doc:`scripting reference <script_ref>`.

$Date: 2011/04/29 20:36:18 $
