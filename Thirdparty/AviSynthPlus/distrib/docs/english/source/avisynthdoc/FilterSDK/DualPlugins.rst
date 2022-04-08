
DualPlugins
===========

There are three ways to make dual plugins. That is 2.6 plugins
(compiled with plugin api v5) that also work with AviSynth 2.5. They
are described here below.

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents


The InvertNeg example
---------------------

For InvertNeg it's straightforward to make it as a dual plugin, since
the code uses the 2.5 feature set. When using the 2.5+ feature set it
becomes more elaborate (and it might not always be possible). See
Simplesample for such an example.


Brute force solution one
~~~~~~~~~~~~~~~~~~~~~~~~

This solution doubles all the code. You need to rename avisynth.h (v3)
to avisynth25.h and avisynth.h (v5) to avisynth26.h and add them both
to your project. You also need to add the following two source files to
your project:
::

   /* InvertNeg25.cpp */

   #include <windows.h>
   #include "avisynth25.h"

   class InvertNeg25 : public GenericVideoFilter {
   public:
      InvertNeg25(PClip _child, IScriptEnvironment* env);
      PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
   };

   InvertNeg25::InvertNeg25(PClip _child, IScriptEnvironment* env) :
      GenericVideoFilter(_child) {
      if (!vi.IsPlanar() || !vi.IsYUV()) {
         env->ThrowError("InvertNeg: planar YUV data only!");
      }
   }

   PVideoFrame __stdcall InvertNeg25::GetFrame(int n, IScriptEnvironment* env) {

      PVideoFrame src = child->GetFrame(n, env);
      PVideoFrame dst = env->NewVideoFrame(vi);

      const unsigned char* srcp;
      unsigned char* dstp;
      int src_pitch, dst_pitch, row_size, height;
      int p, x, y;

      int planes[] = {PLANAR_Y, PLANAR_V, PLANAR_U};

      for (p=0; p<3; p++) {
         srcp = src->GetReadPtr(planes[p]);
         dstp = dst->GetWritePtr(planes[p]);

         src_pitch = src->GetPitch(planes[p]);
         dst_pitch = dst->GetPitch(planes[p]);
         row_size = dst->GetRowSize(planes[p]);
         height = dst->GetHeight(planes[p]);

         for (y = 0; y < height; y++) {
            for (x = 0; x < row_size; x++) {
               dstp[x] = srcp[x] ^ 255;
            }
            srcp += src_pitch;
            dstp += dst_pitch;
         }
      }
      return dst;
   }

   AVSValue __cdecl Create_InvertNeg25(AVSValue args, void* user_data, IScriptEnvironment* env) {
      return new InvertNeg25(args[0].AsClip(), env);
   }

   extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
      env->AddFunction("InvertNeg", "c", Create_InvertNeg25, 0);
      return "InvertNeg sample plugin";
   }


::

   /* InvertNeg26.cpp */

   #include <windows.h>
   #include "avisynth26.h"

   class InvertNeg26 : public GenericVideoFilter {
   public:
      InvertNeg26(PClip _child, IScriptEnvironment* env);
      PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
   };

   InvertNeg26::InvertNeg26(PClip _child, IScriptEnvironment* env) :
      GenericVideoFilter(_child) {
      if (!vi.IsPlanar() || !vi.IsYUV()) {
         env->ThrowError("InvertNeg: planar YUV data only!");
      }
   }

   PVideoFrame __stdcall InvertNeg26::GetFrame(int n, IScriptEnvironment* env) {

      PVideoFrame src = child->GetFrame(n, env);
      PVideoFrame dst = env->NewVideoFrame(vi);

      const unsigned char* srcp;
      unsigned char* dstp;
      int src_pitch, dst_pitch, row_size, height;
      int p, x, y;

      int planes[] = {PLANAR_Y, PLANAR_V, PLANAR_U};

      for (p=0; p<3; p++) {
         srcp = src->GetReadPtr(planes[p]);
         dstp = dst->GetWritePtr(planes[p]);

         src_pitch = src->GetPitch(planes[p]);
         dst_pitch = dst->GetPitch(planes[p]);
         row_size = dst->GetRowSize(planes[p]);
         height = dst->GetHeight(planes[p]);

         for (y = 0; y < height; y++) {
            for (x = 0; x < row_size; x++) {
               dstp[x] = srcp[x] ^ 255;
            }
            srcp += src_pitch;
            dstp += dst_pitch;
         }
      }
      return dst;
   }

   AVSValue __cdecl Create_InvertNeg26(AVSValue args, void* user_data, IScriptEnvironment* env) {
      return new InvertNeg26(args[0].AsClip(), env);
   }

   const AVS_Linkage *AVS_linkage = 0;

   extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
      AVS_linkage = vectors;
      env->AddFunction("InvertNeg", "c", Create_InvertNeg26, 0);
      return "InvertNeg sample plugin";
   }


