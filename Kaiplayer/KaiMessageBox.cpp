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

#include "KaiMessageBox.h"
#include "MappedButton.h"
#include "config.h"
#include <wx/sizer.h>
#include <wx/stattext.h>


KaiMessageDialog::KaiMessageDialog(wxWindow *parent, const wxString& msg, const wxString &caption, long elems)
	: wxDialog(parent, -1, caption)
{
	SetForegroundColour(Options.GetColour("Window Text"));
	SetBackgroundColour(Options.GetColour("Window Background"));
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer2 = new wxBoxSizer(wxVERTICAL);
	wxStaticText *txt = new wxStaticText(this,-1,msg);
	MappedButton *btn=NULL;
	if(elems & wxOK){
		btn = new MappedButton(this,9009,"OK");
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxOK);},9009);
		sizer1->Add(btn,0,wxALL,3);
	}
	if(elems & wxYES_TO_ALL){
		btn = new MappedButton(this,wxYES_TO_ALL,_("Tak dla wszystkich"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxYES_TO_ALL);},wxYES_TO_ALL);
		sizer1->Add(btn,0,wxALL,3);
	}
	if(elems & wxYES){
		btn = new MappedButton(this,wxID_YES,_("Tak"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxYES);},wxID_YES);
		sizer1->Add(btn,0,wxALL,3);
	}
	if(elems & wxNO){
		btn = new MappedButton(this,wxID_NO,_("Nie"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxNO);},wxID_NO);
		sizer1->Add(btn,0,wxALL,3);
	}
	if(elems & wxCANCEL){
		btn = new MappedButton(this,9010,_("Anuluj"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxCANCEL);},9010);
		sizer1->Add(btn,0,wxALL,3);
	}
	if(elems & wxHELP){
		btn = new MappedButton(this,9011,_("Pomoc"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxHELP);},9011);
		sizer1->Add(btn,0,wxALL,3);
	}
	sizer2->Add(txt,0,wxALL|wxALIGN_CENTER_HORIZONTAL,16);
	sizer2->Add(sizer1,0,wxALL|wxALIGN_RIGHT,3);
	SetSizerAndFit(sizer2);
	CenterOnParent();
}




int KaiMessageBox(const wxString& msg, const wxString &caption, long elems, wxWindow *parent){
	KaiMessageDialog dlgmsg(parent, msg, caption, elems);
	return dlgmsg.ShowModal();

}