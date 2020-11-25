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

#include "ContextDX9.h"
#include "config.h"

struct VERTEX
{
	float fX;
	float fY;
	float fZ;
	D3DCOLOR Color;
};

void CreateVERTEX(VERTEX *v, float X, float Y, D3DCOLOR Color, float Z = 1.f)
{
	v->fX = X;
	v->fY = Y;
	v->fZ = Z;
	v->Color = Color;
}


ContextDX9::ContextDX9(RendererVideo * renderer)
	: Context(renderer)
{
}

ContextDX9::~ContextDX9()
{
}

bool ContextDX9::CanCreate()
{
	return false;
}

bool ContextDX9::Init()
{
	return false;
}

void ContextDX9::Clear(bool device)
{
}

void ContextDX9::Render(bool recreateFrame)
{
}

bool ContextDX9::DrawTexture(unsigned char * nframe, bool copy)
{
	return false;
}

void ContextDX9::DrawProgressBar()
{
}

void  ContextDX9::DrawZoom()
{

}
void  ContextDX9::DrawRect(D3DXVECTOR2 position, bool sel, float size)
{ 
	D3DCOLOR fill = (sel) ? 0xAAFCE6B1 : 0xAA121150;
	VERTEX v9[9];
	CreateVERTEX(&v9[0], position.x - size, position.y - size, fill);
	CreateVERTEX(&v9[1], position.x + size, position.y - size, fill);
	CreateVERTEX(&v9[2], position.x - size, position.y + size, fill);
	CreateVERTEX(&v9[3], position.x + size, position.y + size, fill);
	CreateVERTEX(&v9[4], position.x - size, position.y - size, 0xFFBB0000);
	CreateVERTEX(&v9[5], position.x + size, position.y - size, 0xFFBB0000);
	CreateVERTEX(&v9[6], position.x + size, position.y + size, 0xFFBB0000);
	CreateVERTEX(&v9[7], position.x - size, position.y + size, 0xFFBB0000);
	CreateVERTEX(&v9[8], position.x - size, position.y - size, 0xFFBB0000);

	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX)), L"primitive failed");
}

void  ContextDX9::DrawCircle(D3DXVECTOR2 position, bool sel, float size)
{ 
	D3DCOLOR fill = (sel) ? 0xAAFCE6B1 : 0xAA121150;
	VERTEX v5[41];
	float rad = 0.01745329251994329576923690768489f;

	float xx = position.x;
	float yy = position.y;
	CreateVERTEX(&v5[0], xx, yy, fill);
	for (int j = 0; j < 20; j++)
	{
		float xx1 = position.x + (size * sin((j * 20) * rad));
		float yy1 = position.y + (size * cos((j * 20) * rad));
		CreateVERTEX(&v5[j + 1], xx1, yy1, fill);
		CreateVERTEX(&v5[j + 21], xx1, yy1, 0xFFBB0000);
		xx = xx1;
		yy = yy1;

	}

	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 18, v5, sizeof(VERTEX)), L"primitive failed");
	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 18, &v5[21], sizeof(VERTEX)), L"primitive failed");
}

void  ContextDX9::DrawCross(D3DXVECTOR2 position, unsigned int color, bool useBegin)
{ 
	D3DXVECTOR2 cross[4];
	cross[0].x = position.x - 15.0f;
	cross[0].y = position.y;
	cross[1].x = position.x + 15.0f;
	cross[1].y = position.y;
	cross[2].x = position.x;
	cross[2].y = position.y - 15.0f;
	cross[3].x = position.x;
	cross[3].y = position.y + 15.0f;
	if (useBegin) { m_D3DLine->Begin(); }
	m_D3DLine->Draw(cross, 2, color);
	m_D3DLine->Draw(&cross[2], 2, color);
	if (useBegin) { m_D3DLine->End(); }
}
//positionEnd moves end line point to arrow start
void  ContextDX9::DrawArrow(D3DXVECTOR2 positionStart, D3DXVECTOR2 *positionEnd, int diff)
{ 
	D3DXVECTOR2 pdiff = positionStart - (*positionEnd);
	float len = sqrt((pdiff.x * pdiff.x) + (pdiff.y * pdiff.y));
	D3DXVECTOR2 diffUnits = (len == 0) ? D3DXVECTOR2(0, 0) : pdiff / len;
	//length can have values less than zero, change to plus to pravent bad arrow drawing
	D3DXVECTOR2 pend = (*positionEnd) + (diffUnits * (12 + diff));
	D3DXVECTOR2 halfbase = D3DXVECTOR2(-diffUnits.y, diffUnits.x) * 5.f;

	VERTEX v4[7];
	D3DXVECTOR2 v3[3];
	v3[0] = pend - diffUnits * 12;
	v3[1] = pend + halfbase;
	v3[2] = pend - halfbase;

	CreateVERTEX(&v4[0], v3[0].x, v3[0].y, 0xAA121150);
	CreateVERTEX(&v4[1], v3[1].x, v3[1].y, 0xAA121150);
	CreateVERTEX(&v4[2], v3[2].x, v3[2].y, 0xAA121150);
	CreateVERTEX(&v4[3], v3[0].x, v3[0].y, 0xFFBB0000);
	CreateVERTEX(&v4[4], v3[1].x, v3[1].y, 0xFFBB0000);
	CreateVERTEX(&v4[5], v3[2].x, v3[2].y, 0xFFBB0000);
	CreateVERTEX(&v4[6], v3[0].x, v3[0].y, 0xFFBB0000);

	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, v4, sizeof(VERTEX)), L"primitive failed");
	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 3, &v4[3], sizeof(VERTEX)), L"primitive failed");
	*positionEnd = pend;
}

