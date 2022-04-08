//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2001 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_VD2_PLUGIN_VDVIDEOFILT_H
#define f_VD2_PLUGIN_VDVIDEOFILT_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <stddef.h>

#include "vdplugin.h"

//////////////////

struct VDXScriptObject;
struct VDXFilterVTbls;

//////////////////

enum {
	/// Request distinct source and destination buffers. Otherwise, the source and destination buffers
	/// alias (in-place mode).
	FILTERPARAM_SWAP_BUFFERS		= 0x00000001L,

	/// Request an extra buffer for the previous source frame.
	FILTERPARAM_NEEDS_LAST			= 0x00000002L,

	/// Filter supports image formats other than RGB32. Filters that support format negotiation must
	/// set this flag for all calls to paramProc.
	///
	/// (API V16 - Now required)
	FILTERPARAM_SUPPORTS_ALTFORMATS	= 0x00000004L,

	/// Filter requests 16 byte alignment for source and destination buffers. This guarantees that:
	///
	///		- data and pitch fields are multiples of 16 bytes (aligned)
	///		- an integral number of 16 byte vectors may be read, even if the last vector includes
	///		  some bytes beyond the end of the scanline (their values are undefined)
	///		- an integral number of 16 byte vectors may be written, even if the last vector includes
	///		  some bytes beyong the end of the scanline (their values are ignored)
	///
	FILTERPARAM_ALIGN_SCANLINES_16		= 0x00000008L,
	FILTERPARAM_ALIGN_SCANLINES			= FILTERPARAM_ALIGN_SCANLINES_16,

	///		- same with 32,64 bytes alignment
	///
	FILTERPARAM_ALIGN_SCANLINES_32		= 0x00000048L,  // v19
	FILTERPARAM_ALIGN_SCANLINES_64		= 0x00000040L,  // v19

	/// Filter's output is purely a function of configuration parameters and source image data, and not
	/// source or output frame numbers. In other words, two output frames produced by a filter instance
	/// can be assumed to be identical images if:
	///
	///		- the same number of source frames are prefetched
	///		- the same type of prefetches are performed (direct vs. non-direct)
	///		- the frame numbers for the two prefetch lists, taken in order, correspond to identical
	///		  source frames
	///		- the prefetch cookies match
	///
	/// Enabling this flag improves the ability of the host to identify identical frames and drop them
	/// in preview or in the output file.
	///
	FILTERPARAM_PURE_TRANSFORM		= 0x00000010L,

	/// Filter requests that 16-bits input bitmap is normalized. This guarantees that info.ref_r..info.ref_a attributes are 0xFFFF when applicable.
	///
	FILTERPARAM_NORMALIZE16		= 0x00000020L,

	/// Filter cannot support the requested source format. Note that this sets all bits, so the meaning
	/// of other bits is ignored. The one exception is that FILTERPARAM_SUPPORTS_ALTFORMATS is assumed
	/// to be implicitly set.
	FILTERPARAM_NOT_SUPPORTED		= (long)0xFFFFFFFF,

	/// Filter requires that image is not modified further and cpu time is not wasted.
	/// Intended for "analyzis" filters.
	FILTERPARAM_TERMINAL	= 0x00000080L,  // v19
};

/// The filter has a delay from source to output. For instance, a lag of 3 indicates that the
/// filter internally buffers three frames, so when it is fed frames in sequence, frame 0 emerges
/// after frame 3 has been processed. The host attempts to correct timestamps in order to compensate.
///
/// VirtualDub 1.9.1 or later: Setting this flag can have a performance penalty, as it causes the host
/// to request additional frames to try to produce the correct requested output frames.
///
#define FILTERPARAM_HAS_LAG(frames) ((int)(frames) << 16)

///////////////////

class VDXFBitmap;
class VDXFilterActivation;
struct VDXFilterFunctions;
class FilterModActivation;
struct FilterModInitFunctions;
struct VDXFilterModule;
struct VDXFilterDefinition;
class IVDXVideoPrefetcher;
class IVDXAContext;

