#pragma once
#ifndef _WIN32
#include "platform.h"
#ifndef LOWORD
#define LOWORD(l) ((WORD)((uintptr_t)(l) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((uintptr_t)(l) >> 16))
#endif
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif
#endif
