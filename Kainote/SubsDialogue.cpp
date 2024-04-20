//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include "SubsDialogue.h"
#include "config.h"
//#include "Utils.h"
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/log.h>
#include <map>
#include <iostream>


TagData::TagData(const wxString &name, unsigned int _startTextPos)
{
	tagName = name;
	startTextPos = _startTextPos;
}

void TagData::PutValue(const wxString &_value, bool _multiValue)
{
	value = _value;
	multiValue = _multiValue;
}


ParseData::ParseData()
{
}

void ParseData::AddData(TagData *data)
{
	tags.push_back(data);
}

ParseData::~ParseData()
{
	for (auto it = tags.begin(); it != tags.end(); it++){
		delete (*it);
	}
	tags.clear();
}

Dialogue::Dialogue()
{
	Format = ASS;
	Layer = 0;
	End.mstime = 5000;
	Style = L"Default";
	MarginL = 0;
	MarginR = 0;
	MarginV = 0;
	State = 0;
	NonDialogue = false;
	IsComment = false;
}

Dialogue::~Dialogue()
{
	ClearParse();
}

void Dialogue::ClearParse()
{
	if (parseData){ delete parseData; parseData = nullptr; }
}

void Dialogue::AddResetOnMDVDWraps(const wxString & prefix)
{
	size_t textPos = 0;
	wxString copyText = Text;
	size_t diff = 0;
	size_t seekPos = 0;
	bool needAddPrefix = true;
	bool addPrefixes = prefix.length() > 0;
	while (textPos < Text->length()){
		textPos = copyText.find(L"|");
		if (needAddPrefix && addPrefixes){
			Text->insert(diff, L"{" + prefix + L"}");
			diff += (prefix.length() + 2);
			needAddPrefix = false;
		}
		if (textPos == -1)
			break;
		
		if (StartsWithNoBlock(copyText, L'{', &seekPos)){
			Text->insert(textPos + diff, L"{\\r}");
			needAddPrefix = true;
			//{\r} 4 characters
			diff += 4;
		}
		//skip |
		textPos += 1;
		diff += textPos;
		copyText = copyText.Mid(textPos);
	
	}
	//if (addPrefixes)
		//Text->Prepend(L"{" + prefix + L"}");
}

void Dialogue::ReplaceSlashesToItalics()
{
	size_t textPos = 0;
	wxString copyText = Text;
	size_t diff = 0;
	size_t slashPos = 0;
	while (textPos < Text->length()){
		textPos = copyText.find(L"|");
		if (textPos == -1)
			textPos = copyText.length();

		
		if (StartsWith(copyText, L'/', &slashPos)){
			Text->replace(slashPos + diff, 1, L"{\\i1}");
			diff += 4;
		}
		slashPos = textPos;

		if (EndsWith(copyText, L'/', &slashPos)){
			Text->erase(slashPos + diff, 1);
			diff -= 1;
		}
		//skip |
		textPos += 1;
		if (textPos >= copyText.length())
			break;
		diff += textPos;
		copyText = copyText.Mid(textPos);

	}
}

bool Dialogue::StartsWith(const wxString &text, wxUniChar ch, size_t *pos)
{
	bool block = false;
	for(size_t i = 0; i < text.length(); i++){
		wxUniChar c = text[i];
		if (c == L'{')
			block = true;
		else if (c == L'}')
			block = false;
		else if (wxIsspace(c) || block)
			continue;
		else if (c == ch){
			*pos = i;
			return true;
		}
		else
			return false;
	}
	return false;
}
bool Dialogue::StartsWithNoBlock(const wxString &text, wxUniChar ch, size_t *pos)
{
	for (size_t i = 0; i < text.length(); i++){
		wxUniChar c = text[i];
		if (wxIsspace(c))
			continue;
		else if (c == ch){
			*pos = i;
			return true;
		}
		else
			return false;
	}
	return false;
}


