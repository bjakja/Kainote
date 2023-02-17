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



#include "Notebook.h"

#include "SubtitlesProviderManager.h"
#include "Menu.h"
#include "KaiMessageBox.h"
#include "OpennWrite.h"
#include "VideoBox.h"


#include "Editbox.h"
#include "SubsGrid.h"
#include "Toolbar.h"
#include "TabPanel.h"
#include "shiftTimes.h"
#include "KainoteFrame.h"


Notebook::Notebook(wxWindow *parent, int id)
	: wxWindow(parent, id)
	, Kai((KainoteFrame*)parent)
{
	firstVisibleTab = olditer = iter = 0;
	splitline = splititer = 0;
	oldtab = oldI = over = -1;
	block = split = onx = leftArrowHover = rightArrowHover = newTabHover = false;
	allTabsVisible = arrow = true;
	sline = nullptr;
	font = *Options.GetFont(-1);
	sthis = this;
	int fx, fy;
	GetTextExtent(L"X", &fx, &fy, 0, 0, &font);
	TabHeight = fy + 12;
	xWidth = fx + 10;

	int maxTextChars = Options.GetInt(TAB_TEXT_MAX_CHARS);
	if (maxTextChars > 19)
		maxCharPerTab = maxTextChars;

	Pages.push_back(new TabPanel(this, (KainoteFrame*)parent));

	wxString name = Pages[0]->SubsName;
	tabNames.Add(name);

	CalcSizes();
	Hook = nullptr;
	Hook = SetWindowsHookEx(WH_CBT, &PauseOnMinimalize, nullptr, GetCurrentThreadId());//WH_MOUSE

	tabsScroll.SetOwner(this, 6779);
	Bind(wxEVT_TIMER, &Notebook::OnScrollTabs, this, 6779);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		MenuItem *item = (MenuItem*)evt.GetClientData();
		int id = item->id;
		int compareBy = Options.GetInt(SUBS_COMPARISON_TYPE);
		switch (id)
		{
		case MENU_COMPARE + 1:
			compareBy ^= COMPARE_BY_TIMES;
			break;
		case MENU_COMPARE + 2:
			compareBy ^= COMPARE_BY_VISIBLE;
			break;
		case MENU_COMPARE + 3:
			compareBy ^= COMPARE_BY_SELECTIONS;
			break;
		case MENU_COMPARE + 4:
			compareBy ^= COMPARE_BY_STYLES;
			break;
		case 4448:
		{
			if (!SubsGridBase::compareStyles.size() && (compareBy & COMPARE_BY_CHOSEN_STYLES))
				compareBy ^= COMPARE_BY_CHOSEN_STYLES;

			wxString &name = item->label;
			bool found = false;
			for (size_t i = 0; i < SubsGridBase::compareStyles.size(); i++){
				if (SubsGridBase::compareStyles[i] == name){
					if (!item->check){ SubsGridBase::compareStyles.RemoveAt(i); }
					found = true;
					break;
				}
			}
			if (!found && item->check){ SubsGridBase::compareStyles.Add(name); }
			Options.SetTable(SUBS_COMPARISON_STYLES, SubsGridBase::compareStyles);
			if ((SubsGridBase::compareStyles.size() > 0 && !(compareBy & COMPARE_BY_CHOSEN_STYLES)) ||
				(SubsGridBase::compareStyles.size() < 1 && compareBy & COMPARE_BY_CHOSEN_STYLES)){
				compareBy ^= COMPARE_BY_CHOSEN_STYLES;
				Menu *parentMenu = nullptr;
				MenuItem * parentItem = Menu::FindItemGlobally(MENU_COMPARE + 5, &parentMenu);
				if (parentItem){
					parentItem->Check(SubsGridBase::compareStyles.size() > 0);
					if (parentMenu)
						parentMenu->RefreshMenu();
				}
			}
			break;
		}
		default:
			return;
		}
		Options.SetInt(SUBS_COMPARISON_TYPE, compareBy);
	}, ID_CHECK_EVENT);

}


Notebook::~Notebook(){
	for (std::vector<TabPanel*>::iterator i = Pages.begin(); i != Pages.end(); i++)
	{
		(*i)->Destroy();
	}
	Pages.clear();
	tabSizes.Clear();
	tabNames.Clear();
	UnhookWindowsHookEx(Hook);
}

TabPanel *Notebook::GetPage()
{
	return Pages[iter];
}

