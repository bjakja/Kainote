/***************************************************************
* Copyright (c) 2012-2020, Marcin Drob
* Name:      kainoteMain.cpp
* Purpose:   Subtitles editor and player
* Author:    Bjakja (bjakja@op.pl)
* Created:   2012-04-23
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
#include "UtilsWindows.h"
#include "SubsTime.h"
#include "ScriptInfo.h"
#include "Config.h"
#include "OptionsDialog.h"
#include "DropFiles.h"
#include "OpennWrite.h"
#include "Hotkeys.h"
#include "FontCollector.h"
#include "Menu.h"
#include "KaiTextCtrl.h"
#include "KaiMessageBox.h"
#include "SubsResampleDialog.h"
#include "SpellCheckerDialog.h"
#include "SpellChecker.h"
#include "AutoSaveOpen.h"
#include "AutoSavesRemoving.h"
#include "DummyVideo.h"

#include <wx/accel.h>
#include <wx/dir.h>
#include <wx/sysopt.h>
#include <wx/filedlg.h>

#include <boost/locale/generator.hpp>

#undef IsMaximized
#if _DEBUG
#define logging 5
#endif

#if !defined INSTRUCTIONS
#define INSTRUCTIONS L""
#endif


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

	Options.GetTable(SUBS_RECENT_FILES, subsrec);
	Options.GetTable(VIDEO_RECENT_FILES, videorec);
	Options.GetTable(AUDIO_RECENT_FILES, audsrec);
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

	StatusBar = new KaiStatusBar(this, ID_STATUS_BAR);
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
	lastSession->Append(GLOBAL_LOAD_EXTERNAL_SESSION, _("Wczytaj sesję z pliku"), _("Wczytuje sesję z wcześniej zapisanego pliku sesji"));
	lastSession->Append(GLOBAL_SAVE_EXTERNAL_SESSION, _("Zapisz sesję do pliku"), _("Zapisuje sesję do pliku"));
	int lastSessionConfig = Options.GetInt(LAST_SESSION_CONFIG);
	lastSession->Append(GLOBAL_ASK_FOR_LOAD_LAST_SESSION, _("Pytaj o wczytanie ostatniej sesji przy starcie programu"), NULL, _("Pyta, czy wczytać ostatnio zaczytane pliki przy starcie programu"), ITEM_CHECK_AND_HIDE)->Check(lastSessionConfig == 1);
	lastSession->Append(GLOBAL_LOAD_LAST_SESSION_ON_START, _("Wczytaj ostatnią sesję przy starcie programu"), NULL, _("Wczytuje poprzednio zaczytane pliki przy starcie programu"), ITEM_CHECK_AND_HIDE)->Check(lastSessionConfig == 2);
	

	FileMenu->AppendTool(Toolbar, GLOBAL_OPEN_SUBS, _("&Otwórz napisy"), _("Otwórz plik napisów"), PTR_BITMAP_PNG(L"opensubs"));
	FileMenu->AppendTool(Toolbar, GLOBAL_SAVE_SUBS, _("&Zapisz"), _("Zapisz aktualny plik"), PTR_BITMAP_PNG(L"save"), false);
	FileMenu->AppendTool(Toolbar, GLOBAL_SAVE_ALL_SUBS, _("Zapisz &wszystko"), _("Zapisz wszystkie napisy"), PTR_BITMAP_PNG(L"saveall"));
	FileMenu->AppendTool(Toolbar, GLOBAL_SAVE_SUBS_AS, _("Zapisz &jako..."), _("Zapisz jako"), PTR_BITMAP_PNG(L"saveas"));
	FileMenu->AppendTool(Toolbar, GLOBAL_SAVE_TRANSLATION, _("Zapisz &tłumaczenie"), _("Zapisz tłumaczenie"), PTR_BITMAP_PNG(L"savetl"), false);
	FileMenu->AppendTool(Toolbar, GLOBAL_RECENT_SUBS, _("Ostatnio otwa&rte napisy"), _("Ostatnio otwarte napisy"), PTR_BITMAP_PNG(L"recentsubs"), true, SubsRecMenu);
	FileMenu->AppendTool(Toolbar, GLOBAL_REMOVE_SUBS, _("Usuń napisy z e&dytora"), _("Usuń napisy z edytora"), PTR_BITMAP_PNG(L"close"));
	FileMenu->Append(GLOBAL_SAVE_WITH_VIDEO_NAME, _("Zapisuj napisy z nazwą wideo"), _("Zapisuj napisy z nazwą wideo"), true, PTR_BITMAP_PNG(L"SAVEWITHVIDEONAME"), NULL, ITEM_CHECK)->Check(Options.GetBool(SUBS_AUTONAMING));
	Toolbar->AddID(GLOBAL_SAVE_WITH_VIDEO_NAME);
	FileMenu->Append(GLOBAL_OPEN_AUTO_SAVE, _("Otwórz autozapis"), _("Otwiera autozapis wybrany z listy"));
	FileMenu->Append(GLOBAL_DELETE_TEMPORARY_FILES, _("Usuń pliki tymczasowe"), _("Otwiera okno usuwania plików tymczasowych"));
	FileMenu->Append(9989, _("Pokaż / Ukryj okno logów"))->DisableMapping();
	FileMenu->Append(9990, _("Ostatnia sesja"), _("Opcje ostatniej sesji"), true, PTR_BITMAP_PNG(L"OPEN_LAST_SESSION"), lastSession);
	FileMenu->AppendTool(Toolbar, GLOBAL_SETTINGS, _("&Ustawienia"), _("Ustawienia programu"), PTR_BITMAP_PNG(L"SETTINGS"));
	FileMenu->AppendTool(Toolbar, GLOBAL_QUIT, _("Wyjści&e\tAlt-F4"), _("Zakończ działanie programu"), PTR_BITMAP_PNG(L"exit"))->DisableMapping();
	Menubar->Append(FileMenu, _("&Plik"));

	EditMenu = new Menu();
	EditMenu->AppendTool(Toolbar, GLOBAL_UNDO, _("&Cofnij"), _("Cofnij"), PTR_BITMAP_PNG(L"undo"), false);
	EditMenu->AppendTool(Toolbar, GLOBAL_UNDO_TO_LAST_SAVE, _("Cofnij do ostatniego zapisu"), _("Cofnij do ostatniego zapisu"), PTR_BITMAP_PNG(L"UNDOTOLASTSAVE"), false);
	EditMenu->AppendTool(Toolbar, GLOBAL_REDO, _("&Ponów"), _("Ponów"), PTR_BITMAP_PNG(L"redo"), false);
	EditMenu->AppendTool(Toolbar, GLOBAL_HISTORY, _("&Historia"), _("Historia"), PTR_BITMAP_PNG(L"history"), true);
	EditMenu->AppendTool(Toolbar, GLOBAL_FIND_REPLACE, _("Znajdź i za&mień"), _("Szuka i podmienia dane frazy tekstu"), PTR_BITMAP_PNG(L"findreplace"));
	EditMenu->AppendTool(Toolbar, GLOBAL_SEARCH, _("Z&najdź"), _("Szuka dane frazy tekstu"), PTR_BITMAP_PNG(L"search"));
	EditMenu->AppendTool(Toolbar, GLOBAL_FIND_NEXT, _("Znajdź następny"), _("Znajduje kolejne wystąpięnie frazy w tekście"), PTR_BITMAP_PNG(L"search"));
	Menu *SortMenu[2];
	for (int i = 0; i < 2; i++){
		SortMenu[i] = new Menu();
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_START_TIMES + (6 * i), _("Czas początkowy"), _("Sortuj według czasu początkowego"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_END_TIMES + (6 * i), _("Czas końcowy"), _("Sortuj według czasu końcowego"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_STYLE + (6 * i), _("Style"), _("Sortuj według stylów"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_ACTOR + (6 * i), _("Aktor"), _("Sortuj według aktora"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_EFFECT + (6 * i), _("Efekt"), _("Sortuj według efektu"));
		SortMenu[i]->Append(GLOBAL_SORT_ALL_BY_LAYER + (6 * i), _("Warstwa"), _("Sortuj według warstwy"));
	}

	EditMenu->AppendTool(Toolbar, GLOBAL_SORT_LINES, _("Sort&uj wszystkie linie"), _("Sortuje wszystkie linie napisów ASS"), PTR_BITMAP_PNG(L"sort"), true, SortMenu[0]);
	EditMenu->AppendTool(Toolbar, GLOBAL_SORT_SELECTED_LINES, _("Sortu&j zaznaczone linie"), _("Sortuje zaznaczone linie napisów ASS"), PTR_BITMAP_PNG(L"sortsel"), true, SortMenu[1]);
	EditMenu->AppendTool(Toolbar, GLOBAL_MISSPELLS_REPLACER, _("Popraw drobne błędy (eksperymentalne)"), _("Włącza okno poprawiania błędów"), PTR_BITMAP_PNG(L"sellines"));
	EditMenu->AppendTool(Toolbar, GLOBAL_OPEN_SELECT_LINES, _("Zaznacz &linijki"), _("Zaznacza linijki wg danej frazy tekstu"), PTR_BITMAP_PNG(L"sellines"));
	Menubar->Append(EditMenu, _("&Edycja"));

	VidMenu = new Menu();
	VidMenu->AppendTool(Toolbar, GLOBAL_OPEN_VIDEO, _("Otwórz wideo"), _("Otwiera wybrane wideo"), PTR_BITMAP_PNG(L"openvideo"));
	VidsRecMenu = new Menu();
	VidMenu->AppendTool(Toolbar, GLOBAL_RECENT_VIDEO, _("Ostatnio otwarte wideo"), _("Ostatnio otwarte video"), PTR_BITMAP_PNG(L"recentvideo"), true, VidsRecMenu);
	VidMenu->AppendTool(Toolbar, GLOBAL_OPEN_KEYFRAMES, _("Otwórz klatki kluczowe"), _("Otwórz klatki kluczowe"), PTR_BITMAP_PNG(L"OPEN_KEYFRAMES"));
	KeyframesRecentMenu = new Menu();
	VidMenu->AppendTool(Toolbar, GLOBAL_RECENT_KEYFRAMES, _("Ostatnio otwarte klatki kluczowe"), _("Ostatnio otwarte klatki kluczowe"), PTR_BITMAP_PNG(L"RECENT_KEYFRAMES"), true, KeyframesRecentMenu);
	VidMenu->Append(GLOBAL_OPEN_DUMMY_VIDEO, _("Otwórz dummy wideo"), _("Otwórz dummy wideo"));
	VidMenu->AppendTool(Toolbar, GLOBAL_SET_START_TIME, _("Wstaw czas początkowy z wideo"), _("Wstawia czas początkowy z wideo"), PTR_BITMAP_PNG(L"setstarttime"), false);
	VidMenu->AppendTool(Toolbar, GLOBAL_SET_END_TIME, _("Wstaw czas końcowy z wideo"), _("Wstawia czas końcowy z wideo"), PTR_BITMAP_PNG(L"setendtime"), false);
	VidMenu->AppendTool(Toolbar, GLOBAL_PREVIOUS_FRAME, _("Klatka w tył"), _("Przechodzi o jedną klatkę w tył"), PTR_BITMAP_PNG(L"prevframe"), false);
	VidMenu->AppendTool(Toolbar, GLOBAL_NEXT_FRAME, _("Klatka w przód"), _("Przechodzi o jedną klatkę w przód"), PTR_BITMAP_PNG(L"nextframe"), false);
	VidMenu->AppendTool(Toolbar, GLOBAL_SET_VIDEO_AT_START_TIME, _("Przejdź do czasu początkowego linii"), _("Przechodzi wideo do czasu początkowego linii"), PTR_BITMAP_PNG(L"videoonstime"));
	VidMenu->AppendTool(Toolbar, GLOBAL_SET_VIDEO_AT_END_TIME, _("Przejdź do czasu końcowego linii"), _("Przechodzi wideo do czasu końcowego linii"), PTR_BITMAP_PNG(L"videoonetime"));
	VidMenu->AppendTool(Toolbar, GLOBAL_PLAY_PAUSE, _("Odtwarzaj / Pauza"), _("Odtwarza lub pauzuje wideo"), PTR_BITMAP_PNG(L"pausemenu"), false);
	VidMenu->AppendTool(Toolbar, GLOBAL_GO_TO_PREVIOUS_KEYFRAME, _("Przejdź do poprzedniej klatki kluczowej"), L"", PTR_BITMAP_PNG(L"prevkeyframe"));
	VidMenu->AppendTool(Toolbar, GLOBAL_GO_TO_NEXT_KEYFRAME, _("Przejdź do następnej klatki kluczowej"), L"", PTR_BITMAP_PNG(L"nextkeyframe"));
	VidMenu->AppendTool(Toolbar, GLOBAL_SET_AUDIO_FROM_VIDEO, _("Ustaw audio z czasem wideo"), L"", PTR_BITMAP_PNG(L"SETVIDEOTIMEONAUDIO"));
	VidMenu->AppendTool(Toolbar, GLOBAL_SET_AUDIO_MARK_FROM_VIDEO, _("Ustaw znacznik audio z czasem wideo"), L"", PTR_BITMAP_PNG(L"SETVIDEOTIMEONAUDIOMARK"));
	VidMenu->AppendTool(Toolbar, GLOBAL_VIDEO_ZOOM, _("Powiększ wideo"), "", PTR_BITMAP_PNG(L"zoom"));
	bool videoIndex = Options.GetBool(VIDEO_INDEX);
	VidMenu->Append(GLOBAL_VIDEO_INDEXING, _("Otwieraj wideo przez FFMS2"), _("Otwiera wideo przez FFMS2, co daje dokładność klatkową"), true, PTR_BITMAP_PNG(L"FFMS2INDEXING"), 0, ITEM_CHECK)->Check(videoIndex);
	Toolbar->AddID(GLOBAL_VIDEO_INDEXING);
	Menubar->Append(VidMenu, _("&Wideo"));

	AudMenu = new Menu();
	AudMenu->AppendTool(Toolbar, GLOBAL_OPEN_AUDIO, _("Otwórz audio"), _("Otwiera wybrane audio"), PTR_BITMAP_PNG(L"openaudio"));
	AudsRecMenu = new Menu();

	AudMenu->AppendTool(Toolbar, GLOBAL_RECENT_AUDIO, _("Ostatnio otwarte audio"), _("Ostatnio otwarte audio"), PTR_BITMAP_PNG(L"recentaudio"), true, AudsRecMenu);
	AudMenu->AppendTool(Toolbar, GLOBAL_AUDIO_FROM_VIDEO, _("Otwórz audio z wideo"), _("Otwiera audio z wideo"), PTR_BITMAP_PNG(L"audiofromvideo"));
	AudMenu->Append(GLOBAL_OPEN_DUMMY_AUDIO, _("Otwórz puste audio na 2:30 godziny"), _("Otwiera puste audio na 2:30 godziny"));
	AudMenu->AppendTool(Toolbar, GLOBAL_CLOSE_AUDIO, _("Zamknij audio"), _("Zamyka audio"), PTR_BITMAP_PNG(L"closeaudio"));
	Menubar->Append(AudMenu, _("A&udio"));

	ViewMenu = new Menu();
	ViewMenu->Append(GLOBAL_VIEW_ALL, _("Wszystko"), _("Wszystkie okna są widoczne"));
	ViewMenu->Append(GLOBAL_VIEW_VIDEO, _("Wideo i napisy"), _("Widoczne tylko okno wideo i napisów"));
	ViewMenu->Append(GLOBAL_VIEW_AUDIO, _("Audio i napisy"), _("Widoczne tylko okno audio i napisów"));
	ViewMenu->Append(GLOBAL_VIEW_ONLY_VIDEO, _("Tylko wideo"), _("Widoczne tylko okno wideo"));
	ViewMenu->Append(GLOBAL_VIEW_SUBS, _("Tylko napisy"), _("Widoczne tylko okno napisów"));
	Menubar->Append(ViewMenu, _("Wido&k"));

	SubsMenu = new Menu();
	SubsMenu->AppendTool(Toolbar, GLOBAL_EDITOR, _("Włącz / Wyłącz edytor"), _("Włączanie bądź wyłączanie edytora"), PTR_BITMAP_PNG(L"editor"))->Enable(!videoIndex);
	SubsMenu->AppendTool(Toolbar, GLOBAL_OPEN_ASS_PROPERTIES, _("Właściwości pliku ASS"), _("Właściwości napisów ASS"), PTR_BITMAP_PNG(L"ASSPROPS"));
	SubsMenu->AppendTool(Toolbar, GLOBAL_OPEN_STYLE_MANAGER, _("&Menedżer stylów"), _("Służy do zarządzania stylami ASS"), PTR_BITMAP_PNG(L"styles"));
	ConvMenu = new Menu();
	ConvMenu->AppendTool(Toolbar, GLOBAL_CONVERT_TO_ASS, _("Konwertuj do ASS"), _("Konwertuje do formatu ASS"), PTR_BITMAP_PNG(L"convass"), false);
	ConvMenu->AppendTool(Toolbar, GLOBAL_CONVERT_TO_SRT, _("Konwertuj do SRT"), _("Konwertuje do formatu SRT"), PTR_BITMAP_PNG(L"convsrt"));
	ConvMenu->AppendTool(Toolbar, GLOBAL_CONVERT_TO_MDVD, _("Konwertuj do MDVD"), _("Konwertuje do formatu microDVD"), PTR_BITMAP_PNG(L"convmdvd"));
	ConvMenu->AppendTool(Toolbar, GLOBAL_CONVERT_TO_MPL2, _("Konwertuj do MPL2"), _("Konwertuje do formatu MPL2"), PTR_BITMAP_PNG(L"convmpl2"));
	ConvMenu->AppendTool(Toolbar, GLOBAL_CONVERT_TO_TMP, _("Konwertuj do TMP"), _("Konwertuje do formatu TMPlayer (niezalecane)"), PTR_BITMAP_PNG(L"convtmp"));

	SubsMenu->Append(ID_CONVERSION, _("Konwersja"), _("Konwersja z jednego formatu napisów na inny"), true, PTR_BITMAP_PNG(L"convert"), ConvMenu);
	SubsMenu->AppendTool(Toolbar, GLOBAL_SHOW_SHIFT_TIMES, _("Okno zmiany &czasów\tCtrl-I"), _("Przesuwanie czasów napisów"), PTR_BITMAP_PNG(L"times"));
	SubsMenu->AppendTool(Toolbar, GLOBAL_OPEN_FONT_COLLECTOR, _("Kolekcjoner czcionek"), _("Kolekcjoner czcionek"), PTR_BITMAP_PNG(L"fontcollector"));
	SubsMenu->AppendTool(Toolbar, GLOBAL_OPEN_SUBS_RESAMPLE, _("Zmień rozdzielczość napisów"), _("Zmień rozdzielczość napisów"), PTR_BITMAP_PNG(L"subsresample"));
	SubsMenu->AppendTool(Toolbar, GLOBAL_OPEN_SPELLCHECKER, _("Sprawdź poprawność pisowni"), _("Sprawdź poprawność pisowni"), PTR_BITMAP_PNG(L"spellchecker"));
	SubsMenu->AppendTool(Toolbar, GLOBAL_HIDE_TAGS, _("Ukryj tagi w nawiasach"), _("Ukrywa tagi w nawiasach ASS i MDVD"), PTR_BITMAP_PNG(L"hidetags"));
	Menubar->Append(SubsMenu, _("&Napisy"));

	AutoMenu = new Menu();
	AutoMenu->AppendTool(Toolbar, GLOBAL_AUTOMATION_LOAD_SCRIPT, _("Wczytaj skrypt"), _("Wczytaj skrypt"), PTR_BITMAP_PNG(L"automation"));
	AutoMenu->Append(GLOBAL_AUTOMATION_RELOAD_AUTOLOAD, _("Odśwież skrypty autoload"), _("Odśwież skrypty autoload"), true, PTR_BITMAP_PNG(L"automation"));
	AutoMenu->Append(GLOBAL_AUTOMATION_LOAD_LAST_SCRIPT, _("Uruchom ostatnio zaczytany skrypt"), _("Uruchom ostatnio zaczytany skrypt"));
	AutoMenu->Append(GLOBAL_AUTOMATION_OPEN_HOTKEYS_WINDOW, _("Otwórz okno mapowania skrótów"), _("Otwórz okno mapowania skrótów"));
	Menubar->Append(AutoMenu, _("Au&tomatyzacja"));

	HelpMenu = new Menu();
	HelpMenu->AppendTool(Toolbar, GLOBAL_HELP, _("&Pomoc (niekompletna, ale jednak)"), _("Otwiera pomoc w domyślnej przeglądarce"), PTR_BITMAP_PNG(L"help"));
	HelpMenu->AppendTool(Toolbar, GLOBAL_ANSI, _("&Wątek programu na forum AnimeSub.info"), _("Otwiera wątek programu na forum AnimeSub.info"), PTR_BITMAP_PNG(L"ansi"));
	HelpMenu->AppendTool(Toolbar, GLOBAL_ABOUT, _("&O programie"), _("Wyświetla informacje o programie"), PTR_BITMAP_PNG(L"about"));
	HelpMenu->AppendTool(Toolbar, GLOBAL_HELPERS, _("&Lista osób pomocnych przy tworzeniu programu"), _("Wyświetla listę osób pomocnych przy tworzeniu programu"), PTR_BITMAP_PNG(L"helpers"));
	Menubar->Append(HelpMenu, _("Pomo&c"));

	Toolbar->InitToolbar();

	//SetSizer(mains1);

	SetAccels(false);


	Connect(ID_TABS, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageChanged, 0, this);
	Connect(GLOBAL_ADD_PAGE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageAdd);
	Connect(GLOBAL_CLOSE_PAGE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageClose);
	Connect(GLOBAL_NEXT_TAB, GLOBAL_PREVIOUS_TAB, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnPageChange);
	//Here add new ids
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, GLOBAL_SAVE_SUBS, GLOBAL_LOAD_LAST_SESSION);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, 
		GLOBAL_SORT_ALL_BY_START_TIMES, GLOBAL_SORT_SELECTED_BY_LAYER);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, GLOBAL_SHIFT_TIMES);
	Connect(GLOBAL_OPEN_SUBS, GLOBAL_ANSI, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnMenuSelected1);
	Connect(GLOBAL_SELECT_FROM_VIDEO, GLOBAL_STYLE_MANAGER_CLEAN_STYLE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnMenuSelected1);
	Connect(GLOBAL_PREVIOUS_LINE, GLOBAL_JOIN_WITH_NEXT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnChangeLine);
	Connect(GLOBAL_REMOVE_LINES, GLOBAL_REMOVE_TEXT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnDelete);
	Menubar->Connect(EVT_MENU_OPENED, (wxObjectEventFunction)&KainoteFrame::OnMenuOpened, 0, this);
	Connect(wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&KainoteFrame::OnClose1);
	Connect(30000, 30079, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnRecent);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &event){
		LogHandler::ShowLogWindow();
	}, 9989);

	Bind(wxEVT_ACTIVATE, &KainoteFrame::OnActivate, this);
	Connect(GLOBAL_SNAP_WITH_START, GLOBAL_SNAP_WITH_END, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&KainoteFrame::OnAudioSnap);
	Tabs->SetDropTarget(new DragnDrop(this));
	Bind(wxEVT_SIZE, &KainoteFrame::OnSize, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnMenuSelected, this, 
		GLOBAL_ASK_FOR_LOAD_LAST_SESSION, GLOBAL_LOAD_LAST_SESSION_ON_START);

	auto focusFunction = [=](wxFocusEvent &event) -> void {
		TabPanel *tab = GetTab();
		if (tab->lastFocusedWindowId){
			wxWindow *win = FindWindowById(tab->lastFocusedWindowId, tab);
			if (win) {
				win->SetFocus();
				return;
			}
		}
		if (tab->Grid->IsShown()){
			tab->Grid->SetFocus();
		}//test why it was disabled or fix this bug
		else if (tab->Video->IsShown()){
			tab->Video->SetFocus();
		}

	};
	bool im = Options.GetBool(WINDOW_MAXIMIZED);
	if (im){ Maximize(Options.GetBool(WINDOW_MAXIMIZED)); }

	if (!Options.GetBool(EDITOR_ON)){ HideEditor(false); }
	
	SetSubsResolution(false);
	Auto = new Auto::Automation();
	sendFocus.SetOwner(this, 6789);

	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		//if it will crash on last focused window
		//we need to remove last focused window
		//it's not needed here
		//change focus only when main window was activated via frame bar
		//or when tab changed
		if (this->HasFocus() || this->Tabs->HasFocus() || this->Menubar->HasFocus())
			focusFunction(wxFocusEvent());

	}, 6789);

	boost::locale::generator gen;
	// Make system default locale global
	std::locale loc = gen("");
	std::locale::global(loc);
}

KainoteFrame::~KainoteFrame()
{
	//this will prevent crashes when function want to use relesed elements
	//when closing it can be skipped
	Options.SetClosing();
	Unbind(wxEVT_ACTIVATE, &KainoteFrame::OnActivate, this);
	bool im = IsMaximized();
	if (!im && !IsIconized()){
		int posx, posy, sizex = 0, sizey = 0;
		GetPosition(&posx, &posy);
		if (posx < 10000 && posx > -4000){
			Options.SetCoords(WINDOW_POSITION, posx, posy);
		}
		GetSize(&sizex, &sizey);
		if (sizex >= 600 && sizey >= 400)
			Options.SetCoords(WINDOW_SIZE, sizex, sizey);

		wxRect monRect = GetMonitorRect1(0, NULL, wxRect(posx, posy, sizex, sizey));
		Options.SetCoords(MONITOR_POSITION, monRect.x, monRect.y);
		Options.SetCoords(MONITOR_SIZE, monRect.GetWidth(), monRect.GetHeight());
	}

	Toolbar->Destroy();
	Options.SetBool(WINDOW_MAXIMIZED, im);
	Options.SetBool(EDITOR_ON, GetTab()->editor);
	Options.SetTable(SUBS_RECENT_FILES, subsrec);
	Options.SetTable(VIDEO_RECENT_FILES, videorec);
	Options.SetTable(AUDIO_RECENT_FILES, audsrec);
	GetTab()->Video->SaveVolume();
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


//elements of menu that can be disabled
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

	//it should not be blocked, frames should go as fast as possible
	if ((id == GLOBAL_PREVIOUS_FRAME || id == GLOBAL_NEXT_FRAME)){
		tab->Video->ChangePositionByFrame((id == GLOBAL_PREVIOUS_FRAME) ? -1 : 1);
	}

	//if (Options.CheckLastKeyEvent(id))
		//return;

	if (id == GLOBAL_SAVE_SUBS){
		Save(Tabs->HasRecoverySubs());
	}
	else if (id == GLOBAL_SAVE_SUBS_AS){
		Save(true);
	}
	else if (id == GLOBAL_SAVE_ALL_SUBS){
		SaveAll();
	}
	else if (id == GLOBAL_SAVE_TRANSLATION){
		GetTab()->Grid->AddSInfo(L"TLMode", L"Translated", false);
		Save(true, -1, false);
		GetTab()->Grid->AddSInfo(L"TLMode", L"Yes", false);
	}
	else if (id == GLOBAL_REMOVE_SUBS){
		if (SavePrompt(3)){ event.SetInt(-1); return; }
		if (tab->SubsPath != L""){
			tab->SubsName = _("Bez tytułu");
			tab->SubsPath = L"";
			Label();

			tab->Grid->Clearing();
			tab->Grid->file = new SubsFile(&tab->Grid->GetMutex());
			tab->Grid->LoadDefault();
			tab->Edit->RefreshStyle(true);
			tab->Grid->RefreshColumns();

			if (tab->Video->GetState() != None){
				if (tab->Video->RemoveVisual()){
					tab->Video->OpenSubs(CLOSE_SUBTITLES, true, true);
				}
			}
			SetSubsResolution(false);
		}
	}
	else if (id == GLOBAL_UNDO){
		tab->Grid->DoUndo(false);
	}
	else if (id == GLOBAL_REDO){
		tab->Grid->DoUndo(true);
	}
	else if (id == GLOBAL_HISTORY){
		tab->Grid->file->ShowHistory(this, [=](int iter){
			tab->Grid->DoUndo(false, iter);
		});
	}
	else if (id == GLOBAL_SEARCH || id == GLOBAL_FIND_REPLACE){
		if (!FR){ FR = new FindReplaceDialog(this, (id == GLOBAL_FIND_REPLACE) ? WINDOW_REPLACE : WINDOW_FIND); }
		else{ FR->ShowDialog((id == GLOBAL_FIND_REPLACE) ? WINDOW_REPLACE : WINDOW_FIND); }
	}
	else if (id == GLOBAL_OPEN_SELECT_LINES){
		if (!SL){ SL = new SelectLines(this); }
		SL->Show();
	}
	else if (id == GLOBAL_PLAY_PAUSE){
		tab->Video->Pause();
	}
	else if (id == GLOBAL_SET_START_TIME || id == GLOBAL_SET_END_TIME){
		if (tab->Video->GetState() != None){
			if (id == GLOBAL_SET_START_TIME){
				int time = tab->Video->GetFrameTime() + Options.GetInt(GRID_INSERT_START_OFFSET);
				tab->Grid->SetStartTime(ZEROIT(time));
			}
			else{
				int time = tab->Video->GetFrameTime(false) + Options.GetInt(GRID_INSERT_END_OFFSET);
				tab->Grid->SetEndTime(ZEROIT(time));
			}
		}
	}
	else if (id == GLOBAL_SET_AUDIO_FROM_VIDEO || id == GLOBAL_SET_AUDIO_MARK_FROM_VIDEO){
		if (tab->Edit->ABox){
			AudioDisplay *adisp = tab->Edit->ABox->audioDisplay;
			int time = tab->Video->Tell();
			int pos = adisp->GetXAtMS(time);
			if (id == GLOBAL_SET_AUDIO_MARK_FROM_VIDEO)
				adisp->SetMark(time);
			adisp->ChangePosition(time);
		}
	}
	else if (id == GLOBAL_VIDEO_INDEXING || id == GLOBAL_SAVE_WITH_VIDEO_NAME){
		toolitem *ToolItem = Toolbar->FindItem(id);
		MenuItem *Item = (item) ? item : Menubar->FindItem(id);
		CONFIG conf = (id == GLOBAL_VIDEO_INDEXING) ? VIDEO_INDEX : SUBS_AUTONAMING;
		bool ItemOn = true;
		if (Modif == TOOLBAR_EVENT && ToolItem){
			if (Item)
				Item->Check(ToolItem->toggled);
			ItemOn = ToolItem->toggled;
			Options.SetBool(conf, ToolItem->toggled);
		}
		else if (Item){
			if (ToolItem){
				ToolItem->toggled = (id == Modif) ? !ToolItem->toggled : Item->IsChecked();
				Toolbar->Refresh(false);
			}
			if (id == Modif && Item)
				Item->Check(!Item->IsChecked());
			ItemOn = Item->IsChecked();
			Options.SetBool(conf, ItemOn);
		}
		if (id == GLOBAL_VIDEO_INDEXING) {
			toolitem *TurnOffEditorToolItem = Toolbar->FindItem(GLOBAL_EDITOR);
			if (TurnOffEditorToolItem) {
				TurnOffEditorToolItem->Enable(!ItemOn);
			}
		}
	}
	else if (id == GLOBAL_VIDEO_ZOOM){
		tab->Video->SetZoom();
	}
	else if (id >= GLOBAL_OPEN_AUDIO && id <= GLOBAL_CLOSE_AUDIO){
		OnOpenAudio(event);
	}
	else if (id == GLOBAL_OPEN_ASS_PROPERTIES){
		OnAssProps();
	}
	else if (id == GLOBAL_OPEN_STYLE_MANAGER){
		StyleStore::ShowStore();
	}
	else if (id == GLOBAL_OPEN_SUBS_RESAMPLE){
		TabPanel *tab = GetTab();
		int x = 0, y = 0;
		tab->Grid->GetASSRes(&x, &y);
		SubsResampleDialog SRD(this, wxSize(x, y), (tab->Video->GetState() == None) ? wxSize(x, y) : tab->Video->GetVideoSize(), L"", L"");
		SRD.ShowModal();
	}
	else if (id == GLOBAL_OPEN_FONT_COLLECTOR && tab->Grid->subsFormat < SRT){
		if (!FC){ FC = new FontCollector(this); }
		FC->ShowDialog(this);
	}
	else if (id == GLOBAL_MISSPELLS_REPLACER){
		if (!MR){ MR = new MisspellReplacer(this); }
		MR->Show(!MR->IsShown());
	}
	else if (id >= GLOBAL_CONVERT_TO_ASS && id <= GLOBAL_CONVERT_TO_MPL2){
		if (tab->Grid->GetSInfo(L"TLMode") != L"Yes"){
			OnConversion((id - GLOBAL_CONVERT_TO_ASS) + 1);
		}
	}
	else if (id == GLOBAL_OPEN_SPELLCHECKER){
		SpellCheckerDialog *SCD = new SpellCheckerDialog(this);
	}
	else if (id == GLOBAL_HIDE_TAGS){
		tab->Grid->HideOverrideTags();
	}
	else if (id == GLOBAL_SHOW_SHIFT_TIMES){
		bool show = !tab->ShiftTimes->IsShown();
		Options.SetBool(SHIFT_TIMES_ON, show);
		tab->ShiftTimes->Show(show);
		tab->GridShiftTimesSizer->Layout();
	}
	else if (id >= GLOBAL_SORT_ALL_BY_START_TIMES && id <= GLOBAL_SORT_SELECTED_BY_LAYER){
		bool all = id < GLOBAL_SORT_SELECTED_BY_START_TIMES;
		int difid = (all) ? GLOBAL_SORT_ALL_BY_START_TIMES : GLOBAL_SORT_SELECTED_BY_START_TIMES;
		tab->Grid->SortIt(id - difid, all);
	}
	else if (id >= GLOBAL_VIEW_ALL && id <= GLOBAL_VIEW_SUBS){
		bool vidshow = (id == GLOBAL_VIEW_ALL || id == GLOBAL_VIEW_VIDEO || id == GLOBAL_VIEW_ONLY_VIDEO) && tab->Video->GetState() != None;
		bool vidvis = tab->Video->IsShown();
		if (!vidshow && tab->Video->GetState() == Playing){ tab->Video->Pause(); }
		tab->Video->Show(vidshow);
		if (vidshow && !vidvis){
			tab->Video->OpenSubs(OPEN_DUMMY);
		}
		if (tab->Edit->ABox){
			tab->Edit->ABox->Show((id == GLOBAL_VIEW_ALL || id == GLOBAL_VIEW_AUDIO));
			if (id == GLOBAL_VIEW_AUDIO){ tab->Edit->SetMinSize(wxSize(500, 350)); }
			if (id != GLOBAL_VIEW_AUDIO && id != GLOBAL_VIEW_ALL){
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
			Options.GetCoords(VIDEO_WINDOW_SIZE, &x, &y);
			tab->Video->SetMinSize(wxSize(x, y));
		}
		tab->Layout();
	}
	else if (id == GLOBAL_AUTOMATION_LOAD_SCRIPT){
		//if (!Auto){ Auto = new Auto::Automation(); }
		if (Auto->ASSScripts.size() < 1)
			Auto->AddFromSubs();
		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz sktypt"),
			Options.GetString(AUTOMATION_RECENT_FILES),
			L"", _("Pliki skryptów (*.lua),(*.moon)|*.lua;*.moon;"), wxFD_OPEN);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxString file = FileDialog1->GetPath();
			Options.SetString(AUTOMATION_RECENT_FILES, file.AfterLast(L'\\'));
			//if(Auto->Add(file)){Auto->BuildMenu(&AutoMenu);}
			Auto->Add(file);
		}
		FileDialog1->Destroy();

	}
	else if (id == GLOBAL_AUTOMATION_RELOAD_AUTOLOAD){
		Auto->ReloadScripts();
	}
	else if (id == GLOBAL_AUTOMATION_LOAD_LAST_SCRIPT){
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
	else if (id == GLOBAL_AUTOMATION_OPEN_HOTKEYS_WINDOW){
		Auto->ShowScriptHotkeysWindow(this);
	}
	else if (id == GLOBAL_GO_TO_PREVIOUS_KEYFRAME){
		tab->Video->GoToPrevKeyframe();
		tab->Video->RefreshTime();
	}
	else if (id == GLOBAL_GO_TO_NEXT_KEYFRAME){
		tab->Video->GoToNextKeyframe();
		tab->Video->RefreshTime();
	}
	else if (id == GLOBAL_SET_VIDEO_AT_START_TIME){
		int curline = tab->Grid->currentLine;
		tab->Video->Seek(MAX(0, tab->Grid->GetDialogue(curline)->Start.mstime), true);
	}
	else if (id == GLOBAL_SET_VIDEO_AT_END_TIME){
		int curline = tab->Grid->currentLine;
		tab->Video->Seek(MAX(0, tab->Grid->GetDialogue(curline)->End.mstime), false);
	}
	else if (id == GLOBAL_UNDO_TO_LAST_SAVE){
		tab->Grid->DoUndo(false, tab->Grid->file->GetLastSaveIter());
	}
	else if (id == GLOBAL_LOAD_LAST_SESSION){
		Tabs->LoadLastSession();
	}
	else if (id == GLOBAL_LOAD_EXTERNAL_SESSION || id == GLOBAL_SAVE_EXTERNAL_SESSION) {
		OnExternalSession(id);
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
	else if (id == GLOBAL_FIND_NEXT){
		FR->FindNext();
	}
}
//elements of menu all time enabled
void KainoteFrame::OnMenuSelected1(wxCommandEvent& event)
{
	int id = event.GetId();
	int Modif = event.GetInt();
	TabPanel *tab = GetTab();

	if (Modif == wxMOD_SHIFT){
		Hkeys.OnMapHkey(id, L"", this, GLOBAL_HOTKEY);
		return;
	}

	if (Options.CheckLastKeyEvent(id))
		return;

	if (id == GLOBAL_OPEN_SUBS){

		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik napisów"),
			(tab->VideoPath != L"") ? tab->VideoPath.BeforeLast(L'\\') :
			(subsrec.size() > 0) ? subsrec[0].BeforeLast(L'\\') : L"",
			L"", _("Pliki napisów (*.ass),(*.ssa),(*.srt),(*.sub),(*.txt)|*.ass;*.ssa;*.srt;*.sub;*.txt|Pliki wideo z wbudowanymi napisami (*.mkv)|*.mkv"),
			wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxArrayString paths;
			FileDialog1->GetPaths(paths);
			Freeze();
			tab->Hide();
			for (auto &file : paths){
				if (tab->SubsPath != L"") {
					InsertTab(false); 
					//set new tab
					tab = GetTab();
				}
				if (file.AfterLast(L'.') == L"mkv"){
					event.SetString(file);
					tab->Grid->OnMkvSubs(event);
				}
				else{
					OpenFile(file, false, false);
				}
			}
			Thaw();
			tab->Show();
		}
		FileDialog1->Destroy();
	}
	else if (id == GLOBAL_OPEN_VIDEO){
		wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"),
			(tab->SubsPath != L"") ? tab->SubsPath.BeforeLast(L'\\') :
			(videorec.size() > 0) ? videorec[0].BeforeLast(L'\\') : L"",
			L"", _("Pliki wideo(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.mpg),(*.mpeg),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.mpg;*.mpeg;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"),
			wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
		if (FileDialog2->ShowModal() == wxID_OK){
			wxArrayString paths;
			FileDialog2->GetPaths(paths);
			if (paths.size() == 1)
				OpenFile(paths[0]);
			else if(paths.size() > 1)
				OpenFiles(paths);
		}
		FileDialog2->Destroy();
	}
	else if (id == GLOBAL_OPEN_KEYFRAMES){
		wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"),
			tab->VideoPath != L"" ? tab->VideoPath.BeforeLast(L'\\') :
			(keyframesRecent.size() > 0) ? keyframesRecent[0].BeforeLast(L'\\') : L"",
			L"", _("Pliki klatek kluczowych (*.txt),(*.pass),(*.stats),(*.log)|*.txt;*.pass;*.stats;*.log|Wszystkie pliki (*.*)|*.*"),
			wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (FileDialog2->ShowModal() == wxID_OK){
			wxString path = FileDialog2->GetPath();
			tab->KeyframesPath = path;
			tab->Video->OpenKeyframes(path);
			SetRecent(3);
		}
		FileDialog2->Destroy();
	}
	else if (id == GLOBAL_OPEN_DUMMY_VIDEO) {
		DummyVideo dv(this);
		if (dv.ShowModal() == wxID_OK) {
			wxString dresult = dv.GetDummyText();
			if (!dresult.empty())
				Tabs->LoadVideo(GetTab(), dresult, -1, true, true, false, false, true);
		}
	}
	else if (id == GLOBAL_OPEN_DUMMY_AUDIO) {
		event.SetString(L"dummy-audio:silence?sr=44100&bd=16&ch=1&ln=396900000");
		OnOpenAudio(event);
	}
	else if (id == GLOBAL_OPEN_AUTO_SAVE) {
		AutoSaveOpen aso(this);
		aso.ShowModal();
	}
	else if (id == GLOBAL_DELETE_TEMPORARY_FILES) {
		AutoSavesRemoving asr(this);
		asr.ShowModal();
	}
	else if (id == GLOBAL_SETTINGS){
		OptionsDialog od(this, this);
		od.OptionsTree->ChangeSelection(0);
		od.ShowModal();
	}
	else if (id == GLOBAL_SELECT_FROM_VIDEO){
		tab->Grid->SelVideoLine();
	}
	else if (id == GLOBAL_EDITOR){
		HideEditor();
	}
	else if (id == GLOBAL_PLAY_ACTUAL_LINE){
		tab->Edit->TextEdit->SetFocus();
		tab->Video->PlayLine(tab->Edit->line->Start.mstime, tab->Video->GetPlayEndTime(tab->Edit->line->End.mstime));
	}
	else if (id == GLOBAL_STYLE_MANAGER_CLEAN_STYLE){
		StyleStore::Get()->OnCleanStyles(event);
	}
	else if (id == GLOBAL_QUIT){
		Close();
	}
	else if (id == GLOBAL_ABOUT){
		KaiMessageBox(wxString::Format(_("Edytor napisów by Marcin Drob aka Bakura lub Bjakja (bjakja7@gmail.com),\nwersja %s z dnia %s"),
			Options.progname.AfterFirst(L'v'), Options.GetReleaseDate()) + " \n\n" +
			_("Ten program powstał w celu zastąpienia dwóch programów: Bestplayera i Aegisuba.\n\n") +
			_("Jeśli zauważyłeś(aś) jakieś błędy bądź masz jakieś propozycje zmian lub nowych funkcji,\nmożesz napisać o tym na: forum ANSI, Githubie, bądź mailowo.\n\n") +
			_("Kainote zawiera w sobie części następujących projektów:\n") +
			L"wxWidgets - Copyright © Julian Smart, Robert Roebling et al.\n" +
			_("Color picker, wymuxowywanie napisów z mkv, audiobox, odwarzacz audio, automatyzacja\ni kilka innych pojedynczych funkcji wzięte z Aegisuba -\n") +
			L"Copyright © Rodrigo Braz Monteiro.\n"\
			L"Hunspell - Copyright © Kevin Hendricks.\n"\
			L"Matroska Parser - Copyright © Mike Matsnev.\n"\
			L"CSRI - Copyright © David Lamparter.\n"\
			L"Vsfilter - Copyright © Gabest.\n"\
			L"FFMPEGSource2 - Copyright © Fredrik Mellbin.\n"\
			L"ICU - Copyright © 1995-2016 International Business Machines Corporation and others.\n"\
			L"Boost - Copyright © Joe Coder 2004 - 2006."\
			L"FreeType2 - Copyright © 2006-2019 David Turner, Robert Wilhelm, and Werner Lemberg."\
			L"Fribidi - Copyright © 1991, 1999 Free Software Foundation, Inc."\
			L"Libass - Copyright © 2006-2016 libass contributors.",
			_("O Kainote"));
			//L"Interfejs Avisynth - Copyright © Ben Rudiak-Gould et al.\n"
	}
	else if (id == GLOBAL_HELPERS){
		wxString Testers = L"SoheiMajin, KamiTet, Wtas, BadRequest, Ognisty321, Nyah2211, dark, Ksenoform, Zły Los.";
		wxString Credits = _("Pomoc graficzna: (przyciski, obrazki do pomocy itd.)\n") +
			_("- Kostek00 (przyciski do audio i narzędzi wizualnych).\n") +
			_("- Xandros (nowe przyciski do wideo).\n") +
			_("- Devilkan (ikony do menu i paska narzędzi, obrazki do pomocy).\n") +
			_("- Zły Los (ikony do skojarzonych plików i do menu).\n") +
			_("- Areki, duplex (tłumaczenie anglojęzyczne).\n \n") +
			_("Testerzy: (mniej i bardziej wprawieni użytkownicy programu)\n") +
			_("- Funki27 (pierwszy tester mający spory wpływ na obecne działanie programu\n") +
			_("i najbardziej narzekający na wszystko).\n") +
			_("- Sacredus (chyba pierwszy tłumacz używający trybu tłumacza,\n nieoceniona pomoc przy testowaniu wydajności na słabym komputerze).\n") +
			_("- Kostek00 (prawdziwy wynajdywacz błędów, miał duży wpływ na rozwój spektrum audio \n") +
			_("i głównego pola tekstowego, stworzył ciemny i jasny motyw, a także część ikon).\n") +
			_("- Devilkan (crashhunter, ze względu na swój system i przyzwyczajenia wytropił już wiele crashy,\n") +
			_("pomógł w poprawie działania narzędzi do typesettingu, wymyślił wiele innych usprawnień).\n") +
			_("- MatiasMovie (wyłapał parę crashy i zaproponował różne usprawnienia, pomaga w debugowaniu crashy).\n") +
			_("- mas1904 (wyłapał trochę błędów, pomaga w debugowaniu crashy).\n") +/* i jar do Language Tool*/
			_("- Senami (stworzył nowe motywy, a także wyłapał parę błędów).\n") +
			_("- Atalos (zdebugował wiele crashy).\n \n") +
			_("Podziękowania także dla osób, które używają programu i zgłaszali błędy.\n");
		KaiMessageBox(Credits + Testers, _("Lista osób pomocnych przy tworzeniu programu"));

	}
	else if (id == GLOBAL_HELP || id == GLOBAL_ANSI){
		wxString url = (id == GLOBAL_HELP) ? L"https://bjakja.github.io/index.html" : L"http://animesub.info/forum/viewtopic.php?id=258715";
		OpenInBrowser(url);
	}

}


