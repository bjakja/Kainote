
AviSynth Syntax - Numeric functions
===================================

Numeric functions provide common mathematical operations on numeric
variables.

-   Max   |   v2.58   |   Max(float, float [, ...])

Returns the maximum value of a set of numbers.
If all the values are of type Int, the result is an Int. If any of the values
are of type Float, the result is a Float.
This may cause an unexpected result when an Int value greater than 16777216
is mixed with Float values.

*Examples:*
::

    Max (1, 2) = 2
    Max (5, 3.0, 2) = 5.0

-   Min   |   v2.58   |   Min(float, float [, ...])

Returns the minimum value of a set of numbers.

*Examples:*
::

    Max (1, 2) = 1
    Max (5, 3.0, 2) = 2.0

-   MulDiv   |   v2.56   |   MulDiv(int, int, int)

Multiplies two ints (m, n) and divides the product by a third (d) in a single
operation, with 64 bit intermediate result. The actual equation used is ``(m
* n + d / 2) / d``.

*Examples:*
::

    MulDiv (1, 1, 2) = 1
    MulDiv (2, 3, 2) = 3

-   Floor   |     |   Floor(float)

Converts from float to int (round down on any fractional amount).

*Examples:*
::

    Floor(1.2) = 1
    Floor(1.6) = 1
    Floor(-1.2) = -2
    Floor(-1.6) = -2

-   Ceil   |     |   Ceil(float)

Converts from float to int (round up on any fractional amount).
*Examples:*
::

    Ceil(1.2) = 2.0
    Ceil(1.6) = 2.0
    Ceil(-1.2) = -1
    Ceil(-1.6) = -1

-   Round   |     |   Round(float)

Converts from float to int (round off to nearest integer).
*Examples:*
::

    Round(1.2) = 1
    Round(1.6) = 2
    Round(-1.2) = -1
    Round(-1.6) = -2

-   Sin   |   v2   |   Sin(float)

Returns the sine of the argument (assumes it is radians).
*Examples:*
::

    Sin(Pi()/4) = 0.707
    Sin(Pi()/2) = 1.0

-   Cos   |   v2   |   Cos(float)

Returns the cosine of the argument (assumes it is radians).
*Examples:*
::

    Cos(Pi()/4) = 0.707
    Cos(Pi()/2) = 0.0

-   Tan   |   v2.60   |   Tan(float)

Returns the tangent of the argument (assumes it is radians).
*Examples:*
::

    Tan(Pi()/4) = 1.0
    Tan(Pi()/2) = not defined

32 bit ieee floats do not have sufficient resolution to exactly represent
?/2 so AviSynth returns a large positive number for the value slightly less
than ?/2 and a large negative value for the next possible value which is
slightly greater than ?/2.

-   Asin   |   v2.60   |   Asin(float)

Returns the inverse of the sine of the argument (output is radians).

*Examples:*
::

    Asin(0.707) = 0.7852471634 (~ Pi/4)
    Asin(1.0) = 1.570796327 (~ Pi/2)

-   Acos   |   v2.60   |   Acos(float)

Returns the inverse of the cosine of the argument (output is in radians).

*Examples:*
::

    Acos(0.707) = 0.7852471634 (~ Pi/4)
    Acos(0.0) = 1.570796327 (~ Pi/2)

-   Atan   |   v2.60   |   Atan(float)

Returns the inverse of the tangent of the argument (output is in radians).

*Examples:*
::

    Atan(0.707) = 0.6154085176
    Atan(1.0) = 0.7853981634 (~ Pi/4)

-   Atan2   |   v2.60   |   Atan2(float, float)

Returns the angle between the positive x-axis of a plane and the point given
by the coordinates (x, y) on it (output is in radians). See `the wikipedia article on Atan2`_ for
more information. y is the first argument and x is the second argument.

*Examples:*
::

    Atan2(1.0, 0) = 1.570796327 (~ Pi/2)
    Atan2(1.0, 1.0) = 0.7852471634 (~ Pi/4)
    Atan2(-1.0, -1.0) = -2.356194490 (~ -3Pi/4)

-   Sinh   |   v2.60   |   Sinh(float)

Returns the hyperbolic sine of the argument. See `wikipedia`_ for more
information.

*Examples:*
::

    Sinh(2.0) = 3.626860408

