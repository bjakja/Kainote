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

#include "KaiListCtrl.h"
#include "KaiCheckBox.h"
#include "ColorPicker.h"
#include "config.h"
#include "Utils.h"
#include "Menu.h"
#include <wx/clipbrd.h>
#include <wx/dcclient.h>

//std::vector< ItemRow*> List::filteredList = std::vector< ItemRow*>();

wxDEFINE_EVENT(LIST_ITEM_LEFT_CLICK, wxCommandEvent);
wxDEFINE_EVENT(LIST_ITEM_DOUBLECLICKED, wxCommandEvent);
wxDEFINE_EVENT(LIST_ITEM_RIGHT_CLICK, wxCommandEvent);

wxSize Item::GetTextExtents(KaiListCtrl *theList){
	wxSize size = theList->GetTextExtent(name);
	size.x += 10;
	size.y += 4;
	return size;
}

void ItemText::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /*= NULL*/)
{
	if (enter){
		if (needTooltip)
			if (name.length() > 1000)
				theList->SetToolTip(name.Mid(0, 1000));
			else
				theList->SetToolTip(name);

		else if (theList->HasToolTips())
			theList->UnsetToolTip();
	}
}

void ItemText::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = dc->GetTextExtent(name);

	if (modified){ dc->SetTextForeground(Options.GetColour(WINDOW_WARNING_ELEMENTS)); }
	else{ dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WINDOW_TEXT : WINDOW_TEXT_INACTIVE)); }
	needTooltip = ex.x > width - 8;
	wxRect cur(x, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	//if (modified){ dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WINDOW_TEXT : WINDOW_TEXT_INACTIVE)); }
}

void ItemColor::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	if (col.a){
		const wxColour & col1 = Options.GetColour(STYLE_PREVIEW_COLOR1);
		const wxColour & col2 = Options.GetColour(STYLE_PREVIEW_COLOR2);
		int r2 = col.r, g2 = col.g, b2 = col.b;
		int r = col1.Red(), g = col1.Green(), b = col1.Blue();
		int r1 = col2.Red(), g1 = col2.Green(), b1 = col2.Blue();
		int inv_a = 0xFF - col.a;
		int fr = (r2* inv_a / 0xFF) + (r - inv_a * r / 0xFF);
		int fg = (g2* inv_a / 0xFF) + (g - inv_a * g / 0xFF);
		int fb = (b2* inv_a / 0xFF) + (b - inv_a * b / 0xFF);
		wxColour firstMask(fr, fg, fb);
		fr = (r2* inv_a / 0xFF) + (r1 - inv_a * r1 / 0xFF);
		fg = (g2* inv_a / 0xFF) + (g1 - inv_a * g1 / 0xFF);
		fb = (b2* inv_a / 0xFF) + (b1 - inv_a * b1 / 0xFF);
		wxColour secondMask(fr, fg, fb);
		int squareSize = (height - 3) / 4;
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				dc->SetBrush(((i + j) % 2 == 0) ? firstMask : secondMask);
				dc->SetPen(*wxTRANSPARENT_PEN);
				dc->DrawRectangle(x + 2 + (i*squareSize), y + 2 + (j*squareSize), squareSize, squareSize);
			}
		}
	}
	dc->SetBrush(wxBrush(col.a ? *wxTRANSPARENT_BRUSH : col.GetWX()));
	dc->SetPen(wxPen(Options.GetColour(WINDOW_TEXT_INACTIVE)));
	dc->DrawRectangle(x + 1, y + 1, height - 3, height - 3);

	int fw, fh;
	wxString hextext = col.GetHex(true);
	dc->GetTextExtent(hextext, &fw, &fh);

	if (modified){ dc->SetTextForeground(Options.GetColour(WINDOW_WARNING_ELEMENTS)); }
	wxRect cur(x + height + 2, y, width - (height + 8), height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(hextext, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	if (modified){ dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WINDOW_TEXT : WINDOW_TEXT_INACTIVE)); }
}

void ItemColor::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed)
{
	if (event.RightUp()){
		Menu menut;
		menut.Append(7786, _("&Kopiuj"));
		menut.Append(7787, _("&Wklej"));
		int id = menut.GetPopupMenuSelection(event.GetPosition(), theList);
		if (id == 7786){
			wxString whatcopy = col.GetHex(true);
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxTextDataObject(whatcopy));
				wxTheClipboard->Close();
				wxTheClipboard->Flush();
			}
		}
		else if (id == 7787){
			if (wxTheClipboard->Open()){
				if (wxTheClipboard->IsSupported(wxDF_TEXT)){
					wxTextDataObject data;
					wxTheClipboard->GetData(data);
					ItemColor *copy = new ItemColor(*this);
					copy->col.SetAss(data.GetText());
					theList->SetModified(true);
					copy->modified = true;
					(*changed) = copy;
				}
				wxTheClipboard->Close();
			}
		}
	}

};

void ItemColor::Save(){
	if (modified){
		Options.SetColor((COLOR)colOptNum, col);
		modified = false;
	}
}

