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
#include <wx/regex.h>
#include <wx/clipbrd.h>


FindReplace::FindReplace(kainoteFrame* kfparent, bool replace)
	: KaiDialog(kfparent, -1,(replace)? _("Znajdź i zamień") : _("Znajdź"))
{
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	Kai=kfparent;
	lastActive=reprow=posrow=0;
	postxt=0;
	findstart=-1;
	findend=-1;
	fnext=blockTextChange=false;
	repl=replace;
	fromstart=true;
	RepText=NULL;
	StartLine=EndLine=NULL;
	tcstyle=NULL;
	wxArrayString wfind;
	Options.GetTable(FindRecent,wfind,"\f");
	if(wfind.size()>20){wfind.RemoveAt(19,wfind.size()-20);}

	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("SEARCH"));
	SetIcon(icn);

	mainfrbsizer=new DialogSizer(wxVERTICAL);
	wxBoxSizer* mainfrbsizer1=new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainfrbsizer2=new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* mainfrbsizer3=new wxBoxSizer(wxHORIZONTAL);

	//pionowy sizer kolumna 1
	//KaiStaticBoxSizer* frsbsizer=new KaiStaticBoxSizer(wxVERTICAL,this,_("Znajdź"));
	wxBoxSizer* frsbsizer=new wxBoxSizer(wxHORIZONTAL);
	FindText = new KaiChoice(this, ID_FINDTEXT, "", wxDefaultPosition, wxDefaultSize,wfind);
	FindText->SetToolTip(_("Szukany tekst:"));
	FindText->SetMaxLength(MAXINT);
	frsbsizer->Add(new KaiStaticText(this,-1,_("Szukany tekst:")),1,wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxRIGHT,4,0);
	frsbsizer->Add(FindText,4,wxEXPAND,0);
	mainfrbsizer1->Add(frsbsizer,0,wxEXPAND|wxALL,3);

	wxArrayString wrepl;
	Options.GetTable(ReplaceRecent,wrepl,"\f");
	if(wrepl.size()>20){wrepl.RemoveAt(19,wrepl.size()-20);}
	wxBoxSizer *ReplaceStaticSizer = new wxBoxSizer(wxHORIZONTAL);
	//ReplaceStaticSizer=new KaiStaticBoxSizer(wxVERTICAL,this,_("Zamień"));
	RepText = new KaiChoice(this, ID_REPTEXT, "", wxDefaultPosition, wxDefaultSize,wrepl);
	RepText->SetToolTip(_("Zamień na:"));
	RepText->SetMaxLength(MAXINT);
	repDescText = new KaiStaticText(this,-1,_("Zamień na:"));
	ReplaceStaticSizer->Add(repDescText,1,wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxRIGHT,4);
	ReplaceStaticSizer->Add(RepText,4,wxEXPAND,0);
	mainfrbsizer1->Add(ReplaceStaticSizer,0,wxEXPAND|wxALL,3);

	wxBoxSizer* frbsizer1=new wxBoxSizer(wxVERTICAL);
	MatchCase = new KaiCheckBox(this, -1, _("Uwzględniaj wielkość liter"));
	MatchCase->SetValue(false);
	RegEx = new KaiCheckBox(this, -1, _("Wyrażenia regularne"));
	RegEx->SetValue(false);
	StartLine = new KaiCheckBox(this, ID_SLINE, _("Początek tekstu"));
	StartLine->SetValue(false);
	EndLine = new KaiCheckBox(this, ID_ELINE, _("Koniec tekstu"));
	EndLine->SetValue(false);
	frbsizer1->Add(MatchCase,0,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	frbsizer1->Add(RegEx,0,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	frbsizer1->Add(StartLine,0,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	frbsizer1->Add(EndLine,0,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);


	KaiStaticBoxSizer* frsbsizer2=new KaiStaticBoxSizer(wxVERTICAL,this,_("W polu"));
	wxBoxSizer* frbsizer2=new wxBoxSizer(wxHORIZONTAL);
	CollumnText = new KaiRadioButton(this, -1, _("Tekst"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	CollumnTextOriginal = new KaiRadioButton(this, -1, _("Tekst oryginału"));
	CollumnTextOriginal->Enable(Kai->GetTab()->Grid1->transl);

	frbsizer2->Add(CollumnText,1,wxALL,1);
	frbsizer2->Add(CollumnTextOriginal,2,wxALL,1);

	wxBoxSizer* frbsizer3=new wxBoxSizer(wxHORIZONTAL);
	CollumnStyle = new KaiRadioButton(this, -1, _("Styl"));
	CollumnActor = new KaiRadioButton(this, -1, _("Aktor"));
	CollumnEffect = new KaiRadioButton(this, -1, _("Efekt"));


	frbsizer3->Add(CollumnStyle,1,wxEXPAND|wxLEFT,1);
	frbsizer3->Add(CollumnActor,1,wxEXPAND|wxLEFT,1);
	frbsizer3->Add(CollumnEffect,1,wxEXPAND|wxLEFT,1);

	//static box sizer dodanie pierwszego i drugiego rzędu
	frsbsizer2->Add(frbsizer2,1,wxEXPAND|wxLEFT,2);
	frsbsizer2->Add(frbsizer3,1,wxEXPAND|wxLEFT,2);
	//połączenie chceckboxów i radiobutonów z wyborem pola
	mainfrbsizer2->Add(frbsizer1,1,wxEXPAND,0);
	mainfrbsizer2->Add(frsbsizer2,1,wxEXPAND,0);

	//połączenie wcześniejszego sizera z znajdź i zmień
	//dwie poniższe linijki są na samym początku

	mainfrbsizer1->Add(mainfrbsizer2,0,wxEXPAND|wxLEFT|wxRIGHT,3);

	//pionowy sizer kolumna 2
	wxBoxSizer* frbsizer=new wxBoxSizer(wxVERTICAL);
	Button1 = new MappedButton(this, ID_BFIND, _("Znajdź"), -1, wxDefaultPosition, wxSize(124,-1));
	frbsizer->Add(Button1,1,wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT,4);

	Button2 = new MappedButton(this, ID_BREP, _("Zamień następne"));
	Button3 = new MappedButton(this, ID_BREPALL, _("Zamień wszystko"));

	frbsizer->Add(Button2,1,wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT,4);
	frbsizer->Add(Button3,1,wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT,4);


	Button4 = new MappedButton(this, ID_BCLOSE, _("Zamknij"));
	frbsizer->Add(Button4,1,wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT,4);

	//łączenie całości znajdowania i opcji z przyciskami
	mainfrbsizer3->Add(mainfrbsizer1,0,wxEXPAND|wxRIGHT,3);
	mainfrbsizer3->Add(frbsizer,0,wxEXPAND|wxLEFT,3);

	//poziomy sizer spód
	KaiStaticBoxSizer* frsbsizer3=new KaiStaticBoxSizer(wxHORIZONTAL,this,_("Linijki"));

	AllLines = new KaiRadioButton(this, 23156, _("Wszystkie linijki"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	SelectedLines = new KaiRadioButton(this, 23157, _("Zaznaczone linijki"));
	FromSelection = new KaiRadioButton(this, 23158, _("Od zaznaczonej"));
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [=](wxCommandEvent &evt){Reset();}, 23156);
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [=](wxCommandEvent &evt){Reset();}, 23157);
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [=](wxCommandEvent &evt){Reset();}, 23158);

	wxBoxSizer* frbsizer4=new wxBoxSizer(wxHORIZONTAL);
	Bplus = new MappedButton(this, ID_BPLUS, "+", -1, wxDefaultPosition, wxSize(22,22));
	tcstyle = new KaiTextCtrl(this, ID_TCSTYLE,"", wxDefaultPosition, wxSize(-1,22));
	frbsizer4->Add(Bplus,0,0,0);
	frbsizer4->Add(tcstyle,0,wxLEFT,3);

	frsbsizer3->Add(AllLines,1,wxALL|wxEXPAND,2);
	frsbsizer3->Add(SelectedLines,1,wxALL|wxEXPAND,2);
	frsbsizer3->Add(FromSelection,1,wxALL|wxEXPAND,2);
	frsbsizer3->Add(frbsizer4,0,wxALL,2);

	mainfrbsizer->Add(mainfrbsizer3,0,wxEXPAND|wxALL,5);
	mainfrbsizer->Add(frsbsizer3,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,8);

	//Ustawienie sizera
	//SetSizerAndFit(mainfrbsizer);

	Connect(ID_SLINE,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FindReplace::OnRecheck);
	Connect(ID_ELINE,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FindReplace::OnRecheck);

	Connect(ID_BFIND,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FindReplace::OnButtonFind);
	//Connect(ID_FINDTEXT,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FindReplace::OnTextUpdate);

	Connect(ID_BREP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FindReplace::OnButtonRep);
	Connect(ID_BREPALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FindReplace::OnReplaceAll);


	Connect(ID_BPLUS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FindReplace::OnStylesWin);
	Bind(wxEVT_ACTIVATE,&FindReplace::OnSetFocus, this);


	Connect(ID_BCLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FindReplace::OnClose);
	Connect(ID_ENTER_CONFIRM,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FindReplace::OnEnterConfirm);
	SetEscapeId(ID_BCLOSE);
	SetEnterId(ID_ENTER_CONFIRM);
	CenterOnParent();
	ChangeContents(replace);

	wxAcceleratorEntry entries[2];
	entries[0]=Hkeys.GetHKey(idAndType(Search));
	entries[1]=Hkeys.GetHKey(idAndType(FindReplaceDialog));
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if(repl){ChangeContents(false);}
		else{Hide();}
	}, Search);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if(!repl){ChangeContents(true);}
		else{Hide();}
	}, FindReplaceDialog);

}

