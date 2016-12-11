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
#include "KaiTextCtrl.h"
#include <wx/msw/private.h>

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

static int height = 18;


KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
					 const wxSize& size, int n, const wxString choices[],
					 long style, const wxValidator& validator)
					 :wxWindow(parent, id, pos, size, style)
					 ,bmp(NULL)
					 ,list(NULL)
					 ,itemList(NULL)
					 ,choiceText(NULL)
					 ,listIsShown(false)
					 ,enter(false)
					 ,clicked(false)
					 ,choiceChanged(false)
					 ,choice(-1)
{
	list = new wxArrayString(n,choices);
	disabled = new std::map<int, bool>();


	//SetBestSize(newSize);
	//SetMaxSize(wxSize(1000, 50));
	SetFont(parent->GetFont());
	int fw, fh;
	if(size.y<1){
		GetTextExtent("#TWFfGH", &fw, &fh);
	}
	wxSize newSize((size.x<1)? 100 : size.x, (size.y<1)? fh+10 : size.y);
	SetMinSize(newSize);
	//if(style & KAI_COMBO_BOX){
	//	choiceText = new KaiTextCtrl(this, 27789, "", wxPoint(1,1), wxSize(newSize.x-22, newSize.y-2));
	//	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent &evt){
	//		SetSelectionByPartialName(choiceText->GetValue());
	//	}, 27789);
	//	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
	//		//wxLogStatus("on accelerator up");
	//		wxKeyEvent kevt;
	//		kevt.m_keyCode = WXK_UP;
	//		if(itemList&&itemList->IsShown()){itemList->OnKeyPress(kevt);}
	//		else{evt.Skip();}
	//	},ID_TUP);
	//	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
	//		//wxLogStatus("on accelerator down");
	//		wxKeyEvent kevt;
	//		kevt.m_keyCode = WXK_DOWN;
	//		if(itemList&&itemList->IsShown()){itemList->OnKeyPress(kevt);}
	//		else{evt.Skip();}
	//	},ID_TDOWN);
	//}
}

KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
					 const wxSize& size, const wxArrayString &choices,
					 long style, const wxValidator& validator)
					 :wxWindow(parent, id, pos, size, style)
					 ,bmp(NULL)
					 ,list(NULL)
					 ,itemList(NULL)
					 ,choiceText(NULL)
					 ,listIsShown(false)
					 ,enter(false)
					 ,clicked(false)
					 ,choiceChanged(false)
					 ,choice(-1)
{
	list = new wxArrayString(choices);
	disabled = new std::map<int, bool>();


	SetFont(parent->GetFont());
	int fw, fh;
	if(size.y<1){
		GetTextExtent("#TWFfGH", &fw, &fh);
	}
	wxSize newSize((size.x<1)? 100 : size.x, (size.y<1)? fh+10 : size.y);
	SetMinSize(newSize);
	//if(style & KAI_COMBO_BOX){
	//	choiceText = new KaiTextCtrl(this, 27789, "", wxPoint(1,1), wxSize(newSize.x-22, newSize.y-2), wxBORDER_NONE);
	//	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent &evt){
	//		SetSelectionByPartialName(choiceText->GetValue());
	//	}, 27789);
	//	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
	//		//wxLogStatus("on accelerator up");
	//		wxKeyEvent kevt;
	//		kevt.m_keyCode = WXK_UP;
	//		if(itemList&&itemList->IsShown()){itemList->OnKeyPress(kevt);}
	//		else{evt.Skip();}
	//	},ID_TUP);
	//	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
	//		//wxLogStatus("on accelerator down");
	//		wxKeyEvent kevt;
	//		kevt.m_keyCode = WXK_DOWN;
	//		if(itemList&&itemList->IsShown()){itemList->OnKeyPress(kevt);}
	//		else{evt.Skip();}
	//	},ID_TDOWN);
	//}
}