-   Cosh   |   v2.60   |   Cosh(float)

Returns the hyperbolic cosine of the argument.

*Examples:*
::

    Cosh(2.0) = 3.762195691

-   Tanh   |   v2.60   |   Tanh(float)

Returns the hyperbolic tangent of the argument.

*Examples:*
::

    Tanh(2.0) = 0.9640275801

-   Fmod   |   v2.60   |   Fmod(float, float)

Returns the modulo of the argument. Output is float.

*Examples:*
::

    Fmod(3.5, 0.5) = 0 (since 3.5 - 7*0.5 = 0)
    Fmod(3.5, 1.0) = 0.5 (since 3.5 - 3*1.0 = 0.5)

-   Pi   |   v2   |   Pi()

Returns the value of the "pi" constant (the ratio of a circle's circumference
to its diameter).

*Examples:*
::

    d = Pi()    # d == 3.141593

-   Tau   |   v2.60   |   Tau()

Returns the value of the "tau" constant (the ratio of a circle's
circumference to its radius). See `Tau_(2?)`_ for more information.

*Examples:*
::

    d = Tau()   # d == 6.283186

-   Exp   |   v2   |   Exp(float)

Returns the natural (base-e) exponent of the argument.

*Examples:*
::

    Exp(1) = 2.718282
    Exp(0) = 1.0

-   Log   |   v2   |   Log(float)

Returns the natural (base-e) logarithm of the argument.

*Examples:*
::

    Log(1) = 0.0
    Log(10) = 2.30259
    Log(Exp(1)) = 1.0

-   Log10   |   v2.60   |   Log10(float)

Returns the common logarithm of the argument.

*Examples:*
::

    Log10(1.0) = 0
    Log10(10.0) = 1.0
    Log10(2.0) = 0.3010299957

-   Pow   |   v2   |   Pow(float base, float power)

Returns "base" raised to the power indicated by the second argument.

*Examples:*
::

    Pow(2, 3) = 8
    Pow(3, 2) = 9
    Pow(3.45, 1.75) = 8.7334

-   Sqrt   |   v2   |   Sqrt(float)

Returns the square root of the argument.

*Examples:*
::

    Sqrt(1) = 1.0
    Sqrt(2) = 1.4142

-   Abs   |   v2.07   |   Abs(float or int)

Returns the absolute value of its argument (returns float for float, integer
for integer).

*Examples:*
::

    Abs(-3.8) = 3.8
    Abs(-4) = 4

-   Sign   |   v2.07   |   Sign(float)

Returns the sign of the value passed as argument (1, 0 or -1).

*Examples:*
::

    Sign(-3.5) = -1
    Sign(3.5) = 1
    Sign(0) = 0

-   Int   |   v2.07   |   Int(float)

Converts from single-precision, `floating-point`_ value to int (round towards
zero).

*Examples:*
::

    Int(1.2) = 1
    Int(1.6) = 1
    Int(-1.2) = -1
    Int(-1.6) = -1

-   Frac   |   v2.07   |   Frac(float)

Returns the fractional portion of the value provided.

*Examples:*
::

    Frac(3.7) = 0.7
    Frac(-1.8) = -0.8

-   Float   |   v2.07   |   Float(int)

Converts int to single-precision, `floating-point`_ value. Integer values
that require more than 24-bits to be represented will have their lower 8-bits
truncated yielding unexpected values.

*Examples:*
::

    Float(4) = 4.0
    Float(4) / 3 = 1.333 (while 4 / 3 = 1 , due to integer division)

-   Rand   |   v2.07   |   Rand([int max] [, bool scale] [, bool seed])

Returns a random integer value. All parameters are optional.

-   *max* sets the maximum value+1 (default 32768) and can be set
    negative for negative results. It operates either in scaled or modulus
    mode (default scale=true only if abs(max) > 32768, false otherwise).
-   Scaled mode (scale=true) scales the internal random number
    generator value to the maximum value, while modulus mode (scale=false)
    uses the remainder from an integer divide of the random generator value
    by the maximum. I found modulus mode is best for smaller maximums.
-   Using *seed=true* seeds the random number generator with the current
    time. *seed* defaults to false and probably isn't necessary, although
    it's there just in case.

Typically, this function would be used with the Select function for random
clips.