enum {
	kVDXVFEvent_None				= 0,
	kVDXVFEvent_InvalidateCaches	= 1
};

typedef int  (__cdecl *VDXFilterInitProc     )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef void (__cdecl *VDXFilterDeinitProc   )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef int  (__cdecl *VDXFilterRunProc      )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef long (__cdecl *VDXFilterParamProc    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef int  (__cdecl *VDXFilterConfigProc   )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, VDXHWND hWnd);
typedef void (__cdecl *VDXFilterStringProc   )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf);
typedef int  (__cdecl *VDXFilterStartProc    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef int  (__cdecl *VDXFilterEndProc      )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef bool (__cdecl *VDXFilterScriptStrProc)(VDXFilterActivation *fa, const VDXFilterFunctions *, char *, int);
typedef void (__cdecl *VDXFilterStringProc2  )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf, int maxlen);
typedef int  (__cdecl *VDXFilterSerialize    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf, int maxbuf);
typedef void (__cdecl *VDXFilterDeserialize  )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, const char *buf, int maxbuf);
typedef void (__cdecl *VDXFilterCopy         )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, void *dst);
typedef sint64 (__cdecl *VDXFilterPrefetch   )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, sint64 frame);
typedef void (__cdecl *VDXFilterCopy2Proc    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, void *dst, VDXFilterActivation *fa2, const VDXFilterFunctions *ff2);
typedef bool (__cdecl *VDXFilterPrefetch2Proc)(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, sint64 frame, IVDXVideoPrefetcher *prefetcher);
typedef bool (__cdecl *VDXFilterEventProc	 )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, uint32 event, const void *eventData);
typedef void (__cdecl *VDXFilterAccelRunProc )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef void (__cdecl *FilterModActivateProc )(FilterModActivation *fma, const VDXFilterFunctions *ff);
typedef long (__cdecl *FilterModParamProc    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);

typedef int (__cdecl *VDXFilterModuleInitProc)(VDXFilterModule *fm, const VDXFilterFunctions *ff, int& vdfd_ver, int& vdfd_compat);
typedef int (__cdecl *FilterModModuleInitProc)(VDXFilterModule *fm, const FilterModInitFunctions *ff, int& vdfd_ver, int& vdfd_compat, int& mod_ver, int& mod_min);
typedef void (__cdecl *VDXFilterModuleDeinitProc)(VDXFilterModule *fm, const VDXFilterFunctions *ff);

//////////

typedef void (__cdecl *VDXFilterPreviewButtonCallback)(bool fNewState, void *pData);
typedef void (__cdecl *VDXFilterPreviewSampleCallback)(VDXFBitmap *, long lFrame, long lCount, void *pData);

class IFilterModPreviewSample {
public:
	// Run returns combination of these flags
	enum {
		result_image = 1, // destination image can be displayed in preview window
	};

	virtual int Run(const VDXFilterActivation *fa, const VDXFilterFunctions *ff)=0;
	virtual void GetNextFrame(sint64 frame, sint64* next_frame, sint64* total_count)=0;
	virtual void Cancel()=0;
};

// avs+: have to set __cdecl for x86
class IVDXFilterPreview {
public:
	virtual void __cdecl SetButtonCallback(VDXFilterPreviewButtonCallback, void *)=0;
	virtual void __cdecl SetSampleCallback(VDXFilterPreviewSampleCallback, void *)=0;

	virtual bool __cdecl isPreviewEnabled()=0;
	virtual void __cdecl Toggle(VDXHWND)=0;
	virtual void __cdecl Display(VDXHWND, bool)=0;
	virtual void __cdecl RedoFrame()=0;
	virtual void __cdecl RedoSystem()=0;
	virtual void __cdecl UndoSystem()=0;
	virtual void __cdecl InitButton(VDXHWND)=0;
	virtual void __cdecl Close()=0;
	virtual bool __cdecl SampleCurrentFrame()=0;
	virtual long __cdecl SampleFrames()=0;
};