void ItemColor::OnChangeHistory()
{
	modified = (col != Options.GetColor((COLOR)colOptNum));
}

void ItemCheckBox::OnPaint(wxMemoryDC *dc, int x, int y, int w, int h, KaiListCtrl *theList)
{
	wxString bitmapName = (modified) ? L"checkbox_selected" : L"checkbox";
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if (enter){ BlueUp(&checkboxBmp); }
	dc->DrawBitmap(checkboxBmp, x + 1, y + (h - 13) / 2);

	if (w > 18){
		int fw, fh;
		dc->GetTextExtent(name, &fw, &fh);
		wxRect cur(x + 18, y, w - 20, h);
		dc->SetClippingRegion(cur);
		dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
		dc->DestroyClippingRegion();


	}
}


void ItemCheckBox::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **copy){
	if (event.LeftDown()){
		modified = !modified;
		theList->Refresh(false);
		theList->SetModified(true);
	}
	if (!enter && _enter){
		enter = _enter; theList->Refresh(false);
	}
	if (enter && leave){
		enter = false; theList->Refresh(false);
	}
};

//custom multilist
KaiListCtrl::KaiListCtrl(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size, int style)
	:KaiScrolledWindow(parent, id, pos, size, style | wxVERTICAL | wxHORIZONTAL)
	, bmp(NULL)
	, sel(-1)
	, lastSelX(-1)
	, lastSelY(-1)
	, scPosV(0)
	, scPosH(0)
	//, lineHeight(17)
	//, headerHeight(25)
	, lastCollumn(0)
	, modified(false)
	, hasArrow(true)
	, iter(0)
	, itemList(new List())
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetForegroundColour(parent->GetForegroundColour());
	SetMinSize(size);
	SetFont(*Options.GetFont(-1));
	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_CTRL, L'Z', 11642);
	entries[1].Set(wxACCEL_CTRL, L'Y', 11643);
	entries[2].Set(wxACCEL_NORMAL, WXK_UP, ID_TUP);
	entries[3].Set(wxACCEL_NORMAL, WXK_DOWN, ID_TDOWN);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KaiListCtrl::Undo, this, 11642);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KaiListCtrl::Redo, this, 11643);
	int fw, fh;
	GetTextExtent(L"TEX{}", &fw, &fh);
	lineHeight = fh + 3;
	headerHeight = fh + 8;

	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& evt) {
		SetSelection(GetSelection() - 1);
		ScrollTo(scPosV - 1);
		}, ID_TUP);

	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent& evt) {
		SetSelection(GetSelection() + 1);
		ScrollTo(scPosV + 1);
		}, ID_TDOWN);
}

//checkboxList
KaiListCtrl::KaiListCtrl(wxWindow *parent, int id, int numelem, wxString *list, const wxPoint &pos,
	const wxSize &size, int style)
	:KaiScrolledWindow(parent, id, pos, size, style | wxVERTICAL)
	, bmp(NULL)
	, sel(-1)
	, lastSelX(-1)
	, lastSelY(-1)
	, scPosV(0)
	, scPosH(0)
	//, lineHeight(17)
	, headerHeight(3)
	, lastCollumn(0)
	, modified(false)
	, hasArrow(true)
	, iter(0)
	, itemList(new List())
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetForegroundColour(parent->GetForegroundColour());
	SetMinSize(size);
	SetFont(*Options.GetFont(-1));//wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, L"Tahoma", wxFONTENCODING_DEFAULT));
	InsertColumn(0, L"", TYPE_CHECKBOX, -1);
	int maxwidth = -1;
	for (int i = 0; i < numelem; i++){
		AppendItem(new ItemCheckBox(false, list[i]));
		wxSize textSize = GetTextExtent(list[i]);
		if (textSize.x > maxwidth){ maxwidth = textSize.x; }
		//if (textSize.y + 2 > lineHeight){ lineHeight = textSize.y + 2; }
	}
	if (numelem)
		widths[0] = maxwidth + 28;
	int fw, fh;
	GetTextExtent(L"TEX{}", &fw, &fh);
	lineHeight = fh + 3;
}

//textList
KaiListCtrl::KaiListCtrl(wxWindow *parent, int id, const wxArrayString &list, const wxPoint &pos,
	const wxSize &size, int style)
	:KaiScrolledWindow(parent, id, pos, size, style | wxVERTICAL)
	, bmp(NULL)
	, sel(-1)
	, lastSelX(-1)
	, lastSelY(-1)
	, scPosV(0)
	, scPosH(0)
	//, lineHeight(17)
	, headerHeight(3)
	, lastCollumn(0)
	, modified(false)
	, hasArrow(true)
	, iter(0)
	, itemList(new List())
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetForegroundColour(parent->GetForegroundColour());
	SetMinSize(size);
	SetFont(*Options.GetFont(-1));
	InsertColumn(0, L"", TYPE_TEXT, -1);
	int maxwidth = -1;
	for (size_t i = 0; i < list.size(); i++){
		AppendItem(new ItemText(list[i]));
		wxSize textSize = GetTextExtent(list[i]);
		if (textSize.x > maxwidth){ maxwidth = textSize.x; }
		//if (textSize.y + 2 > lineHeight){ lineHeight = textSize.y + 2; }
	}
	if (list.size())
		widths[0] = maxwidth + 10;
	int fw, fh;
	GetTextExtent(L"TEX{}", &fw, &fh);
	lineHeight = fh + 3;
}

