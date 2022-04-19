/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/graphicsd2d.cpp
// Purpose:     Implementation of Direct2D Render Context
// Author:      Pana Alexandru <astronothing@gmail.com>
// Created:     2014-05-20
// Copyright:   (c) 2014 KaiWidgets development team
// Licence:     KaiWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "GraphicsD2D.h"
#include <d2d1.h>

// Minimum supported client: Windows 8 and Platform Update for Windows 7
#define KaiD2D_DEVICE_CONTEXT_SUPPORTED 0

// We load these functions at runtime from the d2d1.dll.
// However, since they are also used inside the d2d1.h header we must provide
// implementations matching the exact declarations. These defines ensures we
// are not violating the ODR rule.
#define D2D1CreateFactory KaiD2D1CreateFactory
#define D2D1MakeRotateMatrix KaiD2D1MakeRotateMatrix
#define D2D1MakeSkewMatrix KaiD2D1MakeSkewMatrix
#define D2D1IsMatrixInvertible KaiD2D1IsMatrixInvertible
#define D2D1InvertMatrix KaiD2D1InvertMatrix
#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
#define D3D11CreateDevice KaiD3D11CreateDevice
#endif

#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
#include <d3d11.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#endif

#ifdef __VISUALC__
	#pragma warning(pop)
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <float.h> // for FLT_MAX, FLT_MIN

#include "GraphicsD2D.h"
//#include "Kai/graphics.h"
#include "wx/dynlib.h"
#include "wx/msw/private/comptr.h"
#include "wx/private/graphics.h"
#include "wx/stack.h"
#include "wx/sharedptr.h"

// This must be the last header included to only affect the DEFINE_GUID()
// occurrences below but not any GUIDs declared in the standard files included
// above.

// Checks the precondition of KaiManagedResourceHolder::AcquireResource, namely
// that it is bound to a manager.
#define KaiCHECK_RESOURCE_HOLDER_PRE()                                         \
	{                                                                         \
	if (IsResourceAcquired()) return;                                         \
	KaiCHECK_RET(IsBound(),                                                    \
		"Cannot acquire a native resource without being bound to a manager"); \
	}

// Checks the postcondition of KaiManagedResourceHolder::AcquireResource, namely
// that it was successful in acquiring the native resource.
#define KaiCHECK_RESOURCE_HOLDER_POST() \
	KaiCHECK_RET(m_nativeResource != nullptr, "Could not acquire native resource");

inline double KaiRadToDeg(double rad) { return (rad * 180.0) / M_PI; }


// Helper class used to check for direct2d availability at runtime and to
// dynamically load the required symbols from d2d1.dll and dwrite.dll
class KaiDirect2D
{
public:

	enum KaiD2DVersion
	{
		KaiD2D_VERSION_1_0,
		KaiD2D_VERSION_1_1,
		KaiD2D_VERSION_NONE
	};

	static bool Initialize()
	{
		if (!m_initialized)
		{
			m_hasDirect2DSupport = LoadLibraries();
			m_initialized = true;
		}

		return m_hasDirect2DSupport;
	}

	static bool HasDirect2DSupport()
	{
		Initialize();

		return m_hasDirect2DSupport;
	}

	static KaiD2DVersion GetDirect2DVersion()
	{
		return m_D2DRuntimeVersion;
	}

private:
	static bool LoadLibraries()
	{
		if ( !m_dllDirect2d.Load(L"d2d1.dll", wxDL_VERBATIM | wxDL_QUIET) )
			return false;

		if ( !m_dllDirectWrite.Load(L"dwrite.dll", wxDL_VERBATIM | wxDL_QUIET) )
			return false;

		#define KaiLOAD_FUNC(dll, name)                    \
		name = (name##_t)dll.RawGetSymbol(#name);         \
			if ( !name )                                  \
			return false;

		KaiLOAD_FUNC(m_dllDirect2d, D2D1CreateFactory);
		KaiLOAD_FUNC(m_dllDirect2d, D2D1MakeRotateMatrix);
		KaiLOAD_FUNC(m_dllDirect2d, D2D1MakeSkewMatrix);
		KaiLOAD_FUNC(m_dllDirect2d, D2D1IsMatrixInvertible);
		KaiLOAD_FUNC(m_dllDirect2d, D2D1InvertMatrix);
		KaiLOAD_FUNC(m_dllDirectWrite, DWriteCreateFactory);


		m_D2DRuntimeVersion = KaiD2D_VERSION_1_0;

		return true;
	}

public:
	typedef HRESULT (WINAPI *D2D1CreateFactory_t)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS*, void**);
	static D2D1CreateFactory_t D2D1CreateFactory;

	typedef void (WINAPI *D2D1MakeRotateMatrix_t)(FLOAT, D2D1_POINT_2F, D2D1_MATRIX_3X2_F*);
	static D2D1MakeRotateMatrix_t D2D1MakeRotateMatrix;

	typedef void (WINAPI *D2D1MakeSkewMatrix_t)(FLOAT, FLOAT, D2D1_POINT_2F, D2D1_MATRIX_3X2_F*);
	static D2D1MakeSkewMatrix_t D2D1MakeSkewMatrix;

	typedef BOOL (WINAPI *D2D1IsMatrixInvertible_t)(const D2D1_MATRIX_3X2_F*);
	static D2D1IsMatrixInvertible_t D2D1IsMatrixInvertible;

	typedef BOOL (WINAPI *D2D1InvertMatrix_t)(D2D1_MATRIX_3X2_F*);
	static D2D1InvertMatrix_t D2D1InvertMatrix;

	typedef HRESULT (WINAPI *DWriteCreateFactory_t)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);
	static DWriteCreateFactory_t DWriteCreateFactory;


private:
	static bool m_initialized;
	static bool m_hasDirect2DSupport;
	static KaiD2DVersion m_D2DRuntimeVersion;

	static wxDynamicLibrary m_dllDirect2d;
	static wxDynamicLibrary m_dllDirectWrite;
#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
	static KaiDynamicLibrary m_dllDirect3d;
#endif
};

// define the members
bool KaiDirect2D::m_initialized = false;
bool KaiDirect2D::m_hasDirect2DSupport = false;
KaiDirect2D::KaiD2DVersion KaiDirect2D::m_D2DRuntimeVersion = KaiD2D_VERSION_NONE;

wxDynamicLibrary KaiDirect2D::m_dllDirect2d;
wxDynamicLibrary KaiDirect2D::m_dllDirectWrite;
#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
KaiDynamicLibrary KaiDirect2D::m_dllDirect3d;
#endif

// define the (not yet imported) functions
KaiDirect2D::D2D1CreateFactory_t KaiDirect2D::D2D1CreateFactory = nullptr;
KaiDirect2D::D2D1MakeRotateMatrix_t KaiDirect2D::D2D1MakeRotateMatrix = nullptr;
KaiDirect2D::D2D1MakeSkewMatrix_t KaiDirect2D::D2D1MakeSkewMatrix = nullptr;
KaiDirect2D::D2D1IsMatrixInvertible_t KaiDirect2D::D2D1IsMatrixInvertible = nullptr;
KaiDirect2D::D2D1InvertMatrix_t KaiDirect2D::D2D1InvertMatrix = nullptr;
KaiDirect2D::DWriteCreateFactory_t KaiDirect2D::DWriteCreateFactory = nullptr;

#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
KaiDirect2D::D3D11CreateDevice_t KaiDirect2D::D3D11CreateDevice = nullptr;
#endif

// define the interface GUIDs
DEFINE_GUID(KaiIID_IWICImagingFactory,
			0xec5ec8a9, 0xc395, 0x4314, 0x9c, 0x77, 0x54, 0xd7, 0xa9, 0x35, 0xff, 0x70);

DEFINE_GUID(KaiIID_ID2D1Factory,
			0x06152247, 0x6f50, 0x465a, 0x92, 0x45, 0x11, 0x8b, 0xfd, 0x3b, 0x60, 0x07);

DEFINE_GUID(KaiIID_IDWriteFactory,
			0xb859ee5a, 0xd838, 0x4b5b, 0xa2, 0xe8, 0x1a, 0xdc, 0x7d, 0x93, 0xdb, 0x48);

DEFINE_GUID(KaiIID_IWICBitmapSource,
			0x00000120, 0xa8f2, 0x4877, 0xba, 0x0a, 0xfd, 0x2b, 0x66, 0x45, 0xfb, 0x94);

DEFINE_GUID(GUID_WICPixelFormat32bppPBGRA,
			0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10);

DEFINE_GUID(GUID_WICPixelFormat32bppBGR,
			0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0e);

#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
DEFINE_GUID(IID_IDXGIDevice,
			0x54ec77fa, 0x1377, 0x44e6, 0x8c, 0x32, 0x88, 0xfd, 0x5f, 0x44, 0xc8, 0x4c);
#endif

#ifndef CLSID_WICImagingFactory
DEFINE_GUID(CLSID_WICImagingFactory,
			0xcacaf262, 0x9370, 0x4615, 0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0xa);
#endif

// Implementation of the Direct2D functions
HRESULT WINAPI KaiD2D1CreateFactory(
	D2D1_FACTORY_TYPE factoryType,
	REFIID riid,
	CONST D2D1_FACTORY_OPTIONS *pFactoryOptions,
	void **ppIFactory)
{
	if (!KaiDirect2D::Initialize())
		return S_FALSE;

	return KaiDirect2D::D2D1CreateFactory(
		factoryType,
		riid,
		pFactoryOptions,
		ppIFactory);
}

void WINAPI KaiD2D1MakeRotateMatrix(
	FLOAT angle,
	D2D1_POINT_2F center,
	D2D1_MATRIX_3X2_F *matrix)
{
	if (!KaiDirect2D::Initialize())
		return;

	KaiDirect2D::D2D1MakeRotateMatrix(angle, center, matrix);
}

void WINAPI KaiD2D1MakeSkewMatrix(
	FLOAT angleX,
	FLOAT angleY,
	D2D1_POINT_2F center,
	D2D1_MATRIX_3X2_F *matrix)
{
	if (!KaiDirect2D::Initialize())
		return;

	KaiDirect2D::D2D1MakeSkewMatrix(angleX, angleY, center, matrix);
}

BOOL WINAPI KaiD2D1IsMatrixInvertible(
	const D2D1_MATRIX_3X2_F *matrix)
{
	if (!KaiDirect2D::Initialize())
		return FALSE;

	return KaiDirect2D::D2D1IsMatrixInvertible(matrix);
}

BOOL WINAPI KaiD2D1InvertMatrix(
	D2D1_MATRIX_3X2_F *matrix)
{
	if (!KaiDirect2D::Initialize())
		return FALSE;

	return KaiDirect2D::D2D1InvertMatrix(matrix);
}

#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
HRESULT WINAPI KaiD3D11CreateDevice(
	IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	ID3D11Device** ppDevice,
	D3D_FEATURE_LEVEL* pFeatureLevel,
	ID3D11DeviceContext** ppImmediateContext)
{
	if (!KaiDirect2D::Initialize())
		return S_FALSE;

	return KaiDirect2D::D3D11CreateDevice(
		pAdapter,
		DriverType,
		Software,
		Flags,
		pFeatureLevels,
		FeatureLevels,
		SDKVersion,
		ppDevice,
		pFeatureLevel,
		ppImmediateContext);
}
#endif

static IWICImagingFactory* gs_WICImagingFactory = nullptr;

IWICImagingFactory* KaiWICImagingFactory()
{
	if (gs_WICImagingFactory == nullptr) {
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			KaiIID_IWICImagingFactory,
			(LPVOID*)&gs_WICImagingFactory);
		KaiCHECK_HRESULT_RET_PTR(hr);
	}
	return gs_WICImagingFactory;
}

static ID2D1Factory* gs_ID2D1Factory = nullptr;

ID2D1Factory* KaiD2D1Factory()
{
	if (!KaiDirect2D::Initialize())
		return nullptr;

	if (gs_ID2D1Factory == nullptr)
	{
		D2D1_FACTORY_OPTIONS factoryOptions = {D2D1_DEBUG_LEVEL_NONE};

		// According to
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ee794287(v=vs.85).aspx
		// the Direct2D Debug Layer is only available starting with Windows 8
		// and Visual Studio 2012.
#if defined(__WXDEBUG__) && defined(__VISUALC__) && KaiCHECK_VISUALC_VERSION(11)
		if (KaiGetWinVersion() >= 0x602)
		{
			factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
		}
#endif  //__WXDEBUG__

		HRESULT hr = KaiDirect2D::D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			KaiIID_ID2D1Factory,
			&factoryOptions,
			reinterpret_cast<void**>(&gs_ID2D1Factory)
			);
		KaiCHECK_HRESULT_RET_PTR(hr);
	}
	return gs_ID2D1Factory;
}

static IDWriteFactory* gs_IDWriteFactory = nullptr;

IDWriteFactory* KaiDWriteFactory()
{
	if (!KaiDirect2D::Initialize())
		return nullptr;

	if (gs_IDWriteFactory == nullptr)
	{
		KaiDirect2D::DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			KaiIID_IDWriteFactory,
			reinterpret_cast<IUnknown**>(&gs_IDWriteFactory)
			);
	}
	return gs_IDWriteFactory;
}

//extern WXDLLIMPEXP_DATA_CORE(KaiGraphicsPen) KaiNullGraphicsPen;
//extern WXDLLIMPEXP_DATA_CORE(KaiGraphicsBrush) KaiNullGraphicsBrush;

// We use the notion of a context supplier because the context
// needed to create Direct2D resources (namely the RenderTarget)
// is itself device-dependent and might change during the lifetime
// of the resources which were created from it.
template <typename C>
class KaiContextSupplier
{
public:
	typedef C ContextType;

	virtual C GetContext() = 0;
};

typedef KaiContextSupplier<ID2D1RenderTarget*> KaiD2DContextSupplier;

// A resource holder manages a generic resource by acquiring
// and releasing it on demand.
class KaiResourceHolder
{
public:
	// Acquires the managed resource if necessary (not already acquired)
	virtual void AcquireResource() = 0;

	// Releases the managed resource
	virtual void ReleaseResource() = 0;

	// Checks if the resources was previously acquired
	virtual bool IsResourceAcquired() = 0;

	// Returns the managed resource or nullptr if the resources
	// was not previously acquired
	virtual void* GetResource() = 0;

	virtual ~KaiResourceHolder() {}
};

class KaiD2DResourceManager;

class KaiD2DManagedObject
{
public:
	virtual void Bind(KaiD2DResourceManager* manager) = 0;
	virtual void UnBind() = 0;
	virtual bool IsBound() = 0;
	virtual KaiD2DResourceManager* GetManager() = 0;

	virtual ~KaiD2DManagedObject() {}
};

class KaiManagedResourceHolder : public KaiResourceHolder, public KaiD2DManagedObject
{
public:
	virtual ~KaiManagedResourceHolder() {}
};

// A Direct2D resource manager handles the device-dependent
// resource holders attached to it by requesting them to
// release their resources when the API invalidates.
// NOTE: We're using a list because we expect to have multiple
// insertions but very rarely a traversal (if ever).
WX_DECLARE_LIST(KaiManagedResourceHolder, KaiManagedResourceListType);
#include <Kai/listimpl.cpp>
WX_DEFINE_LIST(KaiManagedResourceListType);

class KaiD2DResourceManager: public KaiD2DContextSupplier
{
public:
	void RegisterResourceHolder(KaiManagedResourceHolder* resourceHolder)
	{
		m_resources.push_back(resourceHolder);
	}

	void UnregisterResourceHolder(KaiManagedResourceHolder* resourceHolder)
	{
		m_resources.remove(resourceHolder);
	}

	void ReleaseResources()
	{
		KaiManagedResourceListType::iterator it;
		for (it = m_resources.begin(); it != m_resources.end(); ++it)
		{
			(*it)->ReleaseResource();
		}

		// Check that all resources were released
		for (it = m_resources.begin(); it != m_resources.end(); ++it)
		{
			KaiCHECK_RET(!(*it)->IsResourceAcquired(), "One or more device-dependent resources failed to release");
		}
	}

	virtual ~KaiD2DResourceManager()
	{
		while (!m_resources.empty())
		{
			m_resources.front()->ReleaseResource();
			m_resources.front()->UnBind();
		}
	}

private:
	KaiManagedResourceListType m_resources;
};

// A Direct2D resource holder manages device dependent resources
// by storing any information necessary for acquiring the resource
// and releasing the resource when the API invalidates it.
template<typename T>
class KaiD2DResourceHolder: public KaiManagedResourceHolder
{
public:
	KaiD2DResourceHolder() : m_resourceManager(nullptr)
	{
	}

	virtual ~KaiD2DResourceHolder()
	{
		UnBind();
		ReleaseResource();
	}

	bool IsResourceAcquired() override
	{
		return m_nativeResource != nullptr;
	}

	void* GetResource() override
	{
		return GetD2DResource();
	}

	KaiCOMPtr<T>& GetD2DResource()
	{
		if (!IsResourceAcquired())
		{
			AcquireResource();
		}

		return m_nativeResource;
	}

	void AcquireResource() override
	{
		KaiCHECK_RESOURCE_HOLDER_PRE();

		DoAcquireResource();

		KaiCHECK_RESOURCE_HOLDER_POST();
	}

	void ReleaseResource() override
	{
		m_nativeResource.reset();
	}

	KaiD2DContextSupplier::ContextType GetContext()
	{
		return m_resourceManager->GetContext();
	}

	void Bind(KaiD2DResourceManager* manager) override
	{
		if (IsBound())
			return;

		m_resourceManager = manager;
		m_resourceManager->RegisterResourceHolder(this);
	}

	void UnBind() override
	{
		if (!IsBound())
			return;

		m_resourceManager->UnregisterResourceHolder(this);
		m_resourceManager = nullptr;
	}

	bool IsBound() override
	{
		return m_resourceManager != nullptr;
	}

	KaiD2DResourceManager* GetManager() override
	{
		return m_resourceManager;
	}

protected:
	virtual void DoAcquireResource() = 0;

private:
	KaiD2DResourceManager* m_resourceManager;

protected:
	KaiCOMPtr<T> m_nativeResource;
};

// Used as super class for graphics data objects
// to forward the bindings to their internal resource holder.
class KaiD2DManagedGraphicsData : public KaiD2DManagedObject
{
public:
	void Bind(KaiD2DResourceManager* manager) override
	{
		GetManagedObject()->Bind(manager);
	}

	void UnBind() override
	{
		GetManagedObject()->UnBind();
	}

	bool IsBound() override
	{
		return GetManagedObject()->IsBound();
	}

	KaiD2DResourceManager* GetManager() override
	{
		return GetManagedObject()->GetManager();
	}

	virtual KaiD2DManagedObject* GetManagedObject() = 0;

	~KaiD2DManagedGraphicsData() {}
};

D2D1_CAP_STYLE KaiD2DConvertPenCap(KaiPenCap cap)
{
	switch (cap)
	{
	case KaiCAP_ROUND:
		return D2D1_CAP_STYLE_ROUND;
	case KaiCAP_PROJECTING:
		return D2D1_CAP_STYLE_SQUARE;
	case KaiCAP_BUTT:
		return D2D1_CAP_STYLE_FLAT;
	case KaiCAP_INVALID:
		return D2D1_CAP_STYLE_FLAT;
	}

	KaiFAIL_MSG("unknown pen cap");
	return D2D1_CAP_STYLE_FLAT;
}

D2D1_LINE_JOIN KaiD2DConvertPenJoin(KaiPenJoin join)
{
	switch (join)
	{
	case KaiJOIN_BEVEL:
		return D2D1_LINE_JOIN_BEVEL;
	case KaiJOIN_MITER:
		return D2D1_LINE_JOIN_MITER;
	case KaiJOIN_ROUND:
		return D2D1_LINE_JOIN_ROUND;
	case KaiJOIN_INVALID:
		return D2D1_LINE_JOIN_MITER;
	}

	KaiFAIL_MSG("unknown pen join");
	return D2D1_LINE_JOIN_MITER;
}

D2D1_DASH_STYLE KaiD2DConvertPenStyle(KaiPenStyle dashStyle)
{
	switch (dashStyle)
	{
	case KaiPENSTYLE_SOLID:
		return D2D1_DASH_STYLE_SOLID;
	case KaiPENSTYLE_DOT:
		return D2D1_DASH_STYLE_DOT;
	case KaiPENSTYLE_LONG_DASH:
		return D2D1_DASH_STYLE_DASH;
	case KaiPENSTYLE_SHORT_DASH:
		return D2D1_DASH_STYLE_DASH;
	case KaiPENSTYLE_DOT_DASH:
		return D2D1_DASH_STYLE_DASH_DOT;
	case KaiPENSTYLE_USER_DASH:
		return D2D1_DASH_STYLE_CUSTOM;

	// NB: These styles cannot be converted to a D2D1_DASH_STYLE
	// and must be handled separately.
	case KaiPENSTYLE_TRANSPARENT:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_INVALID:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_STIPPLE_MASK_OPAQUE:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_STIPPLE_MASK:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_STIPPLE:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_BDIAGONAL_HATCH:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_CROSSDIAG_HATCH:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_FDIAGONAL_HATCH:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_CROSS_HATCH:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_HORIZONTAL_HATCH:
		KaiFALLTHROUGH;
	case KaiPENSTYLE_VERTICAL_HATCH:
		return D2D1_DASH_STYLE_SOLID;
	}

	KaiFAIL_MSG("unknown pen style");
	return D2D1_DASH_STYLE_SOLID;
}

D2D1_COLOR_F KaiD2DConvertColour(KaiColour colour)
{
	return D2D1::ColorF(
		colour.Red() / 255.0f,
		colour.Green() / 255.0f,
		colour.Blue() / 255.0f,
		colour.Alpha() / 255.0f);
}

D2D1_ANTIALIAS_MODE KaiD2DConvertAntialiasMode(KaiAntialiasMode antialiasMode)
{
	switch (antialiasMode)
	{
	case KaiANTIALIAS_NONE:
		return D2D1_ANTIALIAS_MODE_ALIASED;
	case KaiANTIALIAS_DEFAULT:
		return D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	}

	KaiFAIL_MSG("unknown antialias mode");
	return D2D1_ANTIALIAS_MODE_ALIASED;
}

