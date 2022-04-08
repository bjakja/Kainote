//	VirtualDub - Video processing and capture application
//	Plugin headers
//	Copyright (C) 1998-2007 Avery Lee, All Rights Reserved.
//
//	The plugin headers in the VirtualDub plugin SDK are licensed differently
//	differently than VirtualDub and the Plugin SDK themselves.  This
//	particular file is thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef f_VD2_PLUGIN_VDPLUGIN_H
#define f_VD2_PLUGIN_VDPLUGIN_H

#include <stddef.h>

// Copied from <vd2/system/vdtypes.h>.  Must be in sync.
#ifndef VD_STANDARD_TYPES_DECLARED
	#if defined(_MSC_VER)
		typedef signed __int64		sint64;
		typedef unsigned __int64	uint64;
	#elif defined(__GNUC__)
		typedef signed long long	sint64;
		typedef unsigned long long	uint64;
	#endif
	typedef signed int			sint32;
	typedef unsigned int		uint32;
	typedef signed short		sint16;
	typedef unsigned short		uint16;
	typedef signed char			sint8;
	typedef unsigned char		uint8;

	typedef sint64				int64;
	typedef sint32				int32;
	typedef sint16				int16;
	typedef sint8				int8;

	typedef ptrdiff_t			sintptr;
	typedef size_t				uintptr;
#endif

typedef struct VDXHINSTANCEStruct *VDXHINSTANCE;
typedef struct VDXHDCStruct *VDXHDC;
typedef struct VDXHWNDStruct *VDXHWND;

#ifndef VDXAPIENTRY
	#define VDXAPIENTRY __stdcall
#endif

#ifndef VDXAPIENTRYV
	#define VDXAPIENTRYV __cdecl
#endif

enum VDXCPUFeatureFlags {
	kVDXCPUF_CPUID		= 0x00000001,
	kVDXCPUF_MMX		= 0x00000004,
	kVDXCPUF_ISSE		= 0x00000008,
	kVDXCPUF_SSE		= 0x00000010,
	kVDXCPUF_SSE2		= 0x00000020,
	kVDXCPUF_3DNOW		= 0x00000040,
	kVDXCPUF_3DNOW_EXT	= 0x00000080,
	kVDXCPUF_SSE3		= 0x00000100,
	kVDXCPUF_SSSE3		= 0x00000200,
	kVDXCPUF_SSE41		= 0x00000400,
	kVDXCPUF_AVX		= 0x00000800
};

enum {
	kVDXPlugin_APIVersion		= 12
};


enum {
	kVDXPluginType_Video,		// Updated video filter API is not yet complete.
	kVDXPluginType_Audio,
	kVDXPluginType_Input,
	kVDXPluginType_Tool,
	kVDXPluginType_Output,
	kVDXPluginType_AudioEnc
};

typedef bool (VDXAPIENTRY *VDXShowStaticAboutProc)(VDXHWND parent);
typedef bool (VDXAPIENTRY *VDXShowStaticConfigureProc)(VDXHWND parent);

struct VDXPluginInfo {
	uint32			mSize;				// size of this structure in bytes
	const wchar_t	*mpName;
	const wchar_t	*mpAuthor;
	const wchar_t	*mpDescription;
	uint32			mVersion;			// (major<<24) + (minor<<16) + build.  1.4.1000 would be 0x010403E8.
	uint32			mType;
	uint32			mFlags;
	uint32			mAPIVersionRequired;
	uint32			mAPIVersionUsed;
	uint32			mTypeAPIVersionRequired;
	uint32			mTypeAPIVersionUsed;
	const void *	mpTypeSpecificInfo;

	// NEW
	VDXShowStaticAboutProc		mpStaticAboutProc;
	VDXShowStaticConfigureProc	mpStaticConfigureProc;
};

typedef const VDXPluginInfo *const *(VDXAPIENTRY *tpVDXGetPluginInfo)();

typedef VDXPluginInfo VDPluginInfo;
typedef tpVDXGetPluginInfo tpVDPluginInfo;

class IVDXPluginCallbacks {
public:
	virtual void * VDXAPIENTRY GetExtendedAPI(const char *pExtendedAPIName) = 0;
	virtual void VDXAPIENTRYV SetError(const char *format, ...) = 0;
	virtual void VDXAPIENTRY SetErrorOutOfMemory() = 0;
	virtual uint32 VDXAPIENTRY GetCPUFeatureFlags() = 0;
};

typedef IVDXPluginCallbacks IVDPluginCallbacks;

