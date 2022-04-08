// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
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

// Overlay (c) 2003, 2004 by Klaus Post

#include "444convert.h"
#include "../../core/internal.h"
#include <avs/alignment.h>

// fast in-place conversions from and to 4:4:4

/***** YV12 -> YUV 4:4:4   ******/

template<typename pixel_t>
static void convert_yv12_chroma_to_yv24_c(BYTE *dstp8, const BYTE *srcp8, int dst_pitch, int src_pitch, int src_width, int src_height) {
  pixel_t *dstp = reinterpret_cast<pixel_t *>(dstp8);
  const pixel_t *srcp = reinterpret_cast<const pixel_t *>(srcp8);
  dst_pitch /= sizeof(pixel_t);
  src_pitch /= sizeof(pixel_t);
  for (int y = 0; y < src_height; ++y) {
    for (int x = 0; x < src_width; ++x) {
      dstp[x*2]             = srcp[x];
      dstp[x*2+1]           = srcp[x];
      dstp[x*2+dst_pitch]   = srcp[x];
      dstp[x*2+dst_pitch+1] = srcp[x];
    }
    dstp += dst_pitch*2;
    srcp += src_pitch;
  }
}

void Convert444FromYV12(PVideoFrame &src, PVideoFrame &dst, int pixelsize, int bits_per_pixel, IScriptEnvironment* env)
{
  AVS_UNUSED(bits_per_pixel);
  env->BitBlt(dst->GetWritePtr(PLANAR_Y), dst->GetPitch(PLANAR_Y), src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y), src->GetRowSize(PLANAR_Y), src->GetHeight());

  const BYTE* srcU = src->GetReadPtr(PLANAR_U);
  const BYTE* srcV = src->GetReadPtr(PLANAR_V);

  int srcUVpitch = src->GetPitch(PLANAR_U);

  BYTE* dstU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstV = dst->GetWritePtr(PLANAR_V);

  int dstUVpitch = dst->GetPitch(PLANAR_U);

  int width = src->GetRowSize(PLANAR_U) / pixelsize;
  int height = src->GetHeight(PLANAR_U);

      if (pixelsize == 1) {
        convert_yv12_chroma_to_yv24_c<uint8_t>(dstU, srcU, dstUVpitch, srcUVpitch, width, height);
        convert_yv12_chroma_to_yv24_c<uint8_t>(dstV, srcV, dstUVpitch, srcUVpitch, width, height);
      } else if(pixelsize == 2) {
        convert_yv12_chroma_to_yv24_c<uint16_t>(dstU, srcU, dstUVpitch, srcUVpitch, width, height);
        convert_yv12_chroma_to_yv24_c<uint16_t>(dstV, srcV, dstUVpitch, srcUVpitch, width, height);
      }
      else {
        convert_yv12_chroma_to_yv24_c<float>(dstU, srcU, dstUVpitch, srcUVpitch, width, height);
        convert_yv12_chroma_to_yv24_c<float>(dstV, srcV, dstUVpitch, srcUVpitch, width, height);
      }

  env->BitBlt(dst->GetWritePtr(PLANAR_A), dst->GetPitch(PLANAR_A),
    src->GetReadPtr(PLANAR_A), src->GetPitch(PLANAR_A), dst->GetRowSize(PLANAR_A), dst->GetHeight(PLANAR_A));


}

/***** YV16 -> YUV 4:4:4   ******/

template<typename pixel_t>
static void convert_yv16_chroma_to_yv24_c(BYTE *dstp8, const BYTE *srcp8, int dst_pitch, int src_pitch, int src_width, int src_height) {
  pixel_t *dstp = reinterpret_cast<pixel_t *>(dstp8);
  const pixel_t *srcp = reinterpret_cast<const pixel_t *>(srcp8);
  dst_pitch /= sizeof(pixel_t);
  src_pitch /= sizeof(pixel_t);
  for (int y = 0; y < src_height; ++y) {
    for (int x = 0; x < src_width; ++x) {
      dstp[x*2]             = srcp[x];
      dstp[x*2+1]           = srcp[x];
    }
    dstp += dst_pitch;
    srcp += src_pitch;
  }
}

