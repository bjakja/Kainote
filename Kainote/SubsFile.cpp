//  Copyright (c) 2016 - 2026, Marcin Drob

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
#include "config.h"
#include <unordered_set>


HistoryDialog::HistoryDialog(wxWindow *parent, SubsFile *file, std::function<void(int)> func)
	: KaiDialog(parent, -1, _("History"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
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
	MappedButton *Set = new MappedButton(this, ID_SET_HISTORY, _("Set"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
	}, ID_SET_HISTORY);
	MappedButton *Ok = new MappedButton(this, ID_SET_HISTORY_AND_CLOSE, L"OK");
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=, this](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
		Hide();
	}, ID_SET_HISTORY_AND_CLOSE);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Cancel"));
	buttonSizer->Add(Set, 1, wxALL, 3);
	buttonSizer->Add(Ok, 1, wxALL, 3);
	buttonSizer->Add(Cancel, 1, wxALL, 3);
	main->Add(HistoryList, 1, wxEXPAND | wxALL, 3);
	main->Add(buttonSizer, 0, wxCENTER);
	main->SetMinSize(300, 400);
	SetSizerAndFit(main);
	CenterOnParent();
	HistoryList->SetSelection(file->Iter(), true);
	SetLabel(_("History") + L" (" +
		wxString::Format(wxPLURAL("%d element", "%d elements", history.GetCount()),
			history.GetCount()) + L")");
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
	file->isFiltered = isFiltered;
	return file;
}

static void DestroySnapshot(File *snapshot)
{
	snapshot->Clear();
	delete snapshot;
}

static const int MAX_UNDO_STEPS = 500;

template <typename T>
static void HandOverUsed(std::vector<T*> &owned, std::vector<T*> &to, const std::vector<T*> &used)
{
	if (owned.empty())
		return;
	std::unordered_set<T*> pending(owned.begin(), owned.end());
	for (T *item : used) {
		if (pending.erase(item))
			to.push_back(item);
	}
	owned.assign(pending.begin(), pending.end());
}

// what snapshot created and kept still shows becomes kept's to free
static void HandOverUsed(File *snapshot, File *kept)
{
	HandOverUsed(snapshot->deleteDialogues, kept->deleteDialogues, kept->dialogues);
	HandOverUsed(snapshot->deleteStyles, kept->deleteStyles, kept->styles);
	HandOverUsed(snapshot->deleteSinfo, kept->deleteSinfo, kept->sinfo);
}

// later steps can still show what a dropped step created
static void MergeOwned(File *dropped, File *kept)
{
	auto move = [](auto &from, auto &to) {
		to.insert(to.end(), from.begin(), from.end());
		from.clear();
	};
	move(dropped->deleteDialogues, kept->deleteDialogues);
	move(dropped->deleteStyles, kept->deleteStyles);
	move(dropped->deleteSinfo, kept->deleteSinfo);
}

SubsFile::SubsFile()
	: m_history(DestroySnapshot)
{
	Create();
}

SubsFile::~SubsFile()
{
	Clear(false);
}

void SubsFile::AddEmbeddedSectionLine(const wxString &line)
{
	embeddedSections << line << L"\r\n";
}

void SubsFile::Clear(bool setup/* = true*/)
{
	embeddedSections.clear();
	if (subs) {
		subs->Clear();
		delete subs;
		subs = nullptr;
		m_history.Clear();
		delete[] historyNames;
	}
	if (setup) {
		Create();
	}
}

void SubsFile::SetMutex(wxMutex* editionGuard)
{
	historyGuard = editionGuard;
}

