//  Copyright (c) 2016 - 2020, Marcin Drob

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
#include "Hotkeys.h"
#include "Config.h"
#include "UtilsWindows.h"
#include "Toolbar.h"
#include "KaiScrollbar.h"
#include "wx/msw/private.h"
#include <wx/msw/winundef.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include "KainoteApp.h"
#include <locale>

//cannot use font outside class cause it create before it can get font scale from system
static int height = 22;
static bool showMnemonics = false;
static bool secondAlt = false;
static bool showIcons = true;
static int selectOnStart = -1;
static int minWidth = 0;

wxDEFINE_EVENT(EVT_MENU_OPENED, MenuEvent);

bool MenuSort(MenuItem* i, MenuItem* j) {
	
	const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t>>(KainoteFrame::GetLocale());
	const wchar_t* s1 = i->label.wc_str();
	const wchar_t* s2 = j->label.wc_str();
	return (f.compare(&s1[0], &s1[0] + wcslen(s1),
		&s2[0], &s2[0] + wcslen(s2)) < 0);
}

void Mnemonics::findMnemonics(const wxString &label, int pos){
	int result = label.Find(L'&');
	if (result != -1){
		wxString rawmnemonics = label.Mid(result + 1, 1);
		wchar_t mn = rawmnemonics.Upper()[0];
		mnemonics[mn] = pos;
	}
}

MenuItem::MenuItem(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu *Submenu, byte _type)
{
	icon = _icon;
	label = _label;
	id = _id;
	enabled = _enable;
	type = _type;
	submenu = Submenu;
	if (Submenu){ disableMapping = true; }
	check = false;
	help = _help;
	//accel=nullptr;
}
MenuItem::~MenuItem()
{
	if (submenu){ delete submenu; }
	if (icon){ delete icon; }
}
bool MenuItem::Enable(bool enable)
{
	if (enabled != enable){ enabled = enable; return true; }
	return false;
}
wxBitmap MenuItem::GetBitmap()
{
	if (!icon){ return wxBitmap(); }
	if (!enabled){
		return wxBitmap(icon->ConvertToImage().ConvertToGreyscale());
	}
	return *icon;
}

void MenuItem::SetAccel(wxAcceleratorEntry *entry, const wxString &stringAccel /*= ""*/)
{
	if (label.find(L"\t") != -1){ label = label.BeforeFirst(L'\t'); }
	label += L"\t";
	label += (entry && entry->IsOk()) ? entry->ToString() : stringAccel;
	label.Replace(L"+", L"-");
}

wxString MenuItem::GetAccel(){
	if (label.find(L"\t") == -1)
		return emptyString;

	return label.AfterFirst(L'\t');
}

Menu::Menu(char window)
	:Mnemonics()
	, dialog(nullptr)
	, parentMenu(nullptr)
	, wnd(window)
{
}


int Menu::GetPopupMenuSelection(const wxPoint &pos, wxWindow *parent, int *accels, bool clientPos, bool center)
{
	if (MenuDialog::ParentMenu){
		MenuDialog::ParentMenu->HideMenus(-5);
	}
	wxPoint npos = pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	if (center){ npos.x -= (size.x / 2); }
	showMnemonics = true;

	dialog = new MenuDialog(this, parent, npos, size, false);
	int ret = dialog->ShowPartialModal();
	if (dialog && ret > -5){
		if (accels){ *accels = dialog->accel; }
		dialog->Destroy();
		dialog = nullptr;
	}
	return ret;
}

void Menu::PopupMenu(const wxPoint &pos, wxWindow *parent, bool clientPos, bool center)
{
	if (dialog){ return; }
	wxPoint npos = pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	if (center){ npos.x -= (size.x / 2); }

	dialog = new MenuDialog(this, parent, npos, size, showIcons);
	dialog->Show();
}

void Menu::Sort(int offset)
{
	
	std::sort(items.begin() + offset, items.end(), MenuSort);
}

void Menu::CalcPosAndSize(wxWindow *parent, wxPoint *pos, wxSize *size, bool clientPos)
{
	int tx = 0, ty = 0;
	if (clientPos){ *pos = parent->ClientToScreen(*pos); }
	size_t isize = items.size();
	for (size_t i = 0; i < isize; i++){
		parent->GetTextExtent(items[i]->label, &tx, &ty, 0, 0, &MenuBar::font);
		if (tx > size->x){ size->x = tx; }
	}

	size->x += (showIcons) ? 58 : 20;
	if (minWidth && minWidth > size->x){ size->x = minWidth; }
	if (isize > (size_t)maxVisible) { size->x += 20; isize = maxVisible; }
	size->y = height * isize + 4;
	int w, h;
	wxRect workArea = GetMonitorWorkArea(0, nullptr, wxPoint(pos->x + size->x, pos->y), true);
	w = workArea.width + workArea.x;
	h = workArea.height + workArea.y;
	//It was probably the last bug of this element or maybe there are some wonder monitors combination
	//that make it failed in the future
	//KaiLog(wxString::Format("workarea x %i %i y %i %i pos %i, size %i", workArea.x, workArea.width, workArea.y, workArea.height, pos->x + size->x, pos->y));
	if (size->y > workArea.height){
		size->y = workArea.height;
		maxVisible = size->y / height;
		size->x += 20;
	}
	if ((pos->x + size->x) > w){
		pos->x -= size->x;
		if (parentMenu && parentMenu->dialog){
			wxSize size = parentMenu->dialog->GetClientSize();
			pos->x -= size.x;
		}
	}
	if ((pos->y + size->y) > h){
		if (size->y > h / 2){
			pos->y -= size->y;
			if (pos->y < workArea.y){ pos->y = workArea.y; }
		}
		else{
			pos->y = h - size->y;
		}
	}

}