void Convert444FromYV16(PVideoFrame &src, PVideoFrame &dst, int pixelsize, int bits_per_pixel, IScriptEnvironment* env)
{
  AVS_UNUSED(bits_per_pixel);
  env->BitBlt(dst->GetWritePtr(PLANAR_Y), dst->GetPitch(PLANAR_Y), src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y), src->GetRowSize(PLANAR_Y), src->GetHeight());

  const BYTE* srcU = src->GetReadPtr(PLANAR_U);
  const BYTE* srcV = src->GetReadPtr(PLANAR_V);

  int srcUVpitch = src->GetPitch(PLANAR_U);

  BYTE* dstU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstV = dst->GetWritePtr(PLANAR_V);

  int dstUVpitch = dst->GetPitch(PLANAR_U);

  int width = src->GetRowSize(PLANAR_U) / pixelsize;
  int height = src->GetHeight(PLANAR_U);

      if (pixelsize == 1) {
        convert_yv16_chroma_to_yv24_c<uint8_t>(dstU, srcU, dstUVpitch, srcUVpitch, width, height);
        convert_yv16_chroma_to_yv24_c<uint8_t>(dstV, srcV, dstUVpitch, srcUVpitch, width, height);
      } else if(pixelsize == 2) {
        convert_yv16_chroma_to_yv24_c<uint16_t>(dstU, srcU, dstUVpitch, srcUVpitch, width, height);
        convert_yv16_chroma_to_yv24_c<uint16_t>(dstV, srcV, dstUVpitch, srcUVpitch, width, height);
      }
      else {
        convert_yv16_chroma_to_yv24_c<float>(dstU, srcU, dstUVpitch, srcUVpitch, width, height);
        convert_yv16_chroma_to_yv24_c<float>(dstV, srcV, dstUVpitch, srcUVpitch, width, height);
      }

  env->BitBlt(dst->GetWritePtr(PLANAR_A), dst->GetPitch(PLANAR_A),
    src->GetReadPtr(PLANAR_A), src->GetPitch(PLANAR_A), dst->GetRowSize(PLANAR_A), dst->GetHeight(PLANAR_A));

}

/***** YUY2 -> YUV 4:4:4   ******/

void Convert444FromYUY2(PVideoFrame &src, PVideoFrame &dst, int pixelsize, int bits_per_pixel, IScriptEnvironment* env) {
  AVS_UNUSED(pixelsize);
  AVS_UNUSED(bits_per_pixel);
  AVS_UNUSED(env);

  const BYTE* srcP = src->GetReadPtr();
  int srcPitch = src->GetPitch();

  BYTE* dstY = dst->GetWritePtr(PLANAR_Y);
  BYTE* dstU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstV = dst->GetWritePtr(PLANAR_V);

  int dstPitch = dst->GetPitch();

  int w = src->GetRowSize() / 2;
  int h = src->GetHeight();

  for (int y=0; y<h; y++) {
    for (int x=0; x<w; x+=2) {
      int x2 = x<<1;
      dstY[x]   = srcP[x2];
      dstU[x]   = dstU[x+1] = srcP[x2+1];
      dstV[x]   = dstV[x+1] = srcP[x2+3];
      dstY[x+1] = srcP[x2+2];
    }
    srcP+=srcPitch;

    dstY+=dstPitch;
    dstU+=dstPitch;
    dstV+=dstPitch;
  }
}

template<typename pixel_t>
static void convert_yv24_chroma_to_yv12_c(BYTE *dstp8, const BYTE *srcp8, int dst_pitch, int src_pitch, int dst_width, const int dst_height) {
  const pixel_t *srcp = reinterpret_cast<const pixel_t *>(srcp8);
  pixel_t *dstp = reinterpret_cast<pixel_t *>(dstp8);
  dst_pitch /= sizeof(pixel_t);
  src_pitch /= sizeof(pixel_t);
  for (int y = 0; y < dst_height; y++) {
    for (int x = 0; x < dst_width; x++) {
      if constexpr (sizeof(pixel_t) == 4)
        dstp[x] = (srcp[x * 2] + srcp[x * 2 + 1] + srcp[x * 2 + src_pitch] + srcp[x * 2 + src_pitch + 1]) * 0.25f; // /4
      else
        dstp[x] = (srcp[x * 2] + srcp[x * 2 + 1] + srcp[x * 2 + src_pitch] + srcp[x * 2 + src_pitch + 1] + 2) >> 2;
    }
    srcp += src_pitch * 2;
    dstp += dst_pitch;
  }
}

