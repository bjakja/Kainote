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

#pragma once

#include "Styles.h"
#include "SubsDialogue.h"
#include "KaiDialog.h"
#include <vector>
#include <set>
#include <functional>

enum{
	OPEN_SUBTITLES = 1,
	NEW_SUBTITLES,
	EDITBOX_LINE_EDITION,
	EDITBOX_MULTILINE_EDITION,
	EDITBOX_SPELL_CHECKER,
	GRID_DUPLICATE,
	GRID_JOIN,
	GRID_JOIN_WITH_PREVIOUS,
	GRID_JOIN_WITH_NEXT,
	GRID_JOIN_TO_FIRST,
	GRID_JOIN_TO_LAST,
	GRID_PASTE_LINES,
	GRID_PASTE_DIALOGUE_COLUMNS,
	GRID_PASTE_TRANSLATION_TO_SUBS,
	GRID_TRANSLATION_TEXT_MOVE,
	GRID_MAKE_LINES_CONTINUES,
	GRID_SET_VIDEO_FPS,
	GRID_SET_CUSTOM_FPS,
	GRID_SWAP,
	GRID_CONVERT,
	GRID_SORT_LINES,
	GRID_DELETE_LINES,
	GRID_DELETE_TEXT,
	GRID_SET_START_TIME,
	GRID_SET_END_TIME,
	GRID_TURN_ON_TLMODE,
	GRID_TURN_OFF_TLMODE,
	GRID_APPEND_LINE,
	GRID_INSERT_ROW,
	AUDIO_CHANGE_TIME,
	SNAP_TO_KEYFRAME_OR_LINE_TIME,
	ASS_PROPERTIES,
	SELECT_LINES,
	SHIFT_TIMES,
	SPELL_CHECKER,
	STYLE_MANAGER,
	SUBTITLES_RESAMPLE,
	VISUAL_POSITION,
	VISUAL_MOVE,
	VISUAL_SCALE,
	VISUAL_ROTATION_Z,
	VISUAL_ROTATION_X_Y,
	VISUAL_RECT_CLIP,
	VISUAL_VECTOR_CLIP,
	VISUAL_DRAWING,
	VISUAL_POSITION_SHIFTER,
	VISUAL_SCALE_ROTATION_SHIFTER,
	VISUAL_ALL_TAGS,
	REPLACE_SINGLE,
	REPLACE_ALL,
	REPLACED_BY_MISSPELL_REPLACER,
	TREE_ADD,
	TREE_SET_DESCRIPTION,
	TREE_ADD_LINES,
	TREE_REMOVE,
	AUTOMATION_SCRIPT
};

//Filtering is treated as keys, every dialogue get/set functions use keys, ids are only for paint or mouse using

class File
{
public:
	std::vector<Dialogue*> dialogues;
	std::vector<Styles*> styles;
	std::vector<SInfo*> sinfo;
	std::vector<Dialogue*> deleteDialogues;
	std::vector<Styles*> deleteStyles;
	std::vector<SInfo*> deleteSinfo;
	std::set<int> Selections;

	unsigned char editionType;
	int activeLine;
	int markerLine = 0;
	int scrollPosition = 0;
	File();
	~File();
	void Clear();
	File *Copy(bool copySelections = true);
};

