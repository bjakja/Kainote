/***************************************************************
* Name:      kainoteMain.cpp
* Purpose:   Code for Application Frame
* Author:    Bjakja (bjakja@op.pl)
* Created:   2012-04-23
* Copyright: Bjakja (www.costam.com)
* License:
**************************************************************/


#include "kainoteMain.h"

#include "timeconv.h"
#include "Stylelistbox.h"
#include "ScriptInfo.h"
#include "config.h"
#include "OptionsDialog.h"
#include "DropFiles.h"
#include "OpennWrite.h"
#include "Hotkeys.h"
#include "FontCollector.h"
#include <wx/accel.h>
#include <wx/dir.h>
#include <wx/sysopt.h>


#undef IsMaximized
#if _DEBUG
#define logging 5
#endif
//#define wxIMAGE_PNG(x) wxImage(wxS(#x),wxBITMAP_TYPE_PNG_RESOURCE)

kainoteFrame::kainoteFrame(wxWindow* parent)
	//: wxFrame(parent, id, _T("Bez nazwy -- ")+Options.progname, wxDefaultPosition,wxDefaultSize, wxDEFAULT_FRAME_STYLE)
{
	int isgood=Options.LoadOptions();
	if(!isgood){wxMessageBox(_T("Nie uda³o siê wczytaæ opcji dzia³anie programu zostanie zakoñczone"),_T("Uwaga"));Close();}
	isgood=Hkeys.LoadHkeys();
	if(!isgood){wxMessageBox(_T("Nie uda³o siê wczytaæ hotkeyów dzia³anie programu zostanie zakoñczone"),_T("Uwaga"));Close();}

#if logging
	mylog=new wxLogWindow(this, "logi",true, false);
	mylog->PassMessages(true);
#else
	mylog=NULL;
#endif
	SC=NULL;
	LSD=NULL;
	ss=NULL;
	FR=NULL;
	SL=NULL;
	Auto=NULL;
	/*fontlastmodif=-1;
	fontlastmodifl=-1;*/
	subsrec=Options.GetTable("Subs Recent");
	videorec=Options.GetTable("Video Recent");
	audsrec=Options.GetTable("Recent Audio");
	Create(parent, -1, _T("Bez nazwy -- ")+Options.progname, wxDefaultPosition,wxDefaultSize, wxDEFAULT_FRAME_STYLE);
	wxFont thisFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
	SetFont(thisFont);
	wxIcon kaiicon("aaaa",wxBITMAP_TYPE_ICO_RESOURCE); 
	SetIcon(kaiicon);
	SetMinSize(wxSize(600,400));
	MenuBar = new wxMenuBar();

	mains=new wxBoxSizer(wxHORIZONTAL);
	Tabs=new Notebook (this,ID_TABS);
	Toolbar=new KaiToolbar(this,MenuBar,-1,true);
	mains->Add(Toolbar,0,wxEXPAND,0);
	mains->Add(Tabs,1,wxEXPAND,0);


	FileMenu = new wxMenu();
	AppendBitmap(FileMenu,ID_OPENSUBS, _T("&Otwórz napisy"), _T("Otwórz plik napisów"),wxBITMAP_PNG ("opensubs"));
	AppendBitmap(FileMenu,ID_SAVE, _T("&Zapisz"), _T("Zapisz aktualny plik"),wxBITMAP_PNG("save"));
	AppendBitmap(FileMenu,ID_SAVEALL, _T("Zapisz wszystko"), _T("Zapisz wszystkie napisy"),wxBITMAP_PNG("saveall"));
	AppendBitmap(FileMenu,ID_SAVEAS, _T("Zapisz &jako..."), _T("Zapisz jako"),wxBITMAP_PNG("saveas"));
	AppendBitmap(FileMenu,ID_SAVETL, _T("Zapisz t³umaczenie"), _T("Zapisz t³umaczenie"),wxBITMAP_PNG("savetl"),false);

	SubsRecMenu = new wxMenu();
	//AppendRecent();
	AppendBitmap(FileMenu,ID_RECSUBS, _T("Ostatnio otwarte napisy"), _T("Ostatnio otwarte napisy"),wxBITMAP_PNG("recentsubs"),true, SubsRecMenu);
	AppendBitmap(FileMenu,ID_UNSUBS, _T("Usuñ napisy z edytora"), _T("Usuñ napisy z edytora"),wxBITMAP_PNG("close"));
	AppendBitmap(FileMenu,ID_SETTINGS, _T("&Ustawienia"), _T("Ustawienia Konwersji"),wxBITMAP_PNG("SETTINGS"));
	AppendBitmap(FileMenu,ID_QUIT, _T("&Wyjœcie\tAlt-F4"), _T("Zakoñcz dzia³anie programu"),wxBITMAP_PNG("exit"));
	MenuBar->Append(FileMenu, _T("&Plik"));

	EditMenu = new wxMenu();
	AppendBitmap(EditMenu, ID_UNDO1, _T("&Cofnij"), _T("Cofnij"),wxBITMAP_PNG("undo"),false);
	AppendBitmap(EditMenu, ID_REDO1, _T("&Ponów"), _T("Ponów"),wxBITMAP_PNG("redo"),false);
	AppendBitmap(EditMenu,ID_FINDREP, _T("ZnajdŸ i zmieñ"), _T("Szuka i zmienia dane frazy tekstu"),wxBITMAP_PNG("findreplace"));
	AppendBitmap(EditMenu,ID_FIND, _T("ZnajdŸ"), _T("Szuka dane frazy tekstu"),wxBITMAP_PNG("search"));
	wxMenu *SortMenu[2];
	for(int i=0; i<2; i++){
		SortMenu[i]=new wxMenu();
		SortMenu[i]->Append(7000+(6*i),"Czas Pocz¹tkowy","Sortuj wed³ug czasu pocz¹tkowego");
		SortMenu[i]->Append(7001+(6*i),"Czas Koñcowy","Sortuj wed³ug czasu koñcowego");
		SortMenu[i]->Append(7002+(6*i),"Style","Sortuj wed³ug stylów");
		SortMenu[i]->Append(7003+(6*i),"Aktor","Sortuj wed³ug aktora");
		SortMenu[i]->Append(7004+(6*i),"Efekt","Sortuj wed³ug efektu");
		SortMenu[i]->Append(7005+(6*i),"Warstwa","Sortuj wed³ug warstwy");
	}

	AppendBitmap(EditMenu,ID_SORT, _T("Sort&uj wszystkie linie"), _T("Sortuje wszystkie linie napisów ass"),wxBITMAP_PNG("sort"),true,SortMenu[0]);
	AppendBitmap(EditMenu,ID_SORTSEL, _T("Sort&uj zaznaczone linie"),_T("Sortuje zaznaczone linie napisów ass"),wxBITMAP_PNG("sortsel"),true, SortMenu[1]);
	AppendBitmap(EditMenu,ID_SELLIN, _T("Zaznacz Linijki"), _T("Zaznacza linijki wg danej frazy tekstu"),wxBITMAP_PNG("sellines"));
	MenuBar->Append(EditMenu, _T("&Edycja"));

	VidMenu = new wxMenu();
	AppendBitmap(VidMenu,ID_OPVIDEO, _T("Otwórz wideo"), _T("Otwiera wybrane wideo"),wxBITMAP_PNG("openvideo"));
	VidsRecMenu = new wxMenu();
	AppendBitmap(VidMenu, ID_RECVIDEO, _T("Ostatnio otwarte video"), _T("Ostatnio otwarte video"),wxBITMAP_PNG("recentvideo"),true, VidsRecMenu);
	AppendBitmap(VidMenu, ID_SETSTIME, _T("Wstaw czas pocz¹tkowy z wideo"), _T("Wstawianie czasu pocz¹tkowego z wideo"),wxBITMAP_PNG("setstarttime"),false);
	AppendBitmap(VidMenu, ID_SETETIME, _T("Wstaw czas koñcowy z wideo"), _T("Wstawianie czasu koñcowego z wideo"),wxBITMAP_PNG("setendtime"),false);
	AppendBitmap(VidMenu, ID_PREVFRAME, _T("Klatka w ty³"), _T("Cofa wideo o jedn¹ klatkê"),wxBITMAP_PNG("prevframe"),false);
	AppendBitmap(VidMenu, ID_NEXTFRAME, _T("Klatka w przód"), _T("Przechodzi o jedn¹ klatkê w przód"),wxBITMAP_PNG("nextframe"),false);
	AppendBitmap(VidMenu, ID_SETVIDATSTART,"PrzejdŸ do czasu pocz¹tkowego linii","Przechodzi wideo do czasu pocz¹tkowego linii",wxBITMAP_PNG("videoonstime"));
	AppendBitmap(VidMenu, ID_SETVIDATEND,"PrzejdŸ do czasu koñcowego linii","Przechodzi wideo do czasu koñcowego linii",wxBITMAP_PNG("videoonetime"));
	AppendBitmap(VidMenu, ID_PAUSE, _T("Odtwarzaj / Pauza"), _T("Odtwarza / Pauzuje wideo"),wxBITMAP_PNG("pausemenu"),false);
	VidMenu->Append(ID_OPVIDEOINDEX, _T("Otwieraj wideo przez FFMS2"), _T("Otwiera wideo przez FFMS2 co daje dok³adnoœæ klatkow¹"),wxITEM_CHECK)->Check(Options.GetBool("Index Video"));

	MenuBar->Append(VidMenu, _T("&Wideo"));

	AudMenu = new wxMenu();
	AppendBitmap(AudMenu,ID_OPAUDIO, _T("Otwórz audio"), _T("Otwiera wybrane audio"),wxBITMAP_PNG("openaudio"));
	AudsRecMenu = new wxMenu();

	AppendBitmap(AudMenu,ID_RECAUDIO, _T("Ostatnio otwarte audio"), _T("Ostatnio otwarte audio"),wxBITMAP_PNG("recentaudio"),true , AudsRecMenu);
	AppendBitmap(AudMenu,ID_OPFROMVID, _T("Otwórz audio z wideo"), _T("Otwiera audio z wideo"),wxBITMAP_PNG("audiofromvideo"));
	AppendBitmap(AudMenu,ID_CLOSEAUDIO, _T("Zamknij audio"), _T("Zamyka audio"),wxBITMAP_PNG("closeaudio"));
	MenuBar->Append(AudMenu, "&Audio");

	ViewMenu = new wxMenu();
	ViewMenu->Append(ID_VALL, _T("Wszystko"), _T("Wszystkie okna s¹ widoczne"));
	ViewMenu->Append(ID_VVIDEO, _T("Wideo i napisy"), _T("Widoczne tylko okno wideo i napsów"));
	ViewMenu->Append(ID_VAUDIO, _T("Audio i napisy"), _T("Widoczne tylko okno audio i napsów"));
	ViewMenu->Append(ID_VSUBS, _T("Tylko napisy"), _T("Widoczne tylko okno napisów"));
	MenuBar->Append(ViewMenu, "Wido&k");

	SubsMenu = new wxMenu();
	AppendBitmap(SubsMenu,ID_EDITOR, _T("W³¹cz / Wy³¹cz edytor"), _T("W³¹czanie b¹dŸ w³¹czanie edytora"),wxBITMAP_PNG("editor"));
	AppendBitmap(SubsMenu,ID_ASSPROPS, _T("W³aœciwoœci ASS / SSA"), _T("W³aœciwoœci napisów ASS / SSA"),wxBITMAP_PNG("ASSPROPS"));
	AppendBitmap(SubsMenu,ID_STYLEMNGR, _T("&Mened¿er stylów"), _T("S³u¿y do zarz¹dzania stylami ASS / SSA"),wxBITMAP_PNG("styles"));
	ConvMenu = new wxMenu();
	AppendBitmap(ConvMenu,ID_ASS, _T("Konwertuj do ASS"), _T("Konwertuje do formatu ASS"),wxBITMAP_PNG("convass"),false);
	AppendBitmap(ConvMenu,ID_SRT, _T("Konwertuj do SRT"), _T("Konwertuje do formatu SRT"),wxBITMAP_PNG("convsrt"));
	AppendBitmap(ConvMenu,ID_MDVD, _T("Konwertuj do MDVD"), _T("Konwertuje do formatu microDVD"),wxBITMAP_PNG("convmdvd"));
	AppendBitmap(ConvMenu,ID_MPL2, _T("Konwertuj do MPL2"), _T("Konwertuje do formatu MPL2"),wxBITMAP_PNG("convmpl2"));
	AppendBitmap(ConvMenu,ID_TMP, _T("Konwertuj do TMP"), _T("Konwertuje do formatu TMPlayer (nie zalecene)"),wxBITMAP_PNG("convtmp"));

	AppendBitmap(SubsMenu,ID_CONV, _T("Konwersja"), _T("Konwersja z jednego formatu napisów na inny"),wxBITMAP_PNG("convert"),true, ConvMenu);
	AppendBitmap(SubsMenu,ID_CHANGETIME, _T("Okno zmiany &czasów\tCtrl-I"), _T("Przesuwanie czasów napisów"),wxBITMAP_PNG("times"));
	AppendBitmap(SubsMenu,ID_CROSS, _T("W³¹cz wskaŸnik pozycji"), _T("W³¹cz / Wy³¹cz przesuwanie tekstu"),wxBITMAP_PNG("cross"),false);
	AppendBitmap(SubsMenu,ID_POSITION, _T("W³¹cz przesuwanie"), _T("W³¹cz / Wy³¹cz przesuwanie tekstu"),wxBITMAP_PNG("position"),false);
	AppendBitmap(SubsMenu,ID_MOVEMENT, _T("W³¹cz ruch"), _T("W³¹cz / Wy³¹cz przesuwanie tekstu"),wxBITMAP_PNG("move"),false);
	//AppendBitmap(SubsMenu,ID_MOVEONCURVE, _T("W³¹cz ruch po krzywej"), _T("W³¹cz / Wy³¹cz przesuwanie tekstu"),wxBITMAP_PNG("move"),false);
	AppendBitmap(SubsMenu,ID_SCALE, _T("W³¹cz skalowanie"), _T("W³¹cz / Wy³¹cz skalowanie tekstu"),wxBITMAP_PNG("scale"),false);
	AppendBitmap(SubsMenu,ID_ROTATEZ, _T("W³¹cz obracanie w osi Z"), _T("W³¹cz / Wy³¹cz obracanie tekstu w osi Z"),wxBITMAP_PNG("frz"),false);
	AppendBitmap(SubsMenu,ID_ROTATEXY, _T("W³¹cz obracanie w osi X i Y"), _T("W³¹cz / Wy³¹cz obracanie tekstu w osi X i Y"),wxBITMAP_PNG("frxy"),false);
	//AppendBitmap(SubsMenu,ID_FAXY, _T("W³¹cz / Wy³¹cz pochylanie tekstu"), _T("W³¹cz / Wy³¹cz pochylanie tekstu"),wxBITMAP_PNG("clip"),false);
	AppendBitmap(SubsMenu,ID_CLIPRECT, _T("W³¹cz wycinki prostok¹tne"), _T("W³¹cz / Wy³¹cz tworzenie wycinków prostok¹tnych"),wxBITMAP_PNG("cliprect"),false);
	AppendBitmap(SubsMenu,ID_CLIPS, _T("W³¹cz wycinki wektorowe"), _T("W³¹cz / Wy³¹cz tworzenie wycinków wektorowych"),wxBITMAP_PNG("clip"),false);
	AppendBitmap(SubsMenu,ID_DRAWINGS, _T("W³¹cz rysunki wektorowe"), _T("W³¹cz / Wy³¹cz tworzenie rysunków"),wxBITMAP_PNG("drawing"),false);
	AppendBitmap(SubsMenu,ID_COLLECTOR, _T("Kolekcjoner czcionek"), _T("Kolekcjoner czcionek"),wxBITMAP_PNG("fontcollector"));
	AppendBitmap(SubsMenu,ID_HIDETAGS, _T("Ukryj tagi w nawiasach"), _T("Ukrywa tagi w nawiasach ASS i MDVD"),wxBITMAP_PNG("hidetags"));
	MenuBar->Append(SubsMenu, L"&Napisy");

	AutoMenu = new wxMenu();
	AppendBitmap(AutoMenu,ID_AUTO, _T("Mened¿er skryptów"), _T("Mened¿er skryptów"),wxBITMAP_PNG("automation"));
	MenuBar->Append(AutoMenu, _T("Automatyzacja"));

	HelpMenu = new wxMenu();
	AppendBitmap(HelpMenu,ID_HELP, _T("&Pomoc (niekompletna ale jednak)"), _T("Otwiera pomoc w domyœlnej przegl¹darce"),wxBITMAP_PNG("help"));
	AppendBitmap(HelpMenu,ID_ANSI, _T("&W¹tek programu na forum AnimeSub.info"), _T("Otwiera w¹tek programu na forum AnimeSub.info"),wxBITMAP_PNG("ansi"));
	AppendBitmap(HelpMenu,ID_ABOUT, _T("&O programie\tF2"), _T("Wyœwietla informacje o programie"),wxBITMAP_PNG("about"));
	AppendBitmap(HelpMenu,ID_HELPERS, _T("&Lista osób pomocnych przy tworzeniu programu"), _T("Wyœwietla Listê osób pomocnych przy tworzeniu programu"),wxBITMAP_PNG("helpers"));
	MenuBar->Append(HelpMenu, _T("Pomo&c"));

	SetMenuBar(MenuBar);
	Toolbar->InitToolbar();

	SetSizer(mains);

	SetAccels(false);


	StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1);
	int StatusBarWidths[6] = { -12, 78, 62, 70, 76, -22};
	int StatusBarStyles[6] = { wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL };
	StatusBar1->SetFieldsCount(6,StatusBarWidths);
	StatusBar1->SetStatusStyles(6,StatusBarStyles);
	SetStatusBar(StatusBar1);

	Connect(ID_TABS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageChanged,0,this);
	Connect(ID_ADDPAGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageAdd);
	Connect(ID_CLOSEPAGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageClose);
	Connect(ID_NEXT_TAB,ID_PREV_TAB,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageChange);
	//tutaj dodawaj nowe idy
	Connect(ID_SAVE,ID_SORTSEL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected);
	Connect(7000,7011,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected);
	Connect(ID_OPENSUBS,ID_ANSI,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected1);
	Connect(ID_P5SEC,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnP5Sec);
	Connect(ID_M5SEC,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnM5Sec);
	Connect(ID_SELONVID,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnSelVid);
	Connect(ID_PREV_LINE,GRID_JOINWN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnChangeLine);
	Connect(ID_DELETE,ID_DELETE_TEXT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnDelete);
	Connect(wxEVT_MENU_OPEN,(wxObjectEventFunction)&kainoteFrame::OnMenuOpened);
	Connect(wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&kainoteFrame::OnClose1);
	Connect(30000,30059,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnRecent);
	Connect(30100,30199,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnRunScript);
	Connect(ID_SNAP_START,ID_SNAP_END,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnAudioSnap);
	SetDropTarget(new DragnDrop(this));


	//SetToolBar(Toolbar);

	int posx,posy,sizex,sizey;
	Options.GetCoords("Window Position",&posx,&posy);
	SetPosition(wxPoint(posx,posy));
	Options.GetCoords("Window Size",&sizex,&sizey);
	if(sizex<500 || sizey<350){
		sizex=800;sizey=650;
	}
	SetClientSize(wxSize(sizex,sizey));
	bool im=Options.GetBool("Window Maximize");
	if(im){
		Maximize(Options.GetBool("Window Maximize"));}

	if(!Options.GetBool("Show Editor")){HideEditor();}	
	std::set_new_handler(OnOutofMemory);
}

