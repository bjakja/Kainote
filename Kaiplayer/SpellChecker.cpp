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
	//useSpellchecker checks if hunspell exist, no need check it again
	if (!useSpellChecker) return true;

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

inline void SpellChecker::Check(std::wstring &checkText, TextData *errs, std::vector<MisspellData> *misspells, std::vector<size_t> &textOffset, const wxString &text, bool repltags, int replaceTagsLen) {
	using namespace boost::locale;
	boundary::wssegment_index index(boundary::word, checkText.begin(), checkText.end());
	boundary::wssegment_index::iterator p, e;
	int counter1 = 0;
	int charCounter = 0;
	if (checkText.size() == textOffset.size()) {
		for (p = index.begin(), e = index.end(); p != e; ++p) {
			wxString word = wxString(*p);
			size_t wordLen = word.length();
			if (p->rule() & boundary::word_letters) {
				if (!CheckWord(word)) {
					size_t start = textOffset[counter1];
					size_t end = textOffset[counter1 + wordLen - 1];
					if (end - start > wordLen) {
						size_t j = start, k = start, m = start;
						while (j <= end) {
							const wxUniChar & chr = text[j];
							if (chr == L'{' || j >= end) {
								errs->Add(repltags ? m : k);
								errs->Add(j >= end ? j : j - 1);

								if (misspells)
									misspells->push_back(MisspellData(word, start, end));
								m = j - 1;
							}
							else if (chr == L'}') {
								k = j + 1;
								m += replaceTagsLen;
							}

							j++;
						}
					}
					else {
						errs->Add(start);
						errs->Add(end);
						if (misspells)
							misspells->push_back(MisspellData(word, start, end));
					}

				}
				charCounter += wordLen;
			}
			else if (p->rule() & boundary::word_number) {
				charCounter += wordLen;
			}
			counter1 += wordLen;
		}
	}
	else {
		KaiLog(L"text offset size != text size not spellchecking");
	}
	errs->chars += charCounter;
	if(!errs->badWraps)
		errs->badWraps = (charCounter > 43) || errs->wraps.Freq(L'/') > 1;
	errs->wraps << charCounter << L"/";
	checkText.clear();
	textOffset.clear();
}
//text for both
//errors table for both
//misspells table for MyTextEditor
//tags replacement for SubsGrid

void SpellChecker::CheckTextAndBrackets(const wxString &text, TextData *errs, bool spellchecker, int subsFormat, std::vector<MisspellData> *misspells, int replaceTagsLen)
{
	bool repltags = replaceTagsLen >= 0;
	bool block = false;
	bool hasReplTags = false;
	bool isSlash = false;
	std::wstring checkText;
	int lastStartBracket = -1;
	int lastEndBracket = -1;
	int lastStartTBracket = -1;
	int lastStartCBracket = -1;
	int lastEndCBracket = -1;
	useSpellChecker = spellchecker && hunspell;
	bool drawing = false;
	wxUniChar split = (subsFormat > SRT) ? L'|' : L'\\';
	wxUniChar bracketStart = (subsFormat == SRT) ? L'<' : L'{';
	wxUniChar bracketEnd = (subsFormat == SRT) ? L'>' : L'}';
	std::vector<size_t> textOffset;
	size_t textLen = text.length();
	size_t i = 0;
	size_t tagReplaceI = 0;
	while (i < textLen)
	{
		const wxUniChar &ch = text[i];
		if (block) {
			if (text[i] == L'p' && text[i - 1] == L'\\' && (i + 1 < textLen && wxIsdigit(text[i + 1]))) {
				if (text[i + 1] == L'0') { drawing = false; }
				else { drawing = true; }
			}
			else if (!repltags){
				if (ch == bracketStart) {
					errs->Add(lastStartCBracket);
					errs->Add(lastStartCBracket);
					if (misspells)
						misspells->push_back(MisspellData());
				}
				if (ch == L'\\' && text[(i == 0) ? 0 : i - 1] == L'\\') {
					errs->Add(i);
					errs->Add(i);
					if (misspells)
						misspells->push_back(MisspellData());
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
							misspells->push_back(MisspellData());
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
							misspells->push_back(MisspellData());
					}

					lastEndBracket = i;
				}
			}
		}
		if (lastStartTBracket >= 0 && ch == bracketStart || ch == bracketEnd) {
			errs->Add(lastStartTBracket); 
			errs->Add(lastStartTBracket);
			if (misspells)
				misspells->push_back(MisspellData());
			lastStartTBracket = -1;
		}
		if (ch == bracketStart) {
			if (repltags) {
				tagReplaceI += replaceTagsLen;
			}
			block = true;
			lastStartCBracket = i;
		}
		else if (ch == bracketEnd) {
			if (!block) {
				errs->Add(i);
				errs->Add(i);
				if (misspells)
					misspells->push_back(MisspellData());
			}

			block = false;
			lastEndCBracket = i;
			
			i++;
			continue;
		}
		

		if (!block && !drawing) {
			if (ch == split) {
				if (subsFormat > SRT) {
					checkText += L" ";
					textOffset.push_back(repltags ? tagReplaceI : i);
					Check(checkText, errs, misspells, textOffset, text, repltags, replaceTagsLen);
				}
				else{
					//replace for \n
					wxUniChar &nch = text[(i + 1 < textLen) ? i + 1 : i];
					bool splitSecond = nch == L'N' || nch == L'n';
					if (splitSecond || nch == L'h')
						checkText += L"  ";
					else {
						checkText += ch;
						checkText += nch;
					}
					textOffset.push_back(repltags ? tagReplaceI : i);
					textOffset.push_back(repltags ? tagReplaceI + 1 : i + 1);

					if (splitSecond) {
						Check(checkText, errs, misspells, textOffset, text, repltags, replaceTagsLen);
					}

					i++;
					tagReplaceI++;
				}

			}
			else {
				checkText += ch;
				textOffset.push_back(repltags? tagReplaceI : i);
			}
			tagReplaceI++;
		}

		i++;

	}
	if (checkText.size()) {
		Check(checkText, errs, misspells, textOffset, text, repltags, replaceTagsLen);
	}
	if (errs->wraps.empty())
		errs->wraps = L"0";

	if (lastStartCBracket > lastEndCBracket) { 
		errs->Add(lastStartCBracket); 
		errs->Add(lastStartCBracket); 
		if (misspells)
			misspells->push_back(MisspellData());
	}
	if (lastStartBracket > lastEndBracket) { 
		errs->Add(lastStartBracket); 
		errs->Add(lastStartBracket); 
		if (misspells)
			misspells->push_back(MisspellData());
	}
	if (lastStartTBracket >= 0) { 
		errs->Add(lastStartTBracket); 
		errs->Add(lastStartTBracket); 
		if (misspells)
			misspells->push_back(MisspellData());
	}
	
	
}

