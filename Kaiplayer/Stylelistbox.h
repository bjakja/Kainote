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

#ifndef STYLELISTBOX_H
#define STYLELISTBOX_H

//(*Headers(Stylelistbox)
#include <wx/string.h>
#include "KaiListCtrl.h"
#include "MappedButton.h"
#include <wx/dialog.h>
//*)

class Stylelistbox: public wxDialog
{
	public:

		Stylelistbox(wxWindow* parent, bool styles=true, int numelem = 0, wxString *arr=0, const wxPoint& pos=wxDefaultPosition, int style=0, int type = TYPE_CHECKBOX);
		virtual ~Stylelistbox();

		
		MappedButton* Button1;
		KaiListCtrl* CheckListBox1;
		MappedButton* Button2;
		
};

wxString GetCheckedElements(wxWindow *parent);


#endif
