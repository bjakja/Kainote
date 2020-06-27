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

#include "FindReplace.h"
#include "KainoteMain.h"
#include "KaiMessageBox.h"
#include <wx/clipbrd.h>
#include <wx/dir.h>
#include "FindReplaceDialog.h"
#include "FindReplaceResultsDialog.h"
#include "OpennWrite.h"

FindReplace::FindReplace(KainoteFrame* kfparent, FindReplaceDialog *_FRD)
	:FRD(_FRD)
{
	Kai = kfparent;
	lastActive = reprow = linePosition = 0;
	textPosition = 0;
	findstart = -1;
	findend = -1;
	fnext = blockTextChange = false;
	fromstart = true;

	Options.GetTable(FIND_RECENT_FINDS, findRecent, wxTOKEN_RET_EMPTY_ALL);
	if (findRecent.size() > 20){ findRecent.RemoveAt(20, findRecent.size() - 20); }
	if (findRecent.size())
		actualFind = findRecent[0];

	Options.GetTable(REPLACE_RECENT_REPLACEMENTS, replaceRecent, wxTOKEN_RET_EMPTY_ALL);
	if (replaceRecent.size() > 20){ replaceRecent.RemoveAt(20, replaceRecent.size() - 20); }
	if (replaceRecent.size())
		actualReplace = replaceRecent[0];
	
	Options.GetTable(FIND_IN_SUBS_FILTERS_RECENT, subsFindingFilters, wxTOKEN_RET_EMPTY_ALL);
	if (subsFindingFilters.size() > 20){ subsFindingFilters.RemoveAt(20, subsFindingFilters.size() - 20); }
	if (subsFindingFilters.size())
		actualFilters = subsFindingFilters[0];

	Options.GetTable(FIND_IN_SUBS_PATHS_RECENT, subsFindingPaths, wxTOKEN_RET_EMPTY_ALL);
	if (subsFindingPaths.size() > 20){ subsFindingPaths.RemoveAt(20, subsFindingPaths.size() - 20); }
	if (subsFindingPaths.size())
		actualPaths = subsFindingPaths[0];

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	numOfProcessors = (sysinfo.dwNumberOfProcessors > 0) ? sysinfo.dwNumberOfProcessors : 4;
	CopyPath = Options.pathfull + L"\\ReplaceBackup\\";
}

void FindReplace::ShowResult(TabPanel *tab, const wxString &path, int keyLine, const wxPoint &pos, const wxString & text)
{
	if (tab){
		for (size_t i = 0; i < Kai->Tabs->Size(); i++){
			if (Kai->Tabs->Page(i) == tab){
				if (keyLine < tab->Grid->file->GetCount()){
					if (i != Kai->Tabs->iter)
						Kai->Tabs->ChangePage(i);
					//check if it's not out of range
					if (keyLine < tab->Grid->GetCount()){
						tab->Edit->SetLine(keyLine);
						tab->Grid->SelectRow(keyLine);
						tab->Grid->ScrollTo(keyLine, true);
						tab->Edit->GetEditor(text)->SetSelection(pos.x, pos.x + pos.y);
					}
				}
				break;
			}
		}
	}
	else if(wxFileExists(path)){
		bool foundSubs = false;
		for (size_t i = 0; i < Kai->Tabs->Size(); i++){
			if (Kai->Tabs->Page(i)->SubsPath == path){
				Kai->Tabs->ChangePage(i);
				foundSubs = true;
			}
		}
		if (!foundSubs){
			if (!Kai->GetTab()->SubsPath.empty())
				Kai->InsertTab();

			Kai->OpenFile(path);
		}
		TabPanel *ntab = Kai->GetTab();
		if (keyLine < ntab->Grid->GetCount()){
			ntab->Edit->SetLine(keyLine);
			ntab->Grid->SelectRow(keyLine);
			ntab->Grid->ScrollTo(keyLine, true);
			ntab->Edit->GetEditor(text)->SetSelection(pos.x, pos.x + pos.y);
		}
	}
}

void FindReplace::ReplaceChecked()
{
	if (!FRRD)
		return;

	bool needPrefix = false;
	FRRD->GetReplaceString(&replaceString);
	FRRD->GetFindOptions(&regEx, &matchCase, &needPrefix, &findString);
	if (regEx){
		int rxflags = wxRE_ADVANCED;
		if (!matchCase){ rxflags |= wxRE_ICASE; }
		findReplaceRegEx.Compile(findString, rxflags);
		if (!findReplaceRegEx.IsValid()) {
			return;
		}
		if (needPrefix){ replaceString.Prepend(L"\\1"); }
	}
	bool plainText = false;

	KaiListCtrl *List = FRRD->resultsList;
	int replacementDiff = 0;

	if (FRRD->findInFiles){
		wxString path;
		wxString oldPath;
		wxString copyPath = Options.pathfull + L"\\ReplaceBackup\\";
		DWORD ftyp = GetFileAttributesW(copyPath.wc_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES){
			wxMkDir(copyPath);
		}
		int numChanges = 0;
		std::vector<SeekResults*> results;

		size_t size = List->GetCount();
		for (size_t tt = 0; tt < size; tt++){
			Item *item = List->GetItem(tt, 0);
			if (!item || item->type != TYPE_TEXT || !item->modified)
				continue;

			SeekResults *SeekResult = (SeekResults *)item;
			path = SeekResult->path;
			
			if (oldPath != path){
				numChanges += ReplaceCheckedInSubs(results, copyPath);
				results.clear();
			}

			results.push_back(SeekResult);

			oldPath = path;
		}

		if (results.size()){
			numChanges += ReplaceCheckedInSubs(results, copyPath);
		}
	}
	else{
		TabPanel *oldtab = NULL;
		TabPanel *tab = NULL;
		int oldKeyLine = -1;
		int numOfChanges = 0;
		bool skipTab = false;
		bool skipLine = false;
		bool lastIsTextTl = false;
		size_t size = List->GetCount();
		for (size_t tt = 0; tt < size; tt++){
			Item *item = List->GetItem(tt, 0);
			if (!item || item->type != TYPE_TEXT || !item->modified)
				continue;

			SeekResults *SeekResult = (SeekResults *)item;
			tab = SeekResult->tab;
			// check if skip lines when tab not exist
			if (tab != oldtab){
				skipTab = Kai->Tabs->FindPanel(tab, false) == -1;
				oldKeyLine = -1;
				lastIsTextTl = false;
			}
			//skip lines when are out of table range and not existed tab
			if (skipTab || SeekResult->keyLine >= tab->Grid->file->GetCount())
				continue;

			Dialogue *Dialc = tab->Grid->file->CopyDialogue(SeekResult->keyLine, true, false);

			wxString & lineText = Dialc->Text.CheckTlRef(Dialc->TextTl, SeekResult->isTextTL);
			//skip lines with different texts
			if (oldKeyLine != SeekResult->keyLine || SeekResult->isTextTL != lastIsTextTl){
				replacementDiff = 0;
				if (lineText != SeekResult->name){
					KaiLog(wxString::Format(_("Linia %i nie może być zamieniona,\nbo została zedytowana."),
						SeekResult->idLine));
					skipLine = true;
					continue;
				}
				skipLine = false;
			}
			if (skipLine)
				continue;

			numOfChanges += ReplaceCheckedLine(&lineText, SeekResult->findPosition, &replacementDiff);

			if (tab != oldtab && oldtab && numOfChanges){
				oldtab->Grid->SetModified(REPLACED_BY_MISSPELL_REPLACER);
				oldtab->Grid->SpellErrors.clear();
				oldtab->Grid->Refresh(false);
				numOfChanges = 0;
			}


			oldtab = tab;
			oldKeyLine = SeekResult->keyLine;
			lastIsTextTl = SeekResult->isTextTL;
		}

		if (tab && numOfChanges){
			tab->Grid->SetModified(REPLACED_BY_MISSPELL_REPLACER);
			tab->Grid->SpellErrors.clear();
			tab->Grid->Refresh(false);
		}
	}

}

