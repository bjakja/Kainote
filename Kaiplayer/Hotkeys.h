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

#pragma once

#include <map>
#include "HotkeysNaming.h"
#include "ListControls.h"
#include "KaiDialog.h"
#include "EnumFactory.h"

//Po zmianie PlayPause nale¿y go zmieniæ w dshowrenderer by pauzowanie po odtwarzaniu linii zadzia³a³o
#define IDS(XX) \
	XX( AudioCommitAlt,=620 ) \
	XX( AudioPlayAlt, ) \
	XX( AudioPlayLineAlt, ) \
	XX( AudioPreviousAlt, ) \
	XX( AudioNextAlt, ) \
	XX( AudioCommit,=1620 ) \
	XX( AudioPlay, ) \
	XX( AudioPlayLine, ) \
	XX( AudioPrevious, ) \
	XX( AudioNext, ) \
	XX( AudioStop, ) \
	XX( AudioPlayBeforeMark, ) \
	XX( AudioPlayAfterMark, ) \
	XX( AudioPlay500MSBefore, ) \
	XX( AudioPlay500MSAfter, ) \
	XX( AudioPlay500MSFirst, ) \
	XX( AudioPlay500MSLast, ) \
	XX( AudioPlayToEnd, ) \
	XX( AudioScrollLeft, ) \
	XX( AudioScrollRight, ) \
	XX( AudioGoto, ) \
	XX( AudioLeadin, ) \
	XX( AudioLeadout, ) \
	XX( PlayPause,=2021)\
	XX( StopPlayback,) \
	XX( Plus5Second,)\
	XX( Minus5Second,)\
	XX( MinusMinute, ) \
	XX( PlusMinute, ) \
	XX( VolumePlus, ) \
	XX( VolumeMinus, ) \
	XX( PreviousVideo, ) \
	XX( NextVideo, ) \
	XX( PreviousChapter, ) \
	XX( NextChapter, ) \
	XX( FullScreen, ) \
	XX( HideProgressBar, ) \
	XX( DeleteVideo, ) \
	XX( AspectRatio, ) \
	XX( CopyCoords, ) \
	XX( FrameToPNG, ) \
	XX( FrameToClipboard, ) \
	XX( SubbedFrameToPNG, ) \
	XX( SubbedFrameToClipboard, ) \
	XX( EDITBOX_CHANGE_FONT,=3991) \
	XX( EDITBOX_CHANGE_UNDERLINE,) \
	XX( EDITBOX_CHANGE_STRIKEOUT,) \
	XX( EDITBOX_PASTE_ALL_TO_TRANSLATION,=3996) \
	XX( EDITBOX_PASTE_SELECTION_TO_TRANSLATION,) \
	XX( EDITBOX_HIDE_ORIGINAL,) \
	XX( EDITBOX_CHANGE_COLOR_PRIMARY,) \
	XX( EDITBOX_CHANGE_COLOR_SECONDARY,) \
	XX( EDITBOX_CHANGE_COLOR_OUTLINE,) \
	XX( EDITBOX_CHANGE_COLOR_SHADOW,) \
	XX( EDITBOX_COMMIT,) \
	XX( EDITBOX_COMMIT_GO_NEXT_LINE,) \
	XX( PutBold,) \
	XX( PutItalic, ) \
	XX( SplitLine, ) \
	XX( StartDifference, ) \
	XX( EndDifference, ) \
	XX( FindNextDoubtful, ) \
	XX( FindNextUntranslated, ) \
	XX( SetDoubtful, ) \
	XX( EDITBOX_TAG_BUTTON1,=4900)\
	XX( EDITBOX_TAG_BUTTON2,)\
	XX( EDITBOX_TAG_BUTTON3,)\
	XX( EDITBOX_TAG_BUTTON4,)\
	XX( EDITBOX_TAG_BUTTON5,)\
	XX( EDITBOX_TAG_BUTTON6,)\
	XX( EDITBOX_TAG_BUTTON7,)\
	XX( EDITBOX_TAG_BUTTON8,)\
	XX( EDITBOX_TAG_BUTTON9,)\
	XX( EDITBOX_TAG_BUTTON10,)\
	XX( EDITBOX_TAG_BUTTON11,)\
	XX( EDITBOX_TAG_BUTTON12,)\
	XX( EDITBOX_TAG_BUTTON13,)\
	XX( EDITBOX_TAG_BUTTON14,)\
	XX( EDITBOX_TAG_BUTTON15,)\
	XX( EDITBOX_TAG_BUTTON16,)\
	XX( EDITBOX_TAG_BUTTON17,)\
	XX( EDITBOX_TAG_BUTTON18,)\
	XX( EDITBOX_TAG_BUTTON19,)\
	XX( EDITBOX_TAG_BUTTON20,)\
	XX( GRID_HIDE_LAYER,=5001 ) \
	XX( GRID_HIDE_START, ) \
	XX( GRID_HIDE_END,=5004 ) \
	XX( GRID_HIDE_ACTOR,=5016 ) \
	XX( GRID_HIDE_STYLE,=5008 ) \
	XX( GRID_HIDE_MARGINL,=5032 ) \
	XX( GRID_HIDE_MARGINR,=5064 ) \
	XX( GRID_HIDE_MARGINV,=5128 ) \
	XX( GRID_HIDE_EFFECT,=5256 ) \
	XX( GRID_HIDE_CPS,=5512 ) \
	XX( InsertBefore,=5555 ) \
	XX( InsertAfter, ) \
	XX( InsertBeforeVideo, ) \
	XX( InsertAfterVideo, ) \
	XX( InsertBeforeWithVideoFrame, ) \
	XX( InsertAfterWithVideoFrame, ) \
	XX( Swap, ) \
	XX( Duplicate, ) \
	XX( Join, ) \
	XX( JoinToFirst, ) \
	XX( JoinToLast, ) \
	XX( Copy, ) \
	XX( Paste, ) \
	XX( Cut, ) \
	XX( ShowPreview, ) \
	XX( HideSelected, ) \
	XX( FilterByNothing, ) \
	XX( FilterByStyles, ) \
	XX( FilterBySelections, ) \
	XX( FilterByDialogues, ) \
	XX( FilterByDoubtful, ) \
	XX( FilterByUntranslated, ) \
	XX( GRID_FILTER, ) \
	XX( GRID_FILTER_AFTER_SUBS_LOAD, ) \
	XX( GRID_FILTER_INVERTED, ) \
	XX( GRID_FILTER_DO_NOT_RESET, ) \
	XX( GRID_FILTER_IGNORE_IN_ACTIONS, ) \
	XX( GRID_TREE_MAKE, ) \
	XX( PasteTranslation, ) \
	XX( TranslationDialog, ) \
	XX( SubsFromMKV, ) \
	XX( ContinousPrevious, ) \
	XX( ContinousNext, ) \
	XX( PasteCollumns, ) \
	XX( CopyCollumns, ) \
	XX( FPSFromVideo, ) \
	XX( NewFPS, ) \
	XX( SaveSubs,=6677)\
	XX( SaveAllSubs,)\
	XX( SaveSubsAs,)\
	XX( SaveTranslation,)\
	XX( RemoveSubs,)\
	XX( Search,)\
	XX( GLOBAL_FIND_REPLACE,)\
	XX(	GLOBAL_MISSPELLS_REPLACER,)\
	XX( SelectLinesDialog,)\
	XX( SpellcheckerDialog,)\
	XX( VideoIndexing,)\
	XX( SaveWithVideoName,)\
	XX( OpenAudio,)\
	XX( AudioFromVideo,)\
	XX( CloseAudio,)\
	XX( ASSProperties,)\
	XX( StyleManager,)\
	XX( SubsResample,)\
	XX( FontCollectorID,)\
	XX( ConvertToASS,)\
	XX( ConvertToSRT,)\
	XX( ConvertToTMP,)\
	XX( ConvertToMDVD,)\
	XX( ConvertToMPL2,)\
	XX( HideTags,)\
	XX( ChangeTime,)\
	XX( ViewAll,)\
	XX( ViewAudio,)\
	XX( ViewVideo,)\
	XX( GLOBAL_VIEW_ONLY_VIDEO,)\
	XX( ViewSubs,)\
	XX( AutoLoadScript,)\
	XX( AutoReloadAutoload,)\
	XX( LoadLastScript,)\
	XX( AUTOMATION_OPEN_HOTKEYS_WINDOW,)\
	XX( PlayPauseG,)\
	XX( PreviousFrame,)\
	XX( NextFrame,)\
	XX( VideoZoom,)\
	XX( SetStartTime,)\
	XX( SetEndTime,)\
	XX( SetVideoAtStart,)\
	XX( SetVideoAtEnd,)\
	XX( GoToNextKeyframe,)\
	XX( GoToPrewKeyframe,)\
	XX( SetAudioFromVideo,)\
	XX( SetAudioMarkFromVideo,)\
	XX( Redo,)\
	XX( Undo,)\
	XX( UndoToLastSave,)\
	XX( GLOBAL_LOAD_LAST_SESSION,)\
	XX( History,)\
	XX( OpenSubs,=6800)\
	XX( OpenVideo,)\
	XX( GLOBAL_KEYFRAMES_OPEN,)\
	XX( Settings,)\
	XX( Quit,)\
	XX( Editor,)\
	XX( About,)\
	XX( Helpers,)\
	XX( Help,)\
	XX( ANSI,)\
	XX( PreviousLine,=6850)\
	XX( NextLine,)\
	XX( JoinWithPrevious,)\
	XX( JoinWithNext,)\
	XX( NextTab,)\
	XX( PreviousTab,)\
	XX( Remove, ) \
	XX( RemoveText, ) \
	XX( SnapWithStart,)\
	XX( SnapWithEnd,)\
	XX( Plus5SecondG,)\
	XX( Minus5SecondG,)\
	XX( SortLines,)\
	XX( SortSelected,)\
	XX( RecentAudio,)\
	XX( RecentVideo,)\
	XX( RecentSubs,)\
	XX( GLOBAL_KEYFRAMES_RECENT,)\
	XX( SelectFromVideo,)\
	XX( PlayActualLine,)\
	XX( GLOBAL_SORT_ALL_BY_START_TIMES,=7000)\
	XX( GLOBAL_SORT_ALL_BY_END_TIMES,)\
	XX( GLOBAL_SORT_ALL_BY_STYLE,)\
	XX( GLOBAL_SORT_ALL_BY_ACTOR,)\
	XX( GLOBAL_SORT_ALL_BY_EFFECT,)\
	XX( GLOBAL_SORT_ALL_BY_LAYER,)\
	XX( GLOBAL_SORT_SELECTED_BY_START_TIMES,)\
	XX( GLOBAL_SORT_SELECTED_BY_END_TIMES,)\
	XX( GLOBAL_SORT_SELECTED_BY_STYLE,)\
	XX( GLOBAL_SORT_SELECTED_BY_ACTOR,)\
	XX( GLOBAL_SORT_SELECTED_BY_EFFECT,)\
	XX( GLOBAL_SORT_SELECTED_BY_LAYER,)\
	XX( GLOBAL_SHIFT_TIMES,=11139)\

