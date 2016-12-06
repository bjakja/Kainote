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

//wxBitmap toDisable(const wxBitmap &bmp)
//{
//	wxImage img=bmp.ConvertToImage();
//	int size=bmp.GetWidth()*bmp.GetHeight()*3;
//	byte *data=img.GetData();
//			
//	for(int i=0; i<size; i++)
//	{
//		if(data[i]<226){data[i]+=30;}
//	}
//	return wxBitmap(img);
//}

static wxFont font;
static int height = 18;


KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
    const wxSize& size, int n, const wxString choices[],
    long style, const wxValidator& validator)
	:wxWindow(parent, id, pos, size, style|wxWANTS_CHARS)
	,bmp(NULL)
	,list(NULL)
	,itemList(NULL)
	,listIsShown(false)
	,enter(false)
	,clicked(false)
	,choiceChanged(false)
	,choice(-1)
{
	list = new wxArrayString(n,choices);
	disabled = new std::map<int, bool>();
	
	wxSize newSize((size.x<1)? 100 : size.x, (size.y<1)? 24 : size.y);
	SetMinSize(newSize);
	//SetBestSize(newSize);
	//SetMaxSize(wxSize(1000, 50));
	font = parent->GetFont();
}

KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
    const wxSize& size, const wxArrayString &choices,
    long style, const wxValidator& validator)
	:wxWindow(parent, id, pos, size, style|wxWANTS_CHARS)
	,bmp(NULL)
	,list(NULL)
	,itemList(NULL)
	,listIsShown(false)
	,enter(false)
	,clicked(false)
	,choiceChanged(false)
	,choice(-1)
{
	list = new wxArrayString(choices);
	disabled = new std::map<int, bool>();
	
	wxSize newSize((size.x<1)? 100 : size.x, (size.y<1)? 24 : size.y);
	SetMinSize(newSize);
	//SetBestSize(newSize);
	//SetMaxSize(wxSize(1000, 50));
	font = parent->GetFont();
}

KaiChoice::~KaiChoice()
{
	delete list;
	delete disabled;
	delete bmp;
}

void KaiChoice::SetToolTip(const wxString &tooltip)
{
	if(tooltip!=""){toolTip=tooltip;}
	wxWindow::SetToolTip((choice>=0)? toolTip + "\n" + GetString(choice) : tooltip);
}


