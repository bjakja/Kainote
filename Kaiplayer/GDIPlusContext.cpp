/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/graphics.cpp
// Purpose:     wxGCDC class
// Author:      Stefan Csomor
// Modified by:
// Created:     2006-09-30
// Copyright:   (c) 2006 Stefan Csomor, 2020 Marcin Drob
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "GDIPlusContext.h"
#include "wx/msw/private.h"
#include "wx/msw/dc.h"
#include "wx/rawbmp.h"
#include "GDIPlusInitializer.h"

//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------
namespace{

	inline float dmin(float a, float b) { return a < b ? a : b; }
	inline float dmax(float a, float b) { return a > b ? a : b; }

	// translate a wxColour to a Color
	inline Color wxColourToColor(const wxColour& col)
	{
		return Color(col.Alpha(), col.Red(), col.Green(), col.Blue());
	}
}

#define CHECK_IF_INITIALIZED()\
	if(!Initializer.Check())\
		return NULL;

GDIPlus::GDIPlus(){}

GDIPlus *GDIPlus::Create(wxWindow *win)
{
	CHECK_IF_INITIALIZED();
	HWND hwnd = win->GetHWND();
	RECT rect = wxGetWindowRect(hwnd);
	GDIPlus* context = new GDIPlus();
	context->m_context = new Graphics(hwnd);
	context->Init(rect.right - rect.left, rect.bottom - rect.top);
	//context->EnableOffset(true);
	return context;
}

GDIPlus *GDIPlus::Create(const wxMemoryDC &dc)
{
	CHECK_IF_INITIALIZED();
#if wxUSE_WXDIB
	// It seems that GDI+ sets invalid values for alpha channel when used with
	// a compatible bitmap (DDB). So we need to convert the currently selected
	// bitmap to a DIB before using it with any GDI+ functions to ensure that
	// we get the correct alpha channel values in it at the end.

	wxBitmap bmp = dc.GetSelectedBitmap();
	wxASSERT_MSG(bmp.IsOk(), "Should select a bitmap before creating wxGCDC");

	// We don't need to convert it if it can't have alpha at all (any depth but
	// 32) or is already a DIB with alpha.
	if (bmp.GetDepth() == 32 && (!bmp.IsDIB() || !bmp.HasAlpha()))
	{
		// We need to temporarily deselect this bitmap from the memory DC
		// before modifying it.
		const_cast<wxMemoryDC&>(dc).SelectObject(wxNullBitmap);

		bmp.ConvertToDIB(); // Does nothing if already a DIB.

		if (!bmp.HasAlpha())
		{
			// Initialize alpha channel, even if we don't have any alpha yet,
			// we should have correct (opaque) alpha values in it for GDI+
			// functions to work correctly.
			{
				wxAlphaPixelData data(bmp);
				if (data)
				{
					wxAlphaPixelData::Iterator p(data);
					for (int y = 0; y < data.GetHeight(); y++)
					{
						wxAlphaPixelData::Iterator rowStart = p;

						for (int x = 0; x < data.GetWidth(); x++)
						{
							p.Alpha() = wxALPHA_OPAQUE;
							++p;
						}

						p = rowStart;
						p.OffsetY(data, 1);
					}
				}
			} // End of block modifying the bitmap.

			// Using wxAlphaPixelData sets the internal "has alpha" flag but we
			// don't really have any alpha yet, so reset it back for now.
			bmp.ResetAlpha();
		}

		// Undo SelectObject() at the beginning of this block.
		const_cast<wxMemoryDC&>(dc).SelectObjectAsSource(bmp);
	}
#endif // wxUSE_WXDIB
	wxMSWDCImpl *msw = wxDynamicCast(dc.GetImpl(), wxMSWDCImpl);
	HDC hdc = (HDC)msw->GetHDC();
	wxSize sz = dc.GetSize();
	GDIPlus* context = new GDIPlus();
	context->m_context = new Graphics(hdc);
	context->Init(sz.x, sz.y);
	//context->EnableOffset(true);
	return context;
}

GDIPlus *GDIPlus::Create(const wxWindowDC &dc)
{
	CHECK_IF_INITIALIZED();

	wxMSWDCImpl *msw = wxDynamicCast(dc.GetImpl(), wxMSWDCImpl);
	HDC hdc = (HDC)msw->GetHDC();
	wxSize sz = dc.GetSize();
	
	GDIPlus* context = new GDIPlus();
	context->m_context = new Graphics(hdc);
	context->Init(sz.x, sz.y);
	//context->EnableOffset(true);
	return context;
}


