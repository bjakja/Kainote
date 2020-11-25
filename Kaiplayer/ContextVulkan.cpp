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

//#include "wx/msw/private.h"

//HINSTANCE hInstance = wxGetInstance();

#include "ContextVulkan.h"

ContextVulkan::ContextVulkan(RendererVideo * renderer)
	: Context(renderer)
{
}

ContextVulkan::~ContextVulkan()
{
}

bool ContextVulkan::CanCreate()
{
	return false;
}

bool ContextVulkan::Init()
{
	return false;
}

void ContextVulkan::Render(bool recreateFrame)
{
}

bool ContextVulkan::DrawTexture(unsigned char * nframe, bool copy)
{
	return false;
}

void ContextVulkan::DrawProgressBar()
{
}

void  ContextVulkan::DrawZoom()
{

}
void  ContextVulkan::DrawRect(D3DXVECTOR2 position, bool sel = false, float size = 5.0f)
{

}
void  ContextVulkan::DrawCircle(D3DXVECTOR2 position, bool sel = false, float size = 6.0f)
{

}
void  ContextVulkan::DrawCross(D3DXVECTOR2 position, unsigned int color = 0xFFFF0000, bool useBegin = true)
{

}
//positionEnd moves end line point to arrow start
void  ContextVulkan::DrawArrow(D3DXVECTOR2 positionStart, D3DXVECTOR2 *positionEnd, int diff = 0)
{

}
void  ContextVulkan::DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen = 4, unsigned int color = 0xFFBB0000)
{

}
void  ContextVulkan::DrawLines(D3DXVECTOR2 *vector, size_t vectorSize, unsigned int color)
{

}
int ContextVulkan::CreateFont(const wxFont & font)
{
	return 0;
}
void  ContextVulkan::DrawOutlinedText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color)
{

}
void  ContextVulkan::DrawNormalText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color)
{

}
void  ContextVulkan::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, unsigned int PrimitiveCount, const void* pVertexStreamZeroData, unsigned int VertexStreamZeroStride)
{

}