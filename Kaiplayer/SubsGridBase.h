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

#pragma once

#include <wx/wx.h>

#include "Styles.h"
#include "SubsGridWindow.h"

//class EditBox;
//class kainoteFrame;

class SubsGridBase : public SubsGridWindow
{
	friend class SubsFile;
public:

	wxMutex mutex;
  
	bool Modified;
	void AddLine(Dialogue *line);
	void AddStyle(Styles *nstyl);
	void ChangeLine(unsigned char editionType, Dialogue *line1, int wline, long cells, bool selline=false, bool dummy=false);
	void ChangeCell(long cells, int wline, Dialogue *what);
	void ChangeStyle(Styles *nstyl,int i);
	void Clearing();
	void Convert(char type);

	int FindStyle(wxString name,int *multiplication=NULL);
	wxString GetStyles(bool tld=false);
	Styles *GetStyle(int i,wxString name="");
	std::vector<Styles*> *GetStyleTable();
	
	
	void SaveFile(const wxString &filename, bool cstat=true);
	wxString *SaveText();
	
	
	
	int StylesSize();
	
	
	void DelStyle(int i);
	
	Dialogue *GetDialCor(int ii);
	void ChangeTimes(bool byFrame = false);
	
	void SortIt(short what,bool all=true);
	void DeleteRows();
	void DeleteRow(int rw,int len=1);
	void DeleteText();
	void GetUndo(bool redo, int iter = -2);
	void InsertRows(int Row, std::vector<Dialogue *> RowsTable, bool AddToDestroy=false);
	void InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy=true, bool Save=false);
	void SetSubsForm(wxString ext="");
	void AddSInfo(const wxString &SI, wxString val="", bool save=true);
	void SetModified(unsigned char editionType, bool redit=true, bool dummy=false, int SetEditBoxLine = -1, bool Scroll = true);
	void UpdateUR(bool tolbar=true);
	wxString GetSInfos(bool tld=false);
	wxString GetSInfo(const wxString &key, int *ii=0);
	SInfo *GetSInfoP(const wxString &key, int *ii=0);
	wxArrayInt GetSels(bool deselect=false);
	void SwapRows(int frst, int scnd, bool sav=false);
	void Loadfile(const wxString &str,const wxString &ext);
	void MoveRows(int step, bool sav=false);
	void SetStartTime(int stime);
	void SetEndTime(int etime);
	bool SetTlMode(bool mode);
	void LoadDefault(bool line=true,bool sav=true, bool endload=true);
	void GetASSRes(int *x,int *y);
	int SInfoSize();
	int GetCount();

	void SelVideoLine(int time=-1);
	void NextLine(int dir=1);
	bool IsNumber(const wxString &txt);
	Dialogue *CopyDial(int i, bool push=true);
	Dialogue *GetDial(int i);
	wxString *GetVisible(bool *visible=0, wxPoint *point = NULL, wxArrayInt *selected = NULL);
	//wxString *GetVisibleSubs();
	void RebuildActorEffectLists();
	
	void DummyUndo(int newIter);
	

	bool makebackup;
	
	
	SubsGridBase(wxWindow *parent, const long int id ,const wxPoint& pos,const wxSize& size, long style);
	virtual ~SubsGridBase();

	
protected:
	short numsave;
	
	void OnBackupTimer(wxTimerEvent &event);
	
	wxTimer timer;
	wxTimer nullifyTimer;
	//HANDLE qtimer;

	DECLARE_EVENT_TABLE()
};

bool sortstart(Dialogue *i,Dialogue *j);


