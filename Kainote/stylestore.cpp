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



#include "StyleStore.h"
#include "kainoteFrame.h"
#include "KaiStaticBoxSizer.h"
#include "config.h"
#include "SubsGrid.h"
#include "Notebook.h"
#include "TabPanel.h"
#include "NewCatalog.h"
#include "OpennWrite.h"
#include "Stylelistbox.h"
#include "EditBox.h"
#include <wx/tokenzr.h>
#include <vector>
#include "Styles.h"
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/filedlg.h>
#include "StyleChange.h"
#include "KaiMessageBox.h"
#include "FontEnumerator.h"


StyleStore::StyleStore(wxWindow* parent, const wxPoint& pos)
	: KaiDialog(parent, -1, _("Menedżer stylów"), pos, wxSize(400, -1), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, stayOnTop(false)
{
	bool isDetached = detachedEtit = Options.GetBool(STYLE_MANAGER_DETACH_EDIT_WINDOW);
	wxIcon icn;
	icn.CopyFromBitmap(wxBITMAP_PNG(L"styles"));
	SetIcon(icn);

	cc = new StyleChange(this, !isDetached);
	cc->Hide();

	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));

	wxBitmap arrowDown = wxBITMAP_PNG(L"arrow_list");
	wxBitmap arrowDownDouble = wxBITMAP_PNG(L"ARROW_LIST_DOUBLE");
	wxImage arrowcopy = arrowDown.ConvertToImage();
	arrowcopy = arrowcopy.Rotate180();
	wxImage arrowDoublecopy = arrowDownDouble.ConvertToImage();
	arrowDoublecopy = arrowDoublecopy.Rotate180();
	wxBitmap arrowUp = wxBitmap(arrowcopy);
	wxBitmap arrowUpDouble = wxBitmap(arrowDoublecopy);

	wxBoxSizer *Mainsm = new wxBoxSizer(wxVERTICAL);
	Mainall = new DialogSizer(wxHORIZONTAL);

	KaiStaticBoxSizer *catalogSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Katalog:"));
	catalogList = new KaiChoice(this, ID_CATALOG, wxDefaultPosition, wxDefaultSize, Options.dirs);
	int chc = catalogList->FindString(Options.actualStyleDir);
	catalogList->SetSelection(chc);
	newCatalog = new MappedButton(this, ID_NEWCAT, _("Nowy"));
	MappedButton *deleteCatalog = new MappedButton(this, ID_DELCAT, _("Usuń"));
	deleteCatalog->SetToolTip(_("Usuń wybrany katalog stylów"));
	catalogSizer->Add(catalogList, 1, wxEXPAND | wxALL, 2);
	catalogSizer->Add(newCatalog, 0, wxEXPAND | wxALL, 2);
	catalogSizer->Add(deleteCatalog, 0, wxEXPAND | wxALL, 2);


	KaiStaticBoxSizer *catalogStylesSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Style katalogu:"));
	wxBoxSizer *catalogButtonsSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *catalogMainSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *catalogMoveButtonsSizer = new wxBoxSizer(wxVERTICAL);

	Store = new StyleList(this, ID_STORESTYLES, &Options.assstore, wxDefaultPosition, wxSize(-1, 520));

	storeNew = new MappedButton(this, ID_STORENEW, _("Nowy"));
	storeCopy = new MappedButton(this, ID_STORECOPY, _("Kopiuj"));
	storeEdit = new MappedButton(this, ID_STOREEDIT, _("Edytuj"));
	storeLoad = new MappedButton(this, ID_STORELOAD, _("Wczytaj"));
	storeDelete = new MappedButton(this, ID_STOREDEL, _("Usuń"));
	storeSort = new MappedButton(this, ID_STORESORT, _("Sortuj"));

	catalogButtonsSizer->Add(storeNew, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	catalogButtonsSizer->Add(storeCopy, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	catalogButtonsSizer->Add(storeEdit, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	catalogButtonsSizer->Add(storeLoad, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	catalogButtonsSizer->Add(storeDelete, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	catalogButtonsSizer->Add(storeSort, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	int fw, fh;
	GetTextExtent(L"X#F", &fw, &fh);
	fh += 8;
	fw = fh;
	if (fw % 2 == 0)
		fw--;

	MappedButton *storeMoveToStart = new MappedButton(this, ID_STORE_MOVE_TO_START, _("Przesuń zaznaczone style na sam początek"), arrowUpDouble, wxDefaultPosition, wxSize(fw, fh), -1);
	MappedButton *storeMoveUp = new MappedButton(this, ID_STORE_MOVE_UP, _("Przesuń zaznaczone style w górę"), arrowUp, wxDefaultPosition, wxSize(fw, fh), -1);
	MappedButton *storeMoveDown = new MappedButton(this, ID_STORE_MOVE_DOWN, _("Przesuń zaznaczone style w dół"), arrowDown, wxDefaultPosition, wxSize(fw, fh), -1);
	MappedButton *storeMoveToEnd = new MappedButton(this, ID_STORE_MOVE_TO_END, _("Przesuń zaznaczone style na sam koniec"), arrowDownDouble, wxDefaultPosition, wxSize(fw, fh), -1);

	catalogMoveButtonsSizer->Add(storeMoveToStart, 0, wxTOP | wxBOTTOM, 2);
	catalogMoveButtonsSizer->Add(storeMoveUp, 0, wxTOP | wxBOTTOM, 2);
	catalogMoveButtonsSizer->Add(storeMoveDown, 0, wxTOP | wxBOTTOM, 2);
	catalogMoveButtonsSizer->Add(storeMoveToEnd, 0, wxTOP | wxBOTTOM, 2);

	catalogMainSizer->Add(catalogMoveButtonsSizer, 0, wxRIGHT | wxCENTER, 4);
	catalogMainSizer->Add(Store, 4, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 2);
	catalogMainSizer->Add(catalogButtonsSizer, 1, wxEXPAND, 0);

	catalogStylesSizer->Add(catalogMainSizer, 1, wxEXPAND | wxALL, 2);

	wxBoxSizer *addToButtons = new wxBoxSizer(wxHORIZONTAL);

	addToStore = new MappedButton(this, ID_ADDTOSTORE, _("Dodaj do magazynu"), arrowUp, wxDefaultPosition, wxDefaultSize, -1, 0, _("Dodaj do magazynu"));
	addToAss = new MappedButton(this, ID_ADDTOASS, _("Dodaj do ASS"), arrowDown, wxDefaultPosition, wxDefaultSize, -1, 0, _("Dodaj do ASS"));
	MappedButton* addToAllAss = new MappedButton(this, ID_ADD_TO_ALL_ASS, _("Dodaj do wszystkich otwartych ASS"), arrowDownDouble, wxDefaultPosition, wxDefaultSize, -1, 0, _("Dodaj do wszystkich otwartych ASS"));
	addToButtons->Add(addToStore, 1, wxEXPAND | wxALL, 5);
	addToButtons->Add(addToAss, 1, wxEXPAND | wxALL, 5);
	//addToButtons->AddStretchSpacer(3);

	KaiStaticBoxSizer *ASSStylesSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Style pliku ASS:"));
	wxBoxSizer *ASSButtonsSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *ASSMainSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *ASSMoveButtonsSizer = new wxBoxSizer(wxVERTICAL);

	ASSList = new StyleList(this, ID_ASSSTYLES, Notebook::GetTab()->grid->GetStyleTable(), wxDefaultPosition, wxSize(-1, 520));


	assNew = new MappedButton(this, ID_ASSNEW, _("Nowy"));
	assCopy = new MappedButton(this, ID_ASSCOPY, _("Kopiuj"));
	assEdit = new MappedButton(this, ID_ASSEDIT, _("Edytuj"));
	assLoad = new MappedButton(this, ID_ASSLOAD, _("Wczytaj"));
	assDelete = new MappedButton(this, ID_ASSDEL, _("Usuń"));
	assSort = new MappedButton(this, ID_ASSSORT, _("Sortuj"));
	SClean = new MappedButton(this, ID_ASSCLEAN, _("Oczyść"));

	ASSButtonsSizer->Add(assNew, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	ASSButtonsSizer->Add(assCopy, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	ASSButtonsSizer->Add(assEdit, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	ASSButtonsSizer->Add(assLoad, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	ASSButtonsSizer->Add(assDelete, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	ASSButtonsSizer->Add(assSort, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	ASSButtonsSizer->Add(SClean, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);

	MappedButton *ASSMoveToStart = new MappedButton(this, ID_ASS_MOVE_TO_START, _("Przesuń zaznaczone style na sam początek"), arrowUpDouble, wxDefaultPosition, wxSize(fw, fh), -1);
	MappedButton *ASSMoveUp = new MappedButton(this, ID_ASS_MOVE_UP, _("Przesuń zaznaczone style w górę"), arrowUp, wxDefaultPosition, wxSize(fw, fh), -1);
	MappedButton *ASSMoveDown = new MappedButton(this, ID_ASS_MOVE_DOWN, _("Przesuń zaznaczone style w dół"), arrowDown, wxDefaultPosition, wxSize(fw, fh), -1);
	MappedButton *ASSMoveToEnd = new MappedButton(this, ID_ASS_MOVE_TO_END, _("Przesuń zaznaczone style na sam koniec"), arrowDownDouble, wxDefaultPosition, wxSize(fw, fh), -1);

	ASSMoveButtonsSizer->Add(ASSMoveToStart, 0, wxTOP | wxBOTTOM, 2);
	ASSMoveButtonsSizer->Add(ASSMoveUp, 0, wxTOP | wxBOTTOM, 2);
	ASSMoveButtonsSizer->Add(ASSMoveDown, 0, wxTOP | wxBOTTOM, 2);
	ASSMoveButtonsSizer->Add(ASSMoveToEnd, 0, wxTOP | wxBOTTOM, 2);

	ASSMainSizer->Add(ASSMoveButtonsSizer, 0, wxRIGHT | wxCENTER, 4);
	ASSMainSizer->Add(ASSList, 4, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 2);
	ASSMainSizer->Add(ASSButtonsSizer, 1, wxEXPAND);

	ASSStylesSizer->Add(ASSMainSizer, 1, wxEXPAND | wxALL, 2);
	wxBoxSizer * buttons = new wxBoxSizer(wxHORIZONTAL);
	close = new MappedButton(this, ID_CLOSE_STYLE_MANAGER, _("Zamknij"));
	detachEnable = new ToggleButton(this, ID_DETACH, _("Odepnij okno edycji"));
	detachEnable->SetValue(isDetached);
	buttons->Add(close, 0, wxRIGHT, 2);
	buttons->Add(detachEnable, 0, wxLEFT, 2);

	Mainsm->Add(catalogSizer, 0, wxEXPAND | wxALL, 2);
	Mainsm->Add(catalogStylesSizer, 1, wxEXPAND | wxALL, 2);
	Mainsm->Add(addToButtons, 0, wxEXPAND | wxALL, 2);
	Mainsm->Add(addToAllAss, 0, wxALIGN_CENTER | wxALL, 2);
	Mainsm->Add(ASSStylesSizer, 1, wxEXPAND | wxALL, 2);
	Mainsm->Add(buttons, 0, wxALIGN_CENTER | wxALL, 4);

	Mainall->Add(Mainsm, 1, wxEXPAND);
	if (!isDetached){
		Mainall->Add(cc, 0, wxEXPAND);
		wxSize bs = wxSize(-1, cc->GetBestSize().y + 29);
		Mainall->SetMinSize(bs);
	}
	SetEscapeId(ID_CLOSE_STYLE_MANAGER, true);
	SetEnterId(ID_CONFIRM);

	SetSizerAndFit(Mainall);

	Connect(ID_ASSSTYLES, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, (wxObjectEventFunction)&StyleStore::OnAssStyleChange);
	Connect(ID_ASSSTYLES, wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&StyleStore::OnSwitchLines);
	//Connect(ID_ASSSTYLES, SELECTION_CHANGED, (wxObjectEventFunction)&StyleStore::OnSelectionChanged);
	Connect(ID_STORESTYLES, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, (wxObjectEventFunction)&StyleStore::OnStoreStyleChange);
	//Connect(ID_STORESTYLES, SELECTION_CHANGED, (wxObjectEventFunction)&StyleStore::OnSelectionChanged);
	Connect(ID_ADDTOSTORE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAddToStore);
	Connect(ID_ADDTOASS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAddToAss);
	Connect(ID_ADD_TO_ALL_ASS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAddToAssInAllTabs);
	Connect(ID_CATALOG, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&StyleStore::OnChangeCatalog);
	Connect(ID_NEWCAT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnNewCatalog);
	Connect(ID_DELCAT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnDeleteCatalog);
	Connect(ID_STORENEW, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnStoreNew);
	Connect(ID_STORECOPY, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnStoreCopy);
	Connect(ID_STOREEDIT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnStoreStyleChange);
	Connect(ID_STORELOAD, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnStoreLoad);
	Connect(ID_STOREDEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnStoreDelete);
	Connect(ID_STORESORT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnStoreSort);
	Connect(ID_ASSNEW, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAssNew);
	Connect(ID_ASSCOPY, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAssCopy);
	Connect(ID_ASSEDIT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAssStyleChange);
	Connect(ID_ASSLOAD, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAssLoad);
	Connect(ID_ASSDEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAssDelete);
	Connect(ID_ASSSORT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnAssSort);
	Connect(ID_ASSCLEAN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnCleanStyles);
	Connect(ID_CONFIRM, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnConfirm);
	Connect(ID_CLOSE_STYLE_MANAGER, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnClose);
	Connect(ID_DETACH, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, (wxObjectEventFunction)&StyleStore::OnDetachEdit);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &StyleStore::OnStyleMove, this, ID_ASS_MOVE_TO_START, ID_STORE_MOVE_TO_END);

	DoTooltips();

	
}

StyleStore::~StyleStore()
{
	if (cc){
		cc->Destroy();
		cc = nullptr;
	}
}

void StyleStore::OnSwitchLines(wxCommandEvent& event)
{
	Notebook::GetTab()->edit->RefreshStyle();
}


void StyleStore::OnAssStyleChange(wxCommandEvent& event)
{
	wxArrayInt selects;
	int numSelections = ASSList->GetSelections(selects);
	if (numSelections < 1){ wxBell(); return; }
	selnum = selects[0];
	ASSStyle = true;
	dummy = false;
	StylesWindow();
}
void StyleStore::OnStoreStyleChange(wxCommandEvent& event)
{
	wxArrayInt selects;
	int numSelections = Store->GetSelections(selects);
	if (numSelections < 1){ wxBell(); return; }
	selnum = selects[0];
	ASSStyle = false;
	dummy = false;
	StylesWindow();
}

void StyleStore::OnAddToStore(wxCommandEvent& event)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	wxArrayInt sels;
	int numSelections = ASSList->GetSelections(sels);
	if (numSelections < 1){ wxBell(); return; }
	Store->SetSelection(wxNOT_FOUND);
	prompt = 0;
	for (size_t i = 0; i < sels.GetCount(); i++){
		Styles *stylc = grid->GetStyle(sels[i])->Copy();
		int found = Options.FindStyle(stylc->Name);
		if (found != -1){
			if (prompt != wxYES_TO_ALL){
				prompt = KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmienić go?"),
					stylc->Name), _("Potwierdzenie"), wxYES_TO_ALL | wxYES | wxNO | wxCANCEL, this);
				if (prompt == wxCANCEL){ 
					delete stylc;
					break; 
				}
			}
			if (prompt == wxYES || prompt == wxYES_TO_ALL){
				Options.ChangeStyle(stylc, found); Store->SetSelection(found);
			}
			else{ delete stylc; }
		}
		else{ Options.AddStyle(stylc); Store->SetSelection(Options.StoreSize() - 1); }
	}
	isStoreChanged = true;
}

void StyleStore::OnAddToAss(wxCommandEvent& event)
{
	//wxMutexLocker lock(mutex);
	SubsGrid* grid = Notebook::GetTab()->grid;
	wxArrayInt sels;
	int numSelections = Store->GetSelections(sels);
	if (numSelections < 1){ wxBell(); return; }
	ASSList->SetSelection(wxNOT_FOUND);
	prompt = 0;
	for (int i = 0; i < numSelections; i++)
	{
		Styles *stylc = Options.GetStyle(sels[i])->Copy();
		int found = grid->FindStyle(stylc->Name);
		if (found != -1){
			if (prompt != wxYES_TO_ALL){
				prompt = KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmienić go?"),
					stylc->Name), _("Potwierdzenie"), wxYES_TO_ALL | wxYES | wxNO | wxCANCEL, this);
				if (prompt == wxCANCEL){
					delete stylc;
					break;
				}
			}
			if (prompt == wxYES || prompt == wxYES_TO_ALL){
				grid->ChangeStyle(stylc, found); ASSList->SetSelection(found);
			}
			else{ delete stylc; }
		}
		else{ grid->AddStyle(stylc); ASSList->SetSelection(grid->StylesSize() - 1); }
	}
	SetModified();
}

void StyleStore::OnAddToAssInAllTabs(wxCommandEvent& event)
{
	Notebook *tabs = Notebook::GetTabs();
	KainoteFrame *Kai = (KainoteFrame*)tabs->GetParent();
	wxArrayInt sels;
	int numSelections = Store->GetSelections(sels);
	if (numSelections < 1){ wxBell(); return; }
	ASSList->SetSelection(wxNOT_FOUND);
	prompt = 0;
	for (size_t k = 0; k < tabs->Size(); k++){
		TabPanel *tab = tabs->Page(k);
		SubsGrid *grid = tab->grid;
		if (grid->subsFormat != ASS)
			continue;

		bool isActive = (k == Kai->Tabs->iter);

		for (int i = 0; i < numSelections; i++){
			Styles *stylc = Options.GetStyle(sels[i])->Copy();
			int found = grid->FindStyle(stylc->Name);
			if (found != -1){
				if (prompt != wxYES_TO_ALL && prompt != wxCANCEL){
					prompt = KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmienić go?"),
						stylc->Name), _("Potwierdzenie"), wxYES_TO_ALL | wxYES | wxNO | wxCANCEL, this);
				}
				if (prompt == wxYES || prompt == wxYES_TO_ALL){
					grid->ChangeStyle(stylc, found); 
					if (isActive)
						ASSList->SetSelection(found);
				}
				else{ delete stylc; }
			}
			else{ 
				grid->AddStyle(stylc); 
				if (isActive)
					ASSList->SetSelection(grid->StylesSize() - 1); 
			}
		}
		tab->edit->RefreshStyle();
		grid->SetModified(STYLE_MANAGER, false);
		//refresh to remove missing styles indicators
		grid->Refresh(false);
		if (isActive)
			ASSList->SetArray(grid->GetStyleTable());
		Kai->Label(tab->grid->file->GetActualHistoryIter(), false, k, !isActive);
	}
}

void StyleStore::OnStoreDelete(wxCommandEvent& event)
{
	wxArrayInt sels;
	int numSelections = Store->GetSelections(sels);
	if (numSelections < 1){ wxBell(); return; }
	for (int ii = sels.GetCount() - 1; ii >= 0; ii--)
	{
		Options.DelStyle(sels[ii]);
	}
	Store->SetSelection(0, true);
	isStoreChanged = true;
}

void StyleStore::OnAssDelete(wxCommandEvent& event)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	wxArrayInt sels;
	int numSelections = ASSList->GetSelections(sels);
	if (numSelections < 1){ wxBell(); return; }
	for (int ii = sels.GetCount() - 1; ii >= 0; ii--)
	{
		grid->DelStyle(sels[ii]);
	}

	ASSList->SetSelection(0, true);
	SetModified();
}

void StyleStore::StylesWindow(wxString newname)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	Styles *tab = nullptr;
	if (selnum < 0){ tab = new Styles(); }
	else if (ASSStyle){ tab = grid->GetStyle(selnum)->Copy(); }
	else{ tab = Options.GetStyle(selnum)->Copy(); }
	if (newname != emptyString){ tab->Name = newname; }
	oldname = tab->Name;
	cc->UpdateValues(tab, !dummy, (ASSStyle && ASSList->GetNumSelections() > 1) || 
		(!ASSStyle && Store->GetNumSelections() > 1));
	if (!detachedEtit){ Mainall->Fit(this); }

}

bool StyleStore::ChangeStyle(Styles *changedStyle, int cellsToChange /*= -1*/)
{
	//Update();
	SubsGrid* grid = Notebook::GetTab()->grid;
	int multiplication = 0;
	int foundStyle = (ASSStyle) ? grid->FindStyle(changedStyle->Name, &multiplication) : 
		Options.FindStyle(changedStyle->Name, &multiplication);
	if (foundStyle != -1) 
		multiplication = 1;

	if (!dummy){
		if (ASSStyle && selnum >= grid->StylesSize()){ dummy = true; }
		else if (!ASSStyle && selnum >= Options.StoreSize()){ dummy = true; }
	}

	if (foundStyle != -1 && dummy || (multiplication > 1 || multiplication == 1 && oldname != changedStyle->Name) && !dummy)
	{
		Mainall->Fit(this);
		KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" jest już na liście."), changedStyle->Name));
		delete changedStyle;
		return false;
	}

	if (foundStyle != -1){ selnum = foundStyle; }
	else if (!dummy){ selnum = (ASSStyle) ? grid->FindStyle(oldname) : Options.FindStyle(oldname); }
	if (selnum < 0){ dummy = true; }

	if (cellsToChange != -1){
		wxArrayInt stylesSelection;
		if (ASSStyle)
			ASSList->GetSelections(stylesSelection);
		else
			Store->GetSelections(stylesSelection);

		for (size_t i = 0; i < stylesSelection.size(); i++){
			int numStyle = stylesSelection[i];
			//don't change edited line
			if (numStyle == selnum)
				continue;

			if (ASSStyle){
				//when numStyle is greater then size something was wrong
				if (numStyle >= grid->StylesSize())
					break;

				Styles *copy = grid->GetStyle(numStyle)->Copy();
				copy->CopyChanges(changedStyle, cellsToChange);
				grid->ChangeStyle(copy, numStyle);
			}
			else{
				if (numStyle >= Options.StoreSize())
					break;

				Styles *copy = Options.GetStyle(numStyle)->Copy();
				copy->CopyChanges(changedStyle, cellsToChange);
				Options.ChangeStyle(copy, numStyle);
			}
		}
	}

	if (dummy){
		if (ASSStyle){
			grid->AddStyle(changedStyle);
			ASSList->SetSelection(grid->StylesSize() - 1, cellsToChange == -1);
			selnum = grid->StylesSize() - 1;
		}
		else{
			Options.AddStyle(changedStyle);
			Store->SetSelection(Options.StoreSize() - 1, cellsToChange == -1);
			selnum = Options.StoreSize() - 1;
		}
	}
	else{
		if (ASSStyle){ grid->ChangeStyle(changedStyle, selnum); ASSList->Refresh(false); }
		else{ Options.ChangeStyle(changedStyle, selnum); Store->Refresh(false); }
	}

	Mainall->Fit(this);
	bool refreshActiveLine = false;
	if (oldname != changedStyle->Name && ASSStyle && !dummy){
		int res = KaiMessageBox(_("Nazwa stylu została zmieniona, czy chcesz zmienić ją także w napisach?"), _("Potwierdzenie"), wxYES_NO);
		if (res == wxYES){
			for (size_t i = 0; i < grid->GetCount(); i++){
				if (grid->GetDialogue(i)->Style == oldname)
				{
					grid->CopyDialogue(i)->Style = changedStyle->Name;
				}
			}
			grid->AdjustWidths(STYLE);
			refreshActiveLine = true;
		}
	}
	oldname = changedStyle->Name;
	dummy = false;
	SetModified(refreshActiveLine);
	if (!ASSStyle)
		isStoreChanged = true;

	return true;
}

