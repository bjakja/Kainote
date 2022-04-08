
SimpleSample
============

I have rewritten SimpleSample for AviSynth 2.6. I changed it into an in
place filter (so the source is overwritten). It draws a white square in
the center of the clip and it supports all colorformats.

Here's simplesample.cpp:
::

    #include <windows.h>
    #include "avisynth.h"

    class SimpleSample : public GenericVideoFilter {
       int SquareSize;
    public:
       SimpleSample(PClip _child, int _SquareSize, IScriptEnvironment* env);
       ~SimpleSample();
       PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    };

    SimpleSample::SimpleSample(PClip _child, int _SquareSize, IScriptEnvironment* env) :
       GenericVideoFilter(_child), SquareSize(_SquareSize) {
       if (vi.width<SquareSize || vi.height<SquareSize) {
          env->ThrowError("SimpleSample: square doesn't fit into the clip!");
       }
    }

    SimpleSample::~SimpleSample() {}

    PVideoFrame __stdcall SimpleSample::GetFrame(int n, IScriptEnvironment* env) {

       PVideoFrame src = child->GetFrame(n, env);
       env->MakeWritable(&src);

       unsigned char* srcp = src->GetWritePtr();
       int src_pitch = src->GetPitch();
       int src_width = src->GetRowSize();
       int src_height = src->GetHeight();

       int w, h, woffset;

       if (vi.IsRGB24()) {
          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;

          woffset = src_width/2 - 3*SquareSize/2;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<3*SquareSize; w+=3) {
                *(srcp + woffset + w) = 255;
                *(srcp + woffset + w + 1) = 255;
                *(srcp + woffset + w + 2) = 255;
             }
             srcp += src_pitch;
          }
       }
       if (vi.IsRGB32()) {
          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;

          woffset = src_width/2 - 4*SquareSize/2;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<4*SquareSize; w+=4) {
                *(srcp + woffset + w) = 255;
                *(srcp + woffset + w + 1) = 255;
                *(srcp + woffset + w + 2) = 255;
             }
             srcp += src_pitch;
          }
       }
    /*
       if (vi.IsRGB32()) { // variant 1 - processing a pixel at once
          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;

          woffset = src_width/8 - SquareSize/2;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<SquareSize; w++) {
                *((unsigned int *)srcp + woffset + w) = 0x00FFFFFF;
             }
             srcp += src_pitch;
          }
       }
    */
    /*
       if (vi.IsRGB32()) { // variant 2 - processing a pixel at once
          unsigned int* srcp = (unsigned int*)src->GetWritePtr();

          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch/4;

          woffset = src_width/8 - SquareSize/2;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<SquareSize; w++) {
                srcp[woffset + w] = 0x00FFFFFF;
             }
             srcp += src_pitch/4;
          }
       }
    */
       if (vi.IsYUY2()) {
          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;

          woffset = src_width/8 - SquareSize/4;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<SquareSize/2; w++) {
                *((unsigned int *)srcp + woffset + w) = 0x80EB80EB;
             }
             srcp += src_pitch;
          }
       }
       if (vi.IsPlanar() && vi.IsYUV()) {

          int planes[] = {PLANAR_Y, PLANAR_U, PLANAR_V};
          int square_value[] = {235, 128, 128};
          int p;
          int width_sub, height_sub;

          for (p=0; p<3; p++) {
             srcp = src->GetWritePtr(planes[p]);
             src_pitch = src->GetPitch(planes[p]);
             src_width = src->GetRowSize(planes[p]);
             src_height = src->GetHeight(planes[p]);
             width_sub = vi.GetPlaneWidthSubsampling(planes[p]);
             height_sub = vi.GetPlaneHeightSubsampling(planes[p]);

             srcp = srcp + (src_height/2 - (SquareSize>>height_sub)/2) * src_pitch;

             woffset = src_width/2 - (SquareSize>>width_sub)/2;

             for (h=0; h<(SquareSize>>height_sub); h++) {
                for (w=0; w<(SquareSize>>width_sub); w++) {
                   srcp[woffset + w] = square_value[p];
                }
                srcp += src_pitch;
             }
          }
       }

       return src;
    }

    AVSValue __cdecl Create_SimpleSample(AVSValue args, void* user_data, IScriptEnvironment* env) {
       return new SimpleSample(args[0].AsClip(),
                               args[1].AsInt(100),
                               env);
    }

    const AVS_Linkage *AVS_linkage = 0;

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
       AVS_linkage = vectors;
       env->AddFunction("SimpleSample", "c[size]i", Create_SimpleSample, 0);
       return "SimpleSample plugin";
    }


Compile this file into a DLL named InvertNeg.dll. See :doc:`compiling instructions <CompilingAvisynthPlugins>`.
Now create an Avisynth script which looks something like this:
::

    LoadPlugin("d:\path\simplesample.dll")
    Colorbars().Trim(0,1)
    ConvertTORGB32()
    # ConvertTOYV411()
    SimpleSample(100)


Line by line breakdown
----------------------

Here's a line-by-line breakdown of simplesample.cpp. I won't repeat the
comments in the previous example InvertNeg.cpp, so read that first if
needed. The declaration of the class is as follows
::

    class SimpleSample : public GenericVideoFilter {
       int SquareSize;
    public:
       SimpleSample(PClip _child,   int _SquareSize, IScriptEnvironment* env);
       ~SimpleSample();
       PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    };


With respect to our previous example there are two differences. It
contains a private data member 'SquareSize' (members are private by
default). Note that this is our parameter variable. These have to be
declared here.

The member function ~SimpleSample() is the declaration of the
destructor. It fulfills the opposite functionality as the constructor.
It is automatically called when the filter is destroyed. It is used to
release allocated memory when the filter is destroyed. This is needed
when there is memory allocated in the constructor. This is not the case
in our filter, so we didn't need to declare it.
::

    SimpleSample::~SimpleSample() {}

This is the actual destructor. You can release allocated memory here
using the operator delete.
::

    SimpleSample::SimpleSample(PClip _child, int _SquareSize, IScriptEnvironment* env) :
       GenericVideoFilter(_child), SquareSize(_SquareSize) {
       if (vi.width<SquareSize || vi.height<SquareSize) {
          env->ThrowError("SimpleSample: square doesn't fit into the clip!");
       }
    }


This is the constructor. It initializes the value of SquareSize with
the parameter that is passed to it (which is called _SquareSize here).
It also checks whether the square which will be drawn fits in the
frame, otherwise it will return an error.
::

       unsigned char* srcp = src->GetWritePtr();
       int src_pitch = src->GetPitch();
       int src_width = src->GetRowSize();
       int src_height = src->GetHeight();


The default value of plane is PLANAR_Y (= 0) for the functions
GetReadPtr, GetWritePtr, GetPitch, GetRowSize and GetHeight. For planar
formats this is the luma plane and for interleaved formats this is the
whole frame.
::

       if (vi.IsRGB24()) {


When the clip has color format RGB24 the code path is taken. For RGB24
each pixel is represented by three bytes, blue, green and red (in that
order). RGB is up side down, so srcp[0] will be the bottom-left pixel.
::

          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;


The pointer is moved to the lower line of the square that will be
drawn. Remember that src_height is the height in pixels.
::

          woffset = src_width/2 - 3*SquareSize/2;

The offset is calculated of the left most byte of the square that will
be drawn. Remember that src_width is the width in bytes. Since
SquareSize is specified in pixels, the corresponding number of bytes is
3*SquareSize.
::

          for (h=0; h<SquareSize; h++) {         // Loop from bottom line to top line.
             for (w=0; w<3*SquareSize; w+=3) {   // Loop from left side of the image to the right side 1 pixel (3 bytes) at a time
                *(srcp + woffset + w) = 255;     // this is the same as srcp[woffset+w]=255;
                *(srcp + woffset + w + 1) = 255; // this is the same as srcp[woffset+w+1]=255;
                *(srcp + woffset + w + 2) = 255; // this is the same as srcp[woffset+w+2]=255;
             }

             srcp += src_pitch;
          }
       }


Here the white square is drawn. Each color component is set to 255.

For RGB32 the following code path is taken
::

       if (vi.IsRGB32()) {
          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;

          woffset = src_width/2 - 4*SquareSize/2;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<4*SquareSize; w+=4) {
                *(srcp + woffset + w) = 255;
                *(srcp + woffset + w + 1) = 255;
                *(srcp + woffset + w + 2) = 255;
             }
             srcp += src_pitch;
          }
       }


It's the same as for RGB24, except a pixel is represented by four
bytes, blue, green, red and alpha (in that order). Since SquareSize is
specified in pixels, the corresponding number of bytes is ``4*SquareSize``.
The fourth color component, the alpha channel, ``*(srcp + woffset + w + 3)``
is left untouched.

It is possible to speed the code above up a bit since you can deal with
whole 32bit variables at a time. This can be done in several ways:
::

       if (vi.IsRGB32()) { // variant 1 - processing a pixel at once
          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;

          woffset = src_width/8 - SquareSize/2;   // src_width/2 bytes equals src_width/8 pixels

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<SquareSize; w++) {
                *((unsigned int *)srcp + woffset + w) = 0x00FFFFFF;
             }
             srcp += src_pitch;
          }
       }


You need to cast srcp as unsigned int (instead of unsigned char).
unsigned int is 4 bytes or 32 bits (it runs from 0 to 2^32-1), which is
exactly what we need for one pixel. The casting is done in this line
::

                *((unsigned int *)srcp + woffset + w) = 0x00FFFFFF;


The value of a white pixel is 0x00FFFFFF (where the alpha pixel is set
to black). When writing several bytes at once, you need to write the
right one first and the left one last.

It is also possible to declare srcp as unsigned int when defining it,
but you have to take it into account in the rest of the code:
::

       if (vi.IsRGB32()) { // variant 2 - processing a pixel at once
          unsigned int* srcp = (unsigned int*)src->GetWritePtr();

          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch/4;

          woffset = src_width/8 - SquareSize/2;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<SquareSize; w++) {
                srcp[woffset + w] = 0x00FFFFFF;
             }
             srcp += src_pitch/4;
          }
       }


Remember that src_pitch is given in bytes and it is equal to
src_pitch/4 pixels.

For YUY2, each 4 byte sequence represents 2 pixels, (Y1, U, Y2 and then
V). So we can write two pixels at once by recasting again.
::

       if (vi.IsYUY2()) {
          srcp = srcp + (src_height/2 - SquareSize/2) * src_pitch;

          woffset = src_width/8 - SquareSize/4;

          for (h=0; h<SquareSize; h++) {
             for (w=0; w<SquareSize/2; w++) {   // we are writing two pixels at once
                *((unsigned int *)srcp + woffset + w) = 0x80EB80EB;   // VY2UY1; Y1=Y2=0xEB=235, U=V=0x80=128
             }
             srcp += src_pitch;
          }
       }


