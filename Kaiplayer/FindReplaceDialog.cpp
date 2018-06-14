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

#include "FindReplaceDialog.h"
#include "FindReplace.h"
#include "KainoteMain.h"
#include "Stylelistbox.h"

TabWindow::TabWindow(wxWindow *parent, int id, int tabNum, FindReplace * _FR)
	: wxWindow(parent, id)
	, windowType(tabNum)
	, FR(_FR)
{
	int options = Options.GetInt(FindReplaceOptions);

	wxBoxSizer* mainfrbsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainfrbsizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainfrbsizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* mainfrbsizer3 = new wxBoxSizer(wxHORIZONTAL);

	//find list and description
	wxBoxSizer* frsbsizer = new wxBoxSizer(wxHORIZONTAL);
	FindText = new KaiChoice(this, ID_FIND_TEXT, "", wxDefaultPosition, wxDefaultSize, FR->findRecent);
	FindText->SetToolTip(_("Szukany tekst:"));
	FindText->SetMaxLength(MAXINT);
	frsbsizer->Add(new KaiStaticText(this, -1, _("Szukany tekst:")), 1, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT, 4, 0);
	frsbsizer->Add(FindText, 4, wxEXPAND, 0);
	mainfrbsizer1->Add(frsbsizer, 0, wxEXPAND | wxALL, 4);
	///TODO: add event function
	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent &evt){}, ID_FIND_TEXT);

	if (tabNum != WINDOW_FIND){
		//replace list and description blocked on window find
		wxBoxSizer *ReplaceStaticSizer = new wxBoxSizer(wxHORIZONTAL);
		ReplaceText = new KaiChoice(this, ID_REPLACE_TEXT, "", wxDefaultPosition, wxDefaultSize, FR->replaceRecent);
		ReplaceText->SetToolTip(_("Zamieñ na:"));
		ReplaceText->SetMaxLength(MAXINT);
		KaiStaticText *repDescText = new KaiStaticText(this, -1, _("Zamieñ na:"));
		ReplaceStaticSizer->Add(repDescText, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT, 4);
		ReplaceStaticSizer->Add(ReplaceText, 4, wxEXPAND, 0);
		mainfrbsizer1->Add(ReplaceStaticSizer, 0, wxEXPAND | wxALL, 4);
	}
	if (tabNum == WINDOW_FIND_IN_SUBS){
		wxBoxSizer *SubsFilterStaticSizer = new wxBoxSizer(wxHORIZONTAL);
		FindInSubsPattern = new KaiChoice(this, ID_REPLACE_TEXT, "", wxDefaultPosition, wxDefaultSize, FR->subsFindingFilters);
		FindInSubsPattern->SetToolTip(_("Filtry wyszukiwania windows:"));
		FindInSubsPattern->SetMaxLength(1000);
		SubsFilterStaticSizer->Add(new KaiStaticText(this, -1, _("Filtry:")), 1, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT, 4);
		SubsFilterStaticSizer->Add(FindInSubsPattern, 4, wxEXPAND, 0);
		mainfrbsizer1->Add(SubsFilterStaticSizer, 0, wxEXPAND | wxALL, 4);

		wxBoxSizer *FindInSubsPathStaticSizer = new wxBoxSizer(wxHORIZONTAL);
		FindInSubsPath = new KaiChoice(this, ID_REPLACE_TEXT, "", wxDefaultPosition, wxDefaultSize, FR->subsFindingPaths);
		FindInSubsPath->SetToolTip(_("Katalog szukania napisów:"));
		FindInSubsPath->SetMaxLength(MAXINT);
		FindInSubsPathStaticSizer->Add(new KaiStaticText(this, -1, _("Katalog:")), 1, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT, 4);
		FindInSubsPathStaticSizer->Add(FindInSubsPath, 4, wxEXPAND, 0);
		mainfrbsizer1->Add(FindInSubsPathStaticSizer, 0, wxEXPAND | wxALL, 4);
	}
	wxBoxSizer* frbsizer1 = new wxBoxSizer(wxVERTICAL);
	MatchCase = new KaiCheckBox(this, -1, _("Uwzglêdniaj wielkoœæ liter"));
	MatchCase->SetValue(options & CASE_SENSITIVE);
	RegEx = new KaiCheckBox(this, -1, _("Wyra¿enia regularne"));
	RegEx->SetValue((options & REG_EX) > 0);
	StartLine = new KaiCheckBox(this, ID_START_OF_LINE, _("Pocz¹tek tekstu"));
	StartLine->SetValue((options & START_OF_TEXT) > 0);
	EndLine = new KaiCheckBox(this, ID_END_OF_LINE, _("Koniec tekstu"));
	EndLine->SetValue((options & END_OF_TEXT) > 0);
	frbsizer1->Add(MatchCase, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	frbsizer1->Add(RegEx, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	frbsizer1->Add(StartLine, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	frbsizer1->Add(EndLine, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TabWindow::OnRecheck, this, ID_START_OF_LINE);
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TabWindow::OnRecheck, this, ID_END_OF_LINE);
	//in field
	KaiStaticBoxSizer* frsbsizer2 = new KaiStaticBoxSizer(wxVERTICAL, this, _("W polu"));
	wxBoxSizer* frbsizer2 = new wxBoxSizer(wxHORIZONTAL);

	CollumnText = new KaiRadioButton(this, -1, _("Tekst"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	if (options & IN_FIELD_TEXT)
		CollumnText->SetValue(true);
	CollumnTextOriginal = new KaiRadioButton(this, -1, (tabNum == WINDOW_FIND_IN_SUBS) ? _("Zwyk³y tekst") : _("Tekst orygina³u"));
	CollumnTextOriginal->Enable((tabNum == WINDOW_FIND_IN_SUBS) ? true : FR->Kai->GetTab()->Grid->hasTLMode);
	if (options & IN_FIELD_TEXT_ORIGINAL && CollumnTextOriginal->IsEnabled())
		CollumnTextOriginal->SetValue(true);
	frbsizer2->Add(CollumnText, 1, wxALL, 1);
	frbsizer2->Add(CollumnTextOriginal, 2, wxALL, 1);

	wxBoxSizer* frbsizer3 = new wxBoxSizer(wxHORIZONTAL);
	CollumnStyle = new KaiRadioButton(this, -1, _("Styl"));
	if (options & IN_FIELD_STYLE)
		CollumnStyle->SetValue(true);
	CollumnActor = new KaiRadioButton(this, -1, _("Aktor"));
	if (options & IN_FIELD_ACTOR)
		CollumnActor->SetValue(true);
	CollumnEffect = new KaiRadioButton(this, -1, _("Efekt"));
	if (options & IN_FIELD_EFFECT)
		CollumnEffect->SetValue(true);

	frbsizer3->Add(CollumnStyle, 1, wxEXPAND | wxLEFT, 1);
	frbsizer3->Add(CollumnActor, 1, wxEXPAND | wxLEFT, 1);
	frbsizer3->Add(CollumnEffect, 1, wxEXPAND | wxLEFT, 1);

	//static box sizer dodanie pierwszego i drugiego rzêdu
	frsbsizer2->Add(frbsizer2, 1, wxEXPAND | wxLEFT, 2);
	frsbsizer2->Add(frbsizer3, 1, wxEXPAND | wxLEFT, 2);
	//po³¹czenie chceckboxów i radiobutonów z wyborem pola
	mainfrbsizer2->Add(frbsizer1, 1, wxEXPAND, 0);
	mainfrbsizer2->Add(frsbsizer2, 1, wxEXPAND, 0);

	//po³¹czenie wczeœniejszego sizera z znajdŸ i zmieñ
	//dwie poni¿sze linijki s¹ na samym pocz¹tku

	mainfrbsizer1->Add(mainfrbsizer2, 0, wxEXPAND | wxLEFT, 1);
	//buttons
	//pionowy sizer kolumna 2
	wxBoxSizer* frbsizer = new wxBoxSizer(wxVERTICAL);
	if (tabNum != WINDOW_FIND_IN_SUBS){
		MappedButton *ButtonFind = new MappedButton(this, ID_BUTTON_FIND, _("ZnajdŸ")/*, -1, wxDefaultPosition, wxSize(124, -1)*/);
		frbsizer->Add(ButtonFind, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, (tabNum == WINDOW_REPLACE) ? 2 : 4);
		
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){FR->OnFind(this); }, ID_BUTTON_FIND);
	}
	if (tabNum == WINDOW_FIND){
		MappedButton *ButtonFindInAllOpenedSubs = new MappedButton(this, ID_BUTTON_FIND_IN_ALL_OPENED_SUBS, _("ZnajdŸ we wszystkich\notwartych napisach"));
		MappedButton *ButtonFindAllInCurrentSubs = new MappedButton(this, ID_BUTTON_FIND_ALL_IN_CURRENT_SUBS, _("Zamieñ wszystko\nw bierz¹cych napisach"));
		
		frbsizer->Add(ButtonFindInAllOpenedSubs, 0, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);
		frbsizer->Add(ButtonFindAllInCurrentSubs, 0, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);

		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->FindInAllOpenedSubs(this); }, ID_BUTTON_FIND_IN_ALL_OPENED_SUBS);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->FindAllInCurrentSubs(this); }, ID_BUTTON_FIND_ALL_IN_CURRENT_SUBS);
	}
	else if (tabNum == WINDOW_REPLACE){
		MappedButton *ButtonReplaceNext = new MappedButton(this, ID_BUTTON_REPLACE, _("Zamieñ nastêpne"));
		MappedButton *ButtonReplaceAll = new MappedButton(this, ID_BUTTON_REPLACE_ALL, _("Zamieñ wszystko"));
		MappedButton *ButtonReplaceOnAllTabs = new MappedButton(this, ID_BUTTON_REPLACE_IN_ALL_OPENED_SUBS, _("Zamieñ we wszystkich\notwartch napisach"));

		frbsizer->Add(ButtonReplaceNext, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 2);
		frbsizer->Add(ButtonReplaceAll, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 2);
		frbsizer->Add(ButtonReplaceOnAllTabs, 0, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 2);

		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->Replace(this); }, ID_BUTTON_REPLACE);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->ReplaceAll(this); }, ID_BUTTON_REPLACE_ALL);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->ReplaceInAllOpenedSubs(this); }, ID_BUTTON_REPLACE_IN_ALL_OPENED_SUBS);
	}
	else if (tabNum == WINDOW_FIND_IN_SUBS){
		MappedButton *ButtonFindInSubs = new MappedButton(this, ID_BUTTON_FIND_IN_SUBS, _("ZnajdŸ w napisach")/*, -1, wxDefaultPosition, wxSize(124, -1)*/);
		MappedButton *ButtonReplaceInSubs = new MappedButton(this, ID_BUTTON_REPLACE_IN_SUBS, _("Zamieñ w napisach")/*, -1, wxDefaultPosition, wxSize(124, -1)*/);
		frbsizer->Add(ButtonFindInSubs, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);
		frbsizer->Add(ButtonReplaceInSubs, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);

		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->FindInSubs(this); }, ID_BUTTON_FIND_IN_SUBS);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->ReplaceInSubs(this); }, ID_BUTTON_REPLACE_IN_SUBS);
	}

	MappedButton *ButtonClose = new MappedButton(this, ID_BUTTON_CLOSE, _("Zamknij"));
	frbsizer->Add(ButtonClose, 1, wxEXPAND | wxTOP | wxRIGHT, (tabNum == WINDOW_REPLACE) ? 2 : 4);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){ FR->OnClose(); }, ID_BUTTON_CLOSE);

	//³¹czenie ca³oœci znajdowania i opcji z przyciskami
	mainfrbsizer3->Add(mainfrbsizer1, 0, wxEXPAND | wxRIGHT, 2);
	mainfrbsizer3->Add(frbsizer, 1, wxEXPAND | wxLEFT, 3);
	mainfrbsizer->Add(mainfrbsizer3, 0, wxEXPAND | wxALL, 5);

	if (tabNum != WINDOW_FIND_IN_SUBS){
		//poziomy sizer spód
		KaiStaticBoxSizer* frsbsizer3 = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Linijki"));

		AllLines = new KaiRadioButton(this, 23156, _("Wszystkie linijki"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		if (options & IN_LINES_ALL)
			AllLines->SetValue(true);
		SelectedLines = new KaiRadioButton(this, 23157, _("Zaznaczone linijki"));
		if (options & IN_LINES_SELECTED)
			SelectedLines->SetValue(true);
		FromSelection = new KaiRadioButton(this, 23158, _("Od zaznaczonej"));
		if (options & IN_LINES_FROM_SELECTION)
			FromSelection->SetValue(true);

		Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &TabWindow::Reset, this, 23156);
		Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &TabWindow::Reset, this, 23157);
		Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &TabWindow::Reset, this, 23158);

		wxBoxSizer* frbsizer4 = new wxBoxSizer(wxHORIZONTAL);
		MappedButton *ButtonChooseStyle = new MappedButton(this, ID_BUTTON_CHOOSE_STYLE, "+", -1, wxDefaultPosition, wxSize(22, 22));
		ChoosenStyleText = new KaiTextCtrl(this, ID_CHOOSEN_STYLE_TEXT, "", wxDefaultPosition, wxSize(-1, 22));
		frbsizer4->Add(ButtonChooseStyle, 0, 0, 0);
		frbsizer4->Add(ChoosenStyleText, 0, wxLEFT, 3);

		frsbsizer3->Add(AllLines, 1, wxALL | wxEXPAND, 2);
		frsbsizer3->Add(SelectedLines, 1, wxALL | wxEXPAND, 2);
		frsbsizer3->Add(FromSelection, 1, wxALL | wxEXPAND, 2);
		frsbsizer3->Add(frbsizer4, 0, wxALL, 2);

		mainfrbsizer->Add(frsbsizer3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TabWindow::OnStylesChoose, this, ID_BUTTON_CHOOSE_STYLE);
	}
	
	//Ustawienie sizera
	SetSizerAndFit(mainfrbsizer);

	//commands
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TabWindow::OnRecheck, this, ID_START_OF_LINE);
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TabWindow::OnRecheck, this, ID_END_OF_LINE);
}