void KaiListCtrl::SetTextArray(const wxArrayString &Array)
{
	delete itemList;
	filteredList.clear();
	itemList = new List();
	int maxwidth = -1;
	for (size_t i = 0; i < Array.size(); i++){
		AppendItem(new ItemText(Array[i]));
		wxSize textSize = GetTextExtent(Array[i]);
		if (textSize.x > maxwidth){ maxwidth = textSize.x; }
	}
	widths[0] = maxwidth + 10;
	sel = -1;
	Refresh(false);
}

void KaiListCtrl::FilterRow(int rowkey, int visibility)
{
	if (rowkey < 0 || rowkey >= itemList->size())
		return;

	(*itemList)[rowkey]->isVisible = visibility;
}

void KaiListCtrl::FinalizeFiltering()
{
	RebuildFiltered();
	isFiltered = (filteredList.size() != itemList->size());
	if (!isFiltered){
		for (size_t i = 0; i < itemList->size(); i++){
			if ((*itemList)[i]->isVisible != VISIBLE){
				isFiltered = true;
				return;
			}

		}
	}
	//Refresh(false);
}

void KaiListCtrl::DeleteItem(int row, bool save)
{
	if (row < 0 || row >= itemList->size())
		return;

	ItemRow *ir = (*itemList)[row];
	itemList->Erase(row);
	if (save)
		PushHistory();
}

void KaiListCtrl::SetHeaderHeight(int height)
{
	headerHeight = height;
}

bool KaiListCtrl::SetFont(const wxFont& font)
{
	int fw, fh;
	GetTextExtent(L"TEX{}", &fw, &fh);
	lineHeight = fh + 7;
	if (headerHeight > 15)
		headerHeight = lineHeight + 6;

	bool result = wxWindow::SetFont(font);
	return result;
}

int KaiListCtrl::InsertColumn(size_t col, const wxString &name, byte type, int width)
{
	if (col >= widths.size()){
		header.Insert(col, new ItemText(name));
		widths.push_back(width);
		return col;
	}
	header.Insert(col, new ItemText(name));
	widths.Insert(col, width);
	return col;
}

int KaiListCtrl::AppendItem(Item *item)
{
	ItemRow *newitem = new ItemRow(0, item);
	itemList->push_back(newitem);
	if (newitem->isVisible != NOT_VISIBLE)
		filteredList.push_back(newitem);

	return itemList->size() - 1;
}


int KaiListCtrl::AppendItemWithExtent(Item *item)
{
	ItemRow *newitem = new ItemRow(0, item);
	itemList->push_back(newitem);
	if (newitem->isVisible != NOT_VISIBLE)
		filteredList.push_back(newitem);

	wxSize textSize = item->GetTextExtents(this);
	if (!widths.size())
		widths.push_back(textSize.x);
	else if (textSize.x > widths[0]){ widths[0] = textSize.x; }
	//if (textSize.y + 2 > lineHeight){ lineHeight = textSize.y + 2; }
	return itemList->size() - 1;
}

int KaiListCtrl::SetItem(size_t row, size_t col, Item *item)
{
	if (col >= (*itemList)[row]->row.size()){
		(*itemList)[row]->row.push_back(item);
		return row;
	}
	(*itemList)[row]->row.insert((*itemList)[row]->row.begin() + col, item);
	return row;
}

Item *KaiListCtrl::GetItem(size_t row, size_t col)
{
	if (row < 0 || row >= itemList->size() || col < 0 || col >= (*itemList)[row]->row.size()){ return NULL; }
	return (*itemList)[row]->row[col];
}


void KaiListCtrl::OnSize(wxSizeEvent& evt)
{
	/*if(headerHeight<5 && widths.size()==1){
		widths[0] = -1;
		}*/
	Refresh(false);
}