#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
bool KaiD2DCompositionModeSupported(KaiCompositionMode compositionMode)
{
	if (compositionMode == KaiCOMPOSITION_CLEAR || compositionMode == KaiCOMPOSITION_INVALID)
	{
		return false;
	}

	return true;
}

D2D1_COMPOSITE_MODE KaiD2DConvertCompositionMode(KaiCompositionMode compositionMode)
{
	switch (compositionMode)
	{
	case KaiCOMPOSITION_SOURCE:
		return D2D1_COMPOSITE_MODE_SOURCE_COPY;
	case KaiCOMPOSITION_OVER:
		return D2D1_COMPOSITE_MODE_SOURCE_OVER;
	case KaiCOMPOSITION_IN:
		return D2D1_COMPOSITE_MODE_SOURCE_IN;
	case KaiCOMPOSITION_OUT:
		return D2D1_COMPOSITE_MODE_SOURCE_OUT;
	case KaiCOMPOSITION_ATOP:
		return D2D1_COMPOSITE_MODE_SOURCE_ATOP;
	case KaiCOMPOSITION_DEST_OVER:
		return D2D1_COMPOSITE_MODE_DESTINATION_OVER;
	case KaiCOMPOSITION_DEST_IN:
		return D2D1_COMPOSITE_MODE_DESTINATION_IN;
	case KaiCOMPOSITION_DEST_OUT:
		return D2D1_COMPOSITE_MODE_DESTINATION_OUT;
	case KaiCOMPOSITION_DEST_ATOP:
		return D2D1_COMPOSITE_MODE_DESTINATION_ATOP;
	case KaiCOMPOSITION_XOR:
		return D2D1_COMPOSITE_MODE_XOR;
	case KaiCOMPOSITION_ADD:
		return D2D1_COMPOSITE_MODE_PLUS;

	// unsupported composition modes
	case KaiCOMPOSITION_DEST:
		KaiFALLTHROUGH;
	case KaiCOMPOSITION_CLEAR:
		KaiFALLTHROUGH;
	case KaiCOMPOSITION_INVALID:
		return D2D1_COMPOSITE_MODE_SOURCE_COPY;
	}

	KaiFAIL_MSG("unknown composition mode");
	return D2D1_COMPOSITE_MODE_SOURCE_COPY;
}
#endif // KaiD2D_DEVICE_CONTEXT_SUPPORTED

// Direct2D 1.1 introduces a new enum for specifying the interpolation quality
// which is only used with the ID2D1DeviceContext::DrawImage method.
#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
D2D1_INTERPOLATION_MODE KaiD2DConvertInterpolationMode(KaiInterpolationQuality interpolationQuality)
{
	switch (interpolationQuality)
	{
	case KaiINTERPOLATION_DEFAULT:
		KaiFALLTHROUGH;
	case KaiINTERPOLATION_NONE:
		KaiFALLTHROUGH;
	case KaiINTERPOLATION_FAST:
		return D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
	case KaiINTERPOLATION_GOOD:
		return D2D1_INTERPOLATION_MODE_LINEAR;
	case KaiINTERPOLATION_BEST:
		return D2D1_INTERPOLATION_MODE_CUBIC;
	}

	KaiFAIL_MSG("unknown interpolation quality");
	return D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
}
#endif // KaiD2D_DEVICE_CONTEXT_SUPPORTED

D2D1_BITMAP_INTERPOLATION_MODE KaiD2DConvertBitmapInterpolationMode(KaiInterpolationQuality interpolationQuality)
{
	switch (interpolationQuality)
	{
	case KaiINTERPOLATION_DEFAULT:
		KaiFALLTHROUGH;
	case KaiINTERPOLATION_NONE:
		KaiFALLTHROUGH;
	case KaiINTERPOLATION_FAST:
		KaiFALLTHROUGH;
	case KaiINTERPOLATION_GOOD:
		return D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
	case KaiINTERPOLATION_BEST:
		return D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
	}

	KaiFAIL_MSG("unknown interpolation quality");
	return D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
}

