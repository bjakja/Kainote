//  Copyright (c) 2020, Marcin Drob

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

#include "LineParse.h"
#include "SpellChecker.h"
#include "SubsDialogue.h"

void TextData::Init(const wxString &text, bool spellchecker, int subsFormat, int tagReplaceLen) {
	if (isInit)
		return;

	SpellChecker::Get()->CheckTextAndBrackets(text, this, spellchecker, subsFormat, NULL, tagReplaceLen);
	isInit = true;
}

void TextData::Init2(const wxString & text, bool spellchecker, int subsFormat, wxArrayString * misspels)
{
	SpellChecker::Get()->CheckTextAndBrackets(text, this, spellchecker, subsFormat, misspels, -1);
}

int TextData::GetCPS(Dialogue *line) const
{
	int characterTime = chars / ((line->End.mstime - line->Start.mstime) / 1000.0f);
	if (characterTime < 0 || characterTime > 999) { characterTime = 999; }
	return characterTime;
}