void StyleStore::OnChangeCatalog(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	wxString catalogName = catalogList->GetString(catalogList->GetSelection());
	Options.LoadStyles(catalogName);

	Store->SetSelection(0, true);
	SubsGrid* grid = Notebook::GetTab()->grid;
	grid->AddSInfo(L"Last Style Storage", catalogName);
	isStoreChanged = false;
}

void StyleStore::OnNewCatalog(wxCommandEvent& event)
{
	NewCatalog nc(this);
	if (nc.ShowModal() == wxID_OK){
		if (isStoreChanged){
			Options.SaveOptions(false);
			isStoreChanged = false;
		}

		wxString catalogName = nc.TextCtrl1->GetValue();
		catalogList->SetSelection(catalogList->Append(catalogName));
		Options.dirs.Add(catalogName);
		Options.actualStyleDir = catalogName;
		Options.clearstyles();
		Store->Refresh(false);
		SubsGrid* grid = Notebook::GetTab()->grid;
		grid->AddSInfo(L"Last Style Storage", catalogName);
	}
}

void StyleStore::OnDeleteCatalog(wxCommandEvent& event)
{
	int cat = catalogList->GetSelection();
	if (cat == -1){ return; }
	wxString Cat = catalogList->GetString(cat);
	if (Cat == L"Default"){ wxBell(); return; }
	if (KaiMessageBox(wxString::Format(("Naprawdę chcesz usunąć katalog o nazwie \"%s\"?"), Cat), _("Pytanie"), 
		wxYES_NO, nullptr, wxDefaultPosition, wxNO) == wxNO)
		return;

	catalogList->Delete(cat);
	Options.dirs.RemoveAt(cat);
	Options.actualStyleDir = Options.dirs[MAX(0, cat - 1)];
	catalogList->SetSelection(MAX(0, cat - 1));
	wxString path;
	path << Options.pathfull << L"\\Catalog\\" << Cat << L".sty";
	wxRemoveFile(path);
	Options.LoadStyles(Options.actualStyleDir);
	Store->Refresh(false);
	SubsGrid* grid = Notebook::GetTab()->grid;
	grid->AddSInfo(L"Last Style Storage", Options.actualStyleDir);
}

