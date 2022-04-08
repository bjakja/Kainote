
IntegerSSE
==========

Integer SSE is a set of instructions found in most modern processors.
IntegerSSE is an extension of :doc:`MMX <MMX>`.

SSE: New commands present in P3, P4, newer Celerons, Athlon XP, MP.

Integer SSE: A *subset* of the SSE command set, mostly used for video
processing. These instructions are also present in AMD Athlon (all versions),
AMD Duron (all versions).

Integer SSE instructions:

``MASKMOVQ`` *mmreg1, mmreg2*

``MOVNTQ`` *mem64, mmreg*

``PAVGB`` *mmreg1, mmreg2*

``PAVGB`` *mmreg, mem64*

``PAVGW`` *mmreg1, mmreg2*

``PAVGW`` *mmreg, mem64*

``PEXTRW`` *reg32, mmreg, imm8*

``PINSRW`` *mmreg, reg32, imm8*

``PINSRW`` *mmreg, mem16, imm8*

``PMAXSW`` *mmreg1, mmreg2*

``PMAXSW`` *mmreg, mem64*

``PMAXUB`` *mmreg1, mmreg2*

``PMAXUB`` *mmreg, mem64*

``PMINSW`` *mmreg1, mmreg2*

``PMINSW`` *mmreg, mem64*

``PMINUB`` *mmreg1, mmreg2*

``PMINUB`` *mmreg, mem64*

``PMOVMSKB`` *reg32, mmreg*

``PMULHUW`` *mmreg1, mmreg2*

``PMULHUW`` *mmreg, mem64*

``PSADBW`` *mmreg1, mmreg2*

``PSADBW`` *mmreg, mem64*

``PSHUFW`` *mmreg1, mmreg2, imm8*

``PSHUFW`` *mmreg, mem64, imm8*

``PREFETCHNTA`` *mem8*

``PREFETCHT0`` *mem8*

``PREFETCHT1`` *mem8*

``PREFETCHT2`` *mem8*

``SFENCE``

----

Back to :doc:`AssemblerOptimizing <AssemblerOptimizing>`

$Date: 2014/10/27 22:04:54 $
