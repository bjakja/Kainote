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


#include "Menu.h"
#include "Config.h"
#include "Toolbar.h"

static wxFont font = wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");
const static int height = 22;
static int maxVisible = 30;
static bool showMnemonics=false;
static bool secondAlt=false;

wxDEFINE_EVENT(EVT_MENU_OPENED, MenuEvent);

void Mnemonics::findMnemonics(const wxString &label, int pos){
	int result = label.Find('&');
	if(result != -1){
		wxString rawmnemonics = label.Mid(result + 1, 1);
		char mn = rawmnemonics.Upper()[0];
		mnemonics[mn] = pos;
	}
}

MenuItem::MenuItem(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu *Submenu, byte _type)
{
	icon=_icon; 
	label=_label; 
	id=_id; 
	enabled=_enable;
	type=_type;
	submenu=Submenu; 
	check=false;
	help = _help;
	accel=NULL;
}
MenuItem::~MenuItem()
{
	if(submenu){delete submenu;}
	if(icon){delete icon;}
}
bool MenuItem::Enable(bool enable)
{
	if(enabled!=enable){enabled=enable;return true;}
	return false;
}
wxBitmap MenuItem::GetBitmap()
{
	if(!icon){return wxBitmap();}
	if(!enabled){
		return wxBitmap(icon->ConvertToImage().ConvertToGreyscale());
	}
	return *icon;
}

void MenuItem::SetAccel(wxAcceleratorEntry *entry)
{
	accel = entry;
	if(label.find("\t")!=-1){label = label.BeforeFirst('\t');}
	label += "\t" + entry->ToString();
	//if(MenuBar::Menubar){MenuBar::Menubar->SetAccelerators();}
}

Menu::Menu(char window)
	:Mnemonics()
	,dialog(NULL)
	,parentMenu(NULL)
	,wnd(window)
{
}


int Menu::GetPopupMenuSelection(const wxPoint &pos, wxWindow *parent, int *accels, bool clientPos, int _maxVisible)
{
	wxPoint npos= pos;
	wxSize size;
	maxVisible = _maxVisible;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	
	dialog = new MenuDialog(this, parent, npos, size, false);
	int ret = dialog->ShowPartialModal();
	if(accels){*accels = dialog->accel;}
	dialog->Destroy();
	dialog = NULL;
	return ret;
}

void Menu::PopupMenu(const wxPoint &pos, wxWindow *parent, bool clientPos, int _maxVisible)
{
	wxPoint npos= pos;
	wxSize size;
	maxVisible = _maxVisible;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	//wxLogStatus("Mn size %i", mnemonics.size());
	
	dialog = new MenuDialog(this, parent, npos, size);

	
	//dialog->ShowWithEffect(wxSHOW_EFFECT_BLEND ,1);//wxSHOW_EFFECT_BLEND
	//if((size_t)maxVisible < items.size()){dialog->SetFocus();}
	//dialog->Refresh(false);
	dialog->Show();
}

void Menu::CalcPosAndSize(wxWindow *parent, wxPoint *pos, wxSize *size, bool clientPos)
{
	int tx=0, ty=0;
	if(clientPos){*pos = parent->ClientToScreen(*pos);}
	size_t isize = items.size();
	for(size_t i = 0; i < isize; i++){
		parent->GetTextExtent(items[i]->label, &tx, &ty, 0, 0, &font);
		if(tx > size->x){size->x = tx;}
	}
	
	size->x += 58;
	if(isize > (size_t)maxVisible) {size->x += 20; isize=maxVisible;}
	size->y = height * isize + 4;
	int w,h;
	wxRect workArea = wxGetClientDisplayRect();
	w = workArea.width - workArea.x;
	h = workArea.height - workArea.y;
	if(size->y > h){ size->y = h; }
	if((pos->x + size->x) > w){
		pos->x -= size->x;
		//wxLogStatus("dialog %i %i", (int)parentMenu, (parentMenu)? (int)parentMenu->dialog : 0);
		if(parentMenu && parentMenu->dialog){
			wxSize size = parentMenu->dialog->GetClientSize();
			//wxLogStatus("posx %i", pos->x);
			pos->x -= size.x;
			//wxLogStatus("posx1 %i", pos->x);
		}
	}
	if((pos->y + size->y) > h){
		if(size->y > h/2){pos->y -= size->y;if(pos->y<0){pos->y=0;}}
		else{pos->y = h - size->y;}
	}
	
}

