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

#ifndef _MAPPED_BUTTON_
#define _MAPPED_BUTTON_

#include "wx/button.h"
#include "Hotkeys.h"

class MappedButton :public wxButton
{
public:
	MappedButton(wxWindow *parent, int id, const wxString& label = wxEmptyString, const wxString& tooltip = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int window = EDITBOX_HOTKEY, long style = 0);
	MappedButton(wxWindow *parent, int id, const wxBitmap& bitmap, const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize, int window = AUDIO_HOTKEY, long style = 0);
	void SetTwoHotkeys(){twoHotkeys=true;}
	virtual ~MappedButton();
	void SetToolTip(const wxString &toolTip="");
private:
	void OnLeftClick(wxMouseEvent &evt);
	int Window;
	bool twoHotkeys;
};

#endif