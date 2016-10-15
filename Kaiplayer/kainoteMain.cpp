/***************************************************************
* Name:      kainoteMain.cpp
* Purpose:   Code for Application Frame
* Author:    Bjakja (bjakja@op.pl)
* Created:   2012-04-23
* Copyright: Bjakja (www.costam.com)
* License:
**************************************************************/


#include "KainoteMain.h"
#include "SubsTime.h"
#include "Stylelistbox.h"
#include "ScriptInfo.h"
#include "Config.h"
#include "OptionsDialog.h"
#include "DropFiles.h"
#include "OpennWrite.h"
#include "Hotkeys.h"
#include "FontCollector.h"
#include "Menu.h"
#include <wx/accel.h>
#include <wx/dir.h>
#include <wx/sysopt.h>


#undef IsMaximized
#if _DEBUG
#define logging 5
#endif
//#define wxIMAGE_PNG(x) wxImage(wxS(#x),wxBITMAP_TYPE_PNG_RESOURCE)

kainoteFrame::kainoteFrame(const wxPoint &pos, const wxSize &size)
	: wxFrame(0, -1, _("Bez nazwy - ")+Options.progname, pos, size, wxDEFAULT_FRAME_STYLE)
{

#if logging
	mylog=new wxLogWindow(this, "Logi",true, false);
	mylog->PassMessages(true);
#else
	mylog=NULL;
#endif
	ss=NULL;
	FR=NULL;
	SL=NULL;
	Auto=NULL;
	subsrec=Options.GetTable("Subs Recent");
	videorec=Options.GetTable("Video Recent");
	audsrec=Options.GetTable("Recent Audio");
	wxFont thisFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	SetFont(thisFont);
	wxIcon kaiicon("aaaa",wxBITMAP_TYPE_ICO_RESOURCE); 
	SetIcon(kaiicon);
	SetMinSize(wxSize(600,400));
	Menubar = new MenuBar(this);

	wxBoxSizer *mains1= new wxBoxSizer(wxVERTICAL);
	mains=new wxBoxSizer(wxHORIZONTAL);
	Tabs=new Notebook (this,ID_TABS);
	Toolbar=new KaiToolbar(this,Menubar,-1,true);
	mains->Add(Toolbar,0,wxEXPAND,0);
	mains->Add(Tabs,1,wxEXPAND,0);
	mains1->Add(Menubar,0,wxEXPAND,0);
	mains1->Add(mains,1,wxEXPAND,0);

	FileMenu = new Menu();
	AppendBitmap(FileMenu,OpenSubs, _("&Otwórz napisy"), _("Otwórz plik napisów"),wxBITMAP_PNG ("opensubs"));
	AppendBitmap(FileMenu,SaveSubs, _("&Zapisz"), _("Zapisz aktualny plik"),wxBITMAP_PNG("save"));
	AppendBitmap(FileMenu,SaveAllSubs, _("Zapisz &wszystko"), _("Zapisz wszystkie napisy"),wxBITMAP_PNG("saveall"));
	AppendBitmap(FileMenu,SaveSubsAs, _("Zapisz &jako..."), _("Zapisz jako"),wxBITMAP_PNG("saveas"));
	AppendBitmap(FileMenu,SaveTranslation, _("Zapisz &tłumaczenie"), _("Zapisz tłumaczenie"),wxBITMAP_PNG("savetl"),false);
	SubsRecMenu = new Menu();
	//AppendRecent();
	AppendBitmap(FileMenu,RecentSubs, _("Ostatnio otwa&rte napisy"), _("Ostatnio otwarte napisy"),wxBITMAP_PNG("recentsubs"),true, SubsRecMenu);
	AppendBitmap(FileMenu,RemoveSubs, _("Usuń napisy z e&dytora"), _("Usuń napisy z edytora"),wxBITMAP_PNG("close"));
	FileMenu->Append(9989,"Pokaż / Ukryj okno &logów");
	AppendBitmap(FileMenu,Settings, _("&Ustawienia"), _("Ustawienia programu"),wxBITMAP_PNG("SETTINGS"));
	AppendBitmap(FileMenu,Quit, _("Wyjści&e\tAlt-F4"), _("Zakończ działanie programu"),wxBITMAP_PNG("exit"));
	Menubar->Append(FileMenu, _("&Plik"));

	EditMenu = new Menu();
	AppendBitmap(EditMenu, Undo, _("&Cofnij"), _("Cofnij"),wxBITMAP_PNG("undo"),false);
	AppendBitmap(EditMenu, Redo, _("&Ponów"), _("Ponów"),wxBITMAP_PNG("redo"),false);
	AppendBitmap(EditMenu,FindReplace, _("Znajdź i za&mień"), _("Szuka i podmienia dane frazy tekstu"),wxBITMAP_PNG("findreplace"));
	AppendBitmap(EditMenu,Search, _("Z&najdź"), _("Szuka dane frazy tekstu"),wxBITMAP_PNG("search"));
	Menu *SortMenu[2];
	for(int i=0; i<2; i++){
		SortMenu[i]=new Menu();
		SortMenu[i]->Append(7000+(6*i),_("Czas początkowy"),_("Sortuj według czasu początkowego"));
		SortMenu[i]->Append(7001+(6*i),_("Czas końcowy"),_("Sortuj według czasu końcowego"));
		SortMenu[i]->Append(7002+(6*i),_("Style"),_("Sortuj według styli"));
		SortMenu[i]->Append(7003+(6*i),_("Aktor"),_("Sortuj według aktora"));
		SortMenu[i]->Append(7004+(6*i),_("Efekt"),_("Sortuj według efektu"));
		SortMenu[i]->Append(7005+(6*i),_("Warstwa"),_("Sortuj według warstwy"));
	}

	AppendBitmap(EditMenu,SortLines, _("Sort&uj wszystkie linie"), _("Sortuje wszystkie linie napisów ASS"),wxBITMAP_PNG("sort"),true,SortMenu[0]);
	AppendBitmap(EditMenu,SortSelected, _("Sortuj zazna&czone linie"),_("Sortuje zaznaczone linie napisów ASS"),wxBITMAP_PNG("sortsel"),true, SortMenu[1]);
	AppendBitmap(EditMenu,SelectLines, _("Zaznacz &linijki"), _("Zaznacza linijki wg danej frazy tekstu"),wxBITMAP_PNG("sellines"));
	Menubar->Append(EditMenu, _("&Edycja"));

	VidMenu = new Menu();
	AppendBitmap(VidMenu,OpenVideo, _("Otwórz wideo"), _("Otwiera wybrane wideo"),wxBITMAP_PNG("openvideo"));
	VidsRecMenu = new Menu();
	AppendBitmap(VidMenu, RecentVideo, _("Ostatnio otwarte wideo"), _("Ostatnio otwarte video"),wxBITMAP_PNG("recentvideo"),true, VidsRecMenu);
	AppendBitmap(VidMenu, SetStartTime, _("Wstaw czas początkowy z wideo"), _("Wstawia czas początkowy z wideo"),wxBITMAP_PNG("setstarttime"),false);
	AppendBitmap(VidMenu, SetEndTime, _("Wstaw czas końcowy z wideo"), _("Wstawia czas końcowy z wideo"),wxBITMAP_PNG("setendtime"),false);
	AppendBitmap(VidMenu, PreviousFrame, _("Klatka w tył"), _("Przechodzi o jedną klatkę w tył"),wxBITMAP_PNG("prevframe"),false);
	AppendBitmap(VidMenu, NextFrame, _("Klatka w przód"), _("Przechodzi o jedną klatkę w przód"),wxBITMAP_PNG("nextframe"),false);
	AppendBitmap(VidMenu, SetVideoAtStart,_("Przejdź do czasu początkowego linii"),_("Przechodzi wideo do czasu początkowego linii"),wxBITMAP_PNG("videoonstime"));
	AppendBitmap(VidMenu, SetVideoAtEnd,_("Przejdź do czasu końcowego linii"),_("Przechodzi wideo do czasu końcowego linii"),wxBITMAP_PNG("videoonetime"));
	AppendBitmap(VidMenu, PlayPauseG, _("Odtwarzaj / Pauza"), _("Odtwarza lub pauzuje wideo"),wxBITMAP_PNG("pausemenu"),false);
	VidMenu->Append(VideoIndexing, _("Otwieraj wideo przez FFMS2"), _("Otwiera wideo przez FFMS2, co daje dokładność klatkową"),true,0,0,ITEM_CHECK)->Check(Options.GetBool("Index Video"));

	Menubar->Append(VidMenu, _("&Wideo"));

	AudMenu = new Menu();
	AppendBitmap(AudMenu,OpenAudio, _("Otwórz audio"), _("Otwiera wybrane audio"),wxBITMAP_PNG("openaudio"));
	AudsRecMenu = new Menu();

	AppendBitmap(AudMenu,RecentAudio, _("Ostatnio otwarte audio"), _("Ostatnio otwarte audio"),wxBITMAP_PNG("recentaudio"),true , AudsRecMenu);
	AppendBitmap(AudMenu,AudioFromVideo, _("Otwórz audio z wideo"), _("Otwiera audio z wideo"),wxBITMAP_PNG("audiofromvideo"));
	AppendBitmap(AudMenu,CloseAudio, _("Zamknij audio"), _("Zamyka audio"),wxBITMAP_PNG("closeaudio"));
	Menubar->Append(AudMenu, _("A&udio"));

	ViewMenu = new Menu();
	ViewMenu->Append(ViewAll, _("Wszystko"), _("Wszystkie okna są widoczne"));
	ViewMenu->Append(ViewVideo, _("Wideo i napisy"), _("Widoczne tylko okno wideo i napisów"));
	ViewMenu->Append(ViewAudio, _("Audio i napisy"), _("Widoczne tylko okno audio i napisów"));
	ViewMenu->Append(ViewSubs, _("Tylko napisy"), _("Widoczne tylko okno napisów"));
	Menubar->Append(ViewMenu, _("Wido&k"));

	SubsMenu = new Menu();
	AppendBitmap(SubsMenu,Editor, _("Włącz / Wyłącz edytor"), _("Włączanie bądź wyłączanie edytora"),wxBITMAP_PNG("editor"));
	AppendBitmap(SubsMenu,ASSProperties, _("Właściwości ASS"), _("Właściwości napisów ASS"),wxBITMAP_PNG("ASSPROPS"));
	AppendBitmap(SubsMenu,StyleManager, _("&Menedżer stylów"), _("Służy do zarządzania stylami ASS"),wxBITMAP_PNG("styles"));
	ConvMenu = new Menu();
	AppendBitmap(ConvMenu,ConvertToASS, _("Konwertuj do ASS"), _("Konwertuje do formatu ASS"),wxBITMAP_PNG("convass"),false);
	AppendBitmap(ConvMenu,ConvertToSRT, _("Konwertuj do SRT"), _("Konwertuje do formatu SRT"),wxBITMAP_PNG("convsrt"));
	AppendBitmap(ConvMenu,ConvertToMDVD, _("Konwertuj do MDVD"), _("Konwertuje do formatu microDVD"),wxBITMAP_PNG("convmdvd"));
	AppendBitmap(ConvMenu,ConvertToMPL2, _("Konwertuj do MPL2"), _("Konwertuje do formatu MPL2"),wxBITMAP_PNG("convmpl2"));
	AppendBitmap(ConvMenu,ConvertToTMP, _("Konwertuj do TMP"), _("Konwertuje do formatu TMPlayer (niezalecene)"),wxBITMAP_PNG("convtmp"));

	AppendBitmap(SubsMenu,ID_CONV, _("Konwersja"), _("Konwersja z jednego formatu napisów na inny"),wxBITMAP_PNG("convert"),true, ConvMenu);
	AppendBitmap(SubsMenu,ChangeTime, _("Okno zmiany &czasów\tCtrl-I"), _("Przesuwanie czasów napisów"),wxBITMAP_PNG("times"));
	//AppendBitmap(SubsMenu,CrossPositioner, _("Włącz wskaźnik pozycji"), _("Włącz przesuwanie tekstu"),wxBITMAP_PNG("cross"),false);
	//AppendBitmap(SubsMenu,Positioner, _("Włącz przesuwanie"), _("Włącz przesuwanie tekstu"),wxBITMAP_PNG("position"),false);
	//AppendBitmap(SubsMenu,Movement, _("Włącz ruch"), _("Włącz przesuwanie tekstu"),wxBITMAP_PNG("move"),false);
	////AppendBitmap(SubsMenu,ID_MOVEONCURVE, _("Włącz ruch po krzywej"), _("Włącz / Wyłącz przesuwanie tekstu"),wxBITMAP_PNG("move"),false);
	//AppendBitmap(SubsMenu,Scalling, _("Włącz skalowanie"), _("Włącz skalowanie tekstu"),wxBITMAP_PNG("scale"),false);
	//AppendBitmap(SubsMenu,RotatingZ, _("Włącz obracanie w osi Z"), _("Włącz obracanie tekstu w osi Z"),wxBITMAP_PNG("frz"),false);
	//AppendBitmap(SubsMenu,RotatingXY, _("Włącz obracanie w osi X / Y"), _("Włącz obracanie tekstu w osi X / Y"),wxBITMAP_PNG("frxy"),false);
	////AppendBitmap(SubsMenu,ID_FAXY, _("Włącz / Wyłącz pochylanie tekstu"), _("Włącz / Wyłącz pochylanie tekstu"),wxBITMAP_PNG("clip"),false);
	//AppendBitmap(SubsMenu,RectangleClips, _("Włącz wycinki prostokątne"), _("Włącz tworzenie wycinków prostokątnych"),wxBITMAP_PNG("cliprect"),false);
	//AppendBitmap(SubsMenu,VectorClips, _("Włącz wycinki wektorowe"), _("Włącz tworzenie wycinków wektorowych"),wxBITMAP_PNG("clip"),false);
	//AppendBitmap(SubsMenu,VectorDrawings, _("Włącz rysunki wektorowe"), _("Włącz tworzenie rysunków"),wxBITMAP_PNG("drawing"),false);
	////AppendBitmap(SubsMenu,MoveAll, _("Włącz rysunki wektorowe"), _("Włącz tworzenie rysunków"),wxBITMAP_PNG("drawing"),false);
	//SubsMenu->Append(MoveAll, _("Włącz przesuwanie wielu tagów"), _("Włącz przesuwanie wielu tagów"))->Enable(false);
	AppendBitmap(SubsMenu,FontCollector, _("Kolekcjoner czcionek"), _("Kolekcjoner czcionek"),wxBITMAP_PNG("fontcollector"));
	AppendBitmap(SubsMenu,HideTags, _("Ukryj tagi w nawiasach"), _("Ukrywa tagi w nawiasach ASS i MDVD"),wxBITMAP_PNG("hidetags"));
	Menubar->Append(SubsMenu, _("&Napisy"));

	AutoMenu = new Menu();
	AppendBitmap(AutoMenu,AutoLoadScript, _("Wczytaj skrypt"), _("Wczytaj skrypt"),wxBITMAP_PNG("automation"));
	AppendBitmap(AutoMenu,AutoReloadAutoload, _("Odśwież skrypty autoload"), _("Odśwież skrypty autoload"),wxBITMAP_PNG("automation"));
	Menubar->Append(AutoMenu, _("Au&tomatyzacja"));

	HelpMenu = new Menu();
	AppendBitmap(HelpMenu,Help, _("&Pomoc (niekompletna, ale jednak)"), _("Otwiera pomoc w domyślnej przeglądarce"),wxBITMAP_PNG("help"));
	AppendBitmap(HelpMenu,ANSI, _("&Wątek programu na forum AnimeSub.info"), _("Otwiera wątek programu na forum AnimeSub.info"),wxBITMAP_PNG("ansi"));
	AppendBitmap(HelpMenu,About, _("&O programie"), _("Wyświetla informacje o programie"),wxBITMAP_PNG("about"));
	AppendBitmap(HelpMenu,Helpers, _("&Lista osób pomocnych przy tworzeniu programu"), _("Wyświetla listę osób pomocnych przy tworzeniu programu"),wxBITMAP_PNG("helpers"));
	Menubar->Append(HelpMenu, _("Pomo&c"));

	Toolbar->InitToolbar();

	SetSizer(mains1);

	SetAccels(false);


	StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1);
	int StatusBarWidths[6] = { -12, 78, 65, 70, 76, -22};
	int StatusBarStyles[6] = { wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL };
	StatusBar1->SetFieldsCount(6,StatusBarWidths);
	StatusBar1->SetStatusStyles(6,StatusBarStyles);
	SetStatusBar(StatusBar1);

	Connect(ID_TABS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageChanged,0,this);
	Connect(ID_ADDPAGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageAdd);
	Connect(ID_CLOSEPAGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageClose);
	Connect(NextTab,PreviousTab,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageChange);
	//tutaj dodawaj nowe idy
	Connect(SaveSubs,Undo,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected);
	Connect(7000,7011,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected);
	Connect(OpenSubs,ANSI,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected1);
	Connect(SelectFromVideo,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected1);
	Connect(Plus5SecondG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnP5Sec);
	Connect(Minus5SecondG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnM5Sec);
	Connect(PreviousLine,JoinWithNext,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnChangeLine);
	Connect(Remove,RemoveText,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnDelete);
	Menubar->Connect(EVT_MENU_OPENED,(wxObjectEventFunction)&kainoteFrame::OnMenuOpened,0,this);
	Connect(wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&kainoteFrame::OnClose1);
	Connect(30000,30059,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnRecent);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &event){
		if(!mylog){
			mylog=new wxLogWindow(this, "Logi",true, false);
			mylog->PassMessages(true);
		}else{
			delete mylog; mylog=NULL;
		}
	},9989);
	//Connect(30100,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnRunScript);
	//Connect(30100,30200,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuClick);
	Connect(SnapWithStart,SnapWithEnd,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnAudioSnap);
	SetDropTarget(new DragnDrop(this));


	bool im=Options.GetBool("Window Maximize");
	if(im){Maximize(Options.GetBool("Window Maximize"));}

	if(!Options.GetBool("Show Editor")){HideEditor();}	
	std::set_new_handler(OnOutofMemory);
	//Auto=new Auto::Automation();
	//Auto->BuildMenuWithDelay(&AutoMenu,1000);
}

