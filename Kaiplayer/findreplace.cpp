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
#include "KaiStaticBoxSizer.h"
#include "KaiMessageBox.h"
#include "Stylelistbox.h"
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/regex.h>
#include <wx/clipbrd.h>


findreplace::findreplace(kainoteFrame* kfparent, findreplace* last, bool replace, bool sellines)
	: KaiDialog(kfparent, -1,(sellines)?_("Zaznacz"): (replace)?_("Znajdź i zamień"):_("Znajdź"))
	,hasFocus(false)
{
	SetForegroundColour(Options.GetColour("Window Text"));
	SetBackgroundColour(Options.GetColour("Window Background"));
	Kai=kfparent;
	reprow=posrow=0;
	postxt=0;
	findstart=-1;
	findend=-1;
	fnext=false;
	repl=replace;
	fromstart=true;
	wxString ES;
	RadioButton8=NULL;
	RepText=NULL;
	StartLine=EndLine=NULL;
	tcstyle=NULL;
	wxArrayString wfind=Options.GetTable("Find Recent","\f");
	if(wfind.size()>20){wfind.RemoveAt(19,wfind.size()-20);}

	if(!sellines){
		wxIcon icn;
		icn.CopyFromBitmap(CreateBitmapFromPngResource("SEARCH"));
		SetIcon(icn);

		DialogSizer* mainfrbsizer=new DialogSizer(wxVERTICAL);
		wxBoxSizer* mainfrbsizer1=new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* mainfrbsizer2=new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* mainfrbsizer3=new wxBoxSizer(wxHORIZONTAL);

		//pionowy sizer kolumna 1
		KaiStaticBoxSizer* frsbsizer=new KaiStaticBoxSizer(wxVERTICAL,this,_("Znajdź"));
		FindText = new KaiChoice(this, ID_FINDTEXT, (last)?last->FindText->GetValue() : ES, wxDefaultPosition, wxSize(342,-1),wfind);
		frsbsizer->Add(FindText,0,wxEXPAND,0);
		mainfrbsizer1->Add(frsbsizer,0,wxEXPAND|wxALL,3);

		if(repl){
			wxArrayString wrepl=Options.GetTable("Replace Recent","\f");
			if(wrepl.size()>20){wrepl.RemoveAt(19,wrepl.size()-20);}
			KaiStaticBoxSizer* frsbsizer1=new KaiStaticBoxSizer(wxVERTICAL,this,_("Zamień"));
			RepText = new KaiChoice(this, ID_REPTEXT, (last && last->RepText)?last->RepText->GetValue() : ES, wxDefaultPosition, wxSize(342,-1),wrepl);
			frsbsizer1->Add(RepText,0,wxEXPAND,0);
			mainfrbsizer1->Add(frsbsizer1,0,wxEXPAND|wxALL,3);
		}

		wxBoxSizer* frbsizer1=new wxBoxSizer(wxVERTICAL);
		MatchCase = new KaiCheckBox(this, -1, _("Uwzględniaj wielkość liter"));
		MatchCase->SetValue((last)?last->MatchCase->GetValue():false);
		RegEx = new KaiCheckBox(this, -1, _("Wyrażenia regularne"));
		RegEx->SetValue((last)?last->RegEx->GetValue():false);
		StartLine = new KaiCheckBox(this, ID_SLINE, _("Początek tekstu"));
		StartLine->SetValue((last&&last->StartLine)?last->StartLine->GetValue():false);
		EndLine = new KaiCheckBox(this, ID_ELINE, _("Koniec tekstu"));
		EndLine->SetValue((last&&last->EndLine)?last->EndLine->GetValue():false);
		frbsizer1->Add(MatchCase,0,wxEXPAND|wxTOP|wxBOTTOM,2);
		frbsizer1->Add(RegEx,0,wxEXPAND|wxTOP|wxBOTTOM,2);
		frbsizer1->Add(StartLine,0,wxEXPAND|wxTOP|wxBOTTOM,2);
		frbsizer1->Add(EndLine,0,wxEXPAND|wxTOP|wxBOTTOM,2);


		KaiStaticBoxSizer* frsbsizer2=new KaiStaticBoxSizer(wxVERTICAL,this,_("W polu"));
		wxBoxSizer* frbsizer2=new wxBoxSizer(wxHORIZONTAL);
		RadioButton1 = new KaiRadioButton(this, -1, _("Tekst"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		RadioButton2 = new KaiRadioButton(this, -1, _("Styl"));
		RadioButton1->SetValue((last)?last->RadioButton1->GetValue():true);
		RadioButton2->SetValue((last)?last->RadioButton2->GetValue():false);
		frbsizer2->Add(RadioButton1,1,wxALL|wxLEFT,1);
		frbsizer2->Add(RadioButton2,1,wxALL|wxLEFT,1);

		wxBoxSizer* frbsizer3=new wxBoxSizer(wxHORIZONTAL);
		RadioButton5 = new KaiRadioButton(this, -1, _("Aktor"));
		RadioButton6 = new KaiRadioButton(this, -1, _("Efekt"));
		RadioButton5->SetValue((last)?last->RadioButton5->GetValue():false);
		RadioButton6->SetValue((last)?last->RadioButton6->GetValue():false);
		frbsizer3->Add(RadioButton5,1,wxEXPAND|wxLEFT,1);
		frbsizer3->Add(RadioButton6,1,wxEXPAND|wxLEFT,1);

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
		Button1 = new MappedButton(this, ID_BFIND, _("Znajdź"), 0, wxDefaultPosition, wxSize(124,-1));
		frbsizer->Add(Button1,1,wxEXPAND|wxTOP|wxBOTTOM,6);

		if(repl){
			Button2 = new MappedButton(this, ID_BREP, _("Zamień"));
			Button3 = new MappedButton(this, ID_BREPALL, _("Zamień wszystko"));
			frbsizer->Add(Button2,1,wxEXPAND|wxTOP|wxBOTTOM,6);
			frbsizer->Add(Button3,1,wxEXPAND|wxTOP|wxBOTTOM,6);
		}
		Button4 = new MappedButton(this, ID_BCLOSE, _("Zamknij"));
		frbsizer->Add(Button4,1,wxEXPAND|wxTOP|wxBOTTOM,6);

		//łączenie całości znajdowania i opcji z przyciskami
		mainfrbsizer3->Add(mainfrbsizer1,0,wxEXPAND|wxRIGHT,3);
		mainfrbsizer3->Add(frbsizer,0,wxEXPAND|wxLEFT,3);

		//poziomy sizer spód
		KaiStaticBoxSizer* frsbsizer3=new KaiStaticBoxSizer(wxHORIZONTAL,this,_("Linijki"));

		RadioButton3 = new KaiRadioButton(this, -1, _("Wszystkie linijki"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		RadioButton4 = new KaiRadioButton(this, -1, _("Zaznaczone linijki"));
		RadioButton7 = new KaiRadioButton(this, -1, _("Od zaznaczonej"));
		RadioButton3->SetValue((last)?last->RadioButton3->GetValue():true);
		RadioButton4->SetValue((last)?last->RadioButton4->GetValue():false);
		RadioButton7->SetValue((last&&last->RadioButton7)?last->RadioButton7->GetValue():false);

		wxBoxSizer* frbsizer4=new wxBoxSizer(wxHORIZONTAL);
		Bplus = new MappedButton(this, ID_BPLUS, "+", 0, wxDefaultPosition, wxSize(22,22));
		tcstyle = new KaiTextCtrl(this, ID_TCSTYLE,(last&&last->tcstyle)?last->tcstyle->GetValue():ES, wxDefaultPosition, wxSize(-1,22));
		frbsizer4->Add(Bplus,0,0,0);
		frbsizer4->Add(tcstyle,0,wxLEFT,3);

		frsbsizer3->Add(RadioButton3,0,wxALL,2);
		frsbsizer3->Add(RadioButton4,0,wxALL,2);
		frsbsizer3->Add(RadioButton7,0,wxALL,2);
		frsbsizer3->Add(frbsizer4,0,wxALL,2);

		mainfrbsizer->Add(mainfrbsizer3,0,wxEXPAND|wxALL,5);
		mainfrbsizer->Add(frsbsizer3,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,8);

		//Ustawienie sizera
		SetSizerAndFit(mainfrbsizer);
		//wxEVT_COMMAND_CHECKBOX_CLICKED
		Connect(ID_SLINE,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&findreplace::OnRecheck);
		Connect(ID_ELINE,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&findreplace::OnRecheck);

		Connect(ID_BFIND,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&findreplace::OnButtonFind);
		//Connect(ID_FINDTEXT,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&findreplace::OnTextUpdate);
		if(repl){
			Connect(ID_BREP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&findreplace::OnButtonRep);
			Connect(ID_BREPALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&findreplace::OnReplaceAll);
		}

		Connect(ID_BPLUS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&findreplace::OnStylesWin);
		Connect(wxEVT_ACTIVATE,(wxObjectEventFunction)&findreplace::OnSetFocus);
		//Bind(wxEVT_KILL_FOCUS,[=](wxFocusEvent &evt){wxLogStatus("kill focus");hasFocus=false;});
		
	}

	//Zaznaczenia
	else{

		DialogSizer *slsizer= new DialogSizer(wxVERTICAL);
		wxBoxSizer *slrbsizer= new wxBoxSizer(wxHORIZONTAL);
		RadioButton3 = new KaiRadioButton(this, -1, _("Zawiera"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		RadioButton4 = new KaiRadioButton(this, -1, _("Nie zawiera"));
		RadioButton3->SetValue((last)?last->RadioButton3->GetValue():true);
		RadioButton4->SetValue((last)?last->RadioButton4->GetValue():false);
		slrbsizer->Add(RadioButton3,1,wxALL|wxEXPAND,3);
		slrbsizer->Add(RadioButton4,1,wxALL|wxEXPAND,3);

		KaiStaticBoxSizer* slsbsizer=new KaiStaticBoxSizer(wxVERTICAL,this,_("Znajdź"));
		wxBoxSizer *sltpsizer= new wxBoxSizer(wxHORIZONTAL);
		FindText = new KaiChoice(this, ID_FINDTEXT, (last)?last->FindText->GetValue():ES, wxDefaultPosition, wxSize(-1,24),wfind);
		Bplus = new MappedButton(this, ID_BPLUS, "+", 0, wxDefaultPosition, wxSize(24,24));
		sltpsizer->Add(FindText,1,wxALL|wxEXPAND,3);
		sltpsizer->Add(Bplus,0,wxALL,3);

		MatchCase = new KaiCheckBox(this, -1, _("Uwzględniaj wielkość liter"));
		MatchCase->SetValue((last)?last->MatchCase->GetValue():false);
		RegEx = new KaiCheckBox(this, -1, _("Wyrażenia regularne"));
		RegEx->SetValue((last)?last->RegEx->GetValue():false);

		slsbsizer->Add(slrbsizer,0,wxEXPAND,0);
		slsbsizer->Add(sltpsizer,0,wxEXPAND,0);
		slsbsizer->Add(MatchCase,0,wxALL,3);
		slsbsizer->Add(RegEx,0,wxALL,3);

		KaiStaticBoxSizer* slsbsizer1=new KaiStaticBoxSizer(wxVERTICAL,this,_("W polu"));
		wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL);
		wxBoxSizer *sizer1 = new wxBoxSizer( wxHORIZONTAL);
		RadioButton1 = new KaiRadioButton(this, -1, _("Tekst"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		RadioButton2 = new KaiRadioButton(this, -1, _("Styl"));
		RadioButton5 = new KaiRadioButton(this, -1, _("Aktor"));
		RadioButton6 = new KaiRadioButton(this, -1, _("Efekt"));
		RadioButton7 = new KaiRadioButton(this, -1, _("Czas Początkowy"));
		RadioButton8 = new KaiRadioButton(this, -1, _("Czas Końcowy"));
		RadioButton1->SetValue((last)?last->RadioButton1->GetValue():true);
		RadioButton2->SetValue((last)?last->RadioButton2->GetValue():false);
		RadioButton5->SetValue((last)?last->RadioButton5->GetValue():false);
		RadioButton6->SetValue((last)?last->RadioButton6->GetValue():false);
		RadioButton5->SetValue((last)?last->RadioButton7->GetValue():false);
		RadioButton6->SetValue((last)?last->RadioButton8->GetValue():false);
		sizer->Add(RadioButton1,1,wxALL,3);
		sizer->Add(RadioButton2,1,wxALL,3);
		sizer->Add(RadioButton5,1,wxALL,3);
		sizer->Add(RadioButton6,1,wxALL,3);
		sizer1->Add(RadioButton7,1,wxALL|wxEXPAND,3);
		sizer1->Add(RadioButton8,1,wxALL|wxEXPAND,3);
		slsbsizer1->Add(sizer,1,wxEXPAND);
		slsbsizer1->Add(sizer1,1,wxEXPAND);
		KaiStaticBoxSizer* slsbsizer2=new KaiStaticBoxSizer(wxHORIZONTAL,this,_("Dialogi / komentarze"));

		Fdial = new KaiCheckBox(this, -1, _("Dialogi"));
		Fcomm = new KaiCheckBox(this, -1, _("Komentarze"));
		Fdial->SetValue(true);
		slsbsizer2->Add(Fdial,0,wxALL,3);
		slsbsizer2->Add(Fcomm,0,wxALL,3);

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
		Button1 = new MappedButton(this, ID_BFIND, _("Zaznacz"));
		Button4 = new MappedButton(this, ID_BCLOSE, _("Zamknij"));
		slbtsizer->Add(Button1,1,wxALL,5);
		slbtsizer->Add(Button4,1,wxALL,5);


		slsizer->Add(slsbsizer,0,wxEXPAND|wxLEFT|wxRIGHT,2);
		slsizer->Add(slsbsizer1,0,wxEXPAND|wxLEFT|wxRIGHT,2);
		slsizer->Add(slsbsizer2,0,wxEXPAND|wxLEFT|wxRIGHT,2);
		slsizer->Add(Selections,0,wxEXPAND|wxLEFT|wxRIGHT,2);
		slsizer->Add(Actions,0,wxEXPAND|wxLEFT|wxRIGHT,2);
		slsizer->Add(slbtsizer,0,wxALL|wxALIGN_CENTER,0);

		SetSizerAndFit(slsizer);


		Connect(ID_BFIND,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&findreplace::OnSelections);
		Connect(ID_BPLUS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&findreplace::OnStylesWin1);
	}//koniec okienka zaznaczania

	Connect(ID_BCLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&findreplace::OnClose);
	SetEscapeId(ID_BCLOSE);

	CenterOnParent();
	if(last){last->Destroy();}
}

findreplace::~findreplace()
{

}


void findreplace::OnReplaceAll(wxCommandEvent& event)
{
	long wrep=TXT;
	if(RadioButton2->GetValue()){wrep=STYLE;}
	else if(RadioButton5->GetValue()){wrep=ACTOR;}
	else if(RadioButton6->GetValue()){wrep=EFFECT;}

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

	size_t allreps1=0;
	size_t allreps=0;
	wxString txt;
	wxString styll=tcstyle->GetValue();
	bool notstyles=false;
	if(styll==""){notstyles=true;}else{styll=";"+styll+";";}
	bool onlysel=RadioButton4->GetValue();

	TabPanel *pan=Kai->GetTab();
	//pan->Video->blockpaint=true;
	int fsel=pan->Grid1->FirstSel();


	for(int i=(!RadioButton3->GetValue()&&fsel>=0)?fsel:0;i<pan->Grid1->GetCount();i++)
	{
		Dialogue *Dial=pan->Grid1->GetDial(i);
		if((notstyles||styll.Find(";"+Dial->Style+";")!=-1)&&
			!(onlysel&&!(pan->Grid1->sel.find(i)!=pan->Grid1->sel.end()))){

				if(wrep==STYLE){
					txt=Dial->Style;}
				else if(wrep==TXT || wrep==TXTTL){
					if(pan->Grid1->transl&&Dial->TextTl!=""){txt= Dial->TextTl; wrep=TXTTL;}
					else{txt= Dial->Text; wrep=TXT;}}
				else if(wrep==ACTOR){
					txt=Dial->Actor;}
				else if(wrep==EFFECT){
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
				//}
		}

	}

	pan->Grid1->SetModified();
	pan->Grid1->Refresh(false);

	KaiMessageBox(wxString::Format(_("Zmieniono %i razy."),allreps1),_("Szukaj Zamień"));
	AddRecent();
	//pan->Video->blockpaint=false;
}

void findreplace::SelectLine()
{
	long wrep=TXT;
	if(RadioButton2->GetValue()){wrep=STYLE;}
	else if(RadioButton5->GetValue()){wrep=ACTOR;}
	else if(RadioButton6->GetValue()){wrep=EFFECT;}
	else if(RadioButton7->GetValue()){wrep=START;}
	else if(RadioButton8->GetValue()){wrep=END;}

	wxString find=FindText->GetValue();
	//if(find==""){return;}

	size_t allreps=0;
	wxString txt,whatcopy;
	std::vector<Dialogue *> mdial;

	bool matchcase=MatchCase->GetValue();
	bool regex=RegEx->GetValue();
	bool contain=RadioButton3->GetValue();
	bool notcont=RadioButton4->GetValue();
	bool diall=Fdial->GetValue();
	bool commm=Fcomm->GetValue();
	int sopt = Selections->GetSelection();
	int act = Actions->GetSelection();

	if(!matchcase){find.MakeLower();}
	TabPanel *pan=Kai->GetTab();
	//wxString test;
	if(sopt==0){pan->Grid1->sel.clear();}

	for(int i=0;i<Kai->GetTab()->Grid1->GetCount();i++)
	{
		Dialogue *Dial=pan->Grid1->GetDial(i);

		if(wrep==STYLE){
			txt=Dial->Style;}
		else if(wrep==TXT){
			txt=(Kai->GetTab()->Grid1->transl&&Dial->TextTl!="")? Dial->TextTl : Dial->Text;}
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
			&&((diall&&!Dial->IsComment)||(commm&&Dial->IsComment))){
				bool select=(sopt==2)?false:true;
				if(select){
					//wxLogMessage("zaznaczenia\r\n");
					pan->Grid1->sel[i]=select;
					allreps++;
				}
				else{
					//wxLogMessage("odznaczenia\r\n");
					std::map<int,bool>::iterator it=pan->Grid1->sel.find(i);
					if(it!=pan->Grid1->sel.end()){
						pan->Grid1->sel.erase(it);
						allreps++;
					}
				}
		}

		if((pan->Grid1->sel.find(i)!=pan->Grid1->sel.end())&&act!=0){
			if(act<3){whatcopy<<Dial->GetRaw(pan->Grid1->transl&&Dial->TextTl!="");}
			else if(act<5){Dial->State=1;mdial.push_back(Dial);}
			else if(act<6){
				Dialogue *dialc=pan->Grid1->CopyDial(i); 
				dialc->State=1;
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
		pan->Grid1->DeleteRows();
		if(act==3||act==4)
		{
			pan->Grid1->InsertRows((act==3)? 0 : pan->Grid1->GetCount(), mdial);
			mdial.clear();
		}
	}
	int fsel=pan->Grid1->FirstSel();
	int wset=(fsel<0)? pan->Edit->ebrow : fsel;
	pan->Edit->SetIt(wset);
	pan->Grid1->SetModified(false);
	pan->Grid1->RepaintWindow();
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
}


void findreplace::OnButtonFind(wxCommandEvent& event)
{
	Find();
	fnext=false;
}

void findreplace::OnButtonRep(wxCommandEvent& event)
{
	long wrep=TXT;
	if(RadioButton2->GetValue()){wrep=STYLE;}
	else if(RadioButton5->GetValue()){wrep=ACTOR;}
	else if(RadioButton6->GetValue()){wrep=EFFECT;}
	
	wxString find1=FindText->GetValue();
	if(find1!=oldfind||findstart==-1||findend==-1){fromstart=true;fnext=false;oldfind=find1;Find();}
	if(findstart==-1||findend==-1){return;}
	fnext=true;
	wxString rep=RepText->GetValue();
	Grid *grid=Kai->GetTab()->Grid1;

	Dialogue *Dial = grid->GetDial(reprow);
	MTextEditor *tmp=(Kai->GetTab()->Grid1->transl&&Dial->TextTl!="")?Kai->GetTab()->Edit->TextEditTl : Kai->GetTab()->Edit->TextEdit;
	if(wrep==STYLE){
		Kai->GetTab()->Edit->StyleChoice->SetFocus();
		wxString oldstyle=Dial->Style;
		oldstyle.Remove(findstart, findend-findstart);
		oldstyle.insert(findstart,rep);

		grid->CopyDial(reprow)->Style=oldstyle;}
	else if(wrep==TXT){
		tmp->SetFocus();
		tmp->Replace (findstart, findend, rep);
		grid->CopyDial(reprow)->Text=tmp->GetValue();
	}
	else if(wrep==ACTOR){
		Kai->GetTab()->Edit->ActorEdit->SetFocus();
		Kai->GetTab()->Edit->ActorEdit->choiceText->Replace (findstart, findend, rep);
		grid->CopyDial(reprow)->Actor=Kai->GetTab()->Edit->ActorEdit->GetValue();
	}
	else if(wrep==EFFECT){
		Kai->GetTab()->Edit->EffectEdit->SetFocus();
		Kai->GetTab()->Edit->EffectEdit->choiceText->Replace (findstart, findend, rep);
		grid->CopyDial(reprow)->Effect=Kai->GetTab()->Edit->EffectEdit->GetValue();
	}

	grid->SetModified();
	grid->Refresh(false);
	postxt=findstart+rep.Len();
	Find();
}

void findreplace::Find()
{
	long wrep=TXT;
	if(RadioButton2->GetValue()){wrep=STYLE;}
	else if(RadioButton5->GetValue()){wrep=ACTOR;}
	else if(RadioButton6->GetValue()){wrep=EFFECT;}

	bool matchcase=MatchCase->GetValue();
	bool regex=RegEx->GetValue();
	bool startline=StartLine->GetValue();
	bool endline=EndLine->GetValue();

	wxString find1=FindText->GetValue();
	if(find1!=oldfind){fromstart=true;fnext=false;oldfind=find1;}

	if(startline && regex){
		find1="^"+find1;
	}
	if(endline && regex){
		find1<<"$";
	}
	TabPanel *pan =Kai->GetTab(); 
	//Kai->Freeze();
	

	wxString txt;
	int mwhere=-1;
	size_t mlen=0;
	bool foundsome=false;
	if(fromstart){int fsel=pan->Grid1->FirstSel();posrow=(RadioButton4->GetValue()&&fsel>=0)?fsel:0;postxt=0;}
	wxString styll=tcstyle->GetValue();
	bool notstyles=false;
	if(styll==""){notstyles=true;}else{styll=";"+styll+";";}
	bool onlysel=RadioButton4->GetValue();	

	while(posrow<pan->Grid1->GetCount())
	{

		Dialogue *Dial=pan->Grid1->GetDial(posrow);
		if(notstyles||styll.Find(";"+Dial->Style+";")!=-1
			&& !(onlysel && !(pan->Grid1->sel.find(posrow)!=pan->Grid1->sel.end()))){
				if(wrep==STYLE){
					txt=Dial->Style;}
				else if(wrep==TXT){//W szukaniu tłumaczenie ma pierwszeństwo
					txt=(pan->Grid1->transl&&Dial->TextTl!="")?Dial->TextTl:Dial->Text;}
				else if(wrep==ACTOR){
					txt=Dial->Actor;}
				else if(wrep==EFFECT){
					txt=Dial->Effect;}
				
				//no to szukamy
				if(!(startline || endline) && (find1.empty()||txt.empty()))
				{
					if(txt.empty() && find1.empty())
					{
						mwhere=0; mlen=0;
					}
					else{postxt=0;posrow++;continue;}
					
				}
				else if(regex){
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

				}
				else{
					wxString ltext=(!matchcase)? txt.Lower() : txt;
					wxString lfind= (!matchcase)? find1.Lower() : find1;
					if(startline){if(ltext.StartsWith(lfind) || lfind.empty()){mwhere= 0;postxt=0;} else {mwhere=-1;}}
					if(endline){if(ltext.EndsWith(lfind) || lfind.empty()){mwhere=txt.Len()-lfind.Len();postxt=0;} else {mwhere=-1;}}
					else{mwhere= ltext.find(lfind,postxt);}
					mlen=lfind.Len();
				}
				
				if (mwhere!=-1){
					postxt=mwhere+mlen;
					findstart=mwhere;
					findend=postxt;
					reprow=posrow;

					pan->Grid1->SelectRow(posrow,false,true);
					pan->Grid1->ScrollTo(posrow,true);
					pan->Edit->SetIt(posrow);
					if(wrep==STYLE){
						pan->Edit->StyleChoice->SetFocus();
					}
					else if(wrep==TXT){
						MTextEditor *tmp=(pan->Grid1->transl&&Dial->TextTl=="")?
							pan->Edit->TextEditTl : pan->Edit->TextEdit;
						tmp->SetFocus();
						tmp->SetSelection(mwhere,findend);
					}
					if(wrep==ACTOR){
						pan->Edit->ActorEdit->SetFocus();
						pan->Edit->ActorEdit->choiceText->SetSelection(mwhere,findend);
					}
					if(wrep==EFFECT){
						pan->Edit->EffectEdit->SetFocus();
						pan->Edit->EffectEdit->choiceText->SetSelection(mwhere,findend);
					}

					foundsome=true;
					if((size_t)postxt>=txt.Len()||startline){posrow++;postxt=0;}
					break;
				}
				else{postxt=0; posrow++;}
				if(!foundsome && posrow> pan->Grid1->GetCount()-1){
					if (KaiMessageBox(_("Wyszukiwanie zakończone, rozpocząć od początku?"), _("Potwierdzenie"),
						wxICON_QUESTION | wxYES_NO, this) == wxYES ){
							posrow=0;foundsome=true;
					}else{posrow=0;foundsome=true;break;}
				}
		}else{postxt=0;posrow++;}
	}
	if(!foundsome){KaiMessageBox(_("Nie znaleziono podanej frazy \"")+FindText->GetValue()+"\".", _("Potwierdzenie")); fromstart=true;}
	if(fromstart){AddRecent();fromstart=false;}
	//Kai->Thaw();
}


void findreplace::OnClose(wxCommandEvent& event)
{
	Hide();
	//Destroy();
}

void findreplace::ReloadStyle()
{
	if(tcstyle){tcstyle->SetValue("");}
}

void findreplace::OnStylesWin(wxCommandEvent& event)
{
	tcstyle->SetValue(GetCheckedElements(Kai));
}

void findreplace::OnSelections(wxCommandEvent& event)
{
	SelectLine();
	AddRecent();
}

void findreplace::OnStylesWin1(wxCommandEvent& event)
{
	wxString kkk=GetCheckedElements(Kai);
	kkk.Replace(";","|");
	RadioButton2->SetValue(true);
	RegEx->SetValue(true);
	FindText->SetValue(kkk);
}

void findreplace::AddRecent(){
	wxString text=FindText->GetValue();


	wxArrayString wfind=Options.GetTable("Find Recent","\f");
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
	Options.SetTable("Find Recent",wfind,"\f");
	if(repl){
		wxString text=RepText->GetValue();
		wxArrayString wrepl=Options.GetTable("Replace Recent","\f");
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
		Options.SetTable("Replace Recent",wrepl,"\f");
	}

}

void findreplace::OnRecheck(wxCommandEvent& event)
{
	int id=event.GetId();
	if (id==ID_SLINE && EndLine->GetValue()){
		EndLine->SetValue(false);}
	else if (id==ID_ELINE && StartLine->GetValue()){
		StartLine->SetValue(false);}

}

void findreplace::Reset()
{
	fromstart=true;
	fnext=false;
}

void findreplace::OnSetFocus(wxActivateEvent& event){
	if(hasFocus){hasFocus=false; event.Skip(); return;}
	//wxLogStatus("focus");
	long from, to, fromO, toO;
	EditBox *edit = Kai->GetTab()->Edit;
	edit->TextEdit->GetSelection(&from,&to);
	edit->TextEditTl->GetSelection(&fromO,&toO);
	if(from<to){
		wxString selected = edit->TextEdit->GetValue().SubString(from,to-1);
		if(selected.Lower() != FindText->GetValue().Lower()){FindText->SetValue(selected);}
	}
	else if(fromO<toO){
		wxString selected = edit->TextEditTl->GetValue().SubString(fromO,toO-1);
		if(selected.Lower() != FindText->GetValue().Lower()){FindText->SetValue(selected);}
	}
	hasFocus=true;
	event.Skip();
}