void Menu::SetMaxVisible(byte _maxVisible)
{
	maxVisible = _maxVisible;
}

void Menu::SetShowIcons(bool _showIcons)
{
	showIcons = _showIcons;
}

void Menu::SelectOnStart(int numitem)
{
	selectOnStart = numitem;
}

void Menu::SetMinWidth(int width)
{
	minWidth = width;
}

MenuItem *Menu::AppendTool(KaiToolbar *ktb, int id, wxString text, wxString help, wxBitmap *bitmap, bool enable, Menu *SubMenu)
{
	if (bitmap && bitmap->IsOk()){ ktb->AddID(id); }
	return Append(id, text, help, enable, bitmap, SubMenu);
}

MenuItem *Menu::Append(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if (Submenu){ Submenu->parentMenu = this; }
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	findMnemonics(_label, items.size() - 1);
	return item;
}
MenuItem *Menu::Append(int _id, const wxString& _label, Menu* Submenu, const wxString& _help, byte _type, bool _enable, wxBitmap *_icon)
{
	if (Submenu){ Submenu->parentMenu = this; }
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	findMnemonics(_label, items.size() - 1);
	return item;
}

MenuItem *Menu::Append(MenuItem *item)
{
	if (item->submenu){ item->submenu->parentMenu = this; }
	items.push_back(item);
	findMnemonics(item->label, items.size() - 1);
	return item;
}

MenuItem *Menu::Prepend(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if (Submenu){ Submenu->parentMenu = this; }
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.insert(items.begin(), item);
	findMnemonics(_label, 0);
	return item;
}

MenuItem *Menu::Prepend(MenuItem *item)
{
	if (item->submenu){ item->submenu->parentMenu = this; }
	items.insert(items.begin(), item);
	findMnemonics(item->label, 0);
	return item;
}

MenuItem *Menu::Insert(int position, int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if (Submenu){ Submenu->parentMenu = this; }
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.insert(items.begin() + position, item);
	findMnemonics(_label, position);
	return item;
}

MenuItem *Menu::Insert(int position, MenuItem *item)
{
	if (item->submenu){ item->submenu->parentMenu = this; }
	items.insert(items.begin() + position, item);
	findMnemonics(item->label, items.size() - 1);
	return item;
}

bool Menu::Destroy(MenuItem *item)
{
	for (size_t i = 0; i < items.size(); i++){
		if (items[i] == item){
			delete item;
			items.erase(items.begin() + i);
			return true;
		}
		else if (items[i]->submenu){
			if (items[i]->submenu->Destroy(item)){ return true; }
		}

	}
	delete item;
	return false;
}

void Menu::Delete(int position)
{
	delete items[position];
	items.erase(items.begin() + position);
}

int Menu::GetMenuItemCount()
{
	return items.size();
}

MenuItem *Menu::FindItem(int id, Menu **menu)
{
	for (size_t i = 0; i < items.size(); i++){
		if (items[i]->id == id){
			if (menu)
				*menu = this;
			return items[i];
		}
		else if (items[i]->submenu){
			MenuItem * item = items[i]->submenu->FindItem(id, menu);
			if (item){ return item; }
		}
	}
	return nullptr;
}

MenuItem *Menu::FindItem(const wxString& label)
{
	for (size_t i = 0; i < items.size(); i++){
		if (items[i]->label == label){
			return items[i];
		}
		else if (items[i]->submenu){
			MenuItem * item = items[i]->submenu->FindItem(label);
			if (item){ return item; }
		}

	}
	return nullptr;
}

MenuItem *Menu::FindItemGlobally(int id, Menu **menu)
{
	if (MenuDialog::ParentMenu && MenuDialog::ParentMenu->parent){
		return MenuDialog::ParentMenu->parent->FindItem(id, menu);
	}
	return nullptr;
}

MenuItem *Menu::FindItemByPosition(int pos)
{
	if (pos > (int)items.size() || pos < 0){ return nullptr; }
	return items[pos];
}

void Menu::Check(int id, bool check)
{
	MenuItem * item = FindItem(id);
	if (item){
		item->check = check;
	}
}

void Menu::AppendSeparator()
{
	MenuItem *item = new MenuItem(-2, emptyString, emptyString, false, 0, 0, ITEM_SEPARATOR);
	items.push_back(item);
}

void Menu::DestroyDialog()
{
	if (dialog){
		dialog->Destroy(); dialog = nullptr;
	}
}

void Menu::RefreshMenu()
{
	if (dialog){ dialog->Refresh(false); }
}

MenuItem *Menu::SetAccMenu(int id, const wxString &txt, const wxString &help, bool enable, int kind)
{
	idAndType itype(id, wnd);
	wxString txtcopy = txt;
	txtcopy.Replace(L"&", emptyString);
	wxString hkey = Hkeys.GetStringHotkey(itype, txtcopy);
	wxString mtext = (hkey != emptyString) ? txt.BeforeFirst(L'\t') + L"\t" + hkey : txt;
	return Append(id, mtext, help, enable, 0, 0, kind);
}

MenuItem *Menu::SetAccMenu(MenuItem *menuitem, const wxString &name)
{
	int id = 0;
	const std::map<idAndType, hdata> &hkeys = Hkeys.GetHotkeysMap();
	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++)
	{
		if (cur->first < 30100){ continue; }
		if (cur->second.Name == name){
			id = cur->first.id;
		}
	}

	wxAcceleratorEntry entry = Hkeys.GetHKey(id);
	if (id){ menuitem->SetAccel(&entry);/*menuitem->id=id;*/ }
	return Append(menuitem);
}

