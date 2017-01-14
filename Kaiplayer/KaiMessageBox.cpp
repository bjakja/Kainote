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
	: KaiDialog(parent, -1, caption)
{
	//SetForegroundColour(Options.GetColour("Window Text"));
	//SetBackgroundColour(Options.GetColour("Window Background"));
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	DialogSizer *sizer2 = new DialogSizer(wxVERTICAL);
	wxStaticText *txt = new wxStaticText(this,-1,msg);
	MappedButton *btn=NULL;
	int whichFocus=0;
	bool setFocus = true;
	if(elems & wxOK){
		btn = new MappedButton(this,9009,"OK");
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxOK);},9009);
		sizer1->Add(btn,0,wxALL,3);
		btn -> SetFocus(); setFocus = false;
	}
	if(elems & wxYES_TO_ALL){
		btn = new MappedButton(this,wxYES_TO_ALL,_("Tak dla wszystkich"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxYES_TO_ALL);},wxYES_TO_ALL);
		sizer1->Add(btn,0,wxALL,3);
		if(setFocus){btn -> SetFocus(); setFocus = false;}
	}
	if(elems & wxYES){
		btn = new MappedButton(this,wxID_YES,_("Tak"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxYES);},wxID_YES);
		sizer1->Add(btn,0,wxALL,3);
		if(setFocus){btn -> SetFocus(); setFocus = false;}
	}
	if(elems & wxNO){
		btn = new MappedButton(this,wxID_NO,_("Nie"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxNO);},wxID_NO);
		sizer1->Add(btn,0,wxALL,3);
		if(setFocus){btn -> SetFocus(); setFocus = false;}
	}
	if(elems & wxCANCEL){
		btn = new MappedButton(this,9010,_("Anuluj"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxCANCEL);},9010);
		sizer1->Add(btn,0,wxALL,3);
		if(setFocus){btn -> SetFocus(); setFocus = false;}
	}
	if(elems & wxHELP){
		btn = new MappedButton(this,9011,_("Pomoc"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EndModal(wxHELP);},9011);
		sizer1->Add(btn,0,wxALL,3);
		if(setFocus){btn -> SetFocus(); setFocus = false;}
	}
	sizer2->Add(txt,0,wxALL|wxALIGN_CENTER_HORIZONTAL,16);
	sizer2->Add(sizer1,0,wxALL|wxALIGN_RIGHT,3);
	SetSizerAndFit(sizer2);
	CenterOnParent();
	Bind(wxEVT_CLOSE_WINDOW,[=](wxCloseEvent &evt){EndModal((elems & wxCANCEL)? wxCANCEL : wxNO);});
	SetEscapeId((elems & wxCANCEL)? 9010 : (elems & wxNO)? wxID_NO : 9009);
	
}

void KaiMessageDialog::SetOkLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(9009, this), MappedButton);
	if(btn) btn->SetLabelText(label);
}
	
void KaiMessageDialog::SetYesLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(wxID_YES, this), MappedButton);
	if(btn) btn->SetLabelText(label);
}
	
void KaiMessageDialog::SetNoLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(wxID_NO, this), MappedButton);
	if(btn) btn->SetLabelText(label);
}
	
void KaiMessageDialog::SetHelpLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(9011, this), MappedButton);
	if(btn) btn->SetLabelText(label);
}



int KaiMessageBox(const wxString& msg, const wxString &caption, long elems, wxWindow *parent){
	KaiMessageDialog dlgmsg(parent, msg, caption, elems);
	return dlgmsg.ShowModal();

}