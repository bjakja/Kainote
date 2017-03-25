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


Notebook::Notebook(wxWindow *parent, int id)
	: wxWindow(parent,id)
{
	fvis=olditer=iter=0;
	splitline=splititer=compareSecondTab=0;
	oldtab=oldI=over=-1;
	block=split=onx=farr=rarr=plus=false;
	hasCompare=false;
	TabHeight=25;
	allvis=arrow=true;
	sline=NULL;
	font=wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	sthis=this;

	Pages.push_back(new TabPanel(this,(kainoteFrame*)parent));

	wxString name=Pages[0]->SubsName;

	if(name.Len()>35){name=name.SubString(0,35)+"...";}
	Names.Add(name);

	CalcSizes();
	Hook = NULL;
	Hook = SetWindowsHookEx(WH_CBT, &PauseOnMinimalize, NULL,GetCurrentThreadId());//WH_MOUSE
}


Notebook::~Notebook(){
	for(std::vector<TabPanel*>::iterator i=Pages.begin(); i!=Pages.end(); i++)
	{
		(*i)->Destroy();
	}
	Pages.clear();
	Tabsizes.Clear();
	Names.Clear();
	UnhookWindowsHookEx( Hook );
}

TabPanel *Notebook::GetPage()
{
	return Pages[iter];
}

