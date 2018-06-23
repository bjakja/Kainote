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

	Options.GetTable(FindRecent, findRecent, "\f", wxTOKEN_RET_EMPTY_ALL);
	if (findRecent.size() > 20){ findRecent.RemoveAt(20, findRecent.size() - 20); }
	if (findRecent.size())
		actualFind = findRecent[0];

	Options.GetTable(ReplaceRecent, replaceRecent, "\f", wxTOKEN_RET_EMPTY_ALL);
	if (replaceRecent.size() > 20){ replaceRecent.RemoveAt(20, replaceRecent.size() - 20); }
	if (replaceRecent.size())
		actualReplace = replaceRecent[0];
	
	Options.GetTable(FIND_IN_SUBS_FILTERS_RECENT, subsFindingFilters, "\f", wxTOKEN_RET_EMPTY_ALL);
	if (subsFindingFilters.size() > 20){ subsFindingFilters.RemoveAt(20, subsFindingFilters.size() - 20); }
	if (subsFindingFilters.size())
		actualFilters = subsFindingFilters[0];

	Options.GetTable(FIND_IN_SUBS_PATHS_RECENT, subsFindingPaths, "\f", wxTOKEN_RET_EMPTY_ALL);
	if (subsFindingPaths.size() > 20){ subsFindingPaths.RemoveAt(20, subsFindingPaths.size() - 20); }
	if (subsFindingPaths.size())
		actualPaths = subsFindingPaths[0];
}

