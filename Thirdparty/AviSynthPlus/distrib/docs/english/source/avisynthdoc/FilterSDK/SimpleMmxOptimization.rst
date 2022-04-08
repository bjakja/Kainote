
SimpleMmxOptimization
=====================

This will cover some basics in assembler optimization, and the syntax to use
in Visual Studio C++ for inline assembler.

This will cover a basic routine in C++, and how it is converted to MMX. The
example is from an actual filter in AviSynth, with only minor changes.

Consider the following C-routing:
::

  void Limiter::c_limiter(BYTE* p, int row_size, int height, int modulo, int cmin, int cmax) {

      for(y = 0; y < height; y++) {
          for(int x = 0; x < row_size; x++) {
              if(p[x] < cmin)
                  p[x] = cmin;
              else if(p[x] > cmax)
                  p[x] = cmax;
          }

          p += row_size+modulo;

          }
      }

The parameters of this routine is:

-   *p* - a pointer to the information being processed.
-   *row_size* - the width of a row in bytes.
-   *height* - the height of the image in bytes.
-   *modulo* - the difference between the pitch and the rowsize (pitch -
    rowsize).
-   *cmin, cmax* - the maximum and minimum values of a pixel.

When converting to :doc:`MMX <MMX>` and :doc:`IntegerSSE <IntegerSSE>` it is a good thing to look at
which commands are available for the task needed. In this case we choose to
focus on :doc:`IntegerSSE <IntegerSSE>`, because it contains pminub and pmaxub, which selects
the minimum and maximum bytes of two packed registers. It is always a good
idea to support plain MMX, since there are still many machines out there,
that only support these instructions.

An important aspect of MMX is parallel processing. That means processing
several bytes at once. The MMX instructions all work on 8 bytes at the time,
but in many cases, you have to unpack these bytes to words (8 to 16 bits) to
be able to do things like additions, etc.)

The equivalent of the routine above in :doc:`IntegerSSE <IntegerSSE>` looks like this:
::

  void Limiter::isse_limiter_mod8(BYTE* p, int row_size, int height, int modulo, int cmin, int cmax) {

      cmax|=(cmax<<8);

      cmin|=(cmin<<8);

      __asm {
          mov eax, [height]
          mov ebx, p
          mov ecx, [modulo]
          movd mm7,[cmax]
          movd mm6,[cmin]
          pshufw mm7,mm7,0
          pshufw mm6,mm6,0

          yloop:
          mov edx,[row_size]

          align 16
          xloop:
          movq mm0,[ebx]
          pminub mm0,mm7
          pmaxub mm0,mm6
          movq [ebx],mm0
          add ebx,8
          sub edx,8
          jnz xloop
          add ebx,ecx;
          dec eax
          jnz yloop
          emms
      }
  }


This routine performs the same task as the routine above. The filter requires
mod8 rowsize, because it processes 8 pixels in parallel.

Let's go through the code, line by line.

``cmax|=(cmax<<8);``

``cmin|=(cmin<<8);``

This is code is plain C,
and can be seen as preparation for the assembler. Writing the same code is of
course possible in assembler, but there is no speed gain at all, since this
is only used once.

``__asm {``

This shows MSVC, that an assembler block
is coming. When you enter an assembler block you have to assume that all your
registers contain garbage. There are 6 general purpose 32-bit registers, that
can be used freely within the assembler block, these are *eax, ebx, ecx, edx,
esi* and *edi*.

``mov eax, [height]``

``mov ebx, p``

``mov ecx, [modulo]``

In these lines we put data into the registers. eax contains the
height, ebx contains a pointer to the plane we are processing, ecx contains
the modulo information. Note that the destination is always written FIRST. So
the first line translates to "move height into eax".

``movd mm7,[cmax]``

``movd mm6,[cmin]``

These two lines move data into the mmx registers - there are 8 mmx registers, named from mm0 to mm7.

mm7 now contains "0x0000|0000|0000|cmcm" (| on inserted for readability).
Remember we duplicated the max and min values in the C-part.

``pshufw mm7,mm7,0``

``pshufw mm6,mm6,0``

These commands are
:doc:`IntegerSSE <IntegerSSE>` commands. They can shuffle around words in the mmx registers,
based on the last number.

In this example it results in mm7 containing "0xcmcm|cmcm|cmcm|cmcm". So
basically cmax and cmin are now placed in all 8 bytes in the mm6 and mm7
registers.

``yloop:``

This is a jump destination for a jump routine.

``mov edx,[row_size]``

Every time we are looping on y, the rowsize is moved into edx.

``align 16``

``xloop:``

The "align 16" is to be used before any loop destination, that will be
frequently used. It inserts commands that doesn't do anything, and ensures
that the xloop destination will be aligned on a 16 byte boundary.

``movq mm0,[ebx]``

This command moves 8 bytes from the memory location in ebx into the mm0 register.

mm0 now contains 0xp8p7|p6p5|p4p3|p2p1, where p1 is the leftmost pixel
onscreen. This may look a bit backwards at first, but you'll get used to it.

``pminub mm0,mm7``

``pmaxub mm0,mm6``

These commands compares each byte, and take the minimum and maximum and place it in mm0. (Remember -
result is always placed in the first register).

``movq [ebx],mm0``

This will put back the new values into the memory location that ebx points to.

``add ebx,8``

This command will add 8 to the pointer in ebx - making it
move on to the next 8 pixels. Remember: This is only the pointer we are
incrementing - we are not actually reading it, so even if this values gets
out of bounds nothing will happend here.

``sub edx,8``

``jnz xloop``

Here we subtract 8 from edx, and jump to the xloop location if the
values in edx isn't 0 (jump if not zero).

``add ebx,ecx;``

``dec eax``

``jnz yloop``

This is the code that gets executed whenever the xloop
is finished. Here we add modulo to the ebx-pointer, decrements height and
jumps to the yloop as long as there are still pixels left to process.

``emms``

This instruction must be placed after all mmx code. It reenabled
float point code, which is disabled by mmx code.

----

Back to :doc:`AssemblerOptimizing <AssemblerOptimizing>`

$Date: 2014/10/27 22:04:54 $
