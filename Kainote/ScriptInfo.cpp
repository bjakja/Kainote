//  Copyright (c) 2016 - 2026, Marcin Drob

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
//#include "Utils.h"
#include "KaiStaticBoxSizer.h"
#include "KaiStaticText.h"
#include <wx/sizer.h>

ScriptInfo::ScriptInfo(wxWindow* parent, int w, int h)
	:KaiDialog(parent, -1, _("ASS subtitle properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	res = wxSize(w, h);
	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource(L"ASSPROPS"));
	SetIcon(icn);
	DialogSizer *mainsizer = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer *StaticBox1 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Subtitle information"));
	//wxGridSizer *GridSizer = new wxGridSizer(2, 5, 5);
	wxBoxSizer* bxsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bxsizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bxsizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bxsizer3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bxsizer4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bxsizer5 = new wxBoxSizer(wxHORIZONTAL);

	title = new KaiTextCtrl(this, -1, emptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	script = new KaiTextCtrl(this, -1, emptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	translation = new KaiTextCtrl(this, -1, emptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	editing = new KaiTextCtrl(this, -1, emptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	timing = new KaiTextCtrl(this, -1, emptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	update = new KaiTextCtrl(this, -1, emptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

	bxsizer->Add(new KaiStaticText(this, -1, _("Title")), 1, wxEXPAND );
	bxsizer->Add(title, 2, wxEXPAND);
	bxsizer1->Add(new KaiStaticText(this, -1, _("Author")), 1, wxEXPAND);
	bxsizer1->Add(script, 2, wxEXPAND);
	bxsizer2->Add(new KaiStaticText(this, -1, _("Translator")), 1, wxEXPAND);
	bxsizer2->Add(translation, 2, wxEXPAND);
	bxsizer3->Add(new KaiStaticText(this, -1, _("Proofreading")), 1, wxEXPAND);
	bxsizer3->Add(editing, 2, wxEXPAND);
	bxsizer4->Add(new KaiStaticText(this, -1, _("Timer")), 1, wxEXPAND);
	bxsizer4->Add(timing, 2, wxEXPAND);
	bxsizer5->Add(new KaiStaticText(this, -1, _("Editing")), 1, wxEXPAND);
	bxsizer5->Add(update, 2, wxEXPAND);

	StaticBox1->Add(bxsizer, 0, wxEXPAND | wxALL, 5);
	StaticBox1->Add(bxsizer1, 0, wxEXPAND | wxALL, 5);
	StaticBox1->Add(bxsizer2, 0, wxEXPAND | wxALL, 5);
	StaticBox1->Add(bxsizer3, 0, wxEXPAND | wxALL, 5);
	StaticBox1->Add(bxsizer4, 0, wxEXPAND | wxALL, 5);
	StaticBox1->Add(bxsizer5, 0, wxEXPAND | wxALL, 5);

	KaiStaticBoxSizer *StaticBox2 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Resolution"));
	wxBoxSizer *boxsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* boxsizer3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* boxsizer4 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* boxsizer5 = new wxBoxSizer(wxHORIZONTAL);

	width = new NumCtrl(this, -1, emptyString, 100, 10000, true, wxDefaultPosition, wxSize(60, -1), wxTE_PROCESS_ENTER);
	height = new NumCtrl(this, -1, emptyString, 100, 10000, true, wxDefaultPosition, wxSize(60, -1), wxTE_PROCESS_ENTER);
	layoutWidth = new NumCtrl(this, -1, emptyString, 0, 10000, true, wxDefaultPosition, wxSize(60, -1), wxTE_PROCESS_ENTER);
	layoutHeight = new NumCtrl(this, -1, emptyString, 0, 10000, true, wxDefaultPosition, wxSize(60, -1), wxTE_PROCESS_ENTER);
	resolutionFromVideo = new MappedButton(this, 25456, _("From video"), -1, wxDefaultPosition, wxSize(70, -1));
	resolutionFromVideo->Enable(w > 0);
	layoutFromVideo = new MappedButton(this, 25457, _("From video"), -1, wxDefaultPosition, wxSize(70, -1));
	layoutFromVideo->Enable(w > 0);
	linkResolutions = new ToggleButton(this, 25458, L"", L"", wxDefaultPosition, wxSize(26 * 1.5, 50));
	const wxBitmap& link = wxBITMAP_PNG(L"button_link");
	wxImage imgLink = link.ConvertToImage();
	imgLink = imgLink.Rotate90(false);
	int ih = imgLink.GetHeight();
	int iw = imgLink.GetWidth();
	imgLink = imgLink.Scale(iw * 1.5, ih * 1.5);
	linkResolutions->SetBitmap(wxBitmap(imgLink));
	bool linkRes = Options.GetBool(LINK_RESOLUTIONS);
	linkResolutions->SetValue(linkRes);
	if (linkRes) {
		layoutWidth->Enable(false);
		layoutHeight->Enable(false);
		layoutFromVideo->Enable(false);
	}

	boxsizer->Add(new KaiStaticText(this, -1, _("Subtitles"), wxDefaultPosition, wxSize(-1, -1)), 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	boxsizer->Add(width, 1, wxALL, 5);
	boxsizer->Add(new KaiStaticText(this, -1, L"  X  ", wxDefaultPosition, wxSize(-1, -1)), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	boxsizer->Add(height, 1, wxALL, 5);
	boxsizer->Add(resolutionFromVideo, 2, wxALL, 5);

	boxsizer3->Add(new KaiStaticText(this, -1, _("Layout"), wxDefaultPosition, wxSize(-1, -1)), 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	boxsizer3->Add(layoutWidth, 1, wxALL, 5);
	boxsizer3->Add(new KaiStaticText(this, -1, L"  X  ", wxDefaultPosition, wxSize(-1, -1)), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	boxsizer3->Add(layoutHeight, 1, wxALL, 5);
	boxsizer3->Add(layoutFromVideo, 2, wxALL, 5);

	boxsizer4->Add(boxsizer, 0, wxEXPAND);
	boxsizer4->Add(boxsizer3, 0, wxEXPAND);
	boxsizer5->Add(boxsizer4, 0, wxEXPAND);
	boxsizer5->Add(linkResolutions, 0, wxTOP | wxBOTTOM | wxRIGHT | wxEXPAND, 5);
	StaticBox2->Add(boxsizer5, 0, wxEXPAND);

	matrix = new KaiChoice(this, -1, wxDefaultPosition, wxSize(160, -1));
	matrix->SetSelection(matrix->Append(_("None")));
	matrix->Append("TV.601");
	matrix->Append("PC.601");
	matrix->Append("TV.709");
	matrix->Append("PC.709");
	matrix->Append("TV.FCC");
	matrix->Append("PC.FCC");
	matrix->Append("TV.240M");
	matrix->Append("PC.240M");
	wxBoxSizer *boxsizer2 = new wxBoxSizer(wxHORIZONTAL);
	boxsizer2->Add(new KaiStaticText(this, -1, _("YCbCr matrix"), wxDefaultPosition), 1);
	boxsizer2->Add(matrix, 2, wxLEFT, 3);
	StaticBox2->Add(boxsizer2, 1, wxEXPAND | wxALL, 5);

	KaiStaticBoxSizer *StaticBox3 = new KaiStaticBoxSizer(wxVERTICAL, this, _("Options"));
	wxBoxSizer* boxsizer6 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* boxsizer7 = new wxBoxSizer(wxHORIZONTAL);
	//wxGridSizer *GridSizer1 = new wxGridSizer(2, 5, 5);

	wrapstyle = new KaiChoice(this, -1/*, wxDefaultPosition, wxSize(160,-1)*/);
	wrapstyle->SetSelection(wrapstyle->Append(_("0: Auto, top line wider")));
	wrapstyle->Append(_("1: End-of-line wrapping, only \\N breaks"));
	wrapstyle->Append(_("2: No wrapping, both \\n and \\N break"));
	wrapstyle->Append(_("3: Auto, bottom line wider"));
	collision = new KaiChoice(this, -1);
	collision->SetSelection(collision->Append(_("Normal")));
	collision->Append(_("Reversed"));

	boxsizer6->Add(new KaiStaticText(this, -1, _("Wrap style")), 1, wxEXPAND | wxLEFT, 5);
	boxsizer6->Add(wrapstyle, 2, wxEXPAND | wxALL, 5);
	boxsizer7->Add(new KaiStaticText(this, -1, _("Colliding lines")), 1, wxEXPAND | wxLEFT, 5);
	boxsizer7->Add(collision, 2, wxEXPAND | wxALL, 5);

	scaleBorderAndShadow = new KaiCheckBox(this, -1, _("Scale border and shadow"));
	scaleBorderAndShadow->SetValue(true);

	StaticBox3->Add(boxsizer6, 1, wxEXPAND);
	StaticBox3->Add(boxsizer7, 1, wxEXPAND);
	StaticBox3->Add(scaleBorderAndShadow, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer *boxsizer1 = new wxBoxSizer(wxHORIZONTAL);
	save = new MappedButton(this, wxID_OK, _("Save"));
	cancel = new MappedButton(this, wxID_CANCEL, _("Cancel"));
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
	Connect(25457, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ScriptInfo::OnLayoutRes);
	Connect(25458, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, (wxObjectEventFunction)&ScriptInfo::OnResolutionLink);
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
	tmp = emptyString;
	height->SetValue(tmp << res.y);
	width->SetModified(true);
	height->SetModified(true);
}

void ScriptInfo::OnLayoutRes(wxCommandEvent& event)
{
	wxString tmp;
	layoutWidth->SetValue(tmp << res.x);
	tmp = emptyString;
	layoutHeight->SetValue(tmp << res.y);
	layoutWidth->SetModified(true);
	layoutHeight->SetModified(true);
}

void ScriptInfo::OnResolutionLink(wxCommandEvent& event)
{
	bool toggled = linkResolutions->GetValue();
	Options.SetBool(LINK_RESOLUTIONS, toggled);
	layoutWidth->Enable(!toggled);
	layoutHeight->Enable(!toggled);
	layoutFromVideo->Enable(!toggled && res.x > 0);
}

void ScriptInfo::DoTooltips()
{
	height->SetToolTip(_("Video height"));
	width->SetToolTip(_("Video width"));
	wrapstyle->SetToolTip(_("How to wrap lines"));
	collision->SetToolTip(_("Colliding lines"));
	scaleBorderAndShadow->SetToolTip(_("Scale border and shadow"));
}