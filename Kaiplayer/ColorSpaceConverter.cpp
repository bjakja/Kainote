//  Copyright (c) 2016 - 2020, Marcin Drob

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


#include "ColorSpaceConverter.h"
#include <wx/image.h>
#include <wx/clipbrd.h>
#include <wx/log.h>

#define clamp(x) max(min(255, x), 0)

static void InitConvertTable()
{
	long int crv, cbu, cgu, cgv;
	int i, ind;

	crv = 104597; cbu = 132201;
	cgu = 25675;  cgv = 53279;

	for (i = 0; i < 256; i++)
	{
		crv_tab[i] = (i - 128) * crv;
		cbu_tab[i] = (i - 128) * cbu;
		cgu_tab[i] = (i - 128) * cgu;
		cgv_tab[i] = (i - 128) * cgv;
		tab_76309[i] = 76309 * (i - 16);
	}

	for (i = 0; i < 384; i++)
		clp[i] = 0;
	ind = 384;
	for (i = 0; i < 256; i++)
		clp[ind++] = i;
	ind = 640;
	for (i = 0; i < 384; i++)
		clp[ind++] = 255;
}

static inline void get_rgb_from_yuv(int y, int u, int v, BYTE* r, BYTE* g, BYTE* b)
{
	int c1 = crv_tab[v];
	int c2 = cgu_tab[u];
	int c3 = cgv_tab[v];
	int c4 = cbu_tab[u];

	int y1 = tab_76309[y];
	*r = clp[384 + ((y1 + c1) >> 16)];
	*g = clp[384 + ((y1 - c2 - c3) >> 16)];
	*b = clp[384 + ((y1 + c4) >> 16)];
}

static inline void YUV420_to_RGB32(BYTE* src0, BYTE* src1, BYTE* src2, BYTE* dst, int width, int height, bool isRGB32)
{
	int y1, y2, u, v;
	unsigned char *py1, *py2;
	int i, j, c1, c2, c3, c4;
	unsigned char *d1, *d2;
	int bytesPerColor = isRGB32? 4 : 3;

	py1 = src0;
	py2 = py1 + width;
	d1 = dst;
	d2 = d1 + bytesPerColor * width;
	for (j = 0; j < height; j += 2)
	{
		for (i = 0; i < width; i += 2)
		{
			u = *src1++;
			v = *src2++;

			c4 = (isRGB32) ? crv_tab[v] : cbu_tab[u];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c1 = (isRGB32) ? cbu_tab[u] : crv_tab[v];

			//up-left
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c1) >> 16)]; 
			if(isRGB32)
				*d1++ = 0xff;

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c1) >> 16)]; 
			if (isRGB32)
				*d2++ = 0xff;

			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c1) >> 16)]; 
			if (isRGB32)
				*d1++ = 0xff;

			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c1) >> 16)]; 
			if (isRGB32)
				*d2++ = 0xff;
		}

		d1 += bytesPerColor * width;
		d2 += bytesPerColor * width;
		py1 += width;
		py2 += width;
	}
}

static inline void NV12_to_RGB32(BYTE* luma, BYTE* uv, BYTE* dst, int width, int height, bool isRGB32)
{
	int y1, y2, u, v;
	unsigned char *py1, *py2;
	int i, j, c1, c2, c3, c4;
	unsigned char *d1, *d2;
	int bytesPerColor = isRGB32 ? 4 : 3;

	py1 = luma;
	py2 = py1 + width;
	d1 = dst;
	d2 = d1 + bytesPerColor * width;
	for (j = 0; j < height; j += 2)
	{
		for (i = 0; i < width; i += 2)
		{
			v = *uv++;
			u = *uv++;

			c4 = (isRGB32)? crv_tab[v] : cbu_tab[u];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c1 = (isRGB32) ? cbu_tab[u] : crv_tab[v];

			//up-left
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c1) >> 16)]; 
			if (isRGB32)
				*d1++ = 0xff;

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c1) >> 16)]; 
			if (isRGB32)
				*d2++ = 0xff;

			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c1) >> 16)]; 
			if (isRGB32)
				*d1++ = 0xff;

			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c1) >> 16)]; 
			if (isRGB32)
				*d2++ = 0xff;
		}

		d1 += bytesPerColor * width;
		d2 += bytesPerColor * width;
		py1 += width;
		py2 += width;
	}
}