void Notebook::AddPage(bool refresh)
{
	if(Pages[iter]->Video->GetState()==Playing){Pages[iter]->Video->Pause();}
	int w,h;
	GetClientSize(&w,&h);
	//if(refresh){Freeze();}
	Pages.push_back(new TabPanel(this,(kainoteFrame*)GetParent(),wxPoint(0,0), wxSize(0,0)));
	olditer=iter;
	iter=Size()-1;
	if(refresh){
		Pages[olditer]->Hide();
	}else{
		Pages[iter]->Hide();
	}
	wxString name=Pages[iter]->SubsName;
	if(name.Len()>35){name=name.SubString(0,35)+"...";}
	Names.Add(name);

	Pages[iter]->SetPosition(Pages[olditer]->GetPosition());
	Pages[iter]->SetSize(Pages[olditer]->GetSize());
	CalcSizes();
	if(refresh){
		if(!Options.GetBool(EditorOn)){kainoteFrame *kai=(kainoteFrame *)GetParent();kai->HideEditor();}
		wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		AddPendingEvent(evt2);
		//Thaw();
		RefreshRect(wxRect(0,h-25,w,25),false);
	}else{Pages[iter]->CTime->RefVals(Pages[olditer]->CTime);}
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

void Notebook::SetPageText(int page, wxString label)
{
	Names[page]=label;
	CalcSizes();
	int w,h;
	GetClientSize(&w,&h);
	RefreshRect(wxRect(0,h-25,w,25),false);
}

TabPanel *Notebook::Page(size_t i)
{
	return Pages[i];
}

void Notebook::DeletePage(size_t page)
{
	Freeze();
	kainoteFrame *Kai=(kainoteFrame*)GetParent();
	block=true;
	if(Kai->SavePrompt(1,page)){
		block=false; 
		wxSize siz=GetClientSize();
		RefreshRect(wxRect(0,siz.y-25,siz.x,25),false); 
		Thaw();
		return;
	}
	block=false;
	if(split && Size()>2){
		int i = 0;
		for(; i < (int)Size(); i++){
			if(i!=iter && i!=splititer){break;}
		}
		Pages[i]->SetSize(Pages[page]->GetSize());
		Pages[i]->SetPosition(Pages[page]->GetPosition());
		Pages[i]->Show();
	}
	Pages[page]->Destroy();
	Pages.erase(Pages.begin()+page);
	Names.RemoveAt(page);
	Tabsizes.RemoveAt(page);

	if(split && Size()<2){
		split=false;
		int w,h;
		GetClientSize(&w,&h);
		Pages[0]->SetPosition(wxPoint(0,0));
		Pages[0]->SetSize(w,h-25);
	}


	if(Size()<1){
		Pages.push_back(new TabPanel(this,Kai));
		wxString name=Pages[0]->SubsName;
		Names.Add(name);
		int w,h;
		GetClientSize(&w,&h);
		Pages[0]->SetPosition(wxPoint(0,0));
		Pages[0]->SetSize(w,h-25);

	}

	size_t rsize=Size()-1;
	if(olditer>rsize){olditer=rsize;}
	if(iter>rsize){iter=rsize;}
	else if(page<iter){iter--;}
	if(page>rsize){page=rsize;}
	if(fvis>rsize){fvis=rsize;}

	CalcSizes();
	if(page==iter){
		Pages[iter]->Show();
	}
	Thaw();

	//int w,h;
	//GetClientSize(&w,&h);
	//RefreshRect(wxRect(0,h-25,w,25),false);
	Refresh(false);
}

void Notebook::CalcSizes()
{
	wxClientDC dc(this);
	dc.SetFont(font);
	int all=2;
	int w,h;
	GetClientSize(&w,&h);
	for(size_t i=0;i<Size();i++)
	{
		int fw,fh;
		dc.GetTextExtent(Names[i], &fw, &fh, NULL, NULL, &font);
		if(i==iter){fw+=18;}
		if(i<Tabsizes.size()){Tabsizes[i]=fw+10;}
		else{Tabsizes.Add(fw+10);}
		all+=Tabsizes[i]+2;
	}
	allvis=(all<w-22);
	if(allvis){fvis=0;}
}


void Notebook::OnMouseEvent(wxMouseEvent& event)
{
	int x=event.GetX(), y=event.GetY();
	bool click=event.LeftDown();
	bool dclick=event.LeftDClick();
	bool mdown=event.MiddleDown();

	if(event.ButtonDown()){SetFocus();}

	int w,h,hh;
	GetClientSize(&w,&h);
	hh=h-25;

	//wyłączanie wszystkich aktywności przy wyjściu z zakładek

	if(event.Leaving()){
		if(over!=-1 || onx ||farr||rarr||plus){
			over=-1;onx=farr=rarr=plus=false;
			RefreshRect(wxRect(0,h-25,w,25),false);
		}
		oldtab=-1;
		UnsetToolTip();
		if(!arrow){SetCursor(wxCURSOR_ARROW);arrow=true;}
		return;
	}

	if(y<hh){

		if(click){
			CaptureMouse();
			int px=x, py=2;
			ClientToScreen(&px,&py);
			sline= new wxDialog(this,-1,"",wxPoint(px,py),wxSize(3,h-27),wxSTAY_ON_TOP|wxBORDER_NONE);
			sline->SetBackgroundColour("#000000");
			sline->Show();
		}
		else if(event.LeftUp())
		{
			int npos=x;
			if(sline){
				int yy; 
				sline->GetPosition(&npos,&yy);
				ScreenToClient(&npos,&yy);
				sline->Destroy();
				sline = NULL;
			}
			if(HasCapture()){ReleaseMouse();}
			splitline=npos;
			bool aciter=(Pages[iter]->GetPosition().x==1);
			int tmpiter=(aciter)? iter : splititer;
			int tmpsplititer=(!aciter)? iter : splititer;
			//wxLogStatus("size iter %i splititer %i", tmpiter, tmpsplititer);
			Pages[tmpiter]->SetSize(splitline-3,hh-2);
			Pages[tmpsplititer]->SetSize(w-(splitline+3),hh-2);
			Pages[tmpsplititer]->SetPosition(wxPoint(splitline+2,1));
			Refresh(false);//wxRect(0,hh,w,25),
			SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
		}
		else if(event.LeftIsDown())
		{
			if(x!=splitline){
				int px=MID(200,x,w-200), py=2;
				ClientToScreen(&px,&py);
				if(sline){sline->SetPosition(wxPoint(px,py));}
			}


		}
		if(arrow && split){SetCursor(wxCURSOR_SIZEWE);arrow=false;}
		return;
	}

	if(!arrow){SetCursor(wxCURSOR_ARROW);arrow=true;}



	int num;
	int i = FindTab(x, &num);


	// klik, dwuklik i środkowy
	if(click||dclick||mdown){
		oldI=i;

		if(!allvis && (click || dclick) && x<20){
			if(fvis>0){
				fvis--;RefreshRect(wxRect(0,hh,w,25),false);
			}
			return;
		}
		else if(!allvis && (click || dclick) && x>w-17 && x<=w){
			if(fvis<Size()-1){
				fvis++;RefreshRect(wxRect(w-17,hh,17,25),false);
			}
			return;
		}
		else if(click && x>start && x<start+17){
			AddPage(true);
			return;
		}

		wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		if(i==-1){
			if(dclick){
				AddPendingEvent(evt2);
				AddPage(true);
				return;
			}
		}

		else if(i!=iter&&click){
			ChangePage(i);
		}
		else if(i==iter && click && (x>num+Tabsizes[i]-18 && x<num+Tabsizes[i]-5)){
			DeletePage(i);
			AddPendingEvent(evt2);
			onx=false;
		}
		else if(mdown)
		{
			int tmpiter=iter;
			DeletePage(i);
			if(i==tmpiter){AddPendingEvent(evt2);}
		}

	}	


	if(event.LeftIsDown() && i >= 0 && oldI >= 0 && i != oldI){
		wxString tmpname = Names[i];
		int tmpsize = Tabsizes[i];
		Names[i] = Names[oldI];
		Tabsizes[i] = Tabsizes[oldI];
		Names[oldI] = tmpname;
		Tabsizes[oldI] = tmpsize;
		TabPanel *tmppage = Pages[i];
		Pages[i] = Pages[oldI];
		Pages[oldI] = tmppage;
		iter=i;
		RefreshRect(wxRect(0,hh,w,25),false);
		if(oldI==splititer){
			splititer = i;
		}
		if(i==splititer){
			splititer = oldI;
		}
		oldI=i;
		return;
	}

	//ożywienie zakładek
	if(event.Moving()){

		if(x>=start+17 && HasToolTips()){UnsetToolTip();}


		if(!allvis && x<20){
			if(farr) return;
			farr=true;
			RefreshRect(wxRect(0,hh,20,25),false);return;
		}
		else if(!allvis && x>w-17 && x<=w){
			if(rarr) return;
			rarr=true;plus=false;
			RefreshRect(wxRect(w-17,hh,17,25),false);return;
		}
		else if(x>start && x<start+17){
			if(plus) return;
			plus=true;rarr=false;
			RefreshRect(wxRect(start,hh,start+17,25),false);
			//if(oldtab!=i){
			SetToolTip(_("Otwórz nową zakładkę"));
			//oldtab=i;}
			return;
		}
		else if(farr||rarr||plus){
			farr=rarr=plus=false;
			RefreshRect(wxRect(w-19,hh,19,25),false);return;
		}

		if(i==-1){
			if(over!=-1||onx){
				over=-1;onx=false;
				RefreshRect(wxRect(0,hh,w,25),false);
			}
			return;
		}

		if(i!=iter && i!=over){
			over=i;
			RefreshRect(wxRect(0,hh,w,25),false);
		}else if(i==iter && over!=-1){
			over=-1;
			RefreshRect(wxRect(0,hh,w,25),false);
		}
		if(i==iter && (x>num+Tabsizes[i]-18 && x<num+Tabsizes[i]-5)){
			if(!onx){SetToolTip(_("Zamknij"));}
			onx=true;
			RefreshRect(wxRect(num+Tabsizes[i]-18,hh,15,25),false);
		}else if(onx){
			oldtab=-1;//trick aby po zejściu z x powrócił tip z nazwą napisów i wideo
			onx=false;
			RefreshRect(wxRect(num+Tabsizes[i]-18,hh,15,25),false);
		}
		if(i!=-1 && i!= oldtab){SetToolTip(Pages[i]->SubsName+"\n"+Pages[i]->VideoName); oldtab=i;}
	}

	//menu kontekstowe		
	if(event.RightUp()){

		Menu menu1;

		for(size_t g=0;g<Size();g++)
		{
			menu1.Append(MENU_CHOOSE+g,Page(g)->SubsName,"",true,0,0,(g==iter)? ITEM_RADIO : ITEM_NORMAL);
		}
		//menu1.Check(MENU_CHOOSE+iter,true);
		//może to jednak przerobić na checki, tak by pokazywało nam jednak dwie wyświetlone zakładki
		menu1.AppendSeparator();
		if(i>=0){menu1.Append(MENU_SAVE+i,_("Zapisz"),_("Zapisz"));}
		menu1.Append(MENU_SAVE-1,_("Zapisz wszystko"),_("Zapisz wszystko"));
		menu1.Append(MENU_CHOOSE-1,_("Zamknij wszystkie zakładki"),_("Zamknij wszystkie zakładki"));
		if((i!=iter && Size()>1 && i!=-1) || split){
			wxString txt=(split)? _("Wyświetl jedną zakładkę") : _("Wyświetl dwie zakładki");
			menu1.Append((MENU_CHOOSE-2)-i, txt);
		}
		if((i!=iter && Size()>1 && i!=-1) || hasCompare){
			wxString txt=(hasCompare)? _("Wyłącz porównanie plików") : _("Włącz porównanie plików");
			menu1.Append(MENU_COMPARE, txt);
		}
		int id=menu1.GetPopupMenuSelection(event.GetPosition(),this);
		//wxLogStatus("id %i", id);
		if(id<0){return;}
		if(id >= MENU_CHOOSE-101 && id <= MENU_CHOOSE+99){
			OnTabSel(id);
		}else if(id == MENU_COMPARE){
			if(hasCompare){
				delete Pages[compareFirstTab]->Grid1->Comparsion;
				Pages[compareFirstTab]->Grid1->Comparsion = NULL;
				Pages[compareFirstTab]->Grid1->Refresh(false);
				delete Pages[compareSecondTab]->Grid1->Comparsion;
				Pages[compareSecondTab]->Grid1->Comparsion = NULL;
				Pages[compareSecondTab]->Grid1->Refresh(false);
				compareFirstTab=0;
				compareSecondTab=0;
				hasCompare=false;
				return;
			}
			compareFirstTab=iter;
			compareSecondTab=i;
			SubsComparsion();
			hasCompare=true;
		}else{
			OnSave(id);
		}

		return;
	}


}

void Notebook::OnSize(wxSizeEvent& event)
{
	//sizetimer.Start(500,true);
	int w,h;
	GetClientSize(&w,&h);
	h-=TabHeight;
	bool alvistmp=allvis;
	CalcSizes();
	//RefreshRect(wxRect(w-20,h,w,25),false);
	if(split){
		bool aciter=(Pages[iter]->GetPosition().x==1);
		int tmpsplititer=(!aciter)? iter : splititer;
		int tmpiter=(aciter)? iter : splititer;
		Pages[tmpsplititer]->SetSize(w-(splitline+2),h);
		Pages[tmpiter]->SetSize((splitline-2),h);
	}else{
		Pages[iter]->SetSize(w,h);
	}
	if(alvistmp!=allvis){RefreshRect(wxRect(0,h-25,w,25),false);}
	SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
}

void Notebook::OnPaint(wxPaintEvent& event)
{
	//wxLogStatus("paint %i", (int)block);
	if(block){return;}
	//wxLogStatus("paint iter %i splititer %i", (int)iter, splititer);
	int w,h;
	GetClientSize(&w,&h);
	//wxLogStatus("paint %i %i", w, h);
	//h-=TabHeight;
	wxClientDC cdc(this);
	wxMemoryDC dc;
	dc.SelectObject(wxBitmap(w,TabHeight));
	dc.SetFont(font);
	//dc.SetPen(*wxTRANSPARENT_PEN);
	//dc.SetBrush(wxBrush(Options.GetColour("Menu Bar Background 2")));
	dc.GradientFillLinear(wxRect(0,0,w,TabHeight),
		Options.GetColour(TabsBarBackground2),
		Options.GetColour(TabsBarBackground1),wxTOP);
	wxColour activeLines = Options.GetColour(TabsBorderActive);
	wxColour activeText = Options.GetColour(TabsTextActive);
	wxColour inactiveText = Options.GetColour(TabsTextInactive);

	
	start=(allvis)?2 : 20;


	wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
	//pętla do rysowania zakładek
	for(size_t i=fvis;i<Tabsizes.size();i++){
		//wybrana zakładka
		if(i==iter){
			//rysowanie linii po obu stronach aktywnej zakładki
			dc.SetPen(wxPen(activeLines,1));
			dc.DrawLine(0,0,start,0);
			dc.DrawLine(start+Tabsizes[i],0,w,0);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(Options.GetColour(TabsBackgroundActive));
			dc.DrawRectangle(start+1,0,Tabsizes[i]-1,23);


			//najechany x na wybranej zakładce
			if(onx){
				dc.SetBrush(Options.GetColour(TabsCloseHover));
				dc.DrawRectangle(start+Tabsizes[i]-19,3,15,15);
			}
			//dc.SetTextForeground(Options.GetColour("Tabs Close Hover"));
			dc.SetTextForeground(activeText);
			dc.DrawText("X",start+Tabsizes[i]-15,3);
			

		}else{
			//nieaktywna lub najechana nieaktywna zakładka
			dc.SetTextForeground(inactiveText);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxBrush(Options.GetColour((i==(size_t)over)? TabsBackgroundInactiveHover : TabsBackgroundInactive)));
			dc.DrawRectangle(start+1,1,Tabsizes[i]-1,22);
		}

		//rysowanie konturów zakładki
		if(gc){
			gc->SetPen( wxPen(Options.GetColour((i==iter)? TabsBorderActive : TabsBorderInactive)));
			gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
			wxGraphicsPath path = gc->CreatePath();
			path.MoveToPoint(start,0.0);
			path.AddLineToPoint(start,20.0);
			double strt=start;
			path.AddCurveToPoint(strt, 21.5, strt+1.5, 23.0, strt+3.0, 23.0);
			strt+=Tabsizes[i];
			path.AddLineToPoint(strt-3.0,23.0);
			path.AddCurveToPoint(strt-1.5, 23.0, strt, 21.5, strt, 20.0);
			path.AddLineToPoint(strt,0);
			gc->StrokePath(path);
		}
		else{
			dc.SetPen(wxPen(Options.GetColour((i==iter)? TabsBorderActive : TabsBorderInactive)));
			dc.DrawLine(start,0,start,21);
			dc.DrawLine(start,21,start+2,23);
			dc.DrawLine(start+2,23,start+Tabsizes[i]-2,23);
			dc.DrawLine(start+Tabsizes[i]-2,23,start+Tabsizes[i],21);
			dc.DrawLine(start+Tabsizes[i],21,start+Tabsizes[i],0);
		}
		dc.DrawText(Names[i],start+4,3);

		start+=Tabsizes[i]+2;
	}

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Options.GetColour(TabsBarArrowBackground)));
	//strzałki do przesuwania zakładek
	if(!allvis){
		wxColour backgroundHover = Options.GetColour(TabsBarArrowBackgroundHover);
		wxColour arrow = Options.GetColour(TabsBarArrow);
		dc.DrawRectangle(w-16,0,16,25);
		if(farr){dc.SetBrush(wxBrush(backgroundHover));}
		dc.DrawRectangle(0,0,16,25);

		if(rarr){
			dc.SetBrush(wxBrush(backgroundHover));
			dc.DrawRectangle(w-16,0,16,25);
		}

		dc.SetPen(wxPen(arrow,2));

		dc.DrawLine(17,0,17,25);
		dc.DrawLine(11,5,4,12);
		dc.DrawLine(4,12,11,19);

		dc.DrawLine(w-17,0,w-17,25);
		dc.DrawLine(w-11,5,w-4,12);
		dc.DrawLine(w-4,12,w-11,19);
	}

	//plus który jest zawsze widoczny

	dc.SetBrush(wxBrush(Options.GetColour((plus)? TabsBackgroundInactiveHover : TabsBackgroundInactive)));
	//if(plus){
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(start+1,1,18,22);
	//}

	//dc.SetPen(wxPen(inactiveText));
	dc.SetBrush(wxBrush(inactiveText));
	dc.DrawRectangle(start+4,11,12,2);
	dc.DrawRectangle(start+9,6,2,12);

	if(gc){
		gc->SetPen( wxPen(Options.GetColour(TabsBorderInactive)));
		gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
		wxGraphicsPath path = gc->CreatePath();
		path.MoveToPoint(start,0.0);
		path.AddLineToPoint(start,20.0);
		double strt=start;
		path.AddCurveToPoint(strt, 21.5, strt+1.5, 23.0, strt+3.0, 23.0);
		path.AddLineToPoint(strt+16.0,23.0);
		path.AddCurveToPoint(strt+17.5, 23.0, strt+19.0, 21.5, strt+19.0, 20.0);
		path.AddLineToPoint(strt+19,0);
		gc->StrokePath(path);
	}else{
		dc.SetPen( wxPen(Options.GetColour(TabsBorderInactive)));
		dc.DrawLine(start,0,start,21);
		dc.DrawLine(start,21,start+2,23);
		dc.DrawLine(start+2,23,start+17,23);
		dc.DrawLine(start+17,23,start+19,21);
		dc.DrawLine(start+19,21,start+19,0);
	}


	cdc.Blit(0,h-25,w,TabHeight,&dc,0,0);
	if(split){
		cdc.SetPen(*wxTRANSPARENT_PEN);
		cdc.SetBrush(Options.GetColour(TabsBarBackground1));
		cdc.DrawRectangle(splitline-2,0,4,h-25);
		cdc.SetPen(wxPen(Options.GetColour(WindowBackground)));
		bool aciter=(Pages[iter]->GetPosition().x==1);
		if(aciter){
			cdc.DrawLine(splitline+1,0,w,0);
			cdc.DrawLine(w-1,0,w-1,h-26);
			cdc.DrawLine(splitline+1,h-26,w,h-26);
			cdc.SetPen(wxPen("#FF0000"));
			cdc.DrawLine(0,0,0,h-26);
			cdc.DrawLine(0,0,splitline-1,0);
			cdc.DrawLine(splitline-1,0,splitline-1,h-26);
			cdc.DrawLine(0,h-26,splitline-1,h-26);
		}else{
			cdc.DrawLine(0,0,splitline-1,0);
			cdc.DrawLine(0,h-26,splitline-1,h-26);
			cdc.DrawLine(0,0,0,h-26);
			cdc.SetPen(wxPen("#FF0000"));
			cdc.DrawLine(splitline+1,0,w,0);
			cdc.DrawLine(splitline+1,0,splitline+1,h-26);
			cdc.DrawLine(w-1,0,w-1,h-26);
			cdc.DrawLine(splitline+1,h-26,w,h-26);
		}
	}

	if(gc){delete gc;}
}


