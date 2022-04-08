
ColorSpaces
===========


RGB and YUV colorspaces
-----------------------

RGB and YUV are two different ways to represent colors. In YUV colorspace
there is one component that represent lightness (luma) and two other
components that represent color (chroma). As long as the luma is conveyed
with full detail, detail in the chroma components can be reduced by
subsampling (filtering, or averaging) which can be done in several ways (thus
there are multiple formats for storing a picture in YUV colorspace). In RGB
colorspace there are three components, one for the amount of Red, one for the
amount of Green and one for the amount of Blue. Also in this colorspace there
are multiple formats for storing a picture which differ in the amount of
samples are used for one of the three colors.


RGB Colorspaces
~~~~~~~~~~~~~~~

There are several RGB formats, but the two most widely used are known as
RGB24 and RGB32.

Both of them contain three components, R,G and B - short for Red, Green and
Blue.

RGB24 and RGB32 are both an :doc:`InterleavedImageFormat <InterleavedImageFormat>`. The only difference
between the two are that RGB32 contains an extra byte for each pixel.

The extra byte RGB32 contains is known as an alpha pixel, but it is actually
quite seldom used. The reason RGB32 is considered the "best" format is purely
from a programmers view. Handling RGB32 material is much easier than RGB24
material, because most processors operate on 32bits at the time, and not 24.


YUV colorspace
--------------

There are several YUV formats. There are :doc:`interleaved formats <InterleavedImageFormat>`
and :doc:`planar formats <PlanarImageFormat>` (also called packed formats). The main
difference is how they are stored in memory.

Interleaved images have all color components needed to represent a pixel
placed at the same place in memory. For planar formats the data is not
interleaved, but stored separately for each color channel (also called color
plane). For filter writers this means that they can write one simple function
that is called three times, one for each color channel, assuming that the
operations are channel-independent (which is not always the case). Again,
using aligned for both color and luma channels will allow easy memory access.
The use of a planar format gives in most cases a significant speedup, since
all bytes of each plane can be treated the same way. It can also give a
speedup because your filter doesn't have to work on all planes, if it only
modifies one or two of them.


YUY2 Colorformat
~~~~~~~~~~~~~~~~

YUY2 is an :doc:`Interleaved Image Format<InterleavedImageFormat>`. Bytes are arranged in memory like this:

``YUYV|YUYV|YUYV|YUYV|YUYV|YUYV|...``

``^first byte in a row.``

So each pixel horizontally shares UV (chroma) information with a neighboring
pixel.


YV12 Colorformat
~~~~~~~~~~~~~~~~

The YV12 colorformat is very different from RGB and YUY2 colorformats. The
main difference is that it is a :doc:`PlanarImageFormat <PlanarImageFormat>`. That means that the
different color components are lying separate in memory.

This can in most cases give a significant speedup, since all bytes of each
plane can be treated the same way. It can also give a speedup because your
filter doesn't have to work on all planes, if it only modifies one or two of
them.

All images are given a pitch. The pitch is basically what can be described as
"length of a line". What's funny is that the pitch does not have to be equal
to the width of the image.

For instance, if you crop something off your image, the only thing that
changes is the width of your image; the pitch and the actual byte-count of a
line remains the same.

The image is then laid out like this:

``rrrrrrrrrrrrrrrrpppp``

``rrrrrrrrrrrrrrrrpppp``

``rrrrrrrrrrrrrrrrpppp``


Where 'r' are the pixels inside the image used, and 'p' is the padding
inserted after each line.

In YV12 the Y-plane always has a byte-count that is a multiple of 16, but it
is still possible to have mod2 width images, because the rowsize is different
from the pitch. Similar the UV-planes always have a pitch that is a multiple
of 8.

When you request ``frame->GetRowSize(PLANAR_Y)`` you will get the rowsize of
the pixels _inside_ the image. When you request
``frame->GetRowSize(PLANAR_Y_ALIGNED)`` you will ALWAYS get a rowsize that is
a multiple of 16 (multiple of 8 on UV planes).

The reason for you to use pitch to get from one line to another is that pitch
can be even larger than the aligned rowsize - for instance after a crop.

So what you should do is:

- Get a rowsize.

- For each line in the image:

- Process (aligned) rowsize pixels.

- Skip to next line, by adding pitch for the current plane.


YV12 interlacing
::::::::::::::::

YV12 handles interlaced chroma differently than compared to YUY2, since YV12
only contains chroma information for every second line.

To enable interlacing, chroma is stretched across two luma lines in the same
field! That means that luma and chroma aren't directly mappable to lumaline/2
and lumaline/2+1 as with frame based images.

line 0: Chroma for interlaced luma lines 0+2

line 1: Chroma for interlaced luma lines 1+3

line 2: Chroma for interlaced luma lines 4+6

line 3: Chroma for interlaced luma lines 5+7

...etc!

When viewing the individual lines in each plane this maps to:

Consider the separate planes:

Luma:

``L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1L1``

``L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2L2``

``L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3L3``

``L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4L4``

Chroma that maps to luma plane above:

``C1C1C1C1C1C1C1C1C1C1C1``

``C2C2C2C2C2C2C2C2C2C2C2``

Since luma L1 and L3 are in the same field, the chroma information in C1 is
used for these lines and NOT line L1 + L2 as when the image is frame-based.
C2 is used for lines 2+4 and so on.

This is something that deinterlacers and similar programs need to take into
consideration. Other filters might rely on the use of Separatefields() and
Weave(), to produce framebased images. You can use the
VideoInfo.IsFieldBased() to check your source, and maybe decide to throw an
error, or shift to another processing mode.

If your video is fieldbased your vertical resolution (height) must be
divisible by 4, otherwise AviSynth will not create a new VideoFrame, but will
throw an error.


Links
~~~~~

See more about :doc:`DataStorageInAviSynth <DataStorageInAviSynth>`.

See a general introduction to :doc:`WorkingWithImages <WorkingWithImages>`.

----

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $
