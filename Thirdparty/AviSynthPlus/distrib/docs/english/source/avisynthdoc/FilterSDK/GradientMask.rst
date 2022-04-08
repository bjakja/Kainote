
GradientMask
============

I'll start off with a simple source filter. It's called "GradientMask",
and it produces a gradient.

Here's GradientMask.cpp

::

    #include <windows.h>
    #include "avisynth.h"

    class GradientMask : public IClip {
    public:
      GradientMask(int _w, int _h, int _color, IScriptEnvironment* env);
      PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
      void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {};
      bool __stdcall GetParity(int n) { return false; };
      int __stdcall SetCacheHints(int cachehints, int frame_range) { return 0; };
      const VideoInfo& __stdcall GetVideoInfo() { return vi; };
    private:
      int w;
      int h;
      int color;
      VideoInfo vi;
      PVideoFrame frame;
    };

    GradientMask::GradientMask(int _w, int _h, int _color, IScriptEnvironment* env) :
                               w(_w), h(_h), color(_color) {
      memset(&vi, 0, sizeof(VideoInfo));
      vi.width = w;
      vi.height = h;
      vi.fps_numerator = 30000;
      vi.fps_denominator = 1001;
      vi.num_frames = 1;
      vi.pixel_type = VideoInfo::CS_BGR24;

      frame = env->NewVideoFrame(vi);

      int y, i;
      BYTE* p;
      int fpitch, fheight, fwidth;
      float red, green, blue, luma;

      static const int col[] = {(color >> 16) & 0xff, (color >> 8) & 0xff, (color >> 0) & 0xff};
      luma = float((col[0]+col[1]+col[2])/3.0);

      blue = max(col[2] - luma,0);
      green = max(col[1] - luma,0);
      red = max(col[0] - luma,0);

      p = frame->GetWritePtr();
      fpitch = frame->GetPitch();
      fwidth = frame->GetRowSize(); // in bytes
      fheight = frame->GetHeight(); // in pixels

      for (y=0; y<fheight; y++) {
        for (i=0; i<fwidth; i+=3) {
          p[i] = int(min(i * 255.0/float(fwidth) + blue,255));
          p[i+1] = int(min(i * 255.0/float(fwidth) + green,255));
          p[i+2] = int(min(i * 255.0/float(fwidth) + red,255));
        }
        p += fpitch;
      }
    }

    PVideoFrame __stdcall GradientMask::GetFrame(int n, IScriptEnvironment* env) { return frame; }

    static AVSValue __cdecl Create_GradientMask(AVSValue args, void* user_data, IScriptEnvironment* env) {
      return new GradientMask(args[0].AsInt(640), args[1].AsInt(480), args[2].AsInt(0), env);
    }

    const AVS_Linkage *AVS_linkage = 0;

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
      AVS_linkage = vectors;
      env->AddFunction("GradientMask", "[width]i[height]i[color]i", Create_GradientMask, 0);
      return "GradientMask sample plugin";
    }

Compile this file into a DLL named GradientMask.dll. See :doc:`compiling
instructions <CompilingAvisynthPlugins>`. Now create an Avisynth
script which looks something like this:

::

    LoadPlugin("d:\path\GradientMask.dll")
    # GradientMask(width=1024, height=768)
    GradientMask(color=$FF0000)

Line by line breakdown
----------------------

Here's a line-by-line breakdown of GradientMask.cpp.

As explained in the :doc:`InvertNeg <InvertNeg>` filter, an Avisynth
filter is simply a C++ class implementing the IClip interface. The class
GenericVideoFilter is a simple do-nothing filter defined in avisynth.h.
It derives from IClip and implements all four methods (being being
GetFrame, GetAudio, GetParity and SetCacheHints). Filters that have a
parent clip and thus can inherit from GenericVideoFilter rather than
directly from IClip; this saves you from having to implement methods
that you don't care about. Source filters don't have a parent clip and
need to derive them from IClip.