int FindReplace::ReplaceCheckedLine(wxString *line, const wxPoint &pos, int *replacementDiff)
{
	int reps = 1;
	if (regEx){
		wxString foundString = line->Mid(pos.x - (*replacementDiff), pos.y);
		reps = findReplaceRegEx.Replace(&foundString, replaceString);
		if (reps > 0){
			line->replace(pos.x - (*replacementDiff), pos.y, foundString);
			*replacementDiff += pos.y - foundString.length();
		}
	}
	else{
		line->replace(pos.x - (*replacementDiff), pos.y, replaceString);
		*replacementDiff += pos.y - replaceString.length();
	}

	return reps;
}

void FindReplace::Find(TabWindow *window)
{
	if (window->windowType == WINDOW_FIND_IN_SUBS){
		KaiLogDebug(L"chujnia replace all wywołane z okna find in subs");
		return;
	}
	TabPanel *tab = Kai->GetTab();
	if (UpdateValues(window))
		return;

	if (findString != oldfind){ 
		fromstart = true; 
		fnext = false; 
		oldfind = findString; 
	}
	if (!fromstart && lastActive != tab->Grid->currentLine){
		lastActive = tab->Grid->currentLine;
	}

	wxString txt;
	int foundPosition = -1;
	size_t foundLength = 0;

seekFromStart:

	bool foundsome = false;
	if (fromstart){
		linePosition = tab->Grid->FirstSelection();
		//is it possible to get it -1 if id exists, key also should exist
		linePosition = (!window->AllLines->GetValue() && linePosition != -1) ? linePosition : 0;
		textPosition = 0;
	}
	if (CheckStyles(window, tab))
		return;
	bool styles = !stylesAsText.empty();
		
	bool onlysel = window->SelectedLines->GetValue();
	SubsFile *Subs = tab->Grid->file;
	bool tlmode = tab->Grid->hasTLMode;

	while (linePosition < Subs->GetCount())
	{
		Dialogue *Dial = Subs->GetDialogue(linePosition);
		if (!Dial->isVisible || (skipComments && Dial->IsComment)){ linePosition++; textPosition = 0; continue; }
		
		if ((!styles && !onlysel) ||
			(styles && stylesAsText.Find(L"," + Dial->Style + L",") != -1) ||
			(onlysel && tab->Grid->file->IsSelected(linePosition))){
			Dial->GetTextElement(dialogueColumn, &txt, tlmode);

			foundPosition = -1;
			//Here we seeking
			if (!(startLine || endLine) && (findString.empty() || txt.empty()))
			{
				if (txt.empty() && findString.empty()){
					foundPosition = 0; foundLength = 0;
				}
				else{ textPosition = 0; linePosition++; continue; }

			}
			else if (regEx){
				
				wxString cuttext = txt.Mid(textPosition);
				if (findReplaceRegEx.Matches(cuttext)) {
					size_t regexStart = 0;
					findReplaceRegEx.GetMatch(&regexStart, &foundLength, 0);
					foundPosition = regexStart + textPosition;
				}
				else{ textPosition = 0; linePosition++; continue; }

			}
			else{
				wxString ltext = (!matchCase) ? txt.Lower() : txt;
				wxString lfind = (!matchCase) ? findString.Lower() : findString;
				if (startLine){
					if (ltext.StartsWith(lfind) || lfind.empty()){
						foundPosition = 0;
						textPosition = 0;
					}
				}
				if (endLine){
					if (ltext.EndsWith(lfind) || lfind.empty()){
						foundPosition = txt.length() - lfind.length();
						textPosition = 0;
					}
				}
				else{
					foundPosition = ltext.find(lfind, textPosition);
				}
				foundLength = lfind.length();
			}

			if (foundPosition != -1 && (!onlyOption || KeepFinding(txt, foundPosition))){
				textPosition = foundPosition + foundLength;
				findstart = foundPosition;
				findend = textPosition;
				lastActive = reprow = linePosition;
				if (!onlysel){ tab->Grid->SelectRow(linePosition, false, true); }
				tab->Edit->SetLine(linePosition);
				tab->Grid->ScrollTo(linePosition, true);
				if (onlysel){ tab->Grid->Refresh(false); }
				if (dialogueColumn == STYLE){
					//pan->Edit->StyleChoice->SetFocus();
				}
				else if (dialogueColumn == TXT){
					//find if is txt or txttl
					//txttl is added after \n
					if (tlmode && txt.Find(L'\n') != -1){
						size_t pos = txt.Find(L'\n');
						if(foundPosition > pos)
							tab->Edit->TextEdit->SetSelection(foundPosition - pos - 1, findend - pos - 1);
						else
							tab->Edit->TextEditOrig->SetSelection(foundPosition, findend);
					}else
						tab->Edit->TextEdit->SetSelection(foundPosition, findend);
				}
				if (dialogueColumn == ACTOR){
					//pan->Edit->ActorEdit->SetFocus();
					tab->Edit->ActorEdit->choiceText->SetSelection(foundPosition, findend);
				}
				if (dialogueColumn == EFFECT){
					//pan->Edit->EffectEdit->SetFocus();
					tab->Edit->EffectEdit->choiceText->SetSelection(foundPosition, findend);
				}

				foundsome = true;
				if ((size_t)textPosition >= txt.length() || startLine){
					linePosition++; textPosition = 0;
				}
				break;
			}
			else{
				textPosition = 0;
				linePosition++;
			}

		}
		else{ textPosition = 0; linePosition++; }
		if (!foundsome && linePosition > Subs->GetCount() - 1){
			break;
		}
	}
	if (!foundsome){
		blockTextChange = true;
		linePosition = 0;
		fromstart = true;
		if (!wasResetToStart){
			if (KaiMessageBox(_("Wyszukiwanie zakończone, rozpocząć od początku?"), _("Potwierdzenie"),
				wxICON_QUESTION | wxYES_NO, FRD) == wxYES){
				wasResetToStart = true;
				goto seekFromStart;
			}
		}
		else{
			KaiMessageBox(_("Nie znaleziono podanej frazy \"") + window->FindText->GetValue() + L"\".", _("Potwierdzenie"));
			wasResetToStart = false;
		}
	}
	if (fromstart){ AddRecent(window); fromstart = false; }
}