//void Menu::GetAccelerators(std::vector <wxAcceleratorEntry> *entries)
//{
//	for(auto item : items){
//		if(item->submenu){
//			item->submenu->GetAccelerators(entries);
//		}else if(item->accel){
//			entries->push_back(*item->accel);
//		}
//
//	}
//}

MenuDialog* MenuDialog::ParentMenu = nullptr;
MenuDialog* MenuDialog::lastActiveMenu = nullptr;


MenuDialog::MenuDialog(Menu *_parent, wxWindow *DialogParent, const wxPoint &pos, const wxSize &size, bool sendEvent)
	:wxPopupWindow(DialogParent)/*wxFrame(DialogParent,-1,"",pos, size, wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP|wxWS_EX_TRANSIENT)*/
	, parent(_parent)
	, sel(selectOnStart)
	, scPos(0)
	, subMenuIsShown(false)
	, isPartialModal(false)
	, submenuShown(-1)
	, submenuToHide(-1)
	, bmp(nullptr)
	, scroll(nullptr)
	, accel(0)
	, loop(nullptr)
{
	if (!ParentMenu){
		ParentMenu = this;
	}
	//show
	showSubmenuTimer.SetOwner(this, 13475);
	Bind(wxEVT_TIMER, &MenuDialog::OnShowSubmenu, this, 13475);
	//hide
	hideSubmenuTimer.SetOwner(this, 13476);
	Bind(wxEVT_TIMER, &MenuDialog::OnHideSubmenu, this, 13476);
	//if(!ParentMenu->HasCapture()){CaptureMouse();}
	//this->AcceptFocus(false);
	//MenuBar::Menubar->activChild = this->parent;
	MenuBar::Menubar->md = parent;
	Bind(wxEVT_LEFT_DOWN, &MenuDialog::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &MenuDialog::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &MenuDialog::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &MenuDialog::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &MenuDialog::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &MenuDialog::OnMouseEvent, this);
	//Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent &evt){evt.}, this);
	//BringWindowToTop(GetHWND());
	SetSize(size);
	SetPosition(pos);
}

MenuDialog::~MenuDialog()
{
	if (loop && loop->IsRunning()){
		//-5 is safe retcode for destroyed windows, it's will not destroy again
		loop->Exit(-5);
		//pod ¿adnym pozorem nie niszcz loop bo to gówno przestaje dzia³aæ dopiero gdy wszystkie funkcje siê wykonaj¹
	}
	wxDELETE(bmp);
}

void MenuDialog::OnShowSubmenu(wxTimerEvent &evt)
{
	if (submenuShown == -1 || sel == -1 || sel != submenuShown || submenuShown == submenuToHide){ return; }//

	int scrollPos = submenuShown - scPos;
	wxSize size = GetSize();
	int x, y;
	wxPopupWindowBase::DoGetPosition(&x, &y);
	x += size.x;
	y += scrollPos * height;
	parent->items[submenuShown]->submenu->PopupMenu(wxPoint(x, y), GetParent(), false);
	submenuToHide = submenuShown;
	subMenuIsShown = true;
	selectOnStart = -1;


}

void MenuDialog::OnHideSubmenu(wxTimerEvent &evt)
{
	if (submenuToHide == -1){ return; }
	MenuItem *olditem = parent->items[submenuToHide];

	if (olditem->submenu->dialog){
		wxPoint pos = wxGetMousePosition();
		if (lastActiveMenu == olditem->submenu->dialog){ return; }
		wxRect rc1 = GetScreenRect();
		if (!rc1.Contains(pos)){ sel = -1; Refresh(false); }
		else if (sel == submenuToHide){ subMenuIsShown = true; return; }
		//olditem->submenu->DestroyDialog();
		MenuBar::Menubar->md = parent;
		int subMenu = submenuToHide;
		Menu *menu = olditem->submenu;
		while (1){
			if (menu->dialog){
				subMenu = menu->dialog->submenuToHide;
				menu->DestroyDialog();
			}
			else{ subMenu = -1; break; }
			if (subMenu == -1)
				break;

			menu = menu->items[subMenu]->submenu;
		}
	}
	if (submenuShown != submenuToHide){ submenuToHide = -1; showSubmenuTimer.Start(1, true); return; }
	submenuToHide = -1;
}

void MenuDialog::DoGetPosition(int *x, int *y) const
{
	wxWindow::DoGetPosition(x, y);
}


void MenuDialog::OnMouseEvent(wxMouseEvent &evt)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	bool leftdown = evt.LeftDown();
	int x = evt.GetX();
	int y = evt.GetY();
	int elem = y / height;

	if (evt.Leaving() || y >= h - 4){
		if (submenuToHide != -1){
			hideSubmenuTimer.Start(400, true);
			subMenuIsShown = false;
		}
		if (sel != submenuToHide){ sel = -1; Refresh(false); }
		KainoteFrame * frame = ((kainoteApp*)wxTheApp)->Frame;
		frame->SetStatusText(emptyString, 0);
		return;
	}
	else if (evt.Entering()){
		if (submenuShown != -1){
			subMenuIsShown = true;
		}
		lastActiveMenu = this;
	}

	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -= step;
		if (scPos<0){ scPos = 0; }
		else if (scPos >(int)parent->items.size() - parent->maxVisible){ scPos = parent->items.size() - parent->maxVisible; }
	}

	elem += scPos;
	if (elem >= (int)parent->items.size() || elem < 0){ return; }
	MenuItem *item = parent->items[elem];
	if (leftdown && item->type == ITEM_CHECK_AND_HIDE){
		item->check = !item->check;
		Refresh(false);
	}
	if (subMenuIsShown && elem != submenuToHide){
		hideSubmenuTimer.Start(400, true);
		subMenuIsShown = false;
	}
	if (elem != sel){
		sel = elem;
		if (item->submenu && item->submenu->dialog == nullptr && item->enabled){
			submenuShown = elem;
			if (submenuToHide == -1){ showSubmenuTimer.Start((leftdown) ? 1 : 200, true); }
		}

		Refresh(false);
		KainoteFrame * frame = ((kainoteApp*)wxTheApp)->Frame;
		frame->SetStatusText(item->help, 0);
	}
	if (evt.Entering()){
		KainoteFrame * frame = ((kainoteApp*)wxTheApp)->Frame;
		frame->SetStatusText(item->help, 0);
	}
	if (evt.LeftUp() && !item->submenu){
		SendEvent(item, evt.GetModifiers());
	}
	else if (leftdown && item->submenu && item->submenu->dialog == nullptr && item->enabled && submenuShown != elem){
		submenuShown = elem;
		showSubmenuTimer.Start(1, true);
	}
}

