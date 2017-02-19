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
#include "Config.h"

//static wxFont font;

wxColour WhiteUp(const wxColour &color)
{
	int r = color.Red() + 45, g = color.Green() + 45, b = color.Blue() + 45;
	r = (r<0xFF)? r : 0xFF;
	g = (g<0xFF)? g : 0xFF;
	b = (b<0xFF)? b : 0xFF;
	return wxColour(r,g,b);
}

wxString AddText(int id)
{
	wxString label;
	if(id == wxID_OK){label = "OK";}
	else if(id==wxID_CANCEL){label=_("Anuluj");}
	else if(id == wxID_APPLY){label = _("Zastosuj");}
	else if(id==wxID_YES){label=_("Tak");}
	else if(id==wxID_NO){label=_("Nie");}
	//else jeszcze pewnie tego w chuj jest;
	return label;
}

//w tooltipach nie nale¿y ustawiaæ () bo zostan¹ usuniête
MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, const wxString& toolTip,
             const wxPoint& pos, const wxSize& size, int window, long style)
			 :wxWindow(parent, id, pos, size, style|wxWANTS_CHARS)
			 ,Window(window)
			 ,twoHotkeys(false)
			 ,bmp(0)
			 ,enter(false)
			 ,clicked(false)
			 ,isColorButton(false)
			 ,changedForeground(false)
{
	name = label;
	if(name.IsEmpty()){name = AddText(id);}
	name.Replace("&","");
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
	Bind(wxEVT_LEFT_DCLICK, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		SendEvent();
	});
	Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	wxAcceleratorEntry centries[1];
	centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GetId());
	wxAcceleratorTable caccel(1, centries);
	SetAcceleratorTable(caccel);
	if(toolTip!=""){SetToolTip(toolTip);}
	//SetForegroundColour(parent->GetForegroundColour());
	
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, int window,
            const wxPoint& pos, const wxSize& size, long style)
			:wxWindow(parent, id, pos, size, style|wxWANTS_CHARS)
			 ,Window(window)
			 ,twoHotkeys(false)
			 ,bmp(0)
			 ,enter(false)
			 ,clicked(false)
			 ,isColorButton(false)
			 ,changedForeground(false)
{
	name = label;
	if(name.IsEmpty()){name = AddText(id);}
	name.Replace("&","");
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
	Bind(wxEVT_LEFT_DCLICK, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		SendEvent();
	});
	Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	wxAcceleratorEntry centries[1];
	centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GetId());
	wxAcceleratorTable caccel(1, centries);
	SetAcceleratorTable(caccel);
	//SetForegroundColour(parent->GetForegroundColour());
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxString& tooltip, const wxBitmap& bitmap, const wxPoint& pos,
            const wxSize& size, int window, long style)
			 :wxWindow(parent, id, pos, size, style|wxWANTS_CHARS)
			 ,Window(window)
			 ,twoHotkeys(false)
			 ,bmp(0)
			 ,enter(false)
			 ,clicked(false)
			 ,isColorButton(false)
			 ,changedForeground(false)
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
	Bind(wxEVT_LEFT_DCLICK, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MappedButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &MappedButton::OnSize, this);
	Bind(wxEVT_PAINT, &MappedButton::OnPaint, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		SendEvent();
	});
	Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	wxAcceleratorEntry centries[1];
	centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GetId());
	wxAcceleratorTable caccel(1, centries);
	SetAcceleratorTable(caccel);
	if(tooltip!=""){SetToolTip(tooltip);}
	SetFont(parent->GetFont());
	//SetForegroundColour(parent->GetForegroundColour());
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
	bool enabled = IsThisEnabled();
	tdc.SetBrush(wxBrush((enter && !clicked)? Options.GetColour("Button Background Hover") :
		(clicked)? Options.GetColour("Button Background Pushed") : 
		(enabled)? Options.GetColour("Button Background") : 
		Options.GetColour("Window Inactive Background")));
	tdc.SetPen(wxPen((enter && !clicked)? Options.GetColour("Button Border Hover") : 
		(clicked)? Options.GetColour("Button Border Pushed") : 
		(enabled)? Options.GetColour("Button Border") : 
		Options.GetColour("Button Inactive Border")));
	tdc.DrawRectangle(0,0,w,h);
	
	if(w>10){
		int fw, fh;
		if(icon.IsOk()){
			fw=icon.GetWidth(); fh=icon.GetHeight();
			tdc.DrawBitmap((enabled)? icon : icon.ConvertToDisabled(), (w - fw)/2, (h - fh)/2);
		}else if(isColorButton){
			tdc.SetBrush(wxBrush(buttonColor));
			tdc.SetPen(wxPen(Options.GetColour("Button Border")));
			tdc.DrawRectangle(4,4,w-8,h-8);
		}
		tdc.SetTextForeground((enabled && changedForeground)? GetForegroundColour() : 
			(enabled)? Options.GetColour("Window Text") : 
			Options.GetColour("Window Inactive Text"));
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
	if(event.LeftDown() || event.LeftDClick()){
		clicked=true;
		Refresh(false);
		SetFocus();
	}
	if(event.LeftUp()){
		bool oldclicked = clicked;
		clicked=false;
		Refresh(false);
		if(oldclicked){
			SendEvent();
		}
	}
	//event.Skip();
}

