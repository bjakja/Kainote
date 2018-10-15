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
#include "KaiCheckBox.h"
#include "MappedButton.h"
#include "KaiListCtrl.h"
#include "KaiTextCtrl.h"
#include "ListControls.h"
#include "MispellReplacerDialog.h"
#include <vector>
#include <utility>

class TabPanel;


class MisspellReplacer : public KaiDialog
{
public:
	MisspellReplacer(wxWindow *parent);
	virtual ~MisspellReplacer();
	void ReplaceChecked();
	void ShowResult(TabPanel *tab, int keyLine);
	void RemoveDialog(){ resultDialog = NULL; };
private:
	typedef std::pair<wxString, wxString> Rule;

	void FillRulesList();
	void EditRule();
	void AddRule();
	void RemoveRule();
	void SeekOnce();
	void SeekOnTab(TabPanel *tab);
	void SeekOnActualTab();
	void SeekOnAllTabs();
	void ReplaceOnce();
	void ReplaceOnTab(TabPanel *tab);
	void ReplaceOnActualTab();
	void ReplaceOnAllTabs();
	void GetCheckedRules(std::vector<int> &checkedRules);
	void SaveRules();
	void MoveCase(const wxString &originalCase, wxString *result);
	KaiCheckBox *PutWordBoundary;
	KaiCheckBox *ShowBuiltInRules;
	KaiTextCtrl *PhraseToFind;
	KaiTextCtrl *PhraseToReplace;
	KaiTextCtrl *ChoosenStyles;
	KaiChoice *WhichLines;
	KaiListCtrl *RulesList;

	FindResultDialog *resultDialog;
	std::vector<Rule> rules;
};

enum{
	ID_PUT_WORD_BOUNDARY = 6000,
	ID_SHOW_BUILT_IN_RULES,
	ID_PHRASE_TO_FIND,
	ID_PHRASE_TO_REPLACE,
	ID_RULES_LIST,
	ID_WHICH_LINES_LIST,
	ID_STYLES_CHOOSE,
	ID_ADD_RULE,
	ID_EDIT_RULE,
	ID_REMOVE_RULE,
	ID_FIND_RULE,
	ID_FIND_ALL_RULES,
	ID_FIND_ALL_RULES_ON_ALL_TABS,
	ID_REPLACE_RULE,
	ID_REPLACE_ALL_RULES,
	ID_REPLACE_ALL_RULES_ON_ALL_TABS

};