D2D1_RECT_F KaiD2DConvertRect(const KaiRect& rect)
{
	return D2D1::RectF(rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
}

KaiCOMPtr<ID2D1Geometry> KaiD2DConvertRegionToGeometry(ID2D1Factory* direct2dFactory, const KaiRegion& region)
{
	// Build the array of geometries
	HRESULT hr;
	int i;
	ID2D1Geometry** geometries;
	int rectCount;
	if ( region.IsEmpty() )
	{
		// Empty region is skipped by iterator
		// so we have to create it in a special way.
		rectCount = 1;
		geometries = new ID2D1Geometry*[rectCount];

		geometries[0] = nullptr;
		hr = direct2dFactory->CreateRectangleGeometry(
						D2D1::RectF(0.0F, 0.0F, 0.0F, 0.0F),
						(ID2D1RectangleGeometry**)(&geometries[0]));
		KaiFAILED_HRESULT_MSG(hr);
	}
	else
	{
		// Count the number of rectangles which compose the region
		KaiRegionIterator regionIterator(region);
		rectCount = 0;
		while(regionIterator++)
			rectCount++;

		geometries = new ID2D1Geometry*[rectCount];
		regionIterator.Reset(region);

		i = 0;
		while(regionIterator)
		{
			geometries[i] = nullptr;

			KaiRect rect = regionIterator.GetRect();
			rect.SetWidth(rect.GetWidth() + 1);
			rect.SetHeight(rect.GetHeight() + 1);

			hr = direct2dFactory->CreateRectangleGeometry(
				KaiD2DConvertRect(rect),
				(ID2D1RectangleGeometry**)(&geometries[i]));
			KaiFAILED_HRESULT_MSG(hr);

			i++;
			++regionIterator;
		}
	}

	// Create a geometry group to hold all the rectangles
	KaiCOMPtr<ID2D1GeometryGroup> resultGeometry;
	hr = direct2dFactory->CreateGeometryGroup(
		D2D1_FILL_MODE_WINDING,
		geometries,
		rectCount,
		&resultGeometry);
	KaiFAILED_HRESULT_MSG(hr);

	// Cleanup temporaries
	for (i = 0; i < rectCount; ++i)
	{
		geometries[i]->Release();
	}

	delete[] geometries;

	return KaiCOMPtr<ID2D1Geometry>(resultGeometry);
}


bool operator==(const D2D1::Matrix3x2F& lhs, const D2D1::Matrix3x2F& rhs)
{
	return
		lhs._11 == rhs._11 && lhs._12 == rhs._12 &&
		lhs._21 == rhs._21 && lhs._22 == rhs._22 &&
		lhs._31 == rhs._31 && lhs._32 == rhs._32;
}

//-----------------------------------------------------------------------------
// KaiD2DMatrixData declaration
//-----------------------------------------------------------------------------

class KaiD2DMatrixData : public GraphicsMatrixData
{
public:
	KaiD2DMatrixData(KaiD2DRenderer* renderer);
	KaiD2DMatrixData(KaiD2DRenderer* renderer, const D2D1::Matrix3x2F& matrix);

	//virtual GraphicsObjectRefData* Clone() const override;

	void Concat(const GraphicsMatrixData* t) override;

	void Set(KaiDouble a = 1.0, KaiDouble b = 0.0, KaiDouble c = 0.0, KaiDouble d = 1.0,
		KaiDouble tx = 0.0, KaiDouble ty = 0.0) override;

	void Get(KaiDouble* a = nullptr, KaiDouble* b = nullptr,  KaiDouble* c = nullptr,
		KaiDouble* d = nullptr, KaiDouble* tx = nullptr, KaiDouble* ty = nullptr) const override;

	void Invert() override;

	bool IsEqual(const GraphicsMatrixData* t) const override;

	bool IsIdentity() const override;

	void Translate(KaiDouble dx, KaiDouble dy) override;

	void Scale(KaiDouble xScale, KaiDouble yScale) override;

	void Rotate(KaiDouble angle) override;

	void TransformPoint(KaiDouble* x, KaiDouble* y) const override;

	void TransformDistance(KaiDouble* dx, KaiDouble* dy) const override;

	void* GetNativeMatrix() const override;

	D2D1::Matrix3x2F GetMatrix3x2F() const;

private:
	D2D1::Matrix3x2F m_matrix;
};

//-----------------------------------------------------------------------------
// KaiD2DMatrixData implementation
//-----------------------------------------------------------------------------

KaiD2DMatrixData::KaiD2DMatrixData(KaiD2DRenderer* renderer) : GraphicsMatrixData(renderer)
{
	m_matrix = D2D1::Matrix3x2F::Identity();
}

KaiD2DMatrixData::KaiD2DMatrixData(KaiD2DRenderer* renderer, const D2D1::Matrix3x2F& matrix) :
	GraphicsMatrixData(renderer), m_matrix(matrix)
{
}

//GraphicsObjectRefData* KaiD2DMatrixData::Clone() const
//{
//    return new KaiD2DMatrixData(GetRenderer(), m_matrix);
//}

void KaiD2DMatrixData::Concat(const GraphicsMatrixData* t)
{
	// Elements of resulting matrix are modified in-place in SetProduct()
	// so multiplied matrices cannot be the instances of the resulting matrix.
	// Note that parameter matrix (t) is the multiplicand.
	const D2D1::Matrix3x2F m1(static_cast<const KaiD2DMatrixData*>(t)->m_matrix);
	const D2D1::Matrix3x2F m2(m_matrix);
	m_matrix.SetProduct(m1, m2);
}

void KaiD2DMatrixData::Set(KaiDouble a, KaiDouble b, KaiDouble c, KaiDouble d, KaiDouble tx, KaiDouble ty)
{
	m_matrix._11 = a;
	m_matrix._12 = b;
	m_matrix._21 = c;
	m_matrix._22 = d;
	m_matrix._31 = tx;
	m_matrix._32 = ty;
}

void KaiD2DMatrixData::Get(KaiDouble* a, KaiDouble* b,  KaiDouble* c, KaiDouble* d, KaiDouble* tx, KaiDouble* ty) const
{
	*a = m_matrix._11;
	*b = m_matrix._12;
	*c = m_matrix._21;
	*d = m_matrix._22;
	*tx = m_matrix._31;
	*ty = m_matrix._32;
}

void KaiD2DMatrixData::Invert()
{
	m_matrix.Invert();
}

bool KaiD2DMatrixData::IsEqual(const GraphicsMatrixData* t) const
{
	return m_matrix == static_cast<const KaiD2DMatrixData*>(t)->m_matrix;
}

bool KaiD2DMatrixData::IsIdentity() const
{
	return m_matrix.IsIdentity();
}

void KaiD2DMatrixData::Translate(KaiDouble dx, KaiDouble dy)
{
	m_matrix = D2D1::Matrix3x2F::Translation(dx, dy) * m_matrix;
}

void KaiD2DMatrixData::Scale(KaiDouble xScale, KaiDouble yScale)
{
	m_matrix = D2D1::Matrix3x2F::Scale(xScale, yScale) * m_matrix;
}

void KaiD2DMatrixData::Rotate(KaiDouble angle)
{
	m_matrix = D2D1::Matrix3x2F::Rotation(KaiRadToDeg(angle)) * m_matrix;
}

void KaiD2DMatrixData::TransformPoint(KaiDouble* x, KaiDouble* y) const
{
	D2D1_POINT_2F result = m_matrix.TransformPoint(D2D1::Point2F(*x, *y));
	*x = result.x;
	*y = result.y;
}

void KaiD2DMatrixData::TransformDistance(KaiDouble* dx, KaiDouble* dy) const
{
	D2D1::Matrix3x2F noTranslationMatrix = m_matrix;
	noTranslationMatrix._31 = 0;
	noTranslationMatrix._32 = 0;
	D2D1_POINT_2F result = noTranslationMatrix.TransformPoint(D2D1::Point2F(*dx, *dy));
	*dx = result.x;
	*dy = result.y;
}

void* KaiD2DMatrixData::GetNativeMatrix() const
{
	return (void*)&m_matrix;
}

D2D1::Matrix3x2F KaiD2DMatrixData::GetMatrix3x2F() const
{
	return m_matrix;
}

//const KaiD2DMatrixData* KaiGetD2DMatrixData(const KaiGraphicsMatrix& matrix)
//{
//    return static_cast<const KaiD2DMatrixData*>(matrix.GetMatrixData());
//}

//-----------------------------------------------------------------------------
// KaiD2DPathData declaration
//-----------------------------------------------------------------------------

bool operator==(const D2D1_POINT_2F& lhs, const D2D1_POINT_2F& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

class KaiD2DPathData : public GraphicsPathData
{
public :

	// ID2D1PathGeometry objects are device-independent resources created
	// from a ID2D1Factory. This means we can safely create the resource outside
	// (the KaiD2DRenderer handles this) and store it here since it never gets
	// thrown away by the GPU.
	KaiD2DPathData(KaiD2DRenderer* renderer, ID2D1Factory* d2dFactory);

	~KaiD2DPathData();

	void SetFillMode(D2D1_FILL_MODE fillMode)
	{
		m_fillMode = fillMode;
	}

	D2D1_FILL_MODE GetFillMode() const
	{
		return m_fillMode;
	}

	ID2D1PathGeometry* GetPathGeometry();

	// This closes the geometry sink, ensuring all the figures are stored inside
	// the ID2D1PathGeometry. Calling this method is required before any draw operation
	// involving a path.
	void Flush();

	//GraphicsObjectRefData* Clone() const override;

	// begins a new subpath at (x,y)
	void MoveToPoint(KaiDouble x, KaiDouble y) override;

	// adds a straight line from the current point to (x,y)
	void AddLineToPoint(KaiDouble x, KaiDouble y) override;

	// adds a cubic Bezier curve from the current point, using two control points and an end point
	void AddCurveToPoint(KaiDouble cx1, KaiDouble cy1, KaiDouble cx2, KaiDouble cy2, KaiDouble x, KaiDouble y) override;

	// adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
	void AddArc(KaiDouble x, KaiDouble y, KaiDouble r, KaiDouble startAngle, KaiDouble endAngle, bool clockwise) override;

	// gets the last point of the current path, (0,0) if not yet set
	void GetCurrentPoint(KaiDouble* x, KaiDouble* y) const override;

	// adds another path
	void AddPath(const GraphicsPathData* path) override;

	// closes the current sub-path
	void CloseSubpath() override;

	// returns the native path
	void* GetNativePath() const override;

	// give the native path returned by GetNativePath() back (there might be some deallocations necessary)
	void UnGetNativePath(void* WXUNUSED(p)) const override {}

	// transforms each point of this path by the matrix
	void Transform(const GraphicsMatrixData* matrix) override;

	// gets the bounding box enclosing all points (possibly including control points)
	void GetBox(KaiDouble* x, KaiDouble* y, KaiDouble* w, KaiDouble *h) const override;

	bool Contains(KaiDouble x, KaiDouble y, KaiPolygonFillMode fillStyle = KaiODDEVEN_RULE) const override;

	// appends an ellipsis as a new closed subpath fitting the passed rectangle
	void AddCircle(KaiDouble x, KaiDouble y, KaiDouble r) override;

	// appends an ellipse
	void AddEllipse(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h) override;

private:
	void EnsureGeometryOpen();

	void EnsureSinkOpen();

	void EnsureFigureOpen(const D2D1_POINT_2F& pos);

	void EndFigure(D2D1_FIGURE_END figureEnd);

	ID2D1Geometry* GetFullGeometry(D2D1_FILL_MODE fillMode) const;

	bool IsEmpty() const;
	bool IsStateSafeForFlush() const;

	struct GeometryStateData
	{
		bool m_isCurrentPointSet;
		D2D1_POINT_2F m_currentPoint;
		bool m_isFigureOpen;
		D2D1_POINT_2F m_figureStart;
		bool m_isFigureLogStartSet;
		D2D1_POINT_2F m_figureLogStart;
	};
	void SaveGeometryState(GeometryStateData& data) const;
	void RestoreGeometryState(const GeometryStateData& data);

private :
	KaiCOMPtr<ID2D1PathGeometry> m_pathGeometry;

	KaiCOMPtr<ID2D1GeometrySink> m_geometrySink;

	KaiCOMPtr<ID2D1Factory> m_direct2dfactory;

	mutable KaiCOMPtr<ID2D1GeometryGroup> m_combinedGeometry;
	KaiVector<ID2D1Geometry*> m_pTransformedGeometries;

	bool m_currentPointSet;
	D2D1_POINT_2F m_currentPoint;

	bool m_figureOpened;
	D2D1_POINT_2F m_figureStart;
	bool m_figureLogStartSet;
	D2D1_POINT_2F m_figureLogStart;

	bool m_geometryWritable;

	D2D1_FILL_MODE m_fillMode;
};

//-----------------------------------------------------------------------------
// KaiD2DPathData implementation
//-----------------------------------------------------------------------------

KaiD2DPathData::KaiD2DPathData(KaiD2DRenderer* renderer, ID2D1Factory* d2dFactory) :
	GraphicsPathData(renderer),
	m_direct2dfactory(d2dFactory),
	m_currentPointSet(false),
	m_currentPoint(D2D1::Point2F(0.0f, 0.0f)),
	m_figureOpened(false),
	m_figureStart(D2D1::Point2F(0.0f, 0.0f)),
	m_figureLogStartSet(false),
	m_figureLogStart(D2D1::Point2F(0.0f, 0.0f)),
	m_geometryWritable(true),
	m_fillMode(D2D1_FILL_MODE_ALTERNATE)
{
	m_direct2dfactory->CreatePathGeometry(&m_pathGeometry);
	// To properly initialize path geometry there is also
	// necessary to open at least once its geometry sink.
	m_pathGeometry->Open(&m_geometrySink);
}

KaiD2DPathData::~KaiD2DPathData()
{
	Flush();
	for( size_t i = 0; i < m_pTransformedGeometries.size(); i++ )
	{
		m_pTransformedGeometries[i]->Release();
	}
}

ID2D1PathGeometry* KaiD2DPathData::GetPathGeometry()
{
	return m_pathGeometry;
}

//KaiD2DPathData::GraphicsObjectRefData* KaiD2DPathData::Clone() const
//{
//    KaiD2DPathData* newPathData = new KaiD2DPathData(GetRenderer(), m_direct2dfactory);
//
//    newPathData->EnsureGeometryOpen();
//
//    // Only geometry with closed sink can be
//    // transferred to another geometry object with
//    // ID2D1PathGeometry::Stream() so we have to check
//    // if actual transfer succeeded.
//
//    // Transfer geometry to the new geometry sink.
//    HRESULT hr = m_pathGeometry->Stream(newPathData->m_geometrySink);
//    KaiASSERT_MSG( SUCCEEDED(hr), KaiS("Current geometry is in invalid state") );
//    if ( FAILED(hr) )
//    {
//        delete newPathData;
//        return nullptr;
//    }
//
//    // Copy the collection of transformed geometries.
//    ID2D1TransformedGeometry* pTransformedGeometry;
//    for ( size_t i = 0; i < m_pTransformedGeometries.size(); i++ )
//    {
//        pTransformedGeometry = nullptr;
//        hr = m_direct2dfactory->CreateTransformedGeometry(
//                    m_pTransformedGeometries[i],
//                    D2D1::Matrix3x2F::Identity(), &pTransformedGeometry);
//        KaiASSERT_MSG( SUCCEEDED(hr), KaiFAILED_HRESULT_MSG(hr) );
//        newPathData->m_pTransformedGeometries.push_back(pTransformedGeometry);
//    }
//
//    // Copy positional data.
//    GeometryStateData curState;
//    SaveGeometryState(curState);
//    newPathData->RestoreGeometryState(curState);
//
//    return newPathData;
//}

void KaiD2DPathData::Flush()
{
	if (m_geometrySink != nullptr)
	{
		if ( m_figureOpened )
		{
			m_geometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
			m_figureOpened = false;
		}

		if( m_geometryWritable )
		{
			HRESULT hr = m_geometrySink->Close();
			KaiCHECK_HRESULT_RET(hr);
			m_geometryWritable = false;
		}
	}
}

void KaiD2DPathData::EnsureGeometryOpen()
{
	if (!m_geometryWritable)
	{
		KaiCOMPtr<ID2D1PathGeometry> newPathGeometry;
		HRESULT hr;
		hr = m_direct2dfactory->CreatePathGeometry(&newPathGeometry);
		KaiCHECK_HRESULT_RET(hr);

		m_geometrySink.reset();
		hr = newPathGeometry->Open(&m_geometrySink);
		KaiCHECK_HRESULT_RET(hr);

		if (m_pathGeometry != nullptr)
		{
			hr = m_pathGeometry->Stream(m_geometrySink);
			KaiCHECK_HRESULT_RET(hr);
		}

		m_pathGeometry = newPathGeometry;
		m_geometryWritable = true;
	}
}

void KaiD2DPathData::EnsureSinkOpen()
{
	EnsureGeometryOpen();

	if (m_geometrySink == nullptr)
	{
		HRESULT hr = m_pathGeometry->Open(&m_geometrySink);
		KaiCHECK_HRESULT_RET(hr);
	}
}

void KaiD2DPathData::EnsureFigureOpen(const D2D1_POINT_2F& pos)
{
	EnsureSinkOpen();

	if (!m_figureOpened)
	{
		m_figureStart = pos;
		m_geometrySink->BeginFigure(m_figureStart, D2D1_FIGURE_BEGIN_FILLED);
		m_figureOpened = true;
		m_currentPoint = m_figureStart;
	}
}

void KaiD2DPathData::EndFigure(D2D1_FIGURE_END figureEnd)
{
	if (m_figureOpened)
	{
		// Ensure that sub-path being closed contains at least one point.
		if( figureEnd == D2D1_FIGURE_END_CLOSED )
			m_geometrySink->AddLine(m_currentPoint);

		if( figureEnd == D2D1_FIGURE_END_OPEN ||
			!m_figureLogStartSet ||
			m_figureLogStart == m_figureStart )
		{
			// If figure will remain open or if its logical startpoint
			// is not used or if it is the same as the actual
			// startpoint then we can end the figure in a standard way.
			m_geometrySink->EndFigure(figureEnd);
		}
		else
		{
			// If we want to end and close the figure for which
			// logical startpoint is not the same as actual startpoint
			// we have to fill the gap between the actual and logical
			// endpoints on our own.
			m_geometrySink->AddLine(m_figureLogStart);
			m_geometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
			m_figureStart = m_figureLogStart;
		}
		m_figureOpened = false;
		m_figureLogStartSet = false;

		// If the figure is closed then current point
		// should be moved to the beginning of the figure.
		if( figureEnd == D2D1_FIGURE_END_CLOSED )
			m_currentPoint = m_figureStart;
	}
}

ID2D1Geometry* KaiD2DPathData::GetFullGeometry(D2D1_FILL_MODE fillMode) const
{
	// Our final path geometry is represented by geometry group
	// which contains all transformed geometries plus current geometry.

	// We have to store pointers to all transformed geometries
	// as well as pointer to the current geometry in the auxiliary array.
	const size_t numGeometries = m_pTransformedGeometries.size();
	ID2D1Geometry** pGeometries = new ID2D1Geometry*[numGeometries+1];
	for( size_t i = 0; i < numGeometries; i++ )
		pGeometries[i] = m_pTransformedGeometries[i];

	pGeometries[numGeometries] = m_pathGeometry;

	// And use this array as a source to create geometry group.
	m_combinedGeometry.reset();
	HRESULT hr = m_direct2dfactory->CreateGeometryGroup(fillMode,
								  pGeometries, numGeometries+1, &m_combinedGeometry);
	KaiFAILED_HRESULT_MSG(hr);
	delete []pGeometries;

	return m_combinedGeometry;
}

bool KaiD2DPathData::IsEmpty() const
{
	return !m_currentPointSet && !m_figureOpened &&
			m_pTransformedGeometries.size() == 0;
}

bool KaiD2DPathData::IsStateSafeForFlush() const
{
	// Only geometry with not yet started figure
	// or with started but empty figure can be fully
	// restored to its initial state after invoking Flush().
	if( !m_figureOpened )
		return true;

	D2D1_POINT_2F actFigureStart = m_figureLogStartSet ?
						m_figureLogStart : m_figureStart;
	return m_currentPoint == actFigureStart;
}

void KaiD2DPathData::SaveGeometryState(GeometryStateData& data) const
{
	data.m_isFigureOpen = m_figureOpened;
	data.m_isFigureLogStartSet = m_figureLogStartSet;
	data.m_isCurrentPointSet = m_currentPointSet;
	data.m_currentPoint = m_currentPoint;
	data.m_figureStart = m_figureStart;
	data.m_figureLogStart = m_figureLogStart;
}

void KaiD2DPathData::RestoreGeometryState(const GeometryStateData& data)
{
	if( data.m_isFigureOpen )
	{
		// If the figure has to be re-started at the startpoint
		// which is not the current point then we have to start it
		// physically at the current point but with storing also its
		// logical startpoint to use it later on to close the figure,
		// if necessary.
		// Ending and closing the figure using this proxy startpoint
		// is only a simulation of regular closure and figure can behave
		// in a slightly different way than figure closed with physical
		// startpoint so this action should be avoided if only possible.
		D2D1_POINT_2F actFigureStart = data.m_isFigureLogStartSet ?
						 data.m_figureLogStart : data.m_figureStart;
		if ( !(data.m_currentPoint == actFigureStart) )
		{
			m_figureLogStart = actFigureStart;
			m_figureLogStartSet = true;
			EnsureFigureOpen(data.m_currentPoint);
		}
		else
		{
			EnsureFigureOpen(actFigureStart);
		}
	}
	else
	{
		m_figureOpened = false;
	}

	m_currentPointSet = data.m_isCurrentPointSet;
	m_currentPoint = data.m_isCurrentPointSet ?
				data.m_currentPoint : D2D1::Point2F(0.0F, 0.0F);
}

void KaiD2DPathData::MoveToPoint(KaiDouble x, KaiDouble y)
{
	// Close current sub-path (leaving the figure as is).
	EndFigure(D2D1_FIGURE_END_OPEN);
	// Store new current point
	m_currentPoint = D2D1::Point2F(x, y);
	m_currentPointSet = true;
}

// adds a straight line from the current point to (x,y)
void KaiD2DPathData::AddLineToPoint(KaiDouble x, KaiDouble y)
{
	// If current point is not yet set then
	// this function should behave as MoveToPoint.
	if( !m_currentPointSet )
	{
		MoveToPoint(x, y);
		return;
	}

	EnsureFigureOpen(m_currentPoint);
	m_geometrySink->AddLine(D2D1::Point2F(x, y));

	m_currentPoint = D2D1::Point2F(x, y);
}

// adds a cubic Bezier curve from the current point, using two control points and an end point
void KaiD2DPathData::AddCurveToPoint(KaiDouble cx1, KaiDouble cy1, KaiDouble cx2, KaiDouble cy2, KaiDouble x, KaiDouble y)
{
	// If no current point is set then this function should behave
	// as if preceded by a call to MoveToPoint(cx1, cy1).
	if( !m_currentPointSet )
		MoveToPoint(cx1, cy1);

	EnsureFigureOpen(m_currentPoint);

	D2D1_BEZIER_SEGMENT bezierSegment = {
		{ (FLOAT)cx1, (FLOAT)cy1 },
		{ (FLOAT)cx2, (FLOAT)cy2 },
		{ (FLOAT)x, (FLOAT)y } };
	m_geometrySink->AddBezier(bezierSegment);

	m_currentPoint = D2D1::Point2F(x, y);
}

// adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
void KaiD2DPathData::AddArc(KaiDouble x, KaiDouble y, KaiDouble r, KaiDouble startAngle, KaiDouble endAngle, bool clockwise)
{
	double angle;

	// For the sake of compatibility normalize angles the same way
	// as it is done in Cairo.
	if ( clockwise )
	{
		// If endAngle < startAngle it needs to be progressively
		// increased by 2*M_PI until endAngle > startAngle.
		if ( endAngle < startAngle )
		{
			while ( endAngle <= startAngle )
			{
				endAngle += 2.0*M_PI;
			}
		}

		angle = endAngle - startAngle;
	}
	else
	{
		// If endAngle > startAngle it needs to be progressively
		// decreased by 2*M_PI until endAngle < startAngle.
		if ( endAngle > startAngle )
		{
			while ( endAngle >= startAngle )
			{
				endAngle -= 2.0*M_PI;
			}
		}

		angle = startAngle - endAngle;
	}

	KaiPoint2DDouble start = KaiPoint2DDouble(cos(startAngle) * r, sin(startAngle) * r);
	KaiPoint2DDouble end = KaiPoint2DDouble(cos(endAngle) * r, sin(endAngle) * r);

	// To ensure compatibility with Cairo an initial
	// line segment to the beginning of the arc needs
	// to be added to the path.
	if ( m_figureOpened )
	{
		AddLineToPoint(start.m_x + x, start.m_y + y);
	}
	else if ( m_currentPointSet )
	{
		EnsureFigureOpen(m_currentPoint);
		AddLineToPoint(start.m_x + x, start.m_y + y);
	}
	else
	{
		MoveToPoint(start.m_x + x, start.m_y + y);
		EnsureFigureOpen(m_currentPoint);
	}

	D2D1_SWEEP_DIRECTION sweepDirection = clockwise ?
	   D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	D2D1_SIZE_F size = D2D1::SizeF((FLOAT)r, (FLOAT)r);

	if ( angle >= 2.0*M_PI )
	{
		// In addition to arc we need to draw full circle(s).
		// Remarks:
		// 1. Parity of the number of the circles has to be
		// preserved because this matters when path would be
		// filled with KaiODDEVEN_RULE flag set (using
		// D2D1_FILL_MODE_ALTERNATE mode) when number of the
		// edges is counted.
		// 2. ID2D1GeometrySink::AddArc() doesn't work
		// with 360-degree arcs so we need to construct
		// the circle from two halves.
		D2D1_ARC_SEGMENT circleSegment1 =
		{
			D2D1::Point2((FLOAT)(x - start.m_x), (FLOAT)(y - start.m_y)),  // end point
			size,                     // size
			0.0f,                     // rotation
			sweepDirection,           // sweep direction
			D2D1_ARC_SIZE_SMALL       // arc size
		};
		D2D1_ARC_SEGMENT circleSegment2 =
		{
			D2D1::Point2((FLOAT)(x + start.m_x), (FLOAT)(y + start.m_y)),  // end point
			size,                     // size
			0.0f,                     // rotation
			sweepDirection,           // sweep direction
			D2D1_ARC_SIZE_SMALL       // arc size
		};

		int numCircles = (int)(angle / (2.0*M_PI));
		numCircles = (numCircles - 1) % 2 + 1;
		for( int i = 0; i < numCircles; i++ )
		{
			m_geometrySink->AddArc(circleSegment1);
			m_geometrySink->AddArc(circleSegment2);
		}

		// Reduce the angle to [0..2*M_PI) range.
		angle = fmod(angle, 2.0*M_PI);
	}

	D2D1_ARC_SIZE arcSize = angle > M_PI ?
	   D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL;
	D2D1_POINT_2F endPoint =
	   D2D1::Point2((FLOAT)(end.m_x + x), (FLOAT)(end.m_y + y));

	D2D1_ARC_SEGMENT arcSegment =
	{
		endPoint,                     // end point
		size,                         // size
		0.0f,                         // rotation
		sweepDirection,               // sweep direction
		arcSize                       // arc size
	};

	m_geometrySink->AddArc(arcSegment);

	m_currentPoint = endPoint;
}

// appends an ellipsis as a new closed subpath fitting the passed rectangle
void KaiD2DPathData::AddCircle(KaiDouble x, KaiDouble y, KaiDouble r)
{
	AddEllipse(x - r, y - r, r * 2, r * 2);
}

// appends an ellipse
void KaiD2DPathData::AddEllipse(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	if ( w <= 0.0 || h <= 0.0 )
	  return;

	// Calculate radii
	const KaiDouble rx = w / 2.0;
	const KaiDouble ry = h / 2.0;

	MoveToPoint(x + w, y + ry);
	// Open new subpath
	EnsureFigureOpen(m_currentPoint);

	D2D1_ARC_SEGMENT arcSegmentLower =
	{
		D2D1::Point2((FLOAT)(x), (FLOAT)(y + ry)),     // end point
		D2D1::SizeF((FLOAT)(rx), (FLOAT)(ry)),         // size
		0.0f,
		D2D1_SWEEP_DIRECTION_CLOCKWISE,
		D2D1_ARC_SIZE_SMALL
	};
	m_geometrySink->AddArc(arcSegmentLower);

	D2D1_ARC_SEGMENT arcSegmentUpper =
	{
		D2D1::Point2((FLOAT)(x + w), (FLOAT)(y + ry)), // end point
		D2D1::SizeF((FLOAT)(rx), (FLOAT)(ry)),         // size
		0.0f,
		D2D1_SWEEP_DIRECTION_CLOCKWISE,
		D2D1_ARC_SIZE_SMALL
	};
	m_geometrySink->AddArc(arcSegmentUpper);

	CloseSubpath();
}

// gets the last point of the current path, (0,0) if not yet set
void KaiD2DPathData::GetCurrentPoint(KaiDouble* x, KaiDouble* y) const
{
	if (x != nullptr) *x = m_currentPoint.x;
	if (y != nullptr) *y = m_currentPoint.y;
}

// adds another path
void KaiD2DPathData::AddPath(const GraphicsPathData* path)
{
	KaiD2DPathData* pathSrc =
		 const_cast<KaiD2DPathData*>(static_cast<const KaiD2DPathData*>(path));

	// Nothing to do if geometry of appended path is not initialized.
	if ( pathSrc->m_pathGeometry == nullptr || pathSrc->m_geometrySink == nullptr )
		return;

	// Because only closed geometry (with closed sink)
	// can be transferred to another geometry object with
	// ID2D1PathGeometry::Stream() so we have to close it
	// before any operation and to re-open afterwards.
	// Unfortunately, to close the sink it is also necessary
	// to end the figure it contains, if it was open.
	// After re-opening the geometry we should also re-start
	// the figure (if it was open) and restore its state but
	// it seems there is no straightforward way to do so
	// if the figure is not empty.
	//
	// So, only if appended path has a sub-path closed or
	// has an empty sub-path open there is possible to restore
	// its state after appending it to the current path and only
	// in this case the operation doesn't introduce side effects.

	// Nothing to do if appended path is empty.
	if ( pathSrc->IsEmpty() )
		return;

	// Save positional and auxiliary data
	// of the appended path and its geometry.
	GeometryStateData curStateSrc;
	pathSrc->SaveGeometryState(curStateSrc);

	// Close appended geometry.
	pathSrc->Flush();

	// Close current geometry (leaving the figure as is).
	Flush();

	HRESULT hr;
	ID2D1TransformedGeometry* pTransformedGeometry = nullptr;
	// Add current geometry to the collection transformed geometries.
	hr = m_direct2dfactory->CreateTransformedGeometry(m_pathGeometry,
						D2D1::Matrix3x2F::Identity(), &pTransformedGeometry);
	KaiCHECK_HRESULT_RET(hr);
	m_pTransformedGeometries.push_back(pTransformedGeometry);

	// Add to the collection transformed geometries from the appended path.
	for ( size_t i = 0; i < pathSrc->m_pTransformedGeometries.size(); i++ )
	{
		pTransformedGeometry = nullptr;
		hr = m_direct2dfactory->CreateTransformedGeometry(
					pathSrc->m_pTransformedGeometries[i],
					D2D1::Matrix3x2F::Identity(), &pTransformedGeometry);
		KaiCHECK_HRESULT_RET(hr);
		m_pTransformedGeometries.push_back(pTransformedGeometry);
	}

	// Clear and reopen current geometry.
	m_pathGeometry.reset();
	EnsureGeometryOpen();

	// Transfer appended geometry to the current geometry sink.
	hr = pathSrc->m_pathGeometry->Stream(m_geometrySink);
	KaiCHECK_HRESULT_RET(hr);

	// Apply to the current path positional data from the appended path.
	// This operation fully sets geometry to the required state
	// only if it represents geometry without started figure
	// or with started but empty figure.
	RestoreGeometryState(curStateSrc);

	// Reopen appended geometry.
	pathSrc->EnsureGeometryOpen();
	// Restore its positional data.
	// This operation fully restores geometry to the required state
	// only if it represents geometry without started figure
	// or with started but empty figure.
	pathSrc->RestoreGeometryState(curStateSrc);
}

// closes the current sub-path
void KaiD2DPathData::CloseSubpath()
{
	// If we have a sub-path open by call to MoveToPoint(),
	// which doesn't open a new figure by itself,
	// we have to open a new figure now to get a required 1-point path.
	if ( !m_figureOpened && m_currentPointSet )
	{
		EnsureFigureOpen(m_currentPoint);
	}
	// Close sub-path and close the figure.
	if ( m_figureOpened )
	{
		EndFigure(D2D1_FIGURE_END_CLOSED);
		MoveToPoint(m_figureStart.x, m_figureStart.y);
	}
}

void* KaiD2DPathData::GetNativePath() const
{
	return GetFullGeometry(GetFillMode());
}

void KaiD2DPathData::Transform(const GraphicsMatrixData* matrix)
{
	// Unfortunately, it looks there is no straightforward way to apply
	// transformation to the current underlying path geometry
	// (ID2D1PathGeometry object) "in-place" (ie. transform it and use
	// for further graphics operations, including next transformations too).
	// Some simple methods offered by D2D are not useful for these purposes:
	// 1. ID2D1Factory::CreateTransformedGeometry() converts ID2D1PathGeometry
	// object to ID2D1TransformedGeometry object but ID2D1TransformedGeometry
	// inherits from ID2D1Geometry (not from ID2D1PathGeometry)
	// and hence cannot be used for further path operations.
	// 2. ID2D1Geometry::CombineWithGeometry() which could be used to get final
	// path geometry by combining empty geometry with transformed geometry
	// doesn't offer any combine mode which would produce a "sum" of geometries
	// (D2D1_COMBINE_MODE_UNION produces kind of outline). Moreover the result
	// is stored in ID2D1SimplifiedGeometrySink not in ID2DGeometrySink.

	// So, it seems that ability to transform the KaiGraphicsPath
	// (several times) and still use it after this operation(s)
	// can be achieved (only?) by using a geometry group object
	// (ID2D1GeometryGroup) this way:
	// 1. After applying transformation to the current path geometry with
	// ID2D1Factory::CreateTransformedGeometry() the result is stored
	// in the collection of transformed geometries (an auxiliary array)
	// and after that a new (empty) geometry is open (in the same state
	// as just closed one) and this geometry is used as a current one
	// for further graphics operations.
	// 2. Since above steps are done at every transformation so our effective
	// geometry will be a superposition of all previously transformed
	// geometries stored in the collection (array) and the current
	// operational geometry.
	// 3. If there is necessary to use this combined effective geometry
	// in any operation then ID2D1GeometryGroup created with
	// ID2D1Factory::CreateGeometryGroup() from the collection
	// of stored geometries will act as a proxy geometry.

	const D2D1::Matrix3x2F* m = static_cast<D2D1::Matrix3x2F*>(matrix->GetNativeMatrix());

	// Save current positional data.
	GeometryStateData curState;
	SaveGeometryState(curState);
	// We need to close the geometry what requires also to end a figure
	// (if started). This ended figure should be re-started in its initial
	// state when all path processing is done but due to the Direct2D
	// constraints this can be fully done only if open figure was empty.
	// So, Transform() can be safely called if path doesn't contain the open
	// sub-path or if open sub-path is empty.
	
	// Close current geometry.
	Flush();

	HRESULT hr;
	ID2D1TransformedGeometry* pTransformedGeometry;
	// Apply given transformation to all previously stored geometries too.
	for( size_t i = 0; i < m_pTransformedGeometries.size(); i++ )
	{
		pTransformedGeometry = nullptr;
		hr = m_direct2dfactory->CreateTransformedGeometry(m_pTransformedGeometries[i], m, &pTransformedGeometry);
		KaiCHECK_HRESULT_RET(hr);

		m_pTransformedGeometries[i]->Release();
		m_pTransformedGeometries[i] = pTransformedGeometry;
	}

	// Transform current geometry and add the result
	// to the collection of transformed geometries.
	pTransformedGeometry = nullptr;
	hr = m_direct2dfactory->CreateTransformedGeometry(m_pathGeometry, m, &pTransformedGeometry);
	KaiCHECK_HRESULT_RET(hr);
	m_pTransformedGeometries.push_back(pTransformedGeometry);

	// Clear and reopen current geometry.
	m_pathGeometry.reset();
	EnsureGeometryOpen();
	// Restore the figure with transformed positional data.
	// This operation fully restores geometry to the required state
	// only if IsStateSafeForFlush() returns true.
	curState.m_currentPoint = m->TransformPoint(curState.m_currentPoint);
	curState.m_figureLogStart = m->TransformPoint(curState.m_figureLogStart);
	curState.m_figureStart = m->TransformPoint(curState.m_figureStart);
	RestoreGeometryState(curState);
}

void KaiD2DPathData::GetBox(KaiDouble* x, KaiDouble* y, KaiDouble* w, KaiDouble *h) const
{
	D2D1_RECT_F bounds;
	ID2D1Geometry *curGeometry = GetFullGeometry(GetFillMode());
	HRESULT hr = curGeometry->GetBounds(D2D1::Matrix3x2F::Identity(), &bounds);
	KaiCHECK_HRESULT_RET(hr);
	// Check if bounds are empty
	if ( bounds.left > bounds.right )
	{
		bounds.left = bounds.top = bounds.right = bounds.bottom = 0.0F;
	}
	if (x) *x = bounds.left;
	if (y) *y = bounds.top;
	if (w) *w = bounds.right - bounds.left;
	if (h) *h = bounds.bottom - bounds.top;
}

bool KaiD2DPathData::Contains(KaiDouble x, KaiDouble y, KaiPolygonFillMode fillStyle) const
{
	BOOL result;
	D2D1_FILL_MODE fillMode = (fillStyle == KaiODDEVEN_RULE) ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING;
	ID2D1Geometry *curGeometry = GetFullGeometry(fillMode);
	curGeometry->FillContainsPoint(D2D1::Point2F(x, y), D2D1::Matrix3x2F::Identity(), &result);
	return result != FALSE;
}

//KaiD2DPathData* KaiGetD2DPathData(const GraphicsPathData& path)
//{
//    return static_cast<KaiD2DPathData*>(path.GetGraphicsData());
//}

// This utility class is used to read a color value with the format
// PBGRA from a byte stream and to write a color back to the stream.
// It's used in conjunction with the IWICBitmapSource or IWICBitmap
// pixel data to easily read and write color values.
struct KaiPBGRAColor
{
	KaiPBGRAColor(BYTE* stream) :
		b(*stream), g(*(stream + 1)), r(*(stream + 2)), a(*(stream + 3))
	{}

	KaiPBGRAColor(const KaiColor& color) :
		b(color.Blue()), g(color.Green()), r(color.Red()), a(color.Alpha())
	{}

	bool IsBlack() const { return r == 0 && g == 0 && b == 0; }

	void Write(BYTE* stream) const
	{
		*(stream + 0) = b;
		*(stream + 1) = g;
		*(stream + 2) = r;
		*(stream + 3) = a;
	}

	BYTE b, g, r, a;
};

KaiCOMPtr<IWICBitmapSource> KaiCreateWICBitmap(const WXHBITMAP sourceBitmap, bool hasAlpha = false)
{
	HRESULT hr;

	KaiCOMPtr<IWICBitmap> wicBitmap;
	hr = KaiWICImagingFactory()->CreateBitmapFromHBITMAP(sourceBitmap, nullptr, WICBitmapUseAlpha, &wicBitmap);
	KaiCHECK2_HRESULT_RET(hr, KaiCOMPtr<IWICBitmapSource>(nullptr));

	KaiCOMPtr<IWICFormatConverter> converter;
	hr = KaiWICImagingFactory()->CreateFormatConverter(&converter);
	KaiCHECK2_HRESULT_RET(hr, KaiCOMPtr<IWICBitmapSource>(nullptr));

	WICPixelFormatGUID pixelFormat = hasAlpha ? GUID_WICPixelFormat32bppPBGRA : GUID_WICPixelFormat32bppBGR;

	hr = converter->Initialize(
		wicBitmap,
		pixelFormat,
		WICBitmapDitherTypeNone, nullptr, 0.f,
		WICBitmapPaletteTypeMedianCut);
	KaiCHECK2_HRESULT_RET(hr, KaiCOMPtr<IWICBitmapSource>(nullptr));

	return KaiCOMPtr<IWICBitmapSource>(converter);
}

KaiCOMPtr<IWICBitmapSource> KaiCreateWICBitmap(const KaiBitmap& sourceBitmap, bool hasAlpha = false)
{
	return KaiCreateWICBitmap(sourceBitmap.GetHBITMAP(), hasAlpha);
}

// WIC Bitmap Source for creating hatch patterned bitmaps
class KaiHatchBitmapSource : public IWICBitmapSource
{
public:
	KaiHatchBitmapSource(KaiBrushStyle brushStyle, const KaiColor& color) :
		m_brushStyle(brushStyle), m_color(color), m_refCount(0l)
	{
	}

	virtual ~KaiHatchBitmapSource() {}

	HRESULT STDMETHODCALLTYPE GetSize(__RPC__out UINT *width, __RPC__out UINT *height) override
	{
		if (width != nullptr) *width = 8;
		if (height != nullptr) *height = 8;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetPixelFormat(__RPC__out WICPixelFormatGUID *pixelFormat) override
	{
		if (pixelFormat != nullptr) *pixelFormat = GUID_WICPixelFormat32bppPBGRA;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetResolution(__RPC__out double *dpiX, __RPC__out double *dpiY) override
	{
		if (dpiX != nullptr) *dpiX = 96.0;
		if (dpiY != nullptr) *dpiY = 96.0;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE CopyPalette(__RPC__in_opt IWICPalette*  WXUNUSED(palette)) override
	{
		return WINCODEC_ERR_PALETTEUNAVAILABLE;
	}

	HRESULT STDMETHODCALLTYPE CopyPixels(
		const WICRect* WXUNUSED(prc),
		UINT WXUNUSED(stride),
		UINT WXUNUSED(bufferSize),
		BYTE *buffer) override
	{
		// patterns are encoded in a bit map of size 8 x 8
		static const unsigned char BDIAGONAL_PATTERN[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
		static const unsigned char FDIAGONAL_PATTERN[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
		static const unsigned char CROSSDIAG_PATTERN[8] = { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 };
		static const unsigned char CROSS_PATTERN[8] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF };
		static const unsigned char HORIZONTAL_PATTERN[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF };
		static const unsigned char VERTICAL_PATTERN[8] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

		switch (m_brushStyle)
		{
			case KaiBRUSHSTYLE_BDIAGONAL_HATCH:
				CopyPattern(buffer, BDIAGONAL_PATTERN);
				break;
			case KaiBRUSHSTYLE_CROSSDIAG_HATCH:
				CopyPattern(buffer, CROSSDIAG_PATTERN);
				break;
			case KaiBRUSHSTYLE_FDIAGONAL_HATCH:
				CopyPattern(buffer, FDIAGONAL_PATTERN);
				break;
			case KaiBRUSHSTYLE_CROSS_HATCH:
				CopyPattern(buffer, CROSS_PATTERN);
				break;
			case KaiBRUSHSTYLE_HORIZONTAL_HATCH:
				CopyPattern(buffer, HORIZONTAL_PATTERN);
				break;
			case KaiBRUSHSTYLE_VERTICAL_HATCH:
				CopyPattern(buffer, VERTICAL_PATTERN);
				break;
			default:
				break;
		}

		return S_OK;
	}

	// Implementations adapted from: "Implementing IUnknown in C++"
	// http://msdn.microsoft.com/en-us/library/office/cc839627%28v=office.15%29.aspx

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID referenceId, void** object) override
	{
		if (!object)
		{
			return E_INVALIDARG;
		}

		*object = nullptr;

		if (referenceId == IID_IUnknown || referenceId == KaiIID_IWICBitmapSource)
		{
			*object = (LPVOID)this;
			AddRef();
			return NOERROR;
		}

		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef(void) override
	{
		InterlockedIncrement(&m_refCount);
		return m_refCount;
	}

	ULONG STDMETHODCALLTYPE Release(void) override
	{
		KaiCHECK_MSG(m_refCount > 0, 0, "Unbalanced number of calls to Release");

		ULONG refCount = InterlockedDecrement(&m_refCount);
		if (m_refCount == 0)
		{
			delete this;
		}
		return refCount;
	}

private:

	// Copies an 8x8 bit pattern to a PBGRA byte buffer
	void CopyPattern(BYTE* buffer, const unsigned char* pattern) const
	{
		static const KaiPBGRAColor transparent(KaiTransparentColour);

		int k = 0;

		for (int i = 0; i < 8; ++i)
		{
			for (int j = 7; j >= 0; --j)
			{
				bool isColorBit = (pattern[i] & (1 << j)) > 0;
				(isColorBit ? m_color : transparent).Write(buffer + k);
				k += 4;
			}
		}
	}

private:
	// The hatch style produced by this bitmap source
	const KaiBrushStyle m_brushStyle;

	// The colour of the hatch
	const KaiPBGRAColor m_color;

	// Internally used to implement IUnknown's reference counting
	ULONG m_refCount;
};

// RAII class hosting a WIC bitmap lock used for writing
// pixel data to a WICBitmap
class KaiBitmapPixelWriteLock
{
public:
	KaiBitmapPixelWriteLock(IWICBitmap* bitmap)
	{
		// Retrieve the size of the bitmap
		UINT w, h;
		bitmap->GetSize(&w, &h);
		WICRect lockSize = {0, 0, (INT)w, (INT)h};

		// Obtain a bitmap lock for exclusive write
		bitmap->Lock(&lockSize, WICBitmapLockWrite, &m_pixelLock);
	}

	IWICBitmapLock* GetLock() { return m_pixelLock; }

private:
	KaiCOMPtr<IWICBitmapLock> m_pixelLock;
};

class KaiD2DBitmapResourceHolder : public KaiD2DResourceHolder<ID2D1Bitmap>
{
public:
	KaiD2DBitmapResourceHolder(const KaiBitmap& sourceBitmap) :
		m_sourceBitmap(sourceBitmap)
	{
	}

	const KaiBitmap& GetSourceBitmap() const { return m_sourceBitmap; }

protected:
	void DoAcquireResource() override
	{
		ID2D1RenderTarget* renderTarget = GetContext();

		HRESULT hr;

		if(m_sourceBitmap.GetMask())
		{
			int w = m_sourceBitmap.GetWidth();
			int h = m_sourceBitmap.GetHeight();

			KaiCOMPtr<IWICBitmapSource> colorBitmap = KaiCreateWICBitmap(m_sourceBitmap);
			KaiCOMPtr<IWICBitmapSource> maskBitmap = KaiCreateWICBitmap(m_sourceBitmap.GetMask()->GetMaskBitmap());
			KaiCOMPtr<IWICBitmap> resultBitmap;

			KaiWICImagingFactory()->CreateBitmap(
				w, h,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapCacheOnLoad,
				&resultBitmap);

			BYTE* colorBuffer = new BYTE[4 * w * h];
			BYTE* maskBuffer = new BYTE[4 * w * h];
			BYTE* resultBuffer;

			hr = colorBitmap->CopyPixels(nullptr, w * 4, 4 * w * h, colorBuffer);
			hr = maskBitmap->CopyPixels(nullptr, w * 4, 4 * w * h, maskBuffer);

			{
				KaiBitmapPixelWriteLock lock(resultBitmap);

				UINT bufferSize = 0;
				hr = lock.GetLock()->GetDataPointer(&bufferSize, &resultBuffer);

				static const KaiPBGRAColor transparentColor(KaiTransparentColour);

				// Create the result bitmap
				for (int i = 0; i < w * h * 4; i += 4)
				{
					KaiPBGRAColor color(colorBuffer + i);
					KaiPBGRAColor mask(maskBuffer + i);

					if (mask.IsBlack())
					{
						transparentColor.Write(resultBuffer + i);
					}
					else
					{
						color.a = 255;
						color.Write(resultBuffer + i);
					}
				}
			}

			hr = renderTarget->CreateBitmapFromWicBitmap(resultBitmap, 0, &m_nativeResource);
			KaiCHECK_HRESULT_RET(hr);

			delete[] colorBuffer;
			delete[] maskBuffer;
		}
		else
		{
			KaiCOMPtr<IWICBitmapSource> bitmapSource = KaiCreateWICBitmap(m_sourceBitmap, m_sourceBitmap.HasAlpha());
			hr = renderTarget->CreateBitmapFromWicBitmap(bitmapSource, 0, &m_nativeResource);
		}
	}

private:
	const KaiBitmap m_sourceBitmap;
};

//-----------------------------------------------------------------------------
// KaiD2DBitmapData declaration
//-----------------------------------------------------------------------------

class KaiD2DBitmapData : public GraphicsBitmapData, public KaiD2DManagedGraphicsData
{
public:
	typedef KaiD2DBitmapResourceHolder NativeType;

	KaiD2DBitmapData(KaiD2DRenderer* renderer, const KaiBitmap& bitmap) :
		GraphicsBitmapData(renderer), m_bitmapHolder(bitmap) {}

	KaiD2DBitmapData(KaiD2DRenderer* renderer, const void* pseudoNativeBitmap) :
		GraphicsBitmapData(renderer), m_bitmapHolder(*static_cast<const NativeType*>(pseudoNativeBitmap)) {}

	// returns the native representation
	void* GetNativeBitmap() const override;

	KaiCOMPtr<ID2D1Bitmap> GetD2DBitmap();

	KaiD2DManagedObject* GetManagedObject() override
	{
		return &m_bitmapHolder;
	}

private:
	NativeType m_bitmapHolder;
};

//-----------------------------------------------------------------------------
// KaiD2DBitmapData implementation
//-----------------------------------------------------------------------------

void* KaiD2DBitmapData::GetNativeBitmap() const
{
	return (void*)&m_bitmapHolder;
}

KaiCOMPtr<ID2D1Bitmap> KaiD2DBitmapData::GetD2DBitmap()
{
	return m_bitmapHolder.GetD2DResource();
}

//KaiD2DBitmapData* KaiGetD2DBitmapData(const KaiGraphicsBitmap& bitmap)
//{
//    return static_cast<KaiD2DBitmapData*>(bitmap.GetRefData());
//}

// Helper class used to create and safely release a ID2D1GradientStopCollection from KaiGraphicsGradientStops
class KaiD2DGradientStopsHelper
{
public:
	KaiD2DGradientStopsHelper(const KaiGraphicsGradientStops& gradientStops, ID2D1RenderTarget* renderTarget)
	{
		int stopCount = gradientStops.GetCount();

		D2D1_GRADIENT_STOP* gradientStopArray = new D2D1_GRADIENT_STOP[stopCount];

		for (int i = 0; i < stopCount; ++i)
		{
			gradientStopArray[i].color = KaiD2DConvertColour(gradientStops.Item(i).GetColour());
			gradientStopArray[i].position = gradientStops.Item(i).GetPosition();
		}

		renderTarget->CreateGradientStopCollection(gradientStopArray, stopCount, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &m_gradientStopCollection);

		delete[] gradientStopArray;
	}

	ID2D1GradientStopCollection* GetGradientStopCollection()
	{
		return m_gradientStopCollection;
	}

private:
	KaiCOMPtr<ID2D1GradientStopCollection> m_gradientStopCollection;
};

template <typename B>
class KaiD2DBrushResourceHolder : public KaiD2DResourceHolder<B>
{
public:
	KaiD2DBrushResourceHolder(const KaiBrush& brush) : m_sourceBrush(brush) {}
	virtual ~KaiD2DBrushResourceHolder() {}
protected:
	const KaiBrush m_sourceBrush;
};

class KaiD2DSolidBrushResourceHolder : public KaiD2DBrushResourceHolder<ID2D1SolidColorBrush>
{
public:
	KaiD2DSolidBrushResourceHolder(const KaiBrush& brush) : KaiD2DBrushResourceHolder(brush) {}

protected:
	void DoAcquireResource() override
	{
		KaiColour colour = m_sourceBrush.GetColour();
		HRESULT hr = GetContext()->CreateSolidColorBrush(KaiD2DConvertColour(colour), &m_nativeResource);
		KaiCHECK_HRESULT_RET(hr);
	}
};

class KaiD2DBitmapBrushResourceHolder : public KaiD2DBrushResourceHolder<ID2D1BitmapBrush>
{
public:
	KaiD2DBitmapBrushResourceHolder(const KaiBrush& brush) : KaiD2DBrushResourceHolder(brush) {}

protected:
	void DoAcquireResource() override
	{
		// TODO: cache this bitmap
		KaiD2DBitmapResourceHolder bitmap(*(m_sourceBrush.GetStipple()));
		bitmap.Bind(GetManager());

		HRESULT result = GetContext()->CreateBitmapBrush(
			bitmap.GetD2DResource(),
			D2D1::BitmapBrushProperties(
			D2D1_EXTEND_MODE_WRAP,
			D2D1_EXTEND_MODE_WRAP,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR),
			&m_nativeResource);

		KaiCHECK_HRESULT_RET(result);
	}
};

class KaiD2DHatchBrushResourceHolder : public KaiD2DBrushResourceHolder<ID2D1BitmapBrush>
{
public:
	KaiD2DHatchBrushResourceHolder(const KaiBrush& brush) : KaiD2DBrushResourceHolder(brush) {}

protected:
	void DoAcquireResource() override
	{
		KaiCOMPtr<KaiHatchBitmapSource> hatchBitmapSource(new KaiHatchBitmapSource(m_sourceBrush.GetStyle(), m_sourceBrush.GetColour()));

		KaiCOMPtr<ID2D1Bitmap> bitmap;

		HRESULT hr = GetContext()->CreateBitmapFromWicBitmap(hatchBitmapSource, &bitmap);
		KaiCHECK_HRESULT_RET(hr);

		hr = GetContext()->CreateBitmapBrush(
			bitmap,
			D2D1::BitmapBrushProperties(
			D2D1_EXTEND_MODE_WRAP,
			D2D1_EXTEND_MODE_WRAP,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR),
			&m_nativeResource);
		KaiCHECK_HRESULT_RET(hr);
	}
};

class KaiD2DLinearGradientBrushResourceHolder : public KaiD2DResourceHolder<ID2D1LinearGradientBrush>
{
public:
	struct LinearGradientInfo {
		const KaiDouble x1;
		const KaiDouble y1;
		const KaiDouble x2;
		const KaiDouble y2;
		const KaiGraphicsGradientStops stops;
		KaiD2DMatrixData *matrix;
		LinearGradientInfo(KaiDouble& x1_, KaiDouble& y1_, 
						   KaiDouble& x2_, KaiDouble& y2_, 
						   const KaiGraphicsGradientStops& stops_,
						   KaiD2DMatrixData * matrix_)
			: x1(x1_), y1(y1_), x2(x2_), y2(y2_), stops(stops_), matrix(matrix_) {}
	};

	KaiD2DLinearGradientBrushResourceHolder(KaiDouble& x1, KaiDouble& y1, 
										   KaiDouble& x2, KaiDouble& y2, 
										   const KaiGraphicsGradientStops& stops,
										   KaiD2DMatrixData* matrix)
		: m_linearGradientInfo(x1, y1, x2, y2, stops, matrix) {}

protected:
	void DoAcquireResource() override
	{
		KaiD2DGradientStopsHelper helper(m_linearGradientInfo.stops, GetContext());
		ID2D1LinearGradientBrush  *linearGradientBrush;

		HRESULT hr = GetContext()->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(m_linearGradientInfo.x1, m_linearGradientInfo.y1),
				D2D1::Point2F(m_linearGradientInfo.x2, m_linearGradientInfo.y2)),
			helper.GetGradientStopCollection(),
			&linearGradientBrush);
		KaiCHECK_HRESULT_RET(hr);

		if (! m_linearGradientInfo.matrix)
		{
			D2D1::Matrix3x2F matrix = m_linearGradientInfo.matrix->GetMatrix3x2F();
			matrix.Invert();
			linearGradientBrush->SetTransform(matrix);
		}
		m_nativeResource = linearGradientBrush;
	}
private:
	const LinearGradientInfo m_linearGradientInfo;
};

class KaiD2DRadialGradientBrushResourceHolder : public KaiD2DResourceHolder<ID2D1RadialGradientBrush>
{
public:
	struct RadialGradientInfo {
		const KaiDouble x1;
		const KaiDouble y1;
		const KaiDouble x2;
		const KaiDouble y2;
		const KaiDouble radius;
		const KaiGraphicsGradientStops stops;
		KaiD2DMatrixData *matrix;

		RadialGradientInfo(KaiDouble x1_, KaiDouble y1_, 
						   KaiDouble x2_, KaiDouble y2_, 
						   KaiDouble r, 
						   const KaiGraphicsGradientStops& stops_,
						   KaiD2DMatrixData* matrix_)
			: x1(x1_), y1(y1_), x2(x2_), y2(y2_), radius(r), stops(stops_), matrix(matrix_) {}
	};

	KaiD2DRadialGradientBrushResourceHolder(KaiDouble& x1, KaiDouble& y1, 
										   KaiDouble& x2, KaiDouble& y2, 
										   KaiDouble& r, 
										   const KaiGraphicsGradientStops& stops,
										   KaiD2DMatrixData* matrix)
		: m_radialGradientInfo(x1, y1, x2, y2, r, stops, matrix) {}

protected:
	void DoAcquireResource() override
	{
		KaiD2DGradientStopsHelper helper(m_radialGradientInfo.stops, GetContext());
		ID2D1RadialGradientBrush *radialGradientBrush;

		KaiDouble xo = m_radialGradientInfo.x1 - m_radialGradientInfo.x2;
		KaiDouble yo = m_radialGradientInfo.y1 - m_radialGradientInfo.y2;

		HRESULT hr = GetContext()->CreateRadialGradientBrush(
			D2D1::RadialGradientBrushProperties(
				D2D1::Point2F(m_radialGradientInfo.x1, m_radialGradientInfo.y1),
				D2D1::Point2F(xo, yo),
				m_radialGradientInfo.radius, m_radialGradientInfo.radius),
			helper.GetGradientStopCollection(),
			&radialGradientBrush);
		KaiCHECK_HRESULT_RET(hr);

		if (! m_radialGradientInfo.matrix)
		{
			D2D1::Matrix3x2F matrix = m_radialGradientInfo.matrix->GetMatrix3x2F();
			matrix.Invert();
			radialGradientBrush->SetTransform(matrix);
		}
		m_nativeResource = radialGradientBrush;
	}

private:
	const RadialGradientInfo m_radialGradientInfo;
};

//-----------------------------------------------------------------------------
// KaiD2DBrushData declaration
//-----------------------------------------------------------------------------

class KaiD2DBrushData : /*public GraphicsObjectRefData, */public KaiD2DManagedGraphicsData
{
public:
	KaiD2DBrushData(KaiD2DRenderer* renderer, const KaiBrush brush);

	KaiD2DBrushData(KaiD2DRenderer* renderer);

	void CreateLinearGradientBrush(KaiDouble x1, KaiDouble y1, 
								   KaiDouble x2, KaiDouble y2, 
								   const KaiGraphicsGradientStops& stops,
								   KaiD2DMatrixData* matrix = nullptr);

	void CreateRadialGradientBrush(KaiDouble startX, KaiDouble startY, 
								   KaiDouble endX, KaiDouble endY, 
								   KaiDouble radius, 
								   const KaiGraphicsGradientStops& stops,
								   KaiD2DMatrixData* matrix = nullptr);

	ID2D1Brush* GetBrush() const
	{
		return (ID2D1Brush*)(m_brushResourceHolder->GetResource());
	}

	KaiD2DManagedObject* GetManagedObject() override
	{
		return m_brushResourceHolder.get();
	}

private:
	KaiSharedPtr<KaiManagedResourceHolder> m_brushResourceHolder;
};

//-----------------------------------------------------------------------------
// KaiD2DBrushData implementation
//-----------------------------------------------------------------------------

KaiD2DBrushData::KaiD2DBrushData(KaiD2DRenderer* renderer, const KaiBrush brush)
	: /*GraphicsObjectRefData(renderer), */m_brushResourceHolder(nullptr)
{
	if (brush.GetStyle() == KaiBRUSHSTYLE_SOLID)
	{
		m_brushResourceHolder = new KaiD2DSolidBrushResourceHolder(brush);
	}
	else if (brush.IsHatch())
	{
		m_brushResourceHolder = new KaiD2DHatchBrushResourceHolder(brush);
	}
	else
	{
		m_brushResourceHolder = new KaiD2DBitmapBrushResourceHolder(brush);
	}
}

KaiD2DBrushData::KaiD2DBrushData(KaiD2DRenderer* renderer) 
	: /*GraphicsObjectRefData(renderer), */m_brushResourceHolder(nullptr)
{
}

void KaiD2DBrushData::CreateLinearGradientBrush(
	KaiDouble x1, KaiDouble y1,
	KaiDouble x2, KaiDouble y2,
	const KaiGraphicsGradientStops& stops,
	KaiD2DMatrixData * matrix)
{
	m_brushResourceHolder = new KaiD2DLinearGradientBrushResourceHolder(
		x1, y1, x2, y2, stops, matrix);
}

void KaiD2DBrushData::CreateRadialGradientBrush(
	KaiDouble startX, KaiDouble startY,
	KaiDouble endX, KaiDouble endY,
	KaiDouble radius,
	const KaiGraphicsGradientStops& stops,
	KaiD2DMatrixData * matrix)
{
	m_brushResourceHolder = new KaiD2DRadialGradientBrushResourceHolder(
		startX, startY, endX, endY, radius, stops, matrix);
}

//KaiD2DBrushData* KaiGetD2DBrushData(const KaiGraphicsBrush& brush)
//{
//    return static_cast<KaiD2DBrushData*>(brush.GetGraphicsData());
//}

bool KaiIsHatchPenStyle(KaiPenStyle penStyle)
{
	return penStyle >= KaiPENSTYLE_FIRST_HATCH && penStyle <= KaiPENSTYLE_LAST_HATCH;
}

KaiBrushStyle KaiConvertPenStyleToBrushStyle(KaiPenStyle penStyle)
{
	switch(penStyle)
	{
	case KaiPENSTYLE_BDIAGONAL_HATCH:
		return KaiBRUSHSTYLE_BDIAGONAL_HATCH;
	case KaiPENSTYLE_CROSSDIAG_HATCH:
		return KaiBRUSHSTYLE_CROSSDIAG_HATCH;
	case KaiPENSTYLE_FDIAGONAL_HATCH:
		return KaiBRUSHSTYLE_FDIAGONAL_HATCH;
	case KaiPENSTYLE_CROSS_HATCH:
		return KaiBRUSHSTYLE_CROSS_HATCH;
	case KaiPENSTYLE_HORIZONTAL_HATCH:
		return KaiBRUSHSTYLE_HORIZONTAL_HATCH;
	case KaiPENSTYLE_VERTICAL_HATCH:
		return KaiBRUSHSTYLE_VERTICAL_HATCH;
	default:
		break;
	}

	return KaiBRUSHSTYLE_SOLID;
}

// ----------------------------------------------------------------------------
// KaiGraphicsPenInfo describes a KaiGraphicsPen
// ----------------------------------------------------------------------------

class KaiGraphicsPenInfo : public KaiPenInfoBase<KaiGraphicsPenInfo>
{
public:
	explicit KaiGraphicsPenInfo(const KaiColour& colour = KaiColour(),
		KaiDouble width = 1.0,
		KaiPenStyle style = KaiPENSTYLE_SOLID)
		: KaiPenInfoBase<KaiGraphicsPenInfo>(colour, style)
	{
		m_width = width;
		m_gradientType = KaiGRADIENT_NONE;
	}

	// Setters

	KaiGraphicsPenInfo& Width(KaiDouble width)
	{
		m_width = width; return *this;
	}

	KaiGraphicsPenInfo&
		LinearGradient(KaiDouble x1, KaiDouble y1, KaiDouble x2, KaiDouble y2,
		const KaiColour& c1, const KaiColour& c2,
		KaiD2DMatrixData *  matrix = nullptr)
	{
		m_gradientType = KaiGRADIENT_LINEAR;
		m_x1 = x1;
		m_y1 = y1;
		m_x2 = x2;
		m_y2 = y2;
		m_stops.SetStartColour(c1);
		m_stops.SetEndColour(c2);
		m_matrix = matrix;
		return *this;
	}

	KaiGraphicsPenInfo&
		LinearGradient(KaiDouble x1, KaiDouble y1, KaiDouble x2, KaiDouble y2,
		const KaiGraphicsGradientStops& stops,
		KaiD2DMatrixData *  matrix = nullptr)
	{
		m_gradientType = KaiGRADIENT_LINEAR;
		m_x1 = x1;
		m_y1 = y1;
		m_x2 = x2;
		m_y2 = y2;
		m_stops = stops;
		m_matrix = matrix;
		return *this;
	}

	KaiGraphicsPenInfo&
		RadialGradient(KaiDouble startX, KaiDouble startY,
		KaiDouble endX, KaiDouble endY, KaiDouble radius,
		const KaiColour& oColor, const KaiColour& cColor,
		KaiD2DMatrixData * matrix = nullptr)
	{
		m_gradientType = KaiGRADIENT_RADIAL;
		m_x1 = startX;
		m_y1 = startY;
		m_x2 = endX;
		m_y2 = endY;
		m_radius = radius;
		m_stops.SetStartColour(oColor);
		m_stops.SetEndColour(cColor);
		m_matrix = matrix;
		return *this;
	}

	KaiGraphicsPenInfo&
		RadialGradient(KaiDouble startX, KaiDouble startY,
		KaiDouble endX, KaiDouble endY,
		KaiDouble radius, const KaiGraphicsGradientStops& stops,
		KaiD2DMatrixData * matrix = nullptr)
	{
		m_gradientType = KaiGRADIENT_RADIAL;
		m_x1 = startX;
		m_y1 = startY;
		m_x2 = endX;
		m_y2 = endY;
		m_radius = radius;
		m_stops = stops;
		m_matrix = matrix;
		return *this;
	}

	// Accessors

	KaiDouble GetWidth() const { return m_width; }
	KaiGradientType GetGradientType() const { return m_gradientType; }
	KaiDouble GetX1() const { return m_x1; }
	KaiDouble GetY1() const { return m_y1; }
	KaiDouble GetX2() const { return m_x2; }
	KaiDouble GetY2() const { return m_y2; }
	KaiDouble GetStartX() const { return m_x1; }
	KaiDouble GetStartY() const { return m_y1; }
	KaiDouble GetEndX() const { return m_x2; }
	KaiDouble GetEndY() const { return m_y2; }
	KaiDouble GetRadius() const { return m_radius; }
	const KaiGraphicsGradientStops& GetStops() const { return m_stops; }
	KaiD2DMatrixData * GetMatrix() const { return m_matrix; }

private:
	KaiDouble m_width;
	KaiGradientType m_gradientType;
	KaiDouble m_x1, m_y1, m_x2, m_y2; // also used for m_xo, m_yo, m_xc, m_yc
	KaiDouble m_radius;
	KaiGraphicsGradientStops m_stops;
	KaiD2DMatrixData * m_matrix = nullptr;
};

//-----------------------------------------------------------------------------
// KaiD2DPenData declaration
//-----------------------------------------------------------------------------

class KaiD2DPenData : /*public GraphicsObjectRefData, */public KaiD2DManagedGraphicsData
{
public:
	KaiD2DPenData(KaiD2DRenderer* renderer,
				 ID2D1Factory* direct2dFactory,
				 const KaiGraphicsPenInfo& info);


	void CreateStrokeStyle(ID2D1Factory* const direct2dfactory);

	ID2D1Brush* GetBrush();

	FLOAT GetWidth();

	ID2D1StrokeStyle* GetStrokeStyle();

	KaiD2DManagedObject* GetManagedObject() override
	{
		return m_stippleBrush->GetManagedObject();
	}

private:
	// We store the original pen description for later when we need to recreate
	// the device-dependent resources.
	const KaiGraphicsPenInfo m_penInfo;

	// A stroke style is a device-independent resource.
	// Describes the caps, miter limit, line join, and dash information.
	KaiCOMPtr<ID2D1StrokeStyle> m_strokeStyle;

	// Drawing outlines with Direct2D requires a brush for the color or stipple.
	KaiSharedPtr<KaiD2DBrushData> m_stippleBrush;

	// The width of the stroke
	FLOAT m_width;
};



//-----------------------------------------------------------------------------
// KaiD2DPenData implementation
//-----------------------------------------------------------------------------

KaiD2DPenData::KaiD2DPenData(
	KaiD2DRenderer* renderer,
	ID2D1Factory* direct2dFactory,
	const KaiGraphicsPenInfo& info)
	: /*GraphicsObjectRefData(renderer),*/
	  m_penInfo(info),
	  m_width(info.GetWidth())
{
	CreateStrokeStyle(direct2dFactory);

	KaiBrush strokeBrush;

	if (m_penInfo.GetStyle() == KaiPENSTYLE_STIPPLE)
	{
		strokeBrush.SetStipple(m_penInfo.GetStipple());
		strokeBrush.SetStyle(KaiBRUSHSTYLE_STIPPLE);
	}
	else if(KaiIsHatchPenStyle(m_penInfo.GetStyle()))
	{
		strokeBrush.SetStyle(KaiConvertPenStyleToBrushStyle(m_penInfo.GetStyle()));
		strokeBrush.SetColour(m_penInfo.GetColour());
	}
	else
	{
		strokeBrush.SetColour(m_penInfo.GetColour());
		strokeBrush.SetStyle(KaiBRUSHSTYLE_SOLID);
	}

	switch ( m_penInfo.GetGradientType() )
	{
	case KaiGRADIENT_NONE:
		m_stippleBrush = new KaiD2DBrushData(renderer, strokeBrush);
		break;

	case KaiGRADIENT_LINEAR:
		m_stippleBrush = new KaiD2DBrushData(renderer);
		m_stippleBrush->CreateLinearGradientBrush(
								m_penInfo.GetX1(), m_penInfo.GetY1(),
								m_penInfo.GetX2(), m_penInfo.GetY2(),
								m_penInfo.GetStops(),
								m_penInfo.GetMatrix());
		break;

	case KaiGRADIENT_RADIAL:
		m_stippleBrush = new KaiD2DBrushData(renderer);
		m_stippleBrush->CreateRadialGradientBrush(
								m_penInfo.GetStartX(), m_penInfo.GetStartY(),
								m_penInfo.GetEndX(), m_penInfo.GetEndY(),
								m_penInfo.GetRadius(),
								m_penInfo.GetStops(),
								m_penInfo.GetMatrix());
		break;
	}
}


void KaiD2DPenData::CreateStrokeStyle(ID2D1Factory* const direct2dfactory)
{
	D2D1_CAP_STYLE capStyle = KaiD2DConvertPenCap(m_penInfo.GetCap());
	D2D1_LINE_JOIN lineJoin = KaiD2DConvertPenJoin(m_penInfo.GetJoin());
	D2D1_DASH_STYLE dashStyle = KaiD2DConvertPenStyle(m_penInfo.GetStyle());

	int dashCount = 0;
	FLOAT* dashes = nullptr;

	if (dashStyle == D2D1_DASH_STYLE_CUSTOM)
	{
		dashCount = m_penInfo.GetDashCount();
		dashes = new FLOAT[dashCount];

		for (int i = 0; i < dashCount; ++i)
		{
			dashes[i] = m_penInfo.GetDash()[i];
		}

	}

	direct2dfactory->CreateStrokeStyle(
		D2D1::StrokeStyleProperties(capStyle, capStyle, capStyle, lineJoin, 0, dashStyle, 0.0f),
		dashes, dashCount,
		&m_strokeStyle);

	delete[] dashes;
}

ID2D1Brush* KaiD2DPenData::GetBrush()
{
	return m_stippleBrush->GetBrush();
}

FLOAT KaiD2DPenData::GetWidth()
{
	return m_width;
}

ID2D1StrokeStyle* KaiD2DPenData::GetStrokeStyle()
{
	return m_strokeStyle;
}

//KaiD2DPenData* KaiGetD2DPenData(const KaiGraphicsPen& pen)
//{
//    return static_cast<KaiD2DPenData*>(pen.GetGraphicsData());
//}

class KaiD2DFontData/* : public GraphicsObjectRefData*/// : public KaiD2DManagedGraphicsData
{
public:
	KaiD2DFontData(KaiD2DRenderer* renderer, const KaiFont& font, const KaiRealPoint& dpi, const KaiColor& color);

	KaiCOMPtr<IDWriteTextLayout> CreateTextLayout(const KaiString& text) const;

	KaiD2DBrushData& GetBrushData() { return m_brushData; }

	KaiCOMPtr<IDWriteTextFormat> GetTextFormat() const { return m_textFormat; }

	KaiCOMPtr<IDWriteFont> GetFont() { return m_font; }

private:
	// The native, device-independent font object
	KaiCOMPtr<IDWriteFont> m_font;

	// The native, device-independent font object
	KaiCOMPtr<IDWriteTextFormat> m_textFormat;

	// We use a color brush to render the font
	KaiD2DBrushData m_brushData;

	bool m_underlined;

	bool m_strikethrough;
};

KaiD2DFontData::KaiD2DFontData(KaiD2DRenderer* renderer, const KaiFont& font, const KaiRealPoint& dpi, const KaiColor& color) :
	/*GraphicsObjectRefData(renderer), */m_brushData(renderer, KaiBrush(color)),
	m_underlined(font.GetUnderlined()), m_strikethrough(font.GetStrikethrough())
{
	HRESULT hr;

	KaiCOMPtr<IDWriteGdiInterop> gdiInterop;
	hr = KaiDWriteFactory()->GetGdiInterop(&gdiInterop);
	KaiCHECK_HRESULT_RET(hr);

	LOGFONTW logfont;
	int n = GetObjectW(font.GetHFONT(), sizeof(logfont), &logfont);
	KaiCHECK_RET( n > 0, KaiS("Failed to obtain font info") );

	// Ensure the LOGFONT object contains the correct font face name
	if (logfont.lfFaceName[0] == L'\0')
	{
		// The length of the font name must not exceed LF_FACESIZE TCHARs,
		// including the terminating nullptr.
		KaiString name = font.GetFaceName().Mid(0, WXSIZEOF(logfont.lfFaceName)-1);
		for (unsigned int i = 0; i < name.Length(); ++i)
		{
			logfont.lfFaceName[i] = name.GetChar(i);
		}
		logfont.lfFaceName[name.Length()] = L'\0';
	}

	hr = gdiInterop->CreateFontFromLOGFONT(&logfont, &m_font);
	if ( hr == DWRITE_E_NOFONT )
	{
		// It was attempted to create DirectWrite font from non-TrueType GDI font.
		return;
	}

	KaiCHECK_RET( SUCCEEDED(hr),
				 KaiString::Format("Failed to create font '%s' (HRESULT = %x)", logfont.lfFaceName, hr) );

	KaiCOMPtr<IDWriteFontFamily> fontFamily;
	hr = m_font->GetFontFamily(&fontFamily);
	KaiCHECK_HRESULT_RET(hr);

	KaiCOMPtr<IDWriteLocalizedStrings> familyNames;
	hr = fontFamily->GetFamilyNames(&familyNames);
	KaiCHECK_HRESULT_RET(hr);

	UINT32 length;
	hr = familyNames->GetStringLength(0, &length);
	KaiCHECK_HRESULT_RET(hr);

	wchar_t* name = new wchar_t[length+1];
	hr = familyNames->GetString(0, name, length+1);
	KaiCHECK_HRESULT_RET(hr);

	FLOAT fontSize = (FLOAT)(!dpi.y
		? font.GetPixelSize().GetHeight()
		: (font.GetPointSize()/*.GetFractionalPointSize()*/ * dpi.y / 72.0f));

	hr = KaiDWriteFactory()->CreateTextFormat(
		name,
		nullptr,
		m_font->GetWeight(),
		m_font->GetStyle(),
		m_font->GetStretch(),
		fontSize,
		L"en-us",
		&m_textFormat);

	delete[] name;

	KaiCHECK_HRESULT_RET(hr);
}

KaiCOMPtr<IDWriteTextLayout> KaiD2DFontData::CreateTextLayout(const KaiString& text) const
{
	static const FLOAT MAX_WIDTH = FLT_MAX;
	static const FLOAT MAX_HEIGHT = FLT_MAX;

	HRESULT hr;

	KaiCOMPtr<IDWriteTextLayout> textLayout;

	hr = KaiDWriteFactory()->CreateTextLayout(
		text.c_str(),
		text.length(),
		m_textFormat,
		MAX_WIDTH,
		MAX_HEIGHT,
		&textLayout);
	KaiCHECK2_HRESULT_RET(hr, KaiCOMPtr<IDWriteTextLayout>(nullptr));

	DWRITE_TEXT_RANGE textRange = { 0, (UINT32) text.length() };

	if (m_underlined)
	{
		textLayout->SetUnderline(true, textRange);
	}

	if (m_strikethrough)
	{
		textLayout->SetStrikethrough(true, textRange);
	}

	return textLayout;
}

//KaiD2DFontData* KaiGetD2DFontData(const KaiGraphicsFont& font)
//{
//    return static_cast<KaiD2DFontData*>(font.GetGraphicsData());
//}

// A render target resource holder exposes methods relevant
// for native render targets such as resize
class KaiD2DRenderTargetResourceHolder : public KaiD2DResourceHolder<ID2D1RenderTarget>
{
public:
	// This method is called when an external event signals the underlying DC
	// is resized (e.g. the resizing of a window). Some implementations can leave
	// this method empty, while others must adjust the render target size to match
	// the underlying DC.
	virtual void Resize()
	{
	}

	// We use this method instead of the one provided by the native render target
	// because Direct2D 1.0 render targets do not accept a composition mode
	// parameter, while the device context in Direct2D 1.1 does. This way, we make
	// best use of the capabilities of each render target.
	//
	// The default implementation works for all render targets, but the D2D 1.0
	// render target holders shouldn't need to override it, since none of the
	// 1.0 render targets offer a better version of this method.
	virtual void DrawBitmap(ID2D1Bitmap* bitmap, D2D1_POINT_2F offset,
		D2D1_RECT_F imageRectangle, KaiInterpolationQuality interpolationQuality,
		KaiCompositionMode WXUNUSED(compositionMode))
	{
		D2D1_RECT_F destinationRectangle = D2D1::RectF(offset.x, offset.y, offset.x + imageRectangle.right, offset.y + imageRectangle.bottom);
		m_nativeResource->DrawBitmap(
			bitmap,
			destinationRectangle,
			1.0f,
			KaiD2DConvertBitmapInterpolationMode(interpolationQuality),
			imageRectangle);
	}

	// We use this method instead of the one provided by the native render target
	// because some contexts might require writing to a buffer (e.g. an image
	// context), and some render targets might require additional operations to
	// be executed (e.g. the device context must present the swap chain)
	virtual HRESULT Flush()
	{
		return m_nativeResource->Flush();
	}

	// Composition is not supported at in D2D 1.0, and we only allow for:
	// KaiCOMPOSITION_DEST - which is essentially a no-op and is handled
	//                      externally by preventing any draw calls.
	// KaiCOMPOSITION_OVER - which copies the source over the destination using
	//                      alpha blending. This is the default way D2D 1.0
	//                      draws images.
	virtual bool SetCompositionMode(KaiCompositionMode compositionMode)
	{
		if (compositionMode == KaiCOMPOSITION_DEST ||
			compositionMode == KaiCOMPOSITION_OVER)
		{
			// There's nothing we can do but notify the caller the composition
			// mode is supported
			return true;
		}

		return false;
	}
};

#if KaiUSE_IMAGE
class KaiD2DImageRenderTargetResourceHolder : public KaiD2DRenderTargetResourceHolder
{
public:
	KaiD2DImageRenderTargetResourceHolder(KaiImage* image, ID2D1Factory* factory) :
		m_resultImage(image), m_factory(factory)
	{
	}

	HRESULT Flush() override
	{
		HRESULT hr = m_nativeResource->Flush();
		FlushRenderTargetToImage();
		return hr;
	}

	~KaiD2DImageRenderTargetResourceHolder()
	{
		FlushRenderTargetToImage();
	}

protected:
	void DoAcquireResource() override
	{
		HRESULT hr;

		// Create a compatible WIC Bitmap
		hr = KaiWICImagingFactory()->CreateBitmap(
			m_resultImage->GetWidth(),
			m_resultImage->GetHeight(),
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnDemand,
			&m_wicBitmap);
		KaiCHECK_HRESULT_RET(hr);

		// Copy contents of source image to the WIC bitmap.
		const int width = m_resultImage->GetWidth();
		const int height = m_resultImage->GetHeight();
		WICRect rcLock = { 0, 0, width, height };
		IWICBitmapLock *pLock = nullptr;
		hr = m_wicBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);
		KaiCHECK_HRESULT_RET(hr);

		UINT rowStride = 0;
		hr = pLock->GetStride(&rowStride);
		if ( FAILED(hr) )
		{
			pLock->Release();
			KaiFAILED_HRESULT_MSG(hr);
			return;
		}

		UINT bufferSize = 0;
		BYTE *pBmpBuffer = nullptr;
		hr = pLock->GetDataPointer(&bufferSize, &pBmpBuffer);
		if ( FAILED(hr) )
		{
			pLock->Release();
			KaiFAILED_HRESULT_MSG(hr);
			return;
		}

		const unsigned char *imgRGB = m_resultImage->GetData();    // source RGB buffer
		const unsigned char *imgAlpha = m_resultImage->GetAlpha(); // source alpha buffer
		for( int y = 0; y < height; y++ )
		{
			BYTE *pPixByte = pBmpBuffer;
			for ( int x = 0; x < width; x++ )
			{
				unsigned char r = *imgRGB++;
				unsigned char g = *imgRGB++;
				unsigned char b = *imgRGB++;
				unsigned char a = imgAlpha ? *imgAlpha++ : 255;
				// Premultiply RGB values
				*pPixByte++ = (b * a + 127) / 255;
				*pPixByte++ = (g * a + 127) / 255;
				*pPixByte++ = (r * a + 127) / 255;
				*pPixByte++ = a;
			}

			pBmpBuffer += rowStride;
		}

		pLock->Release();

		// Create the render target
		hr = m_factory->CreateWicBitmapRenderTarget(
			m_wicBitmap,
			D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_SOFTWARE,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			&m_nativeResource);
		KaiCHECK_HRESULT_RET(hr);
	}

private:
	void FlushRenderTargetToImage()
	{
		const int width = m_resultImage->GetWidth();
		const int height = m_resultImage->GetHeight();

		WICRect rcLock = { 0, 0, width, height };
		IWICBitmapLock *pLock = nullptr;
		HRESULT hr = m_wicBitmap->Lock(&rcLock, WICBitmapLockRead, &pLock);
		KaiCHECK_HRESULT_RET(hr);

		UINT rowStride = 0;
		hr = pLock->GetStride(&rowStride);
		if ( FAILED(hr) )
		{
			pLock->Release();
			KaiFAILED_HRESULT_MSG(hr);
			return;
		}

		UINT bufferSize = 0;
		BYTE *pBmpBuffer = nullptr;
		hr = pLock->GetDataPointer(&bufferSize, &pBmpBuffer);
		if ( FAILED(hr) )
		{
			pLock->Release();
			KaiFAILED_HRESULT_MSG(hr);
			return;
		}

		WICPixelFormatGUID pixelFormat;
		hr = pLock->GetPixelFormat(&pixelFormat);
		if ( FAILED(hr) )
		{
			pLock->Release();
			KaiFAILED_HRESULT_MSG(hr);
			return;
		}
		KaiASSERT_MSG( pixelFormat == GUID_WICPixelFormat32bppPBGRA ||
				  pixelFormat == GUID_WICPixelFormat32bppBGR,
				  KaiS("Unsupported pixel format") );

		// Only premultiplied ARGB bitmaps are supported.
		const bool hasAlpha = pixelFormat == GUID_WICPixelFormat32bppPBGRA;

		unsigned char* destRGB = m_resultImage->GetData();
		unsigned char* destAlpha = m_resultImage->GetAlpha();
		for( int y = 0; y < height; y++ )
		{
			BYTE *pPixByte = pBmpBuffer;
			for ( int x = 0; x < width; x++ )
			{
				KaiPBGRAColor color = KaiPBGRAColor(pPixByte);
				unsigned char a =  hasAlpha ? color.a : 255;
				// Undo premultiplication for ARGB bitmap
				*destRGB++ = (a > 0 && a < 255) ? ( color.r * 255 ) / a : color.r;
				*destRGB++ = (a > 0 && a < 255) ? ( color.g * 255 ) / a : color.g;
				*destRGB++ = (a > 0 && a < 255) ? ( color.b * 255 ) / a : color.b;
				if ( destAlpha )
					*destAlpha++ = a;

				pPixByte += 4;
			}

			pBmpBuffer += rowStride;
		}

		pLock->Release();
   }

private:
	KaiImage* m_resultImage;
	KaiCOMPtr<IWICBitmap> m_wicBitmap;

	ID2D1Factory* m_factory;
};
#endif // KaiUSE_IMAGE

class KaiD2DHwndRenderTargetResourceHolder : public KaiD2DRenderTargetResourceHolder
{
public:
	typedef ID2D1HwndRenderTarget* ImplementationType;

	KaiD2DHwndRenderTargetResourceHolder(HWND hwnd, ID2D1Factory* factory) :
		m_hwnd(hwnd), m_factory(factory)
	{
	}

	void Resize() override
	{
		RECT clientRect;
		GetClientRect(m_hwnd, &clientRect);

		D2D1_SIZE_U hwndSize = D2D1::SizeU(
			clientRect.right - clientRect.left,
			clientRect.bottom - clientRect.top);

		D2D1_SIZE_U renderTargetSize = GetRenderTarget()->GetPixelSize();

		if (hwndSize.width != renderTargetSize.width || hwndSize.height != renderTargetSize.height)
		{
			GetRenderTarget()->Resize(hwndSize);
		}
	}

protected:
	void DoAcquireResource() override
	{
		KaiCOMPtr<ID2D1HwndRenderTarget> renderTarget;

		HRESULT result;

		RECT clientRect;
		GetClientRect(m_hwnd, &clientRect);

		D2D1_SIZE_U size = D2D1::SizeU(
			clientRect.right - clientRect.left,
			clientRect.bottom - clientRect.top);

		result = m_factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&renderTarget);

		if (FAILED(result))
		{
			KaiFAIL_MSG("Could not create Direct2D render target");
		}

		renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_nativeResource = renderTarget;
	}

private:
	// Converts the underlying resource pointer of type
	// ID2D1RenderTarget* to the actual implementation type
	ImplementationType GetRenderTarget()
	{
		return static_cast<ImplementationType>(GetD2DResource().get());
	}

private:
	HWND m_hwnd;
	ID2D1Factory* m_factory;
};

#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
class KaiD2DDeviceContextResourceHolder : public KaiD2DRenderTargetResourceHolder
{
public:
	KaiD2DDeviceContextResourceHolder(ID2D1Factory* factory, HWND hwnd) :
		m_factory(nullptr), m_hwnd(hwnd)
	{
		HRESULT hr = factory->QueryInterface(IID_ID2D1Factory1, (void**)&m_factory);
		KaiCHECK_HRESULT_RET(hr);
	}

	void DrawBitmap(ID2D1Bitmap* image, D2D1_POINT_2F offset,
		D2D1_RECT_F imageRectangle, KaiInterpolationQuality interpolationQuality,
		KaiCompositionMode compositionMode) override
	{
		m_context->DrawImage(image,
			offset,
			imageRectangle,
			KaiD2DConvertInterpolationMode(interpolationQuality),
			KaiD2DConvertCompositionMode(compositionMode));
	}

	HRESULT Flush() override
	{
		HRESULT hr = m_nativeResource->Flush();
		DXGI_PRESENT_PARAMETERS params = { 0 };
		m_swapChain->Present1(1, 0, &params);
		return hr;
	}

protected:

	// Adapted from http://msdn.microsoft.com/en-us/library/windows/desktop/hh780339%28v=vs.85%29.aspx
	void DoAcquireResource() override
	{
		HRESULT hr;

		// This flag adds support for surfaces with a different color channel ordering than the API default.
		// You need it for compatibility with Direct2D.
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		// This array defines the set of DirectX hardware feature levels this app  supports.
		// The ordering is important and you should  preserve it.
		// Don't forget to declare your app's minimum required feature level in its
		// description.  All apps are assumed to support 9.1 unless otherwise stated.
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		// Create the DX11 API device object, and get a corresponding context.
		KaiCOMPtr<ID3D11Device> device;
		KaiCOMPtr<ID3D11DeviceContext> context;

		hr = D3D11CreateDevice(
			nullptr,                    // specify null to use the default adapter
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			creationFlags,              // optionally set debug and Direct2D compatibility flags
			featureLevels,              // list of feature levels this app can support
			ARRAYSIZE(featureLevels),   // number of possible feature levels
			D3D11_SDK_VERSION,
			&device,                    // returns the Direct3D device created
			&m_featureLevel,            // returns feature level of device created
			&context);                  // returns the device immediate context
		KaiCHECK_HRESULT_RET(hr);

		// Obtain the underlying DXGI device of the Direct3D11 device.
		hr = device->QueryInterface(IID_IDXGIDevice, (void**)&m_dxgiDevice);
		KaiCHECK_HRESULT_RET(hr);

		// Obtain the Direct2D device for 2-D rendering.
		hr = m_factory->CreateDevice(m_dxgiDevice, &m_device);
		KaiCHECK_HRESULT_RET(hr);

		// Get Direct2D device's corresponding device context object.
		hr = m_device->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_context);
		KaiCHECK_HRESULT_RET(hr);

		m_nativeResource = m_context;

		AttachSurface();
	}

private:
	void AttachSurface()
	{
		HRESULT hr;

		// Allocate a descriptor.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = 0;
		swapChainDesc.Height = 0;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.Flags = 0;

		// Identify the physical adapter (GPU or card) this device is runs on.
		KaiCOMPtr<IDXGIAdapter> dxgiAdapter;
		hr = m_dxgiDevice->GetAdapter(&dxgiAdapter);
		KaiCHECK_HRESULT_RET(hr);

		// Get the factory object that created the DXGI device.
		KaiCOMPtr<IDXGIFactory2> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		KaiCHECK_HRESULT_RET(hr);

		// Get the final swap chain for this window from the DXGI factory.
		hr = dxgiFactory->CreateSwapChainForHwnd(
			m_dxgiDevice,
			m_hwnd,
			&swapChainDesc,
			nullptr,    // allow on all displays
			nullptr,
			&m_swapChain);
		KaiCHECK_HRESULT_RET(hr);

		// Ensure that DXGI doesn't queue more than one frame at a time.
		hr = m_dxgiDevice->SetMaximumFrameLatency(1);
		KaiCHECK_HRESULT_RET(hr);

		// Get the backbuffer for this window which is be the final 3D render target.
		KaiCOMPtr<ID3D11Texture2D> backBuffer;
		hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		KaiCHECK_HRESULT_RET(hr);

		FLOAT dpiX, dpiY;
		m_factory->GetDesktopDpi(&dpiX, &dpiY);

		// Now we set up the Direct2D render target bitmap linked to the swapchain.
		// Whenever we render to this bitmap, it is directly rendered to the
		// swap chain associated with the window.
		D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			dpiX, dpiY);

		// Direct2D needs the dxgi version of the backbuffer surface pointer.
		KaiCOMPtr<IDXGISurface> dxgiBackBuffer;
		hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
		KaiCHECK_HRESULT_RET(hr);

		// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
		hr = m_context->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.get(),
			&bitmapProperties,
			&m_targetBitmap);
		KaiCHECK_HRESULT_RET(hr);

		// Now we can set the Direct2D render target.
		m_context->SetTarget(m_targetBitmap);
	}

	~KaiD2DDeviceContextResourceHolder()
	{
		DXGI_PRESENT_PARAMETERS params = { 0 };
		m_swapChain->Present1(1, 0, &params);
	}

