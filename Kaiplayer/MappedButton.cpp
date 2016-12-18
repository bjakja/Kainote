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

//static wxFont font;

wxColour WhiteUp(const wxColour &color)
{
	int r = color.Red() + 45, g = color.Green() + 45, b = color.Blue() + 45;
	r = (r<0xFF)? r : 0xFF;
	g = (g<0xFF)? g : 0xFF;
	b = (b<0xFF)? b : 0xFF;
	return wxColour(r,g,b);
}

//w tooltipach nie nale¿y ustawiaæ () bo zostan¹ usuniête
MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, const wxString& toolTip,
             const wxPoint& pos, const wxSize& size, int window, long style)
			 :wxWindow(parent, id, pos, size, style)
			 ,Window(window)
			 ,twoHotkeys(false)
			 ,bmp(0)
			 ,enter(false)
			 ,clicked(false)
			 ,isColorButton(false)
{
	name = label;
	wxSize newSize=size;
	SetFont(parent->GetFont());
	int fw, fh;
	GetTextExtent((name=="")? "TEXT" : name, &fw, &fh, 0, 0);
	if(size.x <1){
		newSize.x = fw+10;
		if(newSize.x<60){newSize.x=60;}
	}
	if(size.y <1){
		newSize.y = fh+10;
	}
	SetMinSize(newSize);
	//SetBestSize(newSize);
	Bind(wxEVT_LEFT_DOWN, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &MappedButton::OnKeyPress, this);
	if(toolTip!=""){SetToolTip(toolTip);}
	
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, int window,
            const wxPoint& pos, const wxSize& size, long style)
			:wxWindow(parent, id, pos, size, style)
			 ,Window(window)
			 ,twoHotkeys(false)
			 ,bmp(0)
			 ,enter(false)
			 ,clicked(false)
			 ,isColorButton(false)
{
	name = label;
	wxSize newSize=size;
	SetFont(parent->GetFont());
	int fw, fh;
	GetTextExtent((name=="")? "TEXT" : name, &fw, &fh, 0, 0);
	if(size.x <1){
		newSize.x = fw+16;
		if(newSize.x<60){newSize.x=60;}
	}
	if(size.y <1){
		newSize.y = fh+10;
	}
	SetMinSize(newSize);
	//SetBestSize(newSize);
	Bind(wxEVT_LEFT_DOWN, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &MappedButton::OnKeyPress, this);
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxString& tooltip, const wxBitmap& bitmap, const wxPoint& pos,
            const wxSize& size, int window, long style)
			 :wxWindow(parent, id, pos, size, style)
			 ,Window(window)
			 ,twoHotkeys(false)
			 ,bmp(0)
			 ,enter(false)
			 ,clicked(false)
			 ,isColorButton(false)
{
	icon = bitmap;
	wxSize newSize=size;
	if(size.x <1){
		int fw = icon.GetWidth();
		newSize.x = fw+10;
	}
	if(size.y <1){
		int fh = icon.GetHeight();
		newSize.y = fh+10;
	}
	SetMinSize(newSize);
	//SetBestSize(newSize);
	Bind(wxEVT_LEFT_DOWN, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &MappedButton::OnKeyPress, this);
	if(tooltip!=""){SetToolTip(tooltip);}
	SetFont(parent->GetFont());
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
	tdc.SetFont(GetFont());
	/*wxColour background = GetParent()->GetBackgroundColour();
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);*/
	bool enabled = IsThisEnabled();
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour((clicked)? wxSYS_COLOUR_BTNSHADOW : (enabled)? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_INACTIVECAPTION )));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour((enter)? wxSYS_COLOUR_MENUHILIGHT : (enabled)? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_GRAYTEXT)));
	tdc.DrawRectangle(0,0,w,h);
	
	if(w>10){
		int fw, fh;
		if(icon.IsOk()){
			fw=icon.GetWidth(); fh=icon.GetHeight();
			tdc.DrawBitmap((enabled)? icon : icon.ConvertToDisabled(), (w - fw)/2, (h - fh)/2);
		}else if(isColorButton){
			tdc.SetBrush(wxBrush(buttonColor));
			tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW)));
			tdc.DrawRectangle(4,4,w-8,h-8);
		}
		tdc.SetTextForeground((enabled)? GetForegroundColour() : wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
		tdc.GetTextExtent(name, &fw, &fh);
		wxRect cur(5, (h-fh)/2, w - 10, fh);
		tdc.SetClippingRegion(cur);
		tdc.DrawLabel(name,cur,wxALIGN_CENTER);
		tdc.DestroyClippingRegion();
		
		
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
		clicked=false;
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
	if(event.LeftDown() || event.LeftIsDown() && !clicked){
		if(event.LeftDown()){clicked=true;}
		Refresh(false);
		SetFocus();
		//event
	}
	if(event.LeftUp()){
		bool oldclicked = clicked;
		clicked=false;
		Refresh(false);
		if(oldclicked){
			wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
			this->ProcessEvent(evt);
		}
	}
	//event.Skip();
}

