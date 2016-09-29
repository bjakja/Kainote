
#include "Menu.h"
#include "Config.h"
//#include "Tabs.h"

static wxFont font = wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");

MenuItem::MenuItem(const wxString& _label, const wxString& _help, int _id, bool _enable, wxBitmap *_icon, Menu *Submenu, byte _type)
{
	icon=_icon; label=_label; id=_id; 
	enabled=_enable;type=_type;submenu=Submenu;
}
MenuItem::~MenuItem()
{
	if(submenu){delete submenu;}
}
bool MenuItem::Enable(bool enable)
{
	if(enabled!=enable){enabled=enable;return true;}
	return false;
}
wxBitmap MenuItem::GetBitmap()
{
	if(!enabled){
		return wxBitmap(icon->ConvertToImage().ConvertToGreyscale());
	}
	return *icon;
}

Menu::Menu()
	:dialog(NULL)
{
}

int Menu::GetPopupMenuSelection(const wxPoint &pos, wxWindow *parent)
{
	wxPoint npos= pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size);
	dialog = new MenuDialog(this, parent, npos, size);
	return dialog->ShowModal();
}

void Menu::PopupMenu(const wxPoint &pos, wxWindow *parent)
{
	wxPoint npos= pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size);
	dialog = new MenuDialog(this, parent, npos, size, false);
	dialog->ShowModal();
}

void Menu::CalcPosAndSize(wxWindow *parent, wxPoint *pos, wxSize *size)
{
	int tx=0, ty=0;
	for(size_t i = 0; i < items.size(); i++){
		parent->GetTextExtent(items[i]->label, &tx, &ty, 0, 0, &font);
		if(tx > size->x){size->x = tx;}
	}
	size->x += 20;
	size->y = ty * items.size() + 2;
	int w,h;
	wxRect workArea = wxGetClientDisplayRect();
	w = workArea.width - workArea.x;
	h = workArea.height - workArea.y;
	if(size->y > h){ size->y = h; }
	if((pos->x + size->x) > w){pos->x -= size->x;}
	if((pos->y + size->y) > h){
		if(size->y > h/2){pos->y -= size->y;}
		else{pos->y = h - size->y;}
	}
}

MenuItem *Menu::Append(const wxString& _label, const wxString& _help, int _id, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	MenuItem *item = new MenuItem(_label, _help, _id, _enable, _icon, Submenu, _type);
	items.push_back(item);
	return item;
}
	
MenuItem *Menu::Append(MenuItem *item)
{
	items.push_back(item);
	return item;
}
	
MenuItem *Menu::Prepend(const wxString& _label, const wxString& _help, int _id, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	MenuItem *item = new MenuItem(_label, _help, _id, _enable, _icon, Submenu, _type);
	items.insert(items.begin(),item);
	return item;
}
	
MenuItem *Menu::Prepend(MenuItem *item)
{
	items.insert(items.begin(),item);
	return item;
}
	
MenuItem *Menu::Insert(int position, const wxString& _label, const wxString& _help, int _id, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	MenuItem *item = new MenuItem(_label, _help, _id, _enable, _icon, Submenu, _type);
	items.insert(items.begin() + position,item);
	return item;
}
	
MenuItem *Menu::Insert(int position, MenuItem *item)
{
	items.insert(items.begin() + position,item);
	return item;
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

MenuDialog::MenuDialog(Menu *_parent, wxWindow *DialogParent, const wxPoint &pos, const wxSize &size, bool sendEvent)
	:wxDialog(DialogParent,-1,"",pos, size)
	,parent(_parent)
	,sel(-1)
	,scPos(0)
	,withEvent(sendEvent)
{
}

void MenuDialog::OnMouseEvent(wxMouseEvent &evt)
{
	bool leftdown=evt.LeftDown();
	int x=evt.GetX();
	int y=evt.GetY();
	int w=0;
	int h=0;
	GetSize (&w, &h);

	int elem = y/fh;
	elem+=scPos;
	if(elem>=(int)parent->items.size()){return;}
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
		MenuItem *item=parent->items[elem];
		if(withEvent){
			wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, item->id);
			evt.SetClientData(item);
			AddPendingEvent(evt);
		}
		EndModal(item->id);
	}
}
	
