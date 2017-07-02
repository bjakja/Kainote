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
#include <vector>
#include "config.h"

class KaiStatusBar : public wxWindow{
public:
	KaiStatusBar(wxWindow *parent, int id = -1, int style = 0);
	virtual ~KaiStatusBar(){wxDELETE(bmp);};

	void SetFieldsCount(int num, int *fields);
	void SetLabelText(size_t field, const wxString &label);
	wxString GetStatusText(size_t field) const;
	void SetLabelTextColour(size_t field, COLOR textColor);
	void SetLabelBackgroundColour(size_t field, COLOR backgroundColor);
	void SetTooltips(wxString *tips, int count);

private:
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouseMove(wxMouseEvent &evt);
	void CalcWidths(wxArrayInt *widths);
	bool AcceptsFocus( ) const{return false;}
	bool AcceptsFocusFromKeyboard () const {return false;}
	bool AcceptsFocusRecursively () const{return false;}
	wxBitmap *bmp;
	wxString tip;
	wxArrayInt sizes;
	wxArrayString labels;
	wxArrayString tips;
	std::vector<COLOR> foreground;
	std::vector<COLOR> background;
};