void Notebook::AddPage(bool refresh, bool saveLastSession)
{
	if (iter < Pages.size() && Pages[iter]->video->GetState() == Playing){ Pages[iter]->video->Pause(); }
	int w, h;
	GetClientSize(&w, &h);
	if(refresh){Freeze();}
	Pages.push_back(new TabPanel(this, Kai, wxPoint(0, 0), wxSize(0, 0)));
	olditer = iter;
	iter = Size() - 1;
	if (refresh){
		Pages[olditer]->Hide();
	}
	else{
		Pages[iter]->Hide();
	}
	wxString name = Pages[iter]->SubsName;
	tabNames.Add(name);

	Pages[iter]->SetPosition(Pages[olditer]->GetPosition());
	Pages[iter]->SetSize(olditer != iter ? Pages[olditer]->GetSize() : wxSize(w, h - TabHeight));
	CalcSizes(true);
	if (refresh){
		if (!Options.GetBool(EDITOR_ON)){ 
			Kai->HideEditor(false); 
		}
		wxCommandEvent choiceSelectedEvent(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		AddPendingEvent(choiceSelectedEvent);
		Thaw();
		RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
	}
	else{ Pages[iter]->shiftTimes->RefVals(Pages[olditer]->shiftTimes); }

	if (saveLastSession) {
		SaveLastSession();
	}
}

int Notebook::GetSelection()
{
	return iter;
}

int Notebook::GetOldSelection()
{
	return olditer;
}

size_t Notebook::Size()
{
	return Pages.size();
}

void Notebook::SetPageText(int page, const wxString &label)
{
	tabNames[page] = label;
	CalcSizes(true);
	int w, h;
	GetClientSize(&w, &h);
	RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
}

TabPanel *Notebook::Page(size_t i)
{
	if (i < 0 || i >= Pages.size())
		return nullptr;

	return Pages[i];
}

void Notebook::DeletePage(int page)
{
	wxMutexLocker lock(closeTabMutex);
	if (page < 0 || page >= Pages.size()) {
		KaiLog("You try to delete not existing tab");
		return;
	}

	Freeze();
	block = true;
	if (Kai->SavePrompt(1, page)){
		block = false;
		wxSize siz = GetClientSize();
		RefreshRect(wxRect(0, siz.y - TabHeight, siz.x, TabHeight), false);
		Thaw();
		return;
	}
	block = false;
	//remove compare if it exist
	SubsGridBase::RemoveComparison();

	int tmpSize = Size();
	if (split && tmpSize > 2){
		int tmpiter = iter;
		int tmpsplititer = splititer;
		if (page < iter || (page == iter && iter + 1 == splititer && iter != 0) ||
			(iter >= tmpSize - 1 && iter - 1 != splititer)){
			iter--;
		}
		else if (page == iter && iter + 1 == splititer && iter == 0){ iter += 2; }
		else if (page == iter && iter - 1 == splititer && iter >= tmpSize - 1){ iter -= 2; }
		if (page < splititer || (page == splititer && splititer + 1 == iter && splititer != 0) ||
			(splititer >= tmpSize - 1 && splititer - 1 != iter)){
			splititer--;
		}
		else if (page == splititer && splititer + 1 == iter && splititer == 0){ splititer += 2; }
		else if (page == splititer && splititer - 1 == iter && splititer >= tmpSize - 1){ splititer -= 2; }

		if (page == tmpiter || page == tmpsplititer){
			bool deleteActiveTab = page == tmpiter;
			int newVisibleTab = (deleteActiveTab) ? iter : splititer;

			Pages[newVisibleTab]->SetSize(Pages[page]->GetSize());
			Pages[newVisibleTab]->SetPosition(Pages[page]->GetPosition());
			Pages[newVisibleTab]->Show();
		}
	}
	Pages[page]->Destroy();
	Pages.erase(Pages.begin() + page);
	tabNames.RemoveAt(page);
	tabSizes.RemoveAt(page);

	if (split && Size() < 2){
		split = false;
		int w, h;
		GetClientSize(&w, &h);
		Pages[0]->SetPosition(wxPoint(0, 0));
		Pages[0]->SetSize(w, h - TabHeight);
	}


	if (Size() < 1){
		Pages.push_back(new TabPanel(this, Kai));
		wxString name = Pages[0]->SubsName;
		tabNames.Add(name);
		int w, h;
		GetClientSize(&w, &h);
		Pages[0]->SetPosition(wxPoint(0, 0));
		Pages[0]->SetSize(w, h - TabHeight);

	}

	int rsize = Size() - 1;
	if (olditer > rsize){ olditer = rsize; }
	if (!split){
		if (iter > rsize){ iter = rsize; }
		else if (page < iter){ iter--; }
		Pages[iter]->Show();
	}
	//if (page < splititer){ splititer--; }
	//else if (splititer > rsize){ splititer = rsize; }
	//if(page>  rsize){page=rsize;}
	if (firstVisibleTab > rsize){ firstVisibleTab = tabScrollDestination = rsize; }

	CalcSizes(true);
	Thaw();

	//int w,h;
	//GetClientSize(&w,&h);
	//RefreshRect(wxRect(0,h-TabHeight,w,TabHeight),false);
	Refresh(false);
	SaveLastSession();
}

void Notebook::CalcSizes(bool makeActiveVisible)
{
	int all = 2;
	int w, h;
	GetClientSize(&w, &h);
	int newHeight = 0;
	for (size_t i = 0; i < Size(); i++){
		int fw, fh;
		wxString name = tabNames[i];
		if (name.length() > maxCharPerTab){
			name = name.Mid(0, maxCharPerTab);
		}

		GetTextExtent(name, &fw, &fh, nullptr, nullptr, &font);
		if (i == iter){ fw += xWidth; }
		if (i < tabSizes.size()){ tabSizes[i] = fw + 10; }
		else{ tabSizes.Add(fw + 10); }
		all += tabSizes[i] + 2;
		if (fh + 12 > newHeight){
			newHeight = fh + 12;
		}
	}
	allTabsVisible = (all < w - 22);
	if (allTabsVisible){ firstVisibleTab = tabScrollDestination = 0; }
	if (makeActiveVisible && !allTabsVisible){
		int tabsWidth = 0;
		if (firstVisibleTab > iter)
			firstVisibleTab = tabScrollDestination = 0;

		for (size_t i = firstVisibleTab; i < tabSizes.size(); i++){

			tabsWidth += tabSizes[i];

			if (tabsWidth > w - 22){
				firstVisibleTab = tabScrollDestination = i - 1;
				if (firstVisibleTab < 0) {
					//KaiLog(L"firstVisibleTab < 0");
					firstVisibleTab = tabScrollDestination = 0;
				}
				break;
			}
			if (i == iter)
				break;
			
		}
	}
	if (newHeight != TabHeight){
		TabHeight = newHeight;
		//OnSize have all that needed after changing font size on tabs bar
		wxSizeEvent sizeEvent;
		OnSize(sizeEvent);
	}
}


void Notebook::OnMouseEvent(wxMouseEvent& event)
{
	int y = event.GetY();
	x = event.GetX();
	bool dclick = event.LeftDClick();
	bool click = event.LeftDown() || dclick;
	bool middleDown = event.MiddleDown();


	int w, h, hh;
	GetClientSize(&w, &h);
	hh = h - TabHeight;
	//Remove all activity after tabs exit

	if (event.Leaving()){
		if (over != -1 || onx || leftArrowHover || rightArrowHover || newTabHover){
			over = -1; onx = leftArrowHover = rightArrowHover = newTabHover = false;
			RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
		}
		oldtab = -1;
		UnsetToolTip();
		if (!arrow){ SetCursor(wxCURSOR_ARROW); arrow = true; }
		return;
	}

	if (tabsWasSwapped && event.ButtonUp()) {
		SaveLastSession();
		tabsWasSwapped = false;
	}

	if (y < hh && split){
		bool isInSplitLine = abs(splitline - x) < 4;
		if (click && isInSplitLine){
			CaptureMouse();
			int px = x, py = 2;
			ClientToScreen(&px, &py);
			sline = new wxDialog(this, -1, emptyString, wxPoint(px, py), wxSize(3, h - 27), wxSTAY_ON_TOP | wxBORDER_NONE);
			sline->SetBackgroundColour(L"#000000");
			sline->Show();
			splitLineHolding = true;
		}
		else if (event.LeftUp() && splitLineHolding)
		{
			int npos = x;
			if (sline){
				int yy;
				sline->GetPosition(&npos, &yy);
				ScreenToClient(&npos, &yy);
				sline->Destroy();
				sline = nullptr;
			}
			if (HasCapture()){ ReleaseMouse(); }
			splitline = npos;
			bool leftTab = (Pages[iter]->GetPosition().x == 1);
			int tmpiter = (leftTab) ? iter : splititer;
			int tmpsplititer = (!leftTab) ? iter : splititer;
			Pages[tmpiter]->SetSize(splitline - 3, hh - 2);
			Pages[tmpsplititer]->SetSize(splitline + 2, 1, w - (splitline + 3), hh - 2);
			Refresh(false);
			SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
			splitLineHolding = false;
		}
		else if (event.LeftIsDown() && splitLineHolding)
		{
			if (x != splitline){
				int px = MID(200, x, w - 200), py = 2;
				ClientToScreen(&px, &py);
				if (sline){ sline->SetPosition(wxPoint(px, py)); }
			}

		}
		if (!splitLineHolding && arrow && isInSplitLine){ SetCursor(wxCURSOR_SIZEWE); arrow = false; }
		return;
	}

	if (!arrow && !splitLineHolding){
		SetCursor(wxCURSOR_ARROW); arrow = true;
	}

	int num;

	if (!allTabsVisible && event.GetWheelRotation() != 0) {
		int step = 1 * event.GetWheelRotation() / event.GetWheelDelta();
		tabScrollDestination = MID(0, tabScrollDestination - step, ((int)Size() - 1));
		//RefreshBar();
		//if(!tabsScroll.IsRunning())
			tabsScroll.Start(10);
		return;
	}
	
	int i = FindTab(x, &num);

	//click, double click, middle button
	//click is eqaul also double click
	if (click || middleDown){
		oldI = i;

		if (!allTabsVisible && click && x < 20){
			if (firstVisibleTab > 0){
				firstVisibleTab--;
				tabScrollDestination = firstVisibleTab;
				leftArrowClicked = true;
				RefreshRect(wxRect(0, hh, w, TabHeight), false);
			}
			return;
		}
		else if (!allTabsVisible && click && x > w - 17 && x <= w){
			if (firstVisibleTab < Size() - 1){
				firstVisibleTab++; 
				tabScrollDestination = firstVisibleTab;
				rightArrowClicked = true;
				RefreshRect(wxRect(w - 17, hh, 17, TabHeight), false);
			}
			return;
		}
		else if (click && x > start && x < start + TabHeight - 4){
			AddPage(true, true);
			return;
		}

		wxCommandEvent choiceSelectedEvent(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		if (i == -1){
			if (dclick){
				AddPendingEvent(choiceSelectedEvent);
				AddPage(true, true);
				return;
			}
		}

		else if (i != iter && click){
			ChangePage(i);
		}
		else if (i == iter && click && (x > num + tabSizes[i] - xWidth && x < num + tabSizes[i] - 5)){
			DeletePage(i);
			//after delete tab find a new i
			i = FindTab(x, &num);
			AddPendingEvent(choiceSelectedEvent);
		}
		else if (middleDown)
		{
			int tmpiter = iter;
			DeletePage(i);
			if (i == tmpiter){ AddPendingEvent(choiceSelectedEvent); }
			int tabAfterClose = FindTab(x, &num);
			if (tabAfterClose >= 0)
				SetToolTip(Pages[tabAfterClose]->SubsName + L"\n" + Pages[tabAfterClose]->VideoName);
			else
				UnsetToolTip();
			return;
		}

	}

	//swap tabs
	if (event.LeftIsDown() && i >= 0 && oldI >= 0 && i != oldI && oldI < tabNames.GetCount()){
		wxString tmpname = tabNames[i];
		int tmpsize = tabSizes[i];
		tabNames[i] = tabNames[oldI];
		tabSizes[i] = tabSizes[oldI];
		tabNames[oldI] = tmpname;
		tabSizes[oldI] = tmpsize;
		TabPanel *tmppage = Pages[i];
		Pages[i] = Pages[oldI];
		Pages[oldI] = tmppage;
		iter = i;
		RefreshRect(wxRect(0, hh, w, TabHeight), false);
		if (oldI == splititer){
			splititer = i;
		}
		if (i == splititer){
			splititer = oldI;
		}
		oldI = i;
		tabsWasSwapped = true;
		return;
	}
	if (event.LeftUp() && (rightArrowClicked || leftArrowClicked)){
		rightArrowClicked = leftArrowClicked = false;
		RefreshRect(wxRect(0, hh, w, TabHeight), false); 
		return;
	}

	//Change elements on tabs
	//click to change x when closing tabs
	//can make other bugs
	if (event.Moving() || click){

		if (x > start + TabHeight - 4 && HasToolTips()){ UnsetToolTip(); }


		if (!allTabsVisible && x < 20){
			if (leftArrowHover) return;
			leftArrowHover = true;
			RefreshRect(wxRect(0, hh, 20, TabHeight), false); return;
		}
		else if (!allTabsVisible && x > w - 17 && x <= w){
			if (rightArrowHover) return;
			rightArrowHover = true; 
			newTabHover = false;
			RefreshRect(wxRect(w - 17, hh, 17, TabHeight), false); return;
		}
		else if (x > start && x < start + TabHeight - 4){
			if (newTabHover) return;
			newTabHover = true; 
			rightArrowHover = false;
			RefreshRect(wxRect(start, hh, start + TabHeight - 4, TabHeight), false);
			//if(oldtab!=i){
			SetToolTip(_("Otwórz nową zakładkę"));
			//oldtab=i;}
			return;
		}
		else if (leftArrowHover || rightArrowHover || newTabHover){
			leftArrowHover = rightArrowHover = newTabHover = false;
			RefreshRect(wxRect(w - 19, hh, 19, TabHeight), false); return;
		}

		if (i == -1){
			if (over != -1 || onx){
				over = -1; onx = false;
				RefreshRect(wxRect(0, hh, w, TabHeight), false);
			}
			return;
		}

		if (i != iter && i != over){
			over = i;
			RefreshRect(wxRect(0, hh, w, TabHeight), false);
		}
		else if (i == iter && over != -1){
			over = -1;
			RefreshRect(wxRect(0, hh, w, TabHeight), false);
		}
		if (i == iter && (x > num + tabSizes[i] - xWidth && x < num + tabSizes[i] - 5)){
			if (!onx){ SetToolTip(_("Zamknij")); }
			onx = true;
			RefreshRect(wxRect(num + tabSizes[i] - xWidth, hh, xWidth, TabHeight), false);
		}// it can crash here when i is equal tabSizes.GetCount() cause of no refresh of i
		else if (onx && i < tabSizes.GetCount()){
			//Trick to restore tip with name of subtitles and video after go from x
			oldtab = -1;
			onx = false;
			RefreshRect(wxRect(num + tabSizes[i] - xWidth, hh, xWidth, TabHeight), false);
		}
		if (i != -1 && i != oldtab && i < Pages.size()){ SetToolTip(Pages[i]->SubsName + L"\n" + Pages[i]->VideoName); oldtab = i; }
	}

	//Context menu		
	if (event.RightUp()){
		ContextMenu(event.GetPosition(), i);
	}

}

void Notebook::OnSize(wxSizeEvent& event)
{
	//sizetimer.Start(500,true);
	int w, h;
	GetClientSize(&w, &h);
	h -= TabHeight;
	bool alvistmp = allTabsVisible;
	CalcSizes();
	if (split){
		bool aciter = (Pages[iter]->GetPosition().x == 1);
		int tmpsplititer = (!aciter) ? iter : splititer;
		int tmpiter = (aciter) ? iter : splititer;
		Pages[tmpsplititer]->SetSize(w - (splitline + 3), h - 2);
		Pages[tmpiter]->SetSize((splitline - 3), h - 2);
	}
	else{
		Pages[iter]->SetSize(w, h);
	}
	if (alvistmp != allTabsVisible){ RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false); }
	SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
}

void Notebook::OnPaint(wxPaintEvent& event)
{
	if (block){ return; }
	int w, h;
	GetClientSize(&w, &h);
	//h-=TabHeight;
	wxClientDC cdc(this);
	wxMemoryDC dc;
	wxBitmap notebookBitmap(w, TabHeight);
	dc.SelectObject(notebookBitmap);
	dc.SetFont(font);
	//dc.SetPen(*wxTRANSPARENT_PEN);
	//dc.SetBrush(wxBrush(Options.GetColour("Menu Bar Background 2")));
	dc.GradientFillLinear(wxRect(0, 0, w, TabHeight),
		Options.GetColour(TABSBAR_BACKGROUND2),
		Options.GetColour(TABSBAR_BACKGROUND1), wxTOP);
	const wxColour & activeLines = Options.GetColour(TABS_BORDER_ACTIVE);
	const wxColour & activeText = Options.GetColour(TABS_TEXT_ACTIVE);
	const wxColour & inactiveText = Options.GetColour(TABS_TEXT_INACTIVE);


	start = (allTabsVisible) ? 2 : 20;
	start -= tabOffset;
	int scrollFirstTab = firstVisibleTab;
	if (firstVisibleTab > tabScrollDestination)
	{
		scrollFirstTab = tabScrollDestination;
		int newOffset = 0;
		for (int i = tabScrollDestination; i < firstVisibleTab; i++) {
			newOffset += tabSizes[i];
		}
		start -= newOffset;
	}

	//loop for tabs drawing
	for (size_t i = scrollFirstTab; i < tabSizes.GetCount(); i++){

		int tabSize = tabSizes[i];
		wxString tabName = tabNames[i];
		if (iter < scrollFirstTab){
			dc.SetPen(wxPen(activeLines, 1));
			dc.DrawLine(0, 0, w, 0);
		}
		//choosen tab
		if (i == iter){
			//Drawing lines from both sides of active tab
			dc.SetPen(wxPen(activeLines, 1));
			dc.DrawLine(0, 0, start, 0);
			dc.DrawLine(start + tabSize, 0, w, 0);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(Options.GetColour(TABS_BACKGROUND_ACTIVE));
			dc.DrawRectangle(start + 1, 0, tabSize - 1, TabHeight - 2);

			//X over active tab
			if (onx){
				dc.SetBrush(Options.GetColour(TABS_CLOSE_HOVER));
				dc.DrawRectangle(start + tabSize - xWidth, 4, xWidth - 2, TabHeight - 10);
				dc.SetBrush(Options.GetColour(TABS_BACKGROUND_ACTIVE));
			}
			dc.SetTextForeground(activeText);
			dc.DrawText(L"X", start + tabSize - xWidth + 4, 4);


		}
		else if (split && i == splititer){
			dc.SetTextForeground(inactiveText);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(Options.GetColour((i == (size_t)over && !rightArrowHover && !rightArrowClicked) ? 
				TABS_BACKGROUND_INACTIVE_HOVER : TABS_BACKGROUND_SECOND_WINDOW));
			dc.DrawRectangle(start + 1, 1, tabSize - 1, TabHeight - 2);
		}
		else{
			//nonactive and hover nonactive 
			dc.SetTextForeground(inactiveText);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxBrush(Options.GetColour((i == (size_t)over && !rightArrowHover && !rightArrowClicked) ? 
				TABS_BACKGROUND_INACTIVE_HOVER : TABS_BACKGROUND_INACTIVE)));
			dc.DrawRectangle(start + 1, 1, tabSize - 1, TabHeight - 3);
		}
		//Drawing of contour of tab
		dc.SetPen(wxPen(Options.GetColour((i == iter) ? TABS_BORDER_ACTIVE : TABS_BORDER_INACTIVE)));
		dc.DrawLine(start, 0, start, TabHeight - 4);
		dc.DrawLine(start, TabHeight - 4, start + 2, TabHeight - 2);
		dc.DrawLine(start + 2, TabHeight - 2, start + tabSize - 2, TabHeight - 2);
		dc.DrawLine(start + tabSize - 2, TabHeight - 2, start + tabSize, TabHeight - 4);
		dc.DrawLine(start + tabSize, TabHeight - 4, start + tabSize, 0);
		

		int tabsDiff = (tabName.length() < maxCharPerTab) ? 0 : (i == iter) ? 28 + xWidth : 28;
		dc.SetClippingRegion(start, 0, tabSize - tabsDiff, TabHeight);
		dc.DrawText(tabName, start + 4, 4);
		dc.DestroyClippingRegion();
		if ((tabName.length() >= maxCharPerTab)){
			int pos = start + tabSize - tabsDiff;
			wxColour fadColour = dc.GetTextForeground();
			wxColour fadSecond = dc.GetBrush().GetColour();
			unsigned char alpha = 255;
			for (int k = 0; k < 20; k++){
				fadColour = wxColour(fadColour.Red(), fadColour.Green(), fadColour.Blue(), alpha - (255.f / 20.f) * k);

				dc.SetClippingRegion(pos, 0, 1, TabHeight);
				dc.SetTextForeground(GetColorWithAlpha(fadColour, fadSecond));
				dc.DrawText(tabName, start + 4, 4);
				dc.DestroyClippingRegion();
				pos++;
			}
			//dc.SetTextForeground(inactiveText);
		}

		start += tabSize + 2;
	}

	dc.SetPen(*wxTRANSPARENT_PEN);
	const wxColour & background = Options.GetColour(TABSBAR_ARROW_BACKGROUND);
	dc.SetBrush(wxBrush(background));
	//Arrows for moving tabs
	if (!allTabsVisible){
		const wxColour & backgroundHover = Options.GetColour(TABSBAR_ARROW_BACKGROUND_HOVER);
		//make new color
		const wxColour & backgroundClicked = Options.GetColour(BUTTON_BACKGROUND_PUSHED);
		const wxColour & arrow = Options.GetColour(TABSBAR_ARROW);

		dc.SetPen(wxPen(arrow, 2));
		//left vertical line
		dc.DrawLine(17, 0, 17, TabHeight);
		//right vertical line
		dc.DrawLine(w - 17, 0, w - 17, TabHeight);

		dc.SetPen(*wxTRANSPARENT_PEN);
		//left arrow
		int arrowStart = (TabHeight - 14) / 2;
		wxPoint triangle[3];
		if (firstVisibleTab != 0){
			dc.SetBrush(wxBrush((leftArrowClicked) ? backgroundClicked : (leftArrowHover) ? backgroundHover : background));
			dc.DrawRectangle(0, 0, 16, TabHeight);
			dc.SetBrush(wxBrush(arrow));
			triangle[0] = wxPoint(11, arrowStart);
			triangle[1] = wxPoint(4, arrowStart + 7);
			triangle[2] = wxPoint(11, arrowStart + 14);
			dc.DrawPolygon(3, triangle);
		}
		else{
			dc.SetBrush(wxBrush(background));
			dc.DrawRectangle(0, 0, 16, TabHeight);
		}

		//right arrow
		if (firstVisibleTab < tabSizes.GetCount() - 1){
			dc.SetBrush(wxBrush((rightArrowClicked) ? backgroundClicked : (rightArrowHover) ? backgroundHover : background));
			dc.DrawRectangle(w - 16, 0, 16, TabHeight);
			dc.SetBrush(wxBrush(arrow));
			triangle[0] = wxPoint(w - 11, arrowStart);
			triangle[1] = wxPoint(w - 4, arrowStart + 7);
			triangle[2] = wxPoint(w - 11, arrowStart + 14);
			dc.DrawPolygon(3, triangle);
		}
		else{
			dc.SetBrush(wxBrush(background));
			dc.DrawRectangle(w - 16, 0, 16, TabHeight);
		}
	}

	//Plus is always visible
	dc.SetBrush(wxBrush(Options.GetColour((newTabHover) ? TABS_BACKGROUND_INACTIVE_HOVER : TABS_BACKGROUND_INACTIVE)));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(start + 1, 1, TabHeight - 6, TabHeight - 3);

	//dc.SetPen(wxPen(inactiveText));
	dc.SetBrush(wxBrush(inactiveText));
	dc.DrawRectangle(start + 6, (TabHeight / 2) - 2, TabHeight - 16, 2);
	dc.DrawRectangle(start + (TabHeight / 2) - 3, 7, 2, TabHeight - 16);

	dc.SetPen(wxPen(Options.GetColour(TABS_BORDER_INACTIVE)));
	dc.DrawLine(start, 0, start, TabHeight - 4);
	dc.DrawLine(start, TabHeight - 4, start + 2, TabHeight - 2);
	dc.DrawLine(start + 2, TabHeight - 2, start + TabHeight - 7, TabHeight - 2);
	dc.DrawLine(start + TabHeight - 7, TabHeight - 2, start + TabHeight - 5, TabHeight - 4);
	dc.DrawLine(start + TabHeight - 5, TabHeight - 4, start + TabHeight - 5, 0);


	cdc.Blit(0, h - TabHeight, w, TabHeight, &dc, 0, 0);
	if (split){
		cdc.SetPen(*wxTRANSPARENT_PEN);
		cdc.SetBrush(Options.GetColour(TABSBAR_BACKGROUND1));
		cdc.DrawRectangle(splitline - 2, 0, 4, h - TabHeight);
		cdc.SetPen(wxPen(Options.GetColour(WINDOW_BACKGROUND)));
		bool aciter = (Pages[iter]->GetPosition().x == 1);
		int heightplus = TabHeight + 1;
		if (aciter){
			cdc.DrawLine(splitline + 1, 0, w, 0);
			cdc.DrawLine(w - 1, 0, w - 1, h - heightplus);
			cdc.DrawLine(splitline + 1, h - heightplus, w, h - heightplus);
			cdc.SetPen(wxPen(L"#FF0000"));
			cdc.DrawLine(0, 0, 0, h - heightplus);
			cdc.DrawLine(0, 0, splitline - 1, 0);
			cdc.DrawLine(splitline - 1, 0, splitline - 1, h - heightplus);
			cdc.DrawLine(0, h - heightplus, splitline - 1, h - heightplus);
		}
		else{
			cdc.DrawLine(0, 0, splitline - 1, 0);
			cdc.DrawLine(0, h - heightplus, splitline - 1, h - heightplus);
			cdc.DrawLine(0, 0, 0, h - heightplus);
			cdc.SetPen(wxPen(L"#FF0000"));
			cdc.DrawLine(splitline + 1, 0, w, 0);
			cdc.DrawLine(splitline + 1, 0, splitline + 1, h - heightplus);
			cdc.DrawLine(w - 1, 0, w - 1, h - heightplus);
			cdc.DrawLine(splitline + 1, h - heightplus, w, h - heightplus);
		}
	}
}


