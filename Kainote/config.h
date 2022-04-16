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

#include <wx/tokenzr.h>
#include <map>
#include <vector>
#include <algorithm>
#include "Styles.h"
#include <wx/utils.h> 
#include "EnumFactory.h"
#include <wx/colour.h>
#include <wx/string.h>
#include <wx/window.h>
#include <wx/bitmap.h>
#include "Styles.h"
#include "LogHandler.h"
#include "config.h"

const wxString emptyString;

//Dont change enumeration config and colors from 1 to last, zero for non exist trash
#define CFG(CG) \
	CG(AUDIO_AUTO_COMMIT,=1)\
	CG(AUDIO_AUTO_FOCUS,)\
	CG(AUDIO_AUTO_SCROLL,)\
	CG(AUDIO_BOX_HEIGHT,)\
	CG(AUDIO_CACHE_FILES_LIMIT,)\
	CG(AUDIO_DELAY,)\
	CG(AUDIO_DONT_PLAY_WHEN_LINE_CHANGES,)\
	CG(AUDIO_DRAW_KEYFRAMES,)\
	CG(AUDIO_DRAW_SECONDARY_LINES,)\
	CG(AUDIO_DRAW_SELECTION_BACKGROUND,)\
	CG(AUDIO_DRAW_TIME_CURSOR,)\
	CG(AUDIO_DRAW_VIDEO_POSITION,)\
	CG(AUDIO_GRAB_TIMES_ON_SELECT,)\
	CG(AUDIO_HORIZONTAL_ZOOM,)\
	CG(AUDIO_INACTIVE_LINES_DISPLAY_MODE,)\
	CG(AUDIO_KARAOKE,)\
	CG(AUDIO_KARAOKE_MOVE_ON_CLICK,)\
	CG(AUDIO_KARAOKE_SPLIT_MODE,)\
	CG(AUDIO_LEAD_IN_VALUE,)\
	CG(AUDIO_LEAD_OUT_VALUE,)\
	CG(AUDIO_LINE_BOUNDARIES_THICKNESS,)\
	CG(AUDIO_LINK,)\
	CG(AUDIO_LOCK_SCROLL_ON_CURSOR,)\
	CG(AUDIO_MARK_PLAY_TIME,)\
	CG(AUDIO_MERGE_EVERY_N_WITH_SYLLABLE,)\
	CG(AUDIO_NEXT_LINE_ON_COMMIT,)\
	CG(AUDIO_RAM_CACHE,)\
	CG(AUDIO_SNAP_TO_KEYFRAMES,)\
	CG(AUDIO_SNAP_TO_OTHER_LINES,)\
	CG(AUDIO_SPECTRUM_ON,)\
	CG(AUDIO_SPECTRUM_NON_LINEAR_ON,)\
	CG(AUDIO_START_DRAG_SENSITIVITY,)\
	CG(AUDIO_VERTICAL_ZOOM,)\
	CG(AUDIO_VOLUME,)\
	CG(AUDIO_WHEEL_DEFAULT_TO_ZOOM,)\
	CG(ACCEPTED_AUDIO_STREAM,)\
	CG(ASS_PROPERTIES_TITLE,)\
	CG(ASS_PROPERTIES_SCRIPT,)\
	CG(ASS_PROPERTIES_TRANSLATION,)\
	CG(ASS_PROPERTIES_EDITING,)\
	CG(ASS_PROPERTIES_TIMING,)\
	CG(ASS_PROPERTIES_UPDATE,)\
	CG(ASS_PROPERTIES_TITLE_ON,)\
	CG(ASS_PROPERTIES_SCRIPT_ON,)\
	CG(ASS_PROPERTIES_TRANSLATION_ON,)\
	CG(ASS_PROPERTIES_EDITING_ON,)\
	CG(ASS_PROPERTIES_TIMING_ON,)\
	CG(ASS_PROPERTIES_UPDATE_ON,)\
	CG(ASS_PROPERTIES_ASK_FOR_CHANGE,)\
	CG(AUDIO_RECENT_FILES,)\
	CG(AUTOMATION_LOADING_METHOD,)\
	CG(AUTOMATION_OLD_SCRIPTS_COMPATIBILITY,)\
	CG(AUTOMATION_RECENT_FILES,)\
	CG(AUTOMATION_SCRIPT_EDITOR,)\
	CG(AUTOMATION_TRACE_LEVEL,)\
	CG(AUTO_MOVE_TAGS_FROM_ORIGINAL,)\
	CG(AUTOSAVE_MAX_FILES,)\
	CG(AUTO_SELECT_LINES_FROM_LAST_TAB,)\
	CG(COLORPICKER_RECENT_COLORS,)\
	CG(COLORPICKER_SWITCH_CLICKS,)\
	CG(CONVERT_ASS_TAGS_TO_INSERT_IN_LINE,)\
	CG(CONVERT_FPS,)\
	CG(CONVERT_FPS_FROM_VIDEO,)\
	CG(CONVERT_NEW_END_TIMES,)\
	CG(CONVERT_RESOLUTION_WIDTH,)\
	CG(CONVERT_RESOLUTION_HEIGHT,)\
	CG(CONVERT_SHOW_SETTINGS,)\
	CG(CONVERT_STYLE,)\
	CG(CONVERT_STYLE_CATALOG,)\
	CG(CONVERT_TIME_PER_CHARACTER,)\
	CG(COPY_COLLUMS_SELECTIONS,)\
	CG(DICTIONARY_LANGUAGE,)\
	CG(DISABLE_LIVE_VIDEO_EDITING,)\
	CG(DONT_ASK_FOR_BAD_RESOLUTION,)\
	CG(EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK,)\
	CG(EDITBOX_TIMES_TO_FRAMES_SWITCH,)\
	CG(EDITOR_ON,)\
	CG(FIND_IN_SUBS_FILTERS_RECENT,)\
	CG(FIND_IN_SUBS_PATHS_RECENT,)\
	CG(FIND_REPLACE_STYLES,)\
	CG(FIND_RECENT_FINDS,)\
	CG(FIND_REPLACE_OPTIONS,)\
	CG(FONT_COLLECTOR_ACTION,)\
	CG(FONT_COLLECTOR_DIRECTORY,)\
	CG(FONT_COLLECTOR_FROM_MKV,)\
	CG(FONT_COLLECTOR_USE_SUBS_DIRECTORY,)\
	CG(FFMS2_VIDEO_SEEKING,)\
	CG(GRID_CHANGE_ACTIVE_ON_SELECTION,)\
	CG(GRID_FONT,)\
	CG(GRID_FONT_SIZE,)\
	CG(GRID_ADD_TO_FILTER,)\
	CG(GRID_FILTER_AFTER_LOAD,)\
	CG(GRID_FILTER_BY,)\
	CG(GRID_FILTER_INVERTED,)\
	CG(GRID_FILTER_STYLES,)\
	CG(GRID_HIDE_COLUMNS,)\
	CG(GRID_HIDE_TAGS,)\
	CG(GRID_IGNORE_FILTERING,)\
	CG(GRID_LOAD_SORTED_SUBS,)\
	CG(GRID_SAVE_AFTER_CHARACTER_COUNT,)\
	CG(GRID_TAGS_SWAP_CHARACTER,)\
	CG(GRID_INSERT_END_OFFSET,)\
	CG(GRID_INSERT_START_OFFSET,)\
	CG(GRID_DUPLICATION_DONT_CHANGE_SELECTION,)\
	CG(GRID_DONT_CENTER_ACTIVE_LINE,)\
	CG(KEYFRAMES_RECENT,)\
	CG(LAST_SESSION_CONFIG,)\
	CG(SHIFT_TIMES_BY_TIME,)\
	CG(SHIFT_TIMES_CHANGE_VALUES_WITH_TAB,)\
	CG(SHIFT_TIMES_CORRECT_END_TIMES,)\
	CG(SHIFT_TIMES_MOVE_FORWARD,)\
	CG(SHIFT_TIMES_DISPLAY_FRAMES,)\
	CG(SHIFT_TIMES_ON,)\
	CG(SHIFT_TIMES_OPTIONS,)\
	CG(SHIFT_TIMES_WHICH_LINES,)\
	CG(SHIFT_TIMES_WHICH_TIMES,)\
	CG(SHIFT_TIMES_STYLES,)\
	CG(SHIFT_TIMES_TIME,)\
	CG(MOVE_VIDEO_TO_ACTIVE_LINE,)\
	CG(EDITBOX_DONT_GO_TO_NEXT_LINE_ON_TIMES_EDIT,)\
	CG(OPEN_SUBS_IN_NEW_TAB,)\
	CG(OPEN_VIDEO_AT_ACTIVE_LINE,)\
	CG(PASTE_COLUMNS_SELECTION,)\
	CG(VIDEO_PLAY_AFTER_SELECTION,)\
	CG(POSTPROCESSOR_ON,)\
	CG(POSTPROCESSOR_KEYFRAME_BEFORE_START,)\
	CG(POSTPROCESSOR_KEYFRAME_AFTER_START,)\
	CG(POSTPROCESSOR_KEYFRAME_BEFORE_END,)\
	CG(POSTPROCESSOR_KEYFRAME_AFTER_END,)\
	CG(POSTPROCESSOR_LEAD_IN,)\
	CG(POSTPROCESSOR_LEAD_OUT,)\
	CG(POSTPROCESSOR_THRESHOLD_START,)\
	CG(POSTPROCESSOR_THRESHOLD_END,)\
	CG(STYLE_PREVIEW_TEXT,)\
	CG(PROGRAM_FONT,)\
	CG(PROGRAM_FONT_SIZE,)\
	CG(PROGRAM_LANGUAGE,)\
	CG(PROGRAM_THEME,)\
	CG(REPLACE_RECENT_REPLACEMENTS,)\
	CG(SELECT_LINES_RECENT_SELECTIONS,)\
	CG(SELECT_LINES_OPTIONS,)\
	CG(GRID_SET_VISIBLE_LINE_AFTER_FULL_SCREEN,)\
	CG(SHIFT_TIMES_PROFILES,)\
	CG(SPELLCHECKER_ON,)\
	CG(STYLE_EDIT_FILTER_TEXT,)\
	CG(STYLE_EDIT_FILTER_TEXT_ON,)\
	CG(STYLE_MANAGER_POSITION,)\
	CG(STYLE_MANAGER_DETACH_EDIT_WINDOW,)\
	CG(SUBS_AUTONAMING,)\
	CG(SUBS_COMPARISON_TYPE,)\
	CG(SUBS_COMPARISON_STYLES,)\
	CG(SUBS_RECENT_FILES,)\
	CG(TAB_TEXT_MAX_CHARS,)\
	CG(TEXT_EDITOR_FONT_SIZE,)\
	CG(TEXT_EDITOR_HIDE_STATUS_BAR,)\
	CG(TEXT_EDITOR_CHANGE_QUOTES,)\
	CG(TEXT_EDITOR_TAG_LIST_OPTIONS,)\
	CG(TEXT_FIELD_ALLOW_NUMPAD_HOTKEYS,)\
	CG(TL_MODE_SHOW_ORIGINAL,)\
	CG(TL_MODE_HIDE_ORIGINAL_ON_VIDEO,)\
	CG(TOOLBAR_IDS,)\
	CG(TOOLBAR_ALIGNMENT,)\
	CG(UPDATER_CHECK_INTENSITY,)\
	CG(UPDATER_CHECK_OPTIONS,)\
	CG(UPDATER_CHECK_FOR_STABLE,)\
	CG(UPDATER_LAST_CHECK,)\
	CG(VIDEO_FULL_SCREEN_ON_START,)\
	CG(VIDEO_INDEX,)\
	CG(VIDEO_PAUSE_ON_CLICK,)\
	CG(VIDEO_PROGRESS_BAR,)\
	CG(VIDEO_RECENT_FILES,)\
	CG(VIDEO_VOLUME,)\
	CG(VIDEO_WINDOW_SIZE,)\
	CG(VIDEO_VISUAL_WARNINGS_OFF,)\
	CG(VSFILTER_INSTANCE,)\
	CG(WINDOW_MAXIMIZED,)\
	CG(WINDOW_POSITION,)\
	CG(WINDOW_SIZE,)\
	CG(MONITOR_POSITION,)\
	CG(MONITOR_SIZE,)\
	CG(DONT_SHOW_CRASH_INFO,)\
	CG(EDITBOX_TAG_BUTTONS,)\
	CG(EDITBOX_TAG_BUTTON_VALUE1,)\
	CG(EDITBOX_TAG_BUTTON_VALUE2,)\
	CG(EDITBOX_TAG_BUTTON_VALUE3,)\
	CG(EDITBOX_TAG_BUTTON_VALUE4,)\
	CG(EDITBOX_TAG_BUTTON_VALUE5,)\
	CG(EDITBOX_TAG_BUTTON_VALUE6,)\
	CG(EDITBOX_TAG_BUTTON_VALUE7,)\
	CG(EDITBOX_TAG_BUTTON_VALUE8,)\
	CG(EDITBOX_TAG_BUTTON_VALUE9,)\
	CG(EDITBOX_TAG_BUTTON_VALUE10,)\
	CG(EDITBOX_TAG_BUTTON_VALUE11,)\
	CG(EDITBOX_TAG_BUTTON_VALUE12,)\
	CG(EDITBOX_TAG_BUTTON_VALUE13,)\
	CG(EDITBOX_TAG_BUTTON_VALUE14,)\
	CG(EDITBOX_TAG_BUTTON_VALUE15,)\
	CG(EDITBOX_TAG_BUTTON_VALUE16,)\
	CG(EDITBOX_TAG_BUTTON_VALUE17,)\
	CG(EDITBOX_TAG_BUTTON_VALUE18,)\
	CG(EDITBOX_TAG_BUTTON_VALUE19,)\
	CG(EDITBOX_TAG_BUTTON_VALUE20,)\
	//if you write here a new enum then change configSize below after colors

