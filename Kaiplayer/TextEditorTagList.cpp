//  Copyright (c) 2018, Marcin Drob

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
#include "config.h"
#include "Menu.h"

const int maxVisible = 15;

PopupTagList::PopupTagList(wxWindow *DialogParent)
: wxPopupWindow(DialogParent)
, sel(0)
, scrollPositionV(0)
, scroll(NULL)
, bmp(NULL)
, Parent(DialogParent)
{
	int fw = 0;
	SetFont(DialogParent->GetFont());
	GetTextExtent("#TWFfGH", &fw, &height);
	height += 6;
	Bind(wxEVT_MOTION, &PopupTagList::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &PopupTagList::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &PopupTagList::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &PopupTagList::OnMouseEvent, this);
	Bind(wxEVT_PAINT, &PopupTagList::OnPaint, this);
	Bind(wxEVT_SCROLL_THUMBTRACK, &PopupTagList::OnScroll, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &PopupTagList::OnLostCapture, this);
	//get options from config
	int options = Options.GetInt(TEXT_EDITOR_TAG_LIST_OPTIONS);
	InitList(options);
}

PopupTagList::~PopupTagList()
{
	wxDELETE(bmp);
	for (auto cur = itemsList.begin(); cur != itemsList.end(); cur++)
		delete *cur;
}

void PopupTagList::Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem)
{
	SetSelection(selectedItem);
	wxPoint npos = pos;//Parent->ClientToScreen(pos);
	wxSize size;
	CalcPosAndSize(&npos, &size, controlSize);
	SetPosition(npos);
	if (size.y < 10)
		size.y = 50;

	SetSize(size);
	Show();
	
	if (scroll){
		scroll->SetSize(size.x - 18, 1, 17, size.y - 2);
	}
}

void PopupTagList::CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &_controlSize)
{
	int tx = 0, ty = 0;
	controlSize = _controlSize;
	size_t isize = GetCount();
	wxString items;
	for (size_t i = 0; i < itemsList.size(); i++){
		itemsList[i]->GetTagText(&items);
		GetTextExtent(items, &tx, &ty);
		if (tx > size->x){ size->x = tx; }
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
	wxRect workArea = GetMonitorRect(0, NULL, ScreenPos, true);
	int h = workArea.height + workArea.y;
	if ((ScreenPos.y + size->y) > h){
		pos->y -= (size->y + controlSize.y);
	}
}

void PopupTagList::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockMouseEvent){
		blockMouseEvent = false;
		return;
	}
	int x = evt.GetX();
	int y = evt.GetY();
	wxSize sz = GetClientSize();
	int itemsSize = GetCount();

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
		wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
		this->ProcessEvent(evt);
	}
	else if (evt.RightUp()){
		int options = Options.GetInt(TEXT_EDITOR_TAG_LIST_OPTIONS);
		Menu listMenu;
		listMenu.Append(ID_SHOW_DESCRIPTION, _("Pokaż opis"), NULL, L"", ITEM_CHECK_AND_HIDE)->Check((options & SHOW_DESCRIPTION) != 0);
		listMenu.Append(ID_SHOW_ALL_TAGS, _("Pokaż wszystkie tagi"), NULL, L"", ITEM_CHECK_AND_HIDE)->Check((options & TYPE_TAG_USED_IN_VISUAL) != 0);
		listMenu.Append(ID_SHOW_VSFILTER_MOD_TAGS, _("Pokaż tagi VSFiltermoda"), NULL, L"", ITEM_CHECK_AND_HIDE)->Check((options & TYPE_TAG_VSFILTER_MOD) != 0);
		
		int id = listMenu.GetPopupMenuSelection(evt.GetPosition(), this);
		if (id < 1)
			return;

		int numChecked = id - ID_SHOW_ALL_TAGS;
		if (numChecked < 0 || numChecked > 2)
			return;

		int changedOption = 1 << numChecked;
		
		options ^= changedOption;
		Options.SetInt(TEXT_EDITOR_TAG_LIST_OPTIONS, options);
		FilterListViaOptions(options);
	}
	//evt.Skip();
}