void SubsFile::Create()
{
	historyNames = new wxString[GRID_SPLIT_LINES + 1]{
		//first element is not used but is to secure it from number 0
		emptyString,
			_("Opening subtitles"),
			_("New subtitles"),
			_("Line editing"),
			_("Editing multiple lines"),
			_("Correcting spelling errors in the text field"),
			_("Duplicating lines"),
			_("Joining lines"),
			_("Joining line with the previous line"),
			_("Joining line with the next line"),
			/*10*/_("Joining lines and keeping the first"),
			_("Joining lines and keeping the last"),
			_("Pasting lines"),
			_("Pasting columns"),
			_("Pasting translation"),
			_("Moving translation text"),
			_("Setting line times as continuous"),
			_("Setting FPS from video"),
			_("Setting custom FPS"),
			_("Swapping lines"),
			/*20*/_("Subtitles conversion"),
			_("Sorting subtitles"),
			_("Deleting lines"),
			_("Deleting text"),
			_("Setting start time"),
			_("Setting end time"),
			_("Turning on translator mode"),
			_("Turning off translator mode"),
			_("Adding a new line"),
			_("Inserting line"),
			/*30*/_("Changing time on audio spectrum"),
			_("Snapping to keyframe"),
			_("Changing the subtitle header"),
			_("Selecting lines"),
			_("Shifting times"),
			_("Correcting spelling errors"),
			_("Style editing"),
			_("Changing subtitles resolution"),
			_("Visual positioning tool"),
			_("Visual movement tool"),
			/*40*/_("Visual scaling tool"),
			_("Visual Z-axis rotation tool"),
			_("Visual X/Y-axis rotation tool"),
			_("Visual rectangular clipping tool"),
			_("Visual vector clipping tool"),
			_("Visual vector drawing tool"),
			_("Visual position adjustment tool"),
			_("Visual scale and rotation adjustment tool"),
			_("Visual Hydra tool"),
			_("Replace"),
			_("Replace all"),
			/*50*/_("Fixing minor errors"),
			_("Adding tree"),
			_("Setting tree description"),
			_("Adding line to tree"),
			_("Removing tree"),
			_("Automation script"),
			_("Filtering"),
			_("Removing filtering"),
			_("Splitting lines")
	};
	edited = false;
	++version;
	subs = new File();
}


void SubsFile::SaveUndo(unsigned char editionType, int activeLine, int markerLine)
{
	wxMutexLocker lock(*historyGuard);
	subs->activeLine = activeLine;
	//subs->markerLine = markerLine;
	subs->editionType = editionType;
	File *last = m_history.Current();
	bool amend = typing && lastStepTyping && editionType == EDITBOX_LINE_EDITION && last &&
		last->editionType == EDITBOX_LINE_EDITION && last->activeLine == activeLine && m_history.CanAmend();
	lastStepTyping = typing && editionType == EDITBOX_LINE_EDITION;
	if (amend) {
		HandOverUsed(last, subs);
		DestroySnapshot(m_history.SwapCurrent(subs));
	}
	else {
		m_history.Record(subs);
		if (m_history.Size() > MAX_UNDO_STEPS)
			m_history.DropOldest(m_history.Size() - MAX_UNDO_STEPS + 1, MergeOwned);
	}
	subs = subs->Copy();
	edited = false;
	++version;
}

//the working copy becomes a copy of the current step; call it locked
void SubsFile::LoadCurrentStep()
{
	subs->Clear();
	delete subs;
	subs = m_history.Current()->Copy();
	edited = false;
	++version;
	lastStepTyping = false;
}

bool SubsFile::Redo()
{
	if (!m_history.CanRedo())
		return true;
	wxMutexLocker lock(*historyGuard);
	m_history.Redo();
	LoadCurrentStep();
	return false;
}

bool SubsFile::Undo()
{
	if (!m_history.CanUndo())
		return true;
	wxMutexLocker lock(*historyGuard);
	m_history.Undo();
	LoadCurrentStep();
	return false;
}

bool SubsFile::SetHistory(int step)
{
	if (!m_history.At(step))
		return true;
	wxMutexLocker lock(*historyGuard);
	m_history.GoTo(step);
	LoadCurrentStep();
	return false;
}

void SubsFile::DummyUndoF()
{
	wxMutexLocker lock(*historyGuard);
	LoadCurrentStep();
}

void SubsFile::DummyUndoF(int newIter)
{
	if (!m_history.At(newIter)){ return; }
	wxMutexLocker lock(*historyGuard);
	m_history.Rewind(newIter);
	LoadCurrentStep();
}

int SubsFile::Iter()
{
	return m_history.Step();
}

//size_t SubsFile::GetCount()
//{
//	return subs->dialogues.size();
//}

size_t SubsFile::GetCount()
{
	return subs->dialogues.size();
}

