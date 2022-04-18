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
#include "KaiStaticText.h"
#include "FontEnumerator.h"
#include "OpennWrite.h"
#include "StylePreview.h"
#include "Menu.h"
#include "KaiMessageBox.h"
#include "KaiStaticBoxSizer.h"
#include "LogHandler.h"
#include <wx/tokenzr.h>
#include <wx/filedlg.h>
#include <wx/window.h>

wxDEFINE_EVENT(CATALOG_CHANGED, wxCommandEvent);

wxArrayString* CatalogList::catalogList = nullptr;
PopupList* CatalogList::floatingList = nullptr;
wxString FontSample::previewText;
FontCatalogList* FontCatalogList::This = nullptr;

class CatalogEdition : public KaiDialog
{
public:
	CatalogEdition(wxWindow* parent, wxArrayString* catalogs, const wxPoint& pos, int selectCatalog);
	virtual ~CatalogEdition() {};
	wxString GetNewCatalogName() {
		return newCatalog->GetValue();
	};
	wxString GetOldCatalogName() {
		return currentCatalog->GetValue();
	};
private:
	void OnOKClick(wxCommandEvent& evt);
	KaiChoice* currentCatalog;
	KaiTextCtrl* newCatalog;
	wxArrayString * catalogNames;
	wxPoint dialogPos;
};

CatalogEdition::CatalogEdition(wxWindow* parent, wxArrayString* catalogs, const wxPoint& pos, int selectCatalog)
	:KaiDialog(parent, -1, _("Wybierz nazwę profilu"), pos)
	, catalogNames(catalogs)
{
	DialogSizer* dSizer = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer* descriptionSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Wprowadź nową nazwę katalogu"));
	currentCatalog = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, *FCManagement.GetCatalogNames());
	currentCatalog->SetSelection(selectCatalog != -1? selectCatalog : 0);
	newCatalog = new KaiTextCtrl(this, -1, emptyString);
	descriptionSizer->Add(currentCatalog, 1, wxALL | wxEXPAND, 2);
	descriptionSizer->Add(new KaiStaticText(this, -1, _("Zamień na:")), 0, wxALL | wxEXPAND, 2);
	descriptionSizer->Add(newCatalog, 1, wxALL | wxEXPAND, 2);
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton* OK = new MappedButton(this, wxID_OK, L"OK");
	MappedButton* cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &CatalogEdition::OnOKClick, this, wxID_OK);
	buttonSizer->Add(OK, 1, wxALL | wxEXPAND, 2);
	buttonSizer->Add(cancel, 1, wxALL | wxEXPAND, 2);
	dSizer->Add(descriptionSizer, 0, wxALL | wxEXPAND, 2);
	dSizer->Add(buttonSizer, 0, wxALL, 2);
	SetSizerAndFit(dSizer);
	CenterOnParent(wxHORIZONTAL);
	dialogPos = pos;
}

void CatalogEdition::OnOKClick(wxCommandEvent& evt)
{
	if (newCatalog->GetValue().empty()) {
		KaiMessageBox(_("Proszę wprowadzić nazwę nowego katalogu"), _("Informacja"), wxOK, this);
		return;
	}


	EndModal(wxID_OK);

}