void Notebook::OnTabSel(int id)
{
	int wtab=id - MENU_CHOOSE;
	if(Pages[iter]->Video->GetState()==Playing){Pages[iter]->Video->Pause();}
	//wxLogStatus("wtab %i", wtab);
	if(wtab<-1){
		wtab=abs(wtab+2);
		//wxLogStatus("wtab split %i", wtab);
		Split(wtab);
	}
	else if(wtab<0){
		kainoteFrame *Kai=(kainoteFrame*)GetParent();
		int tmpiter=iter;
		Pages[iter]->Hide();
		for(int i=(int)Pages.size()-1; i>=0; i--)
		{
			iter=i;
			//wxLogStatus("%i", (int)i);
			if(Kai->SavePrompt()){break;}
			Pages[i]->Destroy();
			Pages.pop_back();
			Names.pop_back();
			Tabsizes.pop_back();
		}
		//wxLogStatus("destroyed");
		iter=0;olditer=0;fvis=0;
		int w=-1,h=-1;
		if(Pages.size()<1){
			Pages.push_back(new TabPanel(this,(kainoteFrame*)GetParent()));
			wxString name=Pages[0]->SubsName;
			Names.Add(name);
			GetClientSize(&w,&h);
			Pages[0]->SetPosition(wxPoint(0,0));
			Pages[0]->SetSize(w,h-25);
		}
		CalcSizes();
		if(w<1){GetClientSize(&w,&h);}
		RefreshRect(wxRect(0,h-25,w,25),false);
		Pages[iter]->Show();

	}
	else{
		TabPanel *tmp=Page(fvis);
		tmp->Hide();
		Pages[fvis]=Pages[wtab];
		Pages[fvis]->Show();
		Pages[wtab]=tmp;
		wxString tmp1=Names[fvis];
		Names[fvis]=Names[wtab];
		Names[wtab]=tmp1;
		Tabsizes[iter]-=18;
		int tmp2=Tabsizes[fvis];
		Tabsizes[fvis]=Tabsizes[wtab];
		Tabsizes[wtab]=tmp2;
		Tabsizes[fvis]+=18;

		olditer=iter;
		iter=fvis;
		RefreshBar();
	}
}