const SubsFile::VisibleRows &SubsFile::GetVisibleRows()
{
	VisibleRows &rows = visibleRows;
	unsigned epoch = Visibility::Epoch();
	if (rows.version == version && rows.epoch == epoch && rows.file == subs)
		return rows;
	rows.version = version;
	rows.epoch = epoch;
	rows.file = subs;
	size_t count = subs->dialogues.size();
	rows.keyOfId.clear();
	rows.idOfKey.resize(count);
	for (size_t i = 0; i < count; i++) {
		rows.idOfKey[i] = rows.keyOfId.size();
		if (subs->dialogues[i]->isVisible)
			rows.keyOfId.push_back(i);
	}
	return rows;
}

size_t SubsFile::GetIdCount()
{
	return GetVisibleRows().keyOfId.size();
}

size_t SubsFile::CountLines(size_t key, size_t *dialogueNumber)
{
	size_t count = subs->dialogues.size();
	size_t dialogues = 0;
	if (key < count) {
		for (size_t i = 0; i < key; i++) {
			if (!subs->dialogues[i]->NonDialogue)
				dialogues++;
		}
	}
	*dialogueNumber = (key >= count) ? count - 1 : dialogues;
	return GetIdCount();
}

void SubsFile::AddLine(Dialogue *dial)
{
	MarkEdited();
	subs->deleteDialogues.push_back(dial);
	subs->dialogues.push_back(dial);
}

Dialogue * SubsFile::CopyVisibleDialogue(size_t i, bool push /*= true*/, bool keepstate/*=false*/)
{
	if (i >= subs->dialogues.size() || !subs->dialogues[i]->isVisible)
		return nullptr;

	return CopyDialogueF(i, push, keepstate);
}

Dialogue * SubsFile::CopyDialogueF(size_t i, bool push /*= true*/, bool keepstate /*= false*/)
{
	Dialogue *dial = subs->dialogues[i]->Copy(keepstate, !push);
	subs->deleteDialogues.push_back(dial);
	MarkEdited();
	if (push){ 
		subs->dialogues[i] = dial;
	}
	return dial;
}

Dialogue * SubsFile::GetVisibleDialogue(size_t i)
{
	if (i >= subs->dialogues.size() || !subs->dialogues[i]->isVisible)
		return nullptr;

	return subs->dialogues[i];
}

Dialogue *SubsFile::GetDialogue(size_t i)
{
	if (i >= subs->dialogues.size())
		return nullptr;

	return subs->dialogues[i];
}

void SubsFile::SetDialogue(size_t i, Dialogue *dial, bool addToDestroyer)
{
	MarkEdited();
	if (i >= subs->dialogues.size())
		subs->dialogues.push_back(dial);
	else
		subs->dialogues[i] = dial;

	if (addToDestroyer)
		subs->deleteDialogues.push_back(dial);
}

void SubsFile::DeleteDialogues(size_t from, size_t to)
{
	MarkEdited();
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
		MarkEdited(); 
	}
}

void SubsFile::SortAll(bool func(Dialogue *i, Dialogue *j))
{
	std::vector<Dialogue*> origDialogues = subs->dialogues;
	std::stable_sort(subs->dialogues.begin(), subs->dialogues.end(), func);
	size_t dialsSize = subs->dialogues.size();
	for (size_t i = 0; i < dialsSize; i++) {
		if (subs->dialogues[i] != origDialogues[i]) {
			CopyDialogueF(i);
		}
	}
	origDialogues.clear();
}

void SubsFile::SortSelected(bool func(Dialogue *i, Dialogue *j))
{
	std::vector<Dialogue*> selected;
	for (auto cur = subs->Selections.begin(); cur != subs->Selections.end(); cur++){
		Dialogue *dial = subs->dialogues[*cur];
		selected.push_back(dial);
	}
	std::stable_sort(selected.begin(), selected.end(), func);
	int ii = 0;
	for (auto cur = subs->Selections.begin(); cur != subs->Selections.end(); cur++){
		Dialogue* origDial = subs->dialogues[*cur];
		Dialogue* sortedDial = selected[ii++];
		if (origDial != sortedDial) {
			subs->dialogues[*cur] = sortedDial;
			CopyDialogueF(*cur);
		}
	}
	selected.clear();
}

