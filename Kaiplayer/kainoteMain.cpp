/***************************************************************
* Name:      kainoteMain.cpp
* Purpose:   Subtitles editor and player
* Author:    Bjakja (bjakja@op.pl)
* Created:   2012-04-23
* Copyright: Marcin Drob aka Bjakja (http://animesub.info/forum/viewtopic.php?id=258715)
* License:
* Kainote is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Kainote is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************/


#include "KainoteMain.h"
#include "SubsTime.h"
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
#include "KaiTextCtrl.h"
#include "KaiMessageBox.h"
#include "FontEnumerator.h"
#include "SubsResampleDialog.h"
#include "SpellCheckerDialog.h"
#include "utils.h"

#undef IsMaximized
#if _DEBUG
#define logging 5
#endif

#if !defined INSTRUCTIONS
#define INSTRUCTIONS L""
#endif

void EnableCrashingOnCrashes()
{
	typedef BOOL(WINAPI *tGetPolicy)(LPDWORD lpFlags);
	typedef BOOL(WINAPI *tSetPolicy)(DWORD dwFlags);
	const DWORD EXCEPTION_SWALLOWING = 0x1;

	HMODULE kernel32 = LoadLibraryA("kernel32.dll");
	tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32,
		"GetProcessUserModeExceptionPolicy");
	tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32,
		"SetProcessUserModeExceptionPolicy");
	if (pGetPolicy && pSetPolicy)
	{
		DWORD dwFlags;
		if (pGetPolicy(&dwFlags))
		{
			// Turn off the filter
			pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
		}
	}
}

KainoteFrame::KainoteFrame(const wxPoint &pos, const wxSize &size)
	: KaiFrame(0, -1, _("Bez nazwy - ") + Options.progname + L" " + wxString(INSTRUCTIONS), 
	pos, size, wxDEFAULT_FRAME_STYLE, L"Kainote_main_window")
	, badResolution(false)
{
	LogHandler::Create(this);
	//when need log window on start uncomment this
#ifdef _DEBUG
	//LogHandler::ShowLogWindow();
#endif

	Options.GetTable(SubsRecent, subsrec);
	Options.GetTable(VideoRecent, videorec);
	Options.GetTable(AudioRecent, audsrec);
	Options.GetTable(KEYFRAMES_RECENT, keyframesRecent);

	SetFont(*Options.GetFont());
	wxIcon KaiIcon(L"KAI_SMALL_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
	//::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)GetHiconOf(KaiIcon));
	SetIcon(KaiIcon);
	const wxSize bigIconSize(::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	if (bigIconSize.x > 32){
		wxIcon KaiLargeIcon(L"KAI_TLARGE_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
		::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)GetHiconOf(KaiLargeIcon));
	}
	else{
		::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)GetHiconOf(KaiIcon));
	}

	Menubar = new MenuBar(this);

	//FrameSizer *mains1= new FrameSizer(wxVERTICAL);
	//wxBoxSizer *mains1= new wxBoxSizer(wxVERTICAL);
	//mains=new wxBoxSizer(wxHORIZONTAL);

	Tabs = new Notebook(this, ID_TABS);
	//SetMinSize(wxSize(500,300));
	Toolbar = new KaiToolbar(this, Menubar, -1);

	StatusBar = new KaiStatusBar(this, ID_STATUSBAR1);
	int StatusBarWidths[9] = { -12, 0, 0, 0, 0, 0, 0, 0, -22 };
	StatusBar->SetFieldsCount(9, StatusBarWidths);
	wxString tooltips[] = { L"", _("Skala obrazu wideo"), _("Powiększenie wideo"), _("Czas trwania wideo"),
		_("Klatki na Sekundę"), _("Rozdzielczość wideo"), _("Proporcje wideo"), _("Rozdzielczość napisów"), _("Nazwa pliku wideo") };
	StatusBar->SetTooltips(tooltips, 9);


	/*mains->Add(Toolbar,0,wxEXPAND,0);
	mains->Add(Tabs,1,wxEXPAND,0);
	mains1->Add(Menubar,0,wxEXPAND,0);
	mains1->Add(mains,1,wxEXPAND,0);
	mains1->Add(StatusBar,0,wxEXPAND,0);*/

	FileMenu = new Menu();
	SubsRecMenu = new Menu();
	Menu *lastSession = new Menu();
	lastSession->AppendTool(Toolbar, GLOBAL_LOAD_LAST_SESSION, _("Wczytaj ostatnią sesję"), _("Wczytuje poprzednio zaczytane pliki"), PTR_BITMAP_PNG(L"OPEN_LAST_SESSION"));
	int lastSessionConfig = Options.GetInt(LAST_SESSION_CONFIG);
	lastSession->Append(GLOBAL_ASK_FOR_LOAD_LAST_SESSION, _("Pytaj o wczytanie ostatniej sesji przy starcie programu"), NULL, _("Pyta czy wczytać ostatnio zaczytane pliki przy starcie programu"), ITEM_CHECK_AND_HIDE)->Check(lastSessionConfig == 1);
	lastSession->Append(GLOBAL_LOAD_LAST_SESSION_ON_START, _("Wczytaj ostatnią sesję przy starcie programu"), NULL, _("Wczytuje poprzednio zaczytane pliki przy starcie programu"), ITEM_CHECK_AND_HIDE)->Check(lastSessionConfig == 2);
	

	FileMenu->AppendTool(Toolbar, OpenSubs, _("&Otwórz napisy"), _("Otwórz plik napisów"), PTR_BITMAP_PNG(L"opensubs"));
	FileMenu->AppendTool(Toolbar, SaveSubs, _("&Zapisz"), _("Zapisz aktualny plik"), PTR_BITMAP_PNG(L"save"), false);
	FileMenu->AppendTool(Toolbar, SaveAllSubs, _("Zapisz &wszystko"), _("Zapisz wszystkie napisy"), PTR_BITMAP_PNG(L"saveall"));
	FileMenu->AppendTool(Toolbar, SaveSubsAs, _("Zapisz &jako..."), _("Zapisz jako"), PTR_BITMAP_PNG(L"saveas"));
	FileMenu->AppendTool(Toolbar, SaveTranslation, _("Zapisz &tłumaczenie"), _("Zapisz tłumaczenie"), PTR_BITMAP_PNG(L"savetl"), false);
	FileMenu->AppendTool(Toolbar, RecentSubs, _("Ostatnio otwa&rte napisy"), _("Ostatnio otwarte napisy"), PTR_BITMAP_PNG(L"recentsubs"), true, SubsRecMenu);
	FileMenu->AppendTool(Toolbar, RemoveSubs, _("Usuń napisy z e&dytora"), _("Usuń napisy z edytora"), PTR_BITMAP_PNG(L"close"));
	FileMenu->Append(SaveWithVideoName, _("Zapisuj napisy z nazwą wideo"), _("Zapisuj napisy z nazwą wideo"), true, PTR_BITMAP_PNG(L"SAVEWITHVIDEONAME"), NULL, ITEM_CHECK)->Check(Options.GetBool(SubsAutonaming));
	Toolbar->AddID(SaveWithVideoName);
	FileMenu->Append(9989, _("Pokaż / Ukryj okno logów"))->DisableMapping();
	FileMenu->Append(9990, _("Ostatnia sesja"), _("Opcje ostatniej sesji"), true, PTR_BITMAP_PNG(L"OPEN_LAST_SESSION"), lastSession);
	FileMenu->AppendTool(Toolbar, Settings, _("&Ustawienia"), _("Ustawienia programu"), PTR_BITMAP_PNG(L"SETTINGS"));
	FileMenu->AppendTool(Toolbar, Quit, _("Wyjści&e\tAlt-F4"), _("Zakończ działanie programu"), PTR_BITMAP_PNG(L"exit"))->DisableMapping();
	Menubar->Append(FileMenu, _("&Plik"));

	EditMenu = new Menu();
	EditMenu->AppendTool(Toolbar, Undo, _("&Cofnij"), _("Cofnij"), PTR_BITMAP_PNG(L"undo"), false);
	EditMenu->AppendTool(Toolbar, UndoToLastSave, _("Cofnij do ostatniego zapisu"), _("Cofnij do ostatniego zapisu"), PTR_BITMAP_PNG(L"UNDOTOLASTSAVE"), false);
	EditMenu->AppendTool(Toolbar, Redo, _("&Ponów"), _("Ponów"), PTR_BITMAP_PNG(L"redo"), false);
	EditMenu->AppendTool(Toolbar, History, _("&Historia"), _("Historia"), PTR_BITMAP_PNG(L"history"), true);
	EditMenu->AppendTool(Toolbar, GLOBAL_FIND_REPLACE, _("Znajdź i za&mień"), _("Szuka i podmienia dane frazy tekstu"), PTR_BITMAP_PNG(L"findreplace"));
	EditMenu->AppendTool(Toolbar, Search, _("Z&najdź"), _("Szuka dane frazy tekstu"), PTR_BITMAP_PNG(L"search"));
	Menu *SortMenu[2];
	for (int i = 0; i < 2; i++){
		SortMenu[i] = new Menu();
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_START_TIMES + (6 * i), _("Czas początkowy"), _("Sortuj według czasu początkowego"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_END_TIMES + (6 * i), _("Czas końcowy"), _("Sortuj według czasu końcowego"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_STYLE + (6 * i), _("Style"), _("Sortuj według styli"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_ACTOR + (6 * i), _("Aktor"), _("Sortuj według aktora"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_EFFECT + (6 * i), _("Efekt"), _("Sortuj według efektu"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_LAYER + (6 * i), _("Warstwa"), _("Sortuj według warstwy"));
	}

	EditMenu->AppendTool(Toolbar, SortLines, _("Sort&uj wszystkie linie"), _("Sortuje wszystkie linie napisów ASS"), PTR_BITMAP_PNG(L"sort"), true, SortMenu[0]);
	EditMenu->AppendTool(Toolbar, SortSelected, _("Sortu&j zaznaczone linie"), _("Sortuje zaznaczone linie napisów ASS"), PTR_BITMAP_PNG(L"sortsel"), true, SortMenu[1]);
	EditMenu->AppendTool(Toolbar, GLOBAL_MISSPELLS_REPLACER, _("Popraw drobne błędy (eksperymentalne)"), _("Włącza okno poprawiania błędów"), PTR_BITMAP_PNG(L"sellines"));
	EditMenu->AppendTool(Toolbar, SelectLinesDialog, _("Zaznacz &linijki"), _("Zaznacza linijki wg danej frazy tekstu"), PTR_BITMAP_PNG(L"sellines"));
	Menubar->Append(EditMenu, _("&Edycja"));

	VidMenu = new Menu();
	VidMenu->AppendTool(Toolbar, OpenVideo, _("Otwórz wideo"), _("Otwiera wybrane wideo"), PTR_BITMAP_PNG(L"openvideo"));
	VidsRecMenu = new Menu();
	VidMenu->AppendTool(Toolbar, RecentVideo, _("Ostatnio otwarte wideo"), _("Ostatnio otwarte video"), PTR_BITMAP_PNG(L"recentvideo"), true, VidsRecMenu);
	VidMenu->AppendTool(Toolbar, GLOBAL_KEYFRAMES_OPEN, _("Otwórz klatki kluczowe"), _("Otwórz klatki kluczowe"), PTR_BITMAP_PNG(L"OPEN_KEYFRAMES"));
	KeyframesRecentMenu = new Menu();
	VidMenu->AppendTool(Toolbar, GLOBAL_KEYFRAMES_RECENT, _("Ostatnio otwarte klatki kluczowe"), _("Ostatnio otwarte klatki kluczowe"), PTR_BITMAP_PNG(L"RECENT_KEYFRAMES"), true, KeyframesRecentMenu);
	VidMenu->AppendTool(Toolbar, SetStartTime, _("Wstaw czas początkowy z wideo"), _("Wstawia czas początkowy z wideo"), PTR_BITMAP_PNG(L"setstarttime"), false);
	VidMenu->AppendTool(Toolbar, SetEndTime, _("Wstaw czas końcowy z wideo"), _("Wstawia czas końcowy z wideo"), PTR_BITMAP_PNG(L"setendtime"), false);
	VidMenu->AppendTool(Toolbar, PreviousFrame, _("Klatka w tył"), _("Przechodzi o jedną klatkę w tył"), PTR_BITMAP_PNG(L"prevframe"), false);
	VidMenu->AppendTool(Toolbar, NextFrame, _("Klatka w przód"), _("Przechodzi o jedną klatkę w przód"), PTR_BITMAP_PNG(L"nextframe"), false);
	VidMenu->AppendTool(Toolbar, SetVideoAtStart, _("Przejdź do czasu początkowego linii"), _("Przechodzi wideo do czasu początkowego linii"), PTR_BITMAP_PNG(L"videoonstime"));
	VidMenu->AppendTool(Toolbar, SetVideoAtEnd, _("Przejdź do czasu końcowego linii"), _("Przechodzi wideo do czasu końcowego linii"), PTR_BITMAP_PNG(L"videoonetime"));
	VidMenu->AppendTool(Toolbar, PlayPauseG, _("Odtwarzaj / Pauza"), _("Odtwarza lub pauzuje wideo"), PTR_BITMAP_PNG(L"pausemenu"), false);
	VidMenu->AppendTool(Toolbar, GoToPrewKeyframe, _("Przejdź do poprzedniej klatki kluczowej"), L"", PTR_BITMAP_PNG(L"prevkeyframe"));
	VidMenu->AppendTool(Toolbar, GoToNextKeyframe, _("Przejdź do następnej klatki kluczowej"), L"", PTR_BITMAP_PNG(L"nextkeyframe"));
	VidMenu->AppendTool(Toolbar, SetAudioFromVideo, _("Ustaw audio z czasem wideo"), L"", PTR_BITMAP_PNG(L"SETVIDEOTIMEONAUDIO"));
	VidMenu->AppendTool(Toolbar, SetAudioMarkFromVideo, _("Ustaw znacznik audio z czasem wideo"), L"", PTR_BITMAP_PNG(L"SETVIDEOTIMEONAUDIOMARK"));
	VidMenu->AppendTool(Toolbar, VideoZoom, _("Powiększ wideo"), "", PTR_BITMAP_PNG(L"zoom"));
	bool videoIndex = Options.GetBool(VideoIndex);
	VidMenu->Append(VideoIndexing, _("Otwieraj wideo przez FFMS2"), _("Otwiera wideo przez FFMS2, co daje dokładność klatkową"), true, PTR_BITMAP_PNG(L"FFMS2INDEXING"), 0, ITEM_CHECK)->Check(videoIndex);
	Toolbar->AddID(VideoIndexing);
	Menubar->Append(VidMenu, _("&Wideo"));

	AudMenu = new Menu();
	AudMenu->AppendTool(Toolbar, OpenAudio, _("Otwórz audio"), _("Otwiera wybrane audio"), PTR_BITMAP_PNG(L"openaudio"));
	AudsRecMenu = new Menu();

	AudMenu->AppendTool(Toolbar, RecentAudio, _("Ostatnio otwarte audio"), _("Ostatnio otwarte audio"), PTR_BITMAP_PNG(L"recentaudio"), true, AudsRecMenu);
	AudMenu->AppendTool(Toolbar, AudioFromVideo, _("Otwórz audio z wideo"), _("Otwiera audio z wideo"), PTR_BITMAP_PNG(L"audiofromvideo"));
	AudMenu->AppendTool(Toolbar, CloseAudio, _("Zamknij audio"), _("Zamyka audio"), PTR_BITMAP_PNG(L"closeaudio"));
	Menubar->Append(AudMenu, _("A&udio"));

	ViewMenu = new Menu();
	ViewMenu->Append(ViewAll, _("Wszystko"), _("Wszystkie okna są widoczne"));
	ViewMenu->Append(ViewVideo, _("Wideo i napisy"), _("Widoczne tylko okno wideo i napisów"));
	ViewMenu->Append(ViewAudio, _("Audio i napisy"), _("Widoczne tylko okno audio i napisów"));
	ViewMenu->Append(GLOBAL_VIEW_ONLY_VIDEO, _("Tylko wideo"), _("Widoczne tylko okno wideo"));
	ViewMenu->Append(ViewSubs, _("Tylko napisy"), _("Widoczne tylko okno napisów"));
	Menubar->Append(ViewMenu, _("Wido&k"));

	SubsMenu = new Menu();
	SubsMenu->AppendTool(Toolbar, Editor, _("Włącz / Wyłącz edytor"), _("Włączanie bądź wyłączanie edytora"), PTR_BITMAP_PNG(L"editor"));
	SubsMenu->AppendTool(Toolbar, ASSProperties, _("Właściwości ASS"), _("Właściwości napisów ASS"), PTR_BITMAP_PNG(L"ASSPROPS"));
	SubsMenu->AppendTool(Toolbar, StyleManager, _("&Menedżer stylów"), _("Służy do zarządzania stylami ASS"), PTR_BITMAP_PNG(L"styles"));
	ConvMenu = new Menu();
	ConvMenu->AppendTool(Toolbar, ConvertToASS, _("Konwertuj do ASS"), _("Konwertuje do formatu ASS"), PTR_BITMAP_PNG(L"convass"), false);
	ConvMenu->AppendTool(Toolbar, ConvertToSRT, _("Konwertuj do SRT"), _("Konwertuje do formatu SRT"), PTR_BITMAP_PNG(L"convsrt"));
	ConvMenu->AppendTool(Toolbar, ConvertToMDVD, _("Konwertuj do MDVD"), _("Konwertuje do formatu microDVD"), PTR_BITMAP_PNG(L"convmdvd"));
	ConvMenu->AppendTool(Toolbar, ConvertToMPL2, _("Konwertuj do MPL2"), _("Konwertuje do formatu MPL2"), PTR_BITMAP_PNG(L"convmpl2"));
	ConvMenu->AppendTool(Toolbar, ConvertToTMP, _("Konwertuj do TMP"), _("Konwertuje do formatu TMPlayer (niezalecene)"), PTR_BITMAP_PNG(L"convtmp"));

	SubsMenu->Append(ID_CONV, _("Konwersja"), _("Konwersja z jednego formatu napisów na inny"), true, PTR_BITMAP_PNG(L"convert"), ConvMenu);
	SubsMenu->AppendTool(Toolbar, ChangeTime, _("Okno zmiany &czasów\tCtrl-I"), _("Przesuwanie czasów napisów"), PTR_BITMAP_PNG(L"times"));
	SubsMenu->AppendTool(Toolbar, FontCollectorID, _("Kolekcjoner czcionek"), _("Kolekcjoner czcionek"), PTR_BITMAP_PNG(L"fontcollector"));
	SubsMenu->AppendTool(Toolbar, SubsResample, _("Zmień rozdzielczość napisów"), _("Zmień rozdzielczość napisów"), PTR_BITMAP_PNG(L"subsresample"));
	SubsMenu->AppendTool(Toolbar, SpellcheckerDialog, _("Sprawdź poprawność pisowni"), _("Sprawdź poprawność pisowni"), PTR_BITMAP_PNG(L"spellchecker"));
	SubsMenu->AppendTool(Toolbar, HideTags, _("Ukryj tagi w nawiasach"), _("Ukrywa tagi w nawiasach ASS i MDVD"), PTR_BITMAP_PNG(L"hidetags"));
	Menubar->Append(SubsMenu, _("&Napisy"));

	AutoMenu = new Menu();
	AutoMenu->AppendTool(Toolbar, AutoLoadScript, _("Wczytaj skrypt"), _("Wczytaj skrypt"), PTR_BITMAP_PNG(L"automation"));
	AutoMenu->Append(AutoReloadAutoload, _("Odśwież skrypty autoload"), _("Odśwież skrypty autoload"), true, PTR_BITMAP_PNG(L"automation"));
	AutoMenu->Append(LoadLastScript, _("Uruchom ostatnio zaczytany skrypt"), _("Uruchom ostatnio zaczytany skrypt"));
	AutoMenu->Append(AUTOMATION_OPEN_HOTKEYS_WINDOW, _("Otwórz okno mapowania skrótów"), _("Otwórz okno mapowania skrótów"));
	Menubar->Append(AutoMenu, _("Au&tomatyzacja"));

	HelpMenu = new Menu();
	HelpMenu->AppendTool(Toolbar, Help, _("&Pomoc (niekompletna, ale jednak)"), _("Otwiera pomoc w domyślnej przeglądarce"), PTR_BITMAP_PNG(L"help"));
	HelpMenu->AppendTool(Toolbar, ANSI, _("&Wątek programu na forum AnimeSub.info"), _("Otwiera wątek programu na forum AnimeSub.info"), PTR_BITMAP_PNG(L"ansi"));
	HelpMenu->AppendTool(Toolbar, About, _("&O programie"), _("Wyświetla informacje o programie"), PTR_BITMAP_PNG(L"about"));
	HelpMenu->AppendTool(Toolbar, Helpers, _("&Lista osób pomocnych przy tworzeniu programu"), _("Wyświetla listę osób pomocnych przy tworzeniu programu"), PTR_BITMAP_PNG(L"helpers"));
	Menubar->Append(HelpMenu, _("Pomo&c"));

	Toolbar->InitToolbar();

	//SetSizer(mains1);

	SetAccels(false);


	Connect(ID_TABS, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageChanged, 0, this);
	Connect(ID_ADDPAGE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageAdd);
	Connect(ID_CLOSEPAGE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageClose);
	Connect(NextTab, PreviousTab, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageChange);
	//Here add new ids
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, SaveSubs, History);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, GLOBAL_SORT_ALL_BY_START_TIMES, GLOBAL_SORT_SELECTED_BY_LAYER);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, GLOBAL_SHIFT_TIMES);
	Connect(OpenSubs, ANSI, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnMenuSelected1);
	Connect(SelectFromVideo, PlayActualLine, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnMenuSelected1);
	Connect(Plus5SecondG, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnP5Sec);
	Connect(Minus5SecondG, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnM5Sec);
	Connect(PreviousLine, JoinWithNext, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnChangeLine);
	Connect(Remove, RemoveText, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnDelete);
	Menubar->Connect(EVT_MENU_OPENED, (wxObjectEventFunction)&KainoteFrame::OnMenuOpened, 0, this);
	Connect(wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&KainoteFrame::OnClose1);
	Connect(30000, 30079, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnRecent);
	Connect(PlayActualLine, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&KainoteFrame::OnMenuSelected1);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &event){
		LogHandler::ShowLogWindow();
	}, 9989);

	Bind(wxEVT_ACTIVATE, &KainoteFrame::OnActivate, this);
	Connect(SnapWithStart, SnapWithEnd, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnAudioSnap);
	Tabs->SetDropTarget(new DragnDrop(this));
	Bind(wxEVT_SIZE, &KainoteFrame::OnSize, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, 
		GLOBAL_ASK_FOR_LOAD_LAST_SESSION, GLOBAL_LOAD_LAST_SESSION_ON_START);

	auto focusFunction = [=](wxFocusEvent &event) -> void {
		TabPanel *tab = GetTab();
		if (tab->lastFocusedWindow){
			tab->lastFocusedWindow->SetFocus();
		}
		else if (tab->Grid->IsShown()){
			tab->Grid->SetFocus();
		}//test why it was disabled or fix this bug
		else if (tab->Video->IsShown()){
			tab->Video->SetFocus();
		}

	};
	bool im = Options.GetBool(WindowMaximized);
	if (im){ Maximize(Options.GetBool(WindowMaximized)); }

	if (!Options.GetBool(EditorOn)){ HideEditor(false); }
	std::set_new_handler(OnOutofMemory);
	FontEnum.StartListening(this);
	SetSubsResolution(false);
	Auto = new Auto::Automation();

	EnableCrashingOnCrashes();
	sendFocus.SetOwner(this, 6789);


	Bind(wxEVT_SET_FOCUS, focusFunction);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		//if it will crash on last focused window
		//we need to remove last focused window
		//it's not needed here
		focusFunction(wxFocusEvent());
	}, 6789);
}

