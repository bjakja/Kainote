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

#include "FindReplace.h"
#include "KainoteMain.h"
#include "KaiMessageBox.h"
#include "Stylelistbox.h"
//#include <wx/regex.h>
#include <wx/clipbrd.h>


FindReplace::FindReplace(kainoteFrame* kfparent, bool replace)
	: KaiDialog(kfparent, -1, (replace) ? _("Znajdź i zamień") : _("Znajdź"))
{
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	Kai = kfparent;
	lastActive = reprow = linePosition = 0;
	textPosition = 0;
	findstart = -1;
	findend = -1;
	fnext = blockTextChange = false;
	repl = replace;
	fromstart = true;
	RepText = NULL;
	StartLine = EndLine = NULL;
	tcstyle = NULL;

	Options.GetTable(FindRecent, findRecent, "\f", wxTOKEN_RET_EMPTY_ALL);
	int options = Options.GetInt(FindReplaceOptions);
	if (findRecent.size() > 20){ findRecent.RemoveAt(20, findRecent.size() - 20); }

	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("SEARCH"));
	SetIcon(icn);

	mainfrbsizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer* mainfrbsizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainfrbsizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* mainfrbsizer3 = new wxBoxSizer(wxHORIZONTAL);

	//pionowy sizer kolumna 1
	//KaiStaticBoxSizer* frsbsizer=new KaiStaticBoxSizer(wxVERTICAL,this,_("Znajdź"));
	wxBoxSizer* frsbsizer = new wxBoxSizer(wxHORIZONTAL);
	FindText = new KaiChoice(this, ID_FINDTEXT, "", wxDefaultPosition, wxDefaultSize, findRecent);
	FindText->SetToolTip(_("Szukany tekst:"));
	FindText->SetMaxLength(MAXINT);
	frsbsizer->Add(new KaiStaticText(this, -1, _("Szukany tekst:")), 1, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT, 4, 0);
	frsbsizer->Add(FindText, 4, wxEXPAND, 0);
	mainfrbsizer1->Add(frsbsizer, 0, wxEXPAND | wxALL, 4);

	Options.GetTable(ReplaceRecent, replaceRecent, "\f", wxTOKEN_RET_EMPTY_ALL);
	if (replaceRecent.size() > 20){ replaceRecent.RemoveAt(20, replaceRecent.size() - 20); }
	wxBoxSizer *ReplaceStaticSizer = new wxBoxSizer(wxHORIZONTAL);
	//ReplaceStaticSizer=new KaiStaticBoxSizer(wxVERTICAL,this,_("Zamień"));
	RepText = new KaiChoice(this, ID_REPTEXT, "", wxDefaultPosition, wxDefaultSize, replaceRecent);
	RepText->SetToolTip(_("Zamień na:"));
	RepText->SetMaxLength(MAXINT);
	repDescText = new KaiStaticText(this, -1, _("Zamień na:"));
	ReplaceStaticSizer->Add(repDescText, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT, 4);
	ReplaceStaticSizer->Add(RepText, 4, wxEXPAND, 0);
	mainfrbsizer1->Add(ReplaceStaticSizer, 0, wxEXPAND | wxALL, 4);

	wxBoxSizer* frbsizer1 = new wxBoxSizer(wxVERTICAL);
	MatchCase = new KaiCheckBox(this, -1, _("Uwzględniaj wielkość liter"));
	MatchCase->SetValue(options & CASE_SENSITIVE);
	RegEx = new KaiCheckBox(this, -1, _("Wyrażenia regularne"));
	RegEx->SetValue((options & REG_EX) > 0);
	StartLine = new KaiCheckBox(this, ID_SLINE, _("Początek tekstu"));
	StartLine->SetValue((options & START_OF_TEXT) > 0);
	EndLine = new KaiCheckBox(this, ID_ELINE, _("Koniec tekstu"));
	EndLine->SetValue((options & END_OF_TEXT) > 0);
	frbsizer1->Add(MatchCase, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	frbsizer1->Add(RegEx, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	frbsizer1->Add(StartLine, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);
	frbsizer1->Add(EndLine, 0, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 2);


	KaiStaticBoxSizer* frsbsizer2 = new KaiStaticBoxSizer(wxVERTICAL, this, _("W polu"));
	wxBoxSizer* frbsizer2 = new wxBoxSizer(wxHORIZONTAL);
	CollumnText = new KaiRadioButton(this, -1, _("Tekst"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	if (options & IN_FIELD_TEXT)
		CollumnText->SetValue(true);
	CollumnTextOriginal = new KaiRadioButton(this, -1, _("Tekst oryginału"));
	CollumnTextOriginal->Enable(Kai->GetTab()->Grid->hasTLMode);
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

	//static box sizer dodanie pierwszego i drugiego rzędu
	frsbsizer2->Add(frbsizer2, 1, wxEXPAND | wxLEFT, 2);
	frsbsizer2->Add(frbsizer3, 1, wxEXPAND | wxLEFT, 2);
	//połączenie chceckboxów i radiobutonów z wyborem pola
	mainfrbsizer2->Add(frbsizer1, 1, wxEXPAND, 0);
	mainfrbsizer2->Add(frsbsizer2, 1, wxEXPAND, 0);

	//połączenie wcześniejszego sizera z znajdź i zmień
	//dwie poniższe linijki są na samym początku

	mainfrbsizer1->Add(mainfrbsizer2, 0, wxEXPAND | wxLEFT, 1);

	//pionowy sizer kolumna 2
	wxBoxSizer* frbsizer = new wxBoxSizer(wxVERTICAL);
	Button1 = new MappedButton(this, ID_BFIND, _("Znajdź"), -1, wxDefaultPosition, wxSize(124, -1));
	frbsizer->Add(Button1, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);

	Button2 = new MappedButton(this, ID_BREP, _("Zamień następne"));
	Button3 = new MappedButton(this, ID_BREPALL, _("Zamień wszystko"));

	frbsizer->Add(Button2, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);
	frbsizer->Add(Button3, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);


	Button4 = new MappedButton(this, ID_BCLOSE, _("Zamknij"));
	frbsizer->Add(Button4, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);

	//łączenie całości znajdowania i opcji z przyciskami
	mainfrbsizer3->Add(mainfrbsizer1, 0, wxEXPAND | wxRIGHT, 2);
	mainfrbsizer3->Add(frbsizer, 0, wxEXPAND | wxLEFT, 3);

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

	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [=](wxCommandEvent &evt){Reset(); }, 23156);
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [=](wxCommandEvent &evt){Reset(); }, 23157);
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [=](wxCommandEvent &evt){Reset(); }, 23158);

	wxBoxSizer* frbsizer4 = new wxBoxSizer(wxHORIZONTAL);
	Bplus = new MappedButton(this, ID_BPLUS, "+", -1, wxDefaultPosition, wxSize(22, 22));
	tcstyle = new KaiTextCtrl(this, ID_TCSTYLE, "", wxDefaultPosition, wxSize(-1, 22));
	frbsizer4->Add(Bplus, 0, 0, 0);
	frbsizer4->Add(tcstyle, 0, wxLEFT, 3);

	frsbsizer3->Add(AllLines, 1, wxALL | wxEXPAND, 2);
	frsbsizer3->Add(SelectedLines, 1, wxALL | wxEXPAND, 2);
	frsbsizer3->Add(FromSelection, 1, wxALL | wxEXPAND, 2);
	frsbsizer3->Add(frbsizer4, 0, wxALL, 2);

	mainfrbsizer->Add(mainfrbsizer3, 0, wxEXPAND | wxALL, 5);
	mainfrbsizer->Add(frsbsizer3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	//Ustawienie sizera
	//SetSizerAndFit(mainfrbsizer);

	Connect(ID_SLINE, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FindReplace::OnRecheck);
	Connect(ID_ELINE, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FindReplace::OnRecheck);

	Connect(ID_BFIND, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FindReplace::OnButtonFind);
	//Connect(ID_FINDTEXT,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FindReplace::OnTextUpdate);

	Connect(ID_BREP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FindReplace::OnButtonRep);
	Connect(ID_BREPALL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FindReplace::OnReplaceAll);


	Connect(ID_BPLUS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FindReplace::OnStylesWin);
	Bind(wxEVT_ACTIVATE, &FindReplace::OnSetFocus, this);


	Connect(ID_BCLOSE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FindReplace::OnClose);
	Connect(ID_ENTER_CONFIRM, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FindReplace::OnEnterConfirm);
	SetEscapeId(ID_BCLOSE);
	SetEnterId(ID_ENTER_CONFIRM);
	CenterOnParent();
	ChangeContents(replace);

	wxAcceleratorEntry entries[2];
	entries[0] = Hkeys.GetHKey(idAndType(Search));
	entries[1] = Hkeys.GetHKey(idAndType(FindReplaceDialog));
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if (repl){ ChangeContents(false); }
		else{ Hide(); }
	}, Search);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if (!repl){ ChangeContents(true); }
		else{ Hide(); }
	}, FindReplaceDialog);

}

