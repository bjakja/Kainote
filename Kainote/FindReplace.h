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
#include <wx/msw/winundef.h>
#include <wx/regex.h>
#include <wx/arrstr.h>
#include <wx/gdicmn.h>
#include <vector>
#include <atomic> 
//#include <windows.h>


class KainoteFrame;
class TabWindow;
class FindReplaceDialog;
class FindReplaceResultsDialog;
class TabPanel;
class Dialogue;
class SeekResults;

class FindReplace
{
	friend class TabWindow;
	friend class FindReplaceDialog;
public:

	FindReplace(KainoteFrame* kfparent, FindReplaceDialog *FRD);
	~FindReplace(){};
	void ShowResult(TabPanel *tab, const wxString &path, int keyLine, const wxPoint &pos, const wxString & text);
	void ReplaceChecked();
	KainoteFrame *Kai;
	int linePosition;
	int reprow;
	int textPosition;
	int findstart;
	int findend;
	int lastActive;
	wxString oldfind;
	bool fnext = false;
	bool fromstart = false;
	bool blockTextChange = false;
	bool findTextReset = false;
	bool wasResetToStart = false;
	wxArrayString findRecent;
	wxArrayString replaceRecent;
	wxArrayString subsFindingFilters;
	wxArrayString subsFindingPaths;
	wxString actualFind;
	wxString actualReplace;
	wxString actualFilters;
	wxString actualPaths;
	// find replace styles
	wxString stylesAsText;
	bool wasIngored = false;
	// find replace in tab / subs config
	bool startLine = false;
	bool endLine = false;
	bool regEx = false;
	bool matchCase = false;
	bool skipComments;
	bool onlyText;
	bool onlyOption;
	bool searchInOriginal;
	bool find = false;
	bool selectedLines = false;
	bool allLines = false;
	long dialogueColumn;

	//int tabTextPosition;
	int tabLinePosition;
	int positionId;
	int blockPosition;
	int blockLen;
	int nextBlockPosition;
	wxString findString;
	wxString replaceString;
	//wxString subsPath;
	wxString CopyPath;
	wxRegEx findReplaceRegEx;
	// everythin should be set before FindInSubsLine or ReplaceInSubsLine called

	void Find(TabWindow *window);
	void OnFind(TabWindow *window);
	void FindInAllOpenedSubs(TabWindow *window);
	void FindAllInCurrentSubs(TabWindow *window);
	void FindInSubs(TabWindow *window);
	void FindReplaceInSubs(TabWindow *window);
	void FindInSubsLine(wxString *onlyString, Dialogue *dial, TabPanel *tab, bool *isFirst, 
		int linePos, int linePosId, const wxString &subsPath, int thread, bool hasTlMode);
	static unsigned long FindReplaceInFiles(void *data);
	static unsigned long FindAllInTab(void *data);
	int ReplaceInSubsLine(wxString *onlyString);
	void Replace(TabWindow *window);
	int ReplaceAllInTab(TabPanel *tab, TabWindow *window);
	void ReplaceAll(TabWindow *window);
	void ReplaceInAllOpenedSubs(TabWindow *window);
	void ReplaceInSubs(TabWindow *window);
	void AddRecent(TabWindow *window);
	void OnClose();
	void GetFolderFiles(const wxString &path, const wxString &filters, wxArrayString *paths, bool subFolders, bool hiddenFolders);
	
private:
	bool CheckStyles(TabWindow *window, TabPanel *tab);
	bool KeepFinding(const wxString &text, int textPos);
	//when no only option it returns whole text without copying
	bool GetNextBlock(wxString *text, wxString *block);
	bool UpdateValues(TabWindow *window);
	int ReplaceCheckedInSubs(std::vector<SeekResults *> &results, const wxString &copyPath);
	int ReplaceCheckedLine(wxString *line, const wxPoint &pos, int *replacementDiff);
	FindReplaceDialog *FRD = nullptr;
	FindReplaceResultsDialog *FRRD = nullptr;
	std::atomic<int> AllReplacements = 0;
	int numOfProcessors = 4;
};

enum
{
	CASE_SENSITIVE = 1,
	REG_EX,
	START_OF_TEXT = 4,
	END_OF_TEXT = 8,
	IN_FIELD_TEXT = 16,
	IN_FIELD_TEXT_ORIGINAL = 32,
	IN_FIELD_STYLE = 64,
	IN_FIELD_ACTOR = 128,
	IN_FIELD_EFFECT = 256,
	IN_LINES_ALL = 512,
	IN_LINES_SELECTED = 1024,
	IN_LINES_FROM_SELECTION = 2048,
	SEARCH_SUBFOLDERS = 4096,
	SEARCH_HIDDEN_FOLDERS = 8192,
	SEEK_IN_COMMENTS = 1 << 14,
	SEEK_ONLY_IN_TEXT = 1 << 15,
	SEEK_ONLY_IN_TAGS = 1 << 16,
};

