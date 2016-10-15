
//#include "Toolbar.h"
#include "Config.h"
#include "Hotkeys.h"
#include "KainoteApp.h"
#include "wx/utils.h"

KaiToolbar::KaiToolbar(wxWindow *Parent, MenuBar *mainm, int id, bool _orient)
	:wxWindow(Parent,-1,wxDefaultPosition,wxSize(iconsize,-1))
	,bmp(NULL)
	,vertical(_orient)
	,Clicked(false)
	,wasmoved(false)
	,wh(iconsize)
	,oldelem(-1)
	,sel(-1)
	,mb(mainm)
{
	
}

KaiToolbar::~KaiToolbar()
{
	wxArrayString names;
	wxDELETE(bmp);
	for(auto i = tools.begin(); i!=tools.end(); i++)
	{
		if((*i)->id!=34566){names.Add(GetString((Id)(*i)->id));}
		delete (*i);
	}
	tools.clear();
	Options.SetTable("Toolbar IDs",names);
	ids.clear();
}

void KaiToolbar::InitToolbar()
{
	wxArrayInt IDS;
	wxString idnames=Options.GetString("Toolbar IDs");

	if(idnames!=""){
		wxStringTokenizer cfgtable(idnames,"|",wxTOKEN_STRTOK);
		while(cfgtable.HasMoreTokens()){
			int val=GetIdValue(cfgtable.NextToken().data());
			if(val>0){
				IDS.Add(val);
			}
		}  
	}
	if(IDS.size()<1){
		IDS.Add(OpenSubs);IDS.Add(RecentSubs);IDS.Add(OpenVideo);IDS.Add(RecentVideo);
		IDS.Add(SaveSubs);IDS.Add(SaveSubsAs);IDS.Add(SaveAllSubs);
		IDS.Add(RemoveSubs);IDS.Add(Editor);IDS.Add(FindReplace);
		IDS.Add(StyleManager);IDS.Add(ASSProperties);IDS.Add(ChangeTime);
		IDS.Add(ConvertToASS);IDS.Add(ConvertToSRT);IDS.Add(Settings);
	}
	for(size_t i=0; i<IDS.size();i++)
	{
		MenuItem *item=mb->FindItem(IDS[i]);
		if(!item){wxLogStatus(_("Nie można znaleźć elementu o id %i"), IDS[i]);continue;}
		wxString desc=item->GetLabelText();
		AddItem(IDS[i],desc,item->icon,item->IsEnabled(),(item->GetSubMenu()!=NULL)? 1 : 0);
	}
	tools.push_back(new toolitem(2,16,34566,true));
	
	Refresh(false);
}

	
void KaiToolbar::AddItem(int id, const wxString &label, wxBitmap *normal,bool enable, byte type)
{
	if(tools.size()>0 && tools[tools.size()-1]->GetType()==2){
		tools.insert(tools.begin()+tools.size()-2, new toolitem(normal,label,id,enable,type));
		return;
	}
	tools.push_back(new toolitem(normal,label,id,enable,type));
}
	
void KaiToolbar::InsertItem(int id, int index, const wxString &label,wxBitmap *normal,bool enable, byte type)
{
	tools.insert(tools.begin()+index,new toolitem(normal,label,id,enable,type));
}

void KaiToolbar::AddSpacer()
{
	tools.push_back(new toolitem(3,12));	
}
void KaiToolbar::InsertSpacer(int index)
{
	tools.insert(tools.begin()+index, new toolitem(3,12));	
}

void KaiToolbar::UpdateId(int id, bool enable)
{
	for(auto i=tools.begin(); i!=tools.end(); i++)
	{
		if((*i)->id==id)
		{
			if((*i)->Enable(enable))
			{
				Refresh(false);
			}
		}
	}
	
}

void KaiToolbar::AddID(int id)
{
	ids.Add(id);
}