struct PreviewZoomInfo {
	int version;
	enum {
		popup_update = 1,
		popup_cancel = 2,
		popup_click =  4,
	};
	int flags;
	int x,y;
	float r,g,b,a;

	PreviewZoomInfo() { version = 0; }
};

struct PreviewExInfo {
	int version;
	enum {
		thick_border = 1,
		custom_draw = 2,
		display_source = 4,
		no_exit = 8,
	};
	int flags;

	PreviewExInfo() { version = 0; flags = 0; }
};

struct ClipEditInfo {
	int version;
	enum {
		edit_update = 1,
		edit_finish = 2,
		fill_border = 4,
		init_size = 8,
		edit_time_range = 16,
	};
	int flags;
	int x1,y1,x2,y2;
	int w,h;

	ClipEditInfo() { version = 0; flags = 0; }
};

typedef void (__cdecl *FilterModPreviewPositionCallback)(int64 pos, void *pData);
typedef void (__cdecl *FilterModPreviewZoomCallback)(PreviewZoomInfo& info, void *pData);
typedef void (__cdecl *FilterModPreviewClipEditCallback)(ClipEditInfo& info, void *pData);
struct tagMSG;

class IVDXFilterPreview2 : public IVDXFilterPreview {
public:
	virtual bool __cdecl IsPreviewDisplayed() = 0;
};

class IFilterModPreview {
public:
	virtual int64 FMSetPosition(int64 pos)=0;
	virtual void FMSetPositionCallback(FilterModPreviewPositionCallback, void *)=0;
	virtual void FMSetZoomCallback(FilterModPreviewZoomCallback, void *)=0;

	// FilterModVersion>=4
	virtual int FMTranslateAccelerator(tagMSG* msg)=0;

	// FilterModVersion>=5
	virtual long SampleFrames(IFilterModPreviewSample*)=0;

	// new
	virtual void DisplayEx(VDXHWND, PreviewExInfo& info)=0;
	virtual void SetClipEdit(ClipEditInfo& info) = 0;
	virtual void SetClipEditCallback(FilterModPreviewClipEditCallback, void *) = 0;
};

class IFilterModTimeline {
public:
	virtual int64 GetTimelinePos()=0;
	virtual int64 TimelineToFilterSource(int64 frame)=0;
	virtual int64 FilterSourceToTimeline(int64 frame)=0;
};

class FilterReturnInfo {
public:
	virtual void setName(const char* s)=0;
	virtual void setDesc(const char* s)=0;
	virtual void setMaker(const char* s)=0;
	virtual void setModulePath(const wchar_t* s)=0;
	virtual void setBuiltinDef(const VDXFilterDefinition* desc)=0;
};

class IFilterModSystem {
public:
	virtual bool CreateVideoFilter(VDXHWND hParent, FilterReturnInfo& a)=0;
	virtual bool FindVideoFilter(const char* name, FilterReturnInfo& a)=0;
};

class IFilterModProject {
public:
	virtual bool GetData(void* buf, size_t* buf_size, const wchar_t* id)=0;
	virtual bool SetData(const void* buf, const size_t buf_size, const wchar_t* id)=0;
	virtual bool GetProjectData(void* buf, size_t* buf_size, const wchar_t* id)=0;
	virtual bool SetProjectData(const void* buf, const size_t buf_size, const wchar_t* id)=0;
	virtual bool GetDataDir(wchar_t* buf, size_t* buf_size)=0;
	virtual bool GetProjectDir(wchar_t* buf, size_t* buf_size)=0;
	// FilterModVersion>=6
	virtual bool GetMainSource(wchar_t* buf, size_t* buf_size)=0;
};

