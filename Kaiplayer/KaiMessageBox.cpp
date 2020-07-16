//  Copyright (c) 2016 - 2020, Marcin Drob

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
#include "KaiStaticText.h"


KaiMessageDialog::KaiMessageDialog(wxWindow *parent, const wxString& msg, 
	const wxString &caption, long elems, const wxPoint &pos, long buttonWithFocus)
	: KaiDialog(parent, -1, caption, pos)
{
	SetMinSize(wxSize(300, -1));
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	DialogSizer *sizer2 = new DialogSizer(wxVERTICAL);
	KaiStaticText *txt = new KaiStaticText(this, -1, msg);
	MappedButton *btn = NULL;
	//wxFont thisFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, L"Tahoma");
	//SetFont(thisFont);
	int whichFocus = 0;
	bool setFocus = true;
	if (elems & ASK_ONCE) {
		kcb = new KaiCheckBox(this, -1, _("Zastosuj dla wszystkich"));
		sizer1->Add(kcb, 0, wxALL| wxALIGN_CENTER_VERTICAL, 3);
	}
	if (elems & wxOK){
		btn = new MappedButton(this, 9009, L"OK", -1, wxDefaultPosition, wxSize(60, -1));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			int askOnce = (kcb && kcb->GetValue()) ? ASK_ONCE : 0;
			EndModal(wxOK | askOnce);
		}, 9009);
		sizer1->Add(btn, 1, wxALL, 3);
		if (buttonWithFocus < 0 || buttonWithFocus == wxOK) {
			btn->SetFocus(); setFocus = false;
		}
	}
	if (elems & wxYES_TO_ALL){
		btn = new MappedButton(this, wxYES_TO_ALL, _("Tak dla wszystkich"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			int result = wxYES_TO_ALL | ((kcb && kcb->GetValue()) ? ASK_ONCE : 0);
			EndModal(result);
		}, wxYES_TO_ALL);
		sizer1->Add(btn, 0, wxALL, 3);
		if (setFocus && (buttonWithFocus < 0 || buttonWithFocus == wxYES_TO_ALL)){ btn->SetFocus(); setFocus = false; }
	}
	if (elems & wxYES){
		btn = new MappedButton(this, wxID_YES, _("Tak"), -1, wxDefaultPosition, wxSize(60, -1));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			int result = wxYES | ((kcb && kcb->GetValue()) ? ASK_ONCE : 0);
			EndModal(result);
		}, wxID_YES);
		sizer1->Add(btn, 1, wxALL, 3);
		if (setFocus && (buttonWithFocus < 0 || buttonWithFocus == wxYES)){ btn->SetFocus(); setFocus = false; }
	}
	if (elems & wxNO){
		btn = new MappedButton(this, wxID_NO, _("Nie"), -1, wxDefaultPosition, wxSize(60, -1));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			EndModal(wxNO | ((kcb && kcb->GetValue()) ? ASK_ONCE : 0));
		}, wxID_NO);
		sizer1->Add(btn, 1, wxALL, 3);
		if (setFocus && (buttonWithFocus < 0 || buttonWithFocus == wxNO)){ btn->SetFocus(); setFocus = false; }
	}
	if (elems & wxCANCEL){
		btn = new MappedButton(this, 9010, _("Anuluj"), -1);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			EndModal(wxCANCEL | ((kcb && kcb->GetValue()) ? ASK_ONCE : 0));
		}, 9010);
		sizer1->Add(btn, 1, wxALL, 3);
		if (setFocus && (buttonWithFocus < 0 || buttonWithFocus == wxCANCEL)){ btn->SetFocus(); setFocus = false; }
	}
	if (elems & wxHELP){
		btn = new MappedButton(this, 9011, _("Pomoc"), -1);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			EndModal(wxHELP | ((kcb && kcb->GetValue()) ? ASK_ONCE : 0));
		}, 9011);
		sizer1->Add(btn, 1, wxALL, 3);
		if (setFocus && (buttonWithFocus < 0 || buttonWithFocus == wxHELP)){ btn->SetFocus(); setFocus = false; }
	}
	sizer2->Add(txt, 0, wxALL | wxALIGN_LEFT, 16);
	sizer2->Add(sizer1, 0, wxALL | wxALIGN_RIGHT, 3);
	sizer2->SetMinSize(180, -1);
	SetSizerAndFit(sizer2);
	int dir = 0;
	if (pos != wxDefaultPosition){
		if (pos.x == 0)
			dir |= wxHORIZONTAL;
		if (pos.y == 0)
			dir |= wxVERTICAL;

	}
	else{
		dir = wxBOTH;
	}
	CenterOnParent(dir);
	//Bind(wxEVT_CLOSE_WINDOW,[=](wxCloseEvent &evt){EndModal((elems & wxCANCEL)? wxCANCEL : (elems & wxNO)? wxNO : wxOK);});
	//here is the main problem id number is different than returned value
	SetEscapeId((elems & wxCANCEL) ? 9010 : (elems & wxNO) ? wxID_NO : 9009);

}

void KaiMessageDialog::SetOkLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(9009, this), MappedButton);
	if (btn) btn->SetLabelText(label);
}

void KaiMessageDialog::SetYesLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(wxID_YES, this), MappedButton);
	if (btn) btn->SetLabelText(label);
}

void KaiMessageDialog::SetNoLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(wxID_NO, this), MappedButton);
	if (btn) btn->SetLabelText(label);
}

void KaiMessageDialog::SetHelpLabel(const wxString &label)
{
	MappedButton *btn = wxDynamicCast(FindWindowById(9011, this), MappedButton);
	if (btn) btn->SetLabelText(label);
}



int KaiMessageBox(const wxString& msg, const wxString &caption, 
	long elems, wxWindow *parent, const wxPoint &pos, long buttonWithFocus){
	KaiMessageDialog dlgmsg(parent, msg, caption, elems, pos, buttonWithFocus);
	return dlgmsg.ShowModal();

}