int Notebook::GetHeight()
{
	return TabHeight;
}

void Notebook::OnResized()
{
	KillTimer(sthis->GetHWND(),9876);
	int w,h;
	sthis->GetClientSize(&w,&h);
	if(sthis->split){w=sthis->splitline-2;}
	for(size_t i=0;i<sthis->Size();i++){
		if(i==sthis->iter || (sthis->split && i == sthis->splititer))continue;
		sthis->Pages[i]->SetSize(w,h-sthis->TabHeight);
	}
}

void Notebook::Split(size_t page)
{
	split=!split;
	int w, h;
	GetClientSize(&w,&h);
	if(!split)
	{
		Pages[splititer]->Hide();
		Pages[iter]->SetSize(w,h-TabHeight);
		for(size_t k = 0; k < Size(); k++){
			Pages[k]->SetPosition(wxPoint(0,0));
		}
		SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
		return;
	}
	splitline=w/2;
	splititer=page;
	Pages[iter]->SetSize(splitline-3,h-TabHeight-2);
	Pages[iter]->SetPosition(wxPoint(1,1));
	Pages[splititer]->SetSize(w-(splitline+3),h-TabHeight-2);
	Pages[splititer]->SetPosition(wxPoint(splitline+2,1));
	Pages[splititer]->Show();
	//Pages[iter]->SetWindowStyleFlag(wxBORDER_SUNKEN);
	//Pages[iter]->Refresh();

	SetTimer(GetHWND(), 9876, 500, (TIMERPROC)OnResized);
}