kainoteFrame::~kainoteFrame()
{

	bool im=IsMaximized();
	if(!im && !IsIconized()){
		int posx,posy,sizex,sizey;
		GetPosition(&posx,&posy);
		if(posx<2000){
			Options.SetCoords("Window Position",posx,posy);}
		GetSize(&sizex,&sizey);
		Options.SetCoords("Window Size",sizex,sizey);
	}

	Toolbar->Destroy();
	Options.SetBool("Window Maximize",im);
	Options.SetBool("Show Editor",GetTab()->edytor);
	Options.SetTable("Subs Recent",subsrec);
	Options.SetTable("Video Recent",videorec);
	Options.SetTable("Recent Audio",audsrec);
	Options.SetInt("Video Volume",GetTab()->Video->volslider->GetValue());

	Options.SaveOptions();


	if(ss){ss->Destroy();ss=NULL;}
	if(FR){FR->Destroy();FR=NULL;}
	if(Auto){delete Auto; Auto=NULL;}
	Tabs->Destroy();
	Tabs=NULL;
	SpellChecker::Destroy();
	VideoToolbar::DestroyIcons();
}


//elementy menu które ulegają wyłączaniu
void kainoteFrame::OnMenuSelected(wxCommandEvent& event)
{
	int id=event.GetId();
	int Modif = event.GetInt();
	//MenuItem *item = (MenuItem*)event.GetClientData();

	TabPanel *pan=GetTab();
	//wxLogStatus("menu %i %i %i", id, Modif, item->id);
	if(Modif==wxMOD_SHIFT){
		MenuItem *item=Menubar->FindItem(id);
		wxString wins[1]={"Globalny"};
		//upewnij się, że da się zmienić idy na nazwy, 
		//może i trochę spowolni operację ale skończy się ciągłe wywalanie hotkeysów
		//może od razu funkcji onmaphotkey przekazać item by zrobiła co trzeba
		int ret=-1;
		wxString name=item->GetLabelText();
		ret=Hkeys.OnMapHkey( id, name, this, wins, 1);
		if(ret==-1){item->SetAccel(&Hkeys.GetHKey(id));Hkeys.SaveHkeys();SetAccels(false);}
		else if(ret>0){
			MenuItem *item=Menubar->FindItem(ret);
			wxAcceleratorEntry entry;
			item->SetAccel(&entry);
			SetAccels(false);
		}

		return;
	}
	if(id==SaveSubs){
		Save(false);
	}else if(id==SaveSubsAs){
		Save(true);
	}else if(id==SaveAllSubs){
		SaveAll();
	}else if(id==SaveTranslation){
		GetTab()->Grid1->AddSInfo("TLMode", "Translated",false);
		Save(true);
		GetTab()->Grid1->AddSInfo("TLMode", "Yes",false);
	}else if(id==RemoveSubs){
		if(SavePrompt(3)){event.SetInt(-1);return;}
		if(pan->SubsPath!=("")){
			pan->SubsName=_("Bez tytułu");
			pan->SubsPath="";
			Label();
			pan->Grid1->Clearing();
			pan->Grid1->file=new SubsFile();
			pan->Grid1->LoadDefault();
			pan->Edit->RefreshStyle(true);
			pan->Grid1->RepaintWindow();

			if(pan->Video->GetState()!=None){pan->Video->OpenSubs(NULL);pan->Video->Render();}
		}
	}else if(id==Undo){
		pan->Grid1->GetUndo(false);
	}else if(id==Redo){
		pan->Grid1->GetUndo(true);
	}else if(id==Search || id==FindReplace){
		if(FR && FR->IsShown() && FR->repl && id==FindReplace){FR->Hide();return;}
		FR= new findreplace(this, FR, id==FindReplace);
		FR->Show(true);
		FR->ReloadStyle();
	}else if(id==SelectLines){
		SL= new findreplace(this,SL,false,true);
		SL->Show(true);
	}else if(id==PlayPauseG){
		pan->Video->Pause();
	}else if(id==PreviousFrame||id==NextFrame){
		pan->Video->MovePos((id==PreviousFrame)? -1 : 1);
	}else if(id==SetStartTime||id==SetEndTime){
		if(pan->Video->GetState()!=None){
			if(id==SetStartTime){
				int time= pan->Video->Tell()-(pan->Video->avtpf/2.0f)+Options.GetInt("Offset of start time");
				pan->Grid1->SetStartTime(ZEROIT(time));
			}else{
				int time=pan->Video->Tell()+(pan->Video->avtpf/2.0f)+Options.GetInt("Offset of end time");
				pan->Grid1->SetEndTime(ZEROIT(time));
			}
		}
	}else if(id==VideoIndexing){
		MenuItem *Item=Menubar->FindItem(VideoIndexing);
		Options.SetBool("Index Video",Item->IsChecked());
	}else if(id>=OpenAudio&&id<=CloseAudio){
		OnOpenAudio(event);
	}else if(id==ASSProperties){
		OnAssProps();
	}else if(id==StyleManager){
		if(!ss){
			ss=new stylestore(this);
			int ww,hh;
			Options.GetCoords("Style Manager Position",&ww,&hh);

			ss->SetPosition(wxPoint(ww,hh));
		}
		ss->Store->Refresh(false);
		int chc=ss->Choice1->FindString(Options.acdir);
		ss->Choice1->SetSelection(chc);
		ss->LoadAssStyles();
		ss->Show();
	}else if(id==FontCollector && pan->Grid1->form<SRT){
		FontCollectorDialog fcd(this);
	}else if(id>=ConvertToASS && id<=ConvertToMPL2){
		OnConversion(( id - ConvertToASS ) + 1);
	}else if(id==HideTags){
		pan->Grid1->HideOver();
	}else if(id==ChangeTime){
		bool show=!pan->CTime->IsShown();
		Options.SetBool("Show Change Time",show);
		pan->CTime->Show(show);
		pan->BoxSizer1->Layout();
	}else if(id>6999&&id<7012){
		bool all=id<7006;
		int difid=(all)? 7000 : 7006;
		pan->Grid1->SortIt(id-difid,all);
	}else if(id>=ViewAll&&id<=ViewSubs){
		bool vidshow=( id==ViewAll || id==ViewVideo ) && pan->Video->GetState()!=None;
		bool vidvis=pan->Video->IsShown();
		if(!vidshow && pan->Video->GetState()==Playing){pan->Video->Pause();}
		pan->Video->Show(vidshow);
		if(vidshow && !vidvis){pan->Video->OpenSubs(pan->Grid1->SaveText());}
		if(pan->Edit->ABox){
			pan->Edit->ABox->Show((id==ViewAll||id==ViewAudio));
			if(id==ViewAudio){pan->Edit->SetMinSize(wxSize(500,350));}
		}	
		pan->Layout();
	}else if(id==AutoLoadScript){
		if(!Auto){Auto=new Auto::Automation();}
		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz sktypt"), 
			Options.GetString("Lua Recent Folder"),
			"", _("Pliki skryptów (*.lua),(*.moon)|*.lua;*.moon;"), wxFD_OPEN);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxString file=FileDialog1->GetPath();
			Options.SetString("Lua Recent Folder",file.AfterLast('\\'));
			if(Auto->Add(file)){Auto->BuildMenu(&AutoMenu);}

		}
		FileDialog1->Destroy();

	}else if(id==AutoReloadAutoload){
		if(!Auto){Auto=new Auto::Automation();}
		Auto->ReloadScripts();
		Auto->BuildMenu(&AutoMenu);
	}else if(id==SetVideoAtStart){
		int fsel=pan->Grid1->FirstSel();
		if(fsel<0){return;}
		pan->Video->Seek(MAX(0,pan->Grid1->GetDial(fsel)->Start.mstime),true);
	}else if(id==SetVideoAtEnd){
		int fsel=pan->Grid1->FirstSel();
		if(fsel<0){return;}
		pan->Video->Seek(MAX(0,pan->Grid1->GetDial(fsel)->End.mstime),false);
	}


}
//Stałe elementy menu które nie ulegają wyłączaniu
void kainoteFrame::OnMenuSelected1(wxCommandEvent& event)
{
	int id=event.GetId();
	int Modif = event.GetInt();
	TabPanel *pan=GetTab();
	//byte state[256];
	//if(GetKeyboardState(state)==FALSE){wxLogStatus(_("Nie można pobrać stanu klawiszy"));}
	if(Modif==wxMOD_SHIFT/*state[VK_LSHIFT]>1 || state[VK_RSHIFT]>1 *//*&& (state[VK_LCONTROL]<1 && state[VK_RCONTROL]<1 && state[VK_LMENU]<1 && state[VK_RMENU]<1)*/){
		MenuItem *item=Menubar->FindItem(id);
		wxString win[]={"Globalny"};
		int ret=-1;
		wxString name=item->GetLabelText();
		ret=Hkeys.OnMapHkey( id, name, this, win, 1);
		if(ret==-1){item->SetAccel(&Hkeys.GetHKey(id));Hkeys.SaveHkeys();SetAccels(false);}
		else if(ret>0){
			MenuItem *item=Menubar->FindItem(id);
			wxAcceleratorEntry entry;
			item->SetAccel(&entry);
			SetAccels(false);
		}

		return;
	}
	if(id==OpenSubs){
		if(SavePrompt(2)){return;}

		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik napisów"), 
			(GetTab()->VideoPath!="")? GetTab()->VideoPath.BeforeLast('\\') : (subsrec.size()>0)?subsrec[subsrec.size()-1].BeforeLast('\\') : "", 
			"", _("Pliki napisów (*.ass),(*.ssa),(*.srt),(*.sub),(*.txt)|*.ass;*.ssa;*.srt;*.sub;*.txt|Pliki wideo z wbudowanymi napisami (*.mkv)|*.mkv"), wxFD_OPEN);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxString file=FileDialog1->GetPath();
			if(file.AfterLast('.')=="mkv")
			{
				event.SetString(file);
				GetTab()->Grid1->OnMkvSubs(event);
			}
			else{
				OpenFile(file);
			}
		}
		FileDialog1->Destroy();
	}else if(id==OpenVideo){
		wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"), 
			(videorec.size()>0)? videorec[videorec.size()-1].BeforeLast('\\') : "", 
			"", _("Pliki wideo(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.mpg),(*.mpeg),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.mpg;*.mpeg;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"), wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, "wxFileDialog");
		if (FileDialog2->ShowModal() == wxID_OK){
			OpenFile(FileDialog2->GetPath());
		}
		FileDialog2->Destroy();
	}else if(id==Settings){
		OptionsDialog od(this,this);
		od.OptionsTree->ChangeSelection(0);
		od.ShowModal();
	}else if(id==SelectFromVideo){
		GetTab()->Grid1->SelVideoLine();
	}else if(id==Editor){
		HideEditor();
	}else if(id==Quit){
		Close();
	}else if(id==About){
		wxMessageBox(_("Edytor napisów by Bjakja aka Bakura, wersja ") + Options.progname.AfterFirst(' ') + "\r\n\r\n"+
			_("Ten program to jakby moje zaplecze do nauki C++, więc mogą zdarzyć się różne błędy.\r\n\r\n")+
			_("Kainote zawiera w sobie części następujących projeków:\r\n")+
			L"wxWidgets - Copyright © Julian Smart, Robert Roebling et al;\r\n"\
			L"Color picker, wymuxowywanie napsów z mkv, audiobox, audio player, część funkcji do lua\r\ni kilka innych pojedynczych funkcji wzięte z Aegisuba -\r\n"\
			L"Copyright © Rodrigo Braz Monteiro;\r\n"\
			L"Hunspell - Copyright © Kevin Hendricks;\r\n"\
			L"Matroska Parser - Copyright © Mike Matsnev;\r\n"\
			L"Interfejs CSRI - Copyright © David Lamparter;\r\n"\
			L"Vsfilter - Copyright © Gabest;\r\n"\
			L"FFMPEGSource2 - Copyright © Fredrik Mellbin;\r\n"\
			L"FreeType - Copyright ©  David Turner, Robert Wilhelm, and Werner Lemberg;\r\n"\
			L"Interfejs Avisynth - Copyright © Ben Rudiak-Gould et al.", "O Kainote");
	}else if(id==Helpers){
		wxString Testers=L"Ksenoform, Deadsoul, Zły los, Vessin, Xandros, Areki, Nyah2211, Waski_jestem.";
		wxString Credits=_("Pomoc graficzna: (przyciski, obrazki do pomocy itd.)\r\n")+
			_("- Archer (pierwsze przyciski do wideo).\r\n")+
			_("- Kostek00 (przyciski do audio).\r\n")+
			_("- Xandros (nowe przyciski do wideo).\r\n")+
			_("- Devilkan (ikony do menu i paska narzędzi, obrazki do pomocy).\r\n")+
			_("- Areki, duplex (tłumaczenie anglojęzyczne).\r\n \r\n")+
			_("Testerzy: (mniej i bardziej wprawieni użytkownicy programu)\r\n")+
			_("- Funki27 (pierwszy tester mający spory wpływ na obecne działanie programu\r\n")+
			_("i najbardziej narzekający na wszystko).\r\n")+
			_("- Sacredus (chyba pierwszy tłumacz używający trybu tłumaczenia,\r\n nieoceniona pomoc przy testowaniu wydajności na słabym komputerze).\r\n")+
			_("- Kostek00 (prawdziwy wynajdywacz błędów,\r\n")+
			_("miał duży wpływ na rozwój wykresu audio i głównego pola tekstowego).\r\n")+
			_("- Devilkan (crashhunter, ze względu na swój system i przyzwyczajenia wytropił już wiele crashy).\r\n \r\n")+
			_("Podziękowania także dla osób, które używają programu i zgłaszali błędy.\r\n");
		wxMessageBox(Credits+Testers,_("Lista osób pomocnych przy tworzeniu programu"));

	}else if(id==Help||id==ANSI){
		WinStruct<SHELLEXECUTEINFO> sei;
		wxString url=(id==Help)? L"file://" + Options.pathfull + L"\\Help\\Kainote pomoc.html" : L"http://animesub.info/forum/viewtopic.php?id=258715";
		sei.lpFile = url.c_str();
		sei.lpVerb = wxT("open");
		sei.nShow = SW_RESTORE;
		sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves

		ShellExecuteEx(&sei);
	}

}