KaiChoice::KaiChoice(wxWindow *parent, int id, const wxString &comboBoxText, const wxPoint& pos,
					 const wxSize& size, const wxArrayString &choices,
					 long style, const wxValidator& validator)
					 :wxWindow(parent, id, pos, size, style|KAI_COMBO_BOX)
					 ,bmp(NULL)
					 ,list(NULL)
					 ,itemList(NULL)
					 ,choiceText(NULL)
					 ,listIsShown(false)
					 ,enter(false)
					 ,clicked(false)
					 ,choiceChanged(false)
					 ,choice(-1)
{
	list = new wxArrayString(choices);
	disabled = new std::map<int, bool>();


	SetFont(parent->GetFont());
	int fw, fh;
	if(size.y<1){
		GetTextExtent("#TWFfGH", &fw, &fh);
	}
	wxSize newSize((size.x<1)? 100 : size.x, (size.y<1)? fh+10 : size.y);
	SetMinSize(newSize);
	choiceText = new KaiTextCtrl(this, 27789, comboBoxText, wxPoint(1,1), wxSize(newSize.x-22, newSize.y-2), wxBORDER_NONE);
	choiceText->Bind(wxEVT_ENTER_WINDOW,&KaiChoice::OnMouseEvent,this,27789);
	choiceText->Bind(wxEVT_LEAVE_WINDOW,&KaiChoice::OnMouseEvent,this,27789);
	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent &evt){
		SetSelectionByPartialName(choiceText->GetValue());
	}, 27789);
	//Connect(ID_TDEL,ID_TRETURN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&KaiTextCtrl::OnAccelerator);
	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		//wxLogStatus("on accelerator up");
		wxKeyEvent kevt;
		kevt.m_keyCode = WXK_UP;
		if(itemList&&itemList->IsShown()){itemList->OnKeyPress(kevt);}
		else{evt.Skip();}
	},ID_TUP);
	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		//wxLogStatus("on accelerator down");
		wxKeyEvent kevt;
		kevt.m_keyCode = WXK_DOWN;
		if(itemList&&itemList->IsShown()){itemList->OnKeyPress(kevt);}
		else{evt.Skip();}
	},ID_TDOWN);
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
	if(choiceText){
		wxSize newSize = GetClientSize();
		choiceText->SetSize(wxSize(newSize.x-22, newSize.y-2));
	}
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
	tdc.SetFont(GetFont());
	//int fsize = font.GetPointSize();
	//wxLogStatus("fs paint %i",fsize);
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
				tdc.GetTextExtent(txt, &fw, &fh, &ex, &et/*, &font*/);
				txt = txt.RemoveLast();
				removed++;
			}
			if(removed<2){
				txt = (*list)[choice];
			}else{
				txt = txt.RemoveLast(2)+"...";
			}
			if(!choiceText){
				tdc.SetTextForeground(wxSystemSettings::GetColour((enabled)? wxSYS_COLOUR_WINDOWTEXT : wxSYS_COLOUR_GRAYTEXT));
				//tdc.DrawText(txt, 4, (h-fh));
				wxRect cur(5, (h-fh)/2, w - 19, fh);
				tdc.SetClippingRegion(cur);
				tdc.DrawLabel(txt,cur,wxALIGN_LEFT);
				tdc.DestroyClippingRegion();
			}
		}
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiChoice::OnMouseEvent(wxMouseEvent &event)
{
	//wxMutexLocker mx(mutex);		
	if(listIsShown){return;}
	if(event.LeftDown()/*||event.LeftIsDown() && !clicked*/){
		clicked=true;
		Refresh(false);
		SetFocus();
		UnsetToolTip();
		ShowList();

		if(choiceText){
			choiceText->SetFocus();
			choiceText->SetSelection(0,choiceText->GetValue().Len(),true);
		}

		return;
	}

	if(event.LeftUp()){
		clicked=false;
		Refresh(false);
	}
	if(event.Entering()){
		enter=true;
		Refresh(false);
		return;
	}
	if(event.Leaving()&&enter){
		if(choiceText){
			wxPoint pos = ScreenToClient(wxGetMousePosition());
			if(GetClientRect().Contains(pos)) return;
		}
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


void KaiChoice::OnKeyPress(wxKeyEvent &event)
{
	if(itemList && itemList->IsShown()){
		itemList->OnKeyPress(event);
	}else if(event.GetKeyCode() == WXK_RETURN && !(GetWindowStyle() & wxTE_PROCESS_ENTER)){
		ShowList();
	}
}

void KaiChoice::ShowList()
{
	listIsShown = true;
	wxSize listSize = GetSize();
	if(!itemList){itemList = new PopupList(this, list, disabled);}
	if(choiceText){SetSelectionByPartialName(choiceText->GetValue());}
	itemList->Popup(wxPoint(0, listSize.GetY()), listSize, choice);

}


void KaiChoice::SetSelection(int sel, bool changeText)
{
	if(sel >= (int)list->size()){return;}
	choice=sel; 

	if(itemList && itemList->IsShown()){
		itemList->SetSelection(choice);
	}
	if(choiceText && changeText){
		wxString txt = (sel < 0)? wxEmptyString : (*list)[sel];
		choiceText->SetValue(txt);
	}
	Refresh(false);
	//}
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

void KaiChoice::Append(const wxArrayString &itemsArray)
{
	list->insert(list->end(), itemsArray.begin(), itemsArray.end());
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
	enter=false;
	clicked=false;
	if(_choice>=0){
		//choice = _choice; //Refresh(false);
		SetSelection(_choice);
		wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
	}else{
		Refresh(false);
	}
	SetToolTip();
	listIsShown=false;

}

void KaiChoice::SetSelectionByPartialName(const wxString &PartialName)
{
	if(PartialName==""){SetSelection(0, false);return;}
	int sell=-1;
	wxString PrtName = PartialName.Lower();

	for(size_t i=0; i<list->size(); i++){
		wxString fontname = (*list)[i].Lower();
		if(fontname.StartsWith(PrtName)){
			sell=i;
			break;
		}
	}

	if(sell!=-1){
		SetSelection(sell, false);
	}
}

void KaiChoice::SetValue(const wxString &text){
	if(choiceText){choiceText->SetValue(text);}
}

wxString KaiChoice::GetValue(){
	if(choiceText){
		return choiceText->GetValue();
	} 
	return "";
}

void KaiChoice::SelectChoice(int _choice){
	choice = _choice;
	if(choiceText){
		choiceText->SetValue((*list)[choice]);
		choiceText->SetFocus();
		choiceText->SetSelection(0,choiceText->GetValue().Len(),true);
	}else{Refresh(false);}

}

void KaiChoice::Insert(const wxString &what, int position){
	int pos = MID(0, position, (int)list->size()-1);
	list->Insert(what, pos);	
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiChoice, wxWindow);

BEGIN_EVENT_TABLE(KaiChoice, wxWindow)
	EVT_MOUSE_EVENTS(KaiChoice::OnMouseEvent)
	EVT_PAINT(KaiChoice::OnPaint)
	EVT_SIZE(KaiChoice::OnSize)
	//EVT_KILL_FOCUS(KaiChoice::OnKillFocus)
	EVT_KEY_UP(KaiChoice::OnKeyPress)
	END_EVENT_TABLE()

	static int maxVisible = 20;

PopupList::PopupList(wxWindow *DialogParent, wxArrayString *list, std::map<int, bool> *disabled)
	/*:wxFrame(DialogParent,-1,"",wxDefaultPosition, wxDefaultSize, wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP|wxWS_EX_TRANSIENT)*/
	: wxPopupWindow(DialogParent)
	,sel(0)
	,scPos(0)
	//,fullScPos(20)
	,orgY(0)
	,bmp(NULL)
	,Parent(DialogParent)
	,itemsList(list)
	,disabledItems(disabled)
{
	int fw=0;
	SetFont(DialogParent->GetFont());
	GetTextExtent("#TWFfGH", &fw, &height, 0, 0/*, &font*/);
	height+=6;
	//thislist=this;
}

PopupList::~PopupList()
{
	wxDELETE(bmp);
}

void PopupList::Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem)
{
	SetSelection(selectedItem);
	wxPoint npos = pos;//Parent->ClientToScreen(pos);
	wxSize size;
	CalcPosAndSize(&npos, &size, controlSize);
	SetPosition(npos);
	SetSize(size);
	orgY = size.y;
	Show();
	Bind(wxEVT_IDLE,&PopupList::OnIdle, this);
	//if(!HasCapture()){CaptureMouse();}
	//HookMouse = NULL;
	//HookMouse = SetWindowsHookEx(WH_GETMESSAGE, &OnMouseClick, NULL,GetCurrentThreadId());
}

void PopupList::CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize)
{
	int tx=0, ty=0;
	size_t isize = itemsList->size();
	for(size_t i = 0; i < isize; i++){
		GetTextExtent((*itemsList)[i], &tx, &ty, 0, 0/*, &font*/);
		if(tx > size->x){size->x = tx;}
	}

	size->x += 18;
	if(isize > (size_t)maxVisible) {size->x += 20; isize=maxVisible;}
	if(size->x > 200){size->x=200;}
	if(size->x < controlSize.x){size->x = controlSize.x;}
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
	
	//if(evt.MiddleUp()){return;}
	//wxLogStatus("Mouse");
	bool leftdown=evt.LeftDown();
	int x=evt.GetX();
	int y=evt.GetY();
	wxSize sz = GetClientSize();
	if(leftdown){
		wxPoint posOnScreen = wxGetMousePosition();
		bool contains=false;
		int x, y;
		wxPopupWindowBase::DoGetPosition(&x, &y);
		wxRect rc = GetRect();
		rc.x=x; rc.y=y;
		if(!rc.Contains(posOnScreen)){
			EndPartialModal(-3);
		}
		return;
	}
	/*if((x < 0 || x > orgY || y <0 || y > sz.y)){
		
		return;
	}*/

	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -=step;
		if(scPos<0){scPos=0;}
		else if(scPos > (int)itemsList->size()-maxVisible){scPos = itemsList->size()-maxVisible;}
	}

	int elem = y/height;
	elem+=scPos;
	if(elem>=(int)itemsList->size() || elem < 0 || x < 0 || x > sz.x){return;}
	if(elem!=sel){
		if(elem>= scPos+maxVisible || elem < scPos){return;}
		sel=elem;
		Refresh(false);
	}


	if(evt.LeftUp() && !(disabledItems->find(elem) != disabledItems->end()) && !(x < 0 || x > sz.x || y <0 || y > sz.y)){
		EndPartialModal(elem);
	}
	//evt.Skip();
}