DECLARE_ENUM(CONFIG, CFG)

#define CLR(CR) \
	CR(WINDOW_BACKGROUND,=1)\
	CR(WINDOW_BACKGROUND_INACTIVE,)\
	CR(WINDOW_TEXT,)\
	CR(WINDOW_TEXT_INACTIVE,)\
	CR(WINDOW_BORDER,)\
	CR(WINDOW_BORDER_INACTIVE,)\
	CR(WINDOW_BORDER_BACKGROUND,)\
	CR(WINDOW_BORDER_BACKGROUND_INACTIVE,)\
	CR(WINDOW_HEADER_TEXT,)\
	CR(WINDOW_HEADER_TEXT_INACTIVE,)\
	CR(WINDOW_HOVER_HEADER_ELEMENT,)\
	CR(WINDOW_PUSHED_HEADER_ELEMENT,)\
	CR(WINDOW_HOVER_CLOSE_BUTTON,)\
	CR(WINDOW_PUSHED_CLOSE_BUTTON,)\
	CR(WINDOW_WARNING_ELEMENTS,)\
	CR(GRID_TEXT,)\
	CR(GRID_BACKGROUND,)\
	CR(GRID_DIALOGUE,)\
	CR(GRID_COMMENT,)\
	CR(GRID_SELECTION,)\
	CR(GRID_LINE_VISIBLE_ON_VIDEO,)\
	CR(GRID_COLLISIONS,)\
	CR(GRID_LINES,)\
	CR(GRID_ACTIVE_LINE,)\
	CR(GRID_HEADER,)\
	CR(GRID_HEADER_TEXT,)\
	CR(GRID_LABEL_NORMAL,)\
	CR(GRID_LABEL_MODIFIED,)\
	CR(GRID_LABEL_SAVED,)\
	CR(GRID_LABEL_DOUBTFUL,)\
	CR(GRID_SPELLCHECKER,)\
	CR(GRID_COMPARISON_OUTLINE,)\
	CR(GRID_COMPARISON_BACKGROUND_NOT_MATCH,)\
	CR(GRID_COMPARISON_BACKGROUND_MATCH,)\
	CR(GRID_COMPARISON_COMMENT_BACKGROUND_NOT_MATCH,)\
	CR(GRID_COMPARISON_COMMENT_BACKGROUND_MATCH,)\
	CR(EDITOR_TEXT,)\
	CR(EDITOR_TAG_NAMES,)\
	CR(EDITOR_TAG_VALUES,)\
	CR(EDITOR_CURLY_BRACES,)\
	CR(EDITOR_TAG_OPERATORS,)\
	CR(EDITOR_SPLIT_LINES_AND_DRAWINGS,)\
	CR(EDITOR_TEMPLATE_VARIABLES,)\
	CR(EDITOR_TEMPLATE_CODE_MARKS,)\
	CR(EDITOR_TEMPLATE_FUNCTIONS,)\
	CR(EDITOR_TEMPLATE_KEYWORDS,)\
	CR(EDITOR_TEMPLATE_STRINGS,)\
	CR(EDITOR_PHRASE_SEARCH,)\
	CR(EDITOR_BRACES_BACKGROUND,)\
	CR(EDITOR_BACKGROUND,)\
	CR(EDITOR_SELECTION,)\
	CR(EDITOR_SELECTION_NO_FOCUS,)\
	CR(EDITOR_BORDER,)\
	CR(EDITOR_BORDER_ON_FOCUS,)\
	CR(EDITOR_SPELLCHECKER,)\
	CR(AUDIO_BACKGROUND,)\
	CR(AUDIO_LINE_BOUNDARY_START,)\
	CR(AUDIO_LINE_BOUNDARY_END,)\
	CR(AUDIO_LINE_BOUNDARY_MARK,)\
	CR(AUDIO_LINE_BOUNDARY_INACTIVE_LINE,)\
	CR(AUDIO_PLAY_CURSOR,)\
	CR(AUDIO_SECONDS_BOUNDARIES,)\
	CR(AUDIO_KEYFRAMES,)\
	CR(AUDIO_SYLLABLE_BOUNDARIES,)\
	CR(AUDIO_SYLLABLE_TEXT,)\
	CR(AUDIO_SELECTION_BACKGROUND,)\
	CR(AUDIO_SELECTION_BACKGROUND_MODIFIED,)\
	CR(AUDIO_INACTIVE_LINES_BACKGROUND,)\
	CR(AUDIO_WAVEFORM,)\
	CR(AUDIO_WAVEFORM_INACTIVE,)\
	CR(AUDIO_WAVEFORM_MODIFIED,)\
	CR(AUDIO_WAVEFORM_SELECTED,)\
	CR(AUDIO_SPECTRUM_BACKGROUND,)\
	CR(AUDIO_SPECTRUM_ECHO,)\
	CR(AUDIO_SPECTRUM_INNER,)\
	CR(TEXT_FIELD_BACKGROUND,)\
	CR(TEXT_FIELD_BORDER,)\
	CR(TEXT_FIELD_BORDER_ON_FOCUS,)\
	CR(TEXT_FIELD_SELECTION,)\
	CR(TEXT_FIELD_SELECTION_NO_FOCUS,)\
	CR(BUTTON_BACKGROUND,)\
	CR(BUTTON_BACKGROUND_HOVER,)\
	CR(BUTTON_BACKGROUND_PUSHED,)\
	CR(BUTTON_BACKGROUND_ON_FOCUS,)\
	CR(BUTTON_BORDER,)\
	CR(BUTTON_BORDER_HOVER,)\
	CR(BUTTON_BORDER_PUSHED,)\
	CR(BUTTON_BORDER_ON_FOCUS,)\
	CR(BUTTON_BORDER_INACTIVE,)\
	CR(TOGGLE_BUTTON_BACKGROUND_TOGGLED,)\
	CR(TOGGLE_BUTTON_BORDER_TOGGLED,)\
	CR(SCROLLBAR_BACKGROUND,)\
	CR(SCROLLBAR_THUMB,)\
	CR(SCROLLBAR_THUMB_HOVER,)\
	CR(SCROLLBAR_THUMB_PUSHED,)\
	CR(STATICBOX_BORDER,)\
	CR(STATICLIST_BORDER,)\
	CR(STATICLIST_BACKGROUND,)\
	CR(STATICLIST_SELECTION,)\
	CR(STATICLIST_BACKGROUND_HEADLINE,)\
	CR(STATICLIST_TEXT_HEADLINE,)\
	CR(STATUSBAR_BORDER,)\
	CR(MENUBAR_BACKGROUND1,)\
	CR(MENUBAR_BACKGROUND2,)\
	CR(MENUBAR_BORDER_SELECTION,)\
	CR(MENUBAR_BACKGROUND_HOVER,)\
	CR(MENUBAR_BACKGROUND_SELECTION,)\
	CR(MENUBAR_BACKGROUND,)\
	CR(MENU_BORDER_SELECTION,)\
	CR(MENU_BACKGROUND_SELECTION,)\
	CR(TABSBAR_BACKGROUND1,)\
	CR(TABSBAR_BACKGROUND2,)\
	CR(TABS_BORDER_ACTIVE,)\
	CR(TABS_BORDER_INACTIVE,)\
	CR(TABS_BACKGROUND_ACTIVE,)\
	CR(TABS_BACKGROUND_INACTIVE,)\
	CR(TABS_BACKGROUND_INACTIVE_HOVER,)\
	CR(TABS_BACKGROUND_SECOND_WINDOW,)\
	CR(TABS_TEXT_ACTIVE,)\
	CR(TABS_TEXT_INACTIVE,)\
	CR(TABS_CLOSE_HOVER,)\
	CR(TABSBAR_ARROW,)\
	CR(TABSBAR_ARROW_BACKGROUND,)\
	CR(TABSBAR_ARROW_BACKGROUND_HOVER,)\
	CR(SLIDER_PATH_BACKGROUND,)\
	CR(SLIDER_PATH_BORDER,)\
	CR(SLIDER_BORDER,)\
	CR(SLIDER_BORDER_HOVER,)\
	CR(SLIDER_BORDER_PUSHED,)\
	CR(SLIDER_BACKGROUND,)\
	CR(SLIDER_BACKGROUND_HOVER,)\
	CR(SLIDER_BACKGROUND_PUSHED,)\
	CR(WINDOW_RESIZER_DOTS,)\
	CR(FIND_RESULT_FILENAME_FOREGROUND,)\
	CR(FIND_RESULT_FILENAME_BACKGROUND,)\
	CR(FIND_RESULT_FOUND_PHRASE_FOREGROUND,)\
	CR(FIND_RESULT_FOUND_PHRASE_BACKGROUND,)\
	CR(STYLE_PREVIEW_COLOR1,)\
	CR(STYLE_PREVIEW_COLOR2,)\
	//if you write here a new enum then change colorsSize below
