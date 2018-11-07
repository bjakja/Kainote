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


#include "SubsFile.h"
#include "KaiListCtrl.h"
#include "MappedButton.h"

//pamiętaj ilość elementów tablicy musi być równa ilości enumów
//wxString 

HistoryDialog::HistoryDialog(wxWindow *parent, SubsFile *file, std::function<void(int)> func)
	: KaiDialog(parent, -1, _("Historia"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	wxArrayString history;
	file->GetHistoryTable(&history);
	KaiListCtrl *HistoryList = new KaiListCtrl(this, ID_HISTORY_LIST, history);
	//HistoryList->ScrollTo(file->Iter()-2);
	Bind(LIST_ITEM_DOUBLECLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
	}, ID_HISTORY_LIST);
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *Set = new MappedButton(this, ID_SET_HISTORY, _("Ustaw"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
	}, ID_SET_HISTORY);
	MappedButton *Ok = new MappedButton(this, ID_SET_HISTORY_AND_CLOSE, "OK");
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
		Hide();
	}, ID_SET_HISTORY_AND_CLOSE);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	buttonSizer->Add(Set, 1, wxALL, 3);
	buttonSizer->Add(Ok, 1, wxALL, 3);
	buttonSizer->Add(Cancel, 1, wxALL, 3);
	main->Add(HistoryList, 1, wxEXPAND | wxALL, 3);
	main->Add(buttonSizer, 0, wxCENTER);
	main->SetMinSize(300, 400);
	SetSizerAndFit(main);
	CenterOnParent();
	HistoryList->SetSelection(file->Iter(), true);
	SetLabel(_("Historia") + " (" +
		MakePolishPlural(history.GetCount(), _("element"), _("elementy"), _("elementów")) + ")");
}


File::File()
	:editionType(0)
	, activeLine(0)
{
}

File::~File()
{
	dialogues.clear();
	styles.clear();
	sinfo.clear();
	deleteDialogues.clear();
	deleteStyles.clear();
	deleteSinfo.clear();
	Selections.clear();
}
void File::Clear()
{

	for (std::vector<Dialogue*>::iterator it = deleteDialogues.begin(); it != deleteDialogues.end(); it++)
	{
		delete (*it);
	}

	for (std::vector<Styles*>::iterator it = deleteStyles.begin(); it != deleteStyles.end(); it++)
	{
		delete (*it);
	}

	for (std::vector<SInfo*>::iterator it = deleteSinfo.begin(); it != deleteSinfo.end(); it++)
	{
		delete (*it);
	}
}



File *File::Copy(bool copySelections)
{
	File *file = new File();
	file->dialogues = dialogues;
	file->styles = styles;
	file->sinfo = sinfo;
	file->Selections = Selections;
	file->activeLine = activeLine;
	file->markerLine = markerLine;
	file->scrollPosition = scrollPosition;
	return file;
}

