// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
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
#include "config.h"

#include <math.h>
#include "audio_box.h"
#include "kainoteMain.h"
#include "config.h"
#include "hotkeys.h"


///////////////
// Constructor
//
AudioBox::AudioBox(wxWindow *parent, wxWindow *Wgrid) :
wxPanel(parent,-1,wxDefaultPosition,wxSize(0,0),wxBORDER_RAISED)
{
	// Setup
	loaded = false;
	arrows = holding = false;
	oldy=-1;
	SetMinSize(wxSize(-1,Options.GetInt("Audio Box Height")));
	// Display
	audioScroll = new wxScrollBar(this,Audio_Scrollbar);
	//audioScroll->PushEventHandler(new FocusEvent());
	audioScroll->SetToolTip(_("Pasek szukania"));
	
	audioDisplay = new AudioDisplay(this);
	
	audioDisplay->ScrollBar = audioScroll;
	audioDisplay->box = this;
	audioDisplay->Edit=(EditBox*)parent;
	audioDisplay->grid=(Grid*)Wgrid;
	
	// Zoom

	HorizontalZoom = new wxSlider(this,Audio_Horizontal_Zoom,(audioDisplay->hasKara)?30 : 50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH);
	//HorizontalZoom->PushEventHandler(new FocusEvent());
	HorizontalZoom->SetToolTip(_("Rozci¹gniêcie w poziomie"));
	VerticalZoom = new wxSlider(this,Audio_Vertical_Zoom,50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	//VerticalZoom->PushEventHandler(new FocusEvent());
	VerticalZoom->SetToolTip(_("Rozci¹gniêcie w pionie"));
	VolumeBar = new wxSlider(this,Audio_Volume,50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	//VolumeBar->PushEventHandler(new FocusEvent());
	VolumeBar->SetToolTip(_("G³oœnoœæ"));
	bool link = Options.GetBool(_T("Audio Link"));
	if (link) {
		VolumeBar->SetValue(VerticalZoom->GetValue());
		VolumeBar->Enable(false);
	}
	VerticalLink = new wxToggleButton(this,Audio_Vertical_Link,"", wxDefaultPosition, wxSize(40,24));
	VerticalLink->SetBitmap(wxBitmap("button_link"));
	VerticalLink->SetToolTip(_("Po³¹cz suwak g³oœnoœci i rozci¹gniêcia"));
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
	wxBitmapButton *temp;
	temp = new wxBitmapButton(this,Audio_Button_Prev,wxBITMAP_PNG("button_prev"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz poprzedni¹ linijkê ("+Hkeys.GetMenuH(Audio_Button_Prev)+" lub "+Hkeys.GetMenuH(Audio_Button_Prev-1000)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Next,wxBITMAP_PNG("button_next"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz nastêpn¹ linijkê ("+Hkeys.GetMenuH(Audio_Button_Next)+" lub "+Hkeys.GetMenuH(Audio_Button_Next-1000)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Play,wxBITMAP_PNG("button_playsel"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz aktualn¹ linijkê ("+Hkeys.GetMenuH(Audio_Button_Play)+" lub "+Hkeys.GetMenuH(Audio_Button_Play-1000)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Stop,wxBITMAP_PNG("button_stop"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Zatrzymaj odtwarzanie ("+Hkeys.GetMenuH(Audio_Button_Stop)+")");
	ButtonSizer->Add(temp,0,wxRIGHT,5);
	//ButtonSizer->AddSpacer(2);
	temp = new wxBitmapButton(this,Audio_Button_Play_Before_Mark,wxBITMAP_PNG("button_playbefore"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz przed znacznikiem ("+Hkeys.GetMenuH(Audio_Button_Play_Before_Mark)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_After_Mark,wxBITMAP_PNG("button_playafter"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz po znaczniku ("+Hkeys.GetMenuH(Audio_Button_Play_After_Mark)+")");
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_Before,wxBITMAP_PNG("button_playfivehbefore"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz 500ms przed czasem startowym ("+Hkeys.GetMenuH(Audio_Button_Play_500ms_Before)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_First,wxBITMAP_PNG("button_playfirstfiveh"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz 500ms po czasie startowym ("+Hkeys.GetMenuH(Audio_Button_Play_500ms_First)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_Last,wxBITMAP_PNG("button_playlastfiveh"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz 500ms przed czasem koñcowym ("+Hkeys.GetMenuH(Audio_Button_Play_500ms_Last)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_After,wxBITMAP_PNG("button_playfivehafter"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz 500ms po czasie koñcowym ("+Hkeys.GetMenuH(Audio_Button_Play_500ms_After)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_To_End,wxBITMAP_PNG("button_playtoend"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Odtwórz do koñca ("+Hkeys.GetMenuH(Audio_Button_Play_To_End)+")");
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	temp = new wxBitmapButton(this,Audio_Button_Leadin,wxBITMAP_PNG("button_leadin"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Dodaj wstêp do aktywnej linijki ("+Hkeys.GetMenuH(Audio_Button_Leadin)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Leadout,wxBITMAP_PNG("button_leadout"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("Dodaj zakoñczenie do aktywnej linijki ("+Hkeys.GetMenuH(Audio_Button_Leadout)+")");
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	temp = new wxBitmapButton(this,Audio_Button_Commit,wxBITMAP_PNG("button_audio_commit"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("ZatwierdŸ zmiany ("+Hkeys.GetMenuH(Audio_Button_Commit)+" lub "+Hkeys.GetMenuH(Audio_Button_Commit-1000)+")");
	ButtonSizer->Add(temp,0,0,0);
	temp = new wxBitmapButton(this,Audio_Button_Goto,wxBITMAP_PNG("button_audio_goto"),wxDefaultPosition,wxSize(26,26));
	temp->SetToolTip("PrzejdŸ to zaznaczenia ("+Hkeys.GetMenuH(Audio_Button_Goto)+")");
	ButtonSizer->Add(temp,0,wxRIGHT,5);

	KaraSwitch = new wxToggleButton(this,Audio_Button_Karaoke,"",wxDefaultPosition,wxSize(26,26));
	KaraSwitch->SetToolTip(_("W³¹cz / Wy³¹cz tworzenie karaoke"));
	KaraSwitch->SetBitmap(wxBITMAP_PNG("button_karaoke"));
	KaraSwitch->SetValue(audioDisplay->hasKara);
	ButtonSizer->Add(KaraSwitch,0,wxALIGN_CENTER | wxEXPAND,0);
	KaraMode = new wxToggleButton(this,Audio_Button_Split,"",wxDefaultPosition,wxSize(26,26));
	KaraMode->SetToolTip(_("W³¹cz / Wy³¹cz automatyczne dzielenie sylab"));
	KaraMode->SetBitmap(wxBITMAP_PNG("button_auto_split"));
	KaraMode->SetValue(audioDisplay->karaAuto);
	ButtonSizer->Add(KaraMode,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,5);

	AutoCommit = new wxToggleButton(this,Audio_Check_AutoCommit,"",wxDefaultPosition,wxSize(26,26));
	AutoCommit->SetToolTip(_("Automatycznie zatwierdza zmiany"));
	AutoCommit->SetBitmap(wxBITMAP_PNG("button_auto_commit"));
	AutoCommit->SetValue(Options.GetBool(_T("Audio Autocommit")));
	ButtonSizer->Add(AutoCommit,0,wxALIGN_CENTER | wxEXPAND,0);
	NextCommit = new wxToggleButton(this,Audio_Check_NextCommit,"",wxDefaultPosition,wxSize(26,26));
	NextCommit->SetToolTip(_("Przechodzenie do nastêpnej linijki po zatwierdzeniu zmian"));
	NextCommit->SetBitmap(wxBITMAP_PNG("button_next_a_commit"));
	NextCommit->SetValue(Options.GetBool(_T("Audio Next Line On Commit")));
	ButtonSizer->Add(NextCommit,0,wxALIGN_CENTER | wxEXPAND,0);
	AutoScroll = new wxToggleButton(this,Audio_Check_AutoGoto,"",wxDefaultPosition,wxSize(26,26));
	AutoScroll->SetToolTip(_("Automatyczne przewijanie do aktywnej linijki"));
	AutoScroll->SetBitmap(wxBITMAP_PNG("button_auto_go"));
	AutoScroll->SetValue(Options.GetBool(_T("Audio Autoscroll")));
	ButtonSizer->Add(AutoScroll,0,wxALIGN_CENTER | wxEXPAND,0);
	SpectrumMode = new wxToggleButton(this,Audio_Check_Spectrum,"",wxDefaultPosition,wxSize(26,26));
	SpectrumMode->SetToolTip(_("Tryb spektrum"));
	SpectrumMode->SetBitmap(wxBITMAP_PNG("button_spectrum"));
	SpectrumMode->SetValue(Options.GetBool(_T("Audio Spectrum")));
	ButtonSizer->Add(SpectrumMode,0,wxALIGN_CENTER | wxEXPAND,0);
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
	if (file != _T("")) loaded = audioDisplay->loaded;
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
}


/////////////////////////////
// Vertical zoom bar changed
void AudioBox::OnVerticalZoom(wxScrollEvent &event) {
	int pos = event.GetPosition();
	if (pos < 1) pos = 1;
	if (pos > 100) pos = 100;
	float value = pow(float(pos)/50.0f,3);
	audioDisplay->SetScale(value);
	if (VerticalLink->GetValue()) {
		audioDisplay->player->SetVolume(value);
		VolumeBar->SetValue(pos);
	}
}


//////////////////////
// Volume bar changed
void AudioBox::OnVolume(wxScrollEvent &event) {
	if (!VerticalLink->GetValue()) {
		int pos = event.GetPosition();
		if (pos < 1) pos = 1;
		if (pos > 100) pos = 100;
		audioDisplay->player->SetVolume(pow(float(pos)/50.0f,3));
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
	VolumeBar->Enable(!VerticalLink->GetValue());

	Options.SetBool(_T("Audio Link"),VerticalLink->GetValue());
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
	if(audioDisplay->hasKara){
	if(!audioDisplay->karaoke){audioDisplay->karaoke=new Karaoke(audioDisplay);}
		audioDisplay->karaoke->Split();
		audioDisplay->SetSamplesPercent(30);
		HorizontalZoom->SetValue(30);
	}else{
		audioDisplay->SetSamplesPercent(50);
		HorizontalZoom->SetValue(50);
	}


	audioDisplay->MakeDialogueVisible();

	Options.SetBool(_T("Audio Karaoke"),audioDisplay->hasKara);
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
	Options.SetBool(_T("Audio Autoscroll"),AutoScroll->GetValue());
	Options.SaveAudioOpts();
}


///////////////
// Auto Commit
void AudioBox::OnAutoCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	Options.SetBool(_T("Audio Autocommit"),AutoCommit->GetValue());
	Options.SaveAudioOpts();
}


//////////////////////
// Next line on Commit
void AudioBox::OnNextLineCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	Options.SetBool(_T("Audio Next Line On Commit"),NextCommit->GetValue());
	Options.SaveAudioOpts();
}



//////////////////////////
// Spectrum Analyzer Mode
void AudioBox::OnSpectrumMode(wxCommandEvent &event) {
	Options.SetBool(_T("Audio Spectrum"),SpectrumMode->GetValue());
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
		int id=cur->first;
		if(id<2000){
			if(id<1000){id+=1000;}
			entries.push_back(Hkeys.GetHKey(id));
		}else{break;}

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

	EVT_BUTTON(Audio_Button_Play, AudioBox::OnPlaySelection)
	EVT_BUTTON(Audio_Button_Stop, AudioBox::OnStop)
	EVT_BUTTON(Audio_Button_Next, AudioBox::OnNext)
	EVT_BUTTON(Audio_Button_Prev, AudioBox::OnPrev)
	EVT_BUTTON(Audio_Button_Play_Before_Mark, AudioBox::OnPlayBeforeMark)
	EVT_BUTTON(Audio_Button_Play_After_Mark, AudioBox::OnPlayAfterMark)
	EVT_BUTTON(Audio_Button_Play_500ms_Before, AudioBox::OnPlay500Before)
	EVT_BUTTON(Audio_Button_Play_500ms_After, AudioBox::OnPlay500After)
	EVT_BUTTON(Audio_Button_Play_500ms_First, AudioBox::OnPlay500First)
	EVT_BUTTON(Audio_Button_Play_500ms_Last, AudioBox::OnPlay500Last)
	EVT_BUTTON(Audio_Button_Play_To_End, AudioBox::OnPlayToEnd)
	EVT_BUTTON(Audio_Button_Commit, AudioBox::OnCommit)
	EVT_BUTTON(Audio_Button_Goto, AudioBox::OnGoto)
	EVT_BUTTON(Audio_Button_Leadin,AudioBox::OnLeadIn)
	EVT_BUTTON(Audio_Button_Leadout,AudioBox::OnLeadOut)

	EVT_TOGGLEBUTTON(Audio_Vertical_Link, AudioBox::OnVerticalLink)
	EVT_TOGGLEBUTTON(Audio_Button_Karaoke,AudioBox::OnKaraoke)
	EVT_TOGGLEBUTTON(Audio_Button_Split,AudioBox::OnSplitMode)
	EVT_TOGGLEBUTTON(Audio_Check_AutoGoto,AudioBox::OnAutoGoto)
	EVT_TOGGLEBUTTON(Audio_Check_Spectrum,AudioBox::OnSpectrumMode)
	EVT_TOGGLEBUTTON(Audio_Check_AutoCommit,AudioBox::OnAutoCommit)
	EVT_TOGGLEBUTTON(Audio_Check_NextCommit,AudioBox::OnNextLineCommit)
	EVT_MOUSE_EVENTS(AudioBox::OnMouseEvents)
END_EVENT_TABLE()
