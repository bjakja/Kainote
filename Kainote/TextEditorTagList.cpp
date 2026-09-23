//  Copyright (c) 2018 - 2026, Marcin Drob

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

#include "TextEditorTagList.h"
#include "KaiScrollbar.h"
#include "Menu.h"
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include "config.h"
#include "UtilsWindows.h"

const int maxVisible = 10;

PopupWindow::PopupWindow(wxWindow *DialogParent, PopupTagList *Parent, int Height)
: wxPopupWindow(DialogParent)
, sel(0)
, scrollPositionV(0)
, scroll(nullptr)
, bmp(nullptr)
, parent(Parent)
, height(Height)
{
	int fw = 0;
	SetFont(DialogParent->GetFont());
	//GetTextExtent(L"#TWFfGH", &fw, &height);
	//height += 6;
	Bind(wxEVT_MOTION, &PopupWindow::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &PopupWindow::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &PopupWindow::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &PopupWindow::OnMouseEvent, this);
	Bind(wxEVT_PAINT, &PopupWindow::OnPaint, this);
	Bind(wxEVT_SCROLL_THUMBTRACK, &PopupWindow::OnScroll, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &PopupWindow::OnLostCapture, this);
	
}

PopupWindow::~PopupWindow()
{
	wxDELETE(bmp);
}

void PopupWindow::Popup(const wxPoint &pos, const wxSize &size, int selectedItem)
{
	SetSelection(selectedItem);
	
	SetPosition(pos);
	SetSize(size);
	Show();
	
	if (scroll){
		int thickness = scroll->GetThickness();
		scroll->SetSize(wxMax(0, size.x - thickness - 1), 1, thickness, wxMax(0, size.y - 2));
	}
}

void PopupTagList::CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize)
{
	int tx = 0, ty = 0;
	size_t isize = GetCount();
	wxString items;
	for (size_t i = 0; i < itemsList.size(); i++){
		itemsList[i]->GetTagText(&items);
		Parent->GetTextExtent(items, &tx, &ty);
		if (tx > size->x){ size->x = tx; }
		if (ty > height) { height = ty + 6; }
	}

	size->x += 18;
	if (isize > (size_t)maxVisible) { 
		size->x += 20; 
		isize = maxVisible; 
	}
	if (size->x > 800){ size->x = 800; }
	if (size->x < controlSize.x){ size->x = controlSize.x; }
	size->y = height * isize + 2;
	wxPoint ScreenPos = Parent->ClientToScreen(*pos);
	wxRect workArea = GetMonitorWorkArea(0, nullptr, ScreenPos, true);
	int h = workArea.height + workArea.y;
	if ((ScreenPos.y + size->y) > h){
		pos->y -= (size->y + controlSize.y);
	}
}

void PopupWindow::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockMouseEvent){
		blockMouseEvent = false;
		return;
	}
	int x = evt.GetX();
	int y = evt.GetY();
	wxSize sz = GetClientSize();
	int itemsSize = parent->GetCount();

	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scrollPositionV -= step;
		if (scrollPositionV<0){ scrollPositionV = 0; }
		else if (scrollPositionV > itemsSize - maxVisible){ scrollPositionV = itemsSize - maxVisible; }
		Refresh(false);
		return;
	}

	int elem = y / height;
	elem += scrollPositionV;
	if (elem >= itemsSize || elem < 0/* || x < 0 || x > sz.x || y <0 || y > sz.y*/){ return; }
	if (elem != sel){
		if (elem >= scrollPositionV + maxVisible || elem < scrollPositionV){ return; }
		sel = elem;
		Refresh(false);
	}
	if (evt.LeftUp()){
		wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetParent()->GetId());
		this->ProcessEvent(evt);
	}
	else if (evt.RightUp()){
		int options = Options.GetInt(TEXT_EDITOR_TAG_LIST_OPTIONS);
		Menu listMenu;
		listMenu.Append(ID_SHOW_DESCRIPTION, _("Show description"), nullptr, emptyString, ITEM_CHECK_AND_HIDE)->Check((options & SHOW_DESCRIPTION) != 0);
		listMenu.Append(ID_SHOW_ALL_TAGS, _("Show all tags"), nullptr, emptyString, ITEM_CHECK_AND_HIDE)->Check((options & TYPE_TAG_USED_IN_VISUAL) != 0);
		listMenu.Append(ID_SHOW_VSFILTER_MOD_TAGS, _("Show VSFiltermod tags"), nullptr, emptyString, ITEM_CHECK_AND_HIDE)->Check((options & TYPE_TAG_VSFILTER_MOD) != 0);
		
		int id = listMenu.GetPopupMenuSelection(evt.GetPosition(), this);
		if (id < 1)
			return;

		int numChecked = id - ID_SHOW_ALL_TAGS;
		if (numChecked < 0 || numChecked > 2)
			return;

		int changedOption = 1 << numChecked;
		
		options ^= changedOption;
		Options.SetInt(TEXT_EDITOR_TAG_LIST_OPTIONS, options);
		parent->FilterListViaOptions(options);
	}
	//evt.Skip();
}

