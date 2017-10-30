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


#include "SubsResampleDialog.h"
#include "KaiStaticBoxSizer.h"
#include "kainoteMain.h"

SubsResampleDialog::SubsResampleDialog(wxWindow *parent, const wxSize &subsSize, const wxSize &videoSize, const wxString &subsMatrix, const wxString &videoMatrix)
	: KaiDialog(parent, -1, _("Zmień rozdzielczość"))
{
	DialogSizer *mainSizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *subsResolutionSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *videoResolutionSizer = new wxBoxSizer(wxHORIZONTAL);
	KaiStaticBoxSizer *subsResolutionStaticSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Rozdzielczość napisów"));
	KaiStaticBoxSizer *videoResolutionStaticSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Rozdzielczość docelowa"));
	
	subsResolutionX = new NumCtrl(this,26543,std::to_string(subsSize.x),100, 13000, true, wxDefaultPosition, wxSize(60,-1));
	subsResolutionY = new NumCtrl(this,26544,std::to_string(subsSize.y),100, 10000, true, wxDefaultPosition, wxSize(60,-1));
	MappedButton *fromSubs = new MappedButton(this, 26547, _("Pobierz z napisów"));
	fromSubs->Enable(false);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		subsResolutionX->SetInt(subsSize.x);
		subsResolutionX->SetInt(subsSize.y);
		fromSubs->Enable(false);
	}, 26547);
#ifdef whithMatrix	
	wxString matrices[] = {"TV.601", "PC.601", "TV.709", "PC.709", "TV.FCC", "PC.FCC", "TV.240M", "PC.240M"};
	subsMatrix = new KaiChoice(this, -1 , wxDefaultPosition, wxSize(160,-1), 8, matrices);
#endif
	subsResolutionSizer->Add(subsResolutionX,0, wxALL, 2);
	subsResolutionSizer->Add(new KaiStaticText(this, -1, " x "),0, wxALL|wxALIGN_CENTER, 2);
	subsResolutionSizer->Add(subsResolutionY,0, wxALL, 2);
	subsResolutionSizer->Add(fromSubs,1, wxALL|wxEXPAND, 2);
	subsResolutionStaticSizer->Add(subsResolutionSizer,1,wxEXPAND);
#ifdef whithMatrix	
	subsResolutionStaticSizer->Add(subsMatrix,0, wxALL, 2);
#endif

	destinedResolutionX = new NumCtrl(this,26545,std::to_string(videoSize.x),100, 13000, true, wxDefaultPosition, wxSize(60,-1));
	destinedResolutionY = new NumCtrl(this,26546,std::to_string(videoSize.y),100, 10000, true, wxDefaultPosition, wxSize(60,-1));
	MappedButton *fromVideo = new MappedButton(this, 26548, _("Pobierz z wideo"));
	fromVideo->Enable(false);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		destinedResolutionX->SetInt(videoSize.x);
		destinedResolutionY->SetInt(videoSize.y);
		fromVideo->Enable(false);
	}, 26548);
#ifdef whithMatrix	
	destinedMatrix = new KaiChoice(this, -1 , wxDefaultPosition, wxSize(160,-1), 8, matrices);
#endif
	videoResolutionSizer->Add(destinedResolutionX,0, wxALL, 2);
	videoResolutionSizer->Add(new KaiStaticText(this, -1, " x "),0, wxALL|wxALIGN_CENTER, 2);
	videoResolutionSizer->Add(destinedResolutionY,0, wxALL, 2);
	videoResolutionSizer->Add(fromVideo,1, wxALL|wxEXPAND, 2);
	videoResolutionStaticSizer->Add(videoResolutionSizer,1,wxEXPAND);
#ifdef whithMatrix
	videoResolutionStaticSizer->Add(subsMatrix,0, wxALL, 2);