wxString kainoteFrame::sftc()
{

	Stylelistbox slx(this);

	wxString styletext="";
	for (int j=0;j<GetTab()->Grid1->StylesSize();j++){
		Styles *acstyl=GetTab()->Grid1->GetStyle(j);
		slx.CheckListBox1->Append(acstyl->Name);

	}
	if(slx.ShowModal()==wxID_OK){

		for (size_t v=0;v<slx.CheckListBox1->GetCount();v++)
		{

			if(slx.CheckListBox1->IsChecked(v)){
				styletext<<slx.CheckListBox1->GetString(v)<<";";
			}
		}
	}

	return styletext.BeforeLast(';');
}

void kainoteFrame::OnConversion(char form)
{
	TabPanel *pan=GetTab();
	pan->Grid1->Convert(form);
	if(pan->Video->GetState()!=None){pan->Video->OpenSubs(pan->Grid1->SaveText());pan->Video->Render();}

	pan->CTime->Contents();
	UpdateToolbar();
	pan->Edit->HideControls();
}


void kainoteFrame::OnAssProps()
{
	int x=-1,y=-1;
	if(GetTab()->Video->GetState()!=None){GetTab()->Video->GetVideoSize(&x,&y);}
	ScriptInfo sci(this,x,y);
	Grid *ngrid=GetTab()->Grid1;
	sci.title->SetValue(ngrid->GetSInfo("Title"));
	sci.script->SetValue(ngrid->GetSInfo("Original Script"));
	sci.translation->SetValue(ngrid->GetSInfo("Original Translation"));
	sci.editing->SetValue(ngrid->GetSInfo("Original Editing"));
	sci.timing->SetValue(ngrid->GetSInfo("Original Timing"));
	sci.update->SetValue(ngrid->GetSInfo("Script Updated By"));
	wxString oldx=ngrid->GetSInfo("PlayResX");
	wxString oldy=ngrid->GetSInfo("PlayResY");
	int nx=wxAtoi(oldx);
	int ny=wxAtoi(oldy);
	if(nx<1 && ny<1){nx=384;ny=288;}
	else if(nx<1){nx=(float)ny*(4.0/3.0);if(ny==1024){nx=1280;}}
	else if(ny<1){ny=(float)nx*(3.0/4.0);if(nx==1280){ny=1024;}}
	sci.width->SetInt(nx);
	sci.height->SetInt(ny);
	wxString wraps=ngrid->GetSInfo("WrapStyle");
	int ws = wxAtoi(wraps);
	sci.wrapstyle->SetSelection(ws);
	wxString colls=ngrid->GetSInfo("Collisions");
	if(colls=="Reverse"){sci.collision->SetSelection(1);}
	wxString bords=ngrid->GetSInfo("ScaledBorderAndShadow");
	if (bords=="no"){sci.CheckBox2->SetValue(false);}

	if(sci.ShowModal()==wxID_OK)
	{
		int newx=sci.width->GetInt();
		int newy=sci.height->GetInt();
		if(newx<1 && newy<1){newx=384;newy=288;}
		else if(newx<1){newx=(float)newy*(4.0/3.0);}
		else if(newy<1){newy=(float)newx*(3.0/4.0);if(newx==1280){newy=1024;}}

		bool save=(!sci.CheckBox1->GetValue()&&(newx!=oldx||newy!=oldy));

		if(sci.title->GetValue()!=""){ if(sci.title->IsModified()){ngrid->AddSInfo("Title",sci.title->GetValue());} }
		else{ngrid->AddSInfo("Title","Kainote Ass File");}
		if(sci.script->IsModified()){ngrid->AddSInfo("Original Script",sci.script->GetValue());}
		if(sci.translation->IsModified()){ngrid->AddSInfo("Original Translation",sci.translation->GetValue());}
		if(sci.editing->IsModified()){ngrid->AddSInfo("Original Editing",sci.editing->GetValue());}
		if(sci.timing->IsModified()){ngrid->AddSInfo("Original Timing",sci.timing->GetValue());}
		if(sci.update->IsModified()){ngrid->AddSInfo("Script Updated By",sci.update->GetValue());}

		if(sci.width->IsModified()){ngrid->AddSInfo("PlayResX",wxString::Format("%i",newx));}
		if(sci.height->IsModified()){ngrid->AddSInfo("PlayResY",wxString::Format("%i",newy));}


		if(ws != sci.wrapstyle->GetSelection()){ngrid->AddSInfo("WrapStyle",wxString::Format("%i",sci.wrapstyle->GetSelection()));}
		wxString collis=(sci.collision->GetSelection()==0)?"Normal":"Reverse";
		if(colls!=collis){ngrid->AddSInfo("Collisions",collis);}
		wxString bordas=(sci.CheckBox2->GetValue())?"yes":"no";
		if(bords !=bordas){ ngrid->AddSInfo("ScaledBorderAndShadow",bordas);}


		if(save){
			int ox=wxAtoi(oldx);
			int oy=wxAtoi(oldy);
			ngrid->ResizeSubs((float)newx/(float)ox,(float)newy/(float)oy);}
		ngrid->SetModified(save);
	}
}

