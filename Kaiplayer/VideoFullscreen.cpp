//  Copyright (c) 2016 - 2020, Marcin Drob

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
#include "KainoteApp.h"
#include "Config.h"


Fullscreen::Fullscreen(wxWindow* parent, const wxPoint& pos, const wxSize &size)
	: wxFrame(parent, -1, L"", pos, size, wxWANTS_CHARS| wxBORDER_NONE | wxSTAY_ON_TOP)
{
	SetFont(*Options.GetFont());
	vb = parent;
	VideoCtrl *vc = (VideoCtrl*)parent;
	SetBackgroundColour(L"#000000");
	int fw;
	GetTextExtent(L"#TWFfGH", &fw, &toolBarHeight);
	toolBarHeight += 8;
	if (!vc->IsDirectShow()){ 
		buttonSection = 30 + toolBarHeight - 8;
		panelsize = buttonSection + toolBarHeight;
		vc->SetPanelOnFullScreen(true);
	}
	else{ 
		panelsize = buttonSection = 30 + toolBarHeight - 8;
	}
	panel = new wxPanel(this, -1, wxPoint(0, size.y - panelsize), wxSize(size.x, panelsize));
	panel->SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	panel->SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	panel->SetCursor(wxCURSOR_ARROW);
	vslider = new VideoSlider(panel, ID_SLIDER, wxPoint(0, 1), wxSize(size.x, toolBarHeight - 8));
	vslider->VB = vc;
	/*vslider->Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent &evt){
		panel->SetFocus();
		evt.Skip();
	});*/
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
	showToolbar->SetValue(!vc->IsDirectShow());
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
	vToolbar->Show(!vc->IsDirectShow());
	showToolbar->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [=](wxCommandEvent &evt){
		bool show = showToolbar->GetValue();
		vToolbar->Show(show);
		if (show){ 
			panelsize = buttonSection + toolBarHeight;
			vc->SetPanelOnFullScreen(true);
		}
		else{ 
			panelsize = buttonSection;
			vc->SetPanelOnFullScreen(false);
		}
		OnSize();
		vc->UpdateVideoWindow();
	}, 7777);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Fullscreen::OnUseWindowHotkey, this, VIDEO_PREVIOUS_FILE, VIDEO_NEXT_FILE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Fullscreen::OnUseWindowHotkey, this, VIDEO_PLAY_PAUSE, VIDEO_STOP);
	Bind(wxEVT_SCROLL_CHANGED, [=](wxScrollEvent& evt) {
		vc->OnVolume(evt);
		}, ID_VOL);
	//Connect(wxEVT_SIZE, (wxObjectEventFunction)&Fullscreen::OnSize);
	Bind(wxEVT_SYS_COLOUR_CHANGED, [=](wxSysColourChangedEvent & evt){
		panel->SetForegroundColour(Options.GetColour(WINDOW_TEXT));
		panel->SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	});
	SetAccels();
}

Fullscreen::~Fullscreen()
{
}

void Fullscreen::OnSize()
{
	wxSize asize = GetClientSize();
	VideoCtrl *vc = (VideoCtrl*)vb;
	//if(vc->lastSize == asize){return;}
	vc->SetVideoWindowLastSize(asize);
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

	wxSize toolbarSize = showToolbar->GetMinSize();
	if (oldPanelSize == panelsize) {
		bprev->SetPosition(wxPoint(5, toolBarHeight - 6));
		bpause->SetPosition(wxPoint(40, toolBarHeight - 6));
		bpline->SetPosition(wxPoint(75, toolBarHeight - 6));
		bstop->SetPosition(wxPoint(110, toolBarHeight - 6));
		bnext->SetPosition(wxPoint(145, toolBarHeight - 6));
		
		showToolbar->SetSize(180, toolBarHeight - 6 + ((26 - toolbarSize.y) / 2), toolbarSize.x, toolbarSize.y);
	}
		mstimes->SetSize(180 + toolbarSize.x + 10, toolBarHeight - 6, 13 * toolBarHeight, toolBarHeight - 6);
		if (vToolbar->IsShown()){
			vToolbar->SetSize(0, buttonSection, asize.x, toolBarHeight);
		}
		int posx = 180 + toolbarSize.x + 25 + (13 * toolBarHeight);
		Videolabel->SetSize(posx, toolBarHeight - 6, asize.x - posx - 115, toolBarHeight - 6);
	//}
	//else{
	//	//mstimes->SetSize(asize.x - difSize, -1);
	//	if (vToolbar->IsShown()){
	//		vToolbar->SetSize(asize.x, toolBarHeight);
	//	}
	//	Videolabel->SetSize(asize.x - 758, toolBarHeight - 6);
	//}
	//
	vslider->SetSize(wxSize(asize.x, toolBarHeight - 8));
	if(vc->IsDirectShow()){
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

void Fullscreen::OnMouseEvent(wxMouseEvent& evt)
{
	VideoCtrl* vc = (VideoCtrl*)vb;
	vc->OnMouseEvent(evt);
}

void Fullscreen::OnKeyPress(wxKeyEvent& evt)
{
	VideoCtrl* vc = (VideoCtrl*)vb;
	vc->OnKeyPress(evt);
}

void Fullscreen::SetAccels()
{
	kainoteApp* Kaia = ((kainoteApp*)wxTheApp);
	if (!Kaia)
		return;

	KainoteFrame* Kai = Kaia->Frame;
	std::vector<wxAcceleratorEntry> entries;

	const std::map<idAndType, hdata>& hkeys = Hkeys.GetHotkeysMap();
	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++) {
		//if (cur->first.Type != GLOBAL_HOTKEY) { continue; }
		int id = cur->first.id;
		bool emptyAccel = cur->second.Accel == L"";
		if (id >= 5000 && id < 5150) {
			Bind(wxEVT_COMMAND_MENU_SELECTED, &Fullscreen::OnUseWindowHotkey, this, id);
			wxAcceleratorEntry accel = Hkeys.GetHKey(cur->first, &cur->second);
			entries.push_back(accel);
		}
		else if (emptyAccel)
			continue;
		else if (id >= 5150) {
			if (id >= 30100) {
				//Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnRunScript, this, id);
				continue;
			}
			Bind(wxEVT_COMMAND_MENU_SELECTED, &Fullscreen::OnUseWindowHotkey, this, id);
			entries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}
		else if (id >= 2000 && id < 3000) {
			Bind(wxEVT_COMMAND_MENU_SELECTED, &Fullscreen::OnUseWindowHotkey, this, id);
			entries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}
		if (entries.size() && !entries[entries.size() - 1].IsOk()) {
			entries.pop_back();
		}
	}
	wxAcceleratorTable accel(entries.size(), &entries[0]);
	SetAcceleratorTable(accel);
}

void Fullscreen::OnUseWindowHotkey(wxCommandEvent& event)
{
	VideoCtrl* vc = (VideoCtrl*)vb;
	vc->OnAccelerator(event);
}

void Fullscreen::OnPaint(wxPaintEvent& evt)
{
	VideoCtrl* vc = (VideoCtrl*)vb;
	vc->OnPaint(evt);
}

BEGIN_EVENT_TABLE(Fullscreen, wxFrame)
EVT_MOUSE_EVENTS(Fullscreen::OnMouseEvent)
EVT_KEY_DOWN(Fullscreen::OnKeyPress)
EVT_PAINT(Fullscreen::OnPaint)
END_EVENT_TABLE()