void SubsFile::GetSelections(wxArrayInt &selections, bool deselect/*=false*/, bool checkVisible /*= true*/)
{
	selections.clear();
	for (std::set<int>::iterator i = subs->Selections.begin(); i != subs->Selections.end(); i++){
		int sel = (*i);
		if (!checkVisible || (sel < subs->dialogues.size() && subs->dialogues[sel]->isVisible))
			selections.Add(sel);
	}
	if (deselect){ subs->Selections.clear(); }
}

void SubsFile::InsertSelection(size_t i)
{
	subs->Selections.insert(i);
}

void SubsFile::InsertSelections(size_t from, size_t to, bool deselect /*= false*/, bool skipHidden /*= false*/)
{
	if (deselect){ subs->Selections.clear(); }
	size_t dialsize = subs->dialogues.size();
	if (from >= dialsize){ return; }
	if (to >= dialsize){ to = dialsize - 1; }
	for (size_t i = from; i <= to; i++){
		if (!skipHidden || subs->dialogues[i]->isVisible){
			subs->Selections.insert(i);
		}
	}
}


void SubsFile::EraseSelection(size_t i)
{
	subs->Selections.erase(i);
}

size_t SubsFile::FindVisibleKey(size_t key, int *corrected)
{
	key = MID(0, key, GetCount() - 1);
	Dialogue *dial = subs->dialogues[key];
	if (!dial->isVisible){
		size_t i = key - 1;
		while (i + 1 > 0){
			if (subs->dialogues[i]->isVisible != NOT_VISIBLE){
				if (corrected){ *corrected = i; }
				return i;
			}
			i--;
		}
		i = key + 1;
		while (i < subs->dialogues.size()){
			if (subs->dialogues[i]->isVisible != NOT_VISIBLE){
				if (corrected){ *corrected = i; }
				return i;
			}
			i++;
		}
	}else
		return key;

	return 0;
}

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

size_t SubsFile::GetElementById(size_t id)
{
	const VisibleRows &rows = GetVisibleRows();
	// it's possible when id >= size
	return (id < rows.keyOfId.size()) ? rows.keyOfId[id] : -1;
}

size_t SubsFile::GetElementByKey(size_t key)
{
	const VisibleRows &rows = GetVisibleRows();
	return (key < rows.idOfKey.size()) ? rows.idOfKey[key] : -1;
}

Styles *SubsFile::CopyStyle(size_t i, bool push)
{
	Styles *styl = subs->styles[i]->Copy();
	subs->deleteStyles.push_back(styl);
	MarkEdited();
	if (push){
		subs->styles[i] = styl;
	}
	return styl;
}

SInfo *SubsFile::CopySinfo(size_t i, bool push)
{
	SInfo *sinf = subs->sinfo[i]->Copy();
	subs->deleteSinfo.push_back(sinf);
	MarkEdited();
	if (push){
		subs->sinfo[i] = sinf;
	}
	return sinf;
}

void SubsFile::EndLoad(unsigned char editionType, int activeLine, bool initialSave)
{
	//subs->activeLine = activeLine;
	//subs->markerLine = activeLine;
	subs->editionType = editionType;
	m_history.Record(subs);
	if (initialSave)
		m_history.ForgetSaved();
	subs = subs->Copy();
	edited = false;
	++version;
	lastStepTyping = false;
}

void SubsFile::DropOldestHistory(int num)
{
	wxMutexLocker lock(*historyGuard);
	m_history.DropOldest(num, MergeOwned);
}

void SubsFile::GetURStatus(bool *_undo, bool *_redo)
{
	*_redo = m_history.CanRedo();
	*_undo = m_history.CanUndo();
}

//File *SubsFile::GetSubs()
//{
//	return subs;
//}

unsigned char SubsFile::CheckIfHasHiddenBlock(int i, bool firstLine /*= false*/){
	int size = subs->dialogues.size();
	size_t keyFirst = i + 1;

	if (keyFirst < size){
		int j = keyFirst;
		Dialogue * dial = subs->dialogues[j];
		if (dial->isVisible == VISIBLE_BLOCK){
			if (j == 0)
				return 2;

			Dialogue * dialPrev = subs->dialogues[j - 1];
			if (dialPrev->isVisible != VISIBLE_BLOCK) return 2;
			return 0;
		}
	}
	if (i >= size){ return 0; }
	if (firstLine && i >= 0)
		keyFirst--;

	size_t numOfLines = 0;
	while (keyFirst < subs->dialogues.size()){
		if (subs->dialogues[keyFirst]->isVisible){ 
			if (numOfLines)
				return 1;
			else
				return 0;
		}

		if (!subs->dialogues[keyFirst]->NonDialogue)
			numOfLines++;

		keyFirst++;
	}

	return (numOfLines) ? 1 : 0;
}


