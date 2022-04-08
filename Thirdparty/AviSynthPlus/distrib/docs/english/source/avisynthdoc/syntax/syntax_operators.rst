
AviSynth Syntax - Operators
===========================

As in all programming and scripting languages, operators in AviSynth script
language allow the performance of actions (operations) onto :doc:`variables <syntax_script_variables>`.
Operators form the basis for building up expressions, the building blocks of
AviSynth scripts.

AviSynth operators follow loosely the same rules as C operators, regarding
meaning, precedence and associativity. By loosely we mean that there are some
exceptions, indicated in the text below.

Note that when a binary operator is applied to an int and a float, the int is
first converted to float, as in C.


Available Operators per Type
----------------------------

For **all types** of operands (clip, int, float, string, bool) you can use
the following operators:

+--------+--------------------------------------+
| ``==`` | is equal                             |
+--------+--------------------------------------+
| ``!=`` | not equal                            |
+--------+--------------------------------------+
| ``<>`` | not equal (alternative to !=, v2.07) |
+--------+--------------------------------------+

String comparisons are not case-sensitive, so "abc" == "ABC" returns *true*.

For **numeric** types (int, float) you can use the following int/float-
specific operators:

+--------+-----------------------+
| ``+``  | add                   |
+--------+-----------------------+
| ``-``  | subtract              |
+--------+-----------------------+
| ``*``  | multiply              |
+--------+-----------------------+
| ``/``  | divide                |
+--------+-----------------------+
| ``%``  | mod                   |
+--------+-----------------------+
| ``>=`` | greater or equal than |
+--------+-----------------------+
| ``<=`` | less or equal than    |
+--------+-----------------------+
| ``<``  | less than             |
+--------+-----------------------+
| ``>``  | greater than          |
+--------+-----------------------+

*AviSynth in former versions parsed expressions from right to left, which
gave unexpected results.* For example:

-   a = 10 - 5 - 5 resulted in 10 - (5 - 5) = 10 instead of (10 - 5) - 5
    = 0 !
-   b = 100. / 2. / 4. resulted in 100. / (2. / 4.) = 200 instead of
    (100. / 2.) / 4. = 12.5 !

These "bugs" have been corrected in v2.53!

For **string** type you can use the following string-specific operators:

+--------+-------------------------------+
| ``+``  | concatenate                   |
+--------+-------------------------------+
| ``>=`` | greater or equal than (v2.07) |
+--------+-------------------------------+
| ``<=`` | less or equal than (v2.07)    |
+--------+-------------------------------+
| ``<``  | less than (v2.07)             |
+--------+-------------------------------+
| ``>``  | greater than (v2.07)          |
+--------+-------------------------------+

Like the equality operator, these string comparisons are case-insensitive.

For **clip** type you can use the following clip-specific operators:

+--------+-------------------------------------------------------------------------+
| ``+``  | the same as the function :doc:`UnalignedSplice <../corefilters/splice>` |
+--------+-------------------------------------------------------------------------+
| ``++`` | the same as the function :doc:`AlignedSplice <../corefilters/splice>`   |
+--------+-------------------------------------------------------------------------+

For **bool** type (true/false) you can use the following bool-specific
operators:

+--------+----------------------------+
| ``||`` | or                         |
+--------+----------------------------+
| ``&&`` | and                        |
+--------+----------------------------+
| ``?:`` | execute code conditionally |
+--------+----------------------------+
| ``!``  | not                        |
+--------+----------------------------+

The conditional execution operator is used as in the following example:

::

    b = (a==true) ? 1 : 2

This means in pseudo-basic:

::

    if (a=true) then b=1 else b=2

From version v2.07, AviSynth provides a NOP() function which can be used
inside a conditional execution block in cases where "else" may not otherwise
be desirable (such as a conditional :doc:`Import <../corefilters/import>` or :doc:`LoadPlugin <syntax_plugins>`).


Operator Precedence
-------------------

The precedence of AviSynth operators is presented at the table below.
Operators higher to the top of the table have higher precedence. Operators
inside the same row have the same order of precedence.

+--------+--------+--------+--------+---------+--------+--------+
| ``*``  | ``/``  | ``%``  |        |         |        |        |
+--------+--------+--------+--------+---------+--------+--------+
| ``+``  | ``++`` | ``-``  |        |         |        |        |
+--------+--------+--------+--------+---------+--------+--------+
| ``<``  | ``>``  | ``<=`` | ``>=`` |  ``!=`` | ``<>`` | ``==`` |
+--------+--------+--------+--------+---------+--------+--------+
| ``&&`` |        |        |        |         |        |        |
+--------+--------+--------+--------+---------+--------+--------+
| ``||`` |        |        |        |         |        |        |
+--------+--------+--------+--------+---------+--------+--------+
| ``?:`` |        |        |        |         |        |        |
+--------+--------+--------+--------+---------+--------+--------+

The dot symbol (**.**), used in the "OOP notation" for a function call, is
not strictly an operator, but effectively has a higher precedence than any
operator symbol. So for example, ``a + b.f(args)`` means ``a + f(b, args)``
and not ``f(a+b, args)`` (which could be written as ``(a+b).f(args)``)

$Date: 2012/03/11 16:05:32 $
