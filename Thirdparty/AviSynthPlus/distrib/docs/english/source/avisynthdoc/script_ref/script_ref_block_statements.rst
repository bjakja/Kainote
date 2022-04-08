
Block statements
================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Background
----------

A first glance at Avisynth documentation leaves the impression that aside
from function definitions, block statements are not possible in Avisynth
script. However, there are specific features of the language allowing the
construction of block statements that have remained unaltered to date and
probably will remain so in the future since block statements are very useful
in extending the capabilities of the script language.

Indeed, in most programming and scripting languages, block statements are
very useful tools for grouping together a set of operations that should be
applied together under certain conditions. They are also useful in Avisynth
scripts.

Assume, for example, that after an initial processing of your input video
file, you want to further process your input differently (for example, apply
a different series of :doc:`filters <../corefilters>` or apply the same set of filters with
different order) based on a certain condition calculated during the initial
processing, which is coded at the value of Boolean variable *cond*.

Instead of making an ugly series of successive conditional assignments using
the conditional (ternary) :doc:`operator <../syntax/syntax_operators>`, ``?:``,
as in **Example 1** below (items in brackets are not needed if you use the
implicit *last* variable to hold the result):

**Example 1**

::

    [result_1 = ]cond ? filter1_1 : filter2_1
    [result_2 = ]cond ? filter1_2 : filter2_2
    ...
    [result_n = ]cond ? filter1_n : filter2_n

It would be nice to be able to construct two blocks of filter operations and
branch in a single step, as in the (ideal) **Example 2** below:

**Example 2**

::

    [result = ] cond ? {
        filter1_1
        filter1_2
        ...
        filter1_n
    } : {
        filter2_1
        filter2_2
        ...
        filter2_n
    }

Something approaching this construction (and others) **is** possible; perhaps
some constraints may apply, but you will nevertheless be capable of providing
more powerful flow control to your scripts. The rest of this section will
show you how to implement them using standard Avisynth constructs.

(An alternative, and possibly more user-friendly, approach would be to use
the external `GScript`_ plugin, which extends the Avisynth scripting language
to provide multi-line conditionals (if-then-else blocks), 'while' loops and
'for' loops.)


Features enabling construction of block statements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The list below briefly presents the features making possible the creation of
block statements in your script. Listed first are the more obvious ones,
followed by those that are somewhat more esoteric and require a little
digging inside the Avisynth documentation and experimenting with test cases
to discover them.

-   globals (in particular, variables preceded by the "global" keyword)
    allow the communication of information between code blocks executing in
    different context.

-   The conditional operator (condition ? expr_if_true : expr_if_false)
    can contain an arbitrary number of nested expressions, if grouped by
    parentheses.

