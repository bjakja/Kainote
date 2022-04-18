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


#include "TabPanel.h"
#include "Config.h"
#include "Hotkeys.h"
#include "KainoteMain.h"


TabPanel::TabPanel(wxWindow *parent, KainoteFrame *kai, const wxPoint &pos, const wxSize &size)
	: KaiPanel(parent, -1, pos, size)
	, windowResizer(nullptr)
	, editor(true)
	, holding(false)
{
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	VideoEditboxSizer = new wxBoxSizer(wxHORIZONTAL);
	int vw, vh;
	Options.GetCoords(VIDEO_WINDOW_SIZE, &vw, &vh);
	if (vw < 200){ vw = 550; vh = 400; }
	Video = new VideoCtrl(this, kai, wxSize(vw, vh));
	Video->Hide();
	Edit = new EditBox(this, -1);

	GridShiftTimesSizer = new wxBoxSizer(wxHORIZONTAL);
	Grid = new SubsGrid(this, kai, -1, wxDefaultPosition, wxSize(400, 200), wxWANTS_CHARS);
	Edit->SetGrid1(Grid);

	ShiftTimes = new ShiftTimesWindow(this, kai, -1);
	ShiftTimes->Show(Options.GetBool(SHIFT_TIMES_ON));
	GridShiftTimesSizer->Add(Grid, 1, wxEXPAND, 0);
	GridShiftTimesSizer->Add(ShiftTimes, 0, wxEXPAND, 0);
	VideoEditboxSizer->Add(Video, 0, wxEXPAND | wxALIGN_TOP, 0);
	VideoEditboxSizer->Add(Edit, 1, wxEXPAND | wxALIGN_TOP, 0);

	//check if there is nothing in constructor that crash or get something wrong when construct
	Edit->StartEdit->SetVideoCtrl(Video);
	Edit->EndEdit->SetVideoCtrl(Video);
	Edit->DurEdit->SetVideoCtrl(Video);
	Edit->SetMinSize(wxSize(-1, 200));
	Edit->SetLine(0);

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

	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(VideoEditboxSizer, 0, wxEXPAND | wxALIGN_TOP, 0);
	MainSizer->Add(windowResizer, 0, wxEXPAND, 0);//AddSpacer(3);
	MainSizer->Add(GridShiftTimesSizer, 1, wxEXPAND, 0);
	SetSizerAndFit(MainSizer);

	SubsName = _("Bez tytułu");

	SetAccels();
	//Bind(wxEVT_NAVIGATION_KEY, &TabPanel::OnNavigation, this);
}


TabPanel::~TabPanel(){
	//fix of crashes caused by destroying of editbox on the end
	if (Edit->ABox){ Edit->ABox->audioDisplay->Stop(false); }
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

	// if onlygridaudio than it can put everything it tables but not need to set this accelerators
	// extended filtering has less performance
	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++){
		int id = cur->first.id;
		if (cur->second.Accel == emptyString || cur->first.Type == GLOBAL_HOTKEY ){
			continue;
		}
		auto itype = cur->first;
		if (itype.Type != AUDIO_HOTKEY){
			if (itype.id >= 2000 && itype.id < 3000 && itype.Type != VIDEO_HOTKEY){
				if (itype.Type == GRID_HOTKEY){
					Grid->Bind(wxEVT_COMMAND_MENU_SELECTED, &VideoCtrl::OnAccelerator, Video, id);
					gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				}
				else{
					Edit->Bind(wxEVT_COMMAND_MENU_SELECTED, &VideoCtrl::OnAccelerator, Video, id);
					eentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				}
				continue;
			}
			else if (itype.id >= 3000 && itype.id < 4000 && itype.Type != EDITBOX_HOTKEY){
				if (itype.Type == GRID_HOTKEY){
					Grid->Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnAccelerator, Edit, id);
					gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				}
				else{
					Video->Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnAccelerator, Edit, id);
					ventries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				}
				continue;
			}
			else if (itype.id >= 4000 && itype.id < 5000 && itype.Type != GRID_HOTKEY){
				if (itype.Type == VIDEO_HOTKEY){
					Video->Bind(wxEVT_COMMAND_MENU_SELECTED, &SubsGrid::OnAccelerator, Grid, id);
					ventries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				}
				else if(itype.Type == GRID_HOTKEY){
					Grid->Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnAccelerator, Edit, id);
					gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				}
				continue;
			}
			
		}
		//editor
		if (cur->first.Type == EDITBOX_HOTKEY){
			//do not map hotkeys for hidden tag buttons
			if (cur->first.id >= numTagButtons + EDITBOX_TAG_BUTTON1 && cur->first.id <= EDITBOX_TAG_BUTTON20)
				continue;
			eentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));

		}
		else if (cur->first.Type == GRID_HOTKEY || cur->first.Type == AUDIO_HOTKEY){//grid
			if (id >= AUDIO_COMMIT && id <= AUDIO_NEXT)
				continue;

			gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if ((id >= 4000 && id < 5000) || (id < 1999 && id >= 1000)){
				Grid->ConnectAcc((id < AUDIO_COMMIT) ? id + 10 : id);
				if (id < 1999){
					audioHotkeysLoaded = true;
				}
			}

		}
		else if (cur->first.Type == VIDEO_HOTKEY){//video
			ventries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if (id >= 2000 && id < 2999){ Video->ConnectAcc(id); }
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
		int ww, hh;
		int panelHeight = Video->GetPanelHeight();
		Video->CalcSize(&ww, &hh, w, h, false, true);
		if (ww < 450)
			ww = 450;
		Video->SetMinSize(wxSize(ww, hh + panelHeight));
		Options.SetCoords(VIDEO_WINDOW_SIZE, ww, hh + panelHeight);
	}
	Edit->SetMinSize(wxSize(-1, h));
	MainSizer->Layout();
	if (!allTabs)
		return;

	for (int i = 0; i < nb->Size(); i++){
		TabPanel *tab = nb->Page(i);
		if (tab == this){ continue; }
		if (tab->Video->GetState() != None){
			int ww, hh;
			tab->Video->CalcSize(&ww, &hh, w, h, false, true);
			if (ww < 450)
				ww = 450;
			tab->Video->SetMinSize(wxSize(ww, hh + tab->Video->GetPanelHeight()));
		}
		tab->Edit->SetMinSize(wxSize(-1, h));
		tab->MainSizer->Layout();
	}
}