void  ContextDX9::DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen, unsigned int color)
{ 
	D3DXVECTOR2 actualPoint[2];
	for (size_t i = 0; i < vectorSize - 1; i++) {
		size_t iPlus1 = (i < (vectorSize - 1)) ? i + 1 : 0;
		D3DXVECTOR2 pdiff = vector[i] - vector[iPlus1];
		float len = sqrt((pdiff.x * pdiff.x) + (pdiff.y * pdiff.y));
		if (len == 0) { return; }
		D3DXVECTOR2 diffUnits = pdiff / len;
		float singleMovement = 1 / (len / (dashLen * 2));
		actualPoint[0] = vector[i];
		actualPoint[1] = actualPoint[0];
		for (float j = 0; j <= 1; j += singleMovement) {
			actualPoint[1] -= diffUnits * dashLen;
			if (j + singleMovement >= 1) { actualPoint[1] = vector[iPlus1]; }
			m_D3DLine->Draw(actualPoint, 2, color);
			actualPoint[1] -= diffUnits * dashLen;
			actualPoint[0] -= (diffUnits * dashLen) * 2;
		}
	}
}
void  ContextDX9::DrawLines(D3DXVECTOR2 *vector, size_t vectorSize, unsigned int color)
{ 

}
int ContextDX9::CreateFont(const wxString &fontName, int height, bool bold, bool italic)
{
	size_t size = m_D3DFonts.size();
	LPD3DXFONT D3DFont = NULL;
	HRESULT hr = D3DXCreateFontW(m_D3DDevice, height, 0, bold? FW_BOLD : 0, 0, italic, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName.wc_str(), &D3DFont);

	if (FAILED(hr)) {
		KaiLogSilent(_("Nie mo¿na stworzyæ czcionki D3DX"));
		return size - 1;
	}
	m_D3DFonts.push_back(D3DFont);
	return size;
}
void  ContextDX9::DrawOutlinedText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color)
{ 
	if (fontIndex >= m_D3DFonts.size()) {
		KaiLogDebug(L"Font index not exist");
	}
	LPD3DXFONT font = m_D3DFonts[fontIndex];
	RECT tmpr = textRect;
	tmpr.top--; tmpr.bottom--; 
	tmpr.left--; tmpr.right--; 
	for (int i = 0; i < 9; i++)
	{
		if (i % 3 == 0 && i > 0) { tmpr.left = textRect.left - 1; tmpr.right = textRect.right - 1; tmpr.top++; tmpr.bottom++; }
			if (i != 4) { font->DrawTextW(NULL, text.wchar_str(), -1, &tmpr, textAlign, 0xFF000000); }
				tmpr.left++; tmpr.right++; 
	}
	font->DrawTextW(NULL, text.wchar_str(), -1, &textRect, textAlign, color);
}
void  ContextDX9::DrawNormalText(int fontIndex, const wxString &text, RECT textRect, unsigned int textAlign, unsigned int color)
{ 
	if (fontIndex >= m_D3DFonts.size()) {
		KaiLogDebug(L"Font index not exist");
	}
	LPD3DXFONT font = m_D3DFonts[fontIndex];
	font->DrawTextW(NULL, text.wchar_str(), -1, &textRect, textAlign, color);
}
void  ContextDX9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, unsigned int PrimitiveCount, const void* pVertexStreamZeroData, unsigned int VertexStreamZeroStride)
{ 
	HRN(m_D3DDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride), L"primitive failed");
}