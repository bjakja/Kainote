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


//#include "Toolbar.h"
#include "Hotkeys.h"
#include "KainoteApp.h"
#include "Toolbar.h"
#include "KaiScrollbar.h"
#include "Notebook.h"
#include "wx/utils.h"
#include "wx/dc.h"
#include "wx/dcmemory.h"
#include "wx/dcclient.h"
#include "Config.h"


KaiToolbar::KaiToolbar(wxWindow *Parent, MenuBar *mainm, int id)
	:wxWindow(Parent, -1, wxDefaultPosition, wxSize(thickness, -1))
	, bmp(nullptr)
	, Clicked(false)
	, wasmoved(false)
	, wh(thickness)
	, oldelem(-1)
	, sel(-1)
	, mb(mainm)
{
	alignment = Options.GetInt(TOOLBAR_ALIGNMENT);
	int fw, fh;
	GetTextExtent(L"TEX{}", &fw, &fh);
	thickness = toolbarSize = fh + 8;
}

KaiToolbar::~KaiToolbar()
{
	wxArrayString names;
	wxDELETE(bmp);
	for (auto i = tools.begin(); i != tools.end(); i++)
	{
		if ((*i)->id != 32566){ names.Add(GetString((Id)(*i)->id)); }
		delete (*i);
	}
	tools.clear();
	Options.SetTable(TOOLBAR_IDS, names);
	ids.clear();
}

void KaiToolbar::InitToolbar()
{
	wxArrayInt IDS; 
	wxArrayString stringIDs;
	Options.GetTable(TOOLBAR_IDS, stringIDs);
	for (const auto &stringID : stringIDs){
		int val = GetIdValue(stringID);
		if (val > 0){
			IDS.Add(val);
		}
	}

	if (IDS.size() < 1){
		IDS.Add(GLOBAL_OPEN_SUBS); IDS.Add(GLOBAL_RECENT_SUBS); IDS.Add(GLOBAL_OPEN_VIDEO); IDS.Add(GLOBAL_RECENT_VIDEO);
		IDS.Add(GLOBAL_SAVE_SUBS); IDS.Add(GLOBAL_SAVE_SUBS_AS); IDS.Add(GLOBAL_SAVE_ALL_SUBS);
		IDS.Add(GLOBAL_REMOVE_SUBS); IDS.Add(GLOBAL_EDITOR); IDS.Add(GLOBAL_FIND_REPLACE);
		IDS.Add(GLOBAL_OPEN_STYLE_MANAGER); IDS.Add(GLOBAL_OPEN_ASS_PROPERTIES); IDS.Add(GLOBAL_SHOW_SHIFT_TIMES);
		IDS.Add(GLOBAL_SET_VIDEO_AT_START_TIME); IDS.Add(GLOBAL_SET_VIDEO_AT_END_TIME); IDS.Add(GLOBAL_SET_START_TIME); IDS.Add(GLOBAL_SET_END_TIME);
		IDS.Add(GLOBAL_CONVERT_TO_ASS); IDS.Add(GLOBAL_CONVERT_TO_SRT); IDS.Add(GLOBAL_VIDEO_ZOOM);
		IDS.Add(GLOBAL_OPEN_SUBS_RESAMPLE); IDS.Add(GLOBAL_HIDE_TAGS); IDS.Add(GLOBAL_SETTINGS);
	}
	for (size_t i = 0; i < IDS.size(); i++)
	{
		MenuItem *item = mb->FindItem(IDS[i]);
		if (!item){ KaiLog(wxString::Format(_("Nie można znaleźć elementu o id %i"), IDS[i])); continue; }
		wxString desc = item->GetLabelText();
		bool isToogleButton = item->type == ITEM_CHECK;
		AddItem(IDS[i], desc, item->icon, item->IsEnabled(), (isToogleButton) ? 2 :
			(item->GetSubMenu() != nullptr) ? 1 : 0, (isToogleButton) ? item->check : false);
	}
	tools.push_back(new toolitem(3, 16, 32566, true));

	Refresh(false);
}