DECLARE_ENUM(COLOR, CLR)

//typedef void csri_rend;

class config
{
private:
	//int to silence warnings
	static const int configSize = EDITBOX_TAG_BUTTON_VALUE20 + 1;
	wxString stringConfig[configSize];
	static const int colorsSize = STYLE_PREVIEW_COLOR2 + 1;
	wxColour colors[colorsSize];
	bool isClosing = false;
	std::map<int, wxFont*> programFonts;
	std::map<wxString, wxString> Languages;
	wxColour defaultColour;
	void InitLanguagesTable();
	bool hasCrashed = false;
	bool ConfigNeedToConvert(const wxString & fullVersion);
	int lastCheckedId = -1;
	unsigned long lastCheckedTime = 0;
	int fontDPI = 1.f;
public:
	std::vector<Styles*> assstore;
	wxString progname;
	//actual style catalog --- path to program exe
	wxString actualStyleDir, pathfull, configPath;
	wxArrayString dirs;
	bool AudioOpts;

	void SetClosing(){ isClosing = true; }
	bool GetClosing(){ return isClosing; }

	const wxString &GetString(CONFIG opt);
	bool GetBool(CONFIG opt);
	const wxColour &GetColour(COLOR opt);
	AssColor GetColor(COLOR opt);
	int GetInt(CONFIG opt);
	float GetFloat(CONFIG opt);
	void GetTable(CONFIG opt, wxArrayString &tbl, int mode = 4);
	void GetIntTable(CONFIG opt, wxArrayInt &tbl, int mode = 4);
	void GetTableFromString(CONFIG opt, wxArrayString &tbl, wxString split, int mode = 4);
	void GetCoords(CONFIG opt, int* coordx, int* coordy);