void KaiToolbar::OnMouseEvent(wxMouseEvent &event)
{
	bool leftdown = event.LeftDown();
	
	wxPoint elems= FindElem(event.GetPosition());
	int elem=elems.x;
	
	if(elem<0||event.Leaving()){/*if(HasCapture()){ReleaseMouse();}*/if(HasToolTips()){UnsetToolTip();}
	int tmpsel= sel; 
	sel=-1;
	oldelem=-1;
	Clicked=false;
	if(tmpsel!=sel){Refresh(false);}
	return;
	}
	if(elem==tools.size()-1){
		SetToolTip(_("Wybierz ikony paska narzędzi"));
		sel=elem;oldelem=elem;
		Refresh(false);
	}
	else if(sel!=elem && tools[elem]->type<2 && !event.LeftIsDown()){
		wxString shkeyadd;
		wxString shkey=Hkeys.GetMenuH(tools[elem]->id);
		if (shkey!=""){shkeyadd<<" ("<<shkey<<")";}
		SetToolTip(tools[elem]->label+shkeyadd);
		sel=elem;
		//RefreshRect(wxRect(0,elem*iconsize,iconsize,(elem+1)*iconsize),false);
		Refresh(false);
	}
	if(!tools[elem]->enabled){int tmpsel= sel; sel=-1;if(tmpsel!=sel){Refresh(false);}}
	if((leftdown || (event.Entering() && event.LeftIsDown()))){// && tools[elem]->type<3
		Clicked=true;Refresh(false);oldelem=elem;
	}else if(event.LeftIsDown() && oldelem!=elem && oldelem>=0){
		if(elem==tools.size()-1 || oldelem==tools.size()-1){oldelem=elem;return;}
		toolitem *tmpitem=tools[oldelem];
		tools[oldelem]=tools[elem];
		tools[elem]=tmpitem;
		Refresh(false);
		oldelem=elem;
		wasmoved=true;
	}else if(event.LeftUp()){
		if(!wasmoved && tools[elem]->enabled){
			if(tools[elem]->type==1){
				MenuItem *item=mb->FindItem(tools[elem]->id);
				Menu * smenu=item->GetSubMenu();
				//Menu * shmenu;
				if(tools[elem]->id!=SortLines && tools[elem]->id!=SortSelected){
					kainoteFrame *Kai = (kainoteFrame*) GetParent();
					int what= (smenu==Kai->SubsRecMenu)? 0 : (smenu==Kai->VidsRecMenu)? 1 : 2;
					Kai->AppendRecent(what, smenu);
				}
				/*for(int i=0; i<(int)smenu->GetMenuItemCount(); i++)
				{
					MenuItem *itm=smenu->FindItemByPosition(i);
					shmenu.Append(itm->GetId(),itm->GetLabel(),itm->GetHelp())->Enable(itm->IsEnabled());
				}*/
			
				smenu->PopupMenu(event.GetPosition(), this);
			}else{
				wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,tools[elem]->id);
				ProcessEvent(evt);
			}
		}
		Clicked=false;wasmoved=false;Refresh(false);
	}

}
	