void FindReplace::SaveOptions()
{
	int options = 0;
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
	if (AllLines->GetValue())
		options |= IN_LINES_ALL;
	if (SelectedLines->GetValue())
		options |= IN_LINES_SELECTED;
	if (FromSelection->GetValue())
		options |= IN_LINES_FROM_SELECTION;

	Options.SetInt(FindReplaceOptions, options);
}

void FindReplace::ChangeContents(bool replace)
{
	repl = replace;
	SetLabel((replace) ? _("Znajdź i zamień") : _("Znajdź"));
	repDescText->Show(replace);
	RepText->Show(replace);
	Button2->Show(replace);
	Button3->Show(replace);
	SetSizerAndFit(mainfrbsizer, false);
}

void FindReplace::OnReplaceAll(wxCommandEvent& event)
{
	TabPanel *tab = Kai->GetTab();
	long wrep = (tab->Grid->hasTLMode && !CollumnTextOriginal->GetValue()) ? TXTTL : TXT;
	if (CollumnStyle->GetValue()){ wrep = STYLE; }
	else if (CollumnActor->GetValue()){ wrep = ACTOR; }
	else if (CollumnEffect->GetValue()){ wrep = EFFECT; }

	bool matchcase = MatchCase->GetValue();
	bool regex = RegEx->GetValue();
	bool startline = StartLine->GetValue();
	bool endline = EndLine->GetValue();

	wxString find = FindText->GetValue(), rep = RepText->GetValue();
	if (startline && regex){
		find = "^" + find;
	}
	if (endline&& regex){
		if (find == ""){
			find = "^(.*)$";
			rep = "\\1" + rep;
		}
		else{
			find << "$";
		}
	}

	int allreps1 = 0;
	int allreps = 0;
	wxString txt;
	wxString styll = tcstyle->GetValue();
	bool notstyles = false;
	if (styll == ""){ notstyles = true; }
	else{ styll = "," + styll + ","; }
	bool onlysel = SelectedLines->GetValue();

	int fsel = tab->Grid->FirstSelection();
	File *Subs = tab->Grid->file->GetSubs();
	bool skipFiltered = !tab->Grid->ignoreFiltered;

	for (int i = (!AllLines->GetValue() && fsel >= 0) ? fsel : 0; i < Subs->dials.size(); i++)
	{
		Dialogue *Dial = Subs->dials[i];
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue){ continue; }

		if ((notstyles || styll.Find("," + Dial->Style + ",") != -1) &&
			!(onlysel && !(tab->Grid->file->IsSelectedByKey(i)))){

			if (wrep == STYLE){
				txt = Dial->Style;
			}
			else if (wrep == TXT){
				txt = Dial->Text;
			}
			else if (wrep == TXTTL){
				txt = Dial->TextTl;
			}
			else if (wrep == ACTOR){
				txt = Dial->Actor;
			}
			else if (wrep == EFFECT){
				txt = Dial->Effect;
			}
			if (!(startline || endline) && (find.empty() || txt.empty()))
			{
				if (txt.empty() && find.empty())
				{
					txt = rep; allreps = 1;
				}
				else{ continue; }
			}
			else if (regex){
				int rxflags = wxRE_ADVANCED;
				if (!matchcase){ rxflags |= wxRE_ICASE; }
				wxRegEx nfind(find, rxflags);
				allreps = nfind.ReplaceAll(&txt, rep);
			}
			else if (startline || endline){

				wxString ltext = (!matchcase) ? txt.Lower() : txt;
				wxString lfind = (!matchcase) ? find.Lower() : find;
				if (startline){
					if (ltext.StartsWith(lfind) || lfind.empty()){
						txt.replace(0, lfind.Len(), rep);
						allreps = 1;
					}
				}
				if (endline){
					if (ltext.EndsWith(lfind) || lfind.empty()){
						int lenn = txt.Len();
						txt.replace(lenn - lfind.Len(), lenn, rep);
						allreps = 1;
					}
				}

			}
			else if (!matchcase){
				wxString lfind = find.Lower();
				wxString ltext = txt;
				ltext.MakeLower();

				bool isfind = true;
				int newpos = 0;
				int flen = lfind.Len();
				allreps = 0;
				while (isfind){
					size_t poss = ltext.find(lfind, newpos);
					if (poss == -1){ isfind = false; break; }
					else{
						if (poss < 0){ poss = 0; }
						if (poss > txt.Len()){ poss = txt.Len(); }
						wxString hhh;
						ltext.Remove(poss, flen);
						ltext.insert(poss, rep);
						txt.Remove(poss, flen);
						txt.insert(poss, rep);
						allreps++;
						newpos = poss + rep.Len();
					}

				}
			}
			else{ allreps = txt.Replace(find, rep); }
			if (allreps > 0){
				Dialogue *Dialc = tab->Grid->file->CopyDialogueByKey(i);
				if (wrep == TXT){ Dialc->Text = txt; }
				else if (wrep == TXTTL){ Dialc->TextTl = txt; }
				else if (wrep == STYLE){ Dialc->Style = txt; }
				else if (wrep == ACTOR){ Dialc->Actor = txt; }
				else if (wrep == EFFECT){ Dialc->Effect = txt; }
				allreps1 += allreps;

			}

		}

	}
	if (allreps1){
		tab->Grid->SpellErrors.clear();
		tab->Grid->SetModified(REPLACE_ALL);
		if (wrep < TXT){
			tab->Grid->RefreshColumns(wrep);
		}
		else{
			tab->Grid->Refresh(false);
		}
	}
	blockTextChange = true;
	KaiMessageBox(wxString::Format(_("Zmieniono %i razy."), allreps1), _("Szukaj Zamień"));
	AddRecent();
	findTextReset = true;
}