FindReplace::~FindReplace()
{
}

void FindReplace::ChangeContents(bool replace)
{
	repl = replace;
	SetLabel((replace)? _("Znajdź i zamień") : _("Znajdź"));
	repDescText->Show(replace);
	RepText->Show(replace);
	Button2->Show(replace);
	Button3->Show(replace);
	SetSizerAndFit(mainfrbsizer,false);
}

void FindReplace::OnReplaceAll(wxCommandEvent& event)
{
	TabPanel *pan=Kai->GetTab();
	long wrep= (pan->Grid1->transl && !CollumnTextOriginal->GetValue())? TXTTL : TXT;
	if(CollumnStyle->GetValue()){wrep=STYLE;}
	else if(CollumnActor->GetValue()){wrep=ACTOR;}
	else if(CollumnEffect->GetValue()){wrep=EFFECT;}

	bool matchcase=MatchCase->GetValue();
	bool regex=RegEx->GetValue();
	bool startline=StartLine->GetValue();
	bool endline=EndLine->GetValue();

	wxString find=FindText->GetValue(), rep=RepText->GetValue();
	if(startline && regex){
		find="^"+find;
	}
	if(endline&& regex){
		if(find==""){
			find="^(.*)$";
			rep="\\1"+rep;
		}
		else{
			find<<"$";
		}
	}	

	int allreps1=0;
	int allreps=0;
	wxString txt;
	wxString styll=tcstyle->GetValue();
	bool notstyles=false;
	if(styll==""){notstyles=true;}else{styll=";"+styll+";";}
	bool onlysel=SelectedLines->GetValue();

	int fsel=pan->Grid1->FirstSel();


	for(int i = (!AllLines->GetValue() && fsel>=0)? fsel : 0; i<pan->Grid1->GetCount(); i++)
	{
		Dialogue *Dial=pan->Grid1->GetDial(i);
		if((notstyles||styll.Find(";"+Dial->Style+";")!=-1)&&
			!(onlysel&&!(pan->Grid1->sel.find(i)!=pan->Grid1->sel.end()))){

				if(wrep==STYLE){
					txt=Dial->Style;
				}else if(wrep==TXT){
					txt= Dial->Text;
				}else if(wrep==TXTTL){
					txt= Dial->TextTl;
				}else if(wrep==ACTOR){
					txt=Dial->Actor;
				}else if(wrep==EFFECT){
					txt=Dial->Effect;
				}
				if( !(startline||endline) && (find.empty()||txt.empty()))
				{
					if(txt.empty() && find.empty())
					{
						txt=rep; allreps=1;
					}
					else{continue;}
				}
				else if(regex){
					int rxflags=wxRE_ADVANCED;
					if(!matchcase){rxflags |= wxRE_ICASE;}
					wxRegEx nfind(find, rxflags);
					allreps=nfind.ReplaceAll(&txt,rep);
				}else if(startline||endline){

					wxString ltext=(!matchcase)? txt.Lower() : txt;
					wxString lfind= (!matchcase)? find.Lower() : find;
					if(startline){
						if(ltext.StartsWith(lfind) || lfind.empty()){
							txt.replace(0, lfind.Len(), rep);
							allreps=1;
						}
					} 
					if(endline){
						if(ltext.EndsWith(lfind) || lfind.empty()){
							int lenn=txt.Len();
							txt.replace(lenn - lfind.Len(), lenn, rep);
							allreps=1;
						}
					}

				}else if(!matchcase){
					wxString lfind=find.Lower();
					wxString ltext=txt;
					ltext.MakeLower();

					bool isfind=true;
					int newpos=0;
					int flen=lfind.Len();
					allreps=0;
					while(isfind){
						size_t poss=ltext.find(lfind,newpos);
						if(poss==-1){isfind=false;break;}
						else{
							if(poss<0){poss=0;}
							if(poss>txt.Len()){poss=txt.Len();}
							wxString hhh;
							ltext.Remove(poss,flen);
							ltext.insert(poss,rep);
							txt.Remove(poss,flen);
							txt.insert(poss,rep);
							allreps++;
							newpos=poss+rep.Len();
						}

					}
				}
				else{allreps=txt.Replace(find,rep);}
				if(allreps>0){
					Dialogue *Dialc=pan->Grid1->CopyDial(i);
					if(wrep==TXT){Dialc->Text=txt;}
					else if(wrep==TXTTL){Dialc->TextTl=txt;}
					else if(wrep==STYLE){Dialc->Style=txt;}
					else if(wrep==ACTOR){Dialc->Actor=txt;}
					else if(wrep==EFFECT){Dialc->Effect=txt;}
					allreps1+=allreps;

				}

		}

	}

	pan->Grid1->SetModified(REPLACE_ALL);
	pan->Grid1->Refresh(false);
	blockTextChange=true;
	KaiMessageBox(wxString::Format(_("Zmieniono %i razy."),allreps1),_("Szukaj Zamień"));
	AddRecent();
}


