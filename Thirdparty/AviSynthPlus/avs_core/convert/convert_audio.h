// Avisynth v2.5.  Copyright 2009 Ben Rudiak-Gould et al.
// http://www.avisynth.org

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#ifndef __Convert_Audio_H__
#define __Convert_Audio_H__

#include <avs/types.h>


#define CONVERT_DECLARE(func) void (func)(void *, void *, int);

typedef CONVERT_DECLARE(*convert_proc);

CONVERT_DECLARE(convert32To16);
CONVERT_DECLARE(convert16To32);
CONVERT_DECLARE(convert32To8);
CONVERT_DECLARE(convert8To32);
CONVERT_DECLARE(convert16To8);
CONVERT_DECLARE(convert8To16);
CONVERT_DECLARE(convert32To24);
CONVERT_DECLARE(convert24To32);
CONVERT_DECLARE(convert24To16);
CONVERT_DECLARE(convert16To24);
CONVERT_DECLARE(convert24To8);
CONVERT_DECLARE(convert8To24);
CONVERT_DECLARE(convert8ToFLT);
CONVERT_DECLARE(convertFLTTo8);
CONVERT_DECLARE(convert16ToFLT);
CONVERT_DECLARE(convertFLTTo16);
CONVERT_DECLARE(convert32ToFLT);
CONVERT_DECLARE(convertFLTTo32);

#ifdef INTEL_INTRINSICS
  CONVERT_DECLARE(convert32To16_SSE2);
  CONVERT_DECLARE(convert16To32_SSE2);
  CONVERT_DECLARE(convert32To8_SSE2);
  CONVERT_DECLARE(convert8To32_SSE2);
  CONVERT_DECLARE(convert16To8_SSE2);
  CONVERT_DECLARE(convert8To16_SSE2);
  CONVERT_DECLARE(convert32To24_SSSE3);
  CONVERT_DECLARE(convert24To32_SSSE3);
  CONVERT_DECLARE(convert24To16_SSSE3);
  CONVERT_DECLARE(convert16To24_SSSE3);
  CONVERT_DECLARE(convert24To8_SSSE3);
  CONVERT_DECLARE(convert8To24_SSSE3);
  CONVERT_DECLARE(convert8ToFLT_SSE41);
  CONVERT_DECLARE(convertFLTTo8_SSE2);
  CONVERT_DECLARE(convert16ToFLT_SSE41);
  CONVERT_DECLARE(convertFLTTo16_SSE2);
  CONVERT_DECLARE(convert32ToFLT_SSE2);
  CONVERT_DECLARE(convertFLTTo32_SSE41);

  CONVERT_DECLARE(convert32To16_AVX2);
  CONVERT_DECLARE(convert16To32_AVX2);
  CONVERT_DECLARE(convert8ToFLT_AVX2);
  CONVERT_DECLARE(convertFLTTo8_AVX2);
  CONVERT_DECLARE(convert16ToFLT_AVX2);
  CONVERT_DECLARE(convertFLTTo16_AVX2);
  CONVERT_DECLARE(convert32ToFLT_AVX2);
  CONVERT_DECLARE(convertFLTTo32_AVX2);
#endif

#undef CONVERT_DECLARE

class ConvertAudio : public GenericVideoFilter
/**
  * Helper class to convert audio to any format
 **/
{
public:
  ConvertAudio(PClip _clip, int prefered_format);
  void __stdcall GetAudio(void* buf, int64_t start, int64_t count, IScriptEnvironment* env);
  int __stdcall SetCacheHints(int cachehints,int frame_range);  // We do pass cache requests upwards, to the cache!

  static PClip Create(PClip clip, int sample_type, int prefered_type);
  static AVSValue __cdecl Create_float(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_32bit(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_24bit(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_16bit(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_8bit (AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_Any  (AVSValue args, void*, IScriptEnvironment*);
  virtual ~ConvertAudio();

private:
  int src_format;
  int dst_format;
  int src_bps;
  int tempbuffer_size {0};
  char *tempbuffer {nullptr};

  bool two_stage {false};
  convert_proc convert {nullptr};
  convert_proc convert_float {nullptr};
  convert_proc convert_c {nullptr};
#ifdef INTEL_INTRINSICS
  convert_proc convert_sse2 {nullptr};
  convert_proc convert_ssse3 {nullptr};
  convert_proc convert_sse41 { nullptr };
  convert_proc convert_avx2 {nullptr};
#endif

};

#endif //__Convert_Audio_H__
