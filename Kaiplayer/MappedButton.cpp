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
#include "wx/dcmemory.h"
#include "wx/dcclient.h"
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

//w tooltipach nie należy ustawiać () bo zostaną usunięte
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
	int fw;
	CalculateSize(&fw, &textHeight);
	if(size.x <1){
		newSize.x = fw+10;
		//if(newSize.x<60){newSize.x=60;}
	}
	if(size.y <1){
		newSize.y = textHeight + 10;
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
	Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent &evt){
		if(evt.GetKeyCode()==WXK_RETURN){
			SendEvent();
		}
	});
	//Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	Bind(wxEVT_KILL_FOCUS,[=](wxFocusEvent &evt){Refresh(false);});
	Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent &evt){Refresh(false); });
	//wxAcceleratorEntry centries[1];
	//centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GetId());
	//wxAcceleratorTable caccel(1, centries);
	//SetAcceleratorTable(caccel);
	if(toolTip!=""){SetToolTip(toolTip);}
	//SetForegroundColour(parent->GetForegroundColour());
	
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, int window,
			const wxPoint& pos, const wxSize& size, long style)
			:wxWindow(parent, id, pos, size, style/*|wxWANTS_CHARS*/)
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
	int fw;
	CalculateSize(&fw, &textHeight);
	if(size.x <1){
		newSize.x = fw+16;
		//if(newSize.x<60){newSize.x=60;}
	}
	if(size.y <1){
		newSize.y = textHeight + 10;
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
	/*Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent &evt){
		if(evt.GetKeyCode()==WXK_RETURN){
			SendEvent();
		}
	});*/
	//Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	Bind(wxEVT_KILL_FOCUS,[=](wxFocusEvent &evt){Refresh(false);});
	Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent &evt){Refresh(false); });
	wxAcceleratorEntry centries[1];
	centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GetId());
	wxAcceleratorTable caccel(1, centries);
	SetAcceleratorTable(caccel);
	if (Window < 0){
		Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
			SendEvent();
		}, GetId());
	}
	//SetForegroundColour(parent->GetForegroundColour());
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxString& tooltip, const wxBitmap& bitmap, const wxPoint& pos,
			const wxSize& size, int window, long style, const wxString &text)
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
	int fw=0;
	if(text!=""){
		name = text;
		CalculateSize(&fw, &textHeight);
		fw+=15;
	}
	if(size.x <1){
		fw += icon.GetWidth();
		newSize.x = fw+10;
	}
	if(size.y <1){
		textHeight = (textHeight > icon.GetHeight()) ? textHeight : icon.GetHeight();
		newSize.y = textHeight + 10;
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
	Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent &evt){
		if(evt.GetKeyCode()==WXK_RETURN){
			SendEvent();
		}
	});
	//Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	Bind(wxEVT_KILL_FOCUS,[=](wxFocusEvent &evt){Refresh(false);});
	Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent &evt){Refresh(false); });
	//wxAcceleratorEntry centries[1];
	//centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GetId());
	//wxAcceleratorTable caccel(1, centries);
	//SetAcceleratorTable(caccel);
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
	if(Window>=0){
		wxString toolTip = (_toolTip == "") ? GetToolTipText().BeforeFirst('(').BeforeFirst('\n').Trim() : _toolTip;
		wxString desc = name;
		if (toolTip.empty()){ toolTip = desc; }
		if (desc.empty()){ desc = toolTip; }
	
		idAndType itype(GetId(), Window);
		wxString key = Hkeys.GetStringHotkey(itype, desc);
		if(twoHotkeys){
			idAndType itype(GetId()-1000, Window);
			key += _(" lub ") + Hkeys.GetStringHotkey(itype);
		}

	
		if(key!="")
		{
			toolTip = toolTip + " ("+key+")";
		}
		toolTip << L"\n";
		toolTip << _("Skrót można ustawić Shift + Klik");
		if (twoHotkeys){
			toolTip << L"\n";
			toolTip << _("Drugi skrót można ustawić Control + Klik");
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
	tdc.SetBrush(wxBrush((enter && !clicked)? Options.GetColour(ButtonBackgroundHover) :
		(clicked)? Options.GetColour(ButtonBackgroundPushed) : 
		(HasFocus())? Options.GetColour(ButtonBackgroundOnFocus) : 
		(enabled)? Options.GetColour(ButtonBackground) : 
		Options.GetColour(WindowBackgroundInactive)));
	tdc.SetPen(wxPen((enter && !clicked)? Options.GetColour(ButtonBorderHover) : 
		(clicked)? Options.GetColour(ButtonBorderPushed) :
		(HasFocus())? Options.GetColour(ButtonBorderOnFocus) : 
		(enabled)? Options.GetColour(ButtonBorder) : 
		Options.GetColour(ButtonBorderInactive)));
	tdc.DrawRectangle(0,0,w,h);
	
	if(w>10){
		int fw, fh, iw = 0;
		tdc.GetTextExtent(name, &fw, &fh);
		if(icon.IsOk()){
			iw = icon.GetWidth();
			if(name != ""){
				fw += iw+5;
			}else{
				fw = iw; 
			}
			tdc.DrawBitmap((enabled)? icon : icon.ConvertToDisabled(), (w - fw)/2, (h - icon.GetHeight())/2);
		}else if(isColorButton){
			tdc.SetBrush(wxBrush(buttonColor));
			tdc.SetPen(wxPen(Options.GetColour(ButtonBorder)));
			tdc.DrawRectangle(4,4,w-8,h-8);
		}
		tdc.SetTextForeground((enabled && changedForeground)? GetForegroundColour() : 
			(enabled)? Options.GetColour(WindowText) : 
			Options.GetColour(WindowTextInactive));
		if(name != ""){
			if(iw){
				tdc.DrawText(name, ((w - fw) / 2) + iw + 5, ((h - textHeight) / 2));
			}else{
				wxRect cur(5, ((h - textHeight) / 2), w - 10, textHeight);
				tdc.SetClippingRegion(cur);
				tdc.DrawLabel(name,cur, iw? wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL : wxALIGN_CENTER);
				tdc.DestroyClippingRegion();
			}
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
		clicked=false;
		Refresh(false);
		return;
	}
	if(Window>=0 && event.LeftDown() && (event.ShiftDown() || (twoHotkeys && event.ControlDown()))){
		//upewnij się, że da się zmienić idy na nazwy, 
		//może i trochę spowolni operację ale skończy się ciągłe wywalanie hotkeysów
		//może od razu funkcji onmaphotkey przekazać item by zrobiła co trzeba
		int id= GetId(); 
		if(event.ControlDown()){ id -= 1000; }
		Hkeys.OnMapHkey( GetId(), "", this, Window, false);
		SetToolTip();
		SetFocus();
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
	wxCommandEvent evt((Window>=0)? wxEVT_COMMAND_MENU_SELECTED : wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	ProcessEvent(evt);
}

void MappedButton::CalculateSize(int *w, int *h)
{
	int fw, fh;
	int resultw = 0, resulth = 0;
	wxArrayString namewraps = wxStringTokenize(name, "\n", wxTOKEN_RET_EMPTY_ALL);
	for (auto &token : namewraps){
		GetTextExtent((token == "") ? "TEXT" : token, &fw, &fh);
		resulth += fh;
		if (resultw < fw)
			resultw = fw;
	}
	if (!resultw || !resulth)
		GetTextExtent(L"TEXT", &resultw, &resulth);

	if (w)
		*w = resultw;
	if (h)
		*h = resulth;
}

void MappedButton::SetLabelText(const wxString &label) 
{
	name = label; 
	int fw;
	CalculateSize(&fw, &textHeight);
	wxSize minSize = GetMinSize();
	if (minSize.x != fw + 16 || minSize.y != (textHeight + 10)){
		minSize.x = fw+16;
		minSize.y = textHeight + 10;
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
	int fw;
	CalculateSize(&fw, &textHeight);
	if(size.x <1){
		newSize.x = fw+10;
		if(newSize.x<60){newSize.x=60;}
	}
	if(size.y <1){
		newSize.y = textHeight + 10;
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
	tdc.SetBrush(wxBrush((enter && !clicked)? Options.GetColour(ButtonBackgroundHover) :
		(toggled && !clicked)? Options.GetColour(TogglebuttonBackgroundToggled) :
		(clicked)? Options.GetColour(ButtonBackgroundPushed) : 
		(enabled)? Options.GetColour(ButtonBackground) : 
		Options.GetColour(WindowBackgroundInactive)));
	tdc.SetPen(wxPen((enter && !clicked)? Options.GetColour(ButtonBorderHover) : 
		(toggled && !clicked)? Options.GetColour(TogglebuttonBorderToggled) :
		(clicked)? Options.GetColour(ButtonBorderPushed) : 
		(enabled)? Options.GetColour(ButtonBorder) : 
		Options.GetColour(ButtonBorderInactive)));
	
	tdc.DrawRectangle(0,0,w,h);

	
	if(w>10){
		int fw, fh;
		if(icon.IsOk()){
			fw=icon.GetWidth(); fh=icon.GetHeight();
			tdc.DrawBitmap(icon, (w - fw)/2, (h - fh)/2);
		}else{
			tdc.SetTextForeground((enabled && changedForeground)? GetForegroundColour() : 
			(enabled)? Options.GetColour(WindowText) : 
			Options.GetColour(WindowTextInactive));
			//tdc.GetTextExtent(name, &fw, &fh);
			wxRect cur(5, (h - textHeight) / 2, w - 10, textHeight);
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

void ToggleButton::CalculateSize(int *w, int *h)
{
	int fw, fh;
	int resultw = 0, resulth = 0;
	wxArrayString namewraps = wxStringTokenize(name, "\n", wxTOKEN_RET_EMPTY_ALL);
	for (auto &token : namewraps){
		GetTextExtent((token == "") ? "TEXT" : token, &fw, &fh);
		resulth += fh;
		if (resultw < fw)
			resultw = fw;
	}
	if (w)
		*w = resultw;
	if (h)
		*h = resulth;
}

wxIMPLEMENT_ABSTRACT_CLASS(MappedButton, wxWindow);