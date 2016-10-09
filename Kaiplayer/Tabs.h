#ifndef TABDIALOG
#define TABDIALOG

#pragma once

#include <wx/wx.h>
#include <vector>
//#include <wx/process.h>

class TabPanel;

class Notebook : public wxWindow
	{
	public:
		Notebook(wxWindow *parent, int id);
		
		virtual ~Notebook();
		TabPanel *GetPage();
		void AddPage(bool refresh=true);
		void SetPageText(int page, wxString label);
		void ChangePage(int page);
		TabPanel *GetSecondPage();
		int GetSelection();
		int GetOldSelection();
		size_t Size();
		TabPanel *Page(size_t i);
		void DeletePage(size_t page);
		void Split(size_t page);
		int FindTab(int x, int *num);
		int FindPanel(TabPanel* pan);
		int GetHeight();
		void ChangeActiv();
		void RefreshBar();
		void SubsComparsion();

		size_t iter;
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
		void OnCharHook(wxKeyEvent& event);
		void CalcSizes();
		void CompareTexts(wxString &first, wxString &second, wxArrayInt &firstCompare, wxArrayInt &secondCompare);
		int TabHeight;
		size_t olditer;
		int over;
		size_t fvis;
		int start;
		bool onx;
		bool farr;
		bool rarr;
		bool plus;
		bool allvis;
		bool arrow;
		bool hasCompare;
		int compareFirstTab;
		int compareSecondTab;
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
		static LRESULT CALLBACK PauseOnMinimalize( int code, WPARAM wParam, LPARAM lParam );

		DECLARE_EVENT_TABLE()
	};

enum{
	MENU_CHOOSE=12347,
	MENU_SAVE=23677,
	MENU_COMPARE=16888
	};

#endif