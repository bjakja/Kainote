// Avisynth v2.5.
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

#include <avisynth.h>
#include <avs/win.h>
#include <avs/minmax.h>
#include <cstdio>
#include <new>
#include <stddef.h>
#include <vector>

/********************************************************************
* VirtualDub plugin support
********************************************************************/
// Update by pinterf, headers from VirtualDub2 project, April 2018
// - Update from V6 interface to V20, Filtermod version 6
// - VirtualDub2 support with extended colorspaces:
// - Allow RGB24, RGB48, RGB64 besides RGB32
// - AutoConvert 8 bit Planar RGB to/from RGB24, RGBPA to/from RGB32 (lossless)
// - AutoConvert RGB48 and 16 bit Planar RGB(A) to/from RGB64 (lossless)
// - Support YUV(A) 8 bits: YV12, YV16, YV24, YV411, YUVA420P8, YUVA422P8, YUVA444P8
// - Support YUV(A) 10-16 bits (properly set "ref_x" maximum levels, no autoconvert)
// - Supports prefetchProc2 callback (API >= V14 and prefetchProc2 is defined) for multiple input frames from one input clip
//   PrefetchFrameDirect and PrefetchFrame are supported. PrefetchFrameSymbolic not supported
// - Supports prefetchProc callback (API >= V12 and prefetchProc is defined)
// - Supports when filter changes frame count of the output clip
// - Range hint
//   Imported Virtualdub filters are getting and extra named parameter to the end:
//      String [rangehint]
//   This parameter can tell the filter about a YUV-type clip colorspace info
//   Allowed values:
//     "rec601": limited range + 601
//     "rec709": limited range + 709
//     "PC.601": full range + 601
//     "PC.709": full range + 709
//     ""      : not defined (same as not given)
//   Parameter will be ignored when clip is non-YUV
//   How it works: the hint will _not_ change the internal VirtualDub colorspace
//   constant (e.g. kPixFormat_YUV420_Planar -> kPixFormat_YUV420_Planar_709) but keeps
//   the basic color space and sets colorSpaceMode and colorRangeMode in PixmapLayout.formatEx.
//   Filter can either use this information or not, depending on supported API version
//   and its implementation.
//   E.g. Crossfade(20,30) -> Crossfade(20,30,"rec601") though this specific filter won't use it.


//  VirtualDub - Video processing and capture application
//  Copyright (C) 1998-2009 Avery Lee
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "vdvideofilt.h"
#include "vdplugin.h"
#include "ScriptValue.h"
#include "ScriptInterpreter.h"
#include "VBitmap.h"

int GetCPUFlags(); // to have a prototype

#define VDcall __cdecl

//////////////////// from plugins.h ////////////////////

// we need only VDXFilterModule in Avisynth
struct VDXFilterModule {		// formerly FilterModule
  struct VDXFilterModule *next, *prev;
  VDXHINSTANCE			hInstModule;
  VDXFilterModuleInitProc	initProc;
  FilterModModuleInitProc	filterModInitProc;
  VDXFilterModuleDeinitProc	deinitProc;
};

//////////////////// from videofilt.h ////////////////////

struct VDXFilterDefinition2 : public VDXFilterDefinition {
  FilterModDefinition filterMod;

  VDXFilterDefinition2() {
    fm = 0;
  }

  VDXFilterDefinition2(const VDXFilterDefinition2& a)
    :VDXFilterDefinition(a)
  {
    filterMod = a.filterMod;
    fm = &filterMod;
  }
};

//////////////////// helper for parameter passing ////////////////////

class CScriptValueStringHelper {
public:
  void *lpVoid; // avs+: char ** helper, introduced for x64, the CScriptValue union is already 8 bytes
};

//////////////////// vbitmap things ////////////////////

VBitmap& VBitmap::init(void *lpData, BITMAPINFOHEADER *bmih) {
  data      = (Pixel *)lpData;
  palette     = (Pixel *)(bmih+1);
  depth     = bmih->biBitCount;
  w       = bmih->biWidth;
  h       = bmih->biHeight;
  offset      = 0;
  AlignTo4();

  return *this;
}

VBitmap& VBitmap::init(void *data, PixDim w, PixDim h, int depth) {
  this->data    = (Pixel32 *)data;
  this->palette = NULL;
  this->depth   = depth;
  this->w     = w;
  this->h     = h;
  this->offset  = 0;
  AlignTo8();

  return *this;
}

void VBitmap::MakeBitmapHeader(BITMAPINFOHEADER *bih) const {
  bih->biSize       = sizeof(BITMAPINFOHEADER);
  bih->biBitCount   = (WORD)depth;
  bih->biPlanes     = 1;
  bih->biCompression    = BI_RGB;

  if (pitch == ((w*bih->biBitCount + 31)/32) * 4)
    bih->biWidth    = w;
  else
    bih->biWidth    = LONG(pitch*8 / depth);

  bih->biHeight     = h;
  bih->biSizeImage    = DWORD(pitch*h);
  bih->biClrUsed      = 0;
  bih->biClrImportant   = 0;
  bih->biXPelsPerMeter  = 0;
  bih->biYPelsPerMeter  = 0;
}

void VBitmap::AlignTo4() {
  pitch   = PitchAlign4();
  modulo    = Modulo();
  size    = Size();
}

void VBitmap::AlignTo8() {
  pitch   = PitchAlign8();
  modulo    = Modulo();
  size    = Size();
}

// Avisynth doesn't support the following functions.

void VBitmap::BitBlt(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBlt");
}

void VBitmap::BitBltDither(PixCoord x2, PixCoord y2, const VBitmap *src, PixDim x1, PixDim y1, PixDim dx, PixDim dy, bool to565) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBltDither");
}

void VBitmap::BitBlt565(PixCoord x2, PixCoord y2, const VBitmap *src, PixDim x1, PixDim y1, PixDim dx, PixDim dy) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBlt565");
}

bool VBitmap::BitBltXlat1(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const Pixel8 *tbl) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBltXlat1");
}

bool VBitmap::BitBltXlat3(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const Pixel32 *tbl) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBltXlat3");
}

bool VBitmap::StretchBltNearestFast(PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const VBitmap *src, double x2, double y2, double dx1, double dy1) const
{
  throw AvisynthError("Unsupported VBitmap method: StretchBltNearestFast");
}

bool VBitmap::StretchBltBilinearFast(PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const VBitmap *src, double x2, double y2, double dx1, double dy1) const
{
  throw AvisynthError("Unsupported VBitmap method: StretchBltBilinearFast");
}

bool VBitmap::RectFill(PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, Pixel32 c) const
{
  throw AvisynthError("Unsupported VBitmap method: RectFill");
}

bool VBitmap::Histogram(PixCoord x, PixCoord y, PixCoord dx, PixCoord dy, long *pHisto, int iHistoType) const
{
  throw AvisynthError("Unsupported VBitmap method: Histogram");
}

bool VBitmap::BitBltFromYUY2(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBltFromYUY2");
}

bool VBitmap::BitBltFromI420(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBltFromI420");
}

//// NEW AS OF VIRTUALDUB V1.4C

void VBitmap::MakeBitmapHeaderNoPadding(BITMAPINFOHEADER *bih) const
{
  throw AvisynthError("Unsupported VBitmap method: MakeBitmapHeaderNoPadding");
}