void FindReplace::OnFind(TabWindow *window)
{
	Find(window);
	fnext = false;
	findTextReset = true;
}

//returns true if regex is not valid
//it will inform if pattern is not valid
//instead of ignoring all seeking
//tabLinePosition and positionId must be set
DWORD FindReplace::FindAllInTab(void *data)
{
	std::tuple<FindReplace*, wxPoint, TabWindow*, int> *actualData = 
		(std::tuple<FindReplace*, wxPoint, TabWindow*, int>*)data;
	FindReplace* fr = std::get<0>(*actualData);
	wxPoint tabRange = std::get<1>(*actualData);
	TabWindow* window = std::get<2>(*actualData);
	int thread = std::get<3>(*actualData);

	long dialogueColumn = fr->dialogueColumn;
	
	wxString txt;
	Notebook *tabs = fr->Kai->Tabs;
	//<= for range from 1 to 1 
	for (int i = tabRange.x; i <= tabRange.y; i++){
		TabPanel *tab = tabs->Page(i);
		bool hasTlMode = tab->Grid->hasTLMode;

		int positionId = 0;
		wxString subsPath = tab->SubsName;
		bool isfirst = true;

		size_t firstSelectedId = 0;
		int tabLinePosition = tab->Grid->FirstSelection(&firstSelectedId);
		tabLinePosition = (!window->AllLines->GetValue() && tabLinePosition != -1) ? tabLinePosition : 0;
		if (tabLinePosition > 0 && firstSelectedId != -1)
			positionId = firstSelectedId;

		bool styles = !fr->stylesAsText.empty();

		bool onlySelections = window->SelectedLines->GetValue();
		SubsFile *Subs = tab->Grid->file;

		for (; tabLinePosition < Subs->GetCount(); tabLinePosition++)
		{
			Dialogue *Dial = Subs->GetDialogue(tabLinePosition);
			if (!Dial->isVisible){ continue; }
			if (fr->skipComments && Dial->IsComment){ positionId++; continue; }

			if ((!styles && !onlySelections) ||
				(styles && fr->stylesAsText.Find(L"," + Dial->Style + L",") != -1) ||
				(onlySelections && tab->Grid->file->IsSelected(tabLinePosition))){

				Dial->GetTextElement(dialogueColumn, &txt, hasTlMode);

				fr->FindInSubsLine(&txt, Dial, tab, &isfirst, tabLinePosition, positionId, subsPath, thread, hasTlMode);
			}
			positionId++;
		}
	}
	delete actualData;
	return 0;
}

void FindReplace::FindInAllOpenedSubs(TabWindow *window)
{
	if (!FRRD)
		FRRD = new FindReplaceResultsDialog(Kai, this);
	else
		FRRD->ClearList();

	if (CheckStyles(window, Kai->GetTab()))
		return;

	if (UpdateValues(window))
		return;

	int sizeOfTabs = Kai->Tabs->Size();

	/*for (size_t i = 0; i < sizeOfTabs; i++){
		if (FindAllInTab(Kai->Tabs->Page(i), window)){
			return;
		}
	}*/

	int pathsPerThreads = sizeOfTabs / numOfProcessors;
	int moduloPerThreads = sizeOfTabs % numOfProcessors;
	int loopNumber = (!pathsPerThreads) ? sizeOfTabs : numOfProcessors;

	FRRD->SetupMultiThreading(loopNumber);
	HANDLE *threads = new HANDLE[loopNumber];
	int tabPos = 0;
	for (int i = 0; i < loopNumber; i++){
		int diff = (i < moduloPerThreads) ? 1 : 0;
		wxPoint tabRange(tabPos, tabPos + (pathsPerThreads + diff) - 1);
		
		std::tuple<FindReplace*, wxPoint, TabWindow*, int> *data =
			new std::tuple<FindReplace *, wxPoint, TabWindow*, int>(this, tabRange, window, i);
		threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FindAllInTab, data, 0, 0);
		tabPos += (pathsPerThreads + diff);
	}//for
	WaitForMultipleObjects(loopNumber, threads, TRUE, INFINITE);
	delete[] threads;

	FRRD->EndMultiThreading();
	FRRD->FilterList();
	bool needPrefix = endLine && regEx && findString.empty();
	FRRD->SetFindOptions(regEx, matchCase, needPrefix, findString);
	if (!FRRD->IsShown())
		FRRD->Show();

	AddRecent(window);
}

void FindReplace::FindAllInCurrentSubs(TabWindow *window)
{
	if (!FRRD)
		FRRD = new FindReplaceResultsDialog(Kai, this);
	else
		FRRD->ClearList();

	TabPanel *tab = Kai->GetTab();
	if (CheckStyles(window, tab))
		return;

	if (UpdateValues(window))
		return;

	FRRD->SetupMultiThreading(1);

	/*if (FindAllInTab(tab, window)){
		return;
	}*/
	int iter = Kai->Tabs->iter;
	std::tuple<FindReplace*, wxPoint, TabWindow*, int> *data =
		new std::tuple<FindReplace *, wxPoint, TabWindow*, int>(this, wxPoint(iter, iter), window, 0);

	FindAllInTab(data);

	FRRD->EndMultiThreading();
	bool needPrefix = endLine && regEx && findString.empty();
	FRRD->SetFindOptions(regEx, matchCase, needPrefix, findString);
	FRRD->FilterList();
	if (!FRRD->IsShown())
		FRRD->Show();

	AddRecent(window);
}

void FindReplace::FindInSubs(TabWindow *window)
{
	find = true;
	FindReplaceInSubs(window);
}

