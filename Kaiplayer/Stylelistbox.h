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
#include <wx/stattext.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class Stylelistbox: public wxDialog
{
	public:

		Stylelistbox(wxWindow* parent, bool styles=true, wxString arr[ ]=0, int count=0, const wxPoint& pos=wxDefaultPosition, int style=0);
		virtual ~Stylelistbox();

		
		wxButton* Button1;
		wxCheckListBox* CheckListBox1;
		wxStaticText* StaticText1;
		wxButton* Button2;
		
};

#endif
