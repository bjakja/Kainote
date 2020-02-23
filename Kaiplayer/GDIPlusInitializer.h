//  Copyright (c) 2020, Marcin Drob

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
#include <wx/wx.h>
#define NOMINMAX 1

//  override byte to prevent clashes with <cstddef>
//#define byte win_byte_override

//#include <Windows.h> // gdi plus requires Windows.h
// ...includes for other windows header that may use byte...

//  Define min max macros required by GDI+ headers.
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#else
#error max macro is already defined
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#else
#error min macro is already defined
#endif

#include <GdiPlus.h>

#undef min
#undef max


using namespace Gdiplus;

class GDIPlusInitializer{
private:
	ULONG_PTR m_gditoken;
	int m_loaded = -1;
	// Do not use this pointer directly, it's only used by
	// GetDrawTextStringFormat() and the cleanup code in wxGDIPlusRendererModule.
	StringFormat* m_drawTextStringFormat = NULL;
public:
	GDIPlusInitializer(){
		LoadGDIPlus();
	};
	~GDIPlusInitializer(){
		UnloadGDIPlus();
	};
	void LoadGDIPlus();
	void UnloadGDIPlus();
	bool Check();
	StringFormat* GetDrawTextStringFormat();
	PrivateFontCollection *m_fontCollection = NULL;
	FontFamily *m_fontFamilies = NULL;
};

extern GDIPlusInitializer Initializer;