private:
	ID2D1Factory1* m_factory;

	HWND m_hwnd;

	D3D_FEATURE_LEVEL m_featureLevel;
	KaiCOMPtr<IDXGIDevice1> m_dxgiDevice;
	KaiCOMPtr<ID2D1Device> m_device;
	KaiCOMPtr<ID2D1DeviceContext> m_context;
	KaiCOMPtr<ID2D1Bitmap1> m_targetBitmap;
	KaiCOMPtr<IDXGISwapChain1> m_swapChain;
};
#endif

class KaiD2DDCRenderTargetResourceHolder : public KaiD2DRenderTargetResourceHolder
{
public:
	KaiD2DDCRenderTargetResourceHolder(ID2D1Factory* factory, HDC hdc, D2D1_ALPHA_MODE alphaMode) :
		m_factory(factory), m_hdc(hdc), m_alphaMode(alphaMode)
	{
	}

protected:
	void DoAcquireResource() override
	{
		KaiCOMPtr<ID2D1DCRenderTarget> renderTarget;
		D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, m_alphaMode));

		HRESULT hr = m_factory->CreateDCRenderTarget(
			&renderTargetProperties,
			&renderTarget);
		KaiCHECK_HRESULT_RET(hr);

		// We want draw on the entire device area.
		// GetClipBox() retrieves logical size of DC
		// what is what we need to pass to BindDC.
		RECT r;
		int status = ::GetClipBox(m_hdc, &r);
		KaiCHECK_RET( status != ERROR, KaiS("Error retrieving DC dimensions") );

		hr = renderTarget->BindDC(m_hdc, &r);
		KaiCHECK_HRESULT_RET(hr);
		renderTarget->SetTransform(
					   D2D1::Matrix3x2F::Translation(-r.left, -r.top));

		m_nativeResource = renderTarget;
	}