class SubsFile
{
private:
	std::vector<File*> undo;
	int iter;
	File *subs;
	int lastSave = 0;

public:
	SubsFile(wxMutex * editionGuard);
	~SubsFile();
	void SaveUndo(unsigned char editionType, int activeLine, int markerLine);
	bool Redo();
	bool Undo();
	void DummyUndo();
	void DummyUndo(int newIter);
	void EndLoad(unsigned char editionType, int activeLine, bool initialSave = false);
	size_t GetCount();
	size_t GetIdCount();
	void AppendDialogue(Dialogue *dial);
	//check if exceeds tabe or if dialogue is not visible can return null
	Dialogue *CopyVisibleDialogue(size_t i, bool push = true, bool keepstate = false);
	Dialogue *CopyDialogue(size_t i, bool push = true, bool keepstate = false);
	//check if exceeds tabe or if dialogue is not visible can return null
	Dialogue *GetVisibleDialogue(size_t i);
	Dialogue *GetDialogue(size_t i);
	void SetDialogue(size_t i, Dialogue *dial, bool addToDestroyer = false);
	void DeleteDialogues(size_t from, size_t to);
	void DeleteSelectedDialogues();
	//Warning!! Adding the same dialogue pointer to destroyer cause crash
	//not adding it when needed cause memory leaks.
	void InsertRows(int Row, const std::vector<Dialogue *> &RowsTable, bool AddToDestroy);
	//Warning!! Adding the same dialogue pointer to destroyer cause crash
	//not adding it when needed cause memory leaks.
	void InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy, bool Save);
	void SwapRows(int frst, int scnd);
	void SortAll(bool func(Dialogue *i, Dialogue *j));
	void SortSelected(bool func(Dialogue *i, Dialogue *j));
	Styles *CopyStyle(size_t i, bool push = true);
	SInfo *CopySinfo(size_t i, bool push = true);
	void AddStyle(Styles *nstyl);
	void ChangeStyle(Styles *nstyl, size_t i);
	size_t StylesSize();
	Styles *GetStyle(size_t i, const wxString &name = L"");
	std::vector<Styles*> *GetStyleTable();

	//multiplication must be set to 0
	size_t FindStyle(const wxString &name, int *multip);
	void GetStyles(wxString &stylesText, bool addTlModeStyle = false);
	void DeleleStyle(size_t i);
	const wxString & GetSInfo(const wxString &key, int *ii = 0);
	SInfo *GetSInfoP(const wxString &key, int *ii);
	void DeleteSInfo(size_t i);
	void AddSInfo(const wxString &SI, wxString val, bool save);
	void GetSInfos(wxString &textSinfo, bool addTlMode = false);
	size_t SInfoSize();
	void SaveSelections(bool clear, int currentLine, int markedLine, int scrollPos);
	size_t FirstSelection(size_t *id = NULL);
	File *GetSubs(){ return subs; }
	void GetSelections(wxArrayInt &selections, bool deselect=false, bool checkVisible = true);
	const std::set<int> & GetSelectionsAsKeys(){ return subs->Selections; };
	void InsertSelection(size_t i);
	void InsertSelections(size_t from, size_t to, bool deselect = false, bool skipHidden = true);
	void EraseSelection(size_t i);
	size_t FindVisibleKey(size_t key, int *corrected = NULL);
	bool IsSelected(size_t i);
	size_t SelectionsSize();
	int GetActiveLine(){ return subs->activeLine; }
	int GetMarkerLine(){ return subs->markerLine; }
	int GetScrollPosition(){ return subs->scrollPosition; }
	void ClearSelections();
	size_t GetElementById(size_t Id);
	size_t GetElementByKey(size_t Key);
	unsigned char CheckIfHasHiddenBlock(int i, bool firstLine = false);
	
	size_t GetKeyFromPos(size_t position, size_t numOfLines);
	bool CheckIfIsTree(size_t i);
	int FindEndOfTree(size_t i);
	int OpenCloseTree(size_t i);
	void GetURStatus(bool *_undo, bool *_redo);
	bool IsNotSaved();
	int maxx();
	int Iter();
	void RemoveFirst(int num);
	void ShowHistory(wxWindow *parent, std::function<void(int)> functionAfterChangeHistory);
	void GetHistoryTable(wxArrayString *history);
	bool SetHistory(int iter);
	void SetLastSave();
	int GetActualHistoryIter();
	int GetLastSaveIter(){ return lastSave; }
	bool CanSave(){ return iter != lastSave; }
	const wxString &GetUndoName();
	const wxString &GetRedoName();
	bool edited;
	wxString *historyNames = NULL;
	wxMutex *historyGuard = NULL;
};

class HistoryDialog : public KaiDialog
{
public:
	HistoryDialog(wxWindow *parent, SubsFile *file, std::function<void(int)> functionAfterChangeHistory );
	virtual ~HistoryDialog(){};
};

enum{
	ID_HISTORY_LIST = 777,
	ID_SET_HISTORY,
	ID_SET_HISTORY_AND_CLOSE,
};