KainoteFrame::~KainoteFrame()
{
	//this will prevent crashes when function want to use relesed elements
	//when closing it can be skipped
	Options.SetClosing();
	Unbind(wxEVT_ACTIVATE, &KainoteFrame::OnActivate, this);
	bool im = IsMaximized();
	if (!im && !IsIconized()){
		int posx, posy, sizex, sizey;
		GetPosition(&posx, &posy);
		if (posx < 4000 && posx> -200){
			Options.SetCoords(WindowPosition, posx, posy);
		}
		GetSize(&sizex, &sizey);
		Options.SetCoords(WindowSize, sizex, sizey);
	}

	Toolbar->Destroy();
	Options.SetBool(WindowMaximized, im);
	Options.SetBool(EditorOn, GetTab()->editor);
	Options.SetTable(SubsRecent, subsrec);
	Options.SetTable(VideoRecent, videorec);
	Options.SetTable(AudioRecent, audsrec);
	Options.SetInt(VideoVolume, GetTab()->Video->volslider->GetValue());
	Notebook::SaveLastSession(true);
	//Tabs->Destroy();
	//Tabs = NULL;
	//destroy findreplace before saving options it saving findreplace options in destructor
	if (FR){ FR->SaveOptions(); FR->Destroy(); FR = NULL; }
	if (SL){ SL->SaveOptions(); SL->Destroy(); SL = NULL; }
	Options.SaveOptions();

	StyleStore::DestroyStore();
	if (Auto){ delete Auto; Auto = NULL; }
	if (FC){ delete FC; FC = NULL; }
	SpellChecker::Destroy();
	VideoToolbar::DestroyIcons();
	LogHandler::Destroy();
}

