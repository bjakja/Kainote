//  Copyright (c) 2018 - 2020, Marcin Drob

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
#include "hotkeys.h"
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>

wxDEFINE_EVENT(BEFORE_CHANGING_TAB, wxCommandEvent);
wxDEFINE_EVENT(TAB_CHANGED, wxCommandEvent);

KaiTabBar::KaiTabBar(wxWindow * parent, int id, const wxPoint & position /*= wxDefaultPosition*/, const wxSize & size /*= wxDefaultSize*/)
	:wxWindow(parent, id, position, size)
{
	Bind(wxEVT_PAINT, &KaiTabBar::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiTabBar::OnMouseEvent, this);
	//Bind(wxEVT_LEFT_UP, &KaiTabBar::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiTabBar::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiTabBar::OnMouseEvent, this);
	Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& evt) {Refresh(false); });
	Bind(wxEVT_KILL_FOCUS, [=](wxFocusEvent& evt) {Refresh(false); });
	wxAcceleratorEntry entries[4];
	
	entries[0] = Hkeys.GetHKey(idAndType(GLOBAL_NEXT_TAB));
	entries[1] = Hkeys.GetHKey(idAndType(GLOBAL_PREVIOUS_TAB));
	entries[2].Set(wxACCEL_NORMAL, WXK_LEFT, ID_GO_TO_LEFT_TAB);
	entries[3].Set(wxACCEL_NORMAL, WXK_RIGHT, ID_GO_TO_RIGHT_TAB);

	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);

	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if (tabs.size() < 2){ return; }
		int id = evt.GetId();
		if (id >= ID_GO_TO_LEFT_TAB && id <= ID_GO_TO_RIGHT_TAB && !HasFocus()) {
			wxNavigationKeyEvent event;
			event.SetDirection(id == ID_GO_TO_RIGHT_TAB);
			event.SetWindowChange(false);
			event.SetFromTab(false);
			event.SetEventObject(this);
			wxWindow* win = GetParent();
			while (win) {
				if (win->GetEventHandler()->ProcessEvent(event))
					break;
				win = win->GetParent();
			}
			return;
		}
		int newTab = (id == GLOBAL_NEXT_TAB || id == ID_GO_TO_RIGHT_TAB) ? currentTab + 1 : currentTab - 1;
		if (newTab < 0){ newTab = tabs.size() - 1; }
		else if (newTab >= (int)tabs.size()){ newTab = 0; }
		SetTab(newTab);
	}, GLOBAL_NEXT_TAB, ID_GO_TO_RIGHT_TAB);

	int x = 0, y = 0;
	GetTextExtent(L"#TWFfGHj", &x, &y);
	y += 8;
	if (y > tabHeader){
		tabHeader = y;
	}
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
	if (i < 0)
		i = currentTab;
	if (i < 0 || i >= tabs.size())
		return NULL;

	return tabs[i]->tab;
}