DWORD FindReplace::FindReplaceInFiles(void *data)
{
	std::tuple<FindReplace*, wxArrayString*, int> *actualData = (std::tuple<FindReplace*, wxArrayString*, int>*)data;
	FindReplace* fr = std::get<0>(*actualData);
	wxArrayString* paths = std::get<1>(*actualData);
	int thread = std::get<2>(*actualData);

	OpenWrite ow;
	bool plainText = false;
	wxString subsPath;
	int SubsAllReplacements = 0;

	for (size_t i = 0; i < paths->size(); i++){
		
		wxString subsText;
		subsPath = (*paths)[i];

		//we must not remove files from table
		//every thread need to get about the same number of files +-1
		//filters must remove unneeded files
		//especially video
		wxString ext = subsPath.AfterLast(L'.').Lower();
		//if (onlyAss && ext != L"ass"/* && ext != "ssa"*/)
		//	continue;
		//else if (ext != L"ass"/* && ext == "ssa"*/ && ext != L"srt" && ext != L"mpl2" && ext != L"sub" && ext != L"txt")
		//	continue;

		ow.FileOpen(subsPath, &subsText);

		wxString replacedText;
		int tabLinePosition = 0; int positionId = 0;
		bool isSRT = (ext == L"srt");
		bool isASS = (ext == L"ass");
		wxString TlModeStyle;
		bool hasTlMode = false;

		if ((isASS/* || ext == "ssa"*/) && !plainText){
			size_t tlModeStylePos = subsText.find(L"TLMode Style:");
			if (tlModeStylePos != -1){
				hasTlMode = true;
				size_t nposition = subsText.find(L"\n", tlModeStylePos + 13);
				if (nposition != -1){
					TlModeStyle = subsText.Mid(tlModeStylePos + 13, nposition - (tlModeStylePos + 13));
					TlModeStyle.Trim(true).Trim(false);
				}
			}
			size_t result = subsText.find(/*(ext == "ass") ? */L"Dialogue:"/* : L"Marked="*/);
			size_t result1 = subsText.find(/*(ext == "ass") ? */L"Comment:"/* : L"Marked="*/);
			if (result == -1 && result1 == -1)// no dialogues;
				continue;
			else{
				if (result1 < result)
					result = result1;
				if (!fr->find){
					replacedText = subsText.Mid(0, result);
					replacedText.Replace(L"\n", L"\r\n");
				}
				subsText = subsText.Mid(result);
			}
		}

		wxStringTokenizer tokenizer(subsText, L"\n", wxTOKEN_STRTOK);
		wxString token;
		wxString dialtxt;
		Dialogue *dial = NULL;
		bool isFirst = true;

		while (tokenizer.HasMoreTokens()){

			if (isSRT){
				wxString text = tokenizer.GetNextToken();
				bool noMoreTokens = !tokenizer.HasMoreTokens();
				if (IsNumber(text) || noMoreTokens){
					if (noMoreTokens)
						token << text << L"\r\n";

					if (token != L""){
						token.Trim();
						if (!plainText)
							dial = new Dialogue(token);

					}
					else{
						continue;
					}
				}
				else{
					//lol why I got text without \r?
					//I have to put it myself
					token << text << L"\r\n";
					continue;
				}
			}
			else{
				token = tokenizer.GetNextToken();
				token.Trim();
				if (!plainText)
					dial = new Dialogue(token);
			}
			if (!dial || (plainText && token.empty())){
				continue;
			}

			//here we got dial or plain text
			//we have to get only text
			dial->GetTextElement(fr->dialogueColumn, &dialtxt);
			if (dial->IsComment && fr->skipComments){
				bool notTlStyle = dial->Style != TlModeStyle;
				if (!isASS || !hasTlMode || !dial || notTlStyle){
					tabLinePosition++;
					positionId++;
				}
				if (notTlStyle) {
					delete dial;
					continue;
				}
			}


			if (fr->find){
				fr->FindInSubsLine(&dialtxt, dial, NULL, &isFirst, tabLinePosition, positionId, subsPath, thread, false);
			}
			else{
				int numOfReps = fr->ReplaceInSubsLine(&dialtxt);
				if (numOfReps){
					if (isSRT)
						replacedText << (tabLinePosition + 1) << L"\r\n";

					dial->SetTextElement(fr->dialogueColumn, dialtxt);
					dial->GetRaw(&replacedText);

					SubsAllReplacements += numOfReps;
				}
				else{
					if (isSRT)
						replacedText << (tabLinePosition + 1) << L"\r\n";

					replacedText << token << L"\r\n";

					if (isSRT)
						replacedText << L"\r\n";
				}
			}

			if (!isASS || !hasTlMode || !dial || dial->Style != TlModeStyle){
				tabLinePosition++;
				positionId++;
			}

			if (dial){
				delete dial;
				dial = NULL;
			}
			token.clear();
		}//while
		if (SubsAllReplacements){
			wxCopyFile(subsPath, fr->CopyPath + subsPath.AfterLast(L'\\'));
			ow.FileWrite(subsPath, replacedText);
			fr->AllReplacements.fetch_add(SubsAllReplacements);
		}
	}
	delete actualData;
	delete paths;
	return 0;
}

void FindReplace::FindReplaceInSubs(TabWindow *window)
{
	wxString path = window->FindInSubsPath->GetValue();
	wxArrayString paths;

	bool onlyAss = !(window->CollumnText->GetValue());
	//block to manual edition of filters cause
	//program will not check if it is valid
	wxString filters = window->FindInSubsPattern->GetValue();
	if (filters.empty())
		filters = L"*.ass;*.srt;*.sub;*.txt;*.mpl2";
	if (onlyAss)
		filters = L"*.ass";

	GetFolderFiles(path, filters, &paths,
		window->SeekInSubFolders->GetValue(), window->SeekInHiddenFolders->GetValue());

	int i = 0;
	while (i < paths.size()){
		wxString ext = paths[i].AfterLast(L'.').Lower();
		if (ext != L"ass" && ext != L"srt" && ext != L"sub" && ext != L"txt" && ext != L"mpl2"){
			paths.RemoveAt(i);
		}
		else
			i++;
	}

	if (!paths.size())
		return;

	if (!find){
		DWORD ftyp = GetFileAttributesW(CopyPath.wc_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES){
			wxMkDir(CopyPath);
		}
	}
	//bool plainText = false;//(window->CollumnTextOriginal->GetValue());
	//no tlmode here

	if (UpdateValues(window))
		return;
	//else if (plainText){ replaceColumn = 0; }

	size_t pathsSize = paths.size();
	int pathsPerThreads = pathsSize / numOfProcessors;
	int moduloPerThreads = pathsSize % numOfProcessors;
	int loopNumber = (!pathsPerThreads) ? pathsSize : numOfProcessors;

	if (find){
		if (!FRRD)
			FRRD = new FindReplaceResultsDialog(Kai, this, true);
		else{
			FRRD->findInFiles = true;
			FRRD->ClearList();
		}
		FRRD->SetupMultiThreading(loopNumber);
	}
	AllReplacements = 0;
	
	HANDLE *threads = new HANDLE[loopNumber];
	int filePos = 0;
	for (int i = 0; i < loopNumber; i++){
		int diff = (i < moduloPerThreads) ? 1 : 0;
		wxArrayString *threadPaths = new wxArrayString(
			paths.begin() + filePos,
			paths.begin() + filePos + (pathsPerThreads + diff));
		std::tuple<FindReplace*, wxArrayString*, int> *data =
			new std::tuple<FindReplace *, wxArrayString *, int>(this, threadPaths, i);
		threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FindReplaceInFiles, data, 0, 0);
		filePos += (pathsPerThreads + diff);
	}//for
	WaitForMultipleObjects(loopNumber, threads, TRUE, INFINITE);

	delete[] threads;

	if (!find && AllReplacements.load()){
		blockTextChange = true;
		KaiMessageBox(wxString::Format(_("Zmieniono %i razy."), AllReplacements.load()), _("Szukaj Zamień"));
		AddRecent(window);
		findTextReset = true;
	}
	else if (find){
		FRRD->EndMultiThreading();
		FRRD->FilterList();
		if (!FRRD->IsShown())
			FRRD->Show();
	}
	AddRecent(window);
}

