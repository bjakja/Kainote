﻿//  Copyright (c) 2016 - 2020, Marcin Drob

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


#include "SubsResampleDialog.h"
#include "KaiStaticBoxSizer.h"
#include "kainoteFrame.h"
#include "SubsGrid.h"
#include "Notebook.h"
#include "KaiStaticText.h"
#include "TabPanel.h"

SubsResampleDialog::SubsResampleDialog(wxWindow *parent, const wxSize &subsSize, const wxSize &videoSize, const wxString &subsMatrix, const wxString &videoMatrix)
	: KaiDialog(parent, -1, _("Zmień rozdzielczość"))
{
	DialogSizer *mainSizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *subsResolutionSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *videoResolutionSizer = new wxBoxSizer(wxHORIZONTAL);
	KaiStaticBoxSizer *subsResolutionStaticSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Rozdzielczość napisów"));
	KaiStaticBoxSizer *videoResolutionStaticSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Rozdzielczość docelowa"));

	subsResolutionX = new NumCtrl(this, 26543, std::to_wstring(subsSize.x), 100, 13000, true, wxDefaultPosition, wxSize(60, -1));
	subsResolutionY = new NumCtrl(this, 26544, std::to_wstring(subsSize.y), 100, 10000, true, wxDefaultPosition, wxSize(60, -1));
	MappedButton *fromSubs = new MappedButton(this, 26547, _("Pobierz z napisów"));
	fromSubs->Enable(false);

#ifdef whithMatrix	
	wxString matrices[] = {L"TV.601", L"PC.601", L"TV.709", L"PC.709", L"TV.FCC", L"PC.FCC", L"TV.240M", L"PC.240M"};
	subsMatrix = new KaiChoice(this, -1 , wxDefaultPosition, wxSize(160,-1), 8, matrices);
#endif
	subsResolutionSizer->Add(subsResolutionX, 0, wxALL, 2);
	subsResolutionSizer->Add(new KaiStaticText(this, -1, L" x "), 0, wxALL | wxALIGN_CENTER, 2);
	subsResolutionSizer->Add(subsResolutionY, 0, wxALL, 2);
	subsResolutionSizer->Add(fromSubs, 1, wxALL | wxEXPAND, 2);
	subsResolutionStaticSizer->Add(subsResolutionSizer, 1, wxEXPAND);
#ifdef whithMatrix	
	subsResolutionStaticSizer->Add(subsMatrix,0, wxALL, 2);
#endif

	destinedResolutionX = new NumCtrl(this, 26545, std::to_string(videoSize.x), 100, 13000, true, wxDefaultPosition, wxSize(60, -1));
	destinedResolutionY = new NumCtrl(this, 26546, std::to_string(videoSize.y), 100, 10000, true, wxDefaultPosition, wxSize(60, -1));
	MappedButton *fromVideo = new MappedButton(this, 26548, _("Pobierz z wideo"));
	fromVideo->Enable(false);

#ifdef whithMatrix	
	destinedMatrix = new KaiChoice(this, -1 , wxDefaultPosition, wxSize(160,-1), 8, matrices);
#endif
	videoResolutionSizer->Add(destinedResolutionX, 0, wxALL, 2);
	videoResolutionSizer->Add(new KaiStaticText(this, -1, L" x "), 0, wxALL | wxALIGN_CENTER, 2);
	videoResolutionSizer->Add(destinedResolutionY, 0, wxALL, 2);
	videoResolutionSizer->Add(fromVideo, 1, wxALL | wxEXPAND, 2);
	videoResolutionStaticSizer->Add(videoResolutionSizer, 1, wxEXPAND);
#ifdef whithMatrix
	videoResolutionStaticSizer->Add(subsMatrix,0, wxALL, 2);
#endif

	wxArrayString options;
	options.Add(_("Nie rozciągaj"));
	options.Add(_("Rozciągaj"));
	resamplingOptions = new KaiRadioBox(this, -1, _("Opcje skalowania"), wxDefaultPosition, wxDefaultSize, options);
	resamplingOptions->Enable((videoSize.x / (float)subsSize.x) != (videoSize.y / (float)subsSize.y));
	auto OnChangedResolution = [=](wxCommandEvent &evt)->void{
		int subsSizeX = subsResolutionX->GetInt();
		int subsSizeY = subsResolutionY->GetInt();
		if (!fromSubs->IsEnabled() && (subsSizeX != subsSize.x || subsSizeY != subsSize.y)){
			fromSubs->Enable(true);
		}
		else if (fromSubs->IsEnabled() && subsSizeX == subsSize.x && subsSizeY == subsSize.y){
			fromSubs->Enable(false);
		}
		bool changedAspectRatio = (subsSizeX / (float)videoSize.x) != (subsSizeY / (float)videoSize.y);
		if (!resamplingOptions->IsEnabled() && changedAspectRatio){
			resamplingOptions->Enable();
		}
		else if (resamplingOptions->IsEnabled() && !changedAspectRatio){
			resamplingOptions->Enable(false);
		}
	};
	Bind(NUMBER_CHANGED, OnChangedResolution, 26543, 26544);
	//from subs button bind
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		subsResolutionX->SetInt(subsSize.x);
		subsResolutionY->SetInt(subsSize.y);
		fromSubs->Enable(false);
		wxCommandEvent event;
		OnChangedResolution(event);
	}, 26547);

	auto OnChangedVideoResolution = [=](wxCommandEvent &evt)->void{
		int videoSizeX = destinedResolutionX->GetInt();
		int videoSizeY = destinedResolutionY->GetInt();
		if (!fromVideo->IsEnabled() && (videoSizeX != subsSize.x || videoSizeY != subsSize.y)){
			fromVideo->Enable(true);
		}
		else if (fromVideo->IsEnabled() && videoSizeX == subsSize.x && videoSizeY == subsSize.y){
			fromVideo->Enable(false);
		}
		bool changedAspectRatio = (videoSizeX / (float)subsSize.x) != (videoSizeY / (float)subsSize.y);
		if (!resamplingOptions->IsEnabled() && changedAspectRatio){
			resamplingOptions->Enable();
		}
		else if (resamplingOptions->IsEnabled() && !changedAspectRatio){
			resamplingOptions->Enable(false);
		}
	};
	Bind(NUMBER_CHANGED, OnChangedVideoResolution, 26545, 26546);
	//from video button bind
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		destinedResolutionX->SetInt(videoSize.x);
		destinedResolutionY->SetInt(videoSize.y);
		fromVideo->Enable(false);
		wxCommandEvent cmdEvt;
		OnChangedVideoResolution(cmdEvt);
	}, 26548);

	mainSizer->Add(subsResolutionStaticSizer, 1, wxALL | wxEXPAND, 2);
	mainSizer->Add(videoResolutionStaticSizer, 1, wxALL | wxEXPAND, 2);
	mainSizer->Add(resamplingOptions, 0, wxALL | wxEXPAND, 2);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *OK = new MappedButton(this, 6548, L"OK");
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		int subsSizeX = subsResolutionX->GetInt();
		int subsSizeY = subsResolutionY->GetInt();
		int videoSizeX = destinedResolutionX->GetInt();
		int videoSizeY = destinedResolutionY->GetInt();
		if (subsSizeX == videoSizeX && subsSizeY == videoSizeY){
			return;
		}
		SubsGrid *grid = Notebook::GetTab()->grid;
		grid->AddSInfo(L"PlayResX", std::to_wstring(videoSizeX));
		grid->AddSInfo(L"PlayResY", std::to_wstring(videoSizeY));
		grid->ResizeSubs(videoSizeX / (float)subsSizeX,
			videoSizeY / (float)subsSizeY, resamplingOptions->IsEnabled() &&
			resamplingOptions->GetSelection() == 1);
		grid->SetModified(SUBTITLES_RESAMPLE);
		grid->Refresh(false);
		((KainoteFrame *)parent)->SetSubsResolution();
		EndModal(0);
	}, 6548);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	buttonSizer->Add(OK, 1, wxALL, 2);
	buttonSizer->Add(Cancel, 1, wxALL, 2);
	mainSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 2);
	SetSizerAndFit(mainSizer);
	CenterOnParent();
	SetEnterId(6548);
}


