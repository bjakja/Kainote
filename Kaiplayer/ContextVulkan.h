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

#include "Context.h"

class ContextVulkan : public Context {
public:
	ContextVulkan(RendererVideo *renderer);
	virtual ~ContextVulkan();
	static bool CanCreate();
	bool Init();
	void Render(bool recreateFrame);
	bool DrawTexture(unsigned char *nframe = NULL, bool copy = false);
	void DrawProgressBar();
	void DrawZoom();
	void DrawRect(D3DXVECTOR2 position, bool sel = false, float size = 5.0f);
	void DrawCircle(D3DXVECTOR2 position, bool sel = false, float size = 6.0f);
	void DrawCross(D3DXVECTOR2 position, unsigned int color = 0xFFFF0000, bool useBegin = true);
	//positionEnd moves end line point to arrow start
	void DrawArrow(D3DXVECTOR2 positionStart, D3DXVECTOR2 *positionEnd, int diff = 0);
	void DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen = 4, unsigned int color = 0xFFBB0000);
	void DrawLines(D3DXVECTOR2 *vector, size_t vectorSize, unsigned int color);
	int CreateFont(const wxFont &font) override;
	void DrawOutlinedText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color);
	void DrawNormalText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color);
	void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, unsigned int PrimitiveCount, const void* pVertexStreamZeroData, unsigned int VertexStreamZeroStride);
};