void KaiToolbar::AddItem(int id, const wxString &label, wxBitmap *normal, bool enable, byte type, bool toggled)
{
	if (tools.size() > 0 && tools[tools.size() - 1]->GetType() == 3){
		tools.insert(tools.begin() + tools.size() - 2, new toolitem(normal, label, id, enable, type, toggled));
		return;
	}
	tools.push_back(new toolitem(normal, label, id, enable, type, toggled));
}

void KaiToolbar::InsertItem(int id, int index, const wxString &label, wxBitmap *normal, bool enable, byte type, bool toggled)
{
	tools.insert(tools.begin() + index, new toolitem(normal, label, id, enable, type, toggled));
}

void KaiToolbar::AddSpacer()
{
	tools.push_back(new toolitem(3, 12));
}
void KaiToolbar::InsertSpacer(int index)
{
	tools.insert(tools.begin() + index, new toolitem(3, 12));
}

toolitem * KaiToolbar::FindItem(int id)
{
	for (auto i = tools.begin(); i != tools.end(); i++)
	{
		if ((*i)->id == id)
		{
			return (*i);
		}
	}
	return nullptr;
}

void KaiToolbar::UpdateId(int id, bool enable)
{
	for (auto i = tools.begin(); i != tools.end(); i++)
	{
		if ((*i)->id == id)
		{
			if ((*i)->Enable(enable))
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

	wxPoint elems = FindElem(event.GetPosition());
	int elem = elems.x;

	if (elem < 0 || event.Leaving()){/*if(HasCapture()){ReleaseMouse();}*/if (HasToolTips()){ UnsetToolTip(); }
	int tmpsel = sel;
	sel = -1;
	oldelem = -1;
	Clicked = false;
	if (tmpsel != sel){ Refresh(false); }
	return;
	}
	if (elem == tools.size() - 1){
		SetToolTip(_("Wybierz ikony paska narzędzi"));
		sel = elem; oldelem = elem;
		Refresh(false);
	}
	else if (sel != elem && tools[elem]->type < 3 && !event.LeftIsDown()){
		wxString shkeyadd;
		wxString shkey = Hkeys.GetStringHotkey(tools[elem]->id);
		if (shkey != emptyString){ shkeyadd << L" (" << shkey << L")"; }
		SetToolTip(tools[elem]->label + shkeyadd);
		sel = elem;
		//RefreshRect(wxRect(0,elem*iconsize,iconsize,(elem+1)*iconsize),false);
		Refresh(false);
	}
	if (!tools[elem]->enabled){ int tmpsel = sel; sel = -1; if (tmpsel != sel){ Refresh(false); } }
	if ((leftdown || (event.Entering() && event.LeftIsDown()))){// && tools[elem]->type<3
		Clicked = true;
		Refresh(false); oldelem = elem;
	}
	else if (event.LeftIsDown() && oldelem != elem && oldelem >= 0){
		if (elem == tools.size() - 1 || oldelem == tools.size() - 1){ oldelem = elem; return; }
		toolitem *tmpitem = tools[oldelem];
		tools[oldelem] = tools[elem];
		tools[elem] = tmpitem;
		Refresh(false);
		oldelem = elem;
		wasmoved = true;
	}
	else if (event.LeftUp()){
		if (!wasmoved && tools[elem]->enabled){
			if (tools[elem]->type == 2){
				tools[elem]->toggled = !tools[elem]->toggled;
				Refresh(false);
			}
			if (tools[elem]->type == 1){
				MenuItem *item = mb->FindItem(tools[elem]->id);
				Menu * smenu = item->GetSubMenu();
				//Menu * shmenu;
				if (tools[elem]->id != GLOBAL_SORT_LINES && tools[elem]->id != GLOBAL_SORT_SELECTED_LINES){
					KainoteFrame *Kai = (KainoteFrame*)GetParent();
					int what = (smenu == Kai->SubsRecMenu) ? 0 : (smenu == Kai->VidsRecMenu) ? 1 : 2;
					Kai->AppendRecent(what, smenu);
				}
				/*for(int i=0; i<(int)smenu->GetMenuItemCount(); i++)
				{
				MenuItem *itm=smenu->FindItemByPosition(i);
				shmenu.Append(itm->GetId(),itm->GetLabel(),itm->GetHelp())->Enable(itm->IsEnabled());
				}*/

				smenu->PopupMenu(event.GetPosition(), this);
			}
			else{
				wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, tools[elem]->id);
				evt.SetInt(TOOLBAR_EVENT);
				ProcessEvent(evt);
			}
		}
		Clicked = false; wasmoved = false; Refresh(false);
	}

}


void KaiToolbar::OnPaint(wxPaintEvent &event)
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
	bool vertical = alignment % 2 == 0;
	const wxColour & background = Options.GetColour(WINDOW_BACKGROUND);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0, 0, w, h);
	int pos = 4;
	int pos1 = 0;
	int maxx = (vertical) ? h : w;
	int toolsSize = tools.size();

	for (int i = 0; i < toolsSize; i++)
	{
		if (pos + thickness > maxx && pos1 + thickness < toolbarSize){ pos1 += thickness; pos = 4; }
		bool toggled = tools[i]->toggled;
		if (i == sel || toggled){
			tdc.SetPen(wxPen((Clicked || toggled) ?
				Options.GetColour(BUTTON_BORDER_PUSHED) : Options.GetColour(BUTTON_BORDER_HOVER)));
			tdc.SetBrush(wxBrush((Clicked || toggled) ?
				Options.GetColour(BUTTON_BACKGROUND_PUSHED) : Options.GetColour(BUTTON_BACKGROUND_HOVER)));
			tdc.DrawRoundedRectangle((vertical) ? pos1 + 2 : pos - 2,
				(vertical) ? pos - 2 : pos1 + 2, thickness - 4, 
				(i >= toolsSize - 1) ? (thickness / 2) : thickness - 4, 1.1);
		}
		if (tools[i]->type < 3){
			//wxImage img=tools[i]->GetBitmap().ConvertToImage();
			//img=img.Rescale(20,20,wxIMAGE_QUALITY_HIGH);
			wxBitmap toolBmp = tools[i]->GetBitmap();
			int posX = pos1 + ((thickness - toolBmp.GetWidth()) / 2);
			int posY = pos - 4 + ((thickness - toolBmp.GetHeight()) / 2);
			tdc.DrawBitmap(toolBmp, (vertical) ? posX : posY, (vertical) ? posY : posX);
		}
		/*else if(tools[i]->type==3){
			tdc.SetPen(wxPen("#606060"));
			tdc.DrawLine((vertical)?pos1+2 : pos+2, (vertical)?pos+2 : pos1+2, (vertical)? iconsize-2 : pos+2, (vertical)?pos+2 : iconsize-2);*/
		//}
		pos += (i >= toolsSize - 1) ? (thickness / 2) : thickness;
	}
	tdc.SetPen(wxPen(Options.GetColour(WINDOW_TEXT)));
	tdc.SetBrush(wxBrush(Options.GetColour(WINDOW_TEXT)));
	wxPoint points[3];
	if (vertical){
		tdc.DrawLine(pos1 + thickness - 12, pos - 11, pos1 + thickness - 6, pos - 11);
		points[0] = wxPoint(pos1 + thickness - 12, pos - 8);
		points[1] = wxPoint(pos1 + thickness - 6, pos - 8);
		points[2] = wxPoint(pos1 + thickness - 9, pos - 4);
	}
	else{
		tdc.DrawLine(pos + (thickness / 2) - 16, pos1 + 5, pos + (thickness / 2) - 10, pos1 + 5);
		points[0] = wxPoint(pos + (thickness / 2) - 16, pos1 + 8);
		points[1] = wxPoint(pos + (thickness / 2) - 10, pos1 + 8);
		points[2] = wxPoint(pos + (thickness / 2) - 13, pos1 + 12);
	}

	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.DrawPolygon(3, points);
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}
void KaiToolbar::OnSize(wxSizeEvent &evt)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	bool vertical = alignment % 2 == 0;
	float maxx = (vertical) ? h : w;
	int toolbarrows = ((tools.size() * thickness) - 2) / maxx;

	wh = (toolbarrows + 1) * thickness;
	int maxxwh = (vertical) ? w : h;
	if (maxxwh != wh){
		toolbarSize = wh;
		if (vertical)
			SetSize(wxSize(wh, -1));
		else
			SetSize(wxSize(-1, wh));

		KainoteFrame *Kai = (KainoteFrame*)GetParent();
		//Kai->Layout();
		wxSizeEvent evt;
		Kai->OnSize(evt);
	}

	Refresh(false);
}