void KainoteFrame::DestroyDialogs(){
	if (FR){ FR->SaveOptions(); FR->Destroy(); FR = NULL; }
	if (SL){ SL->SaveOptions(); SL->Destroy(); SL = NULL; }
	if (FC){ delete FC; FC = NULL; }
	if (MR){ MR->Destroy(); MR = NULL; }
	StyleStore::DestroyStore();
};


//elementy menu które ulegają wyłączaniu
void KainoteFrame::OnMenuSelected(wxCommandEvent& event)
{
	int id = event.GetId();
	int Modif = event.GetInt();
	MenuItem *item = Menubar->FindItem(id);

	TabPanel *tab = GetTab();
	if (Modif == wxMOD_SHIFT && item){
		Hkeys.OnMapHkey(id, L"", this, GLOBAL_HOTKEY);
		return;
	}
	if (item && !item->enabled)
		return;


	if (id == SaveSubs){
		Save(false);
	}
	else if (id == SaveSubsAs){
		Save(true);
	}
	else if (id == SaveAllSubs){
		SaveAll();
	}
	else if (id == SaveTranslation){
		GetTab()->Grid->AddSInfo(L"TLMode", L"Translated", false);
		Save(true, -1, false);
		GetTab()->Grid->AddSInfo(L"TLMode", L"Yes", false);
	}
	else if (id == RemoveSubs){
		if (SavePrompt(3)){ event.SetInt(-1); return; }
		if (tab->SubsPath != L""){
			tab->SubsName = _("Bez tytułu");
			tab->SubsPath = L"";
			Label();

			tab->Grid->Clearing();
			tab->Grid->file = new SubsFile();
			tab->Grid->LoadDefault();
			tab->Edit->RefreshStyle(true);
			tab->Grid->RefreshColumns();

			if (tab->Video->GetState() != None){
				if (tab->Video->Visual)
					tab->Video->SetVisual(true);
				else{
					tab->Video->OpenSubs(NULL);
					tab->Video->Render();
				}
			}
			SetSubsResolution(false);
		}
	}
	else if (id == Undo){
		tab->Grid->GetUndo(false);
	}
	else if (id == Redo){
		tab->Grid->GetUndo(true);
	}
	else if (id == History){
		tab->Grid->file->ShowHistory(this, [=](int iter){
			tab->Grid->GetUndo(false, iter);
		});
	}
	else if (id == Search || id == GLOBAL_FIND_REPLACE){
		if (!FR){ FR = new FindReplaceDialog(this, (id == GLOBAL_FIND_REPLACE) ? WINDOW_REPLACE : WINDOW_FIND); }
		else{ FR->ShowDialog((id == GLOBAL_FIND_REPLACE) ? WINDOW_REPLACE : WINDOW_FIND); }
	}
	else if (id == SelectLinesDialog){
		if (!SL){ SL = new SelectLines(this); }
		SL->Show();
	}
	else if (id == PlayPauseG){
		tab->Video->Pause();
	}
	else if ((id == PreviousFrame || id == NextFrame)){
		tab->Video->ChangePositionByFrame((id == PreviousFrame) ? -1 : 1);
	}
	else if (id == SetStartTime || id == SetEndTime){
		if (tab->Video->GetState() != None){
			if (id == SetStartTime){
				int time = tab->Video->GetFrameTime() + Options.GetInt(InsertStartOffset);
				tab->Grid->SetStartTime(ZEROIT(time));
			}
			else{
				int time = tab->Video->GetFrameTime(false) + Options.GetInt(InsertEndOffset);
				tab->Grid->SetEndTime(ZEROIT(time));
			}
		}
	}
	else if (id == SetAudioFromVideo || id == SetAudioMarkFromVideo){
		if (tab->Edit->ABox){
			AudioDisplay *adisp = tab->Edit->ABox->audioDisplay;
			int time = tab->Video->Tell();
			int pos = adisp->GetXAtMS(time);
			if (id == SetAudioMarkFromVideo)
				adisp->SetMark(time);
			adisp->ChangePosition(time);
		}
	}
	else if (id == VideoIndexing || id == SaveWithVideoName){
		toolitem *ToolItem = Toolbar->FindItem(id);
		MenuItem *Item = (item) ? item : Menubar->FindItem(id);
		CONFIG conf = (id == VideoIndexing) ? VideoIndex : SubsAutonaming;
		if (Modif == 1000 && ToolItem){
			if (Item)
				Item->Check(ToolItem->toggled);
			Options.SetBool(conf, ToolItem->toggled);
		}
		else if (Item){
			if (ToolItem){
				ToolItem->toggled = (id == Modif) ? !ToolItem->toggled : Item->IsChecked();
				Toolbar->Refresh(false);
			}
			if (id == Modif && Item)
				Item->Check(!Item->IsChecked());

			Options.SetBool(conf, Item->IsChecked());
		}
	}
	else if (id == VideoZoom){
		tab->Video->SetZoom();
	}
	else if (id >= OpenAudio && id <= CloseAudio){
		OnOpenAudio(event);
	}
	else if (id == ASSProperties){
		OnAssProps();
	}
	else if (id == StyleManager){
		StyleStore::ShowStore();
	}
	else if (id == SubsResample){
		TabPanel *tab = GetTab();
		int x = 0, y = 0;
		tab->Grid->GetASSRes(&x, &y);
		SubsResampleDialog SRD(this, wxSize(x, y), (tab->Video->GetState() == None) ? wxSize(x, y) : tab->Video->GetVideoSize(), L"", L"");
		SRD.ShowModal();
	}
	else if (id == FontCollectorID && tab->Grid->subsFormat < SRT){
		if (!FC){ FC = new FontCollector(this); }
		FC->ShowDialog(this);
	}
	else if (id == GLOBAL_MISSPELLS_REPLACER){
		if (!MR){ MR = new MisspellReplacer(this); }
		MR->Show(!MR->IsShown());
	}
	else if (id >= ConvertToASS && id <= ConvertToMPL2){
		if (tab->Grid->GetSInfo(L"TLMode") != L"Yes"){
			OnConversion((id - ConvertToASS) + 1);
		}
	}
	else if (id == SpellcheckerDialog){
		SpellCheckerDialog *SCD = new SpellCheckerDialog(this);
	}
	else if (id == HideTags){
		tab->Grid->HideOverrideTags();
	}
	else if (id == ChangeTime){
		bool show = !tab->ShiftTimes->IsShown();
		Options.SetBool(MoveTimesOn, show);
		tab->ShiftTimes->Show(show);
		tab->BoxSizer3->Layout();
	}
	else if (id >= GLOBAL_SORT_ALL_BY_START_TIMES && id <= GLOBAL_SORT_SELECTED_BY_LAYER){
		bool all = id < GLOBAL_SORT_SELECTED_BY_START_TIMES;
		int difid = (all) ? GLOBAL_SORT_ALL_BY_START_TIMES : GLOBAL_SORT_SELECTED_BY_START_TIMES;
		tab->Grid->SortIt(id - difid, all);
	}
	else if (id >= ViewAll&&id <= ViewSubs){
		bool vidshow = (id == ViewAll || id == ViewVideo || id == GLOBAL_VIEW_ONLY_VIDEO) && tab->Video->GetState() != None;
		bool vidvis = tab->Video->IsShown();
		if (!vidshow && tab->Video->GetState() == Playing){ tab->Video->Pause(); }
		tab->Video->Show(vidshow);
		if (vidshow && !vidvis){
			tab->Video->OpenSubs(tab->Grid->GetVisible());
		}
		if (tab->Edit->ABox){
			tab->Edit->ABox->Show((id == ViewAll || id == ViewAudio));
			if (id == ViewAudio){ tab->Edit->SetMinSize(wxSize(500, 350)); }
			if (id != ViewAudio && id != ViewAll){
				tab->Edit->windowResizer->Show(false);
			}
			else if (!tab->Edit->windowResizer->IsShown())
				tab->Edit->windowResizer->Show();
		}
		if (id == GLOBAL_VIEW_ONLY_VIDEO){
			tab->Edit->Show(false);
			tab->Grid->Show(false);
			tab->ShiftTimes->Show(false);
			tab->windowResizer->Show(false);
			wxSize tabSize = tab->GetClientSize();
			tab->Video->SetMinSize(tabSize);
		}
		else if (!tab->Edit->IsShown()){
			tab->Edit->Show();
			tab->Grid->Show();
			tab->ShiftTimes->Show();
			tab->windowResizer->Show();
			int x = 0, y = 0;
			Options.GetCoords(VideoWindowSize, &x, &y);
			tab->Video->SetMinSize(wxSize(x, y));
		}
		tab->Layout();
	}
	else if (id == AutoLoadScript){
		//if (!Auto){ Auto = new Auto::Automation(); }
		if (Auto->ASSScripts.size() < 1)
			Auto->AddFromSubs();
		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz sktypt"),
			Options.GetString(AutomationRecent),
			L"", _("Pliki skryptów (*.lua),(*.moon)|*.lua;*.moon;"), wxFD_OPEN);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxString file = FileDialog1->GetPath();
			Options.SetString(AutomationRecent, file.AfterLast(L'\\'));
			//if(Auto->Add(file)){Auto->BuildMenu(&AutoMenu);}
			Auto->Add(file);
		}
		FileDialog1->Destroy();

	}
	else if (id == AutoReloadAutoload){
		/*if (!Auto){ Auto = new Auto::Automation(); }
		else{ */Auto->ReloadScripts(); //}
	//Auto->BuildMenu(&AutoMenu);
	}
	else if (id == LoadLastScript){
		//if (!Auto){ Auto = new Auto::Automation(true); }
		if (Auto->ASSScripts.size() < 1)
			Auto->AddFromSubs();
		int size = Auto->ASSScripts.size();
		if (!size){ KaiMessageBox(_("Ten plik napisów nie ma dodanych żadnych skryptów"), _("Informacja"), wxOK, this); return; }
		auto script = Auto->ASSScripts[size - 1];
		if (script->CheckLastModified(true)){ script->Reload(); }
		auto macro = script->GetMacro(0);
		if (macro){
			if (macro->Validate(tab)){
				macro->Run(tab);
			}
			else{
				KaiMessageBox(wxString::Format(_("Warunki skryptu Lua '%s' nie zostały spełnione"), script->GetPrettyFilename()), _("Błąd"), wxOK, this);
			}
		}
		else{
			KaiMessageBox(wxString::Format(_("Błąd wczytywania skryptu Lua: %s\n%s"), script->GetPrettyFilename(), script->GetDescription()), _("Błąd"), wxOK, this);
			Auto->OnEdit(script->GetFilename());
		}
	}
	else if (id == AUTOMATION_OPEN_HOTKEYS_WINDOW){
		Auto->ShowScriptHotkeysWindow(this);
	}
	else if (id == GoToPrewKeyframe){
		tab->Video->GoToPrevKeyframe();
		tab->Video->RefreshTime();
	}
	else if (id == GoToNextKeyframe){
		tab->Video->GoToNextKeyframe();
		tab->Video->RefreshTime();
	}
	else if (id == SetVideoAtStart){
		int fsel = tab->Grid->FirstSelection();
		if (fsel < 0){ return; }
		tab->Video->Seek(MAX(0, tab->Grid->GetDialogue(fsel)->Start.mstime), true);
	}
	else if (id == SetVideoAtEnd){
		int fsel = tab->Grid->FirstSelection();
		if (fsel < 0){ return; }
		tab->Video->Seek(MAX(0, tab->Grid->GetDialogue(fsel)->End.mstime), false);
	}
	else if (id == UndoToLastSave){
		tab->Grid->GetUndo(false, tab->Grid->file->GetLastSaveIter());
	}
	else if (id == GLOBAL_LOAD_LAST_SESSION){
		Tabs->LoadLastSession(this);
	}
	else if (id == GLOBAL_LOAD_LAST_SESSION_ON_START){
		// 0 nothing 1 ask for load 2 load on start
		int config = (int)item->IsChecked() * 2;
		Options.SetInt(LAST_SESSION_CONFIG, config);
		MenuItem *item = Menubar->FindItem(GLOBAL_ASK_FOR_LOAD_LAST_SESSION);
		if (item && config) { item->Check(false); }
	}
	else if (id == GLOBAL_ASK_FOR_LOAD_LAST_SESSION){
		// 0 nothing 1 ask for load 2 load on start
		int config = (int)item->IsChecked();
		Options.SetInt(LAST_SESSION_CONFIG, config);
		MenuItem *item = Menubar->FindItem(GLOBAL_LOAD_LAST_SESSION_ON_START);
		if (item && config) { item->Check(false); }
	}
	else if (id == GLOBAL_SHIFT_TIMES){
		tab->ShiftTimes->OnOKClick(event);
	}

}
//Stałe elementy menu które nie ulegają wyłączaniu
void KainoteFrame::OnMenuSelected1(wxCommandEvent& event)
{
	int id = event.GetId();
	int Modif = event.GetInt();
	TabPanel *pan = GetTab();

	if (Modif == wxMOD_SHIFT){
		Hkeys.OnMapHkey(id, L"", this, GLOBAL_HOTKEY);
		return;
	}
	if (id == OpenSubs){
		//nie ma potrzeby pytać, bo w open file zapyta
		//if(SavePrompt(2)){return;}

		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik napisów"),
			(GetTab()->VideoPath != L"") ? GetTab()->VideoPath.BeforeLast(L'\\') :
			(subsrec.size() > 0) ? subsrec[0].BeforeLast(L'\\') : L"",
			L"", _("Pliki napisów (*.ass),(*.ssa),(*.srt),(*.sub),(*.txt)|*.ass;*.ssa;*.srt;*.sub;*.txt|Pliki wideo z wbudowanymi napisami (*.mkv)|*.mkv"),
			wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxArrayString paths;
			FileDialog1->GetPaths(paths);
			Freeze();
			GetTab()->Hide();
			for (auto &file : paths){
				if (GetTab()->SubsPath != L""){ InsertTab(false); }
				if (file.AfterLast(L'.') == L"mkv"){
					event.SetString(file);
					GetTab()->Grid->OnMkvSubs(event);
				}
				else{
					OpenFile(file);
				}
			}
			Thaw();
			GetTab()->Show();
		}
		FileDialog1->Destroy();
	}
	else if (id == OpenVideo){
		wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"),
			(videorec.size() > 0) ? videorec[0].BeforeLast(L'\\') : L"",
			L"", _("Pliki wideo(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.mpg),(*.mpeg),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.mpg;*.mpeg;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"),
			wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
		if (FileDialog2->ShowModal() == wxID_OK){
			wxArrayString paths;
			FileDialog2->GetPaths(paths);
			if (paths.size() < 2 && paths.size() > 0)
				OpenFile(paths[0]);
			else
				OpenFiles(paths);
		}
		FileDialog2->Destroy();
	}
	else if (id == GLOBAL_KEYFRAMES_OPEN){
		wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"),
			(keyframesRecent.size() > 0) ? keyframesRecent[0].BeforeLast(L'\\') : L"",
			L"", _("Pliki klatek kluczowych (*.txt),(*.pass),(*.stats),(*.log)|*.txt;*.pass;*.stats;*.log|Wszystkie pliki (*.*)|*.*"),
			wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (FileDialog2->ShowModal() == wxID_OK){
			wxString path = FileDialog2->GetPath();
			GetTab()->KeyframesPath = path;
			GetTab()->Video->OpenKeyframes(path);
			SetRecent(3);
		}
		FileDialog2->Destroy();
	}
	else if (id == Settings){
		OptionsDialog od(this, this);
		od.OptionsTree->ChangeSelection(0);
		od.ShowModal();
	}
	else if (id == SelectFromVideo){
		GetTab()->Grid->SelVideoLine();
	}
	else if (id == Editor){
		HideEditor();
	}
	else if (id == PlayActualLine){
		TabPanel *tab = GetTab();
		tab->Edit->TextEdit->SetFocus();
		tab->Video->PlayLine(tab->Edit->line->Start.mstime, tab->Video->GetPlayEndTime(tab->Edit->line->End.mstime));
	}
	else if (id == Quit){
		Close();
	}
	else if (id == About){
		KaiMessageBox(wxString::Format(_("Edytor napisów by Marcin Drob aka Bakura lub Bjakja (bjakja7@gmail.com),\nwersja %s z dnia %s"),
			Options.progname.AfterFirst(L'v'), Options.GetReleaseDate()) + " \n\n" +
			_("Ten program powstał w celu zastąpienia dwóch programów: Bestplayera i Aegisuba.\n\n") +
			_("Jeśli zauważyłeś(aś) jakieś błędy bądź masz jakieś propozycje zmian lub nowych funkcji,\nmożesz napisać o tym na: forum ANSI, Githubie, bądź mailowo.\n\n") +
			_("Kainote zawiera w sobie części następujących projeków:\n") +
			L"wxWidgets - Copyright © Julian Smart, Robert Roebling et al.\n" +
			_("Color picker, wymuxowywanie napsów z mkv, audiobox, audio player, automation\ni kilka innych pojedynczych funkcji wzięte z Aegisuba -\n") +
			L"Copyright © Rodrigo Braz Monteiro.\n"\
			L"Hunspell - Copyright © Kevin Hendricks.\n"\
			L"Matroska Parser - Copyright © Mike Matsnev.\n"\
			L"CSRI - Copyright © David Lamparter.\n"\
			L"Vsfilter - Copyright © Gabest.\n"\
			L"FFMPEGSource2 - Copyright © Fredrik Mellbin.\n"\
			L"ICU - Copyright © 1995-2016 International Business Machines Corporation and others.\n"\
			L"Boost - Copyright © Joe Coder 2004 - 2006.",
			_("O Kainote"));
			//L"Interfejs Avisynth - Copyright © Ben Rudiak-Gould et al.\n"
	}
	else if (id == Helpers){
		wxString Testers = L"Wtas, BadRequest, Ognisty321, Nyah2211, dark, Ksenoform, Zły Los.";
		wxString Credits = _("Pomoc graficzna: (przyciski, obrazki do pomocy itd.)\n") +
			_("- Kostek00 (przyciski do audio i narzędzi wizualnych).\n") +
			_("- Xandros (nowe przyciski do wideo).\n") +
			_("- Devilkan (ikony do menu i paska narzędzi, obrazki do pomocy).\n") +
			_("- Zły Los (ikony do skojarzonych plików i do menu).\n") +
			_("- Areki, duplex (tłumaczenie anglojęzyczne).\n \n") +
			_("Testerzy: (mniej i bardziej wprawieni użytkownicy programu)\n") +
			_("- Funki27 (pierwszy tester mający spory wpływ na obecne działanie programu\n") +
			_("i najbardziej narzekający na wszystko).\n") +
			_("- Sacredus (chyba pierwszy tłumacz używający trybu tłumaczenia,\n nieoceniona pomoc przy testowaniu wydajności na słabym komputerze).\n") +
			_("- Kostek00 (prawdziwy wynajdywacz błędów, miał duży wpływ na rozwój spektrum audio \n") +
			_("i głównego pola tekstowego, stworzył ciemny i jasny motyw, a także część ikon).\n") +
			_("- Devilkan (crashhunter, ze względu na swój system i przyzwyczajenia wytropił już wiele crashy,\n") +
			_("pomógł w poprawie działania narzędzi do typesettingu, wymyślił wiele innych usprawnień).\n") +
			_("- MatiasMovie (wyłapał parę crashy i zaproponował różne usprawnienia, pomaga w debugowaniu crashy).\n") +
			_("- mas1904 (wyłapał trochę błędów, pomaga w debugowaniu crashy).\n") +/* i jar do Language Tool*/
			_("- Senami (stworzył nowe motywy a także wyłapał parę błędów).\n") +
			_("- Wincenty271 (wyłapał trochę błędów, a także pomaga w debugowaniu kraszy).\n \n") +
			_("Podziękowania także dla osób, które używają programu i zgłaszali błędy.\n");
		KaiMessageBox(Credits + Testers, _("Lista osób pomocnych przy tworzeniu programu"));

	}
	else if (id == Help || id == ANSI){
		//WinStruct<SHELLEXECUTEINFO> sei;
		wxString url = (id == Help) ? L"https://bjakja.github.io/index.html" : L"http://animesub.info/forum/viewtopic.php?id=258715";
		//sei.lpFile = url.c_str();
		//sei.lpVerb = wxT("open");
		//sei.nShow = SW_RESTORE;
		//sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves
		//ShellExecuteEx(&sei);
		OpenInBrowser(url);
	}

}