void KainoteFrame::OnConversion(char form)
{
	TabPanel *tab = GetTab();
	if (tab->Grid->GetSInfo(L"TLMode") == L"Yes"){ return; }
	if (form != ASS){
		tab->Video->RemoveVisual(true, true);
	}
	tab->Video->DisableVisuals(form != ASS);
	tab->Grid->Convert(form);
	tab->ShiftTimes->Contents();
	UpdateToolbar();
	tab->Edit->HideControls();
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

void KainoteFrame::Save(bool showDialog, int tabToSave, bool changeLabel)
{
	TabPanel* atab = (tabToSave < 0) ? GetTab() : Tabs->Page(tabToSave);
	if (!atab)
		return;

	bool nameWasChanged = false;
	if (atab->Grid->originalFormat != atab->Grid->subsFormat
		|| (Options.GetBool(SUBS_AUTONAMING)
		&& atab->SubsName.BeforeLast(L'.') != atab->VideoName.BeforeLast(L'.') && atab->VideoName != L"")
		|| atab->SubsPath == L"" || showDialog)
	{
	repeatOpening:
		wxString extens = _("Plik napisów ");

		if (atab->Grid->subsFormat < SRT){ extens += L"(*.ass)|*.ass"; }
		else if (atab->Grid->subsFormat == SRT){ extens += L"(*.srt)|*.srt"; }
		else{ extens += L"(*.txt, *.sub)|*.txt;*.sub"; };

		wxString path = (atab->VideoPath != L"" && Options.GetBool(SUBS_AUTONAMING)) ? atab->VideoPath : atab->SubsPath;
		wxString name = path.BeforeLast(L'.');
		path = path.BeforeLast(L'\\');

		wxFileDialog saveFileDialog(atab->Video->GetMessageWindowParent(), _("Zapisz plik napisów"),
			path, name, extens, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_OK){
			wxString path = saveFileDialog.GetPath();
			DWORD attributes = ::GetFileAttributesW(path.wc_str());
			if (attributes != -1 && attributes & FILE_ATTRIBUTE_READONLY){
				KaiMessageBox(_("Wybrany plik jest tylko do odczytu,\nproszę zapisać pod inną nazwą lub zmienić atrybuty pliku."), _("Uwaga"), 4L, this);
				goto repeatOpening;
			}

			atab->SubsPath = path;
			wxString ext = (atab->Grid->subsFormat < SRT) ? L"ass" : (atab->Grid->subsFormat == SRT) ? L"srt" : L"txt";
			if (!atab->SubsPath.EndsWith(ext)){ atab->SubsPath << L"." << ext; }
			atab->SubsName = atab->SubsPath.AfterLast(L'\\');
			nameWasChanged = true;
			SetRecent(0, tabToSave);
		}
		else{ return; }
	}
	else{
		DWORD attributes = ::GetFileAttributesW(atab->SubsPath.wc_str());
		if (attributes != -1 && attributes & FILE_ATTRIBUTE_READONLY){
			KaiMessageBox(_("Wybrany plik jest tylko do odczytu,\nproszę zapisać pod inną nazwą lub zmienić atrybuty pliku."), _("Uwaga"), 4L, this);
			goto repeatOpening;
		}
	}
	if (atab->Grid->SwapAssProperties()){ return; }
	atab->Grid->SaveFile(atab->SubsPath);
	atab->Grid->originalFormat = atab->Grid->subsFormat;
	if (changeLabel){
		Toolbar->UpdateId(GLOBAL_SAVE_SUBS, false);
		Menubar->Enable(GLOBAL_SAVE_SUBS, false);
		Label(0, false, tabToSave);
	}
	if (nameWasChanged) {
		Tabs->SaveLastSession();
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
	TabPanel* tab = GetTab();
	if (ext == L"lua" || ext == L"moon"){
		if (Auto->ASSScripts.size() < 1)
			Auto->AddFromSubs();
		Auto->Add(filename);
		return true;
	}
	else if (ext == L"pass" || (ext == L"txt" && filename.EndsWith(L"_keyframes.txt")) || 
		ext == L"stats" || ext == L"log") {
		tab->KeyframesPath = filename;
		tab->Video->OpenKeyframes(filename);
		SetRecent(3);
		return true;
	}

	bool found = false;
	bool nonewtab = true;
	bool changeAudio = true;
	wxString secondFileName;
	bool issubs = (ext == L"ass" || ext == L"txt" || ext == L"sub" || ext == L"srt" || ext == L"ssa");

	if (tab->editor){
		found = FindFile(filename, secondFileName, issubs);
		if (!issubs && found && !fulls && !tab->Video->IsFullScreen()){
			if (tab->SubsPath == secondFileName || 
				KaiMessageBox(wxString::Format(_("Wczytać napisy o nazwie \"%s\"?"), secondFileName.AfterLast(L'\\')),
				_("Potwierdzenie"), wxICON_QUESTION | wxYES_NO, this) == wxNO){
				found = false;
			}
			else{
				ext = secondFileName.AfterLast(L'.');
			}
		}
	}

	if (Options.GetBool(OPEN_SUBS_IN_NEW_TAB) && tab->SubsPath != L"" &&
		!tab->Video->IsFullScreen() && issubs){
		Tabs->AddPage(true);
		tab = Tabs->Page(Tabs->Size() - 1);
		nonewtab = false;
	}

	if (freeze) {
		Tabs->ResetPrompt();
		tab->Freeze();
	}

	if (issubs || found){
		const wxString &fname = (found && !issubs) ? secondFileName : filename;
		if (nonewtab){
			if (SavePrompt(2)){
				if (freeze)
					tab->Thaw();
				return true;
			}
		}
		tab->Video->RemoveVisual(true);

		
		if (!Tabs->LoadSubtitles(tab, fname)){
			if (freeze)
				tab->Thaw();
			return false;
		}
		//remove comparison after every subs load or delete 
		else if (nonewtab && tab->Grid->Comparison){
			SubsGridBase::RemoveComparison();
		}

		SetRecent();
		//set texts on window title and tab
		Label();
		SetSubsResolution(!Options.GetBool(DONT_ASK_FOR_BAD_RESOLUTION));
		//turn on editor
		if (!tab->editor && !fulls && !tab->Video->IsFullScreen()){ HideEditor(); }
	}

	//pass empty string for path to check if paths of subs are valid
	const wxString &fnname = (found && issubs) ? secondFileName : (!issubs)? filename : L"";
	int isload = Tabs->LoadVideo(tab, fnname, -1, true, true, fulls, issubs && !fulls && !tab->Video->IsFullScreen());
	if (!isload){
		if (freeze)
			tab->Thaw();
		return false;
	}

	if (isload == -1 && tab->Video->GetState() != None) {
		//open subs and disable visuals when needed
		bool isgood = tab->Video->OpenSubs((tab->editor) ? OPEN_DUMMY : CLOSE_SUBTITLES, true, true);
		if (!isgood) { KaiMessageBox(_("Nie można otworzyć napisów"), _("Uwaga")); }
		//set color space	
		if (tab->Grid->subsFormat == ASS) {
			tab->Video->SetColorSpace(tab->Grid->GetSInfo(L"YCbCr Matrix"));
		}
	}
	
	//fix to not delete audiocache when using from OpenFiles
	else if (freeze)
		Tabs->GetTab()->Video->DeleteAudioCache();
//done:
	tab->ShiftTimes->Contents();
	UpdateToolbar();
	if (freeze){
		tab->Thaw();
		// do not save options, cause it load many files and save options after it.
		Options.SaveOptions(true, false);
		Tabs->SaveLastSession();
	}
	return true;
}

void KainoteFrame::SetSubsResolution(bool showDialog)
{
	TabPanel *cur = GetTab();
	if (cur->Grid->subsFormat != ASS){
		StatusBar->SetLabelTextColour(5, WINDOW_TEXT);
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
			StatusBar->SetLabelTextColour(5, WINDOW_WARNING_ELEMENTS);
			StatusBar->SetLabelTextColour(7, WINDOW_WARNING_ELEMENTS);
			badResolution = true;
			if (showDialog){
				ShowBadResolutionDialog(vsize, wxSize(x, y));
			}
			return;
		}
	}
	if (badResolution){
		StatusBar->SetLabelTextColour(5, WINDOW_TEXT);
		StatusBar->SetLabelTextColour(7, WINDOW_TEXT);
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
		StatusBar->SetLabelTextColour(5, WINDOW_WARNING_ELEMENTS);
		StatusBar->SetLabelTextColour(7, WINDOW_WARNING_ELEMENTS);
		badResolution = true;
		if (showDialog && cur->Grid->subsFormat == ASS && !cur->SubsPath.empty()){
			ShowBadResolutionDialog(wxSize(w, h), wxSize(x, y));
		}
	}
	else if (badResolution){
		StatusBar->SetLabelTextColour(5, WINDOW_TEXT);
		StatusBar->SetLabelTextColour(7, WINDOW_TEXT);
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
	if (what == 0){ Options.SetTable(SUBS_RECENT_FILES, recs); }
	else if (what == 1){ Options.SetTable(VIDEO_RECENT_FILES, recs); }
	else if (what == 2){ Options.SetTable(AUDIO_RECENT_FILES, recs); }
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
		if (what == 0){ Options.SetTable(SUBS_RECENT_FILES, recs); }
		else if (what == 1){ Options.SetTable(VIDEO_RECENT_FILES, recs); }
		else if (what == 2){ Options.SetTable(AUDIO_RECENT_FILES, recs); }
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
	int toolbarAlignment = Options.GetInt(TOOLBAR_ALIGNMENT);
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
	borders.bottom += Tabs->GetHeight();
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


TabPanel* KainoteFrame::GetTab()
{
	return Tabs->GetPage();
}


void KainoteFrame::Label(int iter/*=0*/, bool video/*=false*/, int wtab/*=-1*/, bool onlyTabs /*= false*/)
{
	TabPanel* atab = (wtab < 0) ? GetTab() : Tabs->Page(wtab);
	if (!atab) {
		KaiLog(wxString::Format(L"cannot get tab %i/%i, label not set", wtab, (int)Tabs->Size()));
		return;
	}
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

	const std::map<idAndType, hdata> &hkeys = Hkeys.GetHotkeysMap();
	for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++){
		if (cur->first.Type != GLOBAL_HOTKEY){ continue; }
		int id = cur->first.id;
		bool emptyAccel = cur->second.Accel == L"";
		if (id >= 5000 && id < 5150){
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
		else if (id >= 5150){
			if (id >= 30100){
				Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnRunScript, this, id);
			}
			entries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}
		else if(id < 5000){
			Bind(wxEVT_COMMAND_MENU_SELECTED, &KainoteFrame::OnUseWindowHotkey, this, id);
			entries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}
		if (entries.size() && !entries[entries.size() - 1].IsOk()){
			entries.pop_back();
		}
	}
	wxAcceleratorTable accel(entries.size(), &entries[0]);
	Tabs->SetAcceleratorTable(accel);

	if (!_all){ return; }
	for (size_t i = 0; i < Tabs->Size(); i++)
	{
		Tabs->Page(i)->SetAccels();
	}
}

void KainoteFrame::OnUseWindowHotkey(wxCommandEvent& event)
{
	int id = event.GetId();
	TabPanel *tab = GetTab();
	if (id < 2000 && tab->Edit->ABox)
		tab->Edit->ABox->OnAccelerator(event);
	else if (id < 3000)
		tab->Video->OnAccelerator(event);
	else if (id < 4000)
		tab->Edit->OnAccelerator(event);
	else if (id < 5000)
		tab->Grid->OnAccelerator(event);
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
	size_t filesSize = files.size();
	for (size_t i = 0; i < filesSize; i++){
		wxString ext = files[i].AfterLast(L'.').Lower();
		if (ext == L"ass" || ext == L"ssa" || ext == L"txt" || ext == L"srt" || ext == L"sub"){
			subs.Add(files[i]);
		}
		else if (ext == L"lua" || ext == L"moon"){
			if (Auto->ASSScripts.size() < 1)
				Auto->AddFromSubs();
			Auto->Add(files[i]);
		}
		else if (ext != L"exe" && ext != L"zip" && ext != L"rar" && ext != L"7z"){
			videos.Add(files[i]);
		}

	}

	Tabs->ResetPrompt();

	if (files.size() == 1){
		OpenFile(files[0], (videos.size() == 1 && Options.GetBool(VIDEO_FULL_SCREEN_ON_START)));
		videos.Clear(); subs.Clear(); files.RemoveAt(0);
		return;
	}
	files.RemoveAt(0, filesSize);
	bool askForRes = !Options.GetBool(DONT_ASK_FOR_BAD_RESOLUTION);
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
		TabPanel *tab = GetTab();
		tab->Video->RemoveVisual();
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
			if (!Tabs->LoadSubtitles(tab, subs[i])){
				break;
			}
			
			if (!tab->editor){ HideEditor(); }
			SetRecent();
			Label();
			SetSubsResolution(askForRes);
		}
		if (i < videosSize){
			if (!Tabs->LoadVideo(tab, videos[i], -1, false, true)){
				break;
			}
		}
		tab->ShiftTimes->Contents();

	}

	Thaw();
	//this order fixes bug of disappearing tabs bar
	Tabs->RefreshBar();
	GetTab()->Show();
	UpdateToolbar();
	Tabs->GetTab()->Video->DeleteAudioCache();
	Options.SaveOptions(true, false);
	Tabs->SaveLastSession();
}