static inline bool yuy2_to_rgb32(const BYTE* input, BYTE* output, int numOfPixels, bool isRGB32)
{
	if (!input || !output)
	{
		return false;
	}

	BYTE r, g, b;
	for (int i = 0; i < numOfPixels / 2; i++)
	{
		int y0 = *(input++);
		int u = *(input++);
		int y1 = *(input++);
		int v = *(input++);

		get_rgb_from_yuv(y0, u, v, &r, &g, &b);
		*(output++) = (isRGB32)? b : r;
		*(output++) = g;
		*(output++) = (isRGB32) ? r : b;
		if (isRGB32)
			*(output++) = 0xff;  

		get_rgb_from_yuv(y1, u, v, &r, &g, &b);
		*(output++) = (isRGB32) ? b : r;
		*(output++) = g;
		*(output++) = (isRGB32) ? r : b;
		if (isRGB32)
			*(output++) = 0xff;         
	}

	return true;
}

CColorSpaceConverter::CColorSpaceConverter(int _fmt, int width, int height)
	: m_pRgbaBuffer(0)
{
	InitConvertTable();
	fmt = _fmt;
	m_width = width;
	m_height = height;
	m_uPlanePos = width * height;
	m_vPlanePos = m_uPlanePos + m_uPlanePos / 4;

	//wxImage release this malloc
	int size = width * height * 4;
	m_pRgbaBuffer = (BYTE*)malloc(size);
	memset(m_pRgbaBuffer, 0, size);
}

CColorSpaceConverter::~CColorSpaceConverter(void)
{
}

BYTE* CColorSpaceConverter::convert_to_rgb32(BYTE* frameBuffer)
{
	if (fmt == 3)
	{
		YUV420_to_RGB32(frameBuffer, &frameBuffer[m_uPlanePos], &frameBuffer[m_vPlanePos], m_pRgbaBuffer, m_width, m_height, true);
	}
	else if (fmt == 5)
	{
		NV12_to_RGB32(frameBuffer, &frameBuffer[m_uPlanePos], m_pRgbaBuffer, m_width, m_height, true);
	}
	else if (fmt == 2)
	{
		yuy2_to_rgb32(frameBuffer, m_pRgbaBuffer, m_width * m_height, true);
	}
	else if (fmt == 1)
	{
		size_t fplane = m_height * m_width * 3;
		BYTE *buff = m_pRgbaBuffer;
		for (size_t i = 0; i < fplane; i += 3) {
			*buff++ = frameBuffer[i + 2];
			*buff++ = frameBuffer[i + 1];
			*buff++ = frameBuffer[i + 0];
			*buff++ = 0xff;
		}
	}
	
	fmt = 0;
	return m_pRgbaBuffer;
}

BYTE* CColorSpaceConverter::convert_to_rgb24(BYTE* frameBuffer)
{
	if (fmt == 3)
	{
		YUV420_to_RGB32(frameBuffer, &frameBuffer[m_uPlanePos], &frameBuffer[m_vPlanePos], m_pRgbaBuffer, m_width, m_height, false);
	}
	else if (fmt == 5)
	{
		NV12_to_RGB32(frameBuffer, &frameBuffer[m_uPlanePos], m_pRgbaBuffer, m_width, m_height, false);
	}
	else if (fmt == 2)
	{
		yuy2_to_rgb32(frameBuffer, m_pRgbaBuffer, m_width * m_height, false);
	}
	else if(fmt == 0) {
		size_t fplane = m_height*m_width * 4;
		int rgb24size = m_height*m_width * 3;
		BYTE *buff = m_pRgbaBuffer;
		for (size_t i = 0; i < fplane; i += 4){
			*buff++ = frameBuffer[i + 2];
			*buff++ = frameBuffer[i + 1];
			*buff++ = frameBuffer[i + 0];
		}

	}
	fmt = 1;

	return m_pRgbaBuffer;
}

void CColorSpaceConverter::SavePNG(wxString path, BYTE* frameBuffer)
{
	BYTE* conv = convert_to_rgb24(frameBuffer);
	wxImage frame(m_width, m_height, false);
	frame.SetData(conv);
	frame.SaveFile(path, wxBITMAP_TYPE_PNG);
}

void CColorSpaceConverter::SavetoClipboard(BYTE* frameBuffer)
{
	BYTE* conv = convert_to_rgb24(frameBuffer);
	wxImage frame(m_width, m_height, false);
	frame.SetData(conv);
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxBitmapDataObject(frame));
		wxTheClipboard->Close();
	}
}