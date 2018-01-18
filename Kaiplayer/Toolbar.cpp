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


//#include "Toolbar.h"
#include "Config.h"
#include "Hotkeys.h"
#include "KainoteApp.h"
#include "wx/utils.h"

KaiToolbar::KaiToolbar(wxWindow *Parent, MenuBar *mainm, int id)
	:wxWindow(Parent,-1,wxDefaultPosition,wxSize(iconsize,-1))
	,bmp(NULL)
	,Clicked(false)
	,wasmoved(false)
	,wh(iconsize)
	,oldelem(-1)
	,sel(-1)
	,mb(mainm)
{
	alignment = Options.GetInt(ToolbarAlignment);
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
	Options.SetTable(ToolbarIDs,names);
	ids.clear();
}

void KaiToolbar::InitToolbar()
{
	wxArrayInt IDS;
	const wxString & idnames= Options.GetString(ToolbarIDs);

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
		IDS.Add(RemoveSubs);IDS.Add(Editor);IDS.Add(FindReplaceDialog);
		IDS.Add(StyleManager); IDS.Add(ASSProperties); IDS.Add(ChangeTime); 
		IDS.Add(ConvertToASS); IDS.Add(ConvertToSRT); IDS.Add(VideoZoom); 
		IDS.Add(SubsResample); IDS.Add(Settings);
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
	bool vertical = alignment % 2 == 0;
	const wxColour & background = Options.GetColour(WindowBackground);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);
	int pos=4;
	int pos1=0;
	int maxx=(vertical)?h : w;
	
	for(int i=0; i<(int)tools.size(); i++)
	{
		if(pos+tools[i]->size>maxx){pos1+=iconsize;pos=4;}
		if(i==sel){
			tdc.SetPen(wxPen((Clicked)?Options.GetColour(ButtonBorderPushed) : Options.GetColour(ButtonBorderHover)));
			tdc.SetBrush(wxBrush((Clicked)?Options.GetColour(ButtonBackgroundPushed) : Options.GetColour(ButtonBackgroundHover)));
			tdc.DrawRoundedRectangle((vertical) ? pos1 + 2 : pos - 2, (vertical) ? pos - 2 : (i >= (int)tools.size() - 1) ? pos1 + 2 + (iconsize - (tools[i]->size)) : pos1 + 2, iconsize - 4, tools[i]->size - 4, 1.1);
		}
		if(tools[i]->type<2){
			//wxImage img=tools[i]->GetBitmap().ConvertToImage();
			//img=img.Rescale(20,20,wxIMAGE_QUALITY_HIGH);

			tdc.DrawBitmap(tools[i]->GetBitmap(),(vertical)?pos1+4 : pos,(vertical)?pos : pos1+4);
		}
		/*else if(tools[i]->type==3){
			tdc.SetPen(wxPen("#606060")); 
			tdc.DrawLine((vertical)?pos1+2 : pos+2, (vertical)?pos+2 : pos1+2, (vertical)? iconsize-2 : pos+2, (vertical)?pos+2 : iconsize-2);*/
		//}
		pos+=tools[i]->size;
	}
	tdc.SetPen(wxPen(Options.GetColour(WindowText)));
	tdc.SetBrush(wxBrush(Options.GetColour(WindowText)));
	wxPoint points[3];
	if(vertical){
		tdc.DrawLine(pos1+14,pos-15,pos1+20,pos-15);
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
	bool vertical = alignment % 2 == 0;
	float maxx=(vertical)? h : w;
	int toolbarrows=((tools.size()*iconsize)-2)/maxx;

	wh=(toolbarrows+1)*iconsize;
	int maxxwh=(vertical)? w : h;
	if(maxxwh!=wh){
		thickness = wh;
		SetSize(wxSize(wh,-1));
		kainoteFrame *Kai= (kainoteFrame*)GetParent();
		//Kai->Layout();
		wxSizeEvent evt;
		Kai->OnSize(evt);
	}
	
	Refresh(false);
}

