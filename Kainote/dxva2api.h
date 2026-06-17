#pragma once
#ifndef _WIN32
#include "platform.h"
using DXVA2_VideoTransferMatrix = int;
constexpr DXVA2_VideoTransferMatrix DXVA2_VideoTransferMatrix_BT601 = 0;
constexpr DXVA2_VideoTransferMatrix DXVA2_VideoTransferMatrix_BT709 = 1;
struct IDirectXVideoProcessorService { ULONG Release(){ delete this; return 0; } };
struct IDirectXVideoProcessor { ULONG Release(){ delete this; return 0; } };
#endif