	void SetString(CONFIG opt, const wxString &sopt);
	void SetBool(CONFIG opt, bool bopt);
	void SetColour(COLOR opt, wxColour &copt);
	void SetColor(COLOR opt, AssColor &copt);
	void SetInt(CONFIG opt, int iopt);
	void SetFloat(CONFIG opt, float fopt);
	void SetTable(CONFIG opt, wxArrayString &iopt);
	void SetStringTable(CONFIG opt, wxArrayString &iopt, wxString split = L"|");
	void SetIntTable(CONFIG opt, wxArrayInt &iopt);
	void SetCoords(CONFIG opt, int coordx, int coordy);
	void GetRawOptions(wxString &options, bool Audio = false);
	void AddStyle(Styles *styl);
	void ChangeStyle(Styles *styl, int i);
	Styles *GetStyle(int i, const wxString &name = emptyString, Styles* styl = NULL);
	int FindStyle(const wxString &name, int *multiplication = NULL);
	void DelStyle(int i);
	int StoreSize();
	void CatchValsLabs(const wxString &rawoptions);
	bool SetRawOptions(const wxString &textconfig);
	int LoadOptions();
	void LoadColors(const wxString &themeName = emptyString);
	//if you want to use defaultOptions alocate table for configSize
	//use with AudioConfig
	void LoadDefaultConfig(wxString * defaultOptions = NULL);
	void LoadDefaultColors(bool dark = true, wxColour *table = NULL);
	//if you want to use defaultOptions alocate table for configSize
	//use with AudioConfig
	void LoadDefaultAudioConfig(wxString * defaultOptions = NULL);
	void LoadMissingColours(const wxString &path);
	bool LoadAudioOpts();
	void ResetDefault();
	void SaveAudioOpts();
	void SaveOptions(bool cfg = true, bool style = true, bool crashed = false);
	void SaveColors(const wxString &path = emptyString);
	void LoadStyles(const wxString &katalog);
	void clearstyles();
	void Sortstyles();
	void SetHexColor(const wxString &nameAndColor);
	//if failed returns symbol
	const wxString & FindLanguage(const wxString & symbol);
	//main value is 10 offset from 10
	wxFont *GetFont(int offset = 0);
	void FontsClear();
	int GetDPI();
	void FontsRescale(int dpi);
	//wxString GetStringColor(unsigned int);
	wxString GetStringColor(size_t optionName);
	wxString GetReleaseDate();
	bool HasCrashed(){ return hasCrashed; }
	//returns true when time is too short
	bool CheckLastKeyEvent(int id, int timeInterval = 100);
	config();
	~config();
};

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if (x !=NULL) { delete x; x = NULL; }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != NULL) { x->Release(); x = NULL; } 
#endif



