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

#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#pragma once


#include "ListControls.h"
#include "KaiTextCtrl.h"
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiDialog.h"
#include <wx/wx.h>
class kainoteFrame;


class findreplace: public KaiDialog
{
	public:

		findreplace(kainoteFrame* kfparent, findreplace* last, bool replace, bool sellines=false);
		virtual ~findreplace();

        kainoteFrame* Kai;
		
		
		MappedButton* Button4;
		KaiRadioButton* RadioButton1;
		KaiRadioButton* RadioButton2;
		KaiRadioButton* RadioButton3;
		KaiRadioButton* RadioButton4;
		KaiRadioButton* RadioButton5;
		KaiRadioButton* RadioButton6;
		KaiRadioButton* RadioButton7;
		KaiRadioButton* RadioButton8;
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
		KaiCheckBox *Fdial;
		KaiCheckBox *Fcomm;
		KaiRadioBox *Actions;
		KaiRadioBox *Selections;
	
		void ReloadStyle();
		void SelectLine();
		void AddRecent();
		void OnStylesWin(wxCommandEvent& event);
		void OnSelections(wxCommandEvent& event);
		void OnStylesWin1(wxCommandEvent& event);
		void OnSetFocus(wxActivateEvent& event);
		void Reset();
		bool repl;
	private:
        int posrow;
		int reprow;
        int postxt;
		int findstart;
		int findend;
		wxString oldfind;
        bool fnext;
		
		bool fromstart;
		//bool hasFocus;
        void Find();
		
		void OnReplaceAll(wxCommandEvent& event);
		void OnButtonFind(wxCommandEvent& event);
		void OnButtonRep(wxCommandEvent& event);
		//void OnTextUpdate(wxCommandEvent& event);
		void OnClose(wxCommandEvent& event);
		void OnRecheck(wxCommandEvent& event);
};

enum
	{
	ID_BREP=13737,
	ID_BREPALL,
	ID_BFIND,
	ID_BCLOSE,
	ID_BPLUS,
	ID_TCSTYLE,
	ID_FINDTEXT,
	ID_REPTEXT,
	ID_SLINE,
	ID_ELINE
	};

#endif