void GDIPlus::Init(int width, int height)
{
	m_state1 = 0;
	m_state2 = 0;
	m_width = width;
	m_height = height;

	m_context->SetTextRenderingHint(TextRenderingHintSystemDefault);
	m_context->SetPixelOffsetMode(PixelOffsetModeHalf);
	m_context->SetSmoothingMode(SmoothingModeHighQuality);

	m_state1 = m_context->Save();
	m_state2 = m_context->Save();
}

GDIPlus::~GDIPlus()
{
	if (m_context)
	{
		m_context->Restore(m_state2);
		m_context->Restore(m_state1);
		delete m_context;
	}

	wxDELETE(m_font);
	wxDELETE(m_pen);
	wxDELETE(m_brush);
	wxDELETE(m_image);
	wxDELETE(m_fontBrush);
	wxDELETE(m_bitmap);
	
}

void GDIPlus::InitFont(const wxFont &font)
{
	//function made to prevent slow opening program when 
	//there is many fonts installed
	//loading fonts from memory to collection 
	if (m_font){
		delete m_font;
		m_font = NULL;
	}
	int style = FontStyleRegular;
	if (font.GetStyle() == wxFONTSTYLE_ITALIC)
		style |= FontStyleItalic;
	if (font.GetUnderlined())
		style |= FontStyleUnderline;
	if (font.GetStrikethrough())
		style |= FontStyleStrikeout;
	if (font.GetWeight() == wxFONTWEIGHT_BOLD)
		style |= FontStyleBold;

	REAL fontSize = (REAL)font.GetPixelSize().GetHeight();

	const int count = Initializer.m_fontCollection->GetFamilyCount();

	// We should find all the families, i.e. "found" should be "count".
	int found = 0;
	Initializer.m_fontCollection->GetFamilies(count, Initializer.m_fontFamilies, &found);
	wxString name = font.GetFaceName();
	for (int j = 0; j < found; j++)
	{
		wchar_t familyName[LF_FACESIZE];
		int rc = Initializer.m_fontFamilies[j].GetFamilyName(familyName);
		if (rc == 0 && name == familyName)
		{
			//need to test if it opens file from collection
			m_font = new Font(&Initializer.m_fontFamilies[j], fontSize, style, UnitPixel);
			return;
		}
	}

	HDC dc = ::CreateCompatibleDC(NULL);
	HFONT hf = font.GetHFONT();
	SelectObject(dc, hf);
	{
		DWORD ttcf = 0x66637474;
		auto size = GetFontData(dc, ttcf, 0, nullptr, 0);
		if (size == GDI_ERROR) {
			ttcf = 0;
			size = GetFontData(dc, 0, 0, nullptr, 0);
		}
		if (size == GDI_ERROR || size == 0){
			//slow version of loading font
			m_font = new Font(name, fontSize, style, UnitPixel);
			goto done;
		}
		std::string buffer;
		buffer.resize(size);
		GetFontData(dc, ttcf, 0, &buffer[0], (int)size);

		Status nResults = Initializer.m_fontCollection->AddMemoryFont(&buffer[0], size);

		if (nResults != Ok)
		{
			//slow version of loading font
			m_font = new Font(name, fontSize, style, UnitPixel);
			goto done;
		}
	}
	done:
		//fast version of loading font from memory
		m_font = new Font(name, fontSize, style, UnitPixel, Initializer.m_fontCollection);

		SelectObject(dc, NULL);
		DeleteObject(hf);
	
}