wxPoint KaiToolbar::FindElem(wxPoint pos)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	wxPoint res(-1, 0);
	bool vertical = alignment % 2 == 0;
	int maxx = (vertical) ? h : w;
	int tmppos = 4;
	int curpos = (vertical) ? pos.y : pos.x;
	int curpos1 = (vertical) ? pos.x : pos.y;
	curpos1 /= thickness;
	for (size_t i = 0; i < tools.size(); i++)
	{
		int toolsize = /*(vertical) ? tools[i]->size : */thickness;
		if (tmppos + toolsize > maxx && (res.y + 1) * thickness < toolbarSize){ 
			res.y++; tmppos = 0; 
		}
		if (curpos > tmppos && curpos <= tmppos + toolsize && res.y == curpos1)
		{
			res.x = i;
			return res;
		}
		tmppos += toolsize;

	}
	return res;
}

bool KaiToolbar::Updatetoolbar()
{
	bool changes = false;
	for (int i = 0; i < (int)tools.size() - 1; i++)
	{
		if (tools[i]->id < 1){ continue; }
		changes |= tools[i]->Enable(mb->FindItem(tools[i]->id)->IsEnabled());
	}


	if (changes){ Refresh(false); }
	return false;
}

void KaiToolbar::OnToolbarOpts(wxCommandEvent &event)
{
	wxPoint point = GetPosition();
	point = GetParent()->ClientToScreen(point);
	wxSize toolbarSize = GetClientSize();

	int fw, fh, width = 0;

	int ysize = ids.size();
	for (int i = 0; i < ysize; i++){
		MenuItem *item = mb->FindItem(ids[i]);
		if (!item)
			continue;
		GetTextExtent(item->label, &fw, &fh);
		if (fw > width)
			width = fw;
	}
	wxSize toolbarMenuSize = wxSize(width + (fh * 2) + 38, (fh + 6) * 25 + fh + 11);

	switch (alignment){
	case 0://left
		point.y += 20;
		point.x += toolbarSize.x;
		break;
	case 1://top
		point.y += toolbarSize.y;
		point.x += (tools.size() * thickness % toolbarSize.x) - (toolbarMenuSize.x / 2);
		if (point.x < 0){ point.x = 0; }
		break;
	case 2://right
		point.y += 20;
		point.x -= toolbarMenuSize.x;
		break;
	case 3://bottom
		point.y -= toolbarMenuSize.y;
		if (point.y < 0){ point.y = 0; }
		point.x += (tools.size() * thickness % toolbarSize.x) - (toolbarMenuSize.x / 2);
		if (point.x < 0){ point.x = 0; }
		break;
	default:
		point.y += 20;
		point.x += toolbarSize.x;
		break;
	}

	ToolbarMenu *tbr = new ToolbarMenu(this, point, toolbarMenuSize, fh + 6);
	tbr->Show();

}