private:
	ID2D1Factory* m_factory;
	HDC m_hdc;
	D2D1_ALPHA_MODE m_alphaMode;
};

// The null context has no state of its own and does nothing.
// It is only used as a base class for the lightweight
// measuring context. The measuring context cannot inherit from
// the default implementation KaiD2DContext, because some methods
// from KaiD2DContext require the presence of a "context"
// (render target) in order to acquire various device-dependent
// resources. Without a proper context, those methods would fail.
// The methods implemented in the null context are fundamentally no-ops.
class KaiNullContext : public GraphicsContext
{
public:
	KaiNullContext(KaiD2DRenderer* _renderer) : GraphicsContext(){ renderer = _renderer; }
	~KaiNullContext(){ KaiDELETE(m_font); }
	void GetTextExtent(const KaiString&, KaiDouble*, KaiDouble*, KaiDouble*, KaiDouble*) const {}
	void GetPartialTextExtents(const KaiString&, KaiArrayDouble&) const {}
	void Clip(const KaiRegion&) {}
	void Clip(KaiDouble, KaiDouble, KaiDouble, KaiDouble) {}
	void ResetClip() {}
	void GetClipBox(KaiDouble*, KaiDouble*, KaiDouble*, KaiDouble*) {}
	void* GetNativeContext() { return nullptr; }
	bool SetAntialiasMode(KaiAntialiasMode) { return false; }
	bool SetInterpolationQuality(KaiInterpolationQuality) { return false; }
	bool SetCompositionMode(KaiCompositionMode) { return false; }
	void BeginLayer(KaiDouble) {}
	void EndLayer() {}
	void Translate(KaiDouble, KaiDouble) {}
	void Scale(KaiDouble, KaiDouble) {}
	void Rotate(KaiDouble) {}
	void ConcatTransform(const KaiGraphicsMatrix&) {}
	void SetTransform(const KaiGraphicsMatrix&) {}
	KaiD2DMatrixData *GetTransform() const { return nullptr; }
	void StrokePath(const KaiGraphicsPath&) {}
	void FillPath(const KaiGraphicsPath&, KaiPolygonFillMode) {}
	void DrawBitmap(const KaiGraphicsBitmap&, KaiDouble, KaiDouble, KaiDouble, KaiDouble) {}
	void DrawBitmap(const KaiBitmap&, KaiDouble, KaiDouble, KaiDouble, KaiDouble) {}
	void DrawIcon(const KaiIcon&, KaiDouble, KaiDouble, KaiDouble, KaiDouble) {}
	void PushState() {}
	void PopState() {}
	void Flush() {}
	void SetFont(const KaiFont& font, const KaiColour& col);

protected:
	void DoDrawText(const KaiString&, KaiDouble, KaiDouble) {}
	KaiD2DFontData *m_font = nullptr;
	KaiD2DRenderer *renderer;
};