void GDIPlus::InitBrush(const wxBrush &brush, bool textBrush)
{
	if (textBrush){
		if (m_fontBrush){
			delete m_fontBrush;
			m_fontBrush = NULL;
		}
	}
	else{
		if (m_brush){
			delete m_brush;
			m_brush = NULL;
		}
	}
	Brush *newbrush = NULL;

	if (brush.GetStyle() == wxBRUSHSTYLE_SOLID)
	{
		newbrush = new SolidBrush(wxColourToColor(brush.GetColour()));
	}
	else if (brush.IsHatch())
	{
		HatchStyle style;
		switch (brush.GetStyle())
		{
		case wxBRUSHSTYLE_BDIAGONAL_HATCH:
			style = HatchStyleBackwardDiagonal;
			break;
		case wxBRUSHSTYLE_CROSSDIAG_HATCH:
			style = HatchStyleDiagonalCross;
			break;
		case wxBRUSHSTYLE_FDIAGONAL_HATCH:
			style = HatchStyleForwardDiagonal;
			break;
		case wxBRUSHSTYLE_CROSS_HATCH:
			style = HatchStyleCross;
			break;
		case wxBRUSHSTYLE_HORIZONTAL_HATCH:
			style = HatchStyleHorizontal;
			break;
		case wxBRUSHSTYLE_VERTICAL_HATCH:
			style = HatchStyleVertical;
			break;
		default:
			style = HatchStyleHorizontal;
		}
		newbrush = new HatchBrush
			(
			style,
			wxColourToColor(brush.GetColour()),
			Color::Transparent
			);
	}
	else
	{
		wxBitmap* bmp = brush.GetStipple();
		if (bmp && bmp->IsOk())
		{
			wxDELETE(m_image);
			m_image = Bitmap::FromHBITMAP((HBITMAP)bmp->GetHBITMAP(),
#if wxUSE_PALETTE
				(HPALETTE)bmp->GetPalette()->GetHPALETTE()
#else
				NULL
#endif
				);
			newbrush = new TextureBrush(m_image);
		}
	}
	if (textBrush){
		m_fontBrush = newbrush;
	}
	else{
		m_brush = newbrush;
	}

}

void GDIPlus::InitPen(const wxPen &pen)
{
	if (m_pen){
		delete m_pen;
		m_pen = NULL;
	}

	if (m_penWidth <= 0.0)
		m_penWidth = 0.1;

	m_pen = new Pen(wxColourToColor(pen.GetColour()), m_penWidth);

	LineCap cap;
	switch (pen.GetCap())
	{
	case wxCAP_ROUND:
		cap = LineCapRound;
		break;

	case wxCAP_PROJECTING:
		cap = LineCapSquare;
		break;

	case wxCAP_BUTT:
		cap = LineCapFlat; // TODO verify
		break;

	default:
		cap = LineCapFlat;
		break;
	}
	m_pen->SetLineCap(cap, cap, DashCapFlat);

	LineJoin join;
	switch (pen.GetJoin())
	{
	case wxJOIN_BEVEL:
		join = LineJoinBevel;
		break;

	case wxJOIN_MITER:
		join = LineJoinMiter;
		break;

	case wxJOIN_ROUND:
		join = LineJoinRound;
		break;

	default:
		join = LineJoinMiter;
		break;
	}

	m_pen->SetLineJoin(join);

	m_pen->SetDashStyle(DashStyleSolid);

	DashStyle dashStyle = DashStyleSolid;
	switch (pen.GetStyle())
	{
	case wxPENSTYLE_SOLID:
		break;

	case wxPENSTYLE_DOT:
		dashStyle = DashStyleDot;
		break;

	case wxPENSTYLE_LONG_DASH:
		dashStyle = DashStyleDash; // TODO verify
		break;

	case wxPENSTYLE_SHORT_DASH:
		dashStyle = DashStyleDash;
		break;

	case wxPENSTYLE_DOT_DASH:
		dashStyle = DashStyleDashDot;
		break;
	case wxPENSTYLE_USER_DASH:
	{
		dashStyle = DashStyleCustom;
		wxDash *dashes;
		int count = pen.GetDashes(&dashes);
		if ((dashes != NULL) && (count > 0))
		{
			REAL *userLengths = new REAL[count];
			for (int i = 0; i < count; ++i)
			{
				userLengths[i] = dashes[i];
			}
			m_pen->SetDashPattern(userLengths, count);
			delete[] userLengths;
		}
	}
	break;
	case wxPENSTYLE_STIPPLE:
	{
		wxBitmap *bmp = pen.GetStipple();
		if (bmp->IsOk())
		{
			m_image = Bitmap::FromHBITMAP((HBITMAP)bmp->GetHBITMAP(),
#if wxUSE_PALETTE
				(HPALETTE)bmp->GetPalette()->GetHPALETTE()
#else
				NULL
#endif
				);
			m_brush = new TextureBrush(m_image);
			m_pen->SetBrush(m_brush);
		}

	}
	break;
	default:
		if (pen.GetStyle() >= wxPENSTYLE_FIRST_HATCH &&
			pen.GetStyle() <= wxPENSTYLE_LAST_HATCH)
		{
			HatchStyle style;
			switch (pen.GetStyle())
			{
			case wxPENSTYLE_BDIAGONAL_HATCH:
				style = HatchStyleBackwardDiagonal;
				break;
			case wxPENSTYLE_CROSSDIAG_HATCH:
				style = HatchStyleDiagonalCross;
				break;
			case wxPENSTYLE_FDIAGONAL_HATCH:
				style = HatchStyleForwardDiagonal;
				break;
			case wxPENSTYLE_CROSS_HATCH:
				style = HatchStyleCross;
				break;
			case wxPENSTYLE_HORIZONTAL_HATCH:
				style = HatchStyleHorizontal;
				break;
			case wxPENSTYLE_VERTICAL_HATCH:
				style = HatchStyleVertical;
				break;
			default:
				style = HatchStyleHorizontal;
			}
			m_brush = new HatchBrush
				(
				style,
				wxColourToColor(pen.GetColour()),
				Color::Transparent
				);
			m_pen->SetBrush(m_brush);
		}
		break;
	}
	if (dashStyle != DashStyleSolid)
		m_pen->SetDashStyle(dashStyle);

	//for now not need gradients
	//Add it later when needed 
	/*switch (pen.GetGradientType())
	{
	case wxGRADIENT_NONE:
		break;

	case wxGRADIENT_LINEAR:
		if (m_brush)
			delete m_brush;
		CreateLinearGradientBrush(info.GetX1(), info.GetY1(),
			info.GetX2(), info.GetY2(),
			info.GetStops());
		m_pen->SetBrush(m_brush);
		break;

	case wxGRADIENT_RADIAL:
		if (m_brush)
			delete m_brush;
		CreateRadialGradientBrush(info.GetStartX(), info.GetStartY(),
			info.GetEndX(), info.GetEndY(),
			info.GetRadius(),
			info.GetStops());
		m_pen->SetBrush(m_brush);
		break;
	}*/
}

