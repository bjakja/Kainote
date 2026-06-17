#pragma once
#ifndef _WIN32
#include "platform.h"
#include "d3d9.h"
struct D3DXVECTOR2 { float x{}, y{}; D3DXVECTOR2()=default; D3DXVECTOR2(float X,float Y):x(X),y(Y){} D3DXVECTOR2& operator-=(const D3DXVECTOR2& o){x-=o.x;y-=o.y;return *this;} D3DXVECTOR2& operator+=(const D3DXVECTOR2& o){x+=o.x;y+=o.y;return *this;} };
inline bool operator==(const D3DXVECTOR2& a, const D3DXVECTOR2& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const D3DXVECTOR2& a, const D3DXVECTOR2& b) { return !(a == b); }
inline D3DXVECTOR2 operator-(const D3DXVECTOR2& a, const D3DXVECTOR2& b) { return {a.x-b.x, a.y-b.y}; }
inline D3DXVECTOR2 operator+(const D3DXVECTOR2& a, const D3DXVECTOR2& b) { return {a.x+b.x, a.y+b.y}; }
inline D3DXVECTOR2 operator/(const D3DXVECTOR2& a, float v) { return {a.x/v, a.y/v}; }
inline D3DXVECTOR2 operator*(const D3DXVECTOR2& a, float v) { return {a.x*v, a.y*v}; }
inline D3DXVECTOR2 operator*(float v, const D3DXVECTOR2& a) { return {a.x*v, a.y*v}; }
struct D3DXVECTOR3 { float x{}, y{}, z{}; D3DXVECTOR3()=default; D3DXVECTOR3(float X,float Y,float Z=0):x(X),y(Y),z(Z){} };
struct D3DXVECTOR4 { float x{}, y{}, z{}, w{}; D3DXVECTOR4()=default; D3DXVECTOR4(float X,float Y,float Z,float W):x(X),y(Y),z(Z),w(W){} };
struct D3DXMATRIX { float m[4][4]{}; };
inline D3DXMATRIX operator*(const D3DXMATRIX& a, const D3DXMATRIX& b) { (void)b; return a; }
struct ID3DXLine : D3DReleasable { HRESULT Begin(){return S_OK;} HRESULT End(){return S_OK;} HRESULT Draw(const D3DXVECTOR2*, DWORD, D3DCOLOR){return S_OK;} HRESULT SetWidth(float){return S_OK;} HRESULT SetAntialias(BOOL){return S_OK;} };
struct ID3DXFont : D3DReleasable { int DrawTextW(void*, const wchar_t*, int, RECT* rect, UINT, D3DCOLOR){ if (rect && rect->right <= rect->left) rect->right = rect->left + 80; if (rect && rect->bottom <= rect->top) rect->bottom = rect->top + 20; return 0; } };
using LPD3DXLINE = ID3DXLine*;
using LPD3DXFONT = ID3DXFont*;
using D3DXCOLOR = D3DCOLOR;
constexpr UINT DT_LEFT = 0x0000;
constexpr UINT DT_CENTER = 0x0001;
constexpr UINT DT_TOP = 0x0000;
constexpr UINT DT_VCENTER = 0x0004;
constexpr UINT DT_BOTTOM = 0x0008;
constexpr UINT DT_CALCRECT = 0x0400;
inline HRESULT D3DXCreateFontW(LPDIRECT3DDEVICE9, int, int, int, UINT, BOOL, DWORD, DWORD, DWORD, DWORD, const wchar_t*, LPD3DXFONT* out){ if(out)*out=new ID3DXFont(); return S_OK; }
inline HRESULT D3DXCreateLine(LPDIRECT3DDEVICE9, LPD3DXLINE* out){ if(out)*out=new ID3DXLine(); return S_OK; }
inline void D3DXMatrixOrthoOffCenterLH(D3DXMATRIX*, float, float, float, float, float, float) {}
inline void D3DXMatrixIdentity(D3DXMATRIX*) {}
inline float D3DXToRadian(float deg) { return deg * 0.017453292519943295769f; }
inline void D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* m, float, float, float) { D3DXMatrixIdentity(m); }
inline void D3DXMatrixTranslation(D3DXMATRIX* m, float, float, float) { D3DXMatrixIdentity(m); }
inline void D3DXMatrixLookAtLH(D3DXMATRIX* m, const D3DXVECTOR3*, const D3DXVECTOR3*, const D3DXVECTOR3*) { D3DXMatrixIdentity(m); }
inline void D3DXMatrixPerspectiveFovLH(D3DXMATRIX* m, float, float, float, float) { D3DXMatrixIdentity(m); }
#endif
