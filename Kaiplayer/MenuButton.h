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

#ifndef _MENU_BUTTON_
#define _MENU_BUTTON_

#include "wx/statbmp.h"
#include "Menu.h"

class MenuButton : public wxStaticBitmap
{
public:
	MenuButton(wxWindow *parent, int id, const wxString &tooltip, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);
	~MenuButton(){if(menu){delete menu;}}
	//it own menu
	void PutMenu(Menu *menu);
	Menu *GetMenu(){return menu;}
private:
	void OnMouseEvent(wxMouseEvent &evt);
	bool IsMenuShown;
	Menu *menu;
};

#endif