bool MenuDialog::SendEvent(MenuItem *item, int accel)
{
	if (!ParentMenu){ Destroy(); return false; }
	if (!item->enabled && accel != wxMOD_SHIFT){ return false; }
	if (item->disableMapping && accel == wxMOD_SHIFT){ return false; }

	if (item->type == ITEM_CHECK && accel != wxMOD_SHIFT){
		item->check = !item->check;
		Refresh(false);
		int evtid = (ParentMenu->isPartialModal) ? ID_CHECK_EVENT : item->id;
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, evtid);
		evt->SetClientData(item);
		evt->SetInt(accel);
		wxQueueEvent(ParentMenu->GetParent(), evt);
		return true;
	}

	if (!ParentMenu->isPartialModal){
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, item->id);
		evt->SetClientData(item);
		evt->SetInt(accel);
		wxQueueEvent(ParentMenu->GetParent(), evt);
	}
	else{
		ParentMenu->accel = accel;
	}
	HideMenus(item->id);
	return true;
}

void MenuDialog::OnPaint(wxPaintEvent &event)
{

	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	int ow = w;
	int itemsize = parent->items.size();
	if (scPos >= itemsize - parent->maxVisible){ scPos = itemsize - parent->maxVisible; }
	if (scPos < 0){ scPos = 0; }
	int maxsize = itemsize;
	//if(sel<scPos && sel!=-1){scPos=sel;}
	//else if(sel>= scPos + maxVisible && (sel-maxVisible+1) >= 0){scPos=sel-maxVisible+1;}
	if (itemsize > parent->maxVisible){
		maxsize = parent->maxVisible;
		if (!scroll){
			int thickness = KaiScrollbar::CalculateThickness(this);
			scroll = new KaiScrollbar(this, -1, wxPoint(w - thickness - 1, 1), wxSize(thickness, h - 2), wxVERTICAL);
			scroll->SetScrollRate(3);
		}
		scroll->SetScrollbar(scPos, parent->maxVisible, itemsize, parent->maxVisible - 1);
		w -= (scroll->GetThickness() + 1);
	}


	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < ow || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = nullptr;
	}
	if (!bmp){ bmp = new wxBitmap(ow, h); }
	tdc.SelectObject(*bmp);
	wxBitmap checkbmp = wxBITMAP_PNG(L"check");
	wxBitmap dot = wxBITMAP_PNG(L"dot");
	wxBitmap separator = wxBITMAP_PNG(L"separator");
	wxBitmap arrow = wxBITMAP_PNG(L"arrow");
	const wxColour & highlight = Options.GetColour(MENU_BORDER_SELECTION);
	const wxColour & text = Options.GetColour(WINDOW_TEXT);
	const wxColour & graytext = Options.GetColour(WINDOW_TEXT_INACTIVE);
	const wxColour & background = Options.GetColour(MENUBAR_BACKGROUND);
	const wxColour & menuhighlight = Options.GetColour(MENU_BACKGROUND_SELECTION);
	tdc.SetFont(MenuBar::font);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(Options.GetColour(WINDOW_BORDER)));
	tdc.DrawRectangle(0, 0, ow, h);
	tdc.SetTextForeground(text);
	wxSize mnbefsize;
	wxSize linesize;
	bool hasMnemonics = false;

	bool noRadio = true;

	int textStart = (showIcons) ? 29 : 5;
	for (int i = 0; i < maxsize; i++)
	{
		int scrollPos = i + scPos;
		int posY = (height * i) + 2;
		MenuItem *item = parent->items[scrollPos];//+scPos
		if (item->type == ITEM_SEPARATOR){
			wxImage img = separator.ConvertToImage();
			img = img.Scale(w - 36, 4, wxIMAGE_QUALITY_BILINEAR);
			tdc.DrawBitmap(wxBitmap(img), 30, posY + ((height - img.GetHeight()) / 2));
			noRadio = true;
			continue;
		}
		if (scrollPos == sel){
			tdc.SetPen(wxPen(highlight));
			tdc.SetBrush(wxBrush(menuhighlight));
			tdc.DrawRectangle(2, posY, w - 4, height);
		}
		//tdc.SetPen(wxPen("#497CB0",2));
		//tdc.SetBrush(*wxTRANSPARENT_BRUSH);

		if (showIcons){
			if (item->type == ITEM_CHECK || item->type == ITEM_CHECK_AND_HIDE){
				if (item->check) tdc.DrawBitmap(checkbmp, 5, posY + ((height - checkbmp.GetHeight()) / 2));
			}
			else if (item->type == ITEM_RADIO && noRadio){
				tdc.DrawBitmap(dot, 5, posY + ((height - checkbmp.GetHeight()) / 2));
				noRadio = false;
			}
			else if (item->icon){
				tdc.DrawBitmap(item->GetBitmap(), 5, posY + ((height - item->icon->GetHeight()) / 2));
			}
		}
		wxString desc = item->GetLabel();
		tdc.SetTextForeground((item->enabled) ? text : graytext);
		if (showMnemonics && desc.Find(L'&') != -1){
			wxString rest;
			wxString beforemn = desc.BeforeFirst(L'&', &rest);
			mnbefsize = tdc.GetTextExtent(beforemn);

			linesize = tdc.GetTextExtent(rest[0]);
			hasMnemonics = true;
		}
		else{ hasMnemonics = false; }

		desc.Replace(L"&", emptyString);
		int find = desc.find(L"\t");
		//tdc.SetPen(wxPen("#497CB0"));
		if (find != -1){
			int fw, fhh;
			wxString accel = desc.AfterLast(L'\t');
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel, w - fw - 20, posY + 2);
			desc = desc.BeforeLast(L'\t');
		}
		tdc.DrawText(desc, textStart, posY + 2);
		if (hasMnemonics){
			tdc.SetPen(wxPen((item->enabled) ? text : graytext));
			tdc.DrawLine(textStart + mnbefsize.x, (height * (i + 1)) - 3, 
				textStart + mnbefsize.x + linesize.x, (height * (i + 1)) - 3);
		}
		if (item->submenu){
			tdc.DrawBitmap(arrow, w - 18, posY + ((height - arrow.GetHeight()) / 2));
		}

	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, ow, h, &tdc, 0, 0);
}

