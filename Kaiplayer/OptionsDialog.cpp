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
#include <wx/fontpicker.h>
#include "NumCtrl.h"
#include "ColorPicker.h"
#include "KaiTextCtrl.h"
#include "FontDialog.h"
#include "KaiListCtrl.h"

OptionsDialog::OptionsDialog(wxWindow *parent, kainoteFrame *kaiparent)
	: wxDialog(parent,-1,_("Opcje"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE,"Options")
	{

	OptionsTree= new wxTreebook(this,-1);
	
	Kai=kaiparent;
	Stylelist=NULL;
	Katlist=NULL;

	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("SETTINGS"));
	SetIcon(icn);

	wxPanel *Main= new wxPanel(OptionsTree);
	wxPanel *ConvOpt= new wxPanel(OptionsTree);
	wxPanel *Hotkeyss= new wxPanel(OptionsTree);
	wxPanel *AudioMain= new wxPanel(OptionsTree);
	wxPanel *Video= new wxPanel(OptionsTree);
	wxPanel *Themes= new wxPanel(OptionsTree);

	hkeymodif=0;
	if(!Options.AudioOpts && !Options.LoadAudioOpts()){wxMessageBox(_("Dupa blada, opcje się nie wczytały, audio nie skonfigurujesz"), _("Błędny błąd"));}

	//Main
	{
		wxBoxSizer *MainSizer=new wxBoxSizer(wxVERTICAL);
		wxString labels[11]={_("Wczytywanie posortowanych napisów"),_("Włącz sprawdzanie pisowni"),
			_("Zaznaczaj linijkę z czasem aktywnej\nlinijki poprzedniej zakładki"),_("Zapisuj napisy z nazwą wideo"),
			_("Pokaż sugestie po dwukrotnym klininięciu na błąd"),_("Otwieraj napisy zawsze w nowej karcie"),
			_("Nie przechodź do następnej linii przy edycji czasów"),_("Zapisuj zmiany po przejściu na inną linię"),
			_("Wyłącz pokazywanie edycji na wideo\n(wymaga ponownego otwarcia zakładek)"),
			_("Włącz szukanie widocznej linii\npo wyjściu z pełnego ekranu"),_("Poziom śledzenia logów skryptów LUA")};
		wxString opts[11]={"Grid Load Sorted","Editbox Spellchecker","Auto Select Lines","Subs Autonaming",
			"Editbox Sugestions On Dclick","Open In New Card","Times Stop On line","Grid save without enter",
			"Disable live editing","Seek For Visible Lines","Automation Trace Level"};

		wxString langopts[2]={"Polski","English"};
		wxStaticBoxSizer *langSizer=new wxStaticBoxSizer(wxVERTICAL, Main, _("Język (wymaga restartu programu)"));
		KaiChoice *lang=new KaiChoice(Main,10000,wxDefaultPosition,wxDefaultSize,2,langopts);
		lang->SetSelection(Options.GetInt("Program Language"));
		lang->SetFocus();
		ConOpt(lang,"Program Language");
		langSizer->Add(lang,0,wxALL|wxEXPAND,2);
		MainSizer->Add(langSizer,0,wxRIGHT|wxEXPAND,5);

		wxArrayString dics;
		SpellChecker::AvailableDics(dics);
		if(dics.size()==0){dics.Add(_("Umieść pliki .dic i .aff do folderu \"Dictionary\""));}
		wxStaticBoxSizer *dicSizer=new wxStaticBoxSizer(wxVERTICAL, Main, _("Język sprawdzania pisowni (folder \"Dictionary\")"));
		
		KaiChoice *dic=new KaiChoice(Main,10001,wxDefaultPosition,wxDefaultSize,dics);

		dic->SetSelection(dic->FindString(Options.GetString("Dictionary Name")));
		ConOpt(dic,"Dictionary Name");
		dicSizer->Add(dic,0,wxALL|wxEXPAND,2);
		MainSizer->Add(dicSizer,0,wxRIGHT|wxEXPAND,5);
	
		for(int i=0;i<10;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(Main,-1,labels[i]);
			opt->SetValue(Options.GetBool(opts[i]));
			ConOpt(opt,opts[i]);
			MainSizer->Add(opt,0,wxALL,2);
		}

	
		wxBoxSizer *MainSizer1=new wxBoxSizer(wxHORIZONTAL);
		wxFlexGridSizer *MainSizer2=new wxFlexGridSizer(5,2,wxSize(5,5));
		//uwaga id 20000 ma tylko numctrl, pola tekstowe musza mieć inny id
		NumCtrl *ltl = new NumCtrl(Main, 20000, Options.GetString(opts[10]), 0, 5,true, wxDefaultPosition, wxSize(60,-1), wxTE_PROCESS_ENTER);
		NumCtrl *sc = new NumCtrl(Main, 20000, Options.GetString("Offset of start time"), -100000, 100000,true, wxDefaultPosition, wxSize(60,-1), wxTE_PROCESS_ENTER);
		NumCtrl *sc1 = new NumCtrl(Main, 20000, Options.GetString("Offset of end time"), -100000, 100000,true, wxDefaultPosition, wxSize(60,-1), wxTE_PROCESS_ENTER);
		KaiTextCtrl *sc2 = new KaiTextCtrl(Main, 22001, Options.GetString("Grid tag changing char"), wxDefaultPosition, wxSize(60,-1), wxTE_PROCESS_ENTER);
		NumCtrl *sc3 = new NumCtrl(Main, 20000, Options.GetString("Editbox tag buttons"), 0, 9,true, wxDefaultPosition, wxSize(60,-1), wxTE_PROCESS_ENTER);
		ConOpt(ltl, opts[10]);
		ConOpt(sc,"Offset of start time");
		ConOpt(sc1,"Offset of end time");
		ConOpt(sc2,"Grid tag changing char");
		ConOpt(sc3,"Editbox tag buttons");
		
		MainSizer2->Add(new wxStaticText(Main,-1,_("Opóźnienie klatek początkowych w ms:")),0,wxALIGN_CENTRE_VERTICAL);
		MainSizer2->Add(sc,0);
		MainSizer2->Add(new wxStaticText(Main,-1,_("Opóźnienie klatek końcowych w ms:")),0,wxALIGN_CENTRE_VERTICAL);
		MainSizer2->Add(sc1,0);
		MainSizer2->Add(new wxStaticText(Main,-1,_("Znak podmiany tagów ASS:")),0,wxALIGN_CENTRE_VERTICAL);
		MainSizer2->Add(sc2,0);
		MainSizer2->Add(new wxStaticText(Main,-1,_("Ilość przycisków wstawiających tagi ASS:")),0,wxALIGN_CENTRE_VERTICAL);
		MainSizer2->Add(sc3,0);
		MainSizer2->Add(new wxStaticText(Main,-1,labels[10]),0,wxALIGN_CENTRE_VERTICAL);
		MainSizer2->Add(ltl,0);
	
		MainSizer->Add(MainSizer2,0,wxLEFT|wxTOP,2);

		FontPickerButton *optf=new FontPickerButton(Main,-1,wxFont(Options.GetInt("Grid Font Size"),wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,Options.GetString("Grid Font Name")));
		ConOpt(optf,"Grid Font");
		MainSizer1->Add(new wxStaticText(Main,-1,_("Czcionka pola napisów:")),0,wxRIGHT| wxALIGN_CENTRE_VERTICAL,5);
		MainSizer1->Add(optf,0);
	
		MainSizer->Add(MainSizer1,0,wxLEFT|wxTOP,2);

		Main->SetSizerAndFit(MainSizer);
	}

		//Ustawienia konwersji
	{
		wxBoxSizer *ConvOptSizer1=new wxBoxSizer(wxVERTICAL);

		wxStaticBoxSizer *obr=new wxStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Wybierz katalog"));
		wxStaticBoxSizer *obr0=new wxStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Wybierz styl"));
		wxStaticBoxSizer *obr1=new wxStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Wybierz FPS"));
		wxStaticBoxSizer *obr2=new wxStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Czas w ms na jedną literę"));
		wxStaticBoxSizer *obr3=new wxStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Tagi wstawiane na początku każdej linijki ass"));
		wxStaticBoxSizer *obr4=new wxStaticBoxSizer(wxHORIZONTAL,ConvOpt,_("Rozdzielczość przy konwersji na ASS"));
		wxArrayString styles;
		wxArrayString fpsy;

		

		fpsy.Add("23.976");fpsy.Add("24");fpsy.Add("25");fpsy.Add("29.97");fpsy.Add("30");fpsy.Add("60");

        for(int i = 0;i<2;i++){
			wxString optname=(i==0)? Options.GetString("Default Style Catalog") : Options.GetString("Default Style");
			if(i!=0){
				for(int i = 0; i<Options.StoreSize();i++){
					styles.Add(Options.GetStyle(i)->Name);
				}
			}
			KaiChoice *cmb = new KaiChoice(ConvOpt, (i==0)?28888 : 28889, wxDefaultPosition, wxSize(200,-1), (i==0)?Options.dirs : styles, wxTE_PROCESS_ENTER);
		
			int sel=cmb->FindString(optname);
			
			if(sel>=0){cmb->SetSelection(sel);if(i==0 && Options.acdir!=optname){Options.LoadStyles(optname);}}
			else{
				if(i==0){sel=cmb->FindString(Options.acdir);}cmb->SetSelection(MAX(0,sel));
				wxString co=(i==0)?_("katalog dla stylu") : _("styl"); 
				wxMessageBox(wxString::Format(_("Wybrany %s konwersji nie istnieje,\nzostanie zmieniony na domyślny"), co),_("Uwaga"));}

			ConOpt(cmb,(i==0)? "Default Style Catalog" : "Default Style");
			if(i==0){
				obr->Add(cmb,1,wxCENTER);
				ConvOptSizer1->Add(obr,0,wxRIGHT|wxEXPAND,5);
				Katlist=cmb;
				Connect(28888,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OptionsDialog::OnChangeCatalog);
			}
			else{
				obr0->Add(cmb,1,wxCENTER);
				ConvOptSizer1->Add(obr0,0,wxRIGHT|wxEXPAND,5);
				Stylelist=cmb;
			}
		}

		KaiChoice *cmb = new KaiChoice(ConvOpt, -1, Options.GetString("Default FPS"), wxDefaultPosition, wxSize(200,-1), fpsy, wxTE_PROCESS_ENTER);
		int sel=cmb->FindString(Options.GetString("Default FPS"));
		if(sel>=0){cmb->SetSelection(sel);}
		else{cmb->SetValue(Options.GetString("Default FPS"));}

	    ConOpt(cmb,"Default FPS");
		obr1->Add(cmb,1,wxCENTER);
		ConvOptSizer1->Add(obr1,0,wxRIGHT|wxEXPAND,5);
		

		for(int i=0;i<3;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(ConvOpt,-1,(i==0)?_("FPS z wideo"):(i==1)?_("Nowe czasy końcowe"):_("Pokaż okno przed konwersją"));
			wxString optname=(i==0)?"FPS from video":(i==1)?"New end times":"Show settings window";
			opt->SetValue(Options.GetBool(optname));
			ConOpt(opt,optname);
			ConvOptSizer1->Add(opt,0,wxRIGHT|wxEXPAND,5);
		}

		NumCtrl *sc = new NumCtrl(ConvOpt, 20000, Options.GetString("Time show of letter"), 30, 1000, true, wxDefaultPosition, wxSize(250,-1), wxTE_PROCESS_ENTER);
		ConOpt(sc,"Time show of letter");
		obr2->Add(sc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		ConvOptSizer1->Add(obr2,0,wxRIGHT|wxEXPAND,5);

		sc = new NumCtrl(ConvOpt, 20000, Options.GetString("Convert Resolution W"), 1, 3000, true, wxDefaultPosition, wxSize(115,-1), wxTE_PROCESS_ENTER);
		ConOpt(sc,"Convert Resolution W");
		obr4->Add(sc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		
		wxStaticText* txt= new wxStaticText(ConvOpt,-1," X ");
		obr4->Add(txt,0,wxALL|wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL|wxEXPAND,2);

		sc = new NumCtrl(ConvOpt, 20000, Options.GetString("Convert Resolution H"), 1, 3000, true, wxDefaultPosition, wxSize(115,-1), wxTE_PROCESS_ENTER);
		ConOpt(sc,"Convert Resolution H");
		obr4->Add(sc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		ConvOptSizer1->Add(obr4,0,wxRIGHT|wxEXPAND,5);
		
		KaiTextCtrl *tc = new KaiTextCtrl(ConvOpt, -1, Options.GetString("Ass Conversion Prefix"), wxDefaultPosition, wxSize(250,-1),wxTE_PROCESS_ENTER);
		ConOpt(tc,"Ass Conversion Prefix");
		obr3->Add(tc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		ConvOptSizer1->Add(obr3,0,wxRIGHT|wxEXPAND,5);

		ConvOpt->SetSizerAndFit(ConvOptSizer1);
	}
	//Video
	{
		wxString voptspl[]={_("Otwórz wideo z menu kontekstowego na pełnym ekranie"),_("Lewy przycisk myszy pauzuje wideo"),
			_("Otwieraj wideo z czasem aktywnej linii"),_("Preferowane ścieżki audio (oddzielone średnikiem)")};
		wxString vopts[]={"Video Fullskreen on Start","Video Pause on Click","Open Video At Active Line","Accepted audio stream"};
		wxBoxSizer *MainSizer=new wxBoxSizer(wxVERTICAL);
		for(int i=0;i<3;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(Video,-1,voptspl[i]);
			opt->SetValue(Options.GetBool(vopts[i]));
			ConOpt(opt,vopts[i]);
			MainSizer->Add(opt,0,wxALL,2);
		}
		wxStaticBoxSizer *prefaudio=new wxStaticBoxSizer(wxHORIZONTAL,Video,voptspl[3]);
		KaiTextCtrl *tc = new KaiTextCtrl(Video, -1, Options.GetString(vopts[3]), wxDefaultPosition, wxSize(250,-1),wxTE_PROCESS_ENTER);
		ConOpt(tc,vopts[3]);
		prefaudio->Add(tc,1,wxALL|wxALIGN_CENTER|wxEXPAND,2);
		MainSizer->Add(prefaudio,0,wxRIGHT|wxEXPAND,5);


		Video->SetSizerAndFit(MainSizer);
	}
		//Hotkeys
	{
		wxBoxSizer *HkeysSizer=new wxBoxSizer(wxVERTICAL);
		Shortcuts = new wxListCtrl(Hotkeyss,26667,wxDefaultPosition,wxDefaultSize,wxLC_REPORT | wxLC_SINGLE_SEL);
		Shortcuts->InsertColumn(0,_("Funkcja"),wxLIST_FORMAT_LEFT,220);
		Shortcuts->InsertColumn(1,_("Skrót"),wxLIST_FORMAT_LEFT,120);
		Connect(26667,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&OptionsDialog::OnMapHkey);
		Connect(26667,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnResetHkey);		

		if(!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){wxMessageBox(_("Dupa blada, skróty klawiszowe nie wczytały się, na audio nie podziałasz"), _("Błędny błąd"));}
		
		std::map<idAndType, hdata> _hkeys;
		Hkeys.LoadDefault(_hkeys);
		Hkeys.LoadDefault(_hkeys,true);
		Notebook::GetTab()->Video->ContextMenu(wxPoint(0,0),true);
		Notebook::GetTab()->Grid1->ContextMenu(wxPoint(0,0),true);

		long ii=0;

		for (auto cur = Hkeys.hkeys.begin();cur != Hkeys.hkeys.end();cur++) {
			if(cur->second.Name==""){
				auto tmpkey = _hkeys.find(cur->first);
				if(tmpkey!=_hkeys.end()){
					cur->second.Name = tmpkey->second.Name;
				}else{
					wxString gnewa = "GNEWA";
					for(int i=0; i<5; i++){
						char window = gnewa[i];
						if(window == cur->first.Type){
							continue;
						}else{
							auto tmpkey = _hkeys.find(idAndType(cur->first.id, window));
							if(tmpkey!=_hkeys.end()){// && tmpkey->second.Name!=""
								cur->second.Name = tmpkey->second.Name;
								break;
							}
						}
					}
				}
				
			}

			wxString name=wxString(cur->first.Type)<<" "<<cur->second.Name;
			long pos = Shortcuts->InsertItem(ii,name);
			Shortcuts->SetItem(pos,1,cur->second.Accel);
			ii++;
		}

		
		HkeysSizer->Add(Shortcuts,1,wxALL|wxEXPAND,2);
		Hotkeyss->SetSizerAndFit(HkeysSizer);
		
	}

		//Audio main

	{
		wxBoxSizer *audio=new wxBoxSizer(wxVERTICAL);

		wxString names[]={_("Wyświetlaj czas przy kursorze"),_("Wyświetlaj znaczniki sekund"),_("Wyświetlaj tło zaznaczenia"),_("Wyświetlaj pozycję wideo"),
			_("Wyświetlaj klatki kluczowe"),_("Przewijaj wykres audio przy odtwarzaniu"), _("Aktywuj okno audio po najechaniu"), _("Przyklejaj do klatek kluczowych"),
			_("Przyklejaj do pozostałych linii"),_("Scalaj wszystkie \"n\" z poprzednią sylabą"),_("Przenoś linie sylab po kliknięciu"),_("Wczytuj audio do pamięci RAM")};

		wxString opts[]={"Audio Draw Cursor Time","Audio Draw Secondary Lines","Audio Draw Selection Background","Audio Draw Video Position",
			"Audio Draw Keyframes","Audio Lock Scroll On Cursor","Audio Autofocus","Audio Snap To Keyframes","Audio Snap To Other Lines",
			"Merge Every N With Syllable","Audio Karaoke Move On Click","Audio RAM Cache"};

		for(int i=0;i<12;i++)
		{
			KaiCheckBox *opt=new KaiCheckBox(AudioMain,-1,names[i]);
			opt->SetValue(Options.GetBool(opts[i]));
			ConOpt(opt,opts[i]);
			audio->Add(opt,0,wxALL,2);
		}

		wxString opts1[3]={"Audio Delay", "Audio Mark Play Time", "Audio Inactive Lines Display Mode"};
		NumCtrl *Delay = new NumCtrl(AudioMain, 20000, Options.GetString(opts1[0]), -50000000, 50000000, true, wxDefaultPosition, wxSize(300,-1), 0);
		NumCtrl *sc = new NumCtrl(AudioMain, 20000, Options.GetString(opts1[1]), 400, 5000, true, wxDefaultPosition, wxSize(300,-1), 0);
		wxString inact[3]={_("Brak"),_("Przed i po aktywnej"),_("Wszystkie widoczne")};
		KaiChoice *sc1 = new KaiChoice(AudioMain, 10000, wxDefaultPosition, wxSize(300,-1), 3, inact);
		sc1->SetSelection(Options.GetInt(opts1[2]));
		ConOpt(Delay,opts1[0]);
		ConOpt(sc,opts1[1]);
		ConOpt(sc1,opts1[2]);
		wxStaticBoxSizer *DelaySizer=new wxStaticBoxSizer(wxVERTICAL,AudioMain,_("Opóźnienie audio w ms"));
		wxStaticBoxSizer *audiocols=new wxStaticBoxSizer(wxVERTICAL,AudioMain,_("Czas odtwarzania audio przed i po znaczniku w ms"));	
		wxStaticBoxSizer *audiocols1=new wxStaticBoxSizer(wxVERTICAL,AudioMain,_("Sposób wyświetlania nieaktywnych linijek"));	
		DelaySizer->Add(Delay,1,wxALL|wxEXPAND,2);
		audiocols->Add(sc,1,wxALL|wxEXPAND,2);
		audiocols1->Add(sc1,1,wxALL|wxEXPAND,2);
		audio->Add(DelaySizer,0,wxRIGHT|wxEXPAND,5);
		audio->Add(audiocols,0,wxRIGHT|wxEXPAND,5);
		audio->Add(audiocols1,0,wxRIGHT|wxEXPAND,5);
		

		AudioMain->SetSizerAndFit(audio);
	}

		//Themes
	{
		//18+29=47+17+2 = 66 + 7 = 73
		wxString labels[74]={
			//okno
			_("Okno tło"),_("Okno nieaktywne tło"),_("Okno tekst"),_("Okno nieaktywny tekst"),
			//napisy
			_("Napisy tekst"),_("Napisy tło"),_("Napisy tło dialogów"),_("Napisy tło komentarzy"),
			_("Napisy zaznaczone dialogi"),_("Napisy zaznaczone komentarze"),
			_("Napisy kolidujące linie"),_("Napisy linie"),_("Napisy obramowanie aktywnej linijki"),
			_("Napisy etykieta"),_("Napisy etykieta zmodyfikowanej linii"),_("Napisy etykieta zapisanej linii"),
			_("Napisy tło błędów pisowni"),_("Napisy porównanie"),_("Napisy tło porównania"),
			_("Napisy tło porównania zaznaczenia"),_("Napisy tło komentarza porównania"),
			_("Napisy tło komentarza zazn. porównania"),
			//edytor
			_("Edytor tekst"),_("Edytor nazwy tagów"),_("Edytor wartości tagów"),
			_("Edytor nawiasy klamrowe"),_("Edytor operatory tagów"),_("Edytor tło"),
			_("Edytor zaznaczenie"),_("Edytor zaznaczenie nieaktywnego okna"),
			_("Edytor obramowanie"),_("Edytor obramowanie aktywnego okna"),_("Edytor błędy pisowni"),
			//audio
			_("Audio tło"),_("Audio znacznik start"),_("Audio znacznik koniec"),_("Audio znacznik przesuwania czasów"),
			_("Audio znaczniki nieaktywnej linijki"),_("Audio kursor"),_("Audio znaczniki sekund"),_("Audio klatki kluczowe"),
			_("Audio zaznaczenie"),_("Audio zaznaczenie po modyfikacji"),_("Audio wykres audio"),
			_("Audio nieaktywny wykres falowy"),_("Audio zmodyfikowany wykres falowy"),_("Audio zaznaczony wykres falowy"),
			_("Audio tło nieaktywnych linijek"),_("Audio tło spektrum"), _("Audio spektrum"), _("Audio echo spektrum"),
			//kontrolki
			_("Przycisk i lista tło"), _("Przycisk i lista tło po najechaniu"), 
			_("Przycisk i lista tło po wciśnięciu"), _("Przycisk i lista obramowanie"),
			_("Przycisk i lista obramowanie nieaktywne"),_("Przycisk i lista obramowanie po najechaniu"),
			_("Przycisk i lista obramowanie po wciśnięciu"),_("Przełącznik tło włączonego"),
			_("Przełącznik obramowanie włączonego"),_("Pasek przewijania tło"),_("Pasek przewijania suwak"),
			_("Pasek przewijania suwak po wciśnięciu"),_("Pasek przewijania suwak po najechaniu"),
			_("Ramka z opisem obramowanie"),
			//pasek menu
			_("Pasek menu tło 1"),_("Pasek menu tło 2"),_("Pasek menu obramowanie zaznaczenia"),
			_("Pasek menu tło zaznaczenia"),_("Menu obramowanie"),_("Menu obramowanie zaznaczenia"),
			_("Menu tło zaznaczenia"),
			_("Pierwszy kolor podglądu styli"),_("Drugi kolor podglądu styli")
		};
		wxString opts[74]={
			//window
			"Window Background", "Window Inactive Background", "Window Text", "Window Inactive Text",
			//grid
			"Grid Text","Grid Background","Grid Dialogue","Grid Comment","Grid Selected Dialogue","Grid Selected Comment",
			"Grid Collisions","Grid Active Line","Grid Lines","Grid Label Normal","Grid Label Modified",
			"Grid Label Saved","Grid Spellchecker","Grid Comparison","Grid Comparison Background",
			"Grid Comparison Background selected","Grid Comparison Comment Background",
			"Grid Comparison Comment Background Selected",
			//text field
			"Editor Text","Editor Tag Names","Editor Tag Values",
			"Editor Curly Braces","Editor Tag Operators","Editor Background",
			"Editor Selection","Editor Selection No Focus","Editor Border",
			"Editor Border Focus","Editor Spellchecker",
			//audio
			"Audio Background","Audio Line Boundary Start","Audio Line Boundary End","Audio Line Boundary Mark",
			"Audio Line Boundary Inactive Line","Audio Play Cursor","Audio Seconds Boundaries","Audio Keyframes",
			"Audio Selection Background","Audio Selection Background Modified","Audio Waveform","Audio Waveform Inactive",
			"Audio Waveform Modified","Audio Waveform Selected","Audio Inactive Lines Background","Audio Spectrum First Color",
			"Audio Spectrum Second Color","Audio Spectrum Third Color",
			//controls
			"Button Background", "Button Background Hover", "Button Background Pushed", "Button Border",
			"Button Inactive Border", "Button Border Hover", "Button Border Pushed","Togglebutton Background Toggled",
			"Togglebutton Border Toggled","Scrollbar Background","Scrollbar Scroll","Scrollbar Scroll Pushed",
			"Scrollbar Scroll Hover","Staticbox Border",
			//menu bar 
			"Menu Bar Background 1","Menu Bar Background 2","Menu Bar Border Selection",
			"Menu Bar Background Selection","Menu Border","Menu Border Selection","Menu Background Selection",
			"Style Preview Color1","Style Preview Color2"
		};
		
		wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
		KaiListCtrl *List = new KaiListCtrl(Themes, -1, wxDefaultPosition, wxSize(300, -1));
		List->InsertCollumn(0, "Nazwa", TYPE_TEXT, 220);
		List->InsertCollumn(1, "Kolor", TYPE_COLOR, 150);
		for(int i=0;i<74;i++)
		{
			int row = List->AppendItem(new ItemText(labels[i]));
			AssColor col = Options.GetColor(opts[i]);
			List->SetItem(row, 1, new ItemColor(col));
		}
		sizer->Add(List, 1, wxALL|wxEXPAND, 2);
		Themes->SetSizerAndFit(sizer);
	}
		
	//Adding pages
	OptionsTree->AddPage(Main,_("Edytor"),true);
	OptionsTree->AddSubPage(ConvOpt,_("Konwersja"),true);
	OptionsTree->AddPage(Video,_("Wideo"),true);
	OptionsTree->AddPage(AudioMain,_("Audio"),true);
	OptionsTree->AddPage(Themes,_("Motywy"),true);
	OptionsTree->AddPage(Hotkeyss,_("Skróty klawiszowe"),true);
	OptionsTree->Fit();
		
	//adding buttons
	wxBoxSizer *ButtonsSizer=new wxBoxSizer(wxHORIZONTAL);

	okok=new wxButton(this,wxID_OK,"OK");
	wxButton *oknow=new wxButton(this,ID_BCOMMIT,_("Zastosuj"));
	wxButton *cancel=new wxButton(this,wxID_CANCEL,_("Anuluj"));

	ButtonsSizer->Add(okok,0,wxRIGHT,2);
	ButtonsSizer->Add(oknow,0,wxRIGHT,2);
	ButtonsSizer->Add(cancel,0,wxRIGHT,2);

	wxBoxSizer *TreeSizer=new wxBoxSizer(wxVERTICAL);
	TreeSizer->Add(OptionsTree,0,wxTOP|wxLEFT,2);
	TreeSizer->Add(ButtonsSizer,0,wxBOTTOM|wxALIGN_CENTER,2);
	SetSizerAndFit(TreeSizer);

	CenterOnParent();

    Connect(ID_BCOMMIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnSaveClick);
    Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent &evt){Hide();});

	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_NORMAL, WXK_RETURN, wxID_OK);
	wxAcceleratorTable accel(1, entries);
	this->SetAcceleratorTable(accel);
	
		
}

OptionsDialog::~OptionsDialog()
{
	if(GetReturnCode ()==wxID_OK){
		SetOptions();
		if(hkeymodif==1){Hkeys.SaveHkeys();Kai->SetAccels();}
		else if(hkeymodif==2){
			Hkeys.SaveHkeys(true);
			if(Kai->GetTab()->Edit->ABox){Kai->GetTab()->Edit->ABox->SetAccels();}
		}
	}
	handles.clear();
}

void OptionsDialog::ConOpt(wxWindow *ctrl,wxString option)
{
	OptionsBind Obind;
	Obind.ctrl=ctrl;
	Obind.option=option;
	handles.push_back(Obind);
}

void OptionsDialog::OnSaveClick(wxCommandEvent& event)
{
	SetOptions(false);
	if(hkeymodif==1){Hkeys.SaveHkeys();Kai->SetAccels();}
	else if(hkeymodif==2){
		Hkeys.SaveHkeys(true);
		if(Kai->GetTab()->Edit->ABox){Kai->GetTab()->Edit->ABox->SetAccels();}
	}
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
				if(OB.option=="Editbox Spellchecker"){
					for(size_t i = 0; i< Kai->Tabs->Size();i++){
						Kai->Tabs->Page(i)->Grid1->SpellErrors.clear();
					}
					Kai->Tabs->GetTab()->Grid1->Refresh(false);
					Kai->Tabs->GetTab()->Edit->TextEdit->SpellcheckerOnOff();
				}
			}
		}
		else if(OB.ctrl->IsKindOf(CLASSINFO(FontPickerButton))){
			FontPickerButton *fpc=(FontPickerButton*)OB.ctrl;
			wxFont font=fpc->GetSelectedFont();
			wxString fontname=font.GetFaceName();
			int fontsize=font.GetPointSize();
			if(Options.GetString(OB.option+" Name")!=fontname){
				Options.SetString(OB.option+" Name",fontname);fontmod=true;
			}
			if(Options.GetInt(OB.option+" Size")!=fontsize){
				Options.SetInt(OB.option+" Size",fontsize);fontmod=true;
			}
		}
		/*else if(OB.ctrl->IsKindOf(CLASSINFO(KaiChoice))){
			KaiChoice *cbx=(KaiChoice*)OB.ctrl;
			wxString kol=cbx->GetValue();
			if(Options.GetString(OB.option)!=kol){
				Options.SetString(OB.option,kol);
			}
		}*/
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
						Kai->GetTab()->Grid1->SpellErrors.clear();
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
			}else{
				NumCtrl *sc=(NumCtrl*)OB.ctrl;
				int num=sc->GetInt();
				if(Options.GetInt(OB.option)!=num){
					Options.SetInt(OB.option,num);
				}
			}
		}else if(OB.ctrl->IsKindOf(CLASSINFO(MappedButton))){
			ColorButton *cpc=(ColorButton*)OB.ctrl;
			AssColor kol=cpc->GetColor();
			if(Options.GetColor(OB.option)!=kol){
				Options.SetColor(OB.option,kol);colmod=true;
			}
		}
	}
	if(fontmod){
		Kai->GetTab()->Grid1->SetStyle();
		Kai->GetTab()->Grid1->RepaintWindow();
		if(Kai->Tabs->split){
			Kai->Tabs->GetSecondPage()->Grid1->SetStyle();
			Kai->Tabs->GetSecondPage()->Grid1->RepaintWindow();
		}
	}
	if(colmod){
		Kai->GetTab()->Grid1->Refresh(false);
		if(Kai->GetTab()->Edit->ABox){Kai->GetTab()->Edit->ABox->audioDisplay->ChangeColours();}
		if(Kai->Tabs->split){
			Kai->Tabs->GetSecondPage()->Grid1->Refresh(false);
			if(Kai->Tabs->GetSecondPage()->Edit->ABox){Kai->Tabs->GetSecondPage()->Edit->ABox->audioDisplay->ChangeColours();}
		}
	}
	Options.SaveOptions();
	Options.SaveAudioOpts();
}

