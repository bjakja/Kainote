/***************************************************************
 * Copyright (c) 2012-2020, Marcin Drob
 * Name:      kainoteMain.h
 * Purpose:   Subtitles editor and player
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
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

#include "KaiFrame.h"
#include "FontCollector.h"
#include "KaiStatusBar.h"
#include "Automation.h"
#include <wx/timer.h>
#include <wx/window.h>


//class FontCollector;
class FindReplaceDialog;
class SelectLines;
class MisspellReplacer;
class Automation;
class Notebook;
class Menu;
class MenuBar;
class KaiToolbar;
class TabPanel;
class MenuEvent;




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
	void OnUseWindowHotkey(wxCommandEvent& event);
	TabPanel* GetTab();

	void InsertTab(bool refresh = true);
	void OpenFiles(wxArrayString &files, bool intab = false, bool nofreeze = false, bool newtab = false);
	void OpenAudioInTab(TabPanel *pan, int id, const wxString &path);
	void HideEditor(bool save = true);
	bool SavePrompt(char mode = 1, int wtab = -1);
	void UpdateToolbar();
	void OnOpenAudio(wxCommandEvent& event);
	void SetStatusText(const wxString& label, int field);
	wxString GetStatusText(int field) { return StatusBar->GetStatusText(field); }
	void SetSubsResolution(bool dialog = false);
	void SetVideoResolution(int w, int h, bool dialog = false);
	void ShowBadResolutionDialog(const wxSize &videoRes, const wxSize &subsRes);
	void OnSize(wxSizeEvent& event);
	bool Layout();
	void DestroyDialogs();
	const static std::locale &GetLocale();
	FindReplaceDialog *FR = nullptr;
	SelectLines *SL = nullptr;
	Auto::Automation *Auto = nullptr;
	FontCollector *FC = nullptr;
	MisspellReplacer *MR = nullptr;
	wxRect borders;
private:


	void OnConversion(char form);
	void OnAssProps();
	void OnMenuOpened(MenuEvent& event);
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
	void OnExternalSession(int id);

	Menu* ConvMenu;
	Menu* FileMenu;
	Menu* HelpMenu;
	Menu* SubsMenu;
	Menu* EditMenu;
	Menu* VidMenu;
	Menu* AudMenu;
	Menu* ViewMenu;
	Menu* m_AutoMenu;

	bool badResolution;

	wxMutex m_BlockOpen;
	wxTimer m_SendFocus;
	static std::locale locale;
};




enum{
	ID_TABS = 6900,
	ID_STATUS_BAR,
	ID_CONVERSION,
	GLOBAL_ASK_FOR_LOAD_LAST_SESSION,
	GLOBAL_LOAD_LAST_SESSION_ON_START,
};

