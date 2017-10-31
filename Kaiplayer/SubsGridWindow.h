//  Copyright (c) 2017, Marcin Drob

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

#include "KaiScrollbar.h"
#include "SubsDialogue.h"
#include "SubsFile.h"
#include <vector>
#include <set>

class EditBox;
class kainoteFrame;

class SubsGridWindow : public KaiScrolledWindow
{
public:
	SubsGridWindow(wxWindow *parent, const long int id, const wxPoint& pos, const wxSize& size, long style);
	virtual ~SubsGridWindow();
	void AdjustWidths(int cell = 8191);
	int CalcChars(const wxString &txt, wxString *lines = NULL, bool *bad = NULL);
	void ChangeTimeDisplay(bool frame);
	void CheckText(wxString text, wxArrayInt &errs);
	int FirstSel();
	void HideOverrideTags();
	void RefreshColumns(int cell = 8191);
	void RefreshIfVisible(int time);
	void ScrollTo(int y, bool center = false);
	void SelectRow(int row, bool addToSelected = false, bool select = true, bool norefresh = false);
	void SetStyle();
	void SetVideoLineTime(wxMouseEvent &evt);
	
	
	
	bool hasTLMode=true;
	bool showOriginal=true;
	char subsFormat = ASS;
	char originalFormat = ASS;
	int markedLine = 0;
	bool showFrames = false;
	std::vector<wxArrayInt> SpellErrors;
	std::vector<wxArrayInt> *Comparison;
	std::set<int> Selections;
	SubsFile* file;
	EditBox *Edit;
protected:
	void OnKeyPress(wxKeyEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnPaint(wxPaintEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent &event){};
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); } holding = false; };
	Dialogue * GetCheckedDialogue(int i);

	int GridHeight=0;
	int GridWidth[13];
	int posY=0;
	int posX=0;
	int row=0;
	int scPos=0;
	int scHor = 0;
	int extendRow=-1;
	int lastsel=-1;
	int oldX=-1;
	bool first;
	bool holding=false;
	bool hideOverrideTags;
	bool ismenushown=false;
	int lastRow=0;
	int visibleColumns;
	int panelrows=0;
	int lastActiveLine=0;

	kainoteFrame* Kai;
	std::vector<bool> visibleLines;
	std::vector<Dialogue*> filteredDialogues;
	wxBitmap* bmp;
	wxFont font;
private:
	virtual void ContextMenu(const wxPoint &pos, bool dummy = false){};
	virtual Dialogue * GetDialogue(int i){ return NULL; };
	virtual int GetCount(){ return 0; };
	virtual int FindStyle(wxString name, int *multiplication = NULL){ return 0; };
	virtual void MoveRows(int step, bool sav = false){};
	virtual void SetModified(unsigned char editionType, bool redit = true, bool dummy = false, int SetEditBoxLine = -1, bool Scroll = true){};
};

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