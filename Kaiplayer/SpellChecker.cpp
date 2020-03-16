// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016, Marcin Drob
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
		if (!isgood) { Options.SetBool(SpellcheckerOn, false); }
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

void SpellChecker::AvailableDics(wxArrayString &dics)
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
			dics.Add(dic[i].AfterLast(L'\\').BeforeFirst(L'.'));
		}
	}
}

bool SpellChecker::Initialize()
{
	Cleaning();

	//wxString pathhh = Options.pathfull + L"\\Dictionary\\";
	wxString name = Options.GetString(DictionaryLanguage);
	if (name == L""){ name = L"pl"; }
	wxString dic = dictionaryPath + name + L".dic";
	wxString aff = dictionaryPath + name + L".aff";
	// Check if language is available
	if (!wxFileExists(dic) || !wxFileExists(aff))
	{
		Options.SetBool(SpellcheckerOn, false);
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