void Notebook::ContextMenu(const wxPoint &pos, int i)
{
	Menu tabsMenu;

	for (int g = 0; g < Size(); g++)
	{
		tabsMenu.Append(MENU_CHOOSE + g, Page(g)->SubsName, emptyString, true, 0, 0, (g == iter) ? ITEM_RADIO : ITEM_NORMAL);
	}
	tabsMenu.AppendSeparator();
	tabsMenu.Append(MENU_SAVE + i, _("Zapisz"), _("Zapisz"))->Enable(i >= 0 && Pages[i]->grid->file->CanSave());
	tabsMenu.Append(MENU_SAVE - 1, _("Zapisz wszystko"), _("Zapisz wszystko"));
	tabsMenu.Append(MENU_CHOOSE - 1, _("Zamknij wszystkie zakładki"), _("Zamknij wszystkie zakładki"));
	int num;
	int tabNum = FindTab(pos.x, &num);
	if (tabNum != -1) {
		TabPanel *tab = Pages[tabNum];
		if (!tab->SubsPath.empty()) {
			tabsMenu.Append(MENU_OPEN_SUBS_FOLDER, _("Otwórz folder zawierający napisy"), _("Otwórz folder zawierający napisy"));
		}
		if (!tab->VideoPath.empty()) {
			tabsMenu.Append(MENU_OPEN_VIDEO_FOLDER, _("Otwórz folder zawierający wideo"), _("Otwórz folder zawierający wideo"));
		}
		if (!tab->AudioPath.empty()) {
			tabsMenu.Append(MENU_OPEN_AUDIO_FOLDER, _("Otwórz folder zawierający audio"), _("Otwórz folder zawierający audio"));
		}
		if (!tab->KeyframesPath.empty()) {
			tabsMenu.Append(MENU_OPEN_KEYFRAMES_FOLDER, _("Otwórz folder zawierający klatki kluczowe"), _("Otwórz folder zawierający klatki kluczowe"));
		}
	}
	if ((i != iter && Size() > 1 && i != -1) || split){
		wxString txt = (split) ? _("Wyświetl jedną zakładkę") : _("Wyświetl dwie zakładki");
		tabsMenu.Append((MENU_CHOOSE - 2) - i, txt);
	}
	bool canCompare = (i != iter && Size() > 1 && i != -1);
	Menu *styleComparisonMenu = new Menu();
	if (canCompare){
		wxArrayString availableStyles;
		Pages[iter]->grid->GetCommonStyles(Pages[i]->grid, availableStyles);
		wxArrayString optionsCompareStyles;
		Options.GetTable(SUBS_COMPARISON_STYLES, optionsCompareStyles);
		for (size_t i = 0; i < availableStyles.size(); i++){
			MenuItem * styleItem = styleComparisonMenu->Append(4448, availableStyles[i], emptyString, true, nullptr, nullptr, ITEM_CHECK);
			if (optionsCompareStyles.Index(availableStyles[i]) != -1){ 
				styleItem->Check(); 
				SubsGridBase::compareStyles.Add(availableStyles[i]); 
			}
		}
	}
	int compareBy = Options.GetInt(SUBS_COMPARISON_TYPE);
	Menu *comparisonMenu = new Menu();
	comparisonMenu->Append(MENU_COMPARE + 1, _("Porównaj według czasów"), nullptr, emptyString, ITEM_CHECK, canCompare)->Check(compareBy & COMPARE_BY_TIMES);
	comparisonMenu->Append(MENU_COMPARE + 2, _("Porównaj według widocznych linijek"), nullptr, emptyString, ITEM_CHECK, canCompare)->Check((compareBy & COMPARE_BY_VISIBLE)>0);
	comparisonMenu->Append(MENU_COMPARE + 3, _("Porównaj według zaznaczeń"), nullptr, emptyString, ITEM_CHECK, canCompare && Pages[iter]->grid->file->SelectionsSize() > 0 && Pages[i]->grid->file->SelectionsSize() > 0)->Check((compareBy & COMPARE_BY_SELECTIONS) > 0);
	comparisonMenu->Append(MENU_COMPARE + 4, _("Porównaj według stylów"), nullptr, emptyString, ITEM_CHECK, canCompare)->Check((compareBy & COMPARE_BY_STYLES) > 0);
	comparisonMenu->Append(MENU_COMPARE + 5, _("Porównaj według wybranych stylów"), styleComparisonMenu, emptyString, ITEM_CHECK, canCompare)->Check(SubsGridBase::compareStyles.size() > 0);
	comparisonMenu->Append(MENU_COMPARE, _("Porównaj"))->Enable(canCompare);
	comparisonMenu->Append(MENU_COMPARE - 1, _("Wyłącz porównanie"))->Enable(SubsGridBase::hasCompare);
	tabsMenu.Append(MENU_COMPARE + 6, _("Porównanie napisów"), comparisonMenu, _("Porównanie napisów"))->Enable(canCompare || SubsGridBase::hasCompare);

	int id = tabsMenu.GetPopupMenuSelection(pos, this);

	if (id < 0){ return; }
	if (id >= MENU_CHOOSE - 101 && id <= MENU_CHOOSE + 99){
		OnTabSel(id);
	}
	else if (id == MENU_COMPARE){
		SubsGridBase::CG1 = Pages[iter]->grid;
		SubsGridBase::CG2 = Pages[i]->grid;
		SubsGridBase::SubsComparison();
		SubsGridBase::hasCompare = true;
		Pages[iter]->grid->ShowSecondComparedLine(
			SubsGridBase::CG2->GetScrollPosition(), false, false, true);
	}
	else if (id == MENU_COMPARE - 1){
		SubsGridBase::RemoveComparison();
	}
	else if (id >= MENU_OPEN_SUBS_FOLDER && id <= MENU_OPEN_KEYFRAMES_FOLDER && tabNum != -1) {
		TabPanel *tab = Pages[tabNum];
		const wxString &path = id == MENU_OPEN_SUBS_FOLDER ? tab->SubsPath :
			id == MENU_OPEN_VIDEO_FOLDER ? tab->VideoPath :
			id == MENU_OPEN_AUDIO_FOLDER ? tab->AudioPath : tab->KeyframesPath;

		SelectInFolder(path);
	}
	else if (id == MENU_SAVE - 1 || (i >= 0 && id == MENU_SAVE + i)){
		OnSave(id);
	}
}