void OptionsDialog::OnMapHkey(wxListEvent& event)
{
	int inum=event.GetIndex();
	//wxListItem item=event.GetItem();
	wxString itemtext=Shortcuts->GetItemText(inum,0);
	wxString hotkey=Shortcuts->GetItemText(inum,1);
	wxString shkey=itemtext.AfterFirst(' ');
	HkeysDialog hkd(this,shkey, itemtext[0], !shkey.StartsWith("Script") );
	if(hkd.ShowModal()==wxID_OK){
	
		const idAndType *itype=NULL;
		for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++){
			if(cur->second.Name == shkey){itype= new idAndType(cur->first.id, hkd.type);}
			//wxLogStatus(cur->second.Name);
			if(cur->second.Accel == hkd.hotkey && (cur->first.Type == hkd.type) ){
				wxMessageDialog msg(this, 
					wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\".\nCo zrobić?"),
					cur->second.Name), _("Uwaga"), wxYES_NO|wxCANCEL);
				msg.SetYesNoLabels (_("Zamień skróty"), _("Usuń skrót"));
				int result = msg.ShowModal();
				if(result!=wxCANCEL)
				{
					if(result==wxNO){hotkey="";}
					cur->second.Accel=hotkey;
					long nitem=Shortcuts->FindItem(-1, wxString(cur->first.Type) + " " + cur->second.Name);
					if(nitem!=-1){
						Shortcuts->SetItem(nitem,1,hotkey);
					}
				}else{ return;}
			}
		}
		
		if(!itype){return;};
		Hkeys.SetHKey(*itype, shkey, hkd.hotkey);
		if(itemtext[0] != hkd.type){
			long pos = Shortcuts->InsertItem(Shortcuts->GetItemCount(),itemtext.replace(0,1,hkd.type));
			Shortcuts->SetItem(pos,1,Hkeys.GetMenuH(*itype));
			Shortcuts->SetItemState(inum, wxLIST_STATE_INUSE, wxLIST_STATE_INUSE);
			Shortcuts->SetItemState(pos, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			wxRect rc;
			Shortcuts->GetItemRect (pos,rc);
			Shortcuts->ScrollList(0,(pos- inum) * rc.height);
		}else{
			Shortcuts->SetItem(inum,1,Hkeys.GetMenuH(*itype));
		}
		
		if(hkd.type=='A'){hkeymodif=2;}
		else{hkeymodif=1;}
		delete itype;
	}
}

void OptionsDialog::OnResetHkey(wxListEvent& event)
{
	
	wxListItem item=event.GetItem();
	const idAndType *itype=NULL;
	for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++){//wxLogStatus(cur->first);
		if(cur->second.Name == item.GetText().AfterFirst(' ')){itype= &cur->first; }
	}
	if(!itype){return;};
	Hkeys.ResetKey(itype);
	Shortcuts->SetItem(event.GetIndex(),1,Hkeys.GetMenuH(*itype));
	if(item.GetText().StartsWith("A")){hkeymodif=2;}
	else{hkeymodif=1;}
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


