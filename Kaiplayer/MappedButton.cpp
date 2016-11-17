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


//w tooltipach nie nale¿y ustawiaæ () bo zostan¹ usuniête
MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, const wxString& toolTip,
             const wxPoint& pos, const wxSize& size, int window, long style)
			 :wxButton(parent, id, label, pos, size, style)
			 ,Window(window)
			 ,twoHotkeys(false)
{
	Bind(wxEVT_LEFT_UP, &MappedButton::OnLeftClick, this);
	SetToolTip(toolTip);
}

MappedButton::MappedButton(wxWindow *parent, int id, const wxBitmap& bitmap, const wxPoint& pos,
            const wxSize& size, int window, long style)
			:wxButton(parent, id, "", pos, size, style)
			,Window(window)
			,twoHotkeys(false)
{
	Bind(wxEVT_LEFT_UP, &MappedButton::OnLeftClick, this);
	SetBitmap(bitmap);
}

MappedButton::~MappedButton()
{
}

void MappedButton::SetToolTip(const wxString &_toolTip)
{
	wxString toolTip = (_toolTip=="")? GetToolTipText().BeforeFirst('(').Trim() : _toolTip;
	wxString desc = GetLabelText();
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
	wxButton::SetToolTip(toolTip);
}
	
void MappedButton::OnLeftClick(wxMouseEvent &evt)
{
	if(evt.ShiftDown() || (twoHotkeys && evt.ControlDown())){
		//upewnij siê, ¿e da siê zmieniæ idy na nazwy, 
		//mo¿e i trochê spowolni operacjê ale skoñczy siê ci¹g³e wywalanie hotkeysów
		//mo¿e od razu funkcji onmaphotkey przekazaæ item by zrobi³a co trzeba
		int id= GetId(); 
		if(evt.ControlDown()){ id -= 1000; }
		wxString buttonName = (GetLabelText()!="")? GetLabelText() : GetToolTipText().BeforeFirst('(').Trim();
		Hkeys.OnMapHkey( GetId(), buttonName, this, Window, false);
		SetToolTip();
		Hkeys.SetAccels(true);
		Hkeys.SaveHkeys();
		
		return;
	}
	evt.Skip();
}