void KainoteFrame::OnConversion(char form)
{
	TabPanel *tab = GetTab();
	if (tab->Grid->GetSInfo(L"TLMode") == L"Yes"){ return; }
	if (form != ASS && tab->Edit->Visual){
		tab->Video->SetVisual(true, false, true);
	}
	tab->Grid->Convert(form);
	tab->ShiftTimes->Contents();
	UpdateToolbar();
	tab->Edit->HideControls();
	tab->Video->vToolbar->DisableVisuals(form != ASS);
}


void KainoteFrame::OnAssProps()
{
	int x = -1, y = -1;
	if (GetTab()->Video->GetState() != None){ GetTab()->Video->GetVideoSize(&x, &y); }
	ScriptInfo sci(this, x, y);
	SubsGrid *ngrid = GetTab()->Grid;
	sci.title->SetValue(ngrid->GetSInfo(L"Title"));
	sci.script->SetValue(ngrid->GetSInfo(L"Original Script"));
	sci.translation->SetValue(ngrid->GetSInfo(L"Original Translation"));
	sci.editing->SetValue(ngrid->GetSInfo(L"Original Editing"));
	sci.timing->SetValue(ngrid->GetSInfo(L"Original Timing"));
	sci.update->SetValue(ngrid->GetSInfo(L"Script Updated By"));
	int nx = 0, ny = 0;
	ngrid->GetASSRes(&nx, &ny);
	sci.width->SetInt(nx);
	sci.height->SetInt(ny);
	const wxString &matrix = ngrid->GetSInfo(L"YCbCr Matrix");
	int result = sci.matrix->FindString(matrix);
	if (matrix.IsEmpty() || result < 0){
		sci.matrix->SetSelection(0);
		result = 0;
	}
	else{
		sci.matrix->SetSelection(result);
	}
	const wxString &wraps = ngrid->GetSInfo(L"WrapStyle");
	int ws = wxAtoi(wraps);
	sci.wrapstyle->SetSelection(ws);
	const wxString &colls = ngrid->GetSInfo(L"Collisions");
	if (colls == L"Reverse"){ sci.collision->SetSelection(1); }
	const wxString &bords = ngrid->GetSInfo(L"ScaledBorderAndShadow");
	if (bords == L"no"){ sci.scaleBorderAndShadow->SetValue(false); }

	if (sci.ShowModal() == wxID_OK)
	{
		int newx = sci.width->GetInt();
		int newy = sci.height->GetInt();
		if (newx < 1 && newy < 1){ newx = 1280; newy = 720; }
		else if (newx < 1){ newx = (float)newy*(4.0 / 3.0); }
		else if (newy < 1){ newy = (float)newx*(3.0 / 4.0); if (newx == 1280){ newy = 1024; } }

		if (sci.title->GetValue() != L""){ if (sci.title->IsModified()){ ngrid->AddSInfo(L"Title", sci.title->GetValue()); } }
		else{ ngrid->AddSInfo(L"Title", L"Kainote Ass File"); }
		if (sci.script->IsModified()){ ngrid->AddSInfo(L"Original Script", sci.script->GetValue()); }
		if (sci.translation->IsModified()){ ngrid->AddSInfo(L"Original Translation", sci.translation->GetValue()); }
		if (sci.editing->IsModified()){ ngrid->AddSInfo(L"Original Editing", sci.editing->GetValue()); }
		if (sci.timing->IsModified()){ ngrid->AddSInfo(L"Original Timing", sci.timing->GetValue()); }
		if (sci.update->IsModified()){ ngrid->AddSInfo(L"Script Updated By", sci.update->GetValue()); }

		if (sci.width->IsModified()){ ngrid->AddSInfo(L"PlayResX", wxString::Format(L"%i", newx)); }
		if (sci.height->IsModified()){ ngrid->AddSInfo(L"PlayResY", wxString::Format(L"%i", newy)); }
		int newMatrix = sci.matrix->GetSelection();
		if (newMatrix != result){
			wxString val = sci.matrix->GetString(sci.matrix->GetSelection());
			ngrid->AddSInfo(L"YCbCr Matrix", val);
			GetTab()->Video->SetColorSpace(val);
		}

		if (ws != sci.wrapstyle->GetSelection()){ ngrid->AddSInfo(L"WrapStyle", wxString::Format(L"%i", sci.wrapstyle->GetSelection())); }
		wxString collis = (sci.collision->GetSelection() == 0) ? L"Normal" : L"Reverse";
		if (colls != collis){ ngrid->AddSInfo(L"Collisions", collis); }
		wxString bordas = (sci.scaleBorderAndShadow->GetValue()) ? L"yes" : L"no";
		if (bords != bordas){ ngrid->AddSInfo(L"ScaledBorderAndShadow", bordas); }
		ngrid->SetModified(ASS_PROPERTIES);
		SetSubsResolution();
	}
}