MenuItem *Menu::AppendTool(KaiToolbar *ktb, int id, wxString text, wxString help, wxBitmap *bitmap, bool enable, Menu *SubMenu)
{
	if(bitmap && bitmap->IsOk()){ktb->AddID(id);}
	return Append(id, text, help, enable, bitmap, SubMenu);
}

MenuItem *Menu::Append(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	findMnemonics(_label,items.size()-1);
	return item;
}
MenuItem *Menu::Append(int _id,const wxString& _label, Menu* Submenu, const wxString& _help, byte _type, bool _enable, wxBitmap *_icon)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	findMnemonics(_label,items.size()-1);
	return item;
}
	
MenuItem *Menu::Append(MenuItem *item)
{
	if(item->submenu){item->submenu->parentMenu=this;}
	items.push_back(item);
	findMnemonics(item->label,items.size()-1);
	return item;
}
	
MenuItem *Menu::Prepend(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.insert(items.begin(),item);
	findMnemonics(_label,0);
	return item;
}
	
MenuItem *Menu::Prepend(MenuItem *item)
{
	if(item->submenu){item->submenu->parentMenu=this;}
	items.insert(items.begin(),item);
	findMnemonics(item->label,0);
	return item;
}
	
MenuItem *Menu::Insert(int position, int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.insert(items.begin() + position,item);
	findMnemonics(_label,position);
	return item;
}
	
MenuItem *Menu::Insert(int position, MenuItem *item)
{
	if(item->submenu){item->submenu->parentMenu=this;}
	items.insert(items.begin() + position,item);
	findMnemonics(item->label,items.size()-1);
	return item;
}
	
bool Menu::Destroy(MenuItem *item)
{
	for(size_t i = 0; i < items.size(); i++ ){
		if(items[i] == item){
			delete item;
			items.erase(items.begin()+i);
			return true;
		}else if(items[i]->submenu){
			if(items[i]->submenu->Destroy(item)){return true;}
		}

	}
	delete item;
	return false;
}

void Menu::Delete(int position)
{
	delete items[position];
	items.erase(items.begin()+position);
}
	
int Menu::GetMenuItemCount()
{
	return items.size();
}
	
MenuItem *Menu::FindItem(int id)
{
	for(size_t i = 0; i < items.size(); i++ ){
		if(items[i]->id==id){
			return items[i];
		}else if(items[i]->submenu){
			MenuItem * item = items[i]->submenu->FindItem(id);
			if(item){return item;}
		}

	}
	return NULL;
}
	
MenuItem *Menu::FindItem(const wxString& label)
{
	for(size_t i = 0; i < items.size(); i++ ){
		if(items[i]->label==label){
			return items[i];
		}else if(items[i]->submenu){
			MenuItem * item = items[i]->submenu->FindItem(label);
			if(item){return item;}
		}

	}
	return NULL;
}

	
MenuItem *Menu::FindItemByPosition(int pos)
{
	if(pos>(int)items.size() || pos<0){return NULL;} 
	return items[pos];
}

void Menu::Check(int id, bool check)
{
	MenuItem * item = FindItem(id);
	if(item){
		item->check = check;
	}
}
	
void Menu::AppendSeparator()
{
	MenuItem *item = new MenuItem(-2, "", "", false, 0, 0, ITEM_SEPARATOR);
	items.push_back(item);
}

void Menu::DestroyDialog()
{
	if(dialog){
		dialog->Destroy(); dialog=NULL;
	}
}

MenuItem *Menu::SetAccMenu(int id, const wxString &txt, const wxString &help, bool enable, int kind)
{
	idAndType itype(id, wnd);
	wxString hkey=Hkeys.GetMenuH(itype, txt);
	wxString mtext=(hkey!="")? txt.BeforeFirst('\t')+"\t"+hkey : txt;
	return Append(id,mtext,help,true,0,0,kind);
}

MenuItem *Menu::SetAccMenu(MenuItem *menuitem, const wxString &name)
{
	int id=0;
	for(auto cur=Hkeys.hkeys.rbegin(); cur!=Hkeys.hkeys.rend(); cur++)
	{
		if(cur->first < 30100){break;}
		if(cur->second.Name == name ){
			//hkey = cur->second.Accel;
			id = cur->first.id;	
		}
	}

	if(id){menuitem->SetAccel(&Hkeys.GetHKey(id));}
	return Append(menuitem);
}

void Menu::GetAccelerators(std::vector <wxAcceleratorEntry> *entries)
{
	for(auto item : items){
		if(item->submenu){
			item->submenu->GetAccelerators(entries);
		}else if(item->accel){
			entries->push_back(*item->accel);
		}

	}
}

