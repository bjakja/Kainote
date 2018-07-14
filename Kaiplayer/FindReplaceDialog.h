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
class KainoteFrame;

class TabWindow : public wxWindow
{
	friend class FindReplace;
public:
	TabWindow(wxWindow *parent, int id, int tabNum, FindReplace * FR);
	virtual ~TabWindow(){};
	void SaveValues();
	void SetValues();

	void OnRecheck(wxCommandEvent& event);
	void Reset(wxCommandEvent& evt);
	void OnStylesChoose(wxCommandEvent& event);
	KaiChoice* FindText;
	KaiChoice* ReplaceText = NULL;
	KaiChoice* FindInSubsPattern = NULL;
	KaiChoice* FindInSubsPath = NULL;
	KaiRadioButton* CollumnText;
	KaiRadioButton* CollumnTextOriginal;
	KaiRadioButton* CollumnStyle;
	KaiRadioButton* CollumnActor;
	KaiRadioButton* CollumnEffect;
	KaiRadioButton* AllLines = NULL;
	KaiRadioButton* SelectedLines = NULL;
	KaiRadioButton* FromSelection = NULL;
	KaiTextCtrl *ChoosenStyleText = NULL;
	KaiCheckBox* MatchCase;
	KaiCheckBox* RegEx;
	KaiCheckBox* StartLine;
	KaiCheckBox* EndLine;
	KaiCheckBox *SeekInSubFolders = NULL;
	KaiCheckBox *SeekInHiddenFolders = NULL;
	FindReplace *FR;
	int windowType = 0;
};

class FindReplaceDialog : public KaiDialog
{
	friend class FindReplace;
public:
	FindReplaceDialog(KainoteFrame *Kai, int whichWindow);
	virtual ~FindReplaceDialog();
	void ShowDialog(int whichWindow);
	void SaveOptions();
	void Reset();
private:
	void OnActivate(wxActivateEvent& event);
	void OnEnterConfirm(wxCommandEvent& event);
	void SetSelection(TabWindow *tab);
	TabWindow *GetTab();
	FindReplace *FR = NULL;
	KainoteFrame *Kai = NULL;
	KaiTabBar * findReplaceTabs = NULL;
	int lastFocusedId = -1;
};

enum{
	WINDOW_FIND = 0,
	WINDOW_REPLACE,
	WINDOW_FIND_IN_SUBS,
	ID_BUTTON_REPLACE = 13737,
	ID_BUTTON_REPLACE_ALL,
	ID_BUTTON_REPLACE_IN_ALL_OPENED_SUBS,
	ID_BUTTON_FIND,
	ID_BUTTON_FIND_IN_ALL_OPENED_SUBS,
	ID_BUTTON_FIND_ALL_IN_CURRENT_SUBS,
	ID_BUTTON_FIND_IN_SUBS,
	ID_BUTTON_REPLACE_IN_SUBS,
	ID_BUTTON_CLOSE,
	ID_BUTTON_CHOOSE_STYLE,
	ID_CHOOSEN_STYLE_TEXT,
	ID_FIND_TEXT,
	ID_REPLACE_TEXT,
	ID_START_OF_LINE,
	ID_END_OF_LINE,
	ID_ENTER_CONFIRM
};