// it starts with position of pos argument
bool Dialogue::EndsWith(const wxString &text, wxUniChar ch, size_t *pos)
{
	bool block = false;
	for (size_t i = *pos; i > 0; i--){
		wxUniChar c = text[i - 1];
		if (c == L'{')
			block = false;
		else if (c == L'}')
			block = true;
		else if (wxIsspace(c) || block)
			continue;
		else if (c == ch){
			*pos = i - 1;
			return true;
		}
		else
			return false;
	}
	return false;
}

const wxString & Dialogue::GetTextNoCopy()
{
	return (TextTl != emptyString) ? TextTl : Text;
}
wxString & Dialogue::GetText()
{
	return Text.CheckTlRef(TextTl, TextTl != emptyString);
}

void Dialogue::SetText(const wxString &text)
{
	if (TextTl != emptyString)
		TextTl = text;
	else
		Text = text;
}

void Dialogue::GetTextElement(int replaceColumn, wxString *elementText, bool appendTextTL)
{
	if (replaceColumn == TXT){ 
		*elementText = Text; 
		if (appendTextTL && !TextTl->empty())
			*elementText << L"\n" << TextTl;
	}
	else if (replaceColumn == TXTTL){ *elementText = TextTl; }
	else if (replaceColumn == STYLE){ *elementText = Style; }
	else if (replaceColumn == ACTOR){ *elementText = Actor; }
	else if (replaceColumn == EFFECT){ *elementText = Effect; }
}

void Dialogue::SetTextElement(int replaceColumn, const wxString &elementText, bool appendTextTL)
{
	if (replaceColumn == TXT){ 
		if (appendTextTL && elementText.Find(L'\n') != -1){
			wxString * txttl = TextTl.Copy();
			Text = elementText.BeforeFirst(L'\n', txttl);
		}else
			Text = elementText; 
	}
	else if (replaceColumn == TXTTL){ TextTl = elementText; }
	else if (replaceColumn == STYLE){ Style = elementText; }
	else if (replaceColumn == ACTOR){ Actor = elementText; }
	else if (replaceColumn == EFFECT){ Effect = elementText; }
}

Dialogue::Dialogue(const wxString &ldial, const wxString &txttl)
{
	parseData = nullptr;
	TextTl = txttl;
	SetRaw(ldial);
}

static wxRegEx expresion1(L"^\\{([0-9-]+)\\}{([0-9-]*)\\}([^\r\n]*)", wxRE_ADVANCED);
static wxRegEx expresion2(L"^\\[([0-9-]+)\\]\\[([0-9-]*)\\]([^\r\n]*)", wxRE_ADVANCED);
static wxRegEx expresion(L"^([0-9]+)[:;]([0-9]+)[:;]([0-9]+)[:;, ]([^\r\n]*)", wxRE_ADVANCED);

char Dialogue::GetState()
{
	return State;
}

void Dialogue::ChangeState(char state)
{
	State ^= state;
}

void Dialogue::AddState(char state)
{
	State |= state;
}

void Dialogue::ChangeDialogueState(char state)
{
	//first thing reset state 1 and 2
	State >>= 2;
	State <<= 2;
	State |= state;
}