void FindReplace::OnButtonFind(wxCommandEvent& event)
{
	Find();
	fnext = false;
	findTextReset = true;
}

void FindReplace::OnButtonRep(wxCommandEvent& event)
{
	TabPanel *tab = Kai->GetTab();
	if (lastActive != tab->Grid->currentLine){ Find(); }
	bool searchInOriginal = CollumnTextOriginal->GetValue();
	long wrep = (tab->Grid->hasTLMode && !searchInOriginal) ? TXTTL : TXT;
	if (CollumnStyle->GetValue()){ wrep = STYLE; }
	else if (CollumnActor->GetValue()){ wrep = ACTOR; }
	else if (CollumnEffect->GetValue()){ wrep = EFFECT; }

	wxString find1 = FindText->GetValue();
	if (find1 != oldfind || findstart == -1 || findend == -1){ fromstart = true; fnext = false; oldfind = find1; Find(); }
	if (findstart == -1 || findend == -1){ return; }
	fnext = true;
	wxString rep = RepText->GetValue();
	SubsGrid *grid = tab->Grid;

	Dialogue *Dialc = grid->CopyDialogueByKey(reprow);
	bool hasRegEx = RegEx->GetValue();

	if (wrep == STYLE){
		wxString oldstyle = Dialc->Style;
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldstyle.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldstyle.replace(findstart, findend - findstart, (reps)? place : rep);
		}
		else{
			oldstyle.replace(findstart, findend - findstart, rep);
		}
		tab->Edit->StyleChoice->SetSelection(tab->Edit->StyleChoice->FindString(oldstyle));
		Dialc->Style = oldstyle;
	}
	else if (wrep == TXT || wrep == TXTTL){
		MTextEditor *tmp = (searchInOriginal) ? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
		wxString oldtext = tmp->GetValue();
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldtext.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldtext.replace(findstart, findend - findstart, (reps) ? place : rep);
		}
		else{
			oldtext.replace(findstart, findend - findstart, rep);
		}
		tmp->SetTextS(oldtext);
		Dialc->Text = oldtext;
	}
	else if (wrep == ACTOR){
		wxString oldtext = tab->Edit->ActorEdit->choiceText->GetValue();
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldtext.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldtext.replace(findstart, findend - findstart, (reps) ? place : rep);
		}
		else{
			oldtext.replace(findstart, findend - findstart, rep);
		}
		tab->Edit->ActorEdit->choiceText->SetValue(oldtext);
		Dialc->Actor = oldtext;
	}
	else if (wrep == EFFECT){
		wxString oldtext = tab->Edit->EffectEdit->choiceText->GetValue();
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldtext.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldtext.replace(findstart, findend - findstart, (reps) ? place : rep);
		}
		else{
			oldtext.replace(findstart, findend - findstart, rep);
		}
		tab->Edit->EffectEdit->choiceText->SetValue(oldtext);
		Dialc->Effect = oldtext;
	}

	grid->SetModified(REPLACE_SINGLE);
	grid->Refresh(false);
	textPosition = findstart + rep.Len();
	Find();
}