void KainoteFrame::Save(bool dial, int wtab, bool changeLabel)
{
	TabPanel* atab = (wtab < 0) ? GetTab() : Tabs->Page(wtab);
	if (atab->Grid->originalFormat != atab->Grid->subsFormat
		|| (Options.GetBool(SubsAutonaming)
		&& atab->SubsName.BeforeLast(L'.') != atab->VideoName.BeforeLast(L'.') && atab->VideoName != L"")
		|| atab->SubsPath == L"" || dial)
	{
	repeatOpening:
		wxString extens = _("Plik napisów ");

		if (atab->Grid->subsFormat < SRT){ extens += L"(*.ass)|*.ass"; }
		else if (atab->Grid->subsFormat == SRT){ extens += L"(*.srt)|*.srt"; }
		else{ extens += L"(*.txt, *.sub)|*.txt;*.sub"; };

		wxString path = (atab->VideoPath != L"" && Options.GetBool(SubsAutonaming)) ? atab->VideoPath : atab->SubsPath;
		wxString name = path.BeforeLast(L'.');
		path = path.BeforeLast(L'\\');

		wxWindow *_parent = (atab->Video->isFullscreen) ? (wxWindow*)atab->Video->TD : this;
		wxFileDialog saveFileDialog(_parent, _("Zapisz plik napisów"),
			path, name, extens, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_OK){
			wxString path = saveFileDialog.GetPath();
			DWORD attributes = ::GetFileAttributesW(path.wc_str());
			if (attributes != -1 && attributes & FILE_ATTRIBUTE_READONLY){
				KaiMessageBox(_("Wybrany plik jest tylko do odczytu,\nproszę zapisać pod inną nazwą lub zmienić atrybuty pliku"), _("Uwaga"), 4L, this);
				goto repeatOpening;
			}

			atab->SubsPath = path;
			wxString ext = (atab->Grid->subsFormat < SRT) ? L"ass" : (atab->Grid->subsFormat == SRT) ? L"srt" : L"txt";
			if (!atab->SubsPath.EndsWith(ext)){ atab->SubsPath << L"." << ext; }
			atab->SubsName = atab->SubsPath.AfterLast(L'\\');
			SetRecent(0, wtab);
		}
		else{ return; }
	}
	else{
		DWORD attributes = ::GetFileAttributesW(atab->SubsPath.wc_str());
		if (attributes != -1 && attributes & FILE_ATTRIBUTE_READONLY){
			KaiMessageBox(_("Wybrany plik jest tylko do odczytu,\nproszę zapisać pod inną nazwą lub zmienić atrybuty pliku"), _("Uwaga"), 4L, this);
			goto repeatOpening;
		}
	}
	if (atab->Grid->SwapAssProperties()){ return; }
	atab->Grid->SaveFile(atab->SubsPath);
	atab->Grid->originalFormat = atab->Grid->subsFormat;
	if (changeLabel){
		Toolbar->UpdateId(SaveSubs, false);
		Menubar->Enable(SaveSubs, false);
		Label(0, false, wtab);
	}
#if _DEBUG
	wxBell();
#endif
}

bool KainoteFrame::OpenFile(const wxString &filename, bool fulls/*=false*/, bool freeze /*= true*/)
{
	wxMutexLocker lock(blockOpen);
	wxString ext = filename.AfterLast(L'.').Lower();
	if (ext == L"exe" || ext == L"zip" || ext == L"rar" || ext == L"7z"){ return false; }
	if (ext == L"lua" || ext == L"moon"){
		//if (!Auto){ Auto = new Auto::Automation(false); }
		if (Auto->ASSScripts.size() < 1)
			Auto->AddFromSubs();
		Auto->Add(filename);
		return true;
	}
	TabPanel *tab = GetTab();

	bool found = false;
	bool nonewtab = true;
	bool changeAudio = true;
	wxString secondFileName;
	bool issubs = (ext == L"ass" || ext == L"txt" || ext == L"sub" || ext == L"srt" || ext == L"ssa");

	/*if (tab->editor && !(issubs && tab->VideoPath.BeforeLast(L'.') == filename.BeforeLast(L'.'))
		&& !(!issubs && tab->SubsPath.BeforeLast(L'.') == filename.BeforeLast(L'.'))){*/
	if (tab->editor){
		found = FindFile(filename, secondFileName, issubs);
		if (!issubs && found && !fulls && !tab->Video->isFullscreen){
			if (tab->SubsPath == secondFileName || KaiMessageBox(wxString::Format(_("Wczytać napisy o nazwie \"%s\"?"), secondFileName.AfterLast(L'\\')),
				_("Potwierdzenie"), wxICON_QUESTION | wxYES_NO, this) == wxNO){
				found = false;
			}
			else{
				ext = secondFileName.AfterLast(L'.');
			}
		}
	}

	if (Options.GetBool(OpenSubsInNewCard) && tab->SubsPath != L"" &&
		!tab->Video->isFullscreen && issubs){
		Tabs->AddPage(true);
		tab = Tabs->Page(Tabs->Size() - 1);
		nonewtab = false;
	}

	if (freeze)
		tab->Freeze();

	if (issubs || found){
		const wxString &fname = (found && !issubs) ? secondFileName : filename;
		if (nonewtab){
			if (SavePrompt(2)){
				if (freeze)
					tab->Thaw();
				return true;
			}
		}
		if (tab->Video->Visual)
			tab->Video->SetVisual(true, false, true);

		OpenWrite ow;
		wxString s;
		if (!ow.FileOpen(fname, &s)){
			if (freeze)
				tab->Thaw();
			return false;
		}
		//remove comparison after every subs load or delete 
		else if (nonewtab && tab->Grid->Comparison){
			SubsGridBase::RemoveComparison();
		}
		tab->Grid->LoadSubtitles(s, ext);

		if (ext == L"ssa"){ ext = L"ass"; tab->SubsPath = fname.BeforeLast(L'.') + L".ass"; }
		else{ tab->SubsPath = fname; }
		tab->SubsName = tab->SubsPath.AfterLast(L'\\');

		//seek for video / audio and (rest writed in future)
		if (issubs && !fulls && !tab->Video->isFullscreen){
			wxString videopath = tab->Grid->GetSInfo(L"Video File");
			wxString audiopath = tab->Grid->GetSInfo(L"Audio File");
			wxString keyframespath = tab->Grid->GetSInfo(L"Keyframes File");
			if (audiopath.StartsWith(L"?")){ audiopath = videopath; }
			if (videopath.StartsWith(L"?dummy")){ videopath = L""; }
			//fix for wxFileExists which working without path when program run from command line
			bool hasVideoPath = (!videopath.empty() && ((wxFileExists(videopath) && videopath.find(L':') == 1) ||
				wxFileExists(videopath.Prepend(filename.BeforeLast(L'\\') + L"\\"))));
			bool hasAudioPath = (!audiopath.empty() && ((wxFileExists(audiopath) && audiopath.find(L':') == 1) ||
				wxFileExists(audiopath.Prepend(filename.BeforeLast(L'\\') + L"\\"))));
			bool hasKeyframePath = (!keyframespath.empty() && ((wxFileExists(keyframespath) && keyframespath.find(L':') == 1) ||
				wxFileExists(keyframespath.Prepend(filename.BeforeLast(L'\\') + L"\\"))));
			int flags = wxNO;
			wxString prompt;
			if (hasVideoPath || hasAudioPath || hasKeyframePath){
				if (hasVideoPath && tab->VideoPath != videopath){ prompt += _("Wideo: ") + videopath + L"\n"; }
				else if (hasVideoPath){ hasVideoPath = false; }
				if (hasAudioPath && tab->AudioPath != audiopath){ prompt += _("Audio: ") + audiopath + L"\n"; }
				else if (hasAudioPath){ hasAudioPath = false; }
				if (hasKeyframePath && tab->KeyframesPath != keyframespath){ prompt += _("Klatki kluczowe: ") + keyframespath + L"\n"; }
				else if (hasKeyframePath){ hasKeyframePath = false; }
				if (!prompt.empty()){ prompt.Prepend(_("Skojarzone pliki:\n")); flags |= wxOK; }
			}
			if (!secondFileName.empty()){
				if (tab->VideoPath != secondFileName){
					if (!prompt.empty()){ prompt += L"\n"; }
					prompt += _("Wideo z folderu:\n") + secondFileName.AfterLast(L'\\'); flags |= wxYES;
				}
				else
					found = false;
			}
			if (!prompt.empty()){
				KaiMessageDialog dlg(this, prompt, _("Potwierdzenie"), flags);
				if (flags & wxYES && flags & wxOK){
					dlg.SetOkLabel(_("Wczytaj skojarzone"));
					dlg.SetYesLabel(_("Wczytaj z folderu"));
				}
				else if (flags & wxOK){ dlg.SetOkLabel(_("Tak")); }
				int result = dlg.ShowModal();
				if (result == wxNO){
					found = false;
				}
				else if (result == wxOK){
					if (!audiopath.empty()){
						if (hasAudioPath && audiopath != videopath){
							audiopath.Replace(L"/", L"\\");
							OpenAudioInTab(tab, 30040, audiopath);
							found = changeAudio = false;
						}
						if (hasVideoPath){
							MenuItem *item = VidMenu->FindItem(VideoIndexing);
							if (item) item->Check();
							toolitem *titem = Toolbar->FindItem(VideoIndexing);
							if (titem){
								titem->toggled = true;
								Toolbar->Refresh(false);
							}
						}
					}
					if (hasVideoPath){
						videopath.Replace(L"/", L"\\");
						secondFileName = videopath;
						found = true;
					}
					if (hasKeyframePath){
						tab->Video->OpenKeyframes(keyframespath);
						tab->KeyframesPath = keyframespath;
					}
				}
			}
		}

		//open subs and disable visuals when needed
		if (tab->Video->GetState() != None){
			if (!found){
				bool isgood = tab->Video->OpenSubs((tab->editor) ? tab->Grid->GetVisible() : 0);
				if (!isgood){ KaiMessageBox(_("Nie można otworzyć napisów"), _("Uwaga")); }
				else{ tab->Video->Render(); }
			}
			tab->Video->vToolbar->DisableVisuals(ext != L"ass");
		}
		SetRecent();
		//set texts on window title and tab
		Label();
		SetSubsResolution(!Options.GetBool(DontAskForBadResolution));
		//turn on editor
		if (!tab->editor && !fulls && !tab->Video->isFullscreen){ HideEditor(); }
		//set color space
		if (!found){
			if (tab->Video->VFF && tab->Video->vstate != None && tab->Grid->subsFormat == ASS){
				tab->Video->SetColorSpace(tab->Grid->GetSInfo(L"YCbCr Matrix"));
			}
			goto done;
		}
	}

	const wxString &fnname = (found && issubs) ? secondFileName : filename;
	bool isload = tab->Video->LoadVideo(fnname, tab->Grid->GetVisible(), fulls, changeAudio);
	if (!isload){
		if (freeze)
			tab->Thaw();
		return false;
	}
	tab->Edit->Frames->Enable(!tab->Video->IsDshow);
	tab->Edit->Times->Enable(!tab->Video->IsDshow);

	//fix to not delete audiocache when using from OpenFiles
	if (freeze)
		Tabs->GetTab()->Video->DeleteAudioCache();
done:
	tab->ShiftTimes->Contents();
	UpdateToolbar();
	if (freeze){
		tab->Thaw();
		// do not save options, cause it load many files and save options after it.
		Options.SaveOptions(true, false);
	}
	return true;
}

void KainoteFrame::SetSubsResolution(bool showDialog)
{
	TabPanel *cur = GetTab();
	if (cur->Grid->subsFormat != ASS){
		StatusBar->SetLabelTextColour(5, WindowText);
		SetStatusText(L"", 7);
		return;
	}
	int x = 0, y = 0;
	cur->Grid->GetASSRes(&x, &y);
	wxString resolution = std::to_string(x) + L" x " + std::to_string(y);
	SetStatusText(resolution, 7);
	wxSize vsize;

	if (cur->Video->GetState() != None && cur->editor){
		vsize = cur->Video->GetVideoSize();
		wxString vres;
		vres << vsize.x << L" x " << vsize.y;
		if (vres != resolution){
			StatusBar->SetLabelTextColour(5, WindowWarningElements);
			StatusBar->SetLabelTextColour(7, WindowWarningElements);
			badResolution = true;
			if (showDialog){
				ShowBadResolutionDialog(vsize, wxSize(x, y));
			}
			return;
		}
	}
	if (badResolution){
		StatusBar->SetLabelTextColour(5, WindowText);
		StatusBar->SetLabelTextColour(7, WindowText);
		badResolution = false;
	}

}

void KainoteFrame::SetVideoResolution(int w, int h, bool showDialog)
{
	TabPanel *cur = GetTab();
	wxString resolution;
	resolution << w << L" x " << h;
	SetStatusText(resolution, 5);
	int x = 0, y = 0;
	cur->Grid->GetASSRes(&x, &y);
	wxString sres = std::to_string(x) + L" x " + std::to_string(y);
	if (resolution != sres && sres.Len() > 3 && cur->editor){
		StatusBar->SetLabelTextColour(5, WindowWarningElements);
		StatusBar->SetLabelTextColour(7, WindowWarningElements);
		badResolution = true;
		if (showDialog && cur->Grid->subsFormat == ASS && !cur->SubsPath.empty()){
			ShowBadResolutionDialog(wxSize(w, h), wxSize(x, y));
		}
	}
	else if (badResolution){
		StatusBar->SetLabelTextColour(5, WindowText);
		StatusBar->SetLabelTextColour(7, WindowText);
		badResolution = false;
	}
}