Note that both functions Create_InvertNeg25 and Create_InvertNeg26 are
added in the function table (by AddFunction) as InvertNeg. Which one is
called by your script will be decided by the AvisynthPluginInit2 and
AvisynthPluginInit3 entry points (and thus your AviSynth version).

Compile both files into a DLL named InvertNeg.dll. See
:doc:`compiling instructions <CompilingAvisynthPlugins>`. Now create an
AviSynth script which looks something like this:
::

    LoadPlugin("d:\path\InvertNeg.dll")
    clip = BlankClip().ConvertToYV12()
    return clip.InvertNeg()


The script should work both for AviSynth 2.5 and 2.6.


Brute force solution two
~~~~~~~~~~~~~~~~~~~~~~~~

To stop having to double your source code you could use the namespace
trick the 2.0 import wrapper uses.

Place all the filter generic code into a separate file,
"InvertNeg.hpp", and #include it twice. Technically you have two copies
of everything binary, but only a single source code. Note: outside the
namespace you uniquely access object names as avs25::name or
avs26::name.
::

   /* InvertNeg.cpp */

   #include <windows.h>

   namespace avs25 {
      #include "avisynth25.h"
      #include "InvertNeg.hpp"
   }

   #undef __AVISYNTH_H__ // name must not be included twice in the inclusions

   namespace avs26 {
      #include "avisynth26.h"
      const AVS_Linkage *AVS_linkage = 0;
      #include "InvertNeg.hpp"
   }

   extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(avs25::IScriptEnvironment* env) {
      env->AddFunction("InvertNeg", "c", avs25::Create_InvertNeg, 0);
      return "InvertNeg sample plugin";
   }

   extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(avs26::IScriptEnvironment* env, const avs26::AVS_Linkage* const vectors) {
      avs26::AVS_linkage = vectors;
      env->AddFunction("InvertNeg", "c", avs26::Create_InvertNeg, 0);
      return "InvertNeg sample plugin";
   }


::

   /* InvertNeg.hpp */

   class InvertNeg : public GenericVideoFilter {
   public:
      InvertNeg(PClip _child, IScriptEnvironment* env);
      PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
   };

   InvertNeg::InvertNeg(PClip _child, IScriptEnvironment* env) :
      GenericVideoFilter(_child) {
      if (!vi.IsPlanar() || !vi.IsYUV()) {
         env->ThrowError("InvertNeg: planar YUV data only!");
      }
   }

   PVideoFrame __stdcall InvertNeg::GetFrame(int n, IScriptEnvironment* env) {

      PVideoFrame src = child->GetFrame(n, env);
      PVideoFrame dst = env->NewVideoFrame(vi);

      const unsigned char* srcp;
      unsigned char* dstp;
      int src_pitch, dst_pitch, row_size, height;
      int p, x, y;

      int planes[] = {PLANAR_Y, PLANAR_V, PLANAR_U};

      for (p=0; p<3; p++) {
         srcp = src->GetReadPtr(planes[p]);
         dstp = dst->GetWritePtr(planes[p]);

         src_pitch = src->GetPitch(planes[p]);
         dst_pitch = dst->GetPitch(planes[p]);
         row_size = dst->GetRowSize(planes[p]);
         height = dst->GetHeight(planes[p]);

         for (y = 0; y < height; y++) {
            for (x = 0; x < row_size; x++) {
               dstp[x] = srcp[x] ^ 255;
            }
            srcp += src_pitch;
            dstp += dst_pitch;
         }
      }
      return dst;
   }

   AVSValue __cdecl Create_InvertNeg(AVSValue args, void* user_data, IScriptEnvironment* env) {
      return new InvertNeg(args[0].AsClip(), env);
   }


