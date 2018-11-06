//  Copyright (c) 2016-2017, Marcin Drob

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

#include "KainoteMain.h"
#include "KaiMessageBox.h"
#include "Stylelistbox.h"
#include "SelectLines.h"
#include "KaiStaticBoxSizer.h"
#include "config.h"
#include <wx/regex.h>
#include <wx/clipbrd.h>

SelectLines::SelectLines(KainoteFrame* kfparent)
	: KaiDialog((wxWindow*)kfparent, -1, _("Zaznacz"))
{
	Kai = kfparent;
	Options.GetTable(SelectionsRecent, selsRecent, L"\f", wxTOKEN_RET_EMPTY_ALL);
	int options = Options.GetInt(SelectionsOptions);
	if (selsRecent.size() > 20){ selsRecent.RemoveAt(20, selsRecent.size() - 20); }

	DialogSizer *slsizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *slrbsizer = new wxBoxSizer(wxHORIZONTAL);
	Contains = new KaiRadioButton(this, -1, _("Zawiera"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	NotContains = new KaiRadioButton(this, -1, _("Nie zawiera"));
	if (options & NOT_CONTAINS)
		NotContains->SetValue(true);
	else
		Contains->SetValue(true);

	slrbsizer->Add(Contains, 1, wxALL | wxEXPAND, 3);
	slrbsizer->Add(NotContains, 1, wxALL | wxEXPAND, 3);

	KaiStaticBoxSizer* slsbsizer = new KaiStaticBoxSizer(wxVERTICAL, this, _("Znajdź"));
	wxBoxSizer *sltpsizer = new wxBoxSizer(wxHORIZONTAL);
	FindText = new KaiChoice(this, -1, L"", wxDefaultPosition, wxSize(-1, 24), selsRecent);
	FindText->SetToolTip(_("Szukany tekst:"));
	FindText->SetMaxLength(MAXINT);
	ChooseStyles = new MappedButton(this, ID_CHOOSE_STYLES, L"+", -1, wxDefaultPosition, wxSize(24, 24));
	sltpsizer->Add(FindText, 1, wxALL | wxEXPAND, 3);
	sltpsizer->Add(ChooseStyles, 0, wxALL, 3);

	MatchCase = new KaiCheckBox(this, -1, _("Uwzględniaj wielkość liter"));
	MatchCase->SetValue((options & MATCH_CASE) > 0);
	RegEx = new KaiCheckBox(this, -1, _("Wyrażenia regularne"));
	RegEx->SetValue((options & REGULAR_EXPRESSIONS) > 0);

	slsbsizer->Add(slrbsizer, 0, wxEXPAND, 0);
	slsbsizer->Add(sltpsizer, 0, wxEXPAND, 0);
	slsbsizer->Add(MatchCase, 0, wxALL, 3);
	slsbsizer->Add(RegEx, 0, wxALL, 3);

	KaiStaticBoxSizer* slsbsizer1 = new KaiStaticBoxSizer(wxVERTICAL, this, _("W polu"));
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	CollumnText = new KaiRadioButton(this, -1, _("Tekst"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	CollumnStyle = new KaiRadioButton(this, -1, _("Styl"));
	CollumnActor = new KaiRadioButton(this, -1, _("Aktor"));
	CollumnEffect = new KaiRadioButton(this, -1, _("Efekt"));
	CollumnStartTime = new KaiRadioButton(this, -1, _("Czas Początkowy"));
	CollumnEndTime = new KaiRadioButton(this, -1, _("Czas Końcowy"));
	//catch first options, when there is more options it means that I did a bug or sameone change options
	if (options & FIELD_TEXT)
		CollumnText->SetValue(true);
	else if (options & FIELD_STYLE)
		CollumnStyle->SetValue(true);
	else if (options & FIELD_ACTOR)
		CollumnActor->SetValue(true);
	else if (options & FIELD_EFFECT)
		CollumnEffect->SetValue(true);
	else if (options & FIELD_START_TIME)
		CollumnStartTime->SetValue(true);
	else if (options & FIELD_END_TIME)
		CollumnEndTime->SetValue(true);

	sizer->Add(CollumnText, 1, wxALL, 3);
	sizer->Add(CollumnStyle, 1, wxALL, 3);
	sizer->Add(CollumnActor, 1, wxALL, 3);
	sizer->Add(CollumnEffect, 1, wxALL, 3);
	sizer1->Add(CollumnStartTime, 1, wxALL | wxEXPAND, 3);
	sizer1->Add(CollumnEndTime, 1, wxALL | wxEXPAND, 3);
	slsbsizer1->Add(sizer, 1, wxEXPAND);
	slsbsizer1->Add(sizer1, 1, wxEXPAND);
	KaiStaticBoxSizer* slsbsizer2 = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Dialogi / komentarze"));

	Dialogues = new KaiCheckBox(this, -1, _("Dialogi"));
	Dialogues->SetValue(options & DIALOGUES || !(options & COMMENTS));
	Comments = new KaiCheckBox(this, -1, _("Komentarze"));
	Comments->SetValue((options & COMMENTS) > 0);

	slsbsizer2->Add(Dialogues, 0, wxALL, 3);
	slsbsizer2->Add(Comments, 0, wxALL, 3);

	wxArrayString sels;
	sels.Add(_("Zaznacz"));
	sels.Add(_("Dodaj do zaznaczenia"));
	sels.Add(_("Odznacz"));

	Selections = new KaiRadioBox(this, -1, _("Zaznaczenie"), wxDefaultPosition, wxDefaultSize, sels, 2);
	int SelettionsOption = options & ADD_TO_SELECTION ? 1 : options & DESELECT ? 2 : 0;
	Selections->SetSelection(SelettionsOption);

	wxArrayString action;
	action.Add(_("Nie rób nic"));
	action.Add(_("Kopiuj"));
	action.Add(_("Wytnij"));
	action.Add(_("Przenieś na początek"));
	action.Add(_("Przenieś na koniec"));
	action.Add(_("Ustaw jako komentarz"));
	action.Add(_("Usuń"));

	Actions = new KaiRadioBox(this, -1, _("Akcja"), wxDefaultPosition, wxDefaultSize, action, 2);
	int ActionsOption = options & DO_COPY ? 1 :
		options & DO_CUT ? 2 :
		options & DO_MOVE_ON_START ? 3 :
		options & DO_MOVE_ON_END ? 4 :
		options & DO_SET_ASS_COMMENT ? 5 :
		options & DO_DELETE ? 6 : 0;
	Actions->SetSelection(ActionsOption);

	wxBoxSizer *slbtsizer = new wxBoxSizer(wxHORIZONTAL);
	Select = new MappedButton(this, ID_SELECTIONS, _("Zaznacz"));
	MappedButton *SelectOnAllTabs = new MappedButton(this, ID_SELECT_ON_ALL_TABS, _("Zaznacz na wszystkich zakładkach"));
	Close = new MappedButton(this, wxID_CANCEL, _("Zamknij"));
	slbtsizer->Add(Select, 1, wxALL, 5);
	slbtsizer->Add(SelectOnAllTabs, 0, wxALL, 5);
	slbtsizer->Add(Close, 1, wxALL, 5);


	slsizer->Add(slsbsizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
	slsizer->Add(slsbsizer1, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
	slsizer->Add(slsbsizer2, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
	slsizer->Add(Selections, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
	slsizer->Add(Actions, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
	slsizer->Add(slbtsizer, 0, wxALL | wxALIGN_CENTER, 0);

	SetSizerAndFit(slsizer);


	Connect(ID_SELECTIONS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SelectLines::OnSelect);
	Connect(ID_SELECT_ON_ALL_TABS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SelectLines::OnSelectInAllTabs);
	Connect(ID_CHOOSE_STYLES, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SelectLines::OnChooseStyles);

	//SetEscapeId(ID_CLOSE_SELECTIONS);
	SetEnterId(ID_SELECTIONS);
	CenterOnParent();
}

void SelectLines::SaveOptions()
{
	int options = 0;
	if (Contains->GetValue())
		options |= CONTAINS;
	if (NotContains->GetValue())
		options |= NOT_CONTAINS;
	if (MatchCase->GetValue())
		options |= MATCH_CASE;
	if (RegEx->GetValue())
		options |= REGULAR_EXPRESSIONS;
	if (CollumnText->GetValue())
		options |= FIELD_TEXT;
	if (CollumnStyle->GetValue())
		options |= FIELD_STYLE;
	if (CollumnActor->GetValue())
		options |= FIELD_ACTOR;
	if (CollumnEffect->GetValue())
		options |= FIELD_EFFECT;
	if (CollumnStartTime->GetValue())
		options |= FIELD_START_TIME;
	if (CollumnEndTime->GetValue())
		options |= FIELD_END_TIME;
	if (Dialogues->GetValue())
		options |= DIALOGUES;
	if (Comments->GetValue())
		options |= COMMENTS;

	int SelectResult = SELECT;
	for (int i = 0; i < Selections->GetSelection(); i++)
		SelectResult <<= 1;

	options |= SelectResult;

	int ActionsResult = DO_NOTHING;
	for (int i = 0; i < Actions->GetSelection(); i++)
		ActionsResult <<= 1;

	options |= ActionsResult;
	Options.SetInt(SelectionsOptions, options);
}

void SelectLines::OnSelect(wxCommandEvent & evt)
{
	selectColumn = TXT;
	if (CollumnStyle->GetValue()){ selectColumn = STYLE; }
	else if (CollumnActor->GetValue()){ selectColumn = ACTOR; }
	else if (CollumnEffect->GetValue()){ selectColumn = EFFECT; }
	else if (CollumnStartTime->GetValue()){ selectColumn = START; }
	else if (CollumnEndTime->GetValue()){ selectColumn = END; }

	find = FindText->GetValue();

	matchcase = MatchCase->GetValue();
	regex = RegEx->GetValue();
	contain = Contains->GetValue();
	notcont = NotContains->GetValue();
	selectDialogues = Dialogues->GetValue();
	selectComments = Comments->GetValue();
	selectOptions = Selections->GetSelection();
	action = Actions->GetSelection();

	TabPanel *tab = Kai->GetTab();
	bool refreshTabLabel = false;
	int allSelections = SelectOnTab(tab, &refreshTabLabel);

	wxString messagetxt = (selectOptions == 0) ? wxString::Format(_("Zaznaczono %i linijek."), allSelections) :
		(selectOptions == 1) ? wxString::Format(_("Dodano do zaznaczenia %i linijek."), allSelections) :
		wxString::Format(_("Odznaczono %i linijek."), allSelections);
	KaiMessageDialog dlg(this, messagetxt, _("Zaznacz"), wxYES_NO);
	dlg.SetYesLabel(_("Zamknij"));
	dlg.SetNoLabel(L"Ok");
	int result = dlg.ShowModal();
	if (result == wxYES){
		Hide();
	}
	AddRecent();
}

void SelectLines::OnSelectInAllTabs(wxCommandEvent& event)
{
	selectColumn = TXT;
	if (CollumnStyle->GetValue()){ selectColumn = STYLE; }
	else if (CollumnActor->GetValue()){ selectColumn = ACTOR; }
	else if (CollumnEffect->GetValue()){ selectColumn = EFFECT; }
	else if (CollumnStartTime->GetValue()){ selectColumn = START; }
	else if (CollumnEndTime->GetValue()){ selectColumn = END; }

	find = FindText->GetValue();

	matchcase = MatchCase->GetValue();
	regex = RegEx->GetValue();
	contain = Contains->GetValue();
	notcont = NotContains->GetValue();
	selectDialogues = Dialogues->GetValue();
	selectComments = Comments->GetValue();
	selectOptions = Selections->GetSelection();
	action = Actions->GetSelection();
	int selectionsOnAllTabs = 0;

	for (size_t i = 0; i < Kai->Tabs->Size(); i++){
		TabPanel *tab = Kai->Tabs->Page(i);
		bool refreshTabLabel = false;
		selectionsOnAllTabs += SelectOnTab(tab, &refreshTabLabel);
		if (refreshTabLabel)
			Kai->Label(tab->Grid->file->GetActualHistoryIter(), false, i, i != Kai->Tabs->iter);
	}

	wxString messagetxt = (selectOptions == 0) ? wxString::Format(_("Zaznaczono %i linijek."), selectionsOnAllTabs) :
		(selectOptions == 1) ? wxString::Format(_("Dodano do zaznaczenia %i linijek."), selectionsOnAllTabs) :
		wxString::Format(_("Odznaczono %i linijek."), selectionsOnAllTabs);
	KaiMessageDialog dlg(this, messagetxt, _("Zaznacz"), wxYES_NO);
	dlg.SetYesLabel(_("Zamknij"));
	dlg.SetNoLabel(L"Ok");
	int result = dlg.ShowModal();
	if (result == wxYES){
		Hide();
	}
	AddRecent();
}

int SelectLines::SelectOnTab(TabPanel *tab, bool *refreshTabLabel)
{
	int allSelections = 0;
	wxString txt, whatcopy;
	std::vector<Dialogue *> mdial;
	if (!matchcase){ find.MakeLower(); }
	tab->Grid->SaveSelections(selectOptions == 0);
	File *Subs = tab->Grid->file->GetSubs();
	bool skipFiltered = !tab->Grid->ignoreFiltered;

	for (int i = 0; i < Subs->dialogues.size(); i++)
	{
		Dialogue *Dial = Subs->dialogues[i];
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue){ continue; }

		if (selectColumn == STYLE){
			txt = Dial->Style;
		}
		else if (selectColumn == TXT){
			txt = (tab->Grid->hasTLMode && Dial->TextTl != L"") ? Dial->TextTl : Dial->Text;
		}
		else if (selectColumn == ACTOR){
			txt = Dial->Actor;
		}
		else if (selectColumn == EFFECT){
			txt = Dial->Effect;
		}
		else if (selectColumn == START){
			txt = Dial->Start.raw();
		}
		else if (selectColumn == END){
			txt = Dial->End.raw();
		}

		bool isfound = false;


		if (txt != L"" && find != L""){
			if (regex){
				int rxflags = wxRE_ADVANCED;
				if (!matchcase){ rxflags |= wxRE_ICASE; }
				wxRegEx rgx(find, rxflags);
				if (rgx.IsValid()) {
					if (rgx.Matches(txt)) {
						isfound = true;
					}
				}
			}
			else{
				if (!matchcase){ txt.MakeLower(); }
				if (txt.Find(find) != -1){ isfound = true; }

			}
		}
		else if (find == L"" && txt == L""){ isfound = true; }

		if (((isfound && contain) || (!isfound && !contain))
			&& ((selectDialogues && !Dial->IsComment) || (selectComments && Dial->IsComment))){
			bool select = (selectOptions == 2) ? false : true;
			if (select){
				tab->Grid->file->InsertSelectionKey(i);
				allSelections++;
			}
			else{
				if (tab->Grid->file->IsSelectedByKey(i)){
					tab->Grid->file->EraseSelectionKey(i);
					allSelections++;
				}
			}
		}

		if (tab->Grid->file->IsSelectedByKey(i) && action != 0){
			if (action < 3){ Dial->GetRaw(&whatcopy, tab->Grid->hasTLMode && Dial->TextTl != L""); }
			else if (action < 5){
				Dial->ChangeDialogueState(1);
				mdial.push_back(Dial);
			}
			else if (action < 6){
				Dialogue *dialc = tab->Grid->file->CopyDialogueByKey(i);
				dialc->ChangeDialogueState(1);
				dialc->IsComment = true;
			}
		}

	}

	//a teraz nasze kochane akcje
	//kopiowanie
	if (action == 1 || action == 2){
		if (wxTheClipboard->Open()){
			wxTheClipboard->SetData(new wxTextDataObject(whatcopy));
			wxTheClipboard->Close();
		}
	}//przenoszenie na początek / koniec
	if (action == 2 || action == 6 || action == 3 || action == 4){
		tab->Grid->DeleteRows();
		if (action == 3 || action == 4)
		{
			tab->Grid->InsertRows((action == 3) ? 0 : -1, mdial, false, true);
			mdial.clear();
		}
	}
	int firstSelected = tab->Grid->FirstSelection();
	int newCurrentLine = (firstSelected < 0) ? tab->Grid->currentLine : firstSelected;
	tab->Edit->SetLine(newCurrentLine);
	if (action > 1 && allSelections){
		tab->Grid->SetModified(SELECT_LINES, false);
		*refreshTabLabel = true;
	}
	else{
		*refreshTabLabel = false;
	}
	tab->Grid->RefreshColumns();

	return allSelections;
}

void SelectLines::AddRecent(){
	wxString text = FindText->GetValue();

	for (size_t i = 0; i < selsRecent.GetCount(); i++)
	{
		if (selsRecent[i] == text){
			selsRecent.RemoveAt(i);
			FindText->Delete(i);
		}
	}

	size_t selsSize = selsRecent.size();

	selsRecent.Insert(text, 0);
	FindText->Insert(text, 0);
	FindText->SetSelection(0);

	if (selsSize > 20){
		FindText->Delete(20, selsSize - 20);
		selsRecent.RemoveAt(20, selsSize - 20);
	}
	Options.SetTable(SelectionsRecent, selsRecent, L"\f");
}

void SelectLines::OnChooseStyles(wxCommandEvent& event)
{
	wxString styles = GetCheckedElements(Kai);
	int numreps = styles.Replace(L",", L"|");
	styles = L"^" + styles + L"$";
	CollumnStyle->SetValue(true);
	RegEx->SetValue(true);
	FindText->SetValue(styles);
}