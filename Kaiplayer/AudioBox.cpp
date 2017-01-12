// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
// Copyright (c) 2016, Drob Marcin
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

///////////
// Headers
#include "AudioBox.h"
#include "KainoteMain.h"
#include "Config.h"
#include "MappedButton.h"
#include <math.h>

///////////////
// Constructor
//
AudioBox::AudioBox(wxWindow *parent, wxWindow *Wgrid) :
	wxPanel(parent,-1,wxDefaultPosition,wxSize(0,0),wxBORDER_SIMPLE)
{
	SetForegroundColour(Options.GetColour("Window Text"));
	SetBackgroundColour(Options.GetColour("Window Background"));
	// Setup
	loaded = false;
	arrows = holding = false;
	oldy=-1;
	int height = Options.GetInt("Audio Box Height");
	SetMinSize(wxSize(-1,height));
	// Display
	audioScroll = new KaiScrollbar(this,Audio_Scrollbar,wxPoint(0,height-17), wxSize(100, 17));
	//audioScroll->PushEventHandler(new FocusEvent());
	audioScroll->SetToolTip(_("Pasek szukania"));

	audioDisplay = new AudioDisplay(this);

	audioDisplay->ScrollBar = audioScroll;
	audioDisplay->box = this;
	audioDisplay->Edit=(EditBox*)parent;
	audioDisplay->grid=(Grid*)Wgrid;

	// Zoom
	int zoom = Options.GetInt("Audio Horizontal Zoom");
	audioDisplay->SetSamplesPercent(zoom,false);
	HorizontalZoom = new KaiSlider(this,Audio_Horizontal_Zoom,zoom,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH);
	//HorizontalZoom->PushEventHandler(new FocusEvent());
	HorizontalZoom->SetToolTip(_("Rozciągnięcie w poziomie"));
	int pos = Options.GetInt("Audio Vertical Zoom");
	float value = pow(float(pos)/50.0f,3);
	audioDisplay->SetScale(value);
	VerticalZoom = new KaiSlider(this,Audio_Vertical_Zoom,pos,1,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	//VerticalZoom->PushEventHandler(new FocusEvent());
	VerticalZoom->SetToolTip(_("Rozciągnięcie w pionie"));
	VolumeBar = new KaiSlider(this,Audio_Volume,Options.GetInt("Audio Volume"),1,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	//VolumeBar->PushEventHandler(new FocusEvent());
	VolumeBar->SetToolTip(_("Głośność"));
	bool link = Options.GetBool("Audio Link");
	if (link) {
		VolumeBar->SetValue(VerticalZoom->GetValue());
		//VolumeBar->Enable(false);
	}
	VerticalLink = new ToggleButton(this,Audio_Vertical_Link,"","", wxDefaultPosition, wxSize(40,24));
	VerticalLink->SetBitmap(wxBITMAP_PNG("button_link"));
	VerticalLink->SetToolTip(_("Połącz suwak głośności i rozciągnięcia"));
	VerticalLink->SetValue(link);

	// Display sizer
	DisplaySizer = new wxBoxSizer(wxVERTICAL);
	DisplaySizer->Add(audioDisplay,1,wxEXPAND,0);
	//DisplaySizer->Add(audioDisplay,0,wxEXPAND,0);
	DisplaySizer->Add(audioScroll,0,wxEXPAND|wxBOTTOM,4);

	// VertVol sider
	wxSizer *VertVol = new wxBoxSizer(wxHORIZONTAL);
	VertVol->Add(VerticalZoom,1,wxEXPAND,0);
	VertVol->Add(VolumeBar,1,wxEXPAND,0);
	wxSizer *VertVolArea = new wxBoxSizer(wxVERTICAL);
	VertVolArea->Add(VertVol,1,wxEXPAND,0);
	VertVolArea->Add(VerticalLink,0,wxEXPAND,0);

	// Top sizer
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(DisplaySizer,1,wxEXPAND,0);
	TopSizer->Add(HorizontalZoom,0,wxEXPAND,0);
	TopSizer->Add(VertVolArea,0,wxEXPAND,0);

	// Buttons sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *temp;
	temp = new MappedButton(this,AudioPrevious,"",wxBITMAP_PNG("button_prev"),wxDefaultPosition,wxSize(26,26), 'A');
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Odtwórz poprzednią linijkę"));
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioNext,"",wxBITMAP_PNG("button_next"),wxDefaultPosition,wxSize(26,26), 'A');
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Odtwórz następną linijkę"));
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioPlay,"",wxBITMAP_PNG("button_playsel"),wxDefaultPosition,wxSize(26,26), 'A');
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Odtwórz aktualną linijkę"));
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioStop,_("Zatrzymaj odtwarzanie"),wxBITMAP_PNG("button_stop"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,5);
	//ButtonSizer->AddSpacer(2);
	temp = new MappedButton(this,AudioPlayBeforeMark,_("Odtwórz przed znacznikiem"),wxBITMAP_PNG("button_playbefore"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioPlayAfterMark,_("Odtwórz po znaczniku"),wxBITMAP_PNG("button_playafter"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	temp = new MappedButton(this,AudioPlay500MSBefore,_("Odtwórz 500ms przed czasem startowym"),wxBITMAP_PNG("button_playfivehbefore"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioPlay500MSFirst,_("Odtwórz 500ms po czasie startowym"),wxBITMAP_PNG("button_playfirstfiveh"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioPlay500MSLast,_("Odtwórz 500ms przed czasem końcowym"),wxBITMAP_PNG("button_playlastfiveh"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioPlay500MSAfter,_("Odtwórz 500ms po czasie końcowym"),wxBITMAP_PNG("button_playfivehafter"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioPlayToEnd,_("Odtwórz do końca"),wxBITMAP_PNG("button_playtoend"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	temp = new MappedButton(this,AudioLeadin,_("Dodaj wstęp do aktywnej linijki"),wxBITMAP_PNG("button_leadin"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioLeadout,_("Dodaj zakończenie do aktywnej linijki"),wxBITMAP_PNG("button_leadout"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	temp = new MappedButton(this,AudioCommit,"",wxBITMAP_PNG("button_audio_commit"),wxDefaultPosition,wxSize(26,26), 'A');
	temp->SetTwoHotkeys();
	temp->SetToolTip(_("Zatwierdź zmiany"));
	ButtonSizer->Add(temp,0,wxRIGHT,2);
	temp = new MappedButton(this,AudioGoto,_("Przejdź to zaznaczenia"),wxBITMAP_PNG("button_audio_goto"),wxDefaultPosition,wxSize(26,26), 'A');
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	KaraSwitch = new ToggleButton(this,Audio_Button_Karaoke,"",_("Włącz / Wyłącz tworzenie karaoke"), wxDefaultPosition,wxSize(26,26));
	KaraSwitch->SetBitmap(wxBITMAP_PNG("button_karaoke"));
	KaraSwitch->SetValue(audioDisplay->hasKara);
	ButtonSizer->Add(KaraSwitch,0,wxALIGN_CENTER | wxEXPAND | wxRIGHT,2);
	KaraMode = new ToggleButton(this,Audio_Button_Split,"",_("Włącz / Wyłącz automatyczne dzielenie sylab"), wxDefaultPosition,wxSize(26,26));
	KaraMode->SetBitmap(wxBITMAP_PNG("button_auto_split"));
	KaraMode->SetValue(audioDisplay->karaAuto);
	ButtonSizer->Add(KaraMode,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,5);

	AutoCommit = new ToggleButton(this,Audio_Check_AutoCommit,"",_("Automatycznie zatwierdza zmiany"),wxDefaultPosition,wxSize(26,26));
	AutoCommit->SetBitmap(wxBITMAP_PNG("button_auto_commit"));
	AutoCommit->SetValue(Options.GetBool("Audio Autocommit"));
	ButtonSizer->Add(AutoCommit,0,wxALIGN_CENTER | wxEXPAND | wxRIGHT,2);
	NextCommit = new ToggleButton(this,Audio_Check_NextCommit,"",_("Przechodzenie do następnej linijki po zatwierdzeniu zmian"),wxDefaultPosition,wxSize(26,26));
	NextCommit->SetBitmap(wxBITMAP_PNG("button_next_a_commit"));
	NextCommit->SetValue(Options.GetBool("Audio Next Line On Commit"));
	ButtonSizer->Add(NextCommit,0,wxALIGN_CENTER | wxEXPAND | wxRIGHT,2);
	AutoScroll = new ToggleButton(this,Audio_Check_AutoGoto,"",_("Automatyczne przewijanie do aktywnej linijki"), wxDefaultPosition,wxSize(26,26));
	AutoScroll->SetBitmap(wxBITMAP_PNG("button_auto_go"));
	AutoScroll->SetValue(Options.GetBool("Audio Autoscroll"));
	ButtonSizer->Add(AutoScroll,0,wxALIGN_CENTER | wxEXPAND | wxRIGHT,2);
	SpectrumMode = new ToggleButton(this,Audio_Check_Spectrum,"",_("Tryb spektrum"), wxDefaultPosition,wxSize(26,26));
	SpectrumMode->SetBitmap(wxBITMAP_PNG("button_spectrum"));
	SpectrumMode->SetValue(Options.GetBool("Audio Spectrum"));
	ButtonSizer->Add(SpectrumMode,0,wxALIGN_CENTER | wxEXPAND | wxRIGHT,2);
	ButtonSizer->AddStretchSpacer(1);


	// Main sizer
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,1,wxEXPAND,0);
	MainSizer->Add(ButtonSizer,0,wxEXPAND,0);
	MainSizer->AddSpacer(2);
	SetSizer(MainSizer);//}

	SetAccels();
	SetFocusIgnoringChildren();
}


//////////////
// Destructor
AudioBox::~AudioBox() {

	//audioScroll->PopEventHandler(true);
	//HorizontalZoom->PopEventHandler(true);
	//VerticalZoom->PopEventHandler(true);
	//VolumeBar->PopEventHandler(true);

}


////////////
// Set file
void AudioBox::SetFile(wxString file, bool fromvideo) {
	loaded = false;
	audioDisplay->SetFile(file, fromvideo);
	if (file != "") loaded = audioDisplay->loaded;
	audioName = file;
}



/////////////////////
// Scrollbar changed
void AudioBox::OnScrollbar(wxScrollEvent &event) {
	audioDisplay->SetPosition(event.GetPosition()*12);
}


///////////////////////////////
// Horizontal zoom bar changed
void AudioBox::OnHorizontalZoom(wxScrollEvent &event) {
	audioDisplay->SetSamplesPercent(event.GetPosition());
	Options.SetInt("Audio Horizontal Zoom",event.GetPosition());
	if(event.GetEventType()==wxEVT_SCROLL_THUMBRELEASE){
		Options.SaveAudioOpts();
	}
}


/////////////////////////////
// Vertical zoom bar changed
void AudioBox::OnVerticalZoom(wxScrollEvent &event) {
	int pos = event.GetPosition();
	float value = pow(float(pos)/50.0f,3);
	audioDisplay->SetScale(value);
	if (VerticalLink->GetValue()) {
		audioDisplay->player->SetVolume(value);
		VolumeBar->SetThumbPosition(VerticalZoom->GetThumbPosition());
	}
	Options.SetInt("Audio Vertical Zoom",pos);
	if(event.GetEventType()==wxEVT_SCROLL_THUMBRELEASE){
		Options.SaveAudioOpts();
	}
}


//////////////////////
// Volume bar changed
void AudioBox::OnVolume(wxScrollEvent &event) {
	int pos = event.GetPosition();
	float value = pow(float(pos)/50.0f,3);
	audioDisplay->player->SetVolume(value);
	Options.SetInt("Audio Volume",pos);
	if(event.GetEventType()==wxEVT_SCROLL_THUMBRELEASE){
		Options.SaveAudioOpts();
	}
	if (VerticalLink->GetValue()) {
		VerticalZoom->SetThumbPosition(VolumeBar->GetThumbPosition());
		audioDisplay->SetScale(value);
	}
}


////////////////////////
// Bars linked/unlinked
void AudioBox::OnVerticalLink(wxCommandEvent &event) {
	int pos = VerticalZoom->GetValue();
	if (pos < 1) pos = 1;
	if (pos > 100) pos = 100;
	float value = pow(float(pos)/50.0f,3);
	if (VerticalLink->GetValue()) {
		audioDisplay->player->SetVolume(value);
		VolumeBar->SetValue(pos);
	}

	Options.SetBool("Audio Link",VerticalLink->GetValue());
	Options.SaveAudioOpts();
}




//////////////////
// Play selection
void AudioBox::OnPlaySelection(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(start,end);
}


/////////////////
// Play dialogue
void AudioBox::OnPlayDialogue(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesDialogue(start,end);
	audioDisplay->SetSelection(start, end);
	audioDisplay->Play(start,end);
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
	audioDisplay->SetFocus();
	audioDisplay->Next();
}


////////////
// Previous
void AudioBox::OnPrev(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->Prev();
}

void AudioBox::OnPlayBeforeMark(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	if(!audioDisplay->hasMark)return;
	int start=audioDisplay->curMarkMS;
	audioDisplay->Play(start-Options.GetInt("Audio Mark Play Time"),start);
}

void AudioBox::OnPlayAfterMark(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	if(!audioDisplay->hasMark)return;
	int start=audioDisplay->curMarkMS;
	audioDisplay->Play(start,start+Options.GetInt("Audio Mark Play Time"));
}

/////////////////
// 500 ms before
void AudioBox::OnPlay500Before(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(start-500,start);
}


////////////////
// 500 ms after
void AudioBox::OnPlay500After(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(end,end+500);
}


////////////////
// First 500 ms
void AudioBox::OnPlay500First(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	int endp = start+500;
	if (endp > end) endp = end;
	audioDisplay->Play(start,endp);
}


///////////////
// Last 500 ms
void AudioBox::OnPlay500Last(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	int startp = end-500;
	if (startp < start) startp = start;
	audioDisplay->Play(startp,end);
}


////////////////////////
// Start to end of file
void AudioBox::OnPlayToEnd(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(start,-1);
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
	int value=50;
	if(audioDisplay->hasKara){
		if(!audioDisplay->karaoke){audioDisplay->karaoke=new Karaoke(audioDisplay);}
		audioDisplay->karaoke->Split();
		value = MAX(HorizontalZoom->GetValue()-20,30);
		audioDisplay->SetSamplesPercent(value);
		HorizontalZoom->SetValue(value);
	}else{
		value = MIN(HorizontalZoom->GetValue()+20,60);
		audioDisplay->SetSamplesPercent(value);
		HorizontalZoom->SetValue(value);
	}
	Options.SetInt("Audio Vertical Zoom",value);

	audioDisplay->MakeDialogueVisible();

	Options.SetBool("Audio Karaoke",audioDisplay->hasKara);
	Options.SaveAudioOpts();
}


////////////////
// Split mode button
void AudioBox::OnSplitMode(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->karaAuto=!audioDisplay->karaAuto;
	if(audioDisplay->hasKara){
		audioDisplay->karaoke->Split();
		audioDisplay->UpdateImage(true);}
	Options.SetBool("Audio Karaoke Split Mode",audioDisplay->karaAuto);
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
	Options.SetBool("Audio Autoscroll",AutoScroll->GetValue());
	Options.SaveAudioOpts();
}


///////////////
// Auto Commit
void AudioBox::OnAutoCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	Options.SetBool("Audio Autocommit",AutoCommit->GetValue());
	Options.SaveAudioOpts();
}


//////////////////////
// Next line on Commit
void AudioBox::OnNextLineCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	Options.SetBool("Audio Next Line On Commit",NextCommit->GetValue());
	Options.SaveAudioOpts();
}



//////////////////////////
// Spectrum Analyzer Mode
void AudioBox::OnSpectrumMode(wxCommandEvent &event) {
	Options.SetBool("Audio Spectrum",SpectrumMode->GetValue());
	Options.SaveAudioOpts();
	audioDisplay->SetFocus();
	audioDisplay->UpdateImage();
}


///////////////
// Lead in/out
void AudioBox::OnLeadIn(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->AddLead(true,false);
}

void AudioBox::OnLeadOut(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->AddLead(false,true);
}

void AudioBox::OnMouseEvents(wxMouseEvent &event)
{
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	int npos=event.GetY();
	int w=0,h=0;
	GetClientSize(&w,&h);

	if(arrows && (event.Leaving()||npos<=(h-4))){
		SetCursor(wxCURSOR_ARROW); arrows=false;}
	else if(!arrows && npos>h-4){
		SetCursor(wxCURSOR_SIZENS); arrows= true;}


	if (left_up && holding) {
		holding = false;
		int x=0; 
		if(sline){
			sline->GetPosition(&x,&npos);
			sline->Destroy();}
		npos+=7;ScreenToClient(&x,&npos);
		SetMinSize(wxSize(w,npos));
		EditBox* EB= (EditBox*)GetParent();
		EB->BoxSizer1->Layout();
		EB->TextEdit->Refresh(false);
		EB->TlMode->Refresh(false);
		ReleaseMouse();
		Options.SetInt("Audio Box Height",npos);
		Options.SaveAudioOpts();

	}

	if (left_up && !holding) {
		return;
	}

	if (click && event.GetY()>h-5) {
		holding = true;
		CaptureMouse();
		sline= new wxDialog(this,-1,"",wxPoint(0,event.GetY()),wxSize(GetSize().GetWidth()+4,3),wxSTAY_ON_TOP|wxBORDER_NONE);
		sline->SetBackgroundColour("#606060");
		int px=-5, py=event.GetY();
		ClientToScreen(&px,&py);
		sline->SetPosition(wxPoint(px,py));
		sline->Show();
	}

	if (holding){

		int npos=event.GetY();

		if(npos!=oldy&&npos>150){
			int px=-5, py=npos;
			ClientToScreen(&px,&py);
			sline->SetPosition(wxPoint(px,py));
		}
		oldy=npos;

	}

}

void AudioBox::SetAccels()
{

	std::vector<wxAcceleratorEntry> entries;
	for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++){
		if(cur->first.Type!='A'){continue;}
		idAndType itype = cur->first;
		entries.push_back(Hkeys.GetHKey(itype));

	}
	wxAcceleratorTable accel(entries.size(), &entries[0]);
	SetAcceleratorTable(accel);

}
/*

//////////////////////////////////////////
// Focus event handling for the scrollbar
BEGIN_EVENT_TABLE(FocusEvent,wxEvtHandler)
EVT_SET_FOCUS(FocusEvent::OnSetFocus)
END_EVENT_TABLE()

void FocusEvent::OnSetFocus(wxFocusEvent &event) {
wxWindow *previous = event.GetWindow();
if (previous) previous->SetFocus();
}

*/

///////////////
// Event table
BEGIN_EVENT_TABLE(AudioBox,wxPanel)
	EVT_COMMAND_SCROLL(Audio_Scrollbar, AudioBox::OnScrollbar)
	EVT_COMMAND_SCROLL(Audio_Horizontal_Zoom, AudioBox::OnHorizontalZoom)
	EVT_COMMAND_SCROLL(Audio_Vertical_Zoom, AudioBox::OnVerticalZoom)
	EVT_COMMAND_SCROLL(Audio_Volume, AudioBox::OnVolume)

	EVT_BUTTON(AudioPlay, AudioBox::OnPlaySelection)
	EVT_BUTTON(AudioStop, AudioBox::OnStop)
	EVT_BUTTON(AudioNext, AudioBox::OnNext)
	EVT_BUTTON(AudioPrevious, AudioBox::OnPrev)
	EVT_BUTTON(AudioPlayBeforeMark, AudioBox::OnPlayBeforeMark)
	EVT_BUTTON(AudioPlayAfterMark, AudioBox::OnPlayAfterMark)
	EVT_BUTTON(AudioPlay500MSBefore, AudioBox::OnPlay500Before)
	EVT_BUTTON(AudioPlay500MSAfter, AudioBox::OnPlay500After)
	EVT_BUTTON(AudioPlay500MSFirst, AudioBox::OnPlay500First)
	EVT_BUTTON(AudioPlay500MSLast, AudioBox::OnPlay500Last)
	EVT_BUTTON(AudioPlayToEnd, AudioBox::OnPlayToEnd)
	EVT_BUTTON(AudioCommit, AudioBox::OnCommit)
	EVT_BUTTON(AudioGoto, AudioBox::OnGoto)
	EVT_BUTTON(AudioLeadin,AudioBox::OnLeadIn)
	EVT_BUTTON(AudioLeadout,AudioBox::OnLeadOut)
	EVT_MENU(AudioPlay, AudioBox::OnPlaySelection)
	EVT_MENU(AudioStop, AudioBox::OnStop)
	EVT_MENU(AudioNext, AudioBox::OnNext)
	EVT_MENU(AudioPrevious, AudioBox::OnPrev)
	EVT_MENU(AudioPlayBeforeMark, AudioBox::OnPlayBeforeMark)
	EVT_MENU(AudioPlayAfterMark, AudioBox::OnPlayAfterMark)
	EVT_MENU(AudioPlay500MSBefore, AudioBox::OnPlay500Before)
	EVT_MENU(AudioPlay500MSAfter, AudioBox::OnPlay500After)
	EVT_MENU(AudioPlay500MSFirst, AudioBox::OnPlay500First)
	EVT_MENU(AudioPlay500MSLast, AudioBox::OnPlay500Last)
	EVT_MENU(AudioPlayToEnd, AudioBox::OnPlayToEnd)
	EVT_MENU(AudioCommit, AudioBox::OnCommit)
	EVT_MENU(AudioGoto, AudioBox::OnGoto)
	EVT_MENU(AudioLeadin,AudioBox::OnLeadIn)
	EVT_MENU(AudioLeadout,AudioBox::OnLeadOut)

	EVT_TOGGLEBUTTON(Audio_Vertical_Link, AudioBox::OnVerticalLink)
	EVT_TOGGLEBUTTON(Audio_Button_Karaoke,AudioBox::OnKaraoke)
	EVT_TOGGLEBUTTON(Audio_Button_Split,AudioBox::OnSplitMode)
	EVT_TOGGLEBUTTON(Audio_Check_AutoGoto,AudioBox::OnAutoGoto)
	EVT_TOGGLEBUTTON(Audio_Check_Spectrum,AudioBox::OnSpectrumMode)
	EVT_TOGGLEBUTTON(Audio_Check_AutoCommit,AudioBox::OnAutoCommit)
	EVT_TOGGLEBUTTON(Audio_Check_NextCommit,AudioBox::OnNextLineCommit)
	EVT_MOUSE_EVENTS(AudioBox::OnMouseEvents)
	END_EVENT_TABLE()

