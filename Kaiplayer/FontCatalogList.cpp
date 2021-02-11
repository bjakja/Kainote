//  Copyright (c) 2018-2020, Marcin Drob

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

#include "FontCatalogList.h"
#include "config.h"
#include "KaiCheckBox.h"
#include "FontEnumerator.h"
#include "OpennWrite.h"
#include "StylePreview.h"
#include "Menu.h"
#include <wx/tokenzr.h>

wxDEFINE_EVENT(CATALOG_CHANGED, wxCommandEvent);

wxArrayString* CatalogList::catalogList = NULL;
PopupList* CatalogList::floatingList = NULL;

FontCatalogList::FontCatalogList(wxWindow *parent)
	: KaiDialog(parent, -1, _("Wyniki szukania"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	DialogSizer * main = new DialogSizer(wxVERTICAL);
	fontList = new KaiListCtrl(this, ID_FONT_LIST, wxDefaultPosition, wxSize(700, 300));
	fontList->InsertColumn(0, _("Nazwa czcionki"), TYPE_TEXT, 200);
	fontList->InsertColumn(1, _("Przykład"), TYPE_TEXT, 250);
	fontList->InsertColumn(2, _("Katalog"), TYPE_LIST, 100);
	GenerateList();
	wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

	MappedButton *addCatalog = new MappedButton(this, ID_ADD_CATALOG, _("Dodaj katalog"));
	MappedButton *removeCatalog = new MappedButton(this, ID_REMOVE_CATALOG, _("Usuń katalog"));
	//replaceChecked = new MappedButton(this, ID_REPLACE_CHECKED, _("Zamień"));
	wxArrayString* catalogList = FCManagement.GetCatalogNames();
	catalog = new KaiChoice(this, -1, L"", wxDefaultPosition, wxDefaultSize, *catalogList);
	
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		wxString ctlg = catalog->GetValue();
		if (!ctlg.empty()) {
			FCManagement.AddCatalog(ctlg);
			catalog->Append(ctlg);
			CatalogList::RefreshCatalogList();
		}
	}, ID_ADD_CATALOG);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		wxString ctlg = catalog->GetValue();
		int ctlgIndex = catalog->FindString(ctlg);
		if (!ctlg.empty() && ctlgIndex != -1) {
			FCManagement.RemoveCatalog(ctlg);
			catalog->Delete(ctlgIndex);
			CatalogList::RefreshCatalogList();
		}
	}, ID_REMOVE_CATALOG);

	Bind(LIST_ITEM_LEFT_CLICK, [=](wxCommandEvent& evt) {
		RefreshPreview();
	}, ID_FONT_LIST);
	preview = new StylePreview(this, -1, wxDefaultPosition, wxSize(-1, 250));

	buttonsSizer->Add(catalog, 3, wxALL | wxEXPAND, 2);
	buttonsSizer->Add(addCatalog, 1, wxALL, 2);
	buttonsSizer->Add(removeCatalog, 1, wxALL, 2);
	//buttonsSizer->Add(replaceChecked, 1, wxALL, 2);
	
	main->Add(buttonsSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(fontList, 1, wxEXPAND | wxALL, 2);
	main->Add(preview, 0, wxEXPAND | wxALL, 2);

	SetSizerAndFit(main);
	fontStyle = Styles(L"FontPreview,Arial,80,&H00FFFFFF,&HFF0000FF,&H00301946,&H7A301946,-1,0,0,0,100,100,1.13208,0,1,3.5,0,2,120,120,40,1");
}

FontCatalogList::~FontCatalogList()
{
}

void FontCatalogList::ClearList()
{
	fontList->ClearList();
	//replaceChecked->Enable();
}

void FontCatalogList::GenerateList()
{
	wxArrayString * fonts = FontEnum.GetFonts(NULL, [=](){});
	for (auto& font : (*fonts)) {
		int row = fontList->AppendItem(new FontItem(font));
		fontList->SetItem(row, 1, new FontSample(font));
		wxString catalog = FCManagement.FindCatalogByFont(font);
		fontList->SetItem(row, 2, new CatalogList(catalog));
	}
	fontList->SetSelection(0);
}

void FontCatalogList::RefreshPreview()
{
	int sel = fontList->GetSelection();
	if (sel != -1) {
		Item *item = fontList->GetItem(sel, 0);
		if (item) {
			fontStyle.Fontname = item->name;
			preview->DrawPreview(&fontStyle);
		}
	}
}

