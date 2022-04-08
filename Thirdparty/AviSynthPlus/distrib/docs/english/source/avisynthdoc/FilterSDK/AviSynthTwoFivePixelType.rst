
`AviSynthTwoFivePixelType`_
===========================

The internals of VideoInfo.pixel_type has been completely changed. The type
has been changed from byte to int, and now contains different information
than before. This is important when building a Source filter, when you have
to set the pixel_type. You have to change your constants to reflect this
change.

In general filters should not care about these values at all - only if they
set or modify them. For checking what colorspace the video material is in,
use one of the following functions in VideoInfo.

-   bool IsRGB()
-   bool IsRGB24()
-   bool IsRGB32()
-   bool IsYUV()
-   bool IsYUY2()
-   bool IsYV12()
-   bool IsPlanar()

If you need to set the pixel type, use a syntax like:

``vi.pixel_type = VideoInfo::CS_YUY2; // Version 2.5 syntax``

instead of:

``vi.pixel_type = VideoInfo::YUY2; // Version 2.0 syntax``

Other possibilities are:

-   CS_BGR24,
-   CS_BGR32,
-   CS_YUY2,
-   CS_YV12, // y-v-u, planar
-   CS_I420, // y-u-v, planar
-   CS_IYUV // same as above

The last two are automatically converted to YV12, so filter writers should
not worry about these.

See also :doc:`WorkingWithImages <WorkingWithImages>`.


Back to :doc:`AviSynthTwoFiveSDK <AviSynthTwoFiveSDK>`

$Date: 2006/11/24 18:21:25 $

.. _AviSynthTwoFivePixelType:
    http://www.avisynth.org/AviSynthTwoFivePixelType