void KainoteFrame::ShowBadResolutionDialog(const wxSize &videoRes, const wxSize &subsRes)
{
	SubsMismatchResolutionDialog badResDialog(this, subsRes, videoRes);
	badResDialog.ShowModal();
}

//0 - subs, 1 - vids, 2 - auds
void KainoteFrame::SetRecent(short what/*=0*/, int numtab /*= -1*/)
{
	int idd = 30000 + (20 * what);
	TabPanel *tab = (numtab < 0) ? GetTab() : Tabs->Page(numtab);
	Menu *wmenu = (what == 0) ? SubsRecMenu : (what == 1) ? VidsRecMenu : (what == 2) ? AudsRecMenu : KeyframesRecentMenu;
	int size = (what == 0) ? subsrec.size() : (what == 1) ? videorec.size() : (what == 2) ? audsrec.size() : keyframesRecent.size();
	wxArrayString &recs = (what == 0) ? subsrec : (what == 1) ? videorec : (what == 2) ? audsrec : keyframesRecent;
	wxString path = (what == 0) ? tab->SubsPath : (what == 1) ? tab->VideoPath : (what == 2) ? tab->Edit->ABox->audioName : tab->KeyframesPath;

	for (int i = 0; i < size; i++){
		if (recs[i] == path){
			recs.erase(recs.begin() + i);
			break;
		}
	}
	recs.Insert(path, 0);
	if (recs.size() > 20){ recs.pop_back(); }
	if (what == 0){ Options.SetTable(SubsRecent, recs); }
	else if (what == 1){ Options.SetTable(VideoRecent, recs); }
	else if (what == 2){ Options.SetTable(AudioRecent, recs); }
	else{ Options.SetTable(KEYFRAMES_RECENT, recs); }
}

//0 - subs, 1 - vids, 2 - auds
void KainoteFrame::AppendRecent(short what, Menu *_Menu)
{
	Menu *wmenu;
	if (_Menu){
		wmenu = _Menu;
	}
	else{
		wmenu = (what == 0) ? SubsRecMenu : (what == 1) ? VidsRecMenu : (what == 2) ? AudsRecMenu : KeyframesRecentMenu;
	}
	int idd = 30000 + (20 * what);
	//int size= (what==0)?subsrec.size() : (what==1)? videorec.size() : audsrec.size();

	wxArrayString &recs = (what == 0) ? subsrec : (what == 1) ? videorec : (what == 2) ? audsrec : keyframesRecent;

	for (int j = wmenu->GetMenuItemCount() - 1; j >= 0; j--){
		wmenu->Destroy(wmenu->FindItemByPosition(j));
	}
	size_t i = 0;
	bool changedRecent = false;
	while (i < recs.size())
	{
		if (!wxFileExists(recs[i])){
			recs.erase(recs.begin() + i);
			changedRecent = true;
			continue;
		}
		MenuItem* MI = new MenuItem(idd + i, std::to_string(i + 1) + L" " + recs[i].AfterLast(L'\\'), _("Otwórz") + L" " + recs[i]);
		wmenu->Append(MI);
		i++;
	}

	if (!wmenu->GetMenuItemCount()){
		MenuItem* MI = new MenuItem(idd, _("Brak"));
		MI->Enable(false);
		wmenu->Append(MI);
	}
	if (changedRecent){
		if (what == 0){ Options.SetTable(SubsRecent, recs); }
		else if (what == 1){ Options.SetTable(VideoRecent, recs); }
		else if (what == 2){ Options.SetTable(AudioRecent, recs); }
		else{ Options.SetTable(KEYFRAMES_RECENT, recs); }
	}
}

void KainoteFrame::OnRecent(wxCommandEvent& event)
{
	int id = event.GetId();
	int numItem = 0;
	int Modif = event.GetInt();
	wxString filename;
	if (id < 30020){
		numItem = id - 30000;
		if (numItem < 0){ return; }
		filename = subsrec[numItem];
	}
	else if (id < 30040){
		numItem = id - 30020;
		filename = videorec[numItem];
	}
	else if (id < 30060){
		numItem = id - 30040;
		filename = audsrec[numItem];
	}
	else if (id < 30080){
		numItem = id - 30060;
		if (numItem >= keyframesRecent.size()){ return; }
		filename = keyframesRecent[numItem];
	}
	else{
		return;
	}

	if (Modif == wxMOD_CONTROL){
		/*CoInitialize(0);
		ITEMIDLIST *pidl = ILCreateFromPathW(filename.wc_str());
		if (pidl) {
			SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
			ILFree(pidl);
		}
		CoUninitialize();*/
		SelectInFolder(filename);
		return;
	}
	if (id < 30040){
		OpenFile(filename);
	}
	else if (id < 30060){
		event.SetString(filename);
		OnOpenAudio(event);
	}
	else{
		GetTab()->KeyframesPath = filename;
		GetTab()->Video->OpenKeyframes(filename);
		SetRecent(3);
	}
}


void KainoteFrame::OnSize(wxSizeEvent& event)
{
	wxSize size = GetSize();
	int fborder, ftopBorder;
	GetBorders(&fborder, &ftopBorder);
	borders.left = borders.right = borders.bottom = fborder;
	borders.top = ftopBorder;

	int menuHeight = Menubar->GetSize().GetHeight();
	int toolbarWidth = Toolbar->GetThickness();
	int statusbarHeight = StatusBar->GetSize().GetHeight();
	//0 left, 1 top, 2 right, 3 bottom
	int toolbarAlignment = Options.GetInt(ToolbarAlignment);
	borders.top += menuHeight;
	borders.bottom += statusbarHeight;
	Menubar->SetSize(fborder, ftopBorder, size.x - (fborder * 2), menuHeight);
	switch (toolbarAlignment){
	case 0://left
		Toolbar->SetSize(borders.left, borders.top, toolbarWidth, size.y - borders.top - borders.bottom);
		borders.left += toolbarWidth;
		break;
	case 1://top
		Toolbar->SetSize(borders.left, borders.top, size.x - borders.left - borders.right, toolbarWidth);
		borders.top += toolbarWidth;
		break;
	case 2://right
		borders.right += toolbarWidth;
		Toolbar->SetSize(size.x - borders.right, borders.top, toolbarWidth, size.y - borders.top - borders.bottom);
		break;
	case 3://bottom
		borders.bottom += toolbarWidth;
		Toolbar->SetSize(borders.left, size.y - borders.bottom, size.x - borders.left - borders.right, toolbarWidth);
		break;
	default:
		Toolbar->SetSize(borders.left, borders.top, toolbarWidth, size.y - borders.top - borders.bottom);
		borders.left += toolbarWidth;
		break;
	}
	Tabs->SetSize(borders.left, borders.top, size.x - borders.left - borders.right, size.y - borders.top - borders.bottom);
	StatusBar->SetSize(fborder, size.y - statusbarHeight - fborder, size.x - (fborder * 2), statusbarHeight);

	event.Skip();
}


bool KainoteFrame::FindFile(const wxString &fn, wxString &foundFile, bool video)
{
	wxString filespec;
	wxString path = fn.BeforeLast(L'\\', &filespec);
	wxArrayString files;

	wxDir kat(path);
	if (kat.IsOpened()){
		kat.GetAllFiles(path, &files, filespec.BeforeLast(L'.') + L".*", wxDIR_FILES);
	}
	if (files.size() < 2){ return false; }

	for (int i = 0; i < (int)files.size(); i++){
		wxString ext = files[i].AfterLast(L'.');
		if ((!video && (ext != L"ass" && ext != L"txt" && ext != L"sub" && ext != L"srt" && ext != L"ssa"))
			|| (video && (ext != L"avi" && ext != L"mp4" && ext != L"mkv" && ext != L"ogm" && ext != L"wmv"
			&& ext != L"asf" && ext != L"rmvb" && ext != L"rm" && ext != L"3gp" &&ext != L"ts"
			&&ext != L"m2ts" &&ext != L"m4v" &&ext != L"flv"))  //&&ext!=L"avs" przynajmniej do momentu dorobienia obsługi przez avisynth
			){
		}
		else{ foundFile = files[i]; return true; }
	}
	return false;
}


void KainoteFrame::OnP5Sec(wxCommandEvent& event)
{
	GetTab()->Video->Seek(GetTab()->Video->Tell() + 5000);
}

void KainoteFrame::OnM5Sec(wxCommandEvent& event)
{
	GetTab()->Video->Seek(GetTab()->Video->Tell() - 5000);

}

TabPanel* KainoteFrame::GetTab()
{
	return Tabs->GetPage();
}


void KainoteFrame::Label(int iter/*=0*/, bool video/*=false*/, int wtab/*=-1*/, bool onlyTabs /*= false*/)
{
	TabPanel* atab = (wtab < 0) ? GetTab() : Tabs->Page(wtab);
	wxString whiter;
	if (atab->Grid->IsModified()){ whiter << iter << L"*"; }

	/*MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	int div=1024;
	int availmem=statex.ullAvailVirtual/div;
	int totalmem=statex.ullTotalVirtual/div;
	wxString memtxt= wxString::Format(" RAM: %i KB / %i KB", totalmem-availmem, totalmem);*/
	wxString name = (video) ? atab->VideoName : whiter + atab->SubsName;
	if (!onlyTabs)
		SetLabel(name + L" - " + Options.progname + L" " + wxString(INSTRUCTIONS));

	Tabs->SetPageText((wtab < 0) ? Tabs->GetSelection() : wtab, name);
}

void KainoteFrame::SetAccels(bool _all)
{
	std::vector<wxAcceleratorEntry> entries;
	entries.resize(2);
	entries[0].Set(wxACCEL_CTRL, (int)L'T', ID_ADDPAGE);
	entries[1].Set(wxACCEL_CTRL, (int)L'W', ID_CLOSEPAGE);

	const std::map<idAndType, hdata> &hkeys = Hkeys.GetHotkeysMap();
	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++){
		if (cur->first.Type != GLOBAL_HOTKEY){ continue; }
		int id = cur->first.id;
		bool emptyAccel = cur->second.Accel == L"";
		if (id > 6000 && id < 6850){
			MenuItem *item = Menubar->FindItem(id);
			if (!item){ /*KaiLog(wxString::Format("no id %i", id));*/ continue; }
			if (emptyAccel){
				item->SetAccel(NULL);
				continue;
			}
			else{
				wxAcceleratorEntry accel = Hkeys.GetHKey(cur->first, &cur->second);
				item->SetAccel(&accel);
				entries.push_back(accel);
			}
		}
		else if (emptyAccel)
			continue;
		else if (id >= 6850){
			if (id >= 30100){
				Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnRunScript, this, id);
			}
			entries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}
		if (!entries[entries.size() - 1].IsOk()){
			entries.pop_back();
		}
	}
	//Menubar->SetAccelerators();
	wxAcceleratorTable accel(entries.size(), &entries[0]);
	Tabs->SetAcceleratorTable(accel);

	if (!_all){ return; }
	for (size_t i = 0; i < Tabs->Size(); i++)
	{
		Tabs->Page(i)->SetAccels();
	}
}


void KainoteFrame::InsertTab(bool refresh/*=true*/)
{
	Tabs->AddPage(refresh);
}

bool comp(wxString first, wxString second)
{
	return (first.CmpNoCase(second) < 0);
}