SubsFile::SubsFile()
{
	historyNames = new wxString[AUTOMATION_SCRIPT + 1]{
		"",//pierwszy element którego nie używamy a musi być ostatni enum weszedł a także ochronić nas przed potencjalnym 0
			_("Otwarcie napisów"),
			_("Nowe napisy"),
			_("Edycja linii"),
			_("Edycja wielu linii"),
			_("Poprawa błędu pisowni w polu tekstowym"),
			_("Powielenie linijek"),
			_("Połączenie linijek"),
			_("Połączenie linijki z poprzednią"),
			_("Połączenie linijki z następną"),
			/*10*/_("Połączenie linijek pozostawienie pierszej"),
			_("Połączenie linijek pozostawienie ostatniej"),
			_("Wklejenie linijek"),
			_("Wklejenie kolumn"),
			_("Wklejenie tłumaczenia"),
			_("Przesunięcie tekstu tłumaczenia"),
			_("Ustawienie czasów linii jako ciągłych"),
			_("Ustawienie FPSu obliczonego z wideo"),
			_("Ustawienie własnego FPSu"),
			_("Zamiana linijek"),
			/*20*/_("Konwersja napisów"),
			_("Sortowanie napisów"),
			_("Usunięcie linijek"),
			_("Usunięcie tekstu"),
			_("Ustawienie czasu początkowego"),
			_("Ustawienie czasu końcowego"),
			_("Włączenie trybu tłumaczenia"),
			_("Wyłączenie trybu tłumaczenia"),
			_("Dodanie nowej linii"),
			_("Wstawienie linii"),
			/*30*/_("Zmiana czasu na wykresie audio"),
			_("Przyklejenie do klatki kluczowej"),
			_("Zmiana nagłówku napisów"),
			_("Akcja zaznacz linijki"),
			_("Przesunięcie czasów"),
			_("Poprawa błędu pisowni"),
			_("Edycja stylów"),
			_("Zmiana rozdzielczości napisów"),
			_("Narzędzie pozycjonowania"),
			_("Narzędzie ruchu"),
			/*40*/_("Narzędzie skalowania"),
			_("Narzędzie obrotów w osi Z"),
			_("Narzędzie obrotów w osiach X i Y"),
			_("Narzędzie wycinów prostokątnych"),
			_("Narzędzie wycinów wektorowych"),
			_("Narzędzie rysunków wektorowych"),
			_("Narzędzie zmieniacz pozycji"),
			_("Narzędzie zmieniacz skali i obrotów"),
			_("Zamień"),
			_("Zamień wszystko"),
			/*50*/_("Poprawa drobnych błędów"),
			_("Wstawienie drzewka"),
			_("Ustawienie opisu"),
			_("Dodanie linii do drzewka"),
			_("Usunięcie drzewka"),
			_("Skrypt automatyzacji")
	};
	iter = 0;
	edited = false;
	subs = new File();
}

SubsFile::~SubsFile()
{
	subs->Clear();
	delete subs;
	for (std::vector<File*>::iterator it = undo.begin(); it != undo.end(); it++)
	{
		(*it)->Clear();
		delete (*it);
	}
	undo.clear();
	delete[] historyNames;
}


void SubsFile::SaveUndo(unsigned char editionType, int activeLine, int markerLine)
{
	if (iter != maxx()){
		for (std::vector<File*>::iterator it = undo.begin() + iter + 1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin() + iter + 1, undo.end());
		if (lastSave >= undo.size()){ lastSave = -1; }
	}
	subs->activeLine = activeLine;
	//subs->markerLine = markerLine;
	subs->editionType = editionType;
	undo.push_back(subs);
	subs = subs->Copy();
	iter++;
	edited = false;
}


bool SubsFile::Redo()
{
	if (iter < maxx()){
		iter++;
		subs->Clear();
		delete subs;
		subs = undo[iter]->Copy();
		return false;
	}
	return true;
}

bool SubsFile::Undo()
{
	if (iter > 0){
		iter--;
		subs->Clear();
		delete subs;
		subs = undo[iter]->Copy();
		return false;
	}
	return true;
}

bool SubsFile::SetHistory(int _iter)
{
	if (_iter < undo.size() && _iter >= 0){
		iter = _iter;
		subs->Clear();
		delete subs;
		subs = undo[iter]->Copy();
		return false;
	}
	return true;
}

void SubsFile::DummyUndo()
{
	subs->Clear();
	delete subs;
	subs = undo[iter]->Copy();
}

void SubsFile::DummyUndo(int newIter)
{
	if (newIter < 0 || newIter >= undo.size()){ return; }
	subs->Clear();
	delete subs;
	subs = undo[newIter]->Copy();
	iter = newIter;
	if (iter < undo.size() - 1){
		for (std::vector<File*>::iterator it = undo.begin() + iter + 1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin() + iter + 1, undo.end());
	}
}

bool SubsFile::IsNotSaved()
{
	if ((subs->deleteDialogues.size() == 0 && subs->deleteStyles.size() == 0 && subs->deleteSinfo.size() == 0 && !edited)){ return false; }
	return true;
}

int SubsFile::maxx()
{
	return undo.size() - 1;
}

int SubsFile::Iter()
{
	return iter;
}

//size_t SubsFile::GetCount()
//{
//	return subs->dialogues.size();
//}

size_t SubsFile::GetCount()
{
	return subs->dialogues.size();
}

void SubsFile::AppendDialogue(Dialogue *dial)
{
	subs->deleteDialogues.push_back(dial);
	subs->dialogues.push_back(dial);
}

