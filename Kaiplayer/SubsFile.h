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

#include "Config.h"
#include "Styles.h"
#include "SubsDialogue.h"
#include "KaiDialog.h"
#include "AVLtree.h"
#include <vector>
#include <set>
#include <functional>

enum{
	OPEN_SUBTITLES = 1,
	NEW_SUBTITLES,
	EDITBOX_LINE_EDITION,
	EDITBOX_MULTILINE_EDITION,
	EDITBOX_SPELL_CHECKER,
	GRID_DUPLICATE,
	GRID_JOIN_LINES,
	GRID_JOIN_WITH_PREVIOUS,
	GRID_JOIN_WITH_NEXT,
	GRID_JOIN_TO_FIRST,
	GRID_JOIN_TO_LAST,
	GRID_PASTE,
	GRID_PASTE_COLLUMNS,
	GRID_PASTE_TRANSLATION,
	GRID_TRANSLATION_TEXT_MOVE,
	GRID_MAKE_LINES_CONTINUES,
	GRID_SET_VIDEO_FPS,
	GRID_SET_CUSTOM_FPS,
	GRID_SWAP_LINES,
	GRID_CONVERT,
	GRID_SORT_LINES,
	GRID_DELETE_LINES,
	GRID_DELETE_TEXT,
	GRID_SET_START_TIME,
	GRID_SET_END_TIME,
	GRID_TURN_ON_TLMODE,
	GRID_TURN_OFF_TLMODE,
	GRID_APPEND_LINE,
	GRID_INSERT_ROW,
	AUDIO_CHANGE_TIME,
	SNAP_TO_KEYFRAME_OR_LINE_TIME,
	ASS_PROPERTIES,
	SELECT_LINES,
	SHIFT_TIMES,
	SPELL_CHECKER,
	STYLE_MANAGER,
	SUBTITLES_RESAMPLE,
	VISUAL_POSITION,
	VISUAL_MOVE,
	VISUAL_SCALE,
	VISUAL_ROTATION_Z,
	VISUAL_ROTATION_X_Y,
	VISUAL_RECT_CLIP,
	VISUAL_VECTOR_CLIP,
	VISUAL_DRAWING,
	VISUAL_POSITION_SHIFTER,
	REPLACE_SINGLE,
	REPLACE_ALL,
	AUTOMATION_SCRIPT,
};



class File
{
public:
	std::vector<Dialogue*> dials;
	std::vector<Styles*> styles;
	std::vector<SInfo*> sinfo;
	std::vector<Dialogue*> ddials;
	std::vector<Styles*> dstyles;
	std::vector<SInfo*> dsinfo;
	std::set<int> sel;

	unsigned char etidtionType;
	int activeLine;
	File();
	~File();
	void Clear();
	File *Copy(bool copySelections = true);
};

class SubsFile
{
	friend class SubsGridBase;
private:
	std::vector<File*> undo;
	int iter;
	File *subs;
	AVLtree *IdConverter;
public:
	SubsFile();
	~SubsFile();
	void SaveUndo(unsigned char editionType, int activeLine);
	void Redo();
	void Undo();
	void DummyUndo();
	void DummyUndo(int newIter);
	void LoadVisibleDialogues();
	void ReloadVisibleDialogues();
	void EndLoad(unsigned char editionType, int activeLine);
	Dialogue *CopyDial(int i, bool push=true, bool keepstate=false);
	Dialogue *GetDialogue(int i);
	Dialogue *&operator[](int i);
	void DeleteDialogues(int from, int to);
	Styles *CopyStyle(int i, bool push=true);
	SInfo *CopySinfo(int i, bool push=true);
	void GetURStatus(bool *_undo, bool *_redo);
	bool IsNotSaved();
	int maxx();
	int Iter();
	void RemoveFirst(int num);
	File *GetSubs();
	void ShowHistory(wxWindow *parent, std::function<void(int)> functionAfterChangeHistory);
	void GetHistoryTable(wxArrayString *history);
	bool SetHistory(int iter);
	bool edited;
};

class HistoryDialog : public KaiDialog
{
public:
	HistoryDialog(wxWindow *parent, SubsFile *file, std::function<void(int)> functionAfterChangeHistory );
	virtual ~HistoryDialog(){};
};

enum{
	ID_HISTORY_LIST =777,
	ID_SET_HISTORY,
	ID_SET_HISTORY_AND_CLOSE,
};