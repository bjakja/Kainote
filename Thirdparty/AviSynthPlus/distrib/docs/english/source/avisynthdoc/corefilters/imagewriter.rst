
ImageWriter
===========

``ImageWriter`` (clip, string "file", int "start", int "end", string "type",
bool "info")

``ImageWriter`` (present in limited form in *v2.51*, full functionality in
*v2.52*) writes frames from a clip as images to your harddisk.

*file* default "c:\": is the path + filename prefix of the saved images. The
images have filenames such as:

   ``[path]000000.[type], [path]000001.[type], etc.``

In *v2.60* support for printf formating of filename is added. The default
reflects the old behaviour (file="C:\%06d").

start and end are the start and end of the frame range that will be written.
They both default to 0 (where ''end''=0 means last frame). If end is negative
(possible since *v2.58*), it specifies the number of frames that will be
written.

*type* default "ebmp", is the filename extension and defines the format of the
image.
The supported values for type, are:

    ``(e)bmp, dds, ebmp, jpg/jpe/jpeg, pal, pcx, png,
    pbm/pgm/ppm, raw, sgi/bw/rgb/rgba, tga, tif/tiff``

*info* default false: optionally overlay text progress information on the
output video clip, showing whether a file is being written, and if so, the
filename (added in *v2.55*).

Format "ebmp" supports all colorspaces (RGB24, RGB32, YUY2, YV12).  The
"ebmp" files written from RGB colorspaces are standard BMP files; those
produced from YUV formats can probably only be read by AviSynth's
:doc:`ImageReader/ImageSource <imagesource>`. This pair of features allows you to save
and reload raw video in any internal format.

For all other formats the input colorspace* must be RGB24 or RGB32 (when the
alpha channel is supported by the format and you want to include it).*

**Examples:**
::

    # Export the entire clip in the current native AviSynth format
    ImageWriter("D:\backup-stills\myvideo")

    # Write frame 5 to "C:\000005.PNG"
    ImageWriter("", 5, 5, "png")

    # Write frames 100 till the end to "F:\pic-000100.JPEG", "F:\pic-000101.JPEG", etc.
    # and display progress info
    ImageWriter(file = "F:\pic", start = 100, type = "jpeg", info = true)

    # Write a jpg as greyscale (note the luma range should be [0,255], not [16,235])
    ImageSource("F:\TestPics\GoldPetals.jpg")
    ConvertToY8(matrix="PC.601")
    ImageWriter("F:\TestPics\GoldPetals-8bit-avs", type = "png")

    # Write a jpg as YV24 ebmp (note the luma range should be [0,255], not [16,235])
    ImageSource("F:\TestPics\GoldPetals.jpg")
    ConvertToYV24(matrix="PC.601")
    ImageWriter("F:\TestPics\GoldPetals-24bit", type = "ebmp")

    # Write all frames to "F:\00.bmp", "F:\01.bmp", ..., "F:\10.bmp", "F:\11.bmp", ..., "F:\100.bmp", ...
    # (thus adding zeros filling two digits)
    ImageWriter(file="F:\%02d.bmp")

+-----------+------------------------------------------------------------------+
| Changelog |                                                                  |
+===========+==================================================================+
| v2.58     | added end=-num_frames                                            |
+-----------+------------------------------------------------------------------+
| v2.60     || ebmp supports all formats; greyscale added for all formats.     |
|           || add support for printf formating of filename string, default is |
|           |  ("%06d.%s", n, ext).                                            |
+-----------+------------------------------------------------------------------+

$Date: 2011/01/16 12:22:43 $
