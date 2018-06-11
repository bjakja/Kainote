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

#include "KaiTabBar.h"
#include "config.h"

KaiTabBar::KaiTabBar(wxWindow * parent, int id, const wxPoint & position /*= wxDefaultPosition*/, const wxSize & size /*= wxDefaultSize*/)
{
	Bind(wxEVT_PAINT, &KaiTabBar::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiTabBar::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiTabBar::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiTabBar::OnMouseEvent, this);
}

KaiTabBar::~KaiTabBar()
{
	for (auto it = tabs.begin(); it != tabs.end(); it++){
		delete *it;
	}
}

void KaiTabBar::AddTab(wxWindow * tab, const wxString & tabName)
{
	tab->Hide();
	tabs.push_back(new TabData(tab, tabName));
}

void KaiTabBar::SetTab(int tabNum)
{
	if (tabNum < 0 || tabNum >= tabs.size())
		return;

	currentTab = tabNum;
	Refresh(false);
}

void KaiTabBar::Fit()
{
	CalculateTabsSizes();
	int width = 0, height = 0;
	for (size_t i = 0; i < tabs.size(); i++){
		wxSize pageSize = tabs[i]->tab->GetBestSize();
		if (pageSize.x > width){
			width = pageSize.x;
		}
		if (pageSize.y > height){
			height = pageSize.y;
		}
	}
	SetMinSize(wxSize(width, height+tabHeader));
	SetColours(Options.GetColour(WindowBackground), Options.GetColour(WindowText));
	tabs[currentTab]->tab->Show();
}

void KaiTabBar::CalculateTabsSizes()
{
	for (size_t i = 0; i < tabs.size(); i++){
		wxSize size = GetTextExtent(tabs[i]->tabName);
		tabs[i]->tabSize = size.x + 10;
	}
}

void KaiTabBar::OnPaint(wxPaintEvent& event)
{
	int w, h;
	GetClientSize(&w, &h);
	wxRect rc(0, 0, w, tabHeader);
	Refresh(false, &rc);
}

void KaiTabBar::OnMouseEvent(wxMouseEvent &event)
{

}

void KaiTabBar::RefreshTabBar()
{

}

void KaiTabBar::SetColours(const wxColour &bgcol, const wxColour &fgcol)
{
	for (size_t i = 0; i < tabs.size(); i++){
		tabs[i]->tab->SetBackgroundColour(bgcol);
		tabs[i]->tab->SetForegroundColour(fgcol);
	}
	SetBackgroundColour(bgcol);
	SetForegroundColour(fgcol);
}