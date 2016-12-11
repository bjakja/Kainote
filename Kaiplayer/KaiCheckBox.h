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

#ifndef __KAICHECKBOX__
#define __KAICHECKBOX__

#include <wx/wx.h>

class KaiCheckBox : public wxWindow
{
public:
	KaiCheckBox(wxWindow *parent, int id, const wxString& label,
             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	virtual ~KaiCheckBox(){};
	bool GetValue(){return value;}
	void SetValue(bool _value){value = _value; Refresh(false);}
	//void SetBackgroundColour(const wxColour &bgcolor);
	//void SetForegroundColour(const wxColour &fgcolor);
	bool isCheckBox;
	bool value;
	bool enter;
	bool clicked;
private:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnKeyPress(wxKeyEvent &evt);
	wxString label;
	int fontHeight;
	//wxColour background;
	//wxColour foreground;
	wxDECLARE_ABSTRACT_CLASS(KaiCheckBox);
};


#endif