void Dialogue::SetRaw(const wxString &ldial)
{
	State = 0;
	//ldial.Trim(false);

	if (ldial.StartsWith(L"Dialogue") || ldial.StartsWith(L"Comment")){
		wxStringTokenizer assdal(ldial, L",", wxTOKEN_RET_EMPTY_ALL);
		if (assdal.CountTokens() >= 9){
			NonDialogue = false;
			wxString token = assdal.GetNextToken();
			if (token.StartsWith(L"Dialogue")){ IsComment = false; }
			else{ IsComment = true; }
			if (token.Find(L"arked=") == -1){ Layer = wxAtoi(token.AfterFirst(L' ')); }
			else{ Layer = wxAtoi(token.AfterLast(L'=')); }
			Format = ASS;
			Start.SetRaw(assdal.GetNextToken(), Format);
			End.SetRaw(assdal.GetNextToken(), Format);
			Style = assdal.GetNextToken();
			Actor = assdal.GetNextToken();
			if (Actor->StartsWith(L"[")){
				if (Actor->Replace(L"[bookmark]", emptyString)){
					State |= 8;
				}
				else if (Actor->Replace(L"[tree_closed]", emptyString)){
					treeState = TREE_CLOSED;
					isVisible = NOT_VISIBLE;
				}
				else if (Actor->Replace(L"[tree_opened]", emptyString)){
					treeState = TREE_OPENED;
				}
				else if (Actor->Replace(L"[tree_description]", emptyString)){
					treeState = TREE_DESCRIPTION;
				}
			}
			Actor.Trim(false);
			Actor.Trim(true);
			MarginL = wxAtoi(assdal.GetNextToken());
			MarginR = wxAtoi(assdal.GetNextToken());
			MarginV = wxAtoi(assdal.GetNextToken());
			Effect = assdal.GetNextToken();
			Effect.Trim(false);
			Effect.Trim(true);
			Text = ldial.Mid(assdal.GetPosition());
			Text.Trim(false);
			Text.Trim(true);
			return;
		}
	}

	Layer = 0;
	MarginL = 0;
	MarginR = 0;
	MarginV = 0;
	if (ldial.Find(L" --> ") != -1){
		wxString eend;
		wxString ttext;
		Format = SRT;
		Start.SetRaw(ldial.BeforeFirst(L' ', &eend), Format);
		eend = eend.AfterFirst(L' ');
		End.SetRaw(eend.BeforeFirst(L'\n', &ttext).Trim(), Format);
		Text = ttext;
		Text->Replace(L"\r", emptyString);
		Text->Replace(L"\n", L"\\N");
		NonDialogue = false;
		IsComment = false;
	}
	else if (expresion1.Matches(ldial))
	{
		NonDialogue = false;
		IsComment = false;
		Format = MDVD;
		Start.SetRaw(expresion1.GetMatch(ldial, 1), Format);
		End.SetRaw(expresion1.GetMatch(ldial, 2), Format);
		Text = expresion1.GetMatch(ldial, 3);
		Text.Trim(false);
		return;
	}
	else if (expresion2.Matches(ldial))
	{
		NonDialogue = false;
		IsComment = false;
		Format = MPL2;
		Start.SetRaw(expresion2.GetMatch(ldial, 1), Format);
		End.SetRaw(expresion2.GetMatch(ldial, 2), Format);
		Text = expresion2.GetMatch(ldial, 3);
		Text.Trim(false);
		return;
	}
	else if (expresion.Matches(ldial))
	{
		NonDialogue = false;
		IsComment = false;
		Format = TMP;
		wxString timeparts;
		Start.SetRaw(timeparts << expresion.GetMatch(ldial, 1) << L":" << expresion.GetMatch(ldial, 2) << L":" << expresion.GetMatch(ldial, 3), Format);
		Text = expresion.GetMatch(ldial, 4);
		Text.Trim(false);
		return;
	}
	else if (ldial.StartsWith(L";") || (ldial.StartsWith(L"{") && ldial.EndsWith(L"}") && ldial.Freq(L'{') == 1 && ldial.Freq(L'}') == 1)){
		NonDialogue = true;
		IsComment = true;
		Style = L"Default";
		Text = ldial;
		Text.Trim(true);
		Format = ASS;
		isVisible = NOT_VISIBLE;
		return;
	}
	else{
		Format = 0;
		NonDialogue = false;
		IsComment = false;
		Style = L"Default";
		Text = ldial;
		Text->Replace(L"\r", emptyString);
		Text->Replace(L"\n", L"\\N");
		Text.Trim(true);
	}

}