void FindReplace::OnButtonFind(wxCommandEvent& event)
{
	Find();
	fnext=false;
}

void FindReplace::OnButtonRep(wxCommandEvent& event)
{
	TabPanel *tab = Kai->GetTab();
	if(lastActive != tab->Edit->ebrow){Find();}
	bool searchInOriginal = CollumnTextOriginal->GetValue();
	long wrep= (tab->Grid1->transl && !searchInOriginal)? TXTTL : TXT;
	if(CollumnStyle->GetValue()){wrep=STYLE;}
	else if(CollumnActor->GetValue()){wrep=ACTOR;}
	else if(CollumnEffect->GetValue()){wrep=EFFECT;}

	wxString find1=FindText->GetValue();
	if(find1!=oldfind||findstart==-1||findend==-1){fromstart=true;fnext=false;oldfind=find1;Find();}
	if(findstart==-1||findend==-1){return;}
	fnext=true;
	wxString rep=RepText->GetValue();
	Grid *grid=tab->Grid1;

	Dialogue *Dial = grid->GetDial(reprow);

	if(wrep==STYLE){
		//Kai->GetTab()->Edit->StyleChoice->SetFocus();
		wxString oldstyle=Dial->Style;
		oldstyle.Remove(findstart, findend-findstart);
		oldstyle.insert(findstart,rep);

		grid->CopyDial(reprow)->Style=oldstyle;}
	else if(wrep==TXT || wrep==TXTTL){
		MTextEditor *tmp= (searchInOriginal)? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
		//tmp->SetFocus();
		tmp->Replace(findstart, findend, rep);
		grid->CopyDial(reprow)->Text=tmp->GetValue();
	}
	else if(wrep==ACTOR){
		//Kai->GetTab()->Edit->ActorEdit->SetFocus();
		tab->Edit->ActorEdit->choiceText->Replace(findstart, findend, rep);
		grid->CopyDial(reprow)->Actor=tab->Edit->ActorEdit->GetValue();
	}
	else if(wrep==EFFECT){
		//Kai->GetTab()->Edit->EffectEdit->SetFocus();
		tab->Edit->EffectEdit->choiceText->Replace(findstart, findend, rep);
		grid->CopyDial(reprow)->Effect=tab->Edit->EffectEdit->GetValue();
	}

	grid->SetModified(REPLACE_SINGLE);
	grid->Refresh(false);
	postxt=findstart + rep.Len();
	Find();
}

