//  Copyright (c) 2018, Marcin Drob

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

#include <wx/window.h>
#include <vector>

wxDECLARE_EVENT(BEFORE_CHANGING_TAB, wxCommandEvent);
wxDECLARE_EVENT(TAB_CHANGED, wxCommandEvent);

class TabData
{
public:
	TabData(wxWindow * _tab, const wxString & _tabName){
		tab = _tab;
		tabName = _tabName;
	}
	wxWindow * tab;
	wxString tabName;
	int tabSize=0;
};

class KaiTabBar : public wxWindow
{
public:
	KaiTabBar(wxWindow * parent, int id, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize);
	virtual ~KaiTabBar();
	//After adding tabs have to call Fit() method to setup sizes
	void AddTab(wxWindow * tab, const wxString & tabName);
	void SetTab(int tabNum);
	wxWindow *GetTab(int i = -1);
	wxString GetTabName(int i = -1);
	void Fit();

private:
	void CalculateTabsSizes();
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void RefreshTabBar();
	void SetColours(const wxColour &bgcol, const wxColour &fgcol);
	int FindCurrentTab(const wxPoint &pos);

	std::vector<TabData *> tabs;
	int tabHeader = 28;
	int currentTab = 0;
	int lastTab = -1;
	int textHeight = 0;
	int tabHighlighted = -1;
};