void PopupList::OnPaint(wxPaintEvent &event)
{
	
	int itemsize = itemsList->size();
	if(scPos>=itemsize-maxVisible){scPos=itemsize-maxVisible;}
	if(scPos<0){scPos=0;}
	int maxsize=itemsize;
	if(itemsize>maxVisible){
		maxsize=maxVisible;
		SetScrollbar(wxVERTICAL, scPos, maxVisible, itemsize);
	}
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	int bitmapw=w;
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < bitmapw || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(bitmapw, h);}
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

	tdc.SetFont(GetFont());
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(text));
	tdc.DrawRectangle(0,0,bitmapw,h);
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
		tdc.DrawText(desc,4,(height*i)+3);
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,bitmapw,h,&tdc,0,0);
}

void PopupList::SetSelection(int pos){
	sel = pos; 
	scPos = pos; 
	if(sel<scPos && sel!=-1){scPos=sel;}
	else if(sel>= scPos + maxVisible && (sel-maxVisible+1) >= 0){scPos=sel-maxVisible+1;}
	Refresh(false);
};

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
	if(HasCapture()){ReleaseMouse();}
	//UnhookWindowsHookEx( HookMouse );
	Unbind(wxEVT_IDLE,&PopupList::OnIdle, this);
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
		scPos+=step;
		if(sel >= (int)itemsList->size()){
			sel=0;
			scPos=0;
		}else if(sel<0){
			sel = itemsList->size()-1;
			scPos=sel;
		}
		((KaiChoice*)Parent)->SelectChoice(sel);		
		Refresh(false);
	}
}

