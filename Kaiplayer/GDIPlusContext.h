/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/graphics.cpp
// Purpose:     wxGCDC class
// Author:      Stefan Csomor
// Modified by:
// Created:     2006-09-30
// Copyright:   (c) 2006 Stefan Csomor, 2020 Marcin Drob
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include <wx/wx.h>
#include "wx/stack.h"
#include <GdiPlus.h>

using namespace Gdiplus;

class GDIPlus{
protected:
	Gdiplus::Graphics *m_context;
	wxStack<GraphicsState> m_stateStack;
	GraphicsState m_state1;
	GraphicsState m_state2;
	int m_width;
	int m_height;
	float m_penWidth = 1.f;
	bool m_loaded = false;
	Font *m_font = NULL;
	Pen *m_pen = NULL;
	Brush *m_brush = NULL;
	Image *m_image = NULL;
	Brush *m_fontBrush = NULL;
	Bitmap *m_bitmap = NULL;
	Bitmap *m_helper = NULL;
	ULONG_PTR m_gditoken;

	void Init(Graphics* graphics, int width, int height);
	void InitFont(const wxFont &font);
	void InitBrush(const wxBrush &brush);
	void InitPen(const wxPen &pen);
	void InitBitmap(const wxBitmap &bitmap);
	void LoadGDIPlus();
	void UnloadGDIPlus();
public:
	GDIPlus();
	~GDIPlus();
	static GDIPlus *Create(wxWindow *win);
	static GDIPlus *Create(const wxMemoryDC &dc);
	static GDIPlus *Create(const wxWindowDC &dc);

	void Clip(const wxRegion &region);
	void Clip(float x, float y, float w, float h);
	//void DrawEllipse(float x, float y, float w, float h);
	void DrawIcon(const wxIcon &icon, float x, float y, float w, float h);
	void DrawLine(float x, float y, float x1, float y1);
	void DrawRectangle(float x, float y, float w, float h);
	//void DrawRoundedRectangle(float x, float y, float w, float h, float radius);
	void DrawText(const wxString &str, float x, float y);
	void DrawText(const wxString &str, float x, float y, float angle);
	Graphics * GetNativeContext();
	void GetTextExtent(const wxString &text, float *width, float *height = NULL, float *descent = NULL, float *externalLeading = NULL) const;
	void ResetClip();
	void Translate(float dx, float dy);
	void Rotate(float angle);
	void Scale(float xScale, float yScale);
	//write this methods
	void SetBrush(const wxBrush &brush);
	void SetFont(const wxFont &font);
	void SetFontBrush(const wxBrush &colour);
	void SetPen(const wxPen &pen, float width = 1.f);
	//void StrokeLine(float x1, float y1, float x2, float y2);
	//void Flush();
	void DrawBitmap(const wxBitmap &bmp, float x, float y, float w, float h);

	static GDIPlus *staticThis;
};