void TabWindow::SaveValues()
{
	int options = Options.GetInt(FindReplaceOptions);
	options = options >> 9;
	options = options << 9;

	if (MatchCase->GetValue())
		options |= CASE_SENSITIVE;
	if (RegEx->GetValue())
		options |= REG_EX;
	if (StartLine->GetValue())
		options |= START_OF_TEXT;
	if (EndLine->GetValue())
		options |= END_OF_TEXT;
	if (CollumnText->GetValue())
		options |= IN_FIELD_TEXT;
	if (CollumnTextOriginal->GetValue())
		options |= IN_FIELD_TEXT_ORIGINAL;
	if (CollumnStyle->GetValue())
		options |= IN_FIELD_STYLE;
	if (CollumnActor->GetValue())
		options |= IN_FIELD_ACTOR;
	if (CollumnEffect->GetValue())
		options |= IN_FIELD_EFFECT;
	if (AllLines && AllLines->GetValue())
		options |= IN_LINES_ALL;
	if (SelectedLines && SelectedLines->GetValue())
		options |= IN_LINES_SELECTED;
	if (FromSelection && FromSelection->GetValue())
		options |= IN_LINES_FROM_SELECTION;

	Options.SetInt(FindReplaceOptions, options);
}

void TabWindow::SetValues()
{
	int options = Options.GetInt(FindReplaceOptions);
	MatchCase->SetValue((options & CASE_SENSITIVE) > 0);
	RegEx->SetValue((options & REG_EX) > 0);
	StartLine->SetValue((options & START_OF_TEXT) > 0);
	EndLine->SetValue((options & END_OF_TEXT) > 0);
	CollumnText->SetValue((options & IN_FIELD_TEXT) > 0);
	CollumnTextOriginal->SetValue((options & IN_FIELD_TEXT_ORIGINAL) > 0);
	CollumnStyle->SetValue((options & IN_FIELD_STYLE) > 0);
	CollumnActor->SetValue((options & IN_FIELD_ACTOR) > 0);
	CollumnEffect->SetValue((options & IN_FIELD_EFFECT) > 0);
	
	if (AllLines)
		AllLines->SetValue((options & IN_LINES_ALL) > 0);
	if (SelectedLines)
		SelectedLines->SetValue((options & IN_LINES_SELECTED) > 0);
	if (FromSelection)
		FromSelection->SetValue((options & IN_LINES_FROM_SELECTION) > 0);
}