Compile this file into a DLL named InvertNeg.dll. See
:doc:`compiling instructions <CompilingAvisynthPlugins>`. Now create an AviSynth script which
looks something like this:
::

   LoadPlugin("d:\path\InvertNeg.dll")
   clip = BlankClip().ConvertToYV12()
   return clip.InvertNeg()


xxx
~~~

http://forum.doom9.org/showthread.php?p=1641346#post1641346 and
subsequent posts ....


The SimpleSample example
------------------------

The SimpleSample code uses the 2.5+ feature set, since it uses
GetPlaneWidthSubsampling and GetPlaneHeightSubsampling. These are
functions of :doc:`VideoInfo`. One way to get it to work is to add these
functions in avisynth25.h in the :doc:`VideoInfo` structure. Note that in
this case they are only used for YV12:
::

   /* avisynth25.h */

   struct VideoInfo {
      ...
      // mimics 2.6 functionality for YV12
      int GetPlaneWidthSubsampling(int plane) const {
         switch(plane) {
         case PLANAR_Y:
            return 0;
         case PLANAR_U:
            return 1;
         case PLANAR_V:
            return 1;
         default:
            return 0;
         }
      }

      int GetPlaneHeightSubsampling(int plane) const {
         switch(plane) {
         case PLANAR_Y:
            return 0;
         case PLANAR_U:
            return 1;
         case PLANAR_V:
            return 1;
         default:
            return 0;
         }
      }
   };


Now we can place all the filter generic code into a separate file,
"simplesample.hpp", #include it twice, and access object names as
avs25::name or avs26::name.
::

    /* simplesample.cpp */

    /*
        SimpleSample plugin for Avisynth -- a simple sample
        Copyright (C) 2002-2013 Simon Walters, Wilbert Dijkhof, All Rights Reserved

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

        The author can be contacted at:
        wilbertdijkhof at yahoo.com

        v2.0 - AviSynth 2.6 api (based on v1.6 of Simon Walters)
        v2.1 - Dual plugin for 2.5 and 2.6
    */

    #include <windows.h>

    namespace avs25 {
        #include "avisynth25.h"
        #include "simplesample.hpp"
    }

    #undef __AVISYNTH_H__ // name must not be included twice in the inclusions

    namespace avs26 {
        #include "avisynth26.h"
        const AVS_Linkage *AVS_linkage = 0;
        #include "simplesample.hpp"
    }

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(avs25::IScriptEnvironment* env) {
        env->AddFunction("SimpleSample", "c[size]i", avs25::Create_SimpleSample, 0);
        return "SimpleSample plugin";
    }

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(avs26::IScriptEnvironment* env, const avs26::AVS_Linkage* const vectors) {
        avs26::AVS_linkage = vectors;
        env->AddFunction("SimpleSample", "c[size]i", avs26::Create_SimpleSample, 0);
        return "SimpleSample plugin";
    }


::

    /* simplesample.hpp */

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


Compile this file into a DLL named SimpleSample.dll. See
:doc:`compiling instructions <CompilingAvisynthPlugins>`. Now create an Avisynth script which
looks something like this:
::

    LoadPlugin("d:\path\simplesample.dll")
    Colorbars().Trim(0,1)
    ConvertToYV12()
    # ConvertToYV411() # requires AviSynth 2.6
    SimpleSample(100)


____

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $
