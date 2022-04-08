
AviSynth FAQ - The color format YV12 and related processing and encoding issues
===============================================================================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



What is YV12?
-------------

These are several different ways to represent colors. For example: YUV and
RGB colorspace. In YUV colorspace there is one component that represent
lightness (luma) and two other components that represent color (chroma). As
long as the luma is conveyed with full detail, detail in the chroma
components can be reduced by subsampling (filtering, or averaging) which can
be done in several ways (thus there are multiple formats for storing a
picture in YUV colorspace). YV12 is such a format (where chroma is shared in
every 2x2 pixel block) that is supported by AviSynth. Many important codecs
stored the video in YV12: MPEG-4 (x264, XviD, DivX and many others), MPEG-2
on DVDs, MPEG-1 and MJPEG.

Where can I download the latest stable AviSynth version which supports YV12?
----------------------------------------------------------------------------

"AviSynth v2.57" (and more recent versions) can be downloaded `on Sourceforge`_.

Where can I download the DGIndex/DGDecode plugin, which supports YV12, to import MPEG-1/MPEG-2/TS/PVA in AviSynth?
------------------------------------------------------------------------------------------------------------------

The latest DGIndex/DGDecode combo can be downloaded `from its website`_.

Which encoding programs support YV12?
-------------------------------------

The regular builds of Virtualdub (by Avery Lee) have YV12 support in fast
recompress mode since v1.5.6. There are also two another options for encoding
to DivX/XviD:

- There is a modified version (called VirtualdubMod) which has YV12 support:
  This modification (by pulco-citron, Suiryc and Belgabor) has OGM and AVS-
  preview support. It can be downloaded from `its Sourceforge page`_. In order to use the YV12
  support (without doing any color conversions) you have to load your AVI in
  VirtualdubMod and select "fast recompress".

- For easy (and fast) YV12 support, you can also try out the command line
  utility `AVS2AVI`_ - compresses video from an AviSynth script using any VFW
  codec, see also `here`_.

- The MPEG-1/MPEG-2 encoders `HC`_ and `QuEnc`_ also support (and even
  require) YV12.

How do I use v2.5x if the encoding programs can't handle YV12 (like TMPGEnc or CCE SP)?
---------------------------------------------------------------------------------------

Using TMPGEnc you have to add the line ":doc:`ConvertToRGB24 <../corefilters/convert>`" (with proper
"interlaced" option) to your script, and for CCE SP you need to add the line
":doc:`ConvertToYUY2 <../corefilters/convert>`" to your script, since Windows has no internal YV12
decompressor.

You can also install some :ref:`YV12 decompressor (codec) <Couldn't locate decompressor>` which will decompress
the YV12-AVI for you when loading the avi in TMPGEnc or CCE SP.

What will be the main advantages of processing in YV12?
-------------------------------------------------------

-   speed increase:

    That depends entirely on the external plugins whether they will have YV12
    support or not. Speed increases like 25-35 percent are expected. Of course
    there will only be a large speed increase if both your source and target are
    in YV12, for example in DVD to DivX/Xvid conversions.

-   no color conversions:

    The colour information doesn't get interpolated (so often) and thus stays
    more realistic.

MPEG-2 encoders such as CCE, Procoder and TMPGEnc can't handle YV12 input
directly. CCE and Procoder needs YUY2, and TMPGEnc RGB24. This only means
that the last line of AviSynth must be a :doc:`ConvertToYUY2 <../corefilters/convert>` (for CCE/Procoder,
or :doc:`ConvertToRGB24 <../corefilters/convert>` for TMPGEnc) call, and that you will not be able to
take full advantage of YV12 colorspace. Still there are two advantages:

1.  All internal filtering in AviSynth will be faster though (less data
    to filter, better structure to filter, and a very fast conversion from
    YV12 to YUY2), and you will definitely be able to tell the difference
    between v2.06 and v2.5.
