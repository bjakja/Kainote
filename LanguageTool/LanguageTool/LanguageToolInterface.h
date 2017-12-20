//  Copyright (c) 2017, Marcin Drob

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

#include <list>
#include <vector>
#include <wx/string.h>
using namespace std;

class RuleMatch{
public:
	wxString message;
	int FromPos;
	int EndPos;
	list<wxString> SuggestedReplacements;
	RuleMatch();
	RuleMatch(const wxString &mes, int from, int to) : message(mes), FromPos(from), EndPos(to){}
};

class LanguageToolModule{
public:
	LanguageToolModule(){};
	virtual ~LanguageToolModule(){};
	virtual void checkText(const wxString &text_to_check, vector<RuleMatch> &result){};
	virtual bool init(){ return false; };
	virtual void getLanguages(vector<wxString> &languages){};
	virtual bool setLanguage(const wxString &language){ return false; };
};