//
DECLARE_ENUM(Id, IDS)


enum{
	GLOBAL_HOTKEY = 0,
	GRID_HOTKEY,
	EDITBOX_HOTKEY,
	VIDEO_HOTKEY,
	AUDIO_HOTKEY
};
class idAndType{
public:
	idAndType(int _id = 0, char _type = GLOBAL_HOTKEY){ id = _id; Type = _type; }
	bool operator < (const idAndType match){ return id < match.id; };
	bool operator >(const idAndType match){ return id > match.id; };
	bool operator <= (const idAndType match){ return id <= match.id; };
	bool operator >= (const idAndType match){ return id >= match.id; };
	bool operator == (const idAndType match){ return id == match.id; };
	bool operator != (const idAndType match){ return id != match.id; };
	int id;
	char Type;
};

bool operator < (const idAndType match, const idAndType match1);
bool operator > (const idAndType match, const idAndType match1);
bool operator <= (const idAndType match, const idAndType match1);
bool operator >= (const idAndType match, const idAndType match1);
bool operator == (const idAndType match, const idAndType match1);
bool operator != (const idAndType match, const idAndType match1);
bool operator == (const idAndType &match, const int match1);
bool operator == (const int match1, const idAndType &match);
bool operator >= (const idAndType &match, const int match1);
bool operator >= (const int match1, const idAndType &match);
bool operator <= (const idAndType &match, const int match1);
bool operator <= (const int match1, const idAndType &match);
bool operator > (const idAndType &match, const int match1);
bool operator > (const int match1, const idAndType &match);
bool operator < (const idAndType &match, const int match1);
bool operator < (const int match1, const idAndType &match);
bool operator != (const idAndType &match, const int match1);
bool operator != (const int match1, const idAndType &match);