int Notebook::FindTab(int x, int *_num)
{
	int num=(allvis)?2 : 20;
	*_num=num;
	int restab=-1;
	for(size_t i=fvis;i<Tabsizes.size();i++){
		if(x>num&&x<num+Tabsizes[i]){
			restab=i;
			*_num=num;
			break;
		}
		num+=Tabsizes[i]+2;
	}
	return restab;
}
void Notebook::ChangeActiv()
{
	int tmp=iter;
	iter=splititer;
	splititer=tmp;
	Tabsizes[iter]+=18;
	Tabsizes[splititer]-=18;
	((kainoteFrame*)GetParent())->UpdateToolbar();

	RefreshBar();
}

void Notebook::RefreshBar()
{
	int w,h;
	GetClientSize(&w,&h);
	RefreshRect(wxRect(0,h-25,w,25),false);
}

void Notebook::OnSave(int id)
{
	id-=MENU_SAVE;
	kainoteFrame *Kai=(kainoteFrame*) GetParent();
	if(id<0){
		Kai->SaveAll();
	}else{
		Kai->Save(false,id);
	}

}

Notebook *Notebook::sthis=NULL;

Notebook *Notebook::GetTabs()
{
	return sthis;
}

TabPanel *Notebook::GetTab()
{
	return sthis->Pages[sthis->iter];
}

