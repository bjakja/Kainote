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

SubsResampleDialog::SubsResampleDialog(wxWindow *parent, const wxSize &subsSize, const wxSize &videoSize, const wxString &videoMatrix)
	: KaiDialog(parent, -1, _("Zmieñ rozdzielczoœæ"))
{
	DialogSizer *mainSizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *subsResolutionSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *videoResolutionSizer = new wxBoxSizer(wxHORIZONTAL);
	KaiStaticBoxSizer *subsResolutionStaticSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Rozdzielczoœæ napisów"));
	KaiStaticBoxSizer *videoResolutionStaticSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Rozdzielczoœæ docelowa"));
	wxString matrices[] = {"TV.601", "PC.601", "TV.709", "PC.709", "TV.FCC", "PC.FCC", "TV.240M", "PC.240M"};
	subsResolutionX = new NumCtrl(this,26543,std::to_string(subsSize.x),100, 13000, true);
	subsResolutionY = new NumCtrl(this,26544,std::to_string(subsSize.y),100, 10000, true);
	MappedButton *fromSubs = new MappedButton(this, 26547, _("Pobierz z napisów"), 0);
	fromSubs->Enable(false);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		subsResolutionX->SetInt(subsSize.x);
		subsResolutionX->SetInt(subsSize.y);
		fromSubs->Enable(false);
	}, 26547);
	
	subsMatrix = new KaiChoice(this, -1 , wxDefaultPosition, wxSize(160,-1), 8, matrices);
	subsResolutionSizer->Add(subsResolutionX,1, wxALL, 2);
	subsResolutionSizer->Add(new wxStaticText(this, -1, " X "),0, wxALL, 2);
	subsResolutionSizer->Add(subsResolutionY,1, wxALL, 2);
	subsResolutionSizer->Add(fromSubs,0, wxALL, 2);
	subsResolutionStaticSizer->Add(subsResolutionSizer,0);
	subsResolutionStaticSizer->Add(subsMatrix,0, wxALL, 2);

	destinedResolutionX = new NumCtrl(this,26545,std::to_string(videoSize.x),100, 13000, true);
	destinedResolutionY = new NumCtrl(this,26546,std::to_string(videoSize.y),100, 10000, true);
	MappedButton *fromVideo = new MappedButton(this, 26548, _("Pobierz z wideo"), 0);
	fromVideo->Enable(false);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		destinedResolutionX->SetInt(videoSize.x);
		destinedResolutionY->SetInt(videoSize.y);
		fromVideo->Enable(false);
	}, 26548);
	destinedMatrix = new KaiChoice(this, -1 , wxDefaultPosition, wxSize(160,-1), 8, matrices);
	videoResolutionSizer->Add(destinedResolutionX,1, wxALL, 2);
	videoResolutionSizer->Add(new wxStaticText(this, -1, " X "),0, wxALL, 2);
	videoResolutionSizer->Add(destinedResolutionY,1, wxALL, 2);
	videoResolutionSizer->Add(fromVideo,0, wxALL, 2);
	videoResolutionStaticSizer->Add(videoResolutionSizer,0);
	videoResolutionStaticSizer->Add(subsMatrix,0, wxALL, 2);

	wxArrayString options;
	options.Add(_("Nie rozci¹gaj"));
	options.Add(_("Rozci¹gaj"));
	resamplingOptions = new KaiRadioBox(this, -1, _("Opcje skalowania"),wxDefaultPosition, wxSize(160,-1), options);
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

	mainSizer->Add(subsResolutionStaticSizer,1, wxALL, 2);
	mainSizer->Add(videoResolutionStaticSizer,1, wxALL, 2);
	mainSizer->Add(resamplingOptions,0, wxALL, 2);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *OK = new MappedButton(this, 26548, "OK", 0);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		int subsSizeX = subsResolutionX->GetInt();
		int subsSizeY = subsResolutionY->GetInt();
		int videoSizeX = destinedResolutionX->GetInt();
		int videoSizeY = destinedResolutionY->GetInt();
		Notebook::GetTab()->Grid1->ResizeSubs(videoSizeX/(float)subsSizeX,
			videoSizeX/(float)subsSizeX, resamplingOptions->IsEnabled() && 
			resamplingOptions->GetSelection() == 1);
	},26548);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"), 0);
	buttonSizer->Add(OK, 0, wxALL, 2);
	buttonSizer->Add(Cancel, 0, wxALL, 2);
	mainSizer->Add(buttonSizer,0, wxALL|wxCENTER, 2);
	SetSizer(mainSizer);
	CenterOnParent();
	SetEnterId(26548);
}

