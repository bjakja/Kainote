//  Copyright (c) 2018, Marcin Drob

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
	names[About] = _("O programie");
	names[ANSI] = _("Wątek programu na forum AnimeSub.info");
	names[AspectRatio] = _("Zmień proporcje wideo");
	names[ASSProperties] = _("Właściwości ASS");
	names[AudioCommit] = _("Zatwierdź");
	names[AudioCommitAlt] = _("Zatwierdź zastępcze");
	names[AudioFromVideo] = _("Otwórz audio z wideo");
	names[AudioPrevious] = _("Poprzednia linijka");
	names[AudioPreviousAlt] = _("Poprzednia linijka zastępcze");
	names[AudioNext] = _("Następna linijka");
	names[AudioNextAlt] = _("Następna linijka zastępcze");
	names[AudioPlay] = _("Odtwarzaj");
	names[AudioPlayAlt] = _("Odtwarzaj zastępcze");
	names[AudioPlayLine] = _("Odtwarzaj linię");
	names[AudioPlayLineAlt] = _("Odtwarzaj linię zastępcze");
	names[AudioStop] = _("Zatrzymaj");
	names[AudioGoto] = _("Przejdź do zaznaczenia");
	names[AudioScrollRight] = _("Przewiń w lewo");
	names[AudioScrollLeft] = _("Przewiń w prawo");
	names[AudioPlayBeforeMark] = _("Odtwarzaj przed znacznikem");
	names[AudioPlayAfterMark] = _("Odtwarzaj po znaczniku");
	names[AudioPlay500MSFirst] = _("Odtwarzaj pierwsze 500ms");
	names[AudioPlay500MSLast] = _("Odtwarzaj końcowe 500ms");
	names[AudioPlay500MSBefore] = _("Odtwarzaj 500ms przed");
	names[AudioPlay500MSAfter] = _("Odtwarzaj 500ms po");
	names[AudioPlayToEnd] = _("Odtwarzaj do końca");
	names[AudioLeadin] = _("Dodaj wstęp");
	names[AudioLeadout] = _("Dodaj zakończenie");
	names[AutoLoadScript] = _("Wczytaj skrypt");
	names[AUTOMATION_OPEN_HOTKEYS_WINDOW] = _("Otwórz okno mapowania skrótów");
	names[AutoReloadAutoload] = _("Odśwież skrypty autoload");
	names[ChangeTime] = _("Okno zmiany czasów");
	names[CloseAudio] = _("Zamknij audio");
	names[ContinousNext] = _("Ustaw czasy jako ciągłe (następna linijka)");
	names[ContinousPrevious] = _("Ustaw czasy jako ciągłe (poprzednia linijka)");
	names[ConvertToASS] = _("Konwertuj do ASS");
	names[ConvertToSRT] = _("Konwertuj do SRT");
	names[ConvertToMDVD] = _("Konwertuj do MDVD");
	names[ConvertToMPL2] = _("Konwertuj do MPL2");
	names[ConvertToTMP] = _("Konwertuj do TMP");
	names[CopyCollumns] = _("Kopiuj kolumny");
	names[DeleteVideo] = _("Usuń plik wideo");
	names[Duplicate] = _("Duplikuj linie");
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
	names[Editor] = _("Włącz / Wyłącz edytor");
	names[EndDifference] = _("Wstaw różnicę końcową");
	names[FilterByDialogues] = _("Ukryj komentarze");
	names[FilterByDoubtful] = _("Pokaż niepewne");
	names[FilterByNothing] = _("Wyłącz filtrowanie");
	names[FilterBySelections] = _("Ukryj zaznaczone linie");
	names[FilterByStyles] = _("Ukryj linie ze stylami");
	names[FilterByUntranslated] = _("Pokaż nieprzetłumaczone");
	names[FindNextDoubtful] = _("Następne niepewne");
	names[FindNextUntranslated] = _("Następne nieprzetłumaczone");
	names[FindReplaceDialog] = _("Znajdź i zmień");
	names[FontCollectorID] = _("Kolekcjoner czcionek");
	names[FPSFromVideo] = _("Ustaw FPS z wideo");
	names[FrameToClipboard] = _("Kopiuj klatkę do schowka");
	names[FrameToPNG] = _("Zapisz klatkę jako PNG");
	names[GLOBAL_SORT_ALL_BY_START_TIMES] = _("Sortuj wszystko według czasu początkowego");
	names[GLOBAL_SORT_ALL_BY_END_TIMES] = _("Sortuj wszystko według czasu końcowego");
	names[GLOBAL_SORT_ALL_BY_STYLE] = _("Sortuj wszystko według stylów");
	names[GLOBAL_SORT_ALL_BY_ACTOR] = _("Sortuj wszystko według aktora");
	names[GLOBAL_SORT_ALL_BY_EFFECT] = _("Sortuj wszystko według efektu");
	names[GLOBAL_SORT_ALL_BY_LAYER] = _("Sortuj wszystko według warstwa");
	names[GLOBAL_SORT_SELECTED_BY_START_TIMES] = _("Sortuj wszystko według czasu początkowego");
	names[GLOBAL_SORT_SELECTED_BY_END_TIMES] = _("Sortuj wszystko według czasu końcowego");
	names[GLOBAL_SORT_SELECTED_BY_STYLE] = _("Sortuj wszystko według stylów");
	names[GLOBAL_SORT_SELECTED_BY_ACTOR] = _("Sortuj wszystko według aktora");
	names[GLOBAL_SORT_SELECTED_BY_EFFECT] = _("Sortuj wszystko według efektu");
	names[GLOBAL_SORT_SELECTED_BY_LAYER] = _("Sortuj wszystko według warstwa");
	names[GLOBAL_SHIFT_TIMES] = _("Przesuń czasy / uruchom post processor");
	names[GoToPrewKeyframe] = _("Przejdź do poprzedniej klatki kluczowej");
	names[GoToNextKeyframe] = _("Przejdź do następnej klatki kluczowej");
	names[GRID_FILTER] = _("Filtruj");
	names[GRID_FILTER_AFTER_SUBS_LOAD] = _("Filtruj po wczytaniu napisów");
	names[GRID_FILTER_DO_NOT_RESET] = _("Nie resetuj wcześniejszego filtrowania");
	names[GRID_FILTER_IGNORE_IN_ACTIONS] = _("Ignoruj filtrowanie przy akcjach");
	names[GRID_FILTER_INVERTED] = _("Filtrowanie odwrócone");
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
	names[Help] = _("Pomoc (niekompletna, ale jednak)");
	names[Helpers] = _("Lista osób pomocnych przy tworzeniu programu");
	names[HideProgressBar] = _("Ukryj / pokaż pasek postępu");
	names[HideSelected] = _("Ukryj zaznaczone linijki");
	names[HideTags] = _("Ukryj tagi w nawiasach");
	names[History] = _("Historia");
	names[InsertAfter] = _("Wstaw po");
	names[InsertAfterVideo] = _("Wstaw po z czasem wideo");
	names[InsertAfterWithVideoFrame] = _("Wstaw po z czasem klatki wideo");
	names[InsertBefore] = _("Wstaw przed");
	names[InsertBeforeVideo] = _("Wstaw przed z czasem wideo");
	names[InsertBeforeWithVideoFrame] = _("Wstaw przed z czasem klatki wideo");
	names[Join] = _("Złącz linijki");
	names[JoinToFirst] = _("Złącz linijki zostaw pierwszą");
	names[JoinToLast] = _("Złącz linijki zostaw ostatnią");
	names[JoinWithPrevious] = _("Scal z poprzednią linijką");
	names[JoinWithNext] = _("Scal z następną linijką");
	names[LoadLastScript] = _("Uruchom ostatnio zaczytany skrypt");
	names[Minus5Second] = _("5 sekund do tyłu");
	names[Minus5SecondG] = _("Wideo minus 5 sekund");
	names[MinusMinute] = _("Minuta do tyłu");
	names[NewFPS] = _("Ustaw nowy FPS");
	names[NextChapter] = _("Następny rozdział");
	names[NextFrame] = _("Klatka w przód");
	names[NextLine] = _("Następna linijka");
	names[NextTab] = _("Następna karta");
	names[NextVideo] = _("Następny plik");
	names[OpenAudio] = _("Otwórz audio");
	names[OpenSubs] = _("Otwórz napisy");
	names[OpenVideo] = _("Otwórz wideo");
	names[PasteCollumns] = _("Wklej kolumny");
	names[PasteTranslation] = _("Wklej tekst tłumaczenia");
	names[PlayActualLine] = _("Odtwórz aktywną linijkę");
	names[PlayPause] = _("Odtwórz / Pauza");
	names[PlayPauseG] = _("Odtwórz / Pauza");
	names[Plus5SecondG] = _("Wideo plus 5 sekund");
	names[Plus5Second] = _("5 sekund do przodu");
	names[PlusMinute] = _("Minuta do przodu");
	names[PreviousChapter] = _("Poprzedni rozdział");
	names[PreviousFrame] = _("Klatka w tył");
	names[PreviousLine] = _("Poprzednia linijka");
	names[PreviousTab] = _("Poprzednia karta");
	names[PreviousVideo] = _("Poprzedni plik");
	names[PutBold] = _("Wstaw pogrubienie");
	names[PutItalic] = _("Wstaw kursywę");
	names[Redo] = _("Ponów");
	names[Remove] = _("Usuń linijkę");
	names[RemoveSubs] = _("Usuń napisy z edytora");
	names[RemoveText] = _("Usuń tekst");
	names[SaveAllSubs] = _("Zapisz wszystkie napisy");
	names[SaveSubs] = _("Zapisz");
	names[SaveSubsAs] = _("Zapisz jako..."); 
	names[SaveTranslation] = _("Zapisz tłumaczenie");
	names[SaveWithVideoName] = _("Zapisuj napisy z nazwą wideo");
	names[Search] = _("Znajdź");
	names[SelectFromVideo] = _("Zaznacz linię z czasem wideo");
	names[SelectLinesDialog] = _("Zaznacz linijki");
	names[SetAudioFromVideo] = _("Ustaw audio z czasem wideo");
	names[SetAudioMarkFromVideo] = _("Ustaw znacznik audio z czasem wideo");
	names[SetDoubtful] = _("Ustaw jako niepewne i przejdź dalej");
	names[SetEndTime] = _("Wstaw czas końcowy z wideo");
	names[SetStartTime] = _("Wstaw czas początkowy z wideo"); 
	names[Settings] = _("Ustawienia");
	names[SetVideoAtStart] = _("Przejdź do czasu początkowego linii");
	names[SetVideoAtEnd] = _("Przejdź do czasu końcowego linii");
	names[ShowPreview] = _("Pokaż podgląd napisów");
	names[SnapWithEnd] = _("Przyklej koniec do klatki kluczowej");
	names[SnapWithStart] = _("Przyklej start do klatki kluczowej");
	names[SpellcheckerDialog] = _("Sprawdź poprawność pisowni");
	names[SplitLine] = _("Wstaw znak podziału");
	names[StartDifference] = _("Wstaw różnicę początkową");
	names[StopPlayback] = _("Zatrzymaj");
	names[StyleManager] = _("Menedżer stylów");
	names[SubbedFrameToClipboard] = _("Kopiuj klatkę z napisami do schowka");
	names[SubbedFrameToPNG] = _("Zapisz klatkę z napisami jako PNG");
	names[SubsResample] = _("Zmień rozdzielczość napisów");
	names[SubsFromMKV] = _("Wczytaj napisy z pliku MKV");
	names[Swap] = _("Zamień");
	names[TranslationDialog] = _("Okno przesuwania dialogów");
	names[Undo] = _("Cofnij");
	names[UndoToLastSave] = _("Cofnij do ostatniego zapisu");
	names[VideoIndexing] = _("Otwieraj wideo przez FFMS2");
	names[VideoZoom] = _("Powiększ wideo");
	names[ViewAll] = _("Widok wszystko");
	names[ViewAudio] = _("Widok audio i napisy");
	names[ViewSubs] = _("Widok tylko napisy");
	names[ViewVideo] = _("Widok wideo i napisy");
	names[VolumePlus] = _("Dźwięk głośniej"); 
	names[VolumeMinus] = _("Dźwięk ciszej"); 
	
}
