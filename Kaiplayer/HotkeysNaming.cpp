//  Copyright (c) 2018 - 2020, Marcin Drob

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
#include "hotkeys.h"
#include "utils.h"

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
	auto & it = names.find(id);
	if (it != names.end())
		return it->second;

	return emptyString;
}

void HotkeysNaming::CreateNamesMap()
{
	names[GLOBAL_ABOUT] = _("O programie");
	names[GLOBAL_ANSI] = _("Wątek programu na forum AnimeSub.info");
	names[VIDEO_ASPECT_RATIO] = _("Zmień proporcje wideo");
	names[GLOBAL_OPEN_ASS_PROPERTIES] = _("Właściwości pliku ASS");
	names[AUDIO_COMMIT] = _("Zatwierdź");
	names[AUDIO_COMMIT_ALT] = _("Zatwierdź zastępcze");
	names[GLOBAL_AUDIO_FROM_VIDEO] = _("Otwórz audio z wideo");
	names[AUDIO_PREVIOUS] = _("Poprzednia linijka");
	names[AUDIO_PREVIOUS_ALT] = _("Poprzednia linijka zastępcze");
	names[AUDIO_NEXT] = _("Następna linijka");
	names[AUDIO_NEXT_ALT] = _("Następna linijka zastępcze");
	names[AUDIO_PLAY] = _("Odtwarzaj");
	names[AUDIO_PLAY_ALT] = _("Odtwarzaj zastępcze");
	names[AUDIO_PLAY_LINE] = _("Odtwarzaj linię");
	names[AUDIO_PLAY_LINE_ALT] = _("Odtwarzaj linię zastępcze");
	names[AUDIO_STOP] = _("Zatrzymaj");
	names[AUDIO_GOTO] = _("Przejdź do zaznaczenia");
	names[AUDIO_SCROLL_RIGHT] = _("Przewiń w lewo");
	names[AUDIO_SCROLL_LEFT] = _("Przewiń w prawo");
	names[AUDIO_PLAY_BEFORE_MARK] = _("Odtwarzaj przed znacznikem");
	names[AUDIO_PLAY_AFTER_MARK] = _("Odtwarzaj po znaczniku");
	names[AUDIO_PLAY_500MS_FIRST] = _("Odtwarzaj pierwsze 500ms");
	names[AUDIO_PLAY_500MS_LAST] = _("Odtwarzaj końcowe 500ms");
	names[AUDIO_PLAY_500MS_BEFORE] = _("Odtwarzaj 500ms przed");
	names[AUDIO_PLAY_500MS_AFTER] = _("Odtwarzaj 500ms po");
	names[AUDIO_PLAY_TO_END] = _("Odtwarzaj do końca");
	names[AUDIO_LEAD_IN] = _("Dodaj wstęp");
	names[AUDIO_LEAD_OUT] = _("Dodaj zakończenie");
	names[GLOBAL_AUTOMATION_LOAD_SCRIPT] = _("Wczytaj skrypt");
	names[GLOBAL_AUTOMATION_OPEN_HOTKEYS_WINDOW] = _("Otwórz okno mapowania skrótów");
	names[GLOBAL_AUTOMATION_RELOAD_AUTOLOAD] = _("Odśwież skrypty autoload");
	names[GLOBAL_SHOW_SHIFT_TIMES] = _("Okno zmiany czasów");
	names[GLOBAL_CLOSE_AUDIO] = _("Zamknij audio");
	names[GRID_MAKE_CONTINOUS_NEXT_LINE] = _("Ustaw czasy jako ciągłe (następna linijka)");
	names[GRID_MAKE_CONTINOUS_PREVIOUS_LINE] = _("Ustaw czasy jako ciągłe (poprzednia linijka)");
	names[GLOBAL_CONVERT_TO_ASS] = _("Konwertuj do ASS");
	names[GLOBAL_CONVERT_TO_SRT] = _("Konwertuj do SRT");
	names[GLOBAL_CONVERT_TO_MDVD] = _("Konwertuj do MDVD");
	names[GLOBAL_CONVERT_TO_MPL2] = _("Konwertuj do MPL2");
	names[GLOBAL_CONVERT_TO_TMP] = _("Konwertuj do TMP");
	names[GRID_COPY_COLUMNS] = _("Kopiuj kolumny");
	names[VIDEO_DELETE_FILE] = _("Usuń plik wideo");
	names[GRID_DUPLICATE_LINES] = _("Duplikuj linie");
	names[EDITBOX_CHANGE_COLOR_OUTLINE] = _("Kolor obwódki");
	names[EDITBOX_CHANGE_COLOR_PRIMARY] = _("Kolor podstawowy");
	names[EDITBOX_CHANGE_COLOR_SECONDARY] = _("Kolor zastępczy do karaoke");
	names[EDITBOX_CHANGE_COLOR_SHADOW] = _("Kolor cienia");
	names[EDITBOX_CHANGE_FONT] = _("Wybór czcionki");
	names[EDITBOX_CHANGE_STRIKEOUT] = _("Przekreślenie");
	names[EDITBOX_CHANGE_UNDERLINE] = _("Podkreślenie");
	names[EDITBOX_COMMIT] = _("Zatwierdź zmiany");
	names[EDITBOX_COMMIT_GO_NEXT_LINE] = _("Zatwierdź zmiany idź do następnej linii");
	names[EDITBOX_HIDE_ORIGINAL] = _("Ukryj oryginał");
	names[EDITBOX_PASTE_ALL_TO_TRANSLATION] = _("Wklej wszystko");
	names[EDITBOX_PASTE_SELECTION_TO_TRANSLATION] = _("Wklej zaznaczone");
	names[EDITBOX_TAG_BUTTON1] = _("Pierwszy przycisk tagów");
	names[EDITBOX_TAG_BUTTON2] = _("Drugi przycisk tagów");
	names[EDITBOX_TAG_BUTTON3] = _("Trzeci przycisk tagów");
	names[EDITBOX_TAG_BUTTON4] = _("Czwarty przycisk tagów");
	names[EDITBOX_TAG_BUTTON5] = _("Piąty przycisk tagów");
	names[EDITBOX_TAG_BUTTON6] = _("Szósty przycisk tagów");
	names[EDITBOX_TAG_BUTTON7] = _("Siódmy przycisk tagów");
	names[EDITBOX_TAG_BUTTON8] = _("Ósmy przycisk tagów");
	names[EDITBOX_TAG_BUTTON9] = _("Dziewiąty przycisk tagów");
	names[EDITBOX_TAG_BUTTON10] = _("Dziesiąty przycisk tagów");
	names[EDITBOX_TAG_BUTTON11] = _("Jedenasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON12] = _("Dwunasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON13] = _("Trzynasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON14] = _("Czternasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON15] = _("Piętnast przycisk tagów");
	names[EDITBOX_TAG_BUTTON16] = _("Szesnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON17] = _("Siedemnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON18] = _("Osiemnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON19] = _("Dziewiętnasty przycisk tagów");
	names[EDITBOX_TAG_BUTTON20] = _("Dwudziesty przycisk tagów");
	names[GLOBAL_EDITOR] = _("Włącz / Wyłącz edytor");
	names[EDITBOX_END_DIFFERENCE] = _("Wstaw różnicę końcową");
	names[GRID_FILTER_BY_DIALOGUES] = _("Ukryj komentarze");
	names[GRID_FILTER_BY_DOUBTFUL] = _("Pokaż niepewne");
	names[GRID_FILTER_BY_NOTHING] = _("Wyłącz filtrowanie");
	names[GRID_FILTER_BY_SELECTIONS] = _("Ukryj zaznaczone linie");
	names[GRID_FILTER_BY_STYLES] = _("Ukryj linie ze stylami");
	names[GRID_FILTER_BY_UNTRANSLATED] = _("Pokaż nieprzetłumaczone");
	names[EDITBOX_FIND_NEXT_DOUBTFUL] = _("Następne niepewne");
	names[EDITBOX_FIND_NEXT_UNTRANSLATED] = _("Następne nieprzetłumaczone");
	names[GLOBAL_FIND_REPLACE] = _("Znajdź i zmień");
	names[GLOBAL_OPEN_FONT_COLLECTOR] = _("Kolekcjoner czcionek");
	names[GRID_SET_FPS_FROM_VIDEO] = _("Ustaw FPS z wideo");
	names[VIDEO_COPY_FRAME_TO_CLIPBOARD] = _("Kopiuj klatkę do schowka");
	names[VIDEO_SAVE_FRAME_TO_PNG] = _("Zapisz klatkę jako PNG");
	names[GLOBAL_SORT_ALL_BY_START_TIMES] = _("Sortuj wszystko według czasu początkowego");
	names[GLOBAL_SORT_ALL_BY_END_TIMES] = _("Sortuj wszystko według czasu końcowego");
	names[GLOBAL_SORT_ALL_BY_STYLE] = _("Sortuj wszystko według stylów");
	names[GLOBAL_SORT_ALL_BY_ACTOR] = _("Sortuj wszystko według aktora");
	names[GLOBAL_SORT_ALL_BY_EFFECT] = _("Sortuj wszystko według efektu");
	names[GLOBAL_SORT_ALL_BY_LAYER] = _("Sortuj wszystko według warstwy");
	names[GLOBAL_SORT_SELECTED_BY_START_TIMES] = _("Sortuj zaznaczenie według czasu początkowego");
	names[GLOBAL_SORT_SELECTED_BY_END_TIMES] = _("Sortuj zaznaczenie według czasu końcowego");
	names[GLOBAL_SORT_SELECTED_BY_STYLE] = _("Sortuj zaznaczenie według stylów");
	names[GLOBAL_SORT_SELECTED_BY_ACTOR] = _("Sortuj zaznaczenie według aktora");
	names[GLOBAL_SORT_SELECTED_BY_EFFECT] = _("Sortuj zaznaczenie według efektu");
	names[GLOBAL_SORT_SELECTED_BY_LAYER] = _("Sortuj zaznaczenie według warstwy");
	names[GLOBAL_SHIFT_TIMES] = _("Przesuń czasy / uruchom post processor");
	names[GLOBAL_GO_TO_PREVIOUS_KEYFRAME] = _("Przejdź do poprzedniej klatki kluczowej");
	names[GLOBAL_GO_TO_NEXT_KEYFRAME] = _("Przejdź do następnej klatki kluczowej");
	names[GRID_FILTER] = _("Filtruj");
	names[GRID_FILTER_AFTER_SUBS_LOAD] = _("Filtruj po wczytaniu napisów");
	names[GRID_FILTER_DO_NOT_RESET] = _("Nie resetuj wcześniejszego filtrowania");
	names[GRID_FILTER_IGNORE_IN_ACTIONS] = _("Ignoruj filtrowanie przy akcjach");
	names[GRID_FILTER_INVERT] = _("Filtrowanie odwrócone");
	names[GRID_HIDE_ACTOR] = _("Ukryj aktora");
	names[GRID_HIDE_CPS] = _("Ukryj znaki na sekundę");
	names[GRID_HIDE_END] = _("Ukryj czas końcowy");
	names[GRID_HIDE_EFFECT] = _("Ukryj efekt");
	names[GRID_HIDE_LAYER] = _("Ukryj warstwę");
	names[GRID_HIDE_MARGINL] = _("Ukryj lewy margines");
	names[GRID_HIDE_MARGINR] = _("Ukryj prawy margines");
	names[GRID_HIDE_MARGINV] = _("Ukryj pionowy margines");
	names[GRID_HIDE_START] = _("Ukryj czas początkowy");
	names[GRID_HIDE_STYLE] = _("Ukryj styl");
	names[GRID_TREE_MAKE] = _("Stwórz drzewko");
	names[GLOBAL_HELP] = _("Pomoc (niekompletna, ale jednak)");
	names[GLOBAL_HELPERS] = _("Lista osób pomocnych przy tworzeniu programu");
	names[VIDEO_HIDE_PROGRESS_BAR] = _("Ukryj / pokaż pasek postępu");
	names[GRID_HIDE_SELECTED] = _("Ukryj zaznaczone linijki");
	names[GLOBAL_HIDE_TAGS] = _("Ukryj tagi w nawiasach");
	names[GLOBAL_HISTORY] = _("Historia");
	names[GRID_INSERT_AFTER] = _("Wstaw po");
	names[GRID_INSERT_AFTER_VIDEO] = _("Wstaw po z czasem wideo");
	names[GRID_INSERT_AFTER_WITH_VIDEO_FRAME] = _("Wstaw po z czasem klatki wideo");
	names[GRID_INSERT_BEFORE] = _("Wstaw przed");
	names[GRID_INSERT_BEFORE_VIDEO] = _("Wstaw przed z czasem wideo");
	names[GRID_INSERT_BEFORE_WITH_VIDEO_FRAME] = _("Wstaw przed z czasem klatki wideo");
	names[GRID_JOIN_LINES] = _("Złącz linijki");
	names[GRID_JOIN_TO_FIRST_LINE] = _("Złącz linijki zostaw pierwszą");
	names[GRID_JOIN_TO_LAST_LINE] = _("Złącz linijki zostaw ostatnią");
	names[GLOBAL_JOIN_WITH_PREVIOUS] = _("Scal z poprzednią linijką");
	names[GLOBAL_JOIN_WITH_NEXT] = _("Scal z następną linijką");
	names[GLOBAL_OPEN_KEYFRAMES] = _("Otwórz klatki kluczowe");
	names[GLOBAL_OPEN_AUTO_SAVE] = _("Otwórz autozapis");
	names[GLOBAL_AUTOMATION_LOAD_LAST_SCRIPT] = _("Uruchom ostatnio zaczytany skrypt");
	names[VIDEO_5_SECONDS_BACKWARD] = _("5 sekund do tyłu");
	names[VIDEO_MINUTE_BACKWARD] = _("Minuta do tyłu");
	names[GRID_SET_NEW_FPS] = _("Ustaw nowy FPS");
	names[VIDEO_NEXT_CHAPTER] = _("Następny rozdział");
	names[GLOBAL_NEXT_FRAME] = _("Klatka w przód");
	names[GLOBAL_NEXT_LINE] = _("Następna linijka");
	names[GLOBAL_NEXT_TAB] = _("Następna karta");
	names[VIDEO_NEXT_FILE] = _("Następny plik");
	names[GLOBAL_OPEN_AUDIO] = _("Otwórz audio");
	names[GLOBAL_OPEN_SUBS] = _("Otwórz napisy");
	names[GLOBAL_OPEN_VIDEO] = _("Otwórz wideo");
	names[GRID_PASTE_COLUMNS] = _("Wklej kolumny");
	names[GRID_PASTE_TRANSLATION] = _("Wklej tekst tłumaczenia");
	names[GLOBAL_PLAY_ACTUAL_LINE] = _("Odtwórz aktywną linijkę");
	names[VIDEO_PLAY_PAUSE] = _("Odtwórz / Pauza");
	names[GLOBAL_PLAY_PAUSE] = _("Odtwórz / Pauza");
	names[VIDEO_5_SECONDS_FORWARD] = _("5 sekund do przodu");
	names[VIDEO_MINUTE_FORWARD] = _("Minuta do przodu");
	names[VIDEO_PREVIOUS_CHAPTER] = _("Poprzedni rozdział");
	names[GLOBAL_PREVIOUS_FRAME] = _("Klatka w tył");
	names[GLOBAL_PREVIOUS_LINE] = _("Poprzednia linijka");
	names[GLOBAL_PREVIOUS_TAB] = _("Poprzednia karta");
	names[VIDEO_PREVIOUS_FILE] = _("Poprzedni plik");
	names[EDITBOX_INSERT_BOLD] = _("Wstaw pogrubienie");
	names[EDITBOX_INSERT_ITALIC] = _("Wstaw kursywę");
	names[GLOBAL_REDO] = _("Ponów");
	names[GLOBAL_REMOVE_LINES] = _("Usuń linijkę");
	names[GLOBAL_REMOVE_SUBS] = _("Usuń napisy z edytora");
	names[GLOBAL_REMOVE_TEXT] = _("Usuń tekst");
	names[GLOBAL_SAVE_ALL_SUBS] = _("Zapisz wszystkie napisy");
	names[GLOBAL_SAVE_SUBS] = _("Zapisz");
	names[GLOBAL_SAVE_SUBS_AS] = _("Zapisz jako..."); 
	names[GLOBAL_SAVE_TRANSLATION] = _("Zapisz tłumaczenie");
	names[GLOBAL_SAVE_WITH_VIDEO_NAME] = _("Zapisuj napisy z nazwą wideo");
	names[GLOBAL_SEARCH] = _("Znajdź");
	names[GLOBAL_SELECT_FROM_VIDEO] = _("Zaznacz linię z czasem wideo");
	names[GLOBAL_OPEN_SELECT_LINES] = _("Zaznacz linijki");
	names[GLOBAL_SET_AUDIO_FROM_VIDEO] = _("Ustaw audio z czasem wideo");
	names[GLOBAL_SET_AUDIO_MARK_FROM_VIDEO] = _("Ustaw znacznik audio z czasem wideo");
	names[EDITBOX_SET_DOUBTFUL] = _("Ustaw jako niepewne i przejdź dalej");
	names[GLOBAL_SET_END_TIME] = _("Wstaw czas końcowy z wideo");
	names[GLOBAL_SET_START_TIME] = _("Wstaw czas początkowy z wideo"); 
	names[GLOBAL_SETTINGS] = _("Ustawienia");
	names[GLOBAL_SET_VIDEO_AT_START_TIME] = _("Przejdź do czasu początkowego linii");
	names[GLOBAL_SET_VIDEO_AT_END_TIME] = _("Przejdź do czasu końcowego linii");
	names[GRID_SHOW_PREVIEW] = _("Pokaż podgląd napisów");
	names[GLOBAL_SNAP_WITH_END] = _("Przyklej koniec do klatki kluczowej");
	names[GLOBAL_SNAP_WITH_START] = _("Przyklej start do klatki kluczowej");
	names[GLOBAL_OPEN_SPELLCHECKER] = _("Sprawdź poprawność pisowni");
	names[EDITBOX_SPLIT_LINE] = _("Wstaw znak podziału");
	names[EDITBOX_START_DIFFERENCE] = _("Wstaw różnicę początkową");
	names[VIDEO_STOP] = _("Zatrzymaj");
	names[GLOBAL_OPEN_STYLE_MANAGER] = _("Menedżer stylów");
	names[VIDEO_COPY_SUBBED_FRAME_TO_CLIPBOARD] = _("Kopiuj klatkę z napisami do schowka");
	names[VIDEO_SAVE_SUBBED_FRAME_TO_PNG] = _("Zapisz klatkę z napisami jako PNG");
	names[GLOBAL_OPEN_SUBS_RESAMPLE] = _("Zmień rozdzielczość napisów");
	names[GRID_SUBS_FROM_MKV] = _("Wczytaj napisy z pliku MKV");
	names[GRID_SWAP_LINES] = _("Zamień linie");
	names[GRID_TRANSLATION_DIALOG] = _("Okno przesuwania dialogów");
	names[GLOBAL_UNDO] = _("Cofnij");
	names[GLOBAL_UNDO_TO_LAST_SAVE] = _("Cofnij do ostatniego zapisu");
	names[GLOBAL_VIDEO_INDEXING] = _("Otwieraj wideo przez FFMS2");
	names[GLOBAL_VIDEO_ZOOM] = _("Powiększ wideo");
	names[GLOBAL_VIEW_ALL] = _("Widok wszystko");
	names[GLOBAL_VIEW_AUDIO] = _("Widok audio i napisy");
	names[GLOBAL_VIEW_SUBS] = _("Widok tylko napisy"); 
	names[GLOBAL_VIEW_VIDEO] = _("Widok wideo i napisy");
	names[GLOBAL_VIEW_ONLY_VIDEO] = _("Widok tylko wideo");
	names[VIDEO_VOLUME_PLUS] = _("Dźwięk głośniej"); 
	names[VIDEO_VOLUME_MINUS] = _("Dźwięk ciszej"); 
	names[GLOBAL_ADD_PAGE] = _("Otwórz nową zakładkę"); 
	names[GLOBAL_CLOSE_PAGE] = _("Zamknij bieżącą zakładkę");
	names[GLOBAL_STYLE_MANAGER_CLEAN_STYLE] = _("Oczyść style pliku ASS");
	names[GLOBAL_MISSPELLS_REPLACER] = _("Popraw drobne błędy (eksperymentalne)");
	names[VIDEO_FULL_SCREEN] = _("Pełny ekran");
}