#ifndef PTR
#define PTR(what,err) if(!what) {KaiLogSilent(err); return false;}
#endif

#ifndef PTR1
#define PTR1(what,err) if(!what) {KaiLogSilent(err); return;}
#endif

#ifndef HR
#define HR(what,err) if(FAILED(what)) {KaiLogSilent(err); return false;}
#endif

#ifndef HRN
#define HRN(what,err) if(FAILED(what)) {KaiLogSilent(err); return;}
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef MID
#define MID(a,b,c) MAX((a),MIN((b),(c)))
#endif


#undef wxBITMAP_PNG

inline wxColour GetColorWithAlpha(const wxColour& colorWithAlpha, const wxColour& background)
{
	int r = colorWithAlpha.Red(), g = colorWithAlpha.Green(), b = colorWithAlpha.Blue();
	int r2 = background.Red(), g2 = background.Green(), b2 = background.Blue();
	int inv_a = 0xFF - colorWithAlpha.Alpha();
	int fr = (r2 * inv_a / 0xFF) + (r - inv_a * r / 0xFF);
	int fg = (g2 * inv_a / 0xFF) + (g - inv_a * g / 0xFF);
	int fb = (b2 * inv_a / 0xFF) + (b - inv_a * b / 0xFF);
	return wxColour(fr, fg, fb);
}

