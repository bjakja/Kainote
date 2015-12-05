#include <wx/graphics.h>
#include "TabDialog.h"
#include "TabPanel.h"
#include "kainoteApp.h"

HHOOK Notebook::g_SSHook = 0;

Notebook::Notebook(wxWindow *parent, int id)
	: wxWindow(parent,id)
{
	fvis=olditer=iter=0;
	splitline=splititer=0;
	oldtab=over=-1;
	block=split=onx=farr=rarr=plus=false;
	TabHeight=25;
	allvis=arrow=true;
	font=wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
	sthis=this;
	
	Pages.push_back(new TabPanel(this,(kainoteFrame*)parent));

	wxString name=Pages[0]->SubsName;

	if(name.Len()>35){name=name.SubString(0,35)+"...";}
	Names.Add(name);

	CalcSizes();
	
	g_SSHook = SetWindowsHookEx(WH_CBT, &BlockSSaver, NULL,GetCurrentThreadId());

	SetThreadExecutionState(ES_DISPLAY_REQUIRED|ES_CONTINUOUS);

}


Notebook::~Notebook(){
	for(std::vector<TabPanel*>::iterator i=Pages.begin(); i!=Pages.end(); i++)
	{
		(*i)->Destroy();
	}
	Pages.clear();
	Tabsizes.Clear();
	Names.Clear();
	UnhookWindowsHookEx( g_SSHook );
	SetThreadExecutionState(ES_CONTINUOUS);
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
	if(refresh){RefreshRect(wxRect(0,h-25,w,25),false);
		if(!Options.GetBool("Show Editor")){kainoteFrame *kai=(kainoteFrame *)GetParent();kai->HideEditor();}
		wxCommandEvent evt2(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		AddPendingEvent(evt2);
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
	
	kainoteFrame *Kai=(kainoteFrame*)GetParent();
	//Kai->ss->IsShown();
	//wxLogStatus("iss1");
	block=true;
	if(Kai->SavePrompt(1,page)){block=false; wxSize siz=GetClientSize();RefreshRect(wxRect(0,siz.y-25,siz.x,25),false); return;}
	block=false;
	//Kai->ss->IsShown();
	//wxLogStatus("iss2");
	Pages[page]->Destroy();
	Pages.erase(Pages.begin()+page);
	Names.RemoveAt(page);
	Tabsizes.RemoveAt(page);

	//Kai->ss->IsShown();
	//wxLogStatus("iss3");

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

	
	int w,h;
	GetClientSize(&w,&h);
	RefreshRect(wxRect(0,h-25,w,25),false);
	
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
	
	//wy³¹czanie wszystkich aktywnoœci przy wyjœciu z zak³adek
	
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
			if(sline){int yy; sline->GetPosition(&npos,&yy);ScreenToClient(&npos,&yy);
			sline->Destroy();}
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

	
	// klik, dwuklik i œrodkowy
	if(click||dclick||mdown){

		
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
	
	
	
	//o¿ywienie zak³adek
	if(event.Moving()){
		if(x>=start+17&&HasToolTips()){UnsetToolTip();}

		
		if(!allvis && x<20){if(farr) return;
			farr=true;
			RefreshRect(wxRect(0,hh,20,25),false);return;
			}
		else if(!allvis && x>w-17 && x<=w){if(rarr) return;
			rarr=true;plus=false;
			RefreshRect(wxRect(w-17,hh,17,25),false);return;
			}
		else if(x>start && x<start+17){if(plus) return;
			plus=true;rarr=false;
			RefreshRect(wxRect(start,hh,start+17,25),false);
			//if(oldtab!=i){
				SetToolTip("Otwórz now¹ zak³adkê");
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
		
		if(i!=iter&&i!=over){
			over=i;
			RefreshRect(wxRect(0,hh,w,25),false);
			
		}else if(i==iter&&over!=-1){
			over=-1;
			RefreshRect(wxRect(0,hh,w,25),false);
		}else if(i==iter){
		}
		if(i==iter && (x>num+Tabsizes[i]-18 && x<num+Tabsizes[i]-5)){
			if(!onx){SetToolTip("Zamknij");}
			onx=true;
			RefreshRect(wxRect(num+Tabsizes[i]-18,hh,15,25),false);
		}else if(onx){
			oldtab=-1;//trick aby po zejœciu z x powróci³ tip z nazw¹ napisów i wideo
			onx=false;
			RefreshRect(wxRect(num+Tabsizes[i]-18,hh,15,25),false);
		}
		if(i!=-1 && i!= oldtab){SetToolTip(Pages[i]->SubsName+"\n"+Pages[i]->VideoName); oldtab=i;}
	}
		
	//menu kontekstowe		
	if(event.RightUp()){
		
		wxMenu menu1;//=new wxMenu();
	
		for(size_t g=0;g<Size();g++)
		{
			menu1.Append(MENU_CHOOSE+g,Page(g)->SubsName,"",wxITEM_CHECK);
		}
		menu1.Check(MENU_CHOOSE+iter,true);
		menu1.AppendSeparator();
		if(i>=0){menu1.Append(MENU_SAVE+i,"Zapisz","Zapisz",wxITEM_NORMAL);}
		menu1.Append(MENU_SAVE-1,"Zapisz wszystko","Zapisz wszystko",wxITEM_NORMAL);
		menu1.Append(MENU_CHOOSE-1,"Zamknij wszystkie zak³adki","Zamknij wszystkie zak³adki",wxITEM_NORMAL);
		if((i!=iter && Size()>1 && i!=-1)||split){
			wxString txt=(split)? "Wyœwietl jedn¹ zak³adkê" : "Wyœwietl dwie zak³adki";
			menu1.Append((MENU_CHOOSE-2)-i, txt, "",wxITEM_NORMAL);
		}
		int id=GetPopupMenuSelectionFromUser(menu1,event.GetPosition());
		if(id >= MENU_CHOOSE-101 && id <= MENU_CHOOSE+99){
			OnTabSel(id);
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
	if(block){return;}
	//wxLogStatus("paint iter %i splititer %i", (int)iter, splititer);
	int w,h;
	GetClientSize(&w,&h);
	//h-=TabHeight;
	wxClientDC cdc(this);
	wxMemoryDC dc;
	dc.SelectObject(wxBitmap(w,TabHeight));
    dc.SetFont(font);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
	dc.GradientFillLinear(wxRect(0,0,w,TabHeight),
		wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
		wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE),wxTOP);
	
	start=(allvis)?2 : 20;
	

	wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
	//pêtla do rysowania zak³adek
	for(size_t i=fvis;i<Tabsizes.size();i++){
		//wybrana zak³adka
		if(i==iter){//wxSYS_COLOUR_INACTIVEBORDER
			
			dc.SetPen(wxPen(wxColour("#000000"),3));
			dc.DrawLine(0,0,start-1,0);
			dc.DrawLine(start+Tabsizes[i]+1,0,w,0);
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
			dc.DrawRectangle(start+1,0,Tabsizes[i]-1,23);
	
			dc.SetTextForeground("#505050");
			dc.SetPen(wxPen("#000000"));

			//najechany x na wybranej zak³adce
			if(onx){
				dc.SetBrush(wxBrush("#9BD7EE"));
				dc.DrawRoundedRectangle(start+Tabsizes[i]-20,3,16,16,1.1);}
			dc.DrawText("X",start+Tabsizes[i]-16,3);
			dc.SetTextForeground("#000000");
			
		}
		//najechana nieaktywna zak³adka
		if(i==(size_t)over){
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxBrush("#FFFFFF"));
			dc.DrawRectangle(start+1,2,Tabsizes[i]-1,21);
		}
		
		//rysowanie konturów zak³adki
		if(gc){
			gc->SetPen( wxPen("#000000"));
			gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
			wxGraphicsPath path = gc->CreatePath();
			path.MoveToPoint(start,0.0);
			path.AddLineToPoint(start,21.0);
			double strt=start;
			path.AddCurveToPoint(strt+0.5, 21.5, strt+1.5, 22.5, strt+2, 23.0);
			strt+=Tabsizes[i];
			path.AddLineToPoint(strt-2.0,23.0);
			path.AddCurveToPoint(strt-1.5, 22.5, strt-0.5, 21.5, strt, 21.0);
			path.AddLineToPoint(strt,0);
			gc->StrokePath(path);
		}
		else{
			dc.SetPen(wxPen("#000000"));
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
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
	//strza³ki do przesuwania zak³adek
	if(!allvis){
		dc.DrawRectangle(w-16,0,16,25);
		if(farr){dc.SetBrush(wxBrush("#9BD7EE"));}
		dc.DrawRectangle(0,0,16,25);

		if(rarr){
			dc.SetBrush(wxBrush("#9BD7EE"));
			dc.DrawRectangle(w-16,0,16,25);
		}

		dc.SetPen(wxPen(wxColour("#000000"),2));

		dc.DrawLine(17,0,17,25);
		dc.DrawLine(11,5,4,12);
		dc.DrawLine(4,12,11,19);

		dc.DrawLine(w-17,0,w-17,25);
		dc.DrawLine(w-11,5,w-4,12);
		dc.DrawLine(w-4,12,w-11,19);
	}
	
	//plus który jest zawsze widoczny
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush("#FFFFFF"));
	if(plus){dc.DrawRectangle(start+1,0,18,23);}

	dc.SetPen(wxPen(wxColour("#000000")));
		//dc.SetBrush(wxBrush("#FFFFFF"));
	dc.DrawRectangle(start+4,11,12,2);
	dc.DrawRectangle(start+9,6,2,12);
		//dc.SetPen(wxPen("#000000"));

	/*if(split){
		dc.SetPen(wxPen("#FF0000",2));
		wxSize siz= Pages[iter]->GetSize();
		wxPoint pos= Pages[iter]->GetPosition();
		dc.DrawLine(pos.x,1,pos.x+siz.x,1);
	}*/
	if(gc){
		gc->SetPen( wxPen("#000000"));
		gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
		wxGraphicsPath path = gc->CreatePath();
		path.MoveToPoint(start,0.0);
		path.AddLineToPoint(start,21.0);
		double strt=start;
		path.AddCurveToPoint(strt+0.5, 21.5, strt+1.5, 22.5, strt+2, 23.0);
		path.AddLineToPoint(strt+17.0,23.0);
		path.AddCurveToPoint(strt+17.5, 22.5, strt+18.5, 21.5, strt+19, 21.0);
		path.AddLineToPoint(strt+19,0);
		gc->StrokePath(path);
	}else{
		dc.DrawLine(start,0,start,21);
		dc.DrawLine(start,21,start+2,23);
		dc.DrawLine(start+2,23,start+17,23);
		dc.DrawLine(start+17,23,start+19,21);
		dc.DrawLine(start+19,21,start+19,0);
	}
	

	cdc.Blit(0,h-25,w,h,&dc,0,0);
	if(split){
		cdc.SetPen(*wxTRANSPARENT_PEN);
		cdc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
		cdc.DrawRectangle(splitline-2,0,4,h-25);
		cdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
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
		if(i==sthis->iter||(sthis->split&&i==sthis->splititer))continue;
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
		Pages[iter]->SetPosition(wxPoint(0,0));
		Pages[iter]->SetSize(w,h-TabHeight);
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
	//Pages[iter]->SetWindowStyleFlag(wxBORDER_SUNKEN);
	//Pages[splititer]->SetWindowStyleFlag(0);
	
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


LRESULT CALLBACK Notebook::BlockSSaver( int code, WPARAM wParam, LPARAM lParam )
{
	if (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER){
		if(sthis->GetTab()->Video->vstate==Playing){return 1;}
		return 0;
	}
	if (wParam == SC_MINIMIZE){
		if(sthis->GetTab()->Video->vstate==Playing){sthis->GetTab()->Video->Pause();}
		return 0;
	}
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
	//wxLogStatus("key %i, unicode %i", event.GetKeyCode(),event.GetUnicodeKey());
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

BEGIN_EVENT_TABLE(Notebook,wxWindow)
	//EVT_MENU_RANGE(MENU_CHOOSE,MENU_CHOOSE+99, Notebook::OnTabSel)
	//EVT_MENU(MENU_CHOOSE-1, Notebook::OnTabSel)
	//EVT_MENU_RANGE(MENU_CHOOSE-101,MENU_CHOOSE-2, Notebook::OnTabSel)
	//EVT_TIMER(33333,Notebook::OnResized)
	EVT_CHAR_HOOK(Notebook::OnCharHook)
	EVT_ERASE_BACKGROUND(Notebook::OnEraseBackground)
	EVT_MOUSE_EVENTS(Notebook::OnMouseEvent)
	EVT_SIZE(Notebook::OnSize)
	EVT_PAINT(Notebook::OnPaint)
END_EVENT_TABLE()
