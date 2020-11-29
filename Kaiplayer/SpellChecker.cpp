// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016 - 2020, Marcin Drob
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

#include "SpellChecker.h"
#include <wx/dir.h>
#include <wx/log.h>
#include "OpennWrite.h"
#include "KaiMessageBox.h"
#include <boost/locale/boundary/index.hpp>
#include <boost/locale/boundary/segment.hpp>
#include <boost/locale/boundary/types.hpp>
#include <set>

SpellChecker *SpellChecker::SC = NULL;

SpellChecker::SpellChecker()
{
	hunspell = NULL;
	conv = NULL;
	SC = NULL;
	dictionaryPath = Options.pathfull + L"\\Dictionary\\";
	userDictionaryPath = dictionaryPath + L"UserDic.udic";

}

SpellChecker *SpellChecker::Get()
{
	if (!SC){
		SC = new SpellChecker();
		bool isgood = SC->Initialize();
		if (!isgood) { Options.SetBool(SPELLCHECKER_ON, false); }
	}
	return SC;
}
void SpellChecker::Destroy()
{
	if (SC){ delete SC; SC = NULL; }
}

SpellChecker::~SpellChecker()
{
	Cleaning();
}

void SpellChecker::Cleaning()
{
	if (hunspell){ delete hunspell; hunspell = NULL; }
	if (conv){ delete conv; conv = NULL; }

}

void SpellChecker::AvailableDics(wxArrayString &dics, wxArrayString &symbols)
{
	wxArrayString dic;
	wxArrayString aff;
	wxString dictionaryPath = Options.pathfull + L"\\Dictionary";
	wxDir kat(dictionaryPath);
	if (kat.IsOpened()){

		kat.GetAllFiles(dictionaryPath, &dic, L"*.dic", wxDIR_FILES);
		kat.GetAllFiles(dictionaryPath, &aff, L"*.aff", wxDIR_FILES);
	}

	for (size_t i = 0; i < dic.size(); i++){
		if (dic[i].BeforeLast(L'.') == aff[i].BeforeLast(L'.')){
			wxString symbolName = dic[i].AfterLast(L'\\').BeforeFirst(L'.');
			symbols.Add(symbolName);
			const wxString &fullName = Options.FindLanguage(symbolName);
			dics.Add(fullName);
		}
	}
}

bool SpellChecker::Initialize()
{
	Cleaning();

	//wxString pathhh = Options.pathfull + L"\\Dictionary\\";
	wxString name = Options.GetString(DICTIONARY_LANGUAGE);
	if (name == L""){ name = L"pl"; }
	wxString dic = dictionaryPath + name + L".dic";
	wxString aff = dictionaryPath + name + L".aff";
	// Check if language is available
	if (!wxFileExists(dic) || !wxFileExists(aff))
	{
		Options.SetBool(SPELLCHECKER_ON, false);
		KaiMessageBox(wxString::Format(_("Brak plików słownika w folderze \"%s\\Dictionary\".\nSprawdzanie pisowni zostanie wyłączone"), Options.pathfull));
		return false;
	}
	// Load
	hunspell = new Hunspell(aff.mb_str(wxConvLocal), dic.mb_str(wxConvLocal));

	if (hunspell) {
		conv = new wxCSConv(wxString(hunspell->get_dic_encoding(), wxConvUTF8));
		if (!conv){ KaiMessageBox(_("Nie można odczytać formatu konwersji słownika.")); }
		// Load user dictionary
		//wxString userpath = pathhh + L"UserDic.udic";
		if (wxFileExists(userDictionaryPath)) {
			OpenWrite op;
			wxString txt;
			if (!op.FileOpen(userDictionaryPath, &txt, false)) { return true; }
			wxStringTokenizer textIn(txt, L"\n");
			while (textIn.HasMoreTokens()) {
				// Read line
				wxString curLine = textIn.NextToken();
				curLine.Trim();
				if (curLine.IsEmpty() || curLine.IsNumber()) continue;

				hunspell->add(curLine.mb_str(*conv));
			}

		}

		return true;
	}
	else{ KaiMessageBox(_("Nie można zainicjalizować sprawdzania pisowni.")); }
	return false;
}

bool SpellChecker::CheckWord(wxString word) {
	if (!hunspell || word == L"") return true;

	wxCharBuffer buf = word.mb_str(*conv);
	if (buf && strlen(buf)) return (hunspell->spell(buf) == 1);

	return false;
}