void KaiChoice::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void KaiChoice::OnPaint(wxPaintEvent& event)
{
	//wxColour background = GetParent()->GetBackgroundColour();
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
	//tdc.SetBrush(wxBrush(background));
	//tdc.SetPen(wxPen(background));
	//tdc.DrawRectangle(0,0,w,h);
	bool enabled = IsThisEnabled();
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour((clicked)? wxSYS_COLOUR_BTNSHADOW : (enabled)? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_INACTIVECAPTION )));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour((enter)? wxSYS_COLOUR_MENUHILIGHT : (enabled)? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_GRAYTEXT)));
	tdc.DrawRectangle(0,0,w,h);
	
	if(w>15){
		wxBitmap arrow = wxBITMAP_PNG("arrow_list");
		tdc.DrawBitmap((enabled)? arrow : arrow.ConvertToDisabled(), w - 17, (h-10)/2);

		if(choice>=0){
			int fh=0, fw=w, ex=0, et=0;
			wxString txt = (*list)[choice];
			int removed=0;
			while(fw > w - 22){
				tdc.GetTextExtent(txt, &fw, &fh, &ex, &et, &font);
				txt = txt.RemoveLast();
				removed++;
			}
			if(removed<2){
				txt = (*list)[choice];
			}else{
				txt = txt.RemoveLast(2)+"...";
			}
			tdc.SetTextForeground(wxSystemSettings::GetColour((enabled)? wxSYS_COLOUR_WINDOWTEXT : wxSYS_COLOUR_GRAYTEXT));
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
	//wxMutexLocker mx(mutex);		
	if(event.LeftDown()||event.LeftIsDown() && !clicked){
		clicked=true;
		Refresh(false);
		//wxPaintEvent evt;
		//OnPaint(evt);
		SetFocus();
		if(!listIsShown){
			UnsetToolTip();
			ShowList();
		}
		else{
			itemList->EndPartialModal(-3);
			listIsShown=false; 

		}
	}
	if(event.LeftUp()){
		clicked=false;
		Refresh(false);
		//wxPaintEvent evt;
		//OnPaint(evt);
		
		//if(!(itemList && itemList->IsShown())){listIsShown=false;}
	}
	if(event.Entering()){
		enter=true;
		Refresh(false);
		return;
	}
	if(event.Leaving()&&enter){
		enter=false;
		clicked=false;
		Refresh(false);
		return;
	}
	if (event.GetWheelRotation() != 0) {
		if(!HasFocus()){event.Skip(); return;}
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		choice -=step;
		if(choice<0){choice=list->size()-1;}
		else if(choice >= (int)list->size()){choice = 0;}
		Refresh(false);
		if(choiceChanged){
			wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
			this->ProcessEvent(evt);
			choiceChanged=false;
		}
	}
}

void KaiChoice::OnKillFocus(wxFocusEvent &evt){
	if(!itemList || wxGetActiveWindow() == itemList) return;
	itemList->EndPartialModal(-3);listIsShown=false;
};

void KaiChoice::OnKeyPress(wxKeyEvent &event)
{
	if(itemList && itemList->IsShown()){
		itemList->OnKeyPress(event);
	}else if(event.GetKeyCode() == WXK_RETURN){
		ShowList();
	}
}

void KaiChoice::ShowList()
{
	listIsShown = true;
	wxSize listSize = GetSize();
	if(!itemList){itemList = new PopupList(this, list, disabled);}
	itemList->Popup(wxPoint(0, listSize.GetY()-1), listSize, choice);
	
}


void KaiChoice::SetSelection(int sel)
{
	if(sel >= (int)list->size()){return;}
	choice=sel; Refresh(false);
	if(sel >=0 ){SetToolTip();}
	else{SetToolTip(toolTip);}
}
	
void KaiChoice::Clear()
{
	list->Clear();
}
	
int KaiChoice::Append(const wxString &what)
{
	list->Add(what);
	return list->size()-1;
}
	
int KaiChoice::GetCount()
{
	return list->size();
}

void KaiChoice::EnableItem(int numItem, bool enable)
{
	auto disabledResult = disabled->find(numItem);
	if(enable && disabledResult!=disabled->end()){
		disabled->erase(disabledResult);
	}else if(!enable && !(disabledResult!=disabled->end())){
		(*disabled)[numItem]=false;
	}

}

int KaiChoice::FindString(const wxString &text, bool caseSensitive)
{
	return list->Index(text, caseSensitive);
}

void KaiChoice::Delete(int num)
{
	list->RemoveAt(num);
}

void KaiChoice::SendEvent(int _choice)
{
	if(_choice>=0){
		choice = _choice; Refresh(false);
		wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
	}
	SetToolTip();
	listIsShown=false;

}

wxIMPLEMENT_ABSTRACT_CLASS(KaiChoice, wxWindow);

BEGIN_EVENT_TABLE(KaiChoice, wxWindow)
	EVT_MOUSE_EVENTS(KaiChoice::OnMouseEvent)
	EVT_PAINT(KaiChoice::OnPaint)
	EVT_SIZE(KaiChoice::OnSize)
	EVT_KILL_FOCUS(KaiChoice::OnKillFocus)
	EVT_KEY_UP(KaiChoice::OnKeyPress)
END_EVENT_TABLE()

static int maxVisible = 20;

PopupList::PopupList(wxWindow *DialogParent, wxArrayString *list, std::map<int, bool> *disabled)
	/*:wxFrame(DialogParent,-1,"",wxDefaultPosition, wxDefaultSize, wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP|wxWS_EX_TRANSIENT)*/
	: wxPopupWindow(DialogParent)
	,sel(0)
	,scPos(0)
	,bmp(NULL)
	,Parent(DialogParent)
	,itemsList(list)
	,disabledItems(disabled)
{
	int fw=0;
	GetTextExtent("#TWFfGH", &fw, &height, 0, 0, &font);
	height+=4;
}

PopupList::~PopupList()
{
	wxDELETE(bmp);
	
}

void PopupList::Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem)
{
	sel = selectedItem;
	wxPoint npos = pos;//Parent->ClientToScreen(pos);
	wxSize size;
	CalcPosAndSize(&npos, &size, controlSize);
	SetPosition(npos);
	SetSize(size);

	Show();
}
	
void PopupList::CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize)
{
	int tx=0, ty=0;
	size_t isize = itemsList->size();
	for(size_t i = 0; i < isize; i++){
		GetTextExtent((*itemsList)[i], &tx, &ty, 0, 0, &font);
		if(tx > size->x){size->x = tx;}
	}
	
	size->x += 18;
	if(size->x < controlSize.x){size->x = controlSize.x;}
	if(isize > (size_t)maxVisible) {size->x += 20; isize=maxVisible;}
	size->y = height * isize + 2;
	int w, h;
	wxRect workArea = wxGetClientDisplayRect();
	w = workArea.width - workArea.x;
	h = workArea.height - workArea.y;
	
	if((pos->y + size->y) > h){
		pos->y -= (size->y + controlSize.y);
	}
}

