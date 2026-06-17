#pragma once
#ifndef _WIN32
#include "platform.h"
#ifndef KAINOTE_GUID_STUB_DEFINED
#define KAINOTE_GUID_STUB_DEFINED
struct GUID { unsigned long Data1{}; unsigned short Data2{}; unsigned short Data3{}; unsigned char Data4[8]{}; };
#endif
inline constexpr GUID CLSID_TaskbarList{};
constexpr int CLSCTX_INPROC_SERVER = 1;
constexpr int TBPF_NOPROGRESS = 0;
constexpr int TBPF_NORMAL = 2;
#define __uuidof(x) GUID{}
struct ITaskbarList3 {
    HRESULT SetProgressState(HWND, int) { return S_OK; }
    HRESULT SetProgressValue(HWND, ULONGLONG, ULONGLONG) { return S_OK; }
};
inline HRESULT CoCreateInstance(GUID, void*, int, GUID, void** out) { if (out) *out = new ITaskbarList3(); return S_OK; }
#endif
