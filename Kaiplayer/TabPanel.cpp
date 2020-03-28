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


#include "TabPanel.h"
#include "Config.h"
#include "Hotkeys.h"
#include "KainoteMain.h"


TabPanel::TabPanel(wxWindow *parent, KainoteFrame *kai, const wxPoint &pos, const wxSize &size)
	: wxWindow(parent, -1, pos, size)
	, windowResizer(NULL)
	, editor(true)
	, holding(false)
{
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	int vw, vh;
	Options.GetCoords(VIDEO_WINDOW_SIZE, &vw, &vh);
	if (vw < 200){ vw = 550; vh = 400; }
	Video = new VideoCtrl(this, kai, wxSize(vw, vh));
	Video->Hide();
	Grid = new SubsGrid(this, kai, -1, wxDefaultPosition, wxSize(400, 200), wxWANTS_CHARS);
	Edit = new EditBox(this, Grid, -1);
	//check if there is nothing in constructor that crash or get something wrong when construct
	Edit->StartEdit->SetVideoCtrl(Video);
	Edit->EndEdit->SetVideoCtrl(Video);
	Edit->DurEdit->SetVideoCtrl(Video);
	Edit->SetMinSize(wxSize(-1, 200));
	Edit->SetLine(0);

	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	ShiftTimes = new ShiftTimesWindow(this, kai, -1);
	ShiftTimes->Show(Options.GetBool(SHIFT_TIMES_ON));
	BoxSizer3->Add(Grid, 1, wxEXPAND, 0);
	BoxSizer3->Add(ShiftTimes, 0, wxEXPAND, 0);
	BoxSizer2->Add(Video, 0, wxEXPAND | wxALIGN_TOP, 0);
	BoxSizer2->Add(Edit, 1, wxEXPAND | wxALIGN_TOP, 0);

	windowResizer = new KaiWindowResizer(this, [=](int newpos){
		int mw, mh;
		GetClientSize(&mw, &mh);
		int limit = (Video->GetState() != None && Video->IsShown()) ? 350 : 150;
		return newpos > limit && newpos < mh - 5;
	}, [=](int newpos, bool shiftDown){
		int w, h;
		Edit->GetClientSize(&w, &h);
		SetVideoWindowSizes(w, newpos, shiftDown);
	});

	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer1->Add(BoxSizer2, 0, wxEXPAND | wxALIGN_TOP, 0);
	BoxSizer1->Add(windowResizer, 0, wxEXPAND, 0);//AddSpacer(3);
	BoxSizer1->Add(BoxSizer3, 1, wxEXPAND, 0);
	SetSizerAndFit(BoxSizer1);

	SubsName = _("Bez tytułu");

	SetAccels();
}


TabPanel::~TabPanel(){
	//fix of crashes caused by destroying of editbox on the end
	if (Video->player){ Video->player->Stop(false); }
}


void TabPanel::SetAccels(bool onlyGridAudio /*= false*/)
{
	std::vector<wxAcceleratorEntry> gentries;
	gentries.resize(3);
	gentries[0].Set(wxACCEL_CTRL, (int) L'X', GRID_CUT);
	gentries[1].Set(wxACCEL_CTRL, (int) L'C', GRID_COPY);
	gentries[2].Set(wxACCEL_CTRL, (int) L'V', GRID_PASTE);

	std::vector<wxAcceleratorEntry> eentries;
	eentries.resize(2);
	eentries[0].Set(wxACCEL_CTRL, WXK_NUMPAD_ENTER, EDITBOX_COMMIT);
	eentries[1].Set(wxACCEL_NORMAL, WXK_NUMPAD_ENTER, EDITBOX_COMMIT_GO_NEXT_LINE);
	std::vector<wxAcceleratorEntry> ventries;
	int numTagButtons = Options.GetInt(EDITBOX_TAG_BUTTONS);
	const std::map<idAndType, hdata> &hkeys = Hkeys.GetHotkeysMap();

	// if onlygridaudio than it can pud everything it tables but not need to set this accelerators
	// extended filtering has less performance
	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++){
		int id = cur->first.id;
		if (cur->second.Accel == L"" /*|| cur->first.Type == AUDIO_HOTKEY*/ || cur->first.Type == GLOBAL_HOTKEY ){/*||
			(onlyGridAudio && ((cur->first.Type != GRID_HOTKEY && cur->first.Type != AUDIO_HOTKEY) || 
			(id < VIDEO_PLAY_PAUSE && id <= VIDEO_5_SECONDS_BACKWARD)))){*/
			continue;
		}
		//editor
		if (cur->first.Type == EDITBOX_HOTKEY){
			//do not map hotkeys for hidden tag buttons
			if (cur->first.id >= numTagButtons + EDITBOX_TAG_BUTTON1 && cur->first.id <= EDITBOX_TAG_BUTTON10)
				continue;
			eentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));

		}
		else if (cur->first.Type == GRID_HOTKEY || cur->first.Type == AUDIO_HOTKEY){//grid
			if (id >= AUDIO_COMMIT && id <= AUDIO_NEXT)
				continue;

			gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if ((id > 5000 && id <= 6000) || (id < 1700 && id>600)){
				Grid->ConnectAcc((id < 1000) ? id + 1000 : id);
				if (id < 1700){
					audioHotkeysLoaded = true;
				}
			}

		}
		else if (cur->first.Type == VIDEO_HOTKEY){//video
			ventries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if (id > 2000 && id < 3990){ Video->ConnectAcc(id); }
			if (id >= VIDEO_PLAY_PAUSE && id <= VIDEO_5_SECONDS_BACKWARD){
				gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				Grid->ConnectAcc(id);
			}
		}

	}

	wxAcceleratorTable accelg(gentries.size(), &gentries[0]);
	Grid->SetAcceleratorTable(accelg);
	if (!onlyGridAudio){
		wxAcceleratorTable accelv(ventries.size(), &ventries[0]);
		Video->SetAcceleratorTable(accelv);
		wxAcceleratorTable accele(eentries.size(), &eentries[0]);
		Edit->SetAcceleratorTable(accele);
	}

	if (Edit->ABox && !onlyGridAudio)
		Edit->ABox->SetAccels();
}

