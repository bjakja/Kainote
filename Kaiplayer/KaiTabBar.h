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

	void AddTab(wxWindow * tab, const wxString & tabName);
	void SetTab(int tabNum);
	void Fit();

private:
	void CalculateTabsSizes();
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void RefreshTabBar();
	void SetColours(const wxColour &bgcol, const wxColour &fgcol);
	std::vector<TabData *> tabs;
	int tabHeader = 15;
	int currentTab = 0;
};