int Notebook::FindPanel(TabPanel* pan)
{
	int i=0;
	for(std::vector<TabPanel*>::iterator it=Pages.begin(); it!=Pages.end(); it++)
	{
		if((*it)==pan){return i;}
		i++;
	}
	return iter;
}


LRESULT CALLBACK Notebook::PauseOnMinimalize( int code, WPARAM wParam, LPARAM lParam )
{
	/*if(code == HCBT_ACTIVATE){
	wxLogStatus("Czyżby aktywacja okna?");
	return 0;
	}
	if(wParam == SC_PREVWINDOW || wParam == SC_NEXTWINDOW){
	wxLogStatus("następne/poprzednie okno?");
	return 0;
	}*/
	if (wParam == SC_MINIMIZE){
		if(sthis->GetTab()->Video->vstate==Playing){sthis->GetTab()->Video->Pause();}
		return 0;
	}
	//wxLogStatus("jakiś event %i %i", code, (int)wParam);
	return CallNextHookEx( 0, code, wParam, lParam );
}



void Notebook::ChangePage(int i)
{

	olditer=iter;
	if(split && splititer==i){
		ChangeActiv();return;
	}
	if(Pages[olditer]->Video->GetState()==Playing){Pages[olditer]->Video->Pause();}
	if(split){
		Pages[i]->SetPosition(Pages[iter]->GetPosition());
		Pages[i]->SetSize(Pages[iter]->GetSize());
	}
	Pages[i]->Show();
	Tabsizes[i]+=18;
	Pages[iter]->Hide();
	Tabsizes[iter]-=18;
	iter=i;
	over=-1;
	RefreshBar();
	wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
	AddPendingEvent(evt2);
}