//void SubsResampleDialog::OnChangeSubsResolution(wxCommandEvent &evt)
//{
//
//}
//	
//void SubsResampleDialog::OnChangeVideoResolution(wxCommandEvent &evt)
//{
//
//}

SubsMismatchResolutionDialog::SubsMismatchResolutionDialog(wxWindow *parent, const wxSize &subsSize, const wxSize &videoSize)
	: KaiDialog(parent, -1, _("Niezgodna rozdzielczoœæ"))
{
	DialogSizer *mainSizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxVERTICAL);
	wxString info= wxString::Format(_("Rozdzielczoœci wideo i napisów ró¿ni¹ siê."
		L"\nMo¿esz zmieniæ je teraz lub skorzystaæ z opcji we w³aœciwoœciach ASS.\n\n"
		L"Rozdzielczoœæ wideo: %i x %i\nRozdzielczoœæ napisów: %i x %i\n\n"
		L"Dopasowaæ rozdzielczoœæ do wideo?\n"),videoSize.x, videoSize.y, subsSize.x, subsSize.y);
	float resizeX = (videoSize.x/(float)subsSize.x);
	float resizeY = (videoSize.y/(float)subsSize.y);

	wxArrayString options;
	options.Add(_("Zmieñ wy³¹cznie rozdzielczoœæ napisów"));
	options.Add(_("Dopasuj skrypt napisów do rozdzielczoœci wideo (Nie rozci¹gaj)"));
	if(resizeX != resizeY){
		options.Add(_("Dopasuj skrypt napisów do rozdzielczoœci wideo (Rozci¹gnij)"));
	}
	resamplingOptions = new KaiRadioBox(this, -1, _("Opcje skalowania"),wxDefaultPosition, wxSize(160,-1), options);
	
	MappedButton *OK = new MappedButton(this, 26548, "OK", 0);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Notebook::GetTab()->Grid1->ResizeSubs(resizeX,resizeY, 
			resamplingOptions->GetSelection() == 2);
	},26548);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"), 0);
	MappedButton *TurnOff = new MappedButton(this, 26549, _("Wy³¹cz ostrze¿enie"), 0);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Grid *grid = Notebook::GetTab()->Grid1;
		grid->AddSInfo("PlayResX",std::to_string(videoSize.x));
		grid->AddSInfo("PlayResY",std::to_string(videoSize.y));
		if(resamplingOptions->GetSelection() != 0){
		grid->ResizeSubs(resizeX,resizeY, 
			resamplingOptions->GetSelection() == 2);
		}
		grid->SetModified();
		((kainoteFrame *)parent)->SetSubsResolution();
	},26548);
	buttonSizer->Add(OK, 0, wxALL, 2);
	buttonSizer->Add(Cancel, 0, wxALL, 2);
	buttonSizer->Add(TurnOff, 0, wxALL, 2);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Options.SetBool(DontAskForBadResolution,true);
		Options.SaveOptions(true,false);
	},26549);

	mainSizer->Add(new wxStaticText(this,-1, info), 0, wxALL, 2);
	mainSizer->Add(resamplingOptions, 0, wxALL, 2);
	mainSizer->Add(buttonSizer, 0, wxALL|wxCENTER, 2);
	SetSizer(mainSizer);
	CenterOnParent();
	SetEnterId(26548);
}