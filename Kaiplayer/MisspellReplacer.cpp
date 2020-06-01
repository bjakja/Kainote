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
#include <algorithm>


MisspellReplacer::MisspellReplacer(wxWindow *parent)
	: KaiDialog(parent, -1, _("Korekcja drobnych błędów"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
	, resultDialog(NULL)
{
	DialogSizer *MainSizer = new DialogSizer(wxHORIZONTAL);
	wxBoxSizer *ListSizer = new wxBoxSizer(wxVERTICAL);
	//PutWordBoundary = new KaiCheckBox(this, ID_PUT_WORD_BOUNDARY, _("Wstawiaj automatycznie granice\npoczątku słowa \\m i końca słowa \\M"));
	//ShowBuiltInRules = new KaiCheckBox(this, ID_SHOW_BUILT_IN_RULES, _("Pokaż wbudowane zasady"));

	KaiStaticBoxSizer *RuleEdition = new KaiStaticBoxSizer(wxVERTICAL, this, _("Edycja reguły"));
	wxBoxSizer *PhrasesDescriptionSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *PhrasesSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *PhrasesOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *PhrasesOptionsSizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *PhrasesOptionsSizer2 = new wxBoxSizer(wxHORIZONTAL);
	RuleDescription = new KaiTextCtrl(this, ID_RULE_DESCRIPTION);
	PhraseToFind = new KaiTextCtrl(this, ID_PHRASE_TO_FIND);
	PhraseToFind->SetMaxLength(MAXINT);
	PhraseToReplace = new KaiTextCtrl(this, ID_PHRASE_TO_REPLACE);
	PhraseToReplace->SetMaxLength(MAXINT);
	MatchCase = new KaiCheckBox(this, ID_MATCH_CASE, _("Rozróżniaj wielkość znaków"));
	ReplaceAsLower = new KaiCheckBox(this, ID_REPLACE_LOWER, _("Zmieniaj na tekst z małej litery"));
	ReplaceAsUpper = new KaiCheckBox(this, ID_REPLACE_UPPER, _("Zmieniaj na tekst z dużej litery"));
	ReplaceWithUnchangedCase = new KaiCheckBox(this, ID_REPLACE_UPPER, _("Nie zmieniaj wielkości liter"));
	ReplaceOnlyTags = new KaiCheckBox(this, ID_REPLACE_ONLY_TAGS, _("Zmieniaj tylko w tagach"));
	ReplaceOnlyText = new KaiCheckBox(this, ID_REPLACE_ONLY_TEXT, _("Zmieniaj tylko w tekście"));

	PhrasesDescriptionSizer->Add(new KaiStaticText(this, -1, _("Szukana fraza (wyrażenia regularne)")), 1, wxALL | wxEXPAND, 2);
	PhrasesDescriptionSizer->Add(new KaiStaticText(this, -1, _("Zmieniana fraza")), 1, wxALL | wxEXPAND, 2);
	PhrasesSizer->Add(PhraseToFind, 1, wxALL | wxEXPAND, 2);
	PhrasesSizer->Add(PhraseToReplace, 1, wxALL | wxEXPAND, 2);
	PhrasesOptionsSizer->Add(MatchCase, 1, wxALL | wxEXPAND, 2);
	PhrasesOptionsSizer->Add(ReplaceAsLower, 1, wxALL | wxEXPAND, 2);
	PhrasesOptionsSizer1->Add(ReplaceAsUpper, 1, wxALL | wxEXPAND, 2);
	PhrasesOptionsSizer1->Add(ReplaceWithUnchangedCase, 1, wxALL | wxEXPAND, 2);
	PhrasesOptionsSizer2->Add(ReplaceOnlyTags, 1, wxALL | wxEXPAND, 2);
	PhrasesOptionsSizer2->Add(ReplaceOnlyText, 1, wxALL | wxEXPAND, 2);
	RuleEdition->Add(new KaiStaticText(this, -1, _("Opis reguły")), 0, wxLEFT | wxBOTTOM | wxEXPAND, 2);
	RuleEdition->Add(RuleDescription, 0, wxALL | wxEXPAND, 2);
	RuleEdition->Add(PhrasesDescriptionSizer, 0, wxEXPAND);
	RuleEdition->Add(PhrasesSizer, 0, wxEXPAND);
	RuleEdition->Add(PhrasesOptionsSizer, 0, wxEXPAND);
	RuleEdition->Add(PhrasesOptionsSizer1, 0, wxEXPAND);
	RuleEdition->Add(PhrasesOptionsSizer2, 0, wxEXPAND);
	RulesList = new KaiListCtrl(this, ID_RULES_LIST, wxDefaultPosition, wxSize(320, 300));
	RulesList->InsertColumn(0, L"", TYPE_CHECKBOX, 20);
	RulesList->InsertColumn(1, _("Opis"), TYPE_TEXT, 290);
	RulesList->InsertColumn(2, _("Reguła znajdź"), TYPE_TEXT, 100);
	RulesList->InsertColumn(3, _("Reguła zamień"), TYPE_TEXT, 100);
	Bind(LIST_ITEM_LEFT_CLICK, [=](wxCommandEvent &evt){
		int sel = RulesList->GetSelection();
		if (sel < 0 || sel >= rules.size())
			return;

		const Rule & ruleclass = rules[sel];
		//add description and option objects and add here set value etc.
		RuleDescription->SetValue(ruleclass.description);
		PhraseToFind->SetValue(ruleclass.findRule);
		PhraseToReplace->SetValue(ruleclass.replaceRule);
		int options = ruleclass.options;
		MatchCase->SetValue((options & OPTION_MATCH_CASE) != 0);
		ReplaceAsLower->SetValue((options & OPTION_REPLACE_AS_LOWER) != 0);
		ReplaceAsUpper->SetValue((options & OPTION_REPLACE_AS_UPPER) != 0);
		ReplaceWithUnchangedCase->SetValue((options & OPTION_REPLACE_WITH_UNCHANGED_CASE) != 0);
		ReplaceOnlyTags->SetValue((options & OPTION_REPLACE_ONLY_TAGS) != 0);
		ReplaceOnlyText->SetValue((options & OPTION_REPLACE_ONLY_TEXT) != 0);
	}, ID_RULES_LIST);
	FillRulesList();
	ListSizer->Add(RuleEdition, 0, wxEXPAND);
	ListSizer->Add(RulesList, 1, wxALL | wxEXPAND, 2);
	//ListSizer->Add(PutWordBoundary, 0, wxALL, 2);
	//ListSizer->Add(ShowBuiltInRules, 0, wxALL, 2);

	KaiStaticBoxSizer *WhichLinesSizer = new KaiStaticBoxSizer(wxVERTICAL, this, _("Które linijki"));
	wxString choices[] = { _("Wszystkie linijki"), _("Zaznaczone linijki"), _("Od zaznaczonej linijki"), _("Według wybranych stylów") };
	WhichLines = new KaiChoice(this, ID_WHICH_LINES_LIST, wxDefaultPosition, wxDefaultSize, 4, choices);
	WhichLines->SetSelection(0);
	wxBoxSizer *styleChooseSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *ChooseStylesButton = new MappedButton(this, ID_STYLES_CHOOSE, L"+");

	ChoosenStyles = new KaiTextCtrl(this, -1);
	styleChooseSizer->Add(ChooseStylesButton, 0, wxRIGHT, 2);
	styleChooseSizer->Add(ChoosenStyles, 1, wxEXPAND);

	WhichLinesSizer->Add(WhichLines, 0, wxBOTTOM, 2);
	WhichLinesSizer->Add(styleChooseSizer, 0, wxEXPAND);

	wxBoxSizer *ButtonsSizer = new wxBoxSizer(wxVERTICAL);
	MappedButton *AddRuleToList = new MappedButton(this, ID_ADD_RULE, _("Dodaj regułę"));
	MappedButton *EditRuleFromList = new MappedButton(this, ID_EDIT_RULE, _("Edytuj regułę"));
	MappedButton *RemoveRuleFromList = new MappedButton(this, ID_REMOVE_RULE, _("Usuń regułę"));
	MappedButton *FindRule = new MappedButton(this, ID_FIND_RULE, _("Znajdź błąd"));
	FindRule->Enable(false);
	MappedButton *FindRulesOnTab = new MappedButton(this, ID_FIND_ALL_RULES, _("Znajdź błędy\nw bieżącej zakładce"));
	MappedButton *FindRulesOnAllTabs = new MappedButton(this, ID_FIND_ALL_RULES_ON_ALL_TABS, _("Znajdź błędy\nwe wszystkich zakładkach"));
	MappedButton *ReplaceRule = new MappedButton(this, ID_REPLACE_RULE, _("Zmień błąd"));
	ReplaceRule->Enable(false);
	MappedButton *ReplaceRules = new MappedButton(this, ID_REPLACE_ALL_RULES, _("Zamień wszystkie błędy\nw bieżącej zakładce"));
	MappedButton *ReplaceRulesOnAllTabs = new MappedButton(this, ID_REPLACE_ALL_RULES_ON_ALL_TABS, _("Zamień wszystkie błędy\nwe wszystkich zakładkach"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){AddRule(); }, ID_ADD_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){EditRule(); }, ID_EDIT_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){RemoveRule(); }, ID_REMOVE_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){SeekOnce(); }, ID_FIND_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){SeekOnActualTab(); }, ID_FIND_ALL_RULES);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){SeekOnAllTabs(); }, ID_FIND_ALL_RULES_ON_ALL_TABS);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ReplaceOnce(); }, ID_REPLACE_RULE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ReplaceOnActualTab(); }, ID_REPLACE_ALL_RULES);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ReplaceOnAllTabs(); }, ID_REPLACE_ALL_RULES_ON_ALL_TABS);
	int withoutBottom = wxLEFT | wxTOP | wxRIGHT;
	ButtonsSizer->Add(WhichLinesSizer, 0, wxTOP | wxEXPAND, 2);
	ButtonsSizer->Add(AddRuleToList, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(EditRuleFromList, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(RemoveRuleFromList, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(FindRule, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(FindRulesOnTab, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(FindRulesOnAllTabs, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(ReplaceRule, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(ReplaceRules, 0, withoutBottom | wxEXPAND, 4);
	ButtonsSizer->Add(ReplaceRulesOnAllTabs, 0, withoutBottom | wxEXPAND, 4);

	MainSizer->Add(ListSizer, 1, wxALL | wxEXPAND, 2);
	MainSizer->Add(ButtonsSizer, 0, wxALL | wxEXPAND, 2);

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
	std::vector<ReplacerSeekResults *> results;
	for (size_t i = 0; i < rules.size(); i++){
		int flags = wxRE_ADVANCED;
		if (!(rules[i].options & OPTION_MATCH_CASE))
			flags |= wxRE_ICASE;

		wxRegEx *rule = new wxRegEx(rules[i].findRule, flags);
		if (!rule->IsValid()){
			//KaiLog(wxString::Format("Szablon wyrażeń regularnych \"%s\" jest nieprawidłowy", rules[checkedRules[i]].first));
			delete rule;
			continue;
		}
		rxrules.push_back(rule);
	}


	KaiListCtrl *List = resultDialog->ResultsList;
	TabPanel *oldtab = NULL;
	TabPanel *tab = NULL;
	Notebook * tabs = Notebook::GetTabs();
	int oldKeyLine = -1;
	bool skipTab = false;
	bool somethingWasChanged = false;
	size_t size = List->GetCount();
	for (size_t tt = 0; tt < size; tt++){
		Item *item = List->GetItem(tt, 0);
		if (!item || item->type != TYPE_TEXT || !item->modified)
			continue;

		ReplacerSeekResults *SeekResult = (ReplacerSeekResults *)item;
		tab = SeekResult->tab;
		// check if skip lines when tab not exist
		if (tab != oldtab){
			skipTab = tabs->FindPanel(tab, false) == -1;
			oldKeyLine = -1;
		}
		//skip lines when are out of table range and not existed tab
		if (skipTab || SeekResult->keyLine >= tab->Grid->file->GetCount())
			continue;

		if (SeekResult->keyLine != oldKeyLine){
			std::sort(results.begin(), results.end(), [=](ReplacerSeekResults *i, ReplacerSeekResults *j){
				return i->findPosition.x > j->findPosition.x;
			});
			somethingWasChanged |= ReplaceBlock(results, rxrules);
			results.clear();
		}

		results.push_back(SeekResult);
		if (tab != oldtab && oldtab && somethingWasChanged){
			oldtab->Grid->SetModified(REPLACED_BY_MISSPELL_REPLACER);
			oldtab->Grid->SpellErrors.clear();
			oldtab->Grid->Refresh(false);
			somethingWasChanged = false;
		}

		oldtab = tab;
		oldKeyLine = SeekResult->keyLine;
	}
	if (results.size()){
		std::sort(results.begin(), results.end(), [=](ReplacerSeekResults *i, ReplacerSeekResults *j){
			return i->findPosition.x > j->findPosition.x;
		});
		somethingWasChanged |= ReplaceBlock(results, rxrules);
	}

	if (tab && somethingWasChanged){
		tab->Grid->SetModified(REPLACED_BY_MISSPELL_REPLACER);
		tab->Grid->SpellErrors.clear();
		tab->Grid->Refresh(false);
	}

	for (auto cur = rxrules.begin(); cur != rxrules.end(); cur++){
		delete *cur;
	}

}

void MisspellReplacer::ShowResult(TabPanel *tab, int keyLine, const wxPoint &pos)
{
	Notebook *nb = Notebook::GetTabs();
	for (size_t i = 0; i < nb->Size(); i++){
		if (nb->Page(i) == tab){
			if (keyLine < tab->Grid->file->GetCount()){
				if (i != nb->iter)
					nb->ChangePage(i);

				tab->Edit->SetLine(keyLine);
				tab->Grid->SelectRow(keyLine);
				tab->Grid->ScrollTo(keyLine, true);
				tab->Edit->GetEditor()->SetSelection(pos.x, pos.x + pos.y);
			}
			break;
		}
	}
}

void MisspellReplacer::FillRulesList()
{
	wxString rulesText;
	OpenWrite ow;
	ow.FileOpen(Options.configPath + L"\\Rules.txt", &rulesText);
	if (rulesText.empty()){
		FillWithDefaultRules(rulesText);
	}
	wxStringTokenizer tokenizer(rulesText, L"\n", wxTOKEN_STRTOK);
	wxString headertoken = tokenizer.GetNextToken();
	if (headertoken.StartsWith(L"#Kainote rules file"))
		headertoken = tokenizer.GetNextToken();
	wxStringTokenizer tokenizerOnOff(headertoken, L"|", wxTOKEN_STRTOK);

	while (tokenizer.HasMoreTokens())
	{
		wxString token = tokenizer.GetNextToken();
		wxString OnOff = (tokenizerOnOff.HasMoreTokens()) ? tokenizerOnOff.GetNextToken() : L"0";
		Rule newRule(token);
		rules.push_back(newRule);

		int row = RulesList->AppendItem(new ItemCheckBox(OnOff == L"1", L""));
		RulesList->SetItem(row, 1, new ItemText(newRule.description));
		RulesList->SetItem(row, 2, new ItemText(newRule.findRule));
		RulesList->SetItem(row, 3, new ItemText(newRule.replaceRule));
	}
}

void MisspellReplacer::EditRule()
{
	if (resultDialog && resultDialog->IsShown()){
		KaiLog(_("Nie można zmieniać reguł,\ngdy okno wyników szukania jest otwarte"));
		return;
	}
	int sel = RulesList->GetSelection();
	if (sel < 0 || sel >= rules.size()){
		KaiLog(wxString::Format(L"Edit rule - bad selection of rule %i - %llu", sel, (unsigned long long)rules.size()));
		return;
	}

	rules[sel] = Rule(RuleDescription->GetValue(), PhraseToFind->GetValue(), PhraseToReplace->GetValue(), GetRuleOptions());
	Item * itemD = RulesList->GetItem(sel, 1);
	Item * itemF = RulesList->GetItem(sel, 2);
	Item * itemR = RulesList->GetItem(sel, 3);
	if (itemD)
		itemD->name = RuleDescription->GetValue();
	if (itemF)
		itemF->name = PhraseToFind->GetValue();
	if (itemR)
		itemR->name = PhraseToReplace->GetValue();
	RulesList->Refresh(false);
}

void MisspellReplacer::AddRule()
{
	if (resultDialog && resultDialog->IsShown()){
		KaiLog(_("Nie można zmieniać reguł,\ngdy okno wyników szukania jest otwarte"));
		return;
	}
	wxString phraseToFind = PhraseToFind->GetValue();
	wxString phraseToReplace = PhraseToReplace->GetValue();
	//if (PutWordBoundary->GetValue())
	//phraseToFind = L"\\m" + phraseToFind + L"\\M";

	rules.push_back(Rule(RuleDescription->GetValue(), phraseToFind, phraseToReplace, GetRuleOptions()));
	int row = RulesList->AppendItem(new ItemCheckBox(false, L""));
	RulesList->SetItem(row, 1, new ItemText(RuleDescription->GetValue()));
	RulesList->SetItem(row, 2, new ItemText(phraseToFind));
	RulesList->SetItem(row, 3, new ItemText(phraseToReplace));
	RulesList->Refresh(false);
}

void MisspellReplacer::RemoveRule()
{
	if (resultDialog && resultDialog->IsShown()){
		KaiLog(_("Nie można zmieniać reguł,\ngdy okno wyników szukania jest otwarte"));
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

	std::vector<std::pair<wxRegEx*, int>> rxrules;
	for (size_t i = 0; i < checkedRules.size(); i++){
		const Rule &actualRule = rules[checkedRules[i]];
		int flags = wxRE_ADVANCED;
		if (!(actualRule.options & OPTION_MATCH_CASE))
			flags |= wxRE_ICASE;

		wxRegEx *rule = new wxRegEx(actualRule.findRule, flags);
		if (!rule->IsValid()){
			//KaiLog(wxString::Format("Szablon wyrażeń regularnych \"%s\" jest nieprawidłowy", rules[checkedRules[i]].first));
			delete rule;
			continue;
		}
		rxrules.push_back(std::make_pair(rule, actualRule.options));
	}

	//0-all lines 1-selected lines 2-from selected 3-by styles
	wxString stylesAsText = L"," + ChoosenStyles->GetValue() + L",";
	int selectedOption = WhichLines->GetSelection();
	size_t firstSelectedId = -1;
	size_t tabLinePosition = tab->Grid->FirstSelection(&firstSelectedId);
	int positionId = 0;
	tabLinePosition = (selectedOption == 2 && tabLinePosition != -1) ? tabLinePosition : 0;
	if (tabLinePosition > 0)
		positionId = firstSelectedId;

	SubsFile *Subs = tab->Grid->file;


	bool isfirst = true;

	while (tabLinePosition < Subs->GetCount())
	{
		Dialogue *Dial = Subs->GetDialogue(tabLinePosition);
		if (!Dial->isVisible || (tab->Grid->ignoreFiltered && Dial->NonDialogue)){ tabLinePosition++; continue; }

		if ((!selectedOption) ||
			(selectedOption == 3 && stylesAsText.Find(L"," + Dial->Style + L",") != -1) ||
			(selectedOption == 1 && tab->Grid->file->IsSelected(tabLinePosition))){
			const wxString & lineText = (Dial->TextTl != L"") ? Dial->TextTl : Dial->Text;

			for (size_t k = 0; k < rxrules.size(); k++){


				int textPos = 0;
				wxString text = lineText;
				wxRegEx *r = rxrules[k].first;

				while (r->Matches(text)) {
					size_t matchstart = 0, matchlen = 0;
					if (r->GetMatch(&matchstart, &matchlen)){
						if (matchlen == 0)
							matchlen++;
						int options = rxrules[k].second;
						if ((options < 16) || KeepFinding(lineText, matchstart + textPos, options)){

							if (isfirst){
								resultDialog->SetHeader(tab->SubsPath);
								isfirst = false;
							}

							resultDialog->SetResults(lineText, wxPoint(matchstart + textPos, matchlen),
								tab, tabLinePosition, positionId + 1, checkedRules[k]);
						}
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
		delete cur->first;
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
	resultDialog->FilterList();
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
	resultDialog->FilterList();
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

	std::vector<std::pair<wxRegEx*, int>> rxrules;
	for (size_t i = 0; i < checkedRules.size(); i++){
		int flags = wxRE_ADVANCED;
		if (!(rules[checkedRules[i]].options & OPTION_MATCH_CASE))
			flags |= wxRE_ICASE;

		wxRegEx *rule = new wxRegEx(rules[checkedRules[i]].findRule, flags);
		if (!rule->IsValid()){
			//KaiLog(wxString::Format("Szablon wyrażeń regularnych \"%s\" jest nieprawidłowy", rules[checkedRules[i]].first));
			delete rule;
			continue;
		}
		rxrules.push_back(std::make_pair(rule, checkedRules[i]));
	}

	//0-all lines 1-selected lines 2-from selected 3-by styles
	wxString stylesAsText = L"," + ChoosenStyles->GetValue() + L",";
	int selectedOption = WhichLines->GetSelection();
	int firstSelectedId = tab->Grid->FirstSelection();
	int positionId = 0;
	int tabLinePosition = (selectedOption == 2 && firstSelectedId >= 0) ? firstSelectedId : 0;
	if (tabLinePosition > 0)
		positionId = firstSelectedId;

	bool changedAnything = false;

	SubsFile *Subs = tab->Grid->file;

	while (tabLinePosition < Subs->GetCount())
	{
		Dialogue *Dial = Subs->GetDialogue(tabLinePosition);
		if (!Dial->isVisible || (tab->Grid->ignoreFiltered && Dial->NonDialogue)){ tabLinePosition++; continue; }

		if ((!selectedOption) ||
			(selectedOption == 3 && stylesAsText.Find(L"," + Dial->Style + L",") != -1) ||
			(selectedOption == 1 && tab->Grid->file->IsSelected(tabLinePosition))){
			const wxString & lineText = Dial->Text.CheckTl(Dial->TextTl, Dial->TextTl != L"");
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

						const Rule & actualrule = rules[rxrules[k].second];
						int options = actualrule.options;
						if ((options < 16) || KeepFinding(text, matchstart, options)){
							wxString replacedResult;
							wxString matchResult = replacedResult = stringChanged.Mid(matchstart + textPos, matchlen);
							int reps = r->Replace(&replacedResult, actualrule.replaceRule);

							MoveCase(matchResult, &replacedResult, actualrule.options);

							stringChanged.replace(matchstart + textPos, matchlen, replacedResult);

							matchlen = replacedResult.length();

							changed = true;
						}
					}
					else
						break;

					textPos += (matchstart + matchlen);
					text = stringChanged.Mid(textPos);
				}

			}
			if (changed){
				Dialogue *Dialc = tab->Grid->file->CopyDialogue(tabLinePosition);
				Dialc->Text.CheckTlRef(Dialc->TextTl, Dialc->TextTl != L"") = stringChanged;
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

bool MisspellReplacer::ReplaceBlock(std::vector<ReplacerSeekResults *> &results, std::vector<wxRegEx*> &rxrules)
{
	if (!results.size())
		return false;

	Dialogue *Dialc = results[0]->tab->Grid->file->CopyDialogue(results[0]->keyLine, true, true);

	wxString & lineText = Dialc->Text.CheckTlRef(Dialc->TextTl, Dialc->TextTl != L"");
	//method maybe not too good but even if there is a 6 replaces in one place then 
	//one of it should be changed.
	//It means that dialogue will be changed and no need to read and change if needed
	if (lineText != results[0]->name){
		KaiLog(wxString::Format(_("Linia %i nie może być zamieniona,\nbo została zedytowana."),
			results[0]->idLine));
		return false;
	}

	bool somethingChanged = false;
	for (auto & SeekResult : results){

		size_t pos = SeekResult->findPosition.x;
		size_t len = SeekResult->findPosition.y;
		const Rule & actualrule = rules[SeekResult->numOfRule];
		wxString replacedResult;
		wxString matchResult = replacedResult = lineText.Mid(pos, len);
		int reps = rxrules[SeekResult->numOfRule]->Replace(&replacedResult, actualrule.replaceRule);
		if (reps > 0){
			MoveCase(matchResult, &replacedResult, actualrule.options);

			lineText.replace(pos, len, replacedResult);
			somethingChanged = true;
		}
		else{
			KaiLog(wxString::Format(_("Nie można zamienić \"%s\" na \"%s\", wykorzystując regułę \"%s\" w linii %i."),
				matchResult, actualrule.replaceRule, actualrule.findRule, SeekResult->idLine));
		}
	}

	if (somethingChanged)
		Dialc->ChangeDialogueState(1);

	return somethingChanged;
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
	for (size_t i = 0; i < RulesList->GetCount(); i++){
		Item *item = RulesList->GetItem(i, 0);
		if (item)
			rulesText << (int)item->modified << L"|";
		else
			rulesText << L"0|";
	}
	if (rulesText.EndsWith(L'|'))
		rulesText.RemoveLast() += L"\r\n";

	for (auto & rule : rules){
		rulesText << rule.description << L"\f" << rule.findRule << L"\f" << rule.replaceRule << L"\f" << rule.options << L"\r\n";
	}
	OpenWrite ow;
	ow.FileWrite(Options.configPath + L"\\Rules.txt", rulesText);
}

void MisspellReplacer::MoveCase(const wxString &originalCase, wxString *result, int options)
{
	if (options & OPTION_REPLACE_WITH_UNCHANGED_CASE || !originalCase.length() || !result->length())
		return;
	if (options & OPTION_REPLACE_AS_LOWER){
		result->MakeLower();
		return;
	}
	if (options & OPTION_REPLACE_AS_UPPER){
		result->MakeUpper();
		return;
	}

	int upperCase = 0;
	size_t len = originalCase.length();
	for (size_t i = 0; i < len; i++){
		if (iswupper(wint_t(originalCase[i])) != 0)
			upperCase++;
	}
	if (upperCase > 1)
		result->MakeUpper();
	else if (upperCase > 0)
		result->at(0) = wxToupper(result->at(0));

}

int MisspellReplacer::GetRuleOptions()
{
	int result = 0;

	result |= (1 * (int)MatchCase->GetValue());
	result |= (2 * (int)ReplaceAsLower->GetValue());
	result |= (4 * (int)ReplaceAsUpper->GetValue());
	result |= (8 * (int)ReplaceWithUnchangedCase->GetValue());
	result |= (16 * (int)ReplaceOnlyTags->GetValue());
	result |= (32 * (int)ReplaceOnlyText->GetValue());

	return result;
}

void MisspellReplacer::FillWithDefaultRules(wxString &rules)
{
	rules = L"#Kainote rules file\n0|0|0|0|0|0|0|0|0|0|0|0\n" +
		_("Usuwanie spacji przed przecinkiem bądź kropką") + L"\f ([,.!?%])\f\\1\f0\n" +
		_("Usuwanie podwójnych spacji") + L"\f(  +)\f \f0\n" +
		_("Zmiana \"....\" i więcej na wielokropek") + L"\f\\.{4,}\f...\f0\n" +
		_("Zmiana \"..\" na wielokropek") + L"\f([^.])\\.\\.([^.])\f\\1...\\2\f0\n" +
		_("Zmiana braku spacji po przecinku czy kropce") + L"\f([^.])([,.!?%])([^ ,.!?%\\\"\\\\0-9-])\f\\1\\2 \\3\f0\n" +
		_("Usuwanie japońskich zwrotów grzecznościowych") + L"\f ?- ?(san|chan|kun|sama|nee|dono|senpai|sensei)\\M\f\f0\n" +
		_("Zamiana \"sie\" na \"się\"") + L"\f\\msie\\M\fsię\f0\n" +
		_("Zamiana \"nie możliwe\" na \"niemożliwe\"") + L"\f\\mnie możliwe\\M\fniemożliwe\f0\n" +
		_("Zamiana \"nie ważne\" na \"nieważne\"") + L"\f\\mnie ważne\\M\fnieważne\f0\n" +
		_("Zamiana błędów wyrażenia \"w ogóle\"") + L"\f\\mw ?og[uo]le\\M\fw ogóle\f0\n" +
		_("Zamiana błędów wyrażenia \"w ogóle\"") + L"\f\\mwogóle\\M\fw ogóle\f0\n" +
		_("Zamiana błędów wyrazu \"będę\"") + L"\f\\mbed[eę]\\M\fbędę\f0\n" +
		_("Zamiana błędów wyrazu \"będę\"") + L"\f\\mbęde\\M\fbędę\f0";

}

//true skipping this find
bool MisspellReplacer::KeepFinding(const wxString &text, int textPos, int options)
{
	bool findStart = false;
	//I don't even need to check end cause when there's no end start will take all line
	for (int i = textPos; i >= 0; i--){
		if (text[i] == L'}')
			break;
		if (text[i] == L'{'){
			findStart = true;
			break;
		}
	}
	if (options & OPTION_REPLACE_ONLY_TAGS && findStart)
		return true;

	if (options & OPTION_REPLACE_ONLY_TEXT && !findStart)
		return true;

	return false;
}

Rule::Rule(const wxString & stringRule)
{
	wxStringTokenizer ruleTokenizer(stringRule, L"\f", wxTOKEN_RET_EMPTY_ALL);
	wxString ruleToken = ruleTokenizer.GetNextToken();
	description = ruleToken;
	if (!ruleTokenizer.HasMoreTokens())
		goto fail;

	ruleToken = ruleTokenizer.GetNextToken();
	findRule = ruleToken;
	if (!ruleTokenizer.HasMoreTokens())
		goto fail;

	ruleToken = ruleTokenizer.GetNextToken();
	replaceRule = ruleToken;
	if (!ruleTokenizer.HasMoreTokens())
		goto fail;

	ruleToken = ruleTokenizer.GetNextToken();
	options = wxAtoi(ruleToken);
	return;

fail:
	KaiLog(wxString::Format(_("Reguła \"%s\" jest nieprawidłowa."), stringRule));
}

//CheckBoxListButton::CheckBoxListButton(wxWindow *parent, int id, const wxString &name, const wxArrayString &listElements, const wxPoint &pos, const wxSize &size, long style) 
//	:MappedButton(parent, id, name, -1, pos, size, style)
//{
//	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
//		if (!checkList)
//			checkList = new CustomCheckListBox(this, listElements, name);
//
//		if (checkList->ShowModal() != wxID_OK){
//			for (size_t i = 0; i < checkList->CheckListBox->GetCount(); i++){
//				Item *item = checkList->CheckListBox->GetItem(i, 0);
//				if (item)
//					item->modified = false;
//			}
//		}
//		
//	}, GetId());
//}
//
//void CheckBoxListButton::GetChecked(wxArrayInt & checks)
//{
//	if (checkList){
//		for (size_t i = 0; i < checkList->CheckListBox->GetCount(); i++){
//			Item *item = checkList->CheckListBox->GetItem(i, 0);
//			if (item)
//				checks.Add(i);
//		}
//	}
//}
