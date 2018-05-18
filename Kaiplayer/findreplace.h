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

#include "ListControls.h"
#include "KaiTextCtrl.h"
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiDialog.h"
#include "KaiStaticBoxSizer.h"
#include "KaiStaticText.h"

class kainoteFrame;


class FindReplace: public KaiDialog
{
	public:

		FindReplace(kainoteFrame* kfparent, bool replace);
		virtual ~FindReplace(){};
		void SaveOptions();

        kainoteFrame* Kai;
		
		MappedButton* Button4;
		KaiRadioButton* CollumnText;
		KaiRadioButton* CollumnTextOriginal;
		KaiRadioButton* CollumnStyle;
		KaiRadioButton* AllLines;
		KaiRadioButton* SelectedLines;
		KaiRadioButton* CollumnActor;
		KaiRadioButton* CollumnEffect;
		KaiRadioButton* FromSelection;
		MappedButton* Button1;
		MappedButton* Bplus;
		
		KaiCheckBox* MatchCase;
		KaiCheckBox* RegEx;
		KaiCheckBox* StartLine;
		KaiCheckBox* EndLine;
		MappedButton* Button2;
		MappedButton* Button3;
		KaiChoice* FindText;
		KaiChoice* RepText;
		KaiTextCtrl* tcstyle;
		//KaiStaticBoxSizer* ReplaceStaticSizer;
		KaiStaticText *repDescText;
		DialogSizer* mainfrbsizer;
	
		void ReloadStyle();
		void AddRecent();
		void ChangeContents(bool replace);
		void OnStylesWin(wxCommandEvent& event);
		void OnSetFocus(wxActivateEvent& event);
		void Reset();
		bool repl;
	private:
        int posrow;
		int reprow;
        int postxt;
		int findstart;
		int findend;
		int lastActive;
		wxString oldfind;
        bool fnext;
		
		bool fromstart;
		bool blockTextChange;
		bool findTextReset = false;
		wxArrayString findRecent;
		wxArrayString replaceRecent;
		
        void Find();
		
		void OnReplaceAll(wxCommandEvent& event);
		void OnButtonFind(wxCommandEvent& event);
		void OnButtonRep(wxCommandEvent& event);
		void OnClose(wxCommandEvent& event);
		void OnRecheck(wxCommandEvent& event);
		void OnEnterConfirm(wxCommandEvent& event);
};

enum
{
	CASE_SENSITIVE = 1,
	REG_EX,
	START_OF_TEXT = 4,
	END_OF_TEXT = 8,
	IN_FIELD_TEXT = 16,
	IN_FIELD_TEXT_ORIGINAL = 32,
	IN_FIELD_STYLE = 64,
	IN_FIELD_ACTOR = 128,
	IN_FIELD_EFFECT = 256,
	IN_LINES_ALL = 512,
	IN_LINES_SELECTED = 1024,
	IN_LINES_FROM_SELECTION = 2048,
	ID_BREP=13737,
	ID_BREPALL,
	ID_BFIND,
	ID_BCLOSE,
	ID_BPLUS,
	ID_TCSTYLE,
	ID_FINDTEXT,
	ID_REPTEXT,
	ID_SLINE,
	ID_ELINE,
	ID_ENTER_CONFIRM
};

