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

	Create(parent, id, _("Wybór nazwy katalogu"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, "id");
	SetClientSize(wxSize(290,112));
	Move(wxDefaultPosition);
	TextCtrl1 = new KaiTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxPoint(24,35), wxSize(244,21), wxTE_PROCESS_ENTER, valid, "ID_TEXTCTRL1");
	StaticBox1 = new wxStaticBox(this, ID_STATICBOX1, _("Podaj nazwę nowego katalogu"), wxPoint(8,8), wxSize(274,64), 0, "ID_STATICBOX1");
	Button1 = new wxButton(this, wxID_OK, _("Utwórz"), wxPoint(24,83), wxDefaultSize, 0, wxDefaultValidator, "wxID_OK");
	Button2 = new wxButton(this, wxID_CANCEL, _("Anuluj"), wxPoint(112,83), wxDefaultSize, 0, wxDefaultValidator, "wxID_CANCEL");

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&NewCatalog::OnCatalogCommit);
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
