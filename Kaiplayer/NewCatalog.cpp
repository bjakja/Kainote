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



NewCatalog::NewCatalog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	:KaiDialog(parent, id, _("Wybór nazwy katalogu"), wxDefaultPosition, wxSize(300,-1), wxDEFAULT_DIALOG_STYLE)
{
	KaiTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString excludes;
	excludes.Add(L"\\");
	excludes.Add(L"/");
	excludes.Add(L"*");
	excludes.Add(L"?");
	excludes.Add(L":");
	excludes.Add(L"\"");
	excludes.Add(L"<");
	excludes.Add(L">");
	excludes.Add(L"|");
	valid.SetExcludes(excludes);
	TextCtrl1 = new KaiTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, valid);
	KaiStaticBoxSizer *StaticBox1 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Podaj nazwę nowego katalogu"));
	StaticBox1->Add(TextCtrl1, 0, wxEXPAND | wxALL, 2);
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	DialogSizer *sizer1 = new DialogSizer(wxVERTICAL);
	Button1 = new MappedButton(this, wxID_OK, _("Utwórz"));
	Button2 = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	sizer->Add(Button1, 0, wxALL, 5);
	sizer->Add(Button2, 0, wxALL, 5);
	sizer1->Add(StaticBox1);
	sizer1->Add(sizer);
	SetSizerAndFit(sizer1);
	CenterOnParent();
}

NewCatalog::~NewCatalog()
{
}
