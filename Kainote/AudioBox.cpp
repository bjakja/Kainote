// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
// Copyright (c) 2016-2026, Drob Marcin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//
// Normal licence opensource



#include "EditBox.h"
#include "AudioBox.h"
//#include "KainoteFrame.h"
#include "Notebook.h"
#include "MappedButton.h"
#include "KaiMessageBox.h"
#include "KaiSlider.h"
#include "EditBox.h"
//#include "Visuals.h"
//#include "VisualDrawingShapes.h"
//#include "TabPanel.h"
#include "WinUndef.h"
#include <wx/slider.h>
#include <math.h>
//#include <dxgicommon.h>

// Both vertical sliders use a cubic response where position 50 means 100%.
// Fed straight to the player that reaches 8x at the top of the slider, and
// Provider::GetBuffer hard clips the result to 16 bit, so loud material tears
// itself apart long before the slider gets there (issue #450). The waveform
// scale keeps the full cubic range, the player gets a linear 100% - 150% ramp
// across the upper half of the slider instead.
static const float MAX_PLAYBACK_VOLUME = 1.5f;

float AudioDisplayScaleFromSlider(int position)
{
	return pow(float(position) / 50.0f, 3);
}

float PlaybackVolumeFromSlider(int position)
{
	if (position <= 50)
		return AudioDisplayScaleFromSlider(position);

	return 1.0f + ((MAX_PLAYBACK_VOLUME - 1.0f) * (float(position - 50) / 50.0f));
}


