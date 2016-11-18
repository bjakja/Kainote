//  Copyright (c) 2016, Marcin Drob

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
#include <wx/string.h>

typedef unsigned char BYTE;

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024];	

class CColorSpaceConverter
{
public:
	CColorSpaceConverter(int fmt, int width, int height);
	virtual ~CColorSpaceConverter(void);

	BYTE* convert_to_rgb24(BYTE* frameBuffer);
	void SavePNG(wxString path, BYTE* frameBuffer);
	void SavetoClipboard(BYTE* frameBuffer);

private:
	BYTE* m_pRgbaBuffer;

	int fmt;
	int m_width; 
	int m_height;

	int m_uPlanePos;
	int m_vPlanePos;
};