void GDIPlus::InitBitmap(const wxBitmap &bmp)
{
	if (m_bitmap){
		delete m_bitmap;
		m_bitmap = NULL;
	}
	
	m_helper = NULL;

	Bitmap* image = NULL;
	if (bmp.GetMask())
	{
		Bitmap interim((HBITMAP)bmp.GetHBITMAP(),
#if wxUSE_PALETTE
			(HPALETTE)bmp.GetPalette()->GetHPALETTE()
#else
			NULL
#endif
			);

		size_t width = interim.GetWidth();
		size_t height = interim.GetHeight();
		Rect bounds(0, 0, width, height);

		image = new Bitmap(width, height, PixelFormat32bppPARGB);

		Bitmap interimMask((HBITMAP)bmp.GetMask()->GetMaskBitmap(), NULL);
		wxASSERT(interimMask.GetPixelFormat() == PixelFormat1bppIndexed);

		BitmapData dataMask;
		interimMask.LockBits(&bounds, ImageLockModeRead,
			interimMask.GetPixelFormat(), &dataMask);


		BitmapData imageData;
		image->LockBits(&bounds, ImageLockModeWrite, PixelFormat32bppPARGB, &imageData);

		BYTE maskPattern = 0;
		BYTE maskByte = 0;
		size_t maskIndex;

		for (size_t y = 0; y < height; ++y)
		{
			maskIndex = 0;
			for (size_t x = 0; x < width; ++x)
			{
				if (x % 8 == 0)
				{
					maskPattern = 0x80;
					maskByte = *((BYTE*)dataMask.Scan0 + dataMask.Stride*y + maskIndex);
					maskIndex++;
				}
				else
					maskPattern = maskPattern >> 1;

				ARGB *dest = (ARGB*)((BYTE*)imageData.Scan0 + imageData.Stride*y + x * 4);
				if ((maskByte & maskPattern) == 0)
					*dest = 0x00000000;
				else
				{
					Color c;
					interim.GetPixel(x, y, &c);
					*dest = (c.GetValue() | Color::AlphaMask);
				}
			}
		}

		image->UnlockBits(&imageData);

		interimMask.UnlockBits(&dataMask);
		interim.UnlockBits(&dataMask);
	}
	else
	{
		image = Bitmap::FromHBITMAP((HBITMAP)bmp.GetHBITMAP(),
#if wxUSE_PALETTE
			(HPALETTE)bmp.GetPalette()->GetHPALETTE()
#else
			NULL
#endif
			);
		if (bmp.HasAlpha() && GetPixelFormatSize(image->GetPixelFormat()) == 32)
		{
			size_t width = image->GetWidth();
			size_t height = image->GetHeight();
			Rect bounds(0, 0, width, height);
			static BitmapData data;

			m_helper = image;
			image = NULL;
			m_helper->LockBits(&bounds, ImageLockModeRead,
				m_helper->GetPixelFormat(), &data);

			image = new Bitmap(data.Width, data.Height, data.Stride,
				PixelFormat32bppPARGB, (BYTE*)data.Scan0);

			m_helper->UnlockBits(&data);
		}
	}
	if (image)
		m_bitmap = image;
}