MenuDialog* MenuDialog::ParentMenu=NULL;
MenuDialog* MenuDialog::lastActiveMenu=NULL;
volatile int MenuDialog::id=-3;

MenuDialog::MenuDialog(Menu *_parent, wxWindow *DialogParent, const wxPoint &pos, const wxSize &size, bool sendEvent)
	:wxFrame(DialogParent,-1,"",pos, size, wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP|wxWS_EX_TRANSIENT)
	,wxGUIEventLoop()
	,parent(_parent)
	,sel(showMnemonics? 0 :-1)
	,scPos(0)
	,subMenuIsShown(false)
	,isPartialModal(false)
	,submenuShown(-1)
	,submenuToHide(-1)
	,bmp(NULL)
	,accel(0)
{
	if(!ParentMenu){
		ParentMenu=this;
		//wxLogStatus("hasHook %i",(int)Hook);
	}
	//show
	showSubmenuTimer.SetOwner(this, 13475);
	Bind(wxEVT_TIMER,[=](wxTimerEvent &evt){
		//wxLogStatus("show %i %i",submenuShown,sel);
		if(submenuShown == -1 || sel == -1){return;}//|| sel != submenuShown
		//if(submenuToHide != -1){hideSubmenuTimer.Start(200,true);}
		int scrollPos = submenuShown-scPos;
		wxSize size = GetSize();
		wxPoint pos = GetPosition();
		pos.x += size.x;
		pos.y += scrollPos * height;
		parent->items[submenuShown]->submenu->PopupMenu(pos, this, false);
		submenuToHide=submenuShown;
		subMenuIsShown=true;
	},13475);
	//hide
	hideSubmenuTimer.SetOwner(this, 13476);
	Bind(wxEVT_TIMER,[=](wxTimerEvent &evt){
		if(submenuToHide == -1){return;}
		//if(sel == submenuToHide){subMenuIsShown=true; return;}
		//wxLogStatus("Hidesubmenu timer %i",submenuToHide);
		MenuItem *olditem=parent->items[submenuToHide];
		if(olditem->submenu->dialog){
			//wxRect rc = olditem->submenu->dialog->GetScreenRect();
			//wxPoint pos = wxGetMousePosition();
			//if(rc.Contains(pos)){return;}
			if(lastActiveMenu==olditem->submenu->dialog){return;}
			wxRect rc1 = GetScreenRect();
			if(!rc1.Contains(pos)){sel=-1; Refresh(false);}
			else if (sel == submenuToHide){subMenuIsShown=true; return;}
			//olditem->submenu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND,1);
			olditem->submenu->DestroyDialog();
			MenuBar::Menubar->md = parent;
		}
		if(submenuShown != submenuToHide){showSubmenuTimer.Start(1,true);}
		submenuToHide=-1;
	},13476);
	//if(!ParentMenu->HasCapture()){CaptureMouse();}
	//this->AcceptFocus(false);
	//MenuBar::Menubar->activChild = this->parent;
	MenuBar::Menubar->md = parent;
}

MenuDialog::~MenuDialog()
{
	//if(HasCapture()){ReleaseMouse();}
	if(IsRunning()){
		Exit(0);
	}
	wxDELETE(bmp);
}

void MenuDialog::OnMouseEvent(wxMouseEvent &evt)
{
	bool leftdown=evt.LeftDown();
	int x=evt.GetX();
	int y=evt.GetY();
	
	int elem = y/height;

	if(evt.Leaving()){
		if(submenuToHide != -1 ){
			hideSubmenuTimer.Start(400,true);
			subMenuIsShown=false;
		}
		if(sel!=submenuToHide){sel=-1; Refresh(false);}
		
		return;
	}else if(evt.Entering()){
		if(submenuShown != -1 ){
			subMenuIsShown=true;
		}
		lastActiveMenu = this;
	}
	
	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -=step;
		if(scPos<0){scPos=0;}
		else if(scPos > (int)parent->items.size()-maxVisible){scPos = parent->items.size()-maxVisible;}
	}
	
	elem+=scPos;
	if(elem>=(int)parent->items.size() || elem < 0){return;}
	if(elem!=sel){
		MenuItem *item=parent->items[elem];
		sel=elem;
		if(subMenuIsShown){
			hideSubmenuTimer.Start(400,true);
			subMenuIsShown=false;
			//wxLogStatus("Hidesubmenu %i",submenuToHide);
		}
		//wxLogStatus("dialog %i", (item->submenu)? (int)item->submenu->dialog : 0);	
		if(item->submenu && item->submenu->dialog==NULL){// 
			submenuShown=elem;
			if(submenuToHide == -1){showSubmenuTimer.Start((leftdown)? 1 : 200,true);}// : (submenuToHide != -1)? 500
		}
		
		Refresh(false);
		if(item->help != ""){
			//wxLogStatus(item->help);
		}
	}
	
	if(evt.LeftUp()){
		MenuItem *item=parent->items[elem];
		SendEvent(item, evt.GetModifiers());
	}else if(leftdown && parent->items[elem]->submenu && submenuShown != elem){
		submenuShown=elem;
		showSubmenuTimer.Start(1,true);
	}
}