bool KaiToolbar::SetFont(const wxFont &font)
{
	wxWindow::SetFont(font);
	//write rest when add custom size
	int fw, fh;
	GetTextExtent(L"TEX{}", &fw, &fh);
	thickness = toolbarSize = fh + 8;
	int w, h;
	GetClientSize(&w, &h);
	bool vertical = alignment % 2 == 0;
	float maxx = (vertical) ? h : w;
	int toolbarrows = ((tools.size() * thickness) - 2) / maxx;

	int wh = (toolbarrows + 1) * thickness;
	int maxxwh = (vertical) ? w : h;
	if (maxxwh != wh){
		toolbarSize = wh;
	}
	//need test if onsize from kainoteframe make rest
	return true;
}

BEGIN_EVENT_TABLE(KaiToolbar, wxWindow)
EVT_MOUSE_EVENTS(KaiToolbar::OnMouseEvent)
EVT_PAINT(KaiToolbar::OnPaint)
EVT_SIZE(KaiToolbar::OnSize)
EVT_MENU(32566, KaiToolbar::OnToolbarOpts)
//EVT_ERASE_BACKGROUND(KaiToolbar::OnEraseBackground)
END_EVENT_TABLE()

ToolbarMenu::ToolbarMenu(KaiToolbar*_parent, const wxPoint &pos, const wxSize &size, int height)
	: wxDialog(_parent, -1, emptyString, pos, size, wxBORDER_NONE)
	, sel(-1)
	, scPos(0)
	, parent(_parent)
	, bmp(nullptr)
{
	fh = height;
	SetFont(parent->GetFont());
	scroll = new KaiScrollbar(this, -1, wxDefaultPosition, wxDefaultSize, wxVERTICAL);
	Bind(wxEVT_IDLE, &ToolbarMenu::OnIdle, this);
	wxString ans[] = { _("Po lewej"), _("U góry"), _("Po prawej"), _("Na dole") };
	alignments = new KaiChoice(this, 32213, wxPoint(4, 4), wxSize(size.x - 8, fh), 4, ans);
	if (parent->alignment > 3 || parent->alignment < 0)
		parent->alignment = 0;
	alignments->SetSelection(parent->alignment);
	alignments->SetToolTip(_("Pozycja paska narzędzi"));
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		parent->alignment = alignments->GetSelection();
		Options.SetInt(TOOLBAR_ALIGNMENT, parent->alignment);
		KainoteFrame *win = (KainoteFrame*)parent->GetParent();
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
	bool leftdown = evt.LeftDown();
	int x = evt.GetX();
	int y = evt.GetY();
	int w = 0;
	int h = 0;
	GetSize(&w, &h);
	if ((x < 0 || y < 0 || x > w || y > h)){
		if (leftdown){
			Unbind(wxEVT_IDLE, &ToolbarMenu::OnIdle, this);
			if (HasCapture()){
				ReleaseMouse();
			}
			Destroy();
		}
		return;
	}

	int elem = (y - (fh + 5)) / fh;
	elem += scPos;
	if (elem >= (int)parent->ids.size() || y <= fh + 5){ sel = -1; Refresh(false); return; }
	if (elem != sel){
		sel = elem;
		Refresh(false);
	}
	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -= step;
		Refresh(false);
		return;
	}
	if (leftdown){
		int result = -1;
		for (size_t j = 0; j < parent->tools.size(); j++)
		{
			if (parent->tools[j]->id == parent->ids[elem]){ result = j; break; }
		}
		if (result == -1){
			MenuItem *item = parent->mb->FindItem(parent->ids[elem]);
			bool isToogleButton = item->type == ITEM_CHECK;
			parent->AddItem(parent->ids[elem], item->GetLabelText(), item->icon, item->IsEnabled(),
				(isToogleButton) ? 2 : (item->GetSubMenu() != nullptr) ? 1 : 0, (isToogleButton) ? item->check : false);
		}
		else{
			delete parent->tools[result];
			parent->tools.erase(parent->tools.begin() + result);

		}
		bool vertical = parent->alignment % 2 == 0;
		parent->GetClientSize(&w, &h);
		float maxx = (vertical) ? h : w;
		int toolbarrows = ((parent->tools.size() * parent->thickness) - 2) / maxx;

		int wh = (toolbarrows + 1) * parent->thickness;
		int maxxwh = (vertical) ? w : h;
		if (maxxwh != wh){
			wxSizeEvent sizeEvent;
			parent->OnSize(sizeEvent);
			wxPoint toolbarPos = GetPosition();
			if (vertical){
				toolbarPos.x += parent->alignment == 0 ? (wh - maxxwh) : (maxxwh - wh);
				SetPosition(toolbarPos);
			}
			else{
				toolbarPos.y += parent->alignment == 1 ? (wh - maxxwh) : (maxxwh - wh);
				SetPosition(toolbarPos);
			}
		}
		else{
			parent->Refresh(false);
		}
		Refresh(false);
	}
}