void KaiListCtrl::OnPaint(wxPaintEvent& evt)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	
	int maxWidth = GetMaxWidth();
	if (isFiltered)
		maxWidth += 12;

	bool shouldStretch = false;
	int stretched = maxWidth;

	if (widths.size() > 1){ maxWidth += 10; }
	else if (maxWidth < w - 1){ 
		maxWidth = w - 1; 
		//With one collumn it should stretch it when is smaller than window
		shouldStretch = (widths.size() == 1);
		stretched = (isFiltered) ? w - 16 : w - 4;
	}
	
	//if (widths.size() == 1){ widths[0] = (isFiltered) ? w - 16 : w - 4; }
	

	if (SetScrollBar(wxHORIZONTAL, scPosH, w, maxWidth, w - 2)){
		GetClientSize(&w, &h);
		if (maxWidth <= w){ scPosH = 0; SetScrollPos(wxHORIZONTAL, 0); }
	}

	int visibleSize = filteredList.size();//GetVisibleSize();
	size_t maxVisible = ((h - headerHeight) / lineHeight) + 1;
	size_t itemsize = visibleSize + 1;
	if ((size_t)scPosV >= itemsize - maxVisible){
		scPosV = itemsize - maxVisible;
	}
	if (scPosV < 0){ scPosV = 0; }
	size_t maxsize = itemsize - 1;
	if (itemsize > maxVisible){
		maxsize = MIN(maxVisible + scPosV, itemsize - 1);
		if (SetScrollBar(wxVERTICAL, scPosV, maxVisible, itemsize, maxVisible - 2)){
			GetClientSize(&w, &h);
		}
	}
	else{
		scPosV = 0;
		if (SetScrollBar(wxVERTICAL, scPosV, maxVisible, itemsize, maxVisible - 2)){
			GetClientSize(&w, &h);
		}
	}

	//if (isFiltered)
		//w -= 12;

	int bitmapw = w;
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < bitmapw || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(bitmapw, h); }

	tdc.SelectObject(*bmp);
	wxMemoryDC fdc;
	bool enabled = IsThisEnabled();
	const wxColour & highlight = Options.GetColour(STATICLIST_SELECTION);
	const wxColour & txt = Options.GetColour(WINDOW_TEXT);
	const wxColour & inactivetxt = Options.GetColour(WINDOW_TEXT_INACTIVE);
	const wxColour & border = Options.GetColour(STATICLIST_BORDER);

	tdc.SetPen(wxPen(border));
	tdc.SetBrush(wxBrush(enabled ? Options.GetColour(STATICLIST_BACKGROUND) : Options.GetColour(WINDOW_BACKGROUND_INACTIVE)));
	tdc.DrawRectangle(0, 0, w, h);
	tdc.SetTextForeground(enabled ? txt : inactivetxt);
	tdc.SetFont(GetFont());
	if (isFiltered){
		fdc.SelectObject(wxBitmap(13, h));
		fdc.SetBrush(wxBrush(enabled ? Options.GetColour(STATICLIST_BACKGROUND) : Options.GetColour(WINDOW_BACKGROUND_INACTIVE)));
		fdc.SetPen(wxPen(border));
		fdc.DrawRectangle(0, 0, 13, h);
	}

	//header
	int filteringHeader = 5;
	int posX = filteringHeader - scPosH;
	int posY = headerHeight;
	bool startBlock = false;
	int startDrawPosYFromPlus = 0;
	//Draw header
	if (headerHeight > 4){
		tdc.SetPen(wxPen(border));
		tdc.SetTextForeground(enabled ? Options.GetColour(STATICLIST_TEXT_HEADLINE) : inactivetxt);
		tdc.SetBrush(Options.GetColour(STATICLIST_BACKGROUND_HEADLINE));
		tdc.DrawRectangle(0, 0, w, headerHeight - 2);
		for (size_t j = 0; j < widths.size(); j++){
			wxString headerTxt = ((ItemText*)header.row[j])->GetName();
			wxSize ex = tdc.GetTextExtent(headerTxt);
			wxRect rect(posX, 0, widths[j] - 3, headerHeight);
			tdc.SetClippingRegion(rect);
			tdc.DrawText(headerTxt, posX, ((headerHeight - ex.y) / 2));
			tdc.DestroyClippingRegion();
			posX += widths[j];
			tdc.DrawLine(posX - 3, 0, posX - 3, h);
		}
		//tdc.DrawLine(0, headerHeight-2, w, headerHeight-2);
	}
	//Draw filter lines and boxes
	posX = filteringHeader - scPosH;
	if (isFiltered){
		unsigned char hasHiddenBlock = CheckIfHasHiddenBlock(scPosV-1);
		if (hasHiddenBlock){
			fdc.SetBrush(*wxTRANSPARENT_BRUSH);
			fdc.SetPen(txt);
			tdc.SetBrush(*wxTRANSPARENT_BRUSH);
			tdc.SetPen(txt);
			int halfLineHeight = (lineHeight / 2);
			int newPosY = posY + 1;
			int startDrawPosY = newPosY + ((lineHeight - 10) / 2) - halfLineHeight;
			fdc.DrawRectangle(2, startDrawPosY, 9, 9);
			fdc.DrawLine(4, newPosY - 1, 9, newPosY - 1);
			if (hasHiddenBlock == 1){
				fdc.DrawLine(6, startDrawPosY + 2, 6, startDrawPosY + 7);
			}
			fdc.DrawLine(10, newPosY - 1, 13, newPosY - 1);
			tdc.DrawLine(0, newPosY - 1, w + scPosH, newPosY - 1);
		}
	}
	//draw lines
	for (size_t i = scPosV; i < maxsize; i++){;
		
		auto &row = filteredList[i]->row;
		
		for (size_t j = 0; j < widths.size(); j++){
			int rowsize = row.size();
			if (j >= rowsize){
				continue;
			}
			int colSize = (shouldStretch) ? stretched : widths[j];
			//drawing
			if (i == sel){
				tdc.SetPen(wxPen(highlight));
				tdc.SetBrush(wxBrush(highlight));
				tdc.DrawRectangle(posX - 5, posY, colSize + 3, lineHeight);
			}
			row[j]->OnPaint(&tdc, posX, posY, colSize, lineHeight, this);
			posX += colSize;

		}

		if (isFiltered){
			unsigned char hasHiddenBlock = CheckIfHasHiddenBlock(i);
			if (hasHiddenBlock){
				fdc.SetBrush(*wxTRANSPARENT_BRUSH);
				fdc.SetPen(txt);
				tdc.SetBrush(*wxTRANSPARENT_BRUSH);
				tdc.SetPen(txt);
				int halfLineHeight = (lineHeight / 2);
				int newPosY = posY + lineHeight + 1;
				int startDrawPosY = newPosY + ((lineHeight - 10) / 2) - halfLineHeight;
				fdc.DrawRectangle(2, startDrawPosY, 9, 9);
				fdc.DrawLine(4, newPosY - 1, 9, newPosY - 1);
				if (hasHiddenBlock == 1){
					fdc.DrawLine(6, startDrawPosY + 2, 6, startDrawPosY + 7);
				}
				fdc.DrawLine(10, newPosY - 1, 13, newPosY - 1);
				tdc.DrawLine(0, newPosY - 1, w + scPosH, newPosY - 1);
			}
			if (!startBlock && filteredList[i]->isVisible == VISIBLE_BLOCK){
				startDrawPosYFromPlus = posY + 4; startBlock = true;
			}
			bool isLastLine = (i >= maxsize - 1);
			bool notVisibleBlock = filteredList[i]->isVisible != VISIBLE_BLOCK;
			if (startBlock && (notVisibleBlock || isLastLine)){
				tdc.SetBrush(*wxTRANSPARENT_BRUSH);
				tdc.SetPen(txt);
				fdc.SetBrush(*wxTRANSPARENT_BRUSH);
				fdc.SetPen(txt);
				int halfLine = posY - 1;
				if (isLastLine && !notVisibleBlock){ halfLine = posY + lineHeight; }
				fdc.DrawLine(6, startDrawPosYFromPlus, 6, halfLine);
				fdc.DrawLine(6, halfLine, 13, halfLine);
				tdc.DrawLine(0, halfLine, w + scPosH, halfLine);
				startBlock = false;
			}
		}
		posY += lineHeight;
		posX = filteringHeader - scPosH;
		//visibleI++;
	}
	
	tdc.SetPen(wxPen(border));
	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	tdc.DrawRectangle(0, 0, (isFiltered) ? w - 12 : w, h);

	wxPaintDC dc(this);
	if (isFiltered){
		dc.Blit(0, 0, 13, h, &fdc, 0, 0);
	}

	dc.Blit((isFiltered) ? 13 : 0, 0, w, h, &tdc, (isFiltered) ? 1 : 0, 0);
}

