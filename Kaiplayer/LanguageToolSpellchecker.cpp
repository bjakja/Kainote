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

#include "SubsGrid.h"
#include "LanguageToolSpellchecker.h"

LanguageToolSpellchecker::LanguageToolSpellchecker(SubsGrid *_grid)
	:grid(_grid)
{
	
}

void LanguageToolSpellchecker::CheckLines(size_t from, size_t to)
{
	if (initializeFailed){ return; }
	if (!LTM){
		LTM = ml.GetModule();
		if (LTM && LTM->init() && LTM->setLanguage("Polish")){
			initializeFailed = false;
		}
		else{
			initializeFailed = true;
		}
	}
	strippedText.Empty();
	size_t size = grid->GetCount();
	size_t ltfrom = (from > 0) ? from-1 : 0;
	size_t ltto = (to < size-1) ? to + 1 : size-1;
	size_t ltstart = 0;
	File *file = grid->file->GetSubs();
	for (size_t i = ltfrom; i <= ltto; i++){
		Dialogue *dial = file->dials[i];
		if (!dial->isVisible){ continue; }
		StripTags(dial);
		if (i < from){ ltstart = strippedText.Len(); }
	}
	if (strippedText.empty())
		return;

	std::vector<RuleMatch *> errors;
	LTM->checkText(strippedText.mb_str(wxConvUTF8).data(), errors);
	GenerateErrors(errors, from, to, ltstart);
}

void LanguageToolSpellchecker::StripTags(Dialogue *dial)
{
	bool block;
	int lastI = -1;
	wxString text = dial->Text.CheckTl(dial->TextTl, dial->TextTl != "" && grid->hasTLMode);
	size_t len = text.Len();
	for (size_t i = 0; i < len; i++)
	{
		const wxUniChar &ch = text[i];
		if (ch == '{' && lastI >= 0){ strippedText << text.SubString(lastI, i - 1); block = true; }
		else if (ch == '}'){ lastI = i + 1; block = false; }
		else if (ch == '\\' && !block && i < len - 1){
			const wxUniChar &chp1 = text[i+1];
			if (chp1 == 'N' || chp1 == 'n'){ text.replace(i, i + 1, "\r\n"); }
			else if (chp1 == 'h'){ text.replace(i, i + 1, ""); }
		}
		else if (i == len - 1 && !block){
			if (lastI < 0){ strippedText << text; }
			else{
				strippedText << text.SubString(lastI, i);
			}
		}
	}
	strippedText << "\r\n";
}

void LanguageToolSpellchecker::GenerateErrors(std::vector<RuleMatch*> &errors, size_t from, size_t to, size_t ltstart)
{
	// jeszcze trzeba napisaæ synchronizacjê, a póŸniej wywaliæ b³êdy z from-1 ltI trzeba nadaæ pocz¹tek 2 linijki.
	bool hideTags = grid->hideOverrideTags;
	size_t swapLen = Options.GetString(GridTagsSwapChar).Len();
	File *file = grid->file->GetSubs();
	size_t ltI = ltstart;
	size_t swapI = 0;
	size_t lineI = 0;
	while (0 < errors.size()){
		if (errors[0]->FromPos < ltI){ errors.erase(errors.begin()); }
		else{ break; }
	}
	for (size_t i = from; i <= to; i++){
		Dialogue *dial = file->dials[i];
		if (!dial->isVisible){ continue; }
		bool block = false;
		const wxString &text = dial->Text.CheckTlRef(dial->TextTl, dial->TextTl != "" && grid->hasTLMode);
		for (size_t j = 0; j < text.Len(); j++)
		{
			const wxUniChar &ch = text[j];
			if (ch == '{'){ swapI += swapLen; block = true; }
			else if (ch == '}'){ block = false; }
			else if (!block){
				RuleMatch * rule = errors[0];
				if (ltI == rule->FromPos){
					rule->FromPos = (hideTags) ? lineI + swapI : j;
					//tu jeszcze wstawiæ do tablicy grida;
				}
				if (ltI == rule->EndPos){ 
					rule->EndPos = (hideTags) ? lineI + swapI : j;
					errors.erase(errors.begin());
				}
				ltI++;
				lineI++;
			}
		}
		swapI = 0;
		lineI = 0;
		ltI += 2;
	}
}