void StyleStore::OnStoreLoad(wxCommandEvent& event)
{
	LoadStylesS(false);
}

void StyleStore::OnAssSort(wxCommandEvent& event)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	std::sort(grid->GetStyleTable()->begin(), grid->GetStyleTable()->end(), sortfunc);
	ASSList->SetSelection(0, true);
	grid->file->edited = true;
	SetModified();
}

void StyleStore::OnAssLoad(wxCommandEvent& event)
{
	LoadStylesS(true);
}

void StyleStore::OnStoreSort(wxCommandEvent& event)
{
	Options.Sortstyles();
	Store->SetSelection(0, true);
}

void StyleStore::LoadStylesS(bool isass)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	wxFileDialog *openFileDialog = new wxFileDialog(this, _("Wybierz plik ASS"),
		Notebook::GetTab()->SubsPath.BeforeLast(L'\\'), L"*.ass", _("Pliki napisów ASS(*.ass)|*.ass"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog->ShowModal() == wxID_OK){
		OpenWrite op;
		wxString ass;
		op.FileOpen(openFileDialog->GetPath(), &ass);
		size_t start = ass.find(L"\nStyle: ");
		size_t end = ass.find(L"[Events]");
		if (end <= start){ return; }
		std::vector<Styles*> tmps;
		wxString styless = ass.SubString(start, end);
		prompt = 0;
		wxStringTokenizer styletkn(styless, L"\n");
		while (styletkn.HasMoreTokens())
		{
			wxString token = styletkn.NextToken();
			if (token.StartsWith(L"Style: ")){
				tmps.push_back(new Styles(token));
			}
		}
		Stylelistbox stl(this);
		for (size_t i = 0; i < tmps.size(); i++){

			stl.CheckListBox->AppendItem(new ItemCheckBox(false, tmps[i]->Name));
		}
		if (isass){ ASSList->SetSelection(wxNOT_FOUND); }
		else{ Store->SetSelection(wxNOT_FOUND); }
		if (stl.ShowModal() == wxID_OK){
			for (size_t v = 0; v < stl.CheckListBox->GetCount(); v++)
			{
				if (stl.CheckListBox->GetItem(v, 0)->modified){

					int fstyle = (isass) ? grid->FindStyle(stl.CheckListBox->GetItem(v, 0)->name) : Options.FindStyle(stl.CheckListBox->GetItem(v, 0)->name);
					if (fstyle == -1){
						if (isass){ grid->AddStyle(tmps[v]); ASSList->Refresh(false); }
						else{ Options.AddStyle(tmps[v]); Store->Refresh(false); }
					}
					else{
						if (prompt != wxYES_TO_ALL && prompt != wxCANCEL){
							prompt = KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmienić go?"),
								stl.CheckListBox->GetItem(v, 0)->name), _("Potwierdzenie"), 
								wxYES_TO_ALL | wxYES | wxNO | wxCANCEL, this);
							//if(prompt == wxID_CANCEL){return;}
						}
						if (prompt == wxYES || prompt == wxYES_TO_ALL){
							if (isass){ grid->ChangeStyle(tmps[v], fstyle); ASSList->SetSelection(fstyle); }
							else{ Options.ChangeStyle(tmps[v], fstyle); Store->SetSelection(fstyle); }
						}
						else{ delete tmps[v]; }
					}

				}
				else{
					delete tmps[v];
				}
			}
		}
		else{
			for (size_t i = 0; i < tmps.size(); i++){
				delete tmps[i];
			}
		}
		tmps.clear();
	}
	openFileDialog->Destroy();
	if (isass){ SetModified(); }
}