void KaiListCtrl::OnMouseEvent(wxMouseEvent &evt)
{
	if (evt.LeftUp() && HasCapture()){
		ReleaseMouse();
	}
	if (evt.ButtonDown()){
		SetFocus();
	}
	if (evt.GetWheelRotation() != 0) {
		int w = 0;
		int h = 0;
		GetClientSize(&w, &h);
		int step = evt.GetWheelRotation() / evt.GetWheelDelta();
		scPosV -= step;
		size_t maxVisible = ((h - headerHeight) / lineHeight) + 1;
		size_t visibleSize = GetVisibleSize();
		if (scPosV<0){ scPosV = 0; }
		else if ((size_t)scPosV > visibleSize + 1 - maxVisible){ scPosV = visibleSize + 1 - maxVisible; }
		Refresh(false);
		return;
	}
	wxPoint cursor = evt.GetPosition();
	if (isFiltered && cursor.x < 18 && cursor.y > headerHeight - 7){
		if (evt.LeftDown()){
			int elemYID = ((cursor.y - headerHeight - (lineHeight / 2)) / lineHeight) + scPosV;
			if (cursor.y < headerHeight + 7)
				elemYID = scPosV - 1;

			int mode = CheckIfHasHiddenBlock(elemYID);
			if (mode){
				size_t startI = 0;
				int elemY = FindKey(elemYID);
				ShowOrHideBlock(elemY);
				return;
			}
		}
		return;
	}
	Item* copy = NULL;
	int elemYID = ((cursor.y - headerHeight) / lineHeight) + scPosV;
	//size_t startI = 0;
	int elemY = elemYID;//FindItemsRow(elemYID, startI);
	if ((elemY < 0) || cursor.y <= headerHeight){
		//if header < 5 wtedy nic nie robimy
		if (headerHeight > 5){
			if (!hasArrow && evt.LeftDown()){
				if (!HasCapture()){ CaptureMouse(); }
				diffX = cursor.x;
				return;
			}
			else if (!hasArrow && evt.LeftIsDown()){
				widths[lastCollumn] += (cursor.x - diffX);
				if (widths[lastCollumn] < 20){ widths[lastCollumn] = 20; }
				diffX = cursor.x;
				Refresh(false);
				return;
			}
			int maxwidth = (isFiltered) ? 13 - scPosH : 1 - scPosH;
			//bool isonpos= false;
			for (size_t i = 0; i < widths.size(); i++){
				maxwidth += widths[i];
				if (abs(maxwidth - cursor.x) < 5){
					if (hasArrow){
						SetCursor(wxCURSOR_SIZEWE);
						hasArrow = false;
						lastCollumn = i;
					}
					break;
				}
			}

		}
		if (HasToolTips())
			UnsetToolTip();
		if (lastSelX != -1 && lastSelY != -1 && lastSelY < filteredList.size() && lastSelX < filteredList[lastSelY]->row.size()) {
			filteredList[lastSelY]->row[lastSelX]->OnMouseEvent(evt, false, true, this, &copy);
			lastSelX = -1; lastSelY = -1;
		}
		if (copy) { delete copy; }
		//lastSelX = -1; lastSelY = -1;
		if (!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow = true; }
		return;
	}
	if (!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow = true; }
	
	if (elemY < 0 || elemY >= filteredList.size()){
		//if (HasToolTips())
			//UnsetToolTip();
		//Nothing to do here below elements on the bottom
		
		return;
	}
	int elemX = -1;
	int startX = (isFiltered) ? 17 - scPosH : 5 - scPosH;
	for (size_t i = 0; i < widths.size(); i++){
		if (cursor.x > startX && cursor.x <= startX + widths[i] && i < filteredList[elemY]->row.size()){
			elemX = i;
			bool enter = (elemX != lastSelX || elemY != lastSelY) || evt.Entering();
			if (isFiltered)
				evt.SetX(evt.GetX() - 12);
			filteredList[elemY]->row[elemX]->OnMouseEvent(evt, enter, false, this, &copy);
			if (copy != NULL){
				ItemRow *newRow = new ItemRow();
				for (size_t g = 0; g < filteredList[elemY]->row.size(); g++){
					if (g == elemX){
						newRow->row.push_back(copy);
						continue;
					}
					newRow->row.push_back(filteredList[elemY]->row[g]->Copy());
					newRow->row[g]->modified = true;
				}
				int elemY = FindKey(elemYID);
				if (elemY >= 0){
					itemList->Change(elemY, newRow);
					filteredList[elemYID] = newRow;
					PushHistory();
					Refresh(false);
				}
				copy = NULL;
			}
			break;
		}
		startX += widths[i];
	}
	if ((elemX != lastSelX || elemY != lastSelY || evt.Leaving())
		&& lastSelX != -1 && lastSelY != -1 && lastSelY < filteredList.size()
		&& lastSelX < filteredList[lastSelY]->row.size()){
		filteredList[lastSelY]->row[lastSelX]->OnMouseEvent(evt, false, true, this, &copy);
		if (copy){ delete copy; copy = NULL; }
	}

	if (evt.LeftDown()){
		sel = elemYID;
		Refresh(false);
		wxCommandEvent evt2(LIST_ITEM_LEFT_CLICK, GetId());
		evt2.SetInt(elemY);
		AddPendingEvent(evt2);
	}
	else if (evt.LeftDClick()){
		wxCommandEvent evt2(LIST_ITEM_DOUBLECLICKED, GetId());
		evt2.SetInt(elemY);
		AddPendingEvent(evt2);
	}
	else if (evt.RightDown()){
		wxCommandEvent evt2(LIST_ITEM_RIGHT_CLICK, GetId());
		evt2.SetInt(elemY);
		AddPendingEvent(evt2);
	}
	lastSelY = elemY;
	lastSelX = elemX;
}