void Dialogue::GetRaw(wxString *txt, bool tl/*=false*/, const wxString &style/*=""*/, bool hideOriginalOnVideo /*= false*/)
{
	wxString line;
	if (Format < SRT){
		if (NonDialogue){ (*txt) << Text << L"\r\n"; return; }
		if (IsComment || hideOriginalOnVideo){ line = L"Comment: "; }
		else{ line = L"Dialogue: "; }
		bool styleTl = style != emptyString;
		const wxString &Styletl = (styleTl) ? style : Style;
		const wxString &EffectTl = (State & 4 && styleTl) ? wxString(L"\fD") : Effect;
		// state 8 - bookmarks
		wxString ActorWithStates = (State & 8) ? L"[bookmark]" + Actor : Actor;
		if (treeState){
			ActorWithStates.Prepend((treeState == TREE_DESCRIPTION) ? wxString(L"[tree_description]") :
				(treeState == TREE_OPENED) ? wxString(L"[tree_opened]") :
				(treeState == TREE_CLOSED) ? wxString(L"[tree_closed]") : emptyString);
		}
		line << Layer << L","
			<< Start.raw(Format) << L","
			<< End.raw(Format) << L","
			<< Styletl << L","
			<< ActorWithStates << L","
			<< MarginL << L","
			<< MarginR << L","
			<< MarginV << L","
			<< EffectTl << L",";
		line += (tl) ? TextTl : Text;
		//line+=wxString::Format("%i,%s,%s,%s,%s,%i,%i,%i,%s,%s",(int)Layer,Start.raw().data(),End.raw().data(),Styletl.data(),Actor.data(),(int)MarginL,(int)MarginR,(int)MarginV,Effect.data(),txttl.data());

	}
	else if (Format == MDVD){
		line << L"{" << Start.raw(Format) << L"}{" << End.raw(Format) << L"}" << Text;
	}
	else if (Format == MPL2){
		line << L"[" << Start.raw(Format) << L"][" << End.raw(Format) << L"]" << Text;
	}
	else if (Format == TMP){
		line << Start.raw(Format) << L":" << Text;
	}
	else if (Format == SRT){
		wxString txt = Text;
		txt.Replace(L"\\N", L"\r\n");
		line << Start.raw(Format) << L" --> " << End.raw(Format) << L"\r\n" << txt << L"\r\n";
	}
	line << L"\r\n";
	(*txt) << line;
}

wxString Dialogue::GetCols(int cols, bool tl, const wxString &style)
{

	wxString line;
	wxString txttl = (tl) ? TextTl : Text;
	if (cols & 2048){
		wxRegEx reg(L"\\{[^\\{]*\\}", wxRE_ADVANCED);
		reg.ReplaceAll(&txttl, emptyString);
		cols |= 1024;
	}
	if (Format < SRT){
		wxString Styletl = (style != emptyString) ? style : Style;
		if (cols & 1){ line << Layer << L","; }
		if (cols & 2){ line << Start.raw() << L","; }
		if (cols & 4){ line << End.raw() << L","; }
		if (cols & 8){ line << Styletl << L","; }
		if (cols & 16){ line << Actor << L","; }
		if (cols & 32){ line << MarginL << L","; }
		if (cols & 64){ line << MarginR << L","; }
		if (cols & 128){ line << MarginV << L","; }
		if (cols & 256){ line << Effect << L","; }
		if (cols & 1024){ line << txttl; }
		//line+=wxString::Format("%i,%s,%s,%s,%s,%i,%i,%i,%s,%s",(int)Layer,Start.raw().data(),End.raw().data(),Styletl.data(),Actor.data(),(int)MarginL,(int)MarginR,(int)MarginV,Effect.data(),txttl.data());

	}
	else if (Format == MDVD){
		if (cols & 2){ line << L"{" << Start.raw() << L"}"; }
		if (cols & 4){ line << L"{" << End.raw() << L"}"; }
		if (cols & 1024){ line << txttl; }
	}
	else if (Format == MPL2){
		if (cols & 2){ line << L"[" << Start.raw() << L"]"; }
		if (cols & 4){ line << L"[" << End.raw() << L"]"; }
		if (cols & 1024){ line << txttl; }
	}
	else if (Format == TMP){
		if (cols & 2){ line << Start.raw() << L":"; }
		if (cols & 1024){ line << txttl; }
	}
	else if (Format == SRT){
		txttl.Replace(L"\\N", L"\r\n");
		if (cols & 2){ line << Start.raw(); }
		if (cols & 4) { 
			if (cols & 2)
				line << L" --> ";

			line << End.raw(); 
			if(cols & 1024)
				line << L"\r\n";
		}
		if (cols & 1024){ line << txttl; }
	}

	line << L"\r\n";
	return line;
}

