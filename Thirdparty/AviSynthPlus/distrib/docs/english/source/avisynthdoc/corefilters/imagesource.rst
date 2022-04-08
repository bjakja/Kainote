
ImageReader / ImageSource / ImageSourceAnim
===========================================

| ``ImageReader`` (string "file", int "start", int "end", float "fps", bool
  "use_DevIL", bool "info", string "pixel_type")
| ``ImageSource`` (string "file", int "start", int "end", float "fps", bool
  "use_DevIL", bool "info", string "pixel_type")
| ``ImageSourceAnim`` (string "file", float "fps", bool "info", string
  "pixel_type")

``ImageReader`` is present in *v2.52*, it replaces WarpEnterprises' plugin,
with some minor functionality changes. As of *v2.55* ``ImageSource`` is
equivalent, with some minor functionality changes. ``ImageSource`` is faster
than ``ImageReader`` when importing one picture.

file: template for the image file(s), where frame number substitution can be
specified using `sprintf syntax`_. For example, the files written by
:doc:`ImageWriter <imagewriter>`'s default parameters can be referenced with
``"c:\%06d.ebmp"``. As of v2.56 if the template points to a single file then
that file is read once and subsequently returned for all requested frames.

*start* = 0, *end* = 1000: Specifies the starting and ending numbers used for
filename generation. The file corresponding to start is always frame 0 in the
clip, the file corresponding to end is frame (end-start). The resulting clip
has (end-start+1) frames. ``end=0`` does NOT mean 'no upper bound' as with
:doc:`ImageWriter <imagewriter>`. The first file in the sequence, i.e., corresponding
to 'start', MUST exist in order for clip parameters to be computed. Any
missing files in the sequence are replaced with a blank frame.

*fps* = 24: frames per second of returned clip. An integer value prior to
*v2.55*.

*use_DevIL* = false: When false, an attempt is made to parse (E)BMP files with
the internal parser, upon failure (prior to v2.56) DevIL processing is
invoked. When true, execution skips directly to DevIL processing. You should
only need to use this if you have BMP files you don't want read by
``ImageReader``'s internal parser.

*NOTE* : DevIL version 1.6.6 as shipped with Avisynth does not correctly
support DIB/BMP type files that use a pallette, these include 8 bit RGB,
Monochrome, RLE8 and RLE4. Because the failure is usually catastrophic, from
revision 2.56, internal BMP processing does not automatically fail over to
DevIL processing. Forcing DevIL processing for these file types is currently
not recommended.

*info* = false: when true, the source filename and DevIL version is written to
each video frame (added in *v2.55*).

*pixel_type* = "rgb24": Allow the output pixel format to be specified, both
rgb24 and rgb32 are supported. The alpha channel is loaded only for rgb32 and
only if DevIL supports it for the loaded image format. (added in *v2.56*).

The resulting video clip colorspace is RGB if DevIL is used, otherwise it is
whatever colorspace an EBMP sequence was written from (all AviSynth formats
are supported).

Supported formats are:

-   (e)bmp, dds, ebmp, jpg/jpe/jpeg, pal, pcx, png, pbm/pgm/ppm, raw,
    sgi/bw/rgb/rgba, tga, tif/tiff.
-   gif, exr, jp2, psd, hdr. [all of them require 1.7.8 DevIL.dll]

``ImageSourceAnim`` (added in *v2.60*; requires 1.7.8 DevIL.dll) lets you
import animations (gif, ppm, tiff or psd). If there is a delay between the
first and second image in the animation, the framerate is set accordingly. If
this delay is zero, the framerate is set to 24 fps by default (and can be
adjusted by setting the *fps* parameter). Note that *pixel_type* is set to
RGB32 by default. If the images in the animation have unequal dimensions,
then the dimension of the first image is taken and the remaining images are
padded with black pixels below and or to the right.