bool MenuDialog::SendEvent(MenuItem *item, int accel)
{
	if(!item->enabled){return false;}
	if(item->type == ITEM_CHECK){
		item->check = !item->check;
		Refresh(false);
		int evtid = (ParentMenu->isPartialModal)? ID_CHECK_EVENT : item->id;
		wxCommandEvent *evt= new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, evtid);
		evt->SetClientData(item);
		evt->SetInt(accel);
		wxQueueEvent(ParentMenu->GetParent(),evt);
		return true;
	}
	id = item->id;
	if(!ParentMenu->isPartialModal){
		wxCommandEvent *evt= new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, item->id);
		evt->SetClientData(item);
		evt->SetInt(accel);
		wxQueueEvent(ParentMenu->GetParent(),evt);
	}
		
	HideMenus();
	return true;
}
	
void MenuDialog::OnPaint(wxPaintEvent &event)
{
	int itemsize = parent->items.size();
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
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	wxBitmap checkbmp = wxBITMAP_PNG("check");
	wxBitmap dot = wxBITMAP_PNG("dot");
	wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
	wxColour graytext = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
	tdc.SetFont(font);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetTextForeground(text);
	wxSize mnbefsize;
	wxSize linesize;
	bool hasMnemonics=false;

	bool noRadio=true;
	for(int i=0;i<maxsize; i++)
	{
		int scrollPos=i+scPos;
		MenuItem *item=parent->items[scrollPos];//+scPos
		if(item->type==ITEM_SEPARATOR){
			tdc.SetPen(wxPen("#FFFFFF"));
			tdc.SetBrush(wxBrush("#000000"));
			tdc.DrawRectangle(30,height*i+10,w-36,1);
			noRadio=true;
			continue;
		}
		if(scrollPos==sel){
			tdc.SetPen(wxPen("#000000"));
			tdc.SetBrush(wxBrush(highlight));
			tdc.DrawRectangle(0, height*i,w,height);
		}
		//tdc.SetPen(wxPen("#497CB0",2));
		//tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		
		
		if(item->type==ITEM_CHECK && item->check){
			tdc.DrawBitmap(checkbmp,4,(height*i)+2);
		}else if(item->type==ITEM_RADIO && noRadio){
			tdc.DrawBitmap(dot,4,(height*i)+2);
			noRadio=false;
		}else if(item->icon){
			tdc.DrawBitmap(item->GetBitmap(),4,(height*i)+2);
		}
		wxString desc=item->GetLabel();
		tdc.SetTextForeground((item->enabled)? text : graytext);
		if(showMnemonics && desc.Find('&')!=-1){
			wxString rest;
			wxString beforemn = desc.BeforeFirst('&', &rest);
			mnbefsize = tdc.GetTextExtent(beforemn);
		
			linesize = tdc.GetTextExtent(rest[0]);
			hasMnemonics=true;
		}else{hasMnemonics=false;}
		
		desc.Replace("&","");
		int find=desc.find("\t");
		//tdc.SetPen(wxPen("#497CB0"));
		if (find!= -1 ){
			int fw, fhh;
			wxString accel=desc.AfterLast('\t');
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel,w-fw-20,(height*i)+2);
			desc=desc.BeforeLast('\t');
		}
		tdc.DrawText(desc,28,(height*i)+2);
		if(hasMnemonics){
			tdc.SetPen(wxPen(text));
			tdc.DrawLine(28+mnbefsize.x, (height*(i+1))-4, 28+mnbefsize.x+linesize.x, (height*(i+1))-4);
		}
		if(item->submenu){
			wxPoint points[3];
			int pos = w-10;
			int pos1= (height*i)+12;
			points[0]=wxPoint(pos,pos1-6);
			points[1]=wxPoint(pos,pos1);
			points[2]=wxPoint(pos+4,pos1-3);
			tdc.SetBrush(wxBrush(text));
			tdc.SetPen(wxPen(graytext));
			tdc.DrawPolygon(3,points);
		}

	}

	/*tdc.SetPen(wxPen("#FFFFFF"));
	tdc.SetBrush(wxBrush(graytext));
	tdc.DrawRectangle(22,6,1,h-12);*/
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}
	
