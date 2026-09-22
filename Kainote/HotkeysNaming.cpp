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
	names[AUDIO_COMMIT] = _("Commit");
	names[AUDIO_COMMIT_ALT] = _("Commit alt");
	names[AUDIO_PREVIOUS] = _("Previous line");
	names[AUDIO_PREVIOUS_ALT] = _("Previous line alt");
	names[AUDIO_NEXT] = _("Next line");
	names[AUDIO_NEXT_ALT] = _("Next line alt");
	names[AUDIO_PLAY] = _("Play");
	names[AUDIO_PLAY_ALT] = _("Play alt");
	names[AUDIO_PLAY_LINE] = _("Play line");
	names[AUDIO_PLAY_LINE_ALT] = _("Play line alt");
	names[AUDIO_STOP] = _("Stop");
	names[AUDIO_GOTO] = _("Go to selection");
	names[AUDIO_SCROLL_RIGHT] = _("Scroll left");
	names[AUDIO_SCROLL_LEFT] = _("Scroll right");
	names[AUDIO_PLAY_BEFORE_MARK] = _("Play before the marker");
	names[AUDIO_PLAY_AFTER_MARK] = _("Play after the marker");
	names[AUDIO_PLAY_500MS_FIRST] = _("Play first 500ms");
	names[AUDIO_PLAY_500MS_LAST] = _("Play last 500ms");
	names[AUDIO_PLAY_500MS_BEFORE] = _("Play 500ms before");
	names[AUDIO_PLAY_500MS_AFTER] = _("Play 500ms after");
	names[AUDIO_PLAY_TO_END] = _("Play to the end");
	names[AUDIO_LEAD_IN] = _("Add lead-in");
	names[AUDIO_LEAD_OUT] = _("Add lead-out");
	names[EDITBOX_CHANGE_COLOR_OUTLINE] = _("Border color");
	names[EDITBOX_CHANGE_COLOR_PRIMARY] = _("Primary color");
	names[EDITBOX_CHANGE_COLOR_SECONDARY] = _("Secondary color for karaoke");
	names[EDITBOX_CHANGE_COLOR_SHADOW] = _("Shadow color");
	names[EDITBOX_CHANGE_FONT] = _("Font selection");
	names[EDITBOX_CHANGE_STRIKEOUT] = _("Strikethrough");
	names[EDITBOX_CHANGE_UNDERLINE] = _("Underline");
	names[EDITBOX_COMMIT] = _("Apply changes");
	names[EDITBOX_COMMIT_GO_NEXT_LINE] = _("Apply the changes and go to the next line");
	names[EDITBOX_FIND_NEXT_DOUBTFUL] = _("Next unconfirmed line");
	names[EDITBOX_FIND_NEXT_UNTRANSLATED] = _("Next untranslated line");
	names[EDITBOX_HIDE_ORIGINAL] = _("Hide original");
	names[EDITBOX_INSERT_BOLD] = _("Add bold");
	names[EDITBOX_INSERT_ITALIC] = _("Add italic");
	names[EDITBOX_PASTE_ALL_TO_TRANSLATION] = _("Paste all");
	names[EDITBOX_PASTE_SELECTION_TO_TRANSLATION] = _("Paste the selected");
	names[EDITBOX_END_DIFFERENCE] = _("Insert difference to the end");
	names[EDITBOX_SET_DOUBTFUL] = _("Mark as unconfirmed and go to the next line");
	names[EDITBOX_SPLIT_LINE] = _("Add line wrap");
	names[EDITBOX_START_DIFFERENCE] = _("Insert difference from the start");
	names[EDITBOX_TAG_BUTTON1] = _("First tag button");
	names[EDITBOX_TAG_BUTTON2] = _("Second tag button");
	names[EDITBOX_TAG_BUTTON3] = _("Third tag button");
	names[EDITBOX_TAG_BUTTON4] = _("4th tag button");
	names[EDITBOX_TAG_BUTTON5] = _("5th tag button");
	names[EDITBOX_TAG_BUTTON6] = _("6th tag button");
	names[EDITBOX_TAG_BUTTON7] = _("7th tag button");
	names[EDITBOX_TAG_BUTTON8] = _("8th tag button");
	names[EDITBOX_TAG_BUTTON9] = _("9th tag button");
	names[EDITBOX_TAG_BUTTON10] = _("10th tag button");
	names[EDITBOX_TAG_BUTTON11] = _("11th tag button");
	names[EDITBOX_TAG_BUTTON12] = _("12th tag button");
	names[EDITBOX_TAG_BUTTON13] = _("13th tag button");
	names[EDITBOX_TAG_BUTTON14] = _("14th tag button");
	names[EDITBOX_TAG_BUTTON15] = _("15th tag button");
	names[EDITBOX_TAG_BUTTON16] = _("16th tag button");
	names[EDITBOX_TAG_BUTTON17] = _("17th tag button");
	names[EDITBOX_TAG_BUTTON18] = _("18th tag button");
	names[EDITBOX_TAG_BUTTON19] = _("19th tag button");
	names[EDITBOX_TAG_BUTTON20] = _("20th tag button");
	names[GRID_MAKE_CONTINOUS_NEXT_LINE] = _("Set times as a continuous (next line)");
	names[GRID_MAKE_CONTINOUS_PREVIOUS_LINE] = _("Set times as a continuous (previous line)");
	names[GRID_COPY_COLUMNS] = _("Copy columns");
	names[GRID_DUPLICATE_LINES] = _("Duplicate lines");
	names[GRID_FILTER_BY_DIALOGUES] = _("Hide comments");
	names[GRID_FILTER_BY_DOUBTFUL] = _("Show unconfirmed");
	names[GRID_FILTER_BY_NOTHING] = _("Turn off filtering");
	names[GRID_FILTER_BY_SELECTIONS] = _("Hide selected lines");
	names[GRID_FILTER_BY_STYLES] = _("Hide lines with styles");
	names[GRID_FILTER_BY_UNTRANSLATED] = _("Show untranslated");
	names[GRID_SET_FPS_FROM_VIDEO] = _("Set FPS from video");
	names[GRID_FILTER] = _("Filter");
	names[GRID_FILTER_AFTER_SUBS_LOAD] = _("Filter after loading subtitles");
	names[GRID_FILTER_DO_NOT_RESET] = _("Do not reset previous filtering");
	names[GRID_FILTER_IGNORE_IN_ACTIONS] = _("Ignore filtering in some actions");
	names[GRID_FILTER_INVERT] = _("Reverse filtering");
	names[GRID_HIDE_ACTOR] = _("Hide actor");
	names[GRID_HIDE_CPS] = _("Hide characters per second");
	names[GRID_HIDE_END] = _("Hide end time");
	names[GRID_HIDE_EFFECT] = _("Hide effect");
	names[GRID_HIDE_LAYER] = _("Hide layer");
	names[GRID_HIDE_MARGINL] = _("Hide left margin");
	names[GRID_HIDE_MARGINR] = _("Hide right margin");
	names[GRID_HIDE_MARGINV] = _("Hide vertical margin");
	names[GRID_HIDE_START] = _("Hide start time");
	names[GRID_HIDE_STYLE] = _("Hide style");
	names[GRID_TREE_MAKE] = _("Make tree");
	names[GRID_SELECT_VISIBLE_LINES] = _("Select all lines visible on video");
	names[GRID_HIDE_SELECTED] = _("Hide selected lines");
	names[GRID_INSERT_AFTER] = _("Insert after");
	names[GRID_INSERT_AFTER_VIDEO] = _("Insert after with video time");
	names[GRID_INSERT_AFTER_WITH_VIDEO_FRAME] = _("Insert after with video frame time");
	names[GRID_INSERT_BEFORE] = _("Insert before");
	names[GRID_INSERT_BEFORE_VIDEO] = _("Insert before with video time");
	names[GRID_INSERT_BEFORE_WITH_VIDEO_FRAME] = _("Insert before with video frame time");
	names[GRID_JOIN_LINES] = _("Join lines");
	names[GRID_JOIN_TO_FIRST_LINE] = _("Join lines and keep first");
	names[GRID_JOIN_TO_LAST_LINE] = _("Join lines and keep last");
	names[GRID_PASTE_COLUMNS] = _("Paste columns");
	names[GRID_PASTE_TRANSLATION] = _("Paste translation text");
	names[GRID_SUBS_FROM_MKV] = _("Load subtitles from an MKV file");
	names[GRID_SWAP_LINES] = _("Swap lines");
	names[GRID_TRANSLATION_DIALOG] = _("Dialogue shifting window");
	names[GRID_SET_NEW_FPS] = _("Set new FPS");
	names[GRID_SHOW_PREVIEW] = _("Show subtitles preview");
	names[GRID_SPLIT_BY_VIDEO_TIME] = _("Split line at video time");
	names[GRID_SPLIT_BY_FRAME] = _("Split lines into frames");
	names[GRID_SPLIT_BY_CHARS] = _("Split lines into characters");
	names[GRID_SPLIT_BY_WORDS] = _("Split lines into words");
	names[GRID_SPLIT_BY_WRAPS] = _("Split lines by wraps");
	names[GLOBAL_OPEN_ASS_PROPERTIES] = _("ASS file properties");
	names[GLOBAL_CHECK_FOR_UPDATES] = _("Check for updates");
	names[GLOBAL_ABOUT] = _("About");
	names[GLOBAL_ANSI] = _("Forum thread on animesub.info (Polish)");
	names[GLOBAL_CONVERT_TO_ASS] = _("Convert to ASS");
	names[GLOBAL_CONVERT_TO_SRT] = _("Convert to SRT");
	names[GLOBAL_CONVERT_TO_MDVD] = _("Convert to MDVD");
	names[GLOBAL_CONVERT_TO_MPL2] = _("Convert to MPL2");
	names[GLOBAL_CONVERT_TO_TMP] = _("Convert to TMP");
	names[GLOBAL_AUDIO_FROM_VIDEO] = _("Open audio from video");
	names[GLOBAL_AUTOMATION_LOAD_SCRIPT] = _("Load script");
	names[GLOBAL_AUTOMATION_OPEN_HOTKEYS_WINDOW] = _("Open shortcut mapping window");
	names[GLOBAL_AUTOMATION_RELOAD_AUTOLOAD] = _("Refresh autoload scripts");
	names[GLOBAL_SHOW_SHIFT_TIMES] = _("Time shift window");
	names[GLOBAL_CLOSE_AUDIO] = _("Close audio");
	names[GLOBAL_EDITOR] = _("Enable / Disable editor");
	names[GLOBAL_FIND_REPLACE] = _("Find and replace");
	names[GLOBAL_OPEN_FONT_COLLECTOR] = _("Font collector");
	names[GLOBAL_HELP] = _("Help (not available in English)");
	names[GLOBAL_HELPERS] = _("Credits");
	names[GLOBAL_HIDE_TAGS] = _("Hide tags");
	names[GLOBAL_HISTORY] = _("History");
	names[GLOBAL_JOIN_WITH_PREVIOUS] = _("Merge with previous line");
	names[GLOBAL_JOIN_WITH_NEXT] = _("Merge with next line");
	names[GLOBAL_OPEN_KEYFRAMES] = _("Open keyframes");
	names[GLOBAL_OPEN_AUTO_SAVE] = _("Open auto save");
	names[GLOBAL_AUTOMATION_LOAD_LAST_SCRIPT] = _("Run the last loaded script");
	names[GLOBAL_SORT_ALL_BY_START_TIMES] = _("Sort all lines by start time");
	names[GLOBAL_SORT_ALL_BY_END_TIMES] = _("Sort all lines by end time");
	names[GLOBAL_SORT_ALL_BY_STYLE] = _("Sort all lines by styles");
	names[GLOBAL_SORT_ALL_BY_ACTOR] = _("Sort all lines by actor");
	names[GLOBAL_SORT_ALL_BY_EFFECT] = _("Sort all lines by effect");
	names[GLOBAL_SORT_ALL_BY_LAYER] = _("Sort all lines by layer");
	names[GLOBAL_SORT_SELECTED_BY_START_TIMES] = _("Sort selected lines by start time");
	names[GLOBAL_SORT_SELECTED_BY_END_TIMES] = _("Sort selected lines by end time");
	names[GLOBAL_SORT_SELECTED_BY_STYLE] = _("Sort selected lines by styles");
	names[GLOBAL_SORT_SELECTED_BY_ACTOR] = _("Sort selected lines by actor");
	names[GLOBAL_SORT_SELECTED_BY_EFFECT] = _("Sort selected lines by effect");
	names[GLOBAL_SORT_SELECTED_BY_LAYER] = _("Sort selected lines by layer");
	names[GLOBAL_SHIFT_TIMES] = _("Shift times / run time post processor");
	names[GLOBAL_GO_TO_PREVIOUS_KEYFRAME] = _("Go to previous keyframe");
	names[GLOBAL_GO_TO_NEXT_KEYFRAME] = _("Go to next keyframe");
	names[GLOBAL_OPEN_SUBS_RESAMPLE] = _("Resample subtitles");
	names[GLOBAL_UNDO] = _("Undo");
	names[GLOBAL_UNDO_TO_LAST_SAVE] = _("Undo to last save");
	names[GLOBAL_VIDEO_INDEXING] = _("Open video with FFMS2");
	names[GLOBAL_VIDEO_ZOOM] = _("Zoom video");
	names[GLOBAL_RESET_VIDEO_ZOOM] = _("Turn off video zoom");
	names[GLOBAL_VIEW_ALL] = _("View all");
	names[GLOBAL_VIEW_AUDIO] = _("View audio and subtitles");
	names[GLOBAL_VIEW_SUBS] = _("View only subtitles");
	names[GLOBAL_VIEW_VIDEO] = _("View video and subtitles");
	names[GLOBAL_VIEW_ONLY_VIDEO] = _("View only video");
	names[GLOBAL_ADD_PAGE] = _("Open new tab");
	names[GLOBAL_CLOSE_PAGE] = _("Close current tab");
	names[GLOBAL_STYLE_MANAGER_CLEAN_STYLE] = _("Clean styles of ASS file");
	names[GLOBAL_MISSPELLS_REPLACER] = _("Fix minor errors (experimental)");
	names[GLOBAL_FIND_NEXT] = _("Find next");
	names[GLOBAL_LOAD_LAST_SESSION] = _("Load last session");
	names[GLOBAL_NEXT_FRAME] = _("Next frame");
	names[GLOBAL_NEXT_LINE] = _("Next line");
	names[GLOBAL_NEXT_TAB] = _("Next tab");
	names[GLOBAL_OPEN_AUDIO] = _("Open audio");
	names[GLOBAL_OPEN_SUBS] = _("Open subtitles");
	names[GLOBAL_OPEN_VIDEO] = _("Open video");
	names[GLOBAL_PLAY_PAUSE] = _("Play / Pause");
	names[GLOBAL_PLAY_ACTUAL_LINE] = _("Play active line");
	names[GLOBAL_PREVIOUS_FRAME] = _("Previous frame");
	names[GLOBAL_PREVIOUS_LINE] = _("Previous line");
	names[GLOBAL_PREVIOUS_TAB] = _("Previous tab");
	names[GLOBAL_REDO] = _("Redo");
	names[GLOBAL_REMOVE_LINES] = _("Delete line");
	names[GLOBAL_REMOVE_SUBS] = _("Remove subtitles from the editor");
	names[GLOBAL_REMOVE_TEXT] = _("Delete text");
	names[GLOBAL_SAVE_ALL_SUBS] = _("Save all subtitles");
	names[GLOBAL_SAVE_SUBS] = _("Save");
	names[GLOBAL_SAVE_SUBS_AS] = _("Save as..."); 
	names[GLOBAL_SAVE_TRANSLATION] = _("Save translation");
	names[GLOBAL_SAVE_WITH_VIDEO_NAME] = _("Save subtitles using the video name");
	names[GLOBAL_SEARCH] = _("Find");
	names[GLOBAL_SELECT_FROM_VIDEO] = _("Select line at current video position");
	names[GLOBAL_OPEN_SELECT_LINES] = _("Select lines");
	names[GLOBAL_SET_AUDIO_FROM_VIDEO] = _("Set audio position to video time");
	names[GLOBAL_SET_AUDIO_MARK_FROM_VIDEO] = _("Set audio marker to video time");
	names[GLOBAL_SET_END_TIME] = _("Insert end time from video");
	names[GLOBAL_SET_START_TIME] = _("Insert start time from video");
	names[GLOBAL_SETTINGS] = _("Settings");
	names[GLOBAL_SET_VIDEO_AT_START_TIME] = _("Go to start time");
	names[GLOBAL_SET_VIDEO_AT_END_TIME] = _("Go to end time of line");
	names[GLOBAL_SNAP_WITH_END] = _("Change end time to nearest keyframe");
	names[GLOBAL_SNAP_WITH_START] = _("Change start time to nearest keyframe");
	names[GLOBAL_OPEN_SPELLCHECKER] = _("Check spelling");
	names[GLOBAL_OPEN_STYLE_MANAGER] = _("Style manager");
	names[VIDEO_COPY_FRAME_TO_CLIPBOARD] = _("Copy frame to clipboard");
	names[VIDEO_DELETE_FILE] = _("Remove video");
	names[VIDEO_SAVE_FRAME_TO_PNG] = _("Save frame as PNG");
	names[VIDEO_5_SECONDS_BACKWARD] = _("5 seconds backward");
	names[VIDEO_MINUTE_BACKWARD] = _("1 minute backward");
	names[VIDEO_HIDE_PROGRESS_BAR] = _("Show / hide progress bar");
	names[VIDEO_NEXT_CHAPTER] = _("Next chapter");
	names[VIDEO_NEXT_FILE] = _("Next file");
	names[VIDEO_PLAY_PAUSE] = _("Play / Pause");
	names[VIDEO_ASPECT_RATIO] = _("Change aspect ratio");
	names[VIDEO_5_SECONDS_FORWARD] = _("5 seconds forward");
	names[VIDEO_MINUTE_FORWARD] = _("1 minute forward");
	names[VIDEO_PREVIOUS_CHAPTER] = _("Previous chapter");
	names[VIDEO_PREVIOUS_FILE] = _("Previous file");
	names[VIDEO_COPY_SUBBED_FRAME_TO_CLIPBOARD] = _("Copy frame with subtitles to clipboard");
	names[VIDEO_FULL_SCREEN] = _("Full screen");
	names[VIDEO_VOLUME_PLUS] = _("Volume up");
	names[VIDEO_VOLUME_MINUS] = _("Volume down");
	names[VIDEO_SAVE_SUBBED_FRAME_TO_PNG] = _("Save frame with subtitles as PNG");
	names[VIDEO_STOP] = _("Stop");
	
}
