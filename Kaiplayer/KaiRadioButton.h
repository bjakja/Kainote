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

#ifndef __KAIRADIOBUTTON__
#define __KAIRADIOBUTTON__

#include "KaiCheckBox.h"
#include <vector>

class KaiRadioButton : public KaiCheckBox
{
public:
	KaiRadioButton(wxWindow *parent, int id, const wxString& label,
             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	virtual ~KaiRadioButton(){};
	void SetValue(bool value);
private:
	
	void OnMouseLeft(wxMouseEvent &evt);
	void DeselectRest();
	void SelectNext(bool last);
	void SelectPrev(bool first);
	//bool hasGroup;

	wxDECLARE_ABSTRACT_CLASS(KaiRadioButton);
};

class KaiRadioBox : public wxWindow
{
public:
	KaiRadioBox(wxWindow *parent, int id, const wxString& label,
             const wxPoint& pos, const wxSize& size, const wxArrayString &names, int spacing=1, long style = wxVERTICAL);
	virtual ~KaiRadioBox(){};
	int GetSelection();
	void SetSelection(int sel);
	bool Enable(bool enable=true);
private:
	int selected;
	std::vector< KaiRadioButton*> buttons;
	wxDECLARE_ABSTRACT_CLASS(KaiRadioBox);
};

enum{
	ID_ACCEL_LEFT=19876,
	ID_ACCEL_RIGHT,
};

#endif
