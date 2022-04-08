
General Purpose To MMX Registers
================================


Overview
--------

It might be necessary to move data back and forth between a general purpose
register (EAX for instance) to an MMX register. This can be done by:

``movd eax,mm0``

``movd mm0,eax``

``pinsrw mm0,eax,0``

``pextrw eax,mm0,0``

``movmskb eax,mm0``

These instructions should however **be avoided as much as possible** since
they are executing very slow. It seems like there are severe penalties when
moving data between the MMX registers and the general purpose registers. Most
of these instructions take more than 16 cycles to execute, which is more than
8 times the cyclecount of normal MMX instructions.

In many cases it is actually faster to write the data to a temporary memory
location, using movd, and move the data back into the registry. For much data
it is actually often faster to process the data one line at the time, and
store all data in a temporary space, and process the data in a separate loop.

----

*phaeron writes:*

Note that the Microsoft Visual C++ compiler will give you trouble here if you
are using MMX intrinsics, as it is unable to directly generate ``movd
mmreg,mem`` and ``movd mem,mmreg`` instructions for the ``_m_to_int()`` and
``_m_from_int()`` intrinsics. Instead, the values trampoline off of the
general purpose registers, triggering the undesirable performance behavior
described above. This is true even with the Visual Studio .NET 2003 compiler
(VC7.1), which fixed most of the MMX code generation embarrassments from the
VC6PP and VC7 compilers, most notably MOVQ hell. As such, you're still better
off sticking with inline or explicit assembly than using MMX intrinsics with
MSVC.

----

Back to :doc:`AssemblerOptimizing <AssemblerOptimizing>`

$Date: 2006/11/08 20:40:17 $
