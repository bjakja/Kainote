//  Copyright (c) 2017, Marcin Drob

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

#pragma once

#include <wx/window.h>
#include "config.h"
#include "KaiScrollbar.h"

class KaiStaticText : public wxWindow
{
public:
	KaiStaticText(wxWindow *parent, int id, const wxString& text, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	virtual ~KaiStaticText(){};

	void OnPaint(wxPaintEvent &evt);
	void SetLabelText(const wxString &_text);
	bool SetForegroundColour(COLOR txtColor){textColour=txtColor, Refresh(false);return false;}
	wxString GetLabelText() const {return text;}
	
private:
	bool AcceptsFocus( ) const{return false;}
	bool AcceptsFocusFromKeyboard () const {return false;}
	bool AcceptsFocusRecursively () const{return false;}
	void OnScroll(wxScrollEvent& event);
	void OnMouseScroll(wxMouseEvent &evt);
	
	int textHeight;
	int scPos;
	COLOR textColour;
	wxString text;
	KaiScrollbar *textScroll;
	DECLARE_EVENT_TABLE()
};
