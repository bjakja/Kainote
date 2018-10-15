//  Copyright (c) 2018, Marcin Drob

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

#include "KaiDialog.h"
#include "KaiListCtrl.h"


wxDECLARE_EVENT(CHOOSE_RESULT, wxCommandEvent);

enum{
	TYPE_HEADER = 3,
	ID_CHECK_ALL = 6600,
	ID_UNCHECK_ALL,
	ID_REPLACE_CHECKED
};

class TabPanel;

class ReplacerResultsHeader : public Item
{
public:
	ReplacerResultsHeader(const wxString &text, int _positionInTable) : Item(TYPE_HEADER){
		name = text;
		positionInTable = _positionInTable;
	}
	virtual ~ReplacerResultsHeader(){};
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	Item* Copy(){ return new ReplacerResultsHeader(*this); }
private:
	int positionInTable = 0;
	bool isVisible = true;
	bool enter = false;
};

class ReplacerSeekResults : public Item
{
public:
	ReplacerSeekResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _keyLine, int _idLine , int _numOfRule/*const wxString &_path*/) : Item(TYPE_TEXT){
		name = text;
		findPosition = pos;
		tab = _tab;
		//path = _path;
		idLine = _idLine;
		keyLine = _keyLine;
		numOfRule = _numOfRule;
		modified = true;
	}
	virtual ~ReplacerSeekResults(){};
	TabPanel *tab = NULL;
	//wxString path;
	int idLine;
	int keyLine;
	int numOfRule;
	wxPoint findPosition;
private:
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	Item* Copy(){ return new ReplacerSeekResults(*this); }
	int OnVisibilityChange(int mode){
		if (mode == 1)
			return VISIBLE_BLOCK;
		else
			return NOT_VISIBLE;
	}
	bool enter = false;
};

class MisspellReplacer;

class FindResultDialog : public KaiDialog
{
public:
	FindResultDialog(wxWindow *parent, MisspellReplacer *MR);
	virtual ~FindResultDialog(){};
	void SetHeader(const wxString &text);
	void SetResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _keyLine, int idLine, int numOfRule);
	void ClearList();
	void CheckUncheckAll(bool check = true);
	KaiListCtrl *ResultsList;
private:
	int resultsCounter = 0;
	MisspellReplacer *MR;
};

