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



#include "OptionsDialog.h"
#include "config.h"
#include "kainoteMain.h"
#include "Hotkeys.h"
#include <wx/dir.h>
#include "NumCtrl.h"
#include "ColorPicker.h"
#include "KaiTextCtrl.h"
#include "FontDialog.h"
#include "OpennWrite.h"
#include "KaiMessageBox.h"
#include "KaiStaticText.h"
#include "OptionsPanels.h"

const wxString windowNames[] = {_("Globalny"),_("Napisy"),_("Edytor"),_("Wideo"),_("Audio")};

void ItemHotkey::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = dc->GetTextExtent(accel);
	
	if(modified){dc->SetTextForeground(Options.GetColour(WindowWarningElements));}
	wxRect cur(x, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(accel,cur,wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	if(modified){dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled()? WindowText : WindowTextInactive));}
}

//void ItemHotkey::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed)
//{
//	if(event.LeftDClick()){
//		int inum=theList->GetSelection();
//		if(inum<0){return;}
//		OnMapHotkey(theList, inum);
//	}
//}

void ItemHotkey::OnMapHotkey(KaiListCtrl *theList, int y)
{
	HkeysDialog hkd(theList,name, hotkeyId.Type, !name.StartsWith("Script") );
	
	if(hkd.ShowModal()==wxID_OK){
		
		wxString hotkey = accel;
		const idAndType *itype=NULL;
		for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++){
			if(cur->second.Name == name){
				if(itype){delete itype;}
				itype= new idAndType(cur->first.id, hkd.type);
			}

			if(cur->second.Accel == hkd.hotkey && (cur->first.Type == hkd.type) ){
				KaiMessageDialog msg(theList, 
					wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\".\nCo zrobić?"),
					cur->second.Name), _("Uwaga"), wxYES_NO|wxCANCEL);
				msg.SetYesLabel (_("Zamień skróty"));
				msg.SetNoLabel (_("Usuń skrót"));
				int result = msg.ShowModal();
				if(result!=wxCANCEL){
					if(result==wxNO){hotkey="";}
					int nitem =theList->FindItem(0, windowNames[cur->first.Type] + " " + cur->second.Name);
					if(nitem>=0){
						ItemHotkey* item = (ItemHotkey*)theList->CopyRow(nitem, 1);
						item->accel = hotkey;
						item->modified = true;
						theList->Refresh(false);
					}
				}else{ if(itype){delete itype; itype = NULL;} return;}
			}
		}

		if(!itype){return;}
		if(hotkeyId.Type != hkd.type){
			int nitem = theList->FindItem(0, windowNames[hkd.type] + " " + name);
			if(nitem<0){
				ItemHotkey* itemcopied = (ItemHotkey*)theList->CopyRow(y, 1, true);
				ItemText *itemtext = (ItemText*)theList->GetItem(theList->GetCount()-1, 0);
				itemtext->name = windowNames[hkd.type] + " " + name;
				itemcopied->accel = hkd.hotkey;
				itemcopied->modified = true;
				int pos = theList->GetCount();
				theList->ScrollTo(pos);
				theList->SetSelection(pos);
				goto done;
			}
			ItemHotkey* item = (ItemHotkey*)theList->CopyRow(nitem, 1);
			item->accel = hkd.hotkey;
			item->modified = true;
			theList->ScrollTo(nitem+1);
			theList->SetSelection(nitem);
			goto done;
		}
		ItemHotkey* item = (ItemHotkey*)theList->CopyRow(y, 1);
		item->accel = hkd.hotkey;
		item->modified = true;
		theList->Refresh(false);
done:
		delete itype; itype = NULL;
		theList->SetModified(true);
		theList->PushHistory();
	}
}

void ItemHotkey::OnResetHotkey(KaiListCtrl *theList, int y)
{
	const wxString &defKet = Hkeys.GetDefaultKey(hotkeyId);
	ItemHotkey *itemKey = (ItemHotkey*)theList->CopyRow(y,1);
	itemKey->accel = defKet;
	itemKey->modified = true;
	theList->SetModified(true);
	theList->Refresh(false);
	theList->PushHistory();
}
	
void ItemHotkey::OnDeleteHotkey(KaiListCtrl *theList, int y)
{
	ItemHotkey *itemKey = (ItemHotkey*)theList->CopyRow(y,1);
	itemKey->accel = "";
	itemKey->modified = true;
	theList->SetModified(true);
	theList->Refresh(false);
	theList->PushHistory();
}

void ItemHotkey::Save()
{
	if(modified){
		Hkeys.SetHKey(hotkeyId, name, accel);
		modified=false;
	}
}