void MenuDialog::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}
//retcode -5 for dialog instant destroy, loop still extists is destroyed in showPartialModal
void MenuDialog::HideMenus(int id)
{
	if (!ParentMenu){ return; }
	KainoteFrame * frame = ((kainoteApp*)wxTheApp)->Frame;
	frame->SetStatusText(emptyString, 0);
	MenuBar::Menubar->md = nullptr;
	int subMenu = ParentMenu->submenuToHide;
	Menu *menu = ParentMenu->parent;
	ParentMenu->submenuToHide = -1;
	ParentMenu->submenuShown = -1;
	while (subMenu != -1 && menu->items[subMenu]->submenu){
		menu = menu->items[subMenu]->submenu;
		if (menu->dialog){
			subMenu = menu->dialog->submenuToHide;
			menu->dialog->submenuToHide = -1;
			menu->dialog->submenuShown = -1;

		}
		else{ subMenu = -1; break; }
		//menu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND,1);
		menu->DestroyDialog();
	}
	if (ParentMenu->isPartialModal){
		ParentMenu->EndPartialModal(id);
		if (id == -5)
			ParentMenu->Destroy();
	}
	else{ ParentMenu->parent->DestroyDialog(); }
	ParentMenu = nullptr;
	showIcons = true;
	minWidth = 0;

}

//void MenuDialog::OnLostCapture(wxMouseCaptureLostEvent &evt){
//	//if(HasCapture()){ReleaseMouse();}
//	
//	if(ParentMenu){HideMenus();}
//}
//Pokazuje okno menu dodaj¹c pêtlê czekaj¹c¹ do odwo³ania
int MenuDialog::ShowPartialModal()
{
	isPartialModal = true;
	//Show();
	int resid = -3;
	Show();
	if (IsShown()){
		wxGUIEventLoop * looplocal = loop = new wxGUIEventLoop();
		resid = looplocal->Run();
		delete looplocal;
		if (resid > -5)
			loop = nullptr;
	}
	return resid;
}
//Odwo³uje pêtlê czekaj¹c¹
void MenuDialog::EndPartialModal(int ReturnId)
{
	if (loop){
		loop->Exit(ReturnId);
	}
	Hide();
}

WXLRESULT MenuDialog::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
	if (message == 28) {
		if (ParentMenu){
			ParentMenu->HideMenus();
			MenuBar::Menubar->HideMnemonics();
		}
		else{
			Destroy();
		}
		return 0;
	}
	return wxPopupWindow::MSWWindowProc(message, wParam, lParam);
}


BEGIN_EVENT_TABLE(MenuDialog, wxPopupWindow/*wxFrame*/)
EVT_PAINT(MenuDialog::OnPaint)
EVT_SCROLL(MenuDialog::OnScroll)
END_EVENT_TABLE()

static int menuIndent = 20;
static int halfIndent = 10;
MenuBar *MenuBar::Menubar = nullptr;
wxFont MenuBar::font = wxFont();

MenuBar::MenuBar(wxWindow *_parent)
	: wxWindow(_parent, -1, wxDefaultPosition, wxSize(-1, height))
	, Mnemonics()
	, bmp(nullptr)
	, sel(-1)
	, clicked(false)
	, md(nullptr)
	, oldelem(-1)
	, shownMenu(-1)
	, altDown(false)
{
	int x = 0, y = 0;
	font = *Options.GetFont();//wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, L"Tahoma");
	wxWindow::SetFont(font);
	GetTextExtent(L"#TWFfGHj", &x, &y);
	y += 6;
	if (y > height){
		SetMinSize(wxSize(200, y));
		SetSize(wxSize(-1, y));
		height = y;
	}
	//Refresh(false);
	
	Menubar = this;
	HookKey = nullptr;
	HookKey = SetWindowsHookEx(WH_KEYBOARD, &OnKey, nullptr, GetCurrentThreadId());
	HookMouse = nullptr;
	HookMouse = SetWindowsHookEx(WH_GETMESSAGE, &OnMouseClick, nullptr, GetCurrentThreadId());
	
}

