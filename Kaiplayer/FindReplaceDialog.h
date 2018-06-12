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
#include "ListControls.h"
#include "KaiTextCtrl.h"
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiTabBar.h"
#include "KaiStaticText.h"

class FindReplace;
//It's faster make one tab with showing/hiding elements then change its values everytime
class TabWindow : public wxWindow
{
public:
	TabWindow(wxWindow *parent, int id, int tabNum, FindReplace * FR);
	virtual ~TabWindow();
	void SaveValues(int tabNum);
	void SetValues(int tabNum);

	KaiChoice* FindText;
	KaiChoice* ReplaceText;
	KaiChoice* FindInSubsPattern;
	KaiChoice* FindInSubsPath;
	KaiRadioButton* CollumnText;
	KaiRadioButton* CollumnTextOriginal;
	KaiRadioButton* CollumnStyle;
	KaiRadioButton* CollumnActor;
	KaiRadioButton* CollumnEffect;
	KaiRadioButton* AllLines;
	KaiRadioButton* SelectedLines;
	KaiRadioButton* FromSelection;
	MappedButton* ChooseStyles;
	KaiTextCtrl* ChooseStylesText;
	KaiCheckBox* MatchCase;
	KaiCheckBox* RegEx;
	KaiCheckBox* StartLine;
	KaiCheckBox* EndLine;
};

class FindReplaceDialog : public KaiDialog
{
public:
	FindReplaceDialog(wxWindow *parent, int id);
	virtual ~FindReplaceDialog();
};