void KaiToolbar::OnPaint(wxPaintEvent &event)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR)));
	tdc.DrawRectangle(0,0,w,h);
	int pos=4;
	int pos1=0;
	int maxx=(vertical)?h : w;
	
	for(int i=0; i<(int)tools.size(); i++)
	{
		if(pos+tools[i]->size>maxx){pos1+=iconsize;pos=4;}
		if(i==sel){
			tdc.SetPen(wxPen((Clicked)?"#000000" : "#80BFFF"));
			tdc.SetBrush(wxBrush((Clicked)?"#80BFFF" : "#C3E8F5"));
			tdc.DrawRoundedRectangle((vertical)?pos1+2 :pos-2, (vertical)?pos-2 : pos1+2,iconsize-4,tools[i]->size-4,1.1);
		}
		if(tools[i]->type<2){
			//wxImage img=tools[i]->GetBitmap().ConvertToImage();
			//img=img.Rescale(20,20,wxIMAGE_QUALITY_HIGH);

			tdc.DrawBitmap(tools[i]->GetBitmap(),(vertical)?pos1+4 : pos,(vertical)?pos : pos1+4);}
		else if(tools[i]->type==3){tdc.SetPen(wxPen("#606060")); 
			tdc.DrawLine((vertical)?pos1+2 : pos+2, (vertical)?pos+2 : pos1+2, (vertical)? iconsize-2 : pos+2, (vertical)?pos+2 : iconsize-2);
		}
		pos+=tools[i]->size;
	}
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)));
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)));
	wxPoint points[3];
	if(vertical){tdc.DrawLine(pos1+14,pos-15,pos1+20,pos-15);
		points[0]=wxPoint(pos1+14,pos-12);
		points[1]=wxPoint(pos1+20,pos-12);
		points[2]=wxPoint(pos1+17,pos-8);
	}
	else{
		tdc.DrawLine(pos-6,pos1+14,pos,pos1+14);
		points[0]=wxPoint(pos-6,pos1+17);
		points[1]=wxPoint(pos,pos1+17);
		points[2]=wxPoint(pos-3,pos1+21);
	}
	
	
	
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.DrawPolygon(3,points);
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}
void KaiToolbar::OnSize(wxSizeEvent &evt)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	
	float maxx=(vertical)? h : w;
	int toolbarrows=((tools.size()*iconsize)-8)/maxx;

	wh=(toolbarrows+1)*iconsize;
	int maxxwh=(vertical)? w : h;
	if(maxxwh!=wh){
		SetMinSize(wxSize(wh,-1));
		kainoteFrame *Kai= (kainoteFrame*)GetParent();
		Kai->Layout();
	}
	
	Refresh(false);
}

wxPoint KaiToolbar::FindElem(wxPoint pos)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	wxPoint res(-1,0);
	int maxx=(vertical)? h : w;
	int tmppos=0;
	int curpos=(vertical)? pos.y : pos.x;
	int curpos1=(vertical)? pos.x : pos.y;
	curpos1/=iconsize;
	//int wrap=0;
	for(size_t i=0; i<tools.size(); i++)
	{
		if(tmppos+tools[i]->size>maxx){res.y++;tmppos=0;}
		if(curpos>tmppos && curpos<=tmppos+tools[i]->size &&res.y==curpos1)
		{
			res.x=i;
			return res;
		}
		tmppos += tools[i]->size;
		
	}
	return res;
}

bool KaiToolbar::Updatetoolbar()
{
	bool changes=false;
	for(int i=0; i<(int)tools.size()-1; i++)
	{
		if(tools[i]->id<1){continue;}
		changes |= tools[i]->Enable(mb->FindItem(tools[i]->id)->IsEnabled());
	}
    
	
	if(changes){Refresh(false);}
	return false;
}

void KaiToolbar::OnToolbarOpts(wxCommandEvent &event)
{
	wxPoint point=GetPosition();
	point=ClientToScreen(point);
	point.y+=20;
	point.x+=GetSize().x;
	ToolbarMenu *tbr=new ToolbarMenu(this,point);
	tbr->Show();
	//wxLogStatus("showed");
}

BEGIN_EVENT_TABLE(KaiToolbar, wxWindow)
	EVT_MOUSE_EVENTS(KaiToolbar::OnMouseEvent)
	EVT_PAINT(KaiToolbar::OnPaint)
	EVT_SIZE(KaiToolbar::OnSize)
	EVT_MENU(34566,KaiToolbar::OnToolbarOpts)
END_EVENT_TABLE()

ToolbarMenu::ToolbarMenu(KaiToolbar*_parent, const wxPoint &pos)
	:wxDialog(_parent,-1,"",pos, wxSize(350,500), 0)
	,sel(-1)
	,scPos(0)
	,parent(_parent)
	,bmp(NULL)
{
	fh=20;
	int ysize = parent->ids.size();
	SetScrollbar(wxVERTICAL,0,25,ysize);
	//SetSize(350,ysize);
	//wxLogStatus("constructor %i",ysize);
	//CaptureMouse();
}

