//  Copyright (c) 2016 - 2026, Marcin Drob

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


#include "KainoteFrame.h"
#include "KaiMessageBox.h"
#include "Stylelistbox.h"
#include "SelectLines.h"
#include "KaiStaticBoxSizer.h"
#include "config.h"
#include "Notebook.h"
#include "TabPanel.h"
#include "SubsGrid.h"
#include "EditBox.h"
#include "Provider.h"
#include <wx/regex.h>
#include <wx/clipbrd.h>

SelectLines::SelectLines(KainoteFrame* kfparent)
	: KaiDialog((wxWindow*)kfparent, -1, _("Select"))
{
	Kai = kfparent;
	Options.GetTable(SELECT_LINES_RECENT_SELECTIONS, selsRecent, wxTOKEN_RET_EMPTY_ALL);
	int options = Options.GetInt(SELECT_LINES_OPTIONS);
	if (selsRecent.size() > 20){ selsRecent.RemoveAt(20, selsRecent.size() - 20); }

	DialogSizer *slsizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *slrbsizer = new wxBoxSizer(wxHORIZONTAL);
	Contains = new KaiRadioButton(this, -1, _("With"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	NotContains = new KaiRadioButton(this, -1, _("Without"));
	if (options & NOT_CONTAINS)
		NotContains->SetValue(true);
	else
		Contains->SetValue(true);

	slrbsizer->Add(Contains, 1, wxALL | wxEXPAND, 3);
	slrbsizer->Add(NotContains, 1, wxALL | wxEXPAND, 3);

	KaiStaticBoxSizer* slsbsizer = new KaiStaticBoxSizer(wxVERTICAL, this, _("Find"));
	wxBoxSizer *sltpsizer = new wxBoxSizer(wxHORIZONTAL);
	FindText = new KaiChoice(this, -1, emptyString, wxDefaultPosition, wxSize(-1, -1), selsRecent);
	FindText->SetToolTip(_("Search text:"));
	FindText->SetMaxLength(MAXINT);
	ChooseStyles = new MappedButton(this, ID_CHOOSE_STYLES, L"+", -1/*, wxDefaultPosition, wxSize(-1, -1)*/);
	sltpsizer->Add(FindText, 1, wxALL | wxEXPAND, 3);
	sltpsizer->Add(ChooseStyles, 0, wxALL, 3);

	MatchCase = new KaiCheckBox(this, -1, _("Match case"));
	MatchCase->SetValue((options & MATCH_CASE) > 0);
	RegEx = new KaiCheckBox(this, -1, _("Regular expressions"));
	RegEx->SetValue((options & REGULAR_EXPRESSIONS) > 0);

	slsbsizer->Add(slrbsizer, 0, wxEXPAND, 0);
	slsbsizer->Add(sltpsizer, 0, wxEXPAND, 0);
	slsbsizer->Add(MatchCase, 0, wxALL, 3);
	slsbsizer->Add(RegEx, 0, wxALL, 3);

	KaiStaticBoxSizer* slsbsizer1 = new KaiStaticBoxSizer(wxVERTICAL, this, _("In field"));
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	CollumnText = new KaiRadioButton(this, -1, _("Text"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	CollumnStyle = new KaiRadioButton(this, -1, _("Styles"));
	CollumnActor = new KaiRadioButton(this, -1, _("Actor"));
	CollumnEffect = new KaiRadioButton(this, -1, _("Effect"));
	CollumnStartTime = new KaiRadioButton(this, -1, _("Start time"));
	CollumnEndTime = new KaiRadioButton(this, -1, _("End time"));
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
	KaiStaticBoxSizer* slsbsizer2 = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Dialogue / comments"));

	Dialogues = new KaiCheckBox(this, -1, _("Dialogue"));
	Dialogues->SetValue(options & DIALOGUES || !(options & COMMENTS));
	Comments = new KaiCheckBox(this, -1, _("Comments"));
	Comments->SetValue((options & COMMENTS) > 0);

	slsbsizer2->Add(Dialogues, 0, wxALL, 3);
	slsbsizer2->Add(Comments, 0, wxALL, 3);

	wxArrayString sels;
	sels.Add(_("Select"));
	sels.Add(_("Add to selection"));
	sels.Add(_("Deselect"));

	Selections = new KaiRadioBox(this, -1, _("Selection"), wxDefaultPosition, wxDefaultSize, sels, 2);
	int SelettionsOption = options & ADD_TO_SELECTION ? 1 : options & DESELECT ? 2 : 0;
	Selections->SetSelection(SelettionsOption);

	wxArrayString action;
	action.Add(_("Do nothing"));
	action.Add(_("Copy"));
	action.Add(_("Cut"));
	action.Add(_("Move to beginning"));
	action.Add(_("Move to end"));
	action.Add(_("Set as comment"));
	action.Add(_("Delete"));

	Actions = new KaiRadioBox(this, -1, _("Action"), wxDefaultPosition, wxDefaultSize, action, 2);
	int ActionsOption = options & DO_COPY ? 1 :
		options & DO_CUT ? 2 :
		options & DO_MOVE_ON_START ? 3 :
		options & DO_MOVE_ON_END ? 4 :
		options & DO_SET_ASS_COMMENT ? 5 :
		options & DO_DELETE ? 6 : 0;
	Actions->SetSelection(ActionsOption);

	wxBoxSizer *slbtsizer = new wxBoxSizer(wxHORIZONTAL);
	Select = new MappedButton(this, ID_SELECTIONS, _("Select"));
	MappedButton *SelectOnAllTabs = new MappedButton(this, ID_SELECT_ON_ALL_TABS, _("Select in all tabs"));
	Close = new MappedButton(this, wxID_CANCEL, _("Close"));
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
	Options.SetInt(SELECT_LINES_OPTIONS, options);
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

	wxString messagetxt = (selectOptions == 0) ? wxString::Format(_("%i lines selected."), allSelections) :
		(selectOptions == 1) ? wxString::Format(_("%i lines added to selection."), allSelections) :
		wxString::Format(_("%i lines deselected."), allSelections);
	KaiMessageDialog dlg(this, messagetxt, _("Select"), wxYES_NO);
	dlg.SetYesLabel(_("Close"));
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
			Kai->Label(tab->grid->GetActualHistoryIter(), false, i, i != Kai->Tabs->iter);
	}

	wxString messagetxt = (selectOptions == 0) ? wxString::Format(_("%i lines selected."), selectionsOnAllTabs) :
		(selectOptions == 1) ? wxString::Format(_("%i lines added to selection."), selectionsOnAllTabs) :
		wxString::Format(_("%i lines deselected."), selectionsOnAllTabs);
	KaiMessageDialog dlg(this, messagetxt, _("Select"), wxYES_NO);
	dlg.SetYesLabel(_("Close"));
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
	if (!matchcase && !regex){ find.MakeLower(); }
	tab->grid->SaveSelections(selectOptions == 0);
	SubsFile *Subs = tab->grid;
	bool skipFiltered = !tab->grid->ignoreFiltered;
	wxRegEx rgx;
	if (regex){
		int rxflags = wxRE_ADVANCED;
		if (!matchcase){ rxflags |= wxRE_ICASE; }
		rgx.Compile(find, rxflags);
		if (!rgx.IsValid()) {
			return 0;
		}
	}

	for (size_t i = 0; i < Subs->GetCount(); i++)
	{
		Dialogue *Dial = Subs->GetDialogue(i);
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue){ continue; }

		if (selectColumn == STYLE){
			txt = Dial->Style;
		}
		else if (selectColumn == TXT){
			txt = (tab->grid->hasTLMode && Dial->TextTl != emptyString) ? Dial->TextTl : Dial->Text;
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


		if (txt != emptyString && find != emptyString){
			if (regex){
				if (rgx.Matches(txt)) {
					isfound = true;
				}
			}
			else{
				if (!matchcase){ txt.MakeLower(); }
				if (txt.Find(find) != -1){ isfound = true; }

			}
		}
		else if (find == emptyString && txt == emptyString){ isfound = true; }

		if (((isfound && contain) || (!isfound && !contain))
			&& ((selectDialogues && !Dial->IsComment) || (selectComments && Dial->IsComment))){
			bool select = (selectOptions == 2) ? false : true;
			if (select){
				tab->grid->InsertSelection(i);
				allSelections++;
			}
			else{
				if (tab->grid->IsSelected(i)){
					tab->grid->EraseSelection(i);
					allSelections++;
				}
			}
		}

		if (tab->grid->IsSelected(i) && action != 0){
			if (action < 3){ Dial->GetRaw(&whatcopy, tab->grid->hasTLMode && Dial->TextTl != emptyString); }
			else if (action < 5){
				Dialogue *copydial = Dial->Copy();
				//Dial->ChangeDialogueState(1);
				mdial.push_back(copydial);
			}
			else if (action < 6){
				Dialogue *dialc = tab->grid->CopyDialogueF(i);
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
		tab->grid->DeleteSelectedDialogues();
		tab->grid->SaveSelections(true);
		tab->grid->SpellErrors.clear();
		if ((action == 3 || action == 4) && mdial.size())
		{
			// we add lines to destroyer cause of it must be copied
			tab->grid->InsertRows((action == 3) ? 0 : -1, mdial, true);
			size_t size = tab->grid->GetCount();
			size_t mdialsize = size - mdial.size();
			tab->grid->InsertSelections((action == 3) ? 0 : mdialsize, 
				(action == 3) ? mdial.size() - 1 : size - 1);

			mdial.clear();
		}
		if (tab->grid->GetCount() < 1){ 
			tab->grid->AddLine(new Dialogue()); 
		}
	}
	size_t firstSelected = tab->grid->FirstSelection();
	if (firstSelected == -1) {
		size_t gridGetCount = tab->grid->GetCount();
		if (tab->grid->currentLine < gridGetCount){
			firstSelected = tab->grid->currentLine;
		}
		else {
			firstSelected = gridGetCount - 1;
		}
	}
	if (action > 1 && allSelections){
		tab->grid->SetModified(SELECT_LINES, true, false, firstSelected);
		*refreshTabLabel = true;
	}
	else{
		tab->edit->SetLine(firstSelected);
		*refreshTabLabel = false;
	}
	tab->grid->RefreshColumns();

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
	Options.SetTable(SELECT_LINES_RECENT_SELECTIONS, selsRecent);
}

void SelectLines::OnChooseStyles(wxCommandEvent& event)
{
	wxString styles = GetCheckedElements(Kai);
	styles.Replace(L"\\", L"\\\\");
	styles.Replace(L"|", L"\\|");
	int numreps = styles.Replace(L",", L"|");
	styles.Replace(L"[", L"\\[");
	styles.Replace(L"]", L"\\]");
	styles.Replace(L"(", L"\\(");
	styles.Replace(L")", L"\\)");
	styles.Replace(L"*", L"\\*");
	styles.Replace(L"+", L"\\+");
	styles.Replace(L".", L"\\.");
	styles = L"^" + styles + L"$";
	
	CollumnStyle->SetValue(true);
	RegEx->SetValue(true);
	FindText->SetValue(styles);
}