void StyleStore::OnAssCopy(wxCommandEvent& event)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	wxArrayInt selects;
	int numSelections = ASSList->GetSelections(selects);
	if (numSelections < 1){ wxBell(); return; }
	Styles *kstyle = grid->GetStyle(selects[0]);
	selnum = selects[0];
	ASSStyle = true;
	dummy = true;
	StylesWindow(_("Kopia ") + kstyle->Name);
}
void StyleStore::OnStoreCopy(wxCommandEvent& event)
{
	wxArrayInt selects;
	int numSelections = Store->GetSelections(selects);
	if (numSelections < 1){ wxBell(); return; }
	Styles *kstyle = Options.GetStyle(selects[0]);
	selnum = selects[0];
	ASSStyle = false;
	dummy = true;
	StylesWindow(_("Kopia ") + kstyle->Name);
}

void StyleStore::OnStoreNew(wxCommandEvent& event)
{
	bool gname = true;
	int count = 0;
	selnum = -1;
	ASSStyle = false;
	dummy = true;
	while (gname){
		wxString ns = _("Nowy Styl");
		wxString nss = (count == 0) ? ns : ns << count;
		if (Options.FindStyle(nss) == -1){ StylesWindow(nss); break; }
		else{ count++; }
	}
}

void StyleStore::OnAssNew(wxCommandEvent& event)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	//Styles nstyle=Styles();
	bool gname = true;
	int count = 0;
	selnum = -1;
	ASSStyle = true;
	dummy = true;

	while (gname){
		wxString ns = _("Nowy Styl");
		wxString nss = (count == 0) ? ns : ns << count;
		if (grid->FindStyle(nss) == -1){ StylesWindow(nss); break; }
		else{ count++; }
	}
}