-   Strings can contain double quote (") characters inside them if they
    are surrounded by three-double-quotes (""").  Thus, the following strings
    are valid in Avisynth script (note that the 2nd and 3rd ones could be
    lines in a script):

    -   """this is a string with " inside it"""
    -   """var = "a string value" """
    -   """var = "a string value" # this is a comment"""

-   There is a script function, :doc:`Eval <../syntax/syntax_internal_functions_control>`, that allows the evaluation of
    strings containing arbitrary script expressions.  Thus, every expression
    that you can write in a script can be, if stored in a string, passed to
    Eval. Eval returns the result of the evaluated expression, ie *anything*
    that can be constructed by such an expression (a clip, a number, a bool,
    a string). The evaluation of the string is done in the same context as
    the call to Eval. Thus, if Eval is called at the script level, the
    expression is assumed to reference script-level variables or / and
    globals (globals are allowed everywhere). But if Eval is called inside a
    user-defined function then the expression is assumed to reference
    variables local to the function (ie arguments and any locally declared
    variable

-   There is a script function, :doc:`Import <../corefilters/import>`, that allows the evaluation
    of arbitrary Avisynth scripts.  Thus, any script written in Avisynth
    script language can be evaluated by Import. Import returns the return
    value of the script.

Despite the common misbelief that this can only be a clip, it can actually be
*any* type of variable (a clip, a number, a bool, a string).

Like Eval, the evaluation of the script is done in the same context as the
call to Import.

Hence, as a side-effect of the script evaluation any functions and variables
declared inside the imported script are accessible from the caller script,
from the point of the Import call and afterwards.

-   Recursion (ie calling a function from inside that function) can be
    used for traversing elements of a collection.  Thus, for..next,
    do..while, do..until loops can be constructed by using recursion.

-   Multiline strings, ie strings that contain newlines (the CR/LF pair)
    inside them, are allowed by the script language.

-   Multiline strings are parsed by :doc:`Eval <../syntax/syntax_internal_functions_control>` as if they were scripts.
    Thus, each line of a multiline string will be evaluated as if it was a
    line in a script. Also, return statements inside the string are allowed
    (the value of their expression will be the return value of Eval(), as
    well as comments, function calls and in general every feature of the
    script language.

Consider the following **Example 3**, of a (useless) script that returns some
black frames followed by some white frames:

**Example 3**

::

     c = BlankClip().Trim(0,23)
     d = BlankClip(color=$ffffff).Trim(0,23)
     b = true
     dummy = b ? Eval("""
         k = c       # here comments are allowed!
         l = d
         return k    # this will be stored in dummy
         """) : Eval("""
         k = d
         l = c
         return k    # this will be stored in dummy
         """)
     # variables declared inside a multiline string
     # are available to the script after calling Eval
     return k + l

Variables *k*, *l* are not declared anywhere before the evaluation of the
if..else block. However, since Eval evaluates the string at the script-level
context, it is as if the statements inside the string were written at the
script level. Therefore, after Eval() they are available to the script. A few
other interesting things to note are the following:

-   The return statement at the end of the selected (by the value of *b*)
    string for evaluation is the value that will be returned to the *dummy*
    variable.

-   Contrary to the case of line continuation by backslashes, a multiline
    string allows comments everywhere that they would be allowed in a script.


Implementation Guide
--------------------

The features above can be used to construct block statements in various ways.
The most common implementation cases are presented in this section, grouped
by block statement type.


The if..else block statement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Using Eval() and three-double-quotes quoted strings
:::::::::::::::::::::::::::::::::::::::::::::::::::

This is by far the more flexible implementation, since the flow of text
approaches most the "natural" (ie the commonly used in other languages) way
of branching code execution.

Using the rather common case illustrated by **Example 1**, the solution would
be (again items in square brackets are optional):

**Example 4**

::

    [result = ] cond ? Eval("""
        filter1_1
        filter1_2
        ...
        filter1_n
      [ return {result of last filter} ]
        """) : Eval("""
        filter2_1
        filter2_2
        ...
        filter2_n
      [ return {result of last filter} ]
       """)

In short, you write the code blocks as if Avisynth script would support block
statements and then enclose the blocks in three-double-quotes to make them
multiline strings, wrap a call to :doc:`Eval <../syntax/syntax_internal_functions_control>` around each string and finally
assemble Eval calls into a conditional operator statement.

The return statements at the end of each block are needed only if you want to
assign a useful value to the *result* variable. If you simply want to execute
the statements without returning a result, then you can omit the *return*
statement at the end of each block.

One important thing to note is that the implicit setting of *last* continues
to work as normal inside the Eval block. If the result of Eval is assigned to
a variable, *last* will not be updated for the final expression in the block
(with or without *return*), but it will be (where appropriate) for other
statements in the block.

If the block statement produces a result you intend to use, it is clearer to
enter a *return {result}* line as the last line of each block, but the
keyword *return* is not strictly necessary.

The following real-case examples illustrate the above:

**Example 5** In this example, all results are assigned to script variables,
so *last* is unchanged.

::

    c = AviSource(...)
    ...
    cond = {expr}
    ...
    cond ? Eval("""
        text = "single double quotes are allowed inside three-double-
        quotes"
        pos = FindStr(text, "llo")   # comments also
        d = c.Subtitle(LeftStr(text, pos - 1))
    """) : Eval("""
        text = "thus by using three-double-quotes you can write
        expressions like you do in a script"
        pos = FindStr(text, "tes")
        d = c.SubTitle(MidStr(text, pos + StrLen("tes")))
    """)
    return d

**Example 6** This example assigns a different clip to d depending on the
:doc:`Framecount <../syntax/syntax_clip_properties>` of a source clip.

::

    a = AviSource(...)
    c = BlankClip().Subtitle("a test case for an if..else block
    statement")
    d = a.Framecount >= c.Framecount ? Eval_("""
        a = a.BilinearResize(c.Width, c.Height)
        c = c.Tweak(hue=120)
        return Overlay(a, c, opacity=0.5)
    """) : Eval("""
        c = c.BilinearResize(a.Width, a.Height)
        a = a.Tweak(hue=120)
        return Overlay(c, a, opacity=0.5)
    """)
    return d

**Example 7** This example is a recode of Example 6 using implicit assignment
to the *last* special variable. Since the result of the entire Eval() is not
assigned to another variable, the implicit assignments to *last* on each line
of the string (including the *last line* of the string) are preserved and
thus the desired result is obtained.

::

    c = BlankClip().SubTitle("a test case for an if..else block statement")
    AviSource(...)
    last.Framecount >= c.Framecount ? Eval("""
        BilinearResize(c.Width, c.Height)
        c = c.Tweak(hue=120)
        Overlay(last, c, opacity=0.5)
    """) : Eval("""
        c = c.BilinearResize(last.Width, last.Height)
        Tweak(hue=120)
        Overlay(c, last, opacity=0.5)
    """)

The only disadvantage of the Eval approach is that coding errors inside the
string blocks are masked by the :doc:`Eval <../syntax/syntax_internal_functions_control>` call, since the parser actually
parses a **single line** of code:

::

    [result = ] cond ? Eval("""block 1""") : Eval("""block 2""")

Thus, any error(s) inside the blocks will be reported as a single error
happening on the above line. You will not be pointed to the exact line of
error as in normal script flow. Therefore, you will have to figure out where
exactly the error occured, which can be a great debugging pain, especially if
you write big blocks.


Using separate scripts as blocks and the Import() function
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Using **Example 1** as above, the solution would be (again items in square
brackets are optional):

**Example 8** Code of script file *block1.avs*:

::

    filter1_1
    filter1_2
    ...
    filter1_n

Code of script file *block2.avs*:

::

    filter2_1
    filter2_2
    ...
    filter2_n

Code of main script where the conditional branch is desired:

::

    ...
    [result = ]cond ? Import("block1.avs") : Import("block2.avs")
    ...

In short, you create separate scripts for each block and then conditionally
import them at the main script.

If you need to pass :doc:`variables <../syntax/syntax_script_variables>` as "parameters" to the blocks, declare them
in your main script and just reference them into the block scripts. The
following example demonstrates this:

**Example 9** Code of script file *block1.avs*:

::

    filter1_1(..., param1, ...)
    filter1_2(..., param2, ...)
    ...
    filter1_n(..., param3, ...)

Code of script file *block2.avs*:

::

    filter2_1(..., param1, ...)
    filter2_2(..., param2, ...)
    ...
    filter2_n(..., param3, ...)

Code of main script where the conditional branch is desired:

::

    # variables must be defined *before* importing the block script
    param1 = ...
    param2 = ...
    param3 = ...
    ...
    [result = ]cond ? Import("block1.avs") : Import("block2.avs")
    ...

Using :doc:`Import <../corefilters/import>` instead of :doc:`Eval <../syntax/syntax_internal_functions_control>` and three-double-quoted multiline
strings has some disadvantages:

-   There is an administration overhead because instead of one file *2k +
    1* files have to be maintained (*k* = the number of conditional branches
    in your script).
-   The code has less clarity, in the sense that it does not visually
    appears as a block statement, neither the communication of parameters is
    apparent by inspection of the main script.

On the other hand:

-   Debugging is not an issue; every error will be reported with accurate
    line information.
-   You can reuse scripts that you frequently use and build more complex
    ones by simply importing ready-made components.
-   For large-scale operations where few parameters have to be
    communicated it is usually a better approach.

One useful general purpose application of this implementation is to
prototype, test and debug a block conditional branch and then recode it (by
adding the Eval() and three-double-quotes wrapper code and removing the
:doc:`global <../syntax/syntax_script_variables>` keyword before the parameter's declarations) so that a single
script using multiline strings as blocks is created. This workaround
compensates for the main disadvantage of the Eval() and three-double-quotes
implementation.


Using functions (one function for each block)
:::::::::::::::::::::::::::::::::::::::::::::

This is the most "loyal" to the Avisynth script's :doc:`syntax <../syntax/syntax_ref>` approach. Using
**Example 1** as above, the solution would be (again items in square brackets
are optional):

**Example 10**

::

    Function block_if_1()
    {
        filter1_1
        filter1_2
        ...
        filter1_n
    }

    Function block_else_1()
    {
        filter2_1
        filter2_2
        ...
        filter2_n
    }
    ...
    [result = ]cond ? block_if_1() : block_else_1()
    ...

In short, you create separate functions for each block and then conditionally
call them at the branch point.

If you need to pass variables as "parameters" to the blocks, either declare
them *global* in your main script and just reference them into the functions
or - better - use argument lists at the functions. The following example
demonstrates this:

**Example 11**

::

    Function block_if_1(arg1, arg2, arg3, ...)
    {
        filter1_1(..., arg1, ...)
        filter1_2(..., arg2, ...)
        ...
        filter1_n(..., arg3, ...)
    }

    Function block_else_1(arg1, arg2, arg3, ...)
    {
        filter2_1(..., arg1, ...)
        filter2_2(..., arg2, ...)
        ...
        filter2_n(..., arg3, ...)
    }
    ...
    [result = ]cond \
        ? block_if_1(arg1, arg2, arg3, ...) \
        : block_else_1(arg1, arg2, arg3, ...)
    ...

Compared to the other two implementations this one has the following
disadvantages:

-   There is an extra overhead due to the need for supplying function
    headers and (typically) argument lists.
-   It tends to "pollute" the global namespace, thus having the potential
    of strange errors due to name conflicts; use a clear naming scheme, as
    the suggested above.

On the other hand:

-   It is **portable**; it does not depend on any type of hack or
    specific behavior to work. It is thus guaranteed to continue working in
    the long term.
-   It does not raise any special debuging difficulties.
-   It has coding clarity.


The if..elif..else block statement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By nesting If..Else block expressions inside the conditional operator, you
can create entire if..elseif...else conditional constructs of any level
desired to accomodate more complex needs.

A generic example for each if..else implementation presented above is
following. Of course, any combination of the three above pure cases is
possible.


Using Eval() and three-double-quotes quoted strings
:::::::::::::::::::::::::::::::::::::::::::::::::::

The solution would be (again items in square brackets are optional):

**Example 12**

::

    [result = \]
        cond_1 ? Eval("""
            statement 1_1
            ...
            statement 1_n
        """) : [(] \
        cond_2 ? Eval(""" # inner a?b:c enclosed in parentheses for
        clarity (optional)
            statement 2_1
            ...           # since backslash line continuation is
            used between Eval blocks
            statement 2_n # place comments only inside the
            strings
        """) : [(] \
        ...
        cond_n ? Eval("""
            statement n_1
            ...
            statement n_n
        """) \
        : Eval("""
            statement n+1_1
            ...
            statement n+1_n
        """)[...))]  # 1 closing parenthesis for Eval() + n-1 to
        balance the opening ones (if used)

Using separate scripts as blocks and the Import() function
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

The solution would be (again items in square brackets are optional):

**Example 13**

::

    # here no comments are allowed; every line but the last must end with a \
    [result = \]
        cond_1 ? \
            Import("block1.avs") : [(] \
        cond_2 ? \
            Import("block2.avs") : [(] \
        ...
        cond_n ? \
            Import("blockn.avs") \
        : \
            Import("block-else.avs") \
        )...))  # n-1 closing parentheses to balance the opening ones

Using functions (one function for each block)
:::::::::::::::::::::::::::::::::::::::::::::

The solution would be (again items in square brackets are optional):

**Example 14**

::

    # here no comments are allowed; every line but the last must end with a \
    [result = \]
        cond_1 ? \
            function_block_1({arguments}) : [(] \
        cond_2 ? \
            function_block_2({arguments}) : [(] \
        ...
        cond_n ? \
            function_block_n({arguments}) \
        : \
            function_block_else({arguments}) \
        [)...))]  # n-1 closing parentheses to balance the opening
        ones (if used)

The for..next block statement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The problem here is to implement the ``for..next`` loop in a way that allows
accessing variables in the local scope, so that changes made in local scope
variables inside the loop can be accessible by the caller when it is
finished. This is the way that the ``for..next`` loop works in most
programming languages that provide it. In addition, a means for getting out
of the loop before is finished (ie breaking out of the loop) should be
available.

There is of course the alternative to implement the ``for..next`` loop in a
way that does not allow access to local variables. This is easier in
AviSynth, since then it can be implemented by a function; but it is also less
useful. However in many cases it would be appropriate to use suc a construct
and thus it will be presented here.


For..Next loop with access to variables in local scope
::::::::::::::::::::::::::::::::::::::::::::::::::::::

1.  Use a ``ForNext(start, end, step, blocktext)`` function to create a
    multiline string (a script) that will unroll the loop in a series of
    statements and then
2.  use Eval() to execute the script in the current scope.

The ``blocktext`` is a script text, typically a multiline string in triple
double quotes, that contains the instructions to be executed in each loop,
along with special variables (say ${i} for the loop counter) that are
textually replaced by the ``ForNext`` function with the current value(s) in
each loop. The `StrReplace()`_ function is particularly suited for the
replacement task.

A little tweak is needed in order to implement the ``break`` statement; the
unrolled string must be constructed in such a way that when the break flag is
set the rest of the code is skipped.

The following proof-of-concept example demonstrates the procedure:

::

    a = AviSource("c:\some.avi")
    cnt = 12
    b = a.Trim(0,-4)
    cond = false

    # here we would like to do the following
    # for (i = 0; i < 6; i++) {
    #    b = b + a.Trim(i*cnt, -4)
    #    cond = b.Framecount() > 20 ? true : false
    #    if (cond)
    #        break
    # }

    return b

In order to make this happen in AviSynth, our script with ``ForNext`` would
look like that:

::

    a = AviSource("c:\some.avi")
    cnt = 12
    b = a.Trim(0,-4)
    cond = false
    block = ForNext(0, 5, 1, """
        b = b + a.Trim(${i}*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        ${break(cond)}
        """)
    void = Eval_(block)
    return b

or more succinctly:

::

    a = AviSource("c:\some.avi")
    cnt = 12
    b = a.Trim(0,-4)
    void = Eval(ForNext(0, 5, 1, """
        b = b + a.Trim(${i}*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        ${break(cond)}
        """))
    return b

and the output of ForNext with the above arguments should be something like
this (the only problem is that string literals cannot be typed inside the
block text):

::

    """
    __break = false
    dummy = __break ? NOP : Eval("
        b = b + a.Trim(0*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        __break = cond ? true : false
    ")
    dummy = __break ? NOP : Eval("
        b = b + a.Trim(1*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        __break = cond ? true : false
    ")
    dummy = __break ? NOP : Eval("
        b = b + a.Trim(2*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        __break = cond ? true : false
    ")
    dummy = __break ? NOP : Eval("
        b = b + a.Trim(3*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        __break = cond ? true : false
    ")
    dummy = __break ? NOP : Eval("
        b = b + a.Trim(4*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        __break = cond ? true : false
    ")
    dummy = __break ? NOP : Eval("
        b = b + a.Trim(5*cnt, -4)
        cond = b.Framecount() > 20 ? true : false
        __break = cond ? true : false
    ")
    """

TO BE CONTINUED...


For..Next loop without access to variables in local scope
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

If we don't care for accessing variables in the local scope, then the
implementation is straightforward:

1.  Create an :doc:`AVSLib array <script_ref_arrays>` with the appropriate loop values.
2.  Define needed globals (for example a bool flag to return immediately
    from the block if true).
3.  Pack the block's code inside a function.
4.  Use an `array operator`_ to execute the block for every loop value.

TO BE CONTINUED...


The do..while and do..until block statements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO...


Deciding which implementation to use
------------------------------------

To be frank, there is no clear-cut answer to this question; it depends on the
purpose that the script will serve, your coding abilities and habits, whether
there are ready-made components available and what type are they (scripts,
function libraries, etc.) and similar factors.

Thus, only some generic guidelines will be presented here, grouped on the
type of block statement


The if..else and if..elif..else block statements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   For short (up to say 10 lines) blocks, using Eval() and three-double-
    quotes quoted strings is generally the best solution; it is fast to code
    and presents a "natural" text flow to the reader (thus it is easy to
    comprehend).

-   For long blocks, using any of the other two implementations is
    generally better because it is easier to debug.

-   If the blocks pre-exist as independent scripts, using :doc:`Import <../corefilters/import>` is,
    obviously, preferred.

-   If building a function library, usually an implementation with
    functions will be easier to maintain and debug. However using :doc:`Eval <../syntax/syntax_internal_functions_control>`
    for small blocks is still an option to consider, to minimise the risk of
    namespace clashing with user's own functions.


The for..next block statement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO...


The do..while and do..until block statements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO...


References
----------

[1] `<http://www.avisynth.org/stickboy/ternary_eval.html>`_

[2] `<http://forum.doom9.org/showthread.php?t=102929>`_

[3] `<http://forum.doom9.org/showthread.php?p=732882#post732882>`_

--------

Back to :doc:`scripting reference <script_ref>`.

$Date: 2011/04/29 20:11:14 $

.. _GScript: http://forum.doom9.org/showthread.php?t=147846
.. _StrReplace():
    http://avslib.sourceforge.net/functions/s/strreplace.html
.. _array operator:
    http://avslib.sourceforge.net/tutorials/operators.html
