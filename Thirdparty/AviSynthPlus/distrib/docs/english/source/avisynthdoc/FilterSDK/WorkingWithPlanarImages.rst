
`WorkingWithPlanarImages`_
==========================

Additional functions have been put in to support planar images. They are the
following:

VideoFrame:

``int GetPitch(int plane)``

``int GetRowSize(int plane)``

``int GetHeight(int plane)``

``VideoFrame* Subframe(int rel_offset, int new_pitch, int new_row_size, int new_height, int !rel_offsetU, int !rel_offsetV, int !pitchUV)``

``const BYTE* GetReadPtr(int plane)``

``BYTE* GetWritePtr(int plane)``


Where plane can be:

* ``PLANAR_Y``
* ``PLANAR_U``
* ``PLANAR_V``

PLANAR_Y is equivalent to calling the old functions.

It can be determined whether or not you are dealing with a planar images by
using vi.IsPlanar(). This will return true, if image is planar. Using
vi.RowSize()from VideoInfo only provides you with correct numbers for the
first plane. Use RowSize(int plane) from VideoFrame to obtain the proper
results. The same goes for vi.BytesFromPixels(), which also only returns
correct results for the first plane.

What can be safely assumed for planar images is:

* Pitch U = Pitch V.  * RowSize U = RowSize V  * Height U = Height V

You should consider the planes as seperate memory regions, and you shouldn't
begin guessing where "U would be relative to Y", since it might as well
change.

The general guideline should (as always) be: Use the functions - never
directly access the members of VideoInfo or VideoFrameBuffer or VideoFrame
structs directly, unless you are creating an input source. Always use the
supplied funtions! This is the only way for us to be able to maintain
compatibility with future releases. All processing filters should be able to
use the methods exclusively.

GetRowSize() is special, since it can return mod16 widths for Luma and mod8
aligned widths for chroma planes. When you request the rowsize, you can add
the parameter "PLANAR_Y_ALIGNED" or "PLANAR_U_ALIGNED"/"PLANAR_V_ALIGNED".
This will give you the width of the image, rounded up to nearest mod16 (or if
already mod16, just rowsize as usual).

This means that you can request an image larger than the actual image,
without risking to write or read into the next line. You can safely do
whatever you like with this data, since AviSynth completely ignores it. This
will enable you to operate on 8 bytes (1 quadword) at the time for all
planes.

Some (producing) filters may want to set a forced pitch. Per default, when
you request a videoframe with a specific modX pitch it might be overruled by
AviSynth. (You do this by using env->NewVideoFrame(VideoInfo, [alignment]))
In this (very rare case), you can use a negative alignment. This will make
the alignment forced.

If you force an alignment you MUST also apply the AlignPlanar(!PClip) filter
after your own, otherwise your filter will not produce valid output. See
source.cpp for examples.


See:
::::

The example of an AviSynyh :doc:`TwoFiveInvert <TwoFiveInvert>` filter.

Something about the specific :doc:`ColorSpaces <ColorSpaces>`.

More about :doc:`DataStorageInAviSynth <DataStorageInAviSynth>`.

A general introduction to :doc:`WorkingWithImages <WorkingWithImages>`.


Back to :doc:`AviSynthTwoFiveSDK <AviSynthTwoFiveSDK>`

$Date: 2006/11/08 20:40:17 $

.. _WorkingWithPlanarImages:
    http://www.avisynth.org/WorkingWithPlanarImages