bool VBitmap::BitBltFromYUY2Fullscale(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const
{
  throw AvisynthError("Unsupported VBitmap method: BitBltFromYUY2Fullscale");
}

// end of old stuff
///////////

class DummyFilterPreview : public IVDXFilterPreview {
  void Die() { throw AvisynthError("VirtualDubFilterProxy: IFilterPreview not supported"); }
public:
  virtual void VDcall SetButtonCallback(VDXFilterPreviewButtonCallback, void *) { Die(); }
  virtual void VDcall SetSampleCallback(VDXFilterPreviewSampleCallback, void *) { Die(); }
  virtual bool VDcall isPreviewEnabled() { return false; }
  virtual void VDcall Toggle(VDXHWND) {}
  virtual void VDcall Display(VDXHWND, bool) {}
  virtual void VDcall RedoFrame() {}
  virtual void VDcall RedoSystem() {}
  virtual void VDcall UndoSystem() {}
  virtual void VDcall InitButton(VDXHWND) {}
  virtual void VDcall Close() {}
  virtual bool VDcall SampleCurrentFrame() { return false; }
  virtual long VDcall SampleFrames() { return 0; }
};

class DummyFilterPreview2 : public IVDXFilterPreview2 {
  void Die() { throw AvisynthError("VirtualDubFilterProxy: IFilterPreview2 not supported"); }
public:
  virtual void VDcall SetButtonCallback(VDXFilterPreviewButtonCallback, void*) { Die(); }
  virtual void VDcall SetSampleCallback(VDXFilterPreviewSampleCallback, void*) { Die(); }

  virtual bool VDcall isPreviewEnabled() { return false; }
  virtual void VDcall Toggle(VDXHWND) {}
  virtual void VDcall Display(VDXHWND, bool) {}
  virtual void VDcall RedoFrame() {}
  virtual void VDcall RedoSystem() {}
  virtual void VDcall UndoSystem() {}
  virtual void VDcall InitButton(VDXHWND) {}
  virtual void VDcall Close() {}
  virtual bool VDcall SampleCurrentFrame() { return false; }
  virtual long VDcall SampleFrames() { return 0; }

  virtual bool VDcall IsPreviewDisplayed() { return false; };
};

////////////////////////////////////////////////////////////

class FilterDefinitionList;

typedef struct MyVDXFilterModule : VDXFilterModule {
  IScriptEnvironment*    env;
  const char*            avisynth_function_name;
  int                    preroll;

  int filterdef_ver_lo, filterdef_ver_hi; // compatible and actual filterdef version
  int filtermod_ver_lo, filtermod_ver_hi; // compatible and actual filtermod version

  int refcounter;

  FilterDefinitionList*  fdl;
} MyVDXFilterModule;

class FilterDefinitionList {
public:
  MyVDXFilterModule * myFilterModule;
  VDXFilterDefinition2* fd;
  FilterDefinitionList* fdl;

  FilterDefinitionList(MyVDXFilterModule* _myFilterModule, VDXFilterDefinition2* _fd) : myFilterModule(_myFilterModule), fd(_fd), fdl(myFilterModule->fdl) { };
};

////////////////// from pixmap.h //////////////////////////
// to have a VDPixmap/VDPixmapLayout with 4th plane (alpha)

typedef sint32		vdpixpos;
typedef sint32		vdpixsize;
typedef ptrdiff_t	vdpixoffset;

struct VDPixmap {
  void			*data;
  const uint32	*palette;
  vdpixsize		w;
  vdpixsize		h;
  vdpixoffset		pitch;
  sint32			format;

  // Auxiliary planes are always byte-per-pixel.

  void			*data2;		// Cb (U) for YCbCr
  vdpixoffset		pitch2;
  void			*data3;		// Cr (V) for YCbCr
  vdpixoffset		pitch3;
  void			*data4;		// Alpha
  vdpixoffset		pitch4;

  FilterModPixmapInfo info;

  void clear() {
    data = 0;
    palette = 0;
    w = 0;
    h = 0;
    pitch = 0;
    format = 0;
    data2 = 0;
    pitch2 = 0;
    data3 = 0;
    pitch3 = 0;
    data4 = 0;
    pitch4 = 0;
    info.clear();
  }

  static VDPixmap copy(const VDXPixmap& a);
};

// from pixmaputils.cpp
// avs+ uses nsVDXPixmap here instead of nsVDPixmap
bool VDPixmapFormatHasAlphaPlane(sint32 format) {
  switch (format) {
  case nsVDXPixmap::kPixFormat_YUV444_Alpha_Planar:
  case nsVDXPixmap::kPixFormat_YUV422_Alpha_Planar:
  case nsVDXPixmap::kPixFormat_YUV420_Alpha_Planar:
  case nsVDXPixmap::kPixFormat_YUV444_Alpha_Planar16:
  case nsVDXPixmap::kPixFormat_YUV422_Alpha_Planar16:
  case nsVDXPixmap::kPixFormat_YUV420_Alpha_Planar16:
    return true;
  }
  return false;
}

VDPixmap VDPixmap::copy(const VDXPixmap& a) {
  VDPixmap b;
  b.data = a.data;
  b.palette = a.palette;
  b.w = a.w;
  b.h = a.h;
  b.pitch = a.pitch;
  b.format = a.format;
  b.data2 = a.data2;
  b.pitch2 = a.pitch2;
  b.data3 = a.data3;
  b.pitch3 = a.pitch3;

  if (VDPixmapFormatHasAlphaPlane(a.format)) {
    const VDXPixmapAlpha& aa = (const VDXPixmapAlpha&)a;
    b.data4 = aa.data4;
    b.pitch4 = aa.pitch4;
  }

  return b;
}

struct VDPixmapFormatEx {
  sint32			format;
  nsVDXPixmap::ColorSpaceMode colorSpaceMode;
  nsVDXPixmap::ColorRangeMode colorRangeMode;

  operator int() const { return format; }

  VDPixmapFormatEx() {
    format = 0;
    colorSpaceMode = nsVDXPixmap::kColorSpaceMode_None;
    colorRangeMode = nsVDXPixmap::kColorRangeMode_None;
  }
  VDPixmapFormatEx(sint32 v) {
    format = v;
    colorSpaceMode = nsVDXPixmap::kColorSpaceMode_None;
    colorRangeMode = nsVDXPixmap::kColorRangeMode_None;
  }
  VDPixmapFormatEx(const VDPixmap& v) {
    format = v.format;
    colorSpaceMode = v.info.colorSpaceMode;
    colorRangeMode = v.info.colorRangeMode;
  }
  VDPixmapFormatEx(const VDPixmapFormatEx& v) {
    format = v.format;
    colorSpaceMode = v.colorSpaceMode;
    colorRangeMode = v.colorRangeMode;
  }
  bool fullEqual(const VDPixmapFormatEx& v) const {
    return (format == v.format &&
      colorSpaceMode == v.colorSpaceMode &&
      colorRangeMode == v.colorRangeMode);
  }
  bool defaultMode() const {
    return colorSpaceMode == nsVDXPixmap::kColorSpaceMode_None && colorRangeMode == nsVDXPixmap::kColorRangeMode_None;
  }
};

struct VDPixmapLayout {
  ptrdiff_t data;
  const uint32 *palette;
  vdpixsize w;
  vdpixsize h;
  vdpixoffset pitch;
  sint32 format;

  // Auxiliary planes are always byte-per-pixel.

  ptrdiff_t data2; // Cb (U) for YCbCr
  vdpixoffset pitch2;
  ptrdiff_t data3; // Cr (V) for YCbCr
  vdpixoffset pitch3;
  ptrdiff_t data4; // Alpha
  vdpixoffset pitch4;

  VDPixmapFormatEx formatEx;
};
/// end of pixmap.h

class MyFilterModPixmap : public IFilterModPixmap {
public:
  // not VDcall calling convention!
  virtual FilterModPixmapInfo* GetPixmapInfo(const VDXPixmap* pixmap) {
    return &((VDPixmap*)pixmap)->info;
  }
  virtual uint64 GetFormat_XRGB64() {
    return nsVDXPixmap::kPixFormat_XRGB64;
  }
};

// Callback definitions

char exception_conversion_buffer[2048];

static void VDcall VDFilterCallbackThrowExcept(const char *format, ...) {
  va_list val;
  va_start(val, format);
  _vsnprintf(exception_conversion_buffer, sizeof(exception_conversion_buffer)-1, format, val);
  va_end(val);
  exception_conversion_buffer[sizeof(exception_conversion_buffer)-1] = '\0';
  throw AvisynthError(exception_conversion_buffer);
}

static void VDcall VDFilterCallbackExceptMemory() {
  throw AvisynthError("VDubFilter: Out of memory.");
}

// original quote from VD source :)
// This is really disgusting...

struct VDXFilterVTbls {
  void *pvtblVBitmap;
};

static void VDcall VDFilterCallbackInitVTables(VDXFilterVTbls *pvtbls) {
  VBitmap tmp;
  pvtbls->pvtblVBitmap = *(void **)&tmp;
}

VDXFilterDefinition *VDcall FilterAdd(VDXFilterModule *fm, VDXFilterDefinition *pfd, int fd_len);
VDXFilterDefinition *VDcall FilterModAdd(VDXFilterModule *fm, VDXFilterDefinition *pfd, int fd_len, FilterModDefinition *pfm, int fm_len);

void VDcall FilterRemove(VDXFilterDefinition*) {}

static long VDFilterCallbackGetHostVersionInfo(char *buf, int len) {
  char tbuf[256];

  long version_num = 2; // why not
  strcpy(tbuf, "Avisynth+ VirtualDub interface");
  _snprintf(buf, len, tbuf, version_num,
#ifdef _DEBUG
    "debug"
#else
    "release"
#endif
  );

  return version_num;
}

static long VDcall VDFilterCallbackGetCPUFlags() {
  return GetCPUFlags();
}

bool VDcall VDFilterCallbackIsFPUEnabled() { return !!(GetCPUFlags() & CPUF_FPU); }
bool VDcall VDFilterCallbackIsMMXEnabled() { return !!(GetCPUFlags() & CPUF_MMX); }

VDXFilterFunctions g_VDFilterCallbacks={
  FilterAdd, FilterRemove, VDFilterCallbackIsFPUEnabled, VDFilterCallbackIsMMXEnabled, VDFilterCallbackInitVTables,
  VDFilterCallbackExceptMemory, VDFilterCallbackThrowExcept, VDFilterCallbackGetCPUFlags,VDFilterCallbackGetHostVersionInfo
};

FilterModInitFunctions g_FilterModCallbacks = {
  FilterModAdd // yes, this is a single function
};

////////////////////////////////////////////////////////////

class MyScriptInterpreter : public IVDScriptInterpreter {
  IScriptEnvironment* const env;
public:
  MyScriptInterpreter(IScriptEnvironment* _env) : env(_env) {}
  void VDcall Destroy() {}
  void VDcall SetRootHandler(VDScriptRootHandlerPtr, void*) {}
  void VDcall ExecuteLine(const char *s) {}
  void VDcall ScriptError(int e) {
    switch (e) {
      case 21: env->ThrowError("VirtualdubFilterProxy: OUT_OF_MEMORY");
      case 24: env->ThrowError("VirtualdubFilterProxy: FCALL_OUT_OF_RANGE");
      case 26: env->ThrowError("VirtualdubFilterProxy: FCALL_UNKNOWN_STR");
      default: env->ThrowError("VirtualdubFilterProxy: Unknown error code %d", e);
    }
  }
  const char* VDcall TranslateScriptError(const VDScriptError&  cse) { return const_cast<char *>(""); }
  char** VDcall AllocTempString(long l) { return (char**)0; }
  VDScriptValue VDcall LookupObjectMember(const VDScriptObject *obj, void *, char *szIdent) { return VDScriptValue(); }
  // vdub2
  const VDScriptFunctionDef* VDcall GetCurrentMethod() { return (VDScriptFunctionDef *)0; }
  int VDcall GetErrorLocation() { return 0; }; // n/a
  VDScriptValue VDcall DupCString(const char *s) { return VDScriptValue((char **)&s); }; // n/a
};

////////////////////////////////////////////////////////////////////////
// prefetchProc2 implementation
// (multiple source frames from the same clip)

#if 0
// just a sample, how a Prefetch2 in crossfade filter is calling
bool Filter::Prefetch2(sint64 frame, IVDXVideoPrefetcher* prefetcher)
{
  if (param.pos == -1 || frame<param.pos - param.width) {
    prefetcher->PrefetchFrameDirect(0, frame);
    return true;
  }
  if (frame >= param.pos) {
    prefetcher->PrefetchFrameDirect(0, frame + param.width);
    return true;
  }

  prefetcher->PrefetchFrame(0, frame, 0);
  prefetcher->PrefetchFrame(0, frame + param.width, 0);

  return true;
}
#endif

///////////////////////////////////////////////////////////////////////////
class VideoPrefetcher : public IVDXVideoPrefetcher {
public:
  VideoPrefetcher(uint32 sourceCount);

  bool operator==(const VideoPrefetcher& other) const;
  bool operator!=(const VideoPrefetcher& other) const;

  void Clear();
#ifdef USE_TransformToNearestUnique
  // not in AVS+
  void TransformToNearestUnique(const vdvector<vdrefptr<IVDFilterFrameSource> >& src);
  void Finalize();
#endif

  int VDXAPIENTRY AddRef() { return 2; }
  int VDXAPIENTRY Release() { return 1; }

  void * VDXAPIENTRY AsInterface(uint32 iid) {
    if (iid == IVDXUnknown::kIID)
      return static_cast<IVDXUnknown *>(this);
    else if (iid == IVDXVideoPrefetcher::kIID)
      return static_cast<IVDXVideoPrefetcher *>(this);

    return NULL;
  }

  virtual void VDXAPIENTRY PrefetchFrame(sint32 srcIndex, sint64 frame, uint64 cookie);
  virtual void VDXAPIENTRY PrefetchFrameDirect(sint32 srcIndex, sint64 frame);
  virtual void VDXAPIENTRY PrefetchFrameSymbolic(sint32 srcIndex, sint64 frame);

  struct PrefetchInfo {
    sint64 mFrame;
    uint64 mCookie;
    uint32 mSrcIndex;

    bool operator==(const PrefetchInfo& other) const {
      return mFrame == other.mFrame && mCookie == other.mCookie && mSrcIndex == other.mSrcIndex;
    }

    bool operator!=(const PrefetchInfo& other) const {
      return mFrame != other.mFrame || mCookie != other.mCookie || mSrcIndex != other.mSrcIndex;
    }
  };

  // PF regular vector in avs
  //typedef vdfastfixedvector<PrefetchInfo, 32> SourceFrames;
  typedef std::vector<PrefetchInfo> SourceFrames;
  SourceFrames mSourceFrames;

  sint64	mDirectFrame;
  sint64	mSymbolicFrame;
  uint32	mDirectFrameSrcIndex;
  uint32	mSymbolicFrameSrcIndex;
  uint32	mSourceCount;
  const char *mpError;
};

VideoPrefetcher::VideoPrefetcher(uint32 sourceCount)
  : mDirectFrame(-1)
  , mDirectFrameSrcIndex(0)
  , mSymbolicFrame(-1)
  , mSymbolicFrameSrcIndex(0)
  , mSourceCount(sourceCount)
  , mpError(NULL)
{
}

bool VideoPrefetcher::operator==(const VideoPrefetcher& other) const {
  return mDirectFrame == other.mDirectFrame && mSourceFrames == other.mSourceFrames;
}

bool VideoPrefetcher::operator!=(const VideoPrefetcher& other) const {
  return mDirectFrame != other.mDirectFrame || mSourceFrames != other.mSourceFrames;
}

void VideoPrefetcher::Clear() {
  mDirectFrame = -1;
  mSymbolicFrame = -1;
  mSourceFrames.clear();
}

#ifdef USE_TransformToNearestUnique
// not in avs+
void VirtualdubFilterProxy::VideoPrefetcher::TransformToNearestUnique(const vdvector<vdrefptr<IVDFilterFrameSource> >& srcs) {
  if (mDirectFrame >= 0)
    mDirectFrame = srcs[mDirectFrameSrcIndex]->GetNearestUniqueFrame(mDirectFrame);

  for (SourceFrames::iterator it(mSourceFrames.begin()), itEnd(mSourceFrames.end()); it != itEnd; ++it) {
    PrefetchInfo& info = *it;

    info.mFrame = srcs[info.mSrcIndex]->GetNearestUniqueFrame(info.mFrame);
  }
}

// not supported in AVS
void VirtualdubFilterProxy::VideoPrefetcher::Finalize() {
  if (mSymbolicFrame < 0 && !mSourceFrames.empty()) {
    vdint128 accum(0);
    int count = 0;

    for (SourceFrames::const_iterator it(mSourceFrames.begin()), itEnd(mSourceFrames.end()); it != itEnd; ++it) {
      const PrefetchInfo& info = *it;
      accum += info.mFrame;
      ++count;
    }

    accum += (count >> 1);
    mSymbolicFrame = accum / count;
  }
}
#endif

void VDXAPIENTRY VideoPrefetcher::PrefetchFrame(sint32 srcIndex, sint64 frame, uint64 cookie) {
  if (srcIndex >= (sint64)mSourceCount) {
    mpError = "An invalid source index was specified in a prefetch operation.";
    return;
  }

  if (frame < 0)
    frame = 0;

  /*
  // VD original:
  PrefetchInfo& info = mSourceFrames.push_back();
  info.mFrame = frame;
  info.mCookie = cookie;
  info.mSrcIndex = srcIndex;
  */
  PrefetchInfo info;
  info.mFrame = frame;
  info.mCookie = cookie;
  info.mSrcIndex = srcIndex;
  mSourceFrames.push_back(info);
}

void VDXAPIENTRY VideoPrefetcher::PrefetchFrameDirect(sint32 srcIndex, sint64 frame) {
  if (srcIndex >= (sint64)mSourceCount) {
    mpError = "An invalid source index was specified in a prefetch operation.";
    return;
  }

  if (mDirectFrame >= 0) {
    mpError = "PrefetchFrameDirect() was called multiple times.";
    return;
  }

  if (frame < 0)
    frame = 0;

  mDirectFrame = frame;
  mDirectFrameSrcIndex = srcIndex;
  mSymbolicFrame = frame;
  mSymbolicFrameSrcIndex = srcIndex;
}

void VDXAPIENTRY VideoPrefetcher::PrefetchFrameSymbolic(sint32 srcIndex, sint64 frame) {
  if (srcIndex >= (sint64)mSourceCount) {
    mpError = "An invalid source index was specified in a prefetch operation.";
    return;
  }

  if (mSymbolicFrame >= 0) {
    mpError = "PrefetchFrameSymbolic() was called after a symbolic frame was already set.";
    return;
  }

  if (frame < 0)
    frame = 0;

  mSymbolicFrame = frame;
  mSymbolicFrameSrcIndex = srcIndex;
}

// End of VideoPrefetcher help from FilterInstance.cpp
///////////////////////////////////////////////////////////////////////////

// filter instance
class VirtualdubFilterProxy : public GenericVideoFilter {
  PVideoFrame src, dst, last;
  VDXFBitmap vbSrc, vbLast, vbDst;
  VDPixmapLayout PixmapLayoutSrc, PixmapLayoutLast, PixmapLayoutDst;
  VDPixmap PixmapSrc, PixmapLast, PixmapDst;

  // for multiple source frames
  std::vector<VDXFBitmap *> mSourceFrameArray;
  std::vector<VDXFBitmap *> mDestFrameArray; // only 1 supported

  FilterDefinitionList* const fdl;
  VDXFilterDefinition2* const fd;
  VDXFilterStateInfo fsi;
  VDXFilterActivation fa;
  FilterModActivation fma;

  DummyFilterPreview fp;
  DummyFilterPreview2 fp2;
  int expected_frame_number;

  MyFilterModPixmap FilterModPixmap;

  // prefetchProc2 support
  VideoPrefetcher *AvsVdubPrefetcher;

  // rangehint extra avs parameter support
  enum RangeHintType {
    kRangeHintNotSpecified = 0,
    kRangeHintLimited601 = 1,
    kRangeHintLimited709 = 2,
    kRangeHintFull601 = 3,
    kRangeHintFull709 = 4
  };
  uint32 rangeHint;

  void CallStartProc() {
    if (fd->startProc) {
      int result = fd->startProc(&fa, &g_VDFilterCallbacks);
      if (result != 0) {
        if (fd->endProc)
          fd->endProc(&fa, &g_VDFilterCallbacks);
        throw AvisynthError("VirtualdubFilterProxy: error calling startProc");
      }
    }
  }

  void CallEndProc() {
    if (fd->endProc) {
      int result = fd->endProc(&fa, &g_VDFilterCallbacks);
      if (result != 0) {
        throw AvisynthError("VirtualdubFilterProxy: error calling endProc");
      }
    }
  }

public:
  VirtualdubFilterProxy(PClip _child, FilterDefinitionList* _fdl, AVSValue args, IScriptEnvironment* env)
    : GenericVideoFilter(_child), fdl(_fdl), fd(_fdl->fd), fa(vbDst, vbSrc, &vbLast), AvsVdubPrefetcher(nullptr), rangeHint(0)
  {
    fdl->myFilterModule->refcounter++;

    _RPT2(0, "VirtualdubFilterProxy: Create called for %s, instanceNo=%d\r\n", fdl->myFilterModule->avisynth_function_name, fdl->myFilterModule->refcounter);

    if (!fd)
      env->ThrowError("VirtualdubFilterProxy: No FilterDefinition structure!");

    if (vi.BitsPerComponent() == 32)
      throw AvisynthError("VirtualdubFilterProxy: 32 bit float colorspaces are supported");

    if (!vi.IsRGB24() && !vi.IsRGB32() && !vi.IsRGB64() && !(vi.IsYV411() || vi.Is420() || vi.Is422() || vi.Is444() || vi.IsY()))
      throw AvisynthError("VirtualdubFilterProxy: video format not supported");

    fma.filterMod = fdl->fd->fm;

    fa.filter = fd;

    fa.pfsi = &fsi; // VDXFilterStateInfo *pfsi;
    fa.ifp = &fp;   // dummy filter preview
    fa.ifp2 = &fp2; // dummy FilterPreview2
    fa.filter_data = 0;

    fa.x1 = 0;
    fa.y1 = 0;
    fa.x2 = 0;
    fa.y2 = 0;

    const int api_ver = fdl->myFilterModule->filterdef_ver_hi;

    // we can specify multiple source frames from the same clip, see prefetch2
    int sourceframecount = 1;

    if (api_ver >= 14) {
      fa.mSourceFrameCount = sourceframecount;
    }

    mSourceFrameArray.resize(sourceframecount);
    mSourceFrameArray[0] = (VDXFBitmap *)&vbSrc;

    // size=1 supported
    mDestFrameArray.resize(1);
    mDestFrameArray[0] = (VDXFBitmap *)&vbDst;

    if (api_ver >= 14) {
      // multiple source frame support from v14 (prefetch2)
      fa.mpSourceFrames = reinterpret_cast<VDXFBitmap * const *>(mSourceFrameArray.data());
      fa.mpOutputFrames = reinterpret_cast<VDXFBitmap * const *>(mDestFrameArray.data());
    }
    if (api_ver >= 15) {
      fa.mpVDXA = nullptr; // IVDXAContext *mpVDXA;
    }
    if (api_ver >= 16) {
      fa.mSourceStreamCount = 0; // (V16+)
      fa.mpSourceStreams = nullptr; // (V16+)
    }

    if (api_ver >= 18) {
      fma.filter = fd;
      fma.fmpixmap = &FilterModPixmap;
      fma.fmpreview = 0;
      fma.fmsystem = 0;
      fma.fmtimeline = 0;
    }

    fsi.lMicrosecsPerFrame = fsi.lMicrosecsPerSrcFrame = MulDiv(vi.fps_denominator, 1000000, vi.fps_numerator);

    if (fd->inst_data_size) {
      fa.filter_data = new char[fd->inst_data_size];
      memset(fa.filter_data, 0, fd->inst_data_size);
      if (fd->initProc) {
        if (fd->initProc(&fa, &g_VDFilterCallbacks) != 0)
          throw AvisynthError("VirtualdubFilterProxy: Error calling initProc");
      }

      if (fma.filterMod) {
        if (fma.filterMod->activateProc) {
          fa.fma = &fma; // FilterModActivation* fma; // (V18+)
          fma.filter_data = fa.filter_data; // just use pointer from fa
          try {
            fma.filterMod->activateProc(&fma, &g_VDFilterCallbacks);
          }
          catch (...) {
            throw AvisynthError("VirtualdubFilterProxy: Error calling filtermod activateProc");
          }
        }
      }

      // here we pass argument values of avs script to the filter's config
      // extra avs-only parameter: [rangeHint]s
      // There is always a clip-parameter-only kind of the filter, rangeHint is not applicable.
      const int skipLastN = args.ArraySize() == 1 ? 0 : 1;

      if (args.ArraySize() > 1 + skipLastN)
        InvokeSyliaConfigFunction(fd, args, env, skipLastN);

      // parse extra parameter seen only by avisynth
      rangeHint = kRangeHintNotSpecified;
      if (skipLastN > 0) {
        const char *rangeHintStr = args[args.ArraySize() - skipLastN].AsString();
        if (rangeHintStr) {
          _RPT1(0, "VdubFilterProxy Extra parameter found: %s", rangeHint);
          if (!lstrcmpi(rangeHintStr, "rec601"))
            rangeHint = kRangeHintLimited601;
          else if (!lstrcmpi(rangeHintStr, "rec709"))
            rangeHint = kRangeHintLimited709;
          else if (!lstrcmpi(rangeHintStr, "PC.601"))
            rangeHint = kRangeHintFull601;
          else if (!lstrcmpi(rangeHintStr, "PC.709"))
            rangeHint = kRangeHintFull709;
          else if (!lstrcmpi(rangeHintStr, ""))
            rangeHint = kRangeHintNotSpecified;
          else
            env->ThrowError("VirtualDubFilterProxy: invalid 'rangehint' for '%s'\r\n", fdl->myFilterModule->avisynth_function_name);
        }
      }

    }

/*  https://github.com/shekh/VirtualDub2/wiki/videofilt_processingvideoframes
    The source frame buffers are specified by the src field of the VDXFilterActivation structure,
    and the output frame buffer is specified by the dst field.The task of runProc is to process
    the source image and write the result into the output image.The host has already allocated
    both frame buffers and fetched the source frame by the time the filter is invoked,
    so the only thing that needs to happen is to process the image data.

    Note that if paramProc specified that the filter is operating in in-place mode, the source
    and output frame buffers will point to the same memory.
*/

    // our intermediate lifetime buffer (when no prefetch2, single source frame case)
    src = env->NewVideoFrame(vi);

    SetVFBitmap(src, &vbSrc, vi, PixmapLayoutSrc, PixmapSrc, api_ver, rangeHint);
    SetVFBitmap(src, &vbLast, vi, PixmapLayoutLast, PixmapLast, api_ver, rangeHint);
    SetVFBitmap(src, &vbDst, vi, PixmapLayoutDst, PixmapDst, api_ver, rangeHint);

    const long flags = fd->paramProc ? fd->paramProc(&fa, &g_VDFilterCallbacks) : FILTERPARAM_SWAP_BUFFERS;

    // filter can change output frame count (length of clip) in paramProc
    if (api_ver >= 12 && fa.dst.mFrameCount >= 0)
      vi.num_frames = (int)fa.dst.mFrameCount;

    // todo check what is does, do we need call both?
    long flags2 = 0;
    if (fd->fm) {
      flags2 = fd->fm->paramProc ? fd->fm->paramProc(&fa, &g_VDFilterCallbacks) : 0;
    }

    if (flags == FILTERPARAM_NOT_SUPPORTED) {
      env->ThrowError("VirtualdubFilterProxy: format not supported by %s", fdl->myFilterModule->avisynth_function_name);
    }

    // Request distinct source and destination buffers. Otherwise, the source and destination buffers alias (in-place mode)
    const bool two_buffers = !!(flags & FILTERPARAM_SWAP_BUFFERS);
    const bool needs_last = !!(flags & FILTERPARAM_NEEDS_LAST); // since V16 this option is deprecated

    // Filter supports image formats other than RGB32.
    const bool supports_alternative_formats = !!(flags & FILTERPARAM_SUPPORTS_ALTFORMATS);
    if (!supports_alternative_formats && !vi.IsRGB32())
      env->ThrowError("VirtualdubFilterProxy: filter supports RGB32 only: %s", fdl->myFilterModule->avisynth_function_name);

    /// Filter requests that 16-bits input bitmap is normalized. This guarantees that info.ref_r..info.ref_a attributes are 0xFFFF when applicable.
    const bool normalize16 = !!(flags & FILTERPARAM_NORMALIZE16);
    if (normalize16)
    {
      if (vi.BitsPerComponent() >= 10 && vi.BitsPerComponent() <= 14)
        env->ThrowError("VirtualdubFilterProxy: 10-14 bit formats are required to be normalized to 16 bits. %s", fdl->myFilterModule->avisynth_function_name);
    }

    // FILTERPARAM_ALIGN_SCANLINES_16/32/64: not checked since avs+ has 64bytes alignment
    // in classic AVS 2.6.0.5 only align=16 was guaranteed (except when unaligned crop is used)

    const bool src_needs_hdc = (vbSrc.dwFlags & VDXFBitmap::NEEDS_HDC);
    const bool dst_needs_hdc = (vbDst.dwFlags & VDXFBitmap::NEEDS_HDC);
    if (src_needs_hdc || dst_needs_hdc) {
      // only for RGB32, old feature
      //      throw AvisynthError("VirtualdubFilterProxy: HDC not supported");
      vbSrc.hdc = vbDst.hdc = vbLast.hdc = (VDXHDC)GetDC(NULL);
    }

    // obsolate since v16
    if (needs_last) {
      last = env->NewVideoFrame(vi);
      SetVFBitmap(last, &vbLast, vi, PixmapLayoutLast, PixmapLast, api_ver, rangeHint);
    }

    if (two_buffers) {
      vi.width = vbDst.w;
      vi.height = vbDst.h;
      // for copying result back to Avisynth
      dst = env->NewVideoFrame(vi);
      SetVFBitmap(dst, &vbDst, vi, PixmapLayoutDst, PixmapDst, api_ver, rangeHint);
    }

    CallStartProc();
    expected_frame_number = 0;
  }

  void SetVFBitmap(const PVideoFrame& pvf, VDXFBitmap* const pvb, const VideoInfo& vi
    , VDPixmapLayout &PixmapLayout, VDPixmap &Pixmap, int api_ver, int rangeHint
  ) {
    pvb->data = (Pixel*)pvf->GetReadPtr();
    pvb->palette = 0; // Not used for video filters.

    /*
    depth: Indicates the bit depth of the image. For video filters (RGB32), this is typically 32,
    indicating 24-bit RGB with an unused 8 - bit alpha channel in the high byte(fourth byte in
    memory order).
    If this is zero, it means the bitmap uses an alternate format and the mpPixmap field must be used.
        For multi-plane (e.g. UV) we should pass info in mpPixmapLayout
    */
    bool hasNewPixelFormats = api_ver >= 12;
    pvb->depth = hasNewPixelFormats ? 0 : vi.BitsPerComponent() * vi.NumComponents(); // // 8*4=32 or 16*4=64
    pvb->w = vi.width;
    pvb->h = pvf->GetHeight();

    /* pitch:
    V12+ only: Bitmaps can be stored top - down as well as bottom-up.
    The pitch value value is positive if the image is stored bottom-up
    in memory and negative if the image is stored top-down. This is only
    permitted if the filter declares support for flexible formats by
    returning FILTERPARAM_SUPPORTS_ALTFORMATS from paramProc; otherwise,
    the host ensures that the filter receives a bottom-up orientation
    with a positive pitch, flipping the bitmap beforehand if necessary.
    */
    pvb->pitch = pvf->GetPitch();

    /*
    Distance from the end of one scanline to the beginning of the next,
    in bytes. This value is positive or zero if the image is stored bottom-up
    in memory and negative if the image is stored top-down. A value of zero
    indicates that the image is stored bottom-up with no padding between
    scanlines. For a 32-bit bitmap, modulo is equal to pitch - 4*w
    */
    pvb->modulo = pvf->GetPitch() - pvf->GetRowSize(); //   (pvb->w*pvb->depth + 7) / 8; // pvb->Modulo();
    pvb->size = pvb->pitch * pvb->h; // size of plane 0 including padding
    pvb->offset = 0; // Offset from the start of the buffer to the beginning of the bitmap, in bytes. V12+ only: For image formats that use multiple planes, this points to the base of plane 0.
    pvb->dwFlags = 0;
    pvb->hdc = 0;
    if (api_ver >= 12) {
      pvb->mFrameRateHi = vi.fps_numerator; // Frame rate numerator (V1.7.4+)
      pvb->mFrameRateLo = vi.fps_denominator; // Frame rate denominator (V1.7.4+)
      pvb->mFrameCount = vi.num_frames; // Frame count; -1 if unlimited or indeterminate (V1.7.4+)

      /* mpPixmapLayout
      V12+ only: More flexible description of bitmap layout, which is
      able to accommodate multi - plane and YCbCr formats.In paramProc,
      this field is always valid when provided by the host. When set by
      the filter in paramProc, this field is activated when the depth
      field of the bitmap is set to zero.
      */
      PixmapLayout.data = (ptrdiff_t)pvb->data; // (Pixel*)pvf->GetReadPtr();
      PixmapLayout.pitch = pvb->pitch;
      PixmapLayout.w = pvb->w;
      PixmapLayout.h = pvb->h;
      if (!vi.IsY() && (vi.IsYUV() || vi.IsYUVA()))
      {
        PixmapLayout.data2 = (ptrdiff_t)pvf->GetReadPtr(PLANAR_U);
        PixmapLayout.data3 = (ptrdiff_t)pvf->GetReadPtr(PLANAR_V);
        PixmapLayout.data4 = (ptrdiff_t)pvf->GetReadPtr(PLANAR_A);
        PixmapLayout.pitch2 = (ptrdiff_t)pvf->GetPitch(PLANAR_U);
        PixmapLayout.pitch3 = (ptrdiff_t)pvf->GetPitch(PLANAR_V);
        PixmapLayout.pitch4 = (ptrdiff_t)pvf->GetPitch(PLANAR_A);;
      }
      else if (vi.IsPlanarRGB() || vi.IsPlanarRGBA()) // not used yet
      {
        PixmapLayout.data = (ptrdiff_t)pvf->GetReadPtr(PLANAR_G);
        PixmapLayout.data2 = (ptrdiff_t)pvf->GetReadPtr(PLANAR_B);
        PixmapLayout.data3 = (ptrdiff_t)pvf->GetReadPtr(PLANAR_R);
        PixmapLayout.data4 = (ptrdiff_t)pvf->GetReadPtr(PLANAR_A);
        PixmapLayout.pitch = (ptrdiff_t)pvf->GetPitch(PLANAR_G);
        PixmapLayout.pitch2 = (ptrdiff_t)pvf->GetPitch(PLANAR_B);
        PixmapLayout.pitch3 = (ptrdiff_t)pvf->GetPitch(PLANAR_R);
        PixmapLayout.pitch4 = (ptrdiff_t)pvf->GetPitch(PLANAR_A);;
      }
      else {
        PixmapLayout.data2 = 0;
        PixmapLayout.data3 = 0;
        PixmapLayout.data4 = 0;
        PixmapLayout.pitch2 = 0;
        PixmapLayout.pitch3 = 0;
        PixmapLayout.pitch4 = 0;
      }

      if (vi.IsRGB24())
        PixmapLayout.format = nsVDXPixmap::kPixFormat_RGB888;
      else if (vi.IsRGB32())
        PixmapLayout.format = nsVDXPixmap::kPixFormat_XRGB8888;
      else if (vi.IsRGB64())
        PixmapLayout.format = nsVDXPixmap::kPixFormat_XRGB64; // same as in GetFormat_XRGB64
      else if (vi.IsY() && vi.BitsPerComponent() == 8)
        PixmapLayout.format = nsVDXPixmap::kPixFormat_Y8;
      else if (vi.IsYV411())
        PixmapLayout.format = nsVDXPixmap::kPixFormat_YUV411_Planar;
      else if (vi.IsYV12())
        PixmapLayout.format = nsVDXPixmap::kPixFormat_YUV420_Planar;
      else if (vi.IsYV16())
        PixmapLayout.format = nsVDXPixmap::kPixFormat_YUV422_Planar;
      else if (vi.IsYV24())
        PixmapLayout.format = nsVDXPixmap::kPixFormat_YUV444_Planar;
      else if (vi.Is420() && vi.IsYUVA() && vi.BitsPerComponent() == 8)
        PixmapLayout.format = nsVDXPixmap::kPixFormat_YUV420_Alpha_Planar;
      else if (vi.Is422() && vi.IsYUVA() && vi.BitsPerComponent() == 8)
        PixmapLayout.format = nsVDXPixmap::kPixFormat_YUV422_Alpha_Planar;
      else if (vi.Is444() && vi.IsYUVA() && vi.BitsPerComponent() == 8)
        PixmapLayout.format = nsVDXPixmap::kPixFormat_YUV444_Alpha_Planar;
      else if (vi.IsY() && vi.BitsPerComponent() <= 16) // allow 10-16 bits to report themselves as 16, normalize later
        PixmapLayout.format = nsVDXPixmap::kPixFormat_Y16;
      else if (vi.Is420() && vi.BitsPerComponent() <= 16)
        PixmapLayout.format = vi.IsYUVA() ? nsVDXPixmap::kPixFormat_YUV420_Alpha_Planar16 : nsVDXPixmap::kPixFormat_YUV420_Planar16;
      else if (vi.Is422() && vi.BitsPerComponent() <= 16)
        PixmapLayout.format = vi.IsYUVA() ? nsVDXPixmap::kPixFormat_YUV422_Alpha_Planar16 : nsVDXPixmap::kPixFormat_YUV422_Planar16;
      else if (vi.Is444() && vi.BitsPerComponent() <= 16)
        PixmapLayout.format = vi.IsYUVA() ? nsVDXPixmap::kPixFormat_YUV444_Alpha_Planar16 : nsVDXPixmap::kPixFormat_YUV444_Planar16;

      PixmapLayout.palette = 0;

      // Avs terminology:
      // don't use original constants, use base formats and formatEx instead
      // Rec.601  kColorRangeMode_Limited, kColorSpaceMode_601 (n/a)
      // Rec.709  kColorRangeMode_Limited, kColorSpaceMode_709 (_709)
      // PC.601   kColorRangeMode_Full   , kColorSpaceMode_601 (_FR)
      // PC.709   kColorRangeMode_Full   , kColorSpaceMode_709 (709_FR)
      if (vi.IsYUV() || vi.IsYUVA() || vi.IsY()) {
        switch (rangeHint) {
        case kRangeHintLimited601:
          PixmapLayout.formatEx.format = PixmapLayout.format;
          PixmapLayout.formatEx.colorRangeMode = nsVDXPixmap::kColorRangeMode_Limited;
          if(!vi.IsY())
            PixmapLayout.formatEx.colorSpaceMode = nsVDXPixmap::kColorSpaceMode_601;
          break;
        case kRangeHintLimited709:
          PixmapLayout.formatEx.format = PixmapLayout.format;
          PixmapLayout.formatEx.colorRangeMode = nsVDXPixmap::kColorRangeMode_Limited;
          if (!vi.IsY())
            PixmapLayout.formatEx.colorSpaceMode = nsVDXPixmap::kColorSpaceMode_709;
          break;
        case kRangeHintFull601:
          PixmapLayout.formatEx.format = PixmapLayout.format;
          PixmapLayout.formatEx.colorRangeMode = nsVDXPixmap::kColorRangeMode_Full;
          if (!vi.IsY())
            PixmapLayout.formatEx.colorSpaceMode = nsVDXPixmap::kColorSpaceMode_601;
          break;
        case kRangeHintFull709:
          PixmapLayout.formatEx.format = PixmapLayout.format;
          PixmapLayout.formatEx.colorRangeMode = nsVDXPixmap::kColorRangeMode_Full;
          if (!vi.IsY())
            PixmapLayout.formatEx.colorSpaceMode = nsVDXPixmap::kColorSpaceMode_709;
          break;
        }
      }

      /* mpPixmap
      V12 + only: More flexible description of bitmap, which is able to accommodate
      multi - plane and YCbCr formats.
      This field is always valid in runProc, even if mpPixmapLayout was not used in paramProc.
      For filters that require V12 + of the API, mpPixmap may be used exclusively in runProc
      in lieu of the regular data, pitch, etc.fields above.
      */
      Pixmap.data = (void *)PixmapLayout.data;
      Pixmap.data2 = (void *)PixmapLayout.data2;
      Pixmap.data3 = (void *)PixmapLayout.data3;
      Pixmap.data4 = (void *)PixmapLayout.data4;
      Pixmap.pitch = PixmapLayout.pitch;
      Pixmap.pitch2 = PixmapLayout.pitch2;
      Pixmap.pitch3 = PixmapLayout.pitch3;
      Pixmap.pitch4 = PixmapLayout.pitch4;
      Pixmap.format = PixmapLayout.format;
      Pixmap.palette = PixmapLayout.palette;
      Pixmap.h = PixmapLayout.h;
      Pixmap.w = PixmapLayout.w;
      // really it's FilterModVersion>=5
      Pixmap.info.colorRangeMode = PixmapLayout.formatEx.colorRangeMode;
      Pixmap.info.colorSpaceMode = PixmapLayout.formatEx.colorSpaceMode;

      /*
      With new 16 - bit formats these fields indicate maximum value(per channels for rgba, and only ref_r for all yuv channels).
      When bitmap is normalized, ref values are 0xFFFF.
      For example when bitmap contains un - normalized 10 - bit from ffmpeg, ref values are 0x3FF.
      Can change with each frame(for example when appending different clips or using image sequence of different formats).
      No matter what ref values are, all pixel values that fit in uint16 are legal (clamp during processing if necessary).
      */
      const int bits_per_pixel = vi.BitsPerComponent();
      if (bits_per_pixel > 8) {
        const int ref = (1 << bits_per_pixel) - 1;
        if (vi.IsYUV() || vi.IsYUVA()) {
          Pixmap.info.ref_r = ref;
          Pixmap.info.ref_g = 0;
          Pixmap.info.ref_b = 0;
          Pixmap.info.ref_a = vi.IsYUVA() ? ref : 0;
        }
        else {
          Pixmap.info.ref_r = ref;
          Pixmap.info.ref_g = ref;
          Pixmap.info.ref_b = ref;
          Pixmap.info.ref_a = ref;
        }
      }

      // originally Pixmap is VDPixmap w/ plane4 and info fields
      pvb->mpPixmapLayout = (VDXPixmapLayout *)&PixmapLayout; // VDXPixmapLayout	*mpPixmapLayout;
      pvb->mpPixmap = (VDXPixmap *)&Pixmap; // const VDXPixmap* mpPixmap;

      pvb->mAspectRatioHi = 0; ///< Pixel aspect ratio fraction (numerator). 0/0 = unknown
      pvb->mAspectRatioLo = 0; ///< Pixel aspect ratio fraction (denominator).

      pvb->mFrameNumber = 0; ///< Current frame number (zero based).
      pvb->mFrameTimestampStart = 0; ///< Starting timestamp of frame, in 100ns units.
      pvb->mFrameTimestampEnd = 0; ///< Ending timestamp of frame, in 100ns units.
      pvb->mCookie = 0; ///< Cookie supplied when frame was requested.

      pvb->mVDXAHandle = 0; ///< Acceleration handle to be used with VDXA routines.
      pvb->mBorderWidth = 0;
      pvb->mBorderHeight = 0;
    }
  }

  void CopyFrame(PVideoFrame &dst, PVideoFrame &src, VideoInfo &vi, IScriptEnvironment* env)
  {
    const int planes_y[4] = { PLANAR_Y, PLANAR_U, PLANAR_V, PLANAR_A };
    const int planes_r[4] = { PLANAR_G, PLANAR_B, PLANAR_R, PLANAR_A };
    const int *planes;

    int planeCount;
    planeCount = vi.IsPlanar() ? vi.NumComponents() : 1;
    planes = (!vi.IsPlanar() || vi.IsYUV() || vi.IsYUVA()) ? planes_y : planes_r;

    for (int j = 0; j < planeCount; ++j)
    {
      int plane = planes[j];
      env->BitBlt(dst->GetWritePtr(plane), dst->GetPitch(plane), src->GetReadPtr(plane), src->GetPitch(plane),
        dst->GetRowSize(plane), dst->GetHeight(plane));
    }
  }

  PVideoFrame FilterFrame(int n, IScriptEnvironment* env, bool in_preroll) {
    // src: intermediate variable of the proxy, of which write pointers are prepared and 'published'
    // Usage of last is not recommended anymore, anyway, old filters may use it.
    if (last)
      CopyFrame(last, src, vi, env);

    /* https://github.com/shekh/VirtualDub2/wiki/videofilt_prefetchingmultiplesourceframes
    ** prefetching multiple source frames
    By default, VirtualDub video filters operate in 1:1 mode, where each source frame corresponds to
    exactly one output frame.The output frame stream has the same frame count, frame rate, and frame
    timings as the source stream.
    V13 or earlier: Not possible.

    Prefetching multiple source frames per output frame is only possible starting with API V14.
    If you are targeting an earlier API version, you need to use alternative ways of handling this,
    such as using internal buffering. See setting filter parameters on how to declare an internal
    lagging delay in your filter to accommodate this.

    In order to receive multiple source frames, a filter must implement the prefetchProc2 method.
    This method receives a prefetcher object which is invoked by the filter for each source frame required.

    ** Processing with multiple source frames
    Normally, the runProc method receives input and output frames via the src and dst fields of
    the VDXFilterActivation structure. The V14 API adds the mpInputFrames and mpOutputFrames arrays,
    which contain pointers to source and output frames. The src and dst fields alias onto the first
    entry of those arrays, whereas any additional frames take up additional slots.

    If your filter does not need more than one source frame, it can continue to use src and dst.

    ** Fetching frames in direct mode
    With the prefetcher object, you can also invoke the PrefetchFrameDirect() method to indicate that
    a frame should be copied directly from source to output. In addition to speeding up rendering for
    frames that your filter does not need to process, this also permits smart rendering logic to
    bypass the entire filter chain whenever possible and copy compressed frames directly from source.
    This runs much faster and preserves full quality in the source. If your filter is designed to
    only process a portion of its video, you should use PrefetchFrameDirect() whenever possible
    to enable direct mode operation.

    Note that even if you mark a frame as direct, your filter's runProc method may still be called.
    One reason this may occur is if the frame cannot be copied in direct mode. When this happens,
    your filter should copy the frame as required.

    Direct mode requires that source and output frame formats be compatible. If the formats or frame
    sizes do not match, it is a logic error to request a direct copy. You can, however, request a
    direct copy between streams that are of different frame rates or aspect ratios.
    */
    const int api_ver = fdl->myFilterModule->filterdef_ver_hi;

#if 0
    if (filter->prefetchProc2) {
      bool handled = false;

      VDExternalCodeBracket bracket(mFilterName.c_str(), __FILE__, __LINE__);
      vdprotected1("prefetching filter \"%s\"", const char *, filter->name) {
        handled = filter->prefetchProc2(AsVDXFilterActivation(), &g_VDFilterCallbacks, outputFrame, &prefetcher);
      }

      if (handled) {
        if (prefetcher.mpError)
          SetLogicErrorF("%s", prefetcher.mpError);

        if (!requireOutput || !prefetcher.mSourceFrames.empty() || prefetcher.mDirectFrame >= 0)
          return;
      }
    }
    else if (filter->prefetchProc) {
      sint64 inputFrame;

      VDExternalCodeBracket bracket(mFilterName.c_str(), __FILE__, __LINE__);
      vdprotected1("prefetching filter \"%s\"", const char *, filter->name) {
        inputFrame = filter->prefetchProc(AsVDXFilterActivation(), &g_VDFilterCallbacks, outputFrame);
      }

      prefetcher.PrefetchFrame(0, inputFrame, 0);
      return;
    }

    double factor = ((double)mRealSrc.mFrameRateHi * (double)mRealDst.mFrameRateLo) / ((double)mRealSrc.mFrameRateLo * (double)mRealDst.mFrameRateHi);

    prefetcher.PrefetchFrame(0, VDFloorToInt64((outputFrame + 0.5f) * factor), 0); #endif
#endif
    std::vector<PVideoFrame> inputFrames;
    std::vector<VDXFBitmap> inputBitmaps;
    std::vector<VDPixmapLayout> inputPixmapLayouts;
    std::vector<VDPixmap> inputPixmaps;

    if (api_ver >= 14 && fd->prefetchProc2) {
      bool handled = false;
      // multiple source frames by using prefetch2
      AvsVdubPrefetcher = new VideoPrefetcher(1); // one source
      handled = fd->prefetchProc2(&fa, &g_VDFilterCallbacks, n, AvsVdubPrefetcher);
      int numOfInputFrames;

      if (handled) {
        if (AvsVdubPrefetcher->mpError) {
          env->ThrowError("VirtualdubFilterProxy: error during Prefetch2: %s\r\n", AvsVdubPrefetcher->mpError);
        }
        const int mDirectFrameSrcIndex = AvsVdubPrefetcher->mDirectFrameSrcIndex; // no multiple sources yet, 0 at the moment
        const int mDirectFrame = (int)AvsVdubPrefetcher->mDirectFrame;
        if (mDirectFrame >= 0) {
          delete AvsVdubPrefetcher;
          // direct frame copy was requested by the filter (fast!)
          PVideoFrame _src = child->GetFrame(mDirectFrame, env);
          return _src;
        }
        // filter assembled a list of one or more frame numbers
        numOfInputFrames = (int)AvsVdubPrefetcher->mSourceFrames.size();
      }
      else {
        // 'not handled' case
        numOfInputFrames = 1;
      }

      fa.mSourceFrameCount = numOfInputFrames;
      mSourceFrameArray.resize(numOfInputFrames);

      inputFrames.reserve(numOfInputFrames);
      inputBitmaps.resize(numOfInputFrames);
      inputPixmapLayouts.resize(numOfInputFrames);
      inputPixmaps.resize(numOfInputFrames);

      for (int i = 0; i < numOfInputFrames; i++) {
        const int mSrcIndex = handled ? AvsVdubPrefetcher->mSourceFrames[i].mSrcIndex : 0; // no multiple sources, 0 at the moment
        const int mFrame = handled ? (int)AvsVdubPrefetcher->mSourceFrames[i].mFrame : n; // frame no set in proc2 or original 'n'

        inputFrames.emplace_back(child->GetFrame(mFrame, env));
        SetVFBitmap(inputFrames[i], &inputBitmaps[i], vi, inputPixmapLayouts[i], inputPixmaps[i], api_ver, rangeHint);
        mSourceFrameArray[i] = &inputBitmaps[i];
      }
      fa.mpSourceFrames = reinterpret_cast<VDXFBitmap * const *>(mSourceFrameArray.data());
      delete AvsVdubPrefetcher;
    }
    else if (api_ver >= 12 && fd->prefetchProc) {
      sint64 inputFrame = fd->prefetchProc(&fa, &g_VDFilterCallbacks, n);
      PVideoFrame _src = child->GetFrame((int)inputFrame, env);
      CopyFrame(src, _src, vi, env);
    }
    else
    { // scoped to kill _src
      PVideoFrame _src = child->GetFrame(n, env);
      // PVideoFrame src is the variable, of which read pointers are filled into 'fa.src'
      CopyFrame(src, _src, vi, env);
    }

    fsi.lCurrentSourceFrame = fsi.lCurrentFrame = n;
    fsi.lDestFrameMS = fsi.lSourceFrameMS = MulDiv(n, fsi.lMicrosecsPerFrame, 1000);

    fsi.flags = 0;
    fsi.mOutputFrame = n; // (V13/V1.8.2+) current output frame

    vbDst.mFrameNumber = n;

    fd->runProc(&fa, &g_VDFilterCallbacks);

    if (in_preroll) {
      return 0;
    } else {
      PVideoFrame _dst = env->NewVideoFrame(vi);
      CopyFrame(_dst, dst ? dst : src, vi, env);
      return _dst;
    }
  }

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) {
    _RPT1(0, "GetFrame called %d\r\n", n);
    if (n != expected_frame_number) {
      CallEndProc();
      CallStartProc();
      int preroll = fdl->myFilterModule->preroll;
      for (int i = min(n, preroll); i > 0; i--)
        FilterFrame(n - i, env, true);
    }
    expected_frame_number = n+1;
    return FilterFrame(n, env, false);
  }

  static int ConvertArgs(const AVSValue* args, VDXScriptValue* sylia_args, CScriptValueStringHelper* sylia_args_string_helper, int count) {
    for (int i=0; i<count; ++i) {
      if (args[i].IsInt()) {
        // the 64 bitness of an 'l' parameters is lost here, since avs has only as 32 bit Int, so we better fill up the whole 64 bits
        sylia_args[i] = (sint64)args[i].AsInt();
      } else if (args[i].IsFloat()) { // new from 160420 double support
        sylia_args[i] = args[i].AsFloat();
      } else if (args[i].IsString()) {
        // Oops, where can we put the pointer to pointer in x64? no place in CScriptValue struct
        // helper class/struct needed.
        sylia_args_string_helper[i].lpVoid = (void*)args[i].AsString();
        sylia_args[i] = (char**)&sylia_args_string_helper[i].lpVoid;
        /* original, works only for 32 bit
        sylia_args[i].lpVoid = (void*)args[i].AsString();
        sylia_args[i] = (char**)&sylia_args[i].lpVoid;
        */
      } else if (args[i].IsArray()) {
        return i+ConvertArgs(&args[i][0], sylia_args+i, sylia_args_string_helper+i, args[i].ArraySize());
      } else {
        return -1000;
      }
    }
    return count;
  }

  // skipLastN: number of optinal extra avs-only parameters
  void InvokeSyliaConfigFunction(VDXFilterDefinition* fd, AVSValue args, IScriptEnvironment* env, int skipLastN) {
    if (fd->script_obj && fd->script_obj->func_list && args.ArraySize() > 1 + skipLastN) {
      for (VDXScriptFunctionDef* i = fd->script_obj->func_list; i->arg_list; i++) {
        const char* p = i->arg_list; // p: original virtualdub param list e.g. 0ddddddddd
        int j;
        for (j = 1; j < args.ArraySize() - skipLastN; j++) {
          if (p[j] == 'i' && args[j].IsInt()) continue;
          else if (p[j] == 'l' && args[j].IsInt()) continue;    // param long is only Int in avs
          else if (p[j] == 'd' && args[j].IsFloat()) continue;  // 160420 type double support
          else if (p[j] == 's' && args[j].IsString()) continue;
          else if (p[j] == '.' && args[j].IsArray()) continue;
          else break;
        }
        if (j == args.ArraySize() - skipLastN && p[j] == 0) {
          // match
          MyScriptInterpreter si(env);
          VDXScriptValue sylia_args[30];
          CScriptValueStringHelper sylia_args_string_helper[30]; // helper class. x64 char ** helper did not fit into CScriptValue class size
          //int sylia_arg_count = ConvertArgs(&args[1], sylia_args, sylia_args_string_helper, args.ArraySize() - 1);
          int sylia_arg_count = ConvertArgs(&args[1], sylia_args, sylia_args_string_helper, args.ArraySize()-1 - skipLastN);
          if (sylia_arg_count < 0)
            env->ThrowError("VirtualdubFilterProxy: arguments (after first) must be integers, double and strings only");

          // was: i->func_ptr(reinterpret_cast<IVDXScriptInterpreter *>(&si), &fa, sylia_args, sylia_arg_count);
          // Casting to void retval is important, see in vdvideofilt.h VDXVideoFilterScriptAdapter:
          // static void AdaptFn(IVDXScriptInterpreter *isi, void *fa0, const VDXScriptValue *argv, int argc)
          VDXScriptVoidFunctionPtr fptr = reinterpret_cast<VDXScriptVoidFunctionPtr>(i->func_ptr);
          fptr(reinterpret_cast<IVDXScriptInterpreter *>(&si), &fa, sylia_args, sylia_arg_count);
          return;
        }
      }
      env->ThrowError("VirtualdubFilterProxy: no matching config function (this shouldn't happen)");
    }
  }

  ~VirtualdubFilterProxy() {
    _RPT3(0, "VirtualdubFilterProxy: destructor called fdl=%p MyFilterModule=%p fd=%p\r\n", fdl, fdl->myFilterModule, fd);
    CallEndProc();
    _RPT4(0, "CallEndProc done fdl=%p MyFilterModule=%p fd=%p, instanceNo=%d\r\n", fdl, fdl->myFilterModule, fd, fdl->myFilterModule->refcounter);
    // filter activation is per instance
    if (fa.filter_data) {
      delete[] (char *)fa.filter_data; // PF 180417 190409 cast to char * before free (gcc warning: delete on void *)
      fa.filter_data = nullptr;
      fma.filter_data = nullptr; // fma had no separate filter data, just a same pointer
    }
    fdl->myFilterModule->refcounter--;
    // don't delete until we reach the last filter instance
    if (fdl->myFilterModule->refcounter == 0) {
      FreeFilterModule(fdl->myFilterModule);
      _RPT1(0, "FreeFilterModule done fd:%p\r\n", fd);
    }
    if (vbSrc.hdc)
      ReleaseDC(NULL, (HDC)vbSrc.hdc);
  }

  void __cdecl FreeFilterModule(MyVDXFilterModule* fm) {
    for (FilterDefinitionList* fdl = fm->fdl; fdl; fdl = fdl->fdl) {
      _RPT1(0, "VDub: FilterDefinition to be freed, fd: %p \r\n", fdl->fd);
      delete fdl->fd;
      fdl->fd = nullptr;
    }

    fm->deinitProc(fm, &g_VDFilterCallbacks);
    FreeLibrary((HMODULE)fm->hInstModule);
    if (fm->prev)
      fm->prev->next = fm->next;
    if (fm->next)
      fm->next->prev = fm->prev;
    delete fm;
  }

  static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env) {
    FilterDefinitionList* fdl = (FilterDefinitionList*)user_data;
    _RPT1(0, "VDub: create new VirtualdubFilterProxy with fdl=%p\r\n", fdl);

    VideoInfo vi = (args[0].AsClip())->GetVideoInfo();

    PClip clip = args[0].AsClip();

    AVSValue new_args[1] = { clip };

    // AutoConvert RGB24/48, and 8/16 bit Planar RGBAs to RGB32/64
    // RGB24, RGBP8, RGBAP8 -> RGB32
    // RGB48, RGBP16, RGBAP16 -> RGB64
    if (vi.BitsPerComponent() == 8 && vi.IsPlanarRGB()) {
      clip = env->Invoke("ConvertToRGB24", AVSValue(new_args, 1)).AsClip();
    }
    else if (vi.BitsPerComponent() == 8 && vi.IsPlanarRGBA()) {
      clip = env->Invoke("ConvertToRGB32", AVSValue(new_args, 1)).AsClip();
    }
    else if (vi.IsRGB48() || (vi.BitsPerComponent() == 16 && (vi.IsPlanarRGB() || vi.IsPlanarRGBA()))) {
      clip = env->Invoke("ConvertToRGB64", AVSValue(new_args, 1)).AsClip();
    }

    clip = new VirtualdubFilterProxy(clip, fdl, args, env);

    AVSValue new_args2[1] = { clip };
    if (vi.IsPlanarRGB()) {
      clip = env->Invoke("ConvertToPlanarRGB", AVSValue(new_args2, 1)).AsClip();
    }
    else if (vi.IsPlanarRGBA()) {
      clip = env->Invoke("ConvertToPlanarRGBA", AVSValue(new_args2, 1)).AsClip();
    }
    else if (vi.IsRGB48()) {
      clip = env->Invoke("ConvertToRGB48", AVSValue(new_args2, 1)).AsClip();
    }

    return clip;
  }
};


