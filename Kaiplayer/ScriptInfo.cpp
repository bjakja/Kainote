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

#include "ScriptInfo.h"
#include "config.h"

#include <wx/string.h>
#include <wx/sizer.h>

ScriptInfo::ScriptInfo(wxWindow* parent, int w, int h)
	:wxDialog(parent, -1, _("Właściwości napisów ASS"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	res=wxSize(w,h);
	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("ASSPROPS"));
	SetIcon(icn);
	wxBoxSizer *mainsizer=new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *StaticBox1 = new wxStaticBoxSizer(wxVERTICAL,this, _("Informacje o napisach"));
	wxGridSizer *GridSizer=new wxGridSizer(2,5,5);
	title = new wxTextCtrl(this, -1,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	script = new wxTextCtrl(this, -1,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	translation = new wxTextCtrl(this, -1,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	editing = new wxTextCtrl(this, -1,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	timing = new wxTextCtrl(this, -1,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	update = new wxTextCtrl(this, -1,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);

	GridSizer->Add(new wxStaticText(this, -1, _("Tytuł")),0,wxEXPAND);
	GridSizer->Add(title,0,wxEXPAND);
	GridSizer->Add(new wxStaticText(this, -1, _("Autor")),0,wxEXPAND);
	GridSizer->Add(script,0,wxEXPAND);
	GridSizer->Add(new wxStaticText(this, -1, _("Tłumaczenie")),0,wxEXPAND);
	GridSizer->Add(translation,0,wxEXPAND);
	GridSizer->Add(new wxStaticText(this, -1, _("Korekta")),0,wxEXPAND);
	GridSizer->Add(editing,0,wxEXPAND);
	GridSizer->Add(new wxStaticText(this, -1, _("Timing")),0,wxEXPAND);
	GridSizer->Add(timing,0,wxEXPAND);
	GridSizer->Add(new wxStaticText(this, -1, _("Edycja")),0,wxEXPAND);
	GridSizer->Add(update,0,wxEXPAND);

	StaticBox1->Add(GridSizer,0,wxEXPAND|wxALL,5);

	wxStaticBoxSizer *StaticBox2 = new wxStaticBoxSizer(wxVERTICAL,this, _("Rozdzielczość"));
	wxBoxSizer *boxsizer= new wxBoxSizer(wxHORIZONTAL);

	width = new NumCtrl(this, -1,"",100,10000,true,wxDefaultPosition,wxSize(60,-1),wxTE_PROCESS_ENTER);
	height = new NumCtrl(this, -1,"",100,10000,true,wxDefaultPosition,wxSize(60,-1),wxTE_PROCESS_ENTER);
	Fvideo= new wxButton(this,25456,_("Z wideo"),wxDefaultPosition,wxSize(85,-1));
	Fvideo->Enable(w>0);

	boxsizer->Add(new wxStaticText(this, -1, _("Szerokość"),wxDefaultPosition,wxSize(60,-1)),0,wxALL,5);
	boxsizer->Add(width,0,wxALL,5);
	boxsizer->Add(new wxStaticText(this, -1, _("Wysokość"),wxDefaultPosition,wxSize(60,-1)),0,wxALL,5);
	boxsizer->Add(height,0,wxALL,5);
	boxsizer->Add(Fvideo,0,wxALL,5);

	CheckBox1 = new wxCheckBox(this, -1, _("Zmiana rozdzielczości tylko w nagłówku napisów"));
	CheckBox1->SetValue(true);

	StaticBox2->Add(boxsizer,0,wxEXPAND);
	StaticBox2->Add(CheckBox1,0,wxEXPAND|wxALL,5);

	wxStaticBoxSizer *StaticBox3 = new wxStaticBoxSizer(wxVERTICAL,this, _("Opcje"));
	wxFlexGridSizer *GridSizer1=new wxFlexGridSizer(2,5,5);

	wrapstyle = new wxChoice(this, -1);
	wrapstyle->SetSelection( wrapstyle->Append(_("0: Autopodział, górna linijka jest szersza")) );
	wrapstyle->Append(_("1: Podział co koniec linijki, dzieli tylko \\N"));
	wrapstyle->Append(_("2: Brak podziału, dzieli \\n i \\N"));
	wrapstyle->Append(_("3: Autopodział, dolna linijka jest szersza"));
	collision = new wxChoice(this, -1);
	collision->SetSelection( collision->Append(_("Normalne")) );
	collision->Append(_("Odwrócone"));

	GridSizer1->Add(new wxStaticText(this, -1, _("Styl dzielenia linijek")),0,wxEXPAND|wxALL,5);
	GridSizer1->Add(wrapstyle,0,wxEXPAND|wxALL,5);
	GridSizer1->Add(new wxStaticText(this, -1, _("Kolidowanie linijek")),0,wxEXPAND|wxALL,5);
	GridSizer1->Add(collision,0,wxEXPAND|wxALL,5);

	CheckBox2 = new wxCheckBox(this, -1, _("Skaluj obwódkę i cień"));
	CheckBox2->SetValue(true);

	StaticBox3->Add(GridSizer1,0,wxEXPAND);
	StaticBox3->Add(CheckBox2,0,wxEXPAND|wxALL,5);

	wxBoxSizer *boxsizer1= new wxBoxSizer(wxHORIZONTAL);
	Button1 = new wxButton(this, wxID_OK, _("Zapisz"));
	Button2 = new wxButton(this, wxID_CANCEL, _("Anuluj"));

	boxsizer1->Add(Button1,0,wxALL,5);
	boxsizer1->Add(Button2,0,wxALL,5);

	mainsizer->Add(StaticBox1,0,wxEXPAND|wxALL,4);
	mainsizer->Add(StaticBox2,0,wxEXPAND|wxALL,4);
	mainsizer->Add(StaticBox3,0,wxEXPAND|wxALL,4);
	mainsizer->Add(boxsizer1,0,wxEXPAND|wxALL,4);

	SetSizerAndFit(mainsizer);

	Connect(25456,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptInfo::OnVideoRes);

	DoTooltips();
	CenterOnParent();
}

ScriptInfo::~ScriptInfo()
{
}

void ScriptInfo::OnVideoRes(wxCommandEvent& event)
{
	wxString tmp;
	width->SetValue(tmp<<res.x);
	tmp="";
	height->SetValue(tmp<<res.y);
	width->SetModified(true);
	height->SetModified(true);
}

void ScriptInfo::DoTooltips()
{
	height->SetToolTip(_("Wysokość wideo"));
	width->SetToolTip(_("Szerokość wideo"));
	wrapstyle->SetToolTip(_("Sposób dzielenia napisów"));
	collision->SetToolTip(_("Kolidowanie linijek"));
	CheckBox1->SetToolTip(_("Zmiana rozdzielczości tylko w nagłówku napisów, odznaczenie powoduje przeskalowanie całych napisów"));
	CheckBox2->SetToolTip(_("Skalowana obwódka i cień"));
}