void CatalogList::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	if (_enter) {
		enter = true;
		theList->Refresh(false);
	}
	if (leave) {
		enter = false;
		theList->Refresh(false);
	}

	bool click = event.LeftDown() || event.LeftDClick();
	if (click) {
		
		int choice = catalogList->Index(name);

		Menu menuList;
		menuList.Append(2999, L"");
		int i = 0;
		for (auto& catalog : *catalogList) {
			menuList.Append(3000 + i, catalog);
			i++;
		}
		menuList.SelectOnStart(choice);
		int result = menuList.GetPopupMenuSelection(wxPoint(lastX, lastY), theList);
		if (result >= 3000 && catalogList->size() > result - 3000) {
			name = (*catalogList)[result - 3000];
			int selection = theList->GetSelection();
			Item* thisItem = theList->GetItem(selection, 0);
			if (!thisItem)
				return;

			thisItem->modified = false;
			FCManagement.AddCatalogFont(name, thisItem->name);
			for (size_t j = 0; j < theList->GetCount(); j++) {
				Item* item = theList->GetItem(j, 0);
				if (item && item->modified) {
					FCManagement.AddCatalogFont(name, item->name);
				}
			}
		}
		else if (result = 2999) {
			int selection = theList->GetSelection();
			Item* thisItem = theList->GetItem(selection, 0);
			if (!thisItem)
				return;

			thisItem->modified = false;
			FCManagement.RemoveCatalogFont(name, thisItem->name);
			for (size_t j = 0; j < theList->GetCount(); j++) {
				Item* item = theList->GetItem(j, 0);
				if (item && item->modified) {
					FCManagement.RemoveCatalogFont(name, item->name);
					item->modified = false;
				}
			}
			name = L"";
		}
	}
}

void CatalogList::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	dc->SetBrush(wxBrush((enter && !clicked) ? Options.GetColour(BUTTON_BACKGROUND_HOVER) :
		(clicked) ? Options.GetColour(BUTTON_BACKGROUND_PUSHED) : BUTTON_BACKGROUND));
	dc->SetPen(wxPen((enter && !clicked) ? Options.GetColour(BUTTON_BORDER_HOVER) :
		(clicked) ? Options.GetColour(BUTTON_BORDER_PUSHED) :
		Options.GetColour(BUTTON_BORDER)));
	dc->DrawRectangle(0, 0, width, height);
	wxBitmap arrow = wxBITMAP_PNG(L"arrow_list");
	dc->DrawBitmap((catalogList->size() > 0) ? arrow : arrow.ConvertToDisabled(), width - 17, (height - 10) / 2);
	wxSize ex = theList->GetTextExtent(name);
	dc->SetTextForeground(Options.GetColour(FIND_RESULT_FILENAME_FOREGROUND));
	dc->SetTextBackground(Options.GetColour(FIND_RESULT_FILENAME_BACKGROUND));
	dc->SetBackgroundMode(wxSOLID);
	needTooltip = ex.x + 18 > width - 8;
	wxRect cur(x + 18, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WINDOW_TEXT : WINDOW_TEXT_INACTIVE));
	dc->SetBackgroundMode(wxTRANSPARENT);
	lastX = x;
	lastY = y;
	lastWidth = width;
	lastHeight = height;
}

wxSize CatalogList::GetTextExtents(KaiListCtrl *theList){
	wxSize size = theList->GetTextExtent(name);
	size.x += 28;
	size.y += 4;
	return size;
}

void CatalogList::RefreshCatalogList()
{
	catalogList = FCManagement.GetCatalogNames();
}

void FontItem::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	bool isOnCheckbox = event.GetX() < 19;
	if ((_enter && isOnCheckbox) || (!enter && isOnCheckbox && !leave)){
		enter = true;
		theList->Refresh(false);
	}
	else if (leave || (enter && !isOnCheckbox)){
		enter = false;
		theList->Refresh(false);
	}

	if (isOnCheckbox && (event.LeftDown() || event.LeftDClick())){
		modified = !modified;
		int i = theList->FindItem(0, this);
		int j = i - 1;
		if (i < 0){
			theList->Refresh(false);
			return;
		}
		bool somethingChecked = false;
		while (theList->GetType(j, 0) == TYPE_TEXT){
			Item *item = theList->GetItem(j, 0);
			if (item && item->modified){
				somethingChecked = true;
			}
			j--;
		}
		while (theList->GetType(i, 0) == TYPE_TEXT){
			Item *item = theList->GetItem(i, 0);
			if (item && item->modified){
				somethingChecked = true;
				goto done;
			}
			i++;
		}
	done:
		Item *header = theList->GetItem(j, 0);
		if (header/* && header->type == TYPE_HEADER*/){
			header->modified = somethingChecked;
		}
		theList->Refresh(false);
	}
	
}

void FontItem::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = theList->GetTextExtent(name);
	wxString bitmapName = (modified) ? L"checkbox_selected" : L"checkbox";
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if (enter){ BlueUp(&checkboxBmp); }
	dc->DrawBitmap(checkboxBmp, x + 5, y + (height - 13) / 2);

	needTooltip = ex.x + 22 > width - 8;
	wxRect cur(x + 22, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
}