wxPoint KaiToolbar::FindElem(wxPoint pos)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	wxPoint res(-1,0);
	bool vertical = alignment % 2 == 0;
	int maxx=(vertical)? h : w;
	int tmppos=0;
	int curpos=(vertical)? pos.y : pos.x;
	int curpos1=(vertical)? pos.x : pos.y;
	curpos1/=iconsize;
	//int wrap=0;
	for(size_t i=0; i<tools.size(); i++)
	{
		int toolsize = (vertical) ? tools[i]->size : iconsize;
		if (tmppos + toolsize > maxx){ res.y++; tmppos = 0; }
		if (curpos > tmppos && curpos <= tmppos + toolsize && res.y == curpos1)
		{
			res.x=i;
			return res;
		}
		tmppos += toolsize;
		
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
	point = GetParent()->ClientToScreen(point);
	switch (alignment){
		case 0://left
			point.y += 20;
			point.x += GetSize().x;
			break;
		case 1://top
			point.y += GetSize().y;
			point.x += tools.size() * iconsize - 175;
			break;
		case 2://right
			point.y += 20;
			point.x -= /*GetSize().x + */350;
			break;
		case 3://bottom
			point.y -= 510;
			if (point.y < 0){ point.y = 0; }
			point.x += tools.size() * iconsize - 175;
			break;
		default:
			point.y += 20;
			point.x += GetSize().x;
			break;
	}
	ToolbarMenu *tbr = new ToolbarMenu(this,point);
	tbr->Show();
	
}

BEGIN_EVENT_TABLE(KaiToolbar, wxWindow)
	EVT_MOUSE_EVENTS(KaiToolbar::OnMouseEvent)
	EVT_PAINT(KaiToolbar::OnPaint)
	EVT_SIZE(KaiToolbar::OnSize)
	EVT_MENU(34566,KaiToolbar::OnToolbarOpts)
END_EVENT_TABLE()

ToolbarMenu::ToolbarMenu(KaiToolbar*_parent, const wxPoint &pos)
	:wxDialog(_parent,-1,"",pos, wxSize(350,510), wxBORDER_NONE)
	,sel(-1)
	,scPos(0)
	,parent(_parent)
	,bmp(NULL)
{
	fh=20;
	int ysize = parent->ids.size();
	//uważaj gdy kiedyś zechcesz zmienić rozmiar tego okna, to ustaw odpowiednio scrollbar
	scroll = new KaiScrollbar(this, -1, wxDefaultPosition, wxDefaultSize, wxVERTICAL);
	Bind(wxEVT_IDLE,&ToolbarMenu::OnIdle, this);
	wxString ans[] = { _("Po lewej"), _("U góry"), _("Po prawej"), _("Na dole") };
	alignments = new KaiChoice(this, 32213, wxPoint(4, 4), wxSize(342, 24), 4, ans);
	if (parent->alignment > 3 || parent->alignment < 0)
		parent->alignment = 0;
	alignments->SetSelection(parent->alignment);
	alignments->SetToolTip(_("Pozycja paska narzędzi"));
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		parent->alignment = alignments->GetSelection();
		Options.SetInt(ToolbarAlignment, parent->alignment);
		kainoteFrame *win = (kainoteFrame*)parent->GetParent();
		if (win){
			wxSizeEvent evt;
			win->OnSize(evt);
			win->Tabs->Refresh(false);
		}
		Destroy();
	}, 32213);
}

