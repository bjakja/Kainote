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

LanguageToolSpellchecker *LanguageToolSpellchecker::LTSC = NULL;

LanguageToolSpellchecker::LanguageToolSpellchecker(SubsGrid *_grid)
	:grid(_grid)
{
	
}

LanguageToolSpellchecker * LanguageToolSpellchecker::Get(SubsGrid *_grid)
{
	if (!LTSC){ LTSC = new LanguageToolSpellchecker(_grid); }
	return LTSC;
}

void LanguageToolSpellchecker::DestroyLTSpellchecker()
{
	if (LTSC){ delete LTSC; LTSC = NULL; }
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
	GenerateErrors(errors, ltfrom, ltto, 0);
}

void LanguageToolSpellchecker::StripTags(Dialogue *dial)
{
	bool block = false;
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
			if (chp1 == 'N' || chp1 == 'n'){ text.replace(i, 2, "\r\n"); }
			else if (chp1 == 'h'){ text.replace(i, 2, ""); len -= 2; }
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
	//size_t lastDialI = from;
	//while (0 < errors.size()){
		//if (errors[0]->FromPos < ltI){ errors.erase(errors.begin()); }
		//else{ break; }
	//}
	if (errors.size() < 1){ return; }
	RuleMatch * lastrule = NULL;
	RuleMatch * rule = errors[0];
	bool setEndTime = false;
	bool setStartTime = false;
	bool needSkip = false;
	for (size_t i = from; i <= to; i++){
		Dialogue *dial = file->dials[i];
		if (!dial->isVisible){ continue; }
		if (!grid->SpellErrors[i])
			grid->SpellErrors[i] = new Misspells();
		if (!grid->SpellErrors[i]->isempty){ 
			needSkip = true; 
		}
		else{
			needSkip = false;
			grid->SpellErrors[i]->isempty = false;
		}
		bool block = false;
		const wxString &text = dial->Text.CheckTlRef(dial->TextTl, dial->TextTl != "" && grid->hasTLMode);
		for (size_t j = 0; j < text.Len(); j++)
		{
			const wxUniChar &ch = text[j];
			if (ch == '{'){ swapI += swapLen; block = true; }
			else if (ch == '}'){ block = false; }
			else if (!block){
				//if (setStartTime){
				//	//pierwsze to trzeba mo¿e jakoœ zakoñczyæ nieszczêsnego b³êda 
				//	//w poprzedniej linii bo na koñcu te¿ mog¹ byæ tagi.
				//	/*Tu nie robimy nic, to jest zrobione przy setEndTime*/
				//	//drugie zrobiæ kopiê rule bo orygina³ tkwi w poprzedniej linii.
				//	if (!needSkip){
				//		//trzecie wrzuciæ to do tablicy grida
				//		grid->SpellErrors[i]->AppendError(rule);
				//		//czwarte trzeba wrzuciæ do kopi jakiœ pocz¹tek, który bêdzie obejmowa³ now¹ liniê
				//		//tylko to trzeba chyba sprawdzaæ na koñcu linii, by móc wyznaczyæ pocz¹tek od pierwszego znaku który nie jest tagiem
				//		rule->FromPos = (hideTags) ? lineI + swapI : j;
				//	}
				//	setStartTime = false;
				//}
				
				if (ltI == rule->EndPos){ 
					if (ltI == rule->FromPos && !needSkip && !setEndTime){
						rule->FromPos = (hideTags) ? lineI + swapI : j;
						//tu jeszcze wstawiæ do tablicy grida;
						grid->SpellErrors[i]->AppendError(rule);
					}
					rule->EndPos = (hideTags) ? lineI + swapI : j;
					lastrule = rule;
					if (needSkip && !setEndTime)
						delete rule;

					errors.erase(errors.begin());
					setEndTime = false;
					if (errors.size() > 0)
						rule = errors[0];
					else{
						return;
					}
				}
				if (ltI == rule->FromPos && !setEndTime){
					if (!needSkip){
						rule->FromPos = (hideTags) ? lineI + swapI : j;
						//tu jeszcze wstawiæ do tablicy grida;
						grid->SpellErrors[i]->AppendError(rule);
					}
					setEndTime = true;
				}
				ltI++;
				lineI++;
			}
		}
		if (rule->FromPos >= ltI && rule->FromPos < ltI + 2){
			rule->FromPos = ltI + 2;
		}
		if (setEndTime){
			if (rule->EndPos < ltI+2){
				rule->EndPos = (hideTags) ? lineI + swapI : text.Len();
				lastrule = rule;
				if (needSkip && !setEndTime)
					delete rule;

				setEndTime = false;
				errors.erase(errors.begin());
				if (errors.size() > 0)
					rule = errors[0];
				else{
					return;
				}
			}
			//pierwsze to trzeba mo¿e jakoœ zakoñczyæ nieszczêsnego b³êda 
			//w poprzedniej linii bo na koñcu te¿ mog¹ byæ tagi.
			//grid->SpellErrors[i]->GetError(grid->SpellErrors[i]->size()-1)->EndPos 
			
			//drugie zrobiæ kopiê rule bo orygina³ tkwi w poprzedniej linii.
			/*Tu nie robimy nic, to jest zrobione przy setEndTime*/
			//trzecie wrzuciæ to do tablicy grida
			/*Tu nie robimy nic, to jest zrobione przy setStartTime*/
			//czwarte trzeba wrzuciæ do kopi jakiœ pocz¹tek, który bêdzie obejmowa³ now¹ liniê
			//tylko to trzeba chyba sprawdzaæ na koñcu linii by móc wyznaczyæ pocz¹tek od pierwszego znaku któy nei jest tagiem
			/*Tu nie robimy nic, to jest zrobione przy setStartTime*/
			
			
		}
		swapI = 0;
		lineI = 0;
		ltI += 2;
	}
	
	if (errors.size() > 0){
		bool thisIsBad = true;
	}
}

void Misspells::AppendError(RuleMatch *rule)
{
	errors.push_back(rule);
	isempty = false;
}

void Misspells::Clear()
{
	for (size_t i = 0; i < errors.size(); i++){
		delete errors[i];
	}
	errors.clear();
	CPS = -1;
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

void Misspells::GetErrorString(size_t i, const wxString& text, wxString &error)
{
	if (i >= errors.size())
		return;
	size_t txtLen = text.Len();
	RuleMatch *rule = errors[i];
	if (rule->FromPos >= txtLen)
		return;
	error = text.SubString(rule->FromPos, rule->EndPos);
}

bool Misspells::GetMesures(size_t i, const wxString &text, wxDC &dc, wxPoint &retPos, bool &isMisspell){
	if (i >= errors.size())
		return false;
	size_t txtLen = text.Len();
	RuleMatch *rule = errors[i];
	if (rule->FromPos >= txtLen)
		return false;
	wxString err;
	if (rule->FromPos > rule->EndPos)
		err = text.SubString(rule->FromPos, text.Len());
	else
		err = text.SubString(rule->FromPos, rule->EndPos-1);
	err.Trim();
	if (rule->FromPos > 0){
		wxString berr = text.Mid(0, rule->FromPos);
		dc.GetTextExtent(berr, &retPos.x, NULL);
	}
	else{ retPos.x = 0; }
	dc.GetTextExtent(err, &retPos.y, NULL);
	if (rule->message){
		size_t len = strlen(rule->message) - 1;
		isMisspell = (rule->message[0] == 'W' && rule->message[len] == 'i');
	}
	return true;
}