void FindReplace::Find()
{
	TabPanel *tab = Kai->GetTab();
	bool searchInOriginal = CollumnTextOriginal->GetValue();
	long wrep = (tab->Grid->hasTLMode && !searchInOriginal) ? TXTTL : TXT;
	if (CollumnStyle->GetValue()){ wrep = STYLE; }
	else if (CollumnActor->GetValue()){ wrep = ACTOR; }
	else if (CollumnEffect->GetValue()){ wrep = EFFECT; }

	bool matchcase = MatchCase->GetValue();
	bool regex = RegEx->GetValue();
	bool startline = StartLine->GetValue();
	bool endline = EndLine->GetValue();

	wxString find1 = FindText->GetValue();
	if (find1 != oldfind){ fromstart = true; fnext = false; oldfind = find1; }
	if (!fromstart && lastActive != tab->Grid->currentLine){ lastActive = tab->Grid->currentLine; }

	if (startline && regex){
		find1 = "^" + find1;
	}
	if (endline && regex){
		find1 << "$";
	}

	//Kai->Freeze();

	wxString txt;
	int foundPosition = -1;
	size_t mlen = 0;
	bool foundsome = false;
	if (fromstart){
		int fsel = tab->Grid->FirstSelection();
		linePosition = (!AllLines->GetValue() && fsel >= 0) ? fsel : 0;
		textPosition = 0;
	}
	wxString stylesList = tcstyle->GetValue();
	bool styles = false;
	if (stylesList != ""){
		styles = true;
		stylesList = "," + stylesList + ",";
	}
	bool onlysel = SelectedLines->GetValue();
	File *Subs = tab->Grid->file->GetSubs();

	while (linePosition < Subs->dials.size())
	{
		Dialogue *Dial = Subs->dials[linePosition];
		if (!Dial->isVisible){ linePosition++; textPosition = 0; continue; }

		if ((!styles && !onlysel) ||
			(styles && stylesList.Find("," + Dial->Style + ",") != -1) ||
			(onlysel && tab->Grid->file->IsSelectedByKey(linePosition))){
			if (wrep == STYLE){
				txt = Dial->Style;
			}
			else if (wrep == TXT || wrep == TXTTL){
				txt = (wrep == TXTTL) ? Dial->TextTl : Dial->Text;
			}
			else if (wrep == ACTOR){
				txt = Dial->Actor;
			}
			else if (wrep == EFFECT){
				txt = Dial->Effect;
			}

			//no to szukamy
			if (!(startline || endline) && (find1.empty() || txt.empty()))
			{
				if (txt.empty() && find1.empty()){
					foundPosition = 0; mlen = 0;
				}
				else{ textPosition = 0; linePosition++; continue; }

			}
			else if (regex){
				int rxflags = wxRE_ADVANCED;
				if (!matchcase){ rxflags |= wxRE_ICASE; }
				rgx.Compile(find1, rxflags);
				if (rgx.IsValid()) {
					wxString cuttext = txt.Mid(textPosition);
					if (rgx.Matches(cuttext)) {
						/*wxString reslt = rgx.GetMatch(cuttext, 0);
						if (reslt == ""){ mwhere = -1; mlen = 0; }
						else{
							mwhere = cuttext.Find(reslt) + postxt;
							mlen = reslt.Len();
						}*/
						size_t regexStart = 0;
						rgx.GetMatch(&regexStart, &mlen, 0);
						foundPosition = regexStart + textPosition;
					}
					else{ textPosition = 0; linePosition++; continue; }

				}

			}
			else{
				wxString ltext = (!matchcase) ? txt.Lower() : txt;
				wxString lfind = (!matchcase) ? find1.Lower() : find1;
				if (startline){
					if (ltext.StartsWith(lfind) || lfind.empty()){
						foundPosition = 0;
						textPosition = 0;
					}
					else{
						foundPosition = -1;
					}
				}
				if (endline){
					if (ltext.EndsWith(lfind) || lfind.empty()){
						foundPosition = txt.Len() - lfind.Len();
						textPosition = 0;
					}
					else{
						foundPosition = -1;
					}
				}
				else{
					foundPosition = ltext.find(lfind, textPosition);
				}
				mlen = lfind.Len();
			}

			if (foundPosition != -1){
				textPosition = foundPosition + mlen;
				findstart = foundPosition;
				findend = textPosition;
				lastActive = reprow = linePosition;
				int posrowId = tab->Grid->file->GetElementByKey(linePosition);
				if (!onlysel){ tab->Grid->SelectRow(posrowId, false, true); }
				tab->Edit->SetLine(posrowId);
				tab->Grid->ScrollTo(posrowId, true);
				if (onlysel){ tab->Grid->Refresh(false); }
				if (wrep == STYLE){
					//pan->Edit->StyleChoice->SetFocus();
				}
				else if (wrep == TXT || wrep == TXTTL){
					MTextEditor *tmp = (searchInOriginal) ? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
					//tmp->SetFocus();
					tmp->SetSelection(foundPosition, findend);
				}
				if (wrep == ACTOR){
					//pan->Edit->ActorEdit->SetFocus();
					tab->Edit->ActorEdit->choiceText->SetSelection(foundPosition, findend);
				}
				if (wrep == EFFECT){
					//pan->Edit->EffectEdit->SetFocus();
					tab->Edit->EffectEdit->choiceText->SetSelection(foundPosition, findend);
				}

				foundsome = true;
				if ((size_t)textPosition >= txt.Len() || startline){
					linePosition++; textPosition = 0;
				}
				break;
			}
			else{
				textPosition = 0;
				linePosition++;
			}

		}
		else{ textPosition = 0; linePosition++; }
		if (!foundsome && linePosition > Subs->dials.size() - 1){
			blockTextChange = true;
			if (wasResetToStart){
				wasResetToStart = false;
				break;
			}
			else if (KaiMessageBox(_("Wyszukiwanie zakończone, rozpocząć od początku?"), _("Potwierdzenie"),
			wxICON_QUESTION | wxYES_NO, this) == wxYES){
				linePosition = 0;
				wasResetToStart = true;
			}
			else{ 
				linePosition = 0; 
				foundsome = true; 
				break;
			}
		}
	}
	if (!foundsome){
		blockTextChange = true;
		KaiMessageBox(_("Nie znaleziono podanej frazy \"") + FindText->GetValue() + "\".", _("Potwierdzenie"));
		linePosition = 0;
		fromstart = true;
	}
	if (fromstart){ AddRecent(); fromstart = false; }
	//Kai->Thaw();
}