void Dialogue::Convert(char type, const wxString &prefix)
{
	if (!Format){ Format = 0; if (type == ASS){ return; } }
	if (Format == TMP && End.mstime == 0){ End = Start; End.mstime += 2000; }
	Start.ChangeFormat(type);
	End.ChangeFormat(type);
	if (type < SRT){
		Layer = 0;
		Style = Options.GetString(CONVERT_STYLE);
		Actor = emptyString;
		MarginL = 0;
		MarginR = 0;
		MarginV = 0;
		Effect = emptyString;
		wxString tmp = Text;
		if (Format != SRT){
			wxRegEx regib(L"\\{y[:+]([ib])\\}", wxRE_ADVANCED | wxRE_ICASE);
			wxRegEx regf(L"\\{f:([^}]*)\\}", wxRE_ADVANCED | wxRE_ICASE);
			wxRegEx regs(L"\\{s:([^}]*)\\}", wxRE_ADVANCED | wxRE_ICASE);
			wxRegEx regc(L"\\{c:\\$([^}]*)\\}", wxRE_ADVANCED | wxRE_ICASE);
			
			regib.ReplaceAll(&tmp, L"{\\\\\\1\t1}");
			tmp.Replace(L"\t", emptyString);
			regf.ReplaceAll(&tmp, L"{\\\\fn\\1}");
			regs.ReplaceAll(&tmp, L"{\\\\fs\\1}");
			regc.ReplaceAll(&tmp, L"{\\\\1c\\&H\\1\\&}");
			//tmp.Replace(L"|", L"\\N");
			//size_t il = tmp.Replace(L"/", emptyString);
			//Text = pref;
			//if (il > 0){ Text << wxString(L"{\\i1}"); }
			Text = tmp;
			ReplaceSlashesToItalics();
			AddResetOnMDVDWraps(prefix);
			Text->Replace(L"|", L"\\N");
			Text->Replace(L"}{", emptyString);
		}
		else{
			wxRegEx regibu(L"\\<([ibu])\\>", wxRE_ADVANCED);
			wxRegEx regibu0(L"\\</([ibu])\\>", wxRE_ADVANCED);
			regibu.ReplaceAll(&tmp, L"{\\\\\\1\t1}");
			regibu0.ReplaceAll(&tmp, L"{\\\\\\1\t0}");
			tmp.Replace(L"\t", emptyString);
			tmp.Replace(L"<br>", L"\\N");
			Text = prefix + tmp;
		}

	}
	else if (Format < SRT){
		wxString tmp = Text;
		tmp.Replace(L"\\h", L" ");
		wxRegEx regp(L"\\\\p[0-9]+", wxRE_ADVANCED);
		if (regp.Matches(tmp)){ Text = emptyString; Format = type; return; }
		if (type == SRT){
			wxRegEx regibu(L"\\\\([ibu])1", wxRE_ADVANCED);
			wxRegEx regibu0(L"\\\\([ibu])0", wxRE_ADVANCED);
			regibu.ReplaceAll(&tmp, L"}<\\1>{");
			regibu0.ReplaceAll(&tmp, L"}</\\1>{");
		}
		wxRegEx reg(L"\\{[^}]*\\}", wxRE_ADVANCED);
		reg.ReplaceAll(&tmp, emptyString);
		if (type != SRT){
			tmp.Replace(L"\\N", L"|");
		}
		Text = tmp;

	}
	else if (Format == SRT){
		wxString tmp = Text;
		tmp.Replace(L"\\N", L"|");
		tmp.Replace(L"<br>", L"|");
		if (type == MDVD){
			tmp.Replace(L"<i>", L"{y:i}");
			tmp.Replace(L"<b>", L"{y:b}");
		}
		else if (type == MPL2){
			tmp.Replace(L"<i>", L"/");
		}
		wxRegEx reg(L"\\<[^>]*\\>", wxRE_ADVANCED);
		reg.ReplaceAll(&tmp, emptyString);
		Text = tmp;
	}
	else if (type == SRT){
		wxString tmp = Text;
		tmp.Replace(L"|", L"\\N");
		if (type == MDVD){
			tmp.Replace(L"{y:i}", L"<i>");
			tmp.Replace(L"{y:b}", L"<b>");
			wxRegEx reg(L"\\{[^}]*\\}", wxRE_ADVANCED);
			reg.ReplaceAll(&tmp, emptyString);
		}
		else if (type == MPL2){
			tmp.Replace(L"/", L"<i>");
		}
		Text = tmp;
	}
	else if (Format == MDVD && type == MPL2){
		wxString tmp = Text;
		tmp.Replace(L"{y:i}", L"/");
		wxRegEx reg(L"\\{[^}]*\\}", wxRE_ADVANCED);
		reg.ReplaceAll(&tmp, emptyString);
		Text = tmp;
	}
	else if (Format == MPL2 && type == MDVD){
		Text->Replace(L"/", L"{y:i}");
	}
	else{
		if (Format == MDVD){
			wxString tmp = Text;
			wxRegEx reg(L"\\{[^}]*\\}", wxRE_ADVANCED);
			reg.ReplaceAll(&tmp, emptyString);
			Text = tmp;
		}
		else if (Format == MPL2){
			Text->Replace(L"/", emptyString);
		}
	}

	Format = type;
}