void PopupList::OnMouseEvent(wxMouseEvent &evt)
{
	//wxLogStatus("Mouse");
	bool leftdown=evt.LeftDown();
	int x=evt.GetX();
	int y=evt.GetY();
	
	int elem = y/height;
	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -=step;
		if(scPos<0){scPos=0;}
		else if(scPos > (int)itemsList->size()-maxVisible){scPos = itemsList->size()-maxVisible;}
	}
	
	elem+=scPos;
	if(elem>=(int)itemsList->size() || elem < 0 ){return;}
	if(elem!=sel){
		sel=elem;
		Refresh(false);
	}
	
	if(evt.LeftUp() && !(disabledItems->find(elem) != disabledItems->end())){
		EndPartialModal(elem);
	}
	
}

void PopupList::OnPaint(wxPaintEvent &event)
{
	int itemsize = itemsList->size();
	if(scPos>=itemsize-maxVisible){scPos=itemsize-maxVisible;}
	if(scPos<0){scPos=0;}
	int maxsize=itemsize;
	if(sel<scPos && sel!=-1){scPos=sel;}
	else if(sel>= scPos + maxVisible && (sel-maxVisible+1) >= 0){scPos=sel-maxVisible+1;}
	if(itemsize>maxVisible){
		maxsize=maxVisible;
		SetScrollbar(wxVERTICAL, scPos, maxVisible, itemsize);
	}
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
	wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
	wxColour graytext = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
	wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);
	int r2 = highlight.Red(), g2 = highlight.Green(), b2 = highlight.Blue();
	int r = background.Red(), g = background.Green(), b = background.Blue();
	int inv_a = 65;
	int fr = (r2* inv_a / 0xFF) + (r - inv_a * r / 0xFF);
	int fg = (g2* inv_a / 0xFF) + (g - inv_a * g / 0xFF);
	int fb = (b2* inv_a / 0xFF) + (b - inv_a * b / 0xFF);
	wxColour menuhighlight(fr,fg,fb);

	tdc.SetFont(font);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(text));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetTextForeground(text);
	for(int i=0;i<maxsize; i++)
	{
		int scrollPos=i+scPos;
		
		if(scrollPos==sel){
			tdc.SetPen(wxPen(highlight));
			tdc.SetBrush(wxBrush(menuhighlight));
			tdc.DrawRectangle(2, (height*i)+2,w-4,height-2);
		}
		wxString desc=(*itemsList)[scrollPos];
		tdc.SetTextForeground((disabledItems->find(scrollPos) != disabledItems->end())? graytext : text);
		tdc.DrawText(desc,4,(height*i)+2);
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void PopupList::OnScroll(wxScrollWinEvent& event)
{
	int newPos=0;
	int tsize= itemsList->size();
	if(event.GetEventType()==wxEVT_SCROLLWIN_LINEUP)
	{
		newPos=scPos-1;
		if(newPos<0){newPos=0;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos=scPos+1;
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos-=(size.y/height - 1);
		newPos=MAX(0,newPos);
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos+=(size.y/height - 1);
		newPos=MIN(newPos,tsize-1);
	}
	else{newPos = event.GetPosition();}
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

//Odwo³uje pêtlê czekaj¹c¹
void PopupList::EndPartialModal(int ReturnId)
{
	((KaiChoice*)Parent)->SendEvent(ReturnId);
	Hide();
}

void PopupList::OnKeyPress(wxKeyEvent &event)
{
	if(event.GetKeyCode() == WXK_RETURN){
		EndPartialModal(sel);
		wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
	}else if(event.GetKeyCode() == WXK_ESCAPE){
		EndPartialModal(-3);
		((KaiChoice*)Parent)->listIsShown=false;
	}else if(event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_DOWN){
		int step = (event.GetKeyCode() == WXK_DOWN)? 1 : -1;
		sel += step;
		if(sel >= (int)itemsList->size()){
			sel=0;
		}else if(sel<0){
			sel = itemsList->size()-1;
		}
				
		Refresh(false);
	}
}
void PopupList::OnKillFocus(wxFocusEvent &evt){
	EndPartialModal(-3);
};

BEGIN_EVENT_TABLE(PopupList,/* wxFrame*/wxPopupWindow)
	EVT_MOUSE_EVENTS(PopupList::OnMouseEvent)
	EVT_PAINT(PopupList::OnPaint)
	EVT_SCROLLWIN(PopupList::OnScroll)
END_EVENT_TABLE()

