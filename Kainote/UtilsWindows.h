//  Copyright (c) 2017 - 2020, Marcin Drob

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
#undef GetClassInfo

//
#include <vector>
#include <wx/dc.h>
//#include <windows.h>

wxRect GetMonitorWorkArea(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxPoint &position, bool workArea);
wxRect GetMonitorRect1(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxRect &programRect);
int FindMonitor(std::vector<tagRECT> *MonitorRects, const wxPoint &pos);


#ifdef _M_IX86
void SetThreadName(DWORD id, LPCSTR szThreadName);
#else
void SetThreadName(size_t id, LPCSTR szThreadName);
#endif