void Notebook::OnEraseBackground(wxEraseEvent &event)
{
}

void Notebook::OnCharHook(wxKeyEvent& event)
{
	int key=event.GetKeyCode();
	int ukey=event.GetUnicodeKey();
	bool nmodif= !(event.AltDown() || event.ControlDown() || event.ShiftDown());
	VideoCtrl *vb=GetTab()->Video;
	if(ukey==179){vb->Pause();}
	else if(ukey==178){wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,11015); vb->OnVButton(evt);}
	else if(ukey==177){vb->PrevChap();}
	else if(ukey==176){vb->NextChap();}
	else if(ukey==175){vb->OnSPlus();return;}
	else if(ukey==174){vb->OnSMinus();return;}
	else if(key==WXK_PAGEDOWN && nmodif && vb->HasFocus()){vb->OnPrew();return;}
	else if(key==WXK_PAGEUP && nmodif && vb->HasFocus()){vb->OnNext();return;}
	event.Skip();
}

TabPanel *Notebook::GetSecondPage()
{
	return Pages[splititer];
}

void Notebook::SubsComparsion()
{
	Grid *G1 = Pages[compareFirstTab]->Grid1;
	Grid *G2 = Pages[compareSecondTab]->Grid1;

	int firstSize= G1->GetCount(), secondSize= G2->GetCount();
	if(G1->Comparsion){G1->Comparsion->clear();}else{G1->Comparsion=new std::vector<wxArrayInt>;}
	if(G2->Comparsion){G2->Comparsion->clear();}else{G2->Comparsion=new std::vector<wxArrayInt>;}
	wxArrayInt emptyarray;
	G1->Comparsion->resize(firstSize, emptyarray);
	G2->Comparsion->resize(secondSize, emptyarray);

	int lastJ=0;

	for(int i=0; i<firstSize; i++){

		int j=lastJ;
		Dialogue *dial1=G1->GetDial(i);
		while(j<secondSize){

			Dialogue *dial2=G2->GetDial(j);
			if(dial1->Start == dial2->Start && dial1->End == dial2->End){
				CompareTexts((G1->transl && dial1->TextTl != "")? dial1->TextTl : dial1->Text, (G2->transl && dial2->TextTl != "")? dial2->TextTl : dial2->Text, G1->Comparsion->at(i), G2->Comparsion->at(j));
				lastJ=j+1;
				break;
			}

			j++;
		}

	}

	G1->Refresh(false);
	G2->Refresh(false);
}