wxString KaiTabBar::GetTabName(int i /*= -1*/)
{
	if (i < 0)
		i = currentTab;
	if (i < 0 || i >= tabs.size())
		return L"";

	return tabs[i]->tabName;
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
		tabs[i]->tab->SetPosition(wxPoint(0, tabHeader));
	}
	SetMinSize(wxSize(width, height + tabHeader));
	SetColours(Options.GetColour(WINDOW_BACKGROUND), Options.GetColour(WINDOW_TEXT));
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
	const wxColour & tabsBarBackground = Options.GetColour(TABS_BACKGROUND_INACTIVE);
	const wxColour & tabsBarBackgroundHover = Options.GetColour(TABS_BACKGROUND_INACTIVE_HOVER);
	const wxColour & activeLines = Options.GetColour(TABS_BORDER_ACTIVE);
	const wxColour & tabsBarBorderInactive = Options.GetColour(TABS_BORDER_INACTIVE); 
	const wxColour & activeText = Options.GetColour(TABS_TEXT_ACTIVE);
	const wxColour & inactiveText = Options.GetColour(TABS_TEXT_INACTIVE);
	const wxColour & tabsBarBackgroundActive = Options.GetColour(TABS_BACKGROUND_ACTIVE);

	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(Options.GetColour(WINDOW_BACKGROUND));
	tdc.DrawRectangle(0, 0, w, tabHeader);
	tdc.SetTextForeground(inactiveText);
	tdc.SetPen(tabsBarBorderInactive);

	int posX = 2;
	int posY = 4;
	int currentTabPos = 0;
	for (size_t i = 0; i < tabs.size(); i++){
		//bottom line to width of window
		if (i >= tabs.size()-1)
			tdc.DrawLine(posX + tabs[i]->tabSize, tabHeader - 1, w - 2, tabHeader - 1);
		//first vertical line not needed if first tab is current tab
		if (i == 0 && i != currentTab)
			tdc.DrawLine(posX, posY, posX, tabHeader - 1);
		//save current tab position and continue
		if (i == currentTab){
			currentTabPos = posX;
			posX += tabs[i]->tabSize;
			continue;
		}
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush((i == tabHighlighted) ? tabsBarBackgroundHover : tabsBarBackground);
		//fill of tab
		tdc.DrawRectangle(posX + 1, posY + 1, tabs[i]->tabSize - 1, tabHeader - posY - 2);
		tdc.SetPen(tabsBarBorderInactive);
		//top tab line
		tdc.DrawLine(posX, posY, posX + tabs[i]->tabSize, posY);
		//bottom tab line
		tdc.DrawLine(posX, tabHeader - 1, posX + tabs[i]->tabSize, tabHeader - 1);
		//right tab line
		tdc.DrawLine(posX + tabs[i]->tabSize, posY, posX + tabs[i]->tabSize, tabHeader - 1);
		//text of tab
		tdc.DrawText(tabs[i]->tabName, posX + 5, posY + ((tabHeader - textHeight) / 2) - 2);
		posX += tabs[i]->tabSize;
	}

	if (currentTab >= 0 || currentTab < tabs.size()) {
		//current tab
		tdc.SetTextForeground(activeText);
		tdc.SetBrush(tabsBarBackgroundActive);
		tdc.SetPen(tabsBarBackgroundActive);
		tdc.DrawRectangle(currentTabPos, 2, tabs[currentTab]->tabSize, tabHeader - 2);
		tdc.SetPen(activeLines);
		tdc.DrawText(tabs[currentTab]->tabName, currentTabPos + 5, 2 + ((tabHeader - textHeight) / 2));
		currentTabPos -= 1;
		tdc.DrawLine(currentTabPos, 2, currentTabPos, tabHeader - 1);
		tdc.DrawLine(currentTabPos, 2, currentTabPos + tabs[currentTab]->tabSize + 1, 2);
		tdc.DrawLine(currentTabPos + tabs[currentTab]->tabSize + 1, 2, currentTabPos + tabs[currentTab]->tabSize + 1, tabHeader - 1);
		if (HasFocus()) {
			wxPoint frame[5] = { wxPoint(currentTabPos + 2, 4), 
				wxPoint(currentTabPos + tabs[currentTab]->tabSize - 1, 4), 
				wxPoint(currentTabPos + tabs[currentTab]->tabSize - 1, tabHeader - 2),
				wxPoint(currentTabPos + 2, tabHeader - 2), wxPoint(currentTabPos + 2, 4) };
			DrawDashedLine(&tdc, frame, 5, 1, Options.GetColour(TABS_TEXT_INACTIVE));
		}
	}
	//blit
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, tabHeader, &tdc, 0, 0);
}

void KaiTabBar::OnMouseEvent(wxMouseEvent &event)
{
	int numTab = FindCurrentTab(event.GetPosition());
	//remove highlight if have one
	if (tabHighlighted != -1 && (numTab < 0 || event.Leaving())){
		tabHighlighted = -1;
		RefreshTabBar();
		return;
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
	if (pos.y <= posY || pos.y > tabHeader)
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

wxIMPLEMENT_ABSTRACT_CLASS(KaiTabBar, wxWindow);