void MenuDialog::OnPaint(wxPaintEvent &event)
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
	wxBitmap checkbmp = wxBITMAP_PNG("check");
	tdc.SetFont(wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma"));
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.DrawRectangle(0,0,w,h);
	int visible=25;//500/fh fh=20;
	int itemsize = parent->items.size();
	if(scPos>=itemsize-visible){scPos=itemsize-visible;}
	else if(scPos<0){scPos=0;}
	int maxsize=MAX(itemsize,scPos+visible);
	SetScrollbar(wxVERTICAL,scPos,visible,itemsize);
	for(int i=0;i<visible; i++)
	{
		MenuItem *item=parent->items[i+scPos];
		
		if(i+scPos==sel){
			tdc.SetPen(wxPen("#000000"));
			tdc.SetBrush(wxBrush("#9BD7EE"));
			tdc.DrawRectangle(0, fh*i,w,fh);
		}
		tdc.SetPen(wxPen("#497CB0",2));
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		
		
		if(item->check){
			tdc.DrawBitmap(checkbmp,4,(fh*i)+2);
		}else{
			tdc.DrawBitmap(item->GetBitmap(),4,(fh*i)+2);
		}
		wxString desc=item->GetLabel();
		desc.Replace("&","");
		int find=desc.find("\t");
		tdc.SetPen(wxPen("#497CB0"));
		if (find!= -1 ){
			int fw, fhh;
			wxString accel=desc.AfterLast('\t');
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel,w-fw-8,(fh*i)+2);
		}
		wxString label=desc.BeforeLast('\t');
		tdc.DrawText(label,(fh*2),(fh*i)+2);
		

	}
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
	
void MenuDialog::OnLostCapture(wxFocusEvent &evt)
{
	EndModal(-3);
}

BEGIN_EVENT_TABLE(MenuDialog, wxDialog)
	EVT_MOUSE_EVENTS(MenuDialog::OnMouseEvent)
	EVT_PAINT(MenuDialog::OnPaint)
	EVT_KILL_FOCUS(MenuDialog::OnLostCapture)
	EVT_SCROLLWIN(MenuDialog::OnScroll)
END_EVENT_TABLE()

MenuBar::MenuBar(wxWindow *_parent)
	:wxWindow(_parent, -1, wxDefaultPosition, wxSize(-1,20))
	,parent(_parent)
	,bmp(NULL)
	,sel(-1)
	,clicked(false)
	,oldelem(-1)
{
	SetFont(font);
}

void MenuBar::Append(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.push_back(menu);
}
	
void MenuBar::Prepend(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin(), menu);
}
	
void MenuBar::Insert(int position, Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin()+position, menu);
}

void MenuBar::OnMouseEvent(wxMouseEvent &evt)
{
	wxPoint pos = evt.GetPosition();
	int elem = CalcMousePos(&pos);
	if(elem != oldelem){
		sel=elem;
	}
	if(evt.LeftDown()){
		wxSize rc= GetClientSize();
		wxPoint pos(pos.x, rc.y); 
		Menus[elem]->PopupMenu(pos, this);
	}
	oldelem=elem;
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
	//tdc.SetFont(font);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT)));
	int posX=5;
	for(size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc=menu->GetTitle();
		desc.Replace("&","");
		wxSize te = tdc.GetTextExtent(desc);
		if(i==(int)sel){
			tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_MENUHILIGHT)));
			tdc.SetPen(wxPen(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_HIGHLIGHT)));
			tdc.DrawRoundedRectangle(posX-2, 2, te.x+4, h-4, 2.0);
			tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT)));
		}
		tdc.DrawText(desc,posX,(h-te.y)/2.0);
		posX += te.x + 5;
	}
}

int MenuBar::CalcMousePos(wxPoint *pos)
{
	int posX=5;
	for(size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc=menu->GetTitle();
		desc.Replace("&","");
		wxSize te = GetTextExtent(desc);
		if(posX > pos->x && posX + te.x + 5 < pos->x){
			pos->x = posX, pos->y = posX + te.x + 5;
			return i;
		}
		posX += te.x + 5;
	}
	return -1;
}

BEGIN_EVENT_TABLE(MenuBar, wxWindow)
	EVT_MOUSE_EVENTS(MenuBar::OnMouseEvent)
	EVT_PAINT(MenuBar::OnPaint)
END_EVENT_TABLE()