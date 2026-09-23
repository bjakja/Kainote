//  Copyright (c) 2016 - 2026, Marcin Drob

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
#include <unordered_set>
#include <wx/hashmap.h>
#include <wx/regex.h>

#include "styles.h"
#include "SubsDialogue.h"
#include "KaiScrollbar.h"
#include "LineParse.h"
#include "SubsFile.h"
#include <vector>
#include <set>

class EditBox;
class KainoteFrame;
class TabPanel;
class SubsGridPreview;
class GraphicsContext;

// What ChangeTimes shifts and how, as the shift times panel sets it.
struct ShiftTimesSettings
{
	//1 forward, 2 start time for video/audio timing, 4 move to video time,
	//8 move to audio time, 16 display frames, 32 move tag times
	int options = 0;
	int time = 0;
	int frames = 0;
	int whichLines = 0;
	int whichTimes = 0;
	int correctEndTimes = 0;
	int timePerCharacter = 0;
	wxString styles;
	//1 lead in, 2 lead out, 4 make times continuous, 8 snap to keyframes, 16 postprocessor shown
	int postprocessor = 0;
	int leadIn = 0;
	int leadOut = 0;
	int thresholdStart = 0;
	int thresholdEnd = 0;
	int keyframeBeforeStart = 0;
	int keyframeAfterStart = 0;
	int keyframeBeforeEnd = 0;
	int keyframeAfterEnd = 0;

	// what the panel last saved
	static ShiftTimesSettings FromOptions();
};

class compareData{
public:
	compareData(){};
	//_wxArraywxArrayInt &operator [](size_t i)const{ return lineCompare[i]; }
	const int &operator [](size_t i)const{ return lineCompare[i]; }
	size_t size()const{ return lineCompare.size(); }
	void push_back(const int &elem){ lineCompare.Add(elem); }
	int secondComparedLine = -1;
	bool differences = true;
private:
	wxArrayInt lineCompare;
};

// The subtitle grid: draws a subtitle file and edits it. Its methods are
// spread over SubsGridBase.cpp (editing), SubsGridWindow.cpp (drawing and
// input) and SubsGrid.cpp (menu commands).
class SubsGrid : public KaiScrolledWindow
{
	friend class SubsGridPreview;
public:
	SubsGrid(wxWindow* parent, KainoteFrame* kfparent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = 0);
	virtual ~SubsGrid();

	// editing
	void ChangeLine(unsigned char editionType, Dialogue *line1, size_t wline, long cells, bool selline = false, bool dummy = false);
	void ChangeCell(long cells, size_t wline, Dialogue *what);
	void Clearing(bool setup = true);
	void Convert(char type);

	void SaveFile(const wxString &filename, bool cstat = true, bool loadFromEditbox = false);
	void ChangeTimes(const ShiftTimesSettings &settings, bool byFrame = false);

	void SortIt(short what, bool all = true);
	void DeleteRows();
	void DeleteRow(int rw, int len = 1);
	void DeleteText();
	void DoUndo(bool redo, int iter = -2);
	//Warning!! Adding the same dialogue pointer to destroyer cause crash
	//not adding it when needed cause memory leaks.
	void InsertRows(int Row, const std::vector<Dialogue *> &RowsTable, bool AddToDestroy = false);
	//Warning!! Adding the same dialogue pointer to destroyer cause crash
	//not adding it when needed cause memory leaks.
	void InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy = true, bool Save = false);
	void SetSubsFormat(wxString ext = emptyString);
	//records an undo step for the changes made since the last one and repaints the grid
	void SetModified(unsigned char editionType, bool redit = true, bool dummy = false, int SetEditBoxLine = -1, bool Scroll = true);
	void UpdateUR(bool tolbar = true);
	void GetAssHeader(wxString* header, bool forFile = false, bool translated = false, bool normalSave = true);
	void SwapRows(int frst, int scnd, bool sav = false);
	void LoadSubtitles(const wxString &str, wxString &ext);
	bool MoveRows(int step, bool keyStep = false);
	void SetStartTime(int stime);
	void SetEndTime(int etime);
	bool SetTlMode(bool turnOn, bool dontShowDialog = false);
	void LoadDefault(bool line = true, bool sav = true, bool endload = true);
	void GetASSRes(int *x, int *y);
	void GetLayoutRes(int* x, int* y);
	void SetLayoutFromSubsRes();
	void NextLine(int dir = 1);
	void SaveSelections(bool clear = false);
	// no checks, check if value is unsure
	Dialogue *CopyDialogue(size_t i, bool push = true);
	// returns null when there's no visible dialogue with that offset or it is out of the table
	Dialogue *CopyDialogueWithOffset(size_t i, int offset, bool push = true);
	// returns null when is out of range
	Dialogue *GetCurrentLine(){
		if (currentLine < file->GetCount())
			return file->GetDialogue(currentLine);

		return NULL;
	}
	// returns null when there's no visible dialogue with that offset or it is out of the table
	Dialogue *GetDialogueWithOffset(size_t i, int offset);
	// returns visible lines as string for Vsfilter
	wxString *GetVisible(bool *visible = 0, wxPoint *point = nullptr, wxArrayInt *selected = nullptr, bool allSubs = false);
	void SelectVisible();
	bool IsLineVisible(bool visibleOnPlay = true);
	//Get line key from scrollPosition.
	//Every value will be stored as key.
	//Simple function to convert key to id from scroll position
	//to use with mouse, scroll events
	size_t GetKeyFromScrollPos(int numOfLines);
	//it should works without checks;
	size_t GetKeyFromPosition(size_t position, int delta, bool safe = true);
	size_t GetDialoguePosition(size_t keyPosition);

	void DummyUndo(int newIter);
	void GetCommonStyles(SubsGrid *grid, wxArrayString &styleTable);
	int GetScrollPosition(){ return scrollPosition; }
	void SetMDVDTime();

	// drawing and input
	void AdjustWidths(int cell = 16383);
	void AdjustWidthsD2D(GraphicsContext *gc, int cell);
	void ChangeActiveLine(int newActiveLine, bool refresh = false, bool scroll = false, bool changeEditboxLine = true);
	void ChangeTimeDisplay(bool frame);
	void HideOverrideTags();
	void RefreshColumns(int cell = 16383);
	void RefreshIfVisible(int time);
	void ScrollTo(int y, bool center = false, int offset = 0, bool useUpdate = false);
	// default value -1 can cause problems with scrolling
	// that I removed it currentLine is easy to obtain
	void MakeVisible(int rowKey);
	void SelectRow(int row, bool addToSelected = false, bool select = true, bool norefresh = false);
	void SelVideoLine(int time = -1);
	void SetStyle();
	void SetVideoLineTime(wxMouseEvent &evt, int mvtal);
	void SetActive(int line);
	void ShowSecondComparedLine(int Line, bool showPreview = false, bool fromPreview = false, bool setViaScroll = false);
	void RefreshPreview();
	void ClosePreviewWindows(bool refresh = false);

	// commands
	void MoveTextTL(char mode);
	void ResizeSubs(float xnsize, float ynsize, bool stretch);
	void OnMkvSubs(wxCommandEvent &event);
	void ConnectAcc(int id);
	void OnAccelerator(wxCommandEvent &event);
	void OnJoin(wxCommandEvent &event);
	void ContextMenu(const wxPoint &pos);
	void ContextMenuTree(const wxPoint &pos, int treeLine);
	bool SwapAssProperties();
	void RefreshSubsOnVideo(int newActiveLine, bool scroll = true);

	// the subtitles this grid shows and edits
	SubsFile *file = nullptr;
	bool hasTLMode = false;
	bool showOriginal = false;
	bool makebackup;
	char subsFormat = ASS;
	char originalFormat = ASS;
	int markedLine = 0;
	int currentLine = 0;
	bool showFrames = false;
	bool savedSelections = false;
	bool ignoreFiltered = false;
	std::vector<TextData> SpellErrors;
	std::vector<compareData> *Comparison;
	EditBox *edit = nullptr;
	SubsGridPreview *preview = nullptr;
	//comparison static pointers needs short name because we not use this class
	static SubsGrid* CG1;
	static SubsGrid* CG2;
	static void SubsComparison();
	static void RemoveComparison();
	static wxArrayString compareStyles;
	static bool hasCompare;
	wxMutex &GetMutex() { return editionMutex; }

