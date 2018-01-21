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

#include "KaiTextCtrl.h"
#include "MappedButton.h"
#include "KaiCheckBox.h"
#include "KaiDialog.h"
#include "KaiListCtrl.h"

class kainoteFrame;
class TabPanel;

class SpellCheckerDialog : public KaiDialog
{
public:
	SpellCheckerDialog(kainoteFrame *parent);
	virtual ~SpellCheckerDialog(){};

private:
	wxString FindNextMisspell();
	void SetNextMisspell();
	void Replace(wxCommandEvent &evt);
	void ReplaceAll(wxCommandEvent &evt);
	void Ignore(wxCommandEvent &evt);
	void IgnoreAll(wxCommandEvent &evt);
	void AddWord(wxCommandEvent &evt);
	void RemoveWord(wxCommandEvent &evt);
	void OnSelectSuggestion(wxCommandEvent &evt);
	void OnActive(wxActivateEvent &evt);
	wxString GetRightCase(const wxString &replaceWord, const wxString &misspellWord);

	KaiTextCtrl *misSpell;
	KaiTextCtrl *replaceWord;
	KaiCheckBox *ignoreComments;
	KaiCheckBox *ignoreUpper;
	MappedButton *replace;
	MappedButton *replaceAll;
	MappedButton *ignore;
	MappedButton *ignoreAll;
	MappedButton *addWord;
	MappedButton *removeWord;
	MappedButton *close;
	KaiListCtrl *suggestionsList;

	int lastLine;
	int lastMisspell;
	int lastActiveLine;
	bool blockOnActive=false;
	wxArrayString ignored;
	wxArrayInt errors;
	wxString lastText;
	kainoteFrame *Kai;
	TabPanel *tab;
};

enum
{
	ID_SUGGESTIONS_LIST = 24315,
	ID_REPLACE,
	ID_REPLACE_ALL,
	ID_IGNORE,
	ID_IGNORE_ALL,
	ID_ADD_WORD,
	ID_REMOVE_WORD,
	ID_CLOSE_DIALOG
};