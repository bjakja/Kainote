
#include "png.h"
//#include "kcsri.h"
#include "kAssParser.h"
#include "kRenderer.h"
#include "windowsx.h" // selectfont
extern "C" {
#include <L:/Kainote/thirdparty/luajit\include\lua.hpp>
}

#define EXTERNC extern "C"

#ifdef KMAIN_EXPORTS
#define KMAIN_API EXTERNC __declspec(dllexport)
#else
#define KMAIN_API __declspec(dllimport)
#endif

// init
KMAIN_API int KHlibinit(lua_State* L);