FontCatalogList::FontCatalogList(wxWindow* parent, const wxString& styleFont)
	: KaiDialog(parent, -1, _("Zarządzanie katalogami czcionek"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	This = this;
	DialogSizer* main = new DialogSizer(wxVERTICAL);
	fontList = new KaiListCtrl(this, ID_FONT_LIST, wxDefaultPosition, wxSize(700, 300));
	fontList->SetFont(*Options.GetFont(4));
	fontList->InsertColumn(0, _("Nazwa czcionki"), TYPE_TEXT, 290);
	fontList->InsertColumn(1, _("Katalog"), TYPE_LIST, 140);
	fontList->InsertColumn(2, _("Przykład"), TYPE_TEXT, 330);
	GenerateList(styleFont);

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

	MappedButton* addCatalog = new MappedButton(this, ID_ADD_CATALOG, _("Dodaj"));
	MappedButton* editCatalog = new MappedButton(this, ID_EDIT_CATALOG, _("Edytuj"));
	MappedButton* removeCatalog = new MappedButton(this, ID_REMOVE_CATALOG, _("Usuń"));
	MappedButton* loadCatalogs = new MappedButton(this, ID_LOAD_CATALOGS, _("Wczytaj"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &FontCatalogList::OnLoadCatalogs, this, ID_LOAD_CATALOGS);
	wxArrayString* catalogList = FCManagement.GetCatalogNames();
	catalog = new KaiChoice(this, -1, emptyString, wxDefaultPosition, wxDefaultSize, *catalogList);
	fontSeek = new KaiTextCtrl(this, ID_FONT_SEEK, emptyString, wxDefaultPosition);
	wxString fontFilterText = Options.GetString(STYLE_EDIT_FILTER_TEXT);
	fontFilter = new KaiTextCtrl(this, -1, fontFilterText);
	MappedButton* saveFilter = new MappedButton(this, ID_SAVE_FILTER, _("Zapisz Filtr"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		wxString ctlg = catalog->GetValue();
		if (!ctlg.empty()) {
			FCManagement.AddCatalog(ctlg);
			catalog->Append(ctlg);
		}
		}, ID_ADD_CATALOG);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		wxString ctlg = catalog->GetValue();
		int ctlgIndex = catalog->FindString(ctlg);
		wxPoint cpos = ClientToScreen(catalog->GetPosition());
		wxSize csize = catalog->GetSize();
		CatalogEdition edit(this, FCManagement.GetCatalogNames(), wxPoint(cpos.x - 200, cpos.y + csize.y), ctlgIndex);
		if (edit.ShowModal() == wxID_OK) {
			if (FCManagement.ChangeCatalogName(this, edit.GetOldCatalogName(), edit.GetNewCatalogName())) {
				RefreshList();
				catalog->SetValue(edit.GetNewCatalogName());
			}
		}
		}, ID_EDIT_CATALOG);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		wxString ctlg = catalog->GetValue();
		int ctlgIndex = catalog->FindString(ctlg);
		if (!ctlg.empty() && ctlgIndex != -1) {
			if (KaiMessageBox(_("Czy na pewno chcesz usunąć ten katalog?"), _("Pytanie"), wxYES_NO, this) == wxYES) {
				FCManagement.RemoveCatalog(ctlg);
				catalog->Delete(ctlgIndex);
				CatalogList::RefreshCatalogList();
			}
		}
		}, ID_REMOVE_CATALOG);

	Bind(LIST_ITEM_LEFT_CLICK, [=](wxCommandEvent& evt) {
		RefreshPreview();
		}, ID_FONT_LIST);

	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent& evt) {
		SetSelectionByPartialName(fontSeek->GetValue());
		}, fontSeek->GetId());

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		Options.SetString(STYLE_EDIT_FILTER_TEXT, fontFilter->GetValue());
		}, ID_SAVE_FILTER);

	preview = new StylePreview(this, -1, wxDefaultPosition, wxSize(-1, 220));
	status = new KaiStatusBar(this);
	int fields[2] = { -11, -21 };
	status->SetFieldsCount(2, fields);
	status->SetLabelText(1, _("Lista katalogów ma opcję autozapisu, pliki znajdują się w folderze \"Config\"."));

	buttonsSizer->Add(new KaiStaticText(this, -1, _("Katalogi:")), 1, wxALL | wxEXPAND | wxALIGN_RIGHT, 2);
	buttonsSizer->Add(catalog, 3, wxALL | wxEXPAND, 2); 
	buttonsSizer->Add(addCatalog, 1, wxALL, 2);
	buttonsSizer->Add(editCatalog, 1, wxALL, 2);
	buttonsSizer->Add(removeCatalog, 1, wxALL, 2);
	buttonsSizer->Add(loadCatalogs, 1, wxALL, 2);
	//buttonsSizer->Add(replaceChecked, 1, wxALL, 2);
	wxBoxSizer* textCtrlsSizer = new wxBoxSizer(wxHORIZONTAL);
	textCtrlsSizer->Add(new KaiStaticText(this, -1, _("Szukaj:")), 1, wxEXPAND | wxALL | wxALIGN_RIGHT, 2);
	textCtrlsSizer->Add(fontSeek, 3, wxEXPAND | wxALL, 2);
	textCtrlsSizer->Add(fontFilter, 3, wxEXPAND | wxALL, 2);
	textCtrlsSizer->Add(saveFilter, 1, wxEXPAND | wxALL, 2);

	main->Add(buttonsSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(textCtrlsSizer, 0, wxEXPAND | wxALL, 2);
	main->Add(fontList, 1, wxEXPAND | wxALL, 2);
	main->Add(preview, 0, wxEXPAND | wxALL, 2);
	main->Add(status, 0, wxEXPAND | wxTOP, 2);

	SetSizerAndFit(main);
	fontStyle = Styles(L"FontPreview,Arial,80,&H00FFFFFF,&HFF0000FF,&H00301946,&H7A301946,-1,0,0,0,100,100,1.13208,0,1,3.5,0,2,120,120,40,1");

	Bind(wxEVT_SHOW, [=](wxShowEvent& evt) {
		//bool show = evt.();
		//if (!show) {
			wxCommandEvent event(CATALOG_CHANGED, GetId());
			parent->GetEventHandler()->AddPendingEvent(event);
		//}
	});
	autoSaveTimer.SetOwner(this, ID_EDIT_TIMER);
	Bind(wxEVT_TIMER, [=](wxTimerEvent& evt) {
		status->SetLabelText(0, _("Autozapis"));
		wxString path = Options.pathfull + L"\\Config\\FontCatalogsAutosave" + std::to_string(autoSaveI) + L".txt";
		FCManagement.SaveCatalogs(path);
		autoSaveI++;
		if (autoSaveI > 2)
			autoSaveI = 0;

		autoSaveTimerRemove.Start(10000, true);
		}, ID_EDIT_TIMER);
	autoSaveTimerRemove.SetOwner(this, ID_EDIT_TIMER_REMOVE);
	Bind(wxEVT_TIMER, [=](wxTimerEvent& evt) {
		status->SetLabelText(0, emptyString);
		}, ID_EDIT_TIMER_REMOVE);
	RefreshPreview();
}

