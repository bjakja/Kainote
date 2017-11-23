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

//pamiętaj ilość elementów tablicy musi być równ ilości enumów
wxString historyNames[] = {
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
	_("Zamień"),
	_("Zamień wszystko"),
	_("Skrypt automatyzacji"),
};

HistoryDialog::HistoryDialog(wxWindow *parent, SubsFile *file, std::function<void(int)> func )
	: KaiDialog(parent, -1, _("Historia"),wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
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
	MappedButton *Ok = new MappedButton(this, ID_SET_HISTORY_AND_CLOSE, _("OK"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
		Hide();
	}, ID_SET_HISTORY_AND_CLOSE);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	buttonSizer->Add(Set,1,wxALL,3);
	buttonSizer->Add(Ok,1,wxALL,3);
	buttonSizer->Add(Cancel,1,wxALL,3);
	main->Add(HistoryList,1,wxEXPAND|wxALL,3);
	main->Add(buttonSizer,0, wxCENTER);
	main->SetMinSize(300,400);
	SetSizerAndFit(main);
	CenterOnParent();
	HistoryList->SetSelection(file->Iter(),true);
}


File::File()
	:etidtionType(0)
	,activeLine(0)
{
}

File::~File()
{
	dials.clear();
	styles.clear();
	sinfo.clear();
	ddials.clear();
	dstyles.clear();
	dsinfo.clear();
	Selections.clear();
	//wxLogStatus("Clearing");
}
void File::Clear()
{
	
	for(std::vector<Dialogue*>::iterator it = ddials.begin(); it != ddials.end(); it++)
	{
		
		delete (*it);
		
	}
	
	for(std::vector<Styles*>::iterator it = dstyles.begin(); it != dstyles.end(); it++)
	{
		delete (*it);
		
	}
	
	for(std::vector<SInfo*>::iterator it = dsinfo.begin(); it != dsinfo.end(); it++)
	{
		delete (*it);
		
	}
}



File *File::Copy(bool copySelections)
{
	File *file=new File();
	file->dials = dials;
	file->styles= styles;
	file->sinfo = sinfo;
	//if (copySelections)
		file->Selections = Selections;
	file->activeLine = activeLine;
	file->markerLine = markerLine;
	file->scrollPosition = scrollPosition;
	return file;
}

SubsFile::SubsFile()
{
    iter=0;
	edited=false;
	subs = new File();
	IdConverter = new AVLtree();
}

SubsFile::~SubsFile()
{
	subs->Clear();
	delete subs;
	for(std::vector<File*>::iterator it = undo.begin(); it != undo.end(); it++)
	{
		(*it)->Clear();
		delete (*it);
	}
    undo.clear();
	delete IdConverter;
}


void SubsFile::SaveUndo(unsigned char editionType, int activeLine, int markerLine)
{
	int size=maxx();
	if(iter!=size){
		for(std::vector<File*>::iterator it = undo.begin()+iter+1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin()+iter+1, undo.end());
	}
	//subs->activeLine = activeLine;
	//subs->markerLine = markerLine;
	subs->etidtionType = editionType;
	undo.push_back(subs);
	subs=subs->Copy();
	iter++;
	edited=false;
}


void SubsFile::Redo()
{
    if(iter<maxx()){
		iter++;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
		ReloadVisibleDialogues();
	}
}

void SubsFile::Undo()
{
    if(iter>0){
		iter--;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
		ReloadVisibleDialogues();
	}
}

bool SubsFile::SetHistory(int _iter)
{
    if(_iter < undo.size() && _iter>=0){
		iter = _iter;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
		ReloadVisibleDialogues();
		return false;
	}
	return true;
}

void SubsFile::DummyUndo()
{
	subs->Clear();
	delete subs;
	subs=undo[iter]->Copy();
	ReloadVisibleDialogues();
}

void SubsFile::DummyUndo(int newIter)
{
	if(newIter < 0 || newIter >= undo.size()){return;}
	subs->Clear();
	delete subs;
	subs=undo[newIter]->Copy();
	ReloadVisibleDialogues();
	iter = newIter;
	if(iter < undo.size() - 1){
		for(std::vector<File*>::iterator it = undo.begin()+iter+1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin()+iter+1, undo.end());
	}
}

bool SubsFile::IsNotSaved()
{
    if((subs->ddials.size()==0 && subs->dstyles.size()==0 && subs->dsinfo.size()==0 && !edited)){return false;}
    return true;
}

int SubsFile::maxx()
{
    return undo.size()-1;
}

int SubsFile::Iter()
{
    return iter;
}