bool TabPanel::Hide()
{
	//Todo: check if grid is not descendant of tabpanel
	wxWindow *win = FindFocus();
	if (win && IsDescendant(win)){
		lastFocusedWindowId = win->GetId();
	}
	else{ lastFocusedWindowId = 0; }
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

void TabPanel::OnSize(wxSizeEvent & evt)
{
	if (!Edit->IsShown() && !ShiftTimes->IsShown() && !Grid->IsShown()) {
		wxSize tabSize = GetClientSize();
		Video->SetMinSize(tabSize);
		//Layout();
	}
	evt.Skip();
}

//void TabPanel::OnNavigation(wxNavigationKeyEvent& evt)
//{
//	SetNextControl(evt.GetDirection());
//}
//
//void TabPanel::SetNextControl(bool next)
//{
//	wxWindow* focused = FindFocus();
//	wxWindow* focusedParent = focused->GetParent();
//	bool nextWindowWasNULL = false;
//	if (focusedParent->IsKindOf(CLASSINFO(KaiChoice))) {
//		focused = focusedParent;
//		focusedParent = focusedParent->GetParent();
//	}
//
//	wxWindowList& list = focusedParent->GetChildren();
//	auto node = list.Find(focused);
//	if (node) {
//		auto nextWindow = next ? node->GetNext() : node->GetPrevious();
//		while (1) {
//			if (!nextWindow) {
//				if (nextWindowWasNULL)
//					break;
//
//				nextWindowWasNULL = true;
//				wxWindow* fparent = focusedParent;
//				while (fparent) {
//					wxWindowList& list1 = fparent->GetParent()->GetChildren();
//					//if panel is empty then just continue
//					//don't give it focus
//					if (!list1.GetCount()) {
//						break;
//					}
//					auto node1 = list1.Find(fparent);
//					if (node1) {
//						fparent = fparent->GetParent();
//						nextWindow = next ? node1->GetNext() : node1->GetPrevious();
//						FindFocusable(next, &nextWindow);
//						if (!nextWindow)
//							nextWindow = next ? list1.GetFirst() : list1.GetLast();
//						FindFocusable(next, &nextWindow);
//						if (nextWindow)
//							break;
//					}
//					else
//						fparent = nullptr;
//				}
//				if(!nextWindow)
//					nextWindow = next ? list.GetFirst() : list.GetLast();
//			}
//			if (nextWindow) {
//				wxObject* data = nextWindow->GetData();
//				if (data) {
//					wxWindow* win = wxDynamicCast(data, wxWindow);
//					while(win->IsKindOf(CLASSINFO(wxPanel)) || win->HasMultiplePages()){
//						wxWindowList& list1 = win->GetChildren();
//						//if panel is empty then just continue
//						//don't give it focus
//						if (!list1.GetCount()) {
//							win = nullptr;
//							break;
//						}
//						nextWindow = next ? list1.GetFirst() : list1.GetLast();
//						wxObject* data1 = nextWindow->GetData();
//						if (data1) {
//							win = wxDynamicCast(data1, wxWindow);
//						}
//					}
//					if (win && win->IsFocusable()) {
//						win->SetFocus(); return;
//					}
//					
//				}
//			}
//			nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
//		}
//	}
//}
//
//void TabPanel::FindFocusable(bool next, wxWindowListNode** node)
//{
//	wxWindowListNode* nextWindow = *node;
//	while (nextWindow) {
//		//check the window and return focusable window
//		//to avoid infinite loop
//		wxObject* data = nextWindow->GetData();
//		if (data) {
//			wxWindow* win = wxDynamicCast(data, wxWindow);
//			if (win && win->IsFocusable()) {
//				break;
//			}
//		}
//		nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
//	}
//	*node = nextWindow;
//}

BEGIN_EVENT_TABLE(TabPanel, wxWindow)
EVT_SIZE(TabPanel::OnSize)
EVT_CHILD_FOCUS(TabPanel::OnFocus)
END_EVENT_TABLE()