//WXLRESULT PopupList::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) {
//
//	if (message == 28) {
//		EndPartialModal(-3);
//		return 0;
//	}
//	return wxPopupWindow::MSWWindowProc(message, wParam, lParam);
//}
//
//
//LRESULT CALLBACK PopupList::OnMouseClick( int code, WPARAM wParam, LPARAM lParam )
//{
//	if(code < 0)
//	{ 
//		CallNextHookEx(thislist->HookMouse, code, wParam, lParam);
//		return 0;
//	}
//	LPMSG msg = (LPMSG)lParam;
//	//if( msg->message == WM_LBUTTONDOWN || msg->message == WM_NCLBUTTONDOWN || 
//	//	msg->message == WM_RBUTTONDOWN || msg->message == WM_NCRBUTTONDOWN){
//	//		wxPoint posOnScreen = wxGetMousePosition();
//	//		bool contains=false;
//	//		int x, y;
//	//		thislist->wxPopupWindowBase::DoGetPosition(&x, &y);
//	//		wxRect rc = thislist->GetRect();
//	//		rc.x=x; rc.y=y;
//	//		//wxLogStatus("contains %i, %i %i %i %i %i", posOnScreen.x, posOnScreen.y, rc.x, rc.y, rc.GetRight(), rc.GetBottom());
//	//		if(rc.Contains(posOnScreen)){
//	//			contains=true;
//	//		}
//
//	//		//wxLogStatus("hide contains %i", (int)contains);
//	//		if(contains) {return 0;}
//	//		thislist->EndPartialModal(-3);
//	//}
//	if(msg->hwnd!=thislist->GetHWND() && msg->hwnd!=thislist->Parent->GetHWND()){
//		msg->hwnd=thislist->GetHWND();
//		if(msg->message != WM_PAINT && msg->message != WM_LBUTTONDOWN && msg->message != WM_NCLBUTTONDOWN){
//			msg->message=WM_MBUTTONUP;
//		}
//	}
//	return CallNextHookEx( 0, code, wParam, lParam );
//}
//
//PopupList *PopupList::thislist=NULL;