void KainoteFrame::OnPageChange(wxCommandEvent& event)
{
	if (Tabs->Size() < 2){ return; }
	int step = (event.GetId() == GLOBAL_NEXT_TAB) ? Tabs->iter + 1 : Tabs->iter - 1;
	if (step < 0){ step = Tabs->Size() - 1; }
	else if (step >= (int)Tabs->Size()){ step = 0; }
	Tabs->ChangePage(step, true);
	Tabs->Update();
}


void KainoteFrame::OnPageChanged(wxCommandEvent& event)
{
	wxString whiter;
	TabPanel *cur = Tabs->GetPage();
	if (!cur)
		return;
	int iter = cur->Grid->file->Iter();
	if (cur->Grid->IsModified()){
		whiter << iter << L"*";
	}
	wxString name = (!cur->editor) ? cur->VideoName : cur->SubsName;
	SetLabel(whiter + name + L" - " + Options.progname + L" " + wxString(INSTRUCTIONS));
	if (cur->Video->GetState() != None){
		float FPS, AspectRatio;
		int AspectRatiox, AspectRatioY;
		cur->Video->GetFPSAndAspectRatio(&FPS, &AspectRatio, &AspectRatiox, &AspectRatioY);
		SetStatusText(getfloat(FPS) + L" FPS", 4);
		wxString tar;
		tar << AspectRatiox << L" : " << AspectRatioY;
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
	//Saving last focused window on tab and restoring when showed
	if (!event.GetInt()){
		//Todo: make some safe way for it, 
		//sometimes this pointer can be deleted
		if (cur->lastFocusedWindowId) {
			wxWindow *win = FindWindowById(cur->lastFocusedWindowId, cur);
			if (win) {
				win->SetFocus();
			}
		}
		if (!cur->lastFocusedWindowId) {
			if (cur->editor) { cur->Grid->SetFocus(); }
			else { cur->Video->SetFocus(); }
		}
		if (Tabs->iter != Tabs->GetOldSelection() && Options.GetBool(SHIFT_TIMES_CHANGE_VALUES_WITH_TAB)){
			cur->ShiftTimes->RefVals(Tabs->Page(Tabs->GetOldSelection())->ShiftTimes);
		}

		if (Options.GetBool(AUTO_SELECT_LINES_FROM_LAST_TAB)){
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

	if (cur->editor){//Turn on of editor

		cur->MainSizer->Detach(cur->Video);
		cur->VideoEditboxSizer->Prepend(cur->Video, 0, wxEXPAND | wxALIGN_TOP, 0);

		cur->Video->ShowVideoToolbar();
		cur->Video->RemoveVisual(false);
		int panelHeight = cur->Video->GetPanelHeight();
		if (cur->Video->GetState() != None && !cur->Video->IsFullScreen()){
			int sx, sy, vw, vh;
			Options.GetCoords(VIDEO_WINDOW_SIZE, &vw, &vh);
			if (vh < 350){ vh = 350, vw = 500; }
			cur->Video->CalcSize(&sx, &sy, vw, vh);
			cur->Video->SetMinSize(wxSize(sx, sy + panelHeight));
		}
		else{ cur->Video->Hide(); }
		if (Options.GetBool(SHIFT_TIMES_ON)){
			cur->ShiftTimes->Show();
		}
		cur->MainSizer->Layout();
		Label();
		if (cur->Video->GetState() != None){ cur->Video->ChangeVobsub(); }
		SetSubsResolution(false);
		if (cur->Video->IsFullScreen())
			cur->Video->GetFullScreenWindow()->HideToolbar(false);
	}
	else{//Turn off of editor
		cur->Video->RemoveVisual(false, true);
		cur->ShiftTimes->Hide();

		if (!cur->Video->IsShown()){ cur->Video->Show(); }

		cur->VideoEditboxSizer->Detach(cur->Video);

		cur->MainSizer->Add(cur->Video, 1, wxEXPAND | wxALIGN_TOP, 0);

		cur->Video->HideVideoToolbar();

		int panelHeight = cur->Video->GetPanelHeight();
		//can crush after turn on of editor
		if (cur->Video->GetState() != None && !cur->Video->IsFullScreen() && !IsMaximized()){
			int sx, sy, sizex, sizey;
			GetClientSize(&sizex, &sizey);
			sizex -= borders.left + borders.right;
			sizey -= (panelHeight + borders.bottom + borders.top);

			cur->Video->CalcSize(&sx, &sy, sizex, sizey, false, true);

			SetClientSize(sx + borders.left + borders.right, sy + panelHeight + borders.bottom + borders.top);

		}
		cur->Video->SetFocus();

		cur->MainSizer->Layout();

		if (cur->VideoName != L""){ Label(0, true); }
		if (cur->Video->GetState() != None){ cur->Video->ChangeVobsub(true); }
		StatusBar->SetLabelTextColour(5, WINDOW_TEXT);
		SetStatusText(L"", 7);
		if (cur->Video->IsFullScreen())
			cur->Video->GetFullScreenWindow()->HideToolbar(true);

		if (FR && FR->IsShown())
			FR->Show(false);
		if (SL && SL->IsShown())
			SL->Show(false);
		if (StyleStore::HasStore() && StyleStore::Get()->IsShown())
			StyleStore::Get()->Show(false);
	}
	UpdateToolbar();
	if (save){ Options.SetBool(EDITOR_ON, cur->editor); Options.SaveOptions(true, false); }
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

//return cancel operation
bool KainoteFrame::SavePrompt(char mode, int wtab)
{
	TabPanel* atab = (wtab < 0) ? GetTab() : Tabs->Page(wtab);
	if (!atab)
		return false;
	if (atab->Grid->IsModified()){
		wxString ext = (atab->Grid->subsFormat == ASS) ? L"ass" : (atab->Grid->subsFormat == SRT) ? L"srt" : L"txt";
		wxString subsExt;
		wxString subsName = atab->SubsName.BeforeLast(L'.', &subsExt);
		if (subsName.empty())
			subsName = subsExt;
		subsExt.MakeLower();
		wxString subsPath = (ext != subsExt && !(ext == L"txt" && subsExt == L"sub")) ?
			subsName + L"." + ext : atab->SubsName;

		int answer = KaiMessageBox(wxString::Format(_("Zapisać napisy o nazwie \"%s\" przed %s?"),
			subsPath, (mode == 0) ? _("zamknięciem programu") :
			(mode == 1) ? _("zamknięciem zakładki") :
			(mode == 2) ? _("wczytaniem nowych napisów") :
			_("usunięciem napisów")),
			_("Potwierdzenie"), wxYES_NO | wxCANCEL, atab->Video->GetMessageWindowParent());
		if (answer == wxCANCEL){ return true; }
		if (answer == wxYES){
			Save(false, wtab);
		}
	}
	return false;
}

//Disabling items that not needed when editor is off
void KainoteFrame::UpdateToolbar()
{
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

	if (id == GLOBAL_CLOSE_AUDIO){
		if (tab->Edit->ABox){
			RendererVideo *renderer = tab->Video->GetRenderer();
			if (renderer)
				renderer->SetAudioPlayer(NULL);

			tab->Edit->CloseAudio();
			tab->AudioPath.clear();
		}
	}
	else{

		if (!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){ KaiMessageBox(_("Nie można wczytać skrótów klawiszowych audio"), _("Błąd")); return; }
		if (!Options.AudioOpts && !Options.LoadAudioOpts()){ KaiMessageBox(_("Nie można wczytać opcji audio"), _("Błąd")); return; }

		wxString audioPath;
		if (id == GLOBAL_OPEN_AUDIO){
			wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik audio"),
				(tab->VideoPath != L"") ? tab->VideoPath.BeforeLast(L'\\') :
				(videorec.size() > 0) ? videorec[0].BeforeLast(L'\\') : L"", L"",
				_("Pliki audio i wideo") +
				L" (*.wav),(*.w64),(*.flac),(*.ac3),(*.aac),(*.ogg),(*.mp3),(*.mp4),(*.m4a),(*.mkv),(*.avi)|*.wav;*.w64;*.flac;*.ac3;*.aac;*.ogg;*.mp3;*.mp4;*.m4a;*.mkv;*.avi|" +
				_("Wszystkie pliki") + L" |*.*", wxFD_OPEN);
			int result = FileDialog1->ShowModal();
			if (result == wxID_OK){
				audioPath = FileDialog1->GetPath();
			}
			FileDialog1->Destroy();
			if (result == wxID_CANCEL){ return; }
		}
		if (id > 30039 || id == GLOBAL_OPEN_DUMMY_AUDIO){ audioPath = path; }
		if (audioPath.empty()){ audioPath = tab->VideoPath; }
		if (audioPath.empty()){ return; }

		if (tab->Edit->LoadAudio(audioPath, (id == 40000))){
			SetRecent(2);
			tab->AudioPath = audioPath;
		}

	}
}



//uses before menu is shown
void KainoteFrame::OnMenuOpened(MenuEvent& event)
{
	Menu *curMenu = event.GetMenu();
	TabPanel *tab = GetTab();

	bool editor = tab->editor;
	char form = tab->Grid->subsFormat;
	bool tlmode = tab->Grid->hasTLMode;

	if (curMenu == FileMenu || !curMenu)
	{
		if(curMenu)
			AppendRecent();

		for (int i = 0; i < FileMenu->GetMenuItemCount(); i++) {
			MenuItem * fitem = FileMenu->FindItemByPosition(i);
			if (fitem) {
				int fid = fitem->GetId();
				switch (fid) {
				case GLOBAL_SAVE_SUBS:
					fitem->Enable(editor && tab->Grid->IsModified());
					break;
				case GLOBAL_SAVE_TRANSLATION:
					fitem->Enable(editor && tlmode);
					break;
				case GLOBAL_SAVE_ALL_SUBS:
				case GLOBAL_SAVE_SUBS_AS:
				case GLOBAL_REMOVE_SUBS:
					fitem->Enable(editor);
					break;
				default:
					break;
				}
			}
		}
	}
	if (curMenu == VidMenu || !curMenu)
	{
		//video recent
		if (curMenu)
			AppendRecent(1);
		//keyframes recent;
		if(editor && curMenu)
			AppendRecent(3);

		for (int i = 0; i < VidMenu->GetMenuItemCount(); i++) {
			MenuItem * vitem = VidMenu->FindItemByPosition(i);
			if (vitem) {
				bool hasFFMS2 = tab->Video->HasFFMS2();
				bool hasVideoLoaded = (tab->Video->GetState() != None);
				switch (vitem->GetId()) {
				case GLOBAL_GO_TO_PREVIOUS_KEYFRAME:
				case GLOBAL_GO_TO_NEXT_KEYFRAME:
					vitem->Enable(hasFFMS2 && editor);
					break;
				case GLOBAL_SET_AUDIO_FROM_VIDEO:
				case GLOBAL_SET_AUDIO_MARK_FROM_VIDEO:
					vitem->Enable(tab->Edit->ABox != NULL && editor);
					break;
				case GLOBAL_PLAY_PAUSE:
				case GLOBAL_PREVIOUS_FRAME:
				case GLOBAL_NEXT_FRAME:
				case GLOBAL_VIDEO_ZOOM:
					vitem->Enable(hasVideoLoaded);
					break;
				case GLOBAL_VIDEO_INDEXING:
				case GLOBAL_OPEN_VIDEO:
				case GLOBAL_RECENT_VIDEO:
				case GLOBAL_OPEN_DUMMY_VIDEO:
					break;
				default:
					vitem->Enable(hasVideoLoaded && editor);
					break;
				}
				
			
			}
		}
		
	}
	if (curMenu == AudMenu || !curMenu)
	{
		if(editor && curMenu)
			AppendRecent(2);

		for (int i = 0; i < AudMenu->GetMenuItemCount(); i++) {
			MenuItem * aitem = AudMenu->FindItemByPosition(i);
			if (aitem) {
				int aid = aitem->GetId();
				if(aid == GLOBAL_CLOSE_AUDIO)
					aitem->Enable(editor && tab->Edit->ABox != NULL);
				else if(aid == GLOBAL_AUDIO_FROM_VIDEO)
					aitem->Enable(editor && tab->Video->GetState() != None);
				else
					aitem->Enable(editor);
			}
		}
	}
	if (curMenu == AutoMenu)
	{
		if (editor) {
			if (!AutoMenu->FindItemByPosition(0)->IsEnabled()) {
				for (size_t i = 0; i < AutoMenu->GetMenuItemCount(); i++) {
					AutoMenu->FindItemByPosition(i)->Enable(true);
				}
			}

			Auto->BuildMenu(&AutoMenu);
		}
		//disable menuitems when editor not enabled
		else {
			for (size_t i = 0; i < AutoMenu->GetMenuItemCount(); i++) {
				AutoMenu->FindItemByPosition(i)->Enable(false);
			}
		}
	}
	if (curMenu == EditMenu || !curMenu){
		for (int i = 0; i < EditMenu->GetMenuItemCount(); i++) {
			MenuItem * eitem = EditMenu->FindItemByPosition(i);
			if (eitem) {
				int eid = eitem->GetId();
				if (eid == GLOBAL_UNDO) {
					if (!curMenu)
						continue;

					const wxString &undoName = tab->Grid->file->GetUndoName();
					wxString accel = eitem->GetAccel();
					wxString accelName = (accel.empty()) ? L"" : L"\t" + accel;
					accelName.Replace(L"+", L"-");
					if (!undoName.empty()) {
						wxString lowerUndoName = wxString(undoName[0]).Lower() + undoName.Mid(1);
						eitem->label = _("&Cofnij do ") + lowerUndoName + accelName;
					}
					else {
						eitem->label = _("&Cofnij") + accelName;
					}
				}
				else if (eid == GLOBAL_REDO) {
					if (!curMenu)
						continue;

					const wxString &redoName = tab->Grid->file->GetRedoName();
					wxString accel = eitem->GetAccel();
					wxString accelName = (accel.empty()) ? L"" : L"\t" + accel;
					accelName.Replace(L"+", L"-");
					if (!redoName.empty()) {
						wxString lowerRedoName = wxString(redoName[0]).Lower() + redoName.Mid(1);
						eitem->label = _("&Ponów do ") + lowerRedoName + accelName;
					}
					else {
						eitem->label = _("&Ponów") + accelName;
					}

				}
				else if (eid == GLOBAL_HISTORY) {
					eitem->Enable(editor && tab->Grid->file->Iter() > 0);
				}
				// undo last save is disabled on start and activated later
				else if(eid != GLOBAL_UNDO_TO_LAST_SAVE){
					eitem->Enable(editor);
				}
			}
		}
	}
	if (curMenu == SubsMenu || !curMenu) {
		for (int i = 0; i < SubsMenu->GetMenuItemCount(); i++) {
			MenuItem * sitem = SubsMenu->FindItemByPosition(i);
			if (sitem) {
				int id = sitem->GetId();
				if (id == GLOBAL_EDITOR) {
					sitem->Enable(tab->Video->IsDirectShow() || tab->Video->GetState() == None);
				}
				else if(id == ID_CONVERSION){
					Menu* conversionMenu = sitem->GetSubMenu();
					if (conversionMenu) {
						bool enable = editor && !tlmode;
						for (int k = 0; k < conversionMenu->GetMenuItemCount(); k++) {
							MenuItem *citem = conversionMenu->FindItemByPosition(k);
							if (citem) {
								int cid = citem->GetId();
								bool tempEnable = enable;
								if (tempEnable) {
									if (cid == GLOBAL_CONVERT_TO_ASS) { tempEnable = form > ASS; }//conversion to ASS
									else if (cid == GLOBAL_CONVERT_TO_SRT) { tempEnable = form != SRT; }//conversion to SRT
									else if (cid == GLOBAL_CONVERT_TO_MDVD) { tempEnable = form != MDVD; }//conversion to MDVD
									else if (cid == GLOBAL_CONVERT_TO_MPL2) { tempEnable = form != MPL2; }//conversion to MPL2
									else if (cid == GLOBAL_CONVERT_TO_TMP) { tempEnable = form != TMP; }//conversion to TMP
								}
								citem->Enable(tempEnable);
							}
						}
					}
					sitem->Enable(editor);
				}
				else {
					bool enable = editor;
					if (enable && id >= GLOBAL_OPEN_ASS_PROPERTIES && id <= GLOBAL_OPEN_FONT_COLLECTOR) { enable = form < SRT; }
					sitem->Enable(enable);
				}

			}
		}
	}
	if (curMenu == ViewMenu || !curMenu) {
		for (int i = 0; i < ViewMenu->GetMenuItemCount(); i++) {
			MenuItem * viitem = ViewMenu->FindItemByPosition(i);
			if (viitem) {
				bool hasVideoLoaded = (tab->Video->GetState() != None);
				bool isOnAnotherMonitor = tab->Video->IsOnAnotherMonitor();
				switch (viitem->GetId())
				{
				case GLOBAL_VIEW_ALL:
				case GLOBAL_VIEW_VIDEO://subs with video
				case GLOBAL_VIEW_ONLY_VIDEO:
					viitem->Enable(editor && hasVideoLoaded && !isOnAnotherMonitor);
					break;
				case GLOBAL_VIEW_AUDIO:
					viitem->Enable(editor && tab->Edit->ABox != NULL);
					break;
				default://only subs
					viitem->Enable(editor);
					break;
				}
			
			}
		}
	}

	//specjalna poprawka do zapisywania w trybie tłumacza, jeśli jest tlmode, to zawsze ma działać.
	//tab->Edit->TlMode->Enable((editor && form == ASS && tab->SubsPath != L""));

}


void KainoteFrame::OnChangeLine(wxCommandEvent& event)
{

	int idd = event.GetId();
	if (idd < GLOBAL_JOIN_WITH_PREVIOUS){//change of line
		GetTab()->Grid->NextLine((idd == GLOBAL_PREVIOUS_LINE) ? -1 : 1);
	}
	else{//merge two lines
		GetTab()->Grid->OnJoin(event);
	}
}

void KainoteFrame::OnDelete(wxCommandEvent& event)
{
	int idd = event.GetId();
	if (idd == GLOBAL_REMOVE_LINES){
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
			tab->lastFocusedWindowId = win->GetId();
		}
		Menubar->HideMnemonics();
	}
	if (evt.GetActive()){
		sendFocus.Start(50, true);
	}
}

void KainoteFrame::OnExternalSession(int id)
{
	if (id == GLOBAL_LOAD_EXTERNAL_SESSION) {
		wxFileDialog FileDialog(this, _("Wybierz plik sesji"),
			Options.configPath,
			L"", _("Plik sesji (*.kls),|*.kls"),
			wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (FileDialog.ShowModal() == wxID_OK) {
			Tabs->LoadLastSession(false, FileDialog.GetPath());
		}
	}
	else if (id == GLOBAL_SAVE_EXTERNAL_SESSION) {
	repeatOpening:
		wxFileDialog saveFileDialog(this, _("Zapisz plik sesji"),
			Options.configPath, L"", _("Plik sesji (*.kls),|*.kls"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_OK) {
			wxString path = saveFileDialog.GetPath();
			DWORD attributes = ::GetFileAttributesW(path.wc_str());
			if (attributes != -1 && attributes & FILE_ATTRIBUTE_READONLY) {
				KaiMessageBox(_("Wybrany plik jest tylko do odczytu,\nproszę zapisać pod inną nazwą lub zmienić atrybuty pliku."), _("Uwaga"), 4L, this);
				goto repeatOpening;
			}
			Tabs->SaveLastSession(false, false, path);
		}
	}
}


void KainoteFrame::OnAudioSnap(wxCommandEvent& event)
{
	TabPanel *tab = GetTab();
	if (!tab->Edit->ABox || !tab->Video->HasFFMS2()){ return; }
	int id = event.GetId();
	bool snapStartTime = (id == GLOBAL_SNAP_WITH_START);
	int time = (snapStartTime) ? tab->Edit->line->Start.mstime : tab->Edit->line->End.mstime;
	int time2 = (snapStartTime) ? tab->Edit->line->End.mstime : tab->Edit->line->Start.mstime;
	int snaptime = time;
	Provider *FFMS2 = tab->Video->GetFFMS2();
	const wxArrayInt &KeyFrames = FFMS2->GetKeyframes();
	int lastDifferents = MAXINT;
	//wxArrayInt boundaries;
	for (unsigned int i = 0; i < KeyFrames.Count(); i++) {
		int keyMS = KeyFrames[i];
		if (keyMS >= time - 5000 && keyMS < time + 5000) {
			int frameTime = 0;
			int frame = FFMS2->GetFramefromMS(keyMS);
			int prevFrameTime = FFMS2->GetMSfromFrame(frame - 1);
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

	int inactiveType = Options.GetInt(AUDIO_INACTIVE_LINES_DISPLAY_MODE);
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
