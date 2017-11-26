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


#pragma once

#include <wx/wx.h>
#include <vector>
//#include <wx/process.h>

class TabPanel;
class SubsGrid;

class Notebook : public wxWindow
{
public:
	Notebook(wxWindow *parent, int id);

	virtual ~Notebook();
	TabPanel *GetPage();
	void AddPage(bool refresh = true);
	void SetPageText(int page, const wxString &label);
	void ChangePage(int page);
	TabPanel *GetSecondPage();
	int GetIterByPos(const wxPoint &pos);
	int GetSelection();
	int GetOldSelection();
	int Size();
	TabPanel *Page(size_t i);
	void DeletePage(int page);
	void Split(size_t page);
	void RemoveComparison();
	int FindTab(int x, int *num);
	int FindPanel(TabPanel* pan);
	int GetHeight();
	void ChangeActive();
	void RefreshBar();
	void SubsComparison();

	int iter;
	bool block;
	bool split;
	static Notebook *GetTabs();
	static Notebook *sthis;
	static TabPanel *GetTab();

private:

	void OnTabSel(int id);
	void OnMouseEvent(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnSave(int id);
	void OnSize(wxSizeEvent& event);
	static void OnResized();
	void OnEraseBackground(wxEraseEvent &event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); } };
	void OnCharHook(wxKeyEvent& event);
	void CalcSizes(bool makeActiveVisible = false);
	void CompareTexts(const wxString &first, const wxString &second, wxArrayInt &firstCompare, wxArrayInt &secondCompare);
	int TabHeight;
	int olditer;
	int over;
	int firstVisibleTab;
	int start;
	bool onx;
	bool farr;
	bool rarr;
	bool plus;
	bool allTabsVisible;
	bool arrow;
	bool hasCompare;
	//int compareFirstTab;
	//int compareSecondTab;
	SubsGrid *compareFirstGrid;
	SubsGrid *compareSecondGrid;
	int splitline;
	int splititer;
	int oldtab;
	int oldI;
	wxDialog* sline;
	wxFont font;
	//wxFrame *TD;
	std::vector<TabPanel*> Pages;
	wxArrayInt Tabsizes;
	wxArrayString Names;
	HHOOK Hook;
	//HHOOK Hooktest; 
	//static LRESULT CALLBACK testhook( int code, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK PauseOnMinimalize(int code, WPARAM wParam, LPARAM lParam);

	DECLARE_EVENT_TABLE()
};

enum{
	MENU_CHOOSE = 12347,
	MENU_SAVE = 23677,
	MENU_COMPARE = 16888,
	COMPARE_BY_TIMES = 1,
	COMPARE_BY_STYLES,
	COMPARE_BY_CHOSEN_STYLES = 4,
	COMPARE_BY_VISIBLE = 8
};