void MenuDialog::OnScroll(wxScrollWinEvent& event)
{
	int newPos=0;
	int tsize=parent->items.size();
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

void MenuDialog::HideMenus()
{
	if(!ParentMenu){return;}
	
	int subMenu=ParentMenu->submenuToHide;
	//wxLogStatus("submenu");
	Menu *menu=ParentMenu->parent;
	while(subMenu!= -1 && menu->items[subMenu]->submenu){
		menu = menu->items[subMenu]->submenu;
		//wxLogStatus("menu %i, %i", subMenu, (int)menu);
		if(menu->dialog){subMenu = menu->dialog->submenuToHide;}
		else{subMenu = -1;break;}
		//menu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND,1);
		menu->DestroyDialog();
	}
	if(ParentMenu->isPartialModal){ParentMenu->EndPartialModal(0);}
	else{/*ParentMenu->HideWithEffect(wxSHOW_EFFECT_BLEND,1);*/ ParentMenu->parent->DestroyDialog();}
	ParentMenu=NULL;
	MenuBar::Menubar->md=NULL;
}

//void MenuDialog::OnLostCapture(wxMouseCaptureLostEvent &evt){
//	//if(HasCapture()){ReleaseMouse();}
//	
//	if(ParentMenu){HideMenus();}
//}
//Pokazuje okno menu dodaj¹c pêtlê czekaj¹c¹ do odwo³ania
int MenuDialog::ShowPartialModal()
{
	isPartialModal=true;
	//ShowWithEffect(wxSHOW_EFFECT_BLEND ,1);
	//Refresh(false);
	Show();
	//if((size_t)maxVisible < parent->items.size()){SetFocus();}
	//MenuBar::Menubar->md=parent;
	//wxLogStatus("partial show");
	if(IsShown()){
		Run();
	}
	//wxLogStatus("partial end");
	return id;
}
//Odwo³uje pêtlê czekaj¹c¹
void MenuDialog::EndPartialModal(int ReturnId)
{
	Exit(ReturnId);
	/*HideWithEffect(wxSHOW_EFFECT_BLEND ,1);*/
	Hide();
}

WXLRESULT MenuDialog::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) {
	
    if (message == 28 ) {
		ParentMenu->HideMenus();
		MenuBar::Menubar->HideMnemonics();
    }
    return wxFrame::MSWWindowProc(message, wParam, lParam);
}


BEGIN_EVENT_TABLE(MenuDialog, wxFrame)
	EVT_MOUSE_EVENTS(MenuDialog::OnMouseEvent)
	EVT_PAINT(MenuDialog::OnPaint)
	//EVT_MOUSE_CAPTURE_LOST(MenuDialog::OnLostCapture)
	EVT_SCROLLWIN(MenuDialog::OnScroll)
END_EVENT_TABLE()

static int menuIndent = 20;
static int halfIndent = 10;
MenuBar *MenuBar::Menubar = NULL;

MenuBar::MenuBar(wxWindow *_parent)
	:wxWindow(_parent, -1, wxDefaultPosition, wxSize(-1,height))
	,Mnemonics()
	,bmp(NULL)
	,sel(-1)
	,clicked(false)
	,md(NULL)
	,oldelem(-1)
	,shownMenu(-1)
{
	SetFont(font);
	Refresh(false);
	Bind(wxEVT_TIMER,[=](wxTimerEvent &event){
		if(shownMenu == -1){return;}
		MenuEvent evt(EVT_MENU_OPENED, GetId(), Menus[shownMenu]);
		ProcessEvent(evt);
		wxSize rc= GetClientSize();
		int posX=halfIndent;
		for(int i = 0; i < shownMenu; i++){
			Menu *menu = Menus[i];
			wxString desc=menu->GetTitle();
			desc.Replace("&","");
			wxSize te = GetTextExtent(desc);
			posX += te.x + menuIndent;
		}
		wxPoint pos1(posX, rc.y); 
		Menus[shownMenu]->PopupMenu(pos1, this->GetParent());
	},56432);
	showMenuTimer.SetOwner(this, 56432);
	Menubar=this;
	HookKey = NULL;
	HookKey = SetWindowsHookEx(WH_KEYBOARD, &OnKey, NULL,GetCurrentThreadId());
	HookMouse = NULL;
	HookMouse = SetWindowsHookEx(WH_GETMESSAGE, &OnMouseClick, NULL,GetCurrentThreadId());
}



