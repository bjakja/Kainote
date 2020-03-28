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

inline void KaiChoice::CalcMaxWidth(wxSize *result, bool changex, bool changey){
	int tx = 0, ty = 0;
	size_t isize = list->size();
	for (size_t i = 0; i < isize; i++){
		GetTextExtent((*list)[i], &tx, &ty);
		if (tx > result->x && changex){ result->x = tx; }
		else if (!changex){ break; }
	}
	if (changex){
		result->x += 26;
		if (result->x > 300){ result->x = 300; }
		if (isize < 1)
			result->x += 100;
	}
	if (changey){
		GetTextExtent(L"TEX{}", &tx, &ty);
		result->y = ty + 10;
	}
}


KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
	const wxSize& size, int n, const wxString choices[],
	long style, const wxValidator& validator)
	:wxWindow(parent, id, pos, size, style/* | wxWANTS_CHARS*/)
	, bmp(NULL)
	, list(NULL)
	, itemList(NULL)
	, choiceText(NULL)
	, listIsShown(false)
	, enter(false)
	, clicked(false)
	, focusSet(false)
	, choice(-1)
	, foreground(WINDOW_TEXT)
{
	list = new wxArrayString(n, choices);
	disabled = new std::map<int, bool>();

	wxWindow::SetFont(parent->GetFont());
	wxSize newSize = size;
	if (size.x < 1 || size.y < 1){
		CalcMaxWidth(&newSize, size.x < 1, size.y < 1);
	}
	SetMinSize(newSize);
	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_NORMAL, WXK_UP, 7865);
	entries[1].Set(wxACCEL_NORMAL, WXK_DOWN, 7866);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	//Bind(wxEVT_CHAR_HOOK, &KaiChoice::OnKeyHook, this);
}

KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
	const wxSize& size, const wxArrayString &choices,
	long style, const wxValidator& validator)
	:wxWindow(parent, id, pos, size, style/* | wxWANTS_CHARS*/)
	, bmp(NULL)
	, list(NULL)
	, itemList(NULL)
	, choiceText(NULL)
	, listIsShown(false)
	, enter(false)
	, clicked(false)
	, focusSet(false)
	, choice(-1)
	, foreground(WINDOW_TEXT)
{
	list = new wxArrayString(choices);
	disabled = new std::map<int, bool>();


	wxWindow::SetFont(parent->GetFont());
	wxSize newSize = size;
	if (size.x < 1 || size.y < 1){
		CalcMaxWidth(&newSize, size.x < 1, size.y < 1);
	}
	SetMinSize(newSize);
	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_NORMAL, WXK_UP, 7865);
	entries[1].Set(wxACCEL_NORMAL, WXK_DOWN, 7866);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	//Bind(wxEVT_CHAR_HOOK, &KaiChoice::OnKeyHook, this);
}

