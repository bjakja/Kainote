#ifndef _ThreadScriptEnvironment_H
#define _ThreadScriptEnvironment_H

#include <avisynth.h>
#ifdef AVS_WINDOWS
#include <avs/win.h>
#else
#include <avs/posix.h>
#endif

#include <cstdarg>
#include "vartable.h"
#include "internal.h"
#include "ThreadPool.h"
#include "BufferPool.h"
#include "DeviceManager.h"
#include "InternalEnvironment.h"

#endif  // _ThreadScriptEnvironment_H