OptionsDialog::OptionsDialog(wxWindow *parent, kainoteFrame *kaiparent)
	: KaiDialog(parent,-1,_("Opcje"))
{
	OptionsTree= new KaiTreebook(this,-1);
	//OptionsTree->SetForegroundColour(text);
	//OptionsTree->SetBackgroundColour(window);

	Kai=kaiparent;
	Stylelist=NULL;
	Katlist=NULL;

	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("SETTINGS"));
	SetIcon(icn);

	wxWindow *Main= new wxWindow(OptionsTree,-1);
	//Main->SetForegroundColour(Options.GetColour(WindowText));
	wxWindow *Main1= new wxWindow(OptionsTree,-1);
	//Main->SetForegroundColour(Options.GetColour(WindowText));
	wxWindow *ConvOpt= new wxWindow(OptionsTree,-1);
	wxWindow *Hotkeyss= new wxWindow(OptionsTree,-1);
	wxWindow *AudioMain= new wxWindow(OptionsTree,-1);
	wxWindow *Video= new wxWindow(OptionsTree,-1);
	wxWindow *Themes= new wxWindow(OptionsTree,-1);
	wxWindow *SubsProps = new SubtitlesProperties(OptionsTree, this);

	hkeymodif=0;
	if(!Options.AudioOpts && !Options.LoadAudioOpts()){KaiMessageBox(_("Nie można wczytać opcji audio"), _("Błąd"));}

	//Main
	{
		const int optsSize = 15;
		wxBoxSizer *MainSizer=new wxBoxSizer(wxVERTICAL);
		wxString labels[optsSize]={_("Wczytywanie posortowanych napisów"),_("Włącz sprawdzanie pisowni"),
			_("Zaznaczaj linijkę z czasem aktywnej\nlinijki poprzedniej zakładki"),_("Zapisuj napisy z nazwą wideo"),
			_("Pokaż sugestie po dwukrotnym klininięciu na błąd"),_("Otwieraj napisy zawsze w nowej karcie"),
			_("Nie przechodź do następnej linii przy edycji czasów"),
			_("Wyłącz pokazywanie edycji na wideo\n(wymaga ponownego otwarcia zakładek)"),
			_("Włącz szukanie widocznej linii\npo wyjściu z pełnego ekranu"),
			_("Włącz przenoszenie wartości pola przesuwania czasów"),_("Zmieniaj aktywną linię przy zaznaczaniu"),
			_("Pokazuj oryginał w trybie tłumaczenia"), _("Używaj skróty klawiszowe numpada w polach tekstowych"),
			_("Nie ostrzegaj o niezgodności rozdzielczości"),
			_("Kompatybilność ze starymi skryptami Kainote")};
		CONFIG opts[optsSize]={GridLoadSortedSubs,SpellcheckerOn,AutoSelectLinesFromLastTab,SubsAutonaming,
			EditboxSugestionsOnDoubleClick,OpenSubsInNewCard,NoNewLineAfterTimesEdition,
			DisableLiveVideoEditing,SelectVisibleLineAfterFullscreen,MoveTimesLoadSetTabOptions,
			GridChangeActiveOnSelection,TlModeShowOriginal, TextFieldAllowNumpadHotkeys,DontAskForBadResolution,
			AutomationOldScriptsCompatybility};

		wxString langopts[2]={"Polski","English"};
		KaiStaticBoxSizer *langSizer=new KaiStaticBoxSizer(wxVERTICAL, Main, _("Język (wymaga restartu programu)"));
		KaiChoice *lang=new KaiChoice(Main,10000,wxDefaultPosition,wxDefaultSize,2,langopts);
		lang->SetSelection(Options.GetInt(ProgramLanguage));
		lang->SetFocus();
		ConOpt(lang, ProgramLanguage);
		langSizer->Add(lang,0,wxALL|wxEXPAND,2);
		MainSizer->Add(langSizer,0,wxRIGHT|wxEXPAND,5);

		wxArrayString dics;
		SpellChecker::AvailableDics(dics);
		if(dics.size()==0){dics.Add(_("Umieść pliki .dic i .aff do folderu \"Dictionary\""));}
		KaiStaticBoxSizer *dicSizer=new KaiStaticBoxSizer(wxVERTICAL, Main, _("Język sprawdzania pisowni (folder \"Dictionary\")"));

		KaiChoice *dic=new KaiChoice(Main,10001,wxDefaultPosition,wxDefaultSize,dics);

		dic->SetSelection(dic->FindString(Options.GetString(DictionaryLanguage)));
		ConOpt(dic, DictionaryLanguage);
		dicSizer->Add(dic,0,wxALL|wxEXPAND,2);
		MainSizer->Add(dicSizer,0,wxRIGHT|wxEXPAND,5);

		for(int i=0;i<optsSize;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(Main,-1,labels[i]);
			opt->SetValue(Options.GetBool(opts[i]));
			ConOpt(opt,opts[i]);
			MainSizer->Add(opt,0,wxALL,2);
		}
		Main->SetSizerAndFit(MainSizer);

		//Main1

		wxBoxSizer *Main1Sizer=new wxBoxSizer(wxVERTICAL);
		wxFlexGridSizer *MainSizer2=new wxFlexGridSizer(8,2,wxSize(5,5));
		//uwaga id 20000 ma tylko numctrl, pola tekstowe musza mieć inny id
		NumCtrl *gridSaveAfter = new NumCtrl(Main1, 20000, Options.GetString(GridSaveAfterCharacterCount), 0, 10000,true, wxDefaultPosition, wxSize(120,-1), wxTE_PROCESS_ENTER);
		gridSaveAfter->SetToolTip(_("Zero całkowicie wyłacza zapis przy edycji"));
		NumCtrl *autoSaveMax = new NumCtrl(Main1, 20000, Options.GetString(AutoSaveMaxFiles), 2, 1000000,true, wxDefaultPosition, wxSize(120,-1), wxTE_PROCESS_ENTER);
		autoSaveMax->SetToolTip(_("Liczbę plików autozapisu można ustawić od 2 do 1000000"));
		NumCtrl *ltl = new NumCtrl(Main1, 20000, Options.GetString(AutomationTraceLevel), 0, 5,true, wxDefaultPosition, wxSize(120,-1), wxTE_PROCESS_ENTER);
		NumCtrl *sc = new NumCtrl(Main1, 20000, Options.GetString(InsertStartOffset), -100000, 100000,true, wxDefaultPosition, wxSize(120,-1), wxTE_PROCESS_ENTER);
		NumCtrl *sc1 = new NumCtrl(Main1, 20000, Options.GetString(InsertEndOffset), -100000, 100000,true, wxDefaultPosition, wxSize(120,-1), wxTE_PROCESS_ENTER);
		KaiTextCtrl *sc2 = new KaiTextCtrl(Main1, 22001, Options.GetString(GridTagsSwapChar), wxDefaultPosition, wxSize(120,-1), wxTE_PROCESS_ENTER);
		NumCtrl *sc3 = new NumCtrl(Main1, 20000, Options.GetString(EditboxTagButtons), 0, 9,true, wxDefaultPosition, wxSize(120,-1), wxTE_PROCESS_ENTER);
		
		ConOpt(gridSaveAfter, GridSaveAfterCharacterCount);
		ConOpt(autoSaveMax, AutoSaveMaxFiles);
		ConOpt(ltl, AutomationTraceLevel);
		ConOpt(sc,InsertStartOffset);
		ConOpt(sc1,InsertEndOffset);
		ConOpt(sc2,GridTagsSwapChar);
		ConOpt(sc3,EditboxTagButtons);

		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Ilość edycji do zapisu"),wxDefaultPosition, wxSize(240,-1)),3,wxALIGN_CENTRE_VERTICAL|wxEXPAND);
		MainSizer2->Add(gridSaveAfter,1,wxEXPAND);
		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Maksymalna ilość plików autozapisu"),wxDefaultPosition, wxSize(240,-1)),3,wxALIGN_CENTRE_VERTICAL|wxEXPAND);
		MainSizer2->Add(autoSaveMax,1,wxEXPAND);
		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Opóźnienie klatek początkowych w ms:"),wxDefaultPosition, wxSize(240,-1)),3,wxALIGN_CENTRE_VERTICAL|wxEXPAND);
		MainSizer2->Add(sc,1,wxEXPAND);
		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Opóźnienie klatek końcowych w ms:")),3,wxALIGN_CENTRE_VERTICAL|wxEXPAND);
		MainSizer2->Add(sc1,1,wxEXPAND);
		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Znak podmiany tagów ASS:")),3,wxALIGN_CENTRE_VERTICAL|wxEXPAND);
		MainSizer2->Add(sc2,1,wxEXPAND);
		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Ilość przycisków wstawiających tagi ASS:")),3,wxALIGN_CENTRE_VERTICAL|wxEXPAND);
		MainSizer2->Add(sc3,1,wxEXPAND);
		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Poziom śledzenia logów skryptów LUA")),3,wxALIGN_CENTRE_VERTICAL|wxEXPAND);
		MainSizer2->Add(ltl,1,wxEXPAND);

		//MainSizer->Add(MainSizer2,0,wxLEFT|wxTOP,2);

		FontPickerButton *optf=new FontPickerButton(Main1,-1,wxFont(Options.GetInt(GridFontSize),wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,Options.GetString(GridFontName)));
		ConOpt(optf,GridFontName);
		MainSizer2->Add(new KaiStaticText(Main1,-1,_("Czcionka pola napisów:")),3,wxRIGHT| wxALIGN_CENTRE_VERTICAL|wxEXPAND,10);
		MainSizer2->Add(optf,1,wxEXPAND);

		KaiStaticBoxSizer *alm=new KaiStaticBoxSizer(wxHORIZONTAL,Main1,_("Sposób wczytywania skryptów autoload"));
		wxString methods[] = {_("Przy starcie programu asynchronicznie"),_("Przy starcie programu"),_("Przy otwarciu menu asynchronicznie"),_("Przy otwarciu menu")};
		KaiChoice *cmb = new KaiChoice(Main1, 10000, wxDefaultPosition, wxSize(200,-1), 4, methods, wxTE_PROCESS_ENTER);
		cmb->SetSelection(Options.GetInt(AutomationLoadingMethod));
		ConOpt(cmb,AutomationLoadingMethod);
		alm->Add(cmb, 1, wxCENTER|wxEXPAND|wxALL,2);
		Main1Sizer->Add(MainSizer2,0,wxRIGHT|wxEXPAND,5);
		Main1Sizer->Add(alm,0,wxRIGHT|wxEXPAND,5);

		Main1->SetSizerAndFit(Main1Sizer);
	}

	//Ustawienia konwersji
	{
		wxBoxSizer *ConvOptSizer1=new wxBoxSizer(wxVERTICAL);

		KaiStaticBoxSizer *obr=new KaiStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Wybierz katalog"));
		KaiStaticBoxSizer *obr0=new KaiStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Wybierz styl"));
		KaiStaticBoxSizer *obr1=new KaiStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Wybierz FPS"));
		KaiStaticBoxSizer *obr2=new KaiStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Czas w ms na jedną literę"));
		KaiStaticBoxSizer *obr3=new KaiStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Tagi wstawiane na początku każdej linijki ass"));
		KaiStaticBoxSizer *obr4=new KaiStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Rozdzielczość przy konwersji na ASS"));
		wxArrayString styles;
		wxArrayString fpsy;



		fpsy.Add("23.976");fpsy.Add("24");fpsy.Add("25");fpsy.Add("29.97");fpsy.Add("30");fpsy.Add("60");

		for(int i = 0;i<2;i++){
			wxString optname=(i==0)? Options.GetString(ConvertStyleCatalog) : Options.GetString(ConvertStyle);
			if(i!=0){
				for(int i = 0; i<Options.StoreSize();i++){
					styles.Add(Options.GetStyle(i)->Name);
				}
			}
			KaiChoice *cmb = new KaiChoice(ConvOpt, (i==0)?28888 : 28889, wxDefaultPosition, wxSize(200,-1), (i==0)?Options.dirs : styles, wxTE_PROCESS_ENTER);

			int sel=cmb->FindString(optname);

			if(sel>=0){cmb->SetSelection(sel);if(i==0 && Options.actualStyleDir!=optname){Options.LoadStyles(optname);}}
			else{
				if(i==0){sel=cmb->FindString(Options.actualStyleDir);}
				cmb->SetSelection(MAX(0,sel));
				wxString co=(i==0)?_("katalog dla stylu") : _("styl"); 
				KaiMessageBox(wxString::Format(_("Wybrany %s konwersji nie istnieje,\nzostanie zmieniony na domyślny"), co),_("Uwaga"));}

			ConOpt(cmb,(i==0)? ConvertStyleCatalog : ConvertStyle);
			if(i==0){
				obr->Add(cmb,1,wxCENTER|wxALL,2);
				ConvOptSizer1->Add(obr,0,wxRIGHT|wxEXPAND,5);
				Katlist=cmb;
				Connect(28888,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OptionsDialog::OnChangeCatalog);
			}
			else{
				obr0->Add(cmb,1,wxCENTER|wxALL,2);
				ConvOptSizer1->Add(obr0,0,wxRIGHT|wxEXPAND,5);
				Stylelist=cmb;
			}
		}
		wxString convFPS = Options.GetString(ConvertFPS);
		KaiChoice *cmb = new KaiChoice(ConvOpt, -1, convFPS, wxDefaultPosition, wxSize(200,-1), fpsy, wxTE_PROCESS_ENTER);
		int sel=cmb->FindString(convFPS);
		if(sel>=0){cmb->SetSelection(sel);}
		else{cmb->SetValue(convFPS);}

		ConOpt(cmb,ConvertFPS);
		obr1->Add(cmb,1,wxCENTER|wxALL,2);
		ConvOptSizer1->Add(obr1,0,wxRIGHT|wxEXPAND,5);


		for(int i=0;i<3;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(ConvOpt,-1,(i==0)?_("FPS z wideo"):(i==1)?_("Nowe czasy końcowe"):_("Pokaż okno przed konwersją"));
			CONFIG optname = (i==0)? ConvertFPSFromVideo : (i==1)? ConvertNewEndTimes : ConvertShowSettings;
			opt->SetValue(Options.GetBool(optname));
			ConOpt(opt,optname);
			ConvOptSizer1->Add(opt,0,wxRIGHT|wxEXPAND,5);
		}

		NumCtrl *sc = new NumCtrl(ConvOpt, 20000, Options.GetString(ConvertTimePerLetter), 30, 1000, true, wxDefaultPosition, wxSize(250,-1), wxTE_PROCESS_ENTER);
		ConOpt(sc,ConvertTimePerLetter);
		obr2->Add(sc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		ConvOptSizer1->Add(obr2,0,wxRIGHT|wxEXPAND,5);

		sc = new NumCtrl(ConvOpt, 20000, Options.GetString(ConvertResolutionWidth), 1, 3000, true, wxDefaultPosition, wxSize(115,-1), wxTE_PROCESS_ENTER);
		ConOpt(sc,ConvertResolutionWidth);
		obr4->Add(sc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);

		KaiStaticText* txt= new KaiStaticText(ConvOpt,-1," X ");
		obr4->Add(txt,0,wxTOP,5);

		sc = new NumCtrl(ConvOpt, 20000, Options.GetString(ConvertResolutionHeight), 1, 3000, true, wxDefaultPosition, wxSize(115,-1), wxTE_PROCESS_ENTER);
		ConOpt(sc,ConvertResolutionHeight);
		obr4->Add(sc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		ConvOptSizer1->Add(obr4,0,wxRIGHT|wxEXPAND,5);

		KaiTextCtrl *tc = new KaiTextCtrl(ConvOpt, -1, Options.GetString(ConvertASSTagsOnLineStart), wxDefaultPosition, wxSize(250,-1),wxTE_PROCESS_ENTER);
		ConOpt(tc,ConvertASSTagsOnLineStart);
		obr3->Add(tc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		ConvOptSizer1->Add(obr3,0,wxRIGHT|wxEXPAND,5);

		ConvOpt->SetSizerAndFit(ConvOptSizer1);
	}
	//Video
	{
		wxString voptspl[]={_("Otwórz wideo z menu kontekstowego na pełnym ekranie"),_("Lewy przycisk myszy pauzuje wideo"),
			_("Otwieraj wideo z czasem aktywnej linii"),_("Preferowane ścieżki audio (oddzielone średnikiem)"),
			_("Sposób szukania wideo w FFMS2 (wymaga ponownego wczytania)")};
		CONFIG vopts[]={VideoFullskreenOnStart,VideoPauseOnClick,OpenVideoAtActiveLine,
			AcceptedAudioStream,FFMS2VideoSeeking};
		wxBoxSizer *MainSizer=new wxBoxSizer(wxVERTICAL);
		for(int i=0;i<3;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(Video,-1,voptspl[i]);
			opt->SetValue(Options.GetBool(vopts[i]));
			ConOpt(opt,vopts[i]);
			MainSizer->Add(opt,0,wxALL,2);
		}
		KaiStaticBoxSizer *prefaudio=new KaiStaticBoxSizer(wxHORIZONTAL,Video,voptspl[3]);
		KaiTextCtrl *tc = new KaiTextCtrl(Video, -1, Options.GetString(vopts[3]), wxDefaultPosition, wxSize(250,-1),wxTE_PROCESS_ENTER);
		ConOpt(tc,vopts[3]);
		prefaudio->Add(tc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		MainSizer->Add(prefaudio,0,wxRIGHT|wxEXPAND,5);
		KaiStaticBoxSizer *seekingsizer = new KaiStaticBoxSizer(wxHORIZONTAL,Video, voptspl[4]);
		wxString seekingOpts[] = {_("Liniowe"), _("Normalne"), _("Niebezpieczne (szybkie w każdym przypadku)"), _("Agresywne (szybkie przy cofaniu)")};
		KaiChoice *sopts = new KaiChoice(Video, 10000, wxDefaultPosition, wxSize(200,-1), 4, seekingOpts, wxTE_PROCESS_ENTER);
		sopts->SetSelection(Options.GetInt(vopts[4]));
		seekingsizer->Add(sopts, 1, wxALL|wxALIGN_CENTER|wxEXPAND,2);
		MainSizer->Add(seekingsizer,0, wxRIGHT|wxEXPAND, 5);
		ConOpt(sopts,vopts[4]);
		Video->SetSizerAndFit(MainSizer);
	}
	//Hotkeys
	{
		wxBoxSizer *HkeysSizer=new wxBoxSizer(wxVERTICAL);
		Shortcuts = new KaiListCtrl(Hotkeyss,26667, wxDefaultPosition);
		Shortcuts->InsertColumn(0,_("Funkcja"),TYPE_TEXT,260);
		Shortcuts->InsertColumn(1,_("Skrót"),TYPE_TEXT,80);
		Connect(26667,LIST_ITEM_DOUBLECLICKED,(wxObjectEventFunction)&OptionsDialog::OnMapHkey);
		//Connect(26667,LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnResetHkey);

		if(!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){KaiMessageBox(_("Nie można wczytać skrótów klawiszowych audio"), _("Błąd"));}

		std::map<idAndType, hdata> _hkeys;
		Hkeys.LoadDefault(_hkeys);
		Hkeys.LoadDefault(_hkeys,true);
		Notebook::GetTab()->Video->ContextMenu(wxPoint(0,0),true);
		Notebook::GetTab()->Grid->ContextMenu(wxPoint(0,0),true);

		long ii=0;

		for (auto cur = Hkeys.hkeys.begin();cur != Hkeys.hkeys.end();cur++) {
			if(cur->second.Name==""){
				auto tmpkey = _hkeys.find(cur->first);
				if(tmpkey!=_hkeys.end()){
					cur->second.Name = tmpkey->second.Name;
				}else{
					for(int window=0; window<5; window++){
						if(window == cur->first.Type){
							continue;
						}else{
							auto tmpkey = _hkeys.find(idAndType(cur->first.id, window));
							if(tmpkey!=_hkeys.end()){
								cur->second.Name = tmpkey->second.Name;
								break;
							}
						}
					}
				}

			}

			wxString name= windowNames[cur->first.Type]+" "+cur->second.Name;
			long pos = Shortcuts->AppendItem(new ItemText(name));
			Shortcuts->SetItem(pos,1,new ItemHotkey(cur->second.Name, cur->second.Accel, cur->first));
			ii++;
		}
		Shortcuts->StartEdition();
		Shortcuts->SetSelection(0);

		HkeysSizer->Add(Shortcuts,1,wxALL|wxEXPAND,2);
		wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
		MappedButton *setHotkey = new MappedButton(Hotkeyss, 23232,_("Mapuj skrót"));
		Connect(23232,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnMapHkey);
		MappedButton *resetHotkey = new MappedButton(Hotkeyss, 23231,_("Przywróć skrót domyślny"));
		Connect(23231,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnResetHkey);
		MappedButton *deleteHotkey = new MappedButton(Hotkeyss, 23230,_("Usuń skrót"));
		Connect(23230,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnDeleteHkey);
		buttonsSizer->Add(setHotkey,0,wxALL,2);
		buttonsSizer->Add(resetHotkey,0,wxALL,2);
		buttonsSizer->Add(deleteHotkey,0,wxALL,2);
		HkeysSizer->Add(buttonsSizer,0,wxALL|wxALIGN_CENTER,2);
		Hotkeyss->SetSizerAndFit(HkeysSizer);
		ConOpt(Shortcuts,(CONFIG)2000);
	}

	//Audio main

	{
		wxBoxSizer *audio=new wxBoxSizer(wxVERTICAL);

		wxString names[]={_("Wyświetlaj czas przy kursorze"),_("Wyświetlaj znaczniki sekund"),_("Wyświetlaj tło zaznaczenia"),_("Wyświetlaj pozycję wideo"),
			_("Wyświetlaj klatki kluczowe"),_("Przewijaj wykres audio przy odtwarzaniu"), _("Aktywuj okno audio po najechaniu"), _("Przyklejaj do klatek kluczowych"),
			_("Przyklejaj do pozostałych linii"),_("Scalaj wszystkie \"n\" z poprzednią sylabą"),_("Przenoś linie sylab po kliknięciu"),_("Wczytuj audio do pamięci RAM")};

		CONFIG opts[]={AudioDrawTimeCursor,AudioDrawSecondaryLines,AudioDrawSelectionBackground,AudioDrawVideoPosition,
			AudioDrawKeyframes,AudioLockScrollOnCursor,AudioAutoFocus,AudioSnapToKeyframes,AudioSnapToOtherLines,
			AudioMergeEveryNWithSyllable,AudioKaraokeMoveOnClick,AudioRAMCache};

		for(int i=0;i<12;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(AudioMain,-1,names[i]);
			opt->SetValue(Options.GetBool(opts[i]));
			ConOpt(opt,opts[i]);
			audio->Add(opt,0,wxALL,2);
		}

		CONFIG opts1[4]={AudioDelay, AudioMarkPlayTime, AudioInactiveLinesDisplayMode,AudioLineBoundariesThickness};
		NumCtrl *Delay = new NumCtrl(AudioMain, 20000, Options.GetString(opts1[0]), -50000000, 50000000, true, wxDefaultPosition, wxSize(300,-1), 0);
		NumCtrl *sc = new NumCtrl(AudioMain, 20000, Options.GetString(opts1[1]), 400, 5000, true, wxDefaultPosition, wxSize(300,-1), 0);
		NumCtrl *lineThickness = new NumCtrl(AudioMain, 20000, Options.GetString(opts1[3]), 1, 5, true, wxDefaultPosition, wxSize(300,-1), 0);
		wxString inact[3]={_("Brak"),_("Przed i po aktywnej"),_("Wszystkie widoczne")};
		KaiChoice *sc1 = new KaiChoice(AudioMain, 10000, wxDefaultPosition, wxSize(300,-1), 3, inact);
		sc1->SetSelection(Options.GetInt(opts1[2]));
		ConOpt(Delay,opts1[0]);
		ConOpt(sc,opts1[1]);
		ConOpt(lineThickness,opts1[3]);
		ConOpt(sc1,opts1[2]);
		KaiStaticBoxSizer *DelaySizer=new KaiStaticBoxSizer(wxVERTICAL,AudioMain,_("Opóźnienie audio w ms"));
		KaiStaticBoxSizer *audiocols=new KaiStaticBoxSizer(wxVERTICAL,AudioMain,_("Czas odtwarzania audio przed i po znaczniku w ms"));	
		KaiStaticBoxSizer *lineThicknessSizer=new KaiStaticBoxSizer(wxVERTICAL,AudioMain,_("Grubość linii znaczników"));
		KaiStaticBoxSizer *audiocols1=new KaiStaticBoxSizer(wxVERTICAL,AudioMain,_("Sposób wyświetlania nieaktywnych linijek"));	
		DelaySizer->Add(Delay,1,wxALL|wxEXPAND,2);
		audiocols->Add(sc,1,wxALL|wxEXPAND,2);
		lineThicknessSizer->Add(lineThickness,1,wxALL|wxEXPAND,2);
		audiocols1->Add(sc1,1,wxALL|wxEXPAND,2);
		audio->Add(DelaySizer,0,wxRIGHT|wxEXPAND,5);
		audio->Add(audiocols,0,wxRIGHT|wxEXPAND,5);
		audio->Add(lineThicknessSizer,0,wxRIGHT|wxEXPAND,5);
		audio->Add(audiocols1,0,wxRIGHT|wxEXPAND,5);


		AudioMain->SetSizerAndFit(audio);
	}

	//Themes
	{
		const int numColors = 126;
		wxString labels[numColors]={
			//okno
			_("Okno tło"),_("Okno nieaktywne tło"),_("Okno tekst"),_("Okno nieaktywny tekst"),
			_("Okno obramowanie"), _("Okno obramowanie nieaktywne"), _("Okno tło obramowania"), 
			_("Okno tło obramowania nieaktywne"), _("Okno tekst nagłówka"), _("Okno tekst nagłówka nieaktywny"), 
			_("Okno najechany element nagłówka"),_("Okno wciśnięty element nagłówka"),
			_("Okno najechane zamykanie"),_("Okno wciśnięte zamykanie"),_("Okno elementy ostrzegające"),
			//napisy
			_("Napisy tekst"),_("Napisy tło"),_("Napisy tło dialogów"),_("Napisy tło komentarzy"),
			_("Napisy zaznaczenia (przezroczystość)"),_("Napisy linijki widoczne na wideo"),
			_("Napisy kolidujące linie"),_("Napisy obramowanie linijki"),_("Napisy obramowanie aktywnej linijki"),
			_("Napisy nagłówek"),_("Napisy tekst nagłówka"),
			_("Napisy etykieta"),_("Napisy etykieta zmodyfikowanej linii"),_("Napisy etykieta zapisanej linii"),
			_("Napisy etykieta niepewnej linii"),_("Napisy tło błędów pisowni"),_("Napisy obramowanie porównania"),
			_("Napisy tło porównania brak zgodności"),_("Napisy tło porównania zgodność"),_("Napisy tło komentarza por. brak zgodności"),
			_("Napisy tło komentarza por. zgodność"),
			//edytor
			_("Edytor tekst"),_("Edytor nazwy tagów"),_("Edytor wartości tagów"),
			_("Edytor nawiasy klamrowe"),_("Edytor operatory tagów"),_("Edytor tło nawiasów"),_("Edytor tło"),
			_("Edytor zaznaczenie"),_("Edytor zaznaczenie w nieaktywnym oknie"),
			_("Edytor obramowanie"),_("Edytor obramowanie aktywnego okna"),_("Edytor błędy pisowni"),
			//audio
			_("Audio tło"),_("Audio znacznik start"),_("Audio znacznik koniec"),_("Audio znacznik przesuwania czasów"),
			_("Audio znaczniki nieaktywnej linijki"),_("Audio kursor"),_("Audio znaczniki sekund"),_("Audio klatki kluczowe"),
			_("Audio znaczniki sylab"),_("Audio tekst sylab"),_("Audio zaznaczenie"),
			_("Audio zaznaczenie po modyfikacji"),_("Audio tło nieaktywnych linijek"),_("Audio wykres falowy"),
			_("Audio nieaktywny wykres falowy"),_("Audio zmodyfikowany wykres falowy"),_("Audio zaznaczony wykres falowy"),
			_("Audio tło spektrum"), _("Audio echo spektrum"), _("Audio spektrum"),
			//kontrolki
			_("Pole tekstowe tło"), _("Pole tekstowe obramowanie"), 
			_("Pole tekstowe obramowanie aktywnego okna"), _("Pole tekstowe zaznaczenie"),
			_("Pole tekstowe zaznaczenie w nieaktywnym oknie"),
			_("Przycisk i lista tło"), _("Przycisk i lista tło po najechaniu"), 
			_("Przycisk i lista tło po wciśnięciu"),_("Przycisk i lista tło aktywnego"),
			_("Przycisk i lista obramowanie"),_("Przycisk i lista obramowanie po najechaniu"),
			_("Przycisk i lista obramowanie po wciśnięciu"),_("Przycisk i lista obramowanie aktywnego"),
			_("Przycisk i lista obramowanie nieaktywne"),_("Przełącznik tło włączonego"),
			_("Przełącznik obramowanie włączonego"),_("Pasek przewijania tło"),_("Pasek przewijania suwak"),
			_("Pasek przewijania suwak po najechaniu"),_("Pasek przewijania suwak po wciśnięciu"),
			_("Ramka z opisem obramowanie"),_("Lista statyczna obramowanie"),_("Lista statyczna tło"),
			_("Lista statyczna zaznaczenie"),_("Lista statyczna tło nagłówka"),_("Lista statyczna tekst nagłówka"),
			_("Pasek statusu obramowanie"),
			//pasek menu
			_("Pasek menu tło 1"),_("Pasek menu tło 2"),_("Pasek menu obramowanie zaznaczenia"),
			_("Pasek menu tło zaznaczenia"),_("Menu tło"),_("Menu obramowanie zaznaczenia"),
			_("Menu tło zaznaczenia"),
			//zakładki
			_("Pasek zakładek tło 1"), _("Pasek zakładek tło 2"), _("Zakładki obramowanie aktywnej"), 
			_("Zakładki obramowanie nieaktywnej"),_("Zakładki tło aktywnej"), _("Zakładki tło nieaktywnej"), 
			_("Zakładki tło nieaktywnej po najechaniu"), _("Zakładki tło drugiej widocznej zakładki"),
			_("Zakładki tekst aktywnej"), _("Zakładki tekst nieaktywnej"), _("Zakładki zamknięcie po najechaniu"), 
			_("Pasek zakładek strzałka"), _("Pasek zakładek strzałka tło"), 
			_("Pasek zakładek strzałka tło po najechaniu"),
			//suwak
			_("Suwak ścieżka tło"), _("Suwak ścieżka obramowanie"), _("Suwak obramowanie"), 
			_("Suwak obramowanie po najechaniu"), _("Suwak obramowanie po wciśnięciu"), _("Suwak tło"), 
			_("Suwak tło po najechaniu"), _("Suwak tło po wciśnięciu"),
			//podgląd styli
			_("Pierwszy kolor podglądu styli"), _("Drugi kolor podglądu styli")
		};
		

		wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
		wxArrayString choices;
		wxArrayString files;
		wxString pathwn = Options.pathfull + "\\Themes\\";
		wxString programTheme = Options.GetString(ProgramTheme);
		wxDir kat(pathwn);
		if(kat.IsOpened()){
			kat.GetAllFiles(pathwn,&files,"*.txt", wxDIR_FILES);
		}
		for (size_t i = 0; i < files.size(); i++){
			choices.Add(files[i].AfterLast('\\').BeforeLast('.'));
		}
		if(choices.Index("DeepDark",false) == -1){
			choices.Insert("DeepDark",0);
		}
		if(choices.Index("DeepLight",false) == -1){
			choices.Insert("DeepLight",1);
		}
		KaiChoice *themeList = new KaiChoice(Themes,14567,wxDefaultPosition, wxDefaultSize, choices);
		themeList->SetSelection(themeList->FindString(programTheme));

		KaiTextCtrl *newTheme = new KaiTextCtrl(Themes,-1,"");
		MappedButton *copyTheme = new MappedButton(Themes,14566,_("Kopiuj"));



		sizer->Add(themeList, 0, wxALL|wxEXPAND, 2);
		sizer1->Add(newTheme, 1, wxRIGHT|wxTOP|wxBOTTOM|wxEXPAND, 2);
		sizer1->Add(copyTheme, 0, wxLEFT|wxTOP|wxBOTTOM, 2);
		sizer->Add(sizer1, 0, wxALL|wxEXPAND, 2);

		KaiListCtrl *List = new KaiListCtrl(Themes, -1, wxDefaultPosition, wxSize(300, -1));
		List->InsertColumn(0, _("Nazwa"), TYPE_TEXT, 240);
		List->InsertColumn(1, _("Kolor"), TYPE_COLOR, 150);
		for(int i=0;i<numColors;i++)
		{
			int row = List->AppendItem(new ItemText(labels[i]));
			AssColor col = Options.GetColor((COLOR)(i+1));
			List->SetItem(row, 1, new ItemColor(col, i+1));
		}
		sizer->Add(List, 1, wxALL|wxEXPAND, 2);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED,[=](wxCommandEvent &evt){
			wxString themeName = newTheme->GetValue();
			if(themeName.IsEmpty() || choices.Index(themeName, false) != -1){wxBell(); return;}
			wxString originalName = themeList->GetString(themeList->GetSelection());
			wxString dir = Options.pathfull + "\\Themes\\";
			wxString copyPath = dir + themeName + ".txt";
			if(originalName == "DeepDark" || originalName == "DeepLight"){
				Options.SaveColors(copyPath);
				List->Enable(true);
				List->Refresh(false);
			}else{

				if(!wxDirExists(dir)){
					wxBell(); return;
				}
				wxString originalPath = dir + originalName + ".txt";
				wxCopyFile(originalPath, copyPath, false);
			}
			Options.SetString(ProgramTheme, themeName);
			if(!List->IsEnabled()){List->Enable(false);}
			newTheme->SetValue("");
			int size = themeList->Append(themeName);
			themeList->SetSelection(size);
		},14566);
		Bind(wxEVT_COMMAND_CHOICE_SELECTED,[=](wxCommandEvent &evt){
			wxString themeName = themeList->GetString(themeList->GetSelection());
			if(themeName.IsEmpty()){return;}
			Options.LoadColors(themeName);
			for(int i=0;i<numColors;i++)
			{
				ItemColor *item = (ItemColor*)List->GetItem(i, 1);
				item->col = Options.GetColor((COLOR)item->colOptNum);
			}
			ChangeColors();
			List->Enable(themeName != "DeepDark" && themeName != "DeepLight");
		},14567);
		if(programTheme == "DeepDark" || programTheme == "DeepLight"){List->Enable(false);}
		Themes->SetSizerAndFit(sizer);
		List->StartEdition();
		List->SetSelection(0);
		ConOpt(List,(CONFIG)1000);
	}

	//Adding pages
	OptionsTree->AddPage(Main,_("Edytor"));
	OptionsTree->AddSubPage(ConvOpt,_("Konwersja"));
	OptionsTree->AddPage(Main1,_("Edytor1"));
	OptionsTree->AddPage(Video,_("Wideo"));
	OptionsTree->AddPage(AudioMain,_("Audio"));
	OptionsTree->AddPage(Themes,_("Motywy"));
	OptionsTree->AddPage(Hotkeyss,_("Skróty klawiszowe"));
	OptionsTree->AddPage(SubsProps,_("Właściwości Napisów"));
	OptionsTree->Fit();

	//adding buttons
	wxBoxSizer *ButtonsSizer=new wxBoxSizer(wxHORIZONTAL);

	okok=new MappedButton(this,wxID_OK,"OK");
	MappedButton *oknow=new MappedButton(this,ID_BCOMMIT,_("Zastosuj"));
	MappedButton *cancel=new MappedButton(this,wxID_CANCEL,_("Anuluj"));

	ButtonsSizer->Add(okok,1,wxRIGHT,2);
	ButtonsSizer->Add(oknow,1,wxRIGHT,2);
	ButtonsSizer->Add(cancel,1,wxRIGHT,2);

	DialogSizer *TreeSizer=new DialogSizer(wxVERTICAL);
	TreeSizer->Add(OptionsTree,0,wxALL,2);
	TreeSizer->Add(ButtonsSizer,0,wxBOTTOM|wxALIGN_CENTER,4);
	SetSizerAndFit(TreeSizer);

	CenterOnParent();

	Connect(wxID_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnSaveClick);
	Connect(ID_BCOMMIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnSaveClick);

}

OptionsDialog::~OptionsDialog()
{
	/*if(GetReturnCode ()==wxID_OK){
		SetOptions();
		if(hkeymodif==1){Hkeys.SaveHkeys();Kai->SetAccels();}
		else if(hkeymodif==2){
			Hkeys.SaveHkeys(true);
			if(Kai->GetTab()->Edit->ABox){Kai->GetTab()->Edit->ABox->SetAccels();}
		}
	}*/
	handles.clear();
}

void OptionsDialog::ConOpt(wxWindow *ctrl,CONFIG option)
{
	OptionsBind Obind;
	Obind.ctrl=ctrl;
	Obind.option=option;
	handles.push_back(Obind);
}

void OptionsDialog::OnSaveClick(wxCommandEvent& event)
{
	SetOptions(false);
	/*if(hkeymodif==1){Hkeys.SaveHkeys();Kai->SetAccels();}
	else if(hkeymodif==2){
		Hkeys.SaveHkeys(true);
		if(Kai->GetTab()->Edit->ABox){Kai->GetTab()->Edit->ABox->SetAccels();}
	}*/
	if(event.GetId()==wxID_OK){Hide();}
}

void OptionsDialog::SetOptions(bool saveall)
{
	bool fontmod=false;
	bool colmod=false;
	for(size_t i = 0; i<handles.size(); i++)
	{
		OptionsBind OB=handles[i];

		if(OB.ctrl->IsKindOf(CLASSINFO(KaiCheckBox))){
			KaiCheckBox *cb=(KaiCheckBox*)OB.ctrl;
			if(Options.GetBool(OB.option)!=cb->GetValue()){
				Options.SetBool(OB.option,cb->GetValue());
				if(OB.option == SpellcheckerOn){
					for(size_t i = 0; i< Kai->Tabs->Size();i++){
						Kai->Tabs->Page(i)->Grid->SpellErrors.clear();
					}
					Kai->Tabs->GetTab()->Grid->Refresh(false);
					Kai->Tabs->GetTab()->Edit->TextEdit->SpellcheckerOnOff();
				}
			}
		}
		else if(OB.ctrl->IsKindOf(CLASSINFO(FontPickerButton))){
			FontPickerButton *fpc=(FontPickerButton*)OB.ctrl;
			wxFont font=fpc->GetSelectedFont();
			wxString fontname=font.GetFaceName();
			int fontsize=font.GetPointSize();
			if(Options.GetString(OB.option)!=fontname){
				Options.SetString(OB.option,fontname);fontmod=true;
			}
			if(Options.GetInt(GridFontSize)!=fontsize){
				Options.SetInt(GridFontSize,fontsize);fontmod=true;
			}
		}
		else if(OB.ctrl->IsKindOf(CLASSINFO(KaiChoice))){
			KaiChoice *cbx=(KaiChoice*)OB.ctrl;
			if(cbx->GetWindowStyle()&KAI_COMBO_BOX){
				wxString kol=cbx->GetValue();
				if(Options.GetString(OB.option)!=kol){
					Options.SetString(OB.option,kol);
				}
			}else if(cbx->GetId()!=10000){
				wxString kol=cbx->GetString(cbx->GetSelection());
				if(Options.GetString(OB.option)!=kol){
					Options.SetString(OB.option,kol);
					if(cbx->GetId()==10001){
						SpellChecker::Destroy();
						colmod=true;
						Kai->GetTab()->Grid->SpellErrors.clear();
					}
				}
			}else{
				if(Options.GetInt(OB.option)!=cbx->GetSelection()){
					Options.SetInt(OB.option,cbx->GetSelection());
				}
			}
		}else if(OB.ctrl->IsKindOf(CLASSINFO(KaiTextCtrl))){

			if(OB.ctrl->GetId()!=20000){
				KaiTextCtrl *sc=(KaiTextCtrl*)OB.ctrl;
				wxString str=sc->GetValue();
				if(Options.GetString(OB.option)!=str){
					Options.SetString(OB.option,str);
				}
				if(sc->GetId() == 22001){
					colmod=true;
				}
			}else{
				NumCtrl *sc=(NumCtrl*)OB.ctrl;
				int num=sc->GetInt();
				if(Options.GetInt(OB.option)!=num){
					Options.SetInt(OB.option,num);
				}
			}
		}else if(OB.ctrl->IsKindOf(CLASSINFO(KaiListCtrl))){
			KaiListCtrl *list = (KaiListCtrl*)OB.ctrl;
			if(list->GetModified()){
				list->SaveAll(1);
				Options.SaveColors();
				if(OB.option == 1000){
					ChangeColors();
				}else{
					Hkeys.SaveHkeys();Kai->SetAccels();
					Hkeys.SaveHkeys(true);
					Notebook *tabs = Kai->Tabs; 
					for(size_t j = 0; j < tabs->Size(); j++){
						TabPanel *tab = tabs->Page(j);
						if(tab->Edit->ABox){tab->Edit->ABox->SetAccels();}
					}
				}
			}
		}
	}
	if(fontmod){
		Kai->GetTab()->Grid->SetStyle();
		Kai->GetTab()->Grid->RefreshColumns();
		if(Kai->Tabs->split){
			Kai->Tabs->GetSecondPage()->Grid->SetStyle();
			Kai->Tabs->GetSecondPage()->Grid->RefreshColumns();
		}
	}
	if(colmod){
		Kai->GetTab()->Grid->Refresh(false);
		if(Kai->GetTab()->Edit->ABox){Kai->GetTab()->Edit->ABox->audioDisplay->ChangeColours();}
		if(Kai->Tabs->split){
			Kai->Tabs->GetSecondPage()->Grid->Refresh(false);
			if(Kai->Tabs->GetSecondPage()->Edit->ABox){Kai->Tabs->GetSecondPage()->Edit->ABox->audioDisplay->ChangeColours();}
		}
	}
	Options.SaveOptions();
	Options.SaveAudioOpts();
}

void OptionsDialog::OnMapHkey(wxCommandEvent& event)
{
	int inum=Shortcuts->GetSelection();
	if(inum<0){return;}
	ItemHotkey * item = (ItemHotkey *)Shortcuts->GetItem(inum,1);
	if(item){item->OnMapHotkey(Shortcuts,inum);}
}

void OptionsDialog::OnResetHkey(wxCommandEvent& event)
{
	int inum=Shortcuts->GetSelection();
	if(inum<0){return;}
	ItemHotkey * item = (ItemHotkey *)Shortcuts->GetItem(inum,1);
	if(item){item->OnResetHotkey(Shortcuts,inum);}
}

void OptionsDialog::OnDeleteHkey(wxCommandEvent& event)
{
	int inum=Shortcuts->GetSelection();
	if(inum<0){return;}
	ItemHotkey * item = (ItemHotkey *)Shortcuts->GetItem(inum,1);
	if(item){item->OnDeleteHotkey(Shortcuts,inum);}
}

void OptionsDialog::OnChangeCatalog(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	Options.LoadStyles(Katlist->GetString(Katlist->GetSelection()));
	Stylelist->Clear();
	for(int i = 0; i<Options.StoreSize();i++){
		Stylelist->Append(Options.GetStyle(i)->Name);
	}
	Stylelist->SetSelection(0);
}

void OptionsDialog::ChangeColors(){

	const wxColour & windowColor = Options.GetColour(WindowBackground);
	const wxColour & textColor = Options.GetColour(WindowText);
	Notebook *nb = Notebook::GetTabs();
	//tabs colors		
	for(size_t i = 0; i < nb->Size(); i++){
		TabPanel *tab = nb->Page(i);
		tab->SetBackgroundColour(windowColor);
		tab->SetForegroundColour(textColor);
		if(tab->Edit->ABox){
			tab->Edit->ABox->audioDisplay->ChangeColours();
		}
		const wxWindowList& siblings = tab->GetChildren();
		for(auto it = siblings.begin(); it != siblings.end(); it++){
			(*it)->SetBackgroundColour(windowColor);
			(*it)->SetForegroundColour(textColor);
		}

	}
	//tree colours
	OptionsTree->SetColours(windowColor, textColor);
	//dialogs colours
	wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
	while (node)
	{
		wxWindow* win = node->GetData();
		// do something with "win"
		win->SetBackgroundColour(windowColor);
		win->SetForegroundColour(textColor);
		win->Refresh();
		node = node->GetNext();
	}
	//wxSysColourChangedEvent evt1;
	//ProcessEvent(evt1);
	//SetBackgroundColour(windowColor);
	
	//this->Refresh();
	//GetParent()->Refresh(false);

}