void StyleStore::OnCleanStyles(wxCommandEvent& event)
{
	std::map<wxString, bool> lineStyles;
	wxString delStyles;
	wxString existsStyles;
	TabPanel *tab = Notebook::GetTab();
	SubsGrid *grid = tab->grid;
	const wxString &tlStyle = grid->GetSInfo(L"TLMode Style");

	for (size_t i = 0; i < grid->file->GetCount(); i++){
		lineStyles[grid->file->GetDialogue(i)->Style] = true;
	}

	size_t j = 0;
	while (j < grid->StylesSize()){
		wxString styleName = grid->GetStyle(j)->Name;
		if (lineStyles.find(styleName) != lineStyles.end()){
			existsStyles << styleName << L"\n";
		}
		else{
			if (styleName != tlStyle){
				delStyles << styleName << L"\n";
				grid->DelStyle(j);
				continue;
			}
			else{ existsStyles << styleName << L"\n"; }
		}
		j++;
	}

	if (!delStyles.empty()){
		SetModified();
	}
	if (existsStyles.IsEmpty()){ existsStyles = _("Brak"); }
	if (delStyles.IsEmpty()){ delStyles = _("Brak"); }
	wxWindow *parent = (tab->video->IsFullScreen()) ? (wxWindow*)tab->video->GetFullScreenWindow() : nullptr;
	KaiMessageBox(wxString::Format(_("Używane style:\n%s\nUsunięte style:\n%s"), existsStyles, delStyles), 
		_("Status usuniętych stylów"), 4L, parent);
}