void GDIPlus::Clip(const wxRegion &region)
{
	Region rgn((HRGN)region.GetHRGN());
	m_context->SetClip(&rgn, CombineModeIntersect);
}

void GDIPlus::Clip(float x, float y, float w, float h)
{
	m_context->SetClip(RectF(x, y, w, h), CombineModeIntersect);
}

void GDIPlus::ResetClip()
{
	m_context->ResetClip();
}

void GDIPlus::DrawLine(float x, float y, float x1, float y1)
{
	if (m_pen)
	{
		m_context->DrawLine(m_pen, (REAL)x, (REAL)y, (REAL)x1, (REAL)y1);
	}
}

void GDIPlus::DrawRectangle(float x, float y, float w, float h)
{
	//if (m_composition == wxCOMPOSITION_DEST)
		//return;

	if (w < 0)
	{
		x += w;
		w = -w;
	}

	if (h < 0)
	{
		y += h;
		h = -h;
	}

	if (m_brush)
	{
		// the offset is used to fill only the inside of the rectangle and not paint underneath
		// its border which may influence a transparent Pen
		REAL offset = 0;
		if (m_pen)
			offset = m_penWidth;
		m_context->FillRectangle(m_brush, (REAL)x + offset / 2, (REAL)y + offset / 2, (REAL)w - offset, (REAL)h - offset);
	}

	if (m_pen)
	{
		m_context->DrawRectangle(m_pen, (REAL)x, (REAL)y, (REAL)w, (REAL)h);
	}
}

//void GDIPlus::DrawEllipse(float x, float y, float w, float h)
//{
//	wxGraphicsPath path = CreatePath();
//	path.AddEllipse(x, y, w, h);
//	DrawPath(path);
//}

void GDIPlus::Translate(float dx, float dy)
{
	m_context->TranslateTransform(dx, dy);
}

void GDIPlus::Rotate(float angle)
{
	m_context->RotateTransform(wxRadToDeg(angle));
}

void GDIPlus::Scale(float xScale, float yScale)
{
	m_context->ScaleTransform(xScale, yScale);
}

void GDIPlus::DrawBitmap(const wxBitmap &bmp, float x, float y, float w, float h)
{
	//if (m_composition == wxCOMPOSITION_DEST)
		//return;
	InitBitmap(bmp);

	if (m_bitmap)
	{
		if (m_bitmap->GetWidth() != (UINT)w || m_bitmap->GetHeight() != (UINT)h)
		{
			Rect drawRect((REAL)x, (REAL)y, (REAL)w, (REAL)h);
			m_context->SetPixelOffsetMode(PixelOffsetModeNone);
			m_context->DrawImage(m_bitmap, drawRect, 0, 0, m_bitmap->GetWidth(), m_bitmap->GetHeight(), UnitPixel);
			m_context->SetPixelOffsetMode(PixelOffsetModeHalf);
		}
		else
			m_context->DrawImage(m_bitmap, (REAL)x, (REAL)y, (REAL)w, (REAL)h);
	}
}

