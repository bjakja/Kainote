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


#include "KaiCheckBox.h"
#include "config.h"

void BlueUp(wxBitmap *bmp)
{
	wxImage img=bmp->ConvertToImage();
	int size=bmp->GetWidth()*bmp->GetHeight()*3;
	byte *data=img.GetData();
			
	for(int i=0; i<size; i++)
	{
		if(i % 3 == 0 && data[i]>=50){data[i]-=50;}
		if(i % 3 == 1 && data[i]>=20){data[i]-=20;}
	}
	*bmp = wxBitmap(img);
}

KaiCheckBox::KaiCheckBox(wxWindow *parent, int id, const wxString& _label,
            const wxPoint& pos, const wxSize& size, long style)
			:wxWindow(parent, id, pos, size, style)
			 ,enter(false)
			 ,clicked(false)
			 ,value(false)
			 ,isCheckBox(true)
			 ,changedBackground(false)
			 ,changedForeground(false)
			 ,fontHeight(0)
{
	label = _label;
	wxSize newSize=size;
	SetFont(parent->GetFont());
	int fullw=0;
	wxArrayString lines = wxStringTokenize(label, "\n",wxTOKEN_RET_EMPTY_ALL);
	for(size_t i=0; i < lines.size(); i++){
		int fw, fh;
		GetTextExtent((lines[i]=="")? "|" : lines[i], &fw, &fh);
		fontHeight += fh;
		if(fullw < fw){fullw = fw;}
	}
	if(size.x <1){
		newSize.x = fullw+20;
		//wxLogStatus("text %i "+ label, newSize.x);
	}
	if(size.y <1){
		newSize.y = fontHeight+2;
		if(fontHeight<17){newSize.y=17;}
	}
	SetMinSize(newSize);
	
	Bind(wxEVT_LEFT_DOWN, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiCheckBox::OnMouseEvent, this);
	Bind(wxEVT_SIZE, &KaiCheckBox::OnSize, this);
	Bind(wxEVT_PAINT, &KaiCheckBox::OnPaint, this);
	Bind(wxEVT_KEY_DOWN, &KaiCheckBox::OnKeyPress, this);
	Bind(wxEVT_ERASE_BACKGROUND, &KaiCheckBox::OnEraseBackground, this);
	//SetBackgroundColour(parent->GetBackgroundColour());
	//SetForegroundColour(parent->GetForegroundColour());
	/*Bind(wxEVT_SYS_COLOUR_CHANGED, [=](wxSysColourChangedEvent & evt){
		SetForegroundColour(GetParent()->GetForegroundColour());
		SetBackgroundColour(GetParent()->GetBackgroundColour());

	});*/
}

void KaiCheckBox::OnPaint(wxPaintEvent& event)
{
	
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	tdc.SelectObject(wxBitmap(w,h));
	tdc.SetFont(GetFont());
	wxColour background = (changedBackground)? GetBackgroundColour() : Options.GetColour("Window Background");
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);
	bool enabled = IsThisEnabled();

	//wxString bitmapName = (enabled && value)? "radio_selected" : (enabled)? "radio" : (value)? "radio_selected_inactive" : "radio_inactive";
	wxString secondName = (enabled && value)? "_selected" : (enabled)? "" : (value)? "_selected_inactive" : "_inactive";
	wxString bitmapName = (isCheckBox)? "checkbox" + secondName : "radio" + secondName;
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if(enter){BlueUp(&checkboxBmp);}
	tdc.DrawBitmap(checkboxBmp, 1, (h-13)/2);

	if(w>18){
		//if(!enabled){
			//tdc.SetTextForeground("#000000");
			/*wxRect cur(18, ((h-fh)/2)+1, w - 20, fh-1);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(label,cur,wxALIGN_LEFT);
			tdc.DestroyClippingRegion();*/
			//tdc.DrawText(label,18, ((h-fontHeight)/2)+1);
		//}
		tdc.SetTextForeground((enabled && changedForeground)? GetForegroundColour() : 
			(enabled)? Options.GetColour("Window Text") : Options.GetColour("Window Inactive Text"));
		//wxRect cur(18, (h-fh)/2, w - 20, fh);
		//tdc.SetClippingRegion(cur);
		tdc.DrawText(label,18, (h-fontHeight)/2);
		//tdc.DestroyClippingRegion();
		
		
	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiCheckBox::OnMouseEvent(wxMouseEvent &event)
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
	
	if(event.LeftDown() || event.LeftDClick()){
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
			value = !value;
			wxCommandEvent evt((isCheckBox)? wxEVT_COMMAND_CHECKBOX_CLICKED : wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
			this->ProcessEvent(evt);
		}
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

void KaiCheckBox::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiCheckBox, wxWindow);