void Notebook::OnTabSel(int id)
{
	int wtab = id - MENU_CHOOSE;
	if (Pages[iter]->video->GetState() == Playing){ Pages[iter]->video->Pause(); }
	if (wtab < -1){
		wtab = abs(wtab + 2);
		Split(wtab);
	}
	else if (wtab < 0){
		if (KaiMessageBox(_("Zostaną zamknięte wszystkie zakładki, kontynuować?"), _("Pytanie"), 
			wxYES_NO, Kai, wxDefaultPosition, wxNO) == wxNO)
			return;

		int tmpiter = iter;
		Pages[iter]->Hide();
		for (int i = (int)Pages.size() - 1; i >= 0; i--)
		{
			iter = i;
			if (Kai->SavePrompt()){ break; }
			SubsGridBase::RemoveComparison();
			Pages[i]->Destroy();
			Pages.pop_back();
			tabNames.pop_back();
			tabSizes.pop_back();
		}
		int pagesSize = Pages.size();
		if (iter <= pagesSize || splititer <= pagesSize)
			split = false;

		if (olditer >= pagesSize){ olditer = 0; }
		firstVisibleTab = tabScrollDestination = 0;
		int w = -1, h = -1;
		if (pagesSize < 1){
			Pages.push_back(new TabPanel(this, Kai));
			wxString name = Pages[0]->SubsName;
			tabNames.Add(name);
			GetClientSize(&w, &h);
			Pages[0]->SetPosition(wxPoint(0, 0));
			Pages[0]->SetSize(w, h - TabHeight);
		}
		CalcSizes();
		if (w < 1){ GetClientSize(&w, &h); }
		RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
		Pages[iter]->Show();
		wxCommandEvent choiceSelectedEvent(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		AddPendingEvent(choiceSelectedEvent);
	}
	else{
		TabPanel *tmp = Page(firstVisibleTab);
		tmp->Hide();
		Pages[firstVisibleTab] = Pages[wtab];
		Pages[firstVisibleTab]->Show();
		Pages[wtab] = tmp;
		wxString tmp1 = tabNames[firstVisibleTab];
		tabNames[firstVisibleTab] = tabNames[wtab];
		tabNames[wtab] = tmp1;
		tabSizes[iter] -= xWidth;
		int tmp2 = tabSizes[firstVisibleTab];
		tabSizes[firstVisibleTab] = tabSizes[wtab];
		tabSizes[wtab] = tmp2;
		tabSizes[firstVisibleTab] += xWidth;

		olditer = iter;
		iter = firstVisibleTab;
		RefreshBar();
	}
}


int Notebook::GetHeight()
{
	return TabHeight;
}

void Notebook::OnResized()
{
	KillTimer(sthis->GetHWND(), 9876);
	int w, h;
	sthis->GetClientSize(&w, &h);
	if (sthis->split){ w = sthis->splitline - 2; }
	for (int i = 0; i < sthis->Size(); i++){
		if (i == sthis->iter || (sthis->split && i == sthis->splititer))continue;
		sthis->Pages[i]->SetSize(w, h - sthis->TabHeight);
	}
}

void Notebook::Split(size_t page)
{
	split = !split;
	int w, h;
	GetClientSize(&w, &h);
	if (!split)
	{
		Pages[splititer]->Hide();
		Pages[iter]->SetSize(w, h - TabHeight);
		for (int k = 0; k < Size(); k++){
			Pages[k]->SetPosition(wxPoint(0, 0));
		}
		SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
		return;
	}
	splitline = w / 2;
	splititer = page;
	Pages[iter]->SetSize(1, 1, splitline - 3, h - TabHeight - 2);
	Pages[splititer]->SetSize(splitline + 2, 1, w - (splitline + 3), h - TabHeight - 2);
	Pages[splititer]->Show();
	SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
}

int Notebook::FindTab(int x, int *_num)
{
	int num = (allTabsVisible) ? 2 : 20;
	num -= tabOffset;
	*_num = num;
	int restab = -1;
	for (size_t i = firstVisibleTab; i < tabSizes.size(); i++){
		if (x > num && x < num + tabSizes[i]){
			restab = i;
			*_num = num;
			break;
		}
		num += tabSizes[i] + 2;
	}
	return restab;
}
void Notebook::ChangeActive()
{
	wxWindow *win = FindFocus();
	if (win && Pages[iter]->IsDescendant(win)){
		Pages[iter]->lastFocusedWindowId = win->GetId();
	}
	else{ Pages[iter]->lastFocusedWindowId = 0; }
	int tmp = iter;
	iter = splititer;
	splititer = tmp;
	tabSizes[iter] += xWidth;
	tabSizes[splititer] -= xWidth;
	wxCommandEvent choiceSelectedEvent(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
	choiceSelectedEvent.SetInt(1);
	AddPendingEvent(choiceSelectedEvent);
	RefreshBar();
}

void Notebook::RefreshBar(bool checkSizes)
{
	if (checkSizes)
		CalcSizes(true);

	int w, h;
	GetClientSize(&w, &h);
	RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
}

bool Notebook::LoadSubtitles(TabPanel *tab, const wxString & path, int active /*= -1*/, int scroll /*= -1*/)
{
	wxString ext = path.AfterLast(L'.').Lower();
	OpenWrite ow;
	wxString s;
	if (!ow.FileOpen(path, &s)){
		return false;
	}
	else{
		tab->grid->LoadSubtitles(s, ext);
	}

	tab->SubsPath = path;
	if (ext == L"ssa"){ ext = L"ass"; tab->SubsPath = tab->SubsPath.BeforeLast(L'.') + L".ass"; }
	tab->SubsName = tab->SubsPath.AfterLast(L'\\');
	tab->video->DisableVisuals(ext != L"ass");
	if (active != -1 && active != tab->grid->currentLine && active < tab->grid->GetCount()){
		tab->grid->SetActive(active);
	}
	if (scroll != -1)
		tab->grid->ScrollTo(scroll);
	loadedRecoverySubs = false;
	return true;
}

int Notebook::LoadVideo(TabPanel *tab, const wxString & path, 
	int position /*= -1*/, bool isFFMS2, bool hasEditor, bool fullscreen, bool loadPrompt, bool dontLoadAudio)
{
	wxString videopath;
	wxString audiopath;
	wxString keyframespath;
	bool hasVideoPath = false;
	bool hasAudioPath = false;
	bool hasKeyframePath = false;
	bool found = !path.empty();

	if (hasEditor) {
		wxString subsPath = (path.empty()) ?
			tab->SubsPath.BeforeLast(L'\\') + L"\\" : path.BeforeLast(L'\\') + L"\\";
		audiopath = tab->grid->GetSInfo(L"Audio File");
		keyframespath = tab->grid->GetSInfo(L"Keyframes File");

		if (loadPrompt) {
			videopath = tab->grid->GetSInfo(L"Video File");
			hasVideoPath = (!videopath.empty() && ((wxFileExists(videopath) && videopath.find(L':') == 1) ||
				videopath.StartsWith(L"?dummy") || wxFileExists(videopath.Prepend(subsPath))));

			//fix for wxFileExists which working without full path when program run from command line
			hasAudioPath = (!audiopath.empty() && ((wxFileExists(audiopath) && audiopath.find(L':') == 1) ||
				audiopath.StartsWith(L"dummy") || wxFileExists(audiopath.Prepend(subsPath))));
			hasKeyframePath = (!keyframespath.empty() && ((wxFileExists(keyframespath) && keyframespath.find(L':') == 1) ||
				wxFileExists(keyframespath.Prepend(subsPath))));
		}
		
		

		const wxString &videoPath = loadPrompt ? videopath : path;
		bool sameAudioPath = audiopath == videoPath;

		if (loadPrompt) {
			int result = promptResult;
			if (!promptResult) {
				int flags = wxNO | ASK_ONCE;
				wxString prompt;
				if (hasVideoPath || hasAudioPath || hasKeyframePath) {
					if (hasVideoPath && tab->VideoPath != videopath) { prompt += _("Wideo: ") + videopath + L"\n"; }
					else if (hasVideoPath) { hasVideoPath = false; }
					if (hasAudioPath && tab->AudioPath != audiopath) { prompt += _("Audio: ") + audiopath + L"\n"; }
					else if (hasAudioPath) { hasAudioPath = false; }
					if (hasKeyframePath && tab->KeyframesPath != keyframespath) { prompt += _("Klatki kluczowe: ") + keyframespath + L"\n"; }
					else if (hasKeyframePath) { hasKeyframePath = false; }
					if (!prompt.empty()) { prompt.Prepend(_("Skojarzone pliki:\n")); flags |= wxOK; }
				}
				if (!path.empty()) {
					if (tab->VideoPath != path) {
						if (!prompt.empty()) { prompt += L"\n"; }
						prompt += _("Wideo z folderu:\n") + path.AfterLast(L'\\'); flags |= wxYES;
					}
					else
						return -1;
				}
				if (!prompt.empty()) {
					KaiMessageDialog dlg(this, prompt, _("Potwierdzenie"), flags);
					if (flags & wxYES && flags & wxOK) {
						dlg.SetOkLabel(_("Wczytaj skojarzone"));
						dlg.SetYesLabel(_("Wczytaj z folderu"));
					}
					else if (flags & wxOK) { dlg.SetOkLabel(_("Tak")); }
					result = dlg.ShowModal();
					if (result & ASK_ONCE)
						promptResult = result;
				}
				else
					result = wxNO;
			}
			if (result & wxNO) {
				return -1;
			}
			else if (result & wxOK) {
				if (!audiopath.empty()) {
					if (hasAudioPath && !sameAudioPath) {
						audiopath.Replace(L"/", L"\\");
					}
					if (hasVideoPath) {
						MenuItem *item = Kai->Menubar->FindItem(GLOBAL_VIDEO_INDEXING);
						if (item) item->Check();
						toolitem *titem = Kai->Toolbar->FindItem(GLOBAL_VIDEO_INDEXING);
						if (titem) {
							titem->toggled = true;
							Kai->Toolbar->Refresh(false);
						}
						toolitem *etitem = Kai->Toolbar->FindItem(GLOBAL_EDITOR);
						if (etitem) {
							etitem->Enable(false);
							Kai->Toolbar->Refresh(false);
						}
					}
				}
				if (hasVideoPath) {
					videopath.Replace(L"/", L"\\");
					found = true;
				}
			}
			else
				hasVideoPath = hasAudioPath = hasKeyframePath = false;

		}
		

		if (sameAudioPath)
			hasAudioPath = false;
	}
	if (found) {
		if (!tab->video->LoadVideo((hasVideoPath) ? 
			videopath : path, (tab->editor) ? OPEN_DUMMY : 0,
			fullscreen, !hasAudioPath && !dontLoadAudio, 
			(position != -1) ? isFFMS2 : -1, (position != -1))) {
			return 0;
		}
	}
	if (hasEditor) {
		if (hasAudioPath) {
			audiopath.Replace(L"/", L"\\");
			Kai->OpenAudioInTab(tab, 30040, audiopath);
		}

		if (hasKeyframePath) {
			tab->video->OpenKeyframes(keyframespath);
			tab->KeyframesPath = keyframespath;
		}

	}

	tab->edit->Frames->Enable(!tab->video->IsDirectShow());
	tab->edit->Times->Enable(!tab->video->IsDirectShow());
	
	if (position != -1)
		tab->video->Seek(position);

	return 1;
}

void Notebook::OnSave(int id)
{
	id -= MENU_SAVE;
	KainoteFrame *Kai = (KainoteFrame*)GetParent();
	if (id < 0){
		Kai->SaveAll();
	}
	else{
		Kai->Save(false, id);
	}
}

Notebook *Notebook::sthis = nullptr;

Notebook *Notebook::GetTabs()
{
	return sthis;
}
//checking and return null when all tabs removed or iter is >= then size
TabPanel *Notebook::GetTab()
{
	if(sthis->iter < sthis->Size())
		return sthis->Pages[sthis->iter];
	else {
		KaiLogDebug(wxString::Format(L"bad iter %i / %i crash", sthis->iter, (int)sthis->Size()));
		//Sleep(5000);
		return nullptr;
	}
}

void Notebook::SaveLastSession(bool beforeClose, bool recovery, const wxString &externalPath)
{
	wxString result = L"[" + Options.progname + L"]\r\n";
	if (beforeClose)
		result << L"[Close session]\r\n";
	int numtab = 0;
	wxString recoveryPath = Options.pathfull + L"\\Recovery\\";
	
	for (std::vector<TabPanel*>::iterator it = sthis->Pages.begin(); it != sthis->Pages.end(); it++){
		TabPanel *tab = *it;
		//put path to recovery on crash
		result << L"Tab: " << numtab << L"\r\nVideo: " << tab->VideoPath <<
			L"\r\nPosition: " << tab->video->Tell() << L"\r\nFFMS2: " << !tab->video->IsDirectShow() <<
			L"\r\nSubtitles: ";
		if (recovery){
			wxString ext = (tab->grid->subsFormat < SRT) ? L"ass" : (tab->grid->subsFormat == SRT) ? L"srt" : L"txt";
			wxString fullPath = recoveryPath + tab->SubsName.BeforeLast(L'.') + L"." + ext;
			tab->grid->SaveFile(fullPath);
			//recovery path for crash close
			result << fullPath;
		}
		else{
			//normal path for end close
			result << tab->SubsPath;
		}
		result << L"\r\nActive: " << tab->grid->currentLine <<
			L"\r\nScroll: " << tab->grid->GetScrollPosition() <<
			L"\r\nEditor: " << tab->editor << L"\r\n";
		if (tab->AudioPath != emptyString && tab->AudioPath != tab->VideoPath) {
			result << L"Audio: " << tab->AudioPath << L"\r\n";
		}
		if (tab->KeyframesPath != emptyString) {
			result << L"Keyframes: " << tab->KeyframesPath << L"\r\n";
		}

		numtab++;
	}
	OpenWrite op;
	wxString sessionPath = externalPath.empty() ? Options.configPath + L"\\LastSession.txt" : externalPath;
	op.FileWrite(sessionPath, result);

}

void Notebook::LoadLastSession(bool loadCrashSession, const wxString &externalPath)
{
	wxString riddenSession;
	wxString sessionPath = externalPath.empty() ? Options.configPath + L"\\LastSession.txt" : externalPath;
	OpenWrite op;
	if (op.FileOpen(sessionPath, &riddenSession, false) && !riddenSession.empty()){
		wxStringTokenizer tokenizer(riddenSession, L"\n", wxTOKEN_STRTOK);
		wxString header = tokenizer.GetNextToken();
		if (!header.StartsWith(L"[Kainote")){
			KaiLog(_("Nieprawidłowy plik sesji"));
			return;
		}

		

		//wxString GoodSession = tokenizer.GetNextToken();
		//if (!GoodSession.StartsWith(L"[")){
			//We can make some steps here to load subtitles from subs
		//}
		//else{

		//}
		sthis->Freeze();
		for (std::vector<TabPanel*>::iterator i = sthis->Pages.begin(); i != sthis->Pages.end(); i++)
		{
			(*i)->Destroy();
		}
		sthis->Pages.clear();
		sthis->tabSizes.Clear();
		sthis->tabNames.Clear();
		sthis->iter = 0;
		wxString video;
		int videoPosition = 0;
		wxString subtitles;
		wxString keyframes;
		wxString audio;
		int activeLine = 0;
		int scrollPosition = 0;
		bool isFFMS2 = true;
		bool hasEditor = true;
		wxString rest;
		while (true)
		{
			wxString token = tokenizer.GetNextToken();
			if (token.StartsWith(L"Video: ", &rest))
				video = rest;
			else if (token.StartsWith(L"Position: ", &rest))
				videoPosition = wxAtoi(rest);
			else if (token.StartsWith(L"Subtitles: ", &rest))
				subtitles = rest;
			else if (token.StartsWith(L"Active: ", &rest))
				activeLine = wxAtoi(rest);
			else if (token.StartsWith(L"Scroll: ", &rest))
				scrollPosition = wxAtoi(rest);
			else if (token.StartsWith(L"FFMS2: ", &rest))
				isFFMS2 = !!wxAtoi(rest);
			else if (token.StartsWith(L"Editor: ", &rest))
				hasEditor = !!wxAtoi(rest);
			else if (token.StartsWith(L"Audio: ", &rest))
				audio = rest;
			else if (token.StartsWith(L"Keyframes: ", &rest))
				keyframes = rest;
			// no else cause hasMoreTokens have to be checked everytime
			bool hasnotMoreTokens = !tokenizer.HasMoreTokens();
			if (token.StartsWith(L"Tab: ", &rest) || hasnotMoreTokens){
				if (rest != L"0" || hasnotMoreTokens){
					sthis->AddPage(false);
					int lastTab = sthis->Size() - 1;
					TabPanel *tab = sthis->Pages[lastTab];
					
					if (!subtitles.empty()){
						//when load crash session better use active line from subtitles
						if (loadCrashSession) {
							FindAutoSaveSubstitute(&subtitles, lastTab);
							sthis->LoadSubtitles(tab, subtitles);
						}else
							sthis->LoadSubtitles(tab, subtitles, activeLine, scrollPosition);

						sthis->Kai->SetRecent();
					}
					if (!keyframes.empty()) {
						tab->video->OpenKeyframes(keyframes);
						tab->KeyframesPath = keyframes;
					}
					if (!video.empty()){
						if (!hasEditor)
							sthis->Kai->HideEditor();
	
						sthis->LoadVideo(tab, video, videoPosition, isFFMS2, hasEditor, false, false, audio != emptyString);
					}
					if (!audio.empty()) {
						sthis->Kai->OpenAudioInTab(tab, 30040, audio);
					}
					
					sthis->Kai->Label();
					tab->shiftTimes->Contents();
					video = emptyString;
					videoPosition = 0;
					subtitles = emptyString;
					activeLine = 0;
					scrollPosition = 0;
					isFFMS2 = true;
					hasEditor = true;
					keyframes = emptyString;
				}
			}
			if (hasnotMoreTokens)
				break;
		}
		if (sthis->Pages.size() < 1)
			sthis->AddPage(true);
	
		sthis->Thaw();
		TabPanel *tab = sthis->GetTab();
		tab->Show();
		tab->video->DeleteAudioCache();
		sthis->Kai->SetSubsResolution(false);
		sthis->Kai->UpdateToolbar();
		Options.SaveOptions(true, false);
		SaveLastSession();
	}
}

void Notebook::FindAutoSaveSubstitute(wxString* path, int tab)
{
	wxString seekpath = path->AfterLast(L'\\');
	wxString seekPathWithoutExt = seekpath. BeforeLast(L'.');
	if (seekPathWithoutExt.empty())
		seekPathWithoutExt = seekpath;

	wxString autosavePath = Options.pathfull + L"\\Subs\\" + 
		seekPathWithoutExt + L"_" + std::to_wstring(tab) + L"*";

	WIN32_FIND_DATAW data;
	HANDLE h = FindFirstFileW(autosavePath.wc_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
	{
		return;
	}
	SYSTEMTIME highiestTime = {0,0,0,0,0,0,0,0};
	wxString latestFile;
	while (1) {
		if (data.nFileSizeLow == 0) { continue; }

		FILETIME accessTime = data.ftLastWriteTime;
		SYSTEMTIME accessSystemTime;
		FileTimeToSystemTime(&accessTime, &accessSystemTime);
		if (CheckDate(&highiestTime, &accessSystemTime) ){
			highiestTime = accessSystemTime;
			latestFile = wxString(data.cFileName);
		}
		int result = FindNextFile(h, &data);
		if (result == ERROR_NO_MORE_FILES || result == 0) { break; }
	}
	if (!latestFile.empty()) {
		FILETIME ft;
		HANDLE ffile = CreateFile(path->wc_str(), 
			GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		GetFileTime(ffile, 0, 0, &ft);
		CloseHandle(ffile);
		SYSTEMTIME accessSystemTime;
		FileTimeToSystemTime(&ft, &accessSystemTime);
		if (CheckDate(&accessSystemTime, &highiestTime)) {
			*path = Options.pathfull + L"\\Subs\\" + latestFile;
			sthis->loadedRecoverySubs = true;
		}
		else {
			sthis->loadedRecoverySubs = false;
		}
	}
	FindClose(h);
}
//first < second
bool Notebook::CheckDate(SYSTEMTIME* firstDate, SYSTEMTIME* secondDate)
{
	if (firstDate->wYear < secondDate->wYear) 
		return true;
	else if(firstDate->wYear > secondDate->wYear)
		return false;

	if (firstDate->wMonth < secondDate->wMonth)
		return true;
	else if(firstDate->wMonth > secondDate->wMonth)
		return false;

	if (firstDate->wDay < secondDate->wDay)
		return true;
	else if (firstDate->wDay > secondDate->wDay)
		return false;

	if (firstDate->wHour < secondDate->wHour)
		return true;
	else if (firstDate->wHour > secondDate->wHour)
		return false;

	if (firstDate->wMinute < secondDate->wMinute)
		return true;
	else if (firstDate->wMinute < secondDate->wMinute)
		return false;

	if (firstDate->wSecond < secondDate->wSecond) 
		return true;
	
	return false;
}

int Notebook::CheckLastSession()
{
	wxString riddenSession;
	OpenWrite op;
	if (op.FileOpen(Options.configPath + L"\\LastSession.txt", &riddenSession, false)){
		size_t CloseSession = riddenSession.find(L"]\n[Close session]\n");
		if (CloseSession != -1)
			return 1;
		else
			return 2;
	}
	return 0;
}

int Notebook::FindPanel(TabPanel* tab, bool safe /*= true*/)
{
	int i = 0;
	for (std::vector<TabPanel*>::iterator it = Pages.begin(); it != Pages.end(); it++)
	{
		if ((*it) == tab){ return i; }
		i++;
	}
	return safe? iter : -1;
}


LRESULT __stdcall Notebook::PauseOnMinimalize(int code, WPARAM wParam, LPARAM lParam)
{

	if (code == HCBT_MINMAX){
		if (lParam == 7 && sthis->GetTab()->video->GetState() == Playing){
			sthis->GetTab()->video->Pause();
		}
	}
	//if (wParam == SC_RESTORE){
		//sthis->GetTab()->grid->SetFocus();//Refresh(false);
	//}
	return CallNextHookEx(0, code, wParam, lParam);
}



void Notebook::ChangePage(int page, bool makeActiveVisible /*= false*/)
{
	if (page == iter || page < 0 || page >= Pages.size())
		return;

	olditer = iter;
	if (split && splititer == page){
		ChangeActive(); return;
	}
	if (Pages[olditer]->video->GetState() == Playing){ Pages[olditer]->video->Pause(); }
	if (split){
		Pages[page]->SetPosition(Pages[iter]->GetPosition());
		Pages[page]->SetSize(Pages[iter]->GetSize());
	}
	Freeze();
	Pages[iter]->Hide();
	tabSizes[iter] -= xWidth;
	Pages[page]->Show();
	tabSizes[page] += xWidth;
	Thaw();
	iter = page;
	over = -1;
	if (makeActiveVisible)
		CalcSizes(true);

	RefreshBar();
	wxCommandEvent choiceSelectedEvent(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
	AddPendingEvent(choiceSelectedEvent);
}

void Notebook::OnEraseBackground(wxEraseEvent &event)
{
}

void Notebook::OnCharHook(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	int ukey = event.GetUnicodeKey();
	//bool nmodif = !(event.AltDown() || event.ControlDown() || event.ShiftDown());
	VideoBox *vb = GetTab()->video;
	if (ukey == 179){ vb->Pause(); }
	//else if(ukey==178){wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,11015); vb->OnVButton(evt);}
	else if (ukey == 177){ vb->PrevChap(); }
	else if (ukey == 176){ vb->NextChap(); }
	//else if(ukey==175){vb->OnSPlus();return;}
	//else if(ukey==174){vb->OnSMinus();return;}
	//else if (key == WXK_PAGEDOWN && nmodif && vb->HasFocus()){ vb->OnPrew(); return; }
	//else if (key == WXK_PAGEUP && nmodif && vb->HasFocus()){ vb->OnNext(); return; }
	event.Skip();
}

void Notebook::OnScrollTabs(wxTimerEvent & event)
{
	if (firstVisibleTab == tabScrollDestination) {
		tabOffset = 0;
		return;
	}
	if (firstVisibleTab < tabScrollDestination) {
		int firstVisibleTabSize = tabSizes[firstVisibleTab];
		tabOffset += firstVisibleTabSize / 8;
		if (tabOffset > firstVisibleTabSize) {
			firstVisibleTab++;
			//sanity check
			if (firstVisibleTab >= tabSizes.GetCount() - 1 || firstVisibleTab == tabScrollDestination) {
				tabOffset = 0;
				tabsScroll.Stop();
				RefreshBar();
				return;
			}
			tabOffset -= firstVisibleTabSize;
		}
	}
	else if(firstVisibleTab > 0){
		int firstVisibleTabSize = tabSizes[firstVisibleTab - 1];
		tabOffset -= firstVisibleTabSize / 8;
		if (tabOffset < (-firstVisibleTabSize)) {
			firstVisibleTab--;
			//sanity check
			if (firstVisibleTab < 0 || firstVisibleTab == tabScrollDestination) {
				tabOffset = 0;
				tabsScroll.Stop();
				RefreshBar();
				return;
			}
			tabOffset += firstVisibleTabSize;
		}
	}
	int num;
	over = FindTab(x, &num);
	RefreshBar();
	//Update();
}

TabPanel *Notebook::GetSecondPage()
{
	return Pages[splititer];
}

int Notebook::GetIterByPos(const wxPoint &pos){
	bool nonActiveFrame = (Pages[iter]->GetPosition().x == 1) ? pos.x > splitline : pos.x <= splitline;
	if (split && nonActiveFrame)
		return splititer;
	else
		return iter;
}

void Notebook::RefreshVideo(bool reloadLibass /*= false*/)
{
	//libass need to reload library and renderer
	bool noReopenSubs = false;
	//when libass is reloaded there is no need to reopen subs 
	//cause it failed and it will reopen when libass load
	if(reloadLibass)
		noReopenSubs = SubtitlesProviderManager::ReloadLibraries();
	
	if (!noReopenSubs) {
		for (int i = 0; i < sthis->Size(); i++) {
			TabPanel *tab = sthis->Page(i);
			if (tab->video->GetState() != None) {
				tab->video->OpenSubs(OPEN_DUMMY, true, true, true);
			}
		}
	}
}

bool Notebook::SetFont(const wxFont &_font)
{
	for (auto &page : Pages){
		page->SetFont(_font);
	}

	//wxWindow::SetFont(_font);
	font = _font;
	int fx, fy;
	GetTextExtent(L"X", &fx, &fy, 0, 0, &font);
	TabHeight = fy + 12;
	xWidth = fx + 10;
	CalcSizes(true);
	Refresh(false);
	return true;
}


BEGIN_EVENT_TABLE(Notebook, wxWindow)
EVT_CHAR_HOOK(Notebook::OnCharHook)
EVT_ERASE_BACKGROUND(Notebook::OnEraseBackground)
EVT_MOUSE_EVENTS(Notebook::OnMouseEvent)
EVT_SIZE(Notebook::OnSize)
EVT_PAINT(Notebook::OnPaint)
EVT_MOUSE_CAPTURE_LOST(Notebook::OnLostCapture)
END_EVENT_TABLE()