class IVDXVideoPrefetcher : public IVDXUnknown {
public:
	enum { kIID = VDXMAKEFOURCC('X', 'v', 'p', 'f') };

	/// Request a video frame fetch from an upstream source.
	virtual void VDXAPIENTRY PrefetchFrame(sint32 srcIndex, sint64 frame, uint64 cookie) = 0;

	/// Request a video frame fetch from an upstream source in direct mode.
	/// This specifies that the output frame is the same as the input frame.
	/// There cannot be more than one direct fetch and there must be no standard
	/// fetches at the same time. There can, however, be symbolic fetches.
	virtual void VDXAPIENTRY PrefetchFrameDirect(sint32 srcIndex, sint64 frame) = 0;

	/// Request a symbolic fetch from a source. This does not actually fetch
	/// any frames, but marks an association from source to output. This is
	/// useful for indicating the approximate center of where an output derives
	/// in a source, even if those frames aren't fetched (perhaps due to caching).
	/// There may be either zero or one symbolic fetch per source.
	///
	/// If no symbolic fetches are performed, the symbolic frame is assumed to
	/// be the rounded mean of the fetched source frames.
	virtual void VDXAPIENTRY PrefetchFrameSymbolic(sint32 srcIndex, sint64 frame) = 0;
};

//////////

enum {
	// This is the highest API version supported by this header file.
	VIRTUALDUB_FILTERDEF_VERSION		= 20,

	// This is the absolute lowest API version supported by this header file.
	// Note that V4 is rather old, corresponding to VirtualDub 1.2.
	// Chances are you will need to declare a higher version.
	VIRTUALDUB_FILTERDEF_COMPATIBLE		= 4,

	// API V9 is a slightly saner baseline, since it is the first API
	// version that has copy constructor support. You may still need to
	// declare a higher vdfd_compat version in your module init if you
	// need features beyond V9 (VirtualDub 1.4.12).
	VIRTUALDUB_FILTERDEF_COMPATIBLE_COPYCTOR = 9,

	// API V17 was last before vdubFM
	VIRTUALDUB_OFFICIAL		= 17,
};

// v3: added lCurrentSourceFrame to FrameStateInfo
// v4 (1.2): lots of additions (VirtualDub 1.2)
// v5 (1.3d): lots of bugfixes - stretchblt bilinear, and non-zero startproc
// v6 (1.4): added error handling functions
// v7 (1.4d): added frame lag, exception handling
// v8 (1.4.11): added string2 proc
// v9 (1.4.12): added (working) copy constructor
// v10 (1.5.10): added preview flag
// v11 (1.7.0): guaranteed src structure setup before configProc; added IVDFilterPreview2
// v12 (1.7.4): support for frame alteration
// v13 (1.8.2): added mOutputFrame field to VDXFilterStateInfo
// v14 (1.9.1): added copyProc2, prefetchProc2, input/output frame arrays
// v15 (1.9.3): added VDXA support
// v16 (1.10.x): added multi-source support, feature deprecation
// v17: added mpStaticAboutProc
// v18: added FilterModActivation
// v19: added flags

struct FilterModDefinition;
/*
struct FilterModDefinition {
  FilterModActivateProc		activateProc;
  FilterModParamProc		  paramProc;
  // avs+:
  //FilterModDefinition(FilterModActivateProc _activateProc, FilterModParamProc _paramProc) : activateProc(_activateProc), paramProc(_paramProc) {};
};
*/
struct VDXFilterDefinition {
	void *_next;		// deprecated - set to NULL
	void *_prev;		// deprecated - set to NULL
	void *_module;		// deprecated - set to NULL

	const char *		name;
	const char *		desc;
	const char *		maker;
	void *				private_data;
	int					inst_data_size;

	VDXFilterInitProc		initProc;
	VDXFilterDeinitProc		deinitProc;
	VDXFilterRunProc		runProc;
	VDXFilterParamProc		paramProc;
	VDXFilterConfigProc		configProc;
	VDXFilterStringProc		stringProc;			// DEPRECATED - ignored as of v16; use stringProc2
	VDXFilterStartProc		startProc;
	VDXFilterEndProc		endProc;