2.  If you are making a progressive clip there is another advantage.
    Putting off the YV12->YUY2 conversion until the end of the script allows
    you to first IVTC or Deinterlace to create progressive frames. But the
    YV12 to YUY2 conversion for progressive frames maintains more chroma
    detail than it does for interlaced or field-based frames.

The color conversions:
CCE: YV12 -> YUY2 -> YV12
TMPGEnc: YV12 -> RGB24 -> YV12


How do I use VirtualDub/VirtualDubMod such that there are no unnecessary color conversions?
-------------------------------------------------------------------------------------------

Just load your avs file in VirtualDub/VirtualdubMod and set the video on
"Fast recompress". In this mode the process will stay in YV12 (all the
necessary filtering has to be done in AviSynth itself). Under compression
select a codec which support YV12, like Xvid, DivX5, RealVideo (provided you
download the lastest binaries) or 3ivx D4 (provided you download the lastest
binaries). Note that DivX3/4 also supports YV12, except that PIV users could
experience crashes when encoding to DivX4 in YV12.
If you want to preview the video you also need a :ref:`YV12 decompressor. <Couldn't locate decompressor>`

Which internal filters support YV12?
------------------------------------

In principal all internal filters support YV12 natively. Which color formats
the filters support is specified in the documentation.

Which external plugins support YV12?
------------------------------------

The plugins which are compiled for AviSynth v2.5 are given in
:doc:`External plugins <../externalplugins>`. New plugins are listed in
this `sticky`_. Most of them support YV12 (see documentation).

Are there any disadvantages of processing in YV12?
--------------------------------------------------

-   If source format is not YV12 (analog capture, DV) or final encoding
    format is not YV12, then color format conversion will results in chroma
    interpolation with some quality decreasing.
-   Filtering of subsampled chroma can result in some chroma broadening
    relatively luminocity pixels, especially for interlaced video.
-   Because the chroma in interlaced YV12 video occurs on alternating
    lines, it is necessary to use a different upsampling/downsampling method
    when converting between YV12 and YUV 4:2:2 or RGB. This can lead to
    chroma upsampling/downsampling errors if the wrong color space conversion
    method is used on the video.
-   If YV12 video is stored in an AVI container, there is no metadata to
    indicate whether the video is interlaced or progressive. This means that
    an application or component doing color space conversion has no easy way
    of choosing the correct conversion method (interlaced or progressive).
    Most color space converters assume progressive which can lead to chroma
    upsampling/downsampling errors when interlaced video is processed in such
    an environment.


How do I know which colorspace I'm using at a given place in my script?
-----------------------------------------------------------------------

To see which colorspace you are using at a given place in your script, add:
::

    Info()

... and AviSynth will give you much information about colorspace amongst
other things!

The colors are swapped when I load a DivX file in AviSynth v2.5?
----------------------------------------------------------------

This happens due to a bug in old versions of DivX5. Download the latest
binaries or use ":doc:`SwapUV() <../corefilters/swap>`".

I got a green (or colored line) at the left or at the right of the clip, how do I get rid of it?
------------------------------------------------------------------------------------------------

Your decoder is probably borked, try a ConvertToRGB() at the end of your
script just to be sure and check whether the line has disappeared. Some
application have trouble displaying YV12 clips where the width or height is
not a multiple of 16.

There are several solutions to this problem:

-   Try having the codec decode to RGB or YUY2 (using pixel_type="..."
    argument in :doc:`AviSource <../corefilters/avisource>` or :doc:`DirectShowSource <../corefilters/directshowsource>`).
-   Use a codec that correctly decodes YV12 clips where the width or
    height is not a multiple of 16.


.. _Couldn't locate decompressor:

I installed AviSynth v2.5 and get the following error message: "Couldn't locate decompressor for format 'YV12' (unknown)."?
---------------------------------------------------------------------------------------------------------------------------

Install a codec which supports YV12. DivX5 or one of the recent `XviD
builds`_ or `Helix YUV codec`_ or some other (ffvfw, ffdshow). If that still
doesn't work, modify your registry as explained in the next question.


