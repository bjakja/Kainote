
Blur / Sharpen
==============

| ``Blur`` (clip, float amount)
| ``Blur`` (clip, float amountH, float amountV, bool MMX)
| ``Sharpen`` (clip, float amount)
| ``Sharpen`` (clip, float amountH, float amountV, bool MMX)

This is a simple 3x3-kernel blurring filter. The largest allowable argument
for ``Blur`` is about 1.58, which corresponds to a (1/3,1/3,1/3) kernel. A
value of 1.0 gets you a (1/4,1/2,1/4) kernel. If you want a large-radius
Gaussian blur, I recommend chaining several copies of ``Blur(1.0)``
together. (Anybody remember Pascal's triangle?)

Negative arguments to ``Blur`` actually sharpen the image, and in fact
``Sharpen(n)`` is just an alias for ``Blur(-n)``. The smallest allowable
argument to ``Blur`` is -1.0 and the largest to ``Sharpen`` is 1.0.

You can use 2 arguments to set independent Vertical and Horizontal amounts.
Like this, you can use ``Blur(0,1)`` to filter only Vertically, for example
to blend interlaced lines together. By default *amountV* = *amountH*.


A Known issue, with the MMX routines is the lack of full 8 bit precision in
the calculations. This can lead to banding in the resultant image. Set the
MMX=False option to use the slower but more accurate C++ routines if this is
a concern.

$Date: 2006/12/03 11:37:04 $