SubsMismatchResolutionDialog::SubsMismatchResolutionDialog(wxWindow *parent, const wxSize &subsSize, const wxSize &videoSize)
	: KaiDialog(parent, -1, _("Niezgodna rozdzielczość"))
{
	DialogSizer *mainSizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString info = wxString::Format(_("Rozdzielczości wideo i napisów się różnią.\nMożesz zmienić je teraz lub skorzystać ze 'zmień rozdzielczość napisów'.\n\nRozdzielczość wideo: %i x %i\nRozdzielczość napisów: %i x %i\n\nDopasować rozdzielczość do wideo?\n"), videoSize.x, videoSize.y, subsSize.x, subsSize.y);
	float resizeX = (videoSize.x / (float)subsSize.x);
	float resizeY = (videoSize.y / (float)subsSize.y);

	wxArrayString options;
	options.Add(_("Zmień wyłącznie rozdzielczość napisów"));
	options.Add(_("Dopasuj skrypt napisów do rozdzielczości wideo (Nie rozciągaj)"));
	if (resizeX != resizeY){
		options.Add(_("Dopasuj skrypt napisów do rozdzielczości wideo (Rozciągnij)"));
	}
	resamplingOptions = new KaiRadioBox(this, -1, _("Opcje skalowania"), wxDefaultPosition, wxSize(160, -1), options);
	resamplingOptions->SetSelection(1);
	MappedButton *OK = new MappedButton(this, 26548, _("Zmień"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Notebook::GetTab()->grid->ResizeSubs(resizeX, resizeY,
			resamplingOptions->GetSelection() == 2);
	}, 26548);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Nie zmieniaj"));
	MappedButton *TurnOff = new MappedButton(this, 26549, _("Wyłącz ostrzeżenie"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		SubsGrid *grid = Notebook::GetTab()->grid;
		grid->AddSInfo(L"PlayResX", std::to_wstring(videoSize.x));
		grid->AddSInfo(L"PlayResY", std::to_wstring(videoSize.y));
		if (resamplingOptions->GetSelection() != 0){
			grid->ResizeSubs(resizeX, resizeY, resamplingOptions->GetSelection() == 2);
		}
		grid->SetModified(SUBTITLES_RESAMPLE);
		((KainoteFrame *)parent)->SetSubsResolution();
		EndModal(0);
	}, 26548);
	buttonSizer->Add(OK, 0, wxALL, 2);
	buttonSizer->Add(Cancel, 0, wxALL, 2);
	buttonSizer->Add(TurnOff, 0, wxALL, 2);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Options.SetBool(DONT_ASK_FOR_BAD_RESOLUTION, true);
		Options.SaveOptions(true, false);
		EndModal(0);
	}, 26549);

	mainSizer->Add(new KaiStaticText(this, -1, info), 0, wxALL, 5);
	mainSizer->Add(resamplingOptions, 0, wxALL, 2);
	mainSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 2);
	SetSizerAndFit(mainSizer);
	CenterOnParent();
	SetEnterId(26548);
}

