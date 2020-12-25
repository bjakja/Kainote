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


#pragma once

#include <wx/wx.h>
#include <vector>
//#include <wx/process.h>

class TabPanel;
class SubsGrid;
class KainoteFrame;

class Notebook : public wxWindow
{
public:
	Notebook(wxWindow *parent, int id);

	virtual ~Notebook();
	TabPanel *GetPage();
	void AddPage(bool refresh = true);
	void SetPageText(int page, const wxString &label);
	void ChangePage(int page, bool makeActiveVisible = false);
	TabPanel *GetSecondPage();
	int GetIterByPos(const wxPoint &pos);
	int GetSelection();
	int GetOldSelection();
	size_t Size();
	TabPanel *Page(size_t i);
	void DeletePage(int page);
	void Split(size_t page);
	int FindTab(int x, int *num);
	int FindPanel(TabPanel* pan, bool safe = true);
	int GetHeight();
	void ChangeActive();
	void RefreshBar(bool checkSizes = false);
	bool LoadSubtitles(TabPanel *tab, const wxString & path, int active = -1, int scroll = -1);
	int LoadVideo(TabPanel *tab, const wxString & path, int position = -1, 
		bool isFFMS2 = true, bool hasEditor = true, bool fullscreen = false, bool loadPrompt = false, bool dontLoadAudio = false);
	bool SetFont(const wxFont &font);
	void ResetPrompt() { promptResult = 0; }

	int iter;
	bool block;
	bool split;
	static Notebook *GetTabs();
	static Notebook *sthis;
	static TabPanel *GetTab();
	static void RefreshVideo(bool reloadLibass = false);
	static void SaveLastSession(bool beforeClose = false);
	static void LoadLastSession();
	//results 0 - no session, 1 - normal session saved at end, 2 crash or bad close session
	static int CheckLastSession();

private:
	void ContextMenu(const wxPoint &pos, int i);
	void OnTabSel(int id);
	void OnMouseEvent(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnSave(int id);
	void OnSize(wxSizeEvent& event);
	static void OnResized();
	void OnEraseBackground(wxEraseEvent &event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ splitLineHolding = false; if (HasCapture()){ ReleaseMouse(); } };
	void OnCharHook(wxKeyEvent& event);
	void OnScrollTabs(wxTimerEvent &event);
	void CalcSizes(bool makeActiveVisible = false);
	
	int TabHeight;
	int olditer;
	int over;
	int firstVisibleTab;
	int start;
	int xWidth;
	bool onx;
	bool leftArrowHover;
	bool rightArrowHover;
	bool leftArrowClicked = false;
	bool rightArrowClicked = false;
	bool newTabHover;
	bool allTabsVisible;
	bool arrow;
	bool splitLineHolding = false;
	int splitline;
	int splititer;
	int oldtab;
	int oldI;
	int maxCharPerTab = 40;
	int promptResult = 0;
	int tabOffset = 0;
	int tabScrollDestination = 0;
	int x = 0;
	wxDialog* sline;
	wxFont font;
	std::vector<TabPanel*> Pages;
	wxArrayInt tabSizes;
	wxArrayString tabNames;
	wxTimer tabsScroll;
	HHOOK Hook;
	KainoteFrame *Kai;
	wxMutex closeTabMutex;

	static LRESULT CALLBACK PauseOnMinimalize(int code, WPARAM wParam, LPARAM lParam);

	DECLARE_EVENT_TABLE()
};

enum{
	MENU_CHOOSE = 12347,
	MENU_SAVE = 23677,
	MENU_COMPARE = 16888,
	MENU_OPEN_SUBS_FOLDER = 16999,
	MENU_OPEN_VIDEO_FOLDER,
	MENU_OPEN_AUDIO_FOLDER,
	MENU_OPEN_KEYFRAMES_FOLDER,
	COMPARE_BY_TIMES = 1,
	COMPARE_BY_STYLES,
	COMPARE_BY_CHOSEN_STYLES = 4,
	COMPARE_BY_VISIBLE = 8,
	COMPARE_BY_SELECTIONS = 16
};

