//  Copyright (c) 2016 - 2026, Marcin Drob

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

#include "KaiStaticBoxSizer.h"
#include "config.h"
#include <wx/statbox.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>

#include <windows.h>
#include <wx/msw/winundef.h>

KaiStaticBox::KaiStaticBox(wxWindow *parent, const wxString& _label)
	:
#ifdef _WIN32
	wxStaticBox(parent, -1, _label)
#else
	wxWindow(parent, -1)
#endif
{
	labels.Add(_label);
#ifdef _WIN32
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
#else
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	Bind(wxEVT_PAINT, [this](wxPaintEvent &evt){
		wxPaintDC dc(this);
		int w, h;
		GetClientSize(&w, &h);
		tagRECT rc = { 0, 0, w, h };
		PaintForeground(dc, rc);
	});
#endif
	SetFont(parent->GetFont());
	wxSize fsize = GetTextExtent(_label);
	SetInitialSize(wxSize(fsize.x + 16, fsize.y + 10));
	heightText = fsize.y;
}

//empty table = crash
KaiStaticBox::KaiStaticBox(wxWindow *parent, int numLabels, wxString * _labels)
	:
#ifdef _WIN32
	wxStaticBox(parent, -1, _labels[0])
#else
	wxWindow(parent, -1)
#endif
{
	for (int i = 0; i < numLabels; i++){
		labels.Add(_labels[i]);
	}
	
#ifdef _WIN32
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
#else
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	Bind(wxEVT_PAINT, [this](wxPaintEvent &evt){
		wxPaintDC dc(this);
		int w, h;
		GetClientSize(&w, &h);
		tagRECT rc = { 0, 0, w, h };
		PaintForeground(dc, rc);
	});
#endif
	SetFont(parent->GetFont());
	int fw, fh, maxfw = 0, maxfh = 0;
	for (auto &label : labels){
		 GetTextExtent(label, &fw, &fh);
		 if (fw > maxfw)
			 maxfw = fw;
		 if (fh > maxfh)
			 maxfh = fh;
	}
	SetInitialSize(wxSize(maxfw + 16, maxfh + 10));
	heightText = maxfh;
}


//void KaiStaticBox::OnSize(wxSizeEvent& event)
//{
//	Refresh(false);
//}
	
void KaiStaticBox::PaintForeground(wxDC& tdc, const tagRECT& rc)
{
	int w = 0;
	int h = 0;
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;
	if (w == 0 || h == 0 || !IsShown() || !IsShownOnScreen()){ return; }

	wxColour background = GetParent()->GetBackgroundColour();
	tdc.SetFont(GetFont());
	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	wxSize fsize = tdc.GetTextExtent(labels[0]);
	int halfY = fsize.y / 2;
#ifdef _WIN32
	tdc.SetPen(wxPen(Options.GetColour(STATICBOX_BORDER)));
	tdc.DrawRectangle(4, halfY, w - 8, h - halfY - 2);
	tdc.SetBackgroundMode(wxSOLID);
	tdc.SetTextBackground(background);
	int posx = 8;
	int cellWidth = (w - 16) / labels.size();
#else
	// On wxGTK these labels are help text; make the helper box layout-only.
	return;
#endif
#ifdef _WIN32
	tdc.SetTextForeground(GetParent()->GetForegroundColour());
	for (int i = 0; i < labels.GetCount(); i++){
		wxString text = wxString(L" ") + labels[i] + wxString(L" ");
		int fw, fh;
		tdc.GetTextExtent(text, &fw, &fh);
		int wdiff = MAX(fw - cellWidth, 0);
		int labelWidth = wxMin(fw, w - posx - 8);
		if (labelWidth > 0){
			int clearX = wxMax(0, posx - 2);
			int clearWidth = wxMin(labelWidth + 4, w - clearX - 6);
			// Clear the caption strip before drawing over the custom top border.
			wxPen oldPen = tdc.GetPen();
			wxBrush oldBrush = tdc.GetBrush();
			tdc.SetPen(wxPen(background));
			tdc.SetBrush(wxBrush(background));
			tdc.DrawRectangle(clearX, 0, clearWidth, fh + 2);
			tdc.SetPen(oldPen);
			tdc.SetBrush(oldBrush);
			tdc.SetClippingRegion(posx, 0, labelWidth, fh + 2);
			tdc.DrawText(text, posx, 0);
			tdc.DestroyClippingRegion();
		}
		posx += cellWidth + wdiff;
	}
#endif
}

wxSize KaiStaticBox::CalcBorders()
{
#ifdef _WIN32
	return wxSize(8, heightText + 5);
#else
	return wxSize(6, 6);
#endif
}

bool KaiStaticBox::Enable(bool enable)
{
	bool succ = wxWindowBase::Enable(enable);
	Refresh(false);
	return succ;
}

KaiStaticBoxSizer::KaiStaticBoxSizer(int orient, wxWindow *parent, const wxString& _label)
	: wxBoxSizer(orient)
	, box(new KaiStaticBox(parent, _label))
{
	box->SetContainingSizer(this);
}

KaiStaticBoxSizer::KaiStaticBoxSizer(int orient, wxWindow *parent, int n, wxString *labels)
	: wxBoxSizer(orient)
	, box(new KaiStaticBox(parent, n, labels))
{
	box->SetContainingSizer(this);
}

KaiStaticBoxSizer::~KaiStaticBoxSizer(){
	if (box){ delete box; box = nullptr; }
};

void KaiStaticBoxSizer::RecalcSizes()
{
	wxSize borders = box->CalcBorders();

	box->SetSize(m_position.x, m_position.y, m_size.x, m_size.y);
#ifndef _WIN32
	// wxWindow-based static boxes on GTK need an explicit refresh after sizer
	// recalculation; otherwise some group outlines may keep the old/blank paint.
	box->Refresh(false);
#endif
	wxSize old_size(m_size);
	m_size.x -= 2 * borders.x;
	m_size.y -= borders.y + borders.x;
	wxPoint old_pos(m_position);
	m_position.x += borders.x;
	m_position.y += borders.y;

	wxBoxSizer::RecalcSizes();

	m_position = old_pos;
	m_size = old_size;
}

wxSize KaiStaticBoxSizer::CalcMin()
{
	wxSize borders = box->CalcBorders();

	wxSize ret(wxBoxSizer::CalcMin());
	ret.x += 2 * borders.x;

#ifdef _WIN32
	// ensure that we're wide enough to show the static box label (there is no
	// need to check for the static box best size in vertical direction though)
	const int boxWidth = box->GetBestSize().x;
	if (ret.x < boxWidth)
		ret.x = boxWidth;
#endif

	ret.y += borders.x + borders.y;

	return ret;
}

void KaiStaticBoxSizer::ShowItems(bool show)
{
	box->Show(show);
	wxBoxSizer::ShowItems(show);
}

bool KaiStaticBoxSizer::Detach(wxWindow *window)
{
	if (window == box)
	{
		box = nullptr;
		return true;
	}

	return wxSizer::Detach(window);
}

bool KaiStaticBoxSizer::Enable(bool enable)
{
	return box->Enable(enable);
}
