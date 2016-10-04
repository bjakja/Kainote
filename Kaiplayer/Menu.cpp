
#include "Menu.h"
#include "Config.h"
#include "Hotkeys.h"

static wxFont font = wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");
static int height = 18;

MenuItem::MenuItem(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu *Submenu, byte _type)
{
	icon=_icon; label=_label; id=_id; 
	enabled=_enable;type=_type;submenu=Submenu; check=false;
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
	if(!icon){return wxBitmap();}
	if(!enabled){
		return wxBitmap(icon->ConvertToImage().ConvertToGreyscale());
	}
	return *icon;
}

Menu::Menu()
	:dialog(NULL)
{
}

int Menu::GetPopupMenuSelection(const wxPoint &pos, wxWindow *parent, bool clientPos)
{
	wxPoint npos= pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	wxLogStatus("Pos & size %i, %i %i %i %i %i", npos.x, npos.y, size.x, size.y, pos.x, pos.y);
	//if(dialog){dialog->Destroy();}
	dialog = new MenuDialog(this, parent, npos, size, false);
	return dialog->ShowModal();
}

void Menu::PopupMenu(const wxPoint &pos, wxWindow *parent, bool clientPos)
{
	wxPoint npos= pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	wxLogStatus("Pos & size %i, %i %i %i %i %i", npos.x, npos.y, size.x, size.y, pos.x, pos.y);
	//if(dialog){dialog->Destroy();}
	dialog = new MenuDialog(this, parent, npos, size);
	dialog->Show();
}