void KaiListCtrl::OnScroll(wxScrollWinEvent& event)
{
	size_t newPos = 0;
	int orient = event.GetOrientation();
	size_t scPos = (orient == wxVERTICAL) ? scPosV : scPosH;
	newPos = event.GetPosition();
	if (scPos != newPos) {
		if (orient == wxVERTICAL){
			scPosV = newPos;
		}
		else{
			scPosH = newPos;
		}
		Refresh(false);
	}
}

int KaiListCtrl::GetMaxWidth()
{

	int maxWidth = 0;
	for (size_t i = 0; i < widths.size(); i++){
		if (widths[i] == -1){ SetWidth(i); }
		maxWidth += widths[i];
	}
	return maxWidth;
}

void KaiListCtrl::SetWidth(size_t j)
{
	int maxwidth = -1;
	for (int i = 0; i < itemList->size(); i++){
		Item * item = (*itemList)[i]->Get(j);
		if (!item) continue;

		wxSize textSize = item->GetTextExtents(this);
		if (textSize.x > maxwidth){ maxwidth = textSize.x; }
	}
	widths[j] = maxwidth + 28;
}

//collumn must be set
void KaiListCtrl::FilterList(int column, int mode)
{
	if (column < 0)
		return;

	scPosV = 0;
	sel = 0;
	isFiltered = false;

	for (size_t i = 0; i < itemList->size(); i++){
		ItemRow * keyRow = (*itemList)[i];
		if (keyRow->row.size() <= (size_t)column){ continue; }
		else{
			int visibility = keyRow->row[column]->OnVisibilityChange(mode);
			if (visibility != VISIBLE){
				isFiltered = true;
			}
			keyRow->isVisible = visibility;
		}
	}
	RebuildFiltered();
	Refresh(false);
}