void FindReplace::FindInSubsLine(wxString *onlyString, Dialogue *dial, TabPanel *tab, bool *isFirst, 
	int linePos, int linePosId, const wxString &subsPath, int thread, bool hasTlMode)
{
	int foundPosition = 0;
	size_t foundLength = 0;
	int tabTextPosition = 0;

	while (1){
		foundPosition = -1;

		if (!(startLine || endLine) && (findString.empty() || onlyString->empty()))
		{
			if (onlyString->empty() && findString.empty()){
				foundPosition = 0; foundLength = 0;
			}
			else{ break; }

		}
		else if (regEx){
			wxString cuttext = onlyString->Mid(tabTextPosition);
			if (findReplaceRegEx.Matches(cuttext)) {
				size_t regexStart = 0;
				findReplaceRegEx.GetMatch(&regexStart, &foundLength, 0);
				foundPosition = regexStart + tabTextPosition;
			}
			else{ break; }
		}
		else{
			wxString ltext = (!matchCase) ? onlyString->Lower() : *onlyString;
			wxString lfind = (!matchCase) ? findString.Lower() : findString;
			if (startLine){
				if (ltext.StartsWith(lfind) || lfind.empty()){
					foundPosition = 0;
					tabTextPosition = 0;
				}
			}
			if (endLine){
				if (ltext.EndsWith(lfind) || lfind.empty()){
					foundPosition = onlyString->length() - lfind.length();
					tabTextPosition = 0;
				}
			}
			else{
				foundPosition = ltext.find(lfind, tabTextPosition);
			}
			foundLength = lfind.length();
		}

		if (foundPosition != -1 && (!onlyOption || KeepFinding(*onlyString, foundPosition))){
			if (*isFirst){
				FRRD->SetHeader(subsPath, thread);
				*isFirst = false;
			}
			if (dialogueColumn < TXT){
				FRRD->SetResults(*onlyString + L"  ->  " + dial->GetTextNoCopy(), wxPoint(foundPosition, foundLength), tab,
					linePosId + 1, linePos, (tab) ? L"" : subsPath, thread);
			}
			else{
				if (hasTlMode && onlyString->Find(L'\n') != -1){
					wxPoint txtPos(foundPosition, foundLength);
					wxString lineText;
					size_t nPos = onlyString->Find(L'\n');
					bool isTextTl = nPos < foundPosition;
					if (isTextTl){
						txtPos.x = foundPosition - nPos - 1;
						lineText = onlyString->Mid(nPos + 1);
					}else
						lineText = onlyString->Mid(0, nPos);

					FRRD->SetResults(lineText, txtPos, tab,
						linePosId + 1, linePos, (tab) ? L"" : subsPath, thread, isTextTl);
				}
				else{
					FRRD->SetResults(*onlyString, wxPoint(foundPosition, foundLength), tab,
						linePosId + 1, linePos, (tab) ? L"" : subsPath, thread);
				}
			}

			if ((size_t)tabTextPosition >= onlyString->length() || startLine){
				tabTextPosition = 0;
				break;
			}
			else
				tabTextPosition = foundPosition + 1;

		}
		else{
			break;
		}
	}
}

int FindReplace::ReplaceInSubsLine(wxString *onlyString)
{
	if (!(startLine || endLine) && (findString.empty() || onlyString->empty()))
	{
		if (onlyString->empty() && findString.empty())
		{
			*onlyString = replaceString;
			return 1;
		}
		else{ return 0; }
	}
	else if (startLine || endLine){

		wxString ltext = (!matchCase) ? onlyString->Lower() : *onlyString;
		wxString lfind = (!matchCase) ? findString.Lower() : findString;
		bool startsTagBlock = ltext.StartsWith(L"{");
		bool endsTagBlock = ltext.EndsWith(L"}");
		if (startLine && (!onlyOption || (!startsTagBlock && onlyText) || (startsTagBlock && !onlyText))){
			if (ltext.StartsWith(lfind) || lfind.empty()){
				onlyString->replace(0, lfind.length(), replaceString);
				return 1;
			}
			return 0;
		}
		if (endLine && (!onlyOption || (!endsTagBlock && onlyText) || (endsTagBlock && !onlyText))){
			if (ltext.EndsWith(lfind) || lfind.empty()){
				int lenn = onlyString->length();
				onlyString->replace(lenn - lfind.length(), lenn, replaceString);
				return 1;
			}
			return 0;
		}
		return 0;
	}
	int linereps = 0;
	
	wxString lfind = (matchCase) ? findString : findString.Lower();
	wxString ltext = (matchCase) ? *onlyString : onlyString->Lower();

	int newpos = 0;
	size_t flen = lfind.length();
	size_t textPos = 0;
	int repsDiff = 0;

	// especially for tags or text elimination I made replace by match
	// should be faster than generate text blocks and replace it within
	while (1){
		if (regEx){
			if (!findReplaceRegEx.Matches(ltext.Mid(newpos)) || !findReplaceRegEx.GetMatch(&textPos, &flen))
				break;

			textPos += newpos;
		}
		else
			textPos = ltext.find(lfind, newpos);

		
		newpos = textPos + lfind.length();
		//diff for replacing and checking function
		if (textPos == -1 || (textPos + repsDiff) >= onlyString->length()){ break; }
		textPos += repsDiff;

		if (!onlyOption || KeepFinding(*onlyString, textPos)){
			
			if (regEx){
				wxString match = onlyString->Mid(textPos, flen);
				findReplaceRegEx.Replace(&match, replaceString);
				onlyString->replace(textPos, flen, match);
				repsDiff += match.length() - flen;
			}
			else{
				onlyString->replace(textPos, flen, replaceString);
				repsDiff += replaceString.length() - flen;
			}

			linereps++;
		}
	}
	
	return linereps;
}