Since we are developing a source filter we need to derive from IClip,
fill out a VideoInfo and implement all four methods (instead of
overriding the ones we need).

In the example below a gradientmask is created. It fades from a fully
saturated color to white. The gradientmask will be created in the
constructor and returned only once.

::

    class GradientMask : public IClip {

As stated we need to inherit from IClip directly ...

::

    public:
      GradientMask(int _w, int _h, int _color, IScriptEnvironment* env);

This filter has three input parameters (being the width, height and
color).

::

      PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

GetFrame will be overridden below and will be filled with our Gradient.

::

      void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {};

Our filter will not return any audio. Have a look at the internal filter
:doc:`Tone <../corefilters/tone>` how this is done.

::

      bool __stdcall GetParity(int n) { return false; };

GetParity will return false.

::

      int __stdcall SetCacheHints(int cachehints, int frame_range) { return 0; };

SetCacheHints will return 0.

::

      const VideoInfo& __stdcall GetVideoInfo() { return vi; };

vi needs to be filled in the constructor.

::

    private:
      int w;
      int h;
      int color;
      VideoInfo vi;
      PVideoFrame frame;
    };

    GradientMask::GradientMask(int _w, int _h, int _color, IScriptEnvironment* env) :
                               w(_w), h(_h), color(_color) {
      memset(&vi, 0, sizeof(VideoInfo));
      vi.width = w;
      vi.height = h;
      vi.fps_numerator = 30000;
      vi.fps_denominator = 1001;
      vi.num_frames = 1;
      vi.pixel_type = VideoInfo::CS_BGR24;

First memory is allocated for VideoInfo and then the video parameters of
vi are filled. In our example, the width and height are taken from the
input parameters and the remaining ones are given default values. So an
one-frame RGB24 gradient will be returned with width 'w' and height 'h'.

In our example, the frame is filled here in the constructor since the
clip consists of only one frame. If it would have multiple different
frames you should fill them in GetFrame().

::

      frame = env->NewVideoFrame(vi);

      int y, i;
      BYTE* p;
      int fpitch, fheight, fwidth;
      float red, green, blue, luma;

      static const int col[] = {(color >> 16) & 0xff, (color >> 8) & 0xff, (color >> 0) & 0xff};
      luma = float((col[0]+col[1]+col[2])/3.0);

      blue = max(col[2] - luma,0);
      green = max(col[1] - luma,0);
      red = max(col[0] - luma,0);

First we calculate the luma (being the average of the color components
and subtract it from the color components. So we get the fully saturated
color.

::

      p = frame->GetWritePtr();
      fpitch = frame->GetPitch();
      fwidth = frame->GetRowSize(); // in bytes
      fheight = frame->GetHeight(); // in pixels

The gradients runs from the fully saturated color ((red,green,blue; for
i=0) to white ((255,255,255) for i=fwidht):

::

      for (y=0; y<fheight; y++) {
        for (i=0; i<fwidth; i+=3) {
          p[i] = int(min(i * 255.0/float(fwidth) + blue,255));
          p[i+1] = int(min(i * 255.0/float(fwidth) + green,255));
          p[i+2] = int(min(i * 255.0/float(fwidth) + red,255));
        }
        p += fpitch;
      }
    }

    PVideoFrame __stdcall GradientMask::GetFrame(int n, IScriptEnvironment* env) { return frame; }

Here our gradient frame is returned.

::

    static AVSValue __cdecl Create_GradientMask(AVSValue args, void* user_data, IScriptEnvironment* env) {
      return new GradientMask(args[0].AsInt(640), args[1].AsInt(480), args[2].AsInt(0), env);
    }

    const AVS_Linkage *AVS_linkage = 0;

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
      AVS_linkage = vectors;
      env->AddFunction("GradientMask", "[width]i[height]i[color]i", Create_GradientMask, 0);
      return "GradientMask sample plugin";
    }

--------------

| Back to :doc:`FilterSDK <FilterSDK>`

$Date: 2015/09/14 20:23:59 $