MenuBar::~MenuBar(){
	for (auto cur = Menus.begin(); cur != Menus.end(); cur++){
		delete (*cur);
	}
	wxDELETE(bmp);
	UnhookWindowsHookEx(HookKey);
	UnhookWindowsHookEx(HookMouse);
	Menubar = nullptr;
}

void MenuBar::ShowMenu(){
	if (shownMenu == -1){ return; }
	MenuEvent evt(EVT_MENU_OPENED, GetId(), Menus[shownMenu]);
	ProcessEvent(evt);
	wxSize rc = GetClientSize();
	wxPoint pos = GetPosition();
	int posX = halfIndent;
	for (int i = 0; i < shownMenu; i++){
		Menu *menu = Menus[i];
		wxString desc = menu->GetTitle();
		desc.Replace(L"&", emptyString);
		wxSize te = GetTextExtent(desc);
		posX += te.x + menuIndent;
	}
	wxPoint pos1(posX + pos.x, rc.y + pos.y);
	Menus[shownMenu]->PopupMenu(pos1, this->GetParent());
	selectOnStart = -1;
}

void MenuBar::Append(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.push_back(menu);
	findMnemonics(title, Menus.size() - 1);
}

void MenuBar::Prepend(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin(), menu);
	findMnemonics(title, 0);
}

void MenuBar::Insert(int position, Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin() + position, menu);
	findMnemonics(title, position);
}

void MenuBar::OnMouseEvent(wxMouseEvent &evt)
{
	if (evt.LeftUp()){ clicked = false; Refresh(false); }

	wxPoint pos = evt.GetPosition();
	int elem = CalcMousePos(&pos);
	if ((evt.Leaving() || elem == -1) && shownMenu == -1 && !showMnemonics){
		oldelem = sel = -1; Refresh(false);
		return;
	}
	if (evt.Entering() || elem == -1){
		if (shownMenu != -1 && Menus[shownMenu]->dialog == nullptr){ shownMenu = -1; }
		oldelem = -1;
		if (shownMenu == elem && !showMnemonics){
			oldelem = sel = elem; 
			Refresh(false); 
			return; 
		}
	}
	//poprawiæ to nieszczêsne menu by nie odpala³o siê wiele razy ani te¿ obwódka z helpa nie znika³a.
	if (elem != oldelem){
		
		oldelem = elem;
		if (elem != -1 && shownMenu != elem){
			sel = elem;
			Refresh(false);
			if (shownMenu != -1/* && elem != -1*/){
				if (Menus[shownMenu]->dialog){ Menus[shownMenu]->dialog->HideMenus(); }
				shownMenu = elem;
				ShowMenu();
			}
		}
		

	}

	if (evt.LeftDown() && elem >= 0){
		clicked = true;
		Refresh(false);
		if (shownMenu == elem){
			shownMenu = -1;
			return;
		}
		shownMenu = elem;
		ShowMenu();
	}

}

void MenuBar::OnPaint(wxPaintEvent &event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }

	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = nullptr;
	}
	if (!bmp){ bmp = new wxBitmap(w, h); }
	tdc.SelectObject(*bmp);
	tdc.SetFont(font);
	tdc.GradientFillLinear(wxRect(0, 0, w, h),
		Options.GetColour(MENUBAR_BACKGROUND2),
		Options.GetColour(MENUBAR_BACKGROUND1), wxTOP);
	tdc.SetTextForeground(Options.GetColour(WINDOW_TEXT));
	int posX = halfIndent;
	wxSize mnbefsize;
	wxSize linesize;
	bool hasMnemonics = false;
	for (size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc = menu->GetTitle();


		if (showMnemonics && desc.Find(L'&') != -1){
			wxString rest;
			wxString beforemn = desc.BeforeFirst(L'&', &rest);
			mnbefsize = tdc.GetTextExtent(beforemn);

			linesize = tdc.GetTextExtent(rest[0]);
			hasMnemonics = true;
		}
		else{ hasMnemonics = false; }

		desc.Replace(L"&", emptyString);
		wxSize te = tdc.GetTextExtent(desc);

		if (i == sel){
			tdc.SetBrush(wxBrush(Options.GetColour(clicked ? MENUBAR_BACKGROUND_SELECTION : MENUBAR_BACKGROUND_HOVER)));
			tdc.SetPen(wxPen(Options.GetColour(MENUBAR_BORDER_SELECTION)));
			tdc.DrawRoundedRectangle(posX - 4, 1, te.x + 8, h - 3, 3.0);
		}
		tdc.DrawText(desc, posX, (h - te.y) / 2);
		if (hasMnemonics){
			tdc.SetPen(wxPen(Options.GetColour(WINDOW_TEXT)));
			tdc.DrawLine(posX + mnbefsize.x, h - 4, posX + mnbefsize.x + linesize.x, h - 4);
		}
		posX += te.x + menuIndent;
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}

void MenuBar::OnSize(wxSizeEvent& event)
{
	/*if (height < 1){
		int x = 0, y = 0;
		font = wxFont(9, wxSWISS, wxFONTSTYLE_NORMAL, wxNORMAL, false, L"Tahoma");
		GetTextExtent(L"#TWFfGH", &x, &y);
		y += 4;
		if (y > 26){
			SetMinSize(wxSize(200, y));
			SetSize(wxSize(-1, y));
		}
		height = y;
	}
	Refresh(false);*/
}