struct VDXPluginConfigEntry {
	enum Type {
		kTypeInvalid	= 0,
		kTypeU32		= 1,
		kTypeS32,
		kTypeU64,
		kTypeS64,
		kTypeDouble,
		kTypeAStr,
		kTypeWStr,
		kTypeBlock
	};

	const VDXPluginConfigEntry *next;

	unsigned	idx;
	uint32		type;
	const wchar_t *name;
	const wchar_t *label;
	const wchar_t *desc;
};

struct VDXRect {
	sint32	left;
	sint32	top;
	sint32	right;
	sint32	bottom;
};

namespace nsVDXPixmap {
	enum ColorSpaceMode {
		kColorSpaceMode_None,
		kColorSpaceMode_601,
		kColorSpaceMode_709,
		kColorSpaceModeCount
	};

	enum ColorRangeMode {
		kColorRangeMode_None,
		kColorRangeMode_Limited,
		kColorRangeMode_Full,
		kColorRangeModeCount
	};
};

struct FilterModPixmapInfo {
	enum MappingType {
		kTransferUnknown = 0,
		kTransferGamma = 1,
		kTransferLinear = 2,
	};
	enum AlphaType {
		kAlphaInvalid = 0,      // not present or garbage
		kAlphaMask = 1,         // arbitrary data, no default display
		kAlphaOpacity_pm = 2,   // display with transparency
		kAlphaOpacity = 3,      // display with transparency
	};

	uint32 ref_r;
	uint32 ref_g;
	uint32 ref_b;
	uint32 ref_a;

	uint32 transfer_type;
	uint32 alpha_type;
	int64 frame_num;

	// FilterModVersion>=5
	nsVDXPixmap::ColorSpaceMode colorSpaceMode;
	nsVDXPixmap::ColorRangeMode colorRangeMode;

	FilterModPixmapInfo() {
		clear();
	}

	void clear() {
		ref_r = 0;
		ref_g = 0;
		ref_b = 0;
		ref_a = 0;
		transfer_type = kTransferUnknown;
		alpha_type = kAlphaInvalid;
		frame_num = -1;
		colorSpaceMode = nsVDXPixmap::kColorSpaceMode_None;
		colorRangeMode = nsVDXPixmap::kColorRangeMode_None;
	}

	void copy_ref(const FilterModPixmapInfo& a) {
		ref_r = a.ref_r;
		ref_g = a.ref_g;
		ref_b = a.ref_b;
		ref_a = a.ref_a;
	}
	void copy_frame(const FilterModPixmapInfo& a) {
		frame_num = a.frame_num;
	}
	void copy_alpha(const FilterModPixmapInfo& a) {
		alpha_type = a.alpha_type;
	}
	void copy_dynamic(const FilterModPixmapInfo& a) {
		copy_ref(a);
		copy_frame(a);
		copy_alpha(a);
		transfer_type = a.transfer_type;
	}
};

struct VDXPixmap {
	void			*data;
	const uint32	*palette;
	sint32			w;
	sint32			h;
	ptrdiff_t		pitch;
	sint32			format;

	// Auxiliary planes are always byte-per-pixel.
	void			*data2;		// Cb (U) for YCbCr
	ptrdiff_t		pitch2;
	void			*data3;		// Cr (V) for YCbCr
	ptrdiff_t		pitch3;
};

struct VDXPixmapLayout {
	ptrdiff_t		data;
	const uint32	*palette;
	sint32			w;
	sint32			h;
	ptrdiff_t		pitch;
	sint32			format;

	// Auxiliary planes are always byte-per-pixel.
	ptrdiff_t		data2;		// Cb (U) for YCbCr
	ptrdiff_t		pitch2;
	ptrdiff_t		data3;		// Cr (V) for YCbCr
	ptrdiff_t		pitch3;
};

// Alpha extensions: safe to upcast only when format defines alpha plane

struct VDXPixmapAlpha: public VDXPixmap {
	void			*data4;
	ptrdiff_t		pitch4;
};

struct VDXPixmapLayoutAlpha: public VDXPixmapLayout {
	ptrdiff_t		data4;
	ptrdiff_t		pitch4;
};

class IFilterModPixmap {
public:
	virtual FilterModPixmapInfo* GetPixmapInfo(const VDXPixmap* pixmap)=0;
	virtual uint64 GetFormat_XRGB64()=0;
};