void PopupWindow::OnPaint(wxPaintEvent &event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w < 1 || h < 1){ return; }
	int itemsize = parent->GetCount();
	if (scrollPositionV >= itemsize - maxVisible){ 
		scrollPositionV = itemsize - maxVisible; 
	}
	if (scrollPositionV < 0){ 
		scrollPositionV = 0; 
	}
	int maxsize = itemsize;
	int ow = w;
	if (itemsize > maxVisible){
		maxsize = maxVisible;
		if (!scroll){
			int thickness = KaiScrollbar::CalculateThickness(this);
			scroll = new KaiScrollbar(this, -1, wxPoint(w - thickness - 1, 1), wxSize(thickness, h - 2), wxVERTICAL);
			scroll->SetScrollRate(3);
		}
		scroll->SetScrollbar(scrollPositionV, maxVisible, itemsize, maxVisible - 1);
		w -= (scroll->GetThickness() + 1);
	}
	else if (scroll){
		scroll->Destroy();
		scroll = nullptr;
	}

	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < ow || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = nullptr;
	}
	if (!bmp){ bmp = new wxBitmap(ow, h); }
	tdc.SelectObject(*bmp);
	const wxColour & text = Options.GetColour(WINDOW_TEXT);

	tdc.SetFont(GetFont());
	tdc.SetBrush(wxBrush(Options.GetColour(MENUBAR_BACKGROUND)));
	tdc.SetPen(wxPen(Options.GetColour(WINDOW_BORDER)));
	tdc.DrawRectangle(0, 0, ow, h);
	int keyPosition = parent->FindItemById(scrollPositionV);
	//it should not return -1 here, but sanity checks needed
	if (keyPosition < 0)
		keyPosition = 0;

	int i = keyPosition, k = 0;
	while (k < maxsize && i < parent->itemsList.size())
	{
		TagListItem *item = parent->itemsList[i];
		if (!item->isVisible){
			i++;
			continue;
		}
		if ((k + scrollPositionV) == sel){
			tdc.SetPen(wxPen(Options.GetColour(MENU_BORDER_SELECTION)));
			tdc.SetBrush(wxBrush(Options.GetColour(MENU_BACKGROUND_SELECTION)));
			tdc.DrawRectangle(2, (height * k) + 2, w - 4, height - 2);
		}
		wxString desc;
		item->GetTagText(&desc);

		tdc.SetTextForeground(text);
		tdc.DrawText(desc, 4, (height * k) + 3);

		i++;
		k++;
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, ow, h, &tdc, 0, 0);
}

void PopupWindow::SetSelection(int pos){
	int elemConut = parent->GetCount();
	if (pos < 0)
		pos = elemConut - 1;
	else if (pos >= elemConut)
		pos = 0;

	sel = pos;
	if (sel < scrollPositionV && sel != -1){
		scrollPositionV = sel; 
	}
	else if (sel >= scrollPositionV + maxVisible && (sel - maxVisible + 1) >= 0){ 
		scrollPositionV = sel - maxVisible + 1; 
	}
	Refresh(false);
}

PopupTagList::PopupTagList(wxWindow *DialogParent) 
	: Parent(DialogParent) 
{ 
	//get options from config
	int options = Options.GetInt(TEXT_EDITOR_TAG_LIST_OPTIONS);
	InitList(options); 
}

PopupTagList::~PopupTagList() { 
	if (popup) popup->Destroy(); 
	for (auto item : itemsList) {
		delete item;
	}
}

void PopupTagList::FilterListViaOptions(int otptions)
{
	for (auto item : itemsList){
		item->ShowItem(otptions);
	}
	Popup(position, controlSize, 0);
}

void PopupTagList::FilterListViaKeyword(const wxString &_keyWord, bool setKeyword)
{
	if (setKeyword)
		keyWord = _keyWord;
	for (auto item : itemsList){
		item->ShowItem(_keyWord);
	}
	if (lastItems > GetCount()) {
		Popup(position, controlSize, 0);
	}
	else if (popup) {
		popup->SetSelection((GetCount()) ? 0 : -1);
		popup->Refresh(false);
	}
}

size_t PopupTagList::GetCount()
{
	size_t count = 0;
	for (auto item : itemsList){
		if (item->isVisible)
			count++;
	}
	return count;
}