KaiChoice::KaiChoice(wxWindow *parent, int id, const wxString &comboBoxText, const wxPoint& pos,
	const wxSize& size, const wxArrayString &choices,
	long style, const wxValidator& validator)
	:wxWindow(parent, id, pos, size, style | KAI_COMBO_BOX /*| wxWANTS_CHARS*/)
	, bmp(NULL)
	, list(NULL)
	, itemList(NULL)
	, choiceText(NULL)
	, listIsShown(false)
	, enter(false)
	, clicked(false)
	, focusSet(false)
	, choice(-1)
	, foreground(WINDOW_TEXT)
{
	list = new wxArrayString(choices);
	disabled = new std::map<int, bool>();


	wxWindow::SetFont(parent->GetFont());
	wxSize newSize = size;
	if (size.x < 1 || size.y < 1){
		CalcMaxWidth(&newSize, size.x < 1, size.y < 1);
	}
	SetMinSize(newSize);
	if (style & wxCB_READONLY){
		txtchoice = comboBoxText;
		choice = FindString(comboBoxText);
		return;
	}
	choiceText = new KaiTextCtrl(this, 27789, comboBoxText, wxPoint(1, 1),
		wxSize(newSize.x - 22, newSize.y - 2), wxBORDER_NONE, validator);
	choiceText->Bind(wxEVT_ENTER_WINDOW, &KaiChoice::OnMouseEvent, this, 27789);
	choiceText->Bind(wxEVT_LEAVE_WINDOW, &KaiChoice::OnMouseEvent, this, 27789);
	choiceText->Bind(wxEVT_MOUSEWHEEL, &KaiChoice::OnMouseEvent, this, 27789);
	choiceText->Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent &evt){
		focusSet = true; Refresh(false);/* choiceText->GetSelection(&sels, &sele);*/
	}, 27789);
	choiceText->Bind(wxEVT_KILL_FOCUS, [=](wxFocusEvent &evt){
		Refresh(false);
		choiceText->SetSelection(0, 0);
	}, 27789);
	choiceText->Bind(wxEVT_LEFT_UP, [=](wxMouseEvent &evt){
		if (focusSet){
			choiceText->SetSelection(0, -1, true);
			focusSet = false;
		}
		evt.Skip();
	}, 27789);
	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent &evt){
		SetSelectionByPartialName(choiceText->GetValue());
	}, 27789);
	//Connect(ID_TDEL,ID_TRETURN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&KaiTextCtrl::OnAccelerator);
	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		wxKeyEvent kevt;
		kevt.m_keyCode = WXK_UP;
		if (itemList&&itemList->IsShown()){ itemList->OnKeyPress(kevt); }
		else{ evt.SetId(7865); OnArrow(evt); }
	}, ID_TUP);
	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		wxKeyEvent kevt;
		kevt.m_keyCode = WXK_DOWN;
		if (itemList&&itemList->IsShown()){ itemList->OnKeyPress(kevt); }
		else{ evt.SetId(7866); OnArrow(evt); }
	}, ID_TDOWN);
	choiceText->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		long start, end;
		choiceText->GetSelection(&start, &end);
		size_t len = choiceText->GetLength();
		if (end == len && start != end)
			choiceText->SetSelection(end - 1, end - 1);
		else
			evt.Skip();
	}, ID_TLEFT);
	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_NORMAL, WXK_UP, 7865);
	entries[1].Set(wxACCEL_NORMAL, WXK_DOWN, 7866);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	choiceText->Bind(wxEVT_KEY_DOWN, &KaiChoice::OnKeyHook, this);
	//Bind(wxEVT_CHAR_HOOK, &KaiChoice::OnKeyHook, this);
}

KaiChoice::~KaiChoice()
{
	delete list;
	delete disabled;
	delete bmp;
}

void KaiChoice::SetToolTip(const wxString &tooltip)
{
	if (tooltip != L""){ toolTip = tooltip; }
	wxString tt = (choice >= 0 || (choiceText && !choiceText->GetValue().empty())) ?
		toolTip + L"\n" + GetString(choice) : tooltip;
	if (tt.length() > 1000){
		tt = tt.Mid(0, 1000) + L"...";
	}
	wxWindow::SetToolTip(tt);
	if (choiceText){ choiceText->SetToolTip(tt); }
}

bool KaiChoice::SetBackgroundColour(COLOR col)
{
	if (choiceText){ choiceText->SetBackgroundColour(col); }
	return true;
}

bool KaiChoice::SetForegroundColour(COLOR col)
{
	if (choiceText){ choiceText->SetForegroundColour(col); }
	foreground = col;
	return true;
}

void KaiChoice::OnSize(wxSizeEvent& event)
{
	wxSize newSize = GetClientSize();
	if (choiceText){
		choiceText->SetSize(wxSize(newSize.x - 22, newSize.y - 2));
	}
	Refresh(false);
}