void ToolbarMenu::OnMouseEvent(wxMouseEvent &evt)
{
	bool leftdown=evt.LeftDown();
	int x=evt.GetX();
	int y=evt.GetY();
	int w=0;
	int h=0;
	GetSize (&w, &h);
	if((x<0||y<0||x>w||y>h)){
		if(leftdown){
			if(HasCapture()){ReleaseMouse();}
			Destroy();
		}
		return;
	}

	int elem = (y-30)/fh;
	elem+=scPos;
	if (elem >= (int)parent->ids.size() || y <= 30){ sel = -1; Refresh(false); return; }
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
			desc = desc.BeforeFirst('\t');
			parent->AddItem(parent->ids[elem], desc, item->icon,item->IsEnabled(), (item->GetSubMenu()!=NULL)? 1 : 0);
		}else{
			delete parent->tools[result];
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
	int ow = w;
	w-=18;
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < ow || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(ow,h);}
	tdc.SelectObject(*bmp);
	wxBitmap checkbmp = wxBITMAP_PNG("check");
	tdc.SetFont(wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma"));
	const wxColour & background = Options.GetColour(MenuBackground);
	const wxColour & txt = Options.GetColour(WindowText);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(txt));
	tdc.DrawRectangle(0,0,ow,h);
	int visible=25;//500/fh fh=20;
	int idssize=parent->ids.size();
	if(scPos>=idssize-visible){scPos=idssize-visible;}
	else if(scPos<0){scPos=0;}
	int maxsize=MAX(idssize,scPos+visible);
	scroll->SetScrollbar(scPos,visible,idssize,visible-1);
	scroll->SetSize(w,31,17,h-32);
	tdc.SetTextForeground(txt);
	for(int i=0;i<visible; i++)
	{
		int posY = (fh*i) + 32;
		MenuItem *item=parent->mb->FindItem(parent->ids[i+scPos]);
		bool check=false;
		for(size_t j=0; j<parent->tools.size();j++)
		{
			if(parent->tools[j]->id==parent->ids[i+scPos]){check=true; break;}
		}
		if(i+scPos==sel){
			tdc.SetPen(wxPen(Options.GetColour(MenuBorderSelection)));
			tdc.SetBrush(wxBrush(Options.GetColour(MenuBackgroundSelection)));
			tdc.DrawRectangle(1, posY-1, w - 1, fh - 2);
		}
		//tdc.SetPen(wxPen("#497CB0",2));
		//tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		
		
		if(check){
			tdc.DrawBitmap(checkbmp,4,posY);
		}

		tdc.DrawBitmap(item->GetBitmap(), fh + 8, posY);
		wxString desc=item->GetLabel();
		desc.Replace("&","");
		size_t reps=desc.Replace("\t"," (");
		wxString accel;
		wxString label=desc;
		//tdc.SetPen(wxPen("#497CB0"));
		if (reps){
			desc.Append(")");
			label=desc.BeforeFirst('(',&accel);
			accel.Prepend("(");
			int fw, fhh;
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel, w - fw - 8, posY);
		}
		tdc.DrawText(label, (fh * 2) + 15, posY);
		

	}
	wxPaintDC dc(this);
	dc.Blit(0,0,ow,h,&tdc,0,0);
}

void ToolbarMenu::OnIdle(wxIdleEvent& event)
{
    event.Skip();

	if(!parent->IsShownOnScreen()){
		Destroy();
	}

    if (IsShown())
    {
        wxPoint pos = ScreenToClient(wxGetMousePosition());
        wxRect rect(GetSize());

        if ( rect.Contains(pos) )
        {
            if ( HasCapture() )
            {
                ReleaseMouse();
            }
        }
        else
        {
            if ( !HasCapture() && !scroll->HasCapture())
            {
                CaptureMouse();
            }
        }
    }
}

void ToolbarMenu::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

BEGIN_EVENT_TABLE(ToolbarMenu, wxDialog)
	EVT_MOUSE_EVENTS(ToolbarMenu::OnMouseEvent)
	EVT_PAINT(ToolbarMenu::OnPaint)
	EVT_MOUSE_CAPTURE_LOST(ToolbarMenu::OnLostCapture)
	EVT_SCROLL(ToolbarMenu::OnScroll)
END_EVENT_TABLE()