//  Copyright (c) 2020, Marcin Drob

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

#include "ConfigConverter.h"
#include "Config.h"
#include <wx/regex.h>

void ConfigConverter::CreateTable()
{
	//config
	convertConfig[L"AudioAutoCommit"] = std::pair<wxString, wxString>(L"AUDIO_AUTO_COMMIT", L"");
	convertConfig[L"AudioAutoFocus"] = std::pair<wxString, wxString>(L"AUDIO_AUTO_FOCUS", L"");
	convertConfig[L"AudioAutoScroll"] = std::pair<wxString, wxString>(L"AUDIO_AUTO_SCROLL", L"");
	convertConfig[L"AudioBoxHeight"] = std::pair<wxString, wxString>(L"AUDIO_BOX_HEIGHT", L"");
	convertConfig[L"AudioDelay"] = std::pair<wxString, wxString>(L"AUDIO_DELAY", L"");
	convertConfig[L"AudioDrawKeyframes"] = std::pair<wxString, wxString>(L"AUDIO_DRAW_KEYFRAMES", L"");
	convertConfig[L"AudioDrawSecondaryLines"] = std::pair<wxString, wxString>(L"AUDIO_DRAW_SECONDARY_LINES", L"");
	convertConfig[L"AudioDrawSelectionBackground"] = std::pair<wxString, wxString>(L"AUDIO_DRAW_SELECTION_BACKGROUND", L"");
	convertConfig[L"AudioDrawTimeCursor"] = std::pair<wxString, wxString>(L"AUDIO_DRAW_TIME_CURSOR", L"");
	convertConfig[L"AudioDrawVideoPosition"] = std::pair<wxString, wxString>(L"AUDIO_DRAW_VIDEO_POSITION", L"");
	convertConfig[L"AudioGrabTimesOnSelect"] = std::pair<wxString, wxString>(L"AUDIO_GRAB_TIMES_ON_SELECT", L"");
	convertConfig[L"AudioHorizontalZoom"] = std::pair<wxString, wxString>(L"AUDIO_HORIZONTAL_ZOOM", L"");
	convertConfig[L"AudioInactiveLinesDisplayMode"] = std::pair<wxString, wxString>(L"AUDIO_INACTIVE_LINES_DISPLAY_MODE", L"");
	convertConfig[L"AudioKaraoke"] = std::pair<wxString, wxString>(L"AUDIO_KARAOKE", L"");
	convertConfig[L"AudioKaraokeMoveOnClick"] = std::pair<wxString, wxString>(L"AUDIO_KARAOKE_MOVE_ON_CLICK", L"");
	convertConfig[L"AudioKaraokeSplitMode"] = std::pair<wxString, wxString>(L"AUDIO_KARAOKE_SPLIT_MODE", L"");
	convertConfig[L"AudioLeadIn"] = std::pair<wxString, wxString>(L"AUDIO_LEAD_IN_VALUE", L"");
	convertConfig[L"AudioLeadOut"] = std::pair<wxString, wxString>(L"AUDIO_LEAD_OUT_VALUE", L"");
	convertConfig[L"AudioLineBoundariesThickness"] = std::pair<wxString, wxString>(L"AUDIO_LINE_BOUNDARIES_THICKNESS", L"");
	convertConfig[L"AudioLink"] = std::pair<wxString, wxString>(L"AUDIO_LINK", L"");
	convertConfig[L"AudioLockScrollOnCursor"] = std::pair<wxString, wxString>(L"AUDIO_LOCK_SCROLL_ON_CURSOR", L"");
	convertConfig[L"AudioMarkPlayTime"] = std::pair<wxString, wxString>(L"AUDIO_MARK_PLAY_TIME", L"");
	convertConfig[L"AudioMergeEveryNWithSyllable"] = std::pair<wxString, wxString>(L"AUDIO_MERGE_EVERY_N_WITH_SYLLABLE", L"");
	convertConfig[L"AudioNextLineOnCommit"] = std::pair<wxString, wxString>(L"AUDIO_NEXT_LINE_ON_COMMIT", L"");
	convertConfig[L"AudioRAMCache"] = std::pair<wxString, wxString>(L"AUDIO_RAM_CACHE", L"");
	convertConfig[L"AudioSnapToKeyframes"] = std::pair<wxString, wxString>(L"AUDIO_SNAP_TO_KEYFRAMES", L"");
	convertConfig[L"AudioSnapToOtherLines"] = std::pair<wxString, wxString>(L"AUDIO_SNAP_TO_OTHER_LINES", L"");
	convertConfig[L"AudioSpectrumOn"] = std::pair<wxString, wxString>(L"AUDIO_SPECTRUM_ON", L"");
	convertConfig[L"AudioSpectrumNonLinearOn"] = std::pair<wxString, wxString>(L"AUDIO_SPECTRUM_NON_LINEAR_ON", L"");
	convertConfig[L"AudioStartDragSensitivity"] = std::pair<wxString, wxString>(L"AUDIO_START_DRAG_SENSITIVITY", L"");
	convertConfig[L"AudioVerticalZoom"] = std::pair<wxString, wxString>(L"AUDIO_VERTICAL_ZOOM", L"");
	convertConfig[L"AudioVolume"] = std::pair<wxString, wxString>(L"AUDIO_VOLUME", L"");
	convertConfig[L"AudioWheelDefaultToZoom"] = std::pair<wxString, wxString>(L"AUDIO_WHEEL_DEFAULT_TO_ZOOM", L"");
	convertConfig[L"AcceptedAudioStream"] = std::pair<wxString, wxString>(L"ACCEPTED_AUDIO_STREAM", L"");
	convertConfig[L"ASSPropertiesTitle"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_TITLE", L"");
	convertConfig[L"ASSPropertiesScript"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_SCRIPT", L"");
	convertConfig[L"ASSPropertiesTranslation"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_TRANSLATION", L"");
	convertConfig[L"ASSPropertiesEditing"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_EDITING", L"");
	convertConfig[L"ASSPropertiesTiming"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_TIMING", L"");
	convertConfig[L"ASSPropertiesUpdate"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_UPDATE", L"");
	convertConfig[L"ASSPropertiesTitleOn"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_TITLE_ON", L"");
	convertConfig[L"ASSPropertiesScriptOn"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_SCRIPT_ON", L"");
	convertConfig[L"ASSPropertiesTranslationOn"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_TRANSLATION_ON", L"");
	convertConfig[L"ASSPropertiesEditingOn"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_EDITING_ON", L"");
	convertConfig[L"ASSPropertiesTimingOn"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_TIMING_ON", L"");
	convertConfig[L"ASSPropertiesUpdateOn"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_UPDATE_ON", L"");
	convertConfig[L"ASSPropertiesAskForChange"] = std::pair<wxString, wxString>(L"ASS_PROPERTIES_ASK_FOR_CHANGE", L"");
	convertConfig[L"AudioRecent"] = std::pair<wxString, wxString>(L"AUDIO_RECENT_FILES", L"|");
	convertConfig[L"AutomationLoadingMethod"] = std::pair<wxString, wxString>(L"AUTOMATION_LOADING_METHOD", L"");
	convertConfig[L"AutomationOldScriptsCompatybility"] = std::pair<wxString, wxString>(L"AUTOMATION_OLD_SCRIPTS_COMPATIBILITY", L"");
	convertConfig[L"AutomationRecent"] = std::pair<wxString, wxString>(L"AUTOMATION_RECENT_FILES", L"");
	convertConfig[L"AutomationScriptEditor"] = std::pair<wxString, wxString>(L"AUTOMATION_SCRIPT_EDITOR", L"");
	convertConfig[L"AutomationTraceLevel"] = std::pair<wxString, wxString>(L"AUTOMATION_TRACE_LEVEL", L"");
	convertConfig[L"AutoMoveTagsFromOriginal"] = std::pair<wxString, wxString>(L"AUTO_MOVE_TAGS_FROM_ORIGINAL", L"");
	convertConfig[L"AutoSaveMaxFiles"] = std::pair<wxString, wxString>(L"AUTOSAVE_MAX_FILES", L"");
	convertConfig[L"AutoSelectLinesFromLastTab"] = std::pair<wxString, wxString>(L"AUTO_SELECT_LINES_FROM_LAST_TAB", L"");
	convertConfig[L"ColorpickerRecent"] = std::pair<wxString, wxString>(L"COLORPICKER_RECENT_COLORS", L"");
	convertConfig[L"ConvertASSTagsOnLineStart"] = std::pair<wxString, wxString>(L"CONVERT_ASS_TAGS_TO_INSERT_IN_LINE", L"");
	convertConfig[L"ConvertFPS"] = std::pair<wxString, wxString>(L"CONVERT_FPS", L"");
	convertConfig[L"ConvertFPSFromVideo"] = std::pair<wxString, wxString>(L"CONVERT_FPS_FROM_VIDEO", L"");
	convertConfig[L"ConvertNewEndTimes"] = std::pair<wxString, wxString>(L"CONVERT_NEW_END_TIMES", L"");
	convertConfig[L"ConvertResolutionWidth"] = std::pair<wxString, wxString>(L"CONVERT_RESOLUTION_WIDTH", L"");
	convertConfig[L"ConvertResolutionHeight"] = std::pair<wxString, wxString>(L"CONVERT_RESOLUTION_HEIGHT", L"");
	convertConfig[L"ConvertShowSettings"] = std::pair<wxString, wxString>(L"CONVERT_SHOW_SETTINGS", L"");
	convertConfig[L"ConvertStyle"] = std::pair<wxString, wxString>(L"CONVERT_STYLE", L"");
	convertConfig[L"ConvertStyleCatalog"] = std::pair<wxString, wxString>(L"CONVERT_STYLE_CATALOG", L"");
	convertConfig[L"ConvertTimePerLetter"] = std::pair<wxString, wxString>(L"CONVERT_TIME_PER_CHARACTER", L"");
	convertConfig[L"CopyCollumnsSelection"] = std::pair<wxString, wxString>(L"COPY_COLLUMS_SELECTIONS", L"");
	convertConfig[L"DictionaryLanguage"] = std::pair<wxString, wxString>(L"DICTIONARY_LANGUAGE", L"");
	convertConfig[L"DisableLiveVideoEditing"] = std::pair<wxString, wxString>(L"DISABLE_LIVE_VIDEO_EDITING", L"");
	convertConfig[L"DontAskForBadResolution"] = std::pair<wxString, wxString>(L"DONT_ASK_FOR_BAD_RESOLUTION", L"");
	convertConfig[L"EditboxSugestionsOnDoubleClick"] = std::pair<wxString, wxString>(L"EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK", L"");
	convertConfig[L"EditorOn"] = std::pair<wxString, wxString>(L"EDITOR_ON", L"");
	convertConfig[L"FIND_IN_SUBS_FILTERS_RECENT"] = std::pair<wxString, wxString>(L"FIND_IN_SUBS_FILTERS_RECENT", L"\f");
	convertConfig[L"FIND_IN_SUBS_PATHS_RECENT"] = std::pair<wxString, wxString>(L"FIND_IN_SUBS_PATHS_RECENT", L"\f");
	convertConfig[L"FindRecent"] = std::pair<wxString, wxString>(L"FIND_RECENT_FINDS", L"\f");
	convertConfig[L"FindReplaceOptions"] = std::pair<wxString, wxString>(L"FIND_REPLACE_OPTIONS", L"");
	convertConfig[L"FontCollectorAction"] = std::pair<wxString, wxString>(L"FONT_COLLECTOR_ACTION", L"");
	convertConfig[L"FontCollectorDirectory"] = std::pair<wxString, wxString>(L"FONT_COLLECTOR_DIRECTORY", L"");
	convertConfig[L"FontCollectorFromMKV"] = std::pair<wxString, wxString>(L"FONT_COLLECTOR_FROM_MKV", L"");
	convertConfig[L"FontCollectorUseSubsDirectory"] = std::pair<wxString, wxString>(L"FONT_COLLECTOR_USE_SUBS_DIRECTORY", L"");
	convertConfig[L"FFMS2VideoSeeking"] = std::pair<wxString, wxString>(L"FFMS2_VIDEO_SEEKING", L"");
	convertConfig[L"GridChangeActiveOnSelection"] = std::pair<wxString, wxString>(L"GRID_CHANGE_ACTIVE_ON_SELECTION", L"");
	convertConfig[L"GridFontName"] = std::pair<wxString, wxString>(L"GRID_FONT", L"");
	convertConfig[L"GridFontSize"] = std::pair<wxString, wxString>(L"GRID_FONT_SIZE", L"");
	convertConfig[L"GridAddToFilter"] = std::pair<wxString, wxString>(L"GRID_ADD_TO_FILTER", L"");
	convertConfig[L"GridFilterAfterLoad"] = std::pair<wxString, wxString>(L"GRID_FILTER_AFTER_LOAD", L"");
	convertConfig[L"GridFilterBy"] = std::pair<wxString, wxString>(L"GRID_FILTER_BY", L"");
	convertConfig[L"GridFilterInverted"] = std::pair<wxString, wxString>(L"GRID_FILTER_INVERTED", L"");
	convertConfig[L"GridFilterStyles"] = std::pair<wxString, wxString>(L"GRID_FILTER_STYLES", L",");
	convertConfig[L"GridHideCollums"] = std::pair<wxString, wxString>(L"GRID_HIDE_COLUMNS", L"");
	convertConfig[L"GridHideTags"] = std::pair<wxString, wxString>(L"GRID_HIDE_TAGS", L"");
	convertConfig[L"GridIgnoreFiltering"] = std::pair<wxString, wxString>(L"GRID_IGNORE_FILTERING", L"");
	convertConfig[L"GridLoadSortedSubs"] = std::pair<wxString, wxString>(L"GRID_LOAD_SORTED_SUBS", L"");
	convertConfig[L"GridSaveAfterCharacterCount"] = std::pair<wxString, wxString>(L"GRID_SAVE_AFTER_CHARACTER_COUNT", L"");
	convertConfig[L"GridTagsSwapChar"] = std::pair<wxString, wxString>(L"GRID_TAGS_SWAP_CHARACTER", L"");
	convertConfig[L"InsertEndOffset"] = std::pair<wxString, wxString>(L"GRID_INSERT_END_OFFSET", L"");
	convertConfig[L"InsertStartOffset"] = std::pair<wxString, wxString>(L"GRID_INSERT_START_OFFSET", L"");
	convertConfig[L"KEYFRAMES_RECENT"] = std::pair<wxString, wxString>(L"KEYFRAMES_RECENT", L"|");
	convertConfig[L"MoveTimesByTime"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_BY_TIME", L"");
	convertConfig[L"MoveTimesLoadSetTabOptions"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_CHANGE_VALUES_WITH_TAB", L"");
	convertConfig[L"MoveTimesCorrectEndTimes"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_CORRECT_END_TIMES", L"");
	convertConfig[L"MoveTimesForward"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_MOVE_FORWARD", L"");
	convertConfig[L"MoveTimesFrames"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_DISPLAY_FRAMES", L"");
	convertConfig[L"MoveTimesOn"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_ON", L"");
	convertConfig[L"MoveTimesOptions"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_OPTIONS", L"");
	convertConfig[L"MoveTimesWhichLines"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_WHICH_LINES", L"");
	convertConfig[L"MoveTimesWhichTimes"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_WHICH_TIMES", L"");
	convertConfig[L"MoveTimesStyles"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_STYLES", L"");
	convertConfig[L"MoveTimesTime"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_TIME", L"");
	convertConfig[L"MoveVideoToActiveLine"] = std::pair<wxString, wxString>(L"MOVE_VIDEO_TO_ACTIVE_LINE", L"");
	convertConfig[L"NoNewLineAfterTimesEdition"] = std::pair<wxString, wxString>(L"EDITBOX_DONT_GO_TO_NEXT_LINE_ON_TIMES_EDIT", L"");
	convertConfig[L"OpenSubsInNewCard"] = std::pair<wxString, wxString>(L"OPEN_SUBS_IN_NEW_TAB", L"");
	convertConfig[L"OpenVideoAtActiveLine"] = std::pair<wxString, wxString>(L"OPEN_VIDEO_AT_ACTIVE_LINE", L"");
	convertConfig[L"PasteCollumnsSelection"] = std::pair<wxString, wxString>(L"PASTE_COLUMNS_SELECTION", L"");
	convertConfig[L"PlayAfterSelection"] = std::pair<wxString, wxString>(L"VIDEO_PLAY_AFTER_SELECTION", L"");
	convertConfig[L"PostprocessorEnabling"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_ON", L"");
	convertConfig[L"PostprocessorKeyframeBeforeStart"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_KEYFRAME_BEFORE_START", L"");
	convertConfig[L"PostprocessorKeyframeAfterStart"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_KEYFRAME_AFTER_START", L"");
	convertConfig[L"PostprocessorKeyframeBeforeEnd"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_KEYFRAME_BEFORE_END", L"");
	convertConfig[L"PostprocessorKeyframeAfterEnd"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_KEYFRAME_AFTER_END", L"");
	convertConfig[L"PostprocessorLeadIn"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_LEAD_IN", L"");
	convertConfig[L"PostprocessorLeadOut"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_LEAD_OUT", L"");
	convertConfig[L"PostprocessorThresholdStart"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_THRESHOLD_START", L"");
	convertConfig[L"PostprocessorThresholdEnd"] = std::pair<wxString, wxString>(L"POSTPROCESSOR_THRESHOLD_END", L"");
	convertConfig[L"PreviewText"] = std::pair<wxString, wxString>(L"STYLE_PREVIEW_TEXT", L"");
	convertConfig[L"ProgramLanguage"] = std::pair<wxString, wxString>(L"PROGRAM_LANGUAGE", L"");
	convertConfig[L"ProgramTheme"] = std::pair<wxString, wxString>(L"PROGRAM_THEME", L"");
	convertConfig[L"ReplaceRecent"] = std::pair<wxString, wxString>(L"REPLACE_RECENT_REPLACEMENTS", L"\f");
	convertConfig[L"SelectionsRecent"] = std::pair<wxString, wxString>(L"SELECT_LINES_RECENT_SELECTIONS", L"\f");
	convertConfig[L"SelectionsOptions"] = std::pair<wxString, wxString>(L"SELECT_LINES_OPTIONS", L"");
	convertConfig[L"SelectVisibleLineAfterFullscreen"] = std::pair<wxString, wxString>(L"GRID_SET_VISIBLE_LINE_AFTER_FULL_SCREEN", L"");
	convertConfig[L"SHIFT_TIMES_PROFILES"] = std::pair<wxString, wxString>(L"SHIFT_TIMES_PROFILES", L"\f");
	convertConfig[L"SpellcheckerOn"] = std::pair<wxString, wxString>(L"SPELLCHECKER_ON", L"");
	convertConfig[L"StyleEditFilterText"] = std::pair<wxString, wxString>(L"STYLE_EDIT_FILTER_TEXT", L"");
	convertConfig[L"StyleFilterTextOn"] = std::pair<wxString, wxString>(L"STYLE_EDIT_FILTER_TEXT_ON", L"");
	convertConfig[L"StyleManagerPosition"] = std::pair<wxString, wxString>(L"STYLE_MANAGER_POSITION", L"");
	convertConfig[L"StyleManagerDetachEditor"] = std::pair<wxString, wxString>(L"STYLE_MANAGER_DETACH_EDIT_WINDOW", L"");
	convertConfig[L"SubsAutonaming"] = std::pair<wxString, wxString>(L"SUBS_AUTONAMING", L"");
	convertConfig[L"SubsComparisonType"] = std::pair<wxString, wxString>(L"SUBS_COMPARISON_TYPE", L"");
	convertConfig[L"SubsComparisonStyles"] = std::pair<wxString, wxString>(L"SUBS_COMPARISON_STYLES", L"");
	convertConfig[L"SubsRecent"] = std::pair<wxString, wxString>(L"SUBS_RECENT_FILES", L"|");
	convertConfig[L"TextFieldAllowNumpadHotkeys"] = std::pair<wxString, wxString>(L"TEXT_FIELD_ALLOW_NUMPAD_HOTKEYS", L"");
	convertConfig[L"TlModeShowOriginal"] = std::pair<wxString, wxString>(L"TL_MODE_SHOW_ORIGINAL", L"");
	convertConfig[L"ToolbarIDs"] = std::pair<wxString, wxString>(L"TOOLBAR_IDS", L"|");
	convertConfig[L"ToolbarAlignment"] = std::pair<wxString, wxString>(L"TOOLBAR_ALIGNMENT", L"");
	convertConfig[L"UpdaterCheckIntensity"] = std::pair<wxString, wxString>(L"UPDATER_CHECK_INTENSITY", L"");
	convertConfig[L"UpdaterCheckOptions"] = std::pair<wxString, wxString>(L"UPDATER_CHECK_OPTIONS", L"");
	convertConfig[L"UpdaterCheckForStable"] = std::pair<wxString, wxString>(L"UPDATER_CHECK_FOR_STABLE", L"");
	convertConfig[L"UpdaterLastCheck"] = std::pair<wxString, wxString>(L"UPDATER_LAST_CHECK", L"");
	convertConfig[L"VideoFullskreenOnStart"] = std::pair<wxString, wxString>(L"VIDEO_FULL_SCREEN_ON_START", L"");
	convertConfig[L"VideoIndex"] = std::pair<wxString, wxString>(L"VIDEO_INDEX", L"");
	convertConfig[L"VideoPauseOnClick"] = std::pair<wxString, wxString>(L"VIDEO_PAUSE_ON_CLICK", L"");
	convertConfig[L"VideoProgressBar"] = std::pair<wxString, wxString>(L"VIDEO_PROGRESS_BAR", L"");
	convertConfig[L"VideoRecent"] = std::pair<wxString, wxString>(L"VIDEO_RECENT_FILES", L"|");
	convertConfig[L"VideoVolume"] = std::pair<wxString, wxString>(L"VIDEO_VOLUME", L"");
	convertConfig[L"VideoWindowSize"] = std::pair<wxString, wxString>(L"VIDEO_WINDOW_SIZE", L"");
	convertConfig[L"VisualWarningsOff"] = std::pair<wxString, wxString>(L"VIDEO_VISUAL_WARNINGS_OFF", L"");
	convertConfig[L"WindowMaximized"] = std::pair<wxString, wxString>(L"WINDOW_MAXIMIZED", L"");
	convertConfig[L"WindowPosition"] = std::pair<wxString, wxString>(L"WINDOW_POSITION", L"");
	convertConfig[L"WindowSize"] = std::pair<wxString, wxString>(L"WINDOW_SIZE", L"");
	convertConfig[L"EditboxTagButtons"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTONS", L"");
	convertConfig[L"EditboxTagButton1"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE1", L"\f");
	convertConfig[L"EditboxTagButton2"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE2", L"\f");
	convertConfig[L"EditboxTagButton3"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE3", L"\f");
	convertConfig[L"EditboxTagButton4"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE4", L"\f");
	convertConfig[L"EditboxTagButton5"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE5", L"\f");
	convertConfig[L"EditboxTagButton6"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE6", L"\f");
	convertConfig[L"EditboxTagButton7"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE7", L"\f");
	convertConfig[L"EditboxTagButton8"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE8", L"\f");
	convertConfig[L"EditboxTagButton9"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE9", L"\f");
	convertConfig[L"EditboxTagButton10"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE10", L"\f");
	convertConfig[L"EditboxTagButton11"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE11", L"\f");
	convertConfig[L"EditboxTagButton12"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE12", L"\f");
	convertConfig[L"EditboxTagButton13"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE13", L"\f");
	convertConfig[L"EditboxTagButton14"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE14", L"\f");
	convertConfig[L"EditboxTagButton15"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE15", L"\f");
	convertConfig[L"EditboxTagButton16"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE16", L"\f");
	convertConfig[L"EditboxTagButton17"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE17", L"\f");
	convertConfig[L"EditboxTagButton18"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE18", L"\f");
	convertConfig[L"EditboxTagButton19"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE19", L"\f");
	convertConfig[L"EditboxTagButton20"] = std::pair<wxString, wxString>(L"EDITBOX_TAG_BUTTON_VALUE20", L"\f");

	//themes
	convertColors[L"WindowBackground"] = L"WINDOW_BACKGROUND";
	convertColors[L"WindowBackgroundInactive"] = L"WINDOW_BACKGROUND_INACTIVE";
	convertColors[L"WindowText"] = L"WINDOW_TEXT";
	convertColors[L"WindowTextInactive"] = L"WINDOW_TEXT_INACTIVE";
	convertColors[L"WindowBorder"] = L"WINDOW_BORDER";
	convertColors[L"WindowBorderInactive"] = L"WINDOW_BORDER_INACTIVE";
	convertColors[L"WindowBorderBackground"] = L"WINDOW_BORDER_BACKGROUND";
	convertColors[L"WindowBorderBackgroundInactive"] = L"WINDOW_BORDER_BACKGROUND_INACTIVE";
	convertColors[L"WindowHeaderText"] = L"WINDOW_HEADER_TEXT";
	convertColors[L"WindowHeaderTextInactive"] = L"WINDOW_HEADER_TEXT_INACTIVE";
	convertColors[L"WindowHoverHeaderElement"] = L"WINDOW_HOVER_HEADER_ELEMENT";
	convertColors[L"WindowPushedHeaderElement"] = L"WINDOW_PUSHED_HEADER_ELEMENT";
	convertColors[L"WindowHoverCloseButton"] = L"WINDOW_HOVER_CLOSE_BUTTON";
	convertColors[L"WindowPushedCloseButton"] = L"WINDOW_PUSHED_CLOSE_BUTTON";
	convertColors[L"WindowWarningElements"] = L"WINDOW_WARNING_ELEMENTS";
	convertColors[L"GridText"] = L"GRID_TEXT";
	convertColors[L"GridBackground"] = L"GRID_BACKGROUND";
	convertColors[L"GridDialogue"] = L"GRID_DIALOGUE";
	convertColors[L"GridComment"] = L"GRID_COMMENT";
	convertColors[L"GridSelection"] = L"GRID_SELECTION";
	convertColors[L"GridVisibleOnVideo"] = L"GRID_LINE_VISIBLE_ON_VIDEO";
	convertColors[L"GridCollisions"] = L"GRID_COLLISIONS";
	convertColors[L"GridLines"] = L"GRID_LINES";
	convertColors[L"GridActiveLine"] = L"GRID_ACTIVE_LINE";
	convertColors[L"GridHeader"] = L"GRID_HEADER";
	convertColors[L"GridHeaderText"] = L"GRID_HEADER_TEXT";
	convertColors[L"GridLabelNormal"] = L"GRID_LABEL_NORMAL";
	convertColors[L"GridLabelModified"] = L"GRID_LABEL_MODIFIED";
	convertColors[L"GridLabelSaved"] = L"GRID_LABEL_SAVED";
	convertColors[L"GridLabelDoubtful"] = L"GRID_LABEL_DOUBTFUL";
	convertColors[L"GridSpellchecker"] = L"GRID_SPELLCHECKER";
	convertColors[L"GridComparisonOutline"] = L"GRID_COMPARISON_OUTLINE";
	convertColors[L"GridComparisonBackgroundNotMatch"] = L"GRID_COMPARISON_BACKGROUND_NOT_MATCH";
	convertColors[L"GridComparisonBackgroundMatch"] = L"GRID_COMPARISON_BACKGROUND_MATCH";
	convertColors[L"GridComparisonCommentBackgroundNotMatch"] = L"GRID_COMPARISON_COMMENT_BACKGROUND_NOT_MATCH";
	convertColors[L"GridComparisonCommentBackgroundMatch"] = L"GRID_COMPARISON_COMMENT_BACKGROUND_MATCH";
	convertColors[L"EditorText"] = L"EDITOR_TEXT";
	convertColors[L"EditorTagNames"] = L"EDITOR_TAG_NAMES";
	convertColors[L"EditorTagValues"] = L"EDITOR_TAG_VALUES";
	convertColors[L"EditorCurlyBraces"] = L"EDITOR_CURLY_BRACES";
	convertColors[L"EditorTagOperators"] = L"EDITOR_TAG_OPERATORS";
	convertColors[L"EditorTemplateVariables"] = L"EDITOR_TEMPLATE_VARIABLES";
	convertColors[L"EditorTemplateCodeMarks"] = L"EDITOR_TEMPLATE_CODE_MARKS";
	convertColors[L"EditorTemplateFunctions"] = L"EDITOR_TEMPLATE_FUNCTIONS";
	convertColors[L"EditorTemplateKeywords"] = L"EDITOR_TEMPLATE_KEYWORDS";
	convertColors[L"EditorTemplateStrings"] = L"EDITOR_TEMPLATE_STRINGS";
	convertColors[L"EditorPhraseSearch"] = L"EDITOR_PHRASE_SEARCH";
	convertColors[L"EditorBracesBackground"] = L"EDITOR_BRACES_BACKGROUND";
	convertColors[L"EditorBackground"] = L"EDITOR_BACKGROUND";
	convertColors[L"EditorSelection"] = L"EDITOR_SELECTION";
	convertColors[L"EditorSelectionNoFocus"] = L"EDITOR_SELECTION_NO_FOCUS";
	convertColors[L"EditorBorder"] = L"EDITOR_BORDER";
	convertColors[L"EditorBorderOnFocus"] = L"EDITOR_BORDER_ON_FOCUS";
	convertColors[L"EditorSpellchecker"] = L"EDITOR_SPELLCHECKER";
	convertColors[L"AudioBackground"] = L"AUDIO_BACKGROUND";
	convertColors[L"AudioLineBoundaryStart"] = L"AUDIO_LINE_BOUNDARY_START";
	convertColors[L"AudioLineBoundaryEnd"] = L"AUDIO_LINE_BOUNDARY_END";
	convertColors[L"AudioLineBoundaryMark"] = L"AUDIO_LINE_BOUNDARY_MARK";
	convertColors[L"AudioLineBoundaryInactiveLine"] = L"AUDIO_LINE_BOUNDARY_INACTIVE_LINE";
	convertColors[L"AudioPlayCursor"] = L"AUDIO_PLAY_CURSOR";
	convertColors[L"AudioSecondsBoundaries"] = L"AUDIO_SECONDS_BOUNDARIES";
	convertColors[L"AudioKeyframes"] = L"AUDIO_KEYFRAMES";
	convertColors[L"AudioSyllableBoundaries"] = L"AUDIO_SYLLABLE_BOUNDARIES";
	convertColors[L"AudioSyllableText"] = L"AUDIO_SYLLABLE_TEXT";
	convertColors[L"AudioSelectionBackground"] = L"AUDIO_SELECTION_BACKGROUND";
	convertColors[L"AudioSelectionBackgroundModified"] = L"AUDIO_SELECTION_BACKGROUND_MODIFIED";
	convertColors[L"AudioInactiveLinesBackground"] = L"AUDIO_INACTIVE_LINES_BACKGROUND";
	convertColors[L"AudioWaveform"] = L"AUDIO_WAVEFORM";
	convertColors[L"AudioWaveformInactive"] = L"AUDIO_WAVEFORM_INACTIVE";
	convertColors[L"AudioWaveformModified"] = L"AUDIO_WAVEFORM_MODIFIED";
	convertColors[L"AudioWaveformSelected"] = L"AUDIO_WAVEFORM_SELECTED";
	convertColors[L"AudioSpectrumBackground"] = L"AUDIO_SPECTRUM_BACKGROUND";
	convertColors[L"AudioSpectrumEcho"] = L"AUDIO_SPECTRUM_ECHO";
	convertColors[L"AudioSpectrumInner"] = L"AUDIO_SPECTRUM_INNER";
	convertColors[L"TextFieldBackground"] = L"TEXT_FIELD_BACKGROUND";
	convertColors[L"TextFieldBorder"] = L"TEXT_FIELD_BORDER";
	convertColors[L"TextFieldBorderOnFocus"] = L"TEXT_FIELD_BORDER_ON_FOCUS";
	convertColors[L"TextFieldSelection"] = L"TEXT_FIELD_SELECTION";
	convertColors[L"TextFieldSelectionNoFocus"] = L"TEXT_FIELD_SELECTION_NO_FOCUS";
	convertColors[L"ButtonBackground"] = L"BUTTON_BACKGROUND";
	convertColors[L"ButtonBackgroundHover"] = L"BUTTON_BACKGROUND_HOVER";
	convertColors[L"ButtonBackgroundPushed"] = L"BUTTON_BACKGROUND_PUSHED";
	convertColors[L"ButtonBackgroundOnFocus"] = L"BUTTON_BACKGROUND_ON_FOCUS";
	convertColors[L"ButtonBorder"] = L"BUTTON_BORDER";
	convertColors[L"ButtonBorderHover"] = L"BUTTON_BORDER_HOVER";
	convertColors[L"ButtonBorderPushed"] = L"BUTTON_BORDER_PUSHED";
	convertColors[L"ButtonBorderOnFocus"] = L"BUTTON_BORDER_ON_FOCUS";
	convertColors[L"ButtonBorderInactive"] = L"BUTTON_BORDER_INACTIVE";
	convertColors[L"TogglebuttonBackgroundToggled"] = L"TOGGLE_BUTTON_BACKGROUND_TOGGLED";
	convertColors[L"TogglebuttonBorderToggled"] = L"TOGGLE_BUTTON_BORDER_TOGGLED";
	convertColors[L"ScrollbarBackground"] = L"SCROLLBAR_BACKGROUND";
	convertColors[L"ScrollbarScroll"] = L"SCROLLBAR_THUMB";
	convertColors[L"ScrollbarScrollHover"] = L"SCROLLBAR_THUMB_HOVER";
	convertColors[L"ScrollbarScrollPushed"] = L"SCROLLBAR_THUMB_PUSHED";
	convertColors[L"StaticboxBorder"] = L"STATICBOX_BORDER";
	convertColors[L"StaticListBorder"] = L"STATICLIST_BORDER";
	convertColors[L"StaticListBackground"] = L"STATICLIST_BACKGROUND";
	convertColors[L"StaticListSelection"] = L"STATICLIST_SELECTION";
	convertColors[L"StaticListBackgroundHeadline"] = L"STATICLIST_BACKGROUND_HEADLINE";
	convertColors[L"StaticListTextHeadline"] = L"STATICLIST_TEXT_HEADLINE";
	convertColors[L"StatusBarBorder"] = L"STATUSBAR_BORDER";
	convertColors[L"MenuBarBackground1"] = L"MENUBAR_BACKGROUND1";
	convertColors[L"MenuBarBackground2"] = L"MENUBAR_BACKGROUND2";
	convertColors[L"MenuBarBorderSelection"] = L"MENUBAR_BORDER_SELECTION";
	convertColors[L"MENU_BAR_BACKGROUND_HOVER"] = L"MENUBAR_BACKGROUND_HOVER";
	convertColors[L"MenuBarBackgroundSelection"] = L"MENUBAR_BACKGROUND_SELECTION";
	convertColors[L"MenuBackground"] = L"MENUBAR_BACKGROUND";
	convertColors[L"MenuBorderSelection"] = L"MENU_BORDER_SELECTION";
	convertColors[L"MenuBackgroundSelection"] = L"MENU_BACKGROUND_SELECTION";
	convertColors[L"TabsBarBackground1"] = L"TABSBAR_BACKGROUND1";
	convertColors[L"TabsBarBackground2"] = L"TABSBAR_BACKGROUND2";
	convertColors[L"TabsBorderActive"] = L"TABS_BORDER_ACTIVE";
	convertColors[L"TabsBorderInactive"] = L"TABS_BORDER_INACTIVE";
	convertColors[L"TabsBackgroundActive"] = L"TABS_BACKGROUND_ACTIVE";
	convertColors[L"TabsBackgroundInactive"] = L"TABS_BACKGROUND_INACTIVE";
	convertColors[L"TabsBackgroundInactiveHover"] = L"TABS_BACKGROUND_INACTIVE_HOVER";
	convertColors[L"TabsBackgroundSecondWindow"] = L"TABS_BACKGROUND_SECOND_WINDOW";
	convertColors[L"TabsTextActive"] = L"TABS_TEXT_ACTIVE";
	convertColors[L"TabsTextInactive"] = L"TABS_TEXT_INACTIVE";
	convertColors[L"TabsCloseHover"] = L"TABS_CLOSE_HOVER";
	convertColors[L"TabsBarArrow"] = L"TABSBAR_ARROW";
	convertColors[L"TabsBarArrowBackground"] = L"TABSBAR_ARROW_BACKGROUND";
	convertColors[L"TabsBarArrowBackgroundHover"] = L"TABSBAR_ARROW_BACKGROUND_HOVER";
	convertColors[L"SliderPathBackground"] = L"SLIDER_PATH_BACKGROUND";
	convertColors[L"SliderPathBorder"] = L"SLIDER_PATH_BORDER";
	convertColors[L"SliderBorder"] = L"SLIDER_BORDER";
	convertColors[L"SliderBorderHover"] = L"SLIDER_BORDER_HOVER";
	convertColors[L"SliderBorderPushed"] = L"SLIDER_BORDER_PUSHED";
	convertColors[L"SliderBackground"] = L"SLIDER_BACKGROUND";
	convertColors[L"SliderBackgroundHover"] = L"SLIDER_BACKGROUND_HOVER";
	convertColors[L"SliderBackgroundPushed"] = L"SLIDER_BACKGROUND_PUSHED";
	convertColors[L"StylePreviewColor1"] = L"STYLE_PREVIEW_COLOR1";
	convertColors[L"StylePreviewColor2"] = L"STYLE_PREVIEW_COLOR2";

	//hotkeys
	convertHotkeys[L"AudioCommitAlt"] = L"AUDIO_COMMIT_ALT";
	convertHotkeys[L"AudioPlayAlt"] = L"AUDIO_PLAY_ALT";
	convertHotkeys[L"AudioPlayLineAlt"] = L"AUDIO_PLAY_LINE_ALT";
	convertHotkeys[L"AudioPreviousAlt"] = L"AUDIO_PREVIOUS_ALT";
	convertHotkeys[L"AudioNextAlt"] = L"AUDIO_NEXT_ALT";
	convertHotkeys[L"AudioCommit"] = L"AUDIO_COMMIT";
	convertHotkeys[L"AudioPlay"] = L"AUDIO_PLAY";
	convertHotkeys[L"AudioPlayLine"] = L"AUDIO_PLAY_LINE";
	convertHotkeys[L"AudioPrevious"] = L"AUDIO_PREVIOUS";
	convertHotkeys[L"AudioNext"] = L"AUDIO_NEXT";
	convertHotkeys[L"AudioStop"] = L"AUDIO_STOP";
	convertHotkeys[L"AudioPlayBeforeMark"] = L"AUDIO_PLAY_BEFORE_MARK";
	convertHotkeys[L"AudioPlayAfterMark"] = L"AUDIO_PLAY_AFTER_MARK";
	convertHotkeys[L"AudioPlay500MSBefore"] = L"AUDIO_PLAY_500MS_BEFORE";
	convertHotkeys[L"AudioPlay500MSAfter"] = L"AUDIO_PLAY_500MS_AFTER";
	convertHotkeys[L"AudioPlay500MSFirst"] = L"AUDIO_PLAY_500MS_FIRST";
	convertHotkeys[L"AudioPlay500MSLast"] = L"AUDIO_PLAY_500MS_LAST";
	convertHotkeys[L"AudioPlayToEnd"] = L"AUDIO_PLAY_TO_END";
	convertHotkeys[L"AudioScrollLeft"] = L"AUDIO_SCROLL_LEFT";
	convertHotkeys[L"AudioScrollRight"] = L"AUDIO_SCROLL_RIGHT";
	convertHotkeys[L"AudioGoto"] = L"AUDIO_GOTO";
	convertHotkeys[L"AudioLeadin"] = L"AUDIO_LEAD_IN";
	convertHotkeys[L"AudioLeadout"] = L"AUDIO_LEAD_OUT";
	convertHotkeys[L"PlayPause"] = L"VIDEO_PLAY_PAUSE";
	convertHotkeys[L"StopPlayback"] = L"VIDEO_STOP";
	convertHotkeys[L"Plus5Second"] = L"VIDEO_5_SECONDS_FORWARD";
	convertHotkeys[L"Minus5Second"] = L"VIDEO_5_SECONDS_BACKWARD";
	convertHotkeys[L"MinusMinute"] = L"VIDEO_MINUTE_BACKWARD";
	convertHotkeys[L"PlusMinute"] = L"VIDEO_MINUTE_FORWARD";
	convertHotkeys[L"VolumePlus"] = L"VIDEO_VOLUME_PLUS";
	convertHotkeys[L"VolumeMinus"] = L"VIDEO_VOLUME_MINUS";
	convertHotkeys[L"PreviousVideo"] = L"VIDEO_PREVIOUS_FILE";
	convertHotkeys[L"NextVideo"] = L"VIDEO_NEXT_FILE";
	convertHotkeys[L"PreviousChapter"] = L"VIDEO_PREVIOUS_CHAPTER";
	convertHotkeys[L"NextChapter"] = L"VIDEO_NEXT_CHAPTER";
	convertHotkeys[L"FullScreen"] = L"VIDEO_FULL_SCREEN";
	convertHotkeys[L"HideProgressBar"] = L"VIDEO_HIDE_PROGRESS_BAR";
	convertHotkeys[L"DeleteVideo"] = L"VIDEO_DELETE_FILE";
	convertHotkeys[L"AspectRatio"] = L"VIDEO_ASPECT_RATIO";
	convertHotkeys[L"CopyCoords"] = L"VIDEO_COPY_COORDS";
	convertHotkeys[L"FrameToPNG"] = L"VIDEO_SAVE_FRAME_TO_PNG";
	convertHotkeys[L"FrameToClipboard"] = L"VIDEO_COPY_FRAME_TO_CLIPBOARD";
	convertHotkeys[L"SubbedFrameToPNG"] = L"VIDEO_SAVE_SUBBED_FRAME_TO_PNG";
	convertHotkeys[L"SubbedFrameToClipboard"] = L"VIDEO_COPY_SUBBED_FRAME_TO_CLIPBOARD";
	convertHotkeys[L"PutBold"] = L"EDITBOX_INSERT_BOLD";
	convertHotkeys[L"PutItalic"] = L"EDITBOX_INSERT_ITALIC";
	convertHotkeys[L"SplitLine"] = L"EDITBOX_SPLIT_LINE";
	convertHotkeys[L"StartDifference"] = L"EDITBOX_START_DIFFERENCE";
	convertHotkeys[L"EndDifference"] = L"EDITBOX_END_DIFFERENCE";
	convertHotkeys[L"FindNextDoubtful"] = L"EDITBOX_FIND_NEXT_DOUBTFUL";
	convertHotkeys[L"FindNextUntranslated"] = L"EDITBOX_FIND_NEXT_UNTRANSLATED";
	convertHotkeys[L"SetDoubtful"] = L"EDITBOX_SET_DOUBTFUL";
	convertHotkeys[L"GRID_FILTER_INVERTED"] = L"GRID_FILTER_INVERT";
	convertHotkeys[L"InsertBefore"] = L"GRID_INSERT_BEFORE";
	convertHotkeys[L"InsertAfter"] = L"GRID_INSERT_AFTER";
	convertHotkeys[L"InsertBeforeVideo"] = L"GRID_INSERT_BEFORE_VIDEO";
	convertHotkeys[L"InsertAfterVideo"] = L"GRID_INSERT_AFTER_VIDEO";
	convertHotkeys[L"InsertBeforeWithVideoFrame"] = L"GRID_INSERT_BEFORE_WITH_VIDEO_FRAME";
	convertHotkeys[L"InsertAfterWithVideoFrame"] = L"GRID_INSERT_AFTER_WITH_VIDEO_FRAME";
	convertHotkeys[L"Swap"] = L"GRID_SWAP_LINES";
	convertHotkeys[L"Duplicate"] = L"GRID_DUPLICATE_LINES";
	convertHotkeys[L"Join"] = L"GRID_JOIN_LINES";
	convertHotkeys[L"JoinToFirst"] = L"GRID_JOIN_TO_FIRST_LINE";
	convertHotkeys[L"JoinToLast"] = L"GRID_JOIN_TO_LAST_LINE";
	convertHotkeys[L"Copy"] = L"GRID_COPY";
	convertHotkeys[L"Paste"] = L"GRID_PASTE";
	convertHotkeys[L"Cut"] = L"GRID_CUT";
	convertHotkeys[L"ShowPreview"] = L"GRID_SHOW_PREVIEW";
	convertHotkeys[L"HideSelected"] = L"GRID_HIDE_SELECTED";
	convertHotkeys[L"FilterByNothing"] = L"GRID_FILTER_BY_NOTHING";
	convertHotkeys[L"FilterByStyles"] = L"GRID_FILTER_BY_STYLES";
	convertHotkeys[L"FilterBySelections"] = L"GRID_FILTER_BY_SELECTIONS";
	convertHotkeys[L"FilterByDialogues"] = L"GRID_FILTER_BY_DIALOGUES";
	convertHotkeys[L"FilterByDoubtful"] = L"GRID_FILTER_BY_DOUBTFUL";
	convertHotkeys[L"FilterByUntranslated"] = L"GRID_FILTER_BY_UNTRANSLATED";
	convertHotkeys[L"PasteTranslation"] = L"GRID_PASTE_TRANSLATION";
	convertHotkeys[L"TranslationDialog"] = L"GRID_TRANSLATION_DIALOG";
	convertHotkeys[L"SubsFromMKV"] = L"GRID_SUBS_FROM_MKV";
	convertHotkeys[L"ContinousPrevious"] = L"GRID_MAKE_CONTINOUS_PREVIOUS_LINE";
	convertHotkeys[L"ContinousNext"] = L"GRID_MAKE_CONTINOUS_NEXT_LINE";
	convertHotkeys[L"PasteCollumns"] = L"GRID_PASTE_COLUMNS";
	convertHotkeys[L"CopyCollumns"] = L"GRID_COPY_COLUMNS";
	convertHotkeys[L"FPSFromVideo"] = L"GRID_SET_FPS_FROM_VIDEO";
	convertHotkeys[L"NewFPS"] = L"GRID_SET_NEW_FPS";
	convertHotkeys[L"SaveSubs"] = L"GLOBAL_SAVE_SUBS";
	convertHotkeys[L"SaveAllSubs"] = L"GLOBAL_SAVE_ALL_SUBS";
	convertHotkeys[L"SaveSubsAs"] = L"GLOBAL_SAVE_SUBS_AS";
	convertHotkeys[L"SaveTranslation"] = L"GLOBAL_SAVE_TRANSLATION";
	convertHotkeys[L"RemoveSubs"] = L"GLOBAL_REMOVE_SUBS";
	convertHotkeys[L"Search"] = L"GLOBAL_SEARCH";
	convertHotkeys[L"SelectLinesDialog"] = L"GLOBAL_OPEN_SELECT_LINES";
	convertHotkeys[L"SpellcheckerDialog"] = L"GLOBAL_OPEN_SPELLCHECKER";
	convertHotkeys[L"VideoIndexing"] = L"GLOBAL_VIDEO_INDEXING";
	convertHotkeys[L"SaveWithVideoName"] = L"GLOBAL_SAVE_WITH_VIDEO_NAME";
	convertHotkeys[L"OpenAudio"] = L"GLOBAL_OPEN_AUDIO";
	convertHotkeys[L"AudioFromVideo"] = L"GLOBAL_AUDIO_FROM_VIDEO";
	convertHotkeys[L"CloseAudio"] = L"GLOBAL_CLOSE_AUDIO";
	convertHotkeys[L"ASSProperties"] = L"GLOBAL_OPEN_ASS_PROPERTIES";
	convertHotkeys[L"StyleManager"] = L"GLOBAL_OPEN_STYLE_MANAGER";
	convertHotkeys[L"SubsResample"] = L"GLOBAL_OPEN_SUBS_RESAMPLE";
	convertHotkeys[L"FontCollectorID"] = L"GLOBAL_OPEN_FONT_COLLECTOR";
	convertHotkeys[L"ConvertToASS"] = L"GLOBAL_CONVERT_TO_ASS";
	convertHotkeys[L"ConvertToSRT"] = L"GLOBAL_CONVERT_TO_SRT";
	convertHotkeys[L"ConvertToTMP"] = L"GLOBAL_CONVERT_TO_TMP";
	convertHotkeys[L"ConvertToMDVD"] = L"GLOBAL_CONVERT_TO_MDVD";
	convertHotkeys[L"ConvertToMPL2"] = L"GLOBAL_CONVERT_TO_MPL2";
	convertHotkeys[L"HideTags"] = L"GLOBAL_HIDE_TAGS";
	convertHotkeys[L"ChangeTime"] = L"GLOBAL_SHOW_SHIFT_TIMES";
	convertHotkeys[L"ViewAll"] = L"GLOBAL_VIEW_ALL";
	convertHotkeys[L"ViewAudio"] = L"GLOBAL_VIEW_AUDIO";
	convertHotkeys[L"ViewVideo"] = L"GLOBAL_VIEW_VIDEO";
	convertHotkeys[L"ViewSubs"] = L"GLOBAL_VIEW_SUBS";
	convertHotkeys[L"AutoLoadScript"] = L"GLOBAL_AUTOMATION_LOAD_SCRIPT";
	convertHotkeys[L"AutoReloadAutoload"] = L"GLOBAL_AUTOMATION_RELOAD_AUTOLOAD";
	convertHotkeys[L"LoadLastScript"] = L"GLOBAL_AUTOMATION_LOAD_LAST_SCRIPT";
	convertHotkeys[L"AUTOMATION_OPEN_HOTKEYS_WINDOW"] = L"GLOBAL_AUTOMATION_OPEN_HOTKEYS_WINDOW";
	convertHotkeys[L"PlayPauseG"] = L"GLOBAL_PLAY_PAUSE";
	convertHotkeys[L"PreviousFrame"] = L"GLOBAL_PREVIOUS_FRAME";
	convertHotkeys[L"NextFrame"] = L"GLOBAL_NEXT_FRAME";
	convertHotkeys[L"VideoZoom"] = L"GLOBAL_VIDEO_ZOOM";
	convertHotkeys[L"SetStartTime"] = L"GLOBAL_SET_START_TIME";
	convertHotkeys[L"SetEndTime"] = L"GLOBAL_SET_END_TIME";
	convertHotkeys[L"SetVideoAtStart"] = L"GLOBAL_SET_VIDEO_AT_START_TIME";
	convertHotkeys[L"SetVideoAtEnd"] = L"GLOBAL_SET_VIDEO_AT_END_TIME";
	convertHotkeys[L"GoToNextKeyframe"] = L"GLOBAL_GO_TO_NEXT_KEYFRAME";
	convertHotkeys[L"GoToPrewKeyframe"] = L"GLOBAL_GO_TO_PREVIOUS_KEYFRAME";
	convertHotkeys[L"SetAudioFromVideo"] = L"GLOBAL_SET_AUDIO_FROM_VIDEO";
	convertHotkeys[L"SetAudioMarkFromVideo"] = L"GLOBAL_SET_AUDIO_MARK_FROM_VIDEO";
	convertHotkeys[L"Redo"] = L"GLOBAL_REDO";
	convertHotkeys[L"Undo"] = L"GLOBAL_UNDO";
	convertHotkeys[L"UndoToLastSave"] = L"GLOBAL_UNDO_TO_LAST_SAVE";
	convertHotkeys[L"History"] = L"GLOBAL_HISTORY";
	convertHotkeys[L"OpenSubs"] = L"GLOBAL_OPEN_SUBS";
	convertHotkeys[L"OpenVideo"] = L"GLOBAL_OPEN_VIDEO";
	convertHotkeys[L"GLOBAL_KEYFRAMES_OPEN"] = L"GLOBAL_OPEN_KEYFRAMES";
	convertHotkeys[L"Settings"] = L"GLOBAL_SETTINGS";
	convertHotkeys[L"Quit"] = L"GLOBAL_QUIT";
	convertHotkeys[L"Editor"] = L"GLOBAL_EDITOR";
	convertHotkeys[L"About"] = L"GLOBAL_ABOUT";
	convertHotkeys[L"Helpers"] = L"GLOBAL_HELPERS";
	convertHotkeys[L"Help"] = L"GLOBAL_HELP";
	convertHotkeys[L"ANSI"] = L"GLOBAL_ANSI";
	convertHotkeys[L"PreviousLine"] = L"GLOBAL_PREVIOUS_LINE";
	convertHotkeys[L"NextLine"] = L"GLOBAL_NEXT_LINE";
	convertHotkeys[L"JoinWithPrevious"] = L"GLOBAL_JOIN_WITH_PREVIOUS";
	convertHotkeys[L"JoinWithNext"] = L"GLOBAL_JOIN_WITH_NEXT";
	convertHotkeys[L"NextTab"] = L"GLOBAL_NEXT_TAB";
	convertHotkeys[L"PreviousTab"] = L"GLOBAL_PREVIOUS_TAB";
	convertHotkeys[L"Remove"] = L"GLOBAL_REMOVE_LINES";
	convertHotkeys[L"RemoveText"] = L"GLOBAL_REMOVE_TEXT";
	convertHotkeys[L"SnapWithStart"] = L"GLOBAL_SNAP_WITH_START";
	convertHotkeys[L"SnapWithEnd"] = L"GLOBAL_SNAP_WITH_END";
	convertHotkeys[L"Plus5SecondG"] = L"GLOBAL_5_SECONDS_FORWARD";
	convertHotkeys[L"Minus5SecondG"] = L"GLOBAL_5_SECONDS_BACKWARD";
	convertHotkeys[L"SortLines"] = L"GLOBAL_SORT_LINES";
	convertHotkeys[L"SortSelected"] = L"GLOBAL_SORT_SELECTED_LINES";
	convertHotkeys[L"RecentAudio"] = L"GLOBAL_RECENT_AUDIO";
	convertHotkeys[L"RecentVideo"] = L"GLOBAL_RECENT_VIDEO";
	convertHotkeys[L"RecentSubs"] = L"GLOBAL_RECENT_SUBS";
	convertHotkeys[L"GLOBAL_KEYFRAMES_RECENT"] = L"GLOBAL_RECENT_KEYFRAMES";
	convertHotkeys[L"SelectFromVideo"] = L"GLOBAL_SELECT_FROM_VIDEO";
	convertHotkeys[L"PlayActualLine"] = L"GLOBAL_PLAY_ACTUAL_LINE";
}

bool ConfigConverter::ConvertConfig(wxString *rawConfig)
{
	if (!convertConfig.size())
		CreateTable();

	wxRegEx configrx(L"^([^=]*)", wxRE_ADVANCED);
	int changed = 0;
	wxString result = L"[" + Options.progname + L"]\r\n";
	wxStringTokenizer tokenize(*rawConfig, L"\n", wxTOKEN_STRTOK);
	while (tokenize.HasMoreTokens()){
		wxString token = tokenize.GetNextToken();
		if (configrx.Matches(token)){
			wxString match = configrx.GetMatch(token, 1);
			auto &it = convertConfig.find(match);
			if (it != convertConfig.end()){
				if (token.StartsWith(L"ToolbarIDs")){
					wxString vals = token.AfterFirst(L'=');
					token = it->second.first + L"={\n";
					wxStringTokenizer idsTokenizer(vals, L"|", wxTOKEN_STRTOK);
					while (idsTokenizer.HasMoreTokens()){
						wxString idToken = idsTokenizer.GetNextToken();
						auto &itID = convertHotkeys.find(idToken);
						if (itID != convertHotkeys.end()){
							token += L"\t" + itID->second + L"\n";
						}
						else{
							token += L"\t" + idToken + L"\n";
						}
					}
					token += L"}";
					result << token << L"\n";
					changed++;
					continue;
				}
				size_t reps = configrx.ReplaceFirst(&token, it->second.first);
				if (reps)
					changed++;
				if (it->second.second != L""){
					token.Replace(L"=", L"={\n\t", false);
					token.Replace(it->second.second, L"\n\t");
					token += L"\n}";
				}
			}
		}
		result << token << L"\n";
	}
	*rawConfig = result;
	return changed > 10;
}

bool ConfigConverter::ConvertColors(wxString *rawColors)
{
	if (!convertColors.size())
		CreateTable();

	wxRegEx configrx(L"^([^=]*)", wxRE_ADVANCED);
	int changed = 0;
	wxString result;
	wxStringTokenizer tokenize(*rawColors, L"\n", wxTOKEN_STRTOK);
	while (tokenize.HasMoreTokens()){
		wxString token = tokenize.GetNextToken();
		if (configrx.Matches(token)){
			wxString match = configrx.GetMatch(token, 1);
			auto &it = convertColors.find(match);
			if (it != convertColors.end()){
				size_t reps = configrx.ReplaceFirst(&token, it->second);
				if (reps)
					changed++;
			}
		}
		result << token << L"\n";
	}
	*rawColors = result;
	return changed > 10;
}

bool ConfigConverter::ConvertHotkeys(wxString *rawHotkeys)
{
	if (!convertConfig.size())
		CreateTable();

	wxRegEx configrx(L"^([^ ]*) (.)=", wxRE_ADVANCED);
	int changed = 0;
	wxString result = L"[" + Options.progname + L"]\n";
	wxStringTokenizer tokenize(*rawHotkeys, L"\n", wxTOKEN_STRTOK);
	while (tokenize.HasMoreTokens()){
		wxString token = tokenize.GetNextToken();
		if (configrx.Matches(token)){
			wxString match = configrx.GetMatch(token, 1);
			auto &it = convertHotkeys.find(match);
			if (it != convertHotkeys.end()){
				wxString match = configrx.GetMatch(token, 2);
				wxString mark = match == L'N' ? L"S" : match == L'W' ? L"V" : match;
				wxString replace = it->second + L" " + mark + L"=";
				size_t reps = configrx.ReplaceFirst(&token, replace);
				if (reps)
					changed++;

				
			}
		}
		result << token << L"\n";
	}
	*rawHotkeys = result;
	return changed > 10;
}

ConfigConverter *ConfigConverter::Get()
{
	return &ccthis;
}

ConfigConverter ConfigConverter::ccthis;