void GDIPlus::DrawIcon(const wxIcon &icon, float x, float y, float w, float h)
{
	//if (m_composition == wxCOMPOSITION_DEST)
		//return;

	// the built-in conversion fails when there is alpha in the HICON (eg XP style icons), we can only
	// find out by looking at the bitmap data whether there really was alpha in it
	HICON hIcon = (HICON)icon.GetHICON();
	AutoIconInfo iconInfo;
	if (!iconInfo.GetFrom(hIcon))
		return;

	Bitmap interim(iconInfo.hbmColor, NULL);

	Bitmap* image = NULL;

	// if it's not 32 bit, it doesn't have an alpha channel, note that since the conversion doesn't
	// work correctly, asking IsAlphaPixelFormat at this point fails as well
	if (GetPixelFormatSize(interim.GetPixelFormat()) != 32)
	{
		image = Bitmap::FromHICON(hIcon);
	}
	else
	{
		size_t width = interim.GetWidth();
		size_t height = interim.GetHeight();
		Rect bounds(0, 0, width, height);
		BitmapData data;

		interim.LockBits(&bounds, ImageLockModeRead,
			interim.GetPixelFormat(), &data);

		bool hasAlpha = false;
		for (size_t yy = 0; yy < height && !hasAlpha; ++yy)
		{
			for (size_t xx = 0; xx < width && !hasAlpha; ++xx)
			{
				ARGB *dest = (ARGB*)((BYTE*)data.Scan0 + data.Stride*yy + xx * 4);
				if ((*dest & Color::AlphaMask) != 0)
					hasAlpha = true;
			}
		}

		if (hasAlpha)
		{
			image = new Bitmap(data.Width, data.Height, data.Stride,
				PixelFormat32bppARGB, (BYTE*)data.Scan0);
		}
		else
		{
			image = Bitmap::FromHICON(hIcon);
		}

		interim.UnlockBits(&data);
	}

	m_context->DrawImage(image, (REAL)x, (REAL)y, (REAL)w, (REAL)h);

	delete image;
}

void GDIPlus::DrawText(const wxString &str, float x, float y, float angle)
{
	Translate(x, y);
	Rotate(-angle);
	DrawText(str, 0, 0);
	Rotate(angle);
	Translate(-x, -y);
}

void GDIPlus::DrawText(const wxString& str,
	float x, float y)
{
	//if (m_composition == wxCOMPOSITION_DEST)
		//return;

	if (!m_font)
		return;

	if (!m_fontBrush)
		return;

	if (str.IsEmpty())
		return;

	m_context->DrawString
		(
		str.wc_str(*wxConvUI),  // string to draw, always Unicode
		-1,                     // length: string is NUL-terminated
		m_font,
		PointF(x, y),
		Initializer.GetDrawTextStringFormat(),
		m_fontBrush
		);
}

void GDIPlus::GetTextExtent(const wxString &str, float *width, float *height,
	float *descent, float *externalLeading) const
{
	if (!m_font)
		return;

	wxWCharBuffer s = str.wc_str(*wxConvUI);

	// Get the font metrics if we actually need them.
	if (descent || externalLeading || (height && str.empty()))
	{
		FontFamily ffamily;
		m_font->GetFamily(&ffamily);

		// Notice that we must use the real font style or the results would be
		// incorrect for italic/bold fonts.
		const INT style = m_font->GetStyle();
		const REAL size = m_font->GetSize();
		const REAL emHeight = ffamily.GetEmHeight(style);
		REAL rDescent = ffamily.GetCellDescent(style) * size / emHeight;
		REAL rAscent = ffamily.GetCellAscent(style) * size / emHeight;
		REAL rHeight = ffamily.GetLineSpacing(style) * size / emHeight;

		if (height && str.empty())
			*height = rHeight;
		if (descent)
			*descent = rDescent;
		if (externalLeading)
			*externalLeading = rHeight - rAscent - rDescent;
	}

	// measuring empty strings is not guaranteed, so do it by hand
	if (str.IsEmpty())
	{
		if (width)
			*width = 0;

		// Height already assigned above if necessary.
	}
	else
	{
		RectF layoutRect(0, 0, 100000.0f, 100000.0f);

		RectF bounds;
		m_context->MeasureString((const wchar_t *)s, wcslen(s), m_font, layoutRect, Initializer.GetDrawTextStringFormat(), &bounds);
		if (width)
			*width = bounds.Width;
		if (height)
			*height = bounds.Height;
	}
}

Graphics* GDIPlus::GetNativeContext()
{
	return m_context;
}

void GDIPlus::SetBrush(const wxBrush &brush)
{
	InitBrush(brush, false);
}
void GDIPlus::SetFont(const wxFont &font)
{
	InitFont(font);
}
void GDIPlus::SetFontBrush(const wxBrush &fontbrush)
{
	InitBrush(fontbrush, true);
}
void GDIPlus::SetPen(const wxPen &pen, float width)
{
	m_width = width;
	InitPen(pen);
}