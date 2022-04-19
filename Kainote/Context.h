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
#if __WXMSW__
//#include <d3d9.h>
//#include <d3dx9.h>
#else
// make some definition for not windows systems
//D3DXVECTOR2
//D3DPRIMITIVETYPE
//RECT
#endif

class RendererVideo;


class Context {
public:
	Context(RendererVideo *renderer) {};
	~Context() {};
	void GetContextList(wxArrayString &list);
	virtual bool Init() {};
	virtual void Clear(bool device = false) {};
	virtual void Render(bool recreateFrame) {};
	virtual bool DrawTexture(unsigned char *nframe = nullptr, bool copy = false) {};
	virtual void DrawProgressBar() {};
	virtual void DrawZoom() {};
	virtual void DrawRect(D3DXVECTOR2 position, bool sel = false, float size = 5.0f) {};
	virtual void DrawCircle(D3DXVECTOR2 position, bool sel = false, float size = 6.0f) {};
	virtual void DrawCross(D3DXVECTOR2 position, unsigned int color = 0xFFFF0000, bool useBegin = true) {};
	//positionEnd moves end line point to arrow start
	virtual void DrawArrow(D3DXVECTOR2 positionStart, D3DXVECTOR2 *positionEnd, int diff = 0) {};
	virtual void DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen = 4, unsigned int color = 0xFFBB0000) {};
	virtual void DrawLines(D3DXVECTOR2 *vector, size_t vectorSize, unsigned int color) {};
	virtual int CreateFont(const wxString &fontName, int height, bool bold, bool italic) {};
	virtual void DrawOutlinedText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color) {};
	virtual void DrawNormalText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color) {};
	virtual void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, unsigned int PrimitiveCount, const void* pVertexStreamZeroData, unsigned int VertexStreamZeroStride) {};
};