	VDXScriptObject			*script_obj;

	VDXFilterScriptStrProc	fssProc;

	// NEW - 1.4.11
	VDXFilterStringProc2	stringProc2;
	VDXFilterSerialize		serializeProc;
	VDXFilterDeserialize	deserializeProc;
	VDXFilterCopy			copyProc;			// DEPRECATED - ignored as of v16; use copyProc2

	VDXFilterPrefetch		prefetchProc;		// (V12/V1.7.4+)

	// NEW - V14 / 1.9.1
	VDXFilterCopy2Proc		copyProc2;
	VDXFilterPrefetch2Proc	prefetchProc2;
	VDXFilterEventProc		eventProc;

	// NEW - V15 / 1.9.3
	VDXFilterAccelRunProc	accelRunProc;

	// NEW - V16 / 1.10.1
	sint32					mSourceCountLowMinus1;
	sint32					mSourceCountHighMinus1;

	// NEW - V17 / 1.10.2
	VDXShowStaticAboutProc		mpStaticAboutProc;
	VDXShowStaticConfigureProc	mpStaticConfigureProc;

	// NEW - V20
	FilterModDefinition* fm;
};

struct FilterModDefinition {
	FilterModActivateProc		activateProc;
	FilterModParamProc		  paramProc;
};

//////////

// FilterStateInfo: contains dynamic info about file being processed

class VDXFilterStateInfo {
public:
	sint32	lCurrentFrame;				// current sequence frame (previously called output frame)
	sint32	lMicrosecsPerFrame;			// microseconds per sequence frame
	sint32	lCurrentSourceFrame;		// current source frame
	sint32	lMicrosecsPerSrcFrame;		// microseconds per source frame
	sint32	lSourceFrameMS;				// source frame timestamp
	sint32	lDestFrameMS;				// output frame timestamp

	enum {
		kStateNone		= 0x00000000,
		kStatePreview	= 0x00000001,	// (V1.5.10+) Job output is not being saved to disk.
		kStateRealTime	= 0x00000002,	// (V1.5.10+) Operation is running in real-time (capture, playback).
		kStateMax		= 0xFFFFFFFF
	};

	uint32	flags;

	sint32	mOutputFrame;				// (V13/V1.8.2+) current output frame
};

// VDXFBitmap: VBitmap extended to hold filter-specific information

class VDXBitmap {
public:
	void *			_vtable;	// Reserved - do not use.
	uint32 *		data;		// Pointer to start of _bottom-most_ scanline of plane 0.
	uint32 *		palette;	// Pointer to palette (reserved - set to NULL).
	sint32			depth;		// Bit depth, in bits. Set to zero if mpPixmap/mpPixmapLayout are active.
	sint32			w;			// Width of bitmap, in pixels.
	sint32			h;			// Height of bitmap, in pixels.
	ptrdiff_t		pitch;		// Distance, in bytes, from the start of one scanline in plane 0 to the next.
	ptrdiff_t		modulo;		// Distance, in bytes, from the end of one scanline in plane 0 to the start of the next.
	ptrdiff_t		size;		// Size of plane 0, including padding.
	ptrdiff_t		offset;		// Offset from beginning of buffer to beginning of plane 0.

	uint32 *Address32(int x, int y) const {
		return Address32i(x, h-y-1);
	}

	uint32 *Address32i(int x, int y) const {
		return (uint32 *)((char *)data + y*pitch + x*4);
	}

	void AlignTo4() {
		pitch = w << 2;
	}

	void AlignTo8() {
		pitch = ((w+1)&~1) << 2;
	}
};