void MappedButton::SendEvent()
{
	wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	ProcessEvent(evt);
}

void MappedButton::SetLabelText(const wxString &label) 
{
	name = label; 
	int fw, fh;
	GetTextExtent((name=="")? "TEXT" : name, &fw, &fh);
	wxSize minSize = GetMinSize();
	if(minSize.x < fw + 16){
		minSize.x = fw+16;
		SetMinSize(minSize);
		GetParent()->Layout();
		return;
	}
	Refresh(false);
}

bool MappedButton::Enable(bool enable)
{
	wxWindow::Enable(enable);
	Refresh(false);
	//Update();
	return true;
}

ToggleButton::ToggleButton(wxWindow *parent, int id, const wxString& label, const wxString& tooltip,
             const wxPoint& pos, const wxSize& size, long style)
			 :wxWindow(parent, id, pos, size, style|wxWANTS_CHARS)
			 ,bmp(0)
			 ,enter(false)
			 ,toggled(false)
			 ,clicked(false)
			 ,changedForeground(false)
{
	name = label;
	if(name.IsEmpty()){name = AddText(id);}
	name.Replace("&","");
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
	Bind(wxEVT_LEFT_DOWN, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &ToggleButton::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &ToggleButton::OnSize, this);
	Bind(wxEVT_PAINT, &ToggleButton::OnPaint, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		toggled = !toggled;
		Refresh(false);
		SendEvent();
	});
	wxAcceleratorEntry centries[1];
	centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GetId());
	wxAcceleratorTable caccel(1, centries);
	SetAcceleratorTable(caccel);
	if(tooltip!=""){SetToolTip(tooltip);}
	//SetForegroundColour(parent->GetForegroundColour());
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
	tdc.SetBrush(wxBrush((enter && !clicked)? Options.GetColour("Button Background Hover") :
		(toggled && !clicked)? Options.GetColour("Togglebutton Background Toggled") :
		(clicked)? Options.GetColour("Button Background Pushed") : 
		(enabled)? Options.GetColour("Button Background") : 
		Options.GetColour("Window Inactive Background")));
	tdc.SetPen(wxPen((enter && !clicked)? Options.GetColour("Button Border Hover") : 
		(toggled && !clicked)? Options.GetColour("Togglebutton Border Toggled") :
		(clicked)? Options.GetColour("Button Border Pushed") : 
		(enabled)? Options.GetColour("Button Border") : 
		Options.GetColour("Button Inactive Border")));
	
	tdc.DrawRectangle(0,0,w,h);

	
	if(w>10){
		int fw, fh;
		if(icon.IsOk()){
			fw=icon.GetWidth(); fh=icon.GetHeight();
			tdc.DrawBitmap(icon, (w - fw)/2, (h - fh)/2);
		}else{
			tdc.SetTextForeground((enabled && changedForeground)? GetForegroundColour() : 
			(enabled)? Options.GetColour("Window Text") : 
			Options.GetColour("Window Inactive Text"));
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
	if(event.LeftDown()||event.LeftDClick()){
		clicked=true;
		toggled = !toggled;
		Refresh(false);
		//event
	}
	if(event.LeftUp()){
		clicked=false;
		Refresh(false);
		SendEvent();
	}
	//event.Skip();
}

void ToggleButton::SendEvent()
{
	wxCommandEvent evt(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, GetId());
	this->ProcessEvent(evt);
}

bool ToggleButton::Enable(bool enable)
{
	wxWindow::Enable(enable);
	Refresh(false);
	//Update();
	return true;
}

wxIMPLEMENT_ABSTRACT_CLASS(MappedButton, wxWindow);