//Dialogue * SubsFile::CopyDialogue(size_t i, bool push/*=true*/, bool keepstate/*=false*/)
//{
//	Dialogue *dial = subs->dialogues[i]->Copy(keepstate, !push);
//	subs->deleteDialogues.push_back(dial);
//	if (push){
//		subs->dialogues[i] = dial;
//	}
//	return dial;
//}

Dialogue * SubsFile::CopyDialogue(size_t i, bool push /*= true*/, bool keepstate /*= false*/)
{
	Dialogue *dial = subs->dialogues[i]->Copy(keepstate, !push);
	subs->deleteDialogues.push_back(dial);
	if (push){ 
		subs->dialogues[i] = dial;
	}
	return dial;
}

Dialogue *SubsFile::GetDialogue(size_t Key)
{
	return subs->dialogues[Key];
}

void SubsFile::SetDialogue(size_t i, Dialogue *dial)
{
	if (i >= subs->dialogues.size()){
		subs->dialogues.push_back(dial);
	}

	subs->dialogues[i] = dial;
}

void SubsFile::DeleteDialogues(size_t from, size_t to)
{
	edited = true;
	if (from >= subs->dialogues.size())
		return;
	else if (to >= subs->dialogues.size())
		to = subs->dialogues.size();

	subs->dialogues.erase(subs->dialogues.begin() + from, subs->dialogues.begin() + to);
}


void SubsFile::DeleteSelectedDialogues()
{
	for (auto i = subs->Selections.rbegin(); i != subs->Selections.rend(); i++)
	{
		subs->dialogues.erase(subs->dialogues.begin() + (*i));
	}
	if (subs->Selections.size() > 0){ 
		edited = true; 
	}
}

void SubsFile::SortAll(bool func(Dialogue *i, Dialogue *j))
{
	std::sort(subs->dialogues.begin(), subs->dialogues.end(), func);
}

void SubsFile::SortSelected(bool func(Dialogue *i, Dialogue *j))
{
	std::vector<Dialogue*> selected;
	for (auto cur = subs->Selections.begin(); cur != subs->Selections.end(); cur++){
		Dialogue *dial = subs->dialogues[*cur];
		dial->ChangeDialogueState(1);
		selected.push_back(dial);
	}
	std::sort(selected.begin(), selected.end(), func);
	int ii = 0;
	for (auto cur = subs->Selections.begin(); cur != subs->Selections.end(); cur++){
		subs->dialogues[*cur] = selected[ii++];
	}
	selected.clear();
}

void SubsFile::GetSelections(wxArrayInt &selections, bool deselect)
{
	selections.clear();
	for (std::set<int>::iterator i = subs->Selections.begin(); i != subs->Selections.end(); i++){
		int sel = (*i);
		if(*subs->dialogues[sel]->isVisible)
			selections.Add(sel);
	}
	if (deselect){ subs->Selections.clear(); }
}

void SubsFile::InsertSelection(size_t i)
{
	subs->Selections.insert(i);
}

void SubsFile::InsertSelections(size_t from, size_t to, bool deselect /*= false*/)
{
	if (deselect){ subs->Selections.clear(); }
	size_t dialsize = subs->dialogues.size();
	if (from >= dialsize){ return; }
	if (to >= dialsize){ to = dialsize - 1; }
	for (size_t i = from; i <= to; i++){
		if (*subs->dialogues[i]->isVisible){
			subs->Selections.insert(i);
		}
	}
}


void SubsFile::EraseSelection(size_t i)
{
	subs->Selections.erase(i);
}

//size_t SubsFile::FindIdFromKey(size_t key, int *corrected)
//{
//	size_t Id = GetElementByKey(key);
//	if (Id == -1){
//		Id = 0;
//		size_t size = subs->dialogues.size();
//		if (key >= size){ key = size - 1; }
//		size_t i = key - 1;
//		while (i >= 0){
//			if (subs->dialogues[i]->isVisible != NOT_VISIBLE){
//				if (corrected){ *corrected = i; }
//				return GetElementByKey(i);
//			}
//			i--;
//		}
//		i = key + 1;
//		while (i < size){
//			if (subs->dialogues[i]->isVisible != NOT_VISIBLE){
//				if (corrected){ *corrected = i; }
//				return GetElementByKey(i);
//			}
//			i++;
//		}
//	}
//	return Id;
//}

