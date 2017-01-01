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
#include "config.h"
#include "KainoteMain.h"


Stylelistbox::Stylelistbox(wxWindow* parent, bool styles, int numelem, wxString *arr,const wxPoint& pos, int style, int type)
	: wxDialog(parent, -1, (styles)?_("Wybór styli") : _("Wybór kolumn"))
{
	SetForegroundColour(Options.GetColour("Window Text"));
	SetBackgroundColour(Options.GetColour("Window Background"));
	wxStaticBoxSizer *sizer1 = new wxStaticBoxSizer(wxVERTICAL, this, (styles)?_("Wybierz style") : _("Wybierz kolumny"));
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	CheckListBox1 = new KaiListCtrl(this, -1,numelem, arr, type, wxDefaultPosition, wxSize(200,300), style);
	Button1 = new MappedButton(this, wxID_OK, "Ok");
	Button2 = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	sizer->Add(Button1, 0, wxALL, 2);
	sizer->Add(Button2, 0, wxALL, 2);
	sizer1->Add(CheckListBox1,0, wxEXPAND);
	sizer1->Add(sizer,0, wxEXPAND);
	SetSizerAndFit(sizer1);
	wxPoint mousepos = wxGetMousePosition();
	SetPosition(mousepos);
}

Stylelistbox::~Stylelistbox()
{
}

wxString GetCheckedElements(wxWindow *parent)
{

	wxString styletext="";
	wxString *elems;
	const std::vector<Styles*> styles = Notebook::GetTab()->Grid1->file->GetSubs()->styles;
	elems = new wxString[styles.size()];
	for (size_t j=0;j<styles.size();j++){
		Styles *acstyl=styles[j];
		elems[j] = acstyl->Name;
	}
	Stylelistbox slx(parent, true, styles.size(), elems);
	if(slx.ShowModal()==wxID_OK){

		for (size_t v=0;v<slx.CheckListBox1->GetCount();v++)
		{

			if(slx.CheckListBox1->GetItem(v, 0)->modified){
				styletext<<slx.CheckListBox1->GetItem(v, 0)->name<<";";
			}
		}
	}
	delete [] elems;
	return styletext.BeforeLast(';');
}