// helper for FilterModAdd and FilterAdd
static void FilterBaseAdd_CreateAvsFnByParams(MyVDXFilterModule * fm2, FilterDefinitionList * fdl, VDXFilterDefinition2 * fd)
{
  const int MAX_PARAMS = 64;
  char converted_paramlist[MAX_PARAMS + 1];

  // send actual filter definition list as 'user_data' for create
  // add optinal rangehint parameter, which is seen only by avisynth.
  // See also skipLastN references, which should match the number of extra parameters here
  fm2->env->AddFunction(fm2->avisynth_function_name, "c", VirtualdubFilterProxy::Create, fdl);
  if (fd->script_obj && fd->script_obj->func_list) {
    for (VDXScriptFunctionDef* i = fd->script_obj->func_list; i->arg_list; i++) {
      // avisynth does not know 'd'ouble or 'l'ong
      // let's fake them to 'f'loat and 'i'nt for avisynth
      const char *p_src = i->arg_list + 1;
      char *p_target = converted_paramlist;
      char ch;
      while ((ch = *p_src++) && (p_target - converted_paramlist)<MAX_PARAMS) {
        if (ch == 'd') ch = 'f';
        else if (ch == 'l') ch = 'i';
        *p_target++ = ch;
      }
      *p_target = '\0';

      // put * if . found
      const char* params = fm2->env->Sprintf("c%s%s[rangehint]s", converted_paramlist, strchr(i->arg_list + 1, '.') ? "*" : "");
      // send actual filter definition list as 'user_data' for create
      fm2->env->AddFunction(fm2->avisynth_function_name, params, VirtualdubFilterProxy::Create, fdl);
    }
  }
}

