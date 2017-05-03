//  Copyright (c) 2016, Marcin Drob

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
#include "Config.h"
#include "Hotkeys.h"

BitmapButton::BitmapButton(wxWindow* parent, wxBitmap bitmap, wxBitmap bitmap1, int hkeyId, const wxString &tooltip, const wxPoint& pos, const wxSize& size, int _window)
	:  wxStaticBitmap(parent, -1, bitmap,pos,size)
	, window(_window)
	, hotkeyId(hkeyId)
{
	enter=false;
	bmp=bitmap;
	bmp1=bitmap1;
	Bind(wxEVT_LEFT_DOWN, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_LEAVE_WINDOW, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_ENTER_WINDOW, &BitmapButton::OnLeftDown, this);
	SetToolTip(tooltip);
}
    
BitmapButton::~BitmapButton()
{
}


void BitmapButton::ChangeBitmap(bool play)
{
	wxString fbmp=(play)? "play" : "pause";
	bmp = CreateBitmapFromPngResource(fbmp);
	bmp1 = CreateBitmapFromPngResource(fbmp+"1");
	if(enter){
		img=bmp.ConvertToImage();
		int size=bmp.GetWidth()*bmp.GetHeight()*3;
		byte *data=img.GetData();
		for(int i=0; i<size; i++)
		{
			if(data[i]<226){data[i]+=30;}
		}
		SetBitmap(wxBitmap(img));
	}else{
		SetBitmap(bmp);
	}
}


void BitmapButton::OnLeftDown(wxMouseEvent& event)
{
	if(event.Entering()){
		enter=true;
		img=bmp.ConvertToImage();
		int size=bmp.GetWidth()*bmp.GetHeight()*3;
		byte *data=img.GetData();
			
		for(int i=0; i<size; i++)
		{
			if(data[i]<226){data[i]+=30;}
		}
		SetBitmap(wxBitmap(img));
		
		return;
	}
	if(event.Leaving() && enter){
		enter=false;
		SetBitmap(bmp);
		return;
	}
			
	if(event.LeftDown()){
		if(event.ShiftDown()){
			wxString buttonName = (name!="")? name : GetToolTipText().BeforeFirst('(').Trim();
			Hkeys.OnMapHkey( hotkeyId, buttonName, this, window);
			SetToolTip();
			Hkeys.SetAccels(true);
			Hkeys.SaveHkeys();
			SetFocus();
			return;

		}
		SetBitmap(bmp1);
	}
	if(event.LeftUp()){
		
		SetBitmap(wxBitmap(img));
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,hotkeyId);this->ProcessEvent(evt);
	}
}
	
void BitmapButton::SetToolTip(const wxString &_toolTip)
{
	wxString toolTip = (_toolTip=="")? GetToolTipText().BeforeFirst('(').Trim() : _toolTip;
	wxString desc = name;
	if(toolTip.empty()){toolTip=desc;}
	if(desc.empty()){desc=toolTip;}
	
	idAndType itype(hotkeyId, window);
	wxString key = Hkeys.GetMenuH(itype, desc);
	
	if(key!="")
	{
		toolTip = toolTip + " ("+key+")";
	}
	wxWindow::SetToolTip(toolTip);
	
}