void KaiChoice::OnPaint(wxPaintEvent& event)
{
	//wxColour background = GetParent()->GetBackgroundColour();
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(w, h); }
	tdc.SelectObject(*bmp);
	tdc.SetFont(GetFont());
	bool enabled = IsThisEnabled();
	if (choiceText && choiceText->IsThisEnabled() != enabled){
		choiceText->Enable(enabled);
	}
	tdc.SetBrush(wxBrush((enter && !clicked) ? Options.GetColour(BUTTON_BACKGROUND_HOVER) :
		(clicked) ? Options.GetColour(BUTTON_BACKGROUND_PUSHED) :
		(enabled) ? Options.GetColour((GetWindowStyle() &KAI_COMBO_BOX) ? TEXT_FIELD_BACKGROUND :
		(HasFocus()) ? BUTTON_BACKGROUND_ON_FOCUS : BUTTON_BACKGROUND) :
		Options.GetColour(WINDOW_BACKGROUND_INACTIVE)));
	tdc.SetPen(wxPen((enter && !clicked) ? Options.GetColour(BUTTON_BORDER_HOVER) :
		(clicked) ? Options.GetColour(BUTTON_BORDER_PUSHED) :
		(HasFocus()) ? Options.GetColour(BUTTON_BORDER_ON_FOCUS) :
		(enabled) ? Options.GetColour(BUTTON_BORDER) :
		Options.GetColour(BUTTON_BORDER_INACTIVE)));
	tdc.DrawRectangle(0, 0, w, h);

	if (w > 15){
		wxBitmap arrow = wxBITMAP_PNG(L"arrow_list");
		tdc.DrawBitmap((enabled && list->size() > 0) ? arrow : arrow.ConvertToDisabled(), w - 17, (h - 10) / 2);

		if ((choice >= 0 || !txtchoice.IsEmpty()) && choice < (int)list->size()){
			int fh = 0, fw = w, ex = 0, et = 0;
			wxString txt = (txtchoice.IsEmpty()) ? (*list)[choice] : txtchoice;
			int removed = 0;
			while (fw > w - 22 && txt != L""){
				tdc.GetTextExtent(txt, &fw, &fh, &ex, &et/*, &font*/);
				txt = txt.RemoveLast();
				removed++;
			}
			if (removed < 2){
				txt = (txtchoice.IsEmpty()) ? (*list)[choice] : txtchoice;
			}
			else{
				txt = txt.RemoveLast(2) + L"...";
			}
			if (!choiceText){
				tdc.SetTextForeground((enabled) ? Options.GetColour(foreground) :
					Options.GetColour(WINDOW_TEXT_INACTIVE));
				//tdc.DrawText(txt, 4, (h-fh));
				wxRect cur(5, (h - fh) / 2, w - 19, fh);
				tdc.SetClippingRegion(cur);
				tdc.DrawLabel(txt, cur, wxALIGN_LEFT);
				tdc.DestroyClippingRegion();
			}
		}
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}

void KaiChoice::OnMouseEvent(wxMouseEvent &event)
{
	if (!listIsShown){
		if (event.LeftDown() && list->size() > 0){
			clicked = true;
			Refresh(false);
			SetFocus();
			UnsetToolTip();
			ShowList();

			if (choiceText){
				choiceText->SetFocus();
				choiceText->SetSelection(0, choiceText->GetValue().length(), true);
			}

			return;
		}

		if (event.LeftUp()){
			clicked = false;
			Refresh(false);
		}
		if (event.Entering() && list->size() > 0){
			enter = true;
			Refresh(false);
			return;
		}
		if (event.Leaving() && enter){
			if (choiceText){
				wxPoint pos = ScreenToClient(wxGetMousePosition());
				if (GetClientRect().Contains(pos)) return;
			}
			enter = false;
			clicked = false;
			Refresh(false);
			return;
		}
	}
	if (event.GetWheelRotation() != 0) {
		if (HasFlag(KAI_SCROLL_ON_FOCUS) && !HasFocus() && !(choiceText && choiceText->HasFocus())){
			event.Skip(); return;
		}
		if (list->size() < 1){ event.Skip(); return; }
		if (itemList && itemList->IsShown()){
			itemList->OnMouseEvent(event);
			return;
		}
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		choice -= step;
		if (choice < 0){ choice = list->size() - 1; }
		else if (choice >= (int)list->size()){ choice = 0; }
		SelectChoice(choice, false);
	}
}


void KaiChoice::OnKeyPress(wxKeyEvent &event)
{
	int key = event.GetKeyCode();
	if (itemList && itemList->IsShown()){
		itemList->OnKeyPress(event);
	}
	else if (key == WXK_RETURN && !(GetWindowStyle() & wxTE_PROCESS_ENTER)){
		ShowList();
	}
}

