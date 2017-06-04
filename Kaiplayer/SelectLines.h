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

#ifndef SELECTLINES_H
#define SELECTLINES_H


#include "ListControls.h"
#include "KaiTextCtrl.h"
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiDialog.h"

class kainoteFrame;


class SelectLines: public KaiDialog
{
public:

	SelectLines(kainoteFrame* kfparent);
	virtual ~SelectLines(){};

    kainoteFrame* Kai;

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
	void OnChooseStyles(wxCommandEvent& event);
	void AddRecent();
};

enum{
	ID_CHOOSE_STYLES=7090,
	ID_CLOSE_SELECTIONS,
	ID_SELECTIONS
};

#endif