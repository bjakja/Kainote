#pragma once
#ifndef _WIN32
#include "platform.h"
using D3DCOLOR = std::uint32_t;
using PVOID = void*;
using ULONGLONG = unsigned long long;
using LONGLONG = long long;
using FLOAT = float;
using D3DFORMAT = int;
constexpr UINT D3D_SDK_VERSION = 32;
constexpr D3DFORMAT D3DFMT_UNKNOWN = 0;
constexpr D3DFORMAT D3DFMT_X8R8G8B8 = 21;
constexpr D3DFORMAT D3DFMT_A8R8G8B8 = 21;
constexpr D3DFORMAT D3DFMT_R5G6B5 = 23;
// FourCC 'YUY2' as a little-endian DWORD, matching MAKEFOURCC('Y','U','Y','2')
constexpr D3DFORMAT D3DFMT_YUY2 = ('Y') | ('U' << 8) | ('Y' << 16) | ('2' << 24);
constexpr HRESULT D3DERR_DEVICELOST = -2005530520L;
constexpr HRESULT D3DERR_DEVICENOTRESET = -2005530519L;
constexpr HRESULT D3DERR_DRIVERINTERNALERROR = -2005530585L;
constexpr int D3DPT_TRIANGLESTRIP = 5;
constexpr int D3DPT_LINESTRIP = 3;
constexpr int D3DPT_LINELIST = 2;
constexpr int D3DTEXF_LINEAR = 1;
using D3DTEXTUREFILTERTYPE = int;
constexpr D3DTEXTUREFILTERTYPE D3DTEXF_POINT = 0;
constexpr int D3DPOOL_DEFAULT = 0;
constexpr DWORD D3DLOCK_NOSYSLOCK = 0x800;
constexpr DWORD D3DFVF_XYZ = 0x002;
constexpr DWORD D3DFVF_DIFFUSE = 0x040;
constexpr DWORD D3DFVF_TEX1 = 0x100;
constexpr DWORD D3DCLEAR_TARGET = 0x1;
constexpr int D3DSWAPEFFECT_COPY = 1;
constexpr DWORD D3DPRESENTFLAG_VIDEO = 0x10;
constexpr int D3DMULTISAMPLE_NONE = 0;
constexpr DWORD D3DCREATE_FPU_PRESERVE = 0x2;
constexpr int D3DPRESENT_INTERVAL_ONE = 1;
constexpr UINT D3DADAPTER_DEFAULT = 0;
constexpr int D3DDEVTYPE_HAL = 1;
constexpr DWORD D3DCREATE_HARDWARE_VERTEXPROCESSING = 0x40;
constexpr DWORD D3DCREATE_SOFTWARE_VERTEXPROCESSING = 0x20;
constexpr DWORD D3DCREATE_MULTITHREADED = 0x4;
constexpr int D3DRS_MULTISAMPLEANTIALIAS=0, D3DRS_ANTIALIASEDLINEENABLE=1, D3DRS_CULLMODE=2, D3DRS_ZENABLE=3, D3DRS_LIGHTING=4, D3DRS_DITHERENABLE=5, D3DRS_ALPHABLENDENABLE=6, D3DRS_SRCBLEND=7, D3DRS_DESTBLEND=8;
constexpr int D3DCULL_NONE=0, D3DZB_FALSE=0, D3DBLEND_SRCALPHA=1, D3DBLEND_INVSRCALPHA=2;
constexpr int D3DTSS_COLOROP=0, D3DTSS_COLORARG1=1, D3DTSS_COLORARG2=2, D3DTSS_ALPHAOP=3, D3DTSS_ALPHAARG1=4, D3DTSS_ALPHAARG2=5;
constexpr int D3DTOP_SELECTARG1=0, D3DTOP_MODULATE=1, D3DTA_TEXTURE=0, D3DTA_SPECULAR=1, D3DTA_DIFFUSE=2;
constexpr int D3DTS_PROJECTION=0, D3DTS_WORLD=1, D3DTS_VIEW=2, D3DBACKBUFFER_TYPE_MONO=0;
struct D3DPRESENT_PARAMETERS { BOOL Windowed{}; HWND hDeviceWindow{}; UINT BackBufferWidth{}; UINT BackBufferHeight{}; UINT BackBufferCount{}; int SwapEffect{}; D3DFORMAT BackBufferFormat{}; DWORD Flags{}; UINT PresentationInterval{}; BOOL EnableAutoDepthStencil{}; int MultiSampleType{}; };
using D3DPRIMITIVETYPE = int;
constexpr D3DPRIMITIVETYPE D3DPT_TRIANGLELIST = 4;
constexpr D3DPRIMITIVETYPE D3DPT_TRIANGLEFAN = 6;
struct D3DLOCKED_RECT { int Pitch{}; void* pBits{}; };
struct D3DReleasable { ULONG Release(){ delete this; return 0; } };
struct IDirect3DSurface9 : D3DReleasable { virtual ~IDirect3DSurface9() = default; HRESULT LockRect(D3DLOCKED_RECT* r, const RECT*, DWORD){ static unsigned char dummy[1048576]{}; if (r){ r->Pitch=1024; r->pBits=dummy;} return S_OK;} HRESULT UnlockRect(){return S_OK;} };
struct IDirect3DDevice9 : D3DReleasable { virtual ~IDirect3DDevice9() = default; HRESULT EndScene(){return S_OK;} HRESULT BeginScene(){return S_OK;} HRESULT Present(const RECT*, const RECT*, HWND, const void*){return S_OK;} HRESULT DrawPrimitiveUP(int, UINT, const void*, UINT){return S_OK;} HRESULT StretchRect(IDirect3DSurface9*, const RECT*, IDirect3DSurface9*, const RECT*, int){return S_OK;} HRESULT CreateOffscreenPlainSurface(UINT, UINT, D3DFORMAT, int, IDirect3DSurface9** out, void*){ if(out)*out=new IDirect3DSurface9(); return S_OK;} HRESULT SetFVF(DWORD){return S_OK;} HRESULT TestCooperativeLevel(){return S_OK;} HRESULT Clear(DWORD, const void*, DWORD, D3DCOLOR, float, DWORD){return S_OK;} HRESULT Reset(D3DPRESENT_PARAMETERS*){return S_OK;} HRESULT SetRenderState(int, DWORD){return S_OK;} HRESULT SetTextureStageState(DWORD, int, DWORD){return S_OK;} HRESULT SetTransform(int, const void*){return S_OK;} HRESULT GetBackBuffer(UINT, UINT, int, IDirect3DSurface9** out){ if(out)*out=new IDirect3DSurface9(); return S_OK;} };
struct IDirect3DTexture9 : D3DReleasable { virtual ~IDirect3DTexture9() = default; };
struct IDirect3DVertexBuffer9 : D3DReleasable { virtual ~IDirect3DVertexBuffer9() = default; };
struct IDirect3D9 : D3DReleasable { virtual ~IDirect3D9() = default; HRESULT CreateDevice(UINT, int, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9** out){ if(out)*out=new IDirect3DDevice9(); return S_OK;} };
using LPDIRECT3D9 = IDirect3D9*;
using LPDIRECT3DDEVICE9 = IDirect3DDevice9*;
using LPDIRECT3DSURFACE9 = IDirect3DSurface9*;
using LPDIRECT3DTEXTURE9 = IDirect3DTexture9*;
using LPDIRECT3DVERTEXBUFFER9 = IDirect3DVertexBuffer9*;
inline LPDIRECT3D9 Direct3DCreate9(UINT){ return new IDirect3D9(); }
#define ZeroMemory(ptr, size) std::memset((ptr), 0, (size))
#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_XRGB(r,g,b) D3DCOLOR_ARGB(0xff,r,g,b)
#endif