void PopupList::OnIdle(wxIdleEvent& event)
{
    event.Skip();

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
            if ( !HasCapture() )
            {
                CaptureMouse();
            }
        }
    }
}

BEGIN_EVENT_TABLE(PopupList,/* wxFrame*/wxPopupWindow)
	EVT_MOUSE_EVENTS(PopupList::OnMouseEvent)
	EVT_PAINT(PopupList::OnPaint)
	EVT_SCROLLWIN(PopupList::OnScroll)
	EVT_MOUSE_CAPTURE_LOST(PopupList::OnLostCapture)
END_EVENT_TABLE()

	/*if(hasScrollbar){
	tdc.SetPen(wxPen(graytext));
	tdc.SetBrush(wxBrush(graytext));
	tdc.DrawRectangle(w-1, 0, 19, h);
	wxBitmap scrollArrow = wxBITMAP_PNG("arrow_list");
	wxImage img = scrollArrow.ConvertToImage();
	img = img.Rotate180();
	tdc.DrawBitmap(wxBitmap(img), w + 3, 3);
	tdc.DrawBitmap(scrollArrow, w + 3, h-16);
	float divScroll = (float)maxVisible / (float)itemsize;
	int scrollHeight = (h-38) * divScroll;
	if(scrollHeight < 10){scrollHeight=10;}
	tdc.SetPen(wxPen(text));
	tdc.SetBrush(wxBrush(text));
	tdc.DrawRectangle(w+1, fullScPos, 16, scrollHeight);
	}*/


	/*if(hasScrollbar && x > sz.x - 20 && x <= sz.x ){
	if(leftdown || evt.LeftIsDown()){
	wxLogStatus("leftdown");
	int itemsize = itemsList->size();
	if(y>=0 && y < 20){
	scPos+=1;
	int diff = itemsize - maxVisible;
	if(scPos> diff){scPos = diff;return;}
	Refresh(false);
	return;
	}else if(y >= sz.y - 20 && y <= sz.y){
	scPos-=1;
	if(scPos<0){scPos = 0;return;}
	Refresh(false);
	return;
	}
	float divScroll = (float)maxVisible / (float)itemsize;
	int scrollbarPaneHeight = (sz.y-38);
	int scrollHeight = scrollbarPaneHeight * divScroll;
	int scrollbarMaxMove = scrollbarPaneHeight - scrollHeight;
	if(y < 20 || y > scrollbarMaxMove){return;}
	int diffY = oldY - y;
	fullScPos += diffY;
	scPos += diffY * divScroll;
	Refresh(false);
	}
	return;
	}*/
