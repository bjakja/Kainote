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
	convertConfig[L"AudioAutoCommit"] = L"AUDIO_AUTO_COMMIT";
	convertConfig[L"AudioAutoFocus"] = L"AUDIO_AUTO_FOCUS";
	convertConfig[L"AudioAutoScroll"] = L"AUDIO_AUTO_SCROLL";
	convertConfig[L"AudioBoxHeight"] = L"AUDIO_BOX_HEIGHT";
	convertConfig[L"AudioDelay"] = L"AUDIO_DELAY";
	convertConfig[L"AudioDrawKeyframes"] = L"AUDIO_DRAW_KEYFRAMES";
	convertConfig[L"AudioDrawSecondaryLines"] = L"AUDIO_DRAW_SECONDARY_LINES";
	convertConfig[L"AudioDrawSelectionBackground"] = L"AUDIO_DRAW_SELECTION_BACKGROUND";
	convertConfig[L"AudioDrawTimeCursor"] = L"AUDIO_DRAW_TIME_CURSOR";
	convertConfig[L"AudioDrawVideoPosition"] = L"AUDIO_DRAW_VIDEO_POSITION";
	convertConfig[L"AudioGrabTimesOnSelect"] = L"AUDIO_GRAB_TIMES_ON_SELECT";
	convertConfig[L"AudioHorizontalZoom"] = L"AUDIO_HORIZONTAL_ZOOM";
	convertConfig[L"AudioInactiveLinesDisplayMode"] = L"AUDIO_INACTIVE_LINES_DISPLAY_MODE";
	convertConfig[L"AudioKaraoke"] = L"AUDIO_KARAOKE";
	convertConfig[L"AudioKaraokeMoveOnClick"] = L"AUDIO_KARAOKE_MOVE_ON_CLICK";
	convertConfig[L"AudioKaraokeSplitMode"] = L"AUDIO_KARAOKE_SPLIT_MODE";
	convertConfig[L"AudioLeadIn"] = L"AUDIO_LEAD_IN_VALUE";
	convertConfig[L"AudioLeadOut"] = L"AUDIO_LEAD_OUT_VALUE";
	convertConfig[L"AudioLineBoundariesThickness"] = L"AUDIO_LINE_BOUNDARIES_THICKNESS";
	convertConfig[L"AudioLink"] = L"AUDIO_LINK";
	convertConfig[L"AudioLockScrollOnCursor"] = L"AUDIO_LOCK_SCROLL_ON_CURSOR";
	convertConfig[L"AudioMarkPlayTime"] = L"AUDIO_MARK_PLAY_TIME";
	convertConfig[L"AudioMergeEveryNWithSyllable"] = L"AUDIO_MERGE_EVERY_N_WITH_SYLLABLE";
	convertConfig[L"AudioNextLineOnCommit"] = L"AUDIO_NEXT_LINE_ON_COMMIT";
	convertConfig[L"AudioRAMCache"] = L"AUDIO_RAM_CACHE";
	convertConfig[L"AudioSnapToKeyframes"] = L"AUDIO_SNAP_TO_KEYFRAMES";
	convertConfig[L"AudioSnapToOtherLines"] = L"AUDIO_SNAP_TO_OTHER_LINES";
	convertConfig[L"AudioSpectrumOn"] = L"AUDIO_SPECTRUM_ON";
	convertConfig[L"AudioSpectrumNonLinearOn"] = L"AUDIO_SPECTRUM_NON_LINEAR_ON";
	convertConfig[L"AudioStartDragSensitivity"] = L"AUDIO_START_DRAG_SENSITIVITY";
	convertConfig[L"AudioVerticalZoom"] = L"AUDIO_VERTICAL_ZOOM";
	convertConfig[L"AudioVolume"] = L"AUDIO_VOLUME";
	convertConfig[L"AudioWheelDefaultToZoom"] = L"AUDIO_WHEEL_DEFAULT_TO_ZOOM";
	convertConfig[L"AcceptedAudioStream"] = L"ACCEPTED_AUDIO_STREAM";
	convertConfig[L"ASSPropertiesTitle"] = L"ASS_PROPERTIES_TITLE";
	convertConfig[L"ASSPropertiesScript"] = L"ASS_PROPERTIES_SCRIPT";
	convertConfig[L"ASSPropertiesTranslation"] = L"ASS_PROPERTIES_TRANSLATION";
	convertConfig[L"ASSPropertiesEditing"] = L"ASS_PROPERTIES_EDITING";
	convertConfig[L"ASSPropertiesTiming"] = L"ASS_PROPERTIES_TIMING";
	convertConfig[L"ASSPropertiesUpdate"] = L"ASS_PROPERTIES_UPDATE";
	convertConfig[L"ASSPropertiesTitleOn"] = L"ASS_PROPERTIES_TITLE_ON";
	convertConfig[L"ASSPropertiesScriptOn"] = L"ASS_PROPERTIES_SCRIPT_ON";
	convertConfig[L"ASSPropertiesTranslationOn"] = L"ASS_PROPERTIES_TRANSLATION_ON";
	convertConfig[L"ASSPropertiesEditingOn"] = L"ASS_PROPERTIES_EDITING_ON";
	convertConfig[L"ASSPropertiesTimingOn"] = L"ASS_PROPERTIES_TIMING_ON";
	convertConfig[L"ASSPropertiesUpdateOn"] = L"ASS_PROPERTIES_UPDATE_ON";
	convertConfig[L"ASSPropertiesAskForChange"] = L"ASS_PROPERTIES_ASK_FOR_CHANGE";
	convertConfig[L"AudioRecent"] = L"AUDIO_RECENT_FILES";
	convertConfig[L"AutomationLoadingMethod"] = L"AUTOMATION_LOADING_METHOD";
	convertConfig[L"AutomationOldScriptsCompatybility"] = L"AUTOMATION_OLD_SCRIPTS_COMPATIBILITY";
	convertConfig[L"AutomationRecent"] = L"AUTOMATION_RECENT_FILES";
	convertConfig[L"AutomationScriptEditor"] = L"AUTOMATION_SCRIPT_EDITOR";
	convertConfig[L"AutomationTraceLevel"] = L"AUTOMATION_TRACE_LEVEL";
	convertConfig[L"AutoMoveTagsFromOriginal"] = L"AUTO_MOVE_TAGS_FROM_ORIGINAL";
	convertConfig[L"AutoSaveMaxFiles"] = L"AUTOSAVE_MAX_FILES";
	convertConfig[L"AutoSelectLinesFromLastTab"] = L"AUTO_SELECT_LINES_FROM_LAST_TAB";
	convertConfig[L"ColorpickerRecent"] = L"COLORPICKER_RECENT_COLORS";
	convertConfig[L"ConvertASSTagsOnLineStart"] = L"CONVERT_ASS_TAGS_TO_INSERT_IN_LINE";
	convertConfig[L"ConvertFPS"] = L"CONVERT_FPS";
	convertConfig[L"ConvertFPSFromVideo"] = L"CONVERT_FPS_FROM_VIDEO";
	convertConfig[L"ConvertNewEndTimes"] = L"CONVERT_NEW_END_TIMES";
	convertConfig[L"ConvertResolutionWidth"] = L"CONVERT_RESOLUTION_WIDTH";
	convertConfig[L"ConvertResolutionHeight"] = L"CONVERT_RESOLUTION_HEIGHT";
	convertConfig[L"ConvertShowSettings"] = L"CONVERT_SHOW_SETTINGS";
	convertConfig[L"ConvertStyle"] = L"CONVERT_STYLE";
	convertConfig[L"ConvertStyleCatalog"] = L"CONVERT_STYLE_CATALOG";
	convertConfig[L"ConvertTimePerLetter"] = L"CONVERT_TIME_PER_CHARACTER";
	convertConfig[L"CopyCollumnsSelection"] = L"COPY_COLLUMS_SELECTIONS";
	convertConfig[L"DictionaryLanguage"] = L"DICTIONARY_LANGUAGE";
	convertConfig[L"DisableLiveVideoEditing"] = L"DISABLE_LIVE_VIDEO_EDITING";
	convertConfig[L"DontAskForBadResolution"] = L"DONT_ASK_FOR_BAD_RESOLUTION";
	convertConfig[L"EditboxSugestionsOnDoubleClick"] = L"EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK";
	convertConfig[L"EditorOn"] = L"EDITOR_ON";
	convertConfig[L"FindRecent"] = L"FIND_RECENT_FINDS";
	convertConfig[L"FindReplaceOptions"] = L"FIND_REPLACE_OPTIONS";
	convertConfig[L"FontCollectorAction"] = L"FONT_COLLECTOR_ACTION";
	convertConfig[L"FontCollectorDirectory"] = L"FONT_COLLECTOR_DIRECTORY";
	convertConfig[L"FontCollectorFromMKV"] = L"FONT_COLLECTOR_FROM_MKV";
	convertConfig[L"FontCollectorUseSubsDirectory"] = L"FONT_COLLECTOR_USE_SUBS_DIRECTORY";
	convertConfig[L"FFMS2VideoSeeking"] = L"FFMS2_VIDEO_SEEKING";
	convertConfig[L"GridChangeActiveOnSelection"] = L"GRID_CHANGE_ACTIVE_ON_SELECTION";
	convertConfig[L"GridFontName"] = L"GRID_FONT";
	convertConfig[L"GridFontSize"] = L"GRID_FONT_SIZE";
	convertConfig[L"GridAddToFilter"] = L"GRID_ADD_TO_FILTER";
	convertConfig[L"GridFilterAfterLoad"] = L"GRID_FILTER_AFTER_LOAD";
	convertConfig[L"GridFilterBy"] = L"GRID_FILTER_BY";
	convertConfig[L"GridFilterInverted"] = L"GRID_FILTER_INVERTED";
	convertConfig[L"GridFilterStyles"] = L"GRID_FILTER_STYLES";
	convertConfig[L"GridHideCollums"] = L"GRID_HIDE_COLUMNS";
	convertConfig[L"GridHideTags"] = L"GRID_HIDE_TAGS";
	convertConfig[L"GridIgnoreFiltering"] = L"GRID_IGNORE_FILTERING";
	convertConfig[L"GridLoadSortedSubs"] = L"GRID_LOAD_SORTED_SUBS";
	convertConfig[L"GridSaveAfterCharacterCount"] = L"GRID_SAVE_AFTER_CHARACTER_COUNT";
	convertConfig[L"GridTagsSwapChar"] = L"GRID_TAGS_SWAP_CHARACTER";
	convertConfig[L"InsertEndOffset"] = L"GRID_INSERT_END_OFFSET";
	convertConfig[L"InsertStartOffset"] = L"GRID_INSERT_START_OFFSET";
	convertConfig[L"KEYFRAMES_RECENT"] = L"KEYFRAMES_RECENT";
	convertConfig[L"MoveTimesByTime"] = L"SHIFT_TIMES_BY_TIME";
	convertConfig[L"MoveTimesLoadSetTabOptions"] = L"SHIFT_TIMES_CHANGE_VALUES_WITH_TAB";
	convertConfig[L"MoveTimesCorrectEndTimes"] = L"SHIFT_TIMES_CORRECT_END_TIMES";
	convertConfig[L"MoveTimesForward"] = L"SHIFT_TIMES_MOVE_FORWARD";
	convertConfig[L"MoveTimesFrames"] = L"SHIFT_TIMES_DISPLAY_FRAMES";
	convertConfig[L"MoveTimesOn"] = L"SHIFT_TIMES_ON";
	convertConfig[L"MoveTimesOptions"] = L"SHIFT_TIMES_OPTIONS";
	convertConfig[L"MoveTimesWhichLines"] = L"SHIFT_TIMES_WHICH_LINES";
	convertConfig[L"MoveTimesWhichTimes"] = L"SHIFT_TIMES_WHICH_TIMES";
	convertConfig[L"MoveTimesStyles"] = L"SHIFT_TIMES_STYLES";
	convertConfig[L"MoveTimesTime"] = L"SHIFT_TIMES_TIME";
	convertConfig[L"MoveVideoToActiveLine"] = L"MOVE_VIDEO_TO_ACTIVE_LINE";
	convertConfig[L"NoNewLineAfterTimesEdition"] = L"EDITBOX_DONT_GO_TO_NEXT_LINE_ON_TIMES_EDIT";
	convertConfig[L"OpenSubsInNewCard"] = L"OPEN_SUBS_IN_NEW_TAB";
	convertConfig[L"OpenVideoAtActiveLine"] = L"OPEN_VIDEO_AT_ACTIVE_LINE";
	convertConfig[L"PasteCollumnsSelection"] = L"PASTE_COLUMNS_SELECTION";
	convertConfig[L"PlayAfterSelection"] = L"VIDEO_PLAY_AFTER_SELECTION";
	convertConfig[L"PostprocessorEnabling"] = L"POSTPROCESSOR_ON";
	convertConfig[L"PostprocessorKeyframeBeforeStart"] = L"POSTPROCESSOR_KEYFRAME_BEFORE_START";
	convertConfig[L"PostprocessorKeyframeAfterStart"] = L"POSTPROCESSOR_KEYFRAME_AFTER_START";
	convertConfig[L"PostprocessorKeyframeBeforeEnd"] = L"POSTPROCESSOR_KEYFRAME_BEFORE_END";
	convertConfig[L"PostprocessorKeyframeAfterEnd"] = L"POSTPROCESSOR_KEYFRAME_AFTER_END";
	convertConfig[L"PostprocessorLeadIn"] = L"POSTPROCESSOR_LEAD_IN";
	convertConfig[L"PostprocessorLeadOut"] = L"POSTPROCESSOR_LEAD_OUT";
	convertConfig[L"PostprocessorThresholdStart"] = L"POSTPROCESSOR_THRESHOLD_START";
	convertConfig[L"PostprocessorThresholdEnd"] = L"POSTPROCESSOR_THRESHOLD_END";
	convertConfig[L"PreviewText"] = L"STYLE_PREVIEW_TEXT";
	convertConfig[L"ProgramLanguage"] = L"PROGRAM_LANGUAGE";
	convertConfig[L"ProgramTheme"] = L"PROGRAM_THEME";
	convertConfig[L"ReplaceRecent"] = L"REPLACE_RECENT_REPLACEMENTS";
	convertConfig[L"SelectionsRecent"] = L"SELECT_LINES_RECENT_SELECTIONS";
	convertConfig[L"SelectionsOptions"] = L"SELECT_LINES_OPTIONS";
	convertConfig[L"SelectVisibleLineAfterFullscreen"] = L"GRID_SET_VISIBLE_LINE_AFTER_FULL_SCREEN";
	convertConfig[L"SpellcheckerOn"] = L"SPELLCHECKER_ON";
	convertConfig[L"StyleEditFilterText"] = L"STYLE_EDIT_FILTER_TEXT";
	convertConfig[L"StyleFilterTextOn"] = L"STYLE_EDIT_FILTER_TEXT_ON";
	convertConfig[L"StyleManagerPosition"] = L"STYLE_MANAGER_POSITION";
	convertConfig[L"StyleManagerDetachEditor"] = L"STYLE_MANAGER_DETACH_EDIT_WINDOW";
	convertConfig[L"SubsAutonaming"] = L"SUBS_AUTONAMING";
	convertConfig[L"SubsComparisonType"] = L"SUBS_COMPARISON_TYPE";
	convertConfig[L"SubsComparisonStyles"] = L"SUBS_COMPARISON_STYLES";
	convertConfig[L"SubsRecent"] = L"SUBS_RECENT_FILES";
	convertConfig[L"TextFieldAllowNumpadHotkeys"] = L"TEXT_FIELD_ALLOW_NUMPAD_HOTKEYS";
	convertConfig[L"TlModeShowOriginal"] = L"TL_MODE_SHOW_ORIGINAL";
	convertConfig[L"ToolbarIDs"] = L"TOOLBAR_IDS";
	convertConfig[L"ToolbarAlignment"] = L"TOOLBAR_ALIGNMENT";
	convertConfig[L"UpdaterCheckIntensity"] = L"UPDATER_CHECK_INTENSITY";
	convertConfig[L"UpdaterCheckOptions"] = L"UPDATER_CHECK_OPTIONS";
	convertConfig[L"UpdaterCheckForStable"] = L"UPDATER_CHECK_FOR_STABLE";
	convertConfig[L"UpdaterLastCheck"] = L"UPDATER_LAST_CHECK";
	convertConfig[L"VideoFullskreenOnStart"] = L"VIDEO_FULL_SCREEN_ON_START";
	convertConfig[L"VideoIndex"] = L"VIDEO_INDEX";
	convertConfig[L"VideoPauseOnClick"] = L"VIDEO_PAUSE_ON_CLICK";
	convertConfig[L"VideoProgressBar"] = L"VIDEO_PROGRESS_BAR";
	convertConfig[L"VideoRecent"] = L"VIDEO_RECENT_FILES";
	convertConfig[L"VideoVolume"] = L"VIDEO_VOLUME";
	convertConfig[L"VideoWindowSize"] = L"VIDEO_WINDOW_SIZE";
	convertConfig[L"VisualWarningsOff"] = L"VIDEO_VISUAL_WARNINGS_OFF";
	convertConfig[L"WindowMaximized"] = L"WINDOW_MAXIMIZED";
	convertConfig[L"WindowPosition"] = L"WINDOW_POSITION";
	convertConfig[L"WindowSize"] = L"WINDOW_SIZE";
	convertConfig[L"EditboxTagButtons"] = L"EDITBOX_TAG_BUTTONS";
	convertConfig[L"EditboxTagButton1"] = L"EDITBOX_TAG_BUTTON_VALUE1";
	convertConfig[L"EditboxTagButton2"] = L"EDITBOX_TAG_BUTTON_VALUE2";
	convertConfig[L"EditboxTagButton3"] = L"EDITBOX_TAG_BUTTON_VALUE3";
	convertConfig[L"EditboxTagButton4"] = L"EDITBOX_TAG_BUTTON_VALUE4";
	convertConfig[L"EditboxTagButton5"] = L"EDITBOX_TAG_BUTTON_VALUE5";
	convertConfig[L"EditboxTagButton6"] = L"EDITBOX_TAG_BUTTON_VALUE6";
	convertConfig[L"EditboxTagButton7"] = L"EDITBOX_TAG_BUTTON_VALUE7";
	convertConfig[L"EditboxTagButton8"] = L"EDITBOX_TAG_BUTTON_VALUE8";
	convertConfig[L"EditboxTagButton9"] = L"EDITBOX_TAG_BUTTON_VALUE9";
	convertConfig[L"EditboxTagButton10"] = L"EDITBOX_TAG_BUTTON_VALUE10";
	convertConfig[L"EditboxTagButton11"] = L"EDITBOX_TAG_BUTTON_VALUE11";
	convertConfig[L"EditboxTagButton12"] = L"EDITBOX_TAG_BUTTON_VALUE12";
	convertConfig[L"EditboxTagButton13"] = L"EDITBOX_TAG_BUTTON_VALUE13";
	convertConfig[L"EditboxTagButton14"] = L"EDITBOX_TAG_BUTTON_VALUE14";
	convertConfig[L"EditboxTagButton15"] = L"EDITBOX_TAG_BUTTON_VALUE15";
	convertConfig[L"EditboxTagButton16"] = L"EDITBOX_TAG_BUTTON_VALUE16";
	convertConfig[L"EditboxTagButton17"] = L"EDITBOX_TAG_BUTTON_VALUE17";
	convertConfig[L"EditboxTagButton18"] = L"EDITBOX_TAG_BUTTON_VALUE18";
	convertConfig[L"EditboxTagButton19"] = L"EDITBOX_TAG_BUTTON_VALUE19";
	convertConfig[L"EditboxTagButton20"] = L"EDITBOX_TAG_BUTTON_VALUE20";

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