void FindReplace::OnClose(wxCommandEvent& event)
{
	Hide();
}

void FindReplace::ReloadStyle()
{
	if (tcstyle){ tcstyle->SetValue(""); }
}

void FindReplace::OnStylesWin(wxCommandEvent& event)
{
	tcstyle->SetValue(GetCheckedElements(Kai));
}

void FindReplace::AddRecent(){
	wxString text = FindText->GetValue();

	for (size_t i = 0; i < findRecent.GetCount(); i++)
	{
		if (findRecent[i] == text){
			findRecent.RemoveAt(i);
			FindText->Delete(i);
		}
	}

	size_t findSize = findRecent.size();

	findRecent.Insert(text, 0);
	FindText->Insert(text, 0);
	FindText->SetSelection(0);

	if (findSize > 20){
		FindText->Delete(20, findSize - 20);
		findRecent.RemoveAt(20, findSize - 20);
	}

	Options.SetTable(FindRecent, findRecent, "\f");
	if (repl){
		wxString text = RepText->GetValue();

		for (size_t i = 0; i < replaceRecent.GetCount(); i++)
		{
			if (replaceRecent[i] == text){
				replaceRecent.RemoveAt(i);
				RepText->Delete(i);
			}
		}

		size_t replaceSize = replaceRecent.size();
		
		replaceRecent.Insert(text, 0);
		RepText->Insert(text, 0);
		RepText->SetSelection(0);

		if (replaceSize > 20){
			RepText->Delete(20, replaceSize - 20);
			replaceRecent.RemoveAt(20, replaceSize - 20);
		}

		Options.SetTable(ReplaceRecent, replaceRecent, "\f");
	}

}