I installed AviSynth v2.5 and DivX5 (or one of the latest Xvid builds of Koepi), all I got is a black screen when opening my avs in VirtualDub/VirtualDubMod/MPEG-2 encoder?
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Ok, apparently your video is not decompressed by DivX 5.02 (or Xvid). Try to
use `VCSwap utility`_ for hot swapping video codecs.

Advanced user can also do it by hand. Go to your windows-dir and rename a
file called MSYUV.DLL, or add the following to your registry file:

::

    REGEDIT4

    [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows
    NT\CurrentVersion\Drivers32]
    "VIDC.YV12"="divx.dll"

Replace "divx.dll" by "xvid.dll" for xvid v0.9 or "xvidvfw.dll" for xvid
v1.0.

Are there any lossless YV12 codecs, which I can use for capturing for example?
------------------------------------------------------------------------------

Capturing in YV12 is not recommended due to issues of interlacing and chroma;
YUY2 will generally pose fewer problems. A lossless YV12 codec is more useful
for saving intermediate files before a multi-pass encode, to avoid having to
run a CPU-intensive script several times. There are several lossless YV12
codecs:

-   `VBLE Codec (by MarcFD)`_: A huffyuv based encoder [`discussion`_].
-   `LocoCodec (by TheRealMoh)`_: see also `here`_.
-   `ffvfw codec`_ - has various modes, in particular HuffYUV yv12.
-   `Lagarith codec (by Ben Greenwood)`_ - better compression than
    Huffyuv but slower.


Some important links
--------------------

-   `Technical explanation of YV12 (and similar formats)`_
-   `Good Microsoft page on YUV`_
-   `4:2:0 Video Pixel Formats`_

| :doc:`Main Page <faq_sections>` | :doc:`General Info <faq_general_info>` | :doc:`Loading Clips <faq_loading_clips>` | :doc:`Loading Scripts <faq_frameserving>` | :doc:`Common Error Messages <faq_common_errors>` | :doc:`Processing Different Content <faq_different_types_content>` | **Dealing with YV12** | :doc:`Processing with Virtualdub Plugins <faq_using_virtualdub_plugins>` |

$Date: 2013/03/19 18:10:26 $

.. _on Sourceforge: http://sourceforge.net/project/showfiles.php?group_id=57023
.. _from its website: http://neuron2.net/dgmpgdec/dgmpgdec.html
.. _its Sourceforge page: http://sourceforge.net/projects/virtualdubmod/virtualdubmod.html
.. _AVS2AVI: http://www.avs2avi.org
.. _here: http://forum.doom9.org/showthread.php?t=71493
.. _HC: http://www.bitburners.com/HC_Encoder/
.. _QuEnc: http://www.bitburners.com/QuEnc/
.. _sticky: http://forum.doom9.org/showthread.php?s=&threadid=84481
.. _XviD builds: http://www.xvid.org/
.. _Helix YUV codec:
    http://forum.doom9.org/showthread.php?s=&threadid=56972
.. _VCSwap utility: http://members.chello.nl/~p.bekke/
.. _VBLE Codec (by MarcFD):
    http://forum.doom9.org/showthread.php?s=&threadid=53305
.. _discussion:
    http://forum.doom9.org/showthread.php?s=&threadid=38389&pagenumber=5
.. _LocoCodec (by TheRealMoh):
    http://forum.doom9.org/showthread.php?s=&threadid=50363
.. _ffvfw codec: http://www.free-codecs.com/ffvfw_download.htm
.. _Lagarith codec (by Ben Greenwood):
    http://lags.leetcode.net/codec.html
.. _Technical explanation of YV12 (and similar formats):
    http://www.fourcc.org/fccyuv.htm#YV12
.. _Good Microsoft page on YUV:
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwmt/html/yuvformats.asp
.. _4:2:0 Video Pixel Formats:
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/Display_d/hh/Display_d/dxvaguide_00174d47-49a2-4c28-b67e-ce5a0a58e8ae.xml.asp