void MenuBar::Append(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.push_back(menu);
	findMnemonics(title,Menus.size()-1);
}
	
void MenuBar::Prepend(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin(), menu);
	findMnemonics(title,0);
}
	
void MenuBar::Insert(int position, Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin()+position, menu);
	findMnemonics(title,position);
}

void MenuBar::OnMouseEvent(wxMouseEvent &evt)
{
	if(evt.LeftUp()){clicked = false; Refresh(false);}
	
	wxPoint pos = evt.GetPosition();
	int elem = CalcMousePos(&pos);
	if(evt.Leaving() && !md){
		sel=-1; Refresh(false); 
		return;
	}
	if(evt.Entering()){
		if(shownMenu!=-1 && Menus[shownMenu]->dialog==NULL){shownMenu=-1;}
		oldelem=-1;
		if(shownMenu == elem){oldelem=sel=elem;Refresh(false);return;}
	}
	if(elem != oldelem && elem != -1){
		oldelem=elem;
		sel=elem;
		Refresh(false);
		if(shownMenu!=-1){
			
			if(Menus[shownMenu]->dialog){Menus[shownMenu]->dialog->HideMenus();}
			shownMenu=elem;
			showMenuTimer.Start(200,true);
			
		}
		
	}
	
	if(evt.LeftDown() && elem>=0){
		clicked = true;
		Refresh(false);
		if(shownMenu==elem ){
			shownMenu = -1;
			return;
		}
		shownMenu=elem;
		showMenuTimer.Start(10,true);
	}
	
}
	
void MenuBar::OnPaint(wxPaintEvent &event)
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
	tdc.SetFont(font);
	tdc.GradientFillLinear(wxRect(0,0,w,h),
		wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE),
		wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR),wxTOP);
	tdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT)));
	tdc.DrawLine(0,h-1,w,h-1);
	int posX=halfIndent;
	wxSize mnbefsize;
	wxSize linesize;
	bool hasMnemonics=false;
	for(size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc=menu->GetTitle();
		
		
		if(showMnemonics && desc.Find('&')!=-1){
			wxString rest;
			wxString beforemn = desc.BeforeFirst('&', &rest);
			mnbefsize = tdc.GetTextExtent(beforemn);
		
			linesize = tdc.GetTextExtent(rest[0]);
			hasMnemonics=true;
		}else{hasMnemonics=false;}

		desc.Replace("&","");
		wxSize te = tdc.GetTextExtent(desc);
		
		if(i == sel){
			tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_HIGHLIGHT)));
			tdc.SetPen(wxPen(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_HIGHLIGHT)));
			tdc.DrawRoundedRectangle(posX-2, 2, te.x+4, h-4, 2.0);
			tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT)));
		}
		tdc.DrawText(desc,posX,2);//(h-te.y)/2.0
		if(hasMnemonics){tdc.DrawLine(posX+mnbefsize.x, h-4, posX+mnbefsize.x+linesize.x, h-4);}
		posX += te.x + menuIndent;
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

int MenuBar::CalcMousePos(wxPoint *pos)
{
	int posX=0;
	for(size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc=menu->GetTitle();
		desc.Replace("&","");
		wxSize te = GetTextExtent(desc);
		if(pos->x > posX && (posX + te.x + menuIndent) > pos->x){
			pos->x = posX, pos->y = posX + te.x + halfIndent;
			return i;
		}
		posX += te.x + menuIndent;
	}
	return -1;
}

MenuItem *MenuBar::FindItem(int id)
{
	MenuItem *item=NULL;
	for(auto menu : Menus){
		item = menu->FindItem(id);
		if (item) {return item;}
	}
	return NULL;
}
	
void MenuBar::Enable(int id, bool enable)
{
	MenuItem * item = FindItem(id);
	if(item){item->Enable(enable);}
	else{wxLogStatus("Cannot enable item with id %i", id);}
}

void MenuBar::AppendAccelerators(std::vector <wxAcceleratorEntry> *entries)
{
	for(size_t i = 0; i<Menus.size(); i++){
		Menus[i]->GetAccelerators(entries);
	}
}

