
VideoInfo
=========

The VideoInfo structure holds global information about a clip (i.e.
information that does not depend on the frame number). The GetVideoInfo
method in IClip returns this structure. Below is a description of it
(for AVISYNTH_INTERFACE_VERSION=6).

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents


Properties and constants
------------------------

**General properties:**
::

    int width, height; // width=0 means no video
    unsigned fps_numerator, fps_denominator;
    int num_frames; // max. num_frames = 2,147,483,647 (signed int32)
    int audio_samples_per_second; // audio_samples_per_second=0 means no audio
    int sample_type; // samples types are defined in avisynth.h
    uint64_t num_audio_samples;
    int nchannels;


**Colorspace properties and constants:**
::

    int pixel_type;

    enum {
        CS_BGR = 1<<28,
        CS_YUV = 1<<29,
        CS_INTERLEAVED = 1<<30,
        CS_PLANAR = 1<<31,

        // added in v5
        CS_Shift_Sub_Width = 0,
        CS_Shift_Sub_Height = 8,
        CS_Shift_Sample_Bits = 16,

        CS_Sub_Width_Mask = 7 << CS_Shift_Sub_Width,
        CS_Sub_Width_1 = 3 << CS_Shift_Sub_Width, // YV24
        CS_Sub_Width_2 = 0 << CS_Shift_Sub_Width, // YV12, I420, YV16
        CS_Sub_Width_4 = 1 << CS_Shift_Sub_Width, // YUV9, YV411

        CS_VPlaneFirst = 1 << 3, // YV12, YV16, YV24, YV411, YUV9
        CS_UPlaneFirst = 1 << 4, // I420

        CS_Sub_Height_Mask = 7 << CS_Shift_Sub_Height,
        CS_Sub_Height_1 = 3 << CS_Shift_Sub_Height, // YV16, YV24, YV411
        CS_Sub_Height_2 = 0 << CS_Shift_Sub_Height, // YV12, I420
        CS_Sub_Height_4 = 1 << CS_Shift_Sub_Height, // YUV9

        CS_Sample_Bits_Mask = 7 << CS_Shift_Sample_Bits,
        CS_Sample_Bits_8 = 0 << CS_Shift_Sample_Bits,
        CS_Sample_Bits_16 = 1 << CS_Shift_Sample_Bits,
        CS_Sample_Bits_32 = 2 << CS_Shift_Sample_Bits,

        CS_PLANAR_MASK = CS_PLANAR | CS_INTERLEAVED | CS_YUV | CS_BGR | CS_Sample_Bits_Mask | CS_Sub_Height_Mask | CS_Sub_Width_Mask,
        CS_PLANAR_FILTER = ~( CS_VPlaneFirst | CS_UPlaneFirst ),
    };


**Colorformat constants:**
::

    enum {
        CS_UNKNOWN = 0,
        CS_BGR24 = 1<<0 | CS_BGR | CS_INTERLEAVED,
        CS_BGR32 = 1<<1 | CS_BGR | CS_INTERLEAVED,
        CS_YUY2 = 1<<2 | CS_YUV | CS_INTERLEAVED,
        CS_YV12 = 1<<3 | CS_YUV | CS_PLANAR, // y-v-u, 4:2:0 planar // only in v3
        CS_I420 = 1<<4 | CS_YUV | CS_PLANAR, // y-u-v, 4:2:0 planar // only in v3
        CS_IYUV = 1<<4 | CS_YUV | CS_PLANAR, // same as above // only in v3

        // added in v5
        CS_RAW32 = 1<<5 | CS_INTERLEAVED,
        CS_YV24 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_1, // YUV 4:4:4 planar
        CS_YV16 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_2, // YUV 4:2:2 planar
        CS_YV12 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_2 | CS_Sub_Width_2, // y-v-u, 4:2:0 planar
        CS_I420 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_UPlaneFirst | CS_Sub_Height_2 | CS_Sub_Width_2, // y-u-v, 4:2:0 planar
        CS_IYUV = CS_I420,
        CS_YUV9 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_4 | CS_Sub_Width_4, // YUV 4:1:0 planar
        CS_YV411 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_4, // YUV 4:1:1 planar

        CS_Y8 = CS_PLANAR | CS_INTERLEAVED | CS_YUV | CS_Sample_Bits_8, // Y 4:0:0 planar
    };


**Image_type properties and constants:**
::

    int image_type;

    enum {
        IT_BFF = 1<<0,
        IT_TFF = 1<<1,
        IT_FIELDBASED = 1<<2
    };


**Chroma placement constants (bits 20 -> 23):**
::

    enum { // added in v5
        CS_UNKNOWN_CHROMA_PLACEMENT = 0 << 20,
        CS_MPEG1_CHROMA_PLACEMENT = 1 << 20,
        CS_MPEG2_CHROMA_PLACEMENT = 2 << 20,
        CS_YUY2_CHROMA_PLACEMENT = 3 << 20,
        CS_TOPLEFT_CHROMA_PLACEMENT = 4 << 20
    };


