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

#include <wx/string.h>
#include "LanguageToolLoader.h"

class Misspells{
public:
	Misspells(){};
	~Misspells(){ Clear(); };
	void AppendError(RuleMatch *rule);
	void Clear();
	static void ClearBlock(std::vector<Misspells*> &table, size_t from, size_t to);
	size_t size() const{
		return errors.size();
	}
	bool empty() const{
		return isempty;
	}
	RuleMatch *GetError(size_t i){ 
		if (i < errors.size())
			return errors[i];
		return NULL;
	}
	void GetErrorString(size_t i, const wxString& text, wxString &error);
	bool GetMesures(size_t i, const wxString &text, wxDC &dc, wxPoint &retPos, bool &isMisspell);
	short CPS = -1;
	bool isempty = true;
private:
	std::vector<RuleMatch*> errors;
};

class SubsGrid;
class Dialogue;

class LanguageToolSpellchecker{
public:
	static LanguageToolSpellchecker *Get(SubsGrid *_grid);
	static void DestroyLTSpellchecker();
	~LanguageToolSpellchecker(){};
	void CheckLines(size_t from, size_t to);
private:
	LanguageToolSpellchecker(SubsGrid *_grid);
	void StripTags(Dialogue *dial);
	void GenerateErrors(std::vector<RuleMatch*> &errors, size_t from, size_t to, size_t ltstart);

	wxString strippedText;
	ModuleLoader ml;
	LanguageToolModule *LTM = NULL;
	SubsGrid *grid = NULL;
	bool initializeFailed = false;
	static LanguageToolSpellchecker *LTSC;
};