void KaiListCtrl::FilterItem(int row, byte type, bool showHidden)
{
	if (row < 0 || row >= itemList->size())
		return;

	ItemRow* keyRow = (*itemList)[row];
	keyRow->isVisible = type;
	if (showHidden && type != VISIBLE) {
		isFiltered = true;
	}
}

void KaiListCtrl::FilterFinalize()
{
	RebuildFiltered();
	Refresh(false);
}

int KaiListCtrl::GetType(int row, int column)
{
	if (row < 0 || row >= itemList->size())
		return -1;

	ItemRow *il = (*itemList)[row];
	if (il->row.size() > column){
		return il->row[column]->type;
	}

	return -1;
}

//when startI > 0 pass elemX decreased of elems before startI
//startI increases automatically
//int KaiListCtrl::FindItemsRow(int elemX, size_t &startI /*= 0*/)
//{
//	if (elemX < 0)
//		return elemX;
//
//	int visibleElemX = 0;
//	for (size_t i = startI; i < itemList->size(); i++){
//		ItemRow * irow = (*itemList)[i];
//		if (irow->isVisible != NOT_VISIBLE){
//			if (visibleElemX == elemX){
//				startI = i + 1;
//				return i;
//			}
//			visibleElemX++;
//		}
//	}
//	startI = itemList->size();
//	return -1;
//}

int KaiListCtrl::CheckIfHasHiddenBlock(int elemX, size_t startI /*= 0*/)
{
	//size_t newStartI = startI;
	// now we need only one lines as elemX
	int lineActual = FindKey(elemX);
	int linePlusOne = FindKey(elemX + 1);
	int linePlusOneSafe = (linePlusOne < 0) ? itemList->size() : linePlusOne;

	if (lineActual + 1 < linePlusOneSafe)
		return 1;

	if (linePlusOne < 0)
		return 0;

	if ((lineActual < 0 || (*itemList)[lineActual]->isVisible != VISIBLE_BLOCK) &&
		(*itemList)[linePlusOne]->isVisible == VISIBLE_BLOCK)
		return 2;

	return 0;
}

void KaiListCtrl::ShowOrHideBlock(int elemRealX)
{
	for (size_t i = elemRealX + 1; i < itemList->size(); i++){
		ItemRow * irow = (*itemList)[i];
		if (irow->isVisible == NOT_VISIBLE)
			irow->isVisible = VISIBLE_BLOCK;
		else if (irow->isVisible == VISIBLE_BLOCK)
			irow->isVisible = NOT_VISIBLE;
		else
			break;
	}
	RebuildFiltered();
	Refresh(false);
}

size_t KaiListCtrl::GetVisibleSize()
{
	/*int visibleElemX = 0;
	for (size_t i = 0; i < itemList->size(); i++){
		ItemRow * irow = (*itemList)[i];
		if (irow->isVisible != NOT_VISIBLE){
			visibleElemX++;
		}
	}*/
	return filteredList.size();//visibleElemX;
}

void KaiListCtrl::SaveAll(int col)
{
	if (!modified){ return; }
	for (size_t i = 0; i < itemList->size(); i++){
		for (size_t j = 0; j < (*itemList)[i]->row.size(); j++){
			if (j == col)
				(*itemList)[i]->row[j]->Save();
			else
				(*itemList)[i]->row[j]->modified = false;
		}
	}
	modified = false;
}

int KaiListCtrl::FindItem(int column, const wxString &textItem, int row /*= 0*/)
{
	for (size_t i = row; i < itemList->size(); i++){
		if (column < 0){
			for (size_t j = 0; j < (*itemList)[i]->row.size(); j++){
				if ((*itemList)[i]->row[j]->name == textItem){
					return i;
				}
			}
		}
		else if ((*itemList)[i]->row.size() <= (size_t)column){ continue; }
		else if ((*itemList)[i]->row[column]->name == textItem){
			return i;
		}
	}
	return -1;
}

int KaiListCtrl::FindItem(int column, Item *item, int row /*= 0*/)
{
	for (size_t i = row; i < itemList->size(); i++){
		if (column < 0){
			for (size_t j = 0; j < (*itemList)[i]->row.size(); j++){
				if ((*itemList)[i]->row[j] == item){
					return i;
				}
			}
		}
		else if ((*itemList)[i]->row.size() <= (size_t)column){ continue; }
		else if ((*itemList)[i]->row[column] == item){
			return i;
		}
	}
	return -1;
}

void KaiListCtrl::ScrollTo(int row){
	scPosV = row;
	SetScrollpos(wxVERTICAL, row);
	Refresh(false);
}

void KaiListCtrl::PushHistory()
{
	if (iter != historyList.size()){
		for (auto it = historyList.begin() + iter + 1; it != historyList.end(); it++)
		{
			delete (*it);
		}
		historyList.erase(historyList.begin() + iter + 1, historyList.end());
	}
	historyList.push_back(itemList);
	itemList = itemList->Copy();
	iter++;
}