void kainoteFrame::Save(bool dial, int wtab)
{
	TabPanel* atab=(wtab<0)? GetTab() : Tabs->Page(wtab);
	if(atab->Grid1->origform!=atab->Grid1->form
		||(Options.GetBool("Subs Autonaming") && atab->SubsName.BeforeLast('.') != atab->VideoName.BeforeLast('.') && atab->VideoName!="")
		||atab->SubsPath=="" || dial)
	{
		wxString extens=_("Plik napisów ");

		if (atab->Grid1->form<SRT){extens+="(*.ass)|*.ass";}
		else if (atab->Grid1->form==SRT){extens+="(*.srt)|*.srt";}
		else{extens+="(*.txt, *.sub)|*.txt;*.sub";};

		wxString path=(atab->VideoPath!="" && Options.GetBool("Subs Autonaming"))? atab->VideoPath : atab->SubsPath;
		wxString name=path.BeforeLast('.');
		path=path.BeforeLast('\\');

		wxWindow *_parent=(atab->Video->isfullskreen)? (wxWindow*)atab->Video->TD : this;
		wxFileDialog saveFileDialog(_parent, _("Zapisz plik napisów"), 
			path, name ,extens, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_OK){

			atab->SubsPath = saveFileDialog.GetPath();
			wxString ext=(atab->Grid1->form<SRT)? "ass" : (atab->Grid1->form==SRT)? "srt" : "txt";
			if(!atab->SubsPath.EndsWith(ext)){atab->SubsPath<<"."<<ext;}
			atab->SubsName=atab->SubsPath.AfterLast('\\');
			SetRecent();
		}else{return;}
	}
	atab->Grid1->SaveFile(atab->SubsPath);
	atab->Grid1->Modified=false;
	atab->Grid1->origform=atab->Grid1->form;
	Label(0,false,wtab);

	wxBell();
}

