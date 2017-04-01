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

#include "VideoFullscreen.h"
#include "Videobox.h"
#include "Config.h"
#include "KaiCheckBox.h"


Fullscreen::Fullscreen(wxWindow* parent, const wxPoint& pos, const wxSize &size)
     : wxFrame(parent, -1,"", pos, size, wxBORDER_NONE|wxSTAY_ON_TOP)//
{
    vb=parent;
	VideoCtrl *vc = (VideoCtrl*)parent;
	SetBackgroundColour("#000000");
	this->SetEventHandler(parent);
	if(!vc->IsDshow){panelsize = 66;vc->panelOnFullscreen=true;}else{panelsize = 44;}
	panel=new wxPanel(this,-1,wxPoint(0,size.y - panelsize),wxSize(size.x,panelsize));
	panel->SetForegroundColour(Options.GetColour(WindowText));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));
	vslider= new VideoSlider(panel, ID_SLIDER,wxPoint(0,1),wxSize(size.x,14));
	vslider->VB= vc;
	bprev = new BitmapButton(panel,CreateBitmapFromPngResource("backward"),CreateBitmapFromPngResource("backward1"), ID_BPREV, wxPoint(5,16), wxSize(26,26));
	bpause = new BitmapButton(panel, CreateBitmapFromPngResource("play"),CreateBitmapFromPngResource("play1"),ID_BPAUSE, wxPoint(40,16), wxSize(26,26));
	bpline = new BitmapButton(panel, CreateBitmapFromPngResource("playline"), CreateBitmapFromPngResource("playline1"),ID_BPLINE, wxPoint(75,16), wxSize(26,26));
	bstop = new BitmapButton(panel, CreateBitmapFromPngResource("stop"),CreateBitmapFromPngResource("stop1"),ID_BSTOP, wxPoint(110,16), wxSize(26,26));
	bnext = new BitmapButton(panel, CreateBitmapFromPngResource("forward"), CreateBitmapFromPngResource("forward1"),ID_BNEXT, wxPoint(145,16), wxSize(26,26));
	volslider=new VolSlider(panel,ID_VOL,Options.GetInt(VideoVolume),wxPoint(size.x-110,17),wxSize(110,25));
	KaiCheckBox *showToolbar = new KaiCheckBox(panel,7777,_("Pokaż pasek narzędzi"), wxPoint(180,21),wxSize(150,-1));
	showToolbar->SetValue(!vc->IsDshow);
	Videolabel=new wxStaticText(panel,-1,"",wxPoint(340,24));
	vToolbar = new VideoToolbar(panel,wxPoint(0, 44));
	vToolbar->SetSize(wxSize(size.x, 22));
	vToolbar->Show(!vc->IsDshow);
	showToolbar->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [=](wxCommandEvent &evt){
		bool show = showToolbar->GetValue();
		vToolbar->Show(show); 
		if(show){panelsize = 66;vc->panelOnFullscreen=true;}else{panelsize = 44;vc->panelOnFullscreen=false;}
		OnSize();
		vc->UpdateVideoWindow();
	},7777);
	Connect(ID_BPREV,ID_BNEXT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoCtrl::OnVButton);
	Connect(ID_VOL,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VideoCtrl::OnVolume);
	//Connect(wxEVT_SIZE, (wxObjectEventFunction)&Fullscreen::OnSize);
	Bind(wxEVT_SYS_COLOUR_CHANGED, [=](wxSysColourChangedEvent & evt){
		panel->SetForegroundColour(Options.GetColour(WindowText));
		panel->SetBackgroundColour(Options.GetColour(WindowBackground));
	});
}


Fullscreen::~Fullscreen()
{
	//PopEventHandler();
	this->SetEventHandler(this);
    //Zwolnić event handler
}

void Fullscreen::OnSize()
{
	wxSize asize = GetClientSize();
	VideoCtrl *vc = (VideoCtrl*)vb;
	//if(vc->lastSize == asize){return;}
	vc->lastSize=asize;
	
	vslider->SetSize(wxSize(asize.x,14));
	if(vc->IsDshow){volslider->Show(); volslider->SetPosition(wxPoint(asize.x-110,17));}
	else{volslider->Show(false);}
	Videolabel->SetSize(asize.x-450,-1);
	if(vToolbar->IsShown()){vToolbar->SetSize(asize.x, 22);}
	panel->SetSize(0, asize.y - panelsize, asize.x, panelsize);
}