int PopupTagList::FindItemById(int id)
{
	if (id < 0)
		return -1;

	size_t idCount = 0;
	size_t keyCount = 0;
	for (auto item : itemsList){
		
		if (item->isVisible){
			if (idCount == id)
				return keyCount;
			idCount++;
		}

		keyCount++;
	}
	if (idCount == id && keyCount < itemsList.size())
		return keyCount;

	return -1;
}


TagListItem * PopupTagList::GetItem(int pos)
{
	int keyPos = FindItemById(pos);
	if (keyPos >= 0)
		return itemsList[keyPos];

	return nullptr;
}

void PopupWindow::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	if (scrollPositionV != newPos) {
		scrollPositionV = newPos;
		Refresh(false);
	}
}


void PopupTagList::InitList(int option)
{
	itemsList.push_back(new TagListItem(L"1a", _("Transparency of primary color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"2a", _("Transparency of secondary color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"3a", _("Transparency of border color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"4a", _("Transparency of shadow color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"1c", _("Primary color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"2c", _("Secondary color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"3c", _("Border color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"4c", _("Shadow color"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"1img", _("PNG mask of primary color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"2img", _("PNG mask of secondary color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"3img", _("PNG mask of border color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"4img", _("PNG mask of shadow color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"1va", _("Transparency gradient of primary color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"2va", _("Transparency gradient of secondary color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"3va", _("Transparency gradient of border color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"4va", _("Transparency gradient of shadow color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"1vc", _("Gradient of primary color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"2vc", _("Gradient of secondary color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"3vc", _("Gradient of border color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"4vc", _("Gradient of shadow color"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"a", _("Text alignment (SSA)"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"alpha", _("Transparency"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"an", _("Text position"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"b", _("Bold text"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"be", _("Edge blur"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"blur", _("Blur of border, shadow or font"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"bord", _("Thickness of border"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"clip", _("Vector clip / rectangle clip"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"distort", _("Font distortion"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"fad", _("Fading in / fading out of text"), TYPE_NORMAL, option, true));
	itemsList.push_back(new TagListItem(L"fade", _("Fading in / fading out of text (advanced version)"), TYPE_NORMAL, option, true));
	itemsList.push_back(new TagListItem(L"fax", _("Skew on X axis"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fay", _("Skew on Y axis"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fe", _("Text encoding"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fn", _("Font name"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"frs", _("Text rounding"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"frx", _("Rotation on X axis"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fry", _("Rotation on Y axis"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"frz", _("Rotation on Z axis"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fs", _("Font size"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fsc", _("Scale on X and Y axes"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"fscx", _("Scale on X axis"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fscy", _("Scale on Y axis"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fsp", _("Font spacing"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fsvp", _("Vertical font spacing"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"i", _("Italic text"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"iclip", _("Reverse vector clip or rectangle clip"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"jitter", _("Text jitter"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"k", _("Karaoke timing"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"K", _("Karaoke timing smooth transition"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"ko", _("Karaoke timing border"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"kt", _("Karaoke timing (not supported)"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"move", _("Text movement"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"mover", _("Text movement along a circle"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"moves3", _("Text movement along a curve (3 points)"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"moves4", _("Text movement along a curve (4 points)"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"movevc", _("Vector drawing movement"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"org", _("Anchor for rotation"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"p", _("Drawing and its scale"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"pbo", _("Offset of Y vector points"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"pos", _("Text position"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"q", _("Text wrap method"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"r", _("Reset tags"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"rnd", _("Font point randomness"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rnds", _("Font point randomness seed"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rndx", _("Font point randomness on X axis"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rndy", _("Font point randomness on Y axis"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rndz", _("Font point randomness on Z axis"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"s", _("Text strikethrough"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"shad", _("Text shadow"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"t", _("Text animation"), TYPE_NORMAL, option, true));
	itemsList.push_back(new TagListItem(L"u", _("Text underline"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"xbord", _("Border on X axis"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"ybord", _("Border on Y axis"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"xshad", _("Shadow on X axis"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"yshad", _("Shadow on Y axis"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"z", _("Z coordinate for tags \\frx and \\fry"), TYPE_TAG_VSFILTER_MOD, option));
}

void PopupTagList::Popup(const wxPoint & pos, const wxSize & _controlSize, int selectedItem)
{
	if (popup)
		popup->Destroy();

	wxPoint npos = pos;
	wxSize size;
	CalcPosAndSize(&npos, &size, controlSize);
	
	if (size.y > 5) {
		popup = new PopupWindow(Parent, this, height);
		popup->Popup(pos, size, selectedItem);
	}
	else
		popup = nullptr;

	position = pos;
	controlSize = _controlSize;
	lastItems = GetCount();
}

void PopupTagList::AppendToKeyword(wxUniChar ch)
{
	keyWord << ch;

	FilterListViaKeyword(keyWord, false);
}