void StyleStore::DoTooltips()
{
	catalogList->SetToolTip(_("Katalog stylów"));
	newCatalog->SetToolTip(_("Nowy katalog stylów"));
	storeNew->SetToolTip(_("Utwórz nowy styl magazynu"));
	storeEdit->SetToolTip(_("Edytuj zaznaczony styl magazynu"));
	storeCopy->SetToolTip(_("Kopiuj styl magazynu"));
	storeLoad->SetToolTip(_("Wczytaj do magazynu styl z pliku ass"));
	storeDelete->SetToolTip(_("Usuń styl z magazynu"));
	storeSort->SetToolTip(_("Sortuj style w magazynie"));
	assNew->SetToolTip(_("Utwórz nowy styl napisów"));
	assEdit->SetToolTip(_("Edytuj zaznaczony styl napisów"));
	assCopy->SetToolTip(_("Kopiuj styl napisów"));
	assLoad->SetToolTip(_("Wczytaj do napisów styl z pliku ass"));
	assDelete->SetToolTip(_("Usuń styl z napisów"));
	assSort->SetToolTip(_("Sortuj style w napisach"));
	addToStore->SetToolTip(_("Dodaj do magazynu styl z napisów"));
	addToAss->SetToolTip(_("Dodaj do napisów styl z magazynu"));
	SClean->SetToolTip(_("Oczyść napisy z nieużywanych stylów"));
}