void KaiListCtrl::StartEdition()
{
	historyList.push_back(itemList);
	itemList = itemList->Copy();
}

void KaiListCtrl::Undo(wxCommandEvent &evt)
{
	if (iter > 0){
		iter--;
		List * actual = historyList[iter]->Copy();
		for (size_t i = 0; i < itemList->size(); i++){
			if (i < actual->size() && (*itemList)[i] != (*actual)[i]){
				for (size_t j = 0; j < (*actual)[i]->row.size(); j++)
					(*actual)[i]->row[j]->OnChangeHistory();

				modified = true;
			}
		}
		delete itemList;
		itemList = actual;
		RebuildFiltered();
		Refresh(false);
	}
}

void KaiListCtrl::Redo(wxCommandEvent &evt)
{
	if (iter < (int)historyList.size() - 1){
		iter++;
		List * actual = historyList[iter]->Copy();
		for (size_t i = 0; i < itemList->size(); i++){
			if (i < actual->size() && (*itemList)[i] != (*actual)[i]){
				for (size_t j = 0; j < (*actual)[i]->row.size(); j++)
					(*actual)[i]->row[j]->OnChangeHistory();

				modified = true;
			}
		}
		delete itemList;
		itemList = actual;
		RebuildFiltered();
		Refresh(false);
	}
}

Item *KaiListCtrl::CopyRow(int y, int x, bool pushBack)
{
	ItemRow *newRow = new ItemRow();
	for (size_t g = 0; g < (*itemList)[y]->row.size(); g++){
		newRow->row.push_back((*itemList)[y]->row[g]->Copy());
	}
	if (pushBack){
		itemList->push_back(newRow);
		filteredList.push_back(newRow);
		int newy = itemList->size() - 1;
		if (x < 0 || x >= (int)(*itemList)[newy]->row.size()){ return NULL; }
		return (*itemList)[newy]->row[x];
	}
	else{
		size_t id = FindId(y);
		itemList->Change(y, newRow);
		if (id != -1)
			filteredList[id] = newRow;
	}
	if (x < 0 || x >= (int)(*itemList)[y]->row.size()){ return NULL; }
	return (*itemList)[y]->row[x];
}

//set selection as key everything form list is get as key
void KaiListCtrl::SetSelection(int selection, bool center, bool isId)
{
	if (isId) {
		sel = selection;
	}
	else {
		sel = FindId(selection);
	}
	if (center){
		int w = 0;
		int h = 0;
		GetClientSize(&w, &h);
		int visibleElems = ((h - headerHeight) / lineHeight);
		int elemY = sel - (visibleElems / 2);
		scPosV = MID(0, elemY, itemList->size() - visibleElems + 2);
	}
	Refresh(false);
}

//GetSelection as key rather not possible to get invisible element
int KaiListCtrl::GetSelection(bool getId)
{
	if (getId)
		return sel;

	//check why this is key
	//getItem no longer uses keys return only id
	return FindKey(sel);
}

//rebuild filtered table after undo redo or som mass actions
void KaiListCtrl::RebuildFiltered(){
	filteredList.clear();
	for (size_t i = 0; i < itemList->size(); i++){
		ItemRow * itemrow = (*itemList)[i];
		if (itemrow->isVisible > NOT_VISIBLE)
			filteredList.push_back(itemrow);
	}

}

size_t KaiListCtrl::FindKey(size_t id){
	if (id >= filteredList.size())
		return -1;

	//size_t diff = 0;
	//size_t fsize = filteredList.size() - 1;
	/*if (id > fsize){
		diff = id - fsize;
		id = fsize;
	}*/

	ItemRow * row = filteredList[id];
	for (size_t i = id; i < itemList->size(); i++){
		if (row == (*itemList)[i]){
			/*size_t key = i;
			if (diff){
				key += diff;
				if (i < itemList->size())
					return key;
				else
					return -1;
			}*/

			return i;

		}
	}
	
	// it should not happen but with bugs it's possible
	return -1;
}

size_t KaiListCtrl::FindId(size_t key)
{
	if (key >= itemList->size() || key < 0)
		return -1;

	ItemRow * row = (*itemList)[key];
	if (row->isVisible == NOT_VISIBLE)
		return -1;

	size_t i = (key < filteredList.size()) ? key : filteredList.size() - 1;
	while(i + 1 > 0){
		if (row == filteredList[i])
			return i;

		i--;
	}
	return -1;
}


BEGIN_EVENT_TABLE(KaiListCtrl, KaiScrolledWindow)
EVT_PAINT(KaiListCtrl::OnPaint)
EVT_SIZE(KaiListCtrl::OnSize)
EVT_SCROLLWIN(KaiListCtrl::OnScroll)
EVT_MOUSE_EVENTS(KaiListCtrl::OnMouseEvent)
EVT_ERASE_BACKGROUND(KaiListCtrl::OnEraseBackground)
END_EVENT_TABLE()

wxIMPLEMENT_ABSTRACT_CLASS(KaiListCtrl, wxWindow);