class VDXFBitmap : public VDXBitmap {
public:
	enum {
		/// Set in paramProc if the filter requires a Win32 GDI display context
		/// for a bitmap.
		///
		/// (Deprecated as of API V12 - do not use)
		/// (Blocked as of API V16)
		NEEDS_HDC		= 0x00000001L,
	};

	uint32		dwFlags;
	VDXHDC		hdc;

	uint32	mFrameRateHi;		// Frame rate numerator (V1.7.4+)
	uint32	mFrameRateLo;		// Frame rate denominator (V1.7.4+)
	sint64	mFrameCount;		// Frame count; -1 if unlimited or indeterminate (V1.7.4+)

	VDXPixmapLayout	*mpPixmapLayout;
	const VDXPixmap	*mpPixmap;

	uint32	mAspectRatioHi;				///< Pixel aspect ratio fraction (numerator).	0/0 = unknown
	uint32	mAspectRatioLo;				///< Pixel aspect ratio fraction (denominator).

	sint64	mFrameNumber;				///< Current frame number (zero based).
	sint64	mFrameTimestampStart;		///< Starting timestamp of frame, in 100ns units.
	sint64	mFrameTimestampEnd;			///< Ending timestamp of frame, in 100ns units.
	sint64	mCookie;					///< Cookie supplied when frame was requested.

	uint32	mVDXAHandle;				///< Acceleration handle to be used with VDXA routines.
	uint32	mBorderWidth;
	uint32	mBorderHeight;
};

// VDXFilterActivation: This is what is actually passed to filters at runtime.

class VDXFilterActivation {
public:
	const VDXFilterDefinition *filter;		//
	void *filter_data;
	VDXFBitmap&	dst;
	VDXFBitmap&	src;
	VDXFBitmap	*_reserved0;
	VDXFBitmap	*const last;
	uint32		x1;
	uint32		y1;
	uint32		x2;
	uint32		y2;

	VDXFilterStateInfo	*pfsi;
	IVDXFilterPreview	*ifp;
	IVDXFilterPreview2	*ifp2;			// (V11+)

	uint32		mSourceFrameCount;		// (V14+)
	VDXFBitmap *const *mpSourceFrames;	// (V14+)
	VDXFBitmap *const *mpOutputFrames;	// (V14+)

	IVDXAContext	*mpVDXA;			// (V15+)

	uint32		mSourceStreamCount;		// (V16+)
	VDXFBitmap *const *mpSourceStreams;	// (V16+)

	FilterModActivation* fma;	// (V18+)
  // avs+:
  VDXFilterActivation(VDXFBitmap& _dst, VDXFBitmap& _src, VDXFBitmap *_last) : dst(_dst), src(_src), last(_last) {};
};

enum {
	// This is the highest API version supported by this header file.
	FILTERMOD_VERSION = 6,
};

class FilterModActivation {
public:
	const VDXFilterDefinition *filter;
	const FilterModDefinition *filterMod;
	void *filter_data;
	IFilterModPreview *fmpreview;
	IFilterModTimeline *fmtimeline;
	IFilterModSystem *fmsystem;

	// FilterModVersion>=2
	IFilterModPixmap *fmpixmap;

	// FilterModVersion>=3
	IFilterModProject *fmproject;
};

// These flags must match those in cpuaccel.h!

#ifndef f_VIRTUALDUB_CPUACCEL_H
#define CPUF_SUPPORTS_CPUID			(0x00000001L)
#define CPUF_SUPPORTS_FPU			(0x00000002L) //  386/486DX
#define CPUF_SUPPORTS_MMX			(0x00000004L) //  P55C, K6, PII
#define CPUF_SUPPORTS_INTEGER_SSE	(0x00000008L) //  PIII, Athlon
#define CPUF_SUPPORTS_SSE			(0x00000010L) //  PIII, Athlon XP/MP
#define CPUF_SUPPORTS_SSE2			(0x00000020L) //  PIV, K8
#define CPUF_SUPPORTS_3DNOW			(0x00000040L) //  K6-2
#define CPUF_SUPPORTS_3DNOW_EXT		(0x00000080L) //  Athlon

