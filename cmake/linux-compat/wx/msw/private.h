// Linux-only shims for the three wx headers wxGTK does not have (see
// CMakeLists.txt).  They must not be reachable from the Windows build: MSVC
// would resolve wx's own "wx/msw/popupwin.h" through the include stack.

#pragma once
