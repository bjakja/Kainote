
Colorspace properties
=====================

In AviSynth v2.60, the colorspace properties are:
::

    // Colorspace properties.
    enum {
        CS_BGR               = 1<<28,
        CS_YUV               = 1<<29,
        CS_INTERLEAVED       = 1<<30,
        CS_PLANAR            = 1<<31,

        CS_VPlaneFirst       = 1<<3
        CS_UPlaneFirst       = 1<<4

        CS_Shift_Sub_Width   =  0
        CS_Shift_Sub_Height  =  8
        CS_Shift_Sample_Bits = 16

        CS_Sub_Width_1       = 3 << CS_Shift_Sub_Width, // YV24
        CS_Sub_Width_2       = 0 << CS_Shift_Sub_Width, // YV12, I420, YV16
        CS_Sub_Width_4       = 1 << CS_Shift_Sub_Width, // YUV9, YV411

        CS_Sub_Height_1      = 3 << CS_Shift_Sub_Height, // YV16, YV24, YV411
        CS_Sub_Height_2      = 0 << CS_Shift_Sub_Height, // YV12, I420
        CS_Sub_Height_4      = 1 << CS_Shift_Sub_Height, // YUV9

        CS_Sample_Bits_8     = 0 << CS_Shift_Sample_Bits,
        CS_Sample_Bits_16    = 1 << CS_Shift_Sample_Bits,
        CS_Sample_Bits_32    = 2 << CS_Shift_Sample_Bits,
    };

    // Specific colorformats
    enum {
        CS_UNKNOWN = 0,
        CS_BGR24 = 1<<0 | CS_BGR | CS_INTERLEAVED,
        CS_BGR32 = 1<<1 | CS_BGR | CS_INTERLEAVED,
        CS_YUY2  = 1<<2 | CS_YUV | CS_INTERLEAVED,
        CS_RAW32 = 1<<5 | CS_INTERLEAVED,
        CS_YV24  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_1,  // YUV 4:4:4 planar
        CS_YV16  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_2,  // YUV 4:2:2 planar
        CS_YV12  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_2 | CS_Sub_Width_2,  // y-v-u, 4:2:0 planar
        CS_I420  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_UPlaneFirst | CS_Sub_Height_2 | CS_Sub_Width_2,  // y-u-v, 4:2:0 planar
        CS_IYUV  = CS_I420,
        CS_YUV9  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_4 | CS_Sub_Width_4,  // YUV 4:1:0 planar
        CS_YV411 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_4,  // YUV 4:1:1 planar
        CS_Y8    = CS_PLANAR | CS_INTERLEAVED | CS_YUV | CS_Sample_Bits_8,                                     // Y   4:0:0 planar
    };


Thus CS_YV12 gives for example:
::

    CS_YV12 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_2 | CS_Sub_Width_2
            = 1<<31 | 1<<29 | 0<<16 | 1<<3 | 0<<8 | 0<<0
            = 1010.0000.0000.0000.0000.1000


Back to :doc:`FilterSDK` | :doc:`VideoInfo <VideoInfo>`

$Date: 2015/01/13 00:24:50 $
