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

#include "KaiCheckBox.h"
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>

#include "config.h"
//#include "Utils.h"
#include "GraphicsD2D.h"
#undef GetClassInfo
#undef DRAWTEXT

void BlueUp(wxBitmap *bmp)
{
	wxImage img = bmp->ConvertToImage();
	int size = bmp->GetWidth() * bmp->GetHeight() * 3;
	unsigned char *data = img.GetData();

	for (int i = 0; i < size; i++)
	{
		if (i % 3 == 0 && data[i] >= 50){ data[i] -= 50; }
		if (i % 3 == 1 && data[i] >= 20){ data[i] -= 20; }
	}
	*bmp = wxBitmap(img);
}

KaiCheckBox::KaiCheckBox(wxWindow *parent, int id, const wxString& _label,
	const wxPoint& pos, const wxSize& size, long style)
	: wxWindow(parent, id, pos, size, style)
	, enter(false)
	, clicked(false)
	, value(false)
	, isCheckBox(true)
	, foreground(WINDOW_TEXT)
	, background(WINDOW_BACKGROUND)
	, fontHeight(0)
{
	label = _label;
	label.Replace(L"&", emptyString);
	wxSize newSize = size;
	wxWindow::SetFont(parent->GetFont());
	int fullw = 0;
	wxArrayString lines = wxStringTokenize(label, L"\n", wxTOKEN_RET_EMPTY_ALL);
	for (size_t i = 0; i < lines.size(); i++){
		int fw, fh;
		GetTextExtent((lines[i].empty()) ? wxString(L"|") : lines[i], &fw, &fh);
		fontHeight += fh;
		if (fullw < fw){ fullw = fw; }
	}
	if (size.x < 1){
		newSize.x = fullw + 20;
	}
	if (size.y < 1){
		newSize.y = fontHeight + 2;
		if (fontHeight < 17){ newSize.y = 17; }
	}
	SetMinSize(newSize);

	Bind(wxEVT_LEFT_DOWN, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &KaiCheckBox::OnSize, this);
	Bind(wxEVT_PAINT, &KaiCheckBox::OnPaint, this);
	//Bind(wxEVT_KEY_DOWN, &KaiCheckBox::OnKeyPress, this);
	Bind(wxEVT_ERASE_BACKGROUND, &KaiCheckBox::OnEraseBackground, this);
	Bind(wxEVT_KILL_FOCUS, &KaiCheckBox::OnKillFocus, this);
	Bind(wxEVT_SET_FOCUS, &KaiCheckBox::OnSetFocus, this);
	//SetBackgroundColour(parent->GetBackgroundColour());
	//SetForegroundColour(parent->GetForegroundColour());
	/*Bind(wxEVT_SYS_COLOUR_CHANGED, [=](wxSysColourChangedEvent & evt){
		SetForegroundColour(GetParent()->GetForegroundColour());
		SetBackgroundColour(GetParent()->GetBackgroundColour());

		});*/
}

void KaiCheckBox::OnPaint(wxPaintEvent& event)
{

	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }

	bool enabled = IsThisEnabled();
	wxString secondName = (enabled && value) ? wxString(L"_selected") : 
		(enabled) ? emptyString : (value) ? wxString(L"_selected_inactive") : wxString(L"_inactive");
	wxString bitmapName = (isCheckBox) ? wxString(L"checkbox") + secondName : wxString("Lradio") + secondName;
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if (enter){ BlueUp(&checkboxBmp); }

	wxMemoryDC tdc;
	wxBitmap paintBitmap(w, h);
	tdc.SelectObject(paintBitmap);
	/*GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext *gc = renderer->CreateContext(tdc);
	if (!gc){*/
	tdc.SetFont(GetFont());
	const wxColour & background = Options.GetColour(this->background);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0, 0, w, h);
	if(checkboxBmp.IsOk()){
		tdc.DrawBitmap(checkboxBmp, 1, (h - 13) / 2);
	}
	if (HasFocus()) {
		wxPoint frame[5] = { wxPoint(0, 0), wxPoint(w - 1, 0), wxPoint(w - 1, h - 1), wxPoint(0, h - 1), wxPoint(0, 0) };
		DrawDashedLine(&tdc, frame, 5, 1, Options.GetColour(WINDOW_TEXT_INACTIVE));
	}
	if (w > 18){
		tdc.SetTextForeground((enabled) ? Options.GetColour(foreground) : Options.GetColour(WINDOW_TEXT_INACTIVE));
		tdc.DrawText(label, 18, (h - fontHeight) / 2);
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}

void KaiCheckBox::OnMouseEvent(wxMouseEvent &event)
{
	if (event.Entering()){
		enter = true;
		Refresh(false);
		return;
	}
	if (event.Leaving() && enter){
		enter = false;
		clicked = false;
		Refresh(false);
		return;
	}

	if (event.LeftDown() || event.LeftDClick()){
		//if(event.LeftDown()){
		clicked = true;//}
		value = !value;
		wxCommandEvent evt((isCheckBox) ? wxEVT_COMMAND_CHECKBOX_CLICKED : wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
		this->ProcessEvent(evt);
		Refresh(false);
		SetFocus();
		//event
	}
	if (event.LeftUp()){
		//bool oldclicked = clicked;
		clicked = false;
		Refresh(false);
		//if(oldclicked){

		//}
	}
	//event.Skip();
}

void KaiCheckBox::OnKeyPress(wxKeyEvent &event)
{
	/*if(event.GetKeyCode() == WXK_RETURN && GetWindowStyle() & wx){
		value = !value;
		Refresh(false);
		wxCommandEvent evt((isCheckBox)? wxEVT_COMMAND_CHECKBOX_CLICKED : wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
		this->ProcessEvent(evt);
		}*/
}

void KaiCheckBox::OnKillFocus(wxFocusEvent& event)
{
	Refresh(false);
}

void KaiCheckBox::OnSetFocus(wxFocusEvent& event)
{
	Refresh(false);
}

void KaiCheckBox::OnSize(wxSizeEvent& event)
{
	Refresh(false);
	Update();
}

bool KaiCheckBox::Enable(bool enable)
{
	wxWindow::Enable(enable);
	Refresh(false);
	//Update();
	return true;
}

bool KaiCheckBox::SetFont(const wxFont &font)
{
	wxSize newSize = GetMinSize();
	wxWindow::SetFont(font);
	int fullw = 0;
	fontHeight = 0;
	wxArrayString lines = wxStringTokenize(label, L"\n", wxTOKEN_RET_EMPTY_ALL);
	for (size_t i = 0; i < lines.size(); i++){
		int fw, fh;
		GetTextExtent((lines[i].empty()) ? wxString(L"|") : lines[i], &fw, &fh);
		fontHeight += fh;
		if (fullw < fw){ fullw = fw; }
	}
	newSize.x = fullw + 20;
	newSize.y = fontHeight + 2;
	if (fontHeight < 17){ newSize.y = 17; }

	SetMinSize(newSize);
	Refresh(false);
	return true;
}
wxIMPLEMENT_ABSTRACT_CLASS(KaiCheckBox, wxWindow);