
`DataAlignment`_
================

(Until something more AviSynth related can be written this is from the
:doc:`AMDOptimizationGuide <AMDOptimizationGuide>`)


Memory alignment
----------------

In general, avoid misaligned data references. All data whose size is a power
of two is considered aligned if it is naturally aligned.

For example:

-   Word accesses are aligned if they access an address divisible by two.
-   Doubleword accesses are aligned if they access an address divisible
    by four.
-   Quadword accesses are aligned if they access an address divisible by
    eight.
-   TBYTE accesses are aligned if they access an address divisible by
    eight.

In AviSynth each frame is most of the time aligned to an address divideable
by 16. The exception for this rule is when crop() has been used. Furthermore
each line is also most of the time aligned to an address divideable by 16 (or
8 for planar chroma).


Store to Load Forwarding
------------------------

Whenever some data is written to memory it is put in the cache. It is however
possible for the processor to access this data quite fast after it has been
written. This is called Store To Load forwarding.

Avoid memory-size mismatches when different instructions operate on the same
data. When an instruction stores and another instruction reloads the same
data, keep their operands aligned and keep the loads/stores of each operand
the same size.

The following code examples result in a store-to-loadforwarding (STLF) stall:

**Example (avoid):**

``MOV [FOO], EAX`` ``MOV [FOO+4], EDX`` ``...`` ``MOVQ MM0, [FOO]``

**Example (preferred):**

``MOV [FOO], EAX`` ``MOV [FOO+4], EDX`` ``...`` ``MOVD MM0, [FOO]``
``PUNPCKLDQ MM0, [FOO+4]``

**Example (avoid):**

``MOVQ [foo], MM0`` ``...`` ``MOV EAX, [foo]`` ``MOV EDX, [foo+4]``

**Example (preferred):**

``MOVD [foo], MM0`` ``PSWAPD MM0, MM0`` ``MOVD [foo+4], MM0`` ``PSWAPD MM0,
MM0`` ``...`` ``MOV EAX, [foo]`` ``MOV EDX, [foo+4]``

Back to :doc:`AssemblerOptimizing <AssemblerOptimizing>`

$Date: 2006/11/24 18:21:26 $

.. _DataAlignment: http://www.avisynth.org/DataAlignment