class KaiD2DMeasuringContext : public KaiNullContext
{
public:
	KaiD2DMeasuringContext(KaiD2DRenderer* renderer) : KaiNullContext(renderer) {}

	void GetTextExtent(const KaiString& str, KaiDouble* width, KaiDouble* height, KaiDouble* descent, KaiDouble* externalLeading) const
	{
		GetTextExtent(m_font, str, width, height, descent, externalLeading);
	}

	void GetPartialTextExtents(const KaiString& text, KaiArrayDouble& widths) const
	{
		GetPartialTextExtents(m_font, text, widths);
	}

	static void GetPartialTextExtents(KaiD2DFontData* fontData, const KaiString& text, KaiArrayDouble& widths)
	{
		for (unsigned int i = 0; i < text.Length(); ++i)
		{
			KaiDouble width;
			GetTextExtent(fontData, text.SubString(0, i), &width, nullptr, nullptr, nullptr);
			widths.push_back(width);
		}
	}

	static void GetTextExtent(KaiD2DFontData* fontData, const KaiString& str, KaiDouble* width, KaiDouble* height, KaiDouble* descent, KaiDouble* externalLeading)
	{
		if (!fontData)
			return;

		KaiCOMPtr<IDWriteTextLayout> textLayout = fontData->CreateTextLayout(str);
		KaiCOMPtr<IDWriteFont> font = fontData->GetFont();

		DWRITE_TEXT_METRICS textMetrics;
		textLayout->GetMetrics(&textMetrics);

		DWRITE_FONT_METRICS fontMetrics;
		font->GetMetrics(&fontMetrics);

		FLOAT ratio = fontData->GetTextFormat()->GetFontSize() / (FLOAT)fontMetrics.designUnitsPerEm;

		if (width != nullptr) *width = textMetrics.widthIncludingTrailingWhitespace;
		if (height != nullptr) *height = textMetrics.height;

		if (descent != nullptr) *descent = fontMetrics.descent * ratio;
		if (externalLeading != nullptr) *externalLeading = KaiMax(0.0f, (fontMetrics.ascent + fontMetrics.descent) * ratio - textMetrics.height);
	}
};



//-----------------------------------------------------------------------------
// KaiD2DContext declaration
//-----------------------------------------------------------------------------

class KaiD2DContext : public GraphicsContext, KaiD2DResourceManager
{
private:
	KaiD2DRenderer* m_renderer;
public:
	// Create the context for the given HWND, which may be associated (if it's
	// non-null) with the given KaiWindow.
	KaiD2DContext(KaiD2DRenderer* renderer,
				 ID2D1Factory* direct2dFactory,
				 HWND hwnd,
				 KaiWindow* window = nullptr);

	// Create the context for the given HDC which may be associated (if it's
	// non-null) with the given KaiDC.
	KaiD2DContext(KaiD2DRenderer* renderer,
				 ID2D1Factory* direct2dFactory,
				 HDC hdc,
				 const KaiDC* dc = nullptr,
				 D2D1_ALPHA_MODE alphaMode = D2D1_ALPHA_MODE_IGNORE);

#if KaiUSE_IMAGE
	KaiD2DContext(KaiD2DRenderer* renderer, ID2D1Factory* direct2dFactory, KaiImage& image);
#endif // KaiUSE_IMAGE

	KaiD2DContext(KaiD2DRenderer* renderer, ID2D1Factory* direct2dFactory, void* nativeContext);

	~KaiD2DContext();

	void Clip(const KaiRegion& region);

	void Clip(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h);

	void ResetClip();

	void GetClipBox(KaiDouble* x, KaiDouble* y, KaiDouble* w, KaiDouble* h);

	// The native context used by KaiD2DContext is a Direct2D render target.
	void* GetNativeContext();

	bool SetAntialiasMode(KaiAntialiasMode antialias);

	bool SetInterpolationQuality(KaiInterpolationQuality interpolation);

	bool SetCompositionMode(KaiCompositionMode op);

	void BeginLayer(KaiDouble opacity);

	void EndLayer();

	void Translate(KaiDouble dx, KaiDouble dy);

	void Scale(KaiDouble xScale, KaiDouble yScale);

	void Rotate(KaiDouble angle);

	void ConcatTransform(GraphicsMatrixData* matrix);

	void SetTransform(GraphicsMatrixData* matrix);

	GraphicsMatrixData *GetTransform() const;

	void StrokePath(GraphicsPathData * p);

	void FillPath(GraphicsPathData * p, KaiPolygonFillMode fillStyle = KaiODDEVEN_RULE);

	void DrawRectangle(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h);

	void DrawRoundedRectangle(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h, KaiDouble radius);

	void DrawEllipse(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h);

	void DrawBitmap(GraphicsBitmapData * bmp, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h);

	void DrawBitmap(const KaiBitmap& bmp, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h);

	void DrawIcon(const KaiIcon& icon, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h);

	GraphicsPathData * CreatePath();

	void SetPen(const KaiPen& pen, double width = 1.0);

	void SetBrush(const KaiBrush& brush);

	void SetFont(const KaiFont& font, const KaiColour& col);

	void PushState();

	void PopState();

	void GetTextExtent(
		const KaiString& str,
		KaiDouble* width,
		KaiDouble* height,
		KaiDouble* descent,
		KaiDouble* externalLeading) const;

	void GetPartialTextExtents(const KaiString& text, KaiArrayDouble& widths) const;

	bool ShouldOffset() const;

	void SetPen(const KaiD2DPenData& pen);

	void Flush();

	void GetDPI(KaiDouble* dpiX, KaiDouble* dpiY) const;

	KaiD2DContextSupplier::ContextType GetContext()
	{
		return GetRenderTarget();
	}

private:
	void Init();

	void DoDrawText(const KaiString& str, KaiDouble x, KaiDouble y);

	void EnsureInitialized();

	HRESULT CreateRenderTarget();

	void AdjustRenderTargetSize();

	void ReleaseDeviceDependentResources();

	ID2D1RenderTarget* GetRenderTarget() const;

	void SetClipLayer(ID2D1Geometry* clipGeometry);

	KaiD2DRenderer *GetRenderer(){ return m_renderer; };

	GraphicsMatrixData *CreateMatrix(KaiDouble a = 1.0, KaiDouble b = 0.0, KaiDouble c = 0.0, KaiDouble d = 1.0,
		KaiDouble tx = 0.0, KaiDouble ty = 0.0) const;

	KaiWindow* GetWindow() const { return m_window; }

	KaiInterpolationQuality GetInterpolationQuality() const { return m_interpolation; }

	KaiCompositionMode GetCompositionMode() const { return m_composition; }

	GraphicsBitmapData *CreateBitmap(const KaiBitmap& bmp) const;

private:
	enum LayerType
	{
		CLIP_LAYER,
		OTHER_LAYER
	};

	struct LayerData
	{
		LayerType type;
		D2D1_LAYER_PARAMETERS params;
		KaiCOMPtr<ID2D1Layer> layer;
		KaiCOMPtr<ID2D1Geometry> geometry;
		D2D1_MATRIX_3X2_F transformMatrix;
	};

	struct StateData
	{
		// A ID2D1DrawingStateBlock represents the drawing state of a render target:
		// the anti aliasing mode, transform, tags, and text-rendering options.
		// The context owns these pointers and is responsible for releasing them.
		KaiCOMPtr<ID2D1DrawingStateBlock> drawingState;
		// We need to store also current layers.
		KaiStack<LayerData> layers;
	};

private:
	KaiDouble m_width,
		m_height;

	KaiD2DPenData * m_pen = nullptr;
	KaiD2DBrushData * m_brush = nullptr;
	KaiD2DFontData * m_font = nullptr;
	KaiAntialiasMode m_antialias;
	KaiCompositionMode m_composition;
	KaiInterpolationQuality m_interpolation;
	bool m_enableOffset;
	ID2D1Factory* m_direct2dFactory;
	KaiSharedPtr<KaiD2DRenderTargetResourceHolder> m_renderTargetHolder;
	KaiStack<StateData> m_stateStack;
	KaiStack<LayerData> m_layers;
	ID2D1RenderTarget* m_cachedRenderTarget;
	D2D1::Matrix3x2F m_initTransform;
	// Clipping box
	bool m_isClipBoxValid;
	double m_clipX1, m_clipY1, m_clipX2, m_clipY2;
	KaiWindow* m_window;
private:
	//KaiDECLARE_NO_COPY_CLASS(KaiD2DContext);
};

class KaiD2DOffsetHelper
{
public:
	KaiD2DOffsetHelper(KaiD2DContext* g);

	~KaiD2DOffsetHelper();

private:
	KaiD2DContext* m_context;
};

KaiD2DOffsetHelper::KaiD2DOffsetHelper(KaiD2DContext* g) : m_context(g)
{
	if (m_context->ShouldOffset())
	{
		m_context->Translate(0.5, 0.5);
	}
}

KaiD2DOffsetHelper::~KaiD2DOffsetHelper()
{
	if (m_context->ShouldOffset())
	{
		m_context->Translate(-0.5, -0.5);
	}
}
//-----------------------------------------------------------------------------
// KaiD2DContext implementation
//-----------------------------------------------------------------------------

KaiD2DContext::KaiD2DContext(KaiD2DRenderer* renderer,
						   ID2D1Factory* direct2dFactory,
						   HWND hwnd,
						   KaiWindow* window) :
	m_direct2dFactory(direct2dFactory),
#if KaiD2D_DEVICE_CONTEXT_SUPPORTED
	m_renderTargetHolder(new KaiD2DDeviceContextResourceHolder(direct2dFactory, hwnd)),
#else
	m_renderTargetHolder(new KaiD2DHwndRenderTargetResourceHolder(hwnd, direct2dFactory)),
