//  Copyright (c) 2016-2020, Marcin Drob

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
#include "EnumFactory.h"
#include "KaiDialog.h"

//Po zmianie VIDEO_PLAY_PAUSE nale¿y go zmieniæ w dshowrenderer by pauzowanie po odtwarzaniu linii zadzia³a³o

#define IDS(XX) \
	XX( AUDIO_COMMIT_ALT,=1000 ) \
	XX( AUDIO_PLAY_ALT, ) \
	XX( AUDIO_PLAY_LINE_ALT, ) \
	XX( AUDIO_PREVIOUS_ALT, ) \
	XX( AUDIO_NEXT_ALT, ) \
	XX( AUDIO_COMMIT,=1010 ) \
	XX( AUDIO_PLAY, ) \
	XX( AUDIO_PLAY_LINE, ) \
	XX( AUDIO_PREVIOUS, ) \
	XX( AUDIO_NEXT, ) \
	XX( AUDIO_STOP, ) \
	XX( AUDIO_PLAY_BEFORE_MARK, ) \
	XX( AUDIO_PLAY_AFTER_MARK, ) \
	XX( AUDIO_PLAY_500MS_BEFORE, ) \
	XX( AUDIO_PLAY_500MS_AFTER, ) \
	XX( AUDIO_PLAY_500MS_FIRST, ) \
	XX( AUDIO_PLAY_500MS_LAST, ) \
	XX( AUDIO_PLAY_TO_END, ) \
	XX( AUDIO_SCROLL_LEFT, ) \
	XX( AUDIO_SCROLL_RIGHT, ) \
	XX( AUDIO_GOTO, ) \
	XX( AUDIO_LEAD_IN, ) \
	XX( AUDIO_LEAD_OUT, ) \
	XX( VIDEO_PLAY_PAUSE,=2000)\
	XX( VIDEO_STOP,) \
	XX( VIDEO_5_SECONDS_FORWARD,)\
	XX( VIDEO_5_SECONDS_BACKWARD,)\
	XX( VIDEO_MINUTE_BACKWARD, ) \
	XX( VIDEO_MINUTE_FORWARD, ) \
	XX( VIDEO_VOLUME_PLUS, ) \
	XX( VIDEO_VOLUME_MINUS, ) \
	XX( VIDEO_PREVIOUS_FILE, ) \
	XX( VIDEO_NEXT_FILE, ) \
	XX( VIDEO_PREVIOUS_CHAPTER, ) \
	XX( VIDEO_NEXT_CHAPTER, ) \
	XX( VIDEO_FULL_SCREEN, ) \
	XX( VIDEO_HIDE_PROGRESS_BAR, ) \
	XX( VIDEO_DELETE_FILE, ) \
	XX( VIDEO_ASPECT_RATIO, ) \
	XX( VIDEO_COPY_COORDS, ) \
	XX( VIDEO_SAVE_FRAME_TO_PNG, ) \
	XX( VIDEO_COPY_FRAME_TO_CLIPBOARD, ) \
	XX( VIDEO_SAVE_SUBBED_FRAME_TO_PNG, ) \
	XX( VIDEO_COPY_SUBBED_FRAME_TO_CLIPBOARD, ) \
	XX( EDITBOX_CHANGE_FONT,=3000) \
	XX( EDITBOX_CHANGE_UNDERLINE,) \
	XX( EDITBOX_CHANGE_STRIKEOUT,) \
	XX( EDITBOX_PASTE_ALL_TO_TRANSLATION,) \
	XX( EDITBOX_PASTE_SELECTION_TO_TRANSLATION,) \
	XX( EDITBOX_HIDE_ORIGINAL,) \
	XX( EDITBOX_CHANGE_COLOR_PRIMARY,) \
	XX( EDITBOX_CHANGE_COLOR_SECONDARY,) \
	XX( EDITBOX_CHANGE_COLOR_OUTLINE,) \
	XX( EDITBOX_CHANGE_COLOR_SHADOW,) \
	XX( EDITBOX_COMMIT,) \
	XX( EDITBOX_COMMIT_GO_NEXT_LINE,) \
	XX( EDITBOX_INSERT_BOLD,) \
	XX( EDITBOX_INSERT_ITALIC, ) \
	XX( EDITBOX_SPLIT_LINE, ) \
	XX( EDITBOX_START_DIFFERENCE, ) \
	XX( EDITBOX_END_DIFFERENCE, ) \
	XX( EDITBOX_FIND_NEXT_DOUBTFUL, ) \
	XX( EDITBOX_FIND_NEXT_UNTRANSLATED, ) \
	XX( EDITBOX_SET_DOUBTFUL,) \
	XX( EDITBOX_TAG_BUTTON1,=3100)\
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
	XX( GRID_HIDE_LAYER,=4001 ) \
	XX( GRID_HIDE_START, ) \
	XX( GRID_HIDE_END,=4004 ) \
	XX( GRID_HIDE_ACTOR,=4016 ) \
	XX( GRID_HIDE_STYLE,=4008 ) \
	XX( GRID_HIDE_MARGINL,=4032 ) \
	XX( GRID_HIDE_MARGINR,=4064 ) \
	XX( GRID_HIDE_MARGINV,=4128 ) \
	XX( GRID_HIDE_EFFECT,=4256 ) \
	XX( GRID_HIDE_CPS,=4512 ) \
	XX( GRID_HIDE_WRAPS,=4513 ) \
	XX( GRID_INSERT_BEFORE, ) \
	XX( GRID_INSERT_AFTER, ) \
	XX( GRID_INSERT_BEFORE_VIDEO, ) \
	XX( GRID_INSERT_AFTER_VIDEO, ) \
	XX( GRID_INSERT_BEFORE_WITH_VIDEO_FRAME, ) \
	XX( GRID_INSERT_AFTER_WITH_VIDEO_FRAME, ) \
	XX( GRID_SWAP_LINES, ) \
	XX( GRID_DUPLICATE_LINES, ) \
	XX( GRID_JOIN_LINES, ) \
	XX( GRID_JOIN_TO_FIRST_LINE, ) \
	XX( GRID_JOIN_TO_LAST_LINE, ) \
	XX( GRID_COPY,) \
	XX( GRID_PASTE,) \
	XX( GRID_CUT,) \
	XX( GRID_SHOW_PREVIEW,) \
	XX( GRID_HIDE_SELECTED,) \
	XX( GRID_FILTER_BY_NOTHING,) \
	XX( GRID_FILTER_BY_STYLES,) \
	XX( GRID_FILTER_BY_SELECTIONS,) \
	XX( GRID_FILTER_BY_DIALOGUES,) \
	XX( GRID_FILTER_BY_DOUBTFUL,) \
	XX( GRID_FILTER_BY_UNTRANSLATED,) \
	XX( GRID_FILTER,) \
	XX( GRID_FILTER_AFTER_SUBS_LOAD,) \
	XX( GRID_FILTER_INVERT,) \
	XX( GRID_FILTER_DO_NOT_RESET,) \
	XX( GRID_FILTER_IGNORE_IN_ACTIONS,) \
	XX( GRID_TREE_MAKE,) \
	XX( GRID_PASTE_TRANSLATION,) \
	XX( GRID_TRANSLATION_DIALOG,) \
	XX( GRID_SUBS_FROM_MKV,) \
	XX( GRID_MAKE_CONTINOUS_PREVIOUS_LINE,) \
	XX( GRID_MAKE_CONTINOUS_NEXT_LINE,) \
	XX( GRID_PASTE_COLUMNS,) \
	XX( GRID_COPY_COLUMNS,) \
	XX( GRID_SET_FPS_FROM_VIDEO,) \
	XX( GRID_SET_NEW_FPS,) \
	XX( GLOBAL_SAVE_SUBS,=5000)\
	XX( GLOBAL_SAVE_ALL_SUBS,)\
	XX( GLOBAL_SAVE_SUBS_AS,)\
	XX( GLOBAL_SAVE_TRANSLATION,)\
	XX( GLOBAL_REMOVE_SUBS,)\
	XX( GLOBAL_REDO,)\
	XX( GLOBAL_UNDO,)\
	XX( GLOBAL_UNDO_TO_LAST_SAVE,)\
	XX( GLOBAL_HISTORY,)\
	XX( GLOBAL_SEARCH,)\
	XX( GLOBAL_FIND_REPLACE,)\
	XX( GLOBAL_FIND_NEXT,)\
	XX( GLOBAL_MISSPELLS_REPLACER,)\
	XX( GLOBAL_OPEN_SELECT_LINES,)\
	XX( GLOBAL_OPEN_SPELLCHECKER,)\
	XX( GLOBAL_VIDEO_INDEXING,)\
	XX( GLOBAL_SAVE_WITH_VIDEO_NAME,)\
	XX( GLOBAL_OPEN_AUDIO,)\
	XX( GLOBAL_AUDIO_FROM_VIDEO,)\
	XX( GLOBAL_CLOSE_AUDIO,)\
	XX( GLOBAL_CONVERT_TO_ASS,)\
	XX( GLOBAL_CONVERT_TO_SRT,)\
	XX( GLOBAL_CONVERT_TO_TMP,)\
	XX( GLOBAL_CONVERT_TO_MDVD,)\
	XX( GLOBAL_CONVERT_TO_MPL2,)\
	XX( GLOBAL_OPEN_ASS_PROPERTIES,)\
	XX( GLOBAL_OPEN_STYLE_MANAGER,)\
	XX( GLOBAL_OPEN_SUBS_RESAMPLE,)\
	XX( GLOBAL_OPEN_FONT_COLLECTOR,)\
	XX( GLOBAL_HIDE_TAGS,)\
	XX( GLOBAL_SHOW_SHIFT_TIMES,)\
	XX( GLOBAL_VIEW_ALL,)\
	XX( GLOBAL_VIEW_AUDIO,)\
	XX( GLOBAL_VIEW_VIDEO,)\
	XX( GLOBAL_VIEW_ONLY_VIDEO,)\
	XX( GLOBAL_VIEW_SUBS,)\
	XX( GLOBAL_AUTOMATION_LOAD_SCRIPT,)\
	XX( GLOBAL_AUTOMATION_RELOAD_AUTOLOAD,)\
	XX( GLOBAL_AUTOMATION_LOAD_LAST_SCRIPT,)\
	XX( GLOBAL_AUTOMATION_OPEN_HOTKEYS_WINDOW,)\
	XX( GLOBAL_PLAY_PAUSE,)\
	XX( GLOBAL_PREVIOUS_FRAME,)\
	XX( GLOBAL_NEXT_FRAME,)\
	XX( GLOBAL_VIDEO_ZOOM,)\
	XX( GLOBAL_SET_START_TIME,)\
	XX( GLOBAL_SET_END_TIME,)\
	XX( GLOBAL_SET_VIDEO_AT_START_TIME,)\
	XX( GLOBAL_SET_VIDEO_AT_END_TIME,)\
	XX( GLOBAL_GO_TO_NEXT_KEYFRAME,)\
	XX( GLOBAL_GO_TO_PREVIOUS_KEYFRAME,)\
	XX( GLOBAL_SET_AUDIO_FROM_VIDEO,)\
	XX( GLOBAL_SET_AUDIO_MARK_FROM_VIDEO,)\
	XX( GLOBAL_LOAD_LAST_SESSION,)\
	XX( GLOBAL_OPEN_SUBS,=5100)\
	XX( GLOBAL_OPEN_VIDEO,)\
	XX( GLOBAL_OPEN_KEYFRAMES,)\
	XX( GLOBAL_OPEN_DUMMY_VIDEO,)\
	XX( GLOBAL_OPEN_DUMMY_AUDIO,)\
	XX( GLOBAL_OPEN_AUTO_SAVE,)\
	XX( GLOBAL_DELETE_TEMPORARY_FILES,)\
	XX( GLOBAL_SETTINGS,)\
	XX( GLOBAL_QUIT,)\
	XX( GLOBAL_EDITOR,)\
	XX( GLOBAL_ABOUT,)\
	XX( GLOBAL_HELPERS,)\
	XX( GLOBAL_HELP,)\
	XX( GLOBAL_ANSI,)\
	XX( GLOBAL_PREVIOUS_LINE,=5150)\
	XX( GLOBAL_NEXT_LINE,)\
	XX( GLOBAL_JOIN_WITH_PREVIOUS,)\
	XX( GLOBAL_JOIN_WITH_NEXT,)\
	XX( GLOBAL_NEXT_TAB,)\
	XX( GLOBAL_PREVIOUS_TAB,)\
	XX( GLOBAL_REMOVE_LINES,) \
	XX( GLOBAL_REMOVE_TEXT,) \
	XX( GLOBAL_SNAP_WITH_START,)\
	XX( GLOBAL_SNAP_WITH_END,)\
	XX( GLOBAL_SORT_LINES,)\
	XX( GLOBAL_SORT_SELECTED_LINES,)\
	XX( GLOBAL_RECENT_AUDIO,)\
	XX( GLOBAL_RECENT_VIDEO,)\
	XX( GLOBAL_RECENT_SUBS,)\
	XX( GLOBAL_RECENT_KEYFRAMES,)\
	XX( GLOBAL_SELECT_FROM_VIDEO,)\
	XX( GLOBAL_PLAY_ACTUAL_LINE,)\
	XX( GLOBAL_STYLE_MANAGER_CLEAN_STYLE,)\
	XX( GLOBAL_SORT_ALL_BY_START_TIMES,=5200)\
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
	XX( GLOBAL_SHIFT_TIMES,=5300)\
	XX( GLOBAL_ADD_PAGE,)\
	XX( GLOBAL_CLOSE_PAGE,)\


DECLARE_ENUM(Id, IDS)


enum{
	GLOBAL_HOTKEY = 0,
	GRID_HOTKEY,
	EDITBOX_HOTKEY,
	VIDEO_HOTKEY,
	AUDIO_HOTKEY
};
#define GET_WINDOW_BY_ID(id) {return 5 - (id / 1000);}

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
	int GetModifier(const wxString& accel);
	std::map<int, wxString> keys;
	bool AudioKeys;
	int lastScriptId;
	HotkeysNaming *hotkeysNaming = NULL;
};


extern Hotkeys Hkeys;