template<typename pixel_t>
static void convert_yv24_chroma_to_yv16_c(BYTE *dstp8, const BYTE *srcp8, int dst_pitch, int src_pitch, int dst_width, const int dst_height) {
  const pixel_t *srcp = reinterpret_cast<const pixel_t *>(srcp8);
  pixel_t *dstp = reinterpret_cast<pixel_t *>(dstp8);
  dst_pitch /= sizeof(pixel_t);
  src_pitch /= sizeof(pixel_t);
  for (int y=0; y < dst_height; y++) {
    for (int x=0; x < dst_width; x++) {
      if constexpr (sizeof(pixel_t) == 4)
        dstp[x] = (srcp[x * 2] + srcp[x * 2 + 1]) * 0.5f;
      else
        dstp[x] = (srcp[x * 2] + srcp[x * 2 + 1] + 1) >> 1;
    }
    srcp+=src_pitch;
    dstp+=dst_pitch;
  }
}

void ConvertYToYV12Chroma(BYTE *dst, BYTE *src, int dstpitch, int srcpitch, int pixelsize, int w, int h, IScriptEnvironment* env)
{
    if(pixelsize==1)
      convert_yv24_chroma_to_yv12_c<uint8_t>(dst, src, dstpitch, srcpitch, w, h);
    else if (pixelsize == 2)
      convert_yv24_chroma_to_yv12_c<uint16_t>(dst, src, dstpitch, srcpitch, w, h);
    else // if (pixelsize == 4)
      convert_yv24_chroma_to_yv12_c<float>(dst, src, dstpitch, srcpitch, w, h);
}

void ConvertYToYV16Chroma(BYTE *dst, BYTE *src, int dstpitch, int srcpitch, int pixelsize, int w, int h, IScriptEnvironment* env)
{
    if(pixelsize==1)
      convert_yv24_chroma_to_yv16_c<uint8_t>(dst, src, dstpitch, srcpitch, w, h);
    else if (pixelsize == 2)
      convert_yv24_chroma_to_yv16_c<uint16_t>(dst, src, dstpitch, srcpitch, w, h);
    else // if (pixelsize == 4)
      convert_yv24_chroma_to_yv16_c<float>(dst, src, dstpitch, srcpitch, w, h);
}

void Convert444ToYV16(PVideoFrame &src, PVideoFrame &dst, int pixelsize, int bits_per_pixel, IScriptEnvironment* env)
{
  AVS_UNUSED(bits_per_pixel);
  env->BitBlt(dst->GetWritePtr(PLANAR_Y), dst->GetPitch(PLANAR_Y),
    src->GetReadPtr(PLANAR_Y), src->GetPitch(), dst->GetRowSize(PLANAR_Y), dst->GetHeight());

  const BYTE* srcU = src->GetReadPtr(PLANAR_U);
  const BYTE* srcV = src->GetReadPtr(PLANAR_V);

  int srcUVpitch = src->GetPitch(PLANAR_U);

  BYTE* dstU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstV = dst->GetWritePtr(PLANAR_V);

  int dstUVpitch = dst->GetPitch(PLANAR_U);

  int w = dst->GetRowSize(PLANAR_U);
  int h = dst->GetHeight(PLANAR_U);

      if(pixelsize==1) {
        convert_yv24_chroma_to_yv16_c<uint8_t>(dstU, srcU, dstUVpitch, srcUVpitch, w, h);
        convert_yv24_chroma_to_yv16_c<uint8_t>(dstV, srcV, dstUVpitch, srcUVpitch, w, h);
      }
      else if (pixelsize == 2) {
        convert_yv24_chroma_to_yv16_c<uint16_t>(dstU, srcU, dstUVpitch, srcUVpitch, w, h);
        convert_yv24_chroma_to_yv16_c<uint16_t>(dstV, srcV, dstUVpitch, srcUVpitch, w, h);
      }
      else { // if (pixelsize == 4)
        convert_yv24_chroma_to_yv16_c<float>(dstU, srcU, dstUVpitch, srcUVpitch, w, h);
        convert_yv24_chroma_to_yv16_c<float>(dstV, srcV, dstUVpitch, srcUVpitch, w, h);
      }

  env->BitBlt(dst->GetWritePtr(PLANAR_A), dst->GetPitch(PLANAR_A),
    src->GetReadPtr(PLANAR_A), src->GetPitch(PLANAR_A), dst->GetRowSize(PLANAR_A), dst->GetHeight(PLANAR_A));
}