int MenuBar::CalcMousePos(wxPoint *pos)
{
	int posX = 0;
	for (size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc = menu->GetTitle();
		desc.Replace(L"&", emptyString);
		wxSize te = GetTextExtent(desc);
		if (pos->x > posX && (posX + te.x + menuIndent) > pos->x){
			pos->x = posX, pos->y = posX + te.x + halfIndent;
			return i;
		}
		posX += te.x + menuIndent;
	}
	return -1;
}

MenuItem *MenuBar::FindItem(int id)
{
	MenuItem *item = nullptr;
	for (auto menu : Menus){
		item = menu->FindItem(id);
		if (item) { return item; }
	}
	return nullptr;
}

void MenuBar::Enable(int id, bool enable)
{
	MenuItem * item = FindItem(id);
	if (item){ item->Enable(enable); }
	else{ KaiLog(wxString::Format(L"Cannot enable item with id %i", id)); }
}

//void MenuBar::AppendAccelerators(std::vector <wxAcceleratorEntry> *entries)
//{
//	for(size_t i = 0; i<Menus.size(); i++){
//		Menus[i]->GetAccelerators(entries);
//	}
//}

LRESULT CALLBACK MenuBar::OnKey(int code, WPARAM wParam, LPARAM lParam){

	if (code < 0)
	{
		CallNextHookEx(Menubar->HookKey, code, wParam, lParam);
		return 0;
	}

	if (wParam == VK_MENU && !(lParam & 1073741824)){//536870912 1073741824 lparam mówi nam o altup, który ma specjalny bajt 
		//close menu after alt up
		if (Menubar->md) {
			Menubar->md->HideMenu();
			return 1;
		}
		byte state[256];
		if (GetKeyboardState(state) == FALSE){ return 0; }
		if (!(state[VK_LMENU]>1 && state[VK_LSHIFT] < 2 && state[VK_RSHIFT] < 2 && state[VK_LCONTROL] < 2 && state[VK_RCONTROL] < 2)){ return 0; }
		if (Menubar->md && showMnemonics){ Menubar->md->HideMenu();/*return 1;*/ }
		showMnemonics = !showMnemonics;
		Menubar->altDown = true;
		if (Menubar->md){
			Menubar->md->dialog->Refresh(false);
			if (Menubar->shownMenu == -1){ return 1; }
		}
		Menubar->sel = -1;
		Menubar->Refresh(false);
		Menubar->blockMenu = false;
		return 1;
	}
	else if (wParam == VK_MENU && !(lParam & 536870912)){
		if (!Menubar->altDown){ showMnemonics = false; }
		if (Menubar->sel == -1 || !showMnemonics){
			Menubar->sel = (showMnemonics && !Menubar->blockMenu) ? 0 : -1;
			Menubar->Refresh(false);
		}
		Menubar->altDown = false;
		Menubar->blockMenu = false;
	}
	else if (showMnemonics){
		if ((wParam >= 0x41 && wParam <= 0x5A) && !(lParam & 2147483648)){//lparam mówi o keyup
			auto mn = (Menubar->md) ? Menubar->md->mnemonics : Menubar->mnemonics;
			auto foundmnemonics = mn.find(wParam);

			if (foundmnemonics != mn.end()){
				byte state[256];
				if (GetKeyboardState(state) == FALSE){ return 0; }
				// when menu open with alt - ... then you can release alt and type only corresponting key
				if (!(/*state[VK_LMENU] > 1 && */state[VK_LSHIFT] < 2 && state[VK_RSHIFT] < 2 && state[VK_LCONTROL] < 2 && state[VK_RCONTROL] < 2)){ 
					return 0; 
				}
				if (Menubar->md){
					if (Menubar->md->items[foundmnemonics->second]->submenu){
						Menubar->md->dialog->sel = Menubar->md->dialog->submenuShown = foundmnemonics->second;
						if (Menubar->md->dialog->submenuToHide == -1){
							selectOnStart = 0;
							Menubar->md->dialog->showSubmenuTimer.Start(1, true);
						}
					}
					else{
						MenuItem *item = Menubar->md->items[foundmnemonics->second];
						if (!Menubar->md->dialog->SendEvent(item, 0)){ return 1; }
						Menubar->HideMnemonics();
					}
				}
				else{
					if (Menubar->shownMenu != -1 && Menubar->Menus[Menubar->shownMenu]->dialog){
						Menubar->Menus[Menubar->shownMenu]->dialog->HideMenus();
					}
					Menubar->sel = Menubar->shownMenu = foundmnemonics->second;
					Menubar->Refresh(false);
					selectOnStart = 0;
					//Menubar->showMenuTimer.Start(10, true);
					Menubar->ShowMenu();
				}
				return 1;
			}
			return 0;
		}
		//if (Menubar->blockMenu && (wParam != VK_CONTROL && wParam != VK_SHIFT)){
		//	Menubar->blockMenu = false;
		//}
		else if (wParam != VK_MENU/*(wParam == VK_CONTROL || wParam == VK_SHIFT)*//* && (lParam & 536870912)*/){
			Menubar->blockMenu = true;
		}
		//else if (wParam != VK_MENU){ 
			//Menubar->altDown = false;
		//}
	}

	if ((wParam == VK_DOWN || wParam == VK_UP || wParam == VK_LEFT || wParam == VK_RIGHT) && !(lParam & 2147483648)){
		if (Menubar->md){
			if (wParam == VK_LEFT && Menubar->md->dialog != MenuDialog::ParentMenu){
				Menubar->md->DestroyDialog();
				MenuBar::Menubar->md = Menubar->md->parentMenu;
				Menubar->md->dialog->submenuToHide = -1;
				Menubar->md->dialog->subMenuIsShown = false;
				return 1;
			}
			else if (wParam == VK_RIGHT && Menubar->md->dialog->sel >= 0
				&& Menubar->md->items[Menubar->md->dialog->sel]->submenu){
				Menubar->md->dialog->submenuShown = Menubar->md->dialog->sel;
				if (Menubar->md->dialog->submenuToHide == -1){
					selectOnStart = 0;
					Menubar->md->dialog->showSubmenuTimer.Start(1, true);
					return 1;
				}
			}
			if (wParam == VK_DOWN || wParam == VK_UP){
				int step = (wParam == VK_DOWN) ? 1 : -1;
				do{
					Menubar->md->dialog->sel += step;
					if (Menubar->md->dialog->sel >= (int)Menubar->md->items.size()){
						Menubar->md->dialog->sel = 0;
					}
					else if (Menubar->md->dialog->sel < 0){
						Menubar->md->dialog->sel = Menubar->md->items.size() - 1;
					}
				} while (Menubar->md->items[Menubar->md->dialog->sel]->type == ITEM_SEPARATOR);
				Menubar->md->dialog->Refresh(false);
				return 1;
			}
		}

		if (Menubar->sel != -1){
			if (wParam == VK_RIGHT || wParam == VK_LEFT){
				int step = (wParam == VK_RIGHT) ? 1 : -1;
				Menubar->sel += step;
				if (Menubar->sel >= (int)Menubar->Menus.size()){
					Menubar->sel = 0;
				}
				else if (Menubar->sel < 0){
					Menubar->sel = Menubar->Menus.size() - 1;
				}
				Menubar->Refresh(false);
				if (!Menubar->md){ return 1; }

			}
			if (Menubar->shownMenu != -1 && Menubar->Menus[Menubar->shownMenu]->dialog){
				Menubar->Menus[Menubar->shownMenu]->dialog->HideMenus();
			}
			Menubar->shownMenu = Menubar->sel;
			selectOnStart = 0;
			//Menubar->showMenuTimer.Start(1, true);
			Menubar->ShowMenu();
			return 1;
		}

	}
	else if (wParam == VK_ESCAPE && Menubar->md){
		MenuDialog::ParentMenu->HideMenus();
		Menubar->HideMnemonics();
		return 1;
	}
	else if (wParam == VK_RETURN && Menubar->md){
		if (Menubar->md->dialog->sel >= 0){
			MenuItem *item = Menubar->md->items[Menubar->md->dialog->sel];
			if (!Menubar->md->dialog->SendEvent(item, 0)){ return 1; }
			Menubar->HideMnemonics();
			return 1;
		}
	}
	//if(showMnemonics && wParam != VK_MENU){Menubar->HideMnemonics();}

	return (Menubar->md) ? 1 : CallNextHookEx(Menubar->HookKey, code, wParam, lParam);
}