void ToolbarMenu::OnMouseEvent(wxMouseEvent &evt)
{
	bool leftdown=evt.LeftDown();
	int x=evt.GetX();
	int y=evt.GetY();
	int w=0;
	int h=0;
	GetSize (&w, &h);
	//if((x<0||y<0||x>w||y>h)&&leftdown){if(HasCapture()){ReleaseMouse();}Destroy();return;}
	//if(x>w-20){}

	int elem = y/fh;
	elem+=scPos;
	if(elem>=(int)parent->ids.size()){return;}
	if(elem!=sel){
		sel=elem;
		Refresh(false);
	}
	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -=step;
		Refresh(false);
		return;
	}
	if(leftdown){
		int result=-1;
		for(size_t j=0; j<parent->tools.size();j++)
		{
			if(parent->tools[j]->id==parent->ids[elem]){result=j; break;}
		}
		if(result==-1){
			MenuItem *item=parent->mb->FindItem(parent->ids[elem]);
			wxString desc=item->GetLabel();
			desc.Replace("&","");
			size_t reps=desc.Replace("\t"," (");
			if (reps){desc.Append(")");}
			parent->AddItem(parent->ids[elem], desc, item->icon,item->IsEnabled(), (item->GetSubMenu()!=NULL)? 1 : 0);
		}else{
			parent->tools.erase(parent->tools.begin()+result);

		}
		parent->Refresh(false);
		Refresh(false);
	}
}

void ToolbarMenu::OnPaint(wxPaintEvent &event)
{
	//wxLogStatus("paint");
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	wxBitmap checkbmp = wxBITMAP_PNG("check");
	tdc.SetFont(wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma"));
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.DrawRectangle(0,0,w,h);
	int visible=25;//500/fh fh=20;
	int idssize=parent->ids.size();
	if(scPos>=idssize-visible){scPos=idssize-visible;}
	else if(scPos<0){scPos=0;}
	int maxsize=MAX(idssize,scPos+visible);
	SetScrollbar(wxVERTICAL,scPos,visible,idssize);
	for(int i=0;i<visible; i++)
	{
		MenuItem *item=parent->mb->FindItem(parent->ids[i+scPos]);
		bool check=false;
		for(size_t j=0; j<parent->tools.size();j++)
		{
			if(parent->tools[j]->id==parent->ids[i+scPos]){check=true; break;}
		}
		if(i+scPos==sel){
			tdc.SetPen(wxPen("#000000"));
			tdc.SetBrush(wxBrush("#9BD7EE"));
			tdc.DrawRectangle(0, fh*i,w,fh);
		}
		tdc.SetPen(wxPen("#497CB0",2));
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		
		
		if(check){
			tdc.DrawBitmap(checkbmp,4,(fh*i)+2);
		}

		tdc.DrawBitmap(item->GetBitmap(),fh+8,(fh*i)+2);
		wxString desc=item->GetLabel();
		desc.Replace("&","");
		size_t reps=desc.Replace("\t"," (");
		wxString accel;
		wxString label=desc;
		tdc.SetPen(wxPen("#497CB0"));
		if (reps){
			desc.Append(")");
			label=desc.BeforeFirst('(',&accel);
			accel.Prepend("(");
			int fw, fhh;
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel,w-fw-8,(fh*i)+2);
		}
		tdc.DrawText(label,(fh*2)+15,(fh*i)+2);
		

	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void ToolbarMenu::OnLostCapture(wxFocusEvent &evt)
{
	Destroy();
}

void ToolbarMenu::OnScroll(wxScrollWinEvent& event)
{
	int newPos=0;
	int tsize=parent->tools.size();
	if(event.GetEventType()==wxEVT_SCROLLWIN_LINEUP)
	{
		newPos=scPos-1;
		if(newPos<0){newPos=0;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos=scPos+1;
		//if(newPos>=tsize){newPos=tsize-1;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos-=(size.y/fh - 1);
		newPos=MAX(0,newPos);
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos+=(size.y/fh - 1);
		newPos=MIN(newPos,tsize-1);
	}
	else{newPos = event.GetPosition();}
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}
BEGIN_EVENT_TABLE(ToolbarMenu, wxDialog)
	EVT_MOUSE_EVENTS(ToolbarMenu::OnMouseEvent)
	EVT_PAINT(ToolbarMenu::OnPaint)
	EVT_KILL_FOCUS(ToolbarMenu::OnLostCapture)
	EVT_SCROLLWIN(ToolbarMenu::OnScroll)
END_EVENT_TABLE()