*Examples:*
::

    Select(Rand(5), clip1, clip2, clip3, clip4, clip5)

-   Spline   |   v2.51   |   Spline(float X, x1, y1, x2, y2, .... [, bool
    cubic])

Interpolates the Y value at point X using the control points x1/y1, ... There
have to be at least 2 x/y-pairs. The interpolation can be cubic (the result
is a spline) or linear (the result is a polygon). Default is cubic.

*Examples:*
::

    Spline(5, 0, 0, 10, 10, 20, 0, false) = 5
    Spline(5, 0, 0, 10, 10, 20, 0, true) = 7

-   ContinuedNumerator   |   v2.60   |   ContinuedNumerator(float, int
    limit)
-   ContinuedNumerator   |   v2.60   |   ContinuedNumerator(int, int, int
    limit)
-   ContinuedDenominator   |   v2.60   |   ContinuedDenominator(float,
    int limit)
-   ContinuedDenominator   |   v2.60   |   ContinuedDenominator(int, int,
    int limit)

The rational pair (ContinuedNumerator,ContinuedDenominator) returned has the
smallest possible denominator such that the absolute error is less than
1/limit. More information can be found on `the wikipedia article for Continued fraction`_. If *limit* is not
specified in the Float case the rational pair returned is to the limit of the
single precision floating point value. Thus (float)((double)Num/(double)Den)
== V. In the Int pair case if *limit* is not specified then the normalised
original values will be returned, i.e. reduced by the GCD.

*Examples:*
::

    ContinuedNumerator(PI(), limit=5000]) = 355
    ContinuedDenominator(PI(), limit=5000) = 113

    ContinuedNumerator(PI(), limit=50]) = 22
    ContinuedDenominator(PI(), limit=50) = 7

    ContinuedNumerator(355, 113, limit=50]) = 22
    ContinuedDenominator(355, 113, limit=50) = 7

-   BitAnd   |   v2.60   |   BitAnd(int, int)

The functions: BitAnd, BitNot, BitOr, BitXor, etc, are bitwise operators.
This means that their arguments (being integers) are converted to binary
numbers, the operation is performed on their bits, and the resulting binary
number is converted back again. BitAnd returns the bitwise AND (sets bit to 1
if both bits are 1 and sets bit to 0 otherwise).

*Examples:*
::

    BitAnd(5, 6) = 4 # since 5 = 101, 6 = 110, and 101&110 = 100

-   BitNot   |   v2.60   |   BitNot(int)

Returns the bit-inversion (sets bit to 1 if bit is 0 and vice-versa).

*Examples:*
::

    BitNOT(5) = -6 # since 5 = 101, and ~101 = 11111111111111111111111111111010

Note: 1111 1111 1111 1111 1111 1111 11111010 = (2^32-1)-2^0-2^2 = 2^32-(1+2^0+2^2) = (signed) -(1+2^0+2^2) = -6.

-   BitOr   |   v2.60   |   BitOr(int, int)

Returns the bitwise inclusive OR (sets bit to 1 if one of the bits (or both)
is 1 and sets bit to 0 otherwise).

*Examples:*
::

    BitOr(5, 6) = 7 # since 5 = 101, 6 = 110, and 101|110 = 111
    BitOr(4, 2) = 6 # since 4 = 100, 2 = 010, and 100|010 = 110

-   BitXor   |   v2.60   |   BitXor(int, int)

Returns the bitwise exclusive OR (sets bit to 1 if exactly one of the bits is
1 and sets bit to 0 otherwise).

*Examples:*
::

    BitXor(5, 6) = 3 # since 5 = 101, 6 = 110, and 101^110 = 011
    BitXor(4, 2) = 6 # since 4 = 100, 2 = 010, and 100^010 = 110

-   BitLShift   |   v2.60   |   BitLShift(int, int)
-   BitShl   |   v2.60   |   BitShl(int, int)
-   BitSal   |   v2.60   |   BitSal(int, int)

Shift the bits of a number to the left.

*Examples:*
::

    Shifts the bits of the number 5 two bits to the left:
    BitLShift(5, 2) = 20 (since 101 << 2 = 10100)

-   BitRShiftL   |   v2.60   |   BitRShiftL(int, int)
-   BitRShiftU   |   v2.60   |   BitRShiftU(int, int)
-   BitShr   |   v2.60   |   BitShr(int, int)