bool SubsFile::IsSelected(size_t i)
{
	return subs->Selections.find(i) != subs->Selections.end();
}

size_t SubsFile::SelectionsSize()
{
	return subs->Selections.size();
}

void SubsFile::ClearSelections()
{
	subs->Selections.clear();
}

//size_t SubsFile::GetElementById(size_t id)
//{
//	if (id >= filtered.size())
//		return -1;
//
//	Dialogue * row = filtered[id];
//	for (size_t i = id; i < subs->dialogues.size(); i++){
//		if (row == subs->dialogues[i]){
//			return i;
//		}
//	}
//
//	// it should not happen but with bugs it's possible
//	return -1;
//}
//
//size_t SubsFile::GetElementByKey(size_t key)
//{
//	if (key >= subs->dialogues.size() || key < 0)
//		return -1;
//
//	Dialogue * row = subs->dialogues[key];
//	if (row->isVisible == NOT_VISIBLE)
//		return -1;
//
//	size_t i = (key < filtered.size()) ? key : filtered.size() - 1;
//	while (i + 1 > 0){
//		if (row == filtered[i])
//			return i;
//
//		i--;
//	}
//	return -1;
//}

Styles *SubsFile::CopyStyle(size_t i, bool push)
{
	Styles *styl = subs->styles[i]->Copy();
	subs->deleteStyles.push_back(styl);
	if (push){
		subs->styles[i] = styl;
	}
	return styl;
}

SInfo *SubsFile::CopySinfo(size_t i, bool push)
{
	SInfo *sinf = subs->sinfo[i]->Copy();
	subs->deleteSinfo.push_back(sinf);
	if (push){
		subs->sinfo[i] = sinf;
	}
	return sinf;
}

void SubsFile::EndLoad(unsigned char editionType, int activeLine, bool initialSave)
{
	//subs->activeLine = activeLine;
	//subs->markerLine = activeLine;
	if (initialSave)
		lastSave = -1;

	subs->editionType = editionType;
	undo.push_back(subs);
	subs = subs->Copy();
}

void SubsFile::RemoveFirst(int num)
{
	//uwaga pierwszym elementem tablicy są napisy zaraz po wczytaniu dlatego też nie należy go usuwać
	for (std::vector<File*>::iterator it = undo.begin() + 1; it != undo.begin() + num; it++)
	{
		(*it)->Clear();
		delete (*it);
	}
	undo.erase(undo.begin() + 1, undo.begin() + num);
	if (lastSave > iter){ lastSave -= (num - 1); }
	iter -= (num - 1);
}

void SubsFile::GetURStatus(bool *_undo, bool *_redo)
{
	*_redo = (iter < (int)undo.size() - 1);
	*_undo = (iter > 0);
}

//File *SubsFile::GetSubs()
//{
//	return subs;
//}

//void SubsFile::ReloadVisibleDialogues(size_t keyfrom, size_t keyto)
//{
//	int i = keyfrom;
//	int size = subs->dialogues.size();
//	if (keyfrom >= size)
//		return;
//	if (keyto >= size)
//		keyto = size - 1;
//
//	while (i <= keyto){
//		filtered.push_back(subs->dialogues[i]);
//		i++;
//	}
//}
//
//void SubsFile::ReloadVisibleDialogues()
//{
//	for (int i = 0; i < subs->dialogues.size(); i++){
//		filtered.push_back(subs->dialogues[i]);
//	}
//}

unsigned char SubsFile::CheckIfHasHiddenBlock(size_t i){
	int size = subs->dialogues.size();
	if (i + 1 < size){
		int j = i + 1;
		Dialogue * dial = subs->dialogues[j];
		if (dial->isVisible == VISIBLE_BLOCK){
			Dialogue * dialPrev = subs->dialogues[j - 1];
			if (dialPrev->isVisible != VISIBLE_BLOCK) return 2;
			return 0;
		}
	}
	if (i >= size){ return 0; }
	size_t keyFirst = i + 1;
	size_t numOfLines = 0;
	while (keyFirst < subs->dialogues.size()){
		if (*subs->dialogues[keyFirst]->isVisible){ 
			if (numOfLines)
				return 1;
			else
				return 0;
		}
		keyFirst++;

		if (!subs->dialogues[keyFirst]->NonDialogue)
			numOfLines++;
	}

	return 0;
}

