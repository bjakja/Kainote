//  Copyright (c) 2012-2017, Marcin Drob

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
#include <vector>
#include <set>

class EditBox;
class kainoteFrame;
class SubsGrid;

class compareData{
public:
	compareData(){};
	_wxArraywxArrayInt &operator [](size_t i)const{ return lineCompare[i]; }
	size_t size()const{ return lineCompare.size(); }
	void push_back(const int &elem){ lineCompare.Add(elem); }
	int secondComparedLine = -1;
	bool differences = true;
private:
	wxArrayInt lineCompare;
};

class SubsGridBase : public KaiScrolledWindow
{
	friend class SubsFile;
public:

	//wxMutex mutex;
	void AddLine(Dialogue *line);
	void AddStyle(Styles *nstyl);
	void ChangeLine(unsigned char editionType, Dialogue *line1, int wline, long cells, bool selline=false, bool dummy=false);
	void ChangeCell(long cells, int wline, Dialogue *what);
	void ChangeStyle(Styles *nstyl,int i);
	void Clearing();
	void Convert(char type);

	int FindStyle(const wxString &name,int *multiplication=NULL);
	void GetStyles(wxString &stylesText, bool tld=false);
	Styles *GetStyle(int i, const wxString &name="");
	std::vector<Styles*> *GetStyleTable();
	bool IsModified();
	
	void SaveFile(const wxString &filename, bool cstat = true, bool loadFromEditbox = false);
	wxString *SaveText();
	
	int StylesSize();
	void DelStyle(int i);
	void ChangeTimes(bool byFrame = false);
	
	void SortIt(short what,bool all=true);
	void DeleteRows();
	void DeleteRow(int rw,int len=1);
	void DeleteText();
	void GetUndo(bool redo, int iter = -2);
	void InsertRows(int Row, const std::vector<Dialogue *> &RowsTable, bool AddToDestroy=false, bool asKey = false);
	void InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy = true, bool Save = false, bool asKey = false);
	void SetSubsFormat(wxString ext="");
	void AddSInfo(const wxString &SI, wxString val="", bool save=true);
	void SetModified(unsigned char editionType, bool redit = true, bool dummy = false, int SetEditBoxLine = -1, bool Scroll = true);
	void UpdateUR(bool tolbar=true);
	void GetSInfos(wxString &textSinfo, bool tld=false);
	const wxString &GetSInfo(const wxString &key, int *ii=0);
	SInfo *GetSInfoP(const wxString &key, int *ii=0);
	int FirstSelection();
	void SwapRows(int frst, int scnd, bool sav=false);
	void LoadSubtitles(const wxString &str, wxString &ext);
	void MoveRows(int step, bool sav=false);
	void SetStartTime(int stime);
	void SetEndTime(int etime);
	bool SetTlMode(bool mode);
	void LoadDefault(bool line=true,bool sav=true, bool endload=true);
	void GetASSRes(int *x,int *y);
	int SInfoSize();
	int GetCount();
	void NextLine(int dir=1);
	void SaveSelections(bool clear=false);
	Dialogue *CopyDialogue(int i, bool push=true);
	Dialogue *CopyDialogueByKey(int i, bool push = true);
	Dialogue *GetDialogue(int i);
	wxString *GetVisible(bool *visible=0, wxPoint *point = NULL, wxArrayInt *selected = NULL);
	void RebuildActorEffectLists();
	
	void DummyUndo(int newIter);
	bool IsNumber(const wxString &txt);
	void GetCommonStyles(SubsGridBase *grid, wxArrayString &styleTable);
	
	SubsGridBase(wxWindow *parent, const long int id ,const wxPoint& pos,const wxSize& size, long style);
	virtual ~SubsGridBase();

	bool hasTLMode = false;
	bool showOriginal = false;
	bool makebackup;
	char subsFormat = ASS;
	char originalFormat = ASS;
	int markedLine = 0;
	bool showFrames = false;
	bool savedSelections = false;
	bool isFiltered = false;
	bool ignoreFiltered = false;
	std::vector<wxArrayInt> SpellErrors;
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
private:
	virtual void AdjustWidths(int cell = 8191){};
	virtual void RefreshColumns(int cell = 8191){};
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
	int scPos = 0;
	int scHor = 0;
	int GridHeight = 0;
	kainoteFrame* Kai;
	std::vector<bool> visibleLines;
	wxTimer timer;
	wxTimer nullifyTimer;
	void OnBackupTimer(wxTimerEvent &event);
	
};

bool sortstart(Dialogue *i,Dialogue *j);

enum{
	LAYER = 1,
	START = 2,
	END = 4,
	STYLE = 8,
	ACTOR = 16,
	MARGINL = 32,
	MARGINR = 64,
	MARGINV = 128,
	EFFECT = 256,
	CNZ = 512,
	TXT = 1024,
	TXTTL = 2048,
	COMMENT = 4096,
	ID_AUTIMER
};
