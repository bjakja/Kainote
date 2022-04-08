
DataStorageInAviSynth
=====================


Part 1: Actual memory storage
-----------------------------

**Warning:** *technical.* Most users will only need to read (and understand)
Part 2.

In AviSynth, we process pixels of video data. These are stored in the memory
in the form of data arrays. As noted elsewhere (like the :doc:`AviSynth FAQ <../faq/faq_sections>`)
there are two different colorspaces in v1.0x/2.0x, RGB and YUY2, with a third
YV12 added in v2.5. These colorspaces store their data in different ways.


Interleaved formats
~~~~~~~~~~~~~~~~~~~

**RGB** (also known as RGB24): Every pixel is associated with three bytes of
data: one red, one green, and one blue. They are interleaved (mixed) in the
memory like this ``RGBRGBRGBRGB``. Since data is most easily read in words of
four bytes, most functions actually process 4 pixels at the same time in
order to be able to read 3 words in a row. Usually memory is allocated as
``aligned`` which means that it always allocate memory in words of 4 bytes.
Since the 3 to 4 relation isn't easy to map, RGB24 is very difficult to work
with, and in general isn't recommended.

**RGBA** (also known as RGB32): This is an extension of RGB where a fourth
color channel has been added. This is called the alpha channel, and is a
definition of transparency in the pixel. For an explanation of how the alpha
channel is used see :doc:`Mask and Layer <../corefilters/layer>`. In general however, you shouldn't rely
on filters processing alpha correctly - in some cases filters will even
produce garbage in this channel.

RGBA also makes this a lot easier as it requires four bytes per pixel, and
thus each memory access of a word, will correspond to exactly one pixel. In
fact, RGB will only be used if you have a source that returns RGB or if you
explicitly use (ConvertToRGB24). :doc:`ConvertToRGB <../corefilters/convert>` will by default create RGBA.
This is the recommended format for RGB-data.

**YUY2**: In this colorspace each pair of pixels will share the color data,
as well as the data being interleaved like this ``YUYV|YUYV|YUYV|YUYV``. A
memory access of one word will get all data for **two** pixels. A pair should
never be split, as this can create strange artifacts when almost all
functions in YUY2 assumes that they should read a full word and process that.
Even if they return the right width when they are done, they will most likely
have used invalid data when processing valid pixels.


Planar formats
~~~~~~~~~~~~~~

Now the real fun begins as this is a :doc:`PlanarImageFormat <PlanarImageFormat>`. This means that
data is not :doc:`interleaved <InterleavedImageFormat>`, but stored separately for each color channel
(also called color plane). For filter writers this means that they can write
one simple function that is called three times, one for each color channel,
assuming that the operations are channel-independent (which is not always the
case). Again, using ``aligned`` for both color and luma channels will allow
easy memory access.

**Y8**: This colorformat is greyscale, i.e. contains no color. It is the only
format which is both planar and interleaved.

**YV12**: Four pixels share the color data. These four pixels are a 2x2
block, i.e. two pairs on adjacent lines in the same field. As fields
themselves are interleaved, this means that for field-based video line 1&3
share color data, as well as line 2&4. Frame-based video though has the more
instinctive way of sharing color, line 1&2 share just as line 3&4 does. The
2x2 block should never be split as this may create strange artifacts.

**YV411**: Four pixels share the color data. These four pixels are a 1x4
block, i.e. a quadruple. The 1x4 block should never be split as this may
create strange artifacts.

**YV16**: In this colorspace each pair of pixels will share the color data.
The pair should never be split as this may create strange artifacts. It is a
planar version of YUY2.

**YV24**: This color format has full color, i.e. each pixel has a unique
color (just like RGB and RGBA).


Part 2: How does this affect me as a user?
------------------------------------------

1.  As long as you don't split a colorsharing unit it's up to the
    filterwriters to take care of problems with memory reading at end of
    lines.
2.  Interlaced (fieldbased) video requires double mod on height.
3.  Required modulos in different colorspaces.

    -   **RGB(A)**

        -   width mod-1 (no restriction)
        -   height mod-1 (no restriction) *if progressive*
        -   height mod-2 (even values) *if interlaced*

    -   **YUY2**

        -   width mod-2 (even values)
        -   height mod-1 (no restriction) *if progressive*
        -   height mod-2 (even values) *if interlaced*

    -   **Y8**

        -   width mod-1 (no restriction)
        -   height mod-1 (no restriction) *if progressive*
        -   height mod-2 (even values) *if interlaced*

    -   **YV411**

        -   width mod-4
        -   height mod-1 (no restriction) *if progressive*
        -   height mod-2 (even values) *if interlaced*

    -   **YV12**

        -   width mod-2 (even values)
        -   height mod-2 (even values) *if progressive*
        -   height mod-4 *if interlaced*

    -   **YV16**

        -   width mod-2 (even values)
        -   height mod-1 (no restriction) *if progressive*
        -   height mod-2 (even values) *if interlaced*

    -   **YV24**

        -   width mod-1 (no restriction)
        -   height mod-1 (no restriction) *if progressive*
        -   height mod-2 (even values) *if interlaced*

4.  Examples of valid :doc:`Crops <../corefilters/crop>` with input 320x240 progressive

    -   **RGB(A)**

        -   Crop(1,7,-32,-19)
        -   Crop(2,4,300,196)

    -   **YUY2**

        -   Crop(2,7,-32,-19)
        -   Crop(2,4,300,196)

    -   **YV12**

        -   Crop(2,8,-32,-18)
        -   Crop(2,4,300,196)

5.  Note that final video may have other restrictions, most MPEG-n
    implementations want mod-16 on all resolutions etc.


More information
----------------

See more about :doc:`ColorSpaces <ColorSpaces>`.

See a general introduction to :doc:`WorkingWithImages <WorkingWithImages>`.

----

This page is a edited summary of `[this thread at Doom9's forum]`_

----

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $

.. _DataStorageInAviSynth:
    http://avisynth.org/mediawiki/Filter_SDK/Data_storage
.. _[this thread at Doom9's forum]:
    http://forum.doom9.org/showthread.php?s=&threadid=40413&highlight=YV12