void TabWindow::OnRecheck(wxCommandEvent& event)
{
	int id = event.GetId();
	if (id == ID_START_OF_LINE && EndLine->GetValue()){
		EndLine->SetValue(false);
	}
	else if (id == ID_END_OF_LINE && StartLine->GetValue()){
		StartLine->SetValue(false);
	}

}

void TabWindow::Reset(wxCommandEvent& evt)
{
	FR->fromstart = true;
	FR->fnext = false;
	if (CollumnTextOriginal->GetValue()){ CollumnText->SetValue(true); }
	CollumnTextOriginal->Enable(FR->Kai->GetTab()->Grid->hasTLMode);
}

void TabWindow::OnStylesChoose(wxCommandEvent& event)
{
	ChoosenStyleText->SetValue(GetCheckedElements(FR->Kai));
}

FindReplaceDialog::FindReplaceDialog(KainoteFrame *_Kai, int whichWindow)
	:KaiDialog(_Kai, -1, (whichWindow == WINDOW_FIND) ? _("ZnajdŸ") : (whichWindow == WINDOW_REPLACE) ? _("ZnajdŸ i zamieñ") : _("ZnajdŸ w napisach"))
	, Kai(_Kai)
{
	FR = new FindReplace(Kai, this);
	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("SEARCH"));
	SetIcon(icn);
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));

	DialogSizer *mainfrbsizer = new DialogSizer(wxVERTICAL);

	findReplaceTabs = new KaiTabBar(this, -1);
	findReplaceTabs->AddTab(new TabWindow(findReplaceTabs, -1, WINDOW_FIND, FR), _("ZnajdŸ"));
	findReplaceTabs->AddTab(new TabWindow(findReplaceTabs, -1, WINDOW_REPLACE, FR), _("ZnajdŸ i zamieñ"));
	findReplaceTabs->AddTab(new TabWindow(findReplaceTabs, -1, WINDOW_FIND_IN_SUBS, FR), _("ZnajdŸ w napisach"));
	findReplaceTabs->Fit();
	findReplaceTabs->Bind(BEFORE_CHANGING_TAB, [=](wxCommandEvent &evt){
		TabWindow *currentTab = GetTab();
		currentTab->SaveValues();
	});
	findReplaceTabs->Bind(TAB_CHANGED, [=](wxCommandEvent &evt){
		TabWindow *currentTab = GetTab();
		currentTab->SetValues();
		SetLabel(findReplaceTabs->GetTabName());
	});
	mainfrbsizer->Add(findReplaceTabs, 0, wxEXPAND | wxALL, 2);
	SetSizerAndFit(mainfrbsizer);

	Bind(wxEVT_ACTIVATE, &FindReplaceDialog::OnActivate, this);
	//Connect(ID_BCLOSE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FindReplace::OnClose);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &FindReplaceDialog::OnEnterConfirm, this, ID_ENTER_CONFIRM);
	SetEscapeId(ID_BUTTON_CLOSE);
	SetEnterId(ID_ENTER_CONFIRM);
	CenterOnParent();

	wxAcceleratorEntry entries[2];
	entries[0] = Hkeys.GetHKey(idAndType(Search));
	entries[1] = Hkeys.GetHKey(idAndType(GLOBAL_FIND_REPLACE));
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		TabWindow *currentTab = GetTab();
		if (currentTab->windowType != WINDOW_FIND){ findReplaceTabs->SetTab(0); }
		else{ Hide(); }
	}, Search);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		TabWindow *currentTab = GetTab();
		if (currentTab->windowType != WINDOW_REPLACE){ findReplaceTabs->SetTab(1); }
		else{ Hide(); }
	}, GLOBAL_FIND_REPLACE);
	Show();
}