void TabPanel::OnFocus(wxChildFocusEvent& event)
{
	Notebook *nt = Notebook::GetTabs();
	//if(!nt){return;}
	if (!nt->split){ event.Skip(); return; }

	if (nt->GetTab() != this)
	{
		nt->ChangeActive();
	}
	event.Skip();
}

void TabPanel::SetVideoWindowSizes(int w, int h, bool allTabs)
{
	Notebook *nb = Notebook::GetTabs();

	if (Video->GetState() != None && Video->IsShown()){
		//Options.GetCoords(VIDEO_WINDOW_SIZE, &w, &h);
		int ww, hh;
		Video->CalcSize(&ww, &hh, w, h, false, true);
		Video->SetMinSize(wxSize(ww, hh + Video->panelHeight));
		Options.SetCoords(VIDEO_WINDOW_SIZE, ww, hh + Video->panelHeight);
	}
	Edit->SetMinSize(wxSize(-1, h));
	BoxSizer1->Layout();
	if (!allTabs)
		return;

	for (int i = 0; i < nb->Size(); i++){
		TabPanel *tab = nb->Page(i);
		if (tab == this){ continue; }
		if (tab->Video->GetState() != None){
			int ww, hh;
			tab->Video->CalcSize(&ww, &hh, w, h, false, true);
			tab->Video->SetMinSize(wxSize(ww, hh + tab->Video->panelHeight));
		}
		tab->Edit->SetMinSize(wxSize(-1, h));
		tab->BoxSizer1->Layout();
	}
}

bool TabPanel::Hide()
{
	//Todo zbadać czemu twierdzi, że grid to nie jest descendant of tabpanel
	wxWindow *win = FindFocus();
	if (win && IsDescendant(win)){
		lastFocusedWindow = win;
	}
	else{ lastFocusedWindow = NULL; }
	return wxWindow::Show(false);
}

//bool TabPanel::Show(bool show)
//{
//	bool ret = wxWindow::Show();
//	if (focusedWindowId !=0){
//		wxWindow *lastFocusedWindow = FindWindowById(focusedWindowId);
//		if (lastFocusedWindow){
//			lastFocusedWindow->SetFocus();
//		}
//	}
//	else{
//		if (Grid1->IsShown()){
//			Grid1->SetFocus();
//		}
//		else if (Video->IsShown()){
//			Video->SetFocus();
//		}
//	}
//	return ret;
//}

bool TabPanel::SetFont(const wxFont &font)
{
	Video->SetFont(font);
	Edit->SetFont(font);
	ShiftTimes->SetFont(font);

	return wxWindow::SetFont(font);
}
BEGIN_EVENT_TABLE(TabPanel, wxWindow)
//EVT_MOUSE_EVENTS(TabPanel::OnMouseEvent)
EVT_CHILD_FOCUS(TabPanel::OnFocus)
END_EVENT_TABLE()
