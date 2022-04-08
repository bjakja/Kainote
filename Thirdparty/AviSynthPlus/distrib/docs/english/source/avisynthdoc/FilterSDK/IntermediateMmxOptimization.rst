
`IntermediateMmxOptimization`_
==============================

*phaeron writes*:

Additive blending is a relatively simple, common, and useful operation. It
takes two images and adds them together, saturating the result to white. It's
useful for some kinds of overlays, as well as incrementally building a final
image from components. We'll start with a simple scalar version:

::

    void additiveblend1(unsigned char *dst, const unsigned char *src, int quads) {
        int bytes = quads << 2;

        do {
            unsigned v = (unsigned)src[0] + dst[0];
            if (v >= 256)
                v = 255;
            dst[0] = v;
            ++src;
            ++dst;
        } while(--bytes);
    }


This code works a byte at a time and works equally well for any byte-per-
channel format, including RGB24, RGB32, YUY2, and YV12. It runs at a dismal
16 clocks/byte on a 1.6GHz Pentium 4.

**Tip:** The CPU's time stamp counter, accessible via the RDTSC instruction,
is a great way to benchmark small pieces of code. However, the results can be
heavily affected by cache effects. For code that is deterministic and quick,
running the test twice in a row and taking the second timing can eliminate
most cache effects.

Now, let's rewrite it in MMX.

::

    void additiveblend2(unsigned char *dst, const unsigned char *src, int quads) {
        __asm {
            mov     ecx, src
            mov     edx, dst
            mov     eax, quads
            pxor        mm7, mm7
    top:
            movd        mm0, [ecx]     ;load four source bytes
            movd        mm1, [edx]     ;load four destination
            bytes
            punpcklbw   mm0, mm7       ;unpack source bytes to
            words
            punpcklbw   mm1, mm7       ;unpack destination bytes
            to words
            paddw       mm0, mm1       ;add words together
            packuswb    mm0, mm1       ;pack words with
            saturation
            movd        [edx], mm0     ;store blended result
            add     ecx, 4             ;advance source pointer
            add     edx, 4             ;advance destination
            pointer
            dec     eax                ;loop back until done
            jne     top
            emms
        }
    }


This version runs *much* better on a random 1K sample, at 1.64 clocks/byte.
However, we can do better. Unpacking to 16-bit words is generally required in
most MMX routines but it's a total waste here, as the CPU has saturating add
instructions that work directly on bytes. We can use those instead.

**Tip:** MMX has a lot of interesting instructions, such as saturating adds
and subtracts, clamping packs, and multiply-plus-shift instructions. Learning
to use these effectively can improve the performance of your code. However,
there are a few omissions in the basic MMX set, including unsigned
multiplies, byte shifts, and saturating 32-bit operations, that you should be
aware of.

So here's the version using the saturating PADDUSB (packed add with unsigned
byte saturation) instruction:

::

    void additiveblend3(unsigned char *dst, const unsigned char *src, int quads) {
        __asm {
            mov     ecx, src
            mov     edx, dst
            mov     eax, quads
    top:
            movd        mm0, [ecx]
            movd        mm1, [edx]
            paddusb     mm0, mm1
            movd        [edx], mm0
            add     ecx, 4
            add     edx, 4
            dec     eax
            jne     top
            emms
        }
    }


This version is still faster at 1.27 clocks/byte and is simpler, too. But why
are we still doing 4 bytes at a time, when we could be doing 8?

::

    void additiveblend4(unsigned char *dst, const unsigned char *src, int quads) {
        __asm {
            mov     ecx, src
            mov     edx, dst
            mov     eax, quads
            pxor        mm7, mm7
    top:
            movq        mm0, [ecx]
            paddusb     mm0, [edx]
            movq        [edx], mm0
            add     ecx, 8
            add     edx, 8
            sub     eax, 2
            jne     top
            emms
        }
    }


Although we only doubled the parallism of the loop, this change makes a huge
difference and kicks us all the way up to 0.49 clocks/pixel. Now notice that
the scalar code is beginning to take a lot of the inner loop -- in fact, half
of it. We can eliminate the pointer updates by switching to array notation:

::

    void additiveblend5(unsigned char *dst, const unsigned char *src, int quads) {
        __asm {
            mov     ecx, src
            mov     edx, dst
            mov     eax, quads
            shl     eax, 2
            add     ecx, eax
            add     edx, eax
            neg     eax
    top:
            movq        mm0, [ecx+eax]
            paddusb     mm0, [edx+eax]
            movq        [edx+eax], mm0
            add     eax, 8
            jne     top
            emms
        }
    }


