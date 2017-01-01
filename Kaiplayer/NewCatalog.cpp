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

#include "NewCatalog.h"
#include "KaiStaticBoxSizer.h"

//(*InternalHeaders(NewCatalog)
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/wx.h>
//*)

//(*IdInit(NewCatalog)
const long NewCatalog::ID_TEXTCTRL1 = wxNewId();
const long NewCatalog::ID_STATICBOX1 = wxNewId();
//*)

NewCatalog::NewCatalog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
	:wxDialog(parent, id, _("Wybór nazwy katalogu"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	KaiTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString excludes;
	excludes.Add("\\");
	excludes.Add("/");
	excludes.Add("*");
	excludes.Add("?");
	excludes.Add(":");
	excludes.Add("\"");
	excludes.Add("<");
	excludes.Add(">");
	excludes.Add("|");
	valid.SetExcludes(excludes);
	TextCtrl1 = new KaiTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, valid);
	KaiStaticBoxSizer *StaticBox1 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Podaj nazwę nowego katalogu"));
	StaticBox1->Add(TextCtrl1, 0, wxEXPAND|wxALL, 2);
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
	Button1 = new MappedButton(this, wxID_OK, _("Utwórz"), 0);
	Button2 = new MappedButton(this, wxID_CANCEL, _("Anuluj"), 0);
	sizer->Add(Button1, 0, wxALL, 2);
	sizer->Add(Button2, 0, wxALL, 2);
	sizer1->Add(StaticBox1);
	sizer1->Add(sizer);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&NewCatalog::OnCatalogCommit);
	SetSizerAndFit(sizer1);
}

NewCatalog::~NewCatalog()
{
	//(*Destroy(NewCatalog)
	//*)
}

void NewCatalog::OnCatalogCommit(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}
