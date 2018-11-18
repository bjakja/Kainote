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

#pragma once

#include <wx/window.h>
#include "config.h"

class KaiCheckBox : public wxWindow
{
public:
	KaiCheckBox(wxWindow *parent, int id, const wxString& label,
			 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	virtual ~KaiCheckBox(){};
	//return false if not enabled
	bool GetValue(){return value;}
	bool GetRealValue(){ return value; }
	void SetValue(bool _value){value = _value; Refresh(false);}
	bool SetBackgroundColour(COLOR bgcolor){
		background = bgcolor;
		Refresh(false);
		return true;
	}
	bool SetForegroundColour(COLOR fgcolor){
		foreground = fgcolor;
		Refresh(false);
		return true;
	}
	bool Enable(bool enable=true);
	bool isCheckBox;
	bool value;
	bool enter;
	bool clicked;
private:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnKeyPress(wxKeyEvent &evt);
	void OnEraseBackground(wxEraseEvent &event){}
	wxString label;
	int fontHeight;
	COLOR background;
	COLOR foreground;
	wxDECLARE_ABSTRACT_CLASS(KaiCheckBox);
};

void BlueUp(wxBitmap *bmp);