LRESULT CALLBACK MenuBar::OnKey( int code, WPARAM wParam, LPARAM lParam ){
	
	if(code < 0)
	{ 
		CallNextHookEx(Menubar->HookKey, code, wParam, lParam);
		return 0;
	}
	if(wParam == VK_MENU && !(lParam & 1073741824)){//536870912 1073741824 lparam mówi nam o altup, który ma specjalny bajt 
		byte state[256];
		if(GetKeyboardState(state)==FALSE){return 0;}
		if(!(state[VK_LMENU]>1 && state[VK_LSHIFT]<2 && state[VK_RSHIFT]<2 && state[VK_LCONTROL]<2 && state[VK_RCONTROL]<2)){return 0;}
		if(Menubar->md && showMnemonics){return 1;}
		showMnemonics = !showMnemonics;
		//wxLogStatus("alt pressed");
		if(Menubar->md){
			//if(!(lParam & 536870912)){Menubar->md->dialog->HideMenus();}
			//else{
				Menubar->md->dialog->sel = (showMnemonics)? 0 : -1;
				Menubar->md->dialog->Refresh(false);
			//}
		}
		Menubar->sel = (showMnemonics)? 0 : -1;
		Menubar->Refresh(false);
		
		//secondAlt = !showMnemonics;
		return 1;
	}else if(showMnemonics){
		if( (wParam >= 0x41 && wParam <= 0x5A) && !(lParam & 2147483648)){//lparam mówi o keyup
			auto mn = (Menubar->md)? Menubar->md->mnemonics : Menubar->mnemonics;
			auto foundmnemonics = mn.find(wParam);
			//wxLogStatus("mnsize %i", mn.size());
			if(foundmnemonics != mn.end()){
				//wxLogStatus("letter up %i", (int)wParam);
				if (Menubar->md){
					//wxLogStatus("md");
					if(Menubar->md->items[foundmnemonics->second]->submenu){// 
						Menubar->md->dialog->sel = Menubar->md->dialog->submenuShown = foundmnemonics->second;
						if(Menubar->md->dialog->submenuToHide == -1){Menubar->md->dialog->showSubmenuTimer.Start(1,true);}
						//wxLogStatus("md submenu");
					}else{
						MenuItem *item=Menubar->md->items[foundmnemonics->second];
						if(!Menubar->md->dialog->SendEvent(item, 0)){return 1;}
						Menubar->HideMnemonics();
					}
				}else{
					//wxLogStatus("menubar");
					if(Menubar->shownMenu != -1 && Menubar->Menus[Menubar->shownMenu]->dialog){
						Menubar->Menus[Menubar->shownMenu]->dialog->HideMenus();
					}
					Menubar->sel = Menubar->shownMenu = foundmnemonics->second;
					Menubar->Refresh(false);
					Menubar->showMenuTimer.Start(10,true);
				}
				return 1;
			}
			
		}else if((wParam == VK_DOWN || wParam == VK_UP || wParam == VK_LEFT || wParam == VK_RIGHT) && !(lParam & 2147483648)){
			if (Menubar->md){
				if(wParam == VK_LEFT && Menubar->md->dialog != MenuDialog::ParentMenu){
					//wxLogStatus("md submenu strza³ka left right");
					//Menubar->md->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND,1);
					Menubar->md->DestroyDialog();
					MenuBar::Menubar->md = Menubar->md->parentMenu;
					Menubar->md->dialog->submenuToHide = -1;
					Menubar->md->dialog->subMenuIsShown = false;
					return 1;
				}else if(wParam == VK_RIGHT && Menubar->md->dialog->sel>=0 
					&& Menubar->md->items[Menubar->md->dialog->sel]->submenu){
					Menubar->md->dialog->submenuShown = Menubar->md->dialog->sel;
					if(Menubar->md->dialog->submenuToHide == -1){
						//wxLogStatus("md submenu strza³ka %i", Menubar->md->dialog->submenuToHide);
						Menubar->md->dialog->showSubmenuTimer.Start(1,true);
						return 1;
					}
				}
				else if(wParam == VK_DOWN || wParam == VK_UP){
					int step = (wParam == VK_DOWN)? 1 : -1;
					Menubar->md->dialog->sel += step;
					if(Menubar->md->dialog->sel >= (int)Menubar->md->items.size()){
						Menubar->md->dialog->sel=0;
					}else if(Menubar->md->dialog->sel<0){
						Menubar->md->dialog->sel = Menubar->md->items.size()-1;
					}
					Menubar->md->dialog->Refresh(false);
					return 1;
				}
			}

			if(wParam == VK_RIGHT || wParam == VK_LEFT){
				int step = (wParam == VK_RIGHT)? 1 : -1;
				Menubar->sel += step;
				//wxLogStatus("menubar strza³ka left right %i", Menubar->sel);
				if(Menubar->sel >= (int)Menubar->Menus.size()){
					Menubar->sel=0;
				}else if(Menubar->sel<0){
					Menubar->sel = Menubar->Menus.size()-1;
				}
				Menubar->Refresh(false);
				if(!Menubar->md){return 1;}
			}
			if(Menubar->shownMenu != -1 && Menubar->Menus[Menubar->shownMenu]->dialog){
				Menubar->Menus[Menubar->shownMenu]->dialog->HideMenus();
			}
			Menubar->shownMenu = Menubar->sel;
			Menubar->showMenuTimer.Start(1,true);
		
			return 1;
		}
	}
	if(wParam == VK_ESCAPE && Menubar->md){
		MenuDialog::ParentMenu->HideMenus();
		Menubar->HideMnemonics();
		return 1;
	}
	if(wParam == VK_RETURN && Menubar->md){
		if(Menubar->md->dialog->sel>=0){
			MenuItem *item = Menubar->md->items[Menubar->md->dialog->sel];
			if(!Menubar->md->dialog->SendEvent(item, 0)){return 1;}
			Menubar->HideMnemonics();
			return 1;
		}
	}
	


	return (Menubar->md)? 1 : CallNextHookEx(Menubar->HookKey, code, wParam, lParam);
}

