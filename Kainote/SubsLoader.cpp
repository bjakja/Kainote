//  Copyright (c) 2012 - 2020, Marcin Drob

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

#include "SubsLoader.h"
#include "SubsGrid.h"
#include "KaiMessageBox.h"
#include "config.h"
#include <wx/tokenzr.h>


SubsLoader::SubsLoader(SubsGrid *_grid, const wxString &text, wxString &ext)
{
	grid = _grid;
	grid->Clearing();
	grid->file = new SubsFile(&grid->GetMutex());
	grid->originalFormat = 0;
	grid->hasTLMode = false;
	bool succeeded = false;
	bool validFormat = false;
	if (ext == L"ass" || ext == L"ssa"){
		succeeded = LoadASS(text);
		if (!succeeded){
			succeeded = LoadSRT(text);
			if (!succeeded){
				succeeded = LoadTXT(text);
				if (succeeded) ext = L"txt";
			}
			else{ ext = L"srt"; }
		}
		else{ validFormat = true; }
	}
	else if (ext == L"srt"){
		succeeded = LoadSRT(text);
		if (!succeeded){
			succeeded = LoadASS(text);
			if (!succeeded){
				succeeded = LoadTXT(text);
				if (succeeded) ext = L"txt";
			}
			else{ ext = L"ass"; }
		}
		else{ validFormat = true; }
	}
	else{
		succeeded = LoadTXT(text);
		grid->SetSubsFormat();
		if (grid->subsFormat == SRT){
			grid->Clearing();
			grid->file = new SubsFile(&grid->GetMutex());
			succeeded = LoadSRT(text);
			if (succeeded) ext = L"srt";
		}
		else if (grid->subsFormat == ASS){
			grid->Clearing();
			grid->file = new SubsFile(&grid->GetMutex());
			succeeded = LoadASS(text);
			ext = L"ass";
			if (!succeeded){
				grid->LoadDefault(false, false, false);
				succeeded = LoadTXT(text);
				KaiMessageBox(_("Ten plik napisów jest zwykłym tekstem, zostaje otwarty jako napisy ASS"));
			}
		}
		else{ validFormat = true; }
	}
	//WARNING! table can not be empty
	//text helper class will crash when gets NULL in = operator
	if (!succeeded){ 
		grid->LoadDefault(); 
		KaiMessageBox(_("Niepoprawny format (plik uszkodzony lub zawiera błędy)")); 
		grid->subsFormat = ASS; 
		ext = "ass"; 
	}
	else{
		grid->SetSubsFormat();
		if (validFormat)
			grid->originalFormat = grid->subsFormat;
	}
}

bool SubsLoader::LoadASS(const wxString &text)
{
	short section = 0;
	char format = ASS;
	wxStringTokenizer tokenizer(text, L"\n", wxTOKEN_STRTOK);

	bool tlmode = false;
	wxString tlstyle;


	while (tokenizer.HasMoreTokens())
	{
		wxString token = tokenizer.GetNextToken().Trim(false);
		if (token.empty()){ continue; }
		if ((token.StartsWith(L"Dial") || token.StartsWith(L"Comm") || (token[0] == L';' && section > 2))){
			Dialogue *dl = new Dialogue(token);
			if (!tlmode){
				grid->AddLine(dl);
			}
			else if (tlmode && dl->Style == tlstyle){
				wxString ntoken = tokenizer.GetNextToken();
				Dialogue *tl = new Dialogue(ntoken);
				tl->TextTl = tl->Text;
				tl->Text = dl->Text;
				if (dl->Effect == L"\fD"){
					tl->ChangeState(4);
				}
				grid->AddLine(tl);
				delete dl;
			}
			else{
				grid->AddLine(dl);
			}
		}
		else if (token.StartsWith(L"Style:"))
		{
			//1 = ASS, 2 = SSA, needs only for subtitles loading.
			grid->AddStyle(new Styles(token, format));
			section = 2;
		}
		else if (token.StartsWith(L"[V4")){
			if (!token.StartsWith(L"[V4+")){
				format = 2;
				grid->AddSInfo(L"ScriptType", L"4.00+");
			}
			//format SSA was removed from working formats
			//need to send to constructor of styles 2 as format SSA, SRT has 3 to avoid bugs
			section = 1;
		}
		else if (token[0] != L';' && token[0] != L'[' && token.Find(L':') != wxNOT_FOUND && !token.StartsWith(L"Format")){
			grid->AddSInfo(token);
		}
		else if (token.StartsWith(L"[Eve")){
			tlmode = (grid->GetSInfo(L"TLMode") == L"Yes");
			if (tlmode){ tlstyle = grid->GetSInfo(L"TLMode Style"); if (tlstyle == emptyString){ tlmode = false; } }
			section = 3;
		}
	}
	grid->hasTLMode = tlmode;
	const wxString &matrix = grid->GetSInfo(L"YCbCr Matrix");
	if (matrix == emptyString || matrix == L"None"){ grid->AddSInfo(L"YCbCr Matrix", L"TV.601"); }
	return grid->GetCount() > 0;
}

bool SubsLoader::LoadSRT(const wxString &text)
{
	wxStringTokenizer tokenizer(text, L"\n", wxTOKEN_STRTOK);
	tokenizer.GetNextToken();

	wxString text1;
	while (tokenizer.HasMoreTokens()){
		wxString text = tokenizer.GetNextToken().Trim();
		if (IsNumber(text)){
			if (text1 != emptyString){
				grid->AddLine(new Dialogue(text1.Trim()));
				text1 = emptyString;
			}
		}
		else{ text1 << text << L"\r\n"; }
	}

	if (text1 != emptyString){
		grid->AddLine(new Dialogue(text1.Trim()));
		text1 = emptyString;
	}
	return grid->GetCount() > 0;
}

bool SubsLoader::LoadTXT(const wxString &text)
{
	wxStringTokenizer tokenizer(text, L"\n", wxTOKEN_STRTOK);
	while (tokenizer.HasMoreTokens()){
		wxString text = tokenizer.GetNextToken().Trim();
		grid->AddLine(new Dialogue(text.Trim()));
	}
	return grid->GetCount() > 0;
}