void SpellChecker::Suggestions(wxString word, wxArrayString &results)
{
	if (!hunspell) return;

	char **result;

	wxCharBuffer buf = word.mb_str(*conv);
	if (!buf) return;

	int n = hunspell->suggest(&result, buf);

	for (int i = 0; i < n; ++i)
	{

		wxString cur(result[i], *conv);
		results.Add(cur);
	}

	hunspell->free_list(&result, n);
}

bool SpellChecker::AddWord(wxString word)
{
	if (word.IsEmpty() || word.IsNumber()) return false;

	hunspell->add(word.mb_str(*conv));//.mb_str(*conv))
	//wxString pathhh = Options.pathfull + L"\\Dictionary\\UserDic.udic";
	OpenWrite ow;
	wxString txt;
	if (!ow.FileOpen(userDictionaryPath, &txt, false)){ txt = word; }
	else{ txt += L"\n" + word; }
	ow.FileWrite(userDictionaryPath, txt);

	return true;
}

bool SpellChecker::RemoveWords(const wxArrayString &words)
{
	if (!words.size())
		return false;

	int succeded = 0;
	int counter = 0;
	//wxString userpath = Options.pathfull + "\\Dictionary\\UserDic.udic";
	if (wxFileExists(userDictionaryPath)) {
		OpenWrite op;
		wxString txt;
		if (!op.FileOpen(userDictionaryPath, &txt, false)) { return false; }
		wxStringTokenizer textIn(txt, L"\n");
		bool found = false;
		wxString newTxt;
		while (textIn.HasMoreTokens()) {
			// Read line
			wxString curLine = textIn.NextToken();
			curLine.Trim();
			int foundWord = words.Index(curLine);
			if (foundWord != -1){
				found = true;
				succeded = hunspell->remove(words[foundWord].mb_str(*conv));
				continue;
			}
			newTxt << curLine << L"\r\n";
		}
		if (found){
			op.FileWrite(userDictionaryPath, newTxt);
			return true;
		}
	}
	return succeded > 0;
}

//text for both
//errors table for both
//misspells table for MyTextEditor
//tags replacement for SubsGrid