void KaiChoice::OnKeyHook(wxKeyEvent &event)
{
	if (event.GetKeyCode() == WXK_RETURN && itemList && itemList->IsShown()){

		itemList->EndPartialModal(itemList->sel);
		wxCommandEvent evt((HasFlag(KAI_COMBO_BOX)) ? wxEVT_COMMAND_COMBOBOX_SELECTED : wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
		return;
	}
	else if (event.GetKeyCode() == WXK_ESCAPE && itemList && itemList->IsShown()){
		itemList->EndPartialModal(-3);
		listIsShown = false;
		return;
	}
	event.Skip();
}

void KaiChoice::OnArrow(wxCommandEvent &evt)
{
	int id = evt.GetId();

	bool up = id == 7865;
	if (choice <= 0 && up || choice >= (int)list->size() - 1 && !up)return;
	if (choiceText){
		int result = FindString(choiceText->GetValue(), true);
		if (result < 0){
			SetSelectionByPartialName(choiceText->GetValue(), true);
			return;
		}
	}
	choice += (up) ? -1 : 1;
	SelectChoice(choice, false);
}

void KaiChoice::ShowList()
{
	listIsShown = true;
	wxSize listSize = GetSize();
	if (!itemList){ itemList = new PopupList(this, list, disabled); }
	if (choiceText){ SetSelectionByPartialName(choiceText->GetValue(), false, true); }
	itemList->Popup(wxPoint(0, listSize.GetY()), listSize, choice);

}


void KaiChoice::SetSelection(int sel, bool changeText)
{
	if (sel >= (int)list->size()){ return; }
	choice = sel;
	txtchoice = (sel < 0) ? L"" : (*list)[sel];

	if (itemList && itemList->IsShown()){
		itemList->SetSelection(choice);
	}
	if (choiceText && changeText){
		wxString txt = (sel < 0) ? L"" : (*list)[sel];
		choiceText->SetValue(txt);
	}
	Refresh(false);

	if (sel >= 0){ SetToolTip(); }
	else{ SetToolTip(toolTip); }
}

void KaiChoice::Clear()
{
	list->Clear();
}

int KaiChoice::Append(const wxString &what)
{
	list->Add(what);
	return list->size() - 1;
}

void KaiChoice::Append(const wxArrayString &itemsArray)
{
	list->insert(list->end(), itemsArray.begin(), itemsArray.end());
}

void KaiChoice::PutArray(wxArrayString *arr)
{
	wxString ce = (choice >= 0 && choice < (int)list->size()) ? (*list)[choice] : L"";
	if (list){ delete list; }
	list = new wxArrayString(*arr);
	if (list->size() < 1){ choice = -1; ce = L""; }
	if (itemList){ itemList->Destroy(); itemList = NULL; }
	if (ce != L""){
		if (choice >= (int)list->size()){
			choice = 0;
		}
		if (ce != (*list)[choice]){
			choice = list->Index(ce);
		}
	}
	Refresh(false);
}

int KaiChoice::GetCount()
{
	return list->size();
}

void KaiChoice::EnableItem(int numItem, bool enable)
{
	auto disabledResult = disabled->find(numItem);
	if (enable && disabledResult != disabled->end()){
		disabled->erase(disabledResult);
	}
	else if (!enable && !(disabledResult != disabled->end())){
		(*disabled)[numItem] = false;
	}

}

int KaiChoice::FindString(const wxString &text, bool caseSensitive)
{
	if (text.empty())
		return -1;

	return list->Index(text, caseSensitive);
}

void KaiChoice::Delete(int num, int nRemove/*=1*/)
{
	list->RemoveAt(num);
}

void KaiChoice::SendEvent(int _choice)
{
	enter = false;
	clicked = false;
	if (_choice >= 0){
		//choice = _choice; //Refresh(false);
		if (choiceText){ choiceText->SetModified(true); }
		SetSelection(_choice);
		wxCommandEvent evt((HasFlag(KAI_COMBO_BOX)) ? wxEVT_COMMAND_COMBOBOX_SELECTED : wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
	}
	else{
		Refresh(false);
	}
	SetToolTip();
	listIsShown = false;

}

void KaiChoice::SetSelectionByPartialName(const wxString &PartialName, bool setText/* = false*/, bool selectOnList/*=false*/)
{
	wxCommandEvent evt(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
	this->ProcessEvent(evt);
	int scrollTo = -1;//(changeText) ? 0 : -1;
	wxString PrtName = PartialName.Lower();
	size_t k = 0;
	int lastMatch = 0;
	if (PartialName == L""){
		goto done;
	}
	for (size_t i = 0; i < list->size(); i++){
		wxString fontname = (*list)[i].Lower();
		if (fontname.length() < 1 || fontname[0] < PrtName[0])
			continue;

		while (k < PrtName.length() && k < fontname.length()){
			if (fontname[k] == PrtName[k]){
				k++;
				lastMatch = i;
				if (k >= PrtName.length()){
					scrollTo = i;
					goto done;
				}
			}
			else if (k > 0 && fontname.Mid(0, k) != PrtName.Mid(0, k)){
				goto done;
			}
			else if (fontname[k] > PrtName[k]){
				scrollTo = i;
				goto done;
			}
			else
				break;
		}
	}


done:
	if (scrollTo < 0)
		scrollTo = lastMatch;

	if (itemList){
		itemList->sel = (selectOnList) ? scrollTo : -1;
		itemList->ScrollTo(scrollTo);
	}
	if (setText){
		SetSelection(scrollTo, true);
		return;
	}

	choice = (selectOnList) ? scrollTo : -1;
	SetToolTip();
	Refresh(false);
}

void KaiChoice::SetValue(const wxString &text){
	if (choiceText){ choiceText->SetValue(text); }
	else{ txtchoice = text; }
	choice = -1;
}

wxString KaiChoice::GetValue(){
	if (choiceText){
		return choiceText->GetValue();
	}
	return txtchoice;
}

void KaiChoice::SelectChoice(int _choice, bool select, bool sendEvent){
	choice = _choice;
	txtchoice = (*list)[choice];
	if (choiceText){
		choiceText->SetValue((*list)[choice], true);
		if (select){
			choiceText->SetFocus();
			choiceText->SetSelection(0, choiceText->GetValue().length(), true);
		}
	}
	else{
		Refresh(false);
	}
	if (sendEvent){
		wxCommandEvent evt((HasFlag(KAI_COMBO_BOX)) ? wxEVT_COMMAND_COMBOBOX_SELECTED : wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
	}
	if (choice >= 0){ SetToolTip(); }
	else{ SetToolTip(toolTip); }
}

wxString KaiChoice::GetString(int pos){
	if (choiceText){
		return choiceText->GetValue();
	}
	else if (pos < 0 || pos >= (int)list->size()){
		return L"";
	}
	return (*list)[pos];
}

void KaiChoice::Insert(const wxString &what, int position){
	int pos = MID(0, position, (int)list->size() - 1);
	list->Insert(what, pos);
}

void KaiChoice::Sort()
{
	list->Sort([](const wxString &first, const wxString &second){return first.CmpNoCase(second); });
}

void KaiChoice::OnActivate(wxFocusEvent &evt)
{
	Refresh(false);
}

bool KaiChoice::HasFocus()
{
	return (wxWindow::HasFocus() || (choiceText && choiceText->HasFocus()));
}

void KaiChoice::SetFocus()
{
	if (choiceText){ choiceText->SetFocus(); }
	else{ wxWindow::SetFocus(); }
}

void KaiChoice::SetMaxLength(int maxLen)
{
	if (choiceText){ choiceText->SetMaxLength(maxLen); }
}

bool KaiChoice::SetFont(const wxFont &font)
{
	wxWindow::SetFont(font);
	if (choiceText){
		choiceText->SetFont(font);
	}
	wxSize newSize = GetMinSize();
	CalcMaxWidth(&newSize, false, true);

	SetMinSize(newSize);
	Refresh(false);
	return true;
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiChoice, wxWindow);

BEGIN_EVENT_TABLE(KaiChoice, wxWindow)
EVT_MOUSE_EVENTS(KaiChoice::OnMouseEvent)
EVT_PAINT(KaiChoice::OnPaint)
EVT_SIZE(KaiChoice::OnSize)
EVT_ERASE_BACKGROUND(KaiChoice::OnEraseBackground)
EVT_SET_FOCUS(KaiChoice::OnActivate)
EVT_KILL_FOCUS(KaiChoice::OnActivate)
EVT_KEY_UP(KaiChoice::OnKeyPress)
EVT_MENU_RANGE(7865, 7866, KaiChoice::OnArrow)
END_EVENT_TABLE()

static int maxVisible = 20;

PopupList::PopupList(wxWindow *DialogParent, wxArrayString *list, std::map<int, bool> *disabled)
/*:wxFrame(DialogParent,-1,"",wxDefaultPosition, wxDefaultSize, wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP|wxWS_EX_TRANSIENT)*/
: wxPopupWindow(DialogParent/*, wxBORDER_NONE | wxWANTS_CHARS*/)
, sel(0)
, scPos(0)
, scroll(NULL)
, orgY(0)
, bmp(NULL)
, Parent(DialogParent)
, itemsList(list)
, disabledItems(disabled)
{
	int fw = 0;
	SetFont(DialogParent->GetFont());
	GetTextExtent(L"#TWFfGH", &fw, &height);
	height += 6;
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
	Bind(wxEVT_IDLE, &PopupList::OnIdle, this);
	if (scroll){
		scroll->SetSize(size.x - 18, 1, 17, size.y - 2);
	}
}

void PopupList::CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize)
{
	int tx = 0, ty = 0;
	size_t isize = itemsList->size();
	for (size_t i = 0; i < isize; i++){
		GetTextExtent((*itemsList)[i], &tx, &ty);
		if (tx > size->x){ size->x = tx; }
	}

	size->x += 18;
	if (isize > (size_t)maxVisible) { size->x += 20; isize = maxVisible; }
	if (size->x > 400){ size->x = 400; }
	if (size->x < controlSize.x){ size->x = controlSize.x; }
	size->y = height * isize + 2;
	wxPoint ScreenPos = Parent->ClientToScreen(*pos);
	wxRect workArea = GetMonitorRect(0, NULL, ScreenPos, true);
	int h = workArea.height + workArea.y;
	//fix for new wxWidgets
	//*pos = ScreenPos;
	if ((ScreenPos.y + size->y) > h){
		pos->y -= (size->y + controlSize.y);
	}
}

void PopupList::OnMouseEvent(wxMouseEvent &evt)
{
	bool leftdown = evt.LeftDown();
	int x = evt.GetX();
	int y = evt.GetY();
	wxSize sz = GetClientSize();
	if (leftdown){
		wxPoint posOnScreen = wxGetMousePosition();
		bool contains = false;
		int x, y;
		wxPopupWindowBase::DoGetPosition(&x, &y);
		wxRect rc = GetRect();
		rc.x = x; rc.y = y;
		if (!rc.Contains(posOnScreen)){
			EndPartialModal(-3);
		}
		return;
	}

	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -= step;
		if (scPos<0){ scPos = 0; }
		else if (scPos >(int)itemsList->size() - maxVisible){ scPos = itemsList->size() - maxVisible; }
		Refresh(false);
		return;
	}

	int elem = y / height;
	elem += scPos;
	if (elem >= (int)itemsList->size() || elem < 0 || x < 0 || x > sz.x || y <0 || y > sz.y){ return; }
	if (elem != sel){
		if (elem >= scPos + maxVisible || elem < scPos){ return; }
		sel = elem;
		Refresh(false);
	}


	if (evt.LeftUp() && !(disabledItems->find(elem) != disabledItems->end()) && !(x < 0 || x > sz.x || y <0 || y > sz.y)){
		EndPartialModal(elem);
	}
	//evt.Skip();
}

void PopupList::OnPaint(wxPaintEvent &event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	int itemsize = itemsList->size();
	if (scPos >= itemsize - maxVisible){ scPos = itemsize - maxVisible; }
	if (scPos < 0){ scPos = 0; }
	int maxsize = itemsize;
	int ow = w;
	if (itemsize > maxVisible){
		maxsize = maxVisible;
		if (!scroll){
			scroll = new KaiScrollbar(this, -1, wxPoint(w - 18, 1), wxSize(17, h - 2), wxVERTICAL);
			scroll->SetScrollRate(3);
		}
		scroll->SetScrollbar(scPos, maxVisible, itemsize, maxVisible - 1);
		w -= 18;
	}

	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < ow || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(ow, h); }
	tdc.SelectObject(*bmp);
	const wxColour & text = Options.GetColour(WINDOW_TEXT);
	const wxColour & graytext = Options.GetColour(WINDOW_TEXT_INACTIVE);

	tdc.SetFont(GetFont());
	tdc.SetBrush(wxBrush(Options.GetColour(MENUBAR_BACKGROUND)));
	tdc.SetPen(wxPen(Options.GetColour(WINDOW_BORDER)));
	tdc.DrawRectangle(0, 0, ow, h);
	//tdc.SetTextForeground(Options.GetColour("Menu Bar Border Selection"));
	for (int i = 0; i < maxsize; i++)
	{
		int scrollPos = i + scPos;

		if (scrollPos == sel){
			tdc.SetPen(wxPen(Options.GetColour(MENU_BORDER_SELECTION)));
			tdc.SetBrush(wxBrush(Options.GetColour(MENU_BACKGROUND_SELECTION)));
			tdc.DrawRectangle(2, (height*i) + 2, w - 4, height - 2);
		}
		wxString desc = (*itemsList)[scrollPos];
		if (desc.length() > 1000)
			desc = desc.Mid(0, 1000);

		tdc.SetTextForeground((disabledItems->find(scrollPos) != disabledItems->end()) ? graytext : text);
		tdc.DrawText(desc, 4, (height*i) + 3);
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, ow, h, &tdc, 0, 0);
}

void PopupList::SetSelection(int pos){
	sel = pos;
	if (sel < scPos && sel != -1){
		scPos = sel;
	}
	else if (sel >= scPos + maxVisible && (sel - maxVisible + 1) >= 0){
		scPos = sel - maxVisible + 1;
	}
	Refresh(false);
};

void PopupList::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

//Odwo³uje pêtlê czekaj¹c¹
void PopupList::EndPartialModal(int ReturnId)
{
	((KaiChoice*)Parent)->SendEvent(ReturnId);
	Unbind(wxEVT_IDLE, &PopupList::OnIdle, this);
	if (HasCapture()){ ReleaseMouse(); }
	Hide();
	((KaiChoice*)Parent)->SetFocus();
}

void PopupList::OnKeyPress(wxKeyEvent &event)
{
	/*if (event.GetKeyCode() == WXK_RETURN){
		EndPartialModal(sel);
		wxCommandEvent evt((HasFlag(KAI_COMBO_BOX)) ? wxEVT_COMMAND_COMBOBOX_SELECTED : wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
		}
		else if (event.GetKeyCode() == WXK_ESCAPE){
		EndPartialModal(-3);
		((KaiChoice*)Parent)->listIsShown = false;
		}
		else */if (event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_DOWN){
			int step = (event.GetKeyCode() == WXK_DOWN) ? 1 : -1;
			sel += step;
			if (sel < scPos && sel != -1){
				scPos = sel;
			}
			else if (sel >= scPos + maxVisible && (sel - maxVisible + 1) >= 0){
				scPos = sel - maxVisible + 1;
			}
			if (sel >= (int)itemsList->size()){
				sel = 0;
				scPos = 0;
			}
			else if (sel < 0){
				sel = itemsList->size() - 1;
				scPos = sel;
			}
			((KaiChoice*)Parent)->SelectChoice(sel, true, false);
			Refresh(false);
		}
}


void PopupList::OnIdle(wxIdleEvent& event)
{
	event.Skip();

	if (!Parent->IsShownOnScreen()){
		EndPartialModal(-3);
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
			if (!HasCapture() && !(scroll && scroll->HasCapture()))
			{
				CaptureMouse();
			}
		}
	}
}


BEGIN_EVENT_TABLE(PopupList, wxPopupWindow)
EVT_MOUSE_EVENTS(PopupList::OnMouseEvent)
EVT_PAINT(PopupList::OnPaint)
EVT_SCROLL(PopupList::OnScroll)
EVT_MOUSE_CAPTURE_LOST(PopupList::OnLostCapture)
END_EVENT_TABLE()