bool kainoteFrame::OpenFile(wxString filename,bool fulls)
{
	wxString ext=filename.Right(3).Lower();
	if(ext=="exe"||ext=="zip"||ext=="rar"||ext=="7z"){return false;}
	if(ext=="lua" || ext == "moon"){if(!Auto){Auto=new Auto::Automation();}Auto->Add(filename);return true;}
	TabPanel *pan=GetTab();
	
	bool found=false;
	bool nonewtab = true;
	wxString fntmp="";
	bool issubs=(ext=="ass"||ext=="txt"||ext=="sub"||ext=="srt"||ext=="ssa");
	
	if(pan->edytor && !(issubs&&pan->VideoPath.BeforeLast('.')==filename.BeforeLast('.'))
		&&!(!issubs&&pan->SubsPath.BeforeLast('.')==filename.BeforeLast('.'))){
			fntmp= FindFile(filename,issubs,!(fulls || pan->Video->isfullskreen) );
			if(fntmp!=""){found=true;if(!issubs){ext=fntmp.AfterLast('.');}}
	}
	pan->Freeze();
	if(Options.GetBool("Open In New Card")&&pan->SubsPath!=""&&
		!pan->Video->isfullskreen&&issubs){
			Tabs->AddPage(true);pan=Tabs->Page(Tabs->Size()-1);
			nonewtab = false;
	}

	if(issubs||found){  
		wxString fname=(found&&!issubs)?fntmp:filename;
		if(nonewtab){if(SavePrompt(2)){pan->Thaw();return true;}}
		OpenWrite ow; 
		wxString s=ow.FileOpen(fname);
		if(s==""){pan->Thaw();return false;}
		pan->Grid1->Loadfile(s,ext);

		if(ext=="ssa"){ext="ass";fname=fname.BeforeLast('.')+".ass";}
		pan->SubsPath=fname;
		pan->SubsName=fname.AfterLast('\\');


		if(ext=="ass"){
			wxString katal=pan->Grid1->GetSInfo("Last Style Storage");

			if(katal!=""){
				for(size_t i=0;i<Options.dirs.size();i++)
				{
					if(katal==Options.dirs[i]){
						Options.LoadStyles(katal);
						if(ss){ss->Store->SetSelection(0,true);
						int chc=ss->Choice1->FindString(Options.acdir);
						ss->Choice1->SetSelection(chc);
						}
					}
				}
			}
		}

		if(pan->Video->GetState()!=None && !found){
			bool isgood=pan->Video->OpenSubs((pan->edytor)? pan->Grid1->SaveText() : 0);
			if(!isgood){wxMessageBox(_("Otwieranie napisów nie powiodło się"), "Uwaga");}
		}
		SetRecent();


		Label();

		if(!pan->edytor && !fulls && !pan->Video->isfullskreen){HideEditor();}
		if(!found){pan->CTime->Contents();UpdateToolbar(); pan->Thaw();return true;}
	}

	wxString fnname=(found && issubs)?fntmp:filename;

	bool isload=pan->Video->Load(fnname,pan->Grid1->SaveText(),fulls);


	if(!isload){
		pan->Thaw();return isload;
	}else{
		pan->CTime->Contents();
		pan->Video->seekfiles=true;
		pan->Edit->Frames->Enable(!pan->Video->IsDshow);
		pan->Edit->Times->Enable(!pan->Video->IsDshow);
		UpdateToolbar();
		//pan->Grid1->SetFocus();
	}
	pan->Thaw();

	Options.SaveOptions(true, false);


	return true;  
}



//0 - subs, 1 - vids, 2 - auds
void kainoteFrame::SetRecent(short what)
{
	int idd=30000+(20*what);
	Menu *wmenu=(what==0)?SubsRecMenu : (what==1)? VidsRecMenu : AudsRecMenu;
	int size= (what==0)?subsrec.size() : (what==1)? videorec.size() : audsrec.size();
	wxArrayString recs=(what==0)?subsrec : (what==1)? videorec : audsrec;
	wxString path=(what==0)?GetTab()->SubsPath : (what==1)? GetTab()->VideoPath : GetTab()->Edit->ABox->audioName;

	for(int i=0;i<size;i++){
		if(recs[i]==path){
			recs.erase(recs.begin()+i);
			break;
		}
	}
	recs.Add(path);
	if(recs.size()>20){recs.erase(recs.begin());}
	if(what==0){subsrec=recs; Options.SetTable("Subs Recent",recs);}
	else if(what==1){videorec=recs; Options.SetTable("Video Recent",recs);}
	else{audsrec=recs; Options.SetTable("Recent Audio",recs);}
}

//0 - subs, 1 - vids, 2 - auds
void kainoteFrame::AppendRecent(short what,Menu *_Menu)
{
	Menu *wmenu;
	if(_Menu){
		wmenu=_Menu;
	}else{
		wmenu=(what==0)?SubsRecMenu : (what==1)? VidsRecMenu : AudsRecMenu;
	}
	int idd=30000+(20*what);
	int size= (what==0)?subsrec.size() : (what==1)? videorec.size() : audsrec.size();

	wxArrayString recs=(what==0)?subsrec : (what==1)? videorec : audsrec;
	//wxLogStatus("count %i", wmenu->GetMenuItemCount());
	for(int j=wmenu->GetMenuItemCount()-1; j>=0; j--){
		//wxLogStatus("deleted %i", j);
		wmenu->Destroy(wmenu->FindItemByPosition(j));
	}

	for(int i=0;i<size;i++)
	{
		if(!wxFileExists(recs[i])){continue;}
		MenuItem* MI= new MenuItem(idd+i, recs[i].AfterLast('\\'), _("Otwórz ")+recs[i]);
		wmenu->Append(MI);
	}

	if(!wmenu->GetMenuItemCount()){
		MenuItem* MI= new MenuItem(idd, _("Brak"));
		MI->Enable(false);
		wmenu->Append(MI);
	}
}