void FindReplace::Find()
{
	TabPanel *pan =Kai->GetTab();
	bool searchInOriginal = CollumnTextOriginal->GetValue();
	long wrep= (pan->Grid1->transl && !searchInOriginal)? TXTTL : TXT;
	if(CollumnStyle->GetValue()){wrep=STYLE;}
	else if(CollumnActor->GetValue()){wrep=ACTOR;}
	else if(CollumnEffect->GetValue()){wrep=EFFECT;}

	bool matchcase=MatchCase->GetValue();
	bool regex=RegEx->GetValue();
	bool startline=StartLine->GetValue();
	bool endline=EndLine->GetValue();

	wxString find1=FindText->GetValue();
	if(find1!=oldfind){fromstart=true;fnext=false;oldfind=find1;}
	if(!fromstart && lastActive != pan->Edit->ebrow){lastActive = pan->Edit->ebrow;}

	if(startline && regex){
		find1="^"+find1;
	}
	if(endline && regex){
		find1<<"$";
	}

	//Kai->Freeze();


	wxString txt;
	int mwhere=-1;
	size_t mlen=0;
	bool foundsome=false;
	if(fromstart){
		int fsel=pan->Grid1->FirstSel();
		posrow= (!AllLines->GetValue() && fsel>=0)? fsel : 0;
		postxt=0;
	}
	wxString styll=tcstyle->GetValue();
	bool styles=false;
	if(styll!=""){
		styles=true;
		styll=";"+styll+";";
	}
	bool onlysel=SelectedLines->GetValue();	

	while(posrow<pan->Grid1->GetCount())
	{
		Dialogue *Dial=pan->Grid1->GetDial(posrow);
		if((!styles && !onlysel) || 
			(styles && styll.Find(";"+Dial->Style+";")!=-1) || 
			(onlysel && pan->Grid1->sel.find(posrow) != pan->Grid1->sel.end())){
				if(wrep==STYLE){
					txt=Dial->Style;
				}else if(wrep==TXT || wrep==TXTTL){
					txt=(wrep==TXTTL)? Dial->TextTl : Dial->Text;
				}else if(wrep==ACTOR){
					txt=Dial->Actor;
				}else if(wrep==EFFECT){
					txt=Dial->Effect;
				}

				//no to szukamy
				if(!(startline || endline) && (find1.empty()||txt.empty()))
				{
					if(txt.empty() && find1.empty()){
						mwhere=0; mlen=0;
					}
					else{postxt=0;posrow++;continue;}

				}else if(regex){
					int rxflags=wxRE_ADVANCED;
					if(!matchcase){rxflags |= wxRE_ICASE;}
					wxRegEx rgx (find1,rxflags);
					if (rgx.IsValid()) {
						wxString cuttext=txt.Mid(postxt);
						if (rgx.Matches(cuttext)) {
							wxString reslt=rgx.GetMatch(cuttext,0);
							if(reslt==""){mwhere=-1;mlen=0;}
							else{
								mwhere=cuttext.Find(reslt)+postxt;
								mlen=reslt.Len();}
						}else{postxt=0;posrow++;continue;}

					}

				}else{
					wxString ltext= (!matchcase)? txt.Lower() : txt;
					wxString lfind= (!matchcase)? find1.Lower() : find1;
					if(startline){
						if(ltext.StartsWith(lfind) || lfind.empty()){
							mwhere = 0;
							postxt = 0; 
						}else{
							mwhere=-1;
						}
					}
					if(endline){
						if(ltext.EndsWith(lfind) || lfind.empty()){
							mwhere=txt.Len()-lfind.Len();
							postxt=0;
						}else{
							mwhere=-1;
						}
					}else{
						mwhere= ltext.find(lfind,postxt);
					}
					mlen=lfind.Len();
				}

				if (mwhere!=-1){
					postxt=mwhere+mlen;
					findstart=mwhere;
					findend=postxt;
					lastActive = reprow = posrow;

					if(!SelectedLines->GetValue()){pan->Grid1->SelectRow(posrow,false,true);}
					pan->Edit->SetLine(posrow);
					pan->Grid1->ScrollTo(posrow,true);
					if(SelectedLines->GetValue()){pan->Grid1->Refresh(false);}
					if(wrep==STYLE){
						//pan->Edit->StyleChoice->SetFocus();
					}
					else if(wrep==TXT || wrep==TXTTL){
						MTextEditor *tmp= ( searchInOriginal)? pan->Edit->TextEditOrig : pan->Edit->TextEdit;
						//tmp->SetFocus();
						tmp->SetSelection(mwhere,findend);
					}
					if(wrep==ACTOR){
						//pan->Edit->ActorEdit->SetFocus();
						pan->Edit->ActorEdit->choiceText->SetSelection(mwhere,findend);
					}
					if(wrep==EFFECT){
						//pan->Edit->EffectEdit->SetFocus();
						pan->Edit->EffectEdit->choiceText->SetSelection(mwhere,findend);
					}

					foundsome=true;
					if((size_t)postxt>=txt.Len() || startline){
						posrow++;postxt=0;
					}
					break;
				}else{
					postxt=0; 
					posrow++;
				}
				
		}else{postxt=0;posrow++;}
		if(!foundsome && posrow> pan->Grid1->GetCount()-1){
			blockTextChange=true;
			if (KaiMessageBox(_("Wyszukiwanie zakończone, rozpocząć od początku?"), _("Potwierdzenie"),
				wxICON_QUESTION | wxYES_NO, this) == wxYES ){
					posrow=0;//foundsome=true;
			}else{posrow=0;foundsome=true;break;}
		}
	}
	if(!foundsome){
		blockTextChange=true;
		KaiMessageBox(_("Nie znaleziono podanej frazy \"")+FindText->GetValue()+"\".", _("Potwierdzenie")); 
		posrow=0;
		fromstart=true;
	}
	if(fromstart){AddRecent();fromstart=false;}
	//Kai->Thaw();
}