size_t SubsFile::GetKeyFromPos(size_t position, size_t numOfLines)
{
	size_t visibleLines = 0;
	for (size_t i = position; i < subs->dialogues.size(); i++){
		if (numOfLines == visibleLines)
			return i;

		if (subs->dialogues[i]->isVisible)
			visibleLines++;
	}

	return -1;
}

bool SubsFile::CheckIfIsTree(size_t i){
	if (i >= subs->dialogues.size())
		return false;

	Dialogue *dial = subs->dialogues[i];
	return dial->treeState == TREE_DESCRIPTION;
}

int SubsFile::FindEndOfTree(size_t i)
{
	size_t size = GetCount();
	if (i >= size)
		return size - 1;

	for (size_t j = i + 1; j < size; j++) {
		if(subs->dialogues[j]->treeState != TREE_OPENED)
			return j - 1;
	}
	return size - 1;
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
	for (int i = 0; i < m_history.Size(); i++){
		File *step = m_history.At(i);
		history->push_back(historyNames[step->editionType] +
			wxString::Format(_(", active line %i"), (int)GetElementByKey(step->activeLine) + 1));
	}
}

void SubsFile::ShowHistory(wxWindow *parent, std::function<void(int)> functionAfterChangeHistory)
{
	HistoryDialog HD(parent, this, functionAfterChangeHistory);
	HD.ShowModal();
}

void SubsFile::SetLastSave()
{
	m_history.MarkSaved();
}

int SubsFile::GetActualHistoryIter()
{
	return m_history.StepsFromSave();
}

const wxString & SubsFile::GetUndoName()
{
	if (!m_history.CanUndo())
		return emptyString;

	return historyNames[m_history.At(m_history.Step() - 1)->editionType];
}

const wxString & SubsFile::GetRedoName()
{
	if (!m_history.CanRedo())
		return emptyString;

	return historyNames[m_history.At(m_history.Step() + 1)->editionType];
}

bool SubsFile::IsFiltered()
{
	return subs->isFiltered;
}

void SubsFile::SetFiltered(bool filtered)
{
	subs->isFiltered = filtered;
}

void SubsFile::AddStyle(Styles *nstyl)
{
	MarkEdited();
	subs->deleteStyles.push_back(nstyl);
	subs->styles.push_back(nstyl);
}

void SubsFile::ChangeStyle(Styles *nstyl, size_t i)
{
	MarkEdited();
	subs->deleteStyles.push_back(nstyl);
	subs->styles[i] = nstyl;
}

size_t SubsFile::StylesSize()
{
	return subs->styles.size();
}

Styles *SubsFile::GetStyle(size_t i, const wxString &name/* = emptyString*/)
{
	if (name != emptyString){
		for (size_t j = 0; j < subs->styles.size(); j++)
		{
			if (name == subs->styles[j]->Name){ return subs->styles[j]; }
		}
	}
	if (!subs->styles.size()) {
		AddStyle(new Styles());
	}
	return subs->styles[i];
}

std::vector<Styles*> *SubsFile::GetStyleTable()
{
	return &subs->styles;
}

void SubsFile::InsertStyle(size_t i, Styles *style)
{
	MarkEdited();
	subs->deleteStyles.push_back(style);
	if (i >= subs->styles.size())
		subs->styles.push_back(style);
	else
		subs->styles.insert(subs->styles.begin() + i, style);
}

void SubsFile::MoveStyle(size_t from, size_t to)
{
	MarkEdited();
	Styles *style = subs->styles[from];
	subs->styles.erase(subs->styles.begin() + from);
	subs->styles.insert(subs->styles.begin() + to, style);
}

void SubsFile::SortStyles(bool func(Styles *i, Styles *j))
{
	MarkEdited();
	std::sort(subs->styles.begin(), subs->styles.end(), func);
}