LRESULT CALLBACK MenuBar::OnMouseClick( int code, WPARAM wParam, LPARAM lParam )
{
	if(code < 0)
	{ 
		CallNextHookEx(Menubar->HookMouse, code, wParam, lParam);
		return 0;
	}
	LPMSG msg = (LPMSG)lParam;
	if( msg->message == WM_MOUSEWHEEL ){
		POINT mouse;
		GetCursorPos (&mouse);
		HWND hWndPointed = WindowFromPoint(mouse);
		if (hWndPointed != NULL && msg->hwnd != hWndPointed){
			msg->hwnd=hWndPointed;
		}
		return 1;
	}
	if(showMnemonics || Menubar->md){
		if( msg->message == WM_LBUTTONDOWN || msg->message == WM_NCLBUTTONDOWN || 
			msg->message == WM_RBUTTONDOWN || msg->message == WM_NCRBUTTONDOWN){
			MenuDialog::id=-3;
			Menubar->HideMnemonics();
			if(!MenuDialog::ParentMenu){return 0;}
			int subMenu=MenuDialog::ParentMenu->submenuShown;
			Menu *menu= MenuDialog::ParentMenu->parent;
			wxPoint posOnScreen = wxGetMousePosition();
			bool contains=false;
			while(menu){
				if(menu->dialog){
					wxRect rc = menu->dialog->GetRect();
					//wxLogStatus("contains %i, %i %i %i %i %i", posOnScreen.x, posOnScreen.y, rc.x, rc.y, rc.GetRight(), rc.GetBottom());
					if(rc.Contains(posOnScreen)){
						contains=true;
					}
				
					subMenu = menu->dialog->submenuShown;
					menu = (subMenu != -1)? menu->items[subMenu]->submenu : NULL;
				}
				else{menu=NULL; subMenu = -1;}
			}
			//wxLogStatus("hide contains %i", (int)contains);
			if(contains) {return 0;}
			MenuDialog::ParentMenu->HideMenus();
			return 1;
		}
		if(msg->message == WM_MBUTTONUP || msg->message == WM_NCMBUTTONUP)
		{
			Menubar->HideMnemonics();
		}
	}

	return CallNextHookEx( 0, code, wParam, lParam );
}

//WXLRESULT MenuBar::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) {
//	
//    if (message == 28 ) {
//		wxLogStatus("message 28");
//		Menubar->HideMnemonics();
//    }
//    return wxWindow::MSWWindowProc(message, wParam, lParam);
//}

void MenuBar::HideMnemonics()
{
	//wxLogStatus("hide mnemonics");
	showMnemonics=false;
	sel=-1;
	Refresh(false);
}

BEGIN_EVENT_TABLE(MenuBar, wxWindow)
	EVT_MOUSE_EVENTS(MenuBar::OnMouseEvent)
	EVT_PAINT(MenuBar::OnPaint)
END_EVENT_TABLE()