namespace nsVDXPixmap {
	enum VDXPixmapFormat {
		kPixFormat_Null						= 0,
		kPixFormat_XRGB1555					= 5,
		kPixFormat_RGB565					= 6,
		kPixFormat_RGB888					= 7,
		kPixFormat_XRGB8888					= 8,
		kPixFormat_Y8						= 9,
		kPixFormat_YUV422_UYVY				= 10,
		kPixFormat_YUV422_YUYV				= 11,
		kPixFormat_YUV444_Planar			= 13,
		kPixFormat_YUV422_Planar			= 14,
		kPixFormat_YUV420_Planar			= 15,
		kPixFormat_YUV411_Planar			= 16,
		kPixFormat_YUV410_Planar			= 17,
		kPixFormat_YUV422_V210				= 21,
		kPixFormat_YUV422_UYVY_709			= 22,
		kPixFormat_Y8_FR					= 24,
		kPixFormat_YUV422_YUYV_709			= 25,
		kPixFormat_YUV444_Planar_709		= 26,
		kPixFormat_YUV422_Planar_709		= 27,
		kPixFormat_YUV420_Planar_709		= 28,
		kPixFormat_YUV411_Planar_709		= 29,
		kPixFormat_YUV410_Planar_709		= 30,
		kPixFormat_YUV422_UYVY_FR			= 31,
		kPixFormat_YUV422_YUYV_FR			= 32,
		kPixFormat_YUV444_Planar_FR			= 33,
		kPixFormat_YUV422_Planar_FR			= 34,
		kPixFormat_YUV420_Planar_FR			= 35,
		kPixFormat_YUV411_Planar_FR			= 36,
		kPixFormat_YUV410_Planar_FR			= 37,
		kPixFormat_YUV422_UYVY_709_FR		= 38,
		kPixFormat_YUV422_YUYV_709_FR		= 39,
		kPixFormat_YUV444_Planar_709_FR		= 40,
		kPixFormat_YUV422_Planar_709_FR		= 41,
		kPixFormat_YUV420_Planar_709_FR		= 42,
		kPixFormat_YUV411_Planar_709_FR		= 43,
		kPixFormat_YUV410_Planar_709_FR		= 44,
		kPixFormat_YUV420i_Planar			= 45,
		kPixFormat_YUV420i_Planar_FR		= 46,
		kPixFormat_YUV420i_Planar_709		= 47,
		kPixFormat_YUV420i_Planar_709_FR	= 48,
		kPixFormat_YUV420it_Planar			= 49,
		kPixFormat_YUV420it_Planar_FR		= 50,
		kPixFormat_YUV420it_Planar_709		= 51,
		kPixFormat_YUV420it_Planar_709_FR	= 52,
		kPixFormat_YUV420ib_Planar			= 53,
		kPixFormat_YUV420ib_Planar_FR		= 54,
		kPixFormat_YUV420ib_Planar_709		= 55,
		kPixFormat_YUV420ib_Planar_709_FR	= 56,

		kPixFormat_XRGB64			= 57,
		kPixFormat_YUV444_Planar16	= 58,
		kPixFormat_YUV422_Planar16	= 59,
		kPixFormat_YUV420_Planar16	= 60,
		kPixFormat_Y16				= 61,
		kPixFormat_YUV444_Y416	= 62,
		kPixFormat_YUV444_V410	= 63,
		kPixFormat_YUV444_Y410	= 64,
		kPixFormat_R210			= 65,
		kPixFormat_R10K			= 66,
		kPixFormat_YUV444_V308	= 67,
		kPixFormat_YUV422_P210	= 68,
		kPixFormat_YUV420_P010	= 69,
		kPixFormat_YUV422_P216	= 70,
		kPixFormat_YUV420_P016	= 71,

		kPixFormat_YUV444_Alpha_Planar = 72,
		kPixFormat_YUV422_Alpha_Planar = 73,
		kPixFormat_YUV420_Alpha_Planar = 74,
		kPixFormat_YUV444_Alpha_Planar16 = 75,
		kPixFormat_YUV422_Alpha_Planar16 = 76,
		kPixFormat_YUV420_Alpha_Planar16 = 77,

		kPixFormat_YUV422_YU64 = 78,

		kPixFormat_VDXA_RGB			= 0x10001,
		kPixFormat_VDXA_YUV			= 0x10002
	};
};

#define VDXMAKEFOURCC(a, b, c, d) ((uint32)(uint8)(d) + ((uint32)(uint8)(c) << 8) + ((uint32)(uint8)(b) << 16) + ((uint32)(uint8)(a) << 24))

class IVDXUnknown {
public:
	enum { kIID = VDXMAKEFOURCC('X', 'u', 'n', 'k') };
	virtual int VDXAPIENTRY AddRef() = 0;
	virtual int VDXAPIENTRY Release() = 0;
	virtual void *VDXAPIENTRY AsInterface(uint32 iid) = 0;
};

#endif