//multiplication musi być ustawione na zero, wtedy zwróci ilość multiplikacji
size_t SubsFile::FindStyle(const wxString &name, int *multiplication)
{
	size_t isfound = -1;
	for (size_t j = 0; j < subs->styles.size(); j++)
	{
		if (name == subs->styles[j]->Name){
			isfound = j;
			if (multiplication){
				(*multiplication)++;
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

void SubsFile::DeleteStyle(size_t i)
{
	MarkEdited();
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
	return nullptr;
}

void SubsFile::DeleteSInfo(size_t i)
{
	subs->sinfo.erase(subs->sinfo.begin() + i);
	MarkEdited();
}

SInfo *SubsFile::GetSInfoAt(size_t i)
{
	return (i < subs->sinfo.size()) ? subs->sinfo[i] : nullptr;
}

void SubsFile::SetSInfoAt(size_t i, SInfo *info)
{
	MarkEdited();
	subs->deleteSinfo.push_back(info);
	subs->sinfo[i] = info;
}

void SubsFile::InsertSInfo(size_t i, SInfo *info)
{
	MarkEdited();
	subs->deleteSinfo.push_back(info);
	if (i >= subs->sinfo.size())
		subs->sinfo.push_back(info);
	else
		subs->sinfo.insert(subs->sinfo.begin() + i, info);
}

size_t SubsFile::SInfoSize()
{
	return subs->sinfo.size();
}

void SubsFile::SaveSelectionsF(bool clear, int currentLine, int markedLine, int scrollPos)
{
	File *step = m_history.Current();
	if (!step){
		if (clear){ ClearSelections(); }
		return;
	}
	step->Selections = subs->Selections;
	//tutaj muszą być przeróbki na klucze
	step->activeLine = currentLine;
	step->markerLine = markedLine;
	step->scrollPosition = scrollPos;
	if (clear){ ClearSelections(); }
}

size_t SubsFile::FirstSelection(size_t *id /*= nullptr*/)
{
	if (!subs->Selections.empty()){
		// return only visible element when nothing is visible, return -1;
		for (auto it = subs->Selections.begin(); it != subs->Selections.end(); it++){
			int sel = (*it);
			if (sel < subs->dialogues.size() && subs->dialogues[sel]->isVisible){
				if (id){
					*id = GetElementByKey(sel);
					if (*id == -1)
						return -1;
				}

				return sel;
			}
		}
	}
	if (id)
		*id = -1;

	return -1;
}

void SubsFile::InsertRowsF(int Row,
	const std::vector<Dialogue *> &RowsTable,
	bool AddToDestroy)
{
	size_t convertedRow = Row;
	if (convertedRow >= subs->dialogues.size()){ convertedRow = subs->dialogues.size(); }
	subs->dialogues.insert(subs->dialogues.begin() + convertedRow, RowsTable.begin(), RowsTable.end());
	MarkEdited();
	if (AddToDestroy){ subs->deleteDialogues.insert(subs->deleteDialogues.end(), RowsTable.begin(), RowsTable.end()); }
}

void SubsFile::InsertRowsF(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy)
{
	size_t convertedRow = Row;
	if (convertedRow >= subs->dialogues.size()){ convertedRow = subs->dialogues.size(); }
	subs->dialogues.insert(subs->dialogues.begin() + convertedRow, NumRows, Dialog);
	MarkEdited();
	if (AddToDestroy){ subs->deleteDialogues.push_back(Dialog); }
}

void SubsFile::SwapRowsF(int frst, int scnd)
{
	Dialogue *first = CopyDialogueF(frst);
	Dialogue *second = CopyDialogueF(scnd);
	subs->dialogues[frst] = second;
	subs->dialogues[scnd] = first;
	first->ChangeDialogueState(1);
	second->ChangeDialogueState(1);
}

void SubsFile::AddSInfo(const wxString &SI, wxString val, bool save)
{
	wxString key;
	if (val == emptyString){
		key = SI.BeforeFirst(L':');
		key.Trim(false);
		key.Trim(true);
		val = SI.AfterFirst(L':');
		val.Trim(false);
		val.Trim(true);
	}
	else{ key = SI; }
	SInfo *oldinfo = nullptr;
	int ii = -1;
	oldinfo = GetSInfoP(key, &ii);

	if (!oldinfo || save){
		MarkEdited();
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