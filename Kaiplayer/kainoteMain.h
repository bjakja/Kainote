/***************************************************************
 * Name:      kainoteMain.h
 * Purpose:   Subtitles editor and player
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * Copyright: Marcin Drob aka Bjakja (http://animesub.info/forum/viewtopic.php?id=258715)
 * License:

 * Kainote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Kainote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

 **************************************************************/

#pragma once

#include <wx/timer.h>
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include "KaiFrame.h"
#include "Tabs.h"
#include <vector>
#include "TabPanel.h"
#include "SpellChecker.h"
#include "StyleStore.h"
#include "FindReplaceDialog.h"
#include "SelectLines.h"
#include "Automation.h"
#include "Toolbar.h"
#include "KaiStatusBar.h"
#include "LogHandler.h"
#include "MisspellReplacer.h"

class FontCollector;

class KainoteFrame : public KaiFrame
{
public:
	KainoteFrame(const wxPoint &pos, const wxSize &size);
	virtual ~KainoteFrame();

	Notebook* Tabs;

	//wxBoxSizer *mains;
	void Save(bool dial, int wtab = -1, bool changeLabel = true);
	void SaveAll();

	bool OpenFile(const wxString &filename, bool fulls = false, bool freeze = true);
	void Label(int iter = 0, bool video = false, int wtab = -1, bool onlyTabs = false);

	void SetRecent(short what = 0, int tab = -1);
	void AppendRecent(short what = 0, Menu *Menu = 0);
	void SetAccels(bool all = true);
	bool FindFile(const wxString &fn, wxString &foundFile, bool video);

	Menu* VidsRecMenu;
	Menu* SubsRecMenu;
	Menu* AudsRecMenu;
	Menu* KeyframesRecentMenu;
	MenuBar* Menubar;

	KaiStatusBar* StatusBar;
	KaiToolbar *Toolbar;
	wxArrayString subsrec;
	wxArrayString videorec;
	wxArrayString audsrec;
	wxArrayString keyframesRecent;
	void OnMenuSelected(wxCommandEvent& event);
	void OnMenuSelected1(wxCommandEvent& event);
	void OnRecent(wxCommandEvent& event);

	TabPanel* GetTab();

	void InsertTab(bool refresh = true);
	void OpenFiles(wxArrayString &files, bool intab = false, bool nofreeze = false, bool newtab = false);
	void OpenAudioInTab(TabPanel *pan, int id, const wxString &path);
	void HideEditor(bool save = true);
	bool SavePrompt(char mode = 1, int wtab = -1);
	void UpdateToolbar();
	void OnOpenAudio(wxCommandEvent& event);
	void OnMenuClick(wxCommandEvent &event);
	void SetStatusText(const wxString &label, int field){ StatusBar->SetLabelText(field, label); }
	wxString GetStatusText(int field){ return StatusBar->GetStatusText(field); }
	void SetSubsResolution(bool dialog = false);
	void SetVideoResolution(int w, int h, bool dialog = false);
	void ShowBadResolutionDialog(const wxSize &videoRes, const wxSize &subsRes);
	void OnSize(wxSizeEvent& event);
	bool Layout();
	FindReplaceDialog *FR;
	SelectLines *SL;
	Auto::Automation *Auto;
	FontCollector *fc;
	MisspellReplacer *MR = NULL;
	RECT borders;
private:


	void OnConversion(char form);
	void OnAssProps();
	void OnMenuOpened(MenuEvent& event);
	void OnP5Sec(wxCommandEvent& event);
	void OnM5Sec(wxCommandEvent& event);
	void OnAudioSnap(wxCommandEvent& event);
	void OnPageChanged(wxCommandEvent& event);
	void OnPageChange(wxCommandEvent& event);
	void OnPageAdd(wxCommandEvent& event);
	void OnPageClose(wxCommandEvent& event);
	void OnRunScript(wxCommandEvent& event);
	void OnChangeLine(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
	void OnClose1(wxCloseEvent& event);
	void OnActivate(wxActivateEvent &evt);
	static void OnOutofMemory();
	Menu* ConvMenu;
	Menu* FileMenu;
	Menu* HelpMenu;
	Menu* SubsMenu;
	Menu* EditMenu;
	Menu* VidMenu;
	Menu* AudMenu;
	Menu* ViewMenu;
	Menu* AutoMenu;

	//wxLogWindow *mylog;
	bool badResolution;

	wxMutex blockOpen;
	wxTimer sendFocus;
};




enum{
	ID_ADDPAGE = 6900,
	ID_CLOSEPAGE,
	ID_TABS,
	ID_STATUSBAR1,
	ID_CONV
};

