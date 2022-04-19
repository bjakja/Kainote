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

#include "KaiGauge.h"
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/gauge.h>
#include "Config.h"

KaiGauge::KaiGauge(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size, long style)
	: wxWindow(parent, id, pos, size, style)
{
	wxSize newSize = size;
	bool sizeChanged = false;
	if (size.x < 10){
		newSize.x = 100;
		sizeChanged = true;
	}
	if (size.y < 1){
		int fw, fh;
		wxFont parentFont = parent->GetFont();
		GetTextExtent(L"TEX{}", &fw, &fh, nullptr, nullptr, &parentFont);
		newSize.y = fh + 6;
		sizeChanged = true;
	}
	SetMinSize(newSize);

	Bind(wxEVT_PAINT, &KaiGauge::OnPaint, this);
	Bind(wxEVT_SIZE, &KaiGauge::OnSize, this);
}

void KaiGauge::SetValue(int parcent)
{
	numParcent = parcent;
	Refresh(false);
	Update();
}

void KaiGauge::OnPaint(wxPaintEvent &evt)
{
	int w = 0, h = 0;
	GetClientSize(&w, &h);
	if (w < 1 || h < 1){ return; }

	float progress = (float)numParcent / 100.f;
	int progressLength = (w - 2) * progress;

	wxMemoryDC dc;
	wxBitmap bmp(w, h);
	dc.SelectObject(bmp);

	dc.SetPen(wxPen(Options.GetColour(WINDOW_BORDER)));
	dc.SetBrush(wxBrush(Options.GetColour(WINDOW_BACKGROUND)));
	dc.DrawRectangle(0, 0, w, h);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Options.GetColour(WINDOW_TEXT)));
	dc.DrawRectangle(1, 1, progressLength, h - 2);

	wxPaintDC pdc(this);
	pdc.Blit(0, 0, w, h, &dc, 0, 0);

}

void KaiGauge::OnSize(wxSizeEvent &evt)
{
	Refresh(false);
}