
`IsMovntqFaster`_
=================

*sh0dan writes:*

It seems movntq is only suited for direct memory copies - otherwise the
penalty is simply too big.

The movntq was a big surprise for me too. It is definately faster in direct
copying (BitBlt) for instance, but not very useful in routines that actually
do some processing.

The data is not read again, and it is 8-byte aligned. But I guess the problem
lies in the fast, that movntq cannot be defered. A movq to memory can be
stored in the data cache for later storage, whereas movntq must be dispatched
directly. When doing some processing the processor can take time to do the
write.

I saw some really big penalties in the AMD Pipeline analysis tool (most
movntq's took an average of 60 cycles!) It showed penalties for Data Cache
miss, and the Load/Store Queue being full. Using movq's, the average
cycle/instruction was about 2-4, with the load-store queue doing ok.

It should be noted that the system I tested on were Athlon Tbird 1200, and an
Athlon XP 2200+ - both with DDR RAM.

**The conclusion can only be to benchmark your code, to be sure if you are
actually gaining something from using movntq.**

--------

*phaeron writes:*

MOVNTQ can be a win even in non-trivial routines if used carefully --
VirtualDub's YCbCr-to-RGB conversions use it for a significant gain on large
MPEG-1 files (around 640x360). However, it is definitely not to be used
lightly.

The purpose of MOVNTQ is to defeat a cache optimization known as write-back.
The early caches introduced into x86 systems were initially write-through,
meaning that writes to memory not present in the cache went directly to main
memory. This led to some interesting behavior on Pentium systems where doing
dummy reads periodically during heavy write loops would boost throughput,
because the reads would pull memory into cache, the writes would then be
combined in the cache, and a lot more burst transfers would occur over the
bus.

Beginning with the Pentium Pro, Intel started using write-back as the L1
cache strategy. Whenever a write occurs to an uncached location, the CPU
pulls an entire cache line of data from main memory, pushes the writes into
the L1 cache, and then later writes the line back to main memory. This is
typically a win as burst transfers to and from the cache are generally faster
than piecemeal 32-bit accesses. Where write-back hurts is when code is only
writing to memory -- in this case, the initial read is a waste as all of the
cache line is going to be overwritten anyway.

The MOVNTQ instruction tells the CPU to use write combining (WC) instead of
write-back (WB) as the mode for memory. Writes are then collected in write
combining buffers, which are bursted out to memory once full. This can be
much, much faster than traditional methods  large memcpy() operations in
particular can be more than twice as fast  but there are a few caveats, as
you noticed. The biggest is that MOVNTQ is only efficient when data must be
bounced back to main memory anyway; it is a loss when data can stay within
the caches, as otherwise without MOVNTQ the data would stay within the much
faster L1 and L2 caches. It is never a good idea to use MOVNTQ when you know
the target is in the cache. Another is that you **must** write all bytes in a
cache line. Piecemeal writes with MOVNTQ result in partial bus transactions,
which means that instead of a nice long burst, you get a bunch of small
writes. In a WB scenario the CPU already has all the data in the cache line
and can rewrite the unwritten parts with the same data, but it can't do this
when streaming writes are involved. Processing vertical columns of single
pixels at a time with MOVNTQ involved, for example, would be a bad idea.

Profiling MOVNTQ can be tricky. A memcpy() rewritten to use block prefetching
and streaming stores can execute more than twice as fast as REP MOVSD. What
the profile won't tell you is that by converting your memcpy() to streaming,
you slowed down the routine right after it that now suffers lots of cache
misses pulling the data back into memory. MOVNTQ is thus most useful when you
are already suffering cache misses because your buffers are way too big, sit
too long between the write and the next read, or need to bounce between CPUs.
Sometimes an even better idea than streaming stores is to block the various
pipeline stages into strips, thus keeping each strip in cache -- Intel calls
this "strip mining." However, it is very difficult to do when many different
modules are involved, such as in an Avisynth filter graph.

--------


Back to :doc:`AssemblerOptimizing <AssemblerOptimizing>`

$Date: 2006/11/24 18:21:26 $

.. _IsMovntqFaster: http://www.avisynth.org/IsMovntqFaster