bool ConfigConverter::ConvertConfig(wxString *rawConfig, bool colors)
{
	if (!convertConfig.size())
		CreateTable();

	const std::map<wxString, wxString> &converter = (colors) ? convertColors : convertConfig;
	wxRegEx configrx(L"^([^=]*)", wxRE_ADVANCED);
	int changed = 0;
	wxString result = L"[" + Options.progname + L"]\r\n";
	wxStringTokenizer tokenize(*rawConfig, L"\n", wxTOKEN_STRTOK);
	while (tokenize.HasMoreTokens()){
		wxString token = tokenize.GetNextToken();
		if (configrx.Matches(token)){
			wxString match = configrx.GetMatch(token, 1);
			auto &it = converter.find(match);
			if (it != converter.end()){
				size_t reps = configrx.ReplaceFirst(&token, it->second);
				if (reps)
					changed++;
			}
		}
		result << token << L"\r\n";
	}
	*rawConfig = result;
	return changed > 10;
}

bool ConfigConverter::ConvertHotkeys(wxString *rawHotkeys)
{
	if (!convertConfig.size())
		CreateTable();

	wxRegEx configrx(L"^([^ ]*) (.)=", wxRE_ADVANCED);
	int changed = 0;
	wxString result = L"[" + Options.progname + L"]\r\n";
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
		result << token << L"\r\n";
	}
	*rawHotkeys = result;
	return changed > 10;
}

ConfigConverter *ConfigConverter::Get()
{
	if (!ccthis)
		ccthis = new ConfigConverter();

	return ccthis;
}

ConfigConverter *ConfigConverter::ccthis = NULL;