LRESULT CALLBACK MenuBar::OnMouseClick(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0)
	{
		CallNextHookEx(Menubar->HookMouse, code, wParam, lParam);
		return 0;
	}
	LPMSG msg = (LPMSG)lParam;

	if (msg->message == WM_MOUSEWHEEL){
		POINT mouse;
		GetCursorPos(&mouse);
		HWND hWndPointed = WindowFromPoint(mouse);
		DWORD threadid;
		GetWindowThreadProcessId(hWndPointed, &threadid);
		DWORD currentthreadid = GetCurrentProcessId();
		if (currentthreadid == threadid && hWndPointed != nullptr && msg->hwnd != hWndPointed){
			msg->hwnd = hWndPointed;
		}
		else if (currentthreadid != threadid){
			msg->hwnd = 0;
		}
		return 1;
	}
	if (showMnemonics || Menubar->md){

		if (msg->message == WM_LBUTTONDOWN || msg->message == WM_NCLBUTTONDOWN /*||
			msg->message == WM_RBUTTONDOWN || msg->message == WM_NCRBUTTONDOWN*/){
			Menubar->HideMnemonics();
			if (!MenuDialog::ParentMenu){ return 0; }
			POINT mouse;
			GetCursorPos(&mouse);
			HWND hWndPointed = WindowFromPoint(mouse);
			wxWindow *win = wxGetWindowFromHWND(hWndPointed);
			bool contains = (win && (win->IsKindOf(wxCLASSINFO(wxPopupWindow)) ||
				win->GetParent() && win->GetParent()->IsKindOf(wxCLASSINFO(wxPopupWindow))));
			if (contains) { return 0; }
			MenuDialog::ParentMenu->HideMenus();
			msg->hwnd = 0;
			return 1;
		}
		if (msg->message == WM_MBUTTONUP || msg->message == WM_NCMBUTTONUP)
		{
			Menubar->HideMnemonics();
		}//else{
		//return 0;
		//}

	}

	return CallNextHookEx(0, code, wParam, lParam);
}

void MenuBar::HideMnemonics()
{
	showMnemonics = false;
	sel = -1;
	altDown = false;
	Refresh(false);
}

bool MenuBar::SetFont(const wxFont &_font)
{
	wxWindow::SetFont(_font);
	font = _font;
	wxWindow::SetFont(font);
	int x, y;
	GetTextExtent(L"#TWFfGHj", &x, &y);
	y += 6;
	if (y != height){
		SetMinSize(wxSize(200, y));
		SetSize(wxSize(-1, y));
		height = y;
	}

	Refresh(false);
	return true;
}

BEGIN_EVENT_TABLE(MenuBar, wxWindow)
EVT_MOUSE_EVENTS(MenuBar::OnMouseEvent)
EVT_PAINT(MenuBar::OnPaint)
EVT_ERASE_BACKGROUND(MenuBar::OnEraseBackground)
//EVT_SIZE(MenuBar::OnSize)
END_EVENT_TABLE()