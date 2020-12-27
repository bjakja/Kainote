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

#include "KaiStatusBar.h"
#include <wx\dc.h>
#include <wx\dcclient.h>
#include <wx\dcmemory.h>
#include "config.h"

KaiStatusBar::KaiStatusBar(wxWindow *parent, int id, int style)
	:wxWindow(parent, id, wxDefaultPosition, wxSize(-1, 26))
	, bmp(NULL)
{
	Bind(wxEVT_SIZE, &KaiStatusBar::OnSize, this);
	Bind(wxEVT_PAINT, &KaiStatusBar::OnPaint, this);
	Bind(wxEVT_MOTION, &KaiStatusBar::OnMouseMove, this);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent evt){});
	SetMinSize(wxSize(200, 26));
	int x = 0, y = 0;
	GetTextExtent(L"#TWFfGHj", &x, &y);
	y += 8;
	if (y > 26){
		SetMinSize(wxSize(200, y));
		SetSize(wxSize(-1, y));
	}
}

void KaiStatusBar::SetFieldsCount(int num, int *fields)
{
	for (int i = 0; i < num; i++){
		sizes.Add(fields[i]);
	}
	labels.resize(num);
	tips.resize(num);
}

void KaiStatusBar::SetTooltips(wxString *_tips, int count)
{
	for (int i = 0; i < count; i++){
		tips[i] = _tips[i];
	}
}

void KaiStatusBar::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void KaiStatusBar::OnPaint(wxPaintEvent& event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(w, h); }
	tdc.SelectObject(*bmp);

	/*GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext *gc = renderer->CreateContext(tdc);
	if (!gc){*/
		tdc.SetFont(GetFont());
		bool enabled = IsThisEnabled();
		const wxColour & wbg = Options.GetColour(WINDOW_BACKGROUND);
		const wxColour & wfg = Options.GetColour(WINDOW_TEXT);
		const wxColour & border = Options.GetColour(STATUSBAR_BORDER);
		tdc.SetBrush(wxBrush(wbg));
		tdc.SetPen(wxPen(border));
		tdc.DrawRectangle(0, 0, w, h);
		wxArrayInt widths;
		CalcWidths(&widths);
		int posX = 1;
		int widthsSize = widths.size();
		for (size_t i = 0; i < widthsSize; i++){
			if (widths[i] > 0 && i > 0){
				tdc.SetPen(border);
				tdc.DrawLine(posX - 1, 1, posX - 1, h - 1);
			}
			if (labels[i] == L""){ posX += widths[i]; continue; }
			wxColour bg = (background.size() > i && background[i] > 0) ? Options.GetColour(background[i]) : wbg;
			tdc.SetTextForeground((foreground.size() > i && foreground[i] > 0) ? Options.GetColour(foreground[i]) : wfg);
			tdc.SetBrush(bg);
			tdc.SetPen(bg);
			tdc.DrawRectangle(posX, 1, widths[i] - 1, h - 2);
			wxRect cur(posX + 4, 1, (i >= widthsSize - 1) ? widths[i] - 20 : widths[i] - 5, h - 2);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(labels[i], cur, wxALIGN_CENTER_VERTICAL);
			tdc.DestroyClippingRegion();
			posX += widths[i];

		}
		tdc.DrawBitmap(wxBITMAP_PNG(L"gripper"), w - 18, h - 18);
	//}
	//else{
	//	
	//	bool enabled = IsThisEnabled();
	//	const wxColour & wbg = Options.GetColour(WINDOW_BACKGROUND);
	//	const wxColour & wfg = Options.GetColour(WINDOW_TEXT);
	//	const wxColour & border = Options.GetColour(STATUSBAR_BORDER);
	//	gc->SetBrush(wxBrush(wbg));
	//	gc->SetPen(wxPen(border));
	//	gc->DrawRectangle(0, 0, w - 1, h - 1);
	//	wxArrayInt widths;
	//	CalcWidths(&widths);
	//	int posX = 1;
	//	int widthsSize = widths.size();
	//	for (size_t i = 0; i < widthsSize; i++){
	//		if (widths[i] > 0 && i > 0){
	//			gc->SetPen(border);
	//			gc->StrokeLine(posX - 1, 1, posX - 1, h - 1);
	//		}
	//		if (labels[i] == ""){ posX += widths[i]; continue; }
	//		wxColour bg = (background.size() > i && background[i] > 0) ? Options.GetColour(background[i]) : wbg;
	//		gc->SetFont(GetFont(), (foreground.size() > i && foreground[i] > 0) ? Options.GetColour(foreground[i]) : wfg);
	//		gc->SetBrush(bg);
	//		gc->SetPen(bg);
	//		gc->DrawRectangle(posX, 1, widths[i] - 2, h - 3);
	//		wxRect cur(posX + 4, 1, (i >= widthsSize - 1) ? widths[i] - 20 : widths[i] - 5, h - 2);
	//		/*gc->SetClippingRegion(cur);
	//		gc->DrawLabel(labels[i], cur, wxALIGN_CENTER_VERTICAL);
	//		gc->DestroyClippingRegion();*/
	//		gc->Clip(cur);
	//		gc->DrawTextU(labels[i], posX + 4., 4.);
	//		gc->ResetClip();
	//		posX += widths[i];

	//	}
	//	wxBitmap bmp = wxBITMAP_PNG("gripper");
	//	gc->DrawBitmap(bmp, w - 18, h - 18, bmp.GetWidth(), bmp.GetHeight());
	//	delete gc;
	//}
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}

