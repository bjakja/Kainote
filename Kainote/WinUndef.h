//  Copyright (c) 2017 - 2026, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

// wx/msw/winundef.h has no include guard by design; applying it twice while the
// macros are still defined redefines DrawText and MSVC reports C2084.  Kainote
// includes it from thirty-odd headers, so apply it once per translation unit.
#include <wx/msw/winundef.h>
