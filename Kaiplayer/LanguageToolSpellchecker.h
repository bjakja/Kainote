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

class DialogueErrors{
public:
	DialogueErrors();
	~DialogueErrors();
	void AppendError(RuleMatch *rule);
	void Clear();
};

class SubsGrid;
class Dialogue;

class LanguageToolSpellchecker{
public:
	LanguageToolSpellchecker(SubsGrid *_grid);
	~LanguageToolSpellchecker(){};
	void CheckLines(size_t from, size_t to);
private:
	void StripTags(Dialogue *dial);
	void GenerateErrors(std::vector<RuleMatch> &errors, size_t from, size_t to);

	wxString strippedText;
	ModuleLoader ml;
	LanguageToolModule *LTM = NULL;
	SubsGrid *grid = NULL;
	bool initializeFailed = false;
};