#define CPUF_SUPPORTS_SSE3			(0x00000100L) //  PIV+, K8 Venice
#define CPUF_SUPPORTS_SSSE3			(0x00000200L) //  Core 2
#define CPUF_SUPPORTS_SSE41			(0x00000400L) //  Penryn, Wolfdale, Yorkfield
#define CPUF_SUPPORTS_AVX			(0x00000800L) //  Sandy Bridge, Bulldozer
#define CPUF_SUPPORTS_SSE42			(0x00001000L) //  Nehalem

// VirtualDubFilterMod specific, identical to AVS+
#define CPUF_SUPPORTS_AVX2			(0x00002000L) //  Haswell
#define CPUF_SUPPORTS_FMA3			(0x00004000L)
#define CPUF_SUPPORTS_F16C			(0x00008000L)
#define CPUF_SUPPORTS_MOVBE			(0x00010000L) // Big Endian move
#define CPUF_SUPPORTS_POPCNT		(0x00020000L)
#define CPUF_SUPPORTS_AES			(0x00040000L)
#define CPUF_SUPPORTS_FMA4			(0x00080000L)

#define CPUF_SUPPORTS_AVX512F		(0x00100000L) // AVX-512 Foundation.
#define CPUF_SUPPORTS_AVX512DQ		(0x00200000L) // AVX-512 DQ (Double/Quad granular) Instructions
#define CPUF_SUPPORTS_AVX512PF		(0x00400000L) // AVX-512 Prefetch
#define CPUF_SUPPORTS_AVX512ER		(0x00800000L) // AVX-512 Exponential and Reciprocal
#define CPUF_SUPPORTS_AVX512CD		(0x01000000L) // AVX-512 Conflict Detection
#define CPUF_SUPPORTS_AVX512BW		(0x02000000L) // AVX-512 BW (Byte/Word granular) Instructions
#define CPUF_SUPPORTS_AVX512VL		(0x04000000L) // AVX-512 VL (128/256 Vector Length) Extensions
#define CPUF_SUPPORTS_AVX512IFMA	(0x08000000L) // AVX-512 IFMA integer 52 bit
#define CPUF_SUPPORTS_AVX512VBMI	(0x10000000L) // AVX-512 VBMI
#endif

struct VDXFilterFunctions {
	VDXFilterDefinition *(__cdecl *addFilter)(VDXFilterModule *, VDXFilterDefinition *, int fd_len);
	void __declspec(deprecated) (__cdecl *removeFilter)(VDXFilterDefinition *);
	bool (__cdecl *isFPUEnabled)();
	bool (__cdecl *isMMXEnabled)();
	void (__cdecl *InitVTables)(VDXFilterVTbls *);

	// These functions permit you to throw MyError exceptions from a filter.
	// YOU MUST ONLY CALL THESE IN runProc, initProc, and startProc.

	void (__cdecl *ExceptOutOfMemory)();						// ADDED: V6 (VirtualDub 1.4)
	void (__cdecl *Except)(const char *format, ...);			// ADDED: V6 (VirtualDub 1.4)

	// These functions are callable at any time.

	long (__cdecl *getCPUFlags)();								// ADDED: V6 (VirtualDub 1.4)
	long (__cdecl *getHostVersionInfo)(char *buffer, int len);	// ADDED: V7 (VirtualDub 1.4d)
};

struct FilterModInitFunctions {
	VDXFilterDefinition *(__cdecl *addFilter)(VDXFilterModule *, VDXFilterDefinition *, int fd_len, FilterModDefinition*, int md_len);
};



///////////////////////////////////////////////////////////////////////////

class VDXScriptValue;
class VDXScriptError;
struct VDXScriptObject;

class VDXScriptError {
public:
	enum {
		PARSE_ERROR=1,
		SEMICOLON_EXPECTED,
		IDENTIFIER_EXPECTED,

