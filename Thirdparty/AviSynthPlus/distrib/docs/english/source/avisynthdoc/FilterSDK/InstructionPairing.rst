
`InstructionPairing`_
=====================

Instruction pairing can almost double the performance of you filter. There
are two execution pipes for :doc:`MMX <MMX>` code, which means that two instructions
can be executed at the same time. Certain instructions can only be issued in
the U pipeline, and certain instructions can only be issued in the V
pipeline. Most MMX instructions can be issued in either

The original pairing rules for MMX code is as follows:

1)  The second instruction does not read or write a register which the
    first instruction writes to.  2) MMX shift, pack or unpack instructions
    can execute in either pipe but cannot pair with other MMX shift, pack or
    unpack instructions.  3) MMX multiply instructions can execute in either
    pipe but cannot pair with other MMX multiply instructions. They take 3
    clock cycles and the last 2 clock cycles can overlap with subsequent
    instructions.  4) an MMX instruction which accesses memory or integer
    registers can execute only in the U-pipe and cannot pair with a non-MMX
    instruction.

**Ad. 1)**

This is the most important rule of them all. Instruction two should never be
dependant on information from instruction one. The formal definition is "The
destination of the first operand should not be used as source or destination
of the second operand".

Consider the following code (paired instructions are in bold):

``movq mm0,[eax]``

``movq mm1,mm0 *; depends on mm0*``

``pavgb mm1,mm7 *; depends on mm1*``

``por mm0,mm1 *; depends on mm1*``

``movq [eax],mm0 *; depends on mm0*``

``**movq mm0,[eax+ebx]**``

``movq mm1,mm0 *; depends on mm0*``

``punpcklwd mm1,mm7 *; depends on mm1*``

``por mm0,mm1 *; depends on mm1*``

``movq [eax+ebx],mm0 *; depends on mm0*``

Rearrange the code to:

``movq mm0,[eax]``

``**movq mm2,[eax+ebx]**``

``movq mm1,mm0``

``**movq mm3,mm0**``

``pavgb mm1,mm7``

``**pavgb mm3,mm7**``

``por mm0,mm1``

``**por mm2,mm3**``

``movq [eax],mm0``

``**movq [eax+ebx],mm2**``

The latter code executes twice as fast as the upper code!

**Ad. 2)**

It seems like this restriction has been loosened on Athlon and P3. Apparently
pack does no longer use the shifter unit. So now only shifts are not
pairable!

**Ad. 3)**

Even though these numbers have changed slighly on Athlon, it still seems like
a good idea to put in two MMX instructions before depending on the result of
a multiply. On AMD Athlon a multiply takes 3 cycles, whereas most other
instructions take 2. See appendix F in the :doc:`AMDOptimizationGuide <AMDOptimizationGuide>`.

--------

*phaeron writes:*

This information is very old -- it applies to the superscalar pipelines of
the original Pentium. Beginning with the Pentium II instruction decode and
execution are decoupled, so the dependency restrictions outlined above likely
don't apply to any modern CPU which does out-of-order (OOO) execution. Also,
the Pentium II, Pentium III, and Pentium-M have a 4-1-1 uop decoder pattern,
not the UV pairing described above. So while this sequence will ideally
decode on a Pentium MMX:

``{movd mm0,[eax]}``

``{pxor mm7,mm7}``

``{movd mm1,[eax+esi]}``

``{punpcklbw mm1,mm7}``

``{movd mm2,[eax+esi]}``

``{punpcklbw mm2,mm7}``

PPro architectures will instead decode the above at **3** instructions/cycle.

I don't think packs are any better on PIII than on Pentium/PII. `[Agner Fog's
optimization tome]`_ says that shifts still only issue in port 1, meaning
that you still can't execute more than one per cycle. This seems to be borne
out in tests as well, as I can get a PADDW loop to execute MMX ops twice as
fast as a PUNPCKLBW loop on a PIII.

Perhaps more dangerous than decoding rules on PII/PIII is the register
renaming limitation:

--------


Intel P4 specifics
~~~~~~~~~~~~~~~~~~

According to the :doc:`IntelOptimizationGuide <IntelOptimizationGuide>`:

**MMX multiply:**

P4 - 8 cycles, K7 - 3 cycles.

MMX multiply now takes 4 times as many cycles as other MMX commands -
actually quite a big problem when having to pair them. Therefore there should
ideally now be at least 4 instructions that doesn't mulitply between two
multiply instructions.

**MMX shifter restrictions:**

It seems like there are still issues with one shifter on the P4 - this is
(alongside the MMX multiplier) one of the biggest problems. I can see that
pshufw is actually done in the MMX shifter - in contrast to the K7, where
this restriction has been lifted.

This means that on P4, none of these instructions are pairable: Pack, unpack,
shift, shuffle. This is a problem, since it will stall the processor, if any
of these two instructions follow eachother - even if they are not depending
on eachother. This is however too often the case.

In case I'm being too crypitic above: None of the instructions marked with
MMX_SHFT can be paired with eachother, just as two instructions with FP_MUL
cannot be paired with eachother. Therefore to have efficient code, the second
pipe should have code that doesn't use the same part of the CPU - and doesn't
depend on the result from the other pipe. The ISSE code above is a perfect
example of this - almost none of the instructions pair on P4.

--------

*phaeron writes:*

I wouldn't worry about having only one shifter on P4. Intel balanced this out
by removing the second MMX ALU, so you can only do one PADDW per cycle now
too.

A much bigger worry with respect to P4 MMX performance is that register-to-
register MOVQs aren't handled entirely by register renaming anymore, and have
a 6 clock latency! It can be *much* faster to move via the ALU (pxor mm1,mm1
/ por mm1, mm0) or the shifter (pshufw mm1, mm0, 0) than via MOVQ. For a sad
joke, benchmark this in a loop:

``movq mm1, mm0``

``movq mm2, mm1``

``movq mm3, mm2``

``movq mm4, mm3``

``movq mm5, mm4``

``movq mm6, mm5``

``movq mm7, mm6``

``movq mm0, mm7``

against this on a P4:

``pxor mm1, mm1``

``por mm1, mm0``

``pxor mm2, mm2``

``por mm2, mm1``

``pxor mm3, mm3``

``por mm3, mm2``

``pxor mm4, mm4``

``por mm4, mm3``

``pxor mm5, mm5``

``por mm5, mm4``

``pxor mm6, mm6``

``por mm6, mm5``

``pxor mm7, mm7``

``por mm7, mm6``

``pxor mm0, mm0``

``por mm0, mm7``

and watch the second loop run **three times faster** than the first. The
reason is that both loops end up being one huge dependency chain, for which
the first takes 8*6 = 48 cycles per iteration, and the second takes 8*2 = 16
cycles per iteration.

(Be careful with the pxor trick. It breaks dependency chains on P4, but has a
false dependency on the source register on PIII and Athlon.)


Back to :doc:`AssemblerOptimizing <AssemblerOptimizing>`

$Date: 2006/11/24 18:21:26 $

.. _InstructionPairing: http://www.avisynth.org/InstructionPairing
.. _[Agner Fog's optimization tome]: http://www.agner.org/assem/