void SpellChecker::CheckTextAndBrackets(const wxString &text, wxArrayInt *errs, wxArrayString *misspells, const wxString &tagsReplacement)
{
	using namespace boost::locale;
	using namespace std;
	size_t tagsReplacementLen = tagsReplacement.length();
	bool repltags = tagsReplacementLen > 0;
	bool block = false;
	bool hasReplTags = false;
	wstring checkText;
	int lastStartBracket = -1;
	int lastEndBracket = -1;
	int lastStartTBracket = -1;
	int lastStartCBracket = -1;
	int lastEndCBracket = -1;
	std::vector<size_t> textOffset;
	size_t textLen = text.length();
	size_t i = 0;
	while (i < textLen)
	{
		const wxUniChar &ch = text[i];
		if (block) {
			if (ch == L'{') { 
				errs->Add(lastStartCBracket); 
				errs->Add(lastStartCBracket);
				if(misspells)
					misspells->Add(L"");
			}
			if (ch == L'\\' && text[(i == 0) ? 0 : i - 1] == L'\\') { 
				errs->Add(i); 
				errs->Add(i); 
				if (misspells)
					misspells->Add(L"");
			}
			if (ch == L'(') {
				if (i > 1 && text[i - 2] == L'\\' && text[i - 1]) { 
					lastStartTBracket = i; 
					i++;
					continue; 
				}
				if (lastStartBracket > lastEndBracket) {
					errs->Add(lastStartBracket); 
					errs->Add(lastStartBracket);
					if (misspells)
						misspells->Add(L"");
				}
				lastStartBracket = i;
			}
			if (ch == L')') {
				if ((lastStartBracket < lastEndBracket || lastStartBracket < 0)) {
					if (lastStartTBracket > 0 && (lastStartTBracket < lastEndBracket || lastStartBracket < lastStartTBracket)) {
						lastStartTBracket = -1; 
						i++;
						continue;
					}
					errs->Add(i); 
					errs->Add(i);
					if (misspells)
						misspells->Add(L"");
				}

				lastEndBracket = i;
			}
		}
		if (lastStartTBracket >= 0 && ch == L'{' || ch == L'}') {
			errs->Add(lastStartTBracket); 
			errs->Add(lastStartTBracket);
			if (misspells)
				misspells->Add(L"");
			lastStartTBracket = -1;
		}
		if (ch == L'{') {
			block = true;
			lastStartCBracket = i;
		}
		else if (ch == L'}') {
			if (!block) {
				errs->Add(i);
				errs->Add(i);
				if (misspells)
					misspells->Add(L"");
			}

			block = false;
			lastEndCBracket = i;
			
			i++;
			continue;
		}
		else if (repltags && tagsReplacement[0] == ch && text.Mid(i, tagsReplacementLen) == tagsReplacement) {
			i += tagsReplacementLen;
			continue;
		}


		if (!block && ch != L'\\') {
			if (!(text.GetChar((i == 0) ? 0 : i - 1) == L'\\' && (ch == L'N' || ch == L'n' || ch == L'h'))) {
				checkText += ch;
				textOffset.push_back(i);
			}
			else {
				//replace for \n
				checkText += L"  ";
				textOffset.push_back(i - 1);
				textOffset.push_back(i);
			}

		}

		i++;

	}
	boundary::wssegment_index index(boundary::word, checkText.begin(), checkText.end());
	boundary::wssegment_index::iterator p, e;
	int counter1 = 0;
	if (checkText.size() == textOffset.size()) {
		for (p = index.begin(), e = index.end(); p != e; ++p) {
			wxString word = wxString(*p);
			if (p->rule() & boundary::word_letters) {
				if (!CheckWord(word)) {
					size_t start = textOffset[counter1];
					size_t end = textOffset[counter1 + word.length() - 1];
					if (end - start > word.length()) {
						size_t j = start, k = start;
						while (j <= end) {
							const wxUniChar & chr = text[j];
							if (chr == L'{' || j >= end) {
								errs->Add(k);
								errs->Add(j >= end? j : j - 1);
								if (misspells)
									misspells->Add(word);
							}
							else if (chr == L'}') {
								k = j + 1;
							}
							else if (repltags && tagsReplacement[0] == chr && text.Mid(j, tagsReplacementLen) == tagsReplacement) {
								errs->Add(k);
								errs->Add(j - 1);
								if (misspells)
									misspells->Add(word);
								j += tagsReplacementLen - 1;
								k = j + 1;
							}
							j++;
						}
					}
					else {
						errs->Add(start);
						errs->Add(end);
						if (misspells)
							misspells->Add(word);
					}
					
				}

			}
			counter1 += word.length();
		}
	}
	else {
		KaiLog(L"text offset size != text size not spellchecking");
	}


	if (lastStartCBracket > lastEndCBracket) { 
		errs->Add(lastStartCBracket); 
		errs->Add(lastStartCBracket); 
		if (misspells)
			misspells->Add(L"");
	}
	if (lastStartBracket > lastEndBracket) { 
		errs->Add(lastStartBracket); 
		errs->Add(lastStartBracket); 
		if (misspells)
			misspells->Add(L"");
	}
	if (lastStartTBracket >= 0) { 
		errs->Add(lastStartTBracket); 
		errs->Add(lastStartTBracket); 
		if (misspells)
			misspells->Add(L"");
	}
	//mytexteditor do not need this
	if (errs->size() < 2 && !misspells) { errs->Add(0); }

}

//for spellchecker window
void SpellChecker::CheckText(const wxString &text, wxArrayInt *errs)
{
	using namespace boost::locale;
	using namespace std;
	bool block = false;
	wstring checkText;
	int firsti = 0;
	size_t textLen = text.length();

	for (size_t i = 0; i < textLen; i++)
	{
		const wxUniChar &ch = text[i];
		
		if (ch == L'{') {
			block = true;
			if (checkText.size())
				goto check;
			else
				continue;
		}
		else if (ch == L'}') {
			block = false;
			firsti = i + 1;
			if (checkText.size())
				checkText.clear();

			continue;
		}
		
		if (!block && ch != L'\\') {
			if (!(text.GetChar((i == 0) ? 0 : i - 1) == L'\\' && (ch == L'N' || ch == L'n' || ch == L'h'))) {
				checkText += ch;
			}
			else {
				//replace for \n
				checkText += L"  ";
			}

		}

		if (i < textLen - 1)
			continue;

	check:


		boundary::wssegment_index index(boundary::word, checkText.begin(), checkText.end());
		boundary::wssegment_index::iterator p, e;
		int counter1 = firsti;
		for (p = index.begin(), e = index.end(); p != e; ++p) {
			wxString word = wxString(*p);
			if (p->rule() & boundary::word_letters) {
				if (!CheckWord(word)) {
					errs->Add(counter1);
					errs->Add(counter1 + word.length() - 1);
				}

			}
			counter1 += word.length();
		}
	}

	if (errs->size() < 2) { errs->Add(0); }
}