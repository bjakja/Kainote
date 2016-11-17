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

#include "Stylelistbox.h"

#include <wx/intl.h>
#include <wx/string.h>


Stylelistbox::Stylelistbox(wxWindow* parent, bool styles, wxString arr[ ], int count,const wxPoint& pos, int style)
	: wxDialog(parent, -1, (styles)?_("Wybór styli") : _("Wybór kolumn"))
{
	SetClientSize(wxSize(302,282));
	CheckListBox1 = new wxCheckListBox(this, -1, wxPoint(24,27), wxSize(248,208), count, arr, style);
	Button1 = new wxButton(this, wxID_OK, "Ok", wxPoint(24,248));
	Button2 = new wxButton(this, wxID_CANCEL, _("Anuluj"), wxPoint(112,248));
	StaticText1 = new wxStaticText(this, -1, (styles)?_("Wybierz style") : _("Wybierz kolumny"), wxPoint(24,8));
	CenterOnParent();
}

Stylelistbox::~Stylelistbox()
{
}

