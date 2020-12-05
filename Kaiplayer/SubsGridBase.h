//  Copyright (c) 2012 - 2020, Marcin Drob

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

#include <wx/wx.h>

#include "Styles.h"
#include "SubsDialogue.h"
#include "SubsFile.h"

#include "KaiScrollbar.h"
#include "LineParse.h"
#include <vector>
#include <set>


class EditBox;
class KainoteFrame;
class SubsGrid;
class SubsGridPreview;
class TabPanel;


class compareData{
public:
	compareData(){};
	//_wxArraywxArrayInt &operator [](size_t i)const{ return lineCompare[i]; }
	const int &operator [](size_t i)const{ return lineCompare[i]; }
	size_t size()const{ return lineCompare.size(); }
	void push_back(const int &elem){ lineCompare.Add(elem); }
	int secondComparedLine = -1;
	bool differences = true;
private:
	wxArrayInt lineCompare;
};

class SubsGridBase : public KaiScrolledWindow
{
	friend class SubsGridPreview;
public:

	//wxMutex mutex;
	void AddLine(Dialogue *line);
	void AddStyle(Styles *nstyl);
	void ChangeLine(unsigned char editionType, Dialogue *line1, size_t wline, long cells, bool selline = false, bool dummy = false);
	void ChangeCell(long cells, size_t wline, Dialogue *what);
	void ChangeStyle(Styles *nstyl, size_t i);
	void Clearing();
	void Convert(char type);

	int FindStyle(const wxString &name, int *multiplication = NULL);
	void GetStyles(wxString &stylesText, bool tld = false);
	//this function is safe, do not return NULL, when failed returns i
	Styles *GetStyle(size_t i, const wxString &name = L"");
	std::vector<Styles*> *GetStyleTable();
	bool IsModified();

	void SaveFile(const wxString &filename, bool cstat = true, bool loadFromEditbox = false);
	wxString *SaveText();

	size_t StylesSize();
	void DelStyle(int i);
	void ChangeTimes(bool byFrame = false);

	void SortIt(short what, bool all = true);
	void DeleteRows();
	void DeleteRow(int rw, int len = 1);
	void DeleteText();
	void GetUndo(bool redo, int iter = -2);
	//Warning!! Adding the same dialogue pointer to destroyer cause crash
	//not adding it when needed cause memory leaks.
	void InsertRows(int Row, const std::vector<Dialogue *> &RowsTable, bool AddToDestroy = false);
	//Warning!! Adding the same dialogue pointer to destroyer cause crash
	//not adding it when needed cause memory leaks.
	void InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy = true, bool Save = false);
	void SetSubsFormat(wxString ext = L"");
	void AddSInfo(const wxString &SI, wxString val = L"", bool save = true);
	void SetModified(unsigned char editionType, bool redit = true, bool dummy = false, int SetEditBoxLine = -1, bool Scroll = true);
	void UpdateUR(bool tolbar = true);
	void GetSInfos(wxString &textSinfo, bool tld = false);
	const wxString &GetSInfo(const wxString &key, int *ii = 0);
	SInfo *GetSInfoP(const wxString &key, int *ii = 0);
	size_t FirstSelection(size_t *firstSelectionId = NULL);
	void SwapRows(int frst, int scnd, bool sav = false);
	void LoadSubtitles(const wxString &str, wxString &ext);
	bool MoveRows(int step, bool keyStep = false);
	void SetStartTime(int stime);
	void SetEndTime(int etime);
	bool SetTlMode(bool mode);
	void LoadDefault(bool line = true, bool sav = true, bool endload = true);
	void GetASSRes(int *x, int *y);
	size_t SInfoSize();
	size_t GetCount();
	void NextLine(int dir = 1);
	void SaveSelections(bool clear = false);
	// no checks, check if value is unsure
	Dialogue *CopyDialogue(size_t i, bool push = true);
	// returns null when there's no visible dialogue with that offset or it is out of the table
	Dialogue *CopyDialogueWithOffset(size_t i, int offset, bool push = true);
	// no checks, check if value is unsure
	Dialogue *GetDialogue(size_t i);
	// returns null when there's no visible dialogue with that offset or it is out of the table
	Dialogue *GetDialogueWithOffset(size_t i, int offset);
	// returns visible lines as string for Vsfilter
	wxString *GetVisible(bool *visible = 0, wxPoint *point = NULL, wxArrayInt *selected = NULL, bool allSubs = false);
	bool IsLineVisible();
	//Get line key from scrollPosition.
	//Every value will be stored as key.
	//Simple function to convert key to id from scroll position
	//to use with mouse, scroll events
	size_t GetKeyFromScrollPos(int numOfLines);
	//it should works without checks;
	size_t GetKeyFromPosition(size_t position, int delta, bool safe = true);

	void DummyUndo(int newIter);
	void GetCommonStyles(SubsGridBase *grid, wxArrayString &styleTable);
	int GetScrollPosition(){ return scrollPosition; }

	SubsGridBase(wxWindow *parent, const long int id, const wxPoint& pos, const wxSize& size, long style);
	virtual ~SubsGridBase();

	bool hasTLMode = false;
	bool showOriginal = false;
	bool makebackup;
	char subsFormat = ASS;
	char originalFormat = ASS;
	int markedLine = 0;
	int currentLine = 0;
	bool showFrames = false;
	bool savedSelections = false;
	bool isFiltered = false;
	bool ignoreFiltered = false;
	std::vector<TextData> SpellErrors;
	std::vector<compareData> *Comparison;
	SubsFile* file;
	EditBox *Edit;
	//comparison static pointers needs short name because we not use this class
	static SubsGrid* CG1;
	static SubsGrid* CG2;
	static void SubsComparison();
	static void RemoveComparison();
	static wxArrayString compareStyles;
	static bool hasCompare;
	wxMutex &GetMutex() { return editionMutex; }
private:
	virtual void AdjustWidths(int cell = 16383){};
	virtual void RefreshColumns(int cell = 16383){};
	virtual void MakeVisible(int row){};
	//to add option to not center lines is need make visible
	virtual void ScrollTo(int y, bool center = false, int offset = 0, bool useUpdate = false){};
	
protected:
	static void CompareTexts(compareData &firstTable, compareData &secondTable, const wxString &first, const wxString &second);
	short numsave;
	bool hideOverrideTags;
	bool ismenushown = false;
	bool first = false;
	int visibleColumns;
	int panelrows = 0;
	int lastActiveLine = 0;
	int lastRow = 0;
	int scrollPosition = 0;
	int scrollPositionId = 0;
	int scHor = 0;
	int GridHeight = 0;
	KainoteFrame* Kai;
	std::vector<bool> visibleLines;
	wxTimer timer;
	wxTimer nullifyTimer;
	void OnBackupTimer(wxTimerEvent &event);
	TabPanel *tab;
	wxMutex editionMutex;
};

bool sortstart(Dialogue *i, Dialogue *j);

enum{
	ID_AUTIMER
};