class hdata{
public:
	hdata(const wxString &accName, const wxString &_Accel){
		Name = accName; Accel = _Accel;
	}
	hdata(const wxString &acc){
		Accel = acc;
	}
	hdata(){}
	wxString Name;
	wxString Accel;
};

class HkeysDialog : public KaiDialog
{
public:
	HkeysDialog(wxWindow *parent, wxString name, char hotkeyWindow = GLOBAL_HOTKEY, bool showWindowSelection = true);
	virtual ~HkeysDialog();
	wxString hotkey;
	wxString hkname;
	KaiChoice *global;
	char type;

private:
	void OnKeyPress(wxKeyEvent& event);
};

class Hotkeys
{
private:
	std::map<idAndType, hdata> hkeys;
public:
	Hotkeys();
	~Hotkeys();
	int LoadHkeys(bool Audio = false);
	void LoadDefault(std::map<idAndType, hdata> &_hkeys, bool Audio = false);
	void SaveHkeys(bool Audio = false);
	void SetHKey(const idAndType &itype, wxString name, wxString hotkey);
	wxAcceleratorEntry GetHKey(const idAndType itype, const hdata *hkey = 0);
	wxString GetStringHotkey(const idAndType &itype, const wxString &name = L"");
	void FillTable();
	void ResetKey(const idAndType *itype, int id = 0, char type = GLOBAL_HOTKEY);
	wxString GetDefaultKey(const idAndType &itype);
	void OnMapHkey(int id, wxString name, wxWindow *parent, char hotkeyWindow = GLOBAL_HOTKEY, bool showWindowSelection = true);
	void SetAccels(bool all = false);
	wxString GetName(const idAndType itype);
	const std::map<int, wxString> &GetNamesTable();
	const wxString &GetName(int id);
	int GetType(int id);
	const std::map<idAndType, hdata> &GetHotkeysMap(){ return hkeys; }
	void SetHotkeysMap(const std::map<idAndType, hdata> &hotkeys){ hkeys = std::map<idAndType, hdata>(hotkeys); }
	void ResetDefaults();
	std::map<int, wxString> keys;
	bool AudioKeys;
	int lastScriptId;
	HotkeysNaming *hotkeysNaming = NULL;
};


extern Hotkeys Hkeys;