void kainoteFrame::OnRecent(wxCommandEvent& event)
{
	int id=event.GetId();
	int Modif=event.GetInt();
	MenuItem* MI=0;
	if(id<30020){
		MI=SubsRecMenu->FindItem(id);
	}else if(id<30040){
		MI=VidsRecMenu->FindItem(id);
	}else{
		MI=AudsRecMenu->FindItem(id);
	}
	if(!MI){wxLogStatus("Item Menu Przepadł");return;}
	wxString filename=MI->GetHelp().AfterFirst(' ');
	//wxLogStatus("onrecent %i %i" + MI->GetHelp() + MI->GetLabel(), MI->GetId(), Modif);
	//return;
	//byte state[256];
	//if(GetKeyboardState(state)==FALSE){wxLogStatus(_("Nie można pobrać stanu klawiszy"));}
	if(Modif==wxMOD_CONTROL){
		wxWCharBuffer buf=filename.BeforeLast('\\').c_str();
		/*wchar_t **cmdline = new wchar_t*[3];
		cmdline[0] = L"explorer";
		cmdline[1] = buf.data();
		cmdline[2] = 0;
		long res = wxExecute(cmdline);
		delete cmdline;*/
		WinStruct<SHELLEXECUTEINFO> sei;
		sei.lpFile = buf;
		sei.lpVerb = wxT("explore");
		sei.nShow = SW_RESTORE;
		sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves


		if(!ShellExecuteEx(&sei)){wxLogStatus(_("Nie można otworzyć folderu"));}
		return;
	}
	if(id<30040){
		OpenFile(filename);
	}else{
		event.SetString(filename);
		OnOpenAudio(event);
	}
}



wxString kainoteFrame::FindFile(wxString fn,bool video,bool prompt)
{
	wxString filespec;
	wxString path=fn.BeforeLast('\\',&filespec);
	wxArrayString pliki;

	wxString plik="";

	wxDir kat(path);
	if(kat.IsOpened()){
		kat.GetAllFiles(path,&pliki,filespec.BeforeLast('.')+".*", wxDIR_FILES);
	} 
	if(pliki.size()<2){return "";}

	for(int i=0;i<(int)pliki.size();i++){
		wxString ext=pliki[i].AfterLast('.');
		if((!video&&(ext!="ass"&&ext!="txt"&&ext!="sub"&&ext!="srt"&&ext!="ssa"))
			||(video&&(ext!="avi"&&ext!="mp4"&&ext!="mkv"&&ext!="ogm"&&ext!="wmv"&&ext!="asf"&&ext!="rmvb"
			&&ext!="rm"&&ext!="3gp"&&ext!="ts"&&ext!="m2ts"&&ext!="m4v"&&ext!="flv"))  //&&ext!="avs" przynajmniej do momentu dorobienia obsługi przez avisynth
			){ }else{plik=pliki[i];break;}
	}
	if(plik!=""&&prompt){
		if (wxMessageBox(wxString::Format(_("Wczytać %s o nazwie %s?"), 
			(video)? _("wideo") :_("napisy"), plik.AfterLast('\\')),
			_("Potwierdzenie"), wxICON_QUESTION | wxYES_NO, this) == wxNO ){plik="";} 
	}
	pliki.clear();
	return plik;
}


void kainoteFrame::OnP5Sec(wxCommandEvent& event)
{
	GetTab()->Video->Seek(GetTab()->Video->Tell()+5000);
}

void kainoteFrame::OnM5Sec(wxCommandEvent& event)
{
	GetTab()->Video->Seek(GetTab()->Video->Tell()-5000);

}

TabPanel* kainoteFrame::GetTab()
{
	return Tabs->GetPage();
}

void kainoteFrame::Label(int iter,bool video, int wtab)
{
	wxString whiter;
	if(iter>0){whiter<<iter<<"*";}
	TabPanel* atab=(wtab<0)? GetTab() : Tabs->Page(wtab);

	/*MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	int div=1024;
	int availmem=statex.ullAvailVirtual/div;
	int totalmem=statex.ullTotalVirtual/div;
	wxString memtxt= wxString::Format(" RAM: %i KB / %i KB", totalmem-availmem, totalmem);*/
	wxString name=(video)?atab->VideoName : atab->SubsName;
	SetLabel(whiter+name+" - "+Options.progname /*+ memtxt*/);
	if(name.Len()>35){name=name.SubString(0,35)+"...";}
	Tabs->SetPageText((wtab<0)? Tabs->GetSelection() : wtab, whiter+name);
}

void kainoteFrame::SetAccels(bool _all)
{
	std::vector<wxAcceleratorEntry> entries;
	entries.resize(2);
	entries[0].Set(wxACCEL_CTRL, (int) 'T', ID_ADDPAGE);
	entries[1].Set(wxACCEL_CTRL, (int) 'W', ID_CLOSEPAGE);

	for(auto cur=Hkeys.hkeys.rbegin(); cur!=Hkeys.hkeys.rend(); cur++){
		int id=cur->first;
		if(cur->second.Accel==""){continue;}
		if(id>=6850){
			entries.push_back(Hkeys.GetHKey(id));
		}else if(id>6000){
			MenuItem *item=Menubar->FindItem(id);
			if(!item){wxLogStatus("no id %i", id); continue;}
			cur->second.Name=item->GetLabelText();
			wxAcceleratorEntry accel = Hkeys.GetHKey(id);
			item->SetAccel(&accel);
			entries.push_back(accel);
		}else if(id<6001){break;}

	}
	//Menubar->SetAccelerators();
	wxAcceleratorTable accel(entries.size(), &entries[0]);
	Tabs->SetAcceleratorTable(accel);

	if(!_all){return;}
	for(size_t i=0;i<Tabs->Size();i++)
	{
		Tabs->Page(i)->SetAccels();
	}
}


void kainoteFrame::InsertTab(bool sel)
{
	Tabs->AddPage(sel);
}

bool comp(wxString first, wxString second)
{
	return (first.CmpNoCase(second)<0);
}

void kainoteFrame::OpenFiles(wxArrayString files,bool intab, bool nofreeze, bool newtab)
{
	std::sort(files.begin(),files.end(),comp);
	wxArrayString subs;
	wxArrayString videos;
	for(size_t i=0;i<files.size();i++){
		wxString ext=files[i].AfterLast('.').Lower();
		if(ext=="ass"||ext=="ssa"||ext=="txt"||ext=="srt"||ext=="sub"){
			subs.Add(files[i]);
		}else if(ext=="lua" || ext=="moon"){
			if(!Auto){Auto=new Auto::Automation();}
			Auto->Add(files[i]);
		}
		else if(ext!="exe" && ext!="zip" && ext!="rar" && ext!="7z"){
			videos.Add(files[i]);
		}

	}

	if(files.size()==1){
		OpenFile(files[0],(videos.size()==1&&Options.GetBool("Video Fullskreen on Start")));
		videos.Clear();subs.Clear();files.Clear();
		return;
	}

	Freeze();
	GetTab()->Hide();
	size_t maxx=(subs.size()>videos.size())?subs.size() : videos.size();

	for(size_t i=0;i<maxx;i++)
	{

		if((i>=Tabs->Size() || Tabs->Page(Tabs->iter)->SubsPath!="" ||
			Tabs->Page(Tabs->iter)->VideoPath!="") && !intab){
			InsertTab(false);
		}
		TabPanel *pan=GetTab();
		if(i<subs.size()){
			wxString ext=subs[i].AfterLast('.').Lower();
			OpenWrite ow;
			wxString s=ow.FileOpen(subs[i]);
			if(s==""){break;
			}else{
				pan->Grid1->Loadfile(s,ext);
			}

			if(ext=="ssa"){ext="ass";subs[i]=subs[i].BeforeLast('.')+".ass";}
			pan->SubsPath=subs[i];
			pan->SubsName=pan->SubsPath.AfterLast('\\');
			if(!pan->edytor){HideEditor();}
			SetRecent();

			Label();
		}
		if(i<videos.size()){
			//wxLogStatus("Video bload %i", i);
			bool isload=pan->Video->Load(videos[i],(pan->edytor)? pan->Grid1->SaveText() : 0);
			//wxLogStatus("Video aload %i %i", i, (int)isload);


			if(!isload){
				if(pan->Video->IsDshow){wxMessageBox(_("Plik nie jest poprawnym plikiem wideo albo jest uszkodzony,\r\nbądź brakuje kodeków czy też splittera"), _("Uwaga"));}
				break;
			}
			pan->Edit->Frames->Enable(!pan->Video->IsDshow);
			pan->Edit->Times->Enable(!pan->Video->IsDshow);

			pan->Video->seekfiles=true;

		}
		pan->CTime->Contents();

	}

	Thaw();
	//taka kolejność naprawia błąd znikających zakładek
	int w,h;
	Tabs->GetClientSize(&w,&h);
	Tabs->RefreshRect(wxRect(0,h-25,w,25),false);
	GetTab()->Show();
	UpdateToolbar();

	files.Clear();
	subs.Clear();
	videos.Clear();

	Options.SaveOptions(true, false);
}