At last we move on to the YUV planar formats. Since we are processing
the color components independently we can loop over the planes.
::

       if (vi.IsPlanar() && vi.IsYUV()) {

All planar YUV formats in the 2.6 api are: YV24, YV16, YV12, YV411 and Y8.
::

          int planes[] = {PLANAR_Y, PLANAR_U, PLANAR_V};
          int square_value[] = {235, 128, 128};
          int p;
          int width_sub, height_sub;

          for (p=0; p<3; p++) {
             srcp = src->GetWritePtr(planes[p]);
             src_pitch = src->GetPitch(planes[p]);
             src_width = src->GetRowSize(planes[p]);
             src_height = src->GetHeight(planes[p]);
             width_sub = vi.GetPlaneWidthSubsampling(planes[p]);
             height_sub = vi.GetPlaneHeightSubsampling(planes[p]);


Since the planes have unequal width (measured in bytes; same for
height) we will need their subsampling. The functions
:doc:`GetPlaneWidthSubsampling <VideoInfo>` and :doc:`GetPlaneHeightSubsampling <VideoInfo>` are new in
the 2.6 api. They return the horizontal and vertical subsampling of the
plane compared to the luma plane. The subsampling of the formats is:

+--------------+------------------------------------+-------------------------------------+
| color format | GetPlaneWidthSubsampling(PLANAR_U) | GetPlaneHeightSubsampling(PLANAR_U) |
+==============+====================================+=====================================+
| YV24         | 0                                  | 0                                   |
+--------------+------------------------------------+-------------------------------------+
| YV16         | 1                                  | 0                                   |
+--------------+------------------------------------+-------------------------------------+
| YV12         | 1                                  | 1                                   |
+--------------+------------------------------------+-------------------------------------+
| YV411        | 2                                  | 1                                   |
+--------------+------------------------------------+-------------------------------------+
| Y8           | 0 ?                                | 0 ?                                 |
+--------------+------------------------------------+-------------------------------------+


The chroma planes PLANAR_U and PLANAR_V have the same subsampling. Also
GetPlaneWidthSubsampling(PLANAR_Y) =
GetPlaneHeightSubsampling(PLANAR_Y) = 0 since PLANAR_Y is not
subsampled.
::

             srcp = srcp + (src_height/2 - (SquareSize>>height_sub)/2) * src_pitch;

Note SquareSize is specified in pixels and src_height the height of the
samples of plane p. Let's look at YV12 for example. For the luma plane
we have SquareSize>>0 = SquareSize since there is no subsampling and
for the chroma planes we have SquareSize>>1 = SquareSize/2 since there
are twice as many luma samples as chroma samples horizontally.
::

             woffset = src_width/2 - (SquareSize>>width_sub)/2;

The vertical subsampling is taken into account again.
::

             for (h=0; h<(SquareSize>>height_sub); h++) {
                for (w=0; w<(SquareSize>>width_sub); w++) {
                   srcp[woffset + w] = square_value[p];
                }
                srcp += src_pitch;
             }
          }
       }


Here the white square is drawn. The luma pixels will get the value
square_value[PLANAR_Y] = 235, and the chroma pixels will get the value
square_value[PLANAR_U] = 128 (idem for PLANAR_V).
::

    AVSValue __cdecl Create_SimpleSample(AVSValue args, void* user_data, IScriptEnvironment* env) {
       return new SimpleSample(args[0].AsClip(),
                               args[1].AsInt(100),
                               env);
    }


args[0], args[1], ... are the arguments of the filter. Here there are
two arguments. The first one is of type clip and the second one of type
int (with default value 100). You can pre-process or post-process your
variables (and clips) here too. See DirectShowSource for some examples.
::

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
       AVS_linkage = vectors;
       env->AddFunction("SimpleSample", "c[size]i", Create_SimpleSample, 0);
       return "SimpleSample plugin";
    }


The :ref:`AddFunction <cplusplus_addfunction>` has the following parameters:
::

    AddFunction(Filtername, Arguments, Function to call, 0);


*Arguments* is a string that defines the types and optional names of the
arguments for your filter. The possible types are:

+---------------------------------+
| Argument type specifier strings |
+=================================+
+ c - clip                        +
+---------------------------------+
+ i - integer                     +
+---------------------------------+
+ f - float                       +
+---------------------------------+
+ s - string                      +
+---------------------------------+
+ b - boolean                     +
+---------------------------------+
+ . - Any type (dot)              +
+---------------------------------+

+-----------------------------------+
+ Array Specifiers                  +
+===================================+
+ i* - Integer Array, zero or more  +
+-----------------------------------+
+ i+ - Integer Array, one or more   +
+-----------------------------------+
+ .* - Any type Array, zero or more +
+-----------------------------------+
+ .+ - Any type Array, one or more  +
+-----------------------------------+

(have a look at StackVertical and SelectEvery for example as to how access such arrays)

The text inside the [ ] lets you used named parameters in your script. Thus
::

    clip = ...
    SimpleSample(clip, size=100)


but
::

    clip = ...
    SimpleSample(clip, 100)


works as well. See :doc:`InvertNeg` for more information.

____

Back to :doc:`FilterSDK`

$Date: 2015/09/14 20:23:59 $