void MappedButton::OnKeyPress(wxKeyEvent &event)
{
	if(event.GetKeyCode() == WXK_RETURN){
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		this->ProcessEvent(evt);
	}
}


ToggleButton::ToggleButton(wxWindow *parent, int id, const wxString& label, const wxString& tooltip,
             const wxPoint& pos, const wxSize& size, long style)
			 :wxWindow(parent, id, pos, size, style)
			 ,bmp(0)
			 ,enter(false)
			 ,toggled(false)
			 ,clicked(false)
{
	name = label;
	wxSize newSize=size;
	SetFont(parent->GetFont());
	if(size.x <1){
		int fw, fh;
		GetTextExtent(name, &fw, &fh, 0, 0/*, &font*/);
		newSize.x = fw+10;
		if(newSize.x<80){newSize.x=80;}
	}
	if(size.y <1){
		newSize.y = 26;
	}
	SetMinSize(newSize);
	Bind(wxEVT_LEFT_DOWN, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &ToggleButton::OnSize, this);
	Bind(wxEVT_PAINT, &ToggleButton::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &ToggleButton::OnKeyPress, this);
	if(tooltip!=""){SetToolTip(tooltip);}

}

void ToggleButton::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void ToggleButton::OnPaint(wxPaintEvent& event)
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
	tdc.SetFont(GetFont());
	wxColour background = GetParent()->GetBackgroundColour();
	bool enabled = IsThisEnabled();
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);
	wxColour btnBackground = wxSystemSettings::GetColour((clicked)? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_BTNFACE);
	wxColour btnToggled = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUHILIGHT);
	wxColour frame = wxSystemSettings::GetColour((enter || toggled)? wxSYS_COLOUR_MENUHILIGHT : wxSYS_COLOUR_BTNSHADOW);
	if(toggled){
		int r2 = btnToggled.Red(), g2 = btnToggled.Green(), b2 = btnToggled.Blue();
		int r = btnBackground.Red(), g = btnBackground.Green(), b = btnBackground.Blue();
		int inv_a = 65;
		int fr = (r2* inv_a / 0xFF) + (r - inv_a * r / 0xFF);
		int fg = (g2* inv_a / 0xFF) + (g - inv_a * g / 0xFF);
		int fb = (b2* inv_a / 0xFF) + (b - inv_a * b / 0xFF);
		btnBackground = wxColour(fr,fg,fb);
	}
	if(enter){
		btnBackground = WhiteUp(btnBackground);
		frame = WhiteUp(frame);
	}
	tdc.SetBrush(wxBrush(btnBackground));
	tdc.SetPen(wxPen(frame));
	tdc.DrawRectangle(1,1,w-2,h-2);
	
	if(w>10){
		int fw, fh;
		if(icon.IsOk()){
			fw=icon.GetWidth(); fh=icon.GetHeight();
			tdc.DrawBitmap(icon, (w - fw)/2, (h - fh)/2);
		}else{
			tdc.SetTextForeground((enabled)? GetForegroundColour() : wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
			tdc.GetTextExtent(name, &fw, &fh);
			wxRect cur(5, (h-fh)/2, w - 10, fh);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(name,cur,wxALIGN_CENTER);
			tdc.DestroyClippingRegion();
		}
		
	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void ToggleButton::OnMouseEvent(wxMouseEvent &event)
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
	if(event.LeftDown()){
		clicked=true;
		toggled = !toggled;
		Refresh(false);
		//event
	}
	if(event.LeftUp()){
		clicked=false;
		Refresh(false);
		wxCommandEvent evt(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, GetId());
		this->ProcessEvent(evt);
	}
	//event.Skip();
}

void ToggleButton::OnKeyPress(wxKeyEvent &event)
{
	if(event.GetKeyCode() == WXK_RETURN){
		toggled = !toggled;
		Refresh(false);
		wxCommandEvent evt(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, GetId());
		this->ProcessEvent(evt);
	}
}

wxIMPLEMENT_ABSTRACT_CLASS(MappedButton, wxWindow);