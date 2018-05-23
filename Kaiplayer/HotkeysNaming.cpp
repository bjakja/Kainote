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
	names[ANSI] = _("W¹tek programu na forum AnimeSub.info");
	names[AspectRatio] = _("Zmieñ proporcje wideo");
	names[ASSProperties] = _("W³aœciwoœci ASS");
	names[AudioCommit] = _("ZatwierdŸ");
	names[AudioCommitAlt] = _("ZatwierdŸ zastêpcze");
	names[AudioFromVideo] = _("Otwórz audio z wideo");
	names[AudioPrevious] = _("Poprzednia linijka");
	names[AudioPreviousAlt] = _("Poprzednia linijka zastêpcze");
	names[AudioNext] = _("Nastêpna linijka");
	names[AudioNextAlt] = _("Nastêpna linijka zastêpcze");
	names[AudioPlay] = _("Odtwarzaj");
	names[AudioPlayAlt] = _("Odtwarzaj zastêpcze");
	names[AudioPlayLine] = _("Odtwarzaj liniê");
	names[AudioPlayLineAlt] = _("Odtwarzaj liniê zastêpcze");
	names[AudioStop] = _("Zatrzymaj");
	names[AudioGoto] = _("PrzejdŸ do zaznaczenia");
	names[AudioScrollRight] = _("Przewiñ w lewo");
	names[AudioScrollLeft] = _("Przewiñ w prawo");
	names[AudioPlayBeforeMark] = _("Odtwarzaj przed znacznikem");
	names[AudioPlayAfterMark] = _("Odtwarzaj po znaczniku");
	names[AudioPlay500MSFirst] = _("Odtwarzaj pierwsze 500ms");
	names[AudioPlay500MSLast] = _("Odtwarzaj koñcowe 500ms");
	names[AudioPlay500MSBefore] = _("Odtwarzaj 500ms przed");
	names[AudioPlay500MSAfter] = _("Odtwarzaj 500ms po");
	names[AudioPlayToEnd] = _("Odtwarzaj do koñca");
	names[AudioLeadin] = _("Dodaj wstêp");
	names[AudioLeadout] = _("Dodaj zakoñczenie");
	names[AutoLoadScript] = _("Wczytaj skrypt");
	names[AutoReloadAutoload] = _("Odœwie¿ skrypty autoload");
	names[ChangeTime] = _("Okno zmiany czasów");
	names[CloseAudio] = _("Zamknij audio");
	names[ContinousNext] = _("Ustaw czasy jako ci¹g³e (nastêpna linijka)");
	names[ContinousPrevious] = _("Ustaw czasy jako ci¹g³e (poprzednia linijka)");
	names[ConvertToASS] = _("Konwertuj do ASS");
	names[ConvertToSRT] = _("Konwertuj do SRT");
	names[ConvertToMDVD] = _("Konwertuj do MDVD");
	names[ConvertToMPL2] = _("Konwertuj do MPL2");
	names[ConvertToTMP] = _("Konwertuj do TMP");
	names[CopyCollumns] = _("Kopiuj kolumny");
	names[DeleteVideo] = _("Usuñ plik wideo");
	names[Duplicate] = _("Duplikuj linie");
	names[EDITBOX_CHANGE_COLOR_OUTLINE] = _("Kolor obwódki");
	names[EDITBOX_CHANGE_COLOR_PRIMARY] = _("Kolor podstawowy");
	names[EDITBOX_CHANGE_COLOR_SECONDARY] = _("Kolor zastêpczy do karaoke");
	names[EDITBOX_CHANGE_COLOR_SHADOW] = _("Kolor cienia");
	names[EDITBOX_CHANGE_UNDERLINE] = _("Wybór czcionki");
	names[EDITBOX_COMMIT] = _("ZatwierdŸ zmiany");
	names[EDITBOX_COMMIT_GO_NEXT_LINE] = _("ZatwierdŸ zmiany idŸ do nastêpnej linii");
	names[EDITBOX_HIDE_ORIGINAL] = _("Ukryj orygina³");
	names[EDITBOX_PASTE_ALL_TO_TRANSLATION] = _("Wklej wszystko");
	names[EDITBOX_PASTE_SELECTION_TO_TRANSLATION] = _("Wklej zaznaczone");
	names[EDITBOX_TAG_BUTTON1] = _("Pierwszy przycisk tagów");
	names[EDITBOX_TAG_BUTTON2] = _("Drugi przycisk tagów");
	names[EDITBOX_TAG_BUTTON3] = _("Trzeci przycisk tagów");
	names[EDITBOX_TAG_BUTTON4] = _("Czwarty przycisk tagów");
	names[EDITBOX_TAG_BUTTON5] = _("Pi¹ty przycisk tagów");
	names[EDITBOX_TAG_BUTTON6] = _("Szósty przycisk tagów");
	names[EDITBOX_TAG_BUTTON7] = _("Siódmy przycisk tagów");
	names[EDITBOX_TAG_BUTTON8] = _("Ósmy przycisk tagów");
	names[EDITBOX_TAG_BUTTON9] = _("Dziewi¹ty przycisk tagów");
	names[EDITBOX_TAG_BUTTON10] = _("Dziesi¹ty przycisk tagów");
	names[Editor] = _("W³¹cz / Wy³¹cz edytor");
	names[EndDifference] = _("Wstaw ró¿nicê koñcow¹");
	names[FilterByDialogues] = _("Ukryj komentarze");
	names[FilterByDoubtful] = _("Poka¿ niepewne");
	names[FilterByNothing] = _("Wy³¹cz filtrowanie");
	names[FilterBySelections] = _("Ukryj zaznaczone linie");
	names[FilterByStyles] = _("Ukryj linie ze stylami");
	names[FilterByUntranslated] = _("Poka¿ nieprzet³umaczone");
	names[FindNextDoubtful] = _("Nastêpne niepewne");
	names[FindNextUntranslated] = _("Nastêpne nieprzet³umaczone");
	names[FindReplaceDialog] = _("ZnajdŸ i zmieñ");
	names[FontCollectorID] = _("Kolekcjoner czcionek");
	names[FPSFromVideo] = _("Ustaw FPS z wideo");
	names[FrameToClipboard] = _("Kopiuj klatkê do schowka");
	names[FrameToPNG] = _("Zapisz klatkê jako PNG");
	names[GLOBAL_SORT_ALL_BY_START_TIMES] = _("Sortuj wszystko wed³ug czasu pocz¹tkowego");
	names[GLOBAL_SORT_ALL_BY_END_TIMES] = _("Sortuj wszystko wed³ug czasu koñcowego");
	names[GLOBAL_SORT_ALL_BY_STYLE] = _("Sortuj wszystko wed³ug stylów");
	names[GLOBAL_SORT_ALL_BY_ACTOR] = _("Sortuj wszystko wed³ug aktora");
	names[GLOBAL_SORT_ALL_BY_EFFECT] = _("Sortuj wszystko wed³ug efektu");
	names[GLOBAL_SORT_ALL_BY_LAYER] = _("Sortuj wszystko wed³ug warstwa");
	names[GLOBAL_SORT_SELECTED_BY_START_TIMES] = _("Sortuj wszystko wed³ug czasu pocz¹tkowego");
	names[GLOBAL_SORT_SELECTED_BY_END_TIMES] = _("Sortuj wszystko wed³ug czasu koñcowego");
	names[GLOBAL_SORT_SELECTED_BY_STYLE] = _("Sortuj wszystko wed³ug stylów");
	names[GLOBAL_SORT_SELECTED_BY_ACTOR] = _("Sortuj wszystko wed³ug aktora");
	names[GLOBAL_SORT_SELECTED_BY_EFFECT] = _("Sortuj wszystko wed³ug efektu");
	names[GLOBAL_SORT_SELECTED_BY_LAYER] = _("Sortuj wszystko wed³ug warstwa");
	names[GLOBAL_SHIFT_TIMES] = _("Przesuñ czasy / uruchom post processor");
	names[GoToPrewKeyframe] = _("PrzejdŸ do poprzedniej klatki kluczowej");
	names[GoToNextKeyframe] = _("PrzejdŸ do nastêpnej klatki kluczowej");
	names[GRID_FILTER] = _("Filtruj");
	names[GRID_FILTER_AFTER_SUBS_LOAD] = _("Filtruj po wczytaniu napisów");
	names[GRID_FILTER_DO_NOT_RESET] = _("Nie resetuj wczeœniejszego filtrowania");
	names[GRID_FILTER_IGNORE_IN_ACTIONS] = _("Ignoruj filtrowanie przy akcjach");
	names[GRID_FILTER_INVERTED] = _("Filtrowanie odwrócone");
	names[GRID_HIDE_ACTOR] = _("Ukryj aktora");
	names[GRID_HIDE_CPS] = _("Ukryj znaki na sekundê");
	names[GRID_HIDE_END] = _("Ukryj czas koñcowy");
	names[GRID_HIDE_EFFECT] = _("Ukryj efekt");
	names[GRID_HIDE_LAYER] = _("Ukryj warstwê");
	names[GRID_HIDE_MARGINL] = _("Ukryj lewy margines");
	names[GRID_HIDE_MARGINR] = _("Ukryj prawy margines");
	names[GRID_HIDE_MARGINV] = _("Ukryj pionowy margines");
	names[GRID_HIDE_START] = _("Ukryj czas pocz¹tkowy");
	names[GRID_HIDE_STYLE] = _("Ukryj styl");
	names[Help] = _("Pomoc (niekompletna, ale jednak)");
	names[Helpers] = _("&Lista osób pomocnych przy tworzeniu programu");
	names[HideProgressBar] = _("Ukryj / poka¿ pasek postêpu");
	names[HideSelected] = _("Ukryj zaznaczone linijki");
	names[HideTags] = _("Ukryj tagi w nawiasach");
	names[History] = _("Historia");
	names[InsertAfter] = _("Wstaw p&o");
	names[InsertAfterVideo] = _("Wstaw po z c&zasem wideo");
	names[InsertAfterWithVideoFrame] = _("Wstaw po z czasem klatki wideo");
	names[InsertBefore] = _("Wstaw &przed");
	names[InsertBeforeVideo] = _("Wstaw przed z &czasem wideo");
	names[InsertBeforeWithVideoFrame] = _("Wstaw przed z czasem klatki wideo");
	names[Join] = _("Z³¹cz &linijki");
	names[JoinToFirst] = _("Z³¹cz linijki zostaw pierwsz¹");
	names[JoinToLast] = _("Z³¹cz linijki zostaw ostatni¹");
	names[JoinWithPrevious] = _("Scal z poprzedni¹ linijk¹");
	names[JoinWithNext] = _("Scal z nastêpn¹ linijk¹");
	names[LoadLastScript] = _("Uruchom ostatnio zaczytany skrypt");
	names[Minus5Second] = _("5 sekund do ty³u");
	names[Minus5SecondG] = _("Wideo minus 5 sekund");
	names[MinusMinute] = _("Minuta do ty³u");
	names[NewFPS] = _("Ustaw nowy FPS");
	names[NextChapter] = _("Nastêpny rozdzia³");
	names[NextFrame] = _("Klatka w przód");
	names[NextLine] = _("Nastêpna linijka");
	names[NextTab] = _("Nastêpna karta");
	names[NextVideo] = _("Nastêpny plik");
	names[OpenAudio] = _("Otwórz audio");
	names[OpenSubs] = _("Otwórz napisy");
	names[OpenVideo] = _("Otwórz wideo");
	names[PasteCollumns] = _("Wklej kolumny");
	names[PasteTranslation] = _("Wklej tekst t³umaczenia");
	names[PlayActualLine] = _("Odtwórz aktywn¹ linijkê");
	names[PlayPause] = _("Odtwórz / Pauza");
	names[PlayPauseG] = _("Odtwórz / Pauza");
	names[Plus5SecondG] = _("Wideo plus 5 sekund");
	names[Plus5Second] = _("5 sekund do przodu");
	names[PlusMinute] = _("Minuta do przodu");
	names[PreviousChapter] = _("Poprzedni rozdzia³");
	names[PreviousFrame] = _("Klatka w ty³");
	names[PreviousLine] = _("Poprzednia linijka");
	names[PreviousTab] = _("Poprzednia karta");
	names[PreviousVideo] = _("Poprzedni plik");
	names[PutBold] = _("Wstaw pogrubienie");
	names[PutItalic] = _("Wstaw kursywê");
	names[Quit] = _("Wyjœcie");
	names[Redo] = _("Ponów");
	names[Remove] = _("Usuñ linijkê");
	names[RemoveSubs] = _("Usuñ napisy z e&dytora");
	names[RemoveText] = _("Usuñ tekst");
	names[SaveAllSubs] = _("Zapisz wszystkie napisy");
	names[SaveSubs] = _("Zapisz");
	names[SaveSubsAs] = _("Zapisz jako..."); 
	names[SaveTranslation] = _("Zapisz &t³umaczenie");
	names[Search] = _("ZnajdŸ");
	names[SelectFromVideo] = _("Zaznacz liniê z czasem wideo");
	names[SelectLinesDialog] = _("Zaznacz linijki");
	names[SetAudioFromVideo] = _("Ustaw audio z czasem wideo");
	names[SetAudioMarkFromVideo] = _("Ustaw znacznik audio z czasem wideo");
	names[SetDoubtful] = _("Ustaw jako niepewne i przejdŸ dalej");
	names[SetEndTime] = _("Wstaw czas koñcowy z wideo");
	names[SetStartTime] = _("Wstaw czas pocz¹tkowy z wideo"); 
	names[Settings] = _("&Ustawienia");
	names[SetVideoAtStart] = _("PrzejdŸ do czasu pocz¹tkowego linii");
	names[SetVideoAtEnd] = _("PrzejdŸ do czasu koñcowego linii");
	names[ShowPreview] = _("Poka¿ podgl¹d napisów");
	names[SnapWithEnd] = _("Przyklej koniec do klatki kluczowej");
	names[SnapWithStart] = _("Przyklej start do klatki kluczowej");
	names[SpellcheckerDialog] = _("SprawdŸ poprawnoœæ pisowni");
	names[SplitLine] = _("Wstaw znak podzia³u");
	names[StartDifference] = _("Wstaw ró¿nicê pocz¹tkow¹");
	names[StopPlayback] = _("Zatrzymaj");
	names[StyleManager] = _("Mened¿er stylów");
	names[SubbedFrameToClipboard] = _("Kopiuj klatkê z napisami do schowka");
	names[SubbedFrameToPNG] = _("Zapisz klatkê z napisami jako PNG");
	names[SubsResample] = _("Zmieñ rozdzielczoœæ napisów");
	names[SubsFromMKV] = _("Wczytaj napisy z pliku MKV");
	names[Swap] = _("Za&mieñ");
	names[TranslationDialog] = _("Okno przesuwania dialogów");
	names[Undo] = _("Cofnij");
	names[UndoToLastSave] = _("Cofnij do ostatniego zapisu");
	names[VideoZoom] = _("Powiêksz wideo");
	names[ViewAll] = _("Widok wszystko");
	names[ViewAudio] = _("Widok audio i napisy");
	names[ViewSubs] = _("Widok tylko napisy");
	names[ViewVideo] = _("Widok wideo i napisy");
	names[VolumePlus] = _("DŸwiêk g³oœniej"); 
	names[VolumeMinus] = _("DŸwiêk ciszej"); 
	
}
