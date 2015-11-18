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
		int splitline;
		int splititer;
		int oldtab;
		wxDialog* sline;
		wxFont font;
		//wxFrame *TD;
		std::vector<TabPanel*> Pages;
		wxArrayInt Tabsizes;
		wxArrayString Names;
		static HHOOK g_SSHook;  
		static LRESULT CALLBACK BlockSSaver( int code, WPARAM wParam, LPARAM lParam );

		DECLARE_EVENT_TABLE()
	};

enum{
	MENU_CHOOSE=12347,
	MENU_SAVE=23677
	};

#endif