Dialogue *Dialogue::Copy(bool keepstate, bool copyIsVisible)
{
	Dialogue *dial = new Dialogue();
	//if(!dial){return nullptr;}
	dial->Actor = Actor;
	dial->Effect = Effect;
	dial->End = End;
	dial->Format = Format;
	dial->IsComment = IsComment;
	dial->Layer = Layer;
	dial->MarginL = MarginL;
	dial->MarginR = MarginR;
	dial->MarginV = MarginV;
	dial->NonDialogue = NonDialogue;
	dial->Start = Start;
	dial->State = State;
	if (!keepstate)
		dial->ChangeDialogueState(1);

	dial->Style = Style;
	dial->Text = Text;
	dial->TextTl = TextTl;
	dial->treeState = treeState;
	dial->isVisible.Store(isVisible, copyIsVisible);
	dial->parseData = nullptr;
	return dial;
}

//Remember parse patterns need "tag1|tag2|..." without slashes.
//Remember string position is start of the value, position of tag -=tagname.len+1
ParseData* Dialogue::ParseTags(wxString *tags, size_t ntags, bool plainText)
{
	if (parseData) {
		delete parseData; 
		parseData = nullptr;
	}
	wxString txt = (TextTl != emptyString) ? TextTl : Text;
	size_t pos = 0;
	size_t plainStart = 0;
	bool hasDrawing = false;
	size_t len = txt.length();
	bool tagsBlock = false;
	parseData = new ParseData();
	double tmpValue;
	if (len < 1){ return parseData; }
	while (pos < len){
		wxUniChar ch = txt[pos];
		if (ch == L'}'){ tagsBlock = false; plainStart = pos + 1; }
		else if (ch == L'{' || pos >= len - 1){
			tagsBlock = true;
			if (pos >= len - 1){ pos++; }
			//to not crash the program when subtract from unsigned 0 just add 1 to plain start
			if ((plainText || hasDrawing) && plainStart + 1 <= pos){
				TagData *newTag = new TagData((hasDrawing) ? L"pvector" : L"plain", plainStart);
				newTag->PutValue(txt.SubString(plainStart, pos - 1));
				parseData->AddData(newTag);
			}
		}
		else if (tagsBlock && ch == L'\\'){
			pos++;
			//wxString tag;
			int slashPos = txt.find(L'\\', pos);
			int bracketPos = txt.find(L'}', pos);
			int tagEnd = (slashPos == -1 && bracketPos == -1) ? len :
				(slashPos == -1) ? bracketPos : (bracketPos == -1) ? slashPos :
				(bracketPos < slashPos) ? bracketPos : slashPos;
			wxString tag = txt.SubString(pos, tagEnd - 1);
			if (tag.EndsWith(L')')){ tag.RemoveLast(); }
			for (size_t i = 0; i < ntags; i++){
				wxString tagName = tags[i];
				int tagLen = tagName.length();
				if (tag.StartsWith(tagName) && tag.length() > tagLen){
					wxUniChar firstValueChar = tag[tagLen];
					if (firstValueChar == L'(' || wxIsdigit(firstValueChar) || tagName == L"fn" ||
						firstValueChar == L'.' || firstValueChar == L'-' || firstValueChar == L'+'){

						TagData *newTag = new TagData(tagName, pos + tagLen);
						wxString tagValue = tag.Mid(tagLen);
						if (tagName == L"p"){
							hasDrawing = (tagValue.Trim().Trim(false) == L"0") ? false : true;
							newTag->PutValue(tagValue);
						}
						else if (tag[tagLen] == L'('){
							newTag->startTextPos++;
							newTag->PutValue(tagValue.After(L'(').BeforeFirst(L')'), true);
						}
						else{
							if (tagName != L"fn" && !tagValue.ToCDouble(&tmpValue)){
								wxString newTagValue;
								for (const auto & ch : tagValue){
									if (!wxIsdigit(ch) && ch != L'.' && ch != L'-' && ch != L'+')
										break;
									newTagValue += ch;
								}
								tagValue = newTagValue;
							}
							newTag->PutValue(tagValue);
						}
						parseData->AddData(newTag);
						pos = tagEnd - 1;
						break;
					}
				}
			}
		}
		pos++;
	}
	return parseData;
}
//adding this time
void Dialogue::ChangeTimes(int start, int end)
{
	wxString tags[] = { L"move", L"t", L"fad" };
	ParseTags(tags, 3);/*|fade*/
	size_t replaceMismatch = 0;
	for (size_t i = 0; i < parseData->tags.size(); i++){
		TagData *tdata = parseData->tags[i];
		wxStringTokenizer splitValues(tdata->value, L",", wxTOKEN_STRTOK);
		int tokenCount = splitValues.CountTokens();
		if (tdata->tagName == L"move" && tokenCount < 5 ||
			tdata->tagName == L"t" && tokenCount < 2 ||
			tdata->tagName == L"fad" && tokenCount < 1){
			continue;
		}
		int numToken = 0;
		while (splitValues.HasMoreTokens()){
			wxString token = splitValues.GetNextToken();
			if (tdata->tagName == L"move" && numToken < 4){
				numToken++;
				continue;
			}
			size_t t1Len = token.length();

			int t1 = wxAtoi(token);
			size_t t1Pos = splitValues.GetPosition() + tdata->startTextPos - t1Len - replaceMismatch - 1;
			token = splitValues.GetNextToken();
			size_t t2Len = token.length();

			size_t totalLen = t1Len + t2Len + 1;
			int t2 = wxAtoi(token);
			t1 = MAX(0, t1 + start);
			t2 = MAX(0, t2 + end);

			wxString timesString;
			timesString << t1 << L"," << t2;
			replaceMismatch += totalLen - timesString.length();
			if (TextTl != emptyString){
				TextTl->replace(t1Pos, totalLen, timesString);
			}
			else{
				Text->replace(t1Pos, totalLen, timesString);
			}
			break;
		}
	}
}
