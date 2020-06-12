//  Copyright (c) 2016 - 2020, Marcin Drob

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


#include "SpellCheckerDialog.h"
#include "KaiStaticText.h"
#include "SpellChecker.h"
#include "KainoteMain.h"
#include "KaiMessageBox.h"
#include "OpennWrite.h"
#include "Stylelistbox.h"
#include <wx/regex.h>

SpellCheckerDialog::SpellCheckerDialog(KainoteFrame *parent)
	:KaiDialog((wxWindow*)parent, -1, _("Sprawdzanie pisowni"))
	, Kai(parent)
	, lastLine(0)
	, lastMisspell(0)
	, lastActiveLine(-1)
{
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *misspellSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *replaceSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *listSizer = new wxBoxSizer(wxHORIZONTAL);
	ignoreComments = new KaiCheckBox(this, -1, _("Ignoruj komentarze"));
	ignoreUpper = new KaiCheckBox(this, -1, _("Ignoruj słowa całe pisane\nwielką literą"));
	//wxString misspellWord = FindNextMisspell();
	misSpell = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	replaceWord = new KaiTextCtrl(this, -1);

	misspellSizer->Add(new KaiStaticText(this, -1, _("Błędne słowo:")), 1, wxEXPAND | wxALL, 2);
	misspellSizer->Add(misSpell, 4, wxEXPAND | wxALL, 2);
	replaceSizer->Add(new KaiStaticText(this, -1, _("Zmień na:")), 1, wxEXPAND | wxALL, 2);
	replaceSizer->Add(replaceWord, 4, wxEXPAND | wxALL, 2);

	suggestionsList = new KaiListCtrl(this, ID_SUGGESTIONS_LIST, wxArrayString());
	replace = new MappedButton(this, ID_REPLACE, _("Zamień"));
	replaceAll = new MappedButton(this, ID_REPLACE_ALL, _("Zamień wszystko"));
	ignore = new MappedButton(this, ID_IGNORE, _("Ignoruj"));
	ignoreAll = new MappedButton(this, ID_IGNORE_ALL, _("Ignoruj wszystko"));
	addWord = new MappedButton(this, ID_ADD_WORD, _("Dodaj do słownika"));
	removeWord = new MappedButton(this, ID_REMOVE_WORD, _("Usuń ze słownika"));
	removeWord->SetToolTip(_("Usuwa ze słownika słowa dodane przez użytkownika."));

	close = new MappedButton(this, ID_CLOSE_DIALOG, _("Zamknij"));
	buttonSizer->Add(ignoreComments, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(ignoreUpper, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(replace, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(replaceAll, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(ignore, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(ignoreAll, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(addWord, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(removeWord, 0, wxEXPAND | wxALL, 2);
	buttonSizer->Add(close, 0, wxEXPAND | wxALL, 2);
	listSizer->Add(suggestionsList, 2, wxEXPAND | wxALL, 2);
	listSizer->Add(buttonSizer, 1, wxEXPAND | wxALL, 2);
	main->Add(misspellSizer, 0, wxEXPAND);
	main->Add(replaceSizer, 0, wxEXPAND);
	main->Add(listSizer, 0, wxEXPAND);
	SetSizerAndFit(main);
	SetNextMisspell();
	CenterOnParent();
	Show();

	Bind(LIST_ITEM_LEFT_CLICK, &SpellCheckerDialog::OnSelectSuggestion, this, ID_SUGGESTIONS_LIST);
	Bind(LIST_ITEM_DOUBLECLICKED, &SpellCheckerDialog::Replace, this, ID_SUGGESTIONS_LIST);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SpellCheckerDialog::Replace, this, ID_REPLACE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SpellCheckerDialog::ReplaceAll, this, ID_REPLACE_ALL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SpellCheckerDialog::Ignore, this, ID_IGNORE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SpellCheckerDialog::IgnoreAll, this, ID_IGNORE_ALL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SpellCheckerDialog::AddWord, this, ID_ADD_WORD);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SpellCheckerDialog::RemoveWord, this, ID_REMOVE_WORD);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Destroy();
	}, ID_CLOSE_DIALOG);
	Bind(wxEVT_ACTIVATE, &SpellCheckerDialog::OnActive, this);
	SetEscapeId(ID_CLOSE_DIALOG);
	SetEnterId(ID_REPLACE);

}

wxString SpellCheckerDialog::FindNextMisspell()
{
	//jakoś zidentyfikować, że użyszkodnik niczego nie zmienił
	bool noComments = ignoreComments->GetValue();
	bool ignoreUpperCase = ignoreUpper->GetValue();

	tab = Kai->GetTab();
	if (lastActiveLine != tab->Grid->currentLine){
		lastLine = lastActiveLine = tab->Grid->currentLine;
		lastMisspell = 0;
	}
	for (size_t i = lastLine; i < tab->Grid->GetCount(); i++){
		errors.clear();
		Dialogue *Dial = tab->Grid->GetDialogue(i);
		if (Dial->IsComment && noComments){ continue; }
		const wxString &Text = (tab->Grid->hasTLMode) ? Dial->TextTl : Dial->Text;
		//w checktext kopiuje tekst więc nie muszę robić tego dwa razy.
		CheckText(Text);
		if (i != lastLine){ lastMisspell = 0; }
		while (errors.size() > 1 && lastMisspell < errors.size()){
			wxString misspellWord = Text.SubString(errors[lastMisspell], errors[lastMisspell + 1]);
			lastMisspell += 2;
			if (ignoreUpperCase && IsAllUpperCase(misspellWord)){
				continue;
			}
			if (ignored.Index(misspellWord, false) == -1){
				lastLine = i;
				return misspellWord;
			}
		}
		lastLine = i;
	}
	return L"";
}

void SpellCheckerDialog::SetNextMisspell()
{
	wxString misspellWord = FindNextMisspell();
	misSpell->SetValue(misspellWord, true);
	if (misspellWord.IsEmpty()){
		//mamy brak błędów trzeba poinformować użyszkodnika i zablokować jakiekolwiek akcje.
		suggestionsList->SetTextArray(wxArrayString());
		blockOnActive = true;
		replaceWord->SetValue(L"", true);
		KaiMessageBox(_("Nie znaleziono więcej błędów pisowni"), _("Uwaga"), wxOK, this);
		return;
	}
	else{
		wxArrayString suggestions;
		SpellChecker::Get()->Suggestions(misspellWord, suggestions);
		suggestionsList->SetTextArray(suggestions);
		replaceWord->SetValue((suggestions.GetCount()) ? suggestions[0] : L"", true);
	}
	tab = Kai->GetTab();
	if (lastActiveLine != lastLine){
		tab->Grid->SelectRow(lastLine, false, true, true);
		tab->Grid->ScrollTo(lastLine, true);
		tab->Edit->SetLine(lastLine);
		lastActiveLine = lastLine;
	}
	else/* if(tab->Grid1->Edit->ebrow != lastActiveLine)*/{
		tab->Grid->ScrollTo(lastLine, true);
	}
	lastText = (tab->Grid->hasTLMode) ? tab->Edit->line->TextTl : tab->Edit->line->Text;
	//tab->Edit->TextEdit->SetFocus();
	tab->Edit->TextEdit->SetSelection(errors[lastMisspell - 2], errors[lastMisspell - 1] + 1);
}

void SpellCheckerDialog::Replace(wxCommandEvent &evt)
{
	if (evt.GetId() == ID_SUGGESTIONS_LIST){
		int sel = evt.GetInt();
		Item *item = suggestionsList->GetItem(sel, 0);
		if (item){
			replaceWord->SetValue(item->name, true);
		}
	}
	wxString replaceTxt = replaceWord->GetValue();
	if (replaceTxt.IsEmpty() || errors.size() < 2){ return; }
	tab = Kai->GetTab();
	Dialogue *Dial = tab->Grid->CopyDialogue(lastLine);
	wxString &Text = Dial->Text.CheckTlRef(Dial->TextTl, tab->Grid->hasTLMode);
	int start = errors[lastMisspell - 2];
	int end = errors[lastMisspell - 1] + 1;
	Text.replace(start, end - start, replaceTxt);
	tab->Grid->SetModified(SPELL_CHECKER);
	tab->Grid->Refresh(false);
	//if(SpellChecker::Get()->CheckWord(replaceTxt)){
	lastMisspell -= 2;
	//}
	SetNextMisspell();
}

void SpellCheckerDialog::ReplaceAll(wxCommandEvent &evt)
{
	wxString replaceTxt = replaceWord->GetValue();
	wxString misspellTxt = misSpell->GetValue();
	if (replaceTxt.IsEmpty() || misspellTxt.IsEmpty()){ return; }
	tab = Kai->GetTab();
	bool noComments = ignoreComments->GetValue();
	//usunąć to nieszczęsne std regex które ć widzi jako word boundary

	wxRegEx r(L"\\m" + misspellTxt.Lower() + L"\\M", wxRE_ADVANCED | wxRE_ICASE); // the pattern \m \M matches a word boundary
	if (!r.IsValid())
		return;

	wxString text;
	int lenMismatch = 0;
	int textPos = 0;

	for (size_t i = 0; i < tab->Grid->GetCount(); i++){
		Dialogue *Dial = tab->Grid->GetDialogue(i);
		if (Dial->IsComment && noComments){ continue; }
		wxString lineText = (tab->Grid->hasTLMode && Dial->TextTl != L"") ? Dial->TextTl : Dial->Text;
		text = lineText;
		//lenMismatch = 0;
		textPos = 0;
		bool changed = false;
		while (r.Matches(text)) {
			size_t pos = 0, len = 0;
			if (r.GetMatch(&pos, &len)){
				wxString misspellToReplace = lineText.Mid(pos + textPos, len);
				lineText.replace(pos + textPos, len, GetRightCase(replaceTxt, misspellToReplace));
				len = replaceTxt.length();
			}
			else
				continue;


			textPos = pos + len;
			text = lineText.Mid(textPos);
			changed = true;
		}
		if (changed){
			Dialogue *Dialc = tab->Grid->CopyDialogue(i);
			wxString &TextToChange = Dialc->Text.CheckTlRef(Dialc->TextTl, tab->Grid->hasTLMode);
			TextToChange = lineText;
			if(tab->Grid->SpellErrors.size() > i)
				tab->Grid->SpellErrors[i].clear();
		}
	}

	tab->Grid->SetModified(SPELL_CHECKER);
	tab->Grid->Refresh(false);
	//if(SpellChecker::Get()->CheckWord(replaceTxt)){
	lastMisspell -= 2;
	//}
	SetNextMisspell();
}

void SpellCheckerDialog::Ignore(wxCommandEvent &evt)
{
	SetNextMisspell();
}

void SpellCheckerDialog::IgnoreAll(wxCommandEvent &evt)
{
	ignored.Add(misSpell->GetValue());
	SetNextMisspell();
}

void SpellCheckerDialog::AddWord(wxCommandEvent &evt)
{
	SpellChecker::Get()->AddWord(misSpell->GetValue());
	if (lastMisspell > 0)
		lastMisspell -= 2;
	//we have checked for example first misspell and after add and check again the next misspell will be first;
	//decrease by 2, cause it gets only values divided by two.
	SetNextMisspell();
	Notebook::GetTab()->Edit->ClearErrs();
}

void SpellCheckerDialog::RemoveWord(wxCommandEvent &evt)
{
	wxArrayString addedMisspels;
	LoadAddedMisspels(addedMisspels);
	CustomCheckListBox *listOfAddedWords = new CustomCheckListBox(this, addedMisspels, _("Słowa dodane do słownika"));
	if (listOfAddedWords->ShowModal() == wxID_OK){
		wxArrayString checkedWords;
		listOfAddedWords->GetCheckedElements(checkedWords);
		SpellChecker::Get()->RemoveWords(checkedWords);
		Notebook::GetTab()->Edit->ClearErrs();
	}
}

void SpellCheckerDialog::OnSelectSuggestion(wxCommandEvent &evt)
{
	int sel = evt.GetInt();
	Item *item = suggestionsList->GetItem(sel, 0);
	if (item){
		replaceWord->SetValue(item->name, true);
	}
}

wxString SpellCheckerDialog::GetRightCase(const wxString &replaceWord, const wxString &misspellWord)
{
	int upperCase = 0;
	wxString result = replaceWord;
	size_t len = misspellWord.length();
	for (size_t i = 0; i < len; i++){
		if (iswupper(wint_t(misspellWord[i])) != 0)
			upperCase++;
	}
	if (upperCase == len)
		result.MakeUpper();
	else if (upperCase > 0)
		result.at(0) = wxToupper(result.at(0));

	return result;
}

bool SpellCheckerDialog::IsAllUpperCase(const wxString &word)
{
	size_t len = word.length();
	int upperCase = 0;
	for (size_t i = 0; i < len; i++){
		if (iswupper(wint_t(word[i])) != 0)
			upperCase++;
	}
	return (upperCase == len);
}

void SpellCheckerDialog::CheckText(wxString Text)
{
	Text += L" ";
	bool block = false;
	wxString word;
	int lasti = 0;
	int firsti = 0;

	for (size_t i = 0; i < Text.Len(); i++)
	{
		const wxUniChar &ch = Text[i];
		if (iswctype(wint_t(ch), _SPACE | _DIGIT | _PUNCT) && ch != L'\''/*notchar.Find(ch) != -1*/ && !block){
			if (word.Len() > 1){
				if (word.StartsWith(L"'")){ word = word.Remove(0, 1); }
				if (word.EndsWith(L"'")){ word = word.RemoveLast(1); }
				word.Trim(false);
				word.Trim(true);
				bool isgood = SpellChecker::Get()->CheckWord(word);
				if (!isgood){
					errors.push_back(firsti);
					errors.push_back(lasti);
				}
			}
			word = L""; firsti = i + 1;
		}

		if (ch == L'{'){
			block = true;
			continue;
		}
		else if (ch == L'}'){
			block = false;
			firsti = i + 1;
			word = L"";
			continue;
		}

		if (!block && (!iswctype(wint_t(ch), _SPACE | _DIGIT | _PUNCT) || ch == L'\'') /*notchar.Find(ch) == -1*/
			&& Text.GetChar((i == 0) ? 0 : i - 1) != L'\\'){
			word << ch; lasti = i;
		}
		else if (!block && Text.GetChar((i == 0) ? 0 : i - 1) == L'\\'){
			word = L"";
			if (ch == L'N' || ch == L'n' || ch == L'h'){
				firsti = i + 1;
			}
			else{
				firsti = i;
				word << ch;
			}
		}
	}
	if (errors.size() < 2){ errors.push_back(0); }
}

void SpellCheckerDialog::LoadAddedMisspels(wxArrayString &addedMisspels)
{
	wxString userpath = Options.pathfull + L"\\Dictionary\\UserDic.udic";
	if (wxFileExists(userpath)){
		OpenWrite op;
		wxString txt;
		if (!op.FileOpen(userpath, &txt, false)) { return; }
		wxStringTokenizer textIn(txt, L"\n");
		while (textIn.HasMoreTokens()) {
			// Read line
			wxString curLine = textIn.NextToken();
			curLine.Trim();
			addedMisspels.Add(curLine);
		}
	}
}

void SpellCheckerDialog::OnActive(wxActivateEvent &evt)
{
	if (evt.GetActive()){
		if (blockOnActive){ blockOnActive = false; return; }
		TabPanel *tab1 = Kai->GetTab();
		wxString &ActualText = tab1->Edit->line->Text.CheckTlRef(tab1->Edit->line->TextTl, tab->Grid->hasTLMode);
		if (tab != tab1 || lastActiveLine != tab1->Grid->currentLine || lastText != ActualText){
			lastMisspell = 0;
			SetNextMisspell();
		}
	}
}