1.7.8 DevIL.dll can be downloaded `here`_. Just put the DevIL.dll in your
system folder (it will overwrite the 1.6.6 DevIL.dll which comes with
AviSynth). Be sure to use the non-unicode version, because the other one
will result in crashes when loading pictures.

DevIL.dll 1.7.8 requires the 2005 VC runtime libraries to be correctly
installed: `Visual C++ 2005 SP1 Redistributable Package (x86)`_.

::

    # Default parameters: read a 1000-frame native AviSynth EBMP sequence (at 24 fps)
    ImageSource()

    # Read files "100.jpeg" through "199.jpeg" into an
    NTSC clip
    ImageSource("%d.jpeg", 100, 199, 29.97)
    # Note: floating-point fps available in v2.56

    # Read files "00.bmp"
    through "50.bmp" bypassing AviSynth's internal BMP reader
    ImageSource("%02d.bmp", end = 50, use_DevIL = true)

    # Read a single
    image, repeat 300 times
    ImageSource("static.png", end = 300, use_DevIL=true)
    # Much, much faster in v2.56

    # Read a greyscale (8-bit) jpg:
    ImageSource("GoldPetals-8bit.jpg", use_DevIL=true)

    # Read a greyscale (8-bit) BMP (using AviSynth's internal BMP reader):
    ImageSource("GoldPetals-8bit.bmp")

    # Read a YV24 BMP (created with ImageWriter):
    ImageSource("GoldPetals-24bit.ebmp")

    # Use a still-frame image with audio:
    audio = DirectShowSource("Gina La Piana - Start Over.flv")
    video = ImageSource("Gina La Piana.jpg", fps=25, start=1, end=ceil(25*AudioLengthF(audio)/AudioRate(audio)))
    return AudioDub(video, audio)

    # Read an animation:
    ImageSourceAnim("F:\TestPics\8bit_animated.gif")

**Notes:**

-   "EBMP" is an AviSynth extension of the standard Microsoft RIFF image
    format that allows you to save raw image data (all color formats are
    supported). See :doc:`ImageWriter <imagewriter>` for more details.
-   Greyscale BMPs are not read and written correctly by DevIL. They
    should be opened using **use_DevIL=false**.
-   DevIL version 1.6.7 as shipped with AviSynth does not correctly
    support DIB/BMP type files that use a pallette, these include 8 bit RGB,
    Monochrome, RLE8 and RLE4. Loading those images requires DevIL version
    1.7.8. (Because the failure is usually catastrophic, from revision v2.56,
    internal BMP processing does not automatically fail over to DevIL
    processing. Forcing DevIL processing with the shipped DevIL version for
    these file types is currently not recommended.)

+---------+-----------------------------------------------------------+
| Changes |                                                           |
+=========+===========================================================+
| v2.60   | - Added ImageSourceAnim.                                  |
|         | - Support user upgrade to 1.7.8 DevIL.dll                 |
|         |   (need to manage CRT dependancies).                      |
|         | - Palette and compressed bmp images load correctly now    |
|         |   (issue 894702) [requires 1.7.8 DevIL.dll]               |
|         | - Support for other formats like: gif, exr, jp2, psd, hdr |
|         |   [requires 1.7.8 DevIL.dll]                              |
|         | - Opening greyscale images (as Y8) added; EBMP supports   |
|         |   all color formats.                                      |
+---------+-----------------------------------------------------------+

$Date: 2012/10/10 13:41:51 $

.. _sprintf syntax:
    http://www.cplusplus.com/reference/clibrary/cstdio/sprintf/
.. _here:
    https://sourceforge.net/projects/openil/files/DevIL%20Win32/1.7.8/DevIL-EndUser-x86-1.7.8.zip
.. _Visual C++ 2005 SP1 Redistributable Package (x86):
    http://www.microsoft.com/downloads/details.aspx?familyid=200B2FD9-AE1A-4A14-984D-389C36F85647&displaylang=en