FindReplaceDialog::~FindReplaceDialog()
{
	delete FR;
}

void FindReplaceDialog::ShowDialog(int whichWindow)
{
	TabWindow *currentTab = GetTab();
	if (currentTab->windowType == whichWindow)
		Show(false);

	findReplaceTabs->SetTab(whichWindow);
	if (!IsShown())
		Show();
}

void FindReplaceDialog::OnActivate(wxActivateEvent& event)
{
	if (!event.GetActive() || FR->blockTextChange){
		if (event.GetActive()){ FR->blockTextChange = false; } return;
	}
	TabWindow *currentTab = GetTab();
	long from, to, fromO, toO;
	EditBox *edit = Kai->GetTab()->Edit;
	edit->TextEdit->GetSelection(&from, &to);
	edit->TextEditOrig->GetSelection(&fromO, &toO);
	KaiChoice * findOrReplace = (currentTab->FindText->GetValue().Len() > 0 && 
		currentTab->windowType != WINDOW_FIND && !FR->findTextReset) ? currentTab->ReplaceText : currentTab->FindText;
	if (from < to){
		if (from == FR->findstart && to == FR->findend)
			return;
		wxString selected = edit->TextEdit->GetValue().SubString(from, to - 1);
		if (selected.Lower() != findOrReplace->GetValue().Lower()){ findOrReplace->SetValue(selected); }
		//when someone changed selection, then restore textposition to 0 maybe restore lineposition too? It's different seeking
		FR->textPosition = FR->linePosition = 0;
	}
	else if (fromO < toO){
		if (fromO == FR->findstart && toO == FR->findend)
			return;
		wxString selected = edit->TextEditOrig->GetValue().SubString(fromO, toO - 1);
		if (selected.Lower() != findOrReplace->GetValue().Lower()){ findOrReplace->SetValue(selected); }
		FR->textPosition = FR->linePosition = 0;
	}
	findOrReplace->SetFocus();
	FR->findTextReset = false;
}

void FindReplaceDialog::OnEnterConfirm(wxCommandEvent& event)
{
	TabWindow *currentTab = GetTab();
	if (currentTab->windowType){
		if (currentTab->windowType == WINDOW_FIND_IN_SUBS && currentTab->ReplaceText->HasFocus() ||
			currentTab->FindInSubsPattern->HasFocus() || currentTab->FindInSubsPath->HasFocus())
			FR->ReplaceInSubs(currentTab);
		if (currentTab->ReplaceText->HasFocus())
			FR->Replace(currentTab);

		return;
	}
	if (currentTab->windowType == WINDOW_FIND_IN_SUBS)
		FR->FindAllInCurrentSubs(currentTab);
	else{
		FR->Find(currentTab);
		FR->fnext = false;
	}
}

TabWindow * FindReplaceDialog::GetTab()
{
	return (TabWindow *)findReplaceTabs->GetTab();
}