kainoteFrame::~kainoteFrame()
{

	bool im=IsMaximized();
	if(!im && !IsIconized()){
		int posx,posy,sizex,sizey;
		GetPosition(&posx,&posy);
		if(posx<2000){
			Options.SetCoords("Window Position",posx,posy);}
		GetClientSize(&sizex,&sizey);
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
	if(LSD){LSD->Destroy();LSD=NULL;}
	if(FR){FR->Destroy();FR=NULL;}
	if(Auto){delete Auto; Auto=NULL;}
	Tabs->Destroy();
	Tabs=NULL;

	if(SC){delete SC;SC=NULL;}

}


//elementy menu które ulegaj¹ wy³¹czaniu
void kainoteFrame::OnMenuSelected(wxCommandEvent& event)
{
	int id=event.GetId();
	TabPanel *pan=GetTab();
	byte state[256];
	if(GetKeyboardState(state)==FALSE){wxLogStatus("nie mo¿na pobraæ stanu przycisków");}
	if(state[VK_LSHIFT]>1 || state[VK_RSHIFT]>1){
		wxMenuItem *item=MenuBar->FindItem(id);
		wxString wins[1]={"Globalny"};
		//upewnij siê, ¿e da siê zmieniæ idy na nazwy, 
		//mo¿e i trochê spowolni operacjê ale skoñczy siê ci¹g³e wywalanie hotkeysów
		//mo¿e od razu funkcji onmaphotkey przekazaæ item by zrobi³a co trzeba
		int ret=-1;
		wxString name=item->GetItemLabelText();
		ret=Hkeys.OnMapHkey( id, name, this, wins, 1);
		if(ret==-1){item->SetAccel(&Hkeys.GetHKey(id));Hkeys.SaveHkeys();}
		else if(ret>0){
			wxMenuItem *item=MenuBar->FindItem(ret);
			wxAcceleratorEntry entry;
			item->SetAccel(&entry);
		}

		return;
	}
	//wxLogStatus("%i", id);
	if(id==ID_SAVE){
		Save(false);
	}else if(id==ID_SAVEAS){
		Save(true);
	}else if(id==ID_SAVEALL){
		SaveAll();
	}else if(id==ID_SAVETL){
		GetTab()->Grid1->AddSInfo("TLMode", "Translated",false);
		Save(true);
		GetTab()->Grid1->AddSInfo("TLMode", "Yes",false);
	}else if(id==ID_UNSUBS){
		if(SavePrompt(3)){event.SetInt(-1);return;}
		if(pan->SubsPath!=("")){
			pan->SubsName=_T("Bez Nazwy");
			pan->SubsPath=_T("");
			Label();
			pan->Grid1->Clearing();
			pan->Grid1->file=new SubsFile();
			pan->Grid1->LoadDefault();
			pan->Edit->RefreshStyle(true);
			pan->Grid1->RepaintWindow();

			if(pan->Video->GetState()!=None){pan->Video->OpenSubs(NULL);pan->Video->Render();}
		}
	}else if(id==ID_UNDO1){
		pan->Grid1->GetUndo(false);
	}else if(id==ID_REDO1){
		pan->Grid1->GetUndo(true);
	}else if(id==ID_FIND || id==ID_FINDREP){
		if(FR && FR->IsShown() && FR->repl && id==ID_FINDREP){FR->Hide();return;}
		FR= new findreplace(this, FR, id==ID_FINDREP);
		FR->Show(true);
		FR->ReloadStyle();
	}else if(id==ID_SELLIN){
		SL= new findreplace(this,SL,false,true);
		SL->Show(true);
	}else if(id==ID_PAUSE){
		pan->Video->Pause();
	}else if(id==ID_PREVFRAME||id==ID_NEXTFRAME){
		pan->Video->MovePos((id==ID_PREVFRAME)? -1 : 1);
	}else if(id==ID_SETSTIME||id==ID_SETETIME){
		if(pan->Video->GetState()!=None){
			if(id==ID_SETSTIME){
				int time= pan->Video->Tell()-(pan->Video->avtpf/2.0f)+Options.GetInt("Offset of start time");
				pan->Grid1->SetStartTime(ZEROIT(time));
			}else{
				int time=pan->Video->Tell()+(pan->Video->avtpf/2.0f)+Options.GetInt("Offset of end time");
				pan->Grid1->SetEndTime(ZEROIT(time));
			}
		}
	}else if(id==ID_OPVIDEOINDEX){
		wxMenuItem *Item=MenuBar->FindItem(ID_OPVIDEOINDEX);
		Options.SetBool("Index Video",Item->IsChecked());
	}else if(id>=ID_OPAUDIO&&id<=ID_CLOSEAUDIO){
		OnOpenAudio(event);
	}else if(id==ID_ASSPROPS){
		OnAssProps();
	}else if(id==ID_STYLEMNGR){
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
	}else if(id==ID_COLLECTOR){
		FontCollectorDialog fcd(this);
	}else if(id>=ID_CROSS && id<=ID_DRAWINGS){
		VideoCtrl *vb=pan->Video;
		EditBox *eb = pan->Edit;
		int vis = (id - ID_CROSS);

		if(vis==pan->Edit->Visual){return;}
		if(vb->Vclips && vis == 0){ 
			vb->SetVisual(0,0,true); 
		}else if( vis != pan->Edit->Visual ){
			eb->Visual = vis;
			vb->SetVisual(eb->line->Start.mstime, eb->line->End.mstime);
			if(!vb->isarrow){vb->SetCursor(wxCURSOR_ARROW);vb->isarrow=true;}
		}
		UpdateToolbar();
	}else if(id>=ID_ASS&&id<=ID_MPL2){
		OnConversion(( id - ID_ASS ) + 1);
	}else if(id==ID_HIDETAGS){
		pan->Grid1->HideOver();
	}else if(id==ID_CHANGETIME){
		bool show=!pan->CTime->IsShown();
		Options.SetBool("Show Change Time",show);
		pan->CTime->Show(show);
		pan->BoxSizer1->Layout();
	}else if(id>6999&&id<7012){
		bool all=id<7006;
		int difid=(all)? 7000 : 7006;
		pan->Grid1->SortIt(id-difid,all);
	}else if(id>=ID_VALL&&id<=ID_VSUBS){
		bool vidshow=( id==ID_VALL || id==ID_VVIDEO ) && pan->Video->GetState()!=None;
		bool vidvis=pan->Video->IsShown();
		if(!vidshow && pan->Video->GetState()==Playing){pan->Video->Pause();}
		pan->Video->Show(vidshow);
		if(vidshow && !vidvis){pan->Video->OpenSubs(pan->Grid1->SaveText());}
		if(pan->Edit->ABox){
			pan->Edit->ABox->Show((id==ID_VALL||id==ID_VAUDIO));
			if(id==ID_VAUDIO){pan->Edit->SetMinSize(wxSize(500,350));}
		}	
		pan->Layout();
	}else if(id==ID_AUTO){
		if(!LSD){LSD = new ScriptsDialog(this);LSD->AddFromSubs();}
		LSD->Show();
	}else if(id==ID_SETVIDATSTART){
		int fsel=pan->Grid1->FirstSel();
		if(fsel<0){return;}
		pan->Video->Seek(MAX(0,pan->Grid1->GetDial(fsel)->Start.mstime),true);
	}else if(id==ID_SETVIDATEND){
		int fsel=pan->Grid1->FirstSel();
		if(fsel<0){return;}
		pan->Video->Seek(MAX(0,pan->Grid1->GetDial(fsel)->End.mstime),false);
	}


}
//Sta³e elementy menu które nie ulegaj¹ wy³¹czaniu
void kainoteFrame::OnMenuSelected1(wxCommandEvent& event)
{
	int id=event.GetId();
	TabPanel *pan=GetTab();
	byte state[256];
	if(GetKeyboardState(state)==FALSE){wxLogStatus("nie mo¿na pobraæ stanu przycisków");}
	if(state[VK_LSHIFT]>1 || state[VK_RSHIFT]>1){
		wxMenuItem *item=MenuBar->FindItem(id);
		wxString win[]={"Globalny"};
		int ret=-1;
		wxString name=item->GetItemLabelText();
		ret=Hkeys.OnMapHkey( id, name, this, win, 1);
		if(ret==-1){item->SetAccel(&Hkeys.GetHKey(id));Hkeys.SaveHkeys();}
		else if(ret>0){
			wxMenuItem *item=MenuBar->FindItem(id);
			wxAcceleratorEntry entry;
			item->SetAccel(&entry);
		}

		return;
	}
	if(id==ID_OPENSUBS){
		if(SavePrompt(2)){return;}

		wxFileDialog *FileDialog1 = new wxFileDialog(this, _T("Wybierz Plik Napisów"), 
			(GetTab()->VideoPath!="")? GetTab()->VideoPath.BeforeLast('\\') : (subsrec.size()>0)?subsrec[subsrec.size()-1].BeforeLast('\\') : "", 
			_T(""), _T("Pliki napisów (*.ass),(*.ssa),(*.srt),(*.sub),(*.txt)|*.ass;*.ssa;*.srt;*.sub;*.txt|Pliki wideo z wbudowanymi napisami (*.mkv)|*.mkv"), wxFD_OPEN);
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
	}else if(id==ID_OPVIDEO){
		wxFileDialog* FileDialog2 = new wxFileDialog(this, _T("Wybierz Plik"), 
			(videorec.size()>0)?videorec[videorec.size()-1].BeforeLast('\\'):"", 
			_T(""), _T("Pliki video(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.mpg),(*.mpeg),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.mpg;*.mpeg;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"), wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
		if (FileDialog2->ShowModal() == wxID_OK){
			OpenFile(FileDialog2->GetPath());
		}
		FileDialog2->Destroy();
	}else if(id==ID_SETTINGS){
		OptionsDialog od(this,this);
		od.OptionsTree->ChangeSelection(0);
		od.ShowModal();
	}else if(id==ID_EDITOR){
		HideEditor();
	}else if(id==ID_CLOSE){
		Close();
	}else if(id==ID_ABOUT){
		wxMessageBox(_T("Edytor napisów by Bjakja aka Bakura, wersja ")+Options.progname.AfterFirst(' ')+_T("\r\n\r\n")\
			_T("Ten program to jakby moje zaplecze do nauki C++, wiêc mog¹ zdarzyæ siê ró¿ne b³êdy\r\n\r\n")\
			_T("Kainote zawiera w sobie czêœci nastêpuj¹cych projeków:\r\n")\
			_T("wxWidgets - Copyright © Julian Smart, Robert Roebling et al;\r\n")\
			_T("Color picker, wymuxowywanie napsów z mkv, audiobox, audio player, czêœæ funkcji do lua\r\ni kilka innych pojedynczych funkcji wziête z Aegisuba -\r\n")\
			_T("Copyright © Rodrigo Braz Monteiro;\r\n")\
			_T("Hunspell - Copyright © Kevin Hendricks;\r\n")\
			_T("Matroska Parser - Copyright © Mike Matsnev;\r\n")\
			_T("Interfejs CSRI - Copyright © David Lamparter;\r\n")\
			_T("Vsfilter - Copyright © Gabest;\r\n")\
			_T("FFMPEGSource2 - Copyright © Fredrik Mellbin;\r\n")\
			_T("FreeType - Copyright ©  David Turner, Robert Wilhelm, and Werner Lemberg;\r\n")\
			_T("Interfejs Avisynth - Copyright © Ben Rudiak-Gould et al."), _T("O Kainote"));
	}else if(id==ID_HELPERS){
		wxMessageBox("Pomoc graficzna: (przyciski, obrazki do pomocy itd)\r\n"\
			"- Archer (pierwsze przyciski do wideo).\r\n"\
			"- Kostek00 (przyciski do audio).\r\n"\
			"- Xandros (nowe przyciski do wideo, obrazki do pomocy).\r\n"\
			"- Devilkan (przyciski do menu i paska narzêdzi).\r\n \r\n"\
			"Testerzy: (mniej i bardziej wprawieni u¿ytkownicy programu)\r\n"\
			"- Funki27 (pierwszy tester maj¹cy spory wp³yw na obecne dzia³anie programu\r\n"\
			"i najbardziej narzekaj¹cy na wszystko).\r\n"\
			"- Sacredus (chyba pierwszy t³umacz u¿ywaj¹cy trybu t³umaczenia,\r\n nieoceniona pomoc przy testowaniu wydajnoœci na s³abym komputerze).\r\n"\
			"- Kostek00 (prawdziwy wynajdywacz b³êdów,\r\n"\
			"mia³ du¿y wp³yw na rozwój wykresu audio i g³ównego pola tekstowego).\r\n"\
			"- Devilkan (crashhunter, ze wzglêdu na swój system i przyzwyczajenia wytropi³ ju¿ wiele crashy).\r\n \r\n"\

			"Podziêkowania tak¿e dla osób które u¿ywaj¹ programu i od czasu do czasu coœ wynajdowali.\r\n"\
			"Ksenoform, Deadsoul, Z³y los, Vessin, Xandros, Areki, Nyah2211, Waski_jestem.",
			"Lista osób pomocnych przy tworzeniu programu");
	}else if(id==ID_HELP||id==ID_ANSI){
		WinStruct<SHELLEXECUTEINFO> sei;
		wxString url=(id==ID_HELP)? Options.pathfull+"\\Pomoc\\Kainote pomoc.html" : "http://animesub.info/forum/viewtopic.php?id=258715";
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

	wxString styletext=_T("");
	for (int j=0;j<GetTab()->Grid1->StylesSize();j++){
		Styles *acstyl=GetTab()->Grid1->GetStyle(j);
		slx.CheckListBox1->Append(acstyl->Name);

	}
	if(slx.ShowModal()==wxID_OK){

		for (size_t v=0;v<slx.CheckListBox1->GetCount();v++)
		{

			if(slx.CheckListBox1->IsChecked(v)){
				styletext<<slx.CheckListBox1->GetString(v)<<_T(";");
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
	sci.title->SetValue(ngrid->GetSInfo(_T("Title")));
	sci.script->SetValue(ngrid->GetSInfo(_T("Original Script")));
	sci.translation->SetValue(ngrid->GetSInfo(_T("Original Translation")));
	sci.editing->SetValue(ngrid->GetSInfo(_T("Original Editing")));
	sci.timing->SetValue(ngrid->GetSInfo(_T("Original Timing")));
	sci.update->SetValue(ngrid->GetSInfo(_T("Script Updated By")));
	wxString oldx=ngrid->GetSInfo(_T("PlayResX"));
	wxString oldy=ngrid->GetSInfo(_T("PlayResY"));
	int nx=wxAtoi(oldx);
	int ny=wxAtoi(oldy);
	if(nx<1 && ny<1){nx=384;ny=288;}
	else if(nx<1){nx=(float)ny*(4.0/3.0);if(ny==1024){nx=1280;}}
	else if(ny<1){ny=(float)nx*(3.0/4.0);if(nx==1280){ny=1024;}}
	sci.width->SetInt(nx);
	sci.height->SetInt(ny);
	wxString wraps=ngrid->GetSInfo(_T("WrapStyle"));
	int ws = wxAtoi(wraps);
	sci.wrapstyle->SetSelection(ws);
	wxString colls=ngrid->GetSInfo(_T("Collisions"));
	if(colls==_T("Reverse")){sci.collision->SetSelection(1);}
	wxString bords=ngrid->GetSInfo(_T("ScaledBorderAndShadow"));
	if (bords==_T("no")){sci.CheckBox2->SetValue(false);}

	if(sci.ShowModal()==wxID_OK)
	{
		int newx=sci.width->GetInt();
		int newy=sci.height->GetInt();
		if(newx<1 && newy<1){newx=384;newy=288;}
		else if(newx<1){newx=(float)newy*(4.0/3.0);}
		else if(newy<1){newy=(float)newx*(3.0/4.0);if(newx==1280){newy=1024;}}

		bool save=(!sci.CheckBox1->GetValue()&&(newx!=oldx||newy!=oldy));
		//wxLogStatus(oldx+" "+oldy+" %i %i %i",newx, newx, save);
		if(sci.title->GetValue()!=_T("")){ if(sci.title->IsModified()){ngrid->AddSInfo(_T("Title"),sci.title->GetValue());} }
		else{ngrid->AddSInfo(_T("Title"),_T("Kainote Ass File"));}
		if(sci.script->IsModified()){ngrid->AddSInfo(_T("Original Script"),sci.script->GetValue());}
		if(sci.translation->IsModified()){ngrid->AddSInfo(_T("Original Translation"),sci.translation->GetValue());}
		if(sci.editing->IsModified()){ngrid->AddSInfo(_T("Original Editing"),sci.editing->GetValue());}
		if(sci.timing->IsModified()){ngrid->AddSInfo(_T("Original Timing"),sci.timing->GetValue());}
		if(sci.update->IsModified()){ngrid->AddSInfo(_T("Script Updated By"),sci.update->GetValue());}

		if(sci.width->IsModified()){ngrid->AddSInfo(_T("PlayResX"),wxString::Format("%i",newx));}
		if(sci.height->IsModified()){ngrid->AddSInfo(_T("PlayResY"),wxString::Format("%i",newy));}


		if(ws != sci.wrapstyle->GetSelection()){ngrid->AddSInfo(_T("WrapStyle"),wxString::Format(_T("%i"),sci.wrapstyle->GetSelection()));}
		wxString collis=(sci.collision->GetSelection()==0)?_T("Normal"):_T("Reverse");
		if(colls!=collis){ngrid->AddSInfo(_T("Collisions"),collis);}
		wxString bordas=(sci.CheckBox2->GetValue())?_T("yes"):_T("no");
		if(bords !=bordas){ ngrid->AddSInfo(_T("ScaledBorderAndShadow"),bordas);}


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
		wxString extens="Plik napisów ";

		if (atab->Grid1->form<SRT){extens+=_T("(*.ass)|*.ass");}
		else if (atab->Grid1->form==SRT){extens+=_T("(*.srt)|*.srt");}
		else{extens+=_T("(*.txt, *.sub)|*.txt;*.sub");};

		wxString path=(atab->VideoPath!="" && Options.GetBool("Subs Autonaming"))? atab->VideoPath : atab->SubsPath;
		wxString name=path.BeforeLast('.');
		path=path.BeforeLast('\\');

		wxWindow *_parent=(atab->Video->isfullskreen)? (wxWindow*)atab->Video->TD : this;
		wxFileDialog saveFileDialog(_parent, _T("Zapisz Plik Napisów"), 
			path, name ,extens, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_OK){

			atab->SubsPath = saveFileDialog.GetPath();
			wxString ext=(atab->Grid1->form<SRT)? "ass" : (atab->Grid1->form==SRT)? "srt" : "txt";
			if(!atab->SubsPath.EndsWith(ext)){atab->SubsPath<<"."<<ext;}
			atab->SubsName=atab->SubsPath.AfterLast('\\');SetRecent();
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
	if(ext==_T("exe")||ext==_T("zip")||ext==_T("rar")){return false;}
	TabPanel *pan=GetTab();
	//pan->Freeze();
	bool found=false;
	wxString fntmp="";
	bool issubs=(ext==_T("ass")||ext==_T("txt")||ext==_T("sub")||ext==_T("srt")||ext==_T("ssa"));
	if(Options.GetBool("Open In New Card")&&pan->SubsPath!=""&&!pan->Video->isfullskreen&&issubs){Tabs->AddPage(true);pan=Tabs->Page(Tabs->Size()-1);}
	if(pan->edytor && !(issubs&&pan->VideoPath.BeforeLast('.')==filename.BeforeLast('.'))
		&&!(!issubs&&pan->SubsPath.BeforeLast('.')==filename.BeforeLast('.'))){
			fntmp= FindFile(filename,issubs,!(fulls || pan->Video->isfullskreen) );
			if(fntmp!=""){found=true;if(!issubs){ext=fntmp.AfterLast('.');}}
	}
	//pan->Thaw();

	if(issubs||found){  
		wxString fname=(found&&!issubs)?fntmp:filename;
		if(SavePrompt(2)){return true;}
		OpenWrite ow; 
		wxString s=ow.FileOpen(fname);
		if(s==_T("")){return false;}
		pan->Grid1->Loadfile(s,ext);

		if(ext=="ssa"){ext="ass";fname=fname.BeforeLast('.')+".ass";}
		pan->SubsPath=fname;
		pan->SubsName=fname.AfterLast('\\');


		if(ext==_T("ass")){
			wxString katal=pan->Grid1->GetSInfo(_T("Last Style Storage"));

			if(katal!=_T("")){
				for(size_t i=0;i<Options.dirs.size();i++)
				{
					if(katal==Options.dirs[i]){Options.LoadStyles(katal);
					if(ss){ss->Store->SetSelection(0,true);
					int chc=ss->Choice1->FindString(Options.acdir);
					ss->Choice1->SetSelection(chc);
					}
					}
				}
			}

		}
		//wxLogStatus("State %i found", GetTab()->Video->GetState(), found);
		if(pan->Video->GetState()!=None&&!found){//wxLogStatus("opensubs");
			bool isgood=pan->Video->OpenSubs((pan->edytor)? pan->Grid1->SaveText() : 0);
			if(!isgood){wxMessageBox(_T("otwieranie napisów failed"), _T("Uwaga"));}
		}
		SetRecent();


		Label();

		if(!pan->edytor&&!fulls&&!pan->Video->isfullskreen){HideEditor();}
		if(!found){pan->CTime->Contents();UpdateToolbar();return true;}
	}
	//wxMessageBox("All subs");

	wxString fnname=(found&&issubs)?fntmp:filename;

	bool isload=pan->Video->Load(fnname,pan->Grid1->SaveText(),fulls);


	if(!isload){
		return isload;}
	else{
		pan->CTime->Contents();
		pan->Video->seekfiles=true;
		UpdateToolbar();
	}


	Options.SaveOptions(true, false);


	return true;  
}



//0 - subs, 1 - vids, 2 - auds
void kainoteFrame::SetRecent(short what)
{
	int idd=30000+(20*what);
	wxMenu *wmenu=(what==0)?SubsRecMenu : (what==1)? VidsRecMenu : AudsRecMenu;
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
void kainoteFrame::AppendRecent(short what,wxMenu *Menu)
{
	wxMenu *wmenu;
	if(Menu){
		wmenu=Menu;
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
		wxMenuItem* MI= new wxMenuItem(wmenu, idd+i, recs[i].AfterLast('\\'), _T("Otwórz ")+recs[i], wxITEM_NORMAL);
		wmenu->Append(MI);
	}

	if(!wmenu->GetMenuItemCount()){
		wxMenuItem* MI= new wxMenuItem(wmenu, idd, "Brak","", wxITEM_NORMAL);
		MI->Enable(false);
		wmenu->Append(MI);
	}
}

void kainoteFrame::OnRecent(wxCommandEvent& event)
{
	int id=event.GetId();
	wxMenuItem* MI=0;
	if(id<30020){
		MI=SubsRecMenu->FindItem(id);
	}else if(id<30040){
		MI=VidsRecMenu->FindItem(id);
	}else{
		MI=AudsRecMenu->FindItem(id);
	}
	wxString filename=MI->GetHelp().AfterFirst(' ');
	byte state[256];
	if(GetKeyboardState(state)==FALSE){wxLogStatus("nie mo¿na pobraæ stanu przycisków");}
	if(state[VK_LCONTROL]>1 || state[VK_RCONTROL]>1){
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

			
		if(!ShellExecuteEx(&sei)){wxLogStatus("nie mo¿na otworzyæ folderu");}
		return;}
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
			&&ext!="rm"&&ext!="3gp"&&ext!="ts"&&ext!="m2ts"&&ext!="m4v"&&ext!="flv"))  //&&ext!="avs" przynajmniej do momentu dorobienia obs³ugi przez avisynth
			){ }else{plik=pliki[i];break;}
	}
	if(plik!=""&&prompt){
		wxString co=(video)?"wideo":"napisy"; 
		if (wxMessageBox(_T("Wczytaæ ")+co+_T(" o nazwie ")+plik.AfterLast('\\')+_T("?"), _T("Potwierdzenie"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO ){plik="";} 
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


void kainoteFrame::OnSelVid(wxCommandEvent& event)
{
	GetTab()->Grid1->SelVideoLine();
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
	wxString name=(video)?atab->VideoName : atab->SubsName;
	SetLabel(whiter+name+_T(" -- ")+Options.progname);
	if(name.Len()>35){name=name.SubString(0,35)+"...";}
	Tabs->SetPageText((wtab<0)? Tabs->GetSelection() : wtab, whiter+name);
}

void kainoteFrame::SetAccels(bool _all)
{
	std::vector<wxAcceleratorEntry> entries;
	entries.resize(3);
	entries[0].Set(wxACCEL_NORMAL, WXK_F1, ID_SELONVID);
	entries[1].Set(wxACCEL_CTRL, (int) 'T', ID_ADDPAGE);
	entries[2].Set(wxACCEL_CTRL, (int) 'W', ID_CLOSEPAGE);

	for(auto cur=Hkeys.hkeys.rbegin(); cur!=Hkeys.hkeys.rend(); cur++){
		int id=cur->first;
		if(id>=6850){
			entries.push_back(Hkeys.GetHKey(id));
		}else if(id>6000){
			wxMenuItem *item=MenuBar->FindItem(id);
			if(!item){continue;}
			if(item->GetItemLabelText() != cur->second.Name){

				int cnt= MenuBar->GetMenuCount();
				int ret;
				for(int i=0; i<cnt; i++){
					ret=MenuBar->FindMenuItem(MenuBar->GetMenu(i)->GetTitle(),cur->second.Name);
					if(ret!=-1){break;}
				}
				item=MenuBar->FindItem(ret);
				if(!item){continue;}
			}
			item->SetAccel(&Hkeys.GetHKey(id));
		}else if(id<6001){break;}

	}

	wxAcceleratorTable accel(entries.size(), &entries[0]);
	Tabs->SetAcceleratorTable(accel);

	if(!_all){return;}
	for(size_t i=0;i<Tabs->Size();i++)
	{
		Tabs->Page(i)->SetAccels();
	}
}

bool kainoteFrame::SpellcheckerOn()
{
	SC=new SpellChecker();
	return SC->Initialize();
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
	Tabs->Freeze();
	for(size_t i=0;i<files.size();i++){
		wxString ext=files[i].AfterLast('.').Lower();
		if(ext=="ass"||ext=="ssa"||ext=="txt"||ext=="srt"||ext=="sub"){
			subs.Add(files[i]);
		}
		else if(ext!="exe"){
			videos.Add(files[i]);
		}

	}
	//wxLogMessage("przed openfile %i",(int)files.size());
	if(files.size()==1){OpenFile(files[0],(videos.size()==1&&Options.GetBool("Video Fullskreen on Start")));
	videos.Clear();subs.Clear();files.Clear();
	Tabs->Thaw();
	return;}


	GetTab()->Hide();
	size_t maxx=(subs.size()>videos.size())?subs.size() : videos.size();

	for(size_t i=0;i<maxx;i++)
	{

		if((i>=Tabs->Size()||Tabs->Page(Tabs->iter)->SubsPath!=""||Tabs->Page(Tabs->iter)->VideoPath!="")&&!intab){InsertTab(false);}
		TabPanel *pan=GetTab();
		if(i<subs.size()){
			wxString ext=subs[i].AfterLast('.').Lower();
			OpenWrite ow;
			wxString s=ow.FileOpen(subs[i]);
			if(s==_T("")){break;
			}else{
				pan->Grid1->Loadfile(s,ext);
			}

			if(ext=="ssa"){ext="ass";subs[i]=subs[i].BeforeLast('.')+".ass";}
			pan->SubsPath=subs[i];
			pan->SubsName=pan->SubsPath.AfterLast('\\');
			if(!pan->edytor){HideEditor();}
			SetRecent();

			if(i==maxx-1){UpdateToolbar();}
			Label();
			//wxLogStatus("Subs %i", i);
		}
		if(i<videos.size()){
			//wxLogStatus("Video bload %i", i);
			bool isload=pan->Video->Load(videos[i],(pan->edytor)? pan->Grid1->SaveText() : 0);
			//wxLogStatus("Video aload %i %i", i, (int)isload);


			if(!isload){
				if(pan->Video->IsDshow){wxMessageBox("Plik nie jest poprawnym plikiem wideo, albo jest uszkodzony,"\
					"\r\nb¹dŸ brakuje kodeków czy te¿ splittera", _T("Uwaga"));}
				break;
			}
			pan->CTime->Contents();
			pan->Video->seekfiles=true;

		}

	}

	Tabs->Thaw();
	GetTab()->Show();
	int w,h;
	Tabs->GetClientSize(&w,&h);
	Tabs->RefreshRect(wxRect(0,h-25,w,25),false);

	files.Clear();
	subs.Clear();
	videos.Clear();

	Options.SaveOptions(true, false);
}

void kainoteFrame::OnPageChange(wxCommandEvent& event)
{
	int step=(event.GetId()==ID_NEXT_TAB)? Tabs->iter+1 : Tabs->iter-1;
	if(step<0){step=Tabs->Size()-1;}
	else if(step>=(int)Tabs->Size()){step=0;}
	Tabs->ChangePage(step);
}


void kainoteFrame::OnPageChanged(wxCommandEvent& event)
{
	wxString whiter;
	TabPanel *cur=Tabs->GetPage();
	int iter=cur->Grid1->file->Iter();
	if(iter>0&&cur->Grid1->Modified){whiter<<iter<<"*";}
	wxString name=(!cur->edytor)? cur->VideoName : cur->SubsName;
	SetLabel(whiter+name+_T(" -- ")+Options.progname);

	if(cur->Video->GetState()!=None){
		//wxLogStatus("video opts");
		//if(cur->Video->GetState()==Paused){cur->Video->Render();}
		SetStatusText(getfloat(cur->Video->fps)+" fps",2);
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

	cur->Grid1->UpdateUR(false);

	UpdateToolbar();
	//wxLogStatus("oldpage %i", Tabs->GetOldSelection());
	cur->Grid1->SetFocus();
	if(Tabs->iter!=Tabs->GetOldSelection()){
		cur->CTime->RefVals(Tabs->Page( Tabs->GetOldSelection() )->CTime);}
	//wxLogStatus("focused");
	//wxLogMessage("autoselect");
	if(Options.GetBool("Auto Select Lines")){
		Grid *old=Tabs->Page(Tabs->GetOldSelection())->Grid1;
		if(old->FirstSel()>-1){
			cur->Grid1->SelVideoLine(old->GetDial(old->FirstSel())->Start.mstime);
		}
	}
	if(ss&&ss->IsShown()){ss->LoadAssStyles();}
	if(FR){FR->Reset();FR->ReloadStyle();}
}

void kainoteFrame::HideEditor()
{
	TabPanel *cur=GetTab();
	if(cur->edytor){
		//wxCommandEvent evt;
		//evt.SetInt(5);
		//OnUnSubs(evt);
		//if(evt.GetInt()==-1){return;}
	}

	cur->edytor = !cur->edytor;

	cur->Grid1->Show(cur->edytor);

	cur->Edit->Show(cur->edytor);

	if(cur->edytor){//Za³¹czanie Edytora

		cur->BoxSizer1->Detach(cur->Video);
		cur->BoxSizer2->Prepend(cur->Video, 0, wxEXPAND|wxALIGN_TOP, 0);
		cur->BoxSizer1->InsertSpacer(1,3);
		if(cur->Video->GetState()!=None&&!cur->Video->isfullskreen){
			int sx,sy,vw,vh;
			Options.GetCoords("Video Window Size",&vw,&vh);
			if(vh<350){vh=350,vw=500;}
			cur->Video->CalcSize(&sx,&sy,vw,vh);
			cur->Video->SetMinSize(wxSize(sx,sy+44));
		}else{cur->Video->Hide();}
		if(Options.GetBool("Show Change Time")){
			cur->CTime->Show();}
		cur->BoxSizer1->Layout();
		Label();
		if(cur->Video->GetState()!=None){cur->Video->ChangeVobsub();}
	}
	else{//Wy³¹czanie edytora

		cur->CTime->Hide();

		cur->BoxSizer1->Remove(1);

		if(!cur->Video->IsShown()){cur->Video->Show();}

		cur->BoxSizer2->Detach(cur->Video);

		cur->BoxSizer1->Add(cur->Video, 1, wxEXPAND|wxALIGN_TOP, 0);

		if(cur->Video->GetState()!=None&&!cur->Video->isfullskreen&&!IsMaximized()){
			int sx,sy,sizex,sizey;
			GetClientSize(&sizex,&sizey);

			cur->Video->CalcSize(&sx,&sy,sizex,sizey);

			SetClientSize(sx+iconsize,sy+44+Tabs->GetHeight());

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

void kainoteFrame::AppendBitmap(wxMenu *menu, int id, wxString text, wxString help, wxBitmap bitmap, bool enable, wxMenu *SubMenu)
{
	
	wxMenuItem *item=new wxMenuItem(0, id, text, help, wxITEM_NORMAL, SubMenu);
	if(bitmap.IsOk()){item->SetBitmap(bitmap);if(id!=ID_CONV){Toolbar->AddID(id);}}
	if(!enable){item->Enable(false);}
	menu->Append(item);
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


bool kainoteFrame::SavePrompt(char mode, int wtab)
{
	TabPanel* atab=(wtab<0)? GetTab() : Tabs->Page(wtab);	
	if(atab->Grid1->file->Iter()>0 && atab->Grid1->Modified){
		wxWindow *_parent=(atab->Video->isfullskreen)? (wxWindow*)atab->Video->TD : this;

		wxString info=(mode==0)? "zamkniêciem programu" : (mode==1)? "zamkniêciem zak³adki" : (mode==2)? "wczytaniem nowych napisów" : "usuniêciem napisów";
		int answer = wxMessageBox("Zapisaæ napisy o nazwie \""+atab->SubsName+"\" przed "+info+"?", "Potwierdzenie", wxYES_NO | wxCANCEL, _parent);
		if(answer==wxCANCEL){return true;}

		if(answer==wxYES){
			Save(false, wtab);
		}
	}
	return false;
}


void kainoteFrame::UpdateToolbar()
{//disejblowanie rzeczy niepotrzebnych przy wy³¹czonym edytorze
	wxMenuEvent evt;
	OnMenuOpened(evt);
	Toolbar->Updatetoolbar();
}

void kainoteFrame::OnOpenAudio(wxCommandEvent& event)
{
	int id=event.GetId();
	if(id==ID_CLOSEAUDIO && GetTab()->Edit->ABox){
		GetTab()->Video->player=NULL; 
		GetTab()->Edit->ABox->Destroy(); 
		GetTab()->Edit->ABox=NULL; 
		GetTab()->Edit->Layout();}
	else{

		if(!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){wxMessageBox("Dupa blada, skróty klawiszowe siê nie wczyta³y, na audio nie podzia³asz", "B³êdny b³¹d");return;}
		if(!Options.AudioOpts && !Options.LoadAudioOpts()){wxMessageBox("Dupa blada, opcje siê nie wczyta³y, na audio nie podzia³asz", "B³êdny b³¹d");return;}

		wxString Path;
		if(id==ID_OPAUDIO){
			wxFileDialog *FileDialog1 = new wxFileDialog(this, _T("Wybierz Plik Audio"), 
				(GetTab()->VideoPath!="")? GetTab()->VideoPath.BeforeLast('\\') : (videorec.size()>0)?subsrec[videorec.size()-1].BeforeLast('\\') : "", 
				_T(""), _T("Pliki audio i video (*.wav),(*.w64),(*.aac),(*.mp3),(*.mp4),(*.mkv),(*.avi)|*.wav;*.w64;*.aac;*.mp3;*.mp4;*.mkv;*.avi|Wszystkie Pliki |*.*"), wxFD_OPEN);
			int result = FileDialog1->ShowModal();
			if (result == wxID_OK){
				Path=FileDialog1->GetPath();
			}
			FileDialog1->Destroy();
			if(result == wxID_CANCEL){return;}
		}
		if(id>30039){Path=event.GetString();}
		if(Path.IsEmpty()){Path=GetTab()->VideoPath;}
		if(Path.IsEmpty()){return;}


		if(GetTab()->Edit->ABox){GetTab()->Edit->ABox->SetFile(Path,(id==40000));
		if(!GetTab()->Edit->ABox->audioDisplay->loaded){
			GetTab()->Edit->ABox->Destroy(); 
			GetTab()->Edit->ABox=NULL;
		}else{SetRecent(2);}
		}
		else{
			GetTab()->Edit->ABox=new AudioBox(GetTab()->Edit, GetTab()->Grid1);
			GetTab()->Edit->ABox->SetFile(Path, (id==40000));

			if(GetTab()->Edit->ABox->audioDisplay->loaded){
				GetTab()->Edit->BoxSizer1->Prepend(GetTab()->Edit->ABox, 4, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 2);
				//int sizew,sizeh;
				//Options.GetCoords("Video Window Size",&sizew,&sizeh);
				if (!GetTab()->Video->IsShown()){
					GetTab()->Edit->SetMinSize(wxSize(500,350));}
				GetTab()->Layout();
				Tabs->Refresh(false);
				GetTab()->Edit->ABox->audioDisplay->SetFocus();
				SetRecent(2);
			}
			else{GetTab()->Edit->ABox->Destroy(); GetTab()->Edit->ABox=NULL;}
		}
	}
}



//uses before menu is shown
void kainoteFrame::OnMenuOpened(wxMenuEvent& event)
{
	wxMenu *curMenu = event.GetMenu();


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
	TabPanel *pan = GetTab();
	bool enable=(pan->Video->GetState()!=None);
	bool edytor=pan->edytor;
	for(int i = ID_PAUSE; i<=ID_SETVIDATEND; i++)
	{
		MenuBar->Enable(i,(i<ID_SETSTIME)? enable : enable && edytor);
	}
	//kolejno numery id
	bool forms;
	char form = pan->Grid1->form;
	bool tlmode = pan->Grid1->transl;
	bool editor = pan->edytor;
	for(int i=ID_SAVE;i<=ID_AUTO;i++){//po kolejne idy zajrzyj do enuma z pliku h, ostatni jest ID_AUTO
		forms=true;
		
		if(i>=ID_ASSPROPS&&i<ID_ASS){forms=form<SRT;}//menager stylów i sinfo
		else if(i==ID_ASS){forms=form>ASS;}//konwersja na ass
		else if(i==ID_SRT){forms=form!=SRT;}//konwersja na srt
		else if(i==ID_MDVD){forms=form!=MDVD;}//konwersja na mdvd
		else if(i==ID_MPL2){forms=form!=MPL2;}//konwersja na mpl2
		else if(i==ID_TMP){forms=form!=TMP;}//konwersja na tmp
		if((i>=ID_ASS && i<=ID_TMP) && tlmode){forms=false;}
		if(i>=ID_CROSS && i<=ID_DRAWINGS){forms= pan->Video->IsShown() && form<SRT && ( i - ID_CROSS ) != pan->Edit->Visual;}
		if(i==ID_VAUDIO || i==ID_CLOSEAUDIO){forms= pan->Edit->ABox!=0;}
		if(i==ID_VVIDEO||i==ID_OPFROMVID){forms= pan->Video->GetState()!=None;}
		if(i==ID_SAVETL){forms=tlmode;}
		MenuBar->Enable(i, editor && forms);
	}
	//specjalna poprawka do zapisywania w trybie t³umaczenia, jeœli jest tlmode, to zawsze ma dzia³aæ.
	pan->Edit->TlMode->Enable((editor && form==ASS && pan->SubsPath!=""));

}


void kainoteFrame::OnRunScript(wxCommandEvent& event)
{

	wxString wscript=Hkeys.hkeys[event.GetId()].Name;
	int line, macro;
	wscript=wscript.AfterFirst(' ');
	line=wxAtoi(wscript.BeforeFirst('-'));
	macro=wxAtoi(wscript.AfterFirst('-'));
	if(!Auto){Auto = new Auto::Automation(this);}
	if(line>=(int)Auto->Scripts.size()){wxMessageBox(wxString::Format("Brak wczytanego skryptu o numerze %i",line));}
	Auto::LuaScript *scr=Auto->Scripts[line];
	if((int)scr->Macros.size()<=macro){
		wxString msg;
		if(scr->loaded){msg<<"Skrypt o nazwie \""<<scr->name<<"\" nie posiada "<<macro<<" makra.";}
		else{msg=scr->description;}
		wxMessageBox(msg); return;
	}
	TabPanel* pan=GetTab();
	int diff=(pan->Grid1->SInfoSize() + pan->Grid1->StylesSize()+1);
	wxArrayInt sels = pan->Grid1->GetSels(true);
	if(scr->Macros[macro]->Validate(sels, pan->Edit->ebrow, diff)){
		if(scr->CheckLastModified()){scr->Reload();}	
		scr->Macros[macro]->Process(sels,GetTab()->Edit->ebrow,diff,this);

		GetTab()->Grid1->SetModified(); 

		for(size_t i=0; i<sels.size(); i++){
			GetTab()->Grid1->sel[sels[i]]=true;
		}
		GetTab()->Grid1->RepaintWindow();
	}

}


void kainoteFrame::OnChangeLine(wxCommandEvent& event)
{

	int idd=event.GetId();
	if(idd<GRID_JOINWP){//zmiana linijki
		GetTab()->Grid1->NextLine((idd==ID_PREV_LINE)?-1 : 1);}
	else{//scalanie dwóch linijek
		GetTab()->Grid1->OnJoin(event);
	}
}

void kainoteFrame::OnDelete(wxCommandEvent& event)
{
	int idd=event.GetId();
	if(idd==ID_DELETE){
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
	int time= (id==ID_SNAP_START)? pan->Edit->line->Start.mstime : pan->Edit->line->End.mstime;
	int time2= (id==ID_SNAP_START)? pan->Edit->line->End.mstime : pan->Edit->line->Start.mstime;
	int snaptime= pan->Edit->ABox->audioDisplay->GetBoundarySnap(time,1000,!Options.GetBool(_T("Audio Snap To Keyframes")),(id==ID_SNAP_START),true);
	//wxLogStatus(" times %i %i", snaptime, time);
	if(time!= snaptime){
		if(id==ID_SNAP_START){
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
		wxLogStatus("Zabrak³o pamiêci RAM, usuniêto czêœæ historii");
		return;
	}else if(Notebook::GetTabs()->Size()>1){
		for(size_t i=0; i<Notebook::GetTabs()->Size(); i++)
		{
			if( i!= Notebook::GetTabs()->GetSelection()){
				if(Notebook::GetTabs()->Page(i)->Grid1->file->maxx()>3){
					Notebook::GetTabs()->Page(i)->Grid1->file->RemoveFirst(2);
					wxLogStatus("Zabrak³o pamiêci RAM, usuniêto czêœæ historii");
					return;
				}
			}
		}
	}

	std::exit(1);
}