//for spellchecker window
void SpellChecker::CheckText(const wxString &text, std::vector<MisspellData> *errs, int subsFormat)
{
	using namespace boost::locale;
	using namespace std;
	bool block = false;
	bool drawing = false;
	useSpellChecker = hunspell;
	wstring checkText;
	size_t textLen = text.length();
	size_t i = 0;
	wxUniChar split = (subsFormat > SRT) ? L'|' : L'\\';
	wxUniChar bracketStart = (subsFormat == SRT) ? L'<' : L'{';
	wxUniChar bracketEnd = (subsFormat == SRT) ? L'>' : L'}';
	std::vector<size_t> textOffset;
	while (i < textLen)
	{
		const wxUniChar &ch = text[i];
		if (block) {
			if (text[i] == L'p' && text[i - 1] == L'\\' && (i + 1 < textLen && wxIsdigit(text[i + 1]))) {
				if (text[i + 1] == L'0') { drawing = false; }
				else { drawing = true; }
			}
		}
		if (ch == bracketStart) {
			block = true;
			i++;
			continue;
		}
		else if (ch == bracketEnd) {
			block = false;
			i++;
			continue;
		}
		
		if (!block && !drawing) {
			if (ch == split) {
				if (subsFormat > SRT) {
					checkText += L" ";
					textOffset.push_back(i);
				}
				else {
					//replace for \n
					wxUniChar &nch = text[(i + 1 < textLen) ? i + 1 : i];
					bool splitSecond = nch == L'N' || nch == L'n';
					if (splitSecond || nch == L'h')
						checkText += L"  ";
					else {
						checkText += ch;
						checkText += nch;
					}
					textOffset.push_back(i);
					textOffset.push_back(i + 1);

					i++;
				}

			}
			else {
				checkText += ch;
				textOffset.push_back(i);
			}

		}
		i++;
	}

	if (textOffset.size() == checkText.size()) {
		boundary::wssegment_index index(boundary::word, checkText.begin(), checkText.end());
		boundary::wssegment_index::iterator p, e;
		int counter1 = 0;
		for (p = index.begin(), e = index.end(); p != e; ++p) {
			wxString word = wxString(*p);
			size_t wordLen = word.length();
			if (p->rule() & boundary::word_letters) {
				if (!CheckWord(word)) {
					size_t start = textOffset[counter1];
					size_t end = textOffset[counter1 + wordLen - 1];
					errs->push_back(MisspellData(word, start, end));
				}

			}
			counter1 += wordLen;
		}
	}
	else {
		KaiLog(L"text offset size != text size no spellchecking");
	}
}

void SpellChecker::ReplaceMisspell(const wxString & misspell, const wxString &misspellReplace, int start, int end, wxString * textToReplace, int *newPosition)
{
	bool hasTags = misspell.length() < end - start - 1;
	if (hasTags) {
		int j = start;
		int k = 0;
		size_t replaceLen = misspellReplace.Length();
		wxString newReplace;
		bool block = false;
		while (j <= end) {
			wxUniChar ch = (*textToReplace)[j];
			if (block) {
				newReplace << ch;
			}
			else if (ch == L'{') {
				block = true;
				newReplace << ch;
			}
			else if (!block && k < replaceLen) {
				newReplace << misspellReplace[k];
				k++;
			}
			if (ch == L'}') {
				block = false;
			}
			j++;
		}
		if (k < misspellReplace.size()) {
			newReplace << misspellReplace.Mid(k);
		}
		textToReplace->replace(start, end - start + 1, newReplace);
		if(newPosition)
			*newPosition = start + newReplace.Length();
	}
	else {
		textToReplace->replace(start, end - start + 1, misspellReplace);
		if(newPosition)
			*newPosition = start + misspellReplace.Length();
	}
}