#endif
	m_renderer(renderer)
{
	RECT r = KaiGetWindowRect(hwnd);
	m_width = r.right - r.left;
	m_height = r.bottom - r.top;
	Init();
}

KaiD2DContext::KaiD2DContext(KaiD2DRenderer* renderer,
						   ID2D1Factory* direct2dFactory,
						   HDC hdc,
						   const KaiDC* dc,
						   D2D1_ALPHA_MODE alphaMode) :
	m_direct2dFactory(direct2dFactory),
	m_renderTargetHolder(new KaiD2DDCRenderTargetResourceHolder(direct2dFactory, hdc, alphaMode)),
	m_renderer(renderer)
{
	if ( dc )
	{
		const KaiSize dcSize = dc->GetSize();
		m_width = dcSize.GetWidth();
		m_height = dcSize.GetHeight();
	}

	Init();
}

#if KaiUSE_IMAGE
KaiD2DContext::KaiD2DContext(KaiD2DRenderer* renderer, ID2D1Factory* direct2dFactory, KaiImage& image) :
	m_direct2dFactory(direct2dFactory),
	m_renderTargetHolder(new KaiD2DImageRenderTargetResourceHolder(&image, direct2dFactory)),
	m_renderer(renderer)
{
	m_width = image.GetWidth();
	m_height = image.GetHeight();
	Init();
}
#endif // KaiUSE_IMAGE

KaiD2DContext::KaiD2DContext(KaiD2DRenderer* renderer, ID2D1Factory* direct2dFactory, void* nativeContext) :
	m_direct2dFactory(direct2dFactory),
	m_renderer(renderer)
{
	m_renderTargetHolder = *((KaiSharedPtr<KaiD2DRenderTargetResourceHolder>*)nativeContext);
	m_width = 0;
	m_height = 0;
	Init();
}

void KaiD2DContext::Init()
{
	m_cachedRenderTarget = nullptr;
	m_composition = KaiCOMPOSITION_OVER;
	m_renderTargetHolder->Bind(this);
	m_enableOffset = true;
	m_isClipBoxValid = false;
	m_clipX1 = m_clipY1 = m_clipX2 = m_clipY2 = 0.0;
	m_interpolation = KaiINTERPOLATION_BEST;
	m_antialias = KaiANTIALIAS_NONE;
	EnsureInitialized();
}

KaiD2DContext::~KaiD2DContext()
{
	// Remove all layers from the stack of layers.
	while ( !m_layers.empty() )
	{
		LayerData ld = m_layers.top();
		m_layers.pop();

		GetRenderTarget()->PopLayer();
		ld.layer.reset();
		ld.geometry.reset();
	}

	HRESULT result = GetRenderTarget()->EndDraw();
	KaiCHECK_HRESULT_RET(result);

	ReleaseResources();

	if (m_pen)
		delete m_pen;

	if (m_brush)
		delete m_brush;

	if (m_font)
		delete m_font;
}

ID2D1RenderTarget* KaiD2DContext::GetRenderTarget() const
{
	return m_cachedRenderTarget;
}

void KaiD2DContext::Clip(const KaiRegion& region)
{
	KaiCOMPtr<ID2D1Geometry> clipGeometry = KaiD2DConvertRegionToGeometry(m_direct2dFactory, region);

	SetClipLayer(clipGeometry);
}

void KaiD2DContext::Clip(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	KaiCOMPtr<ID2D1RectangleGeometry> clipGeometry;
	HRESULT hr = m_direct2dFactory->CreateRectangleGeometry(
						D2D1::RectF(x, y, x + w, y + h), &clipGeometry);
	KaiCHECK_HRESULT_RET(hr);

	SetClipLayer(clipGeometry);
}

void KaiD2DContext::SetClipLayer(ID2D1Geometry* clipGeometry)
{
	EnsureInitialized();

	KaiCOMPtr<ID2D1Layer> clipLayer;
	HRESULT hr = GetRenderTarget()->CreateLayer(&clipLayer);
	KaiCHECK_HRESULT_RET(hr);

	LayerData ld;
	ld.type = CLIP_LAYER;
	ld.params = D2D1::LayerParameters(D2D1::InfiniteRect(), clipGeometry,
									  KaiD2DConvertAntialiasMode(m_antialias));
	ld.layer = clipLayer;
	ld.geometry = clipGeometry;
	// We need to store CTM to be able to re-apply
	// the layer at the original position later on.
	GetRenderTarget()->GetTransform(&ld.transformMatrix);

	GetRenderTarget()->PushLayer(ld.params, clipLayer);
	// Store layer parameters.
	m_layers.push(ld);

	m_isClipBoxValid = false;
}

void KaiD2DContext::ResetClip()
{
	KaiStack<LayerData> layersToRestore;
	// Remove all clipping layers from the stack of layers.
	while ( !m_layers.empty() )
	{
		LayerData ld = m_layers.top();
		m_layers.pop();

		if ( ld.type == CLIP_LAYER )
		{
			GetRenderTarget()->PopLayer();
			ld.layer.reset();
			ld.geometry.reset();
			continue;
		}

		GetRenderTarget()->PopLayer();
		// Save non-clipping layer
		layersToRestore.push(ld);
	}

	HRESULT hr = GetRenderTarget()->Flush();
	KaiCHECK_HRESULT_RET(hr);

	// Re-apply all remaining non-clipping layers.
	// First, save current transformation matrix.
	D2D1_MATRIX_3X2_F currTransform;
	GetRenderTarget()->GetTransform(&currTransform);
	while ( !layersToRestore.empty() )
	{
		LayerData ld = layersToRestore.top();
		layersToRestore.pop();

		// Restore layer at original position.
		GetRenderTarget()->SetTransform(&ld.transformMatrix);
		GetRenderTarget()->PushLayer(ld.params, ld.layer);
		// Store layer parameters.
		m_layers.push(ld);
	}
	// Restore current transformation matrix.
	GetRenderTarget()->SetTransform(&currTransform);

	m_isClipBoxValid = false;
}

void KaiD2DContext::GetClipBox(KaiDouble* x, KaiDouble* y, KaiDouble* w, KaiDouble* h)
{
	if ( !m_isClipBoxValid )
	{
		// To obtain actual clipping box we have to start with rectangle
		// covering the entire render target and interesect with this rectangle
		// all clipping layers. Bounding box of the final geometry
		// (being intersection of all clipping layers) is a clipping box.

		HRESULT hr;
		KaiCOMPtr<ID2D1RectangleGeometry> rectGeometry;
		hr = m_direct2dFactory->CreateRectangleGeometry(
					D2D1::RectF(0.0F, 0.0F, (FLOAT)m_width, (FLOAT)m_height),
					&rectGeometry);
		KaiCHECK_HRESULT_RET(hr);

		KaiCOMPtr<ID2D1Geometry> clipGeometry(rectGeometry);

		KaiStack<LayerData> layers(m_layers);
		while( !layers.empty() )
		{
			LayerData ld = layers.top();
			layers.pop();

			if ( ld.type == CLIP_LAYER )
			{
				// If current geometry is empty (null region)
				// or there is no intersection between geometries
				// then final result is "null" rectangle geometry.
				FLOAT area;
				hr = ld.geometry->ComputeArea(ld.transformMatrix, &area);
				KaiCHECK_HRESULT_RET(hr);
				D2D1_GEOMETRY_RELATION geomRel;
				hr = clipGeometry->CompareWithGeometry(ld.geometry, ld.transformMatrix, &geomRel);
				KaiCHECK_HRESULT_RET(hr);
				if ( area <= FLT_MIN || geomRel == D2D1_GEOMETRY_RELATION_DISJOINT )
				{
					KaiCOMPtr<ID2D1RectangleGeometry> nullGeometry;
					hr = m_direct2dFactory->CreateRectangleGeometry(
								D2D1::RectF(0.0F, 0.0F, 0.0F, 0.0F), &nullGeometry);
					KaiCHECK_HRESULT_RET(hr);

					clipGeometry.reset();
					clipGeometry = nullGeometry;
					break;
				}

				KaiCOMPtr<ID2D1PathGeometry> pathGeometryClip;
				hr = m_direct2dFactory->CreatePathGeometry(&pathGeometryClip);
				KaiCHECK_HRESULT_RET(hr);
				KaiCOMPtr<ID2D1GeometrySink> pGeometrySink;
				hr = pathGeometryClip->Open(&pGeometrySink);
				KaiCHECK_HRESULT_RET(hr);

				hr = clipGeometry->CombineWithGeometry(ld.geometry, D2D1_COMBINE_MODE_INTERSECT,
													   ld.transformMatrix, pGeometrySink);
				KaiCHECK_HRESULT_RET(hr);
				hr = pGeometrySink->Close();
				KaiCHECK_HRESULT_RET(hr);
				pGeometrySink.reset();

				clipGeometry = pathGeometryClip;
				pathGeometryClip.reset();
			}
		}

		// Final clipping geometry is given in device coordinates
		// so we need to transform its bounds to logical coordinates.
		D2D1::Matrix3x2F currTransform;
		GetRenderTarget()->GetTransform(&currTransform);
		currTransform.Invert();

		D2D1_RECT_F bounds;
		// First check if clip region is empty.
		FLOAT clipArea;
		hr = clipGeometry->ComputeArea(currTransform, &clipArea);
		KaiCHECK_HRESULT_RET(hr);
		if ( clipArea <= FLT_MIN )
		{
			bounds.left = bounds.top = bounds.right = bounds.bottom = 0.0F;
		}
		else
		{
			// If it is not empty then get it bounds.
			hr = clipGeometry->GetBounds(currTransform, &bounds);
			KaiCHECK_HRESULT_RET(hr);
		}

		m_clipX1 = bounds.left;
		m_clipY1 = bounds.top;
		m_clipX2 = bounds.right;
		m_clipY2 = bounds.bottom;
		m_isClipBoxValid = true;
	}

	if ( x )
		*x = m_clipX1;
	if ( y )
		*y = m_clipY1;
	if ( w )
		*w = m_clipX2 - m_clipX1;
	if ( h )
		*h = m_clipY2 - m_clipY1;
}

void* KaiD2DContext::GetNativeContext()
{
	return &m_renderTargetHolder;
}

void KaiD2DContext::StrokePath(GraphicsPathData * p)
{
	if (m_composition == KaiCOMPOSITION_DEST)
		return;

	KaiD2DOffsetHelper helper(this);

	EnsureInitialized();
	AdjustRenderTargetSize();

	KaiD2DPathData *pathData = (KaiD2DPathData*)p;
	pathData->Flush();

	if (m_pen)
	{
		//KaiD2DPenData* penData = KaiGetD2DPenData(m_pen);
		m_pen->Bind(this);

		ID2D1Brush* nativeBrush = m_pen->GetBrush();
		GetRenderTarget()->DrawGeometry((ID2D1Geometry*)pathData->GetNativePath(), nativeBrush, m_pen->GetWidth(), m_pen->GetStrokeStyle());
	}
	delete pathData;
}

void KaiD2DContext::FillPath(GraphicsPathData * p, KaiPolygonFillMode fillStyle)
{
	if (m_composition == KaiCOMPOSITION_DEST)
		return;

	EnsureInitialized();
	AdjustRenderTargetSize();

	KaiD2DPathData *pathData = (KaiD2DPathData*)p;
	pathData->SetFillMode(fillStyle == KaiODDEVEN_RULE ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING);
	pathData->Flush();

	if (m_brush)
	{
		//KaiD2DBrushData* brushData = KaiGetD2DBrushData(m_brush);
		m_brush->Bind(this);
		GetRenderTarget()->FillGeometry((ID2D1Geometry*)pathData->GetNativePath(), m_brush->GetBrush());
	}
}

bool KaiD2DContext::SetAntialiasMode(KaiAntialiasMode antialias)
{
	if (m_antialias == antialias)
	{
		return true;
	}

	GetRenderTarget()->SetAntialiasMode(KaiD2DConvertAntialiasMode(antialias));

	m_antialias = antialias;
	return true;
}

bool KaiD2DContext::SetInterpolationQuality(KaiInterpolationQuality interpolation)
{
	// Since different versions of Direct2D have different enumerations for
	// interpolation quality, we deffer the conversion to the method which
	// does the actual drawing.

	m_interpolation = interpolation;
	return true;
}

bool KaiD2DContext::SetCompositionMode(KaiCompositionMode compositionMode)
{
	if (m_composition == compositionMode)
		return true;

	if (m_renderTargetHolder->SetCompositionMode(compositionMode))
	{
		// the composition mode is supported by the render target
		m_composition = compositionMode;
		return true;
	}

	return false;
}

void KaiD2DContext::BeginLayer(KaiDouble opacity)
{
	EnsureInitialized();

	KaiCOMPtr<ID2D1Layer> layer;
	HRESULT hr = GetRenderTarget()->CreateLayer(&layer);
	KaiCHECK_HRESULT_RET(hr);

	LayerData ld;
	ld.type = OTHER_LAYER;
	ld.params = D2D1::LayerParameters(D2D1::InfiniteRect(),
						nullptr,
						D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
						D2D1::IdentityMatrix(), opacity);
	ld.layer = layer;
	// We need to store CTM to be able to re-apply
	// the layer at the original position later on.
	GetRenderTarget()->GetTransform(&ld.transformMatrix);

	GetRenderTarget()->PushLayer(ld.params, layer);

	// Store layer parameters.
	m_layers.push(ld);
}

void KaiD2DContext::EndLayer()
{
	KaiStack<LayerData> layersToRestore;
	// Temporarily remove all clipping layers
	// above the first standard layer
	// and next permanently remove this layer.
	while ( !m_layers.empty() )
	{
		LayerData ld = m_layers.top();
		m_layers.pop();

		if ( ld.type == CLIP_LAYER )
		{
			GetRenderTarget()->PopLayer();
			layersToRestore.push(ld);
			continue;
		}

		// We found a non-clipping layer to remove.
		GetRenderTarget()->PopLayer();
		ld.layer.reset();
		break;
	}

	if ( m_layers.empty() )
	{
		HRESULT hr = GetRenderTarget()->Flush();
		KaiCHECK_HRESULT_RET(hr);
	}

	// Re-apply all stored clipping layers.
	// First, save current transformation matrix.
	D2D1_MATRIX_3X2_F currTransform;
	GetRenderTarget()->GetTransform(&currTransform);
	while ( !layersToRestore.empty() )
	{
		LayerData ld = layersToRestore.top();
		layersToRestore.pop();

		if ( ld.type == CLIP_LAYER )
		{
			// Restore layer at original position.
			GetRenderTarget()->SetTransform(&ld.transformMatrix);
			GetRenderTarget()->PushLayer(ld.params, ld.layer);
		}
		else
		{
			KaiFAIL_MSG( KaiS("Invalid layer type") );
		}
		// Store layer parameters.
		m_layers.push(ld);
	}
	// Restore current transformation matrix.
	GetRenderTarget()->SetTransform(&currTransform);
}

void KaiD2DContext::Translate(KaiDouble dx, KaiDouble dy)
{
	GraphicsMatrixData * translationMatrix = CreateMatrix();
	translationMatrix->Translate(dx, dy);
	ConcatTransform(translationMatrix);
	delete translationMatrix;
}

void KaiD2DContext::Scale(KaiDouble xScale, KaiDouble yScale)
{
	GraphicsMatrixData * scaleMatrix = CreateMatrix();
	scaleMatrix->Scale(xScale, yScale);
	ConcatTransform(scaleMatrix);
	delete scaleMatrix;
}

void KaiD2DContext::Rotate(KaiDouble angle)
{
	GraphicsMatrixData *rotationMatrix = CreateMatrix();
	rotationMatrix->Rotate(angle);
	ConcatTransform(rotationMatrix);
	delete rotationMatrix;
}

void KaiD2DContext::ConcatTransform(GraphicsMatrixData* matrix)
{
	GraphicsMatrixData *getTransform = GetTransform();
	D2D1::Matrix3x2F localMatrix = ((KaiD2DMatrixData*)getTransform)->GetMatrix3x2F();
	D2D1::Matrix3x2F concatMatrix = ((KaiD2DMatrixData*)matrix)->GetMatrix3x2F();

	D2D1::Matrix3x2F resultMatrix;
	resultMatrix.SetProduct(concatMatrix, localMatrix);

	KaiD2DMatrixData *resultTransform = new KaiD2DMatrixData(m_renderer, resultMatrix);

	delete getTransform;

	SetTransform(resultTransform);
	delete resultTransform;
}

void KaiD2DContext::SetTransform(GraphicsMatrixData* matrix)
{
	EnsureInitialized();

	D2D1::Matrix3x2F m;
	m.SetProduct(((KaiD2DMatrixData*)matrix)->GetMatrix3x2F(), m_initTransform);
	GetRenderTarget()->SetTransform(&m);

	m_isClipBoxValid = false;
}

GraphicsMatrixData *KaiD2DContext::GetTransform() const
{
	D2D1::Matrix3x2F transformMatrix;

	if (GetRenderTarget() != nullptr)
	{
		GetRenderTarget()->GetTransform(&transformMatrix);

		if ( m_initTransform.IsInvertible() )
		{
			D2D1::Matrix3x2F invMatrix = m_initTransform;
			invMatrix.Invert();

			D2D1::Matrix3x2F m;
			m.SetProduct(transformMatrix, invMatrix);
			transformMatrix = m;
		}
	}
	else
	{
		transformMatrix = D2D1::Matrix3x2F::Identity();
	}

	KaiD2DMatrixData* matrixData = new KaiD2DMatrixData(m_renderer, transformMatrix);


	return matrixData;
}

void KaiD2DContext::DrawBitmap(GraphicsBitmapData* bmp, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	if (m_composition == KaiCOMPOSITION_DEST)
		return;

	KaiD2DBitmapData* bitmapData = (KaiD2DBitmapData*)bmp;
	bitmapData->Bind(this);

	m_renderTargetHolder->DrawBitmap(
		bitmapData->GetD2DBitmap(),
		D2D1::Point2F(x, y),
		D2D1::RectF(0, 0, w, h),
		GetInterpolationQuality(),
		GetCompositionMode());
}

void KaiD2DContext::DrawBitmap(const KaiBitmap& bmp, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	GraphicsBitmapData * graphicsBitmap = CreateBitmap(bmp);
	DrawBitmap(graphicsBitmap, x, y, w, h);
	delete graphicsBitmap;
}

void KaiD2DContext::DrawIcon(const KaiIcon& icon, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	DrawBitmap(KaiBitmap(icon), x, y, w, h);
}

void KaiD2DContext::PushState()
{
	EnsureInitialized();

	StateData state;
	m_direct2dFactory->CreateDrawingStateBlock(&state.drawingState);
	GetRenderTarget()->SaveDrawingState(state.drawingState);
	state.layers = m_layers;

	m_stateStack.push(state);
}

void KaiD2DContext::PopState()
{
	KaiCHECK_RET(!m_stateStack.empty(), KaiS("No state to pop"));

	// Remove all layers from the stack of layers.
	while ( !m_layers.empty() )
	{
		LayerData ld = m_layers.top();
		m_layers.pop();

		GetRenderTarget()->PopLayer();
		ld.layer.reset();
		ld.geometry.reset();
	}

	// Retrieve state data.
	StateData state;
	state = m_stateStack.top();
	m_stateStack.pop();

	// Restore all saved layers.
	KaiStack<LayerData> layersToRestore;
	// We have to restore layers on the stack from "bottom" to "top",
	// so we have to create a "reverted" stack.
	while ( !state.layers.empty() )
	{
		LayerData ld = state.layers.top();
		state.layers.pop();

		layersToRestore.push(ld);
	}
	// And next set layers from the top of "reverted" stack.
	while ( !layersToRestore.empty() )
	{
		LayerData ld = layersToRestore.top();
		layersToRestore.pop();

		// Restore layer at original position.
		GetRenderTarget()->SetTransform(&ld.transformMatrix);
		GetRenderTarget()->PushLayer(ld.params, ld.layer);

		// Store layer parameters.
		m_layers.push(ld);
	}

	// Restore drawing state.
	GetRenderTarget()->RestoreDrawingState(state.drawingState);

	m_isClipBoxValid = false;
}

void KaiD2DContext::GetTextExtent(
	const KaiString& str,
	KaiDouble* width,
	KaiDouble* height,
	KaiDouble* descent,
	KaiDouble* externalLeading) const
{
	//KaiCHECK_RET(m_font,
		//KaiS("KaiD2DContext::GetTextExtent - no valid font set"));
	if (!m_font)
		return;

	KaiD2DMeasuringContext::GetTextExtent(
		m_font, str, width, height, descent, externalLeading);
}

void KaiD2DContext::GetPartialTextExtents(const KaiString& text, KaiArrayDouble& widths) const
{
	KaiCHECK_RET(m_font,
		KaiS("KaiD2DContext::GetPartialTextExtents - no valid font set"));

	KaiD2DMeasuringContext::GetPartialTextExtents(
		m_font, text, widths);
}

bool KaiD2DContext::ShouldOffset() const
{
	if (!m_enableOffset)
	{
		return false;
	}

	int penWidth = 0;
	if (m_pen)
	{
		penWidth = m_pen->GetWidth();
		penWidth = KaiMax(penWidth, 1);
	}

	return (penWidth % 2) == 1;
}

void KaiD2DContext::DoDrawText(const KaiString& str, KaiDouble x, KaiDouble y)
{
	//KaiCHECK_RET(m_font,
		//KaiS("KaiD2DContext::DrawText - no valid font set"));
	if (!m_font)
		return;

	if (m_composition == KaiCOMPOSITION_DEST)
		return;

	//KaiD2DFontData* fontData = m_font;
	//fontData->GetBrushData().Bind(this);
	m_font->GetBrushData().Bind(this);
	KaiCOMPtr<IDWriteTextLayout> textLayout = m_font->CreateTextLayout(str);

	// Render the text
	GetRenderTarget()->DrawTextLayout(
		D2D1::Point2F(x, y),
		textLayout,
		m_font->GetBrushData().GetBrush());
}

void KaiD2DContext::EnsureInitialized()
{
	if (!m_renderTargetHolder->IsResourceAcquired())
	{
		//loop it avoid to crash when target is null cause of context problem
		while (!m_cachedRenderTarget) {
			m_cachedRenderTarget = m_renderTargetHolder->GetD2DResource();
			Sleep(10);
		}
		GetRenderTarget()->GetTransform(&m_initTransform);
		GetRenderTarget()->BeginDraw();
	}
	else
	{
		m_cachedRenderTarget = m_renderTargetHolder->GetD2DResource();
	}
}

