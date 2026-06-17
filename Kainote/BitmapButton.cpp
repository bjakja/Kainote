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

#include "BitmapButton.h"
#include "config.h"
#include "Hotkeys.h"

BitmapButton::BitmapButton(wxWindow* parent, wxBitmap bitmap, wxBitmap bitmap1, int hkeyId, const wxString &tooltip, const wxPoint& pos, const wxSize& size, int _window)
	: wxStaticBitmap(parent, hkeyId + 300, bitmap, pos, size)
	, window(_window)
	, hotkeyId(hkeyId)
{
	enter = false;
	bmp = bitmap;
	bmp1 = bitmap1;
	Bind(wxEVT_LEFT_DOWN, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_LEAVE_WINDOW, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_ENTER_WINDOW, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
	SetToolTip(tooltip);
}

BitmapButton::~BitmapButton()
{
}


void BitmapButton::ChangeBitmap(bool play)
{
	wxString fbmp = (play) ? L"play" : L"pause";
	wxBitmap newBmp = CreateBitmapFromPngResource(fbmp);
	wxBitmap newBmp1 = CreateBitmapFromPngResource(fbmp + L"1");
	if (newBmp.IsOk())
		bmp = newBmp;
	if (newBmp1.IsOk())
		bmp1 = newBmp1;
	if (enter && bmp.IsOk()){
		img = bmp.ConvertToImage();
		if (!img.IsOk()) {
			SetBitmap(bmp);
			return;
		}
		int size = bmp.GetWidth()*bmp.GetHeight() * 3;
		byte *data = img.GetData();
		for (int i = 0; i < size; i++)
		{
			if (data[i] < 226){ data[i] += 30; }
		}
		SetBitmap(wxBitmap(img));
	}
	else if (bmp.IsOk()){
		SetBitmap(bmp);
	}
}


void BitmapButton::OnLeftDown(wxMouseEvent& event)
{
	if (event.Entering()){
		enter = true;
		if (!bmp.IsOk())
			return;
		img = bmp.ConvertToImage();
		if (!img.IsOk())
			return;
		int size = bmp.GetWidth()*bmp.GetHeight() * 3;
		byte *data = img.GetData();

		for (int i = 0; i < size; i++)
		{
			if (data[i] < 226){ data[i] += 30; }
		}
		SetBitmap(wxBitmap(img));

		return;
	}
	if (event.Leaving() && enter){
		enter = false;
		SetBitmap(bmp);
		return;
	}

	if (event.LeftDown()){
		if (event.ShiftDown()){
			//wxString buttonName = (name!="")? name : GetToolTipText().BeforeFirst('(').Trim();
			Hkeys.OnMapHkey(hotkeyId, emptyString, this, window);
			SetToolTip();
			//Hkeys.SetAccels(true);
			//Hkeys.SaveHkeys();
			SetFocus();
			return;

		}
		if (bmp1.IsOk())
			SetBitmap(bmp1);
	}
	if (event.LeftUp()){

		if (img.IsOk())
			SetBitmap(wxBitmap(img));
		else if (bmp.IsOk())
			SetBitmap(bmp);
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, hotkeyId); this->ProcessEvent(evt);
	}
}

void BitmapButton::SetToolTip(const wxString &_toolTip)
{
	wxString toolTip = (_toolTip == emptyString) ? name : _toolTip;
	if (!_toolTip.empty()){ name = _toolTip; }

	idAndType itype(hotkeyId, window);
	wxString key = Hkeys.GetStringHotkey(itype);

	if (key != emptyString)
	{
		toolTip = toolTip + L" (" + key + L")";
	}
	toolTip << L"\n";
	toolTip << _("Skrót można ustawić Shift + Klik");
	wxWindow::SetToolTip(toolTip);

}