Functions [need to add examples]
--------------------------------

HasVideo
~~~~~~~~

::

    bool HasVideo();


This will return true if there is any video in the given clip.


IsRGB / IsRGB24 / IsRGB32
~~~~~~~~~~~~~~~~~~~~~~~~~

::

    bool IsRGB();
    bool IsRGB24();
    bool IsRGB32();


This will return true if the colorspace is `RGB`_ (in any way). The first two
return true if the clip has the specific RGB colorspace (:doc:`RGB24 <ColorSpaces>` and
:doc:`RGB32 <ColorSpaces>`). The third returns true for any RGB colorspace; future formats
could also apply.


IsYUV / IsYUY2 / IsYV24 / IsYV16 / IsYV12 / IsYV411 / IsY8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    bool IsYUV() const;
    bool IsYUY2() const;
    bool IsYV24() const;  // v5
    bool IsYV16() const;  // v5
    bool IsYV12() const;
    bool IsYV411() const; // v5
    bool IsY8() const;    // v5


This will return true if the colorspace is :doc:`YUV <ColorSpaces>` (in any way). The first two
return true if the clip has the specific YUV colorspace (:doc:`YUY2 <ColorSpaces>` and
:doc:`YV12 <ColorSpaces>`). The third returns true for any YUV colorspace; future formats could
also apply. Note that I420 is also reported as YV12, because planes are
automatically swapped.


IsColorSpace / IsSameColorspace / IsPlanar
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    bool IsColorSpace(int c_space);


This function will check if the colorspace (VideoInfo.pixel_type) is the same
as given c_space (or more general it checks for a :doc:`Colorspace property <ColorspaceProperties>` (see
avisynth.h)).


::

    bool IsSameColorspace(const VideoInfo& vi2);


This function will compare two VideoInfos, and check if the colorspace is the
same. Note: It does not check imagesize or similar properties.


::

    bool IsPlanar();


This will return true if the video is planar. For now only YV12 returns true,
but future formats might also do so. See the :doc:`Planar <PlanarImageFormat>` image format.


Is / IsFieldBased / IsParityKnown / IsBFF / IsTFF
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    bool Is(int property);


From v6 this will return true if the image type (VideoInfo.image_type)
is the same as the given property (being IT_BFF, IT_TFF or
IT_FIELDBASED).


::

    bool IsFieldBased();


This will return true if the video has been through a :doc:`SeparateFields <../corefilters/separatefields>`, and
the video has not been :doc:`weaved <../corefilters/weave>` yet. Otherwise it will return false.

::

    bool IsParityKnown();


This will return true if the video parity is known.

::

    bool IsBFF();
    bool IsTFF();


This will return true if the video is bottom-field-first or top-field-first
respectively.


SetFieldBased / Set / Clear
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    void SetFieldBased(bool isfieldbased);


This will set the field-based property to true (respectively false) if
isfieldbased=true (respectively false).

::

    void Set(int property);
    void Clear(int property);


This sets respectively clears an image_type property like: IT_BFF, IT_TFF or
IT_FIELDBASED. See field.h for examples.


BitsPerPixel
~~~~~~~~~~~~

::

    int BitsPerPixel();


This will return the number of bits per pixel. This can be:

+------------------+------------+
| pixel_type       | nr of bits |
+==================+============+
| CS_BGR24         | 24         |
+------------------+------------+
| CS_BGR32         | 32         |
+------------------+------------+
| CS_YUY2          | 16         |
+------------------+------------+
| CS_YV12, CS_I420 | 12         |
+------------------+------------+


SetFPS / MulDivFPS
~~~~~~~~~~~~~~~~~~

::

    void SetFPS(unsigned numerator, unsigned denominator);


This will set the framerate.

::

    void MulDivFPS(unsigned multiplier, unsigned divisor);


This will multiply the denominator by *multiplier* and scale the numerator
and modified denominator.

There is some other useful information in VideoInfo structure (width, height,
fps_numerator, fps_denominator, num_frames, pixel_type and image_type). See
'avisynth.h' header file.


BytesFromPixels
~~~~~~~~~~~~~~~

::

    int BytesFromPixels(int pixels) const;


For interleaved formats it will return the number of bytes from the
specified number of pixels. For planar formats it will do the same
except it operates on the first plane.


RowSize / BMPSize
~~~~~~~~~~~~~~~~~

::

    int RowSize(int plane=0) const;


For interleaved formats it will return the width of the frame in bytes.
For planar formats it will return the width of the specified plane in
bytes.

examples:

| 640x480 RGB24: RowSize() = 3*640 = 1920

| 640x480 YV12: RowSize(PLANAR_Y) = 640
| 640x480 YV12: RowSize(PLANAR_U) = 320

::

    int BMPSize() const;

For interleaved formats it will return the size of the frame in bytes
where the width is rounded up to a multiple of 4 bytes. For planar
formats it will do the same for the luma plane then add the two chroma
planes scaled by the subsampling. So, it's the number of bytes of a
frame as if it was a `BMP frame`_.