// FilterAdd and FilterModAdd are callbacks
// Each DLL (vdf filter group) informs the host about their available internal filters
// These internal filters of a FilterModule are listed in the fdl (FilterDefinitionList) inside each fm (FilterModule)

VDXFilterDefinition *VDcall FilterAdd(VDXFilterModule *fm, VDXFilterDefinition *pfd, int fd_len) {
  VDXFilterDefinition2 *fd = new(std::nothrow) VDXFilterDefinition2;

  MyVDXFilterModule *fm2 = reinterpret_cast<MyVDXFilterModule *>(fm);
  _RPT1(0, "VDXFilterDefinition created for %s\r\n", fm2->avisynth_function_name);

  FilterDefinitionList _fdl(fm2, fd);
  // saving the fdl to the safe String-store
  FilterDefinitionList *fdl = (FilterDefinitionList*)(fm2->env->SaveString((const char*)&_fdl, sizeof(_fdl)));
  fm2->fdl = fdl;

  if (fd) {
    memcpy(fd, pfd, min(size_t(fd_len), sizeof(VDXFilterDefinition)));

    fd->_module = NULL; // fd->module  = fm2; // deprecated, set to null
    fd->_prev = NULL;
    fd->_next = NULL;
  }

  FilterBaseAdd_CreateAvsFnByParams(fm2, fdl, fd);

  return fd;
}