size_t SubsFile::GetKeyFromScrollPos(size_t numOfLines)
{

}

size_t SubsFile::GetKeyFromPos(size_t position, size_t numOfLines)
{

}

bool SubsFile::CheckIfIsTree(size_t i){
	Dialogue *dial = subs->dialogues[i];
	return dial->treeState == TREE_DESCRIPTION;
}

int SubsFile::OpenCloseTree(size_t i){
	size_t endOfTree = -1;
	int visibility = NOT_VISIBLE;
	for (size_t k = i + 1; k < subs->dialogues.size(); k++){
		Dialogue *dial = subs->dialogues[k];
		if (dial->treeState < TREE_CLOSED){
			endOfTree = k - 1;
			break;
		}
		if (dial->treeState == TREE_CLOSED){
			dial->isVisible = visibility = VISIBLE;
			dial->treeState = TREE_OPENED;
		}
		else{
			dial->isVisible = visibility;
			dial->treeState = TREE_CLOSED;
		}
	}
	if (endOfTree < 0){
		endOfTree = subs->dialogues.size() - 1;
	}
	if (i + 1 < endOfTree){
		int diff = endOfTree - (i + 1);
		return (visibility) ? diff : -diff;
	}
	return 0;
}

void SubsFile::GetHistoryTable(wxArrayString *history)
{
	for (size_t i = 0; i < undo.size(); i++){
		history->push_back(historyNames[undo[i]->editionType] +
			wxString::Format(_(", aktywna linia %i"), GetElementByKey(undo[i]->activeLine) + 1));
	}
}

void SubsFile::ShowHistory(wxWindow *parent, std::function<void(int)> functionAfterChangeHistory)
{
	HistoryDialog HD(parent, this, functionAfterChangeHistory);
	HD.ShowModal();
}

void SubsFile::SetLastSave()
{
	lastSave = iter;
}

int SubsFile::GetActualHistoryIter()
{
	if (lastSave < 0)
		return iter;
	return iter - lastSave;
}

const wxString & SubsFile::GetUndoName()
{
	if (iter < 1)
		return emptyString;

	return historyNames[undo[iter - 1]->editionType];
}

const wxString & SubsFile::GetRedoName()
{
	if (iter >= maxx())
		return emptyString;

	return historyNames[undo[iter + 1]->editionType];
}

void SubsFile::AddStyle(Styles *nstyl)
{
	subs->deleteStyles.push_back(nstyl);
	subs->styles.push_back(nstyl);
}

void SubsFile::ChangeStyle(Styles *nstyl, size_t i)
{
	subs->deleteStyles.push_back(nstyl);
	subs->styles[i] = nstyl;
}

size_t SubsFile::StylesSize()
{
	return subs->styles.size();
}

Styles *SubsFile::GetStyle(size_t i, const wxString &name/* = L""*/)
{
	if (name != L""){
		for (size_t j = 0; j < subs->styles.size(); j++)
		{
			if (name == subs->styles[j]->Name){ return subs->styles[j]; }
		}
	}
	return subs->styles[i];
}

std::vector<Styles*> *SubsFile::GetStyleTable()
{
	return &subs->styles;
}

//multiplication musi być ustawione na zero, wtedy zwróci ilość multiplikacji
size_t SubsFile::FindStyle(const wxString &name, int *multip)
{
	size_t isfound = -1;
	for (size_t j = 0; j < subs->styles.size(); j++)
	{
		if (name == subs->styles[j]->Name){
			isfound = j;
			if (multip){
				*multip++;
			}
			else{ break; }
		}
	}
	return isfound;
}

void SubsFile::GetStyles(wxString &stylesText, bool tld/* = false*/)
{
	wxString tmpst;
	if (tld){ tmpst = GetSInfo(L"TLMode Style"); }
	for (size_t i = 0; i < subs->styles.size(); i++)
	{
		if (!(tld && subs->styles[i]->Name == tmpst)){
			stylesText << subs->styles[i]->GetRaw();
		}
	}
}

void SubsFile::DeleleStyle(size_t i)
{
	edited = true;
	subs->styles.erase(subs->styles.begin() + i);
}

