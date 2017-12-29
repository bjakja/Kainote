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
	size_t lastDialI = from;
	while (0 < errors.size()){
		if (errors[0]->FromPos < ltI){ errors.erase(errors.begin()); }
		else{ break; }
	}
	if (errors.size() < 1){ return; }
	RuleMatch * rule = errors[0];
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
				
				if (ltI == rule->FromPos){
					rule->FromPos = (hideTags) ? lineI + swapI : j;
					//tu jeszcze wstawiæ do tablicy grida;
					grid->SpellErrors[i]->AppendError(rule);
					lastDialI = i;
				}
				if (ltI == rule->EndPos){ 
					if (i != lastDialI){
						//pierwsze to trzeba mo¿e jakoœ zakoñczyæ nieszczêsnego b³êda 
						//w poprzedniej linii bo na koñcu te¿ mog¹ byæ tagi.
						//drugie zrobiæ kopiê rule bo orygina³ tkwi w poprzedniej linii.
						rule = rule->Copy();
						//trzecie wrzuciæ to do tablicy grida
						grid->SpellErrors[i]->AppendError(rule);
						//czwarte trzeba wrzuciæ do kopi jakiœ pocz¹tek, który bêdzie obejmowa³ now¹ liniê
						//tylko to trzeba chyba sprawdzaæ na koñcu linii by móc wyznaczyæ pocz¹tek od pierwszego znaku któy nei jest tagiem
					}
					rule->EndPos = (hideTags) ? lineI + swapI : j;
					errors.erase(errors.begin());
					if (errors.size() > 0)
						rule = errors[0];
					else
						return;
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

void Misspells::AppendError(RuleMatch *rule)
{
	errors.push_back(rule);
	isempty = false;
}

void Misspells::Clear()
{
	for (auto it = errors.begin(); it != errors.end(); it++){
		delete (*it);
	}
	errors.clear();
	isempty = true;
}

void Misspells::ClearBlock(std::vector<Misspells*> &table, size_t from, size_t to)
{
	auto end = table.begin() + (to + 1);
	for (auto it = table.begin() + from; it != table.end() && it != end; it++){
		Misspells* ms = *it;
		if(ms)
			ms->Clear();
	}
}

bool Misspells::GetMesures(size_t i, const wxString &text, wxDC &dc, wxPoint &retPos){
	if (i >= errors.size())
		return false;
	size_t txtLen = text.Len();
	RuleMatch *rule = errors[i];
	if (rule->FromPos >= txtLen)
		return false;
	int fh;
	wxString err = text.SubString(rule->FromPos, rule->EndPos);
	err.Trim();
	if (rule->FromPos > 0){
		wxString berr = text.Mid(0, rule->FromPos);
		dc.GetTextExtent(berr, &retPos.x, NULL);
	}
	else{ retPos.x = 0; }
	dc.GetTextExtent(err, &retPos.y, NULL);
	return true;
}