void Menu::CalcPosAndSize(wxWindow *parent, wxPoint *pos, wxSize *size, bool clientPos)
{
	int tx=0, ty=0;
	if(clientPos){*pos = parent->ClientToScreen(*pos);}
	for(size_t i = 0; i < items.size(); i++){
		parent->GetTextExtent(items[i]->label, &tx, &ty, 0, 0, &font);
		if(tx > size->x){size->x = tx;}
	}
	size->x += 50;
	size->y = height * items.size() + 2;
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

MenuItem *Menu::Append(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	return item;
}
MenuItem *Menu::Append(int _id,const wxString& _label, Menu* Submenu, const wxString& _help, byte _type, bool _enable, wxBitmap *_icon)
{
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	return item;
}
	
MenuItem *Menu::Append(MenuItem *item)
{
	items.push_back(item);
	return item;
}
	
MenuItem *Menu::Prepend(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.insert(items.begin(),item);
	return item;
}
	
MenuItem *Menu::Prepend(MenuItem *item)
{
	items.insert(items.begin(),item);
	return item;
}
	
MenuItem *Menu::Insert(int position, int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
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
	if(dialog){dialog->Destroy();}//if(dialog->IsModal()){dialog->EndModal(-3);}else{}
}

MenuItem *Menu::SetAccMenu(int id, const wxString &txt, const wxString &help, bool enable, int kind)
{
	wxString hkey=Hkeys.GetMenuH(id);
	wxString mtext=(hkey!="")? txt.BeforeFirst('\t')+"\t"+hkey : txt;
	if(hkey!="" && Hkeys.hkeys[id].Name==""){Hkeys.hkeys[id].Name=txt.BeforeFirst('\t');}
	return Append(id,mtext,help,true,0,0,kind);
}

wxDEFINE_EVENT(EVT_SHOW_DIAL, wxThreadEvent);
MenuDialog::MenuDialog(Menu *_parent, wxWindow *DialogParent, const wxPoint &pos, const wxSize &size, bool sendEvent)
	:wxDialog(DialogParent,-1,"",pos, size, 0)
	,parent(_parent)
	,sel(-1)
	,scPos(0)
	,withEvent(sendEvent)
	,blockHideDialog(false)
	,submenuShown(-1)
	,bmp(NULL)
	,id(-3)
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
	int elem = y/height;
	
	if(evt.Leaving()){
		wxLogStatus("leaving");
		//if(!HasCapture()){CaptureMouse();}
		//else if(evt.RightDown()){SetPosition(wxGetMousePosition());}
		sel=-1; Refresh(false);return;
	}else if(evt.Entering()){
		wxLogStatus("entering");
		
		//if(HasCapture()){ReleaseMouse();}
		sel=elem; Refresh(false);return;
	}
	//if(evt.ButtonDown() && HasCapture()){
		//ReleaseMouse();if(IsModal()){EndModal(-3);}else{id=-3;Hide();}
	//}
	//if(evt.Leaving()){sel=-1; Refresh(false);return;}
	
	//elem+=scPos;
	if(elem>=(int)parent->items.size() || elem < 0){return;}
	if(elem!=sel){
		
			
			if(submenuShown!=-1){
				MenuItem *olditem=parent->items[submenuShown];
				blockHideDialog=false;
				olditem->submenu->DestroyDialog();
				SetFocus();
				submenuShown=-1;
				//if(IsModal() && !HasCapture()){CaptureMouse();}
			}
			
		
		MenuItem *item=parent->items[elem];
		sel=elem;
		if(item->submenu){
			blockHideDialog=true;
			//if(IsModal() && HasCapture()){ReleaseMouse();}
			wxPoint pos = GetPosition();
			pos.x += w;
			pos.y += elem * height;
			//if(IsModal()){item->submenu->GetPopupMenuSelection(pos, this->GetParent(), false);}
			//else{
			item->submenu->PopupMenu(pos, this->GetParent(), false);//}
			submenuShown=elem;
		}
		
		Refresh(false);
		//wxLogStatus(item->help);
	}
	/*if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -=step;
		Refresh(false);
		return;
	}*/
	if(leftdown){
		MenuItem *item=parent->items[elem];
		if(!item->enabled){return;}
		if(item->type == ITEM_CHECK){
			item->check = !item->check;
			Refresh(false);
			return;
		}
		if(withEvent){
			wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, item->id);
			evt.SetClientData(item);
			AddPendingEvent(evt);
		}
		id = item->id;
		if(IsModal()){EndModal(id);}
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
	wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
	wxColour graytext = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
	tdc.SetFont(font);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetTextForeground(text);
	//wxLogStatus("refresh");
	//int visible=25;//500/fh fh=20;
	int itemsize = parent->items.size();
	/*if(scPos>=itemsize-visible){scPos=itemsize-visible;}
	else if(scPos<0){scPos=0;}
	int maxsize=MAX(itemsize,scPos+visible);
	SetScrollbar(wxVERTICAL,scPos,visible,itemsize);*/
	for(int i=0;i<itemsize; i++)
	{
		MenuItem *item=parent->items[i];//+scPos
		if(item->type==ITEM_SEPARATOR){
			tdc.SetPen(wxPen("#FFFFFF"));
			tdc.SetBrush(wxBrush("#000000"));
			tdc.DrawRectangle(24,height*i+10,w-4,1);
			continue;
		}
		if(i/*+scPos*/==sel){
			tdc.SetPen(wxPen("#000000"));
			tdc.SetBrush(wxBrush(highlight));
			tdc.DrawRectangle(0, height*i,w,height);
		}
		tdc.SetPen(wxPen("#497CB0",2));
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		
		
		if(item->check){
			tdc.DrawBitmap(checkbmp,4,(height*i)+2);
		}else if(item->icon){
			tdc.DrawBitmap(item->GetBitmap(),4,(height*i)+2);
		}
		wxString desc=item->GetLabel();
		tdc.SetTextForeground((item->enabled)? text : graytext);
		desc.Replace("&","");
		int find=desc.find("\t");
		tdc.SetPen(wxPen("#497CB0"));
		if (find!= -1 ){
			int fw, fhh;
			wxString accel=desc.AfterLast('\t');
			//wxLogStatus("accel "+accel);
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel,w-fw-20,(height*i)+2);
			desc=desc.BeforeLast('\t');
		}
		//wxLogStatus("desc1 "+label);
		tdc.DrawText(desc,24,(height*i)+2);
		
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
void MenuDialog::OnLostCapture(wxMouseCaptureLostEvent &evt)
{
	if(HasCapture()){ReleaseCapture();}
	//if(IsModal()){}EndModal(-3);}
	//else{Hide(); id=-3;}
}


BEGIN_EVENT_TABLE(MenuDialog, wxDialog)
	EVT_MOUSE_EVENTS(MenuDialog::OnMouseEvent)
	EVT_PAINT(MenuDialog::OnPaint)
	EVT_MOUSE_CAPTURE_LOST(MenuDialog::OnLostCapture)
	//EVT_SCROLLWIN(MenuDialog::OnScroll)
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