void StyleStore::OnConfirm(wxCommandEvent& event)
{

	if (cc->IsShown()){
		cc->OnOKClick(event);
	}
	else{
		SetModified();
		OnClose(event);
	}
}

void StyleStore::OnClose(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	int ww, hh;
	GetPosition(&ww, &hh);
	Options.SetCoords(STYLE_MANAGER_POSITION, ww, hh);
	Hide();
	if (detachedEtit){ cc->Show(false); }
}

void StyleStore::SetModified(bool refreshActiveLine /*= false*/)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	Notebook::GetTab()->edit->RefreshStyle();
	grid->SetModified(STYLE_MANAGER, refreshActiveLine);
	grid->Refresh(false);
	ASSList->SetArray(grid->GetStyleTable());
}

void StyleStore::LoadAssStyles(const wxString &styleName /*= ""*/)
{
	SubsGrid* grid = Notebook::GetTab()->grid;
	if (grid->StylesSize() < 1){
		grid->AddStyle(new Styles());
	}
	ASSList->SetArray(grid->GetStyleTable());
	int wstyle = MAX(0, grid->FindStyle((styleName.empty())? 
		Notebook::GetTab()->edit->line->Style : styleName));
	ASSList->SetSelection(wstyle, true);
	if (cc->IsShown())
	{
		wxCommandEvent evt;
		OnAssStyleChange(evt);
	}
	//grid->LoadStyleCatalog();
	if (grid->subsFormat != ASS){ return; }
	const wxString &catalog = grid->GetSInfo(L"Last Style Storage");

	if (catalog.empty() || catalog == Options.actualStyleDir){ return; }
	for (size_t i = 0; i < Options.dirs.size(); i++){
		if (catalog == Options.dirs[i]){
			if (isStoreChanged){
				Options.SaveOptions(false);
				isStoreChanged = false;
			}
			Options.LoadStyles(catalog);
			Store->SetSelection(0, true);
			int chc = catalogList->FindString(Options.actualStyleDir);
			catalogList->SetSelection(chc);
		}
	}

}

void StyleStore::ReloadFonts()
{

	wxArrayString *fontList = (!Options.GetBool(STYLE_EDIT_FILTER_TEXT_ON)) ?
		FontEnum.GetFonts(0, [](){}) :
		FontEnum.GetFilteredFonts(0, [](){}, Options.GetString(STYLE_EDIT_FILTER_TEXT));
	cc->styleFont->PutArray(fontList);
	Store->Refresh(false);
	ASSList->Refresh(false);
	cc->UpdatePreview();
}

bool StyleStore::SetForegroundColour(const wxColour &col)
{
	wxWindow::SetForegroundColour(col);
	//wxWindow::Refresh();
	if (cc){ cc->SetForegroundColour(col); }
	return true;
}

bool StyleStore::SetBackgroundColour(const wxColour &col)
{
	wxWindow::SetBackgroundColour(col);
	//wxWindow::Refresh();
	if (cc){ cc->SetBackgroundColour(col); }
	return true;
}