FontCatalogList::~FontCatalogList()
{
	FCManagement.SaveCatalogs();
}

void FontCatalogList::ClearList()
{
	fontList->ClearList();
	//replaceChecked->Enable();
}

void FontCatalogList::GenerateList(const wxString& styleFont)
{
	wxArrayString * fonts = FontEnum.GetFonts(nullptr, [=](){});
	int sel = 0;
	const wxString& previewText = Options.GetString(STYLE_PREVIEW_TEXT);
	FontSample::SetPreviewText(previewText);
	for (auto& font : (*fonts)) {
		int row = fontList->AppendItem(new FontItem(font));
		wxString catalog = FCManagement.FindCatalogByFont(font);
		if (!catalog.empty()) {
			bool isGood = true;
		}
		fontList->SetItem(row, 1, new CatalogList(catalog));
		fontList->SetItem(row, 2, new FontSample(font));
		if (!sel && font.CmpNoCase(styleFont) == 0)
			sel = row;
	}
	fontList->SetSelection(sel);
	fontList->ScrollTo(sel - 2);
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

void FontCatalogList::SetSelectionByPartialName(const wxString& PartialName)
{
	wxCommandEvent evt(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
	this->ProcessEvent(evt);
	int scrollTo = -1;
	wxString PrtName = PartialName.Lower();
	size_t k = 0;
	int lastMatch = 0;
	if (PartialName == emptyString) {
		goto done;
	}
	for (size_t i = 0; i < fontList->GetCount(); i++) {
		Item* item = fontList->GetItem(i, 0);
		if (!item)
			continue;

		wxString fontname = item->name.Lower();
		if (fontname.length() < 1 || fontname[0] < PrtName[0])
			continue;

		while (k < PrtName.length() && k < fontname.length()) {
			if (fontname[k] == PrtName[k]) {
				k++;
				lastMatch = i;
				if (k >= PrtName.length()) {
					scrollTo = i;
					goto done;
				}
			}
			else if (k > 0 && fontname.Mid(0, k) != PrtName.Mid(0, k)) {
				goto done;
			}
			else if (fontname[k] > PrtName[k]) {
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


	fontList->SetSelection(scrollTo);
	fontList->ScrollTo(scrollTo - 2);
}

void FontCatalogList::OnLoadCatalogs(wxCommandEvent& evt)
{
	wxFileDialog* FileDialog = new wxFileDialog(this, _("Wybierz plik wideo"), emptyString,
		emptyString, _("Pliki tekstowe (*.txt)|*.txt"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (FileDialog->ShowModal() == wxID_OK) {
		FCManagement.LoadCatalogs(FileDialog->GetPath());
		RefreshList();
	}
	FileDialog->Destroy();
}

void FontCatalogList::RefreshList(bool catalogListToo)
{
	size_t listSize = fontList->GetCount();
	for (size_t i = 0; i < listSize; i++) {
		Item* fontItem = fontList->GetItem(i, 0);
		Item* catalogItem = fontList->GetItem(i, 1);
		if (fontItem && catalogItem) {
			wxString font = fontItem->name;
			wxString catalog = FCManagement.FindCatalogByFont(font);
			catalogItem->name = catalog;
		}
		else {
			KaiLog(_("Nie można odnaleźć elementów linii %i"));
		}
	}
	if(catalogListToo)
		catalog->PutArray(FCManagement.GetCatalogNames());

	fontList->Refresh(false);
}

void FontCatalogList::SetStyleFont(const wxString& styleFont)
{
	int sel = fontList->FindItem(0, styleFont);

	if (sel >= 0) {
		fontList->SetSelection(sel);
		fontList->ScrollTo(sel - 2);
	}
}

void FontCatalogList::StartEditionTimer(int ms)
{
	if (!This->autoSaveTimer.IsRunning())
		This->autoSaveTimer.Start(ms, true);
}

void CatalogList::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **changed /* = nullptr */)
{
	bool click = event.LeftUp();
	if(isMenuShown && event.LeftDown())
		isMenuShown = false;

	if (_enter) {
		enter = true;
		theList->Refresh(false);
	}
	else if (leave) {
		enter = false;
		theList->Refresh(false);
	}

	else if (click) {
		if (isMenuShown) {
			return;
		}

		RefreshCatalogList();
		int choice = catalogList->Index(name);
		int selection = theList->GetSelection();
		Item* thisItem = theList->GetItem(selection, 0);
		if (!thisItem)
			return;

		Menu menuList;
		int i = 0;
		for (auto& catalog : *catalogList) {
			bool checked = FCManagement.IsFontInCatalog(catalog, thisItem->name);
			menuList.Append(3000 + i, catalog, nullptr, emptyString, ITEM_CHECK_AND_HIDE)->Check(checked);
			i++;
		}
		menuList.SelectOnStart(choice);
		menuList.SetMinWidth(lastWidth);
		
		isMenuShown = true;
		int result = menuList.GetPopupMenuSelection(wxPoint(lastX - 2, lastY + lastHeight), theList);

		
		if (result >= 3000 && catalogList->size() > result - 3000) {
			int listPos = result - 3000;
			int selection = theList->GetSelection();
			Item* thisItem = theList->GetItem(selection, 0);
			if (!thisItem)
				return;

			MenuItem* mitem = menuList.FindItemByPosition(listPos);
			if (!mitem)
				return;

			bool addFont = mitem->IsChecked();
			name = (*catalogList)[listPos];
			thisItem->modified = false;

			if(addFont)
				FCManagement.AddCatalogFont(name, thisItem->name);
			else
				FCManagement.RemoveCatalogFont(name, thisItem->name);
			for (size_t j = 0; j < theList->GetCount(); j++) {
				Item* item = theList->GetItem(j, 0);
				Item* item1 = theList->GetItem(j, 1);
				if (item && item->modified) {
					if (addFont)
						FCManagement.AddCatalogFont(name, item->name);
					else
						FCManagement.RemoveCatalogFont(name, item->name);
					item->modified = false;
					if (item1) {
						item1->name = addFont? name : wxString(emptyString);
					}
				}
			}
			if (!addFont) {
				name = FCManagement.FindCatalogByFont(thisItem->name);
			}
		}
	}
}

void CatalogList::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	dc->SetBrush(wxBrush(Options.GetColour((enter && !clicked) ? BUTTON_BACKGROUND_HOVER :
		(clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND)));
	dc->SetPen(wxPen(Options.GetColour((enter && !clicked) ? BUTTON_BORDER_HOVER :
		(clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER)));
	dc->DrawRectangle(x - 2, y, width, height);
	wxBitmap arrow = wxBITMAP_PNG(L"arrow_list");
	dc->DrawBitmap((catalogList->size() > 0) ? arrow : arrow.ConvertToDisabled(), x + width - 17, y + (height - 10) / 2);
	wxSize ex = theList->GetTextExtent(name);
	//dc->SetBackgroundMode(wxSOLID);
	needTooltip = ex.x > width - 28;
	wxRect cur(x, y, width - 22, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	//dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WINDOW_TEXT : WINDOW_TEXT_INACTIVE));
	//dc->SetBackgroundMode(wxTRANSPARENT);
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

void FontItem::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **changed /* = nullptr */)
{
	bool isOnCheckbox = event.GetX() < 23;
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
	dc->DrawLabel(previewText, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetFont(font);
}

FontCatalogManagement::~FontCatalogManagement()
{
	//cannot save from here cause options is already released
	//SaveCatalogs();
	DestroyCatalogs();
}

void FontCatalogManagement::LoadCatalogs(const wxString& external)
{
	bool isExternal = !external.empty();
	if (isInit && !isExternal)
		return;

	wxString path = (isExternal)? external : Options.pathfull + L"\\Config\\FontCatalogs.txt";
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
			if (token.EndsWith(L"{") && !token.StartsWith(L"\t")) {
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
						token.Trim(false);
						if (!token.empty()) {
							if (it->second->Index(token) == -1)
								it->second->Add(token);
						}
					}
					else
					{
						KaiLog(wxString::Format(_("Nie można dodać czcionki %s."), token));
					}
				}
				continue;
			}
		}
	}
	if(!isExternal)
		isInit = true;
}

void FontCatalogManagement::SaveCatalogs(const wxString& external)
{
	wxString path = external.empty()? Options.pathfull + L"\\Config\\FontCatalogs.txt" : external;
	OpenWrite ow(path);
	for (auto it = fontCatalogs.begin(); it != fontCatalogs.end(); it++) {
		wxString fontCatalogsText;
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
		std::sort(it->second->begin(), it->second->end(), [](const wxString& first, const wxString& second)
			{
				return first.CmpNoCase(second) < 0;
			});
		return it->second;
	}
	return nullptr;
}

bool FontCatalogManagement::IsFontInCatalog(const wxString& catalog, const wxString& font)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end() && it->second->Index(font) != -1)
		return true;

	return false;
}

void FontCatalogManagement::AddToCatalog(const wxString& font, const wxPoint& pos, wxWindow *parent)
{
	if (!fontCatalogsNames.GetCount()) {
		KaiLog(_("Aby móc dodawać czcionki do katalogu,\nnależy najpierw kliknąć przycisk \"Zarządzaj\",\nby utworzyć nowy katalog."));
		return;
	}
	Menu menuList;
	std::vector<bool> checkTable;
	int i = 0;
	for (auto& catalog : fontCatalogsNames) {
		bool checked = IsFontInCatalog(catalog, font);
		menuList.Append(3000 + i, catalog, nullptr, emptyString, ITEM_CHECK_AND_HIDE)->Check(checked);
		checkTable.push_back(checked);
		i++;
	}
	int result = menuList.GetPopupMenuSelection(wxPoint(pos.x, pos.y), parent);
	bool changed = false;
	for (size_t j = 0; j < checkTable.size(); j++) {
		MenuItem *item = menuList.FindItemByPosition(j);
		if (item && item->IsChecked() != checkTable[j]) {
			if (item->IsChecked()) {
				AddCatalogFont(item->GetLabel(), font, false);
			}
			else {
				RemoveCatalogFont(item->GetLabel(), font, false);
			}
			changed = true;
		}
	}
	if (changed)
		SaveCatalogs();
}

void FontCatalogManagement::AddCatalog(const wxString& catalog, std::map<wxString, fontList>::iterator* it)
{
	auto itc = fontCatalogs.find(catalog);
	bool itcEnd = !(itc != fontCatalogs.end());
	if (itcEnd) {
		fontCatalogs[catalog] = new wxArrayString;
		if (fontCatalogsNames.Index(catalog) == -1)
			fontCatalogsNames.Add(catalog);
		//is it possible that it cannot find iterator here?
		if (it) {
			(*it) = fontCatalogs.find(catalog);
		}
	}
	else{
		(*it) = itc;
	}

	if(!it) {
		FontCatalogList::StartEditionTimer(saveInterval);
	}
}

bool FontCatalogManagement::ChangeCatalogName(wxWindow* messagesParent, const wxString& oldCatalog, const wxString& newCatalog)
{
	wxArrayString* fontTable = nullptr;
	auto itn = fontCatalogs.find(newCatalog);
	if (itn != fontCatalogs.end()) {
		KaiMessageDialog dlg(messagesParent, wxString::Format(_("Katalog o nazwie \"%s\" istnieje, co zrobić?"), newCatalog), _("Pytanie"), wxYES_NO | wxCANCEL);
		dlg.SetYesLabel(_("Scal"));
		dlg.SetNoLabel(_("Usuń"));
		int result = dlg.ShowModal();
		if (result == wxYES) {
			fontTable = itn->second;
		}
		else if (result == wxNO) {
			delete itn->second;
		}
		else
			return false;
	}
	auto it = fontCatalogs.find(oldCatalog);
	if (it != fontCatalogs.end()) {
		if (fontTable) {
			for (auto& font : *it->second) {
				if (fontTable->Index(font) == -1)
					fontTable->Add(font);
			}
			delete it->second;
		}else
			fontTable = it->second;
		fontCatalogs.erase(it);
		int result = fontCatalogsNames.Index(oldCatalog);
		if (result != -1)
			fontCatalogsNames.RemoveAt(result);
	}
	
	fontCatalogs[newCatalog] = (fontTable) ? fontTable : new wxArrayString();
	FontCatalogList::StartEditionTimer(saveInterval);
	return true;
}

void FontCatalogManagement::RemoveCatalog(const wxString& catalog)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		int result = fontCatalogsNames.Index(catalog);
		if (result != -1) {
			fontCatalogsNames.RemoveAt(result);
		}
		delete it->second;
		fontCatalogs.erase(it);
	}
	FontCatalogList::StartEditionTimer(saveInterval);
}

void FontCatalogManagement::AddCatalogFont(const wxString& catalog, const wxString& font, bool AutoSave)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		if(it->second->Index(font) == -1)
			it->second->Add(font);
	}
	if(AutoSave)
		FontCatalogList::StartEditionTimer(saveInterval);
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
	FontCatalogList::StartEditionTimer(saveInterval);
}

void FontCatalogManagement::RemoveCatalogFont(const wxString& catalog, const wxString& font, bool AutoSave)
{
	auto it = fontCatalogs.find(catalog);
	if (it != fontCatalogs.end()) {
		int fontI = it->second->Index(font);
		if (fontI != -1) {
			it->second->RemoveAt(fontI);
		}
	}
	if(AutoSave)
		FontCatalogList::StartEditionTimer(saveInterval);
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
	FontCatalogList::StartEditionTimer(saveInterval);
}

FontCatalogManagement FCManagement;