private:
	// sets the edit box line and scrolls to it after an edit
	void ShowEditedLine(int newActive, bool scroll);
	void ShowEditOnVideo(bool afterUndo);
	static void CompareTexts(compareData &firstTable, compareData &secondTable,
		const wxString &first, const wxString &second);
	void OnBackupTimer(wxTimerEvent &event);

	void OnKeyPress(wxKeyEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnPaint(wxPaintEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent& evt);
	bool ShowPreviewWindow(SubsGrid *previewGrid, SubsGrid *windowToDraw,
		int activeLine, int diffPosition);
	void PaintD2D(GraphicsContext *gc, int w, int h, int size, int scrows,
		wxPoint previewpos, wxSize previewsize, bool bg);

	void CopyRows(int id);
	void OnInsertBefore();
	void OnInsertAfter();
	void OnDuplicate();
	void OnPaste(int id);
	void OnPasteTextTl();
	void OnJoinToFirst(int id);
	void InsertWithVideoTime(bool before, bool frameTime = false);
	void OnSetFPSFromVideo();
	void OnSetNewFPS();
	void OnMakeContinous(int id);
	void OnShowPreview();
	void Filter(int id);
	void Split(int id);
	void TreeAddLines(int treeLine);
	void TreeCopy(int treeLine);
	void TreeChangeName(int treeLine);
	void TreeRemove(int treeLine);
	void TreeSelect(int treeLine);

	short numsave;
	bool hideOverrideTags;
	bool ismenushown = false;
	bool first = false;
	int visibleColumns;
	int panelrows = 0;
	int lastActiveLine = 0;
	int lastRow = 0;
	int scrollPosition = 0;
	int scrollPositionId = 0;
	int scHor = 0;
	int GridHeight = 0;
	KainoteFrame* Kai;
	std::vector<bool> visibleLines;
	wxTimer timer;
	wxTimer nullifyTimer;
	TabPanel *tab;
	wxMutex editionMutex;

	int GridWidth[14];
	int posY = 0;
	int posX = 0;
	int row = 0;
	int extendRow = -1;
	int lastsel = -1;
	int oldX = -1;
	bool holding = false;
	int lastWidth = 0;
	int lastHeight = 0;
	wxBitmap* bmp = nullptr;
	// the tree arrows, loaded once: pointing down for a closed tree, up for an open one
	wxBitmap m_TreeArrows[2];
	const wxBitmap &TreeArrow(bool closed);
public:
	// override tags as hide-tags shows them, compiled once
	static wxRegEx &TagsPattern(char format);
	// the style names, to tell unknown styles apart without a search per row
	std::unordered_set<wxString, wxStringHash, wxStringEqual> StyleNames();
private:
	wxFont font;
	SubsGridPreview *thisPreview = nullptr;

	wxArrayInt selections;
	wxArrayString filterStyles;

	DECLARE_EVENT_TABLE()
};

bool sortstart(Dialogue *i, Dialogue *j);

enum{
	ID_AUTIMER
};

enum {
	ID_FILTERING_STYLES = 8005
};
