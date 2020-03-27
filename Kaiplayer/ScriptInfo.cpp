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
#include "KaiStaticBoxSizer.h"
#include "KaiStaticText.h"
#include <wx/sizer.h>

ScriptInfo::ScriptInfo(wxWindow* parent, int w, int h)
	:KaiDialog(parent, -1, _("Właściwości napisów ASS"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	res = wxSize(w, h);
	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource(L"ASSPROPS"));
	SetIcon(icn);
	DialogSizer *mainsizer = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer *StaticBox1 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Informacje o napisach"));
	wxGridSizer *GridSizer = new wxGridSizer(2, 5, 5);
	title = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	script = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	translation = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	editing = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	timing = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	update = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

	GridSizer->Add(new KaiStaticText(this, -1, _("Tytuł")), 0, wxEXPAND);
	GridSizer->Add(title, 0, wxEXPAND);
	GridSizer->Add(new KaiStaticText(this, -1, _("Autor")), 0, wxEXPAND);
	GridSizer->Add(script, 0, wxEXPAND);
	GridSizer->Add(new KaiStaticText(this, -1, _("Tłumaczenie")), 0, wxEXPAND);
	GridSizer->Add(translation, 0, wxEXPAND);
	GridSizer->Add(new KaiStaticText(this, -1, _("Korekta")), 0, wxEXPAND);
	GridSizer->Add(editing, 0, wxEXPAND);
	GridSizer->Add(new KaiStaticText(this, -1, _("Timing")), 0, wxEXPAND);
	GridSizer->Add(timing, 0, wxEXPAND);
	GridSizer->Add(new KaiStaticText(this, -1, _("Edycja")), 0, wxEXPAND);
	GridSizer->Add(update, 0, wxEXPAND);

	StaticBox1->Add(GridSizer, 0, wxEXPAND | wxALL, 5);

	KaiStaticBoxSizer *StaticBox2 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Rozdzielczość"));
	wxBoxSizer *boxsizer = new wxBoxSizer(wxHORIZONTAL);

	width = new NumCtrl(this, -1, L"", 100, 10000, true, wxDefaultPosition, wxSize(60, -1), wxTE_PROCESS_ENTER);
	height = new NumCtrl(this, -1, L"", 100, 10000, true, wxDefaultPosition, wxSize(60, -1), wxTE_PROCESS_ENTER);
	resolutionFromVideo = new MappedButton(this, 25456, _("Z wideo"), -1, wxDefaultPosition, wxSize(70, -1));
	resolutionFromVideo->Enable(w > 0);

	boxsizer->Add(new KaiStaticText(this, -1, _("Szerokość"), wxDefaultPosition, wxSize(-1, -1)), 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	boxsizer->Add(width, 1, wxALL, 5);
	boxsizer->Add(new KaiStaticText(this, -1, _("Wysokość"), wxDefaultPosition, wxSize(-1, -1)), 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	boxsizer->Add(height, 1, wxALL, 5);
	boxsizer->Add(resolutionFromVideo, 1, wxALL, 5);

	StaticBox2->Add(boxsizer, 0, wxEXPAND);


	matrix = new KaiChoice(this, -1, wxDefaultPosition, wxSize(160, -1));
	matrix->SetSelection(matrix->Append(_("Brak")));
	matrix->Append("TV.601");
	matrix->Append("PC.601");
	matrix->Append("TV.709");
	matrix->Append("PC.709");
	matrix->Append("TV.FCC");
	matrix->Append("PC.FCC");
	matrix->Append("TV.240M");
	matrix->Append("PC.240M");
	wxBoxSizer *boxsizer2 = new wxBoxSizer(wxHORIZONTAL);
	boxsizer2->Add(new KaiStaticText(this, -1, _("Macierz YCbCr"), wxDefaultPosition), 1);
	boxsizer2->Add(matrix, 1, wxLEFT, 3);
	StaticBox2->Add(boxsizer2, 1, wxEXPAND | wxALL, 5);

	KaiStaticBoxSizer *StaticBox3 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Opcje"));
	wxGridSizer *GridSizer1 = new wxGridSizer(2, 5, 5);

	wrapstyle = new KaiChoice(this, -1/*, wxDefaultPosition, wxSize(160,-1)*/);
	wrapstyle->SetSelection(wrapstyle->Append(_("0: Autopodział, górna linijka jest szersza")));
	wrapstyle->Append(_("1: Podział co koniec linijki, dzieli tylko \\N"));
	wrapstyle->Append(_("2: Brak podziału, dzieli \\n i \\N"));
	wrapstyle->Append(_("3: Autopodział, dolna linijka jest szersza"));
	collision = new KaiChoice(this, -1);
	collision->SetSelection(collision->Append(_("Normalne")));
	collision->Append(_("Odwrócone"));

	GridSizer1->Add(new KaiStaticText(this, -1, _("Styl dzielenia linijek")), 1, wxEXPAND | wxLEFT, 5);
	GridSizer1->Add(wrapstyle, 1, wxEXPAND | wxRIGHT, 5);
	GridSizer1->Add(new KaiStaticText(this, -1, _("Kolidowanie linijek")), 1, wxEXPAND | wxLEFT, 5);
	GridSizer1->Add(collision, 1, wxEXPAND | wxRIGHT, 5);

	scaleBorderAndShadow = new KaiCheckBox(this, -1, _("Skaluj obwódkę i cień"));
	scaleBorderAndShadow->SetValue(true);

	StaticBox3->Add(GridSizer1, 1, wxEXPAND);
	StaticBox3->Add(scaleBorderAndShadow, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer *boxsizer1 = new wxBoxSizer(wxHORIZONTAL);
	save = new MappedButton(this, wxID_OK, _("Zapisz"));
	cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	save->SetFocus();
	SetTmpDefaultItem(save);
	boxsizer1->Add(save, 1, wxALL, 5);
	boxsizer1->Add(cancel, 1, wxALL, 5);

	mainsizer->Add(StaticBox1, 0, wxEXPAND | wxALL, 4);
	mainsizer->Add(StaticBox2, 0, wxEXPAND | wxALL, 4);
	mainsizer->Add(StaticBox3, 1, wxEXPAND | wxALL, 4);
	mainsizer->Add(boxsizer1, 0, wxBOTTOM | wxALIGN_CENTER, 3);

	SetSizerAndFit(mainsizer);

	Connect(25456, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ScriptInfo::OnVideoRes);
	DoTooltips();
	CenterOnParent();
}

ScriptInfo::~ScriptInfo()
{
}

void ScriptInfo::OnVideoRes(wxCommandEvent& event)
{
	wxString tmp;
	width->SetValue(tmp << res.x);
	tmp = L"";
	height->SetValue(tmp << res.y);
	width->SetModified(true);
	height->SetModified(true);
}

void ScriptInfo::DoTooltips()
{
	height->SetToolTip(_("Wysokość wideo"));
	width->SetToolTip(_("Szerokość wideo"));
	wrapstyle->SetToolTip(_("Sposób dzielenia napisów"));
	collision->SetToolTip(_("Kolidowanie linijek"));
	scaleBorderAndShadow->SetToolTip(_("Skalowana obwódka i cień"));
}