void kainoteFrame::OnPageChange(wxCommandEvent& event)
{
	int step=(event.GetId()==NextTab)? Tabs->iter+1 : Tabs->iter-1;
	if(step<0){step=Tabs->Size()-1;}
	else if(step>=(int)Tabs->Size()){step=0;}
	Tabs->ChangePage(step);
}


void kainoteFrame::OnPageChanged(wxCommandEvent& event)
{
	wxString whiter;
	TabPanel *cur=Tabs->GetPage();
	int iter=cur->Grid1->file->Iter();
	if(iter>0 && cur->Grid1->Modified){whiter<<iter<<"*";}
	wxString name=(!cur->edytor)? cur->VideoName : cur->SubsName;
	SetLabel(whiter+name+" - "+Options.progname);

	if(cur->Video->GetState()!=None){
		SetStatusText(getfloat(cur->Video->fps)+" FPS",2);
		wxString tar;
		tar<<cur->Video->ax<<" : "<<cur->Video->ay;
		SetStatusText(tar,4);
		int x, y;
		cur->Video->GetVideoSize(&x, &y);
		tar.Empty();
		tar<<x<<" x "<<y;
		SetStatusText(tar,3);
		cur->Video->displaytime();

		STime kkk1;
		kkk1.mstime=cur->Video->GetDuration();
		SetStatusText(kkk1.raw(SRT),1);
		if(cur->edytor){SetStatusText(cur->VideoName,5);}
		else{SetStatusText("",5);}
	}else{SetStatusText("",5);SetStatusText("",4);SetStatusText("",3);SetStatusText("",2);SetStatusText("",1);}

	if(cur->edytor){cur->Grid1->SetFocus();}else{cur->Video->SetFocus();}
	cur->Grid1->UpdateUR(false);

	UpdateToolbar();

	cur->Grid1->SetFocus();
	if(Tabs->iter!=Tabs->GetOldSelection()){
		cur->CTime->RefVals(Tabs->Page( Tabs->GetOldSelection() )->CTime);}

	if(Options.GetBool("Auto Select Lines")){
		Grid *old=Tabs->Page(Tabs->GetOldSelection())->Grid1;
		if(old->FirstSel()>-1){
			cur->Grid1->SelVideoLine(old->GetDial(old->FirstSel())->Start.mstime);
		}
	}
	if(ss && ss->IsShown()){ss->LoadAssStyles();}
	if(FR){FR->Reset();FR->ReloadStyle();}
}

void kainoteFrame::HideEditor()
{
	TabPanel *cur=GetTab();
	//if(cur->edytor){
	//wxCommandEvent evt;
	//evt.SetInt(5);
	//OnUnSubs(evt);
	//if(evt.GetInt()==-1){return;}
	//}

	cur->edytor = !cur->edytor;

	cur->Grid1->Show(cur->edytor);

	cur->Edit->Show(cur->edytor);

	if(cur->edytor){//Załączanie Edytora

		cur->BoxSizer1->Detach(cur->Video);
		cur->BoxSizer2->Prepend(cur->Video, 0, wxEXPAND|wxALIGN_TOP, 0);
		cur->BoxSizer1->InsertSpacer(1,3);
		if(cur->Video->GetState()!=None&&!cur->Video->isfullskreen){
			int sx,sy,vw,vh;
			Options.GetCoords("Video Window Size",&vw,&vh);
			if(vh<350){vh=350,vw=500;}
			cur->Video->CalcSize(&sx,&sy,vw,vh);
			cur->Video->SetMinSize(wxSize(sx,sy + cur->Video->panelHeight));
		}else{cur->Video->Hide();}
		if(Options.GetBool("Show Change Time")){
			cur->CTime->Show();}
		cur->BoxSizer1->Layout();
		Label();
		if(cur->Video->GetState()!=None){cur->Video->ChangeVobsub();}
	}
	else{//Wyłączanie edytora

		cur->CTime->Hide();

		cur->BoxSizer1->Remove(1);

		if(!cur->Video->IsShown()){cur->Video->Show();}

		cur->BoxSizer2->Detach(cur->Video);

		cur->BoxSizer1->Add(cur->Video, 1, wxEXPAND|wxALIGN_TOP, 0);

		if(cur->Video->GetState()!=None&&!cur->Video->isfullskreen&&!IsMaximized()){
			int sx,sy,sizex,sizey;
			GetClientSize(&sizex,&sizey);

			cur->Video->CalcSize(&sx,&sy,sizex,sizey);

			SetClientSize(sx+iconsize,sy + cur->Video->panelHeight + Tabs->GetHeight());

		}
		cur->Video->SetFocus();

		cur->BoxSizer1->Layout();

		if(cur->VideoName!=""){Label(0,true);}
		if(cur->Video->GetState()!=None){cur->Video->ChangeVobsub(true);}
	}
	UpdateToolbar();
}

void kainoteFrame::OnPageAdd(wxCommandEvent& event)
{
	InsertTab();
}

void kainoteFrame::OnPageClose(wxCommandEvent& event)
{
	Tabs->DeletePage(Tabs->GetSelection());
	OnPageChanged(event);
}

void kainoteFrame::AppendBitmap(Menu *menu, int id, wxString text, wxString help, wxBitmap bitmap, bool enable, Menu *SubMenu)
{
	wxBitmap *bmp = NULL;
	if(bitmap.IsOk()){bmp = new wxBitmap(bitmap); if(id!=ID_CONV){Toolbar->AddID(id);}}
	menu->Append(id, text, help, enable, bmp, SubMenu);
}

void kainoteFrame::SaveAll()
{
	for(size_t i=0;i<Tabs->Size();i++)
	{
		if(!Tabs->Page(i)->Grid1->Modified){continue;}
		Save(false,i);
		Label(0,false,i);
	}

}

//return anulowanie operacji
bool kainoteFrame::SavePrompt(char mode, int wtab)
{
	TabPanel* atab=(wtab<0)? GetTab() : Tabs->Page(wtab);	
	if(atab->Grid1->file->Iter()>0 && atab->Grid1->Modified){
		wxWindow *_parent=(atab->Video->isfullskreen)? (wxWindow*)atab->Video->TD : this;
		int answer = wxMessageBox(wxString::Format(_("Zapisać napisy o nazwie \"%s\" przed %s?"), 
			atab->SubsName, (mode==0)? _("zamknięciem programu") :
			(mode==1)? _("zamknięciem zakładki") : 
			(mode==2)? _("wczytaniem nowych napisów") : 
			_("usunięciem napisów")), 
			_("Potwierdzenie"), wxYES_NO | wxCANCEL, _parent);
		if(answer==wxCANCEL){return true;}
		if(answer==wxYES){
			Save(false, wtab);
		}
	}
	return false;
}


void kainoteFrame::UpdateToolbar()
{//disejblowanie rzeczy niepotrzebnych przy wyłączonym edytorze
	MenuEvent evt;
	OnMenuOpened(evt);
	Toolbar->Updatetoolbar();
}

void kainoteFrame::OnOpenAudio(wxCommandEvent& event)
{
	OpenAudioInTab(GetTab(), event.GetId(), event.GetString());
}

