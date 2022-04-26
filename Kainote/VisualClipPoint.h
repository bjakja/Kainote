//  Copyright (c) 2022, Marcin Drob

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
//#include "VisualClips.h"
#include <wx/string.h>
#include <wx/window.h>
#include <d3d9.h>
#include <d3dx9.h>

class DrawingAndClip;

class ClipPoint
{
public:
	ClipPoint(float x, float y, wxString type, bool isstart);
	ClipPoint();
	bool IsInPos(D3DXVECTOR2 pos, float diff);
	D3DXVECTOR2 GetVector(DrawingAndClip* parent);
	float wx(DrawingAndClip* parent, bool zoomConversion = false);
	float wy(DrawingAndClip* parent, bool zoomConversion = false);
	float x;
	float y;
	wxString type;
	bool start;
	bool isSelected;
};