//OpenFiles automatically cleares files table
void KainoteFrame::OpenFiles(wxArrayString &files, bool intab, bool nofreeze, bool newtab)
{
	wxMutexLocker lock(blockOpen);
	std::sort(files.begin(), files.end(), comp);
	wxArrayString subs;
	wxArrayString videos;
	for (size_t i = 0; i < files.size(); i++){
		wxString ext = files[i].AfterLast(L'.').Lower();
		if (ext == L"ass" || ext == L"ssa" || ext == L"txt" || ext == L"srt" || ext == L"sub"){
			subs.Add(files[i]);
		}
		else if (ext == L"lua" || ext == L"moon"){
			//if (!Auto){ Auto = new Auto::Automation(false); }
			if (Auto->ASSScripts.size() < 1)
				Auto->AddFromSubs();
			Auto->Add(files[i]);
		}
		else if (ext != L"exe" && ext != L"zip" && ext != L"rar" && ext != L"7z"){
			videos.Add(files[i]);
		}

	}

	if (files.size() == 1){
		OpenFile(files[0], (videos.size() == 1 && Options.GetBool(VideoFullskreenOnStart)));
		videos.Clear(); subs.Clear(); files.Clear();
		return;
	}
	bool askForRes = !Options.GetBool(DontAskForBadResolution);
	Freeze();
	GetTab()->Hide();
	size_t subsSize = subs.size();
	size_t videosSize = videos.size();
	size_t maxx = (subsSize > videosSize) ? subsSize : videosSize;

	for (size_t i = 0; i < maxx; i++)
	{

		if ((i >= Tabs->Size() || Tabs->Page(Tabs->iter)->SubsPath != L"" ||
			Tabs->Page(Tabs->iter)->VideoPath != L"") && !intab){
			InsertTab(false);
		}
		else if (GetTab()->Video->Visual){
			GetTab()->Video->SetVisual(true);
		}
		TabPanel *tab = GetTab();
		if (i >= videosSize){
			if (!OpenFile(subs[i], false, false))
				break;

			continue;
		}
		else if (i >= subsSize){
			if (!OpenFile(videos[i], false, false))
				break;

			continue;
		}
		if (i < subsSize){
			wxString ext = subs[i].AfterLast(L'.').Lower();
			OpenWrite ow;
			wxString s;
			if (!ow.FileOpen(subs[i], &s)){
				break;
			}
			else{
				tab->Grid->LoadSubtitles(s, ext);
			}

			if (ext == L"ssa"){ ext = L"ass"; subs[i] = subs[i].BeforeLast(L'.') + L".ass"; }
			tab->SubsPath = subs[i];
			tab->SubsName = tab->SubsPath.AfterLast(L'\\');
			if (!tab->editor){ HideEditor(); }
			SetRecent();

			Label();
			SetSubsResolution(askForRes);
			tab->Video->vToolbar->DisableVisuals(ext != L"ass");
		}
		if (i < videosSize){
			bool isload = tab->Video->LoadVideo(videos[i], (tab->editor) ? tab->Grid->GetVisible() : 0);

			if (!isload){
				break;
			}
			tab->Edit->Frames->Enable(!tab->Video->IsDshow);
			tab->Edit->Times->Enable(!tab->Video->IsDshow);

		}
		tab->ShiftTimes->Contents();

	}

	Thaw();
	//taka kolejność naprawia błąd znikających zakładek
	/*int w, h;
	Tabs->GetClientSize(&w, &h);
	Tabs->RefreshRect(wxRect(0, h - 25, w, 25), false);*/
	Tabs->RefreshBar();
	GetTab()->Show();
	UpdateToolbar();

	files.Clear();
	subs.Clear();
	videos.Clear();
	Tabs->GetTab()->Video->DeleteAudioCache();
	Options.SaveOptions(true, false);
}

void KainoteFrame::OnPageChange(wxCommandEvent& event)
{
	if (Tabs->Size() < 2){ return; }
	int step = (event.GetId() == NextTab) ? Tabs->iter + 1 : Tabs->iter - 1;
	if (step < 0){ step = Tabs->Size() - 1; }
	else if (step >= (int)Tabs->Size()){ step = 0; }
	Tabs->ChangePage(step, true);
	Tabs->Update();
}


void KainoteFrame::OnPageChanged(wxCommandEvent& event)
{
	wxString whiter;
	TabPanel *cur = Tabs->GetPage();
	int iter = cur->Grid->file->Iter();
	if (cur->Grid->IsModified()){
		whiter << iter << L"*";
	}
	wxString name = (!cur->editor) ? cur->VideoName : cur->SubsName;
	SetLabel(whiter + name + L" - " + Options.progname + L" " + wxString(INSTRUCTIONS));
	if (cur->Video->GetState() != None){
		SetStatusText(getfloat(cur->Video->fps) + L" FPS", 4);
		wxString tar;
		tar << cur->Video->ax << L" : " << cur->Video->ay;
		SetStatusText(tar, 6);
		int x, y;
		cur->Video->GetVideoSize(&x, &y);
		tar.Empty();
		tar << x << L" x " << y;
		SetStatusText(tar, 5);
		cur->Video->RefreshTime();

		STime kkk1;
		kkk1.mstime = cur->Video->GetDuration();
		SetStatusText(kkk1.raw(SRT), 3);
		if (cur->editor){
			SetStatusText(cur->VideoName, 8);
		}
		else{ SetStatusText(L"", 8); }
		cur->Video->SetScaleAndZoom();
	}
	else{
		SetStatusText(L"", 8);
		SetStatusText(L"", 6);
		SetStatusText(L"", 5);
		SetStatusText(L"", 4);
		SetStatusText(L"", 3);
		SetStatusText(L"", 2);
		SetStatusText(L"", 1);
	}
	SetSubsResolution();

	cur->Grid->UpdateUR(false);

	UpdateToolbar();
	//blokada zmiany focusa przy przejściu na drugą widoczną zakładkę
	if (!event.GetInt()){
		//Todo: zrobić jakiś bezpieczny sposób, bo wbrew pozorom element do którego ten wskaźnik należy może zniknąć
		if (cur->lastFocusedWindow != NULL){
			cur->lastFocusedWindow->SetFocus();
		}
		else if (cur->editor){ cur->Grid->SetFocus(); }
		else{ cur->Video->SetFocus(); }
		if (Tabs->iter != Tabs->GetOldSelection() && Options.GetBool(MoveTimesLoadSetTabOptions)){
			cur->ShiftTimes->RefVals(Tabs->Page(Tabs->GetOldSelection())->ShiftTimes);
		}

		if (Options.GetBool(AutoSelectLinesFromLastTab)){
			SubsGrid *old = Tabs->Page(Tabs->GetOldSelection())->Grid;
			if (old->FirstSelection() > -1){
				cur->Grid->SelVideoLine(old->GetDialogue(old->FirstSelection())->Start.mstime);
			}
		}
	}
	if (StyleStore::HasStore() && StyleStore::Get()->IsShown()){ StyleStore::Get()->LoadAssStyles(); }
	if (FR){ FR->Reset(); }
}

void KainoteFrame::HideEditor(bool save)
{
	TabPanel *cur = GetTab();

	cur->editor = !cur->editor;
	cur->Grid->Show(cur->editor);

	cur->Edit->Show(cur->editor);
	cur->windowResizer->Show(cur->editor);

	if (cur->editor){//Załączanie Edytora

		cur->BoxSizer1->Detach(cur->Video);
		cur->BoxSizer2->Prepend(cur->Video, 0, wxEXPAND | wxALIGN_TOP, 0);

		cur->Video->panelHeight = 66;
		cur->Video->vToolbar->Show();
		if (cur->Video->GetState() != None && !cur->Video->isFullscreen){
			int sx, sy, vw, vh;
			Options.GetCoords(VideoWindowSize, &vw, &vh);
			if (vh < 350){ vh = 350, vw = 500; }
			cur->Video->CalcSize(&sx, &sy, vw, vh);
			cur->Video->SetMinSize(wxSize(sx, sy + cur->Video->panelHeight));
		}
		else{ cur->Video->Hide(); }
		if (Options.GetBool(MoveTimesOn)){
			cur->ShiftTimes->Show();
		}
		cur->BoxSizer1->Layout();
		Label();
		if (cur->Video->GetState() != None){ cur->Video->ChangeVobsub(); }
		SetSubsResolution(false);
		if (cur->Video->isFullscreen)
			cur->Video->TD->HideToolbar(false);
	}
	else{//Wyłączanie edytora
		if (cur->Video->Visual){
			cur->Video->SetVisual(true);
		}
		cur->Video->panelHeight = 44;
		cur->ShiftTimes->Hide();

		//cur->BoxSizer1->Remove(1);

		if (!cur->Video->IsShown()){ cur->Video->Show(); }

		cur->BoxSizer2->Detach(cur->Video);

		cur->BoxSizer1->Add(cur->Video, 1, wxEXPAND | wxALIGN_TOP, 0);

		cur->Video->vToolbar->Hide();
		//potencjalny krasz po wyłączaniu edytora
		if (cur->Video->GetState() != None && !cur->Video->isFullscreen && !IsMaximized()){
			int sx, sy, sizex, sizey;
			GetClientSize(&sizex, &sizey);
			sizex -= borders.left + borders.right;
			sizey -= (cur->Video->panelHeight + borders.bottom + borders.top);

			cur->Video->CalcSize(&sx, &sy, sizex, sizey, false, true);

			SetClientSize(sx + borders.left + borders.right, sy + cur->Video->panelHeight + borders.bottom + borders.top);

		}
		cur->Video->SetFocus();

		cur->BoxSizer1->Layout();

		if (cur->VideoName != L""){ Label(0, true); }
		if (cur->Video->GetState() != None){ cur->Video->ChangeVobsub(true); }
		//cur->Video->vToolbar->Enable(false);
		StatusBar->SetLabelTextColour(5, WindowText);
		SetStatusText(L"", 7);
		if (cur->Video->isFullscreen)
			cur->Video->TD->HideToolbar(true);

		if (FR && FR->IsShown())
			FR->Show(false);
		if (SL && SL->IsShown())
			SL->Show(false);
		if (StyleStore::HasStore() && StyleStore::Get()->IsShown())
			StyleStore::Get()->Show(false);
	}
	UpdateToolbar();
	if (save){ Options.SetBool(EditorOn, cur->editor); Options.SaveOptions(true, false); }
}

void KainoteFrame::OnPageAdd(wxCommandEvent& event)
{
	InsertTab();
}

void KainoteFrame::OnPageClose(wxCommandEvent& event)
{
	Tabs->DeletePage(Tabs->GetSelection());
	OnPageChanged(event);
}


void KainoteFrame::SaveAll()
{
	for (size_t i = 0; i < Tabs->Size(); i++)
	{
		if (!Tabs->Page(i)->Grid->IsModified()){ continue; }
		Save(false, i, false);
		Label(0, false, i);
	}
}

//return anulowanie operacji
bool KainoteFrame::SavePrompt(char mode, int wtab)
{
	TabPanel* atab = (wtab < 0) ? GetTab() : Tabs->Page(wtab);
	if (atab->Grid->IsModified()){
		wxString ext = (atab->Grid->subsFormat == ASS) ? L"ass" : (atab->Grid->subsFormat == SRT) ? L"srt" : L"txt";
		wxString subsExt;
		wxString subsName = atab->SubsName.BeforeLast(L'.', &subsExt);
		if (subsName.empty())
			subsName = subsExt;
		subsExt.MakeLower();
		wxString subsPath = (ext != subsExt && !(ext == L"txt" && subsExt == L"sub")) ?
			subsName + L"." + ext : atab->SubsName;

		wxWindow *_parent = (atab->Video->isFullscreen) ? (wxWindow*)atab->Video->TD : this;
		int answer = KaiMessageBox(wxString::Format(_("Zapisać napisy o nazwie \"%s\" przed %s?"),
			subsPath, (mode == 0) ? _("zamknięciem programu") :
			(mode == 1) ? _("zamknięciem zakładki") :
			(mode == 2) ? _("wczytaniem nowych napisów") :
			_("usunięciem napisów")),
			_("Potwierdzenie"), wxYES_NO | wxCANCEL, _parent);
		if (answer == wxCANCEL){ return true; }
		if (answer == wxYES){
			Save(false, wtab);
		}
	}
	return false;
}


void KainoteFrame::UpdateToolbar()
{//disejblowanie rzeczy niepotrzebnych przy wyłączonym edytorze
	MenuEvent evt;
	OnMenuOpened(evt);
	Toolbar->Updatetoolbar();
}

void KainoteFrame::OnOpenAudio(wxCommandEvent& event)
{
	OpenAudioInTab(GetTab(), event.GetId(), event.GetString());
}

void KainoteFrame::OpenAudioInTab(TabPanel *tab, int id, const wxString &path)
{

	if (id == CloseAudio && tab->Edit->ABox){
		tab->Video->player = NULL;
		tab->Edit->CloseAudio();
		tab->AudioPath.clear();
	}
	else{

		if (!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){ KaiMessageBox(_("Nie można wczytać skrótów klawiszowych audio"), _("Błąd")); return; }
		if (!Options.AudioOpts && !Options.LoadAudioOpts()){ KaiMessageBox(_("Nie można wczytać opcji audio"), _("Błąd")); return; }

		wxString Path;
		if (id == OpenAudio){
			wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik audio"),
				(tab->VideoPath != L"") ? tab->VideoPath.BeforeLast(L'\\') :
				(videorec.size() > 0) ? subsrec[0].BeforeLast(L'\\') : L"", L"",
				_("Pliki audio i wideo") +
				L" (*.wav),(*.w64),(*.flac),(*.ac3),(*.aac),(*.ogg),(*.mp3),(*.mp4),(*.m4a),(*.mkv),(*.avi)|*.wav;*.w64;*.flac;*.ac3;*.aac;*.ogg;*.mp3;*.mp4;*.m4a;*.mkv;*.avi|" +
				_("Wszystkie pliki") + L" |*.*", wxFD_OPEN);
			int result = FileDialog1->ShowModal();
			if (result == wxID_OK){
				Path = FileDialog1->GetPath();
			}
			FileDialog1->Destroy();
			if (result == wxID_CANCEL){ return; }
		}
		if (id > 30039){ Path = path; }
		if (Path.IsEmpty()){ Path = tab->VideoPath; }
		if (Path.IsEmpty()){ return; }

		if (tab->Edit->LoadAudio(Path, (id == 40000))){
			SetRecent(2);
			tab->AudioPath = path;
		}

	}
}