There is a slight trick here, in that we index negatively from the end of the
arrays rather than positively from the start, and count up instead of down.
Removing the pointer operations trims execution time by a tiny amount, down
to 0.48 clocks/pixel. We're still not done yet, though. Let's unroll the
loop. That way, the CPU has less loop overhead instructions to execute, and
also relaxes the loop-carried dependency on the loop count (EAX in a loop
iteration is dependent on the last loop's EAX, which is dependent on EAX from
the loop before that, etc).

::

    void additiveblend6(unsigned char *dst, const unsigned char *src, int quads) {
        __asm {
            mov     ecx, src
            mov     edx, dst
            mov     eax, quads
            shl     eax, 2
            add     ecx, eax
            add     edx, eax
            neg     eax
    top:
            movq        mm0, [ecx+eax]
            movq        mm1, [ecx+eax+8]
            paddusb     mm0, [edx+eax]
            paddusb     mm1, [edx+eax+8]
            movq        [edx+eax], mm0
            movq        [edx+eax+8], mm1
            add     eax, 16
            jne     top
            emms
        }
    }


This version now runs at 0.42 clocks/pixel, about 38x faster than the
original scalar version. That's fast enough, right? Well, yes, except for one
problem: it's wrong. Namely, this routine handles 16 bytes at a time instead
of 4 bytes at a time, so if the quad count isn't divisible by 4, this routine
doesn't work properly. Whoops.

**Tip:** Faster code is generally less desirable than working code.

::

    void additiveblend7(unsigned char *dst, const unsigned char *src, int quads) {
        __asm {
                mov             ecx, src
                mov             edx, dst
                mov             eax, quads
                shl             eax, 2
                lea             ecx, [ecx+eax-12]
                lea             edx, [edx+eax-12]
                neg             eax
                add             eax, 12
                jc              look_for_oddballs
    top:
                movq            mm0, [ecx+eax]
                movq            mm1, [ecx+eax+8]
                paddusb         mm0, [edx+eax]
                paddusb         mm1, [edx+eax+8]
                movq            [edx+eax], mm0
                movq            [edx+eax+8], mm1
                add             eax, 16
                jnc             top
    look_for_oddballs:
                sub             eax, 12
                jnc             no_oddballs
    oddball_loop:
                movd            mm0, [ecx+eax+12]
                movd            mm1, [edx+eax+12]
                paddusb         mm0, mm1
                movd            [edx+eax+12], mm0
                add             eax, 4
                jne             oddball_loop
    no_oddballs:
                emms
        }
    }


Here is the final version. It's probably still not as fast as it could be but
for most uses it's fast enough. Unfortunately, the code to handle odd fixups
at the end of the line takes more code than the optimized version. It is
often the case that handling odd cases will greatly complicate an optimized
routine. There is an trick to handle odd widths that unfortunately cannot be
used here, but is useful when the destination and sources don't overlap: back
off the source and destination pointers and redo some bytes that have already
been done. For instance, if a routine handles 8 bytes at a time and is asked
to do 21 bytes, do 16 bytes in the fast loop, then do 8 unaligned bytes at
the end with bytes 13-15 getting redone. This can let you handle odd widths
with very little code.

--------


Omake
~~~~~

Remember that scalar version at the top that ran at 16 clocks/pixel? Well,
that was a pretty bad scalar version. Try this one:

::

    void additiveblend1s(unsigned char *dst, const unsigned char *src, int quads) {
        unsigned *dst2 = (unsigned *)dst;
        const unsigned *src2 = (const unsigned *)src;

        do {
                unsigned a = src2[0];
                unsigned b = dst2[0];
                unsigned sats = ((a&b) + (((a^b)&0xfefefefe)>>1)) &
    0x80808080;
                sats = sats + sats - (sats >> 7);

                dst2[0] = (a|sats) + (b&~sats);

                ++src2;
                ++dst2;
        } while(--quads);
    }


This version handles 32 bits at a time, and runs at 3.43 clocks/byte. MMX is
still faster here, but there are some cases in which it isn't -- for
instance, when I tried writing an HSV filter, an optimized scalar version
turned out to be faster than an optimized MMX version. So don't think that
MMX is always the best way.


Back to :doc:`AssemblerOptimizing <AssemblerOptimizing>`

$Date: 2006/11/24 18:21:26 $

.. _IntermediateMmxOptimization:
    http://www.avisynth.org/IntermediateMmxOptimization