void StyleStore::OnDetachEdit(wxCommandEvent& event)
{
	bool detach = detachEnable->GetValue();
	bool show = cc->IsShown();
	if (detachedEtit && !detach){
		cc->Destroy();
		cc = new StyleChange(this);
		if (show){ StylesWindow(); }
		else{ cc->Show(false); }
		Mainall->Add(SS->cc, 0, wxEXPAND);
		wxSize bs = wxSize(-1, cc->GetBestSize().y + 29);
		Mainall->SetMinSize(bs);
		detachedEtit = false;
	}
	else if (!detachedEtit && detach){
		Mainall->Detach(cc);
		cc->Destroy();
		cc = new StyleChange(this, false);
		//Mainall->Fit(this);
		if (show){ StylesWindow(); }
		else{ cc->Show(false); }
		detachedEtit = true;
	}
	Mainall->Fit(this);
	Options.SetBool(STYLE_MANAGER_DETACH_EDIT_WINDOW, detach);
}

StyleStore *StyleStore::SS = nullptr;

void StyleStore::ShowStore()
{
	StyleStore *SS = Get();
	bool detach = Options.GetBool(STYLE_MANAGER_DETACH_EDIT_WINDOW);
	/*if(SS->stayOnTop){
		TabPanel* pan=Notebook::GetTab();
		SS->Reparent(pan);
		SS->stayOnTop=false;
		}*/
	if (SS->detachedEtit && !detach){
		SS->cc->Destroy();
		SS->cc = new StyleChange(SS);
		SS->cc->Show(false);
		SS->Mainall->Add(SS->cc, 0, wxEXPAND);
		wxSize bs = wxSize(-1, SS->cc->GetBestSize().y + 29);
		SS->Mainall->SetMinSize(bs);
		SS->Mainall->Fit(SS);
		SS->detachedEtit = false;
	}
	else if (!SS->detachedEtit && detach){
		SS->Mainall->Detach(SS->cc);
		SS->cc->Destroy();
		SS->cc = new StyleChange(SS, false);
		SS->cc->Show(false);
		SS->Mainall->Fit(SS);
		SS->detachedEtit = true;
	}
	SS->Store->Refresh(false);
	int chc = SS->catalogList->FindString(Options.actualStyleDir);
	SS->catalogList->SetSelection(chc);
	SS->LoadAssStyles();
	if (SS->IsShown()) {
		SS->SetFocus();
	}
	else {
		SS->Show();
	}
}

void StyleStore::ShowStyleEdit(const wxString &styleName /*= ""*/)
{
	StyleStore *SS = Get();
	if (!SS->detachedEtit && ((SS->IsShown() && !SS->cc->IsShown()) || !SS->IsShown())){
		SS->Mainall->Detach(SS->cc);
		SS->cc->Destroy();
		SS->cc = new StyleChange(SS, false);
		SS->detachedEtit = true;
	}
	SS->cc->Show();
	SS->LoadAssStyles(styleName);

}

StyleStore *StyleStore::Get()
{
	if (!SS){
		int ww, hh;
		Options.GetCoords(STYLE_MANAGER_POSITION, &ww, &hh);
		SS = new StyleStore(Notebook::GetTabs()->GetParent(), wxPoint(ww, hh));
	}
	return SS;
}

void StyleStore::DestroyStore()
{
	if (SS){ delete SS; SS = nullptr; }
}

void StyleStore::OnStyleMove(wxCommandEvent& event)
{
	int action = event.GetId() - ID_ASS_MOVE_TO_START;
	wxArrayInt sels;
	int selSize = (action < 4) ? ASSList->GetSelections(sels) : Store->GetSelections(sels);
	if (selSize < 1){ wxBell(); return; }
	std::vector<Styles *> *styleTable = (action < 4) ? Notebook::GetTab()->grid->GetStyleTable() : &Options.assstore;
	int styleTableSize = styleTable->size();
	int move = (action % 4 == 0) ? -styleTableSize : (action % 4 == 1) ? -1 :
		(action % 4 == 2) ? 1 : styleTableSize;


	bool moveUp = (action % 4 == 0 || action % 4 == 1);
	int lastUpSelection = 0;
	int lastDownSelection = styleTableSize - 1;
	int i = (moveUp) ? 0 : selSize - 1;

	while ((moveUp) ? i < selSize : i >= 0){
		int sel = sels[i];
		int moveSelections = sel + move;

		if (moveUp && moveSelections < lastUpSelection){
			moveSelections = lastUpSelection;
		}
		else if (!moveUp && moveSelections > lastDownSelection){
			moveSelections = lastDownSelection;
		}
		Styles *Selected = (*styleTable)[sel];
		styleTable->erase(styleTable->begin() + sel);
		styleTable->insert(styleTable->begin() + moveSelections, Selected);
		sels[i] = moveSelections;
		if (moveUp){ i++; lastUpSelection++; }
		else{ i--; lastDownSelection--; }
	}
	if (action < 4){ 
		ASSList->SetSelections(sels); 
		Notebook::GetTab()->grid->file->edited = true; 
		SetModified(); 
	}
	else{ Store->SetSelections(sels); }
}

bool StyleStore::HaveMultiEdition()
{
	if (ASSStyle){
		wxArrayInt sels;
		int numsels = ASSList->GetSelections(sels);
		return numsels > 1;
	}

	wxArrayInt sels;
	int numsels = Store->GetSelections(sels);
	return numsels > 1;
}

