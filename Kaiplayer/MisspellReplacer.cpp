//  Copyright (c) 2018, Marcin Drob

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


#include "MisspellReplacer.h"
#include "OpennWrite.h"
#include "KaiStaticBoxSizer.h"
#include "Tabs.h"
#include "KainoteMain.h"
#include <regex>
#include <wx/regex.h>


MisspellReplacer::MisspellReplacer(wxWindow *parent)
	:KaiDialog(parent, -1, _("Korekcja drobnych b³êdów"))
	, resultDialog(NULL)
{
	DialogSizer *MainSizer = new DialogSizer(wxHORIZONTAL);
	wxBoxSizer *ListSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *PhrasesSizer = new wxBoxSizer(wxHORIZONTAL);
	PutWordBoundary = new KaiCheckBox(this, ID_PUT_WORD_BOUNDARY, _("Wstawiaj automatycznie granice s³ów \\b"));
	ShowBuiltInRules = new KaiCheckBox(this, ID_SHOW_BUILT_IN_RULES, _("Poka¿ wbudowane zasady"));
	PhraseToFind = new KaiTextCtrl(this, ID_PHRASE_TO_FIND);
	PhraseToFind->SetMaxLength(MAXINT);
	PhraseToReplace = new KaiTextCtrl(this, ID_PHRASE_TO_REPLACE);
	PhraseToReplace->SetMaxLength(MAXINT);
	PhrasesSizer->Add(PhraseToFind, 1, wxALL | wxEXPAND, 2);
	PhrasesSizer->Add(PhraseToReplace, 1, wxALL | wxEXPAND, 2);
	RulesList = new KaiListCtrl(this, ID_RULES_LIST,wxDefaultPosition, wxSize(320, 400));
	RulesList->InsertColumn(0, L"", TYPE_CHECKBOX, 20);
	RulesList->InsertColumn(1, _("Opis / szukana fraza"), TYPE_TEXT, 200);
	RulesList->InsertColumn(2, _("Zmieniania fraza"), TYPE_TEXT, 90);
	Bind(LIST_ITEM_LEFT_CLICK, [=](wxCommandEvent &evt){
		int sel = RulesList->GetSelection();
		if (sel < 0 || sel >= rules.size())
			return;

		const std::pair<wxString, wxString> & rulepair = rules[sel];
		PhraseToFind->SetValue(rulepair.first);
		PhraseToReplace->SetValue(rulepair.second);
	}, ID_RULES_LIST);
	FillRulesList();
	ListSizer->Add(PhrasesSizer, 0, wxEXPAND);
	ListSizer->Add(RulesList, 1, wxALL | wxEXPAND, 2);
	ListSizer->Add(PutWordBoundary, 0, wxALL, 2);
	ListSizer->Add(ShowBuiltInRules, 0, wxALL, 2);

	KaiStaticBoxSizer *WhichLinesSizer = new KaiStaticBoxSizer(wxVERTICAL, this, _("Które linijki"));
	wxString choices[] = { _("Wszystkie linijki"), _("Zaznaczone linijki"), _("Od zaznaczonej linijki"), _("Wed³ug wybranych stylów") };
	WhichLines = new KaiChoice(this, ID_WHICH_LINES_LIST, wxDefaultPosition, wxDefaultSize, 4, choices);
	WhichLines->SetSelection(0);
	wxBoxSizer *styleChooseSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *ChooseStylesButton = new MappedButton(this, ID_STYLES_CHOOSE, L"+");
	
	ChoosenStyles = new KaiTextCtrl(this, -1);
	styleChooseSizer->Add(ChooseStylesButton, 0, wxRIGHT, 2);
	styleChooseSizer->Add(ChoosenStyles, 0, wxEXPAND);

	WhichLinesSizer->Add(WhichLines, 0, wxBOTTOM, 2);
	WhichLinesSizer->Add(styleChooseSizer, 0, wxEXPAND);

	wxBoxSizer *ButtonsSizer = new wxBoxSizer(wxVERTICAL);
	MappedButton *AddRuleToList = new MappedButton(this, ID_ADD_RULE, _("Dodaj regu³ê"));
	MappedButton *EditRuleFromList = new MappedButton(this, ID_EDIT_RULE, _("Edytuj regu³ê"));
	MappedButton *RemoveRuleFromList = new MappedButton(this, ID_REMOVE_RULE, _("Usuñ regu³ê"));
	MappedButton *FindRule = new MappedButton(this, ID_FIND_RULE, _("ZnajdŸ b³¹d"));
	FindRule->Enable(false);
	MappedButton *FindRulesOnTab = new MappedButton(this, ID_FIND_ALL_RULES, _("ZnajdŸ b³êdy\nw bie¿¹cej zak³adce"));
	MappedButton *FindRulesOnAllTabs = new MappedButton(this, ID_FIND_ALL_RULES_ON_ALL_TABS, _("ZnajdŸ b³êdy\nwe wszystkich zak³adkach"));
	MappedButton *ReplaceRule = new MappedButton(this, ID_REPLACE_RULE, _("Zmieñ b³¹d"));
	ReplaceRule->Enable(false);
	MappedButton *ReplaceRules = new MappedButton(this, ID_REPLACE_ALL_RULES, _("Zamieñ wszystkie b³êdy\nw bie¿¹cej zak³adce"));
	MappedButton *ReplaceRulesOnAllTabs = new MappedButton(this, ID_REPLACE_ALL_RULES_ON_ALL_TABS, _("Zamieñ wszystkie b³êdy\nwe wszystkich zak³adkach"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){AddRule(); }, ID_ADD_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EditRule(); }, ID_EDIT_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){RemoveRule(); }, ID_REMOVE_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){SeekOnce(); }, ID_FIND_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){SeekOnActualTab(); }, ID_FIND_ALL_RULES);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){SeekOnAllTabs(); }, ID_FIND_ALL_RULES_ON_ALL_TABS);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ReplaceOnce(); }, ID_REPLACE_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ReplaceOnActualTab(); }, ID_REPLACE_ALL_RULES);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ReplaceOnAllTabs(); }, ID_REPLACE_ALL_RULES_ON_ALL_TABS);
	ButtonsSizer->Add(WhichLinesSizer, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(AddRuleToList, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(EditRuleFromList, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(RemoveRuleFromList, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(FindRule, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(FindRulesOnTab, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(FindRulesOnAllTabs, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(ReplaceRule, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(ReplaceRules, 0, wxALL | wxEXPAND, 2);
	ButtonsSizer->Add(ReplaceRulesOnAllTabs, 0, wxALL | wxEXPAND, 2);

	MainSizer->Add(ListSizer, 0, wxALL, 2);
	MainSizer->Add(ButtonsSizer, 0, wxALL, 2);

	SetSizerAndFit(MainSizer);
	CenterOnParent();
}

MisspellReplacer::~MisspellReplacer()
{
	if (rules.size())
		SaveRules();
}

void MisspellReplacer::ReplaceChecked()
{
	std::vector<wxRegEx*> rxrules;
	for (size_t i = 0; i < rules.size(); i++){
		wxRegEx *rule = new wxRegEx(rules[i].first, wxRE_ADVANCED | wxRE_ICASE);
		if (!rule->IsValid()){
			//KaiLog(wxString::Format("Szablon wyra¿eñ regularnych \"%s\" jest nieprawid³owy", rules[checkedRules[i]].first));
			delete rule;
			continue;
		}
		rxrules.push_back(rule);
	}
	

	KaiListCtrl *List = resultDialog->ResultsList;
	TabPanel *oldtab = NULL;
	TabPanel *tab = NULL;
	int oldKeyLine = -1;
	int oldNumOfRule = -1;
	int replaceDiff = 0;
	for (size_t tt = 0; tt < List->GetCount(); tt++){
		Item *item = List->GetItem(tt, 0);
		if (!item || item->type != TYPE_TEXT || !item->modified)
			continue;

		ReplacerSeekResults *SeekResult = (ReplacerSeekResults *)item;
		tab = SeekResult->tab;

		Dialogue *Dialc = tab->Grid->file->CopyDialogueByKey(SeekResult->keyLine);
		
		wxString & lineText = Dialc->Text.CheckTlRef(Dialc->TextTl, Dialc->TextTl != "");

		//replace differents for not matching sizes in one line
		if (SeekResult->keyLine != oldKeyLine)
			replaceDiff = 0;
	
		size_t pos = SeekResult->findPosition.x + replaceDiff;
		size_t len = SeekResult->findPosition.y;
		wxString replacedResult;
		wxString matchResult = replacedResult = lineText.Mid(pos, len);
		int reps = rxrules[SeekResult->numOfRule]->Replace(&replacedResult, rules[SeekResult->numOfRule].second);

		MoveCase(matchResult, &replacedResult);

		lineText.replace(pos, len, replacedResult);

		replaceDiff += replacedResult.length() - matchResult.length();

		if (oldtab && oldtab != tab){
			oldtab->Grid->SetModified(REPLACED_BY_MISSPELL_REPLACER);
			oldtab->Grid->SpellErrors.clear();
			oldtab->Grid->Refresh(false);
		}

		oldtab = tab;
		oldKeyLine = SeekResult->keyLine;
		oldNumOfRule = SeekResult->numOfRule;
	}
	if (tab){
		tab->Grid->SetModified(REPLACED_BY_MISSPELL_REPLACER);
		tab->Grid->SpellErrors.clear();
		tab->Grid->Refresh(false);
	}

	for (auto cur = rxrules.begin(); cur != rxrules.end(); cur++){
		delete *cur;
	}

}

void MisspellReplacer::ShowResult(TabPanel *tab, int keyLine)
{
	Notebook *nb = Notebook::GetTabs();
	for (size_t i = 0; i < nb->Size(); i++){
		if (nb->Page(i) == tab){
			if (keyLine < tab->Grid->file->GetAllCount()){
				if (i != nb->iter)
					nb->ChangePage(i);

				int lineId = tab->Grid->file->GetElementByKey(keyLine);
				tab->Edit->SetLine(lineId);
				tab->Grid->SelectRow(lineId);
				tab->Grid->ScrollTo(lineId, true);
			}
			break;
		}
	}
}

void MisspellReplacer::FillRulesList()
{
	wxString rulesText;
	OpenWrite ow;
	ow.FileOpen(Options.pathfull + L"\\Rules.txt", &rulesText);
	if (rulesText.empty())
		return;

	wxStringTokenizer tokenizer(rulesText, "\n", wxTOKEN_STRTOK);
	wxString headertoken = tokenizer.GetNextToken();
	if (headertoken.StartsWith(L"#Kainote rules file"))
		headertoken = tokenizer.GetNextToken();
	wxStringTokenizer tokenizerOnOff(headertoken, "|", wxTOKEN_STRTOK);

	while(tokenizer.HasMoreTokens())
	{
		wxString token = tokenizer.GetNextToken();
		wxString replaceRule;
		wxString findrule = token.BeforeFirst('\f', &replaceRule);
		wxString OnOff = (tokenizerOnOff.HasMoreTokens())? tokenizerOnOff.GetNextToken() : "0";
		rules.push_back(std::make_pair(findrule, replaceRule));

		int row = RulesList->AppendItem(new ItemCheckBox(OnOff == "1", L""));
		RulesList->SetItem(row, 1, new ItemText(findrule));
		RulesList->SetItem(row, 2, new ItemText(replaceRule));
	}
}

void MisspellReplacer::EditRule()
{
	if (resultDialog && resultDialog->IsShown()){
		KaiLog(_("Nie mo¿na zmieniaæ regu³,\ngdy okno wyników szukania jest otwarte"));
		return;
	}
	int sel = RulesList->GetSelection();
	if (sel < 0 || sel >= rules.size()){
		KaiLog(wxString::Format(L"Edit rule - bad selection of rule %i - %llu", sel, (unsigned long long)rules.size()));
		return;
	}

	rules[sel] = std::make_pair(PhraseToFind->GetValue(), PhraseToReplace->GetValue());
	Item * itemF = RulesList->GetItem(sel, 1);
	Item * itemR = RulesList->GetItem(sel, 2);
	if (itemF)
		itemF->name = PhraseToFind->GetValue();
	if (itemR)
		itemR->name = PhraseToReplace->GetValue();
	RulesList->Refresh(false);
}

void MisspellReplacer::AddRule()
{
	if (resultDialog && resultDialog->IsShown()){
		KaiLog(_("Nie mo¿na zmieniaæ regu³,\ngdy okno wyników szukania jest otwarte"));
		return;
	}
	wxString phraseToFind = PhraseToFind->GetValue();
	wxString phraseToReplace = PhraseToReplace->GetValue();
	if (PutWordBoundary->GetValue())
		phraseToFind = L"\\b" + phraseToFind + L"\\b";

	rules.push_back(std::make_pair(phraseToFind, phraseToReplace));
	int row = RulesList->AppendItem(new ItemCheckBox(false, L""));
	RulesList->SetItem(row, 1, new ItemText(phraseToFind));
	RulesList->SetItem(row, 2, new ItemText(phraseToReplace));
	RulesList->Refresh(false);
}

void MisspellReplacer::RemoveRule()
{
	if (resultDialog && resultDialog->IsShown()){
		KaiLog(_("Nie mo¿na zmieniaæ regu³,\ngdy okno wyników szukania jest otwarte"));
		return;
	}
	int sel = RulesList->GetSelection();
	if (sel < 0 || sel >= rules.size()){
		KaiLog(wxString::Format(L"Edit rule - bad selection of rule %i - %llu", sel, (unsigned long long)rules.size()));
		return;
	}
	rules.erase(rules.begin() + sel);
	RulesList->DeleteItem(sel, false);
	RulesList->Refresh(false);
}

void MisspellReplacer::SeekOnce()
{
	//:todo seek once
}

void MisspellReplacer::SeekOnTab(TabPanel *tab)
{
	std::vector<int> checkedRules;
	GetCheckedRules(checkedRules);
	if (checkedRules.size() == 0)
		return;
	//maybe some info needed

	std::vector<wxRegEx*> rxrules;
	for (size_t i = 0; i < checkedRules.size(); i++){
		wxRegEx *rule = new wxRegEx(rules[checkedRules[i]].first, wxRE_ADVANCED/* | wxRE_ICASE*/);
		if (!rule->IsValid()){
			//KaiLog(wxString::Format("Szablon wyra¿eñ regularnych \"%s\" jest nieprawid³owy", rules[checkedRules[i]].first));
			delete rule;
			continue;
		}
		rxrules.push_back(rule);
	}

	//0-all lines 1-selected lines 2-from selected 3-by styles
	wxString stylesAsText = L"," + ChoosenStyles->GetValue() + L",";
	int selectedOption = WhichLines->GetSelection();
	int firstSelectedId = tab->Grid->FirstSelection();
	int positionId = 0;
	int tabLinePosition = (selectedOption == 2 && firstSelectedId >= 0) ? tab->Grid->file->GetElementById(firstSelectedId) : 0;
	if (tabLinePosition > 0)
		positionId = firstSelectedId;

	File *Subs = tab->Grid->file->GetSubs();
	

	bool isfirst = true;

	while (tabLinePosition < Subs->dials.size())
	{
		Dialogue *Dial = Subs->dials[tabLinePosition];
		if (!Dial->isVisible){ tabLinePosition++; continue; }

		if ((!selectedOption) ||
			(selectedOption == 3 && stylesAsText.Find("," + Dial->Style + ",") != -1) ||
			(selectedOption == 1 && tab->Grid->file->IsSelectedByKey(tabLinePosition))){
			const wxString & lineText = (Dial->TextTl != "") ? Dial->TextTl : Dial->Text;

			for (size_t k = 0; k < rxrules.size(); k++){
				

				int textPos = 0;
				wxString text = lineText;
				wxRegEx *r = rxrules[k];

				while (r->Matches(text)) {
					size_t matchstart=0, matchlen=0;
					if (r->GetMatch(&matchstart, &matchlen)){
						if (matchlen == 0)
							matchlen++;

						if (isfirst){
							resultDialog->SetHeader(tab->SubsPath);
							isfirst = false;
						}

						resultDialog->SetResults(lineText, wxPoint(matchstart + textPos, matchlen), tab, tabLinePosition, positionId + 1, checkedRules[k]);
					}
					else
						break;
						
					textPos += (matchstart + matchlen);
					text = lineText.Mid(textPos);
				}
				
			}
		}
		positionId++;
		tabLinePosition++;
	}
	for (auto cur = rxrules.begin(); cur != rxrules.end(); cur++){
		delete *cur;
	}
}

void MisspellReplacer::SeekOnActualTab()
{
	if (resultDialog){
		resultDialog->ClearList();
	}
	else{
		resultDialog = new FindResultDialog(GetParent(), this);
	}
	SeekOnTab(Notebook::GetTab());
	if (!resultDialog->IsShown())
		resultDialog->Show();
}

void MisspellReplacer::SeekOnAllTabs()
{
	if (resultDialog){
		resultDialog->ClearList();
	}
	else{
		resultDialog = new FindResultDialog(GetParent(), this);
	}
	Notebook *nb = Notebook::GetTabs();
	for (size_t i = 0; i < nb->Size(); i++){
		SeekOnTab(nb->Page(i));
	}
	if (!resultDialog->IsShown())
		resultDialog->Show();
}

void MisspellReplacer::ReplaceOnce()
{
	//:todo replace once
}

void MisspellReplacer::ReplaceOnTab(TabPanel *tab)
{
	std::vector<int> checkedRules;
	GetCheckedRules(checkedRules);
	if (checkedRules.size() == 0)
		return;
	//maybe some info needed

	std::vector<std::pair<wxRegEx*, wxString>> rxrules;
	for (size_t i = 0; i < checkedRules.size(); i++){
		wxRegEx *rule = new wxRegEx(rules[checkedRules[i]].first, wxRE_ADVANCED | wxRE_ICASE);
		if (!rule->IsValid()){
			//KaiLog(wxString::Format("Szablon wyra¿eñ regularnych \"%s\" jest nieprawid³owy", rules[checkedRules[i]].first));
			delete rule;
			continue;
		}
		rxrules.push_back(std::make_pair(rule, rules[checkedRules[i]].second));
	}

	//0-all lines 1-selected lines 2-from selected 3-by styles
	wxString stylesAsText = L"," + ChoosenStyles->GetValue() + L",";
	int selectedOption = WhichLines->GetSelection();
	int firstSelectedId = tab->Grid->FirstSelection();
	int positionId = 0;
	int tabLinePosition = (selectedOption == 2 && firstSelectedId >= 0) ? tab->Grid->file->GetElementById(firstSelectedId) : 0;
	if (tabLinePosition > 0)
		positionId = firstSelectedId;

	bool changedAnything = false;

	File *Subs = tab->Grid->file->GetSubs();

	while (tabLinePosition < Subs->dials.size())
	{
		Dialogue *Dial = Subs->dials[tabLinePosition];
		if (!Dial->isVisible){ tabLinePosition++; continue; }

		if ((!selectedOption) ||
			(selectedOption == 3 && stylesAsText.Find("," + Dial->Style + ",") != -1) ||
			(selectedOption == 1 && tab->Grid->file->IsSelectedByKey(tabLinePosition))){
			const wxString & lineText = Dial->Text.CheckTl(Dial->TextTl, Dial->TextTl != "");
			wxString stringChanged = lineText;
			bool changed = false;
			for (size_t k = 0; k < rxrules.size(); k++){
				int textPos = 0;
				//int replaceDiff = 0;
				wxString text = lineText;
				wxRegEx *r = rxrules[k].first;

				while (r->Matches(text)) {
					size_t matchstart = 0, matchlen = 0;
					if (r->GetMatch(&matchstart, &matchlen)){
						if (matchlen == 0)
							matchlen++;

						wxString replacedResult;
						wxString matchResult = replacedResult = stringChanged.Mid(matchstart + textPos, matchlen);
						int reps = r->Replace(&replacedResult, rxrules[k].second);

						MoveCase(matchResult, &replacedResult);

						stringChanged.replace(matchstart + textPos, matchlen, replacedResult);

						matchlen = replacedResult.length();

						changed = true;
						
					}
					else
						break;

					textPos += (matchstart + matchlen);
					text = stringChanged.Mid(textPos);
				}
				
			}
			if (changed){
				Dialogue *Dialc = tab->Grid->file->CopyDialogueByKey(tabLinePosition);
				Dialc->Text.CheckTlRef(Dialc->TextTl, Dialc->TextTl != "") = stringChanged;
				changedAnything = true;
			}
		}
		positionId++;
		tabLinePosition++;
	}
	for (auto cur = rxrules.begin(); cur != rxrules.end(); cur++){
		delete cur->first;
	}
	if (changedAnything){
		tab->Grid->SetModified(REPLACED_BY_MISSPELL_REPLACER);
		tab->Grid->SpellErrors.clear();
		tab->Grid->Refresh(false);
	}
}

void MisspellReplacer::ReplaceOnActualTab()
{
	ReplaceOnTab(Notebook::GetTab());
}

void MisspellReplacer::ReplaceOnAllTabs()
{
	Notebook *nb = Notebook::GetTabs();
	for (size_t i = 0; i < nb->Size(); i++){
		ReplaceOnTab(nb->Page(i));
	}
}

void MisspellReplacer::GetCheckedRules(std::vector<int> &checkedRules)
{
	for (size_t i = 0; i < RulesList->GetCount(); i++){
		Item *item = RulesList->GetItem(i, 0);
		if (item && item->modified){
			checkedRules.push_back(i);
		}
	}
}

void MisspellReplacer::SaveRules()
{
	wxString rulesText = L"#Kainote rules file\r\n";
	for (int i = 0; i < RulesList->GetCount(); i++){
		Item *item = RulesList->GetItem(i, 0);
		if (item)
			rulesText << (int)item->modified << L"|";
		else
			rulesText << L"0|";
	}
	if (rulesText.EndsWith('|'))
		rulesText.RemoveLast() += L"\r\n";

	for (auto & rule : rules){
		rulesText << rule.first << "\f" << rule.second << "\r\n";
	}
	OpenWrite ow;
	ow.FileWrite(Options.pathfull + L"\\Rules.txt", rulesText);
}

void MisspellReplacer::MoveCase(const wxString &originalCase, wxString *result)
{
	int upperCase = 0;
	size_t len = originalCase.length();
	for (size_t i = 0; i < len; i++){
		if (iswupper(WXWCHAR_T_CAST(originalCase[i])) != 0)
			upperCase++;
	}
	if (upperCase > 1)
		result->MakeUpper();
	else if (upperCase > 0)
		result->at(0) = wxToupper(result->at(0));

}
