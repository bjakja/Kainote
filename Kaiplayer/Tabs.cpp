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

#include <wx/graphics.h>
#include "Tabs.h"
#include "TabPanel.h"
#include "kainoteApp.h"
#include "Menu.h"
#include "KaiMessageBox.h"


Notebook::Notebook(wxWindow *parent, int id)
	: wxWindow(parent, id)
{
	firstVisibleTab = olditer = iter = 0;
	splitline = splititer = 0;
	oldtab = oldI = over = -1;
	block = split = onx = farr = rarr = plus = false;
	TabHeight = 25;
	allTabsVisible = arrow = true;
	sline = NULL;
	font = wxFont(9, wxSWISS, wxFONTSTYLE_NORMAL, wxNORMAL, false, "Tahoma", wxFONTENCODING_DEFAULT);
	sthis = this;

	Pages.push_back(new TabPanel(this, (KainoteFrame*)parent));

	wxString name = Pages[0]->SubsName;

	if (name.Len() > 35){ name = name.SubString(0, 35) + "..."; }
	Names.Add(name);

	CalcSizes();
	Hook = NULL;
	Hook = SetWindowsHookEx(WH_CBT, &PauseOnMinimalize, NULL, GetCurrentThreadId());//WH_MOUSE
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		MenuItem *item = (MenuItem*)evt.GetClientData();
		int id = item->id;
		int compareBy = Options.GetInt(SubsComparisonType);
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
			for (int i = 0; i < SubsGridBase::compareStyles.size(); i++){
				if (SubsGridBase::compareStyles[i] == name){
					if (!item->check){ SubsGridBase::compareStyles.RemoveAt(i); }
					found = true;
					break;
				}
			}
			if (!found && item->check){ SubsGridBase::compareStyles.Add(name); }
			Options.SetTable(SubsComparisonStyles, SubsGridBase::compareStyles, ",");
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
		Options.SetInt(SubsComparisonType, compareBy);
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
	if (Pages[iter]->Video->GetState() == Playing){ Pages[iter]->Video->Pause(); }
	int w, h;
	GetClientSize(&w, &h);
	//if(refresh){Freeze();}
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
	if (name.Len() > 35){ name = name.SubString(0, 35) + "..."; }
	Names.Add(name);

	Pages[iter]->SetPosition(Pages[olditer]->GetPosition());
	Pages[iter]->SetSize(Pages[olditer]->GetSize());
	CalcSizes(true);
	if (refresh){
		if (!Options.GetBool(EditorOn)){ KainoteFrame *kai = (KainoteFrame *)GetParent(); kai->HideEditor(false); }
		wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		AddPendingEvent(evt2);
		//Thaw();
		RefreshRect(wxRect(0, h - 25, w, 25), false);
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

int Notebook::Size()
{
	return Pages.size();
}

void Notebook::SetPageText(int page, const wxString &label)
{
	Names[page] = label;
	CalcSizes();
	int w, h;
	GetClientSize(&w, &h);
	RefreshRect(wxRect(0, h - 25, w, 25), false);
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
		RefreshRect(wxRect(0, siz.y - 25, siz.x, 25), false);
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
		Pages[0]->SetSize(w, h - 25);
	}


	if (Size() < 1){
		Pages.push_back(new TabPanel(this, Kai));
		wxString name = Pages[0]->SubsName;
		Names.Add(name);
		int w, h;
		GetClientSize(&w, &h);
		Pages[0]->SetPosition(wxPoint(0, 0));
		Pages[0]->SetSize(w, h - 25);

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
	//RefreshRect(wxRect(0,h-25,w,25),false);
	Refresh(false);
}

void Notebook::CalcSizes(bool makeActiveVisible)
{
	int all = 2;
	int w, h;
	GetClientSize(&w, &h);
	for (size_t i = 0; i < Size(); i++){
		int fw, fh;
		GetTextExtent(Names[i], &fw, &fh, NULL, NULL, &font);
		if (i == iter){ fw += 18; }
		if (i < Tabsizes.size()){ Tabsizes[i] = fw + 10; }
		else{ Tabsizes.Add(fw + 10); }
		all += Tabsizes[i] + 2;
	}
	allTabsVisible = (all < w - 22);
	if (allTabsVisible){ firstVisibleTab = 0; }
	if (makeActiveVisible && !allTabsVisible){
		int tabsWidth = 0;
		for (int i = iter; i >= 0; i--){
			tabsWidth += Tabsizes[i];
			if (tabsWidth > w - 52 || i == 0){
				if (firstVisibleTab < i || firstVisibleTab > iter){
					firstVisibleTab = i + ((iter - i) / 2);
				}
				break;
			}

		}
	}
}


void Notebook::OnMouseEvent(wxMouseEvent& event)
{
	int x = event.GetX(), y = event.GetY();
	bool click = event.LeftDown();
	bool dclick = event.LeftDClick();
	bool mdown = event.MiddleDown();


	int w, h, hh;
	GetClientSize(&w, &h);
	hh = h - 25;
	//if (event.ButtonDown()){ SetFocus(); }
	//wyłączanie wszystkich aktywności przy wyjściu z zakładek


	if (event.Leaving()){
		if (over != -1 || onx || farr || rarr || plus){
			over = -1; onx = farr = rarr = plus = false;
			RefreshRect(wxRect(0, h - 25, w, 25), false);
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
			sline = new wxDialog(this, -1, "", wxPoint(px, py), wxSize(3, h - 27), wxSTAY_ON_TOP | wxBORDER_NONE);
			sline->SetBackgroundColour("#000000");
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
		firstVisibleTab = MID(0, firstVisibleTab - step, Size() - 1);
		over = -1;
		RefreshBar();
		return;
	}

	int num;
	int i = FindTab(x, &num);

	// klik, dwuklik i środkowy
	if (click || dclick || mdown){
		oldI = i;

		if (!allTabsVisible && (click || dclick) && x < 20){
			if (firstVisibleTab > 0){
				firstVisibleTab--; RefreshRect(wxRect(0, hh, w, 25), false);
			}
			return;
		}
		else if (!allTabsVisible && (click || dclick) && x > w - 17 && x <= w){
			if (firstVisibleTab<Size() - 1){
				firstVisibleTab++; RefreshRect(wxRect(w - 17, hh, 17, 25), false);
			}
			return;
		}
		else if (click && x>start && x<start + 17){
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
		else if (i == iter && click && (x>num + Tabsizes[i] - 18 && x < num + Tabsizes[i] - 5)){
			DeletePage(i);
			AddPendingEvent(evt2);
			onx = false;
		}
		else if (mdown)
		{
			int tmpiter = iter;
			DeletePage(i);
			if (i == tmpiter){ AddPendingEvent(evt2); }
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
		RefreshRect(wxRect(0, hh, w, 25), false);
		if (oldI == splititer){
			splititer = i;
		}
		if (i == splititer){
			splititer = oldI;
		}
		oldI = i;
		return;
	}

	//ożywienie zakładek
	if (event.Moving()){

		if (x >= start + 17 && HasToolTips()){ UnsetToolTip(); }


		if (!allTabsVisible && x<20){
			if (farr) return;
			farr = true;
			RefreshRect(wxRect(0, hh, 20, 25), false); return;
		}
		else if (!allTabsVisible && x>w - 17 && x <= w){
			if (rarr) return;
			rarr = true; plus = false;
			RefreshRect(wxRect(w - 17, hh, 17, 25), false); return;
		}
		else if (x > start && x < start + 17){
			if (plus) return;
			plus = true; rarr = false;
			RefreshRect(wxRect(start, hh, start + 17, 25), false);
			//if(oldtab!=i){
			SetToolTip(_("Otwórz nową zakładkę"));
			//oldtab=i;}
			return;
		}
		else if (farr || rarr || plus){
			farr = rarr = plus = false;
			RefreshRect(wxRect(w - 19, hh, 19, 25), false); return;
		}

		if (i == -1){
			if (over != -1 || onx){
				over = -1; onx = false;
				RefreshRect(wxRect(0, hh, w, 25), false);
			}
			return;
		}

		if (i != iter && i != over){
			over = i;
			RefreshRect(wxRect(0, hh, w, 25), false);
		}
		else if (i == iter && over != -1){
			over = -1;
			RefreshRect(wxRect(0, hh, w, 25), false);
		}
		if (i == iter && (x > num + Tabsizes[i] - 18 && x < num + Tabsizes[i] - 5)){
			if (!onx){ SetToolTip(_("Zamknij")); }
			onx = true;
			RefreshRect(wxRect(num + Tabsizes[i] - 18, hh, 15, 25), false);
		}
		else if (onx){
			oldtab = -1;//trick aby po zejściu z x powrócił tip z nazwą napisów i wideo
			onx = false;
			RefreshRect(wxRect(num + Tabsizes[i] - 18, hh, 15, 25), false);
		}
		if (i != -1 && i != oldtab){ SetToolTip(Pages[i]->SubsName + "\n" + Pages[i]->VideoName); oldtab = i; }
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
	//RefreshRect(wxRect(w-20,h,w,25),false);
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
	if (alvistmp != allTabsVisible){ RefreshRect(wxRect(0, h - 25, w, 25), false); }
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
		Options.GetColour(TabsBarBackground2),
		Options.GetColour(TabsBarBackground1), wxTOP);
	const wxColour & activeLines = Options.GetColour(TabsBorderActive);
	const wxColour & activeText = Options.GetColour(TabsTextActive);
	const wxColour & inactiveText = Options.GetColour(TabsTextInactive);


	start = (allTabsVisible) ? 2 : 20;


	wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
	//pętla do rysowania zakładek
	for (size_t i = firstVisibleTab; i < Tabsizes.size(); i++){
		//wybrana zakładka
		if (i == iter){
			//rysowanie linii po obu stronach aktywnej zakładki
			dc.SetPen(wxPen(activeLines, 1));
			dc.DrawLine(0, 0, start, 0);
			dc.DrawLine(start + Tabsizes[i], 0, w, 0);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(Options.GetColour(TabsBackgroundActive));
			dc.DrawRectangle(start + 1, 0, Tabsizes[i] - 1, 23);


			//najechany x na wybranej zakładce
			if (onx){
				dc.SetBrush(Options.GetColour(TabsCloseHover));
				dc.DrawRectangle(start + Tabsizes[i] - 19, 3, 15, 15);
			}
			//dc.SetTextForeground(Options.GetColour("Tabs Close Hover"));
			dc.SetTextForeground(activeText);
			dc.DrawText("X", start + Tabsizes[i] - 15, 3);


		}
		else if (split && i == splititer){
			dc.SetTextForeground(inactiveText);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(Options.GetColour((i == (size_t)over) ? TabsBackgroundInactiveHover : TabsBackgroundSecondWindow));
			dc.DrawRectangle(start + 1, 1, Tabsizes[i] - 1, 23);
		}
		else{
			//nieaktywna lub najechana nieaktywna zakładka
			dc.SetTextForeground(inactiveText);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxBrush(Options.GetColour((i == (size_t)over) ? TabsBackgroundInactiveHover : TabsBackgroundInactive)));
			dc.DrawRectangle(start + 1, 1, Tabsizes[i] - 1, 22);
		}

		//rysowanie konturów zakładki
		if (gc){
			gc->SetPen(wxPen(Options.GetColour((i == iter) ? TabsBorderActive : TabsBorderInactive)));
			gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
			wxGraphicsPath path = gc->CreatePath();
			path.MoveToPoint(start, 0.0);
			path.AddLineToPoint(start, 20.0);
			double strt = start;
			path.AddCurveToPoint(strt, 21.5, strt + 1.5, 23.0, strt + 3.0, 23.0);
			strt += Tabsizes[i];
			path.AddLineToPoint(strt - 3.0, 23.0);
			path.AddCurveToPoint(strt - 1.5, 23.0, strt, 21.5, strt, 20.0);
			path.AddLineToPoint(strt, 0);
			gc->StrokePath(path);
		}
		else{
			dc.SetPen(wxPen(Options.GetColour((i == iter) ? TabsBorderActive : TabsBorderInactive)));
			dc.DrawLine(start, 0, start, 21);
			dc.DrawLine(start, 21, start + 2, 23);
			dc.DrawLine(start + 2, 23, start + Tabsizes[i] - 2, 23);
			dc.DrawLine(start + Tabsizes[i] - 2, 23, start + Tabsizes[i], 21);
			dc.DrawLine(start + Tabsizes[i], 21, start + Tabsizes[i], 0);
		}
		dc.DrawText(Names[i], start + 4, 3);

		start += Tabsizes[i] + 2;
	}

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Options.GetColour(TabsBarArrowBackground)));
	//strzałki do przesuwania zakładek
	if (!allTabsVisible){
		const wxColour & backgroundHover = Options.GetColour(TabsBarArrowBackgroundHover);
		const wxColour & arrow = Options.GetColour(TabsBarArrow);
		dc.DrawRectangle(w - 16, 0, 16, 25);
		if (farr){ dc.SetBrush(wxBrush(backgroundHover)); }
		dc.DrawRectangle(0, 0, 16, 25);

		if (rarr){
			dc.SetBrush(wxBrush(backgroundHover));
			dc.DrawRectangle(w - 16, 0, 16, 25);
		}

		dc.SetPen(wxPen(arrow, 2));

		dc.DrawLine(17, 0, 17, 25);
		dc.DrawLine(11, 5, 4, 12);
		dc.DrawLine(4, 12, 11, 19);

		dc.DrawLine(w - 17, 0, w - 17, 25);
		dc.DrawLine(w - 11, 5, w - 4, 12);
		dc.DrawLine(w - 4, 12, w - 11, 19);
	}

	//plus który jest zawsze widoczny

	dc.SetBrush(wxBrush(Options.GetColour((plus) ? TabsBackgroundInactiveHover : TabsBackgroundInactive)));
	//if(plus){
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(start + 1, 1, 18, 22);
	//}

	//dc.SetPen(wxPen(inactiveText));
	dc.SetBrush(wxBrush(inactiveText));
	dc.DrawRectangle(start + 4, 11, 12, 2);
	dc.DrawRectangle(start + 9, 6, 2, 12);

	if (gc){
		gc->SetPen(wxPen(Options.GetColour(TabsBorderInactive)));
		gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
		wxGraphicsPath path = gc->CreatePath();
		path.MoveToPoint(start, 0.0);
		path.AddLineToPoint(start, 20.0);
		double strt = start;
		path.AddCurveToPoint(strt, 21.5, strt + 1.5, 23.0, strt + 3.0, 23.0);
		path.AddLineToPoint(strt + 16.0, 23.0);
		path.AddCurveToPoint(strt + 17.5, 23.0, strt + 19.0, 21.5, strt + 19.0, 20.0);
		path.AddLineToPoint(strt + 19, 0);
		gc->StrokePath(path);
	}
	else{
		dc.SetPen(wxPen(Options.GetColour(TabsBorderInactive)));
		dc.DrawLine(start, 0, start, 21);
		dc.DrawLine(start, 21, start + 2, 23);
		dc.DrawLine(start + 2, 23, start + 17, 23);
		dc.DrawLine(start + 17, 23, start + 19, 21);
		dc.DrawLine(start + 19, 21, start + 19, 0);
	}


	cdc.Blit(0, h - 25, w, TabHeight, &dc, 0, 0);
	if (split){
		cdc.SetPen(*wxTRANSPARENT_PEN);
		cdc.SetBrush(Options.GetColour(TabsBarBackground1));
		cdc.DrawRectangle(splitline - 2, 0, 4, h - 25);
		cdc.SetPen(wxPen(Options.GetColour(WindowBackground)));
		bool aciter = (Pages[iter]->GetPosition().x == 1);
		if (aciter){
			cdc.DrawLine(splitline + 1, 0, w, 0);
			cdc.DrawLine(w - 1, 0, w - 1, h - 26);
			cdc.DrawLine(splitline + 1, h - 26, w, h - 26);
			cdc.SetPen(wxPen("#FF0000"));
			cdc.DrawLine(0, 0, 0, h - 26);
			cdc.DrawLine(0, 0, splitline - 1, 0);
			cdc.DrawLine(splitline - 1, 0, splitline - 1, h - 26);
			cdc.DrawLine(0, h - 26, splitline - 1, h - 26);
		}
		else{
			cdc.DrawLine(0, 0, splitline - 1, 0);
			cdc.DrawLine(0, h - 26, splitline - 1, h - 26);
			cdc.DrawLine(0, 0, 0, h - 26);
			cdc.SetPen(wxPen("#FF0000"));
			cdc.DrawLine(splitline + 1, 0, w, 0);
			cdc.DrawLine(splitline + 1, 0, splitline + 1, h - 26);
			cdc.DrawLine(w - 1, 0, w - 1, h - 26);
			cdc.DrawLine(splitline + 1, h - 26, w, h - 26);
		}
	}

	if (gc){ delete gc; }
}


void Notebook::ContextMenu(const wxPoint &pos, int i)
{
	Menu tabsMenu;

	for (int g = 0; g < Size(); g++)
	{
		tabsMenu.Append(MENU_CHOOSE + g, Page(g)->SubsName, "", true, 0, 0, (g == iter) ? ITEM_RADIO : ITEM_NORMAL);
	}
	//może to jednak przerobić na checki, tak by pokazywało nam jednak dwie wyświetlone zakładki
	tabsMenu.AppendSeparator();
	tabsMenu.Append(MENU_SAVE + i, _("Zapisz"), _("Zapisz"))->Enable(i >= 0 && Pages[i]->Grid->file->CanSave());
	tabsMenu.Append(MENU_SAVE - 1, _("Zapisz wszystko"), _("Zapisz wszystko"));
	tabsMenu.Append(MENU_CHOOSE - 1, _("Zamknij wszystkie zakładki"), _("Zamknij wszystkie zakładki"));
	if ((i != iter && Size()>1 && i != -1) || split){
		wxString txt = (split) ? _("Wyświetl jedną zakładkę") : _("Wyświetl dwie zakładki");
		tabsMenu.Append((MENU_CHOOSE - 2) - i, txt);
	}
	bool canCompare = (i != iter && Size() > 1 && i != -1);
	Menu *styleComparisonMenu = new Menu();
	if (canCompare){
		wxArrayString availableStyles;
		Pages[iter]->Grid->GetCommonStyles(Pages[i]->Grid, availableStyles);
		wxArrayString optionsCompareStyles;
		Options.GetTable(SubsComparisonStyles, optionsCompareStyles, ",");
		for (int i = 0; i < availableStyles.size(); i++){
			MenuItem * styleItem = styleComparisonMenu->Append(4448, availableStyles[i], "", true, NULL, NULL, ITEM_CHECK);
			if (optionsCompareStyles.Index(availableStyles[i]) != -1){ 
				styleItem->Check(); 
				SubsGridBase::compareStyles.Add(availableStyles[i]); 
			}
		}
	}
	int compareBy = Options.GetInt(SubsComparisonType);
	Menu *comparisonMenu = new Menu();
	comparisonMenu->Append(MENU_COMPARE + 1, _("Porównaj według czasów"), NULL, "", ITEM_CHECK, canCompare)->Check(compareBy & COMPARE_BY_TIMES);
	comparisonMenu->Append(MENU_COMPARE + 2, _("Porównaj według widocznych linijek"), NULL, "", ITEM_CHECK, canCompare)->Check((compareBy & COMPARE_BY_VISIBLE)>0);
	comparisonMenu->Append(MENU_COMPARE + 3, _("Porównaj według zaznaczeń"), NULL, "", ITEM_CHECK, canCompare && Pages[iter]->Grid->file->SelectionsSize() > 0 && Pages[i]->Grid->file->SelectionsSize() > 0)->Check((compareBy & COMPARE_BY_SELECTIONS) > 0);
	comparisonMenu->Append(MENU_COMPARE + 4, _("Porównaj według stylów"), NULL, "", ITEM_CHECK, canCompare)->Check((compareBy & COMPARE_BY_STYLES) > 0);
	comparisonMenu->Append(MENU_COMPARE + 5, _("Porównaj według wybranych stylów"), styleComparisonMenu, "", ITEM_CHECK, canCompare)->Check(SubsGridBase::compareStyles.size() > 0);
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
			Pages[0]->SetSize(w, h - 25);
		}
		CalcSizes();
		if (w < 1){ GetClientSize(&w, &h); }
		RefreshRect(wxRect(0, h - 25, w, 25), false);
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
		Tabsizes[iter] -= 18;
		int tmp2 = Tabsizes[firstVisibleTab];
		Tabsizes[firstVisibleTab] = Tabsizes[wtab];
		Tabsizes[wtab] = tmp2;
		Tabsizes[firstVisibleTab] += 18;

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
	Tabsizes[iter] += 18;
	Tabsizes[splititer] -= 18;
	//kainoteFrame * Kai = ((kainoteFrame*)GetParent());
	wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
	evt2.SetInt(1);
	AddPendingEvent(evt2);
	RefreshBar();
}

void Notebook::RefreshBar()
{
	int w, h;
	GetClientSize(&w, &h);
	RefreshRect(wxRect(0, h - 25, w, 25), false);
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

TabPanel *Notebook::GetTab()
{
	return sthis->Pages[sthis->iter];
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
		if (lParam == 7 && sthis->GetTab()->Video->vstate == Playing){
			sthis->GetTab()->Video->Pause();
		}
	}
	//if (wParam == SC_RESTORE){
	//sthis->GetTab()->Refresh(false);
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
	Pages[page]->Show();
	Tabsizes[page] += 18;
	Pages[iter]->Hide();
	Tabsizes[iter] -= 18;
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


BEGIN_EVENT_TABLE(Notebook, wxWindow)
EVT_CHAR_HOOK(Notebook::OnCharHook)
EVT_ERASE_BACKGROUND(Notebook::OnEraseBackground)
EVT_MOUSE_EVENTS(Notebook::OnMouseEvent)
EVT_SIZE(Notebook::OnSize)
EVT_PAINT(Notebook::OnPaint)
EVT_MOUSE_CAPTURE_LOST(Notebook::OnLostCapture)
END_EVENT_TABLE()
