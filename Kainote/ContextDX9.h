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
#include <dxva2api.h>
#include <vector>

class ContextDX9 : public Context {
public:
	ContextDX9(RendererVideo *renderer);
	virtual ~ContextDX9();
	static bool CanCreate();
	bool Init() override;
	void Clear(bool device = false) override;
	void Render(bool recreateFrame) override;
	bool DrawTexture(unsigned char *nframe = nullptr, bool copy = false) override;
	void DrawProgressBar() override;
	void DrawZoom() override;
	void DrawRect(D3DXVECTOR2 position, bool sel = false, float size = 5.0f) override;
	void DrawCircle(D3DXVECTOR2 position, bool sel = false, float size = 6.0f) override;
	void DrawCross(D3DXVECTOR2 position, unsigned int color = 0xFFFF0000, bool useBegin = true) override;
	//positionEnd moves end line point to arrow start
	void DrawArrow(D3DXVECTOR2 positionStart, D3DXVECTOR2 *positionEnd, int diff = 0) override;
	void DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen = 4, unsigned int color = 0xFFBB0000) override;
	void DrawLines(D3DXVECTOR2 *vector, size_t vectorSize, unsigned int color) override;
	int CreateFont(const wxString &fontName, int height, bool bold, bool italic) override;
	void DrawOutlinedText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color) override;
	void DrawNormalText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color) override;
	void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, unsigned int PrimitiveCount, const void* pVertexStreamZeroData, unsigned int VertexStreamZeroStride) override;

private:
	IDirectXVideoProcessorService *m_DXVAService = nullptr;
	IDirectXVideoProcessor *m_DXVAProcessor = nullptr;
	LPDIRECT3D9 m_D3DObject = nullptr;
	LPDIRECT3DSURFACE9 m_BlackBarsSurface = nullptr;
	LPDIRECT3DSURFACE9 m_MainSurface = nullptr;
	LPDIRECT3DDEVICE9 m_D3DDevice = nullptr;
	D3DFORMAT m_D3DFormat;
	ID3DXLine *m_D3DLine = nullptr;
	std::vector<LPD3DXFONT> m_D3DFonts;
	D3DXVECTOR2 vectors[12];
};