VDXFilterDefinition *VDcall FilterModAdd(VDXFilterModule *fm, VDXFilterDefinition *pfd, int fd_len, FilterModDefinition *pfm, int fm_len) {
  VDXFilterDefinition2 *fd = new(std::nothrow) VDXFilterDefinition2;

  MyVDXFilterModule *fm2 = reinterpret_cast<MyVDXFilterModule *>(fm); // cast to access our extra fields
  _RPT1(0, "VDXFilterDefinition2 created for %s\r\n", fm2->avisynth_function_name);

  FilterDefinitionList _fdl(fm2, fd);
  // saving the fdl to the safe String-store
  FilterDefinitionList *fdl = (FilterDefinitionList*)(fm2->env->SaveString((const char*)&_fdl, sizeof(_fdl)));
  fm2->fdl = fdl;

  if (fd) {
    memcpy(fd, pfd, min(size_t(fd_len), sizeof(VDXFilterDefinition)));
    // copy additional function pointer fields of FilterModDefinition
    memcpy(&fd->fm, &pfm, min(size_t(fm_len), sizeof(FilterModDefinition)));

    fd->_module = NULL; // deprecated, set to null
    fd->_prev = NULL;
    fd->_next = NULL;
  }

  FilterBaseAdd_CreateAvsFnByParams(fm2, fdl, fd);

  return fd;
}

