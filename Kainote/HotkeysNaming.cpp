//  Copyright (c) 2018 - 2026, Marcin Drob

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

#include "HotkeysNaming.h"
#include "Hotkeys.h"
#include "config.h"

HotkeysNaming::HotkeysNaming()
{
	CreateNamesMap();
}

HotkeysNaming::~HotkeysNaming()
{

}

const std::map<int, wxString> & HotkeysNaming::GetNamesTable()
{
	return names;
}

const wxString & HotkeysNaming::GetName(int id)
{
	const auto & it = names.find(id);
	if (it != names.end())
		return it->second;

	return emptyString;
}

void HotkeysNaming::CreateNamesMap()
{
	names[AUDIO_COMMIT] = _(L"Zatwierdź");
	names[AUDIO_COMMIT_ALT] = _(L"Zatwierdź zastępcze");
	names[AUDIO_PREVIOUS] = _("Poprzednia linijka");
	names[AUDIO_PREVIOUS_ALT] = _(L"Poprzednia linijka zastępcze");
	names[AUDIO_NEXT] = _(L"Następna linijka");
	names[AUDIO_NEXT_ALT] = _(L"Następna linijka zastępcze");
	names[AUDIO_PLAY] = _("Odtwarzaj");
	names[AUDIO_PLAY_ALT] = _(L"Odtwarzaj zastępcze");
	names[AUDIO_PLAY_LINE] = _(L"Odtwarzaj linię");
	names[AUDIO_PLAY_LINE_ALT] = _(L"Odtwarzaj linię zastępcze");
	names[AUDIO_STOP] = _("Zatrzymaj");
	names[AUDIO_GOTO] = _(L"Przejdź do zaznaczenia");
	names[AUDIO_SCROLL_RIGHT] = _(L"Przewiń w lewo");
	names[AUDIO_SCROLL_LEFT] = _(L"Przewiń w prawo");
	names[AUDIO_PLAY_BEFORE_MARK] = _("Odtwarzaj przed znacznikem");
	names[AUDIO_PLAY_AFTER_MARK] = _("Odtwarzaj po znaczniku");
	names[AUDIO_PLAY_500MS_FIRST] = _("Odtwarzaj pierwsze 500ms");
	names[AUDIO_PLAY_500MS_LAST] = _(L"Odtwarzaj końcowe 500ms");
	names[AUDIO_PLAY_500MS_BEFORE] = _("Odtwarzaj 500ms przed");
	names[AUDIO_PLAY_500MS_AFTER] = _("Odtwarzaj 500ms po");
	names[AUDIO_PLAY_TO_END] = _(L"Odtwarzaj do końca");
	names[AUDIO_LEAD_IN] = _(L"Dodaj wstęp");
	names[AUDIO_LEAD_OUT] = _(L"Dodaj zakończenie");
	names[EDITBOX_CHANGE_COLOR_OUTLINE] = _(L"Kolor obwódki");
	names[EDITBOX_CHANGE_COLOR_PRIMARY] = _("Kolor podstawowy");
	names[EDITBOX_CHANGE_COLOR_SECONDARY] = _(L"Kolor zastępczy do karaoke");
	names[EDITBOX_CHANGE_COLOR_SHADOW] = _("Kolor cienia");
	names[EDITBOX_CHANGE_FONT] = _(L"Wybór czcionki");
	names[EDITBOX_CHANGE_STRIKEOUT] = _(L"Przekreślenie");
	names[EDITBOX_CHANGE_UNDERLINE] = _(L"Podkreślenie");
	names[EDITBOX_COMMIT] = _(L"Zatwierdź zmiany");
	names[EDITBOX_COMMIT_GO_NEXT_LINE] = _(L"Zatwierdź zmiany idź do następnej linii");
	names[EDITBOX_FIND_NEXT_DOUBTFUL] = _(L"Następne niepewne");
	names[EDITBOX_FIND_NEXT_UNTRANSLATED] = _(L"Następne nieprzetłumaczone");
	names[EDITBOX_HIDE_ORIGINAL] = _(L"Ukryj oryginał");
	names[EDITBOX_INSERT_BOLD] = _("Wstaw pogrubienie");
	names[EDITBOX_INSERT_ITALIC] = _(L"Wstaw kursywę");
	names[EDITBOX_PASTE_ALL_TO_TRANSLATION] = _("Wklej wszystko");
	names[EDITBOX_PASTE_SELECTION_TO_TRANSLATION] = _("Wklej zaznaczone");
	names[EDITBOX_END_DIFFERENCE] = _(L"Wstaw różnicę końcową");
	names[EDITBOX_SET_DOUBTFUL] = _(L"Ustaw jako niepewne i przejdź dalej");
	names[EDITBOX_SPLIT_LINE] = _(L"Wstaw znak podziału");
	names[EDITBOX_START_DIFFERENCE] = _(L"Wstaw różnicę początkową");
	names[EDITBOX_TAG_BUTTON1] = _(L"Pierwszy przycisk tagów");
	names[EDITBOX_TAG_BUTTON2] = _(L"Drugi przycisk tagów");
	names[EDITBOX_TAG_BUTTON3] = _(L"Trzeci przycisk tagów");
	names[EDITBOX_TAG_BUTTON4] = _(L"Czwarty przycisk tagów");
	names[EDITBOX_TAG_BUTTON5] = _(L"Piąty przycisk tagów");
	names[EDITBOX_TAG_BUTTON6] = _(L"Szósty przycisk tagów");
	names[EDITBOX_TAG_BUTTON7] = _(L"Siódmy przycisk tagów");
	names[EDITBOX_TAG_BUTTON8] = _(L"Ósmy przycisk tagów");
	names[EDITBOX_TAG_BUTTON9] = _(L"Dziewiąty przycisk tagów");
	names[EDITBOX_TAG_BUTTON10] = _(L"Dziesiąty przycisk tagów");
	names[EDITBOX_TAG_BUTTON11] = _(L"Jedenasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON12] = _(L"Dwunasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON13] = _(L"Trzynasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON14] = _(L"Czternasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON15] = _(L"Piętnast przycisk tagów");
	names[EDITBOX_TAG_BUTTON16] = _(L"Szesnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON17] = _(L"Siedemnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON18] = _(L"Osiemnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON19] = _(L"Dziewiętnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON20] = _(L"Dwudziesty przycisk tagów");
	names[GRID_MAKE_CONTINOUS_NEXT_LINE] = _(L"Ustaw czasy jako ciągłe (następna linijka)");
	names[GRID_MAKE_CONTINOUS_PREVIOUS_LINE] = _(L"Ustaw czasy jako ciągłe (poprzednia linijka)");
	names[GRID_COPY_COLUMNS] = _("Kopiuj kolumny");
	names[GRID_DUPLICATE_LINES] = _("Duplikuj linie");
	names[GRID_FILTER_BY_DIALOGUES] = _("Ukryj komentarze");
	names[GRID_FILTER_BY_DOUBTFUL] = _(L"Pokaż niepewne");
	names[GRID_FILTER_BY_NOTHING] = _(L"Wyłącz filtrowanie");
	names[GRID_FILTER_BY_SELECTIONS] = _("Ukryj zaznaczone linie");
	names[GRID_FILTER_BY_STYLES] = _("Ukryj linie ze stylami");
	names[GRID_FILTER_BY_UNTRANSLATED] = _(L"Pokaż nieprzetłumaczone");
	names[GRID_SET_FPS_FROM_VIDEO] = _("Ustaw FPS z wideo");
	names[GRID_FILTER] = _("Filtruj");
	names[GRID_FILTER_AFTER_SUBS_LOAD] = _(L"Filtruj po wczytaniu napisów");
	names[GRID_FILTER_DO_NOT_RESET] = _(L"Nie resetuj wcześniejszego filtrowania");
	names[GRID_FILTER_IGNORE_IN_ACTIONS] = _("Ignoruj filtrowanie przy akcjach");
	names[GRID_FILTER_INVERT] = _(L"Filtrowanie odwrócone");
	names[GRID_HIDE_ACTOR] = _("Ukryj aktora");
	names[GRID_HIDE_CPS] = _(L"Ukryj znaki na sekundę");
	names[GRID_HIDE_END] = _(L"Ukryj czas końcowy");
	names[GRID_HIDE_EFFECT] = _("Ukryj efekt");
	names[GRID_HIDE_LAYER] = _(L"Ukryj warstwę");
	names[GRID_HIDE_MARGINL] = _("Ukryj lewy margines");
	names[GRID_HIDE_MARGINR] = _("Ukryj prawy margines");
	names[GRID_HIDE_MARGINV] = _("Ukryj pionowy margines");
	names[GRID_HIDE_START] = _(L"Ukryj czas początkowy");
	names[GRID_HIDE_STYLE] = _("Ukryj styl");
	names[GRID_TREE_MAKE] = _(L"Stwórz drzewko");
	names[GRID_SELECT_VISIBLE_LINES] = _("Zaznacz wszystkie linie widoczne na wideo");
	names[GRID_HIDE_SELECTED] = _("Ukryj zaznaczone linijki");
	names[GRID_INSERT_AFTER] = _("Wstaw po");
	names[GRID_INSERT_AFTER_VIDEO] = _("Wstaw po z czasem wideo");
	names[GRID_INSERT_AFTER_WITH_VIDEO_FRAME] = _("Wstaw po z czasem klatki wideo");
	names[GRID_INSERT_BEFORE] = _("Wstaw przed");
	names[GRID_INSERT_BEFORE_VIDEO] = _("Wstaw przed z czasem wideo");
	names[GRID_INSERT_BEFORE_WITH_VIDEO_FRAME] = _("Wstaw przed z czasem klatki wideo");
	names[GRID_JOIN_LINES] = _(L"Złącz linijki");
	names[GRID_JOIN_TO_FIRST_LINE] = _(L"Złącz linijki zostaw pierwszą");
	names[GRID_JOIN_TO_LAST_LINE] = _(L"Złącz linijki zostaw ostatnią");
	names[GRID_PASTE_COLUMNS] = _("Wklej kolumny");
	names[GRID_PASTE_TRANSLATION] = _(L"Wklej tekst tłumaczenia");
	names[GRID_SUBS_FROM_MKV] = _("Wczytaj napisy z pliku MKV");
	names[GRID_SWAP_LINES] = _(L"Zamień linie");
	names[GRID_TRANSLATION_DIALOG] = _(L"Okno przesuwania dialogów");
	names[GRID_SET_NEW_FPS] = _("Ustaw nowy FPS");
	names[GRID_SHOW_PREVIEW] = _(L"Pokaż podgląd napisów");
	names[GRID_SPLIT_BY_VIDEO_TIME] = _(L"Podziel linię do czasu wideo");
	names[GRID_SPLIT_BY_FRAME] = _("Podziel linie na klatki");
	names[GRID_SPLIT_BY_CHARS] = _("Podziel linie na znaki");
	names[GRID_SPLIT_BY_WORDS] = _(L"Podziel linie na słowa");
	names[GRID_SPLIT_BY_WRAPS] = _(L"Podziel linie według łamań");
	names[GLOBAL_OPEN_ASS_PROPERTIES] = _(L"Właściwości pliku ASS");
	names[GLOBAL_ABOUT] = _("O programie");
	names[GLOBAL_ANSI] = _(L"Wątek programu na forum AnimeSub.info");
	names[GLOBAL_CONVERT_TO_ASS] = _("Konwertuj do ASS");
	names[GLOBAL_CONVERT_TO_SRT] = _("Konwertuj do SRT");
	names[GLOBAL_CONVERT_TO_MDVD] = _("Konwertuj do MDVD");
	names[GLOBAL_CONVERT_TO_MPL2] = _("Konwertuj do MPL2");
	names[GLOBAL_CONVERT_TO_TMP] = _("Konwertuj do TMP");
	names[GLOBAL_AUDIO_FROM_VIDEO] = _(L"Otwórz audio z wideo");
	names[GLOBAL_AUTOMATION_LOAD_SCRIPT] = _("Wczytaj skrypt");
	names[GLOBAL_AUTOMATION_OPEN_HOTKEYS_WINDOW] = _(L"Otwórz okno mapowania skrótów");
	names[GLOBAL_AUTOMATION_RELOAD_AUTOLOAD] = _(L"Odśwież skrypty autoload");
	names[GLOBAL_SHOW_SHIFT_TIMES] = _(L"Okno zmiany czasów");
	names[GLOBAL_CLOSE_AUDIO] = _("Zamknij audio");
	names[GLOBAL_EDITOR] = _(L"Włącz / Wyłącz edytor");
	names[GLOBAL_FIND_REPLACE] = _(L"Znajdź i zmień");
	names[GLOBAL_OPEN_FONT_COLLECTOR] = _("Kolekcjoner czcionek");
	names[GLOBAL_HELP] = _("Pomoc (niekompletna, ale jednak)");
	names[GLOBAL_HELPERS] = _(L"Lista osób pomocnych przy tworzeniu programu");
	names[GLOBAL_HIDE_TAGS] = _("Ukryj tagi w nawiasach");
	names[GLOBAL_HISTORY] = _("Historia");
	names[GLOBAL_JOIN_WITH_PREVIOUS] = _(L"Scal z poprzednią linijką");
	names[GLOBAL_JOIN_WITH_NEXT] = _(L"Scal z następną linijką");
	names[GLOBAL_OPEN_KEYFRAMES] = _(L"Otwórz klatki kluczowe");
	names[GLOBAL_OPEN_AUTO_SAVE] = _(L"Otwórz autozapis");
	names[GLOBAL_AUTOMATION_LOAD_LAST_SCRIPT] = _("Uruchom ostatnio zaczytany skrypt");
	names[GLOBAL_SORT_ALL_BY_START_TIMES] = _(L"Sortuj wszystko według czasu początkowego");
	names[GLOBAL_SORT_ALL_BY_END_TIMES] = _(L"Sortuj wszystko według czasu końcowego");
	names[GLOBAL_SORT_ALL_BY_STYLE] = _(L"Sortuj wszystko według stylów");
	names[GLOBAL_SORT_ALL_BY_ACTOR] = _(L"Sortuj wszystko według aktora");
	names[GLOBAL_SORT_ALL_BY_EFFECT] = _(L"Sortuj wszystko według efektu");
	names[GLOBAL_SORT_ALL_BY_LAYER] = _(L"Sortuj wszystko według warstwy");
	names[GLOBAL_SORT_SELECTED_BY_START_TIMES] = _(L"Sortuj zaznaczenie według czasu początkowego");
	names[GLOBAL_SORT_SELECTED_BY_END_TIMES] = _(L"Sortuj zaznaczenie według czasu końcowego");
	names[GLOBAL_SORT_SELECTED_BY_STYLE] = _(L"Sortuj zaznaczenie według stylów");
	names[GLOBAL_SORT_SELECTED_BY_ACTOR] = _(L"Sortuj zaznaczenie według aktora");
	names[GLOBAL_SORT_SELECTED_BY_EFFECT] = _(L"Sortuj zaznaczenie według efektu");
	names[GLOBAL_SORT_SELECTED_BY_LAYER] = _(L"Sortuj zaznaczenie według warstwy");
	names[GLOBAL_SHIFT_TIMES] = _(L"Przesuń czasy / uruchom post processor");
	names[GLOBAL_GO_TO_PREVIOUS_KEYFRAME] = _(L"Przejdź do poprzedniej klatki kluczowej");
	names[GLOBAL_GO_TO_NEXT_KEYFRAME] = _(L"Przejdź do następnej klatki kluczowej");
	names[GLOBAL_OPEN_SUBS_RESAMPLE] = _(L"Zmień rozdzielczość napisów");
	names[GLOBAL_UNDO] = _("Cofnij");
	names[GLOBAL_UNDO_TO_LAST_SAVE] = _("Cofnij do ostatniego zapisu");
	names[GLOBAL_VIDEO_INDEXING] = _("Otwieraj wideo przez FFMS2");
	names[GLOBAL_VIDEO_ZOOM] = _(L"Powiększ wideo");
	names[GLOBAL_RESET_VIDEO_ZOOM] = _(L"Wyłącz powiększenie wideo");
	names[GLOBAL_VIEW_ALL] = _("Widok wszystko");
	names[GLOBAL_VIEW_AUDIO] = _("Widok audio i napisy");
	names[GLOBAL_VIEW_SUBS] = _("Widok tylko napisy");
	names[GLOBAL_VIEW_VIDEO] = _("Widok wideo i napisy");
	names[GLOBAL_VIEW_ONLY_VIDEO] = _("Widok tylko wideo");
	names[GLOBAL_ADD_PAGE] = _(L"Otwórz nową zakładkę");
	names[GLOBAL_CLOSE_PAGE] = _(L"Zamknij bieżącą zakładkę");
	names[GLOBAL_STYLE_MANAGER_CLEAN_STYLE] = _(L"Oczyść style pliku ASS");
	names[GLOBAL_MISSPELLS_REPLACER] = _(L"Popraw drobne błędy (eksperymentalne)");
	names[GLOBAL_FIND_NEXT] = _(L"Znajdź następny");
	names[GLOBAL_LOAD_LAST_SESSION] = _(L"Wczytaj ostatnią sesję");
	names[GLOBAL_NEXT_FRAME] = _(L"Klatka w przód");
	names[GLOBAL_NEXT_LINE] = _(L"Następna linijka");
	names[GLOBAL_NEXT_TAB] = _(L"Następna karta");
	names[GLOBAL_OPEN_AUDIO] = _(L"Otwórz audio");
	names[GLOBAL_OPEN_SUBS] = _(L"Otwórz napisy");
	names[GLOBAL_OPEN_VIDEO] = _(L"Otwórz wideo");
	names[GLOBAL_PLAY_PAUSE] = _(L"Odtwórz / Pauza");
	names[GLOBAL_PLAY_ACTUAL_LINE] = _(L"Odtwórz aktywną linijkę");
	names[GLOBAL_PREVIOUS_FRAME] = _(L"Klatka w tył");
	names[GLOBAL_PREVIOUS_LINE] = _("Poprzednia linijka");
	names[GLOBAL_PREVIOUS_TAB] = _("Poprzednia karta");
	names[GLOBAL_REDO] = _(L"Ponów");
	names[GLOBAL_REMOVE_LINES] = _(L"Usuń linijkę");
	names[GLOBAL_REMOVE_SUBS] = _(L"Usuń napisy z edytora");
	names[GLOBAL_REMOVE_TEXT] = _(L"Usuń tekst");
	names[GLOBAL_SAVE_ALL_SUBS] = _("Zapisz wszystkie napisy");
	names[GLOBAL_SAVE_SUBS] = _("Zapisz");
	names[GLOBAL_SAVE_SUBS_AS] = _("Zapisz jako..."); 
	names[GLOBAL_SAVE_TRANSLATION] = _(L"Zapisz tłumaczenie");
	names[GLOBAL_SAVE_WITH_VIDEO_NAME] = _(L"Zapisuj napisy z nazwą wideo");
	names[GLOBAL_SEARCH] = _(L"Znajdź");
	names[GLOBAL_SELECT_FROM_VIDEO] = _(L"Zaznacz linię z czasem wideo");
	names[GLOBAL_OPEN_SELECT_LINES] = _("Zaznacz linijki");
	names[GLOBAL_SET_AUDIO_FROM_VIDEO] = _("Ustaw audio z czasem wideo");
	names[GLOBAL_SET_AUDIO_MARK_FROM_VIDEO] = _("Ustaw znacznik audio z czasem wideo");
	names[GLOBAL_SET_END_TIME] = _(L"Wstaw czas końcowy z wideo");
	names[GLOBAL_SET_START_TIME] = _(L"Wstaw czas początkowy z wideo");
	names[GLOBAL_SETTINGS] = _("Ustawienia");
	names[GLOBAL_SET_VIDEO_AT_START_TIME] = _(L"Przejdź do czasu początkowego linii");
	names[GLOBAL_SET_VIDEO_AT_END_TIME] = _(L"Przejdź do czasu końcowego linii");
	names[GLOBAL_SNAP_WITH_END] = _("Przyklej koniec do klatki kluczowej");
	names[GLOBAL_SNAP_WITH_START] = _("Przyklej start do klatki kluczowej");
	names[GLOBAL_OPEN_SPELLCHECKER] = _(L"Sprawdź poprawność pisowni");
	names[GLOBAL_OPEN_STYLE_MANAGER] = _(L"Menedżer stylów");
	names[VIDEO_COPY_FRAME_TO_CLIPBOARD] = _(L"Kopiuj klatkę do schowka");
	names[VIDEO_DELETE_FILE] = _(L"Usuń plik wideo");
	names[VIDEO_SAVE_FRAME_TO_PNG] = _(L"Zapisz klatkę jako PNG");
	names[VIDEO_5_SECONDS_BACKWARD] = _(L"5 sekund do tyłu");
	names[VIDEO_MINUTE_BACKWARD] = _(L"Minuta do tyłu");
	names[VIDEO_HIDE_PROGRESS_BAR] = _(L"Ukryj / pokaż pasek postępu");
	names[VIDEO_NEXT_CHAPTER] = _(L"Następny rozdział");
	names[VIDEO_NEXT_FILE] = _(L"Następny plik");
	names[VIDEO_PLAY_PAUSE] = _(L"Odtwórz / Pauza");
	names[VIDEO_ASPECT_RATIO] = _(L"Zmień proporcje wideo");
	names[VIDEO_5_SECONDS_FORWARD] = _("5 sekund do przodu");
	names[VIDEO_MINUTE_FORWARD] = _("Minuta do przodu");
	names[VIDEO_PREVIOUS_CHAPTER] = _(L"Poprzedni rozdział");
	names[VIDEO_PREVIOUS_FILE] = _("Poprzedni plik");
	names[VIDEO_COPY_SUBBED_FRAME_TO_CLIPBOARD] = _(L"Kopiuj klatkę z napisami do schowka");
	names[VIDEO_FULL_SCREEN] = _(L"Pełny ekran");
	names[VIDEO_VOLUME_PLUS] = _(L"Dźwięk głośniej");
	names[VIDEO_VOLUME_MINUS] = _(L"Dźwięk ciszej");
	names[VIDEO_SAVE_SUBBED_FRAME_TO_PNG] = _(L"Zapisz klatkę z napisami jako PNG");
	names[VIDEO_STOP] = _("Zatrzymaj");
	
}