void Notebook::CompareTexts(wxString &first, wxString &second, wxArrayInt &firstCompare, wxArrayInt &secondCompare)
{

	if(first==second){
		return;
	}
	firstCompare.push_back(1);
	secondCompare.push_back(1);


	size_t l1 = first.Len(), l2 = second.Len();
	size_t sz = (l1 + 1) * (l2 + 1) * sizeof(size_t);
	size_t w = l2 + 1;
	size_t* dpt;
	size_t i1, i2;
	dpt = new size_t[sz];

	if (//sz / (l1 + 1) / (l2 + 1) != sizeof(size_t) ||
		//(
			dpt == NULL)
	{
		wxLogStatus("memory allocation failed");
		return ;
	}

	/*for (i1 = 0; i1 <= l1; i1++)
	dpt[w * i1 + 0] = 0;
	for (i2 = 0; i2 <= l2; i2++)
	dpt[w * 0 + i2] = 0;*/
	memset(dpt, 0, sz);

	for (i1 = 1; i1 <= l1; i1++){
		for (i2 = 1; i2 <= l2; i2++)
		{
			if (first[l1 - i1] == second[l2 - i2])
			{
				dpt[w * i1 + i2] = dpt[w * (i1 - 1) + (i2 - 1)] + 1;
			}
			else if (dpt[w * (i1 - 1) + i2] > dpt[w * i1 + (i2 - 1)])
			{
				dpt[w * i1 + i2] = dpt[w * (i1 - 1) + i2];
			}
			else
			{
				dpt[w * i1 + i2] = dpt[w * i1 + (i2 - 1)];
			}
		}
	}

	int sfirst=-1, ssecond=-1;
	i1 = l1; i2 = l2;
	for (;;){
		if ((i1 > 0) && (i2 > 0) && (first[l1 - i1] == second[l2 - i2])){
			if(sfirst>=0){
				firstCompare.push_back(sfirst);
				firstCompare.push_back((l1 - i1) -1);
				sfirst=-1;
			}
			if(ssecond>=0){
				secondCompare.push_back(ssecond);
				secondCompare.push_back((l2 - i2) -1);
				ssecond=-1;
			}
			i1--; i2--; continue;
		}
		else{
			if (i1 > 0 && (i2 == 0 || dpt[w * (i1 - 1) + i2] >= dpt[w * i1 + (i2 - 1)])){
				if(sfirst==-1){sfirst = l1 - i1;}
				i1--; continue;
			}
			else if (i2 > 0 && (i1 == 0 || dpt[w * (i1 - 1) + i2] < dpt[w * i1 + (i2 - 1)])){
				if(ssecond==-1){ssecond = l2 - i2;}
				i2--; continue;
			}
		}

		break;
	}
	if(sfirst>=0){
		firstCompare.push_back(sfirst);
		firstCompare.push_back((l1 - i1) -1);
	}
	if(ssecond>=0){
		secondCompare.push_back(ssecond);
		secondCompare.push_back((l2 - i2) -1);
	}

	//free(dpt);
	delete dpt;
}


BEGIN_EVENT_TABLE(Notebook,wxWindow)
	EVT_CHAR_HOOK(Notebook::OnCharHook)
	EVT_ERASE_BACKGROUND(Notebook::OnEraseBackground)
	EVT_MOUSE_EVENTS(Notebook::OnMouseEvent)
	EVT_SIZE(Notebook::OnSize)
	EVT_PAINT(Notebook::OnPaint)
	EVT_MOUSE_CAPTURE_LOST(Notebook::OnLostCapture)
END_EVENT_TABLE()
