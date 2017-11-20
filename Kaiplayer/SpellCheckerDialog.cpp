//  Copyright (c) 2016, Marcin Drob

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
#include <regex>

SpellCheckerDialog::SpellCheckerDialog(kainoteFrame *parent)
	:KaiDialog((wxWindow*)parent,-1, _("Sprawdzanie pisowni"))
	,Kai(parent)
	,lastLine(0)
	,lastMisspell(0)
	,lastActiveLine(-1)
{
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *misspellSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *replaceSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *listSizer = new wxBoxSizer(wxHORIZONTAL);
	ignoreComments = new KaiCheckBox(this, -1, _("Ignoruj komentarze"));
	ignoreUpper = new KaiCheckBox(this, -1, _("Ignoruj słowa z duzej litery"));
	ignoreUpper->Enable(false);
	//wxString misspellWord = FindNextMisspell();
	misSpell = new KaiTextCtrl(this,-1,"");
	replaceWord = new KaiTextCtrl(this,-1);
	misspellSizer->Add(new KaiStaticText(this,-1, _("Błędne słowo:")), 1, wxEXPAND|wxALL, 2);
	misspellSizer->Add(misSpell, 4, wxEXPAND|wxALL, 2);
	replaceSizer->Add(new KaiStaticText(this,-1, _("Zmień na:")), 1, wxEXPAND|wxALL, 2);
	replaceSizer->Add(replaceWord, 4, wxEXPAND|wxALL, 2);
	suggestionsList = new KaiListCtrl(this, ID_SUGGESTIONS_LIST, wxArrayString());
	replace = new MappedButton(this, ID_REPLACE, _("Zamień"));
	replaceAll = new MappedButton(this, ID_REPLACE_ALL, _("Zamień wszystko"));
	ignore = new MappedButton(this, ID_IGNORE, _("Ignoruj"));
	ignoreAll = new MappedButton(this, ID_IGNORE_ALL, _("Ignoruj wszystko"));
	addWord = new MappedButton(this, ID_ADD_WORD, _("Dodaj do słownika"));
	removeWord = new MappedButton(this, ID_REMOVE_WORD, _("Usuń ze słownika"));
	removeWord->Enable(false);
	close = new MappedButton(this, ID_CLOSE_DIALOG, _("Zamknij"));
	buttonSizer->Add(ignoreComments, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(ignoreUpper, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(replace, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(replaceAll, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(ignore, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(ignoreAll, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(addWord, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(removeWord, 0, wxEXPAND|wxALL, 2);
	buttonSizer->Add(close, 0, wxEXPAND|wxALL, 2);
	listSizer->Add(suggestionsList, 2, wxEXPAND|wxALL, 2);
	listSizer->Add(buttonSizer, 1, wxEXPAND|wxALL, 2);
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
	
	tab = Kai->GetTab();
	if(lastActiveLine != tab->Edit->ebrow){
		lastLine = lastActiveLine = tab->Edit->ebrow;
		lastMisspell = 0;
	}
	for(int i = lastLine; i < tab->Grid->GetCount(); i++){
		errors.clear();
		Dialogue *Dial = tab->Grid->GetDialogue(i);
		if(Dial->IsComment && noComments){continue;}
		wxString &Text = (tab->Grid->hasTLMode)? Dial->TextTl : Dial->Text;
		//w checktext kopiuje tekst więc nie muszę robić tego dwa razy.
		tab->Grid->CheckText(Text, errors);
		if(i != lastLine){lastMisspell=0;}
		while(errors.size()>1 && lastMisspell < errors.size()){
			wxString misspellWord = Text.SubString(errors[lastMisspell], errors[lastMisspell+1]);
			lastMisspell += 2;
			if(ignored.Index(misspellWord, false) == -1){
				lastLine = i;
				return misspellWord;
			}
		}
		lastLine = i;
	}
	return "";
}
	
void SpellCheckerDialog::SetNextMisspell()
{
	wxString misspellWord = FindNextMisspell();
	misSpell->SetValue(misspellWord,true);
	replaceWord->SetValue("",true);
	if(misspellWord.IsEmpty()){
		//mamy brak błędów trzeba poinformować użyszkodnika i zablokować jakiekolwiek akcje.
		suggestionsList->SetTextArray(wxArrayString());
		KaiMessageBox(_("Nie znaleziono więcej błędów pisowni"), _("Uwaga"), wxOK, this);
		return;
	}else{
		suggestionsList->SetTextArray(SpellChecker::Get()->Suggestions(misspellWord));
	}
	tab = Kai->GetTab();
	if(lastActiveLine != lastLine){
		tab->Grid->SelectRow(lastLine,false,true,true);
		tab->Grid->ScrollTo(lastLine,true);
		tab->Edit->SetLine(lastLine);
		lastActiveLine = lastLine;
	}else/* if(tab->Grid1->Edit->ebrow != lastActiveLine)*/{
		tab->Grid->ScrollTo(lastLine,true);
	}
	lastText = (tab->Grid->hasTLMode)? tab->Edit->line->TextTl : tab->Edit->line->Text;
	//tab->Edit->TextEdit->SetFocus();
	tab->Edit->TextEdit->SetSelection(errors[lastMisspell-2], errors[lastMisspell-1]+1);
}
	
void SpellCheckerDialog::Replace(wxCommandEvent &evt)
{
	if(evt.GetId() == ID_SUGGESTIONS_LIST){
		int sel = evt.GetInt();
		Item *item = suggestionsList->GetItem(sel, 0);
		if(item){
			replaceWord->SetValue(item->name, true);
		}
	}
	wxString replaceTxt = replaceWord->GetValue();
	if(replaceTxt.IsEmpty() || errors.size()<2){return;}
	tab = Kai->GetTab();
	Dialogue *Dial = tab->Grid->CopyDialogue(lastLine);
	wxString &Text = (tab->Grid->hasTLMode)? Dial->TextTl : Dial->Text;
	int start = errors[lastMisspell-2];
	int end = errors[lastMisspell-1]+1;
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
	if(replaceTxt.IsEmpty() || misspellTxt.IsEmpty()){return;}
	tab = Kai->GetTab();
	bool noComments = ignoreComments->GetValue();
	std::wregex r("\\b" + misspellTxt.Lower().ToStdWstring() + "\\b"); // the pattern \b matches a word boundary
	std::wsmatch m;
	std::wstring text;
	int lenMismatch = 0;
	int textPos = 0;
	
	for(int i = 0; i < tab->Grid->GetCount(); i++){
		Dialogue *Dial = tab->Grid->GetDialogue(i);
		if(Dial->IsComment && noComments){continue;}
		wxString Text = (tab->Grid->hasTLMode)? Dial->TextTl : Dial->Text;
		text = Text.Lower().ToStdWstring();
		lenMismatch = 0;
		textPos = 0;
		bool changed = false;
		while(std::regex_search(text, m, r)) {
			int pos = m.position(0) + textPos;
			int len = m.length(0);
			//zrób coś z tymi danymi
			wxString misspellToReplace = Text.Mid(pos - lenMismatch, len);
			Text.replace(pos - lenMismatch, len, GetRightCase(replaceTxt, misspellToReplace));
			lenMismatch += (len - replaceTxt.Len()) + 1;
			text = m.suffix().str();
			text.insert(0, L" ");
			textPos = pos + len;
			changed=true;
		}
		if(changed){
			Dialogue *Dialc = tab->Grid->CopyDialogue(i);
			wxString &TextToChange = (tab->Grid->hasTLMode)? Dialc->TextTl : Dialc->Text;
			TextToChange=Text;
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
	SetNextMisspell();
}
	
void SpellCheckerDialog::RemoveWord(wxCommandEvent &evt)
{
	// na razie shithappens, bo nie wiem jeszcze co z tym fantem zrobić

}

void SpellCheckerDialog::OnSelectSuggestion(wxCommandEvent &evt)
{
	int sel = evt.GetInt();
	Item *item = suggestionsList->GetItem(sel, 0);
	if(item){
		replaceWord->SetValue(item->name, true);
	}
}

wxString SpellCheckerDialog::GetRightCase(const wxString &replaceWord, const wxString &misspellWord)
{
	wxString firstCharacterR = replaceWord.Mid(0, 1);
	wxString firstCharacterM = misspellWord.Mid(0, 1);
	wxString firstCharacterRL = firstCharacterR.Lower();
	wxString firstCharacterML = firstCharacterM.Lower();
	bool replaceIsLower = (firstCharacterR == firstCharacterRL);
	bool misspellIsLower = (firstCharacterM == firstCharacterML);

	if(replaceIsLower != misspellIsLower){
		if(misspellIsLower){
			return firstCharacterRL + replaceWord.Mid(1);
		}else{
			return firstCharacterR.Upper() + replaceWord.Mid(1);
		}
	}
	return replaceWord;
}

void SpellCheckerDialog::OnActive(wxActivateEvent &evt)
{
	if(evt.GetActive()){
		TabPanel *tab1 = Kai->GetTab();
		wxString &ActualText = (tab1->Grid->hasTLMode)? tab1->Edit->line->TextTl : tab1->Edit->line->Text;
		if(tab != tab1 || lastActiveLine != tab1->Edit->ebrow || lastText != ActualText){
			lastMisspell=0;
			SetNextMisspell();
		}
	}
}