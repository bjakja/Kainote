//  Copyright (c) 2012-2017, Marcin Drob

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

SubsLoader::SubsLoader(SubsGrid *_grid, const wxString &text, wxString &ext)
{
	grid = _grid;
	grid->Clearing();
	grid->file = new SubsFile();
	bool succeeded = false;
	if (ext == "ass" || ext == "ssa"){
		succeeded = LoadASS(text);
		if (!succeeded){
			succeeded = LoadSRT(text);
			if (!succeeded){
				succeeded = LoadTXT(text);
				if (succeeded) ext = "txt";
			}
			else{ ext = "srt"; }
		}
	}
	else if (ext == "srt"){
		succeeded = LoadSRT(text);
		if (!succeeded){
			succeeded = LoadASS(text);
			if (!succeeded){
				succeeded = LoadTXT(text);
				if (succeeded) ext = "txt";
			}
			else{ ext = "ass"; }
		}
	}
	else{
		LoadTXT(text);
		grid->SetSubsFormat();
		if (grid->subsFormat == SRT){
			grid->Clearing();
			grid->file = new SubsFile();
			succeeded = LoadSRT(text);
			if (succeeded) ext = "srt";
		}
		else if (grid->subsFormat == ASS){
			grid->Clearing();
			grid->file = new SubsFile();
			succeeded = LoadASS(text);
			if (succeeded) ext = "ass";
		}
	}

	if (!succeeded){ grid->LoadDefault(); KaiMessageBox(_("Niepoprawny format (plik uszkodzony lub zawiera b³êdy)")); grid->subsFormat = ASS; }
	else{ grid->SetSubsFormat(); }
	grid->originalFormat = grid->subsFormat;
}

bool SubsLoader::LoadASS(const wxString &text)
{
	short section = 0;
	char format = ASS;
	wxStringTokenizer tokenizer(text, "\n", wxTOKEN_STRTOK);

	bool tlmode = false;
	wxString tlstyle;
	
	
	while (tokenizer.HasMoreTokens())
	{
		wxString token = tokenizer.GetNextToken().Trim(false);
		if (token.empty()){ continue; }
		if ((token.StartsWith("Dial") || token.StartsWith("Comm") || (token[0] == ';' && section > 2))){
			Dialogue *dl = new Dialogue(token);
			if (!tlmode){
				grid->AddLine(dl);
			}
			else if (tlmode && dl->Style == tlstyle){
				wxString ntoken = tokenizer.GetNextToken();
				Dialogue tl(ntoken);
				dl->Style = tl.Style;
				dl->TextTl = tl.Text;
				if (dl->Effect == "\fD"){
					dl->State |= 4;
				}
				dl->Effect = tl.Effect;
				grid->AddLine(dl);
			}
			else{
				//stary tryb t³umaczenia nie istnieje od 2 lat, nie ma sensu usuwaæ pustych linii, zwa¿ywszy na to, ¿e to mo¿e usun¹æ coœ wa¿nego.
				grid->AddLine(dl);
			}
		}
		else if (token.StartsWith("Style:"))
		{
			//1 = ASS, 2 = SSA, potrzebne tylko przy odczycie napisów.
			grid->AddStyle(new Styles(token, format));
			section = 2;
		}
		else if (token.StartsWith("[V4")){
			if (!token.StartsWith("[V4+")){
				format = 2;
				grid->AddSInfo("ScriptType", "4.00+");
			}//ze wzglêdu na to, ¿e wywali³em ssa z formatów
			//muszê pos³aæ do konstruktora styli 2 jako format SSA, nie u¿ywam tu srt, 
			//¿eby póŸniej Ÿle tego nie zinterpretowaæ
			section = 1;
		}
		else if (!token.StartsWith(";") && !token.StartsWith("[") && token.Find(':') != wxNOT_FOUND){
			grid->AddSInfo(token);
		}
		else if(token.StartsWith("[Eve")){
			tlmode = (grid->GetSInfo("TLMode") == "Yes");
			if (tlmode){ tlstyle = grid->GetSInfo("TLMode Style"); if (tlstyle == ""){ tlmode = false; } }
			section = 3;
		}
	}
	grid->hasTLMode = tlmode;
	wxString matrix = grid->GetSInfo("YCbCr Matrix");
	if (matrix == "" || matrix == "None"){ grid->AddSInfo("YCbCr Matrix", "TV.601"); }
	return grid->GetCount() > 0;
}
	
bool SubsLoader::LoadSRT(const wxString &text)
{
	wxStringTokenizer tokenizer(text, "\n", wxTOKEN_STRTOK);
	tokenizer.GetNextToken();

	wxString text1;
	while (tokenizer.HasMoreTokens()){
		wxString text = tokenizer.GetNextToken().Trim();
		if (grid->IsNumber(text)){
			if (text1 != ""){
				grid->AddLine(new Dialogue(text1.Trim())); text1 = "";
			}
		}
		else{ text1 << text << "\r\n"; }
	}

	if (text1 != ""){ grid->AddLine(new Dialogue(text1.Trim())); text1 = ""; }
	return grid->GetCount() > 0;
}

bool SubsLoader::LoadTXT(const wxString &text)
{
	wxStringTokenizer tokenizer(text, "\n", wxTOKEN_STRTOK);
	while (tokenizer.HasMoreTokens()){
		wxString text = tokenizer.GetNextToken().Trim();
		grid->AddLine(new Dialogue(text.Trim()));
	}
	return grid->GetCount() > 0;
}