void FindReplace::Replace(TabWindow *window)
{
	if (window->windowType != WINDOW_REPLACE){
		KaiLog(L"chujnia replace all wywołane nie z okna replace");
		return;
	}
	TabPanel *tab = Kai->GetTab();
	if (lastActive != tab->Grid->currentLine){ Find(window); }
	long wrep = TXT;
	if (window->CollumnStyle->GetValue()){ wrep = STYLE; }
	else if (window->CollumnActor->GetValue()){ wrep = ACTOR; }
	else if (window->CollumnEffect->GetValue()){ wrep = EFFECT; }

	wxString find1 = window->FindText->GetValue();
	if (find1 != oldfind || findstart == -1 || findend == -1){ 
		fromstart = true; 
		fnext = false; 
		oldfind = find1; 
		Find(window); 
	}
	if (findstart == -1 || findend == -1){ return; }
	fnext = true;
	wxString rep = window->ReplaceText->GetValue();
	SubsGrid *grid = tab->Grid;

	Dialogue *Dialc = grid->CopyDialogue(reprow);
	bool hasRegEx = window->RegEx->GetValue();
	wxString replacedText;
	Dialc->GetTextElement(wrep, &replacedText, tab->Grid->hasTLMode);
	if (hasRegEx && findReplaceRegEx.IsValid()){
		wxString place = replacedText.Mid(findstart, findend - findstart);
		int reps = findReplaceRegEx.Replace(&place, rep, 1);
		replacedText.replace(findstart, findend - findstart, (reps) ? place : rep);
	}
	else{
		replacedText.replace(findstart, findend - findstart, rep);
	}

	if (wrep == STYLE){
		tab->Edit->StyleChoice->SetSelection(tab->Edit->StyleChoice->FindString(replacedText));
	}
	else if (wrep == TXT){
		if (tab->Grid->hasTLMode && replacedText.Find(L'\n') != -1){
			wxString textTl;
			wxString text = replacedText.BeforeFirst(L'\n', &textTl);
			tab->Edit->TextEditOrig->SetTextS(text);
			tab->Edit->TextEdit->SetTextS(textTl);
		}
		else
			tab->Edit->TextEdit->SetTextS(replacedText);
	}
	else if (wrep == ACTOR){
		tab->Edit->ActorEdit->choiceText->SetValue(replacedText);
	}
	else if (wrep == EFFECT){
		tab->Edit->EffectEdit->choiceText->SetValue(replacedText);
	}
	Dialc->SetTextElement(wrep, replacedText, tab->Grid->hasTLMode);
	grid->SetModified(REPLACE_SINGLE);
	grid->Refresh(false);
	textPosition = findstart + rep.length();
	Find(window);
}

int FindReplace::ReplaceAllInTab(TabPanel *tab, TabWindow *window)
{
	int allRelpacements = 0;
	int allreps = 0;
	wxString txt;
	
	bool notstyles = stylesAsText.empty();

	bool onlysel = window->SelectedLines->GetValue();

	size_t firstSelection = tab->Grid->FirstSelection();
	SubsFile *Subs = tab->Grid->file;
	bool skipFiltered = !tab->Grid->ignoreFiltered;
	bool hasTlMode = tab->Grid->hasTLMode;

	for (size_t i = (!window->AllLines->GetValue() && firstSelection != -1) ? firstSelection : 0; i < Subs->GetCount(); i++)
	{
		Dialogue *Dial = Subs->GetDialogue(i);
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue || (skipComments && Dial->IsComment)){ continue; }
		
		if ((notstyles || stylesAsText.Find(L"," + Dial->Style + L",") != -1) &&
			!(onlysel && !(tab->Grid->file->IsSelected(i)))){

			Dial->GetTextElement(dialogueColumn, &txt, hasTlMode);
			allreps = ReplaceInSubsLine(&txt);
			if (allreps > 0){
				Dialogue *Dialc = tab->Grid->file->CopyDialogue(i);
				Dialc->SetTextElement(dialogueColumn, txt, hasTlMode);
				allRelpacements += allreps;
			}
		}

	}

	return allRelpacements;
}

void FindReplace::ReplaceAll(TabWindow *window)
{
	if (window->windowType != WINDOW_REPLACE){
		KaiLogDebug(L"chujnia replace all wywołane nie z okna replace");
		return;
	}

	TabPanel *tab = Kai->GetTab();
	if (UpdateValues(window))
		return;

	if (CheckStyles(window, tab))
		return;
	
	int allReplacements = ReplaceAllInTab(tab, window);
	if (allReplacements < 0){
		return;
	}
	else if (allReplacements){
		tab->Grid->SpellErrors.clear();
		tab->Grid->SetModified(REPLACE_ALL);
		if (dialogueColumn < TXT)
			tab->Grid->RefreshColumns(dialogueColumn);
		else
			tab->Grid->Refresh(false);
	}
	blockTextChange = true;
	KaiMessageBox(wxString::Format(_("Zmieniono %i razy."), allReplacements), _("Szukaj Zamień"));
	AddRecent(window);
	findTextReset = true;
}


void FindReplace::ReplaceInAllOpenedSubs(TabWindow *window)
{
	if (window->windowType != WINDOW_REPLACE){
		KaiLogDebug(L"Ups, replace all on subs called from window replace");
		return;
	}

	if (CheckStyles(window, Kai->GetTab()))
		return;

	int allTabsReplacements = 0;
	for (size_t i = 0; i < Kai->Tabs->Size(); i++){
		TabPanel *tab = Kai->Tabs->Page(i);
		if (UpdateValues(window))
			return;

		int allReplacements = ReplaceAllInTab(tab, window);
		if (allReplacements < 0){
			return;
		}
		if (allReplacements){
			tab->Grid->SpellErrors.clear();
			tab->Grid->SetModified(REPLACE_ALL);
			if (dialogueColumn < TXT){
				tab->Grid->RefreshColumns(dialogueColumn);
			}
			else{
				tab->Grid->Refresh(false);
			}
			Kai->Label(tab->Grid->file->GetActualHistoryIter(), false, i, i != Kai->Tabs->iter);
			allTabsReplacements += allReplacements;
		}
	}
	
	blockTextChange = true;
	KaiMessageBox(wxString::Format(_("Zmieniono %i razy."), allTabsReplacements), _("Szukaj Zamień"));
	AddRecent(window);
	findTextReset = true;
}

void FindReplace::ReplaceInSubs(TabWindow *window)
{
	int result = KaiMessageBox(_("Czy na pewno dokonać zmian we wszystkich napisach?\nW razie pomyłki kopie zapasowe są w folderze 'ReplaceBackup'"), _("Informacja"), wxYES_NO, FRD);
	if (result == wxYES){
		find = false;
		FindReplaceInSubs(window);
	}
}