AVSValue __cdecl LoadVirtualdubPlugin(AVSValue args, void*, IScriptEnvironment* env) {
  const char* const szModule = args[0].AsString();
  const char* const avisynth_function_name = args[1].AsString();
  const int preroll = args[2].AsInt(0);

  HMODULE hmodule = LoadLibrary(szModule);
  if (!hmodule)
    env->ThrowError("LoadVirtualdubPlugin: Error opening \"%s\", error=0x%x", szModule, GetLastError());

  VDXFilterModuleInitProc initProc   = (VDXFilterModuleInitProc  )GetProcAddress(hmodule, "VirtualdubFilterModuleInit2");
  VDXFilterModuleDeinitProc deinitProc = (VDXFilterModuleDeinitProc)GetProcAddress(hmodule, "VirtualdubFilterModuleDeinit");

  FilterModModuleInitProc filterModInitProc = (FilterModModuleInitProc)GetProcAddress(hmodule, "FilterModModuleInit");

  if (!initProc || !deinitProc) {
    FreeLibrary(hmodule);
    env->ThrowError("LoadVirtualdubPlugin: Module \"%s\" does not contain VirtualDub filters.", szModule);
  }

  VDXFilterModule* loaded_modules = 0;
  try {
    loaded_modules = (VDXFilterModule*)env->GetVar("$LoadVirtualdubPlugin$").AsString();
  }
  catch (IScriptEnvironment::NotFound) {}

  for (VDXFilterModule* i = loaded_modules; i; i = i->next) {
    if (i->hInstModule == (VDXHINSTANCE)hmodule) {
      FreeLibrary(hmodule);
      return AVSValue();
    }
  }

  // For each different virtualdub dlls (vfd) one MyVDXFilterModule fm is created
  // which contains necessary callbacks.
  // The 'next' field of MyVDXFilterModule will link to the previous vdf entry.
  MyVDXFilterModule* fm = new MyVDXFilterModule;
  fm->refcounter = 0; // counts multiple instances of a filter

  fm->hInstModule = (VDXHINSTANCE)hmodule;
  fm->initProc = initProc;
  fm->deinitProc = deinitProc;
  fm->filterModInitProc = filterModInitProc;

  fm->env = env;
  fm->avisynth_function_name = avisynth_function_name;
  fm->preroll = preroll;

  // chain previous tree
  fm->next = loaded_modules;
  fm->prev = 0;

  fm->fdl = 0; // no filter definition list yet

  fm->filterdef_ver_hi = VIRTUALDUB_FILTERDEF_VERSION;
  fm->filterdef_ver_lo = VIRTUALDUB_FILTERDEF_COMPATIBLE;
  fm->filtermod_ver_hi = 0;
  fm->filtermod_ver_lo = 0;

  int errorNo = 0;

  // trying filterModInitProc
  if (filterModInitProc) {
    fm->filtermod_ver_lo = FILTERMOD_VERSION;
    fm->filtermod_ver_hi = FILTERMOD_VERSION;

    if (filterModInitProc(fm, &g_FilterModCallbacks, fm->filterdef_ver_hi, fm->filterdef_ver_lo, fm->filtermod_ver_hi, fm->filtermod_ver_lo)) {
      errorNo = 1;
    }
    else {
      _RPT5(0, "VDub2 filter added %s FILTERDEF_VER compat:%d hi:%d, FILTERMOD_VER lo:%d hi:%d \r\n",
        szModule, fm->filterdef_ver_lo, fm->filterdef_ver_hi, fm->filtermod_ver_lo, fm->filtermod_ver_hi);
      // Check if the plugin's minimal requirement is higher than used by this Avs+ simulation
      if (fm->filtermod_ver_lo > FILTERMOD_VERSION) {
        fm->deinitProc(fm, &g_VDFilterCallbacks);
        errorNo = 2;
      }
    }
  }
  else
  {
    if (fm->initProc(fm, &g_VDFilterCallbacks, fm->filterdef_ver_hi, fm->filterdef_ver_lo)) {
      errorNo = 3;
    }
    else {
      _RPT3(0, "VDub oldstyle filter added %s FILTERDEF_VER compat:%d hi:%d\r\n", szModule, fm->filterdef_ver_lo, fm->filterdef_ver_hi);
    }
  }

  if (errorNo > 0) {
    // Neuter any AVS functions that may have been created
    for (FilterDefinitionList* fdl = fm->fdl; fdl; fdl = fdl->fdl) {
      delete fdl->fd;
      fdl->fd = 0;
    }
    FreeLibrary(hmodule);
    delete fm;
    switch (errorNo) {
    case 1: env->ThrowError("LoadVirtualdubPlugin (mod): Error initializing module \"%s\"", szModule);
    case 2: env->ThrowError("LoadVirtualdubPlugin (mod): This filter uses too new of a filter interface!"
      "You'll need to upgrade to a newer version of Avisynth VirtualDub plugin to use this filter \"%s\"", szModule);
    case 3: env->ThrowError("LoadVirtualdubPlugin: Error initializing module \"%s\"", szModule);
    }
  }

  // allow two directions
  if (fm->next)
    fm->next->prev = fm;

  env->SetGlobalVar("$LoadVirtualdubPlugin$", (const char*)fm);

  return AVSValue();
}

const AVS_Linkage * AVS_linkage = 0;
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
	AVS_linkage = vectors;

  // clip, base filename, start, end, image format/extension, info
  env->AddFunction("LoadVirtualdubPlugin", "ss[preroll]i", LoadVirtualdubPlugin, 0);

  return "`LoadVirtualdubPlugin' Allows to load and use filters written for VirtualDub.";
}