void KaiD2DContext::SetPen(const KaiD2DPenData& pen)
{
	m_pen = new KaiD2DPenData(pen);

	if (m_pen)
	{
		EnsureInitialized();

		//KaiD2DPenData* penData = KaiGetD2DPenData(pen);
		m_pen->Bind(this);
	}
}

void KaiD2DContext::AdjustRenderTargetSize()
{
	m_renderTargetHolder->Resize();

	// Currently GetSize() can only be called when using MSVC because gcc
	// doesn't handle returning aggregates by value as done by D2D libraries,
	// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64384. Not updating the
	// size is not great, but it's better than crashing.
#ifdef __VISUALC__
	D2D1_SIZE_F renderTargetSize = m_renderTargetHolder->GetD2DResource()->GetSize();
	m_width = renderTargetSize.width;
	m_height =  renderTargetSize.height;
#endif // __VISUALC__
}

void KaiD2DContext::ReleaseDeviceDependentResources()
{
	ReleaseResources();
}

void KaiD2DContext::DrawRectangle(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	if (m_composition == KaiCOMPOSITION_DEST)
		return;

	KaiD2DOffsetHelper helper(this);

	EnsureInitialized();
	AdjustRenderTargetSize();

	D2D1_RECT_F rect = { (FLOAT)x, (FLOAT)y, (FLOAT)(x + w), (FLOAT)(y + h) };


	if (m_brush)
	{
		//KaiD2DBrushData* brushData = m_brush;
		m_brush->Bind(this);
		GetRenderTarget()->FillRectangle(rect, m_brush->GetBrush());
	}

	if (m_pen)
	{
		//KaiD2DPenData* penData = m_pen;
		m_pen->Bind(this);
		GetRenderTarget()->DrawRectangle(rect, m_pen->GetBrush(), m_pen->GetWidth(), m_pen->GetStrokeStyle());
	}
}

void KaiD2DContext::DrawRoundedRectangle(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h, KaiDouble radius)
{
	if (m_composition == KaiCOMPOSITION_DEST)
		return;

	KaiD2DOffsetHelper helper(this);

	EnsureInitialized();
	AdjustRenderTargetSize();

	D2D1_RECT_F rect = { (FLOAT)x, (FLOAT)y, (FLOAT)(x + w), (FLOAT)(y + h) };

	D2D1_ROUNDED_RECT roundedRect = { rect, (FLOAT)radius, (FLOAT)radius };

	if (m_brush)
	{
		//KaiD2DBrushData* brushData = m_brush;
		m_brush->Bind(this);
		GetRenderTarget()->FillRoundedRectangle(roundedRect, m_brush->GetBrush());
	}

	if (m_pen)
	{
		//KaiD2DPenData* penData = m_pen;
		m_pen->Bind(this);
		GetRenderTarget()->DrawRoundedRectangle(roundedRect, m_pen->GetBrush(), m_pen->GetWidth(), m_pen->GetStrokeStyle());
	}
}

void KaiD2DContext::DrawEllipse(KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	if (m_composition == KaiCOMPOSITION_DEST)
		return;

	KaiD2DOffsetHelper helper(this);

	EnsureInitialized();
	AdjustRenderTargetSize();

	D2D1_ELLIPSE ellipse = {
		{ (FLOAT)(x + w / 2), (FLOAT)(y + h / 2) }, // center point
		(FLOAT)(w / 2),                      // radius x
		(FLOAT)(h / 2)                       // radius y
	};

	if (m_brush)
	{
		//KaiD2DBrushData* brushData = m_brush;
		m_brush->Bind(this);
		GetRenderTarget()->FillEllipse(ellipse, m_brush->GetBrush());
	}

	if (m_pen)
	{
		//KaiD2DPenData* penData = m_pen;
		m_pen->Bind(this);
		GetRenderTarget()->DrawEllipse(ellipse, m_pen->GetBrush(), m_pen->GetWidth(), m_pen->GetStrokeStyle());
	}
}

void KaiD2DContext::Flush()
{
	KaiStack<LayerData> layersToRestore;
	// Temporarily remove all layers from the stack of layers.
	while ( !m_layers.empty() )
	{
		LayerData ld = m_layers.top();
		m_layers.pop();

		GetRenderTarget()->PopLayer();

		// Save layer data.
		layersToRestore.push(ld);
	}

	HRESULT hr = m_renderTargetHolder->Flush();

	if ( hr == (HRESULT)D2DERR_RECREATE_TARGET )
	{
		ReleaseDeviceDependentResources();
	}
	else
	{
		KaiCHECK_HRESULT_RET(hr);
	}

	// Re-apply all layers.
	// First, save current transformation matrix.
	D2D1_MATRIX_3X2_F currTransform;
	GetRenderTarget()->GetTransform(&currTransform);
	while ( !layersToRestore.empty() )
	{
		LayerData ld = layersToRestore.top();
		layersToRestore.pop();

		// Restore layer at original position.
		GetRenderTarget()->SetTransform(&ld.transformMatrix);
		GetRenderTarget()->PushLayer(ld.params, ld.layer);

		// Store layer parameters.
		m_layers.push(ld);
	}
	// Restore current transformation matrix.
	GetRenderTarget()->SetTransform(&currTransform);
}

void KaiD2DContext::GetDPI(KaiDouble* dpiX, KaiDouble* dpiY) const
{
  //  if ( GetWindow() )
  //  {
		//
  //      const KaiSize dpi = GetWindow()->GetDPI();

  //      if ( dpiX )
  //          *dpiX = dpi.x;
  //      if ( dpiY )
  //          *dpiY = dpi.y;
  //  }
  //  else
  //  {
		FLOAT x, y;
		GetRenderTarget()->GetDpi(&x, &y);

		if ( dpiX )
			*dpiX = x;
		if ( dpiY )
			*dpiY = y;
   // }
}



//-----------------------------------------------------------------------------
// KaiD2DRenderer implementation
//-----------------------------------------------------------------------------

class KaiD2DRenderer : public GraphicsRenderer
{
public:
	KaiD2DRenderer();

	virtual ~KaiD2DRenderer();

	static KaiD2DRenderer* GetDirect2DRenderer();

	GraphicsContext * CreateContext(const KaiWindowDC& dc);

	GraphicsContext * CreateContext(const KaiMemoryDC& dc);

	GraphicsContext * CreateContextFromNativeContext(void* context);

	GraphicsContext * CreateContextFromNativeWindow(void* window);

	GraphicsContext * CreateContextFromNativeHDC(WXHDC dc);

	GraphicsContext * CreateContext(KaiWindow* window);

#if KaiUSE_IMAGE
	GraphicsContext * CreateContextFromImage(KaiImage& image);
#endif // KaiUSE_IMAGE

	GraphicsContext * CreateMeasuringContext();

	KaiD2DPathData * CreatePath();

	GraphicsMatrixData *CreateMatrix(
		KaiDouble a = 1.0, KaiDouble b = 0.0, KaiDouble c = 0.0, KaiDouble d = 1.0,
		KaiDouble tx = 0.0, KaiDouble ty = 0.0);

	KaiD2DPenData * CreatePen(const KaiGraphicsPenInfo& info);

	KaiD2DBrushData * CreateBrush(const KaiBrush& brush);

	KaiD2DBrushData * CreateLinearGradientBrush(
		KaiDouble x1, KaiDouble y1,
		KaiDouble x2, KaiDouble y2,
		const KaiGraphicsGradientStops& stops,
		KaiD2DMatrixData * matrix = nullptr);

	KaiD2DBrushData * CreateRadialGradientBrush(
		KaiDouble startX, KaiDouble startY,
		KaiDouble endX, KaiDouble endY,
		KaiDouble radius,
		const KaiGraphicsGradientStops& stops,
		KaiD2DMatrixData * matrix = nullptr);

	// create a native bitmap representation
	GraphicsBitmapData * CreateBitmap(const KaiBitmap& bitmap);

	//#if KaiUSE_IMAGE
	//	GraphicsBitmapData * CreateBitmapFromImage(const KaiImage& image);
	//	KaiImage CreateImageFromBitmap(const KaiGraphicsBitmap& bmp);
	//#endif

	KaiD2DFontData * CreateFont(const KaiFont& font, const KaiColour& col);

	/*KaiD2DFontData * CreateFont(
	double sizeInPixels, const KaiString& facename,
	int flags = KaiFONTFLAG_DEFAULT,
	const KaiColour& col = *KaiBLACK);*/

	KaiD2DFontData * CreateFontAtDPI(const KaiFont& font,
		const KaiRealPoint& dpi,
		const KaiColour& col);

	// create a graphics bitmap from a native bitmap
	GraphicsBitmapData * CreateBitmapFromNativeBitmap(void* bitmap);

	// create a sub-image from a native image representation
	GraphicsBitmapData * CreateSubBitmap(const GraphicsBitmapData& bitmap, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h);

	KaiString GetName() const;
	void GetVersion(int* major, int* minor, int* micro) const;

	ID2D1Factory* GetD2DFactory();

private:
	KaiCOMPtr<ID2D1Factory> m_direct2dFactory;

private:
	//KaiDECLARE_DYNAMIC_CLASS_NO_COPY(KaiD2DRenderer);
};

//KaiIMPLEMENT_DYNAMIC_CLASS(KaiD2DRenderer,KaiGraphicsRenderer);

void KaiNullContext::SetFont(const KaiFont& font, const KaiColour& col){
	KaiDELETE(m_font);
	m_font = renderer->CreateFont(font, col);
}

GraphicsPathData * KaiD2DContext::CreatePath(){ return m_renderer->CreatePath(); };

void KaiD2DContext::SetPen(const KaiPen& pen, double width){
	KaiGraphicsPenInfo info(pen.GetColour(), width, pen.GetStyle());
	KaiDELETE(m_pen);
	m_pen = m_renderer->CreatePen(info);
}

void KaiD2DContext::SetBrush(const KaiBrush& brush){
	KaiDELETE(m_brush);
	m_brush = m_renderer->CreateBrush(brush);
}

void KaiD2DContext::SetFont(const KaiFont& font, const KaiColour& col){
	KaiDELETE(m_font);
	m_font = m_renderer->CreateFont(font, col);
}

GraphicsBitmapData *KaiD2DContext::CreateBitmap(const KaiBitmap& bmp) const
{
	return m_renderer->CreateBitmap(bmp);
}

GraphicsMatrixData *KaiD2DContext::CreateMatrix(KaiDouble a, KaiDouble b, KaiDouble c, KaiDouble d,
	KaiDouble tx, KaiDouble ty) const {
	return m_renderer->CreateMatrix(a, b, c, d, tx, ty);
}

static KaiD2DRenderer *gs_D2DRenderer = nullptr;

KaiD2DRenderer* KaiD2DRenderer::GetDirect2DRenderer()
{
	if (!KaiDirect2D::Initialize())
		return nullptr;

	if (!gs_D2DRenderer)
	{
		gs_D2DRenderer = new KaiD2DRenderer();
	}

	return gs_D2DRenderer;
}

KaiD2DRenderer::KaiD2DRenderer()
	: m_direct2dFactory(KaiD2D1Factory())
{
	if ( m_direct2dFactory.get() == nullptr )
	{
		KaiFAIL_MSG("Could not create Direct2D Factory.");
	}
}

KaiD2DRenderer::~KaiD2DRenderer()
{
	m_direct2dFactory.reset();
}

GraphicsContext* KaiD2DRenderer::CreateContext(const KaiWindowDC& dc)
{
	return new KaiD2DContext(this, m_direct2dFactory, dc.GetHDC(), &dc);
}

GraphicsContext* KaiD2DRenderer::CreateContext(const KaiMemoryDC& dc)
{
	KaiBitmap bmp = dc.GetSelectedBitmap();
	KaiASSERT_MSG( bmp.IsOk(), KaiS("Should select a bitmap before creating KaiGraphicsContext") );

	return new KaiD2DContext(this, m_direct2dFactory, dc.GetHDC(), &dc,
							bmp.HasAlpha() ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE);
}

GraphicsContext* KaiD2DRenderer::CreateContextFromNativeContext(void* nativeContext)
{
	return new KaiD2DContext(this, m_direct2dFactory, nativeContext);
}

GraphicsContext* KaiD2DRenderer::CreateContextFromNativeWindow(void* window)
{
	return new KaiD2DContext(this, m_direct2dFactory, (HWND)window);
}

GraphicsContext* KaiD2DRenderer::CreateContextFromNativeHDC(WXHDC dc)
{
	return new KaiD2DContext(this, m_direct2dFactory, (HDC)dc);
}

GraphicsContext* KaiD2DRenderer::CreateContext(KaiWindow* window)
{
	return new KaiD2DContext(this, m_direct2dFactory, (HWND)window->GetHWND(), window);
}

#if KaiUSE_IMAGE
GraphicsContext* KaiD2DRenderer::CreateContextFromImage(KaiImage& image)
{
	return new KaiD2DContext(this, m_direct2dFactory, image);
}
#endif // KaiUSE_IMAGE

GraphicsContext* KaiD2DRenderer::CreateMeasuringContext()
{
	return new KaiD2DMeasuringContext(this);
}

KaiD2DPathData * KaiD2DRenderer::CreatePath()
{
	return new KaiD2DPathData(this, m_direct2dFactory);
}

GraphicsMatrixData *KaiD2DRenderer::CreateMatrix(
	KaiDouble a, KaiDouble b, KaiDouble c, KaiDouble d,
	KaiDouble tx, KaiDouble ty)
{
	KaiD2DMatrixData* matrixData = new KaiD2DMatrixData(this);
	matrixData->Set(a, b, c, d, tx, ty);

  /*  KaiGraphicsMatrix matrix;
	matrix.SetRefData(matrixData);*/

	return matrixData;
}

KaiD2DPenData *KaiD2DRenderer::CreatePen(const KaiGraphicsPenInfo& info)
{
	if ( info.GetStyle() == KaiPENSTYLE_TRANSPARENT )
	{
		return nullptr;
	}
	else
	{
		
		KaiD2DPenData* penData = new KaiD2DPenData(this, m_direct2dFactory, info);
		return penData;
	}
}

KaiD2DBrushData *KaiD2DRenderer::CreateBrush(const KaiBrush& brush)
{
	if ( !brush.IsOk() || brush.GetStyle() == KaiBRUSHSTYLE_TRANSPARENT )
	{
		return nullptr;
	}
	else
	{
		
		return new KaiD2DBrushData(this, brush);
	}
}

KaiD2DBrushData * KaiD2DRenderer::CreateLinearGradientBrush(
	KaiDouble x1, KaiDouble y1,
	KaiDouble x2, KaiDouble y2,
	const KaiGraphicsGradientStops& stops,
	KaiD2DMatrixData * matrix)
{
	KaiD2DBrushData* brushData = new KaiD2DBrushData(this);
	brushData->CreateLinearGradientBrush(x1, y1, x2, y2, stops, matrix);


	return brushData;
}

KaiD2DBrushData * KaiD2DRenderer::CreateRadialGradientBrush(
	KaiDouble startX, KaiDouble startY,
	KaiDouble endX, KaiDouble endY,
	KaiDouble radius,
	const KaiGraphicsGradientStops& stops,
	KaiD2DMatrixData * matrix)
{
	KaiD2DBrushData* brushData = new KaiD2DBrushData(this);
	brushData->CreateRadialGradientBrush(startX, startY, endX, endY, radius, stops, matrix);

	return brushData;
}

// create a native bitmap representation
GraphicsBitmapData * KaiD2DRenderer::CreateBitmap(const KaiBitmap& bitmap)
{
	KaiD2DBitmapData* bitmapData = new KaiD2DBitmapData(this, bitmap);

	return bitmapData;
}

// create a graphics bitmap from a native bitmap
GraphicsBitmapData * KaiD2DRenderer::CreateBitmapFromNativeBitmap(void* bitmap)
{
	KaiD2DBitmapData* bitmapData = new KaiD2DBitmapData(this, bitmap);

	return bitmapData;
}

//#if KaiUSE_IMAGE
//GraphicsBitmapData * KaiD2DRenderer::CreateBitmapFromImage(const KaiImage& image)
//{
//    return CreateBitmap(KaiBitmap(image));
//}
//
//KaiImage KaiD2DRenderer::CreateImageFromBitmap(const KaiGraphicsBitmap& bmp)
//{
//    return static_cast<KaiD2DBitmapData::NativeType*>(bmp.GetNativeBitmap())
//        ->GetSourceBitmap().ConvertToImage();
//}
//#endif

KaiD2DFontData * KaiD2DRenderer::CreateFont(const KaiFont& font, const KaiColour& col)
{
	return CreateFontAtDPI(font, KaiRealPoint(), col);
}

//KaiD2DFontData * KaiD2DRenderer::CreateFont(
//    double sizeInPixels, const KaiString& facename,
//    int flags,
//    const KaiColour& col)
//{
//    // Use the same DPI as KaiFont will use in SetPixelSize, so these cancel
//    // each other out and we are left with the actual pixel size.
//    ScreenHDC hdc;
//    KaiRealPoint dpi(::GetDeviceCaps(hdc, LOGPIXELSX),
//                    ::GetDeviceCaps(hdc, LOGPIXELSY));
//
//    return CreateFontAtDPI(
//        KaiFont(KaiSize(sizeInPixels, sizeInPixels)).AllFlags(flags).FaceName(facename),
//        dpi, col);
//}

KaiD2DFontData * KaiD2DRenderer::CreateFontAtDPI(const KaiFont& font,
											  const KaiRealPoint& dpi,
											  const KaiColour& col)
{
	KaiD2DFontData* fontData = new KaiD2DFontData(this, font, dpi, col);
	if ( !fontData->GetFont() )
	{
		// Apparently a non-TrueType font is given and hence
		// corresponding DirectWrite font couldn't be created.
		delete fontData;
		return nullptr;
	}

	return fontData;
}

// create a sub-image from a native image representation
GraphicsBitmapData* KaiD2DRenderer::CreateSubBitmap(const GraphicsBitmapData& bitmap, KaiDouble x, KaiDouble y, KaiDouble w, KaiDouble h)
{
	typedef KaiD2DBitmapData::NativeType* NativeBitmap;
	KaiBitmap sourceBitmap = static_cast<NativeBitmap>(bitmap.GetNativeBitmap())->GetSourceBitmap();
	return CreateBitmap(sourceBitmap.GetSubBitmap(KaiRect(x, y, w, h)));
}

KaiString KaiD2DRenderer::GetName() const
{
	return "direct2d";
}

void KaiD2DRenderer::GetVersion(int* major, int* minor, int* micro) const
{
	if (KaiDirect2D::HasDirect2DSupport())
	{
		if (major)
			*major = 1;

		if (minor)
		{
			switch(KaiDirect2D::GetDirect2DVersion())
			{
			case KaiDirect2D::KaiD2D_VERSION_1_0:
				*minor = 0;
				break;
			case KaiDirect2D::KaiD2D_VERSION_1_1:
				*minor = 1;
				break;
			case KaiDirect2D::KaiD2D_VERSION_NONE:
				// This is not supposed to happen, but we handle this value in
				// the switch to ensure that we'll get warnings if any new
				// values, not handled here, are added to the enum later.
				*minor = -1;
				break;
			}
		}

		if (micro)
			*micro = 0;
	}
}

ID2D1Factory* KaiD2DRenderer::GetD2DFactory()
{
	return m_direct2dFactory;
}

ID2D1Factory* KaiGetD2DFactory(KaiD2DRenderer* renderer)
{
	return renderer->GetD2DFactory();
}

// ----------------------------------------------------------------------------
// Module ensuring all global/singleton objects are destroyed on shutdown.
// ----------------------------------------------------------------------------

class KaiDirect2DModule : public KaiModule
{
public:
	KaiDirect2DModule()
	{
	}

	virtual bool OnInit() override
	{
		HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		// RPC_E_CHANGED_MODE is not considered as an error
		// - see remarks for KaiOleInitialize().
		return SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;
	}

	virtual void OnExit() override
	{
		if ( gs_WICImagingFactory )
		{
			gs_WICImagingFactory->Release();
			gs_WICImagingFactory = nullptr;
		}

		if ( gs_IDWriteFactory )
		{
			gs_IDWriteFactory->Release();
			gs_IDWriteFactory = nullptr;
		}

		if ( gs_D2DRenderer )
		{
			delete gs_D2DRenderer;
			gs_D2DRenderer = nullptr;
		}

		if ( gs_ID2D1Factory )
		{
			gs_ID2D1Factory->Release();
			gs_ID2D1Factory = nullptr;
		}

		::CoUninitialize();
	}

private:
	KaiDECLARE_DYNAMIC_CLASS(KaiDirect2DModule);
};

KaiIMPLEMENT_DYNAMIC_CLASS(KaiDirect2DModule, KaiModule);

GraphicsContext * Create(const KaiWindowDC& dc)
{
	KaiD2DRenderer * renderer = KaiD2DRenderer::GetDirect2DRenderer();
	return renderer->CreateContext(dc);
}

GraphicsContext * Create(const KaiMemoryDC& dc)
{
	KaiD2DRenderer * renderer = KaiD2DRenderer::GetDirect2DRenderer();
	return renderer->CreateContext(dc);
}

GraphicsContext * Create(KaiWindow* window)
{
	KaiD2DRenderer * renderer = KaiD2DRenderer::GetDirect2DRenderer();
	return renderer->CreateContext(window);
}

void GraphicsContext::StrokeLine(KaiDouble x1, KaiDouble y1, KaiDouble x2, KaiDouble y2)
{
	GraphicsPathData * path = CreatePath();
	path->MoveToPoint(x1, y1);
	path->AddLineToPoint(x2, y2);
	StrokePath(path);
}

void GraphicsContext::DrawTextU(const KaiString& str, KaiDouble x, KaiDouble y)
{
	DoDrawText(str, x, y);
}

GraphicsRenderer* GraphicsRenderer::GetDirect2DRenderer()
{
	return KaiD2DRenderer::GetDirect2DRenderer();
}
#endif // KaiUSE_GRAPHICS_DIRECT2D
