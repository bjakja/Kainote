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

wxDEFINE_EVENT(BEFORE_CHANGING_TAB, wxCommandEvent);
wxDEFINE_EVENT(TAB_CHANGED, wxCommandEvent);

KaiTabBar::KaiTabBar(wxWindow * parent, int id, const wxPoint & position /*= wxDefaultPosition*/, const wxSize & size /*= wxDefaultSize*/)
{
	Bind(wxEVT_PAINT, &KaiTabBar::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiTabBar::OnMouseEvent, this);
	//Bind(wxEVT_LEFT_UP, &KaiTabBar::OnMouseEvent, this);
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

	wxCommandEvent evt(BEFORE_CHANGING_TAB, GetId());
	ProcessEvent(evt);
	tabs[currentTab]->tab->Show(false);
	currentTab = tabNum;
	RefreshTabBar();
	tabs[currentTab]->tab->Show();
	wxCommandEvent evt1(TAB_CHANGED, GetId());
	ProcessEvent(evt1);
}

wxWindow * KaiTabBar::GetTab(int i)
{
	if (i < 0 || i >= tabs.size())
		return NULL;

	return tabs[i]->tab;
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
		tabs[i]->tab->SetPosition(wxPoint(0, 0));
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
		if (textHeight < size.y)
			textHeight = size.y;
	}
}

void KaiTabBar::OnPaint(wxPaintEvent& event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	wxMemoryDC tdc;
	tdc.SelectObject(wxBitmap(w, tabHeader));
	tdc.SetFont(GetFont());
	const wxColour & tabsBarBackground = Options.GetColour(TabsBarBackground1);
	const wxColour & tabsBarBackgroundHover = Options.GetColour(TabsBackgroundInactiveHover);
	const wxColour & activeLines = Options.GetColour(TabsBorderActive);
	const wxColour & tabsBarBorderInactive = Options.GetColour(TabsBorderInactive); 
	const wxColour & activeText = Options.GetColour(TabsTextActive);
	const wxColour & inactiveText = Options.GetColour(TabsTextInactive);
	const wxColour & tabsBarBackgroundActive = Options.GetColour(TabsBackgroundActive);
	
	tdc.SetTextForeground(inactiveText);
	tdc.SetPen(tabsBarBorderInactive);

	int posX = 2;
	int posY = 4;
	int currentTabPos = 0;
	for (size_t i = 0; i < tabs.size(); i++){
		//bottom line to width of window
		if (i >= tabs.size()-1)
			tdc.DrawLine(posX + tabs[i]->tabSize, tabHeader - posY, w - 2, tabHeader - posY);
		//first vertical line not needed if first tab is current tab
		if (i == 0 && i != currentTab)
			tdc.DrawLine(posX, posY, posX, tabHeader - posY);
		//save current tab position and continue
		if (i == currentTab){
			currentTabPos = posX;
			continue;
		}
		tdc.SetBrush((i == tabHighlighted) ? tabsBarBackgroundHover : tabsBarBackground);
		//fill of tab
		tdc.DrawRectangle(posX + 1, posY + 1, tabs[i]->tabSize - 2, tabHeader - posY - 2);
		//top tab line
		tdc.DrawLine(posX, posY, posX + tabs[i]->tabSize, posY);
		//bottom tab line
		tdc.DrawLine(posX, tabHeader - posY, posX + tabs[i]->tabSize, tabHeader - posY);
		//right tab line
		tdc.DrawLine(posX + tabs[i]->tabSize, posY, posX + tabs[i]->tabSize, tabHeader - posY);
		//text of tab
		tdc.DrawText(tabs[i]->tabName, posX + 5, posY + ((tabHeader - textHeight) / 2));
		posX += tabs[i]->tabSize;
	}

	if (currentTab < 0 || currentTab >= tabs.size())
		return;
	//current tab
	tdc.SetTextForeground(inactiveText);
	tdc.SetBrush(tabsBarBackground);
	tdc.SetPen(tabsBarBorderInactive);
	tdc.DrawRectangle(currentTabPos, 2, tabs[currentTab]->tabSize - 2, tabHeader - 2);
	tdc.DrawText(tabs[currentTab]->tabName, currentTabPos + 5, 2 + ((tabHeader - textHeight) / 2));
	currentTabPos -= 1;
	tdc.DrawLine(currentTabPos, 2, currentTabPos, tabHeader - 2);
	tdc.DrawLine(currentTabPos, 2, currentTabPos + tabs[currentTab]->tabSize + 2, 2);
	tdc.DrawLine(currentTabPos + tabs[currentTab]->tabSize + 2, 2, currentTabPos + tabs[currentTab]->tabSize + 2, tabHeader - 2);

	//blit
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, tabHeader, &tdc, 0, 0);
}

void KaiTabBar::OnMouseEvent(wxMouseEvent &event)
{
	int numTab = FindCurrentTab(event.GetPosition());
	//remove highlight if have one
	if (numTab < 0){
		tabHighlighted = -1;
	}
	//clicking tab
	if (event.LeftDown()){
		SetTab(numTab);
	}
	else if (numTab != lastTab){
		tabHighlighted = numTab;
		RefreshTabBar();
	}
}

void KaiTabBar::RefreshTabBar()
{
	int w, h;
	GetClientSize(&w, &h);
	wxRect rc(0, 0, w, tabHeader);
	Refresh(false, &rc);
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

int KaiTabBar::FindCurrentTab(const wxPoint &pos)
{
	int posX = 2;
	int posY = 2;
	if (pos.y > posY)
		return -1;

	for (size_t i = 0; i < tabs.size(); i++){
		if (pos.x > posX && pos.x < (posX += tabs[i]->tabSize)){
			if (i != currentTab && pos.y < 5)
				return -1;

			return i;
		}
		//posX += tabs[i]->tabSize;
	}
	return -1;
}
