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


#include "MappedButton.h"

static const wxFont font = wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");

//w tooltipach nie nale¿y ustawiaæ () bo zostan¹ usuniête
MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, const wxString& toolTip,
             const wxPoint& pos, const wxSize& size, int window, long style)
			 :wxWindow(parent, id, pos, size, style)
			 ,Window(window)
			 ,twoHotkeys(false)
			 ,bmp(0)
			 ,enter(false)
			 ,clicked(false)
{
	name = label;
	bool changeSize=false;
	wxSize newSize=size;
	if(size.x <1){
		int fw, fh;
		GetTextExtent(name, &fw, &fh, 0, 0, &font);
		newSize.x = fw+10;
		if(newSize.x<80){newSize.x=80;}
		changeSize=true;
	}
	if(size.y <1){
		newSize.y = 26;
		changeSize=true;
	}
	if(changeSize){SetMinSize(newSize);}
	Bind(wxEVT_LEFT_DOWN, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &MappedButton::OnKeyPress, this);
	SetToolTip(toolTip);
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxBitmap& bitmap, const wxPoint& pos,
            const wxSize& size, int window, long style)
			:wxWindow(parent, id, pos, size, style)
			,Window(window)
			,twoHotkeys(false)
			,bmp(0)
			,enter(false)
			,clicked(false)
{
	bool changeSize=false;
	wxSize newSize=size;
	if(size.x <1){
		int fw, fh;
		GetTextExtent(name, &fw, &fh, 0, 0, &font);
		newSize.x = bitmap.GetWidth()+10;
		changeSize=true;
	}
	if(size.y <1){
		newSize.y = bitmap.GetHeight()+10;
		changeSize=true;
	}
	if(changeSize){SetMinSize(newSize);}
	Bind(wxEVT_LEFT_DOWN, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &MappedButton::OnKeyPress, this);
	icon = bitmap;
}

MappedButton::~MappedButton()
{
	if(bmp){delete bmp;}
}

void MappedButton::SetToolTip(const wxString &_toolTip)
{
	if(Window){
		wxString toolTip = (_toolTip=="")? GetToolTipText().BeforeFirst('(').Trim() : _toolTip;
		wxString desc = name;
		if(toolTip.empty()){toolTip=desc;}
		if(desc.empty()){desc=toolTip;}
	
		idAndType itype(GetId(), Window);
		wxString key = Hkeys.GetMenuH(itype, desc);
		if(twoHotkeys){
			idAndType itype(GetId()-1000, Window);
			key += _(" lub ") + Hkeys.GetMenuH(itype);
		}

	
		if(key!="")
		{
			toolTip = toolTip + " ("+key+")";
		}
		wxWindow::SetToolTip(toolTip);
	}else{
		wxWindow::SetToolTip(_toolTip);
	}
}

void MappedButton::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void MappedButton::OnPaint(wxPaintEvent& event)
{
	
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	tdc.SetFont(font);
	wxColour background = GetParent()->GetBackgroundColour();
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour((clicked)? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_BTNFACE)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour((enter)? wxSYS_COLOUR_MENUHILIGHT : wxSYS_COLOUR_BTNSHADOW)));
	tdc.DrawRectangle(1,1,w-2,h-2);
	
	if(w>10){
		int fw, fh;
		if(icon.IsOk()){
			fw=icon.GetWidth(); fh=icon.GetHeight();
			tdc.DrawBitmap(icon, (w - fw)/2, (h - fh)/2);
		}else{
			tdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
			tdc.GetTextExtent(name, &fw, &fh, 0, 0, &font);
			wxRect cur(5, (h-fh)/2, w - 10, fh);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(name,cur,wxALIGN_CENTER);
			tdc.DestroyClippingRegion();
		}
		
	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void MappedButton::OnMouseEvent(wxMouseEvent &event)
{
	if(event.Entering()){
		enter=true;
		Refresh(false);
		return;
	}
	if(event.Leaving()&&enter){
		enter=false;
		Refresh(false);
		return;
	}
	if(Window && event.LeftDown() && (event.ShiftDown() || (twoHotkeys && event.ControlDown()))){
		//upewnij siê, ¿e da siê zmieniæ idy na nazwy, 
		//mo¿e i trochê spowolni operacjê ale skoñczy siê ci¹g³e wywalanie hotkeysów
		//mo¿e od razu funkcji onmaphotkey przekazaæ item by zrobi³a co trzeba
		int id= GetId(); 
		if(event.ControlDown()){ id -= 1000; }
		wxString buttonName = (name!="")? name : GetToolTipText().BeforeFirst('(').Trim();
		Hkeys.OnMapHkey( GetId(), buttonName, this, Window, false);
		SetToolTip();
		Hkeys.SetAccels(true);
		Hkeys.SaveHkeys();
		
		return;
	}		
	if(event.LeftDown()){
		clicked=true;
		Refresh(false);
		//event
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		this->ProcessEvent(evt);
	}
	if(event.LeftUp()){
		clicked=false;
		Refresh(false);
		
	}
	event.Skip();
}

void MappedButton::OnKeyPress(wxKeyEvent &event)
{
	if(event.GetKeyCode() == WXK_RETURN){
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		this->ProcessEvent(evt);
	}
}