wxSize FontItem::GetTextExtents(KaiListCtrl *theList){
	wxSize size = theList->GetTextExtent(name);
	size.x += 32;
	size.y += 4;
	return size;
}

void FontSample::OnPaint(wxMemoryDC* dc, int x, int y, int width, int height, KaiListCtrl* theList)
{
	wxRect cur(x, y, width - 8, height);
	wxFont font = dc->GetFont();
	wxFont copy = font;
	copy.SetFaceName(name);
	dc->SetFont(copy);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetFont(font);
}

FontCatalogManagement::~FontCatalogManagement()
{
	SaveCatalogs();
	DestroyCatalogs();
}

void FontCatalogManagement::LoadCatalogs()
{
	if (isInit)
		return;

	wxString path = Options.pathfull + L"\\Config\\FontCatalogs.txt";
	OpenWrite ow;
	wxString fontCatalogsText;
	std::map<wxString, fontList>::iterator it;
	if (ow.FileOpen(path, &fontCatalogsText, false)) {
		wxStringTokenizer cfg(fontCatalogsText, L"\n", wxTOKEN_STRTOK);
		bool block = false;
		while (cfg.HasMoreTokens())
		{
			wxString token = cfg.NextToken();
			token.Trim();
			token.Trim(false);
			if (token.EndsWith(L"{")) {
				block = true;
				AddCatalog(token.BeforeLast(L'='), &it);
				continue;
			}
			else if (block) {
				if (token == L"}") {
					block = false;
				}
				else
				{
					if (it != fontCatalogs.end()) {
						if(!token.empty())
							it->second->Add(token);
					}
					else
					{
						KaiLog(wxString::Format(_("Nie można dodać czcionki %s"), token));
					}
				}
				continue;
			}
		}
	}

	isInit = true;
}

void FontCatalogManagement::SaveCatalogs()
{
	wxString path = Options.pathfull + L"\\Config\\FontCatalogs.txt";
	OpenWrite ow(path);
	wxString fontCatalogsText;
	for (auto it = fontCatalogs.begin(); it != fontCatalogs.end(); it++) {
		fontCatalogsText.Append(it->first + L"={\r\n");
		for(auto & font : *it->second) {
			fontCatalogsText.Append(L"\t" + font + L"\r\n");
		}
		fontCatalogsText.Append(L"}\r\n");
		ow.PartFileWrite(fontCatalogsText);
	}
	ow.CloseFile();
}

void FontCatalogManagement::DestroyCatalogs()
{
	for (auto it = fontCatalogs.begin(); it != fontCatalogs.end(); it++) {
		delete it->second;
	}
	fontCatalogs.clear();
}

wxArrayString* FontCatalogManagement::GetCatalogNames()
{
	return &fontCatalogsNames;
}

wxString FontCatalogManagement::FindCatalogByFont(const wxString& font)
{
	for (auto it = fontCatalogs.begin(); it != fontCatalogs.end(); it++) {
		if(it->second->Index(font) != -1)
			return it->first;
	}
	return wxString();
}

wxArrayString* FontCatalogManagement::GetCatalogFonts(const wxString& catalog)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		return it->second;
	}
	return nullptr;
}

void FontCatalogManagement::AddCatalog(const wxString& catalog, std::map<wxString, fontList>::iterator* it)
{
	auto itc = fontCatalogs.find(catalog);
	if (!(itc != fontCatalogs.end())) {
		fontCatalogs[catalog] = new wxArrayString;
		//is it possible that it cannot find iterator here?
		if (it)
			(*it) = fontCatalogs.find(catalog);
	}
	if (it)
		it = &itc;
}

void FontCatalogManagement::RemoveCatalog(const wxString& catalog)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		delete it->second;
		fontCatalogs.erase(it);
	}
}

void FontCatalogManagement::AddCatalogFont(const wxString& catalog, const wxString& font)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		if(it->second->Index(font) == -1)
			it->second->Add(font);
	}
}

void FontCatalogManagement::AddCatalogFonts(const wxString& catalog, const wxArrayString& fonts)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		for (auto& font : fonts) {
			if (it->second->Index(font) == -1)
				it->second->Add(font);
		}
	}
}

void FontCatalogManagement::RemoveCatalogFont(const wxString& catalog, const wxString& font)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		int fontI = it->second->Index(font);
		if (fontI != -1) {
			it->second->RemoveAt(fontI);
		}
	}
}

void FontCatalogManagement::ReplaceCatalogFonts(const wxString& catalog, const wxArrayString& fonts)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		it->second->Clear();
		for (auto& font : fonts) {
			it->second->Add(font);
		}
	}
}

FontCatalogManagement FCManagement;