void FindReplace::OnRecheck(wxCommandEvent& event)
{
	int id = event.GetId();
	if (id == ID_SLINE && EndLine->GetValue()){
		EndLine->SetValue(false);
	}
	else if (id == ID_ELINE && StartLine->GetValue()){
		StartLine->SetValue(false);
	}

}

void FindReplace::Reset()
{
	fromstart = true;
	fnext = false;
	if (CollumnTextOriginal->GetValue()){ CollumnText->SetValue(true); }
	CollumnTextOriginal->Enable(Kai->GetTab()->Grid->hasTLMode);
}

void FindReplace::OnSetFocus(wxActivateEvent& event){
	if (!event.GetActive() || blockTextChange){ 
		if (event.GetActive()){ blockTextChange = false; } return; 
	}
	long from, to, fromO, toO;
	EditBox *edit = Kai->GetTab()->Edit;
	edit->TextEdit->GetSelection(&from, &to);
	edit->TextEditOrig->GetSelection(&fromO, &toO);
	KaiChoice * findOrReplace = (FindText->GetValue().Len() > 0 && repl && !findTextReset) ? RepText : FindText;
	if (from < to){
		if (from == findstart && to == findend)
			return;
		wxString selected = edit->TextEdit->GetValue().SubString(from, to - 1);
		if (selected.Lower() != findOrReplace->GetValue().Lower()){ findOrReplace->SetValue(selected); }
		//when someone changed selection, then restore textposition to 0 maybe restore lineposition too? It's different seeking
		textPosition = linePosition = 0;
	}
	else if (fromO < toO){
		if (fromO == findstart && toO == findend)
			return;
		wxString selected = edit->TextEditOrig->GetValue().SubString(fromO, toO - 1);
		if (selected.Lower() != findOrReplace->GetValue().Lower()){ findOrReplace->SetValue(selected); }
		textPosition = linePosition = 0;
	}
	findOrReplace->SetFocus();
	findTextReset = false;
}

void FindReplace::OnEnterConfirm(wxCommandEvent& event)
{
	if (RepText && RepText->HasFocus()){
		OnButtonRep(event);
	}
	else{
		Find();
		fnext = false;
	}
}