void FindReplace::OnClose(wxCommandEvent& event)
{
	Hide();
	//Destroy();
}

void FindReplace::ReloadStyle()
{
	if(tcstyle){tcstyle->SetValue("");}
}

void FindReplace::OnStylesWin(wxCommandEvent& event)
{
	tcstyle->SetValue(GetCheckedElements(Kai));
}

void FindReplace::AddRecent(){
	wxString text=FindText->GetValue();


	wxArrayString wfind;
	Options.GetTable(FindRecent,wfind,"\f");
	if(wfind.size()>20){wfind.RemoveAt(20,wfind.size()-20);}
	for(size_t i=0;i<wfind.GetCount();i++)
	{
		if(wfind[i]==text){
			wfind.RemoveAt(i);
			FindText->Delete(i);
		}
	}
	wfind.Insert(text,0);
	FindText->Insert(text,0);

	FindText->SetSelection(0);
	Options.SetTable(FindRecent,wfind,"\f");
	if(repl){
		wxString text=RepText->GetValue();
		wxArrayString wrepl;
		Options.GetTable(ReplaceRecent,wrepl,"\f");
		if(wrepl.size()>20){wrepl.RemoveAt(20,wrepl.size()-20);}

		for(size_t i=0;i<wrepl.GetCount();i++)
		{
			if(wrepl[i]==text){
				wrepl.RemoveAt(i);
				RepText->Delete(i);
			}
		}

		wrepl.Insert(text,0);
		RepText->Insert(text,0);

		RepText->SetSelection(0);
		Options.SetTable(ReplaceRecent,wrepl,"\f");
	}

}

