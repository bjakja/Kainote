//  Copyright (c) 2021, Marcin Drob

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


#include "Styles.h"
//
//#include "TabPanel.h"
//#include "DialogueTextEditor.h"
//#include "EditBox.h"
//#include "SubsGrid.h"
#include <wx/msw/winundef.h>
#include <wx/string.h>
#include <wx/gdicmn.h>
#include <functional>
#include <wx/regex.h>

class TabPanel;

class FindData {
public:
	FindData() {};
	FindData(const wxString &textResult, const wxPoint &pos, bool inbracket, bool endsel) {
		finding = textResult;
		positionInText = pos;
		inBracket = inbracket;
		hasSelection = endsel;
	};
	wxPoint positionInText = wxPoint(0, 0);
	int cursorPosition = 0;
	bool inBracket = false;
	bool hasSelection = false;
	wxString finding;
};


class TagFindReplace
{
public:
	//empty tab == works only from 0, not selection
	TagFindReplace() {};
	TagFindReplace(TabPanel *_tab) : currentTab(_tab) {};
	void SetTabPanel(TabPanel* _tab) { currentTab = _tab; }
	//when founds nothing it returns place and inbracket that says if need to put in brackets or not
	//always for seeking first tag in cursor bracket
	bool FindTag(const wxString &pattern, 
		const wxString &text = wxString(L""),
		int mode = 0, bool toEndOfSelection = false);
	//replaces all values using normal regex seeking
	//cause it works only when tags exists else always return false
	//void (*function)(EventArg &)
	//void FindAllTags(const wxString& pattern, const wxString& text, std::function<void(const FindData&)> func, bool returnPosWhenNoTags = false);
	//no need to put in bracket function do it when needed
	//replace that's was captured don't put all tag when it was not taptured
	int ReplaceAll(const wxString& pattern, const wxString& tag, wxString *text, 
		std::function<void (const FindData &, wxString *)> func, bool returnPosWhenNoTags = false);
	int ReplaceAllByChar(const wxString& pattern, const wxString& tag, wxString* text, 
		std::function<void(const FindData&, wxString*, size_t numOfChars)> func);
	int Replace(const wxString& replaceTxt, wxString* txt);
	//int ReplaceFromFindData(const wxString& replaceTxt, const FindData& data);
	const FindData& GetResult() { return result; };
	bool GetDouble(double* retval);
	bool GetInt(int* retval);
	bool GetTwoValueInt(int* retval, int* retval2);
	bool GetTwoValueDouble(double* retval, double* retval2);
	bool GetTextResult(wxString* rettext);
	wxPoint GetPositionInText();
	//when need to change position for using tag placing functions
	void SetPositionInText(const wxPoint &pos, int inBracket = -1);
	//editbox function, put tag in text and reset it when selected
	//that's need reset tag that is given when seeking
	//full tag with value and full reset tag with value
	//restore selection is reserved
	void PutTagInText(const wxString& tag, const wxString& resettag, bool focus = true, bool restoreSelection = false);
	//function return 1 when need to add bracket or 0
	int ChangeText(wxString* txt, const wxString& what, bool inbracket, const wxPoint& pos);
	bool TagValueFromStyle(Styles* style, const wxString& tag, wxString *value);
	bool TagValueToStyle(Styles* style, const wxString& tag, const wxString& value);
	void SetFromTo(long _from, long _to) {
		from = _from;
		to = _to;
	};
	void SetResult(const FindData& res) {
		result = res;
	}
private:
	int ReplaceValue(wxString* txt, const wxString& what, const FindData& fdata);
	FindData result;
	wxRegEx regex;
	TabPanel* currentTab = nullptr;
	wxString lastPattern;
	wxPoint lastSelection;
	long from = 0, to = 0;

};

wxPoint FindBrackets(const wxString& text, long from);