Dialogue *SubsFile::CopyDialogue(int i, bool push, bool keepstate)
{
	Dialogue *dial = GetDialogue(i)->Copy(keepstate, !push);
	subs->ddials.push_back(dial);
	if (push){ (*this)[i] = dial; }
	return dial;
}

Dialogue *SubsFile::CopyDialogueByKey(int i, bool push, bool keepstate)
{
	Dialogue *dial = subs->dials[i]->Copy(keepstate, !push);
	subs->ddials.push_back(dial);
	if (push){ subs->dials[i] = dial; }
	return dial;
}

Dialogue *SubsFile::GetDialogue(int i, int *key)
{
	int Key = IdConverter->getElementById(i);
	if (Key < 0){
		Key = (*IdConverter)[IdConverter->size() - 1];
		wxLogStatus("przekroczone drzewko %i, %i", i, IdConverter->size());
	}
	if (Key >= subs->dials.size()){
		wxLogStatus("tablica dialogów przekroczona %i, %i", Key, (int)subs->dials.size());
		Key = subs->dials.size() - 1;
	}
	if (key){ *key = Key; }
	return subs->dials[Key];
}

Dialogue *SubsFile::GetDialogueByKey(int Key)
{
	return subs->dials[Key];
}

Dialogue *&SubsFile::operator[](int i)
{
	int Key = IdConverter->getElementById(i);
	if (Key < 0){
		Key = (*IdConverter)[IdConverter->size() - 1];
		wxLogStatus("przekroczone drzewko %i, %i", i, IdConverter->size());
	}
	if (Key >= subs->dials.size()){
		wxLogStatus("tablica dialogów przekroczona %i, %i", Key, (int)subs->dials.size());
		Key = subs->dials.size() - 1;
	}
	return subs->dials[Key];
}

void SubsFile::DeleteDialogues(int from, int to)
{
	edited = true;
	subs->dials.erase(subs->dials.begin() + IdConverter->getElementById(from), subs->dials.begin() + IdConverter->getElementById(to));
	for (int i = from; i <= to; i++){
		IdConverter->deleteItemById(i);
	}
}

void SubsFile::DeleteDialoguesByKeys(int from, int to)
{
	edited = true;
	subs->dials.erase(subs->dials.begin() + from, subs->dials.begin() + to);
	for (int i = from; i <= to; i++){
		IdConverter->deleteItemByKey(i);
	}
}

void SubsFile::GetSelections(wxArrayInt &selections, bool deselect)
{
	selections.clear();
	for (std::set<int>::iterator i = subs->Selections.begin(); i != subs->Selections.end(); i++){
		int sel = IdConverter->getElementByKey(*i);
		if(sel >= 0) 
			selections.Add(sel);
	}
	if (deselect){ subs->Selections.clear(); }
}

void SubsFile::GetSelectionsAsKeys(wxArrayInt &selectionsKeys, bool deselect)
{
	selectionsKeys.clear();
	for (std::set<int>::iterator i = subs->Selections.begin(); i != subs->Selections.end(); i++){
		selectionsKeys.Add(*i);
	}
	if (deselect){ subs->Selections.clear(); }
}

void SubsFile::InsertSelection(int i)
{
	//insert -1 raczej nic nie zaszkodzi bo przechowujemy int
	subs->Selections.insert(IdConverter->getElementById(i));
}

void SubsFile::InsertSelections(int from, int to)
{
	int fromKey = IdConverter->getElementById(from);
	int toKey = IdConverter->getElementById(to);
	for (int i = fromKey; i < toKey; i++)
		subs->Selections.insert(i);
}

void SubsFile::InsertSelectionKey(int i)
{
	subs->Selections.insert(i);
}

void SubsFile::EraseSelection(int i)
{
	//trzeba to sprawdzić czy erase skraszuje gdy dostanie -1
	subs->Selections.erase(IdConverter->getElementById(i));
}

void SubsFile::EraseSelectionKey(int i)
{
	subs->Selections.erase(i);
}

int SubsFile::FindIdFromKey(int key, int *corrected)
{
	int Id = IdConverter->getElementByKey(key);
	if (Id < 0){
		Id = 0;
		int size = subs->dials.size();
		if (key >= size){ key = size - 1; }
		int i = key-1;
		while (i >= 0){
			if (subs->dials[i]->isVisible != NOT_VISIBLE){
				if (corrected){ *corrected = i; }
				return IdConverter->getElementByKey(i);
			}
			i--;
		}
		i = key+1;
		while (i < size){
			if (subs->dials[i]->isVisible != NOT_VISIBLE){
				if (corrected){ *corrected = i; }
				return IdConverter->getElementByKey(i);
			}
			i++;
		}
	}
	return Id;
}