void KaiStatusBar::OnMouseMove(wxMouseEvent &evt)
{
	wxArrayInt widths;
	CalcWidths(&widths);
	int x = evt.GetX();
	int posX = 1;
	for (size_t i = 0; i < widths.size(); i++){
		if (widths[i] > 0 && posX < x && posX + widths[i] > x){

			if (tips[i] != tip){
				tip = (labels[i].IsEmpty()) ? L"" : tips[i];
				SetToolTip(tip);
			}
			break;
		}
		posX += widths[i];

	}
}

void KaiStatusBar::CalcWidths(wxArrayInt *widths)
{
	widths->resize(sizes.size());
	int w, h, wRest = 0, perspective = 0;
	GetClientSize(&w, &h);
	wRest = w;
	for (size_t i = 0; i < sizes.size(); i++){
		if (sizes[i] < 0){ perspective += sizes[i]; continue; }
		if (labels[i] == L"" || sizes[i]){ (*widths)[i] = sizes[i]; wRest -= sizes[i]; continue; }
		wxSize size = GetTextExtent(labels[i]);
		(*widths)[i] = size.x + 10; wRest -= size.x + 10;
	}
	for (size_t i = 0; i < sizes.size(); i++){
		if (sizes[i] >= 0){ continue; }
		(*widths)[i] = ((float)sizes[i] / (float)perspective) * wRest;
	}
}

void KaiStatusBar::SetLabelText(size_t field, const wxString &label)
{
	if (field >= labels.size()){ return; }
	labels[field] = label;
	Refresh(false);
}

wxString KaiStatusBar::GetStatusText(size_t field) const
{
	if (field >= labels.size()){ return L""; }
	return labels[field];
}

void KaiStatusBar::SetLabelTextColour(size_t field, COLOR textColour)
{
	if (field >= foreground.size()){ foreground.resize(field + 1, (COLOR)0); }
	foreground[field] = textColour;
	Refresh(false);
}

void KaiStatusBar::SetLabelBackgroundColour(size_t field, COLOR backgroundColour)
{
	if (field >= background.size()){ background.resize(field + 1, (COLOR)0); }
	background[field] = backgroundColour;
	Refresh(false);
}

bool KaiStatusBar::SetFont(const wxFont &font)
{
	wxWindow::SetFont(font);
	int x = 0, y = 0;
	GetTextExtent(L"#TWFfGHj", &x, &y);
	y += 8;
	if (y != GetClientSize().GetHeight()){
		SetMinSize(wxSize(200, y));
		SetSize(wxSize(-1, y));
	}
	return true;
}