examples:

| 640x480 RGB24: BMPSize() = 480 * 3*640 = 921600
| 643x480 RGB24: BMPSize() = 480 * 3*644 = 927360

| 640x480 YV12: BMPSize() = 480 * 640 * (1+2*0.25) = 460800
| 640x480 YV16: BMPSize() = 480 * 640 * (1+2*0.5) = 614400
| 640x480 YV24: BMPSize() = 480 * 640 * (1+2*1) = 921600
| 643x480 YV24: BMPSize() = 480 * 644 * (1+2*1) = 927360


GetPlaneWidthSubsampling / GetPlaneHeightSubsampling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    int GetPlaneWidthSubsampling(int plane) const; // v5

This will return the subsampling of the width in bitshifts.

examples:

| YV24: GetPlaneWidthSubsampling(PLANAR_U) = 0 // since there is no
  horizontal subsampling on a chroma plane
| YV16: GetPlaneWidthSubsampling(PLANAR_U) = 0 // since there is no
  horizontal subsampling on a chroma plane

::

    int GetPlaneHeightSubsampling(int plane) const; // v5

This will return the subsampling of the height in bitshifts.

examples:

| YV24: GetPlaneHeightSubsampling(PLANAR_U) = 0 // since there is no
  vertical subsampling on a chroma plane
| YV16: GetPlaneHeightSubsampling(PLANAR_U) = 1 // since vertically there
  are two times less samples on a chroma plane compared to a plane which
  is not subsampled

+--------------+------------------------------------+-------------------------------------+
| color format | GetPlaneWidthSubsampling(PLANAR_U) | GetPlaneHeightSubsampling(PLANAR_U) |
+==============+====================================+=====================================+
| YV24         | 0                                  | 0                                   |
+--------------+------------------------------------+-------------------------------------+
| YV16/YUY2    | 1                                  | 0                                   |
+--------------+------------------------------------+-------------------------------------+
| YV12         | 1                                  | 1                                   |
+--------------+------------------------------------+-------------------------------------+
| YV411        | 2                                  | 1                                   |
+--------------+------------------------------------+-------------------------------------+
| Y8           | Error thrown                       | Error thrown                        |
+--------------+------------------------------------+-------------------------------------+


HasAudio
~~~~~~~~

::

    bool HasAudio();


This will return true if there is any audio in the given clip.


AudioChannels / SampleType
~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    int AudioChannels();


This will return the number of audio channels.


::

    int SampleType();


This will return the sampletype. This can be:

+--------------+------------+
| pixel_type   | nr of bits |
+==============+============+
| SAMPLE_INT8  | 1<<0       |
+--------------+------------+
| SAMPLE_INT16 | 1<<1       |
+--------------+------------+
| SAMPLE_INT24 | 1<<2       |
+--------------+------------+
| SAMPLE_INT32 | 1<<3       |
+--------------+------------+
| SAMPLE_FLOAT | 1<<4       |
+--------------+------------+


IsSampleType
~~~~~~~~~~~~

::

    bool IsSampleType(int testtype);


This function will check if the sampletype (VideoInfo.sample_type) is the
same as testtype.


SamplesPerSecond / BytesPerAudioSample / BytesPerChannelSample
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    int SamplesPerSecond();


This will return the number of bytes per second.

::

    int BytesPerAudioSample();


This will return the number of bytes per sample:

::

    int BytesPerChannelSample()


This will return the number of bytes per channel-sample. This can be:

+--------------+----------------------+
| sample       | nr of bytes          |
+==============+======================+
| SAMPLE_INT8  | sizeof(signed char)  |
+--------------+----------------------+
| SAMPLE_INT16 | sizeof(signed short) |
+--------------+----------------------+
| SAMPLE_INT24 | 3                    |
+--------------+----------------------+
| SAMPLE_INT32 | sizeof(signed int)   |
+--------------+----------------------+
| SAMPLE_FLOAT | sizeof(SFLOAT)       |
+--------------+----------------------+


AudioSamplesFromFrames / FramesFromAudioSamples / AudioSamplesFromBytes / BytesFromAudioSamples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    __int64 AudioSamplesFromFrames(__int64 frames);


This returns the number of audiosamples from the first *frames* frames.

::

    int FramesFromAudioSamples(__int64 samples);


This returns the number of frames from the first *samples* audiosamples.

::

    __int64 AudioSamplesFromBytes(__int64 bytes);


This returns the number of audiosamples from the first *bytes* bytes.

::

    __int64 BytesFromAudioSamples(__int64 samples);


This returns the number of bytes from the first *samples* audiosamples.

There is some other useful information in VideoInfo structure
(audio_samples_per_second, sample_type, num_audio_samples and nchannels). See
'avisynth.h' header file.

----

Back to :doc:`FilterSDK`

$Date: 2015/01/13 00:24:50 $

.. _RGB: http://avisynth.org/mediawiki/RGB
.. _BMP frame: http://en.wikipedia.org/wiki/BMP_file_format