void FindReplace::AddRecent(TabWindow *window)
{
	wxString text = window->FindText->GetValue();

	for (size_t i = 0; i < findRecent.GetCount(); i++)
	{
		if (findRecent[i] == text){
			findRecent.RemoveAt(i);
			window->FindText->Delete(i);
		}
	}

	size_t findSize = findRecent.size();

	findRecent.Insert(text, 0);
	window->FindText->Insert(text, 0);
	window->FindText->SetSelection(0);

	if (findSize > 20){
		window->FindText->Delete(20, findSize - 20);
		findRecent.RemoveAt(20, findSize - 20);
	}

	Options.SetTable(FIND_RECENT_FINDS, findRecent);
	if (window->windowType != WINDOW_FIND){
		wxString text = window->ReplaceText->GetValue();

		for (size_t i = 0; i < replaceRecent.GetCount(); i++)
		{
			if (replaceRecent[i] == text){
				replaceRecent.RemoveAt(i);
				window->ReplaceText->Delete(i);
			}
		}

		size_t replaceSize = replaceRecent.size();
		
		replaceRecent.Insert(text, 0);
		window->ReplaceText->Insert(text, 0);
		window->ReplaceText->SetSelection(0);

		if (replaceSize > 20){
			window->ReplaceText->Delete(20, replaceSize - 20);
			replaceRecent.RemoveAt(20, replaceSize - 20);
		}

		Options.SetTable(REPLACE_RECENT_REPLACEMENTS, replaceRecent);
	}
	if (window->windowType == WINDOW_FIND_IN_SUBS){
		wxString filter = window->FindInSubsPattern->GetValue();
		wxString path = window->FindInSubsPath->GetValue();
		for (size_t i = 0; i < subsFindingFilters.GetCount(); i++)
		{
			if (subsFindingFilters[i] == filter){
				subsFindingFilters.RemoveAt(i);
				window->FindInSubsPattern->Delete(i);
			}
		}
		for (size_t i = 0; i < subsFindingPaths.GetCount(); i++)
		{
			if (subsFindingPaths[i] == path){
				subsFindingPaths.RemoveAt(i);
				window->FindInSubsPath->Delete(i);
			}
		}
		size_t filtersSize = subsFindingFilters.size();
		size_t pathsSize = subsFindingPaths.size();

		subsFindingFilters.Insert(filter, 0);
		window->FindInSubsPattern->Insert(filter, 0);
		window->FindInSubsPattern->SetSelection(0);
		subsFindingPaths.Insert(path, 0);
		window->FindInSubsPath->Insert(path, 0);
		window->FindInSubsPath->SetSelection(0);

		if (filtersSize > 20){
			window->FindInSubsPattern->Delete(20, filtersSize - 20);
			subsFindingFilters.RemoveAt(20, filtersSize - 20);
		}

		if (pathsSize > 20){
			window->FindInSubsPath->Delete(20, pathsSize - 20);
			subsFindingPaths.RemoveAt(20, pathsSize - 20);
		}

		Options.SetTable(FIND_IN_SUBS_FILTERS_RECENT, subsFindingFilters);
		Options.SetTable(FIND_IN_SUBS_PATHS_RECENT, subsFindingPaths);
	}
}

void FindReplace::OnClose()
{ 
	FRD->Hide(); 
}

void FindReplace::GetFolderFiles(const wxString &path, const wxString &filters, wxArrayString *paths, bool subFolders, bool hiddenFolders)
{
	wxDir subsDir(path);
	long flags = wxDIR_FILES;
	if (subFolders)
		flags |= wxDIR_DIRS;
	if (hiddenFolders)
		flags |= wxDIR_HIDDEN;

	if (subsDir.IsOpened()){
		wxStringTokenizer tokenizer(filters, L";");
		while (tokenizer.HasMoreTokens()){
			subsDir.GetAllFiles(path, paths, tokenizer.NextToken(), flags);
		}
	}
	else{
		KaiMessageBox(_("Ścieżka szukania jest nieprawidłowa"));
	}
}

//no const cause styles will be changed when user choose it
bool FindReplace::CheckStyles(TabWindow *window, TabPanel *tab)
{
	stylesAsText = window->ChoosenStyleText->GetValue();
	if (stylesAsText.empty())
		return false;

	wxStringTokenizer tknzr(stylesAsText, L",", wxTOKEN_STRTOK);
	wxString notFoundStyles;
	wxString foundStyles;

	while (tknzr.HasMoreTokens()){
		wxString styleName = tknzr.GetNextToken();
		int result = tab->Grid->FindStyle(styleName);
		if (result == -1){
			notFoundStyles << styleName << L", ";
		}
		else{
			foundStyles << styleName << L",";
		}
	}
	if ((!notFoundStyles.empty() && !wasIngored) || foundStyles.empty()){
		notFoundStyles.RemoveLast();
		KaiMessageDialog *KMD;
		if (foundStyles.empty()){
			KMD = new KaiMessageDialog(FRD, _("Wszystkich wybranych stylów nie ma w przeszukiwanych napisach,\nprzez co nic nie zostanie znalezione.\nCo zrobić?"), _("Potwierdzenie"), wxYES | wxCANCEL);
			KMD->SetYesLabel(_("Wyczyść style"));
		}
		else{
			KMD = new KaiMessageDialog(FRD, wxString::Format(_("Stylów o nazwach \"%s\" nie ma w przeszukiwanych napisach,\nco może znacząco zmniejszyć ilość wyników szukania.\nCo zrobić?"), notFoundStyles), _("Potwierdzenie"), wxOK | wxYES_NO | wxCANCEL);
			KMD->SetOkLabel(_("Usuń nieistniejące style"));
			KMD->SetYesLabel(_("Wyczyść style"));
			KMD->SetNoLabel(_("Ignoruj"));
		}
		int result = KMD->ShowModal();
		if (result == wxOK){
			stylesAsText = L"," + foundStyles;
			window->ChoosenStyleText->SetValue(foundStyles.RemoveLast());
		}
		else if (result == wxYES){
			stylesAsText = L"";
			window->ChoosenStyleText->SetValue(stylesAsText);
		}
		else if (result == wxNO){
			wasIngored = true;
		}
		else if (result == wxCANCEL){
			return true;
		}
		//if (result != wxNO)
			//wasIngored = false;
	}
	if (!stylesAsText.empty() && !stylesAsText.StartsWith(L","))
		stylesAsText = L"," + stylesAsText + L",";

	return false;
}