void FindReplace::ShowResult(TabPanel *tab, const wxString &path, int keyLine)
{
	if (tab){
		for (size_t i = 0; i < Kai->Tabs->Size(); i++){
			if (Kai->Tabs->Page(i) == tab){
				if (keyLine < tab->Grid->file->GetAllCount()){
					if (i != Kai->Tabs->iter)
						Kai->Tabs->ChangePage(i);

					int lineId = tab->Grid->file->GetElementByKey(keyLine);
					tab->Edit->SetLine(lineId);
					tab->Grid->SelectRow(lineId);
					tab->Grid->ScrollTo(lineId, true);
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
			int lineId = ntab->Grid->file->GetElementByKey(keyLine);
			ntab->Edit->SetLine(lineId);
			ntab->Grid->SelectRow(lineId);
			ntab->Grid->ScrollTo(lineId, true);
		}
	}
}

void FindReplace::Find(TabWindow *window)
{
	if (window->windowType == WINDOW_FIND_IN_SUBS){
		wxLogStatus("chujnia replace all wywołane z okna find in subs");
		return;
	}
	TabPanel *tab = Kai->GetTab();
	bool searchInOriginal = window->CollumnTextOriginal->GetValue();
	long wrep = (tab->Grid->hasTLMode && !searchInOriginal) ? TXTTL : TXT;
	if (window->CollumnStyle->GetValue()){ wrep = STYLE; }
	else if (window->CollumnActor->GetValue()){ wrep = ACTOR; }
	else if (window->CollumnEffect->GetValue()){ wrep = EFFECT; }

	bool matchcase = window->MatchCase->GetValue();
	bool regex = window->RegEx->GetValue();
	bool startline = window->StartLine->GetValue();
	bool endline = window->EndLine->GetValue();

	wxString find1 = window->FindText->GetValue();
	if (find1 != oldfind){ fromstart = true; fnext = false; oldfind = find1; }
	if (!fromstart && lastActive != tab->Grid->currentLine){
		lastActive = tab->Grid->currentLine;
	}

	if (startline && regex){
		find1 = "^" + find1;
	}
	if (endline && regex){
		find1 << "$";
	}

	//Kai->Freeze();

	wxString txt;
	int foundPosition = -1;
	size_t mlen = 0;
	bool foundsome = false;
	if (fromstart){
		int firstSelectionId = tab->Grid->FirstSelection();
		//is it possible to get it -1 if id exists, key also should exist
		linePosition = (!window->AllLines->GetValue() && firstSelectionId >= 0) ? tab->Grid->file->GetElementById(firstSelectionId) : 0;
		textPosition = 0;
	}
	if (CheckStyles(window, tab))
		return;
	bool styles = !stylesAsText.empty();
		
	bool onlysel = window->SelectedLines->GetValue();
	File *Subs = tab->Grid->file->GetSubs();

	while (linePosition < Subs->dials.size())
	{
		Dialogue *Dial = Subs->dials[linePosition];
		if (!Dial->isVisible){ linePosition++; textPosition = 0; continue; }

		if ((!styles && !onlysel) ||
			(styles && stylesAsText.Find("," + Dial->Style + ",") != -1) ||
			(onlysel && tab->Grid->file->IsSelectedByKey(linePosition))){
			if (wrep == STYLE){
				txt = Dial->Style;
			}
			else if (wrep == TXT || wrep == TXTTL){
				txt = (wrep == TXTTL) ? Dial->TextTl : Dial->Text;
			}
			else if (wrep == ACTOR){
				txt = Dial->Actor;
			}
			else if (wrep == EFFECT){
				txt = Dial->Effect;
			}

			//no to szukamy
			if (!(startline || endline) && (find1.empty() || txt.empty()))
			{
				if (txt.empty() && find1.empty()){
					foundPosition = 0; mlen = 0;
				}
				else{ textPosition = 0; linePosition++; continue; }

			}
			else if (regex){
				int rxflags = wxRE_ADVANCED;
				if (!matchcase){ rxflags |= wxRE_ICASE; }
				rgx.Compile(find1, rxflags);
				if (rgx.IsValid()) {
					wxString cuttext = txt.Mid(textPosition);
					if (rgx.Matches(cuttext)) {
						size_t regexStart = 0;
						rgx.GetMatch(&regexStart, &mlen, 0);
						foundPosition = regexStart + textPosition;
					}
					else{ textPosition = 0; linePosition++; continue; }

				}

			}
			else{
				wxString ltext = (!matchcase) ? txt.Lower() : txt;
				wxString lfind = (!matchcase) ? find1.Lower() : find1;
				if (startline){
					if (ltext.StartsWith(lfind) || lfind.empty()){
						foundPosition = 0;
						textPosition = 0;
					}
					else{
						foundPosition = -1;
					}
				}
				if (endline){
					if (ltext.EndsWith(lfind) || lfind.empty()){
						foundPosition = txt.Len() - lfind.Len();
						textPosition = 0;
					}
					else{
						foundPosition = -1;
					}
				}
				else{
					foundPosition = ltext.find(lfind, textPosition);
				}
				mlen = lfind.Len();
			}

			if (foundPosition != -1){
				textPosition = foundPosition + mlen;
				findstart = foundPosition;
				findend = textPosition;
				lastActive = reprow = linePosition;
				int posrowId = tab->Grid->file->GetElementByKey(linePosition);
				if (!onlysel){ tab->Grid->SelectRow(posrowId, false, true); }
				tab->Edit->SetLine(posrowId);
				tab->Grid->ScrollTo(posrowId, true);
				if (onlysel){ tab->Grid->Refresh(false); }
				if (wrep == STYLE){
					//pan->Edit->StyleChoice->SetFocus();
				}
				else if (wrep == TXT || wrep == TXTTL){
					MTextEditor *tmp = (searchInOriginal) ? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
					//tmp->SetFocus();
					tmp->SetSelection(foundPosition, findend);
				}
				if (wrep == ACTOR){
					//pan->Edit->ActorEdit->SetFocus();
					tab->Edit->ActorEdit->choiceText->SetSelection(foundPosition, findend);
				}
				if (wrep == EFFECT){
					//pan->Edit->EffectEdit->SetFocus();
					tab->Edit->EffectEdit->choiceText->SetSelection(foundPosition, findend);
				}

				foundsome = true;
				if ((size_t)textPosition >= txt.Len() || startline){
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
		if (!foundsome && linePosition > Subs->dials.size() - 1){
			blockTextChange = true;
			if (wasResetToStart){
				wasResetToStart = false;
				break;
			}
			else if (KaiMessageBox(_("Wyszukiwanie zakończone, rozpocząć od początku?"), _("Potwierdzenie"),
				wxICON_QUESTION | wxYES_NO, FRD) == wxYES){
				linePosition = 0;
				wasResetToStart = true;
			}
			else{
				linePosition = 0;
				foundsome = true;
				break;
			}
		}
	}
	if (!foundsome){
		blockTextChange = true;
		KaiMessageBox(_("Nie znaleziono podanej frazy \"") + window->FindText->GetValue() + "\".", _("Potwierdzenie"));
		linePosition = 0;
		fromstart = true;
	}
	if (fromstart){ AddRecent(window); fromstart = false; }
	//Kai->Thaw();
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
bool FindReplace::FindAllInTab(TabPanel *tab, TabWindow *window)
{
	bool isfirst = true;
	bool searchInOriginal = window->CollumnTextOriginal->GetValue();
	long wrep = (tab->Grid->hasTLMode && !searchInOriginal) ? TXTTL : TXT;
	if (window->CollumnStyle->GetValue()){ wrep = STYLE; }
	else if (window->CollumnActor->GetValue()){ wrep = ACTOR; }
	else if (window->CollumnEffect->GetValue()){ wrep = EFFECT; }

	bool matchcase = window->MatchCase->GetValue();
	bool regex = window->RegEx->GetValue();
	bool startline = window->StartLine->GetValue();
	bool endline = window->EndLine->GetValue();

	findString = window->FindText->GetValue();

	if (startline && regex){
		findString = "^" + findString;
	}
	if (endline && regex){
		findString << "$";
	}

	wxString txt;
	int foundPosition = -1;
	size_t foundLength = 0;
	bool foundsome = false;
	int positionId = 0;
	
	int firstSelectedId = tab->Grid->FirstSelection();
	int tabLinePosition = (!window->AllLines->GetValue() && firstSelectedId >= 0) ? tab->Grid->file->GetElementById(firstSelectedId) : 0;
	int tabTextPosition = 0;
	if (tabLinePosition > 0)
		positionId = firstSelectedId;
	
	bool styles = !stylesAsText.empty();

	bool onlySelections = window->SelectedLines->GetValue();
	File *Subs = tab->Grid->file->GetSubs();
	wxRegEx seekRegex;
	if (regex){
		int rxflags = wxRE_ADVANCED;
		if (!matchcase){ rxflags |= wxRE_ICASE; }
		seekRegex.Compile(findString, rxflags);
		if (!seekRegex.IsValid()) {
			KaiMessageBox(_("Szablon wyrażeń regularnych jest nieprawidłowy"));
			return true;
		}
	}

	while (tabLinePosition < Subs->dials.size())
	{
		Dialogue *Dial = Subs->dials[tabLinePosition];
		if (!Dial->isVisible){ tabLinePosition++; tabTextPosition = 0; continue; }

		if ((!styles && !onlySelections) ||
			(styles && stylesAsText.Find("," + Dial->Style + ",") != -1) ||
			(onlySelections && tab->Grid->file->IsSelectedByKey(tabLinePosition))){
			if (wrep == STYLE){
				txt = Dial->Style;
			}
			else if (wrep == TXT || wrep == TXTTL){
				txt = (wrep == TXTTL) ? Dial->TextTl : Dial->Text;
			}
			else if (wrep == ACTOR){
				txt = Dial->Actor;
			}
			else if (wrep == EFFECT){
				txt = Dial->Effect;
			}

			//no to szukamy
			if (!(startline || endline) && (findString.empty() || txt.empty()))
			{
				if (txt.empty() && findString.empty()){
					foundPosition = 0; foundLength = 0;
				}
				else{ tabTextPosition = 0; tabLinePosition++; positionId++; continue; }

			}
			else if (regex){
				wxString cuttext = txt.Mid(tabTextPosition);
				if (seekRegex.Matches(cuttext)) {
					size_t regexStart = 0;
					seekRegex.GetMatch(&regexStart, &foundLength, 0);
					foundPosition = regexStart + tabTextPosition;
				}
				else{ tabTextPosition = 0; tabLinePosition++; positionId++; continue; }
			}
			else{
				wxString ltext = (!matchcase) ? txt.Lower() : txt;
				wxString lfind = (!matchcase) ? findString.Lower() : findString;
				if (startline){
					if (ltext.StartsWith(lfind) || lfind.empty()){
						foundPosition = 0;
						tabTextPosition = 0;
					}
					else{
						foundPosition = -1;
					}
				}
				if (endline){
					if (ltext.EndsWith(lfind) || lfind.empty()){
						foundPosition = txt.Len() - lfind.Len();
						tabTextPosition = 0;
					}
					else{
						foundPosition = -1;
					}
				}
				else{
					foundPosition = ltext.find(lfind, tabTextPosition);
				}
				foundLength = lfind.Len();
			}

			if (foundPosition != -1){
				if (isfirst){
					FRRD->SetHeader(tab->SubsPath);
					isfirst = false;
				}
				wxString lineNum = wxString::Format(_("Linia %i: "), positionId + 1);
				FRRD->SetResults(lineNum + txt, wxPoint(foundPosition + lineNum.Len(), foundLength), tab, tabLinePosition, L"");
				
				if ((size_t)tabTextPosition >= txt.Len() || startline){
					tabLinePosition++; 
					positionId++;
					tabTextPosition = 0;
				}
				else
					tabTextPosition = foundPosition + 1;

				foundPosition = -1;
			}
			else{
				tabTextPosition = 0;
				tabLinePosition++;
				positionId++;
			}

		}
		else{ tabTextPosition = 0; tabLinePosition++; positionId++; }
	}
	return false;
}

void FindReplace::FindInAllOpenedSubs(TabWindow *window)
{
	if (!FRRD)
		FRRD = new FindReplaceResultsDialog(Kai, this);
	else
		FRRD->ClearList();

	if (CheckStyles(window, Kai->GetTab()))
		return;

	for (size_t i = 0; i < Kai->Tabs->Size(); i++){
		if (FindAllInTab(Kai->Tabs->Page(i), window)){
			return;
		}
	}
	FRRD->FilterList();
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

	if (FindAllInTab(tab, window)){
		return;
	}

	FRRD->FilterList();
	if (!FRRD->IsShown())
		FRRD->Show();

	AddRecent(window);
}

void FindReplace::FindInSubs(TabWindow *window)
{
	FindReplaceInSubs(window, true);
}

void FindReplace::FindReplaceInSubs(TabWindow *window, bool find)
{
	wxString path = window->FindInSubsPath->GetValue();
	wxArrayString paths;
	GetFolderFiles(path, window->FindInSubsPattern->GetValue(), &paths, 
		window->SeekInSubFolders->GetValue(), window->SeekInHiddenFolders->GetValue());

	if (!paths.size())
		return;

	wxString CopyPath = Options.pathfull + "\\ReplaceBackup\\";
	DWORD ftyp = GetFileAttributesW(CopyPath.wc_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES){
		wxMkDir(CopyPath);
	}
	
	bool onlyAss = !(window->CollumnText->GetValue() || window->CollumnTextOriginal->GetValue());
	bool plainText = (window->CollumnTextOriginal->GetValue());
	long replaceColumn = TXT;
	if (window->CollumnStyle->GetValue()){ replaceColumn = STYLE; }
	else if (window->CollumnActor->GetValue()){ replaceColumn = ACTOR; }
	else if (window->CollumnEffect->GetValue()){ replaceColumn = EFFECT; }
	else if (plainText){ replaceColumn = 0; }

	matchCase = window->MatchCase->GetValue();
	regEx = window->RegEx->GetValue();
	startLine = window->StartLine->GetValue();
	endLine = window->EndLine->GetValue();

	findString = window->FindText->GetValue();
	if (!find)
		replaceString = window->ReplaceText->GetValue();

	if (startLine && regEx && !findString.StartsWith(L"^")){
		findString = "^" + findString;
	}
	if (endLine && regEx && !findString.EndsWith(L"$")){
		if (findString == "" && !find){
			findString = "^(.*)$";
			replaceString = "\\1" + replaceString;
		}
		else{
			findString << "$";
		}
	}
	if (regEx){
		int rxflags = wxRE_ADVANCED;
		if (!matchCase){ rxflags |= wxRE_ICASE; }
		findReplaceRegEx.Compile(findString, rxflags);
		if (!findReplaceRegEx.IsValid()) {
			KaiMessageBox(_("Szablon wyrażeń regularnych jest nieprawidłowy"));
			return;
		}
	}

	//chosing by style have no sense cause every file can have 
	//different styles and we choose styles from opened file
	/*wxString stylesList = window->ChoosenStyleText->GetValue();
	bool styles = false;
	if (stylesList != "" && !plainText){
		styles = true;
		stylesList = "," + stylesList + ",";
	}*/

	if (find){
		if (!FRRD)
			FRRD = new FindReplaceResultsDialog(Kai, this);
		else
			FRRD->ClearList();
	}
	int AllReplacements = 0;

	OpenWrite ow;
	for (size_t i = 0; i < paths.size(); i++){
		int SubsAllReplacements = 0;
		wxString subsText;
		subsPath = paths[i];
		ow.FileOpen(subsPath, &subsText);

		wxString ext = subsPath.AfterLast('.').Lower();
		if (onlyAss && ext != "ass"/* && ext != "ssa"*/)
			continue;
		else if (ext == "ass"/* && ext == "ssa"*/ && ext == "srt" && ext == "mpl2" && ext == "sub" && ext == "txt")
			continue;

		wxString replacedText;
		tabLinePosition = positionId = 0;
		bool isSRT = (ext == "srt");

		if ((ext == "ass"/* || ext == "ssa"*/) && !plainText){
			size_t result = subsText.find(/*(ext == "ass") ? */L"Dialogue:"/* : L"Marked="*/);
			size_t result1 = subsText.find(/*(ext == "ass") ? */L"Comment:"/* : L"Marked="*/);
			if (result == -1 && result1 == -1)// no dialogues;
				continue;
			else{
				if (result1 < result)
					result = result1;
				if (!find){
					replacedText = subsText.Mid(0, result);
					replacedText.Replace(L"\n", L"\r\n");
				}
				subsText = subsText.Mid(result);
			}
		}

		wxStringTokenizer tokenizer(subsText, "\n", wxTOKEN_STRTOK);
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

					if (token != ""){
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
			//if (styles && stylesList.find(L"," + dial->Style + L",") == -1)
				//continue;
			//here we got dial or plain text
			//we have to get only text
			if (replaceColumn == TXT){
				dialtxt = dial->Text;
			}
			else if (replaceColumn == STYLE){
				dialtxt = dial->Style;
			}
			else if (replaceColumn == ACTOR){
				dialtxt = dial->Actor;
			}
			else if (replaceColumn == EFFECT){
				dialtxt = dial->Effect;
			}
			else{
				dialtxt = token;
			}

			if (find){
				FindInSubsLine(&dialtxt, &isFirst);
			}
			else{
				int numOfReps = ReplaceInSubsLine(&dialtxt);
				if (numOfReps){
					if (isSRT)
						replacedText << (tabLinePosition + 1) << L"\r\n";

					if (replaceColumn == TXT){ dial->Text = dialtxt; }
					else if (replaceColumn == STYLE){ dial->Style = dialtxt; }
					else if (replaceColumn == ACTOR){ dial->Actor = dialtxt; }
					else if (replaceColumn == EFFECT){ dial->Effect = dialtxt; }
					else{ replacedText << dialtxt << L"\r\n"; }
					if (replaceColumn)
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

			tabLinePosition++;
			positionId++;

			if (dial){
				delete dial;
				dial = NULL;
			}
			token = "";
		}//while
		if (SubsAllReplacements){
			wxCopyFile(subsPath, CopyPath + subsPath.AfterLast('\\'));
			ow.FileWrite(subsPath, replacedText);
			AllReplacements += SubsAllReplacements;
		}
	}//for
	if (!find && AllReplacements){
		blockTextChange = true;
		KaiMessageBox(wxString::Format(_("Zmieniono %i razy."), AllReplacements), _("Szukaj Zamień"));
		AddRecent(window);
		findTextReset = true;
	}
	else if (find){
		FRRD->FilterList();
		if (!FRRD->IsShown())
			FRRD->Show();
	}
	AddRecent(window);
}

void FindReplace::FindInSubsLine(wxString *txt, bool *isFirst)
{
	int foundPosition = 0;
	size_t foundLength = 0;
	tabTextPosition = 0;

	while (1){
		foundPosition = -1;

		if (!(startLine || endLine) && (findString.empty() || txt->empty()))
		{
			if (txt->empty() && findString.empty()){
				foundPosition = 0; foundLength = 0;
			}
			else{ break; }

		}
		else if (regEx){
			wxString cuttext = txt->Mid(tabTextPosition);
			if (findReplaceRegEx.Matches(cuttext)) {
				size_t regexStart = 0;
				findReplaceRegEx.GetMatch(&regexStart, &foundLength, 0);
				foundPosition = regexStart + tabTextPosition;
			}
			else{ break; }
		}
		else{
			wxString ltext = (!matchCase) ? txt->Lower() : *txt;
			wxString lfind = (!matchCase) ? findString.Lower() : findString;
			if (startLine){
				if (ltext.StartsWith(lfind) || lfind.empty()){
					foundPosition = 0;
					tabTextPosition = 0;
				}
			}
			if (endLine){
				if (ltext.EndsWith(lfind) || lfind.empty()){
					foundPosition = txt->Len() - lfind.Len();
					tabTextPosition = 0;
				}
			}
			else{
				foundPosition = ltext.find(lfind, tabTextPosition);
			}
			foundLength = lfind.Len();
		}

		if (foundPosition != -1){
			if (*isFirst){
				FRRD->SetHeader(subsPath);
				*isFirst = false;
			}
			wxString lineNum = wxString::Format(_("Linia %i: "), positionId + 1);
			FRRD->SetResults(lineNum + (*txt), wxPoint(foundPosition + lineNum.Len(), foundLength), NULL, tabLinePosition, subsPath);

			if ((size_t)tabTextPosition >= txt->Len() || startLine){
				tabTextPosition = 0;
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
	int allreps = 0;
	if (!(startLine || endLine) && (findString.empty() || onlyString->empty()))
	{
		if (onlyString->empty() && findString.empty())
		{
			*onlyString = replaceString; allreps = 1;
		}
		else{ return 0; }
	}
	else if (regEx){
		allreps = findReplaceRegEx.ReplaceAll(onlyString, replaceString);
	}
	else if (startLine || endLine){

		wxString ltext = (!matchCase) ? onlyString->Lower() : *onlyString;
		wxString lfind = (!matchCase) ? findString.Lower() : findString;
		if (startLine){
			if (ltext.StartsWith(lfind) || lfind.empty()){
				onlyString->replace(0, lfind.Len(), replaceString);
				allreps = 1;
			}
		}
		if (endLine){
			if (ltext.EndsWith(lfind) || lfind.empty()){
				int lenn = onlyString->Len();
				onlyString->replace(lenn - lfind.Len(), lenn, replaceString);
				allreps = 1;
			}
		}

	}
	else if (!matchCase){
		wxString lfind = findString.Lower();
		wxString ltext = *onlyString;
		ltext.MakeLower();

		bool isfind = true;
		int newpos = 0;
		int flen = lfind.Len();
		allreps = 0;
		while (isfind){
			size_t poss = ltext.find(lfind, newpos);
			if (poss == -1){ isfind = false; break; }
			else{
				if (poss < 0){ poss = 0; }
				if (poss > onlyString->Len()){ poss = onlyString->Len(); }
				wxString hhh;
				ltext.Remove(poss, flen);
				ltext.insert(poss, replaceString);
				onlyString->Remove(poss, flen);
				onlyString->insert(poss, replaceString);
				allreps++;
				newpos = poss + replaceString.Len();
			}

		}
	}
	else{ allreps = onlyString->Replace(findString, replaceString); }
	return allreps;
}

void FindReplace::Replace(TabWindow *window)
{
	if (window->windowType != WINDOW_REPLACE){
		wxLogStatus("chujnia replace all wywołane nie z okna replace");
		return;
	}
	TabPanel *tab = Kai->GetTab();
	if (lastActive != tab->Grid->currentLine){ Find(window); }
	bool searchInOriginal = window->CollumnTextOriginal->GetValue();
	long wrep = (tab->Grid->hasTLMode && !searchInOriginal) ? TXTTL : TXT;
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

	Dialogue *Dialc = grid->CopyDialogueByKey(reprow);
	bool hasRegEx = window->RegEx->GetValue();

	if (wrep == STYLE){
		wxString oldstyle = Dialc->Style;
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldstyle.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldstyle.replace(findstart, findend - findstart, (reps)? place : rep);
		}
		else{
			oldstyle.replace(findstart, findend - findstart, rep);
		}
		tab->Edit->StyleChoice->SetSelection(tab->Edit->StyleChoice->FindString(oldstyle));
		Dialc->Style = oldstyle;
	}
	else if (wrep == TXT || wrep == TXTTL){
		MTextEditor *tmp = (searchInOriginal) ? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
		wxString oldtext = tmp->GetValue();
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldtext.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldtext.replace(findstart, findend - findstart, (reps) ? place : rep);
		}
		else{
			oldtext.replace(findstart, findend - findstart, rep);
		}
		tmp->SetTextS(oldtext);
		Dialc->Text = oldtext;
	}
	else if (wrep == ACTOR){
		wxString oldtext = tab->Edit->ActorEdit->choiceText->GetValue();
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldtext.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldtext.replace(findstart, findend - findstart, (reps) ? place : rep);
		}
		else{
			oldtext.replace(findstart, findend - findstart, rep);
		}
		tab->Edit->ActorEdit->choiceText->SetValue(oldtext);
		Dialc->Actor = oldtext;
	}
	else if (wrep == EFFECT){
		wxString oldtext = tab->Edit->EffectEdit->choiceText->GetValue();
		if (hasRegEx && rgx.IsValid()){
			wxString place = oldtext.Mid(findstart, findend - findstart);
			int reps = rgx.Replace(&place, rep, 1);
			oldtext.replace(findstart, findend - findstart, (reps) ? place : rep);
		}
		else{
			oldtext.replace(findstart, findend - findstart, rep);
		}
		tab->Edit->EffectEdit->choiceText->SetValue(oldtext);
		Dialc->Effect = oldtext;
	}

	grid->SetModified(REPLACE_SINGLE);
	grid->Refresh(false);
	textPosition = findstart + rep.Len();
	Find(window);
}

int FindReplace::ReplaceAllInTab(TabPanel *tab, TabWindow *window, long replaceColumn)
{
	bool matchcase = window->MatchCase->GetValue();
	bool regex = window->RegEx->GetValue();
	bool startline = window->StartLine->GetValue();
	bool endline = window->EndLine->GetValue();

	wxString find = window->FindText->GetValue(),
		rep = window->ReplaceText->GetValue();
	if (startline && regex){
		find = "^" + find;
	}
	if (endline && regex){
		if (find == ""){
			find = "^(.*)$";
			rep = "\\1" + rep;
		}
		else{
			find << "$";
		}
	}

	int allRelpacements = 0;
	int allreps = 0;
	wxString txt;
	
	bool notstyles = stylesAsText.empty();

	bool onlysel = window->SelectedLines->GetValue();

	int firstSelectionId = tab->Grid->FirstSelection();
	File *Subs = tab->Grid->file->GetSubs();
	bool skipFiltered = !tab->Grid->ignoreFiltered;
	wxRegEx nfind;
	if (regex){
		int rxflags = wxRE_ADVANCED;
		if (!matchcase){ rxflags |= wxRE_ICASE; }
		nfind.Compile(find, rxflags);
		if (!nfind.IsValid()) {
			KaiMessageBox(_("Szablon wyrażeń regularnych jest nieprawidłowy"));
			return -1;
		}
	}

	for (int i = (!window->AllLines->GetValue() && firstSelectionId >= 0) ?
		tab->Grid->file->GetElementById(firstSelectionId) : 0; i < Subs->dials.size(); i++)
	{
		Dialogue *Dial = Subs->dials[i];
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue){ continue; }

		if ((notstyles || stylesAsText.Find("," + Dial->Style + ",") != -1) &&
			!(onlysel && !(tab->Grid->file->IsSelectedByKey(i)))){

			if (replaceColumn == STYLE){
				txt = Dial->Style;
			}
			else if (replaceColumn == TXT){
				txt = Dial->Text;
			}
			else if (replaceColumn == TXTTL){
				txt = Dial->TextTl;
			}
			else if (replaceColumn == ACTOR){
				txt = Dial->Actor;
			}
			else if (replaceColumn == EFFECT){
				txt = Dial->Effect;
			}
			if (!(startline || endline) && (find.empty() || txt.empty()))
			{
				if (txt.empty() && find.empty())
				{
					txt = rep; allreps = 1;
				}
				else{ continue; }
			}
			else if (regex){
				allreps = nfind.ReplaceAll(&txt, rep);
			}
			else if (startline || endline){

				wxString ltext = (!matchcase) ? txt.Lower() : txt;
				wxString lfind = (!matchcase) ? find.Lower() : find;
				if (startline){
					if (ltext.StartsWith(lfind) || lfind.empty()){
						txt.replace(0, lfind.Len(), rep);
						allreps = 1;
					}
				}
				if (endline){
					if (ltext.EndsWith(lfind) || lfind.empty()){
						int lenn = txt.Len();
						txt.replace(lenn - lfind.Len(), lenn, rep);
						allreps = 1;
					}
				}

			}
			else if (!matchcase){
				wxString lfind = find.Lower();
				wxString ltext = txt;
				ltext.MakeLower();

				bool isfind = true;
				int newpos = 0;
				int flen = lfind.Len();
				allreps = 0;
				while (isfind){
					size_t poss = ltext.find(lfind, newpos);
					if (poss == -1){ isfind = false; break; }
					else{
						if (poss < 0){ poss = 0; }
						if (poss > txt.Len()){ poss = txt.Len(); }
						wxString hhh;
						ltext.Remove(poss, flen);
						ltext.insert(poss, rep);
						txt.Remove(poss, flen);
						txt.insert(poss, rep);
						allreps++;
						newpos = poss + rep.Len();
					}

				}
			}
			else{ allreps = txt.Replace(find, rep); }
			if (allreps > 0){
				Dialogue *Dialc = tab->Grid->file->CopyDialogueByKey(i);
				if (replaceColumn == TXT){ Dialc->Text = txt; }
				else if (replaceColumn == TXTTL){ Dialc->TextTl = txt; }
				else if (replaceColumn == STYLE){ Dialc->Style = txt; }
				else if (replaceColumn == ACTOR){ Dialc->Actor = txt; }
				else if (replaceColumn == EFFECT){ Dialc->Effect = txt; }
				allRelpacements += allreps;

			}

		}

	}

	return allRelpacements;
}

void FindReplace::ReplaceAll(TabWindow *window)
{
	if (window->windowType != WINDOW_REPLACE){
		wxLogStatus("chujnia replace all wywołane nie z okna replace");
		return;
	}

	TabPanel *tab = Kai->GetTab();
	long replaceColumn = (tab->Grid->hasTLMode && !window->CollumnTextOriginal->GetValue()) ? TXTTL : TXT;
	if (window->CollumnStyle->GetValue()){ replaceColumn = STYLE; }
	else if (window->CollumnActor->GetValue()){ replaceColumn = ACTOR; }
	else if (window->CollumnEffect->GetValue()){ replaceColumn = EFFECT; }

	if (CheckStyles(window, tab))
		return;
	
	int allReplacements = ReplaceAllInTab(tab, window, replaceColumn);
	if (allReplacements < 0){
		return;
	}
	else if (allReplacements){
		tab->Grid->SpellErrors.clear();
		tab->Grid->SetModified(REPLACE_ALL);
		if (replaceColumn < TXT){
			tab->Grid->RefreshColumns(replaceColumn);
		}
		else{
			tab->Grid->Refresh(false);
		}
	}
	blockTextChange = true;
	KaiMessageBox(wxString::Format(_("Zmieniono %i razy."), allReplacements), _("Szukaj Zamień"));
	AddRecent(window);
	findTextReset = true;
}


void FindReplace::ReplaceInAllOpenedSubs(TabWindow *window)
{
	if (window->windowType != WINDOW_REPLACE){
		wxLogStatus("chujnia replace all we wszystkich otwartch subach wywołane nie z okna replace");
		return;
	}

	if (CheckStyles(window, Kai->GetTab()))
		return;

	int allTabsReplacements = 0;
	for (size_t i = 0; i < Kai->Tabs->Size(); i++){
		TabPanel *tab = Kai->Tabs->Page(i);
		long replaceColumn = (tab->Grid->hasTLMode && !window->CollumnTextOriginal->GetValue()) ? TXTTL : TXT;
		if (window->CollumnStyle->GetValue()){ replaceColumn = STYLE; }
		else if (window->CollumnActor->GetValue()){ replaceColumn = ACTOR; }
		else if (window->CollumnEffect->GetValue()){ replaceColumn = EFFECT; }
		int allReplacements = ReplaceAllInTab(tab, window, replaceColumn);
		if (allReplacements < 0){
			return;
		}
		if (allReplacements){
			tab->Grid->SpellErrors.clear();
			tab->Grid->SetModified(REPLACE_ALL);
			if (replaceColumn < TXT){
				tab->Grid->RefreshColumns(replaceColumn);
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
	FindReplaceInSubs(window, false);
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

	Options.SetTable(FindRecent, findRecent, "\f");
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

		Options.SetTable(ReplaceRecent, replaceRecent, "\f");
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

		Options.SetTable(FIND_IN_SUBS_FILTERS_RECENT, subsFindingFilters, "\f");
		Options.SetTable(FIND_IN_SUBS_PATHS_RECENT, subsFindingPaths, "\f");
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
		subsDir.GetAllFiles(path, paths, filters, flags);
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

	wxStringTokenizer tknzr(stylesAsText, ",", wxTOKEN_STRTOK);
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
			KMD = new KaiMessageDialog(FRD, _("Wszystkich wybranych stylów nie ma w szukanych napisach,\nprzez co nic nie zostanie znalezione\nCo zrobić?"), _("Potwierdzenie"), wxYES | wxCANCEL);
			KMD->SetYesLabel(_("Wyczyść style"));
		}
		else{
			KMD = new KaiMessageDialog(FRD, wxString::Format(_("Stylów o nazwach \"%s\" nie ma w szukanych napisach,\nco może znacząco zmniejszyć ilość wyników szukania\nCo zrobić?"), notFoundStyles), _("Potwierdzenie"), wxOK | wxYES_NO | wxCANCEL);
			KMD->SetOkLabel(_("Usuń nie istniejące style"));
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
	if (!stylesAsText.empty() && !stylesAsText.StartsWith(","))
		stylesAsText = L"," + stylesAsText + L",";

	return false;
}
