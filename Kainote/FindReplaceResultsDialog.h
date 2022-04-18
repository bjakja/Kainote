//  Copyright (c) 2018-2020, Marcin Drob

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
#include "kainoteMain.h"
#include "MispellReplacerDialog.h"

wxDECLARE_EVENT(CHOOSE_RESULT, wxCommandEvent);


class ResultsHeader : public Item
{
public:
	ResultsHeader(const wxString &text/*, int _positionInTable*/) : Item(TYPE_HEADER){
		name = text;
		//positionInTable = _positionInTable;
		modified = true;
	}
	virtual ~ResultsHeader(){};
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = nullptr */);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	Item* Copy(){ return new ResultsHeader(*this); }
	wxSize GetTextExtents(KaiListCtrl *theList);
private:
	int positionInTable = 0;
	bool isVisible = true;
	bool enter = false;
};

class SeekResults : public Item
{
public:
	SeekResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _idLine, int _keyLine, 
		const wxString &_path, bool isTextTl = false) : Item(TYPE_TEXT){
		name = text;
		findPosition = pos;
		tab = _tab;
		path = _path;
		keyLine = _keyLine;
		idLine = _idLine;
		modified = true;
		isTextTL = isTextTl;
	}
	virtual ~SeekResults(){};
	TabPanel *tab = nullptr;
	wxString path;
	int keyLine;
	int idLine;
	bool isTextTL;
	wxPoint findPosition;
	wxSize GetTextExtents(KaiListCtrl *theList);
private:
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = nullptr */);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	Item* Copy(){ return new SeekResults(*this); }
	int OnVisibilityChange(int mode){
		if (mode == 1)
			return VISIBLE_BLOCK;
		else
			return NOT_VISIBLE;
	}
	bool enter = false;
};

class FindReplaceDialog;

class FindReplaceResultsDialog : public KaiDialog
{
public:
	FindReplaceResultsDialog(wxWindow *parent, FindReplace *FR, bool findInFiles = false);
	virtual ~FindReplaceResultsDialog();
	void SetHeader(const wxString &text, int thread);
	void SetResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, 
		int _idLine, int _keyLine, const wxString &_path, int thread, bool isTextTl = false);
	void ClearList();
	void FilterList();
	//use before run multithreading
	void SetupMultiThreading(int numThreads);
	//use after runing threads
	void EndMultiThreading();
	void CheckUncheckAll(bool check = true);
	void GetFindOptions(bool *_hasRegEx, bool *matchcase, bool *needPrefix, wxString *_findString){
		*_hasRegEx = hasRegEx;
		*_findString = findString;
		*matchcase = matchCase;
		*needPrefix = needToAddPrefix;
	}
	void SetFindOptions(bool _hasRegEx, bool matchcase, bool needPrefix, const wxString &_findString){
		hasRegEx = _hasRegEx;
		matchCase = matchcase;
		needToAddPrefix = needPrefix;
		findString = _findString;
	}
	void GetReplaceString(wxString *replaceString);
	KaiListCtrl *resultsList;
	bool findInFiles = false;
private:
	KaiChoice* ReplaceText;
	MappedButton *replaceChecked;
	//int resultsCounter = 0;
	//config for replace
	bool hasRegEx = false;
	bool matchCase = false;
	bool needToAddPrefix = false;
	//only for regex
	wxString findString;
	typedef std::vector<Item*> ItemList;
	ItemList *multiThreadList = nullptr;
	int multiThreadListSize = 0;
};