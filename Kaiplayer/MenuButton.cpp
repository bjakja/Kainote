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

#include "MenuButton.h"

MenuButton::MenuButton(wxWindow *parent, int id, const wxString &tooltip, const wxPoint &pos, const wxSize &size)
	: MappedButton(parent, id, tooltip, wxBITMAP_PNG("ARROW_LIST_DOUBLE"), pos, size, -1)
	, IsMenuShown(false)
	, menu(NULL)
{
	//SetToolTip(tooltip);
	Bind(wxEVT_LEFT_DOWN, &MenuButton::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &MenuButton::OnMouseEvent, this);
	//Bind(wxEVT_LEFT_UP, &MenuButton::OnMouseEvent, this);
}

void MenuButton::OnMouseEvent(wxMouseEvent &evt)
{
	if(!menu){return;}
	//wxPoint pos = //GetPosition();
	wxSize size = GetClientSize();
	wxPoint pos;
	pos.y = size.y+1;
	pos.x = size.x/2;
	menu->PopupMenu(pos, this, true, true);
	
}

void MenuButton::PutMenu(Menu *_menu)
{
	if(menu){delete menu; menu = NULL;}
	menu = _menu;
}