void kainoteFrame::OpenAudioInTab(TabPanel *pan, int id, const wxString &path)
{

	if(id==CloseAudio && pan->Edit->ABox){
		pan->Video->player=NULL; 
		pan->Edit->ABox->Destroy(); 
		pan->Edit->ABox=NULL; 
		pan->Edit->Layout();}
	else{

		if(!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){wxMessageBox(_("Dupa blada, skróty klawiszowe się nie wczytały, na audio nie podziałasz"), _("Błędny błąd"));return;}
		if(!Options.AudioOpts && !Options.LoadAudioOpts()){wxMessageBox(_("Dupa blada, opcje się nie wczytały, na audio nie podziałasz"), _("Błędny błąd"));return;}

		wxString Path;
		if(id==OpenAudio){
			wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik audio"), 
				(pan->VideoPath!="")? pan->VideoPath.BeforeLast('\\') : (videorec.size()>0)?subsrec[videorec.size()-1].BeforeLast('\\') : "", 
				"", _("Pliki audio i wideo (*.wav),(*.w64),(*.aac),(*.mp3),(*.mp4),(*.mkv),(*.avi)|*.wav;*.w64;*.aac;*.mp3;*.mp4;*.mkv;*.avi|Wszystkie Pliki |*.*"), wxFD_OPEN);
			int result = FileDialog1->ShowModal();
			if (result == wxID_OK){
				Path=FileDialog1->GetPath();
			}
			FileDialog1->Destroy();
			if(result == wxID_CANCEL){return;}
		}
		if(id>30039){Path=path;}
		if(Path.IsEmpty()){Path=pan->VideoPath;}
		if(Path.IsEmpty()){return;}


		if(pan->Edit->ABox){pan->Edit->ABox->SetFile(Path,(id==40000));
		if(!pan->Edit->ABox->audioDisplay->loaded){
			pan->Edit->ABox->Destroy(); 
			pan->Edit->ABox=NULL;
		}else{SetRecent(2);}
		}
		else{
			pan->Edit->ABox=new AudioBox(pan->Edit, pan->Grid1);
			pan->Edit->ABox->SetFile(Path, (id==40000));

			if(pan->Edit->ABox->audioDisplay->loaded){
				pan->Edit->BoxSizer1->Prepend(pan->Edit->ABox, 4, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 2);
				//int sizew,sizeh;
				//Options.GetCoords("Video Window Size",&sizew,&sizeh);
				if (!pan->Video->IsShown()){
					pan->Edit->SetMinSize(wxSize(500,350));
				}
				pan->Layout();
				Tabs->Refresh(false);
				pan->Edit->ABox->audioDisplay->SetFocus();
				SetRecent(2);
			}
			else{pan->Edit->ABox->Destroy(); pan->Edit->ABox=NULL;}
		}
	}
}



//uses before menu is shown
void kainoteFrame::OnMenuOpened(MenuEvent& event)
{
	Menu *curMenu = event.GetMenu();
	//wxLogStatus("opened " + curMenu->GetTitle());

	if(curMenu==FileMenu)
	{
		AppendRecent();
	}
	else if(curMenu==VidMenu)
	{
		AppendRecent(1);
	}
	else if(curMenu==AudMenu)
	{
		AppendRecent(2);
	}
	else if(curMenu==AutoMenu)
	{
		if(!Auto){Auto=new Auto::Automation();}
		Auto->BuildMenu(&AutoMenu);
		SetAccels();
	}
	TabPanel *pan = GetTab();
	bool enable=(pan->Video->GetState()!=None);
	bool edytor=pan->edytor;
	for(int i = PlayPauseG; i<=SetVideoAtEnd; i++)
	{
		Menubar->Enable(i,(i<SetStartTime)? enable : enable && edytor);
	}
	//kolejno numery id
	bool forms;
	char form = pan->Grid1->form;
	bool tlmode = pan->Grid1->transl;
	bool editor = pan->edytor;
	for(int i=SaveSubs;i<=ViewSubs;i++){//po kolejne idy zajrzyj do enuma z pliku h, ostatni jest Automation
		forms=true;

		if(i>=ASSProperties&&i<ConvertToASS){forms=form<SRT;}//menager stylów i sinfo
		else if(i==ConvertToASS){forms=form>ASS;}//konwersja na ass
		else if(i==ConvertToSRT){forms=form!=SRT;}//konwersja na srt
		else if(i==ConvertToMDVD){forms=form!=MDVD;}//konwersja na mdvd
		else if(i==ConvertToMPL2){forms=form!=MPL2;}//konwersja na mpl2
		else if(i==ConvertToTMP){forms=form!=TMP;}//konwersja na tmp
		if((i>=ConvertToASS && i<=ConvertToTMP) && tlmode){forms=false;}
		//if(i>=CrossPositioner && i<=MoveAll){forms= pan->Video->IsShown() && form<SRT && ( i - CrossPositioner ) != pan->Edit->Visual;}
		if(i==ViewAudio || i==CloseAudio){forms= pan->Edit->ABox!=0;}
		if(i==ViewVideo||i==AudioFromVideo){forms= pan->Video->GetState()!=None;}
		if(i==SaveTranslation){forms=tlmode;}
		Menubar->Enable(i, editor && forms);
	}
	//specjalna poprawka do zapisywania w trybie tłumaczenia, jeśli jest tlmode, to zawsze ma działać.
	pan->Edit->TlMode->Enable((editor && form==ASS && pan->SubsPath!=""));
	//wxLogStatus("opened end " + curMenu->GetTitle());
	//if(curMenu){Menubar->ShowMenu();}
}

//void kainoteFrame::OnMenuClick(wxCommandEvent &event)
//{
//	wxLogStatus("menu click %i", event.GetId());
//	auto action =  Auto->Actions.find(event.GetId());
//	if(action!=Auto->Actions.end()){
//		action->second.Run();
//	}
//}

//void kainoteFrame::OnRunScript(wxCommandEvent& event)
//{
//	int id =event.GetId();
//	
//	wxString wscript=Hkeys.hkeys[id].Name;
//	int line, macro;
//	wscript=wscript.AfterFirst(' ');
//	line=wxAtoi(wscript.BeforeFirst('-'));
//	macro=wxAtoi(wscript.AfterFirst('-'));
//	if(!Auto){Auto = new Auto::Automation();}
//	if(line>=(int)Auto->Scripts.size()){wxMessageBox(wxString::Format(_("Brak wczytanego skryptu o numerze %i"),line));}
//	Auto::LuaScript *scr=Auto->Scripts[line];
//	auto macros=scr->GetMacros();
//	if((int)macros.size()<=macro){
//		wxString msg;
//		if(scr->GetLoadedState()){msg = wxString::Format(_("Skrypt o nazwie \"%s\" nie posiada makra %s."), scr->GetName(), macro);}
//		else{msg=scr->GetDescription();}
//		wxMessageBox(msg); return;
//	}
//	Auto->RunScript(line, macro);
//
//}


void kainoteFrame::OnChangeLine(wxCommandEvent& event)
{

	int idd=event.GetId();
	if(idd<JoinWithPrevious){//zmiana linijki
		GetTab()->Grid1->NextLine((idd==PreviousLine)?-1 : 1);}
	else{//scalanie dwóch linijek
		GetTab()->Grid1->OnJoin(event);
	}
}

void kainoteFrame::OnDelete(wxCommandEvent& event)
{
	int idd=event.GetId();
	if(idd==Remove){
		GetTab()->Grid1->DeleteRows();
	}
	else{
		GetTab()->Grid1->DeleteText();
	}

}

void kainoteFrame::OnClose1(wxCloseEvent& event)
{
	size_t curit=Tabs->iter;
	for(size_t i=0;i<Tabs->Size();i++)
	{
		Tabs->iter=i;
		if(SavePrompt(0)){Tabs->iter=curit;return;}
	}
	Tabs->iter=curit;
	event.Skip();
}

void kainoteFrame::OnAudioSnap(wxCommandEvent& event)
{
	TabPanel *pan=GetTab();
	if(!pan->Edit->ABox){return;}
	int id=event.GetId();
	int time= (id==SnapWithStart)? pan->Edit->line->Start.mstime : pan->Edit->line->End.mstime;
	int time2= (id==SnapWithStart)? pan->Edit->line->End.mstime : pan->Edit->line->Start.mstime;
	int snaptime= pan->Edit->ABox->audioDisplay->GetBoundarySnap(time,1000,!Options.GetBool("Audio Snap To Keyframes"),(id==SnapWithStart),true);
	//wxLogStatus(" times %i %i", snaptime, time);
	if(time!= snaptime){
		if(id==SnapWithStart){
			if (snaptime>=time2){return;}
			pan->Edit->StartEdit->SetTime(STime(snaptime));
			pan->Edit->StartEdit->SetModified(true);
		}
		else{
			if (snaptime<=time2){return;}
			pan->Edit->EndEdit->SetTime(STime(snaptime));
			pan->Edit->EndEdit->SetModified(true);
		}
		pan->Edit->Send(false);
		pan->Edit->ABox->audioDisplay->SetDialogue(pan->Edit->line,pan->Edit->ebrow);
	}
}

//void kainoteFrame::OnOpenVideo(wxCommandEvent& event)
//{
//	event.SetInt((int)OpenFile(event.GetString()));
//}

void kainoteFrame::OnOutofMemory()
{
	TabPanel *pan = Notebook::GetTab();

	if(pan->Grid1->file->maxx()>3){
		pan->Grid1->file->RemoveFirst(2);
		wxLogStatus(_("Zabrakło pamięci RAM, usunięto część historii"));
		return;
	}else if(Notebook::GetTabs()->Size()>1){
		for(size_t i=0; i<Notebook::GetTabs()->Size(); i++)
		{
			if( i!= Notebook::GetTabs()->GetSelection()){
				if(Notebook::GetTabs()->Page(i)->Grid1->file->maxx()>3){
					Notebook::GetTabs()->Page(i)->Grid1->file->RemoveFirst(2);
					wxLogStatus(_("Zabrakło pamięci RAM, usunięto część historii"));
					return;
				}
			}
		}
	}

	std::exit(1);
}


DEFINE_ENUM(Id,IDS)