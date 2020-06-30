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

//#include <wx/graphics.h>
#include "Tabs.h"
#include "TabPanel.h"
#include "kainoteApp.h"
#include "Menu.h"
#include "KaiMessageBox.h"
#include "OpennWrite.h"
#include "Utils.h"

Notebook::Notebook(wxWindow *parent, int id)
	: wxWindow(parent, id)
{
	firstVisibleTab = olditer = iter = 0;
	splitline = splititer = 0;
	oldtab = oldI = over = -1;
	block = split = onx = leftArrowHover = rightArrowHover = newTabHover = false;
	allTabsVisible = arrow = true;
	sline = NULL;
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
	Names.Add(name);

	CalcSizes();
	Hook = NULL;
	Hook = SetWindowsHookEx(WH_CBT, &PauseOnMinimalize, NULL, GetCurrentThreadId());//WH_MOUSE
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
				Menu *parentMenu = NULL;
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
	Tabsizes.Clear();
	Names.Clear();
	UnhookWindowsHookEx(Hook);
}

TabPanel *Notebook::GetPage()
{
	return Pages[iter];
}

void Notebook::AddPage(bool refresh)
{
	if (iter < Pages.size() && Pages[iter]->Video->GetState() == Playing){ Pages[iter]->Video->Pause(); }
	int w, h;
	GetClientSize(&w, &h);
	if(refresh){Freeze();}
	Pages.push_back(new TabPanel(this, (KainoteFrame*)GetParent(), wxPoint(0, 0), wxSize(0, 0)));
	olditer = iter;
	iter = Size() - 1;
	if (refresh){
		Pages[olditer]->Hide();
	}
	else{
		Pages[iter]->Hide();
	}
	wxString name = Pages[iter]->SubsName;
	Names.Add(name);

	Pages[iter]->SetPosition(Pages[olditer]->GetPosition());
	Pages[iter]->SetSize(olditer != iter ? Pages[olditer]->GetSize() : wxSize(w, h - TabHeight));
	CalcSizes(true);
	if (refresh){
		if (!Options.GetBool(EDITOR_ON)){ 
			KainoteFrame *kai = (KainoteFrame *)GetParent(); 
			kai->HideEditor(false); 
		}
		wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		AddPendingEvent(evt2);
		Thaw();
		RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
	}
	else{ Pages[iter]->ShiftTimes->RefVals(Pages[olditer]->ShiftTimes); }
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
	Names[page] = label;
	CalcSizes(true);
	int w, h;
	GetClientSize(&w, &h);
	RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
}

TabPanel *Notebook::Page(size_t i)
{
	return Pages[i];
}

void Notebook::DeletePage(int page)
{
	Freeze();
	KainoteFrame *Kai = (KainoteFrame*)GetParent();
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
	Names.RemoveAt(page);
	Tabsizes.RemoveAt(page);

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
		Names.Add(name);
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
	if (firstVisibleTab > rsize){ firstVisibleTab = rsize; }

	CalcSizes(true);
	Thaw();

	//int w,h;
	//GetClientSize(&w,&h);
	//RefreshRect(wxRect(0,h-TabHeight,w,TabHeight),false);
	Refresh(false);
}

void Notebook::CalcSizes(bool makeActiveVisible)
{
	int all = 2;
	int w, h;
	GetClientSize(&w, &h);
	int newHeight = 0;
	for (size_t i = 0; i < Size(); i++){
		int fw, fh;
		wxString name = Names[i];
		if (name.length() > maxCharPerTab){
			name = name.Mid(0, maxCharPerTab);
		}

		GetTextExtent(name, &fw, &fh, NULL, NULL, &font);
		if (i == iter){ fw += xWidth; }
		if (i < Tabsizes.size()){ Tabsizes[i] = fw + 10; }
		else{ Tabsizes.Add(fw + 10); }
		all += Tabsizes[i] + 2;
		if (fh + 12 > newHeight){
			newHeight = fh + 12;
		}
	}
	allTabsVisible = (all < w - 22);
	if (allTabsVisible){ firstVisibleTab = 0; }
	if (makeActiveVisible && !allTabsVisible){
		int tabsWidth = 0;
		if (firstVisibleTab > iter)
			firstVisibleTab = 0;

		for (size_t i = firstVisibleTab; i < Tabsizes.size(); i++){

			tabsWidth += Tabsizes[i];

			if (tabsWidth > w - 22){
				firstVisibleTab = i - 1;
				break;
			}
			if (i == iter)
				break;
			
		}
	}
	if (newHeight != TabHeight){
		TabHeight = newHeight;
		//OnSize have all that needed after changing font size on tabs bar
		OnSize(wxSizeEvent());
	}
}