void FindReplace::OnRecheck(wxCommandEvent& event)
{
	int id=event.GetId();
	if (id==ID_SLINE && EndLine->GetValue()){
		EndLine->SetValue(false);}
	else if (id==ID_ELINE && StartLine->GetValue()){
		StartLine->SetValue(false);}

}

void FindReplace::Reset()
{
	fromstart=true;
	fnext=false;
	if(CollumnTextOriginal->GetValue()){CollumnText->SetValue(true);}
	CollumnTextOriginal->Enable(Kai->GetTab()->Grid1->transl);
}

void FindReplace::OnSetFocus(wxActivateEvent& event){
	if(!event.GetActive() || blockTextChange){blockTextChange=false; return;}
	//wxLogStatus("focus");
	long from, to, fromO, toO;
	EditBox *edit = Kai->GetTab()->Edit;
	edit->TextEdit->GetSelection(&from,&to);
	edit->TextEditOrig->GetSelection(&fromO,&toO);
	if(from<to){
		wxString selected = edit->TextEdit->GetValue().SubString(from,to-1);
		if(selected.Lower() != FindText->GetValue().Lower()){FindText->SetValue(selected);}
	}
	else if(fromO<toO){
		wxString selected = edit->TextEditOrig->GetValue().SubString(fromO,toO-1);
		if(selected.Lower() != FindText->GetValue().Lower()){FindText->SetValue(selected);}
	}
	FindText->SetFocus();
	//hasFocus=true;
}

void FindReplace::OnEnterConfirm(wxCommandEvent& event)
{
	if(RepText && RepText->HasFocus()){
		OnButtonRep(event);
	}else{
		Find();
		fnext=false;
	}
}