Shift the bits of an unsigned integer to the right. (Logical, zero fill,
Right Shift)

*Examples:*
::

    Shifts the bits of the number -42 one bit to the right, treating it as unsigned:
    BitRShiftL(-42, 1) = 2147483627 (since
    11111111111111111111111111010110 >>> 1 =
    01111111111111111111111111101011) Note: -42 = -(1+2^0+2^3+2^5) =
    (unsigned) 2^32-(1+2^0+2^3+2^5) = (2^32-1)-2^0-2^3-2^5 =
    11111111111111111111111111010110.

-   BitRShiftA   |   v2.60   |   BitRShiftA(int, int)
-   BitRShiftS   |   v2.60   |   BitRShiftS(int, int)
-   BitSar   |   v2.60   |   BitSar(int, int)

Shift the bits of an integer to the right. (Arithmetic, Sign bit fill, Right
Shift)

*Examples:*
::

    Shifts the bits of the number -42 one bit to the right, treating it as signed:
    BitRShiftA(-42, 1) = -21 (since 11111111111111111111111111010110 >> 1
    = 11111111111111111111111111101011)

-   BitLRotate   |   v2.60   |   BitLRotate(int, int)
-   BitRol   |   v2.60   |   BitRol(int, int)

Rotates the bits of an unsigned integer to the left.

*Examples:*
::

    Rotates the bits of the number -2147483642 one bit to the right:
    BitLRotate(-2147483642, 1) = 13 (since
    10000000000000000000000000000110 ROL 1 =
    00000000000000000000000000001101)

-   BitRRotate   |   v2.60   |   BitRRotate(int, int)
-   BitRor   |   v2.60   |   BitRor(int, int)

Rotate the bits of an integer to the right.

*Examples:*
::

    Rotates the bits of the number 13 one bit to the right:
    BitRRotate(13, 1) = -2147483642 (since
    00000000000000000000000000001101 ROR 1 =
    10000000000000000000000000000110)

-   BitChange   |   v2.60   |   BitChange(int, int)
-   BitChg   |   v2.60   |   BitChg(int, int)

Invert the state of the n'th bit of an integer. The 1 bit is bit 0. The sign
bit is bit 31.

*Examples:*
::

    Change the state of the fourth bit:
    BitChange(3, 4) = 19
    BitChange(19, 4) = 3

    Change the state of the sign bit:
    BitChange(-1, 31) = 2147483647

-   BitClear   |   v2.60   |   BitClear(int, int)
-   BitClr   |   v2.60   |   BitClr(int, int)

Clear the state of the n'th bit of an integer.

*Examples:*
::

    Clear the state of the fourth bit:
    BitClear(3, 4) = 3
    BitClear(19, 4) = 3

    Clear the state of the sign bit:
    BitClear(-1, 31) = 2147483647

-   BitSet   |   v2.60   |   BitSet(int, int)

Set the state of the n'th bit of an integer.

*Examples:*
::

    Set the state of the fourth bit:
    BitSet(3, 4) = 19
    BitSet(19, 4) = 19

    Set the state of the sign bit:
    BitSet(-1, 31) = -1
    BitSet(2147483647, 31) = -1

-   BitTest   |   v2.60   |   BitTest(int, int)
-   BitTst   |   v2.60   |   BitTst(int, int)

Test the state of the n'th bit of an integer. Result is a bool.

*Examples:*
::

    Check the state of the fourth bit:
    BitTest(3, 4) = False
    BitTest(19, 4) = True

    Check the state of the sign bit:
    BitTest(-1, 31) = True
    BitTest(2147483647, 31) = False

--------

Back to :doc:`Internal functions <syntax_internal_functions>`.

$Date: 2013/01/06 13:38:34 $

.. _the wikipedia article on Atan2: http://en.wikipedia.org/wiki/Atan2
.. _wikipedia: http://en.wikipedia.org/wiki/Hyperbolic_function
.. _tau_(2?): http://en.wikipedia.org/wiki/Tau_(2%CF%80)
.. _floating-point: http://en.wikipedia.org/wiki/Floating_point
.. _the wikipedia article for Continued fraction: http://en.wikipedia.org/wiki/Continued_fraction
