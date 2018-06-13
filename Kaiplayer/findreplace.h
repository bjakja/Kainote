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
#include <wx/regex.h>
#include "FindReplaceDialog.h"

class kainoteFrame;


class FindReplace
{
	friend class TabWindow;
	public:

		FindReplace(kainoteFrame* kfparent, bool replace);
		virtual ~FindReplace(){};
		void SaveOptions();
	
		void ReloadStyle();
		void AddRecent(TabWindow *window);
		void ChangeContents(bool replace);
		void OnStylesWin(wxCommandEvent& event);
		void OnSetFocus(wxActivateEvent& event);
		void Reset();
	private:
		kainoteFrame *Kai;
        int linePosition;
		int reprow;
        int textPosition;
		int findstart;
		int findend;
		int lastActive;
		wxString oldfind;
        bool fnext;
		
		bool fromstart;
		bool blockTextChange;
		bool findTextReset = false;
		bool wasResetToStart = false;
		wxArrayString findRecent;
		wxArrayString replaceRecent;
		wxArrayString subsFindingFilters;
		wxArrayString subsFindingPaths;
		wxRegEx rgx;
		
		void Find(TabWindow *window);
		void OnFind(TabWindow *window);
		void FindInAllOpenedSubs(TabWindow *window);
		void FindAllInCurrentSubs(TabWindow *window);
		void FindInSubs(TabWindow *window);
		void Replace(TabWindow *window);
		void ReplaceAll(TabWindow *window);
		void ReplaceInAllOpenedSubs(TabWindow *window);
		void ReplaceInSubs(TabWindow *window);
		//void OnRecheck(wxCommandEvent& event);
		//void OnEnterConfirm(wxCommandEvent& event);
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
	ID_ENTER_CONFIRM
};