inline wxString GetTruncateText(const wxString& textToTruncate, int width, wxWindow* window)
{
	int w, h;
	window->GetTextExtent(textToTruncate, &w, &h);
	if (w > width) {
		size_t len = textToTruncate.length() - 1;
		while (w > width && len > 3) {
			window->GetTextExtent(textToTruncate.SubString(0, len), &w, &h);
			len--;
		}
		return textToTruncate.SubString(0, len - 2i64) + L"...";
	}
	return textToTruncate;
}

void SelectInFolder(const wxString& filename);

void OpenInBrowser(const wxString& adress);

bool IsNumberFloat(const wxString& test);

bool sortfunc(Styles* styl1, Styles* styl2);
//formating here works like this, 
//first digit - digits before dot, second digit - digits after dot, for example 5.3f;
wxString getfloat(float num, const wxString& format = L"5.3f", bool Truncate = true);
wxBitmap CreateBitmapFromPngResource(const wxString& t_name);
wxBitmap* CreateBitmapPointerFromPngResource(const wxString& t_name);
wxImage CreateImageFromPngResource(const wxString& t_name);
#define wxBITMAP_PNG(x) CreateBitmapFromPngResource(x)
#define PTR_BITMAP_PNG(x) CreateBitmapPointerFromPngResource(x)
void MoveToMousePosition(wxWindow* win);
wxString MakePolishPlural(int num, const wxString& normal, const wxString& plural24, const wxString& pluralRest);


bool IsNumber(const wxString& txt);
void DrawDashedLine(wxDC* dc, wxPoint* vector, size_t vectorSize, int dashLen, const wxColour& color);
size_t FindFromEnd(const wxString& text, const wxString& whatToFind, bool ignoreCase = false);

extern config Options;

//#define TEST_FFMPEG