void ToolbarMenu::OnPaint(wxPaintEvent &event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	int ow = w;
	int thickness = scroll->GetThickness();
	w -= (thickness + 1);
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < ow || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = nullptr;
	}
	if (!bmp){ bmp = new wxBitmap(ow, h); }
	tdc.SelectObject(*bmp);
	wxBitmap checkbmp = wxBITMAP_PNG(L"check");
	tdc.SetFont(GetFont());
	const wxColour & background = Options.GetColour(MENUBAR_BACKGROUND);
	const wxColour & txt = Options.GetColour(WINDOW_TEXT);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(Options.GetColour(WINDOW_BORDER)));
	tdc.DrawRectangle(0, 0, ow, h);
	int visible = (h - fh + 8) / fh;
	int idssize = parent->ids.size();
	if (scPos >= idssize - visible){ scPos = idssize - visible; }
	else if (scPos < 0){ scPos = 0; }
	int maxsize = MAX(idssize, scPos + visible);
	scroll->SetScrollbar(scPos, visible, idssize, visible - 1);
	scroll->SetSize(w, fh + 8, thickness, h - (fh + 9));
	tdc.SetTextForeground(txt);
	for (int i = 0; i < visible; i++)
	{
		int posY = (fh * i) + fh + 6;
		MenuItem *item = parent->mb->FindItem(parent->ids[i + scPos]);
		bool check = false;
		for (size_t j = 0; j < parent->tools.size(); j++)
		{
			if (parent->tools[j]->id == parent->ids[i + scPos]){ check = true; break; }
		}
		if (i + scPos == sel){
			tdc.SetPen(wxPen(Options.GetColour(MENU_BORDER_SELECTION)));
			tdc.SetBrush(wxBrush(Options.GetColour(MENU_BACKGROUND_SELECTION)));
			tdc.DrawRectangle(1, posY, w - 1, fh - 2);
		}


		if (check){
			tdc.DrawBitmap(checkbmp, 4, posY + (fh - checkbmp.GetHeight()) / 2);
		}

		tdc.DrawBitmap(item->GetBitmap(), fh + 8, posY + (fh - item->icon->GetHeight()) / 2);
		wxString desc = item->GetLabel();
		desc.Replace(L"&", emptyString);
		size_t reps = desc.Replace(L"\t", L" (");
		wxString accel;
		wxString label = desc;
		if (reps){
			desc.Append(L")");
			label = desc.BeforeFirst(L'(', &accel);
			accel.Prepend(L"(");
			int fw, fhh;
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel, w - fw - 8, posY + 2);
		}
		tdc.DrawText(label, (fh * 2) + 15, posY + 2);


	}
	wxPaintDC dc(this);
	dc.Blit(0, 0, ow, h, &tdc, 0, 0);
}

void ToolbarMenu::OnIdle(wxIdleEvent& event)
{
	event.Skip();

	if (!parent->IsShownOnScreen()){
		Destroy();
	}

	if (IsShown())
	{
		wxPoint pos = ScreenToClient(wxGetMousePosition());
		wxRect rect(GetSize());

		if (rect.Contains(pos))
		{
			if (HasCapture())
			{
				ReleaseMouse();
			}
		}
		else
		{
			if (!HasCapture() && !scroll->HasCapture())
			{
				CaptureMouse();
			}
		}
	}
}

void ToolbarMenu::OnLostCapture(wxMouseCaptureLostEvent &evt) 
{ 
	if (HasCapture()) { ReleaseMouse(); } 
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

