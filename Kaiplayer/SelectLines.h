//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include "ListControls.h"
#include "KaiTextCtrl.h"
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiDialog.h"

class KainoteFrame;
class TabPanel;


class SelectLines: public KaiDialog
{
public:

	SelectLines(KainoteFrame* kfparent);
	virtual ~SelectLines(){};
	void SaveOptions();

	KainoteFrame* Kai;

	KaiRadioButton* Contains;
	KaiRadioButton* NotContains;
	KaiRadioButton* CollumnText;
	KaiRadioButton* CollumnStyle;
	KaiRadioButton* CollumnActor;
	KaiRadioButton* CollumnEffect;
	KaiRadioButton* CollumnStartTime;
	KaiRadioButton* CollumnEndTime;
	KaiRadioBox *Actions;
	KaiRadioBox *Selections;
	KaiChoice* FindText;
	KaiCheckBox* MatchCase;
	KaiCheckBox* RegEx;
	KaiCheckBox* Dialogues;
	KaiCheckBox* Comments;
	MappedButton *ChooseStyles;
	MappedButton *Select;
	MappedButton *Close;

private:
	void OnSelect(wxCommandEvent& event);
	void OnSelectInAllTabs(wxCommandEvent& event);
	int SelectOnTab(TabPanel *tab, bool *refreshTabLabel);
	void OnChooseStyles(wxCommandEvent& event);
	void AddRecent();
	wxArrayString selsRecent;
	long selectColumn;
	bool matchcase;
	bool regex;
	bool contain;
	bool notcont;
	bool selectDialogues;
	bool selectComments;
	int selectOptions;
	int action;
	wxString find;

};

enum{
	CONTAINS = 1,
	NOT_CONTAINS,
	MATCH_CASE = 1 << 2,
	REGULAR_EXPRESSIONS = 1 << 3,
	FIELD_TEXT = 1 << 4,
	FIELD_STYLE = 1 << 5,
	FIELD_ACTOR = 1 << 6,
	FIELD_EFFECT = 1 << 7,
	FIELD_START_TIME = 1 << 8,
	FIELD_END_TIME = 1 << 9,
	DIALOGUES = 1 << 10,
	COMMENTS = 1 << 11,
	SELECT = 1 << 12,
	ADD_TO_SELECTION = 1 << 13,
	DESELECT = 1 << 14,
	DO_NOTHING = 1 << 15,
	DO_COPY = 1 << 16,
	DO_CUT = 1 << 17,
	DO_MOVE_ON_START = 1 << 18,
	DO_MOVE_ON_END = 1 << 19,
	DO_SET_ASS_COMMENT = 1 << 20,
	DO_DELETE = 1 << 21,
	ID_CHOOSE_STYLES=7090,
	ID_CLOSE_SELECTIONS,
	ID_SELECTIONS,
	ID_SELECT_ON_ALL_TABS
};