bool SubsFile::IsSelectedByKey(int key)
{
	return subs->Selections.find(key) != subs->Selections.end();
}

bool SubsFile::IsSelected(int i)
{
	return subs->Selections.find(IdConverter->getElementById(i)) != subs->Selections.end();
}

int SubsFile::SelectionsSize()
{
	return subs->Selections.size();
}

void SubsFile::ClearSelections()
{
	subs->Selections.clear();
}

int SubsFile::GetElementById(int Id)
{
	return IdConverter->getElementById(Id);
}

int SubsFile::GetElementByKey(int Key)
{
	return IdConverter->getElementByKey(Key);
}
	
Styles *SubsFile::CopyStyle(int i, bool push)
{
	Styles *styl=subs->styles[i]->Copy();
	subs->dstyles.push_back(styl);
	if(push){subs->styles[i]=styl;}
	return styl;
}
	
SInfo *SubsFile::CopySinfo(int i, bool push)
{
	SInfo *sinf=subs->sinfo[i]->Copy();
	subs->dsinfo.push_back(sinf);
	if(push){subs->sinfo[i]=sinf;}
	return sinf;
}

void SubsFile::EndLoad(unsigned char editionType, int activeLine)
{
	//subs->activeLine = activeLine;
	//subs->markerLine = activeLine;
	subs->etidtionType = editionType;
	undo.push_back(subs);
	subs=subs->Copy();
}

void SubsFile::RemoveFirst(int num)
{
	//uwaga pierwszym elementem tablicy są napisy zaraz po wczytaniu dlatego też nie należy go usuwać
	for(std::vector<File*>::iterator it = undo.begin()+1; it != undo.begin()+num; it++)
	{
		(*it)->Clear();
		delete (*it);
	}
	undo.erase(undo.begin()+1, undo.begin()+num);
	iter-=(num-1);
}

void SubsFile::GetURStatus(bool *_undo, bool *_redo)
{
	*_redo= (iter<(int)undo.size()-1);
	*_undo= (iter>0);
}

File *SubsFile::GetSubs()
{
	return subs;
}

void SubsFile::ReloadVisibleDialogues(int keyfrom, int keyto)
{
	int i = keyfrom;
	int size = subs->dials.size();
	while (i <= keyto){
		bool visible = (i < size) ? subs->dials[i]->isVisible > 0 : false;
		if (visible && IdConverter->getElementByKey(i) == -1){
			IdConverter->insert(i,false);
		}
		else if (!visible && IdConverter->getElementByKey(i) != -1){
			IdConverter->deleteItemByKey(i,false);
		}
		i++;
	}
}

void SubsFile::ReloadVisibleDialogues()
{
	int i = 0;
	int size = subs->dials.size();
	int lastElem = IdConverter->getElementById(IdConverter->size() - 1);
	while (i <= lastElem || i < size){
		bool visible = (i<size)? subs->dials[i]->isVisible > 0 : false;
		if (visible && IdConverter->getElementByKey(i) == -1){
			IdConverter->insert(i, false);
		}
		else if (!visible && IdConverter->getElementByKey(i) != -1){
			IdConverter->deleteItemByKey(i,false);
		}
		i++;
	}
}

unsigned char SubsFile::CheckIfHasHiddenBlock(int i){
	int size = IdConverter->size();
	if (i + 1 < size){
		int j = i + 1;
		Dialogue * dial = GetDialogue(j);
		if (dial->isVisible == VISIBLE_BLOCK){
			if (j < 1) return 2;
			Dialogue * dialPrev = GetDialogue(j - 1);
			if (dialPrev->isVisible != VISIBLE_BLOCK) return 2;
			return 0;
		}
	}
	if (i >= size){ return 0; }
	int keyFirst = (i < 0) ? -1 : IdConverter->getElementById(i);
	int keySecond = IdConverter->getElementById(i + 1);
	if ((keyFirst + 1) != keySecond){
		int j = keyFirst + 1;
		if (keySecond < 0){ keySecond = subs->dials.size(); }
		while (j < keySecond){
			if (!subs->dials[j]->NonDialogue){ return 1; }
			j++;
		}
	}
	
	return 0;
}

void SubsFile::GetHistoryTable(wxArrayString *history)
{
	for(size_t i = 0; i < undo.size(); i++){
		history->push_back(historyNames[undo[i]->etidtionType] + 
			wxString::Format(_(", aktywna linia %i"), GetElementByKey(undo[i]->activeLine) + 1));
	}
}

void SubsFile::ShowHistory(wxWindow *parent, std::function<void(int)> functionAfterChangeHistory)
{
	HistoryDialog HD(parent, this, functionAfterChangeHistory);
	HD.ShowModal();
}