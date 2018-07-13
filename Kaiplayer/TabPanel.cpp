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
	, sline(NULL)
	, editor(true)
	, holding(false)
{
	SetBackgroundColour(Options.GetColour(WindowBackground));
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	int vw, vh;
	Options.GetCoords(VideoWindowSize, &vw, &vh);
	if (vw < 200){ vw = 550; vh = 400; }
	Video = new VideoCtrl(this, kai, wxSize(vw, vh));
	Video->Hide();
	Grid = new SubsGrid(this, kai, -1, wxDefaultPosition, wxSize(400, 200),/*wxBORDER_SIMPLE|*/wxWANTS_CHARS);//
	Edit = new EditBox(this, Grid, -1);
	//check if there is nothing in constructor that crash or get something wrong when construct
	Edit->StartEdit->SetVideoCtrl(Video);
	Edit->EndEdit->SetVideoCtrl(Video);
	Edit->DurEdit->SetVideoCtrl(Video);
	Edit->SetMinSize(wxSize(-1, 200));
	Edit->SetLine(0);

	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	ShiftTimes = new ShiftTimesWindow(this, kai, -1, wxDefaultPosition, wxDefaultSize/*,wxBORDER_SIMPLE*/);
	ShiftTimes->Show(Options.GetBool(MoveTimesOn));
	BoxSizer3->Add(Grid, 1, wxEXPAND, 0);
	BoxSizer3->Add(ShiftTimes, 0, wxEXPAND, 0);
	BoxSizer2->Add(Video, 0, wxEXPAND | wxALIGN_TOP, 0);
	BoxSizer2->Add(Edit, 1, wxEXPAND | wxALIGN_TOP, 0);

	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer1->Add(BoxSizer2, 0, wxEXPAND | wxALIGN_TOP, 0);
	BoxSizer1->AddSpacer(3);
	BoxSizer1->Add(BoxSizer3, 1, wxEXPAND, 0);
	SetSizerAndFit(BoxSizer1);

	SubsName = _("Bez tytułu");

	SetAccels();
}


TabPanel::~TabPanel(){
	if (Video->player){ Video->player->Stop(false); } //fix kraszów powodowanych przez niszczenie editboxa na samym końcu i próbując pobrać audio na play kraszuje.
}


void TabPanel::SetAccels(bool onlyGridAudio /*= false*/)
{
	std::vector<wxAcceleratorEntry> gentries;
	gentries.resize(3);
	gentries[0].Set(wxACCEL_CTRL, (int) 'X', Cut);
	gentries[1].Set(wxACCEL_CTRL, (int) 'C', Copy);
	gentries[2].Set(wxACCEL_CTRL, (int) 'V', Paste);

	std::vector<wxAcceleratorEntry> eentries;
	eentries.resize(2);
	eentries[0].Set(wxACCEL_CTRL, WXK_NUMPAD_ENTER, EDITBOX_COMMIT);
	eentries[1].Set(wxACCEL_NORMAL, WXK_NUMPAD_ENTER, EDITBOX_COMMIT_GO_NEXT_LINE);
	std::vector<wxAcceleratorEntry> ventries;
	int numTagButtons = Options.GetInt(EditboxTagButtons);
	const std::map<idAndType, hdata> &hkeys = Hkeys.GetHotkeysMap();

	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++){
		int id = cur->first.id;
		if (cur->second.Accel == "" /*|| cur->first.Type == AUDIO_HOTKEY*/ || cur->first.Type == GLOBAL_HOTKEY ||
			(onlyGridAudio && (cur->first.Type != GRID_HOTKEY && cur->first.Type != AUDIO_HOTKEY))){
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
			if (id >= AudioCommit && id <= AudioNext)
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
			if (id >= PlayPause && id <= Minus5Second){
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

	if (Edit->ABox)
		Edit->ABox->SetAccels();
}

void TabPanel::OnMouseEvent(wxMouseEvent& event)
{
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	int w, h;
	Edit->GetClientSize(&w, &h);
	int npos = event.GetY();
	if (event.Leaving()){
		SetCursor(wxCURSOR_ARROW);
	}
	else if (npos < h && !click && !holding && !left_up){
		SetCursor(wxCURSOR_SIZENS);/*SetCursor(wxCURSOR_NO_ENTRY);*/ return;
	}
	else{
		SetCursor(wxCURSOR_SIZENS);
	}

	if (!holding && HasCapture())
		ReleaseMouse();

	if (left_up && holding) {
		holding = false;
		ReleaseMouse();
		if (sline){
			int x;
			sline->GetPosition(&x, &npos);
			ScreenToClient(&x, &npos);
			sline->Destroy();
			sline = NULL;
		}

		int mw, mh;
		GetClientSize(&mw, &mh);
		if (npos >= mh){
			npos = mh - 3;
		}

		if (Video->GetState() != None&&Video->IsShown()){
			Options.GetCoords(VideoWindowSize, &w, &h);
			int ww, hh;
			Video->CalcSize(&ww, &hh, w, npos, false, true);
			Video->SetMinSize(wxSize(ww, hh + Video->panelHeight));
			Options.SetCoords(VideoWindowSize, ww, hh + Video->panelHeight);
		}/*else{*/Edit->SetMinSize(wxSize(-1, npos));/*}*/
		BoxSizer1->Layout();
		if (event.ShiftDown()){
			SetVideoWindowSizes(w, npos);
		}
	}

	if (left_up && !holding) {
		return;
	}

	if (click && !holding) {
		holding = true;
		CaptureMouse();
		int px = 2, py = npos;
		ClientToScreen(&px, &py);
		sline = new wxDialog(this, -1, "", wxPoint(px, py), wxSize(GetSize().GetWidth(), 2), wxSTAY_ON_TOP | wxBORDER_NONE);
		sline->SetBackgroundColour(Options.GetColour(WindowText));
		sline->Show();
	}

	if (holding){
		int w = 0, h = 0;
		Video->GetClientSize(&w, &h);
		int limit = (Video->GetState() != None && Video->IsShown()) ? 350 : 150;
		if (npos != h&&npos > limit){
			int px = 2, py = npos;
			ClientToScreen(&px, &py);
			sline->SetPosition(wxPoint(px, py));
		}

	}
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

void TabPanel::SetVideoWindowSizes(int w, int h)
{
	Notebook *nb = Notebook::GetTabs();
	for (int i = 0; i < nb->Size(); i++){
		TabPanel *tab = nb->Page(i);
		if (tab == this){ continue; }
		if (tab->Video->GetState() != None/* && tab->Video->IsShown()*/){
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

BEGIN_EVENT_TABLE(TabPanel, wxWindow)
EVT_MOUSE_EVENTS(TabPanel::OnMouseEvent)
EVT_CHILD_FOCUS(TabPanel::OnFocus)
END_EVENT_TABLE()
