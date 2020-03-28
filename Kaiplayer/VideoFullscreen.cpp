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
#include "KainoteMain.h"


Fullscreen::Fullscreen(wxWindow* parent, const wxPoint& pos, const wxSize &size)
	: wxFrame(parent, -1, L"", pos, size, wxBORDER_NONE | wxSTAY_ON_TOP)//
{
	SetFont(*Options.GetFont());
	vb = parent;
	VideoCtrl *vc = (VideoCtrl*)parent;
	SetBackgroundColour(L"#000000");
	int fw;
	GetTextExtent(L"#TWFfGH", &fw, &toolBarHeight);
	toolBarHeight += 8;
	if (!vc->IsDshow){ 
		buttonSection = 30 + toolBarHeight - 8;
		panelsize = buttonSection + toolBarHeight;
		vc->panelOnFullscreen = true; 
	}
	else{ 
		panelsize = buttonSection = 30 + toolBarHeight - 8;
	}
	panel = new wxPanel(this, -1, wxPoint(0, size.y - panelsize), wxSize(size.x, panelsize));
	panel->SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	panel->SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	vslider = new VideoSlider(panel, ID_SLIDER, wxPoint(0, 1), wxSize(size.x, toolBarHeight - 8));
	vslider->VB = vc;
	vslider->Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent &evt){
		panel->SetFocus();
		evt.Skip();
	});
	bprev = new BitmapButton(panel, CreateBitmapFromPngResource(L"backward"), CreateBitmapFromPngResource(L"backward1"), 
		VIDEO_PREVIOUS_FILE, _("Poprzedni plik wideo"), wxPoint(5, toolBarHeight - 6), wxSize(26, 26));
	bpause = new BitmapButton(panel, CreateBitmapFromPngResource(L"play"), CreateBitmapFromPngResource(L"play1"), 
		VIDEO_PLAY_PAUSE, _("Odtwórz / Pauza"), wxPoint(40, toolBarHeight - 6), wxSize(26, 26));
	bpline = new BitmapButton(panel, CreateBitmapFromPngResource(L"playline"), CreateBitmapFromPngResource(L"playline1"), 
		GLOBAL_PLAY_ACTUAL_LINE, _("Odtwórz aktywną linię"), wxPoint(75, toolBarHeight - 6), wxSize(26, 26));
	bstop = new BitmapButton(panel, CreateBitmapFromPngResource(L"stop"), CreateBitmapFromPngResource(L"stop1"), 
		VIDEO_STOP, _("Zatrzymaj"), wxPoint(110, toolBarHeight - 6), wxSize(26, 26));
	bnext = new BitmapButton(panel, CreateBitmapFromPngResource(L"forward"), CreateBitmapFromPngResource(L"forward1"), 
		VIDEO_NEXT_FILE, _("Następny plik"), wxPoint(145, toolBarHeight - 6), wxSize(26, 26));
	volslider = new VolSlider(panel, ID_VOL, Options.GetInt(VIDEO_VOLUME), wxPoint(size.x - 110, toolBarHeight - 5), wxSize(110, 25));
	showToolbar = new KaiCheckBox(panel, 7777, _("Pokaż pasek narzędzi"), wxPoint(180, toolBarHeight - 4), wxSize(-1, -1));
	showToolbar->SetValue(!vc->IsDshow);
	mstimes = new KaiTextCtrl(panel, -1, L"", wxPoint(340, toolBarHeight - 6), wxSize(300, 26), wxTE_READONLY);
	mstimes->SetWindowStyle(wxBORDER_NONE);
	mstimes->SetCursor(wxCURSOR_ARROW);
	mstimes->SetBackgroundColour(WINDOW_BACKGROUND);
	Videolabel = new KaiStaticText(panel, -1, L"", wxPoint(644, toolBarHeight - 6), wxSize(1200, 26));
	Videolabel->Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent &evt){
		panel->SetFocus();
		evt.Skip();
	});
	vToolbar = new VideoToolbar(panel, wxPoint(0, buttonSection), wxSize(-1, -1));
	vToolbar->SetSize(wxSize(size.x, toolBarHeight));
	vToolbar->Show(!vc->IsDshow);
	showToolbar->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [=](wxCommandEvent &evt){
		bool show = showToolbar->GetValue();
		vToolbar->Show(show);
		if (show){ 
			panelsize = buttonSection + toolBarHeight;
			vc->panelOnFullscreen = true; 
		}
		else{ 
			panelsize = buttonSection;
			vc->panelOnFullscreen = false; 
		}
		OnSize();
		vc->UpdateVideoWindow();
	}, 7777);
	Connect(VIDEO_PREVIOUS_FILE, VIDEO_NEXT_FILE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(VIDEO_PLAY_PAUSE, VIDEO_STOP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	//Connect(GLOBAL_PLAY_ACTUAL_LINE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	//Connect(ID_BPREV,ID_BNEXT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(ID_VOL, wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&VideoCtrl::OnVolume);
	//Connect(wxEVT_SIZE, (wxObjectEventFunction)&Fullscreen::OnSize);
	Bind(wxEVT_SYS_COLOUR_CHANGED, [=](wxSysColourChangedEvent & evt){
		panel->SetForegroundColour(Options.GetColour(WINDOW_TEXT));
		panel->SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	});
	this->SetEventHandler(parent);
	//sprawdzić jeszcze co się dzieje z focusem gdy klikamy w wideo a później w slider albo w static text
	wxAcceleratorTable *VBaccels = parent->GetAcceleratorTable();
	panel->SetAcceleratorTable(*VBaccels);
}

Fullscreen::~Fullscreen()
{
	this->SetEventHandler(this);
	//Zwolnić event handler
}

void Fullscreen::OnSize()
{
	wxSize asize = GetClientSize();
	VideoCtrl *vc = (VideoCtrl*)vb;
	//if(vc->lastSize == asize){return;}
	vc->lastSize = asize;
	int fw;
	int oldPanelSize = panelsize;
	bool toolbarShown = panelsize > buttonSection;
	
	GetTextExtent(L"#TWFfGH", &fw, &toolBarHeight);
	toolBarHeight += 8;
	buttonSection = 30 + toolBarHeight - 8;
	panelsize = buttonSection;
	if (toolbarShown)
		panelsize += toolBarHeight;

	panel->SetSize(0, asize.y - panelsize, asize.x, panelsize);

	if (oldPanelSize == panelsize){
		bprev->SetPosition(wxPoint(5, toolBarHeight - 6));
		bpause->SetPosition(wxPoint(40, toolBarHeight - 6));
		bpline->SetPosition(wxPoint(75, toolBarHeight - 6));
		bstop->SetPosition(wxPoint(110, toolBarHeight - 6));
		bnext->SetPosition(wxPoint(145, toolBarHeight - 6));
		wxSize toolbarSize = showToolbar->GetMinSize();
		showToolbar->SetSize(180, toolBarHeight - 6 + ((26 - toolbarSize.y) / 2), toolbarSize.x, toolbarSize.y);
		mstimes->SetSize(180 + toolbarSize.x + 10, toolBarHeight - 6, 13 * toolBarHeight, 26);
		if (vToolbar->IsShown()){
			vToolbar->SetSize(0, buttonSection, asize.x, toolBarHeight);
		}
		Videolabel->SetSize(180 + toolbarSize.x + 25 + (13 * toolBarHeight), toolBarHeight - 6, asize.x - 758, 26);
	}
	else{
		//mstimes->SetSize(asize.x - difSize, -1);
		if (vToolbar->IsShown()){
			vToolbar->SetSize(asize.x, toolBarHeight);
		}
		Videolabel->SetSize(asize.x - 758, 26);
	}
	
	vslider->SetSize(wxSize(asize.x, 14));
	if(vc->IsDshow){
		volslider->Show(); 
		volslider->SetPosition(wxPoint(asize.x - 110, toolBarHeight - 5));
	}
	else{volslider->Show(false);}
}

void Fullscreen::HideToolbar(bool hide){
	if (hide && panelsize == buttonSection || !hide && panelsize != buttonSection)
		return;
	Videolabel->Show(!hide);
	if (hide){
		panelsize = buttonSection;
	}
	else{
		panelsize = buttonSection + toolBarHeight;
	}
	OnSize();
	VideoCtrl *vc = (VideoCtrl*)vb;
	vc->UpdateVideoWindow();
}