bool FindReplace::KeepFinding(const wxString &text, int textPos)
{
	bool findStart = false;
	//I don't even need to check end cause when there's no end start will take all line
	for (int i = (textPos >= text.length()) ? text.length() - 1 : textPos; i >= 0; i--){
		if (text[i] == L'}'){
			if (endLine && i == text.length() - 1)
				findStart = true;

			break;
		}
		else if (text[i] == L'{'){
			findStart = true;
			break;
		}
	}
	if (!onlyText && findStart)
		return true;

	if (onlyText && !findStart)
		return true;

	return false;
}

bool FindReplace::GetNextBlock(wxString *text, wxString *block)
{
	if (nextBlockPosition >= text->length())
		return false;

	block->clear();

	if (!onlyOption){
		*block = *text;
		nextBlockPosition = blockLen = text->length();
	}
	else if (onlyText){
		bool isblocked = false;
		for (; nextBlockPosition < text->length(); nextBlockPosition++){
			if ((*text)[nextBlockPosition] == L'}'){
				isblocked = false;
				if (!block->empty())
					break;
			}
			else if ((*text)[nextBlockPosition] == L'{'){
				isblocked = true;
			}
			else if (!isblocked){
				if (block->empty())
					blockPosition = nextBlockPosition;

				*block << (*text)[nextBlockPosition];
			}
		}
		blockLen = block->length();
		if (blockLen)
			return true;
	}
	else{
		bool isblocked = false;
		for (; nextBlockPosition < text->length(); nextBlockPosition++){
			if ((*text)[nextBlockPosition] == L'{'){
				isblocked = true;
				if (!block->empty()){
					nextBlockPosition--;
					break;
				}
			}
			if (isblocked){
				if (block->empty())
					blockPosition = nextBlockPosition;

				*block << (*text)[nextBlockPosition];

			}
			if ((*text)[nextBlockPosition] == L'}')
				isblocked = false;
		}
		blockLen = block->length();
		if (blockLen)
			return true;
	}
	return false;
}

bool FindReplace::UpdateValues(TabWindow *window)
{
	dialogueColumn = TXT;
	if (window->CollumnStyle->GetValue()){ dialogueColumn = STYLE; }
	else if (window->CollumnActor->GetValue()){ dialogueColumn = ACTOR; }
	else if (window->CollumnEffect->GetValue()){ dialogueColumn = EFFECT; }

	matchCase = window->MatchCase->GetValue();
	regEx = window->RegEx->GetValue();
	startLine = window->StartLine->GetValue();
	endLine = window->EndLine->GetValue();
	skipComments = !window->UseComments->GetValue();
	onlyText = window->OnlyText->GetValue();
	bool onlyTags = window->OnlyTags->GetValue();
	onlyOption = onlyText || onlyTags;

	findString = window->FindText->GetValue();
	replaceString = (window->ReplaceText)? window->ReplaceText->GetValue() : L"";

	if (startLine && regEx){
		findString = L"^" + findString;
	}
	if (endLine && regEx){
		if (findString.empty()){
			findString = L"^(.*)$";
			replaceString = L"\\1" + replaceString;
		}
		else{
			findString << L"$";
		}
	}

	if (regEx){
		int rxflags = wxRE_ADVANCED;
		if (!matchCase){ rxflags |= wxRE_ICASE; }
		findReplaceRegEx.Compile(findString, rxflags);
		if (!findReplaceRegEx.IsValid()) {
			return true;
		}
	}
	return false;
}

int FindReplace::ReplaceCheckedInSubs(std::vector<SeekResults *> &results, const wxString &copyPath)
{
	if (!results.size())
		return 0;

	wxString path;
	wxString replacedText;
	wxString subsText;
	int numOfChanges = 0;
	int replacementDiff = 0;
	size_t numOfResult = 0;

	SeekResults *SeekResult = results[0];
	path = SeekResult->path;
	wxString ext = path.AfterLast('.').Lower();
	OpenWrite ow;
	if (!ow.FileOpen(path, &subsText))
		return 0;

	bool isSRT = (ext == L"srt");

	if ((ext == L"ass"/* || ext == "ssa"*/)){
		size_t result = subsText.find(/*(ext == "ass") ? */L"Dialogue:"/* : L"Marked="*/);
		size_t result1 = subsText.find(/*(ext == "ass") ? */L"Comment:"/* : L"Marked="*/);
		if (result == -1 && result1 == -1)// no dialogues;
			return 0;
		else{
			if (result1 < result)
				result = result1;

			replacedText = subsText.Mid(0, result);
			replacedText.Replace(L"\n", L"\r\n");

			subsText = subsText.Mid(result);
		}
	}

	wxStringTokenizer tokenizer(subsText, L"\n", wxTOKEN_STRTOK);
	wxString token;
	wxString dialtxt;
	Dialogue *dial = NULL;
	size_t lineNum = 0;
	while (tokenizer.HasMoreTokens()){

		if (isSRT){
			wxString text = tokenizer.GetNextToken();
			bool noMoreTokens = !tokenizer.HasMoreTokens();
			if (IsNumber(text) || noMoreTokens){
				if (noMoreTokens)
					token << text << L"\r\n";

				if (token != L""){
					token.Trim();
				}
				else{
					continue;
				}
			}
			else{
				//lol why I got text without \r?
				//I have to put it myself
				token << text << L"\r\n";
				continue;
			}
		}
		else{
			token = tokenizer.GetNextToken();
			token.Trim();
		}
		if (SeekResult->keyLine != lineNum){

			if (isSRT)
				replacedText << (lineNum + 1) << L"\r\n";

			replacedText << token << L"\r\n";

			if (isSRT)
				replacedText << L"\r\n";

			lineNum++;
			token = L"";
			continue;
		}
		
		dial = new Dialogue(token);
		token = L"";
		if (!dial){
			continue;
		}

		dial->GetTextElement(dialogueColumn, &dialtxt);
		replacementDiff = 0;
		//need checks
		if (dialtxt != SeekResult->name){
			KaiLog(wxString::Format(_("Linia %i nie może być zamieniona,\nbo została zedytowana."),
				SeekResult->idLine));
			continue;
		}
		while (SeekResult->keyLine == lineNum){
			int numOfReps = ReplaceCheckedLine(&dialtxt, SeekResult->findPosition, &replacementDiff);

			numOfChanges += numOfReps;


			numOfResult++;
			if (numOfResult < results.size())
				SeekResult = results[numOfResult];
			else
				break;
		}
		if (isSRT)
			replacedText << (lineNum + 1) << L"\r\n";

		dial->SetTextElement(dialogueColumn, dialtxt);
		//else{ replacedText << dialtxt << L"\r\n"; }
		//if (replaceColumn)
		dial->GetRaw(&replacedText);
		delete dial;

		lineNum++;
		
	}

	if (numOfChanges){
		wxCopyFile(path, copyPath + path.AfterLast(L'\\'));
		ow.FileWrite(path, replacedText);
	}

	return numOfChanges;
}

