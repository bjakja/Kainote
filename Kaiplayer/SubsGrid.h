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

#ifndef SUBSGRID_H_INCLUDED
#define SUBSGRID_H_INCLUDED

//#pragma once

#include <wx/wx.h>
#include "SubsDialogue.h"
#include <vector>
#include <map>
#include "Styles.h"
#include "SubsFile.h"
#include "KaiScrollbar.h"

class EditBox;
class kainoteFrame;

class SubsGrid : public KaiScrolledWindow
{
	friend class SubsFile;
public:
	std::map<int,bool> sel;
  
	char form;
	char origform;
	EditBox *Edit;
	wxMutex mutex;
  
	bool Modified;
	void AddLine(Dialogue *line);
	void AddStyle(Styles *nstyl);
	void AdjustWidths(int cell=8191);
	int CalcChars(const wxString &txt, wxString *lines=NULL, bool *bad=NULL);
	void ChangeLine(Dialogue *line1, int wline, long cells, bool selline=false, bool dummy=false);
	void ChangeCell(long cells, int wline, Dialogue *what);
	void ChangeStyle(Styles *nstyl,int i);
	void Clearing();
	void Convert(char type);

	int FindStyle(wxString name,int *multiplication=NULL);
	wxString GetStyles(bool tld=false);
	Styles *GetStyle(int i,wxString name="");
	std::vector<Styles*> *GetStyleTable();
	void SetStyle();
	void RepaintWindow(int cell=8191);
	void SelectRow(int row, bool addToSelected = false, bool select=true, bool norefresh=false);
	void ScrollTo(int y, bool center=false);
	
	void SaveFile(const wxString &filename, bool cstat=true);
	wxString *SaveText();
	void HideOver();
	
	
	int StylesSize();
	
	
	void DelStyle(int i);
	
	Dialogue *GetDialCor(int ii);
	void ChangeTimes(bool byFrame = false);
	int GetCount();
	int FirstSel();
	void SortIt(short what,bool all=true);
	void DeleteRows();
	void DeleteRow(int rw,int len=1);
	void DeleteText();
	void GetUndo(bool redo);
	void InsertRows(int Row, std::vector<Dialogue *> RowsTable, bool AddToDestroy=false);
	void InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy=true, bool Save=false);
	void SetSubsForm(wxString ext="");
	void AddSInfo(const wxString &SI, wxString val="", bool save=true);
	void SetModified(bool redit=true, bool dummy=false, int SetEditBoxLine = -1);
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
  
	bool transl;
	bool showtl;

	void SelVideoLine(int time=-1);
	void NextLine(int dir=1);
	bool IsNum(const wxString &txt);
	Dialogue *CopyDial(int i, bool push=true);
	Dialogue *GetDial(int i);
	wxString *GetVisible(bool *visible=0, wxPoint *point = NULL, wxArrayInt *selected = NULL);
	//wxString *GetSubsToEnd(bool *visible=0, wxPoint *point=0);
	void RebuildActorEffectLists();
	void RefreshIfVisible(int time);
	void SetVideoLineTime(wxMouseEvent &evt);
	void ChangeTimeDisplay(bool frame);

	bool makebkp;
	bool showFrames;
	int lastRow;
	int visible;
	int markedLine;
	int panelrows;
	int lastActiveLine;
	SubsFile* file;
	std::vector<wxArrayInt> SpellErrors;
	std::vector<wxArrayInt> *Comparsion;
	
	SubsGrid(wxWindow *parent, const long int id ,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS, const wxString& name=wxPanelNameStr);
	virtual ~SubsGrid();
private:
	virtual void ContextMenu(const wxPoint &pos, bool dummy=false){};
	

	void CheckText(wxString text, wxArrayInt &errs);
protected:
	std::vector<bool> visibleLines;
	wxBitmap* bmp;
	wxFont font;
	int GridHeight;
	int GridWidth[13];
	int posY;
	int posX;
	int row;
	int scPos;
	int extendRow;
	int lastsel;
	int scHor;
	int oldX;
	short numsave;
	bool first;
	bool holding;
	bool hideover;
	bool ismenushown;
	kainoteFrame* Kai;

	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyPress(wxKeyEvent &event);
	void OnBcktimer(wxTimerEvent &event);
	void OnEraseBackground(wxEraseEvent &event){};
	void OnLostCapture(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();} holding=false;};
	wxTimer timer;
	wxTimer nullifyTimer;
	//HANDLE qtimer;

	DECLARE_EVENT_TABLE()
};

bool sortstart(Dialogue *i,Dialogue *j);

enum{
	LAYER=1,
	START=2,
	END=4,
	STYLE=8,
	ACTOR=16,
	MARGINL=32,
	MARGINR=64,
	MARGINV=128,
	EFFECT=256,
	CNZ=512,
	TXT=1024,
	TXTTL=2048,
	COMMENT=4096,
	ID_AUTIMER
};

#endif // SUBSGRID_H_INCLUDED