void Convert444ToYV12(PVideoFrame &src, PVideoFrame &dst, int pixelsize, int bits_per_pixel, IScriptEnvironment* env)
{
  AVS_UNUSED(bits_per_pixel);
  env->BitBlt(dst->GetWritePtr(PLANAR_Y), dst->GetPitch(PLANAR_Y),
    src->GetReadPtr(PLANAR_Y), src->GetPitch(), dst->GetRowSize(PLANAR_Y), dst->GetHeight());

  const BYTE* srcU = src->GetReadPtr(PLANAR_U);
  const BYTE* srcV = src->GetReadPtr(PLANAR_V);

  int srcUVpitch = src->GetPitch(PLANAR_U);

  BYTE* dstU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstV = dst->GetWritePtr(PLANAR_V);

  int dstUVpitch = dst->GetPitch(PLANAR_U);

  int w = dst->GetRowSize(PLANAR_U);
  int h = dst->GetHeight(PLANAR_U);

  if(pixelsize==1) {
    convert_yv24_chroma_to_yv12_c<uint8_t>(dstU, srcU, dstUVpitch, srcUVpitch, w, h);
    convert_yv24_chroma_to_yv12_c<uint8_t>(dstV, srcV, dstUVpitch, srcUVpitch, w, h);
  }
  else if (pixelsize == 2) {
    convert_yv24_chroma_to_yv12_c<uint16_t>(dstU, srcU, dstUVpitch, srcUVpitch, w, h);
    convert_yv24_chroma_to_yv12_c<uint16_t>(dstV, srcV, dstUVpitch, srcUVpitch, w, h);
  }
  else { // if (pixelsize == 4)
    convert_yv24_chroma_to_yv12_c<float>(dstU, srcU, dstUVpitch, srcUVpitch, w, h);
    convert_yv24_chroma_to_yv12_c<float>(dstV, srcV, dstUVpitch, srcUVpitch, w, h);
  }

  env->BitBlt(dst->GetWritePtr(PLANAR_A), dst->GetPitch(PLANAR_A),
    src->GetReadPtr(PLANAR_A), src->GetPitch(PLANAR_A), dst->GetRowSize(PLANAR_A), dst->GetHeight(PLANAR_A));

}

/*****   YUV 4:4:4 -> YUY2   *******/

void Convert444ToYUY2(PVideoFrame &src, PVideoFrame &dst, int pixelsize, int bits_per_pixel, IScriptEnvironment* env) {
  AVS_UNUSED(bits_per_pixel);
  AVS_UNUSED(env);

  const BYTE* srcY = src->GetReadPtr(PLANAR_Y);
  const BYTE* srcU = src->GetReadPtr(PLANAR_U);
  const BYTE* srcV = src->GetReadPtr(PLANAR_V);

  int srcPitch = src->GetPitch();

  BYTE* dstP = dst->GetWritePtr();

  int dstPitch = dst->GetPitch();

  int w = src->GetRowSize() / pixelsize;
  int h = src->GetHeight();

  for (int y=0; y<h; y++) {
    for (int x=0; x<w; x+=2) {
      int x2 = x<<1;
      dstP[x2]   = srcY[x];
      dstP[x2+1] = (srcU[x] + srcU[x+1] + 1)>>1;
      dstP[x2+2] = srcY[x+1];
      dstP[x2+3] = (srcV[x] + srcV[x+1] + 1)>>1;
    }
    srcY+=srcPitch;
    srcU+=srcPitch;
    srcV+=srcPitch;
    dstP+=dstPitch;
  }
}