#endif

	wxArrayString options;
	options.Add(_("Nie rozciągaj"));
	options.Add(_("Rozciągaj"));
	resamplingOptions = new KaiRadioBox(this, -1, _("Opcje skalowania"),wxDefaultPosition,wxDefaultSize, options);
	resamplingOptions->Enable((videoSize.x/(float)subsSize.x) != (videoSize.y/(float)subsSize.y));
	auto OnChangedResolution = [=](wxCommandEvent &evt)->void{
		int subsSizeX = subsResolutionX->GetInt();
		int subsSizeY = subsResolutionY->GetInt();
		if(!fromSubs-> IsEnabled() && (subsSizeX != subsSize.x || subsSizeY != subsSize.y)){
			fromSubs->Enable(true);
		}else if(fromSubs-> IsEnabled() && subsSizeX == subsSize.x && subsSizeY == subsSize.y){
			fromSubs->Enable(false);
		}
		bool changedAspectRatio = (subsSizeX/(float)videoSize.x) != (subsSizeY/(float)videoSize.y);
		if(!resamplingOptions->IsEnabled() && changedAspectRatio){
			resamplingOptions->Enable();
		}else if(resamplingOptions->IsEnabled() && !changedAspectRatio){
			resamplingOptions->Enable(false);
		}
	};
	Bind(NUMBER_CHANGED, OnChangedResolution, 26543, 26544);

	auto OnChangedVideoResolution = [=](wxCommandEvent &evt)->void{
		int videoSizeX = destinedResolutionX->GetInt();
		int videoSizeY = destinedResolutionY->GetInt();
		if(!fromVideo-> IsEnabled() && (videoSizeX != subsSize.x || videoSizeY != subsSize.y)){
			fromVideo->Enable(true);
		}else if(fromVideo-> IsEnabled() && videoSizeX == subsSize.x && videoSizeY == subsSize.y){
			fromVideo->Enable(false);
		}
		bool changedAspectRatio = (videoSizeX/(float)subsSize.x) != (videoSizeY/(float)subsSize.y);
		if(!resamplingOptions->IsEnabled() && changedAspectRatio){
			resamplingOptions->Enable();
		}else if(resamplingOptions->IsEnabled() && !changedAspectRatio){
			resamplingOptions->Enable(false);
		}
	};
	Bind(NUMBER_CHANGED, OnChangedVideoResolution, 26545, 26546);

	mainSizer->Add(subsResolutionStaticSizer,1, wxALL|wxEXPAND, 2);
	mainSizer->Add(videoResolutionStaticSizer,1, wxALL|wxEXPAND, 2);
	mainSizer->Add(resamplingOptions,0, wxALL|wxEXPAND, 2);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *OK = new MappedButton(this, 6548, "OK");
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		int subsSizeX = subsResolutionX->GetInt();
		int subsSizeY = subsResolutionY->GetInt();
		int videoSizeX = destinedResolutionX->GetInt();
		int videoSizeY = destinedResolutionY->GetInt();
		Grid *grid = Notebook::GetTab()->Grid1;
		grid->AddSInfo("PlayResX",std::to_string(videoSizeX));
		grid->AddSInfo("PlayResY",std::to_string(videoSizeY));
		grid->ResizeSubs(videoSizeX/(float)subsSizeX,
			videoSizeX/(float)subsSizeX, resamplingOptions->IsEnabled() && 
			resamplingOptions->GetSelection() == 1);
		grid->SetModified(SUBTITLES_RESAMPLE);
		((kainoteFrame *)parent)->SetSubsResolution();
		EndModal(0);
	},6548);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	buttonSizer->Add(OK, 1, wxALL, 2);
	buttonSizer->Add(Cancel, 1, wxALL, 2);
	mainSizer->Add(buttonSizer,0, wxALL|wxCENTER, 2);
	SetSizerAndFit(mainSizer);
	CenterOnParent();
	SetEnterId(6548);
}


SubsMismatchResolutionDialog::SubsMismatchResolutionDialog(wxWindow *parent, const wxSize &subsSize, const wxSize &videoSize)
	: KaiDialog(parent, -1, _("Niezgodna rozdzielczość"))
{
	DialogSizer *mainSizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString info= wxString::Format(_("Rozdzielczości wideo i napisów różnią się."
		L"\nMożesz zmienić je teraz lub skorzystać ze 'zmień rozdzielczość napisów'.\n\n"
		L"Rozdzielczość wideo: %i x %i\nRozdzielczość napisów: %i x %i\n\n"
		L"Dopasować rozdzielczość do wideo?\n"),videoSize.x, videoSize.y, subsSize.x, subsSize.y);
	float resizeX = (videoSize.x/(float)subsSize.x);
	float resizeY = (videoSize.y/(float)subsSize.y);

	wxArrayString options;
	options.Add(_("Zmień wyłącznie rozdzielczość napisów"));
	options.Add(_("Dopasuj skrypt napisów do rozdzielczości wideo (Nie rozciągaj)"));
	if(resizeX != resizeY){
		options.Add(_("Dopasuj skrypt napisów do rozdzielczości wideo (Rozciągnij)"));
	}
	resamplingOptions = new KaiRadioBox(this, -1, _("Opcje skalowania"),wxDefaultPosition, wxSize(160,-1), options);
	resamplingOptions->SetSelection(1);
	MappedButton *OK = new MappedButton(this, 26548, _("Zmień"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Notebook::GetTab()->Grid1->ResizeSubs(resizeX,resizeY, 
			resamplingOptions->GetSelection() == 2);
	},26548);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Nie zmieniaj"));
	MappedButton *TurnOff = new MappedButton(this, 26549, _("Wyłącz ostrzeżenie"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Grid *grid = Notebook::GetTab()->Grid1;
		grid->AddSInfo("PlayResX",std::to_string(videoSize.x));
		grid->AddSInfo("PlayResY",std::to_string(videoSize.y));
		if(resamplingOptions->GetSelection() != 0){
			grid->ResizeSubs(resizeX,resizeY, resamplingOptions->GetSelection() == 2);
		}
		grid->SetModified(SUBTITLES_RESAMPLE);
		((kainoteFrame *)parent)->SetSubsResolution();
		EndModal(0);
	},26548);
	buttonSizer->Add(OK, 0, wxALL, 2);
	buttonSizer->Add(Cancel, 0, wxALL, 2);
	buttonSizer->Add(TurnOff, 0, wxALL, 2);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Options.SetBool(DontAskForBadResolution,true);
		Options.SaveOptions(true,false);
		EndModal(0);
	},26549);

	mainSizer->Add(new KaiStaticText(this,-1, info), 0, wxALL, 5);
	mainSizer->Add(resamplingOptions, 0, wxALL, 2);
	mainSizer->Add(buttonSizer, 0, wxALL|wxCENTER, 2);
	SetSizerAndFit(mainSizer);
	CenterOnParent();
	SetEnterId(26548);
}