//  Copyright (c) 2021, Marcin Drob

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

#include "DummyVideo.h"
#include "SubsDialogue.h"
#include "KaiMessageBox.h"
#include "MappedButton.h"

DummyVideo::DummyVideo(wxWindow* parent)
	:KaiDialog(parent, -1, _("Opcje dummy wideo"))
{
	DialogSizer* main = new DialogSizer(wxVERTICAL);
	wxBoxSizer* resolutionSizer = new wxBoxSizer(wxHORIZONTAL);
	wxArrayString resolutions;
	resolutions.Add(L"640x480 (SD fullscreen)");
	resolutions.Add(L"704x480 (SD anamorphic)");
	resolutions.Add(L"640x360 (SD widescreen)");
	resolutions.Add(L"704x396 (SD widescreen)");
	resolutions.Add(L"640x352 (SD widescreen MOD16)");
	resolutions.Add(L"704x400 (SD widescreen MOD16)");
	resolutions.Add(L"1024x576 (SuperPAL widescreen)");
	resolutions.Add(L"1280x720 (HD 720p)");
	resolutions.Add(L"1920x1080 (HD 1080p)");
	resolutions.Add(L"3840x2160 (4K)");
	wxBoxSizer* resolutionSizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* resolutionSizer2 = new wxBoxSizer(wxHORIZONTAL);
	videoResolution = new KaiChoice(this, ID_VIDEO_RESOLUTION, wxDefaultPosition, wxDefaultSize, resolutions);
	videoResolution->SetSelection(8);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DummyVideo::OnResolutionChoose, this, ID_VIDEO_RESOLUTION);
	videoResolutionWidth = new NumCtrl(this, -1, L"1920", 300, 8000, true);
	videoResolutionHeight = new NumCtrl(this, -1, L"1080", 200, 4500, true);
	resolutionSizer2->Add(videoResolutionWidth, 1, wxRIGHT | wxEXPAND, 4);
	resolutionSizer2->Add(new KaiStaticText(this, -1, L"x"), 0, wxRIGHT | wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL, 4);
	resolutionSizer2->Add(videoResolutionHeight, 1, wxEXPAND, 0);
	resolutionSizer1->Add(videoResolution, 0, wxBOTTOM | wxEXPAND, 8);
	resolutionSizer1->Add(resolutionSizer2, 0, wxEXPAND, 0);
	resolutionSizer->Add(new KaiStaticText(this, -1, _("Rozdzielczość wideo:")), 1, wxALL | wxALIGN_BOTTOM | wxEXPAND, 4);
	resolutionSizer->Add(resolutionSizer1, 2, wxALL | wxEXPAND, 4);

	wxBoxSizer* colorSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* colorSizer1 = new wxBoxSizer(wxHORIZONTAL);

	color = new ButtonColorPicker(this, AssColor(wxString(L"&HFEA32F&")), wxSize(100, -1));
	pattern = new KaiCheckBox(this, -1, _("Siatka"));
	colorSizer1->Add(color, 1, wxRIGHT | wxEXPAND, 4);
	colorSizer1->Add(pattern, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL, 0);
	colorSizer->Add(new KaiStaticText(this, -1, _("Kolor:")), 1, wxALL | wxALIGN_TOP | wxEXPAND, 4);
	colorSizer->Add(colorSizer1, 2, wxALL | wxEXPAND, 4);

	wxBoxSizer* FPSSizer = new wxBoxSizer(wxHORIZONTAL);
	KaiTextValidator valid(wxFILTER_INCLUDE_CHAR_LIST);
	wxArrayString includes;
	includes.Add(_T("0"));
	includes.Add(_T("1"));
	includes.Add(_T("2"));
	includes.Add(_T("3"));
	includes.Add(_T("4"));
	includes.Add(_T("5"));
	includes.Add(_T("6"));
	includes.Add(_T("7"));
	includes.Add(_T("8"));
	includes.Add(_T("9"));
	includes.Add(_T("."));

	valid.SetIncludes(includes);

	wxArrayString FPSes;
	FPSes.Add(L"23.976"); 
	FPSes.Add(L"24"); 
	FPSes.Add(L"25"); 
	FPSes.Add(L"29.97"); 
	FPSes.Add(L"30"); 
	FPSes.Add(L"60");
	frameRate = new KaiChoice(this, -1, L"23.976", wxDefaultPosition, wxDefaultSize, FPSes, 0, valid);
	FPSSizer->Add(new KaiStaticText(this, -1, _("Klatki na sekundę:")), 1, wxALL | wxALIGN_TOP | wxEXPAND, 4);
	FPSSizer->Add(frameRate, 2, wxALL | wxEXPAND, 4);
	wxBoxSizer* durationSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* durationSizer2 = new wxBoxSizer(wxVERTICAL);
	duration = new TimeCtrl(this, -1, L"0:25:00.00");
	//duration in ms 1 500 000
	float frametime = 1000.f / 23.976f;
	int frames = 1500000.f / frametime;
	frameDuration = new KaiStaticText(this, -1, wxString::Format(_("Co daje %i klatek"), frames));
	durationSizer2->Add(duration, 1, wxEXPAND, 0);
	durationSizer2->Add(frameDuration, 1, wxEXPAND, 0);
	durationSizer->Add(new KaiStaticText(this, -1, _("Czas trwania:")), 1, wxALL | wxALIGN_TOP | wxEXPAND, 4);
	durationSizer->Add(durationSizer2, 2, wxALL | wxEXPAND, 4);
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton* OK = new MappedButton(this, wxID_OK, L"OK", -1, wxDefaultPosition, wxSize(100, -1));
	MappedButton* cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"), -1, wxDefaultPosition, wxSize(100, -1));
	buttonSizer->Add(OK, 1, wxALL, 4);
	buttonSizer->Add(cancel, 1, wxALL, 4);

	main->Add(resolutionSizer, 0, wxALL | wxEXPAND, 0);
	main->Add(colorSizer, 0, wxALL | wxEXPAND, 0);
	main->Add(FPSSizer, 0, wxALL | wxEXPAND, 0);
	main->Add(durationSizer, 0, wxALL | wxEXPAND, 0);
	main->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 0);
	SetSizerAndFit(main);
	CenterOnParent();
}

wxString DummyVideo::GetDummyText()
{
	wxString strFPS = frameRate->GetValue();
	double fps = 0;
	if (!strFPS.ToCDouble(&fps)) {
		KaiMessageBox(_("Nieprawidłowa wartość fps."));
		return wxString();
	}
	if (fps < 15 || fps > 120) {
		KaiMessageBox(_("Nieprawidłowa wartość fps."));
		return wxString();
	}
	STime dur = duration->GetTime();

	float frametime = 1000.f / fps;
	int frames = dur.mstime / frametime;
	AssColor fcolor = color->GetColor();
	wxString result = wxString::Format("?dummy:%f:%i:%i:%i:%i:%i:%i:%s",
		(float)fps, frames, videoResolutionWidth->GetInt(), videoResolutionHeight->GetInt(),
		fcolor.r, fcolor.g, fcolor.b, pattern->GetValue()? L"c" : L"");

	return result;
}

void DummyVideo::OnResolutionChoose(wxCommandEvent& evt)
{
	wxString resolution = videoResolution->GetString(videoResolution->GetSelection());
	wxString rest;
	wxString resX = resolution.BeforeFirst('x', &rest);
	wxString resY = rest.BeforeFirst(' ');
	int resx = wxAtoi(resX);
	int resy = wxAtoi(resY);
	videoResolutionWidth->SetInt(resx);
	videoResolutionHeight->SetInt(resy);
}