		TYPE_INT_REQUIRED,
		TYPE_ARRAY_REQUIRED,
		TYPE_FUNCTION_REQUIRED,
		TYPE_OBJECT_REQUIRED,

		OBJECT_MEMBER_NAME_REQUIRED,
		FUNCCALLEND_EXPECTED,
		TOO_MANY_PARAMS,
		DIVIDE_BY_ZERO,
		VAR_NOT_FOUND,
		MEMBER_NOT_FOUND,
		OVERLOADED_FUNCTION_NOT_FOUND,
		IDENT_TOO_LONG,
		OPERATOR_EXPECTED,
		CLOSEPARENS_EXPECTED,
		CLOSEBRACKET_EXPECTED,

		VAR_UNDEFINED,

		OUT_OF_STRING_SPACE,
		OUT_OF_MEMORY,
		INTERNAL_ERROR,
		EXTERNAL_ERROR,

		FCALL_OUT_OF_RANGE,
		FCALL_INVALID_PTYPE,
		FCALL_UNKNOWN_STR,

		ARRAY_INDEX_OUT_OF_BOUNDS,

		NUMERIC_OVERFLOW,
		STRING_NOT_AN_INTEGER_VALUE,
		STRING_NOT_A_REAL_VALUE,

		ASSERTION_FAILED,
		AMBIGUOUS_CALL,
		CANNOT_CAST
	};
};

class IVDXScriptInterpreter {
public:
	virtual	void _placeholder1() {}
	virtual void _placeholder2(void *, void *) {}
	virtual void _placeholder3(char *s) {}

	virtual void ScriptError(int e)=0;
	virtual void _placeholder4(VDXScriptError& cse) {}
	virtual char** AllocTempString(long l)=0;

	virtual void _placeholder5() {}
};

#define EXT_SCRIPT_ERROR(x)	(isi->ScriptError((VDXScriptError::x)))

typedef VDXScriptValue (*VDXScriptFunctionPtr)(IVDXScriptInterpreter *, void *, const VDXScriptValue *, int);
typedef void (*VDXScriptVoidFunctionPtr)(IVDXScriptInterpreter *, void *, const VDXScriptValue *, int);
typedef int (*VDXScriptIntFunctionPtr)(IVDXScriptInterpreter *, void *, const VDXScriptValue *, int);

struct VDXScriptFunctionDef {
	VDXScriptFunctionPtr func_ptr;
	const char *name;
	const char *arg_list;
};

struct VDXScriptObject {
	void *_lookup;							// reserved - set to NULL
	VDXScriptFunctionDef	*func_list;
	void *_obj_list;						// reserved - set to NULL
};

class VDXScriptValue {
public:
	enum { T_VOID, T_INT, T_PINT, T_STR, T_ARRAY, T_OBJECT, T_FNAME, T_FUNCTION, T_VARLV, T_LONG, T_DOUBLE } type;
	VDXScriptObject *thisPtr;
	union {
		int i;
		char **s;
		sint64 l;
		double d;
	} u;

	VDXScriptValue()					{ type = T_VOID; }
	VDXScriptValue(int i)				{ type = T_INT;			u.i = i; }
	VDXScriptValue(sint64 l)			{ type = T_LONG;		u.l = l; }
	VDXScriptValue(double d)			{ type = T_DOUBLE;		u.d = d; }
	VDXScriptValue(char **s)			{ type = T_STR;			u.s = s; }

	bool isVoid() const					{ return type == T_VOID; }
	bool isInt() const					{ return type == T_INT; }
	bool isString() const				{ return type == T_STR; }
	bool isLong() const					{ return type == T_LONG; }
	bool isDouble() const				{ return type == T_DOUBLE; }

	int		asInt() const				{ return u.i; }
	sint64	asLong() const				{ return u.l; }
	double	asDouble() const			{ return u.d; }
	char **	asString() const 			{ return u.s; }
};

#endif