void Notebook::OnMouseEvent(wxMouseEvent& event)
{
	int x = event.GetX(), y = event.GetY();
	bool click = event.LeftDown();
	bool dclick = event.LeftDClick();
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

	if (y < hh && split){
		bool isInSplitLine = abs(splitline - x) < 4;
		if (click && isInSplitLine){
			CaptureMouse();
			int px = x, py = 2;
			ClientToScreen(&px, &py);
			sline = new wxDialog(this, -1, L"", wxPoint(px, py), wxSize(3, h - 27), wxSTAY_ON_TOP | wxBORDER_NONE);
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
				sline = NULL;
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

	if (!allTabsVisible && event.GetWheelRotation() != 0) {
		int step = 1 * event.GetWheelRotation() / event.GetWheelDelta();
		firstVisibleTab = MID(0, firstVisibleTab - step, ((int)Size() - 1));
		over = -1;
		RefreshBar();
		return;
	}

	int num;
	int i = FindTab(x, &num);

	//click, double click, middle button
	if (click || dclick || middleDown){
		oldI = i;

		if (!allTabsVisible && (click || dclick) && x < 20){
			if (firstVisibleTab > 0){
				firstVisibleTab--;
				leftArrowClicked = true;
				RefreshRect(wxRect(0, hh, w, TabHeight), false);
			}
			return;
		}
		else if (!allTabsVisible && (click || dclick) && x > w - 17 && x <= w){
			if (firstVisibleTab < Size() - 1){
				firstVisibleTab++; 
				rightArrowClicked = true;
				RefreshRect(wxRect(w - 17, hh, 17, TabHeight), false);
			}
			return;
		}
		else if (click && x > start && x < start + TabHeight - 4){
			AddPage(true);
			return;
		}

		wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		if (i == -1){
			if (dclick){
				AddPendingEvent(evt2);
				AddPage(true);
				return;
			}
		}

		else if (i != iter && click){
			ChangePage(i);
		}
		else if (i == iter && click && (x > num + Tabsizes[i] - xWidth && x < num + Tabsizes[i] - 5)){
			DeletePage(i);
			AddPendingEvent(evt2);
			onx = false;
		}
		else if (middleDown)
		{
			int tmpiter = iter;
			DeletePage(i);
			if (i == tmpiter){ AddPendingEvent(evt2); }
			int tabAfterClose = FindTab(x, &num);
			if (tabAfterClose >= 0)
				SetToolTip(Pages[tabAfterClose]->SubsName + L"\n" + Pages[tabAfterClose]->VideoName);
			else
				UnsetToolTip();
			return;
		}

	}

	//swap tabs
	if (event.LeftIsDown() && i >= 0 && oldI >= 0 && i != oldI){
		wxString tmpname = Names[i];
		int tmpsize = Tabsizes[i];
		Names[i] = Names[oldI];
		Tabsizes[i] = Tabsizes[oldI];
		Names[oldI] = tmpname;
		Tabsizes[oldI] = tmpsize;
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
		return;
	}
	if (event.LeftUp() && (rightArrowClicked || leftArrowClicked)){
		rightArrowClicked = leftArrowClicked = false;
		RefreshRect(wxRect(0, hh, w, TabHeight), false); 
		return;
	}

	//Change elements on tabs
	if (event.Moving()){

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
		if (i == iter && (x > num + Tabsizes[i] - xWidth && x < num + Tabsizes[i] - 5)){
			if (!onx){ SetToolTip(_("Zamknij")); }
			onx = true;
			RefreshRect(wxRect(num + Tabsizes[i] - xWidth, hh, xWidth, TabHeight), false);
		}
		else if (onx){
			//Trick to restore tip with name of subtitles and video after go from x
			oldtab = -1;
			onx = false;
			RefreshRect(wxRect(num + Tabsizes[i] - xWidth, hh, xWidth, TabHeight), false);
		}
		if (i != -1 && i != oldtab){ SetToolTip(Pages[i]->SubsName + L"\n" + Pages[i]->VideoName); oldtab = i; }
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
	//RefreshRect(wxRect(w-20,h,w,TabHeight),false);
	if (split){
		bool aciter = (Pages[iter]->GetPosition().x == 1);
		int tmpsplititer = (!aciter) ? iter : splititer;
		int tmpiter = (aciter) ? iter : splititer;
		Pages[tmpsplititer]->SetSize(w - (splitline + 2), h);
		Pages[tmpiter]->SetSize((splitline - 2), h);
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
	dc.SelectObject(wxBitmap(w, TabHeight));
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

	//wxGraphicsContext *gc = NULL;//wxGraphicsContext::Create(dc);
	//loop for tabs drawing
	for (size_t i = firstVisibleTab; i < Tabsizes.GetCount(); i++){

		int tabSize = Tabsizes[i];
		wxString tabName = Names[i];
		if (iter < firstVisibleTab){
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
		/*if (gc){
			gc->SetPen(wxPen(Options.GetColour((i == iter) ? TABS_BORDER_ACTIVE : TABS_BORDER_INACTIVE)));
			gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
			wxGraphicsPath path = gc->CreatePath();
			path.MoveToPoint(start, 0.0);
			path.AddLineToPoint(start, TabHeight - 5);
			double strt = start;
			path.AddCurveToPoint(strt, TabHeight - 3.5, strt + 1.5, TabHeight - 2, strt + 3.0, TabHeight - 2);
			strt += tabSize;
			path.AddLineToPoint(strt - 3.0, TabHeight - 2);
			path.AddCurveToPoint(strt - 1.5, TabHeight - 2, strt, TabHeight - 3.5, strt, TabHeight - 5);
			path.AddLineToPoint(strt, 0);
			gc->StrokePath(path);
		}
		else{*/
			dc.SetPen(wxPen(Options.GetColour((i == iter) ? TABS_BORDER_ACTIVE : TABS_BORDER_INACTIVE)));
			dc.DrawLine(start, 0, start, TabHeight - 4);
			dc.DrawLine(start, TabHeight - 4, start + 2, TabHeight - 2);
			dc.DrawLine(start + 2, TabHeight - 2, start + tabSize - 2, TabHeight - 2);
			dc.DrawLine(start + tabSize - 2, TabHeight - 2, start + tabSize, TabHeight - 4);
			dc.DrawLine(start + tabSize, TabHeight - 4, start + tabSize, 0);
		//}

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
		if (firstVisibleTab < Tabsizes.GetCount() - 1){
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

	/*if (gc){
		gc->SetPen(wxPen(Options.GetColour(TABS_BORDER_INACTIVE)));
		gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
		wxGraphicsPath path = gc->CreatePath();
		path.MoveToPoint(start, 0.0);
		path.AddLineToPoint(start, TabHeight - 5);
		double strt = start;
		path.AddCurveToPoint(strt, TabHeight - 3.5, strt + 1.5, TabHeight - 2, strt + 3.0, TabHeight - 2);
		path.AddLineToPoint(strt + TabHeight - 8, TabHeight - 2);
		path.AddCurveToPoint(strt + TabHeight - 6.5, TabHeight - 2, strt + TabHeight - 5, TabHeight - 3.5, strt + TabHeight - 5, TabHeight - 5);
		path.AddLineToPoint(strt + TabHeight - 5, 0);
		gc->StrokePath(path);
	}
	else{*/
		dc.SetPen(wxPen(Options.GetColour(TABS_BORDER_INACTIVE)));
		dc.DrawLine(start, 0, start, TabHeight - 4);
		dc.DrawLine(start, TabHeight - 4, start + 2, TabHeight - 2);
		dc.DrawLine(start + 2, TabHeight - 2, start + TabHeight - 7, TabHeight - 2);
		dc.DrawLine(start + TabHeight - 7, TabHeight - 2, start + TabHeight - 5, TabHeight - 4);
		dc.DrawLine(start + TabHeight - 5, TabHeight - 4, start + TabHeight - 5, 0);
	//}


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

	//if (gc){ delete gc; }
}


void Notebook::ContextMenu(const wxPoint &pos, int i)
{
	Menu tabsMenu;

	for (int g = 0; g < Size(); g++)
	{
		tabsMenu.Append(MENU_CHOOSE + g, Page(g)->SubsName, L"", true, 0, 0, (g == iter) ? ITEM_RADIO : ITEM_NORMAL);
	}
	tabsMenu.AppendSeparator();
	tabsMenu.Append(MENU_SAVE + i, _("Zapisz"), _("Zapisz"))->Enable(i >= 0 && Pages[i]->Grid->file->CanSave());
	tabsMenu.Append(MENU_SAVE - 1, _("Zapisz wszystko"), _("Zapisz wszystko"));
	tabsMenu.Append(MENU_CHOOSE - 1, _("Zamknij wszystkie zakładki"), _("Zamknij wszystkie zakładki"));
	if ((i != iter && Size() > 1 && i != -1) || split){
		wxString txt = (split) ? _("Wyświetl jedną zakładkę") : _("Wyświetl dwie zakładki");
		tabsMenu.Append((MENU_CHOOSE - 2) - i, txt);
	}
	bool canCompare = (i != iter && Size() > 1 && i != -1);
	Menu *styleComparisonMenu = new Menu();
	if (canCompare){
		wxArrayString availableStyles;
		Pages[iter]->Grid->GetCommonStyles(Pages[i]->Grid, availableStyles);
		wxArrayString optionsCompareStyles;
		Options.GetTable(SUBS_COMPARISON_STYLES, optionsCompareStyles);
		for (size_t i = 0; i < availableStyles.size(); i++){
			MenuItem * styleItem = styleComparisonMenu->Append(4448, availableStyles[i], L"", true, NULL, NULL, ITEM_CHECK);
			if (optionsCompareStyles.Index(availableStyles[i]) != -1){ 
				styleItem->Check(); 
				SubsGridBase::compareStyles.Add(availableStyles[i]); 
			}
		}
	}
	int compareBy = Options.GetInt(SUBS_COMPARISON_TYPE);
	Menu *comparisonMenu = new Menu();
	comparisonMenu->Append(MENU_COMPARE + 1, _("Porównaj według czasów"), NULL, L"", ITEM_CHECK, canCompare)->Check(compareBy & COMPARE_BY_TIMES);
	comparisonMenu->Append(MENU_COMPARE + 2, _("Porównaj według widocznych linijek"), NULL, L"", ITEM_CHECK, canCompare)->Check((compareBy & COMPARE_BY_VISIBLE)>0);
	comparisonMenu->Append(MENU_COMPARE + 3, _("Porównaj według zaznaczeń"), NULL, L"", ITEM_CHECK, canCompare && Pages[iter]->Grid->file->SelectionsSize() > 0 && Pages[i]->Grid->file->SelectionsSize() > 0)->Check((compareBy & COMPARE_BY_SELECTIONS) > 0);
	comparisonMenu->Append(MENU_COMPARE + 4, _("Porównaj według stylów"), NULL, L"", ITEM_CHECK, canCompare)->Check((compareBy & COMPARE_BY_STYLES) > 0);
	comparisonMenu->Append(MENU_COMPARE + 5, _("Porównaj według wybranych stylów"), styleComparisonMenu, L"", ITEM_CHECK, canCompare)->Check(SubsGridBase::compareStyles.size() > 0);
	comparisonMenu->Append(MENU_COMPARE, _("Porównaj"))->Enable(canCompare);
	comparisonMenu->Append(MENU_COMPARE - 1, _("Wyłącz porównanie"))->Enable(SubsGridBase::hasCompare);
	tabsMenu.Append(MENU_COMPARE + 6, _("Porównanie napisów"), comparisonMenu, _("Porównanie napisów"))->Enable(canCompare || SubsGridBase::hasCompare);

	int id = tabsMenu.GetPopupMenuSelection(pos, this);

	if (id < 0){ return; }
	if (id >= MENU_CHOOSE - 101 && id <= MENU_CHOOSE + 99){
		OnTabSel(id);
	}
	else if (id == MENU_COMPARE){
		SubsGridBase::CG1 = Pages[iter]->Grid;
		SubsGridBase::CG2 = Pages[i]->Grid;
		SubsGridBase::SubsComparison();
		SubsGridBase::hasCompare = true;
		SubsGridBase::CG1->ShowSecondComparedLine(SubsGridBase::CG2->GetScrollPosition(), false, false, true);
	}
	else if (id == MENU_COMPARE - 1){
		SubsGridBase::RemoveComparison();
	}
	else if (id == MENU_SAVE - 1 || (i >= 0 && id == MENU_SAVE + i)){
		OnSave(id);
	}
}

void Notebook::OnTabSel(int id)
{
	int wtab = id - MENU_CHOOSE;
	if (Pages[iter]->Video->GetState() == Playing){ Pages[iter]->Video->Pause(); }
	if (wtab < -1){
		wtab = abs(wtab + 2);
		Split(wtab);
	}
	else if (wtab < 0){
		KainoteFrame *Kai = (KainoteFrame*)GetParent();
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
			Names.pop_back();
			Tabsizes.pop_back();
		}
		int pagesSize = Pages.size();
		if (iter <= pagesSize || splititer <= pagesSize)
			split = false;

		if (olditer >= pagesSize){ olditer = 0; }
		firstVisibleTab = 0;
		int w = -1, h = -1;
		if (pagesSize < 1){
			Pages.push_back(new TabPanel(this, (KainoteFrame*)GetParent()));
			wxString name = Pages[0]->SubsName;
			Names.Add(name);
			GetClientSize(&w, &h);
			Pages[0]->SetPosition(wxPoint(0, 0));
			Pages[0]->SetSize(w, h - TabHeight);
		}
		CalcSizes();
		if (w < 1){ GetClientSize(&w, &h); }
		RefreshRect(wxRect(0, h - TabHeight, w, TabHeight), false);
		Pages[iter]->Show();
		wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		AddPendingEvent(evt2);
	}
	else{
		TabPanel *tmp = Page(firstVisibleTab);
		tmp->Hide();
		Pages[firstVisibleTab] = Pages[wtab];
		Pages[firstVisibleTab]->Show();
		Pages[wtab] = tmp;
		wxString tmp1 = Names[firstVisibleTab];
		Names[firstVisibleTab] = Names[wtab];
		Names[wtab] = tmp1;
		Tabsizes[iter] -= xWidth;
		int tmp2 = Tabsizes[firstVisibleTab];
		Tabsizes[firstVisibleTab] = Tabsizes[wtab];
		Tabsizes[wtab] = tmp2;
		Tabsizes[firstVisibleTab] += xWidth;

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
	*_num = num;
	int restab = -1;
	for (size_t i = firstVisibleTab; i < Tabsizes.size(); i++){
		if (x > num && x < num + Tabsizes[i]){
			restab = i;
			*_num = num;
			break;
		}
		num += Tabsizes[i] + 2;
	}
	return restab;
}
void Notebook::ChangeActive()
{
	wxWindow *win = FindFocus();
	if (win && Pages[iter]->IsDescendant(win)){
		Pages[iter]->lastFocusedWindow = win;
	}
	else{ Pages[iter]->lastFocusedWindow = NULL; }
	int tmp = iter;
	iter = splititer;
	splititer = tmp;
	Tabsizes[iter] += xWidth;
	Tabsizes[splititer] -= xWidth;
	//kainoteFrame * Kai = ((kainoteFrame*)GetParent());
	wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
	evt2.SetInt(1);
	AddPendingEvent(evt2);
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
		tab->Grid->LoadSubtitles(s, ext);
	}

	tab->SubsPath = path;
	if (ext == L"ssa"){ ext = L"ass"; tab->SubsPath = tab->SubsPath.BeforeLast(L'.') + L".ass"; }
	tab->SubsName = tab->SubsPath.AfterLast(L'\\');
	tab->Video->DisableVisuals(ext != L"ass");
	if (active != -1 && active != tab->Grid->currentLine && active < tab->Grid->GetCount()){
		tab->Grid->SetActive(active);
	}
	if (scroll != -1)
		tab->Grid->ScrollTo(scroll);

	return true;
}

bool Notebook::LoadVideo(TabPanel *tab, KainoteFrame *main, const wxString & path, int position /*= -1*/, bool isFFMS2)
{
	bool isload = tab->Video->LoadVideo(path, (tab->editor) ? OPEN_DUMMY : 0, false, true, (position != -1)? isFFMS2 : -1);

	if (!isload){
		return false;
	}

	wxString audiopath = tab->Grid->GetSInfo(L"Audio File");
	wxString keyframespath = tab->Grid->GetSInfo(L"Keyframes File");
	if (audiopath.StartsWith(L"?")){ audiopath = path; }
	bool hasAudioPath = (!audiopath.empty() && ((wxFileExists(audiopath) && audiopath.find(L':') == 1) ||
		wxFileExists(audiopath.Prepend(path.BeforeLast(L'\\') + L"\\"))));
	bool hasKeyframePath = (!keyframespath.empty() && ((wxFileExists(keyframespath) && keyframespath.find(L':') == 1) ||
		wxFileExists(keyframespath.Prepend(path.BeforeLast(L'\\') + L"\\"))));

	if (hasAudioPath && audiopath != path){
		audiopath.Replace(L"/", L"\\");
		main->OpenAudioInTab(tab, 30040, audiopath);
	}

	if (hasKeyframePath){
		tab->Video->OpenKeyframes(keyframespath);
		tab->KeyframesPath = keyframespath;
	}

	tab->Edit->Frames->Enable(!tab->Video->IsDirectShow());
	tab->Edit->Times->Enable(!tab->Video->IsDirectShow());
	if (position != -1)
		tab->Video->Seek(position);

	return true;
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

Notebook *Notebook::sthis = NULL;

Notebook *Notebook::GetTabs()
{
	return sthis;
}
//checking and return null when all tabs removed or iter is >= then size
TabPanel *Notebook::GetTab()
{
	return (sthis->iter < sthis->Size())? sthis->Pages[sthis->iter] : NULL;
}

void Notebook::SaveLastSession(bool beforeClose)
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
			L"\r\nPosition: " << tab->Video->Tell() << L"\r\nFFMS2: " << !tab->Video->IsDirectShow() <<
			L"\r\nSubtitles: ";
		if (!beforeClose){
			wxString ext = (tab->Grid->subsFormat < SRT) ? L"ass" : (tab->Grid->subsFormat == SRT) ? L"srt" : L"txt";
			wxString fullPath = recoveryPath + tab->SubsName.BeforeLast(L'.') + L"." + ext;
			tab->Grid->SaveFile(fullPath);
			//recovery path for crash close
			result << fullPath;
		}
		else{
			//normal path for end close
			result << tab->SubsPath;
		}
		result << L"\r\nActive: " << tab->Grid->currentLine <<
			L"\r\nScroll: " << tab->Grid->GetScrollPosition() << L"\r\n";
		numtab++;
	}
	OpenWrite op;
	op.FileWrite(Options.configPath + L"\\LastSession.txt", result);

}

void Notebook::LoadLastSession(KainoteFrame* main)
{
	wxString riddenSession;
	OpenWrite op;
	if (op.FileOpen(Options.configPath + L"\\LastSession.txt", &riddenSession, false) && !riddenSession.empty()){
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
		sthis->Tabsizes.Clear();
		sthis->Names.Clear();
		sthis->iter = 0;
		wxString video;
		int videoPosition = 0;
		wxString subtitles;
		int activeLine = 0;
		int scrollPosition = 0;
		bool isFFMS2 = true;
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
			// no else cause hasMoreTokens have to be checked everytime
			bool hasnotMoreTokens = !tokenizer.HasMoreTokens();
			if (token.StartsWith(L"Tab: ", &rest) || hasnotMoreTokens){
				if (rest != L"0" || hasnotMoreTokens){
					sthis->AddPage(false);
					int lastTab = sthis->Size() - 1;
					TabPanel *tab = sthis->Pages[lastTab];
					if (!subtitles.empty()){
						sthis->LoadSubtitles(tab, subtitles, activeLine, scrollPosition);
						main->SetRecent();
					}
					if (!video.empty()){
						sthis->LoadVideo(tab, main, video, videoPosition, isFFMS2);
					}
					main->Label();
					tab->ShiftTimes->Contents();
					video = L"";
					videoPosition = 0;
					subtitles = L"";
					activeLine = 0;
					scrollPosition = 0;
					isFFMS2 = true;
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
		tab->Video->DeleteAudioCache();
		main->SetSubsResolution(false);
		main->UpdateToolbar();
		Options.SaveOptions(true, false);
	}
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

int Notebook::FindPanel(TabPanel* pan, bool safe /*= true*/)
{
	int i = 0;
	for (std::vector<TabPanel*>::iterator it = Pages.begin(); it != Pages.end(); it++)
	{
		if ((*it) == pan){ return i; }
		i++;
	}
	return safe? iter : -1;
}


LRESULT CALLBACK Notebook::PauseOnMinimalize(int code, WPARAM wParam, LPARAM lParam)
{

	if (code == HCBT_MINMAX){
		if (lParam == 7 && sthis->GetTab()->Video->GetState() == Playing){
			sthis->GetTab()->Video->Pause();
		}
	}
	//if (wParam == SC_RESTORE){
		//sthis->GetTab()->Grid->SetFocus();//Refresh(false);
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
	if (Pages[olditer]->Video->GetState() == Playing){ Pages[olditer]->Video->Pause(); }
	if (split){
		Pages[page]->SetPosition(Pages[iter]->GetPosition());
		Pages[page]->SetSize(Pages[iter]->GetSize());
	}
	Freeze();
	Pages[iter]->Hide();
	Tabsizes[iter] -= xWidth;
	Pages[page]->Show();
	Tabsizes[page] += xWidth;
	Thaw();
	iter = page;
	over = -1;
	if (makeActiveVisible)
		CalcSizes(true);

	RefreshBar();
	wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
	AddPendingEvent(evt2);
}

void Notebook::OnEraseBackground(wxEraseEvent &event)
{
}

void Notebook::OnCharHook(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	int ukey = event.GetUnicodeKey();
	bool nmodif = !(event.AltDown() || event.ControlDown() || event.ShiftDown());
	VideoCtrl *vb = GetTab()->Video;
	if (ukey == 179){ vb->Pause(); }
	//else if(ukey==178){wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,11015); vb->OnVButton(evt);}
	else if (ukey == 177){ vb->PrevChap(); }
	else if (ukey == 176){ vb->NextChap(); }
	//else if(ukey==175){vb->OnSPlus();return;}
	//else if(ukey==174){vb->OnSMinus();return;}
	else if (key == WXK_PAGEDOWN && nmodif && vb->HasFocus()){ vb->OnPrew(); return; }
	else if (key == WXK_PAGEUP && nmodif && vb->HasFocus()){ vb->OnNext(); return; }
	event.Skip();
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

void Notebook::RefreshVideo(bool resetParameters /*= false*/)
{
	for (int i = 0; i < sthis->Size(); i++){
		TabPanel *tab = sthis->Page(i);
		if (tab->Video->GetState() != None){
			tab->Video->OpenSubs(OPEN_DUMMY, true, true, true);
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