void PopupTagList::OnPaint(wxPaintEvent &event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	int itemsize = GetCount();
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
			scroll = new KaiScrollbar(this, -1, wxPoint(w - 18, 1), wxSize(17, h - 2), wxVERTICAL);
			scroll->SetScrollRate(3);
		}
		scroll->SetScrollbar(scrollPositionV, maxVisible, itemsize, maxVisible - 1);
		w -= 18;
	}
	else if (scroll){
		scroll->Destroy();
		scroll = NULL;
	}

	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < ow || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(ow, h); }
	tdc.SelectObject(*bmp);
	const wxColour & text = Options.GetColour(WindowText);

	tdc.SetFont(GetFont());
	tdc.SetBrush(wxBrush(Options.GetColour(MenuBackground)));
	tdc.SetPen(wxPen(Options.GetColour(WindowBorder)));
	tdc.DrawRectangle(0, 0, ow, h);
	int keyPosition = FindItemById(scrollPositionV);
	//it should not return -1 here, but sanity checks needed
	if (keyPosition < 0)
		keyPosition = 0;

	int i = keyPosition, k = 0;
	while (k < maxsize && i < itemsList.size())
	{
		TagListItem *item = itemsList[i];
		if (!item->isVisible){
			i++;
			continue;
		}
		if ((k + scrollPositionV) == sel){
			tdc.SetPen(wxPen(Options.GetColour(MenuBorderSelection)));
			tdc.SetBrush(wxBrush(Options.GetColour(MenuBackgroundSelection)));
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

void PopupTagList::SetSelection(int pos){
	int elemConut = GetCount();
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
};

void PopupTagList::FilterListViaOptions(int otptions)
{
	for (auto item : itemsList){
		item->ShowItem(otptions);
	}
	wxPoint pos;
	wxSize size;
	//CalcPosAndSize(&pos, &size, controlSize);
	//sel = (GetCount()) ? 0 : -1;
	//Show(false);
	//wxWindowBase::SetSize(wxSize(500,300));
	//Show();
	Refresh(false);
}

void PopupTagList::FilterListViaKeyword(const wxString &keyWord)
{
	for (auto item : itemsList){
		item->ShowItem(keyWord);
	}
	sel = (GetCount()) ? 0 : -1;
	Refresh(false);
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

void PopupTagList::AppendToKeyword(wxUniChar ch)
{
	keyWord << ch;
	FilterListViaKeyword(keyWord);
}

TagListItem * PopupTagList::GetItem(int pos)
{
	int keyPos = FindItemById(pos);
	if (keyPos >= 0)
		return itemsList[keyPos];

	return NULL;
}

void PopupTagList::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	if (scrollPositionV != newPos) {
		scrollPositionV = newPos;
		Refresh(false);
	}
}


void PopupTagList::InitList(int option)
{
	itemsList.push_back(new TagListItem(L"1a", _("Przezroczystość koloru podstawowego"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"2a", _("Przezroczystość koloru pomocniczego"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"3a", _("Przezroczystość koloru obwódki"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"4a", _("Przezroczystość koloru cienia"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"1c", _("Kolor podstawowy"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"2c", _("Kolor pomocniczy"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"3c", _("Kolor obwódki"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"4c", _("Kolor cienia"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"1img", _("Maska PNG koloru podstawowego"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"2img", _("Maska PNG koloru pomocniczego"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"3img", _("Maska PNG koloru obwódki"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"4img", _("Maska PNG koloru cienia"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"1va", _("Gradient przezroczystości koloru podstawowego"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"2va", _("Gradient przezroczystości koloru pomocniczego"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"3va", _("Gradient przezroczystości koloru obwódki"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"4va", _("Gradient przezroczystości koloru cienia"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"1vc", _("Gradient koloru podstawowego"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"2vc", _("Gradient koloru pomocniczego"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"3vc", _("Gradient koloru obwódki"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"4vc", _("Gradient koloru cienia"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"a", _("Położenie tekstu (układ SSA)"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"alpha", _("Przezroczystość całego tekstu"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"an", _("Położenie tekstu"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"b", _("Pogrubienie tekstu"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"be", _("Rozmycie krawędzi"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"blur", _("Rozmycie"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"bord", _("Grubość obwódki"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"clip", _("Wyciknki wektorowe / prostokątne"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"distort", _("Deformacja czcionki"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"fad", _("Pojawianie / znikanie tekstu"), TYPE_NORMAL, option, true));
	itemsList.push_back(new TagListItem(L"fade", _("Pojawianie / znikanie tekstu (zaawansowana wersja)"), TYPE_NORMAL, option, true));
	itemsList.push_back(new TagListItem(L"fax", _("Pochylenie w osi X"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fay", _("Pochylenie w osi Y"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fe", _("Kodowanie znaków tekstu"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fn", _("Nazwa czcionki"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"frs", _("Zaokrąglenie tekstu"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"frx", _("Obrót w osi X"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fry", _("Obrót w osi Y"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"frz", _("Obrót w osi Z"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fs", _("Wielkość czcionki"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fsc", _("Skala w osi X i Y"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"fscx", _("Skala w osi X"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fscy", _("Skala w osi Y"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"fsp", _("Odstępy między znakami"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"fsvp", _("Odstęp między znakami w pionie"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"i", _("Kursywa / pochylenie tekstu"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"iclip", _("Odwrócone wycinki wektorowe / prostokątne"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"jitter", _("Trzęsienie tekstu"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"k", _("Timing karaoke"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"K", _("Timing karaoke płynne przejście"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"ko", _("Timing karaoke obwódka"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"kt", _("Timing karaoke (nieobsługiwane)"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"move", _("Ruch tekstu"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"mover", _("Ruch po okręgu"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"moves3", _("Ruch po krzywej (3 punkty)"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"moves4", _("Ruch po krzywej (4 punkty)"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"movevc", _("Ruch rysunku wektorowego"), TYPE_TAG_VSFILTER_MOD, option, true));
	itemsList.push_back(new TagListItem(L"org", _("Kotwica dla obrotów"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"p", _("Rysunek wektorowy i skala"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"pbo", _("Przesunięcie punktów wektora w osi Y"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"pos", _("Pozycja tekstu"), TYPE_TAG_USED_IN_VISUAL, option, true));
	itemsList.push_back(new TagListItem(L"q", _("Sposób łamania linii"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"r", _("Reset tagów"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"rnd", _("Losowość punktów czcionki"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rnds", _("Losowość punktów czcionki seed"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rndx", _("Losowość punktów czcionki oś X"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rndy", _("Losowość punktów czcionki oś Y"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"rndz", _("Losowość punktów czcionki oś Z"), TYPE_TAG_VSFILTER_MOD, option));
	itemsList.push_back(new TagListItem(L"s", _("Przekreślenie tekstu"), TYPE_TAG_USED_IN_VISUAL, option));
	itemsList.push_back(new TagListItem(L"shad", _("Cień tekstu"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"t", _("Animacja tekstu"), TYPE_NORMAL, option, true));
	itemsList.push_back(new TagListItem(L"u", _("Podkreślenie tekstu"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"xbord", _("Obwódka w osi X"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"ybord", _("Obwódka w osi Y"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"xshad", _("Cień w osi X"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"yshad", _("Cień w osi Y"), TYPE_NORMAL, option));
	itemsList.push_back(new TagListItem(L"z", _("Koordynata Z dla tagów \\frx i \\fry"), TYPE_TAG_VSFILTER_MOD, option));
}




