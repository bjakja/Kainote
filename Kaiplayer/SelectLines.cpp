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

SelectLines::SelectLines(kainoteFrame* kfparent)
	: KaiDialog((wxWindow*)kfparent,-1,_("Zaznacz"))
{
	Kai = kfparent;
	wxArrayString selsRecent;
	Options.GetTable(SelectionsRecent,selsRecent,"\f");
	if(selsRecent.size()>20){selsRecent.RemoveAt(19,selsRecent.size()-20);}

	DialogSizer *slsizer= new DialogSizer(wxVERTICAL);
	wxBoxSizer *slrbsizer= new wxBoxSizer(wxHORIZONTAL);
	Contains = new KaiRadioButton(this, -1, _("Zawiera"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	NotContains = new KaiRadioButton(this, -1, _("Nie zawiera"));
	Contains->SetValue(true);
	slrbsizer->Add(Contains,1,wxALL|wxEXPAND,3);
	slrbsizer->Add(NotContains,1,wxALL|wxEXPAND,3);

	KaiStaticBoxSizer* slsbsizer=new KaiStaticBoxSizer(wxVERTICAL,this,_("Znajdź"));
	wxBoxSizer *sltpsizer= new wxBoxSizer(wxHORIZONTAL);
	FindText = new KaiChoice(this, -1, "", wxDefaultPosition, wxSize(-1,24),selsRecent);
	FindText->SetToolTip(_("Szukany tekst:"));
	FindText->SetMaxLength(MAXINT);
	ChooseStyles = new MappedButton(this, ID_CHOOSE_STYLES, "+", -1, wxDefaultPosition, wxSize(24,24));
	sltpsizer->Add(FindText,1,wxALL|wxEXPAND,3);
	sltpsizer->Add(ChooseStyles,0,wxALL,3);

	MatchCase = new KaiCheckBox(this, -1, _("Uwzględniaj wielkość liter"));
	RegEx = new KaiCheckBox(this, -1, _("Wyrażenia regularne"));

	slsbsizer->Add(slrbsizer,0,wxEXPAND,0);
	slsbsizer->Add(sltpsizer,0,wxEXPAND,0);
	slsbsizer->Add(MatchCase,0,wxALL,3);
	slsbsizer->Add(RegEx,0,wxALL,3);

	KaiStaticBoxSizer* slsbsizer1=new KaiStaticBoxSizer(wxVERTICAL,this,_("W polu"));
	wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL);
	wxBoxSizer *sizer1 = new wxBoxSizer( wxHORIZONTAL);
	CollumnText = new KaiRadioButton(this, -1, _("Tekst"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	CollumnStyle = new KaiRadioButton(this, -1, _("Styl"));
	CollumnActor = new KaiRadioButton(this, -1, _("Aktor"));
	CollumnEffect = new KaiRadioButton(this, -1, _("Efekt"));
	CollumnStartTime = new KaiRadioButton(this, -1, _("Czas Początkowy"));
	CollumnEndTime = new KaiRadioButton(this, -1, _("Czas Końcowy"));
		
	sizer->Add(CollumnText,1,wxALL,3);
	sizer->Add(CollumnStyle,1,wxALL,3);
	sizer->Add(CollumnActor,1,wxALL,3);
	sizer->Add(CollumnEffect,1,wxALL,3);
	sizer1->Add(CollumnStartTime,1,wxALL|wxEXPAND,3);
	sizer1->Add(CollumnEndTime,1,wxALL|wxEXPAND,3);
	slsbsizer1->Add(sizer,1,wxEXPAND);
	slsbsizer1->Add(sizer1,1,wxEXPAND);
	KaiStaticBoxSizer* slsbsizer2=new KaiStaticBoxSizer(wxHORIZONTAL,this,_("Dialogi / komentarze"));

	Dialogues = new KaiCheckBox(this, -1, _("Dialogi"));
	Comments = new KaiCheckBox(this, -1, _("Komentarze"));
	Dialogues->SetValue(true);
	slsbsizer2->Add(Dialogues,0,wxALL,3);
	slsbsizer2->Add(Comments,0,wxALL,3);

	wxArrayString sels;
	sels.Add(_("Zaznacz"));
	sels.Add(_("Dodaj do zaznaczenia"));
	sels.Add(_("Odznacz"));

	Selections = new KaiRadioBox(this,-1,_("Zaznaczenie"),wxDefaultPosition,wxDefaultSize,sels,2);

	wxArrayString action;
	action.Add(_("Nie rób nic"));
	action.Add(_("Kopiuj"));
	action.Add(_("Wytnij"));
	action.Add(_("Przenieś na początek"));
	action.Add(_("Przenieś na koniec"));
	action.Add(_("Ustaw jako komentarz"));
	action.Add(_("Usuń"));

	Actions = new KaiRadioBox(this,-1,_("Akcja"),wxDefaultPosition,wxDefaultSize,action,2);

	wxBoxSizer *slbtsizer=new wxBoxSizer(wxHORIZONTAL);
	Select = new MappedButton(this, ID_SELECTIONS, _("Zaznacz"));
	Close = new MappedButton(this, wxID_CANCEL, _("Zamknij"));
	slbtsizer->Add(Select,1,wxALL,5);
	slbtsizer->Add(Close,1,wxALL,5);


	slsizer->Add(slsbsizer,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	slsizer->Add(slsbsizer1,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	slsizer->Add(slsbsizer2,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	slsizer->Add(Selections,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	slsizer->Add(Actions,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	slsizer->Add(slbtsizer,0,wxALL|wxALIGN_CENTER,0);

	SetSizerAndFit(slsizer);


	Connect(ID_SELECTIONS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectLines::OnSelect);
	Connect(ID_CHOOSE_STYLES,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectLines::OnChooseStyles);

	//SetEscapeId(ID_CLOSE_SELECTIONS);
	SetEnterId(ID_SELECTIONS);
	CenterOnParent();
}

void SelectLines::OnSelect(wxCommandEvent & evt)
{
	long wrep=TXT;
	if(CollumnStyle->GetValue()){wrep=STYLE;}
	else if(CollumnActor->GetValue()){wrep=ACTOR;}
	else if(CollumnEffect->GetValue()){wrep=EFFECT;}
	else if(CollumnStartTime->GetValue()){wrep=START;}
	else if(CollumnEndTime->GetValue()){wrep=END;}

	wxString find=FindText->GetValue();

	int allreps=0;
	wxString txt,whatcopy;
	std::vector<Dialogue *> mdial;

	bool matchcase=MatchCase->GetValue();
	bool regex=RegEx->GetValue();
	bool contain=Contains->GetValue();
	bool notcont=NotContains->GetValue();
	bool diall=Dialogues->GetValue();
	bool commm=Comments->GetValue();
	int sopt = Selections->GetSelection();
	int act = Actions->GetSelection();

	if(!matchcase){find.MakeLower();}
	TabPanel *tab=Kai->GetTab();
	tab->Grid->SaveSelections(sopt == 0);
	File *Subs = tab->Grid->file->GetSubs();
	bool skipFiltered = !tab->Grid->ignoreFiltered;

	for (int i = 0; i < Subs->dials.size(); i++)
	{
		Dialogue *Dial = Subs->dials[i];
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue){ continue; }

		if(wrep==STYLE){
			txt=Dial->Style;}
		else if(wrep==TXT){
			txt=(tab->Grid->hasTLMode && Dial->TextTl!="")? Dial->TextTl : Dial->Text;}
		else if(wrep==ACTOR){
			txt=Dial->Actor;}
		else if(wrep==EFFECT){
			txt=Dial->Effect;}
		else if(wrep==START){
			txt=Dial->Start.raw();}
		else if(wrep==END){
			txt=Dial->End.raw();}

		bool isfound=false;


		if (txt!="" && find!=""){
			if(regex){
				int rxflags=wxRE_ADVANCED;
				if(!matchcase){rxflags |= wxRE_ICASE;}
				wxRegEx rgx (find,rxflags);
				if (rgx.IsValid()) {
					if (rgx.Matches(txt)) {
						isfound=true;}
				}
			}
			else{
				if(!matchcase){txt.MakeLower();} 
				if(txt.Find(find)!=-1){isfound=true;}

			}
		}else if(find=="" && txt==""){isfound=true;}

		if(((isfound&&contain)||(!isfound&&!contain))
			&&((diall && !Dial->IsComment) || (commm && Dial->IsComment))){
				bool select=(sopt==2)?false:true;
				if(select){
					tab->Grid->file->InsertSelectionKey(i);
					allreps++;
				}
				else{
					if (tab->Grid->file->IsSelectedByKey(i)){
						tab->Grid->file->EraseSelectionKey(i);
						allreps++;
					}
				}
		}

		if (tab->Grid->file->IsSelectedByKey(i) && act != 0){
			if(act<3){Dial->GetRaw(&whatcopy, tab->Grid->hasTLMode && Dial->TextTl!="");}
			else if(act<5){Dial->State= 1 + (Dial->State & 4); mdial.push_back(Dial);}
			else if(act<6){
				Dialogue *dialc = tab->Grid->file->CopyDialogueByKey(i); 
				dialc->State=1 + (dialc->State & 4);
				dialc->IsComment=true;
			}
		}	

	}

	//a teraz nasze kochane akcje
	//kopiowanie
	if(act==1||act==2){
		if (wxTheClipboard->Open()){
			wxTheClipboard->SetData( new wxTextDataObject(whatcopy) );
			wxTheClipboard->Close();
		}
	}//przenoszenie na początek / koniec
	if(act==2||act==6||act==3||act==4){
		tab->Grid->DeleteRows();
		if(act==3||act==4)
		{
			tab->Grid->InsertRows((act==3)? 0 : -1, mdial, false, true);
			mdial.clear();
		}
	}
	int fsel=tab->Grid->FirstSelection();
	int wset=(fsel<0)? tab->Edit->ebrow : fsel;
	tab->Edit->SetLine(wset);
	tab->Grid->SetModified(SELECT_LINES, false);
	tab->Grid->RefreshColumns();
	wxString messagetxt= (sopt==0)? wxString::Format(_("Zaznaczono %i linijek."), allreps) :
		(sopt==1)? wxString::Format(_("Dodano do zaznaczenia %i linijek."), allreps) : 
		wxString::Format(_("Odznaczono %i linijek."), allreps);
	KaiMessageDialog dlg(this, messagetxt, _("Zaznacz"), wxYES_NO);
	dlg.SetYesLabel("Zamknij");
	dlg.SetNoLabel("Ok");
	int result = dlg.ShowModal();
	if(result == wxYES){
		Hide();
	}
	AddRecent();
}

void SelectLines::AddRecent(){
	wxString text=FindText->GetValue();

	wxArrayString selsRecent;
	Options.GetTable(SelectionsRecent,selsRecent,"\f");
	if(selsRecent.size()>20){selsRecent.RemoveAt(20,selsRecent.size()-20);}
	for(size_t i=0;i<selsRecent.GetCount();i++)
	{
		if(selsRecent[i]==text){
			selsRecent.RemoveAt(i);
			FindText->Delete(i);
		}
	}
	selsRecent.Insert(text,0);
	FindText->Insert(text,0);

	FindText->SetSelection(0);
	Options.SetTable(SelectionsRecent,selsRecent,"\f");
}

void SelectLines::OnChooseStyles(wxCommandEvent& event)
{
	wxString kkk=GetCheckedElements(Kai);
	kkk.Replace(";","|");
	CollumnStyle->SetValue(true);
	RegEx->SetValue(true);
	FindText->SetValue(kkk);
}