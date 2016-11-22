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

#include "ListControls.h"
#include "Config.h"


static const wxFont font = wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");

	KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
    const wxSize& size, int n, const wxString choices[],
    long style, const wxValidator& validator)
{
	list = new wxArrayString(n,choices);
	KaiChoice(parent, id, pos, size, *list, style, validator);
}

KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
    const wxSize& size, const wxArrayString &choices,
    long style, const wxValidator& validator)
	:wxWindow(parent, id, pos, size, style)
	,bmp(NULL)
	,list(NULL)
	,listMenu(NULL)
	,listIsShown(false)
	,enter(false)
	,clicked(false)
	,choiceChanged(false)
	,choice(-1)
{
	if(!list){list = new wxArrayString(choices);}
	listMenu= new Menu();
	Bind(wxEVT_PAINT, &KaiChoice::OnPaint, this);
	Bind(wxEVT_SIZE, &KaiChoice::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_KEY_UP, &KaiChoice::OnKeyPress, this);
	
	for(size_t i = 0; i < list->size(); i++){
		listMenu->Append(8000+i, (*list)[i]);
	}
	//SetMinSize(wxSize(24, 24));
	SetMaxSize(wxSize(1000, 50));
}

KaiChoice::~KaiChoice()
{
	delete list;
	delete listMenu;
	delete bmp;
}


void KaiChoice::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void KaiChoice::OnPaint(wxPaintEvent& event)
{
	wxColour background = GetParent()->GetBackgroundColour();
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
	tdc.SetFont(font);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour((clicked)? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_BTNFACE)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour((enter)? wxSYS_COLOUR_MENUHILIGHT : wxSYS_COLOUR_BTNSHADOW)));
	tdc.DrawRectangle(1,1,w-2,h-2);
	
	if(w>15){
		/*wxPoint points[3];
		int pos1 = h/2;
		int pos  = w - 10;
		points[0]=wxPoint(pos-6,pos1-2);
		points[1]=wxPoint(pos,pos1-2);
		points[2]=wxPoint(pos-3,pos1+2);
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)));
		tdc.DrawPolygon(3,points);*/
		tdc.DrawBitmap(wxBITMAP_PNG("arrow_list"), w - 17, (h-10)/2);

		if(choice>=0){
			int fh=0, fw=w, ex=0, et=0;
			wxString txt = (*list)[choice];
			int removed=0;
			while(fw > w - 15){
				tdc.GetTextExtent(txt, &fw, &fh, &ex, &et, &font);
				txt = txt.RemoveLast();
				removed++;
			}
			if(removed<2){
				txt = (*list)[choice];
			}else{
				txt = txt.RemoveLast(2)+"...";
			}
			tdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
			//tdc.DrawText(txt, 4, (h-fh));
			wxRect cur(5, (h-fh)/2, w - 19, fh);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(txt,cur,wxALIGN_LEFT);
			tdc.DestroyClippingRegion();
		}
	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiChoice::OnMouseEvent(wxMouseEvent &event)
{
	if(event.Entering()){
		enter=true;
		Refresh(false);
		return;
	}
	if(event.Leaving()&&enter){
		enter=false;
		Refresh(false);
		return;
	}
			
	if(event.LeftDown()){
		clicked=true;
		Refresh(false);
		if(listIsShown){
			listMenu->HideMenu();
		}else{
			ShowList();
		}
	}
	if(event.LeftUp()){
		clicked=false;
		Refresh(false);
	}
	if (event.GetWheelRotation() != 0) {
		if(!HasFocus()){event.Skip(); return;}
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		choice -=step;
		if(choice<0){choice=0;}
		else if(choice > (int)list->size()){choice = list->size();}
		Refresh(false);
		if(choiceChanged){
			wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
			this->ProcessEvent(evt);
			choiceChanged=false;
		}
	}
}

void KaiChoice::OnKeyPress(wxKeyEvent &event)
{
	if(event.GetKeyCode() == WXK_RETURN){
		ShowList();
	}
}

void KaiChoice::ShowList()
{
	listIsShown = true;
	listMenu->SetShowIcons(false);
	int elem = listMenu->GetPopupMenuSelection(wxPoint(0, GetSize().GetY()), this)-8000;
	if(elem>=0){
		choice = elem; Refresh(false);
		wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
	}
	listIsShown=false; 
}


void KaiChoice::SetSelection(int sel)
{
	choice=sel; Refresh(false);
}
	
void KaiChoice::Clear()
{
	list->Clear();
	listMenu->Clear();
}
	
void KaiChoice::Append(wxString what)
{
	list->Add(what);
	listMenu->Append(8000+list->size()-1, what);
}
	