AudioBox::AudioBox(wxWindow *parent, SubsGrid *grid) :
	KaiPanel(parent, -1, wxDefaultPosition, wxSize(0, 0))
{
	// Setup
	loaded = false;
	arrows = holding = false;
	oldy = -1;
	int height = Options.GetInt(AUDIO_BOX_HEIGHT);
	SetMinSize(wxSize(-1, height));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	// Display
	int thickness = KaiScrollbar::CalculateThickness(this);
	audioScroll = new KaiScrollbar(this, Audio_Scrollbar, 
		wxPoint(0, height - thickness), wxSize(100, thickness));
	audioScroll->SetToolTip(_("Search bar"));

	audioDisplay = new AudioDisplay(this);

	audioDisplay->ScrollBar = audioScroll;
	audioDisplay->box = this;
	audioDisplay->edit = (EditBox*)parent;
	audioDisplay->grid = grid;
	audioDisplay->tab = (TabPanel *)parent->GetParent();
	// Zoom
	int zoom = Options.GetInt(AUDIO_HORIZONTAL_ZOOM);
	audioDisplay->SetSamplesPercent(zoom, false);
	HorizontalZoom = new KaiSlider(this, Audio_Horizontal_Zoom, 
		zoom, 0, 100, wxDefaultPosition, wxSize(-1, 20), wxSL_VERTICAL | wxSL_BOTH);
	HorizontalZoom->SetToolTip(_("Horizontal stretching"));
	int pos = Options.GetInt(AUDIO_VERTICAL_ZOOM);
	float value = AudioDisplayScaleFromSlider(pos);
	audioDisplay->SetScale(value);
	VerticalZoom = new KaiSlider(this, Audio_Vertical_Zoom, pos, 1, 100, 
		wxDefaultPosition, wxSize(-1, 20), wxSL_VERTICAL | wxSL_BOTH | wxSL_INVERSE);
	VerticalZoom->SetToolTip(_("Vertical stretching"));
	VolumeBar = new KaiSlider(this, Audio_Volume, Options.GetInt(AUDIO_VOLUME), 1, 100, wxDefaultPosition, wxSize(-1, 20), wxSL_VERTICAL | wxSL_BOTH | wxSL_INVERSE);
	VolumeBar->SetToolTip(_("Volume"));
	bool link = Options.GetBool(AUDIO_LINK);
	if (link) {
		int volume = VerticalZoom->GetValue();
		VolumeBar->SetValue(volume);
		Options.SetInt(AUDIO_VOLUME, volume);
	}
	VerticalLink = new ToggleButton(this, Audio_Vertical_Link, emptyString, emptyString, wxDefaultPosition, wxSize(40, -1));
	VerticalLink->SetBitmap(wxBITMAP_PNG(L"button_link"));
	VerticalLink->SetToolTip(_("Link the volume and stretch sliders"));
	VerticalLink->SetValue(link);

	// Display sizer
	DisplaySizer = new wxBoxSizer(wxVERTICAL);
	DisplaySizer->Add(audioDisplay, 1, wxEXPAND, 0);
	DisplaySizer->Add(audioScroll, 0, wxEXPAND | wxBOTTOM, 4);

	// VertVol sider
	wxSizer *VertVol = new wxBoxSizer(wxHORIZONTAL);
	VertVol->Add(VerticalZoom, 1, wxEXPAND, 0);
	VertVol->Add(VolumeBar, 1, wxEXPAND, 0);
	wxSizer *VertVolArea = new wxBoxSizer(wxVERTICAL);
	VertVolArea->Add(VertVol, 1, wxEXPAND, 0);
	VertVolArea->Add(VerticalLink, 0, wxEXPAND | wxBOTTOM, 2);

	// Top sizer
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(DisplaySizer, 1, wxEXPAND, 0);
	TopSizer->Add(HorizontalZoom, 0, wxEXPAND, 0);
	TopSizer->Add(VertVolArea, 0, wxEXPAND, 0);

	// Buttons sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *temp;
	temp = new MappedButton(this, AUDIO_PREVIOUS, emptyString, wxBITMAP_PNG(L"button_prev"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Play the previous line"));
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_NEXT, emptyString, wxBITMAP_PNG(L"button_next"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Play the next line"));
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_PLAY, emptyString, wxBITMAP_PNG(L"BUTTON_PLAY_LINE"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Play the current syllable / line"));
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_PLAY_LINE, emptyString, wxBITMAP_PNG(L"button_playsel"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Play the current line"));
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_STOP, _("Stop playback"), wxBITMAP_PNG(L"button_stop"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 8);
	//ButtonSizer->AddSpacer(2);
	temp = new MappedButton(this, AUDIO_PLAY_BEFORE_MARK, _("Play before the tag"), wxBITMAP_PNG(L"button_playbefore"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_PLAY_AFTER_MARK, _("Play after the tag"), wxBITMAP_PNG(L"button_playafter"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 8);

	temp = new MappedButton(this, AUDIO_PLAY_500MS_BEFORE, _("Play 500ms before the start time"), wxBITMAP_PNG(L"button_playfivehbefore"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_PLAY_500MS_FIRST, _("Play 500 ms after the start time"), wxBITMAP_PNG(L"button_playfirstfiveh"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_PLAY_500MS_LAST, _("Play 500ms before the end time"), wxBITMAP_PNG(L"button_playlastfiveh"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_PLAY_500MS_AFTER, _("Play 500ms after the end time"), wxBITMAP_PNG(L"button_playfivehafter"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_PLAY_TO_END, _("Play to the end"), wxBITMAP_PNG(L"button_playtoend"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 8);

	temp = new MappedButton(this, AUDIO_LEAD_IN, _("Add lead-in to the active line"), wxBITMAP_PNG(L"button_leadin"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_LEAD_OUT, _("Add lead-out to the active line"), wxBITMAP_PNG(L"button_leadout"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 8);

	temp = new MappedButton(this, AUDIO_COMMIT, emptyString, wxBITMAP_PNG(L"button_audio_commit"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Apply changes"));
	ButtonSizer->Add(temp, 0, wxRIGHT, 2);
	temp = new MappedButton(this, AUDIO_GOTO, _("Go to selection"), wxBITMAP_PNG(L"button_audio_goto"), wxDefaultPosition, wxDefaultSize, AUDIO_HOTKEY, MAKE_SQUARE_BUTTON);
	ButtonSizer->Add(temp, 0, wxRIGHT, 8);

	KaraSwitch = new ToggleButton(this, Audio_Button_Karaoke, emptyString, _("Enable / disable karaoke creation"), wxDefaultPosition, wxDefaultSize, MAKE_SQUARE_BUTTON);
	KaraSwitch->SetBitmap(wxBITMAP_PNG(L"button_karaoke"));
	KaraSwitch->SetValue(audioDisplay->hasKara);
	ButtonSizer->Add(KaraSwitch, 0, wxRIGHT, 2);
	KaraMode = new ToggleButton(this, Audio_Button_Split, emptyString, _("Enable / Disable automatic splitting of syllables"), wxDefaultPosition, wxDefaultSize, MAKE_SQUARE_BUTTON);
	KaraMode->SetBitmap(wxBITMAP_PNG(L"button_auto_split"));
	KaraMode->SetValue(audioDisplay->karaAuto);
	ButtonSizer->Add(KaraMode, 0, wxRIGHT, 8);

	AutoCommit = new ToggleButton(this, Audio_Check_AutoCommit, emptyString, _("Automatically apply changes"), wxDefaultPosition, wxDefaultSize, MAKE_SQUARE_BUTTON);
	AutoCommit->SetBitmap(wxBITMAP_PNG(L"button_auto_commit"));
	AutoCommit->SetValue(Options.GetBool(AUDIO_AUTO_COMMIT));
	ButtonSizer->Add(AutoCommit, 0, wxRIGHT, 2);
	NextCommit = new ToggleButton(this, Audio_Check_NextCommit, emptyString, _("Go to the next line after applying changes"), wxDefaultPosition, wxDefaultSize, MAKE_SQUARE_BUTTON);
	NextCommit->SetBitmap(wxBITMAP_PNG(L"button_next_a_commit"));
	NextCommit->SetValue(Options.GetBool(AUDIO_NEXT_LINE_ON_COMMIT));
	ButtonSizer->Add(NextCommit, 0, wxRIGHT, 2);
	AutoScroll = new ToggleButton(this, Audio_Check_AutoGoto, emptyString, _("Auto-scroll to the active line"), wxDefaultPosition, wxDefaultSize, MAKE_SQUARE_BUTTON);
	AutoScroll->SetBitmap(wxBITMAP_PNG(L"button_auto_go"));
	AutoScroll->SetValue(Options.GetBool(AUDIO_AUTO_SCROLL));
	ButtonSizer->Add(AutoScroll, 0, wxRIGHT, 2);
	SpectrumMode = new ToggleButton(this, Audio_Check_Spectrum, emptyString, _("Spectrum mode"), wxDefaultPosition, wxDefaultSize, MAKE_SQUARE_BUTTON);
	SpectrumMode->SetBitmap(wxBITMAP_PNG(L"button_spectrum"));
	SpectrumMode->SetValue(Options.GetBool(AUDIO_SPECTRUM_ON));
	ButtonSizer->Add(SpectrumMode, 0, wxRIGHT, 2);
	SpectrumNonLinear = new ToggleButton(this, Audio_Check_Spectrum_Non_Linear, emptyString, _("Enhance speech frequencies in the spectrum"), wxDefaultPosition, wxDefaultSize, MAKE_SQUARE_BUTTON);
	SpectrumNonLinear->SetBitmap(wxBITMAP_PNG(L"SPECTRUM_NON_LINEAR"));
	SpectrumNonLinear->SetValue(Options.GetBool(AUDIO_SPECTRUM_NON_LINEAR_ON));
	ButtonSizer->Add(SpectrumNonLinear, 0, wxRIGHT, 2);
	ButtonSizer->AddStretchSpacer(1);


	// Main sizer
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer, 1, wxEXPAND, 0);
	MainSizer->Add(ButtonSizer, 0, wxEXPAND, 0);
	MainSizer->AddSpacer(2);
	SetSizer(MainSizer);//}

	SetAccels();
	sliderPositionSave.SetOwner(this, AUDIO_TIMER);
	Bind(wxEVT_TIMER, [=](wxTimerEvent) {
		Options.SaveAudioOpts();
	}, AUDIO_TIMER);
}




////////////
// Set file
void AudioBox::SetFile(wxString file, bool fromvideo) {
	loaded = false;
	audioDisplay->SetFile(file, fromvideo);
	if (file != emptyString) loaded = audioDisplay->loaded;
	audioName = file;
	if (loaded && audioDisplay->player){
		float value = PlaybackVolumeFromSlider(Options.GetInt(AUDIO_VOLUME));
		audioDisplay->player->SetVolume(value);
	}
}



/////////////////////
// Scrollbar changed
void AudioBox::OnScrollbar(wxScrollEvent &event) {
	audioDisplay->SetPosition(event.GetPosition() * 12);
}


///////////////////////////////
// Horizontal zoom bar changed
void AudioBox::OnHorizontalZoom(wxScrollEvent &event) {
	audioDisplay->SetSamplesPercent(event.GetPosition());
	Options.SetInt(AUDIO_HORIZONTAL_ZOOM, event.GetPosition());
	if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
		Options.SaveAudioOpts();
	else if (event.GetEventType() == wxEVT_SCROLL_CHANGED)
		sliderPositionSave.Start(1000, true);
}


/////////////////////////////
// Vertical zoom bar changed
void AudioBox::OnVerticalZoom(wxScrollEvent &event) {
	int pos = event.GetPosition();
	float value = AudioDisplayScaleFromSlider(pos);
	audioDisplay->SetScale(value);
	if (VerticalLink->GetValue()) {
		audioDisplay->player->SetVolume(PlaybackVolumeFromSlider(pos));
		VolumeBar->SetThumbPosition(VerticalZoom->GetThumbPosition());
		Options.SetInt(AUDIO_VOLUME, pos);
	}
	Options.SetInt(AUDIO_VERTICAL_ZOOM, pos);
	if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
		Options.SaveAudioOpts();
	else if (event.GetEventType() == wxEVT_SCROLL_CHANGED)
		sliderPositionSave.Start(1000, true);
}


//////////////////////
// Volume bar changed
void AudioBox::OnVolume(wxScrollEvent &event) {
	int pos = event.GetPosition();
	audioDisplay->player->SetVolume(PlaybackVolumeFromSlider(pos));
	Options.SetInt(AUDIO_VOLUME, pos);
	if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
		Options.SaveAudioOpts();
	else if (event.GetEventType() == wxEVT_SCROLL_CHANGED)
		sliderPositionSave.Start(1000, true);

	if (VerticalLink->GetValue()) {
		VerticalZoom->SetThumbPosition(VolumeBar->GetThumbPosition());
		audioDisplay->SetScale(AudioDisplayScaleFromSlider(pos));
		Options.SetInt(AUDIO_VERTICAL_ZOOM, pos);
	}
}


////////////////////////
// Bars linked/unlinked
void AudioBox::OnVerticalLink(wxCommandEvent &event) {
	int pos = VerticalZoom->GetValue();
	if (pos < 1) pos = 1;
	if (pos > 100) pos = 100;
	if (VerticalLink->GetValue()) {
		audioDisplay->player->SetVolume(PlaybackVolumeFromSlider(pos));
		//VolumeBar->SetValue(pos);
		VolumeBar->SetThumbPosition(VerticalZoom->GetThumbPosition());
		Options.SetInt(AUDIO_VOLUME, pos);
	}

	Options.SetBool(AUDIO_LINK, VerticalLink->GetValue());
	Options.SaveAudioOpts();
}




//////////////////
// Play selection
void AudioBox::OnPlaySelection(wxCommandEvent &event) {
	int start = 0, end = 0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start, end, false, event.GetId() == AUDIO_PLAY_LINE);
	audioDisplay->Play(start, end);
}


/////////////////
// Play dialogue
void AudioBox::OnPlayDialogue(wxCommandEvent &event) {
	int start = 0, end = 0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesDialogue(start, end);
	audioDisplay->SetSelection(start, end);
	audioDisplay->Play(start, end);
}

////////////////
// Stop Playing
void AudioBox::OnStop(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->Stop();
}


////////
// Next
void AudioBox::OnNext(wxCommandEvent &event) {
	bool playAudio = !Options.GetBool(AUDIO_DONT_PLAY_WHEN_LINE_CHANGES);
	if(playAudio)
		audioDisplay->SetFocus();
	audioDisplay->Next(playAudio);
}


////////////
// Previous
void AudioBox::OnPrev(wxCommandEvent &event) {
	bool playAudio = !Options.GetBool(AUDIO_DONT_PLAY_WHEN_LINE_CHANGES);
	if (playAudio)
		audioDisplay->SetFocus();
	audioDisplay->Prev(!Options.GetBool(AUDIO_DONT_PLAY_WHEN_LINE_CHANGES));
}

void AudioBox::OnPlayBeforeMark(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	if (!audioDisplay->hasMark)return;
	int start = audioDisplay->curMarkMS;
	audioDisplay->Play(start - Options.GetInt(AUDIO_MARK_PLAY_TIME), start);
}

void AudioBox::OnPlayAfterMark(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	if (!audioDisplay->hasMark)return;
	int start = audioDisplay->curMarkMS;
	audioDisplay->Play(start, start + Options.GetInt(AUDIO_MARK_PLAY_TIME));
}

/////////////////
// 500 ms before
void AudioBox::OnPlay500Before(wxCommandEvent &event) {
	int start = 0, end = 0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start, end);
	audioDisplay->Play(start - 500, start);
}


////////////////
// 500 ms after
void AudioBox::OnPlay500After(wxCommandEvent &event) {
	int start = 0, end = 0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start, end);
	audioDisplay->Play(end, end + 500);
}


////////////////
// First 500 ms
void AudioBox::OnPlay500First(wxCommandEvent &event) {
	int start = 0, end = 0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start, end);
	int endp = start + 500;
	if (endp > end) endp = end;
	audioDisplay->Play(start, endp);
}


///////////////
// Last 500 ms
void AudioBox::OnPlay500Last(wxCommandEvent &event) {
	int start = 0, end = 0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start, end);
	int startp = end - 500;
	if (startp < start) startp = start;
	audioDisplay->Play(startp, end);
}


////////////////////////
// Start to end of file
void AudioBox::OnPlayToEnd(wxCommandEvent &event) {
	int start = 0, end = 0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start, end);
	audioDisplay->Play(start, -1);
}


//////////////////
// Commit changes
void AudioBox::OnCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->CommitChanges(true);
}


//////////////////
// Toggle karaoke
void AudioBox::OnKaraoke(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->hasKara = !audioDisplay->hasKara;
	int value = -1;
	if (audioDisplay->hasKara){
		lastHorizontalZoom = HorizontalZoom->GetValue();
		if (!audioDisplay->karaoke){ audioDisplay->karaoke = new Karaoke(audioDisplay); }
		audioDisplay->karaoke->Split();
		value = MAX(lastHorizontalZoom - 20, 30);
	}
	else{
		if (lastHorizontalZoom > -1)
			value = lastHorizontalZoom;
		else
			value = MIN(HorizontalZoom->GetValue() + 20, 70);
	}
	if (value > -1){
		audioDisplay->SetSamplesPercent(value);
		HorizontalZoom->SetValue(value);
		Options.SetInt(AUDIO_VERTICAL_ZOOM, value);
	}

	audioDisplay->MakeDialogueVisible();

	Options.SetBool(AUDIO_KARAOKE, audioDisplay->hasKara);
	Options.SaveAudioOpts();
}


////////////////
// Split mode button
void AudioBox::OnSplitMode(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->karaAuto = !audioDisplay->karaAuto;
	if (audioDisplay->hasKara){
		audioDisplay->karaoke->Split();
		audioDisplay->UpdateImage(true);
	}
	Options.SetBool(AUDIO_KARAOKE_SPLIT_MODE, audioDisplay->karaAuto);
	Options.SaveAudioOpts();
}


///////////////
// Goto button
void AudioBox::OnGoto(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->MakeDialogueVisible(true);
}


/////////////
// Auto Goto
void AudioBox::OnAutoGoto(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	Options.SetBool(AUDIO_AUTO_SCROLL, AutoScroll->GetValue());
	Options.SaveAudioOpts();
}


///////////////
// Auto Commit
void AudioBox::OnAutoCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	Options.SetBool(AUDIO_AUTO_COMMIT, AutoCommit->GetValue());
	Options.SaveAudioOpts();
}


//////////////////////
// Next line on Commit
void AudioBox::OnNextLineCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	Options.SetBool(AUDIO_NEXT_LINE_ON_COMMIT, NextCommit->GetValue());
	Options.SaveAudioOpts();
}



//////////////////////////
// Spectrum Analyzer Mode
void AudioBox::OnSpectrumMode(wxCommandEvent &event) {
	audioDisplay->spectrumOn = SpectrumMode->GetValue();
	Options.SetBool(AUDIO_SPECTRUM_ON, audioDisplay->spectrumOn);
	Options.SaveAudioOpts();
	audioDisplay->SetFocus();
	audioDisplay->UpdateImage();
}


void AudioBox::OnSpectrumNonLinear(wxCommandEvent &event)
{
	bool value = SpectrumNonLinear->GetValue();
	if (audioDisplay->spectrumRenderer){
		audioDisplay->spectrumRenderer->SetNonLinear(value);
	}
	Options.SetBool(AUDIO_SPECTRUM_NON_LINEAR_ON, value);
	Options.SaveAudioOpts();
	audioDisplay->SetFocus();
	audioDisplay->UpdateImage();
}

///////////////
// Lead in/out
void AudioBox::OnLeadIn(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->AddLead(true, false);
}

void AudioBox::OnLeadOut(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->AddLead(false, true);
}

void AudioBox::OnScrollSpectrum(wxCommandEvent &event)
{
	long long pos = audioDisplay->Position;
	if (event.GetId() == AUDIO_SCROLL_RIGHT)
		pos -= 50;
	else
		pos += 50;
	audioDisplay->UpdatePosition(pos);
	audioDisplay->UpdateImage();
}

void AudioBox::SetAccels()
{
	if (!Notebook::GetTab()->audioHotkeysLoaded){
		Notebook::GetTab()->SetAccels(true);
	}
	std::vector<wxAcceleratorEntry> entries;
	const std::map<idAndType, hdata> &hkeys = Hkeys.GetHotkeysMap();
	TabPanel *tab = (TabPanel*)GetGrandParent();
	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++){
		//check if it's sorted from lower to higher if yes then change continue to break;
		if (cur->first.Type != AUDIO_HOTKEY){ continue; }
		idAndType itype = cur->first;
		if (itype.id < 2000){
			//do nothing
		}
		else if (itype.id < 3000){
			Bind(wxEVT_COMMAND_MENU_SELECTED, &VideoBox::OnAccelerator, tab->video, itype.id);
		}
		else if (itype.id < 4000){
			Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnAccelerator, tab->edit, itype.id);
		}
		else if (itype.id < 5000){
			Bind(wxEVT_COMMAND_MENU_SELECTED, &SubsGrid::OnAccelerator, tab->grid, itype.id);
		}
		/*else{
			Notebook *nt = Notebook::GetTabs();
			KainoteFrame *Kai = (KainoteFrame *)nt->GetParent();
			Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, Kai, itype.id);
		}*/
		
		entries.push_back(Hkeys.GetHKey(itype));
	}
	wxAcceleratorTable accel(entries.size(), &entries[0]);
	SetAcceleratorTable(accel);

}

void AudioBox::SetVolume(int vol)
{
	audioDisplay->player->SetVolume(PlaybackVolumeFromSlider(vol));
	Options.SetInt(AUDIO_VOLUME, vol);
	Options.SaveAudioOpts();
	VolumeBar->SetValue(vol);
	if (VerticalLink->GetValue()) {
		VerticalZoom->SetThumbPosition(VolumeBar->GetThumbPosition());
		audioDisplay->SetScale(AudioDisplayScaleFromSlider(vol));
	}

}

int AudioBox::GetVolume()
{
	return VolumeBar->GetValue();
}

bool AudioBox::SetFont(const wxFont &font)
{
	int thickness = audioScroll->GetThickness();
	audioScroll->SetMinSize(wxSize(-1, thickness));

	const wxWindowList& siblings = GetChildren();

	for (wxWindowList::compatibility_iterator nodeAfter = siblings.GetFirst();
		nodeAfter;
		nodeAfter = nodeAfter->GetNext()){

		wxWindow *win = nodeAfter->GetData();
		win->SetFont(font);
	}

	wxWindow::SetFont(font);
	return true;
}

bool AudioBox::Show(bool show) {
	audioDisplay->isHidden = !show;
	return wxWindow::Show(show);
}
bool AudioBox::Hide() {
	audioDisplay->isHidden = true;
	return wxWindow::Show(false);
}

void AudioBox::OnAccelerator(wxCommandEvent &event)
{
	int id = event.GetId();
	if (Options.CheckLastKeyEvent(id))
		return;

	switch (id){
	case AUDIO_PLAY: OnPlaySelection(event); break;
	case AUDIO_PLAY_LINE: OnPlaySelection(event); break;
	case AUDIO_STOP: OnStop(event); break;
	case AUDIO_NEXT: OnNext(event); break;
	case AUDIO_PREVIOUS: OnPrev(event); break;
	case AUDIO_PLAY_BEFORE_MARK: OnPlayBeforeMark(event); break;
	case AUDIO_PLAY_AFTER_MARK: OnPlayAfterMark(event); break;
	case AUDIO_PLAY_500MS_BEFORE: OnPlay500Before(event); break;
	case AUDIO_PLAY_500MS_AFTER: OnPlay500After(event); break;
	case AUDIO_PLAY_500MS_FIRST: OnPlay500First(event); break;
	case AUDIO_PLAY_500MS_LAST: OnPlay500Last(event); break;
	case AUDIO_PLAY_TO_END: OnPlayToEnd(event); break;
	case AUDIO_COMMIT: OnCommit(event); break;
	case AUDIO_GOTO: OnGoto(event); break;
	case AUDIO_LEAD_IN: OnLeadIn(event); break;
	case AUDIO_LEAD_OUT: OnLeadOut(event); break;
	case AUDIO_SCROLL_LEFT: OnScrollSpectrum(event); break;
	case AUDIO_SCROLL_RIGHT: OnScrollSpectrum(event); break;
		default:
			break;
	}
}
///////////////
// Event table
BEGIN_EVENT_TABLE(AudioBox, wxPanel)
EVT_COMMAND_SCROLL(Audio_Scrollbar, AudioBox::OnScrollbar)
EVT_COMMAND_SCROLL(Audio_Horizontal_Zoom, AudioBox::OnHorizontalZoom)
EVT_COMMAND_SCROLL(Audio_Vertical_Zoom, AudioBox::OnVerticalZoom)
EVT_COMMAND_SCROLL(Audio_Volume, AudioBox::OnVolume)

EVT_MENU(AUDIO_PLAY, AudioBox::OnPlaySelection)
EVT_MENU(AUDIO_PLAY_LINE, AudioBox::OnPlaySelection)
EVT_MENU(AUDIO_STOP, AudioBox::OnStop)
EVT_MENU(AUDIO_NEXT, AudioBox::OnNext)
EVT_MENU(AUDIO_PREVIOUS, AudioBox::OnPrev)
EVT_MENU(AUDIO_PLAY_BEFORE_MARK, AudioBox::OnPlayBeforeMark)
EVT_MENU(AUDIO_PLAY_AFTER_MARK, AudioBox::OnPlayAfterMark)
EVT_MENU(AUDIO_PLAY_500MS_BEFORE, AudioBox::OnPlay500Before)
EVT_MENU(AUDIO_PLAY_500MS_AFTER, AudioBox::OnPlay500After)
EVT_MENU(AUDIO_PLAY_500MS_FIRST, AudioBox::OnPlay500First)
EVT_MENU(AUDIO_PLAY_500MS_LAST, AudioBox::OnPlay500Last)
EVT_MENU(AUDIO_PLAY_TO_END, AudioBox::OnPlayToEnd)
EVT_MENU(AUDIO_COMMIT, AudioBox::OnCommit)
EVT_MENU(AUDIO_GOTO, AudioBox::OnGoto)
EVT_MENU(AUDIO_LEAD_IN, AudioBox::OnLeadIn)
EVT_MENU(AUDIO_LEAD_OUT, AudioBox::OnLeadOut)
EVT_MENU(AUDIO_SCROLL_LEFT, AudioBox::OnScrollSpectrum)
EVT_MENU(AUDIO_SCROLL_RIGHT, AudioBox::OnScrollSpectrum)

EVT_TOGGLEBUTTON(Audio_Vertical_Link, AudioBox::OnVerticalLink)
EVT_TOGGLEBUTTON(Audio_Button_Karaoke, AudioBox::OnKaraoke)
EVT_TOGGLEBUTTON(Audio_Button_Split, AudioBox::OnSplitMode)
EVT_TOGGLEBUTTON(Audio_Check_AutoGoto, AudioBox::OnAutoGoto)
EVT_TOGGLEBUTTON(Audio_Check_Spectrum, AudioBox::OnSpectrumMode)
EVT_TOGGLEBUTTON(Audio_Check_Spectrum_Non_Linear, AudioBox::OnSpectrumNonLinear)
EVT_TOGGLEBUTTON(Audio_Check_AutoCommit, AudioBox::OnAutoCommit)
EVT_TOGGLEBUTTON(Audio_Check_NextCommit, AudioBox::OnNextLineCommit)
END_EVENT_TABLE()