const wxString & SubsFile::GetSInfo(const wxString &key, int *ii/* = 0*/)
{
	int i = 0;
	for (std::vector<SInfo*>::iterator it = subs->sinfo.begin(); it != subs->sinfo.end(); it++)
	{
		if (key == (*it)->Name) { if (ii){ *ii = i; } return (*it)->Val; }
		i++;
	}
	return emptyString;
}

SInfo *SubsFile::GetSInfoP(const wxString &key, int *ii)
{
	int i = 0;
	for (std::vector<SInfo*>::iterator it = subs->sinfo.begin(); it != subs->sinfo.end(); it++)
	{
		if (key == (*it)->Name) { if (ii){ *ii = i; }; return (*it); }
		i++;
	}
	*ii = -1;
	return NULL;
}

void SubsFile::DeleteSInfo(size_t i)
{
	subs->sinfo.erase(subs->sinfo.begin() + i);
	edited = true;
}

size_t SubsFile::SInfoSize()
{
	return subs->sinfo.size();
}

void SubsFile::SaveSelections(bool clear, int currentLine, int markedLine, int scrollPos)
{
	undo[iter]->Selections = subs->Selections;
	//tutaj muszą być przeróbki na klucze
	undo[iter]->activeLine =currentLine;
	undo[iter]->markerLine = markedLine;
	undo[iter]->scrollPosition = scrollPos;
	if (clear){ ClearSelections(); }
}

int SubsFile::FirstSelection()
{
	if (!subs->Selections.empty()){
		// return only visible element when nothing is visible, return -1;
		for (auto it = subs->Selections.begin(); it != subs->Selections.end(); it++){
			int sel = (*it);
			if (*subs->dialogues[sel]->isVisible)
				return sel;
		}
	}
	return -1;
}

void SubsFile::InsertRows(int Row,
	const std::vector<Dialogue *> &RowsTable,
	bool AddToDestroy, bool asKey)
{
	size_t convertedRow = Row;
	if (convertedRow >= subs->dialogues.size()){ convertedRow = subs->dialogues.size(); }
	subs->dialogues.insert(subs->dialogues.begin() + convertedRow, RowsTable.begin(), RowsTable.end());
	if (AddToDestroy){ subs->deleteDialogues.insert(subs->deleteDialogues.end(), RowsTable.begin(), RowsTable.end()); }
}

//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsFile::InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy, bool Save, bool asKey)
{
	size_t convertedRow = Row;
	if (convertedRow >= subs->dialogues.size()){ convertedRow = subs->dialogues.size(); }
	subs->dialogues.insert(subs->dialogues.begin() + convertedRow, NumRows, Dialog);
	if (AddToDestroy){ subs->deleteDialogues.push_back(Dialog); }
}

void SubsFile::SwapRows(int frst, int scnd)
{

	Dialogue *tmp = subs->dialogues[frst];
	subs->dialogues[frst] = subs->dialogues[scnd];
	subs->dialogues[scnd] = tmp;

}

void SubsFile::AddSInfo(const wxString &SI, wxString val, bool save)
{
	wxString key;
	if (val == L""){
		key = SI.BeforeFirst(L':');
		key.Trim(false);
		key.Trim(true);
		val = SI.AfterFirst(L':');
		val.Trim(false);
		val.Trim(true);
	}
	else{ key = SI; }
	SInfo *oldinfo = NULL;
	int ii = -1;
	oldinfo = GetSInfoP(key, &ii);

	if (!oldinfo || save){
		oldinfo = new SInfo(key, val);
		if (ii < 0){
			subs->sinfo.push_back(oldinfo);
		}
		else{
			subs->sinfo[ii] = oldinfo;
		}
		subs->deleteSinfo.push_back(oldinfo);
	}
	else{
		oldinfo->Val = val;
	}
}

void SubsFile::GetSInfos(wxString &textSinfo, bool tld/* = false*/)
{
	for (std::vector<SInfo*>::iterator cur = subs->sinfo.begin(); cur != subs->sinfo.end(); cur++) {
		if (!(tld && (*cur)->Name.StartsWith(L"TLMode"))){
			textSinfo << (*cur)->Name << L": " << (*cur)->Val << L"\r\n";
		}
	}
}