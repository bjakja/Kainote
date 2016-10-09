#ifndef SUBSGRID_H_INCLUDED
#define SUBSGRID_H_INCLUDED

#pragma once

#include <wx/wx.h>
#include "SubsDialogue.h"
#include <vector>
#include <map>
#include "Styles.h"
#include "SubsFile.h"

class EditBox;
class kainoteFrame;

class SubsGrid : public wxWindow
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
	int CalcChars(wxString txt, wxString *lines=NULL, bool *bad=NULL);
	void ChangeLine(Dialogue *line1, int wline, long cells, bool selline=false, bool dummy=false);
	void ChangeCell(long cells, int wline, Dialogue *what);
	void ChangeStyle(Styles *nstyl,int i);
	void Clearing();
	void Convert(char type);

	int FindStyle(wxString name,int *multiplication=NULL);
	wxString GetStyles(bool tld=false);
	Styles *GetStyle(int i,wxString name=_(""));
	std::vector<Styles*> *GetStyleTable();
	void SetStyle();
	void RepaintWindow(int cell=8191);
	void SelectRow(int row, bool addToSelected = false, bool select=true, bool norefresh=false);
	void ScrollTo(int y, bool center=false);
	
	void SaveFile(wxString filename, bool cstat=true);
	wxString *SaveText();
	void HideOver();
	
	
	int StylesSize();
	
	
	void DelStyle(int i);
	
	Dialogue *GetDialCor(int ii);
	void ChangeTime();
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
	void AddSInfo(wxString SI, wxString val="", bool save=true);
	void SetModified(bool redit=true, bool dummy=false, int SetEditBoxLine = -1);
	void UpdateUR(bool tolbar=true);
	wxString GetSInfos(bool tld=false);
	wxString GetSInfo(wxString key, int *ii=0);
	SInfo *GetSInfoP(wxString key, int *ii=0);
	wxArrayInt GetSels(bool deselect=false);
	void SwapRows(int frst, int scnd, bool sav=false);
	void Loadfile(wxString str,wxString ext);
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
	bool IsNum(wxString txt);
	Dialogue *CopyDial(int i, bool push=true);
	Dialogue *GetDial(int i);
	wxString *GetVisible(bool *visible=0, wxPoint *point=0, bool trimSels=false);

	bool makebkp;
	bool showFrames;
	int lastRow;
	int visible;
	int mtimerow;
	SubsFile* file;
	std::vector<wxArrayInt> SpellErrors;
	std::vector<wxArrayInt> *Comparsion;

	SubsGrid(wxWindow *parent, const long int id ,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS, const wxString& name=wxPanelNameStr);
	virtual ~SubsGrid();
private:
	virtual void ContextMenu(const wxPoint &pos, bool dummy=false){};
	

	void CheckText(wxString text, wxArrayInt &errs);
protected:
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
	void DrawGrid(wxDC &dc,int w, int h);
	void OnBcktimer(wxTimerEvent &event);
	wxTimer timer;
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
