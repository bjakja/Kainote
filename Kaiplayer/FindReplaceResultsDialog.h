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
#include "kainoteMain.h"

wxDECLARE_EVENT(CHOOSE_RESULT, wxCommandEvent);

class ResultsHeader : public Item
{
public:
	ResultsHeader(const wxString &text, int _startFilteredLine) : Item(TYPE_TEXT){
		name = text;
		firstFilteredLine = _startFilteredLine;
	}
	virtual ~ResultsHeader(){};
	void SetLastFilteredLine(int lastfiltered){ lastFilteredLine = lastfiltered; }
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
private:
	int firstFilteredLine = 0;
	int lastFilteredLine = 0;
	bool isVisible = true;
};

class SeekResults : public Item
{
public:
	SeekResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _keyLine, const wxString &_path) : Item(TYPE_TEXT){
		name = text;
		findPosition = pos;
		tab = _tab;
		path = _path;
		keyLine = _keyLine;
	}
	virtual ~SeekResults(){};
	TabPanel *tab = NULL;
	int keyLine;
private:
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	wxPoint findPosition;
	
	wxString path;
	
};

class FindReplaceDialog;

class FindReplaceResultsDialog : public KaiDialog
{
public:
	FindReplaceResultsDialog(wxWindow *parent, FindReplace *FR);
	virtual ~FindReplaceResultsDialog();
	void SetHeader(const wxString &text);
	void SetResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _keyLine, const wxString &_path);
	void ClearList();
private:
	KaiListCtrl *resultsList;
	ResultsHeader *header = NULL;
	int resultsCounter = 0;
};