//uses before menu is shown
void KainoteFrame::OnMenuOpened(MenuEvent& event)
{
	Menu *curMenu = event.GetMenu();
	TabPanel *tab = GetTab();

	if (curMenu == FileMenu)
	{
		AppendRecent();
	}
	else if (curMenu == VidMenu)
	{
		//video recent
		AppendRecent(1);
		//keyframes recent;
		AppendRecent(3);
	}
	else if (curMenu == AudMenu)
	{
		AppendRecent(2);
	}
	else if (curMenu == AutoMenu)
	{
		Auto->BuildMenu(&AutoMenu);
	}
	else if (curMenu == EditMenu){
		const wxString &undoName = tab->Grid->file->GetUndoName();
		const wxString &redoName = tab->Grid->file->GetRedoName();
		MenuItem *undoItem = EditMenu->FindItem(Undo);
		MenuItem *redoItem = EditMenu->FindItem(Redo);
		if (undoItem){
			wxString accel = undoItem->GetAccel();
			wxString accelName = (accel.empty()) ? L"" : L"\t" + accel;
			accelName.Replace(L"+", L"-");
			if (!undoName.empty()){
				wxString lowerUndoName = wxString(undoName[0]).Lower() + undoName.Mid(1);
				undoItem->label = _("&Cofnij do ") + lowerUndoName + accelName;
			}
			else{
				undoItem->label = _("&Cofnij") + accelName;
			}
		}
		if (redoItem){
			wxString accel = redoItem->GetAccel();
			wxString accelName = (accel.empty()) ? L"" : L"\t" + accel;
			accelName.Replace(L"+", L"-");
			if (!redoName.empty()){
				wxString lowerRedoName = wxString(redoName[0]).Lower() + redoName.Mid(1);
				redoItem->label = _("&Ponów do ") + lowerRedoName + accelName;
			}
			else{
				redoItem->label = _("&Ponów") + accelName;
			}
		}
	}

	bool enable = (tab->Video->GetState() != None);
	bool editor = tab->editor;
	for (int i = PlayPauseG; i <= SetVideoAtEnd; i++)
	{
		Menubar->Enable(i, (i < SetStartTime) ? enable : enable && editor);
	}
	enable = (tab->Video->VFF != NULL);
	Menubar->Enable(GoToPrewKeyframe, enable);
	Menubar->Enable(GoToNextKeyframe, enable);
	enable = (tab->Edit->ABox != NULL);
	Menubar->Enable(SetAudioFromVideo, enable);
	Menubar->Enable(SetAudioMarkFromVideo, enable);
	//kolejno numery id
	char form = tab->Grid->subsFormat;
	bool tlmode = tab->Grid->hasTLMode;
	for (int i = SaveSubs; i <= ViewSubs; i++){//po kolejne idy zajrzyj do enuma z pliku h, ostatnim jest Automation
		enable = true;

		if (i >= ASSProperties && i < ConvertToASS){ enable = form < SRT; }//menager stylów i sinfo
		else if (i == ConvertToASS){ enable = form > ASS; }//konwersja na ass
		else if (i == ConvertToSRT){ enable = form != SRT; }//konwersja na srt
		else if (i == ConvertToMDVD){ enable = form != MDVD; }//konwersja na mdvd
		else if (i == ConvertToMPL2){ enable = form != MPL2; }//konwersja na mpl2
		else if (i == ConvertToTMP){ enable = form != TMP; }//konwersja na tmp
		if ((i >= ConvertToASS && i <= ConvertToMPL2) && tlmode){ enable = false; }
		else if (i == ViewAudio || i == CloseAudio){ enable = tab->Edit->ABox != 0; }
		else if ((i == ViewVideo || i == ViewAll) || i == AudioFromVideo || i == GLOBAL_VIEW_ONLY_VIDEO){
			enable = tab->Video->GetState() != None;
			if (i != AudioFromVideo){ enable = (enable && !tab->Video->isOnAnotherMonitor); }
		}
		else if (i == SaveTranslation){ enable = tlmode; }
		else if (i == SaveSubs){ if (!tab->Grid->IsModified()){ enable = false; } }
		//else if(i==SaveAllSubs){
		//for(size_t k = 0; k < Tabs->Size(); k){}
		//}
		Menubar->Enable(i, editor && enable);
	}
	//specjalna poprawka do zapisywania w trybie tłumaczenia, jeśli jest tlmode, to zawsze ma działać.
	tab->Edit->TlMode->Enable((editor && form == ASS && tab->SubsPath != L""));
	//if(curMenu){Menubar->ShowMenu();}
}


void KainoteFrame::OnChangeLine(wxCommandEvent& event)
{

	int idd = event.GetId();
	if (idd < JoinWithPrevious){//zmiana linijki
		GetTab()->Grid->NextLine((idd == PreviousLine) ? -1 : 1);
	}
	else{//scalanie dwóch linijek
		GetTab()->Grid->OnJoin(event);
	}
}

void KainoteFrame::OnDelete(wxCommandEvent& event)
{
	int idd = event.GetId();
	if (idd == Remove){
		GetTab()->Grid->DeleteRows();
	}
	else{
		GetTab()->Grid->DeleteText();
	}

}

void KainoteFrame::OnClose1(wxCloseEvent& event)
{
	size_t curit = Tabs->iter;
	for (size_t i = 0; i < Tabs->Size(); i++)
	{
		Tabs->iter = i;
		if (SavePrompt(0)){ Tabs->iter = curit; return; }
	}
	Tabs->iter = curit;
	event.Skip();
}

void KainoteFrame::OnActivate(wxActivateEvent &evt)
{
	if (!evt.GetActive() && Tabs->Size()){
		TabPanel *tab = GetTab();
		wxWindow *win = FindFocus();
		if (win && tab->IsDescendant(win)){
			tab->lastFocusedWindow = win;
		}

	}
	if (evt.GetActive()){
		sendFocus.Start(50, true);
	}
}

void KainoteFrame::OnAudioSnap(wxCommandEvent& event)
{
	TabPanel *tab = GetTab();
	if (!tab->Edit->ABox){ return; }
	int id = event.GetId();
	bool snapStartTime = (id == SnapWithStart);
	int time = (snapStartTime) ? tab->Edit->line->Start.mstime : tab->Edit->line->End.mstime;
	int time2 = (snapStartTime) ? tab->Edit->line->End.mstime : tab->Edit->line->Start.mstime;
	int snaptime = time;// tab->Edit->ABox->audioDisplay->GetBoundarySnap(time,1000,!Options.GetBool(AudioSnapToKeyframes),(id==SnapWithStart),true);
	wxArrayInt &KeyFrames = tab->Video->VFF->KeyFrames;
	int lastDifferents = MAXINT;
	//wxArrayInt boundaries;
	for (unsigned int i = 0; i < KeyFrames.Count(); i++) {
		int keyMS = KeyFrames[i];
		if (keyMS >= time - 5000 && keyMS < time + 5000) {
			int frameTime = 0;
			int frame = tab->Video->VFF->GetFramefromMS(keyMS);
			int prevFrameTime = tab->Video->VFF->GetMSfromFrame(frame - 1);
			frameTime = keyMS + ((prevFrameTime - keyMS) / 2);
			frameTime = ZEROIT(frameTime);
			int actualDiff = abs(time - frameTime);
			if (actualDiff < lastDifferents && actualDiff > 0){
				if ((snapStartTime && frameTime >= time2) || (!snapStartTime && frameTime <= time2)){ continue; }
				snaptime = frameTime;
				lastDifferents = actualDiff;
			}
		}
	}

	int inactiveType = Options.GetInt(AudioInactiveLinesDisplayMode);
	if (inactiveType > 0) {
		Dialogue *shade;
		int shadeFrom, shadeTo;

		// Get range
		if (inactiveType == 1) {
			//this function is safe, not return -1 or more then size - 1
			shadeFrom = tab->Grid->GetKeyFromPosition(tab->Grid->currentLine, -1);
			shadeTo = tab->Grid->GetKeyFromPosition(tab->Grid->currentLine, 1);
		}
		else {
			shadeFrom = 0;
			shadeTo = tab->Grid->GetCount();
		}

		for (int j = shadeFrom; j < shadeTo; j++) {
			shade = tab->Grid->GetDialogue(j);
			if (!shade->isVisible || j == tab->Grid->currentLine) continue;

			int start = shade->Start.mstime;
			int end = shade->End.mstime;
			int startDiff = abs(time - start);
			int endDiff = abs(time - end);
			if (startDiff < lastDifferents && startDiff>0){
				if ((snapStartTime && start >= time2) || (!snapStartTime && start <= time2)){ continue; }
				snaptime = start;
				lastDifferents = startDiff;
			}
			if (endDiff < lastDifferents && endDiff>0){
				if ((snapStartTime && end >= time2) || (!snapStartTime && end <= time2)){ continue; }
				snaptime = end;
				lastDifferents = endDiff;
			}
		}
	}

	if (time != snaptime && abs(time - snaptime) < 5000){
		if (snapStartTime){
			if (snaptime >= time2){ return; }
			tab->Edit->StartEdit->SetTime(STime(snaptime), false, 1);
			tab->Edit->StartEdit->SetModified(true);
		}
		else{
			if (snaptime <= time2){ return; }
			tab->Edit->EndEdit->SetTime(STime(snaptime), false, 2);
			tab->Edit->EndEdit->SetModified(true);
		}
		STime durTime = tab->Edit->EndEdit->GetTime() - tab->Edit->StartEdit->GetTime();
		if (durTime.mstime < 0){ durTime.mstime = 0; }
		tab->Edit->DurEdit->SetTime(durTime, false, 1);
		tab->Edit->Send(SNAP_TO_KEYFRAME_OR_LINE_TIME, false);
		tab->Edit->ABox->audioDisplay->SetDialogue(tab->Edit->line, tab->Grid->currentLine, !snapStartTime);
		tab->Video->RefreshTime();
	}
}


void KainoteFrame::OnOutofMemory()
{
	TabPanel *tab = Notebook::GetTab();

	if (tab->Grid->file->maxx()>3){
		tab->Grid->file->RemoveFirst(2);
		KaiLog(_("Zabrakło pamięci RAM, usunięto część historii"));
		return;
	}
	else if (Notebook::GetTabs()->Size() > 1){
		for (size_t i = 0; i < Notebook::GetTabs()->Size(); i++)
		{
			if (i != Notebook::GetTabs()->GetSelection()){
				if (Notebook::GetTabs()->Page(i)->Grid->file->maxx()>3){
					Notebook::GetTabs()->Page(i)->Grid->file->RemoveFirst(2);
					KaiLog(_("Zabrakło pamięci RAM, usunięto część historii"));
					return;
				}
			}
		}
	}

	std::exit(1);
}

void KainoteFrame::OnRunScript(wxCommandEvent& event)
{
	//if (!Auto){ Auto = new Auto::Automation(true, true); }
	//else 
	if (Auto->Scripts.size() < 1){ Auto->ReloadScripts(true); }
	wxString name = Hkeys.GetName(idAndType(event.GetId()));
	if (!name.StartsWith(L"Script ")){ KaiMessageBox(wxString::Format(_("Skrót o nazwie '%s' nie należy do skrypru.")), _("Błąd")); return; }
	else{ name = name.Mid(7); }
	wxString path = name.BeforeLast(L'-');
	int wmacro = 0;
	if (path.IsEmpty()){ path = name; }
	else{ wmacro = wxAtoi(name.AfterLast(L'-')); }
	if (!wxFileExists(path)){
		Hkeys.SetHKey(idAndType(event.GetId()), L"Script " + name, L"");
		Hkeys.SaveHkeys(false);
		return;
	}
	Auto::LuaScript *script = Auto->FindScript(path);
	if (!script){
		Auto->Add(path);
		script = Auto->ASSScripts[Auto->ASSScripts.size() - 1];
	}
	else{
		if (script->CheckLastModified(true)){ script->Reload(); }
	}
	auto macro = script->GetMacro(wmacro);
	if (macro){
		TabPanel *pan = GetTab();
		if (macro->Validate(pan)){
			macro->Run(pan);
		}
		else{
			KaiMessageBox(wxString::Format(_("Warunki skryptu Lua '%s' nie zostały spełnione"), script->GetPrettyFilename()), _("Błąd"));
		}
	}
	else{
		KaiMessageBox(wxString::Format(_("Błąd wczytywania skryptu Lua: %s\n%s"), script->GetPrettyFilename(), script->GetDescription()), _("Błąd"));
		Auto->OnEdit(script->GetFilename());
	}
}

bool KainoteFrame::Layout()
{
	OnSize(wxSizeEvent());
	return true;
}
