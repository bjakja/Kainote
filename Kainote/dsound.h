#pragma once
#ifndef _WIN32
#include "platform.h"
#include <cstring>
#include <type_traits>

using LPVOID = void*;

#ifndef KAINOTE_GUID_STUB_DEFINED
#define KAINOTE_GUID_STUB_DEFINED
struct GUID { unsigned long Data1{}; unsigned short Data2{}; unsigned short Data3{}; unsigned char Data4[8]{}; };
#endif
inline constexpr GUID GUID_NULL{};
inline constexpr GUID IID_IDirectSoundBuffer8{};
inline constexpr GUID DSDEVID_DefaultPlayback{};

constexpr HRESULT DS_OK = S_OK;
constexpr HRESULT DSERR_BUFFERLOST = static_cast<HRESULT>(0x88780096u);
constexpr HRESULT DSERR_INVALIDPARAM = static_cast<HRESULT>(0x88780057u);
constexpr HRESULT DSERR_INVALIDCALL = static_cast<HRESULT>(0x88780032u);
constexpr HRESULT DSERR_PRIOLEVELNEEDED = static_cast<HRESULT>(0x88780046u);
constexpr DWORD DSSCL_PRIORITY = 2;
constexpr WORD WAVE_FORMAT_PCM = 1;
constexpr DWORD DSBSIZE_MIN = 4;
constexpr DWORD DSBSIZE_MAX = 0x0FFFFFFF;
constexpr DWORD DSBCAPS_GETCURRENTPOSITION2 = 0x00010000;
constexpr DWORD DSBCAPS_GLOBALFOCUS = 0x00008000;
constexpr DWORD DSBLOCK_ENTIREBUFFER = 0x00000002;
constexpr DWORD DSBPLAY_LOOPING = 0x00000001;
constexpr DWORD DSBSTATUS_LOOPING = 0x00000004;
constexpr DWORD WAIT_ABANDONED = 0x00000080;

struct WAVEFORMATEX {
    WORD wFormatTag{};
    WORD nChannels{};
    DWORD nSamplesPerSec{};
    DWORD nAvgBytesPerSec{};
    WORD nBlockAlign{};
    WORD wBitsPerSample{};
    WORD cbSize{};
};

struct DSBUFFERDESC {
    DWORD dwSize{};
    DWORD dwFlags{};
    DWORD dwBufferBytes{};
    DWORD dwReserved{};
    WAVEFORMATEX* lpwfxFormat{};
    GUID guid3DAlgorithm{};
};

struct IDirectSoundBuffer8 {
    virtual ~IDirectSoundBuffer8() = default;
    HRESULT QueryInterface(const GUID&, LPVOID* out) { if (out) *out = this; return S_OK; }
    ULONG Release() { return 0; }
    HRESULT Stop() { return S_OK; }
    HRESULT Play(DWORD, DWORD, DWORD) { return S_OK; }
    HRESULT SetCurrentPosition(DWORD) { return S_OK; }
    HRESULT GetCurrentPosition(DWORD* play, DWORD* write) { if (play) *play = 0; if (write) *write = 0; return S_OK; }
    HRESULT GetStatus(DWORD* status) { if (status) *status = 0; return S_OK; }
    template <typename Size1, typename Ptr2, typename Size2>
    HRESULT Lock(DWORD, DWORD bytes, LPVOID* p1, Size1* b1, Ptr2 p2, Size2 b2, DWORD) {
        static unsigned char dummy[8192];
        if (p1) *p1 = dummy;
        if (b1) *b1 = static_cast<Size1>(bytes ? bytes : sizeof(dummy));
        if constexpr (std::is_pointer_v<Ptr2>) {
            if (p2) *p2 = nullptr;
        }
        if constexpr (std::is_pointer_v<Size2>) {
            if (b2) *b2 = 0;
        }
        return S_OK;
    }
    HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD) { return S_OK; }
    HRESULT Restore() { return S_OK; }
};

using IDirectSoundBuffer = IDirectSoundBuffer8;

struct IDirectSound8 {
    HRESULT SetCooperativeLevel(HWND, DWORD) { return S_OK; }
    HRESULT CreateSoundBuffer(DSBUFFERDESC*, IDirectSoundBuffer** out, void*) { static IDirectSoundBuffer buffer; if (out) *out = &buffer; return S_OK; }
    ULONG Release() { return 0; }
};

inline HRESULT DirectSoundCreate8(const GUID*, IDirectSound8** out, void*) { static IDirectSound8 ds; if (out) *out = &ds; return S_OK; }

#endif
