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

#include "SubsDialogue.h"
#include "config.h"
#include "UtilsWindows.h"
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/log.h>
#include <wx/tokenzr.h>
#include <map>
#include <iostream>
#include <boost/locale/boundary/index.hpp>

std::atomic<unsigned> Visibility::s_Epoch{ 0 };
#include <boost/locale/boundary/segment.hpp>
#include <boost/locale/boundary/types.hpp>

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

void TagData::AppendValue(const wxString& _value)
{
	value += _value;
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

void Dialogue::GetTextStripped(wxString* textStripped, const wxString& text, bool removeFirstBlock/* = false*/)
{
	const wxString& txt = text.empty() ? GetTextNoCopy() : text;
	if (txt.empty())
		return;
	if (removeFirstBlock) {
		size_t length = txt.Len();
		bool block = false;
		for (size_t i = 0; i < length; i++) {
			wxUniChar c = txt[i];
			if (c == L'{')
				block = true;
			else if (block && c == L'}') {
				block = false;
				continue;
			}
			if (!block && c != L'{') {
				*textStripped = text.substr(i);
				break;
			}
		}
	}
	else {
		wxRegEx re(L"{[^}]*}", wxRE_ADVANCED | wxRE_ICASE);
		if (!re.IsValid()) {
			KaiLogSilent(L"Text stripped regex is not valid");
			return;
		}
		*textStripped = txt;
		re.ReplaceAll(textStripped, L"");
	}
}

void Dialogue::GetFirstTagsBlock(wxString* tagBlock, const wxString& text/* = L""*/)
{
	const wxString& txt = text.empty() ? GetTextNoCopy() : text;
	size_t length = txt.Len();
	bool block = false;
	for (size_t i = 0; i < length; i++) {
		wxUniChar c = txt[i];
		if (c == L'{')
			block = true;
		else if (c == L'}') {
			*tagBlock << c;
			block = false;
		}
		if (block) {
			*tagBlock << c;
		}
		else if (i < length - 1 && txt[i + 1] != L'{') {
			break;
		}
	}
}

void Dialogue::MergeTagBlocks(wxString* output, const wxString& blockToMerge)
{
	if (blockToMerge.empty())
		return;

	if (output->Len() < 3) {
		*output = blockToMerge;
		return;
	}

	
	wxStringTokenizer token(blockToMerge.substr(1, blockToMerge.Len() - 2), L"\\", wxTOKEN_STRTOK);
	while (token.HasMoreTokens()) {
		wxString curToken = token.NextToken();
		wxString tag;
		GetTagName(curToken, &tag);
		wxRegEx re;
		if(tag == L"c" || tag == L"1c")
			re.Compile(L"(\\\\1?c&[^\\\\}]*)", wxRE_ADVANCED | wxRE_ICASE);
		else if(tag == L"fr" || tag == L"frz")
			re.Compile(L"(\\\\frz?[^\\\\}]*)", wxRE_ADVANCED | wxRE_ICASE);
		else
			re.Compile(L"(\\\\" + tag + L"[^\\\\}]*)", wxRE_ADVANCED | wxRE_ICASE);

		if (!re.IsValid()) {
			KaiLogSilent("Regex tag seeking is invalid '\\\\" + tag + L"([^\\}]*)'");
			continue;
		}//wxRegEx replace, replaceall sux
		if (re.Matches(*output)) {
			size_t start, len;
			if (re.GetMatch(&start, &len)) {
				wxString tagvalue = output->substr(start, len);
				wxString foundTag;
				GetTagName(tagvalue, &foundTag);
				wxString slashTag = L"\\" + tag;
				if(slashTag == foundTag)
					output->replace(start, len, L"\\" + curToken);
			}
		}
		else {
			output->insert((output->Len() - 1), L"\\" + curToken);
		}
	}
}

void Dialogue::GetTagName(const wxString& tagWithValue, wxString* name)
{
	if (tagWithValue.StartsWith(L"fn")) {
		*name = L"fn";
	}
	else if (tagWithValue.StartsWith(L"\\fn")) {
		*name = L"\\fn";
	}
	else {
		size_t len = tagWithValue.Len();
		if (len < 1)
			return;

		wxString delims = L"0123456789-.(&";
		*name << tagWithValue[0];
		size_t i = 1;
		while (i < len) {
			wxUniChar ch = tagWithValue[i];
			if (delims.find(ch) != -1)
				break;

			*name << ch;
			i++;
		}
	}
}

bool Dialogue::GetTaggedTextExtents(Styles* lineStyle, const wxString& text, float* _width, float* _height, 
	float* _descent, float* _extlead, bool copyStyle/* = true*/)
{
	Styles* curStyle = copyStyle? lineStyle->Copy() : lineStyle;
	const wxString& txt = text.empty() ? GetTextNoCopy() : text;
	wxString tags[] = { L"fscx", L"fscy", L"fsp", L"fs", L"fn", L"b", L"i" };
	ParseTags(tags, 7, true, txt);
	size_t parseDataSize = parseData->tags.size();
	size_t parseDataStart = 0;
	float fullWidth = 0, fullHeight = 0;
	float fullDescent = 0, fullExtlead = 0;
	for (size_t i = 0; i < parseDataSize; i++) {
		TagData* tdata = parseData->tags[i];
		if (tdata->tagName == L"plain") {
			if(i > 0)
				curStyle->SetStyleFromParseData(parseData, parseDataStart, i - 1);
			bool succeded = false;
			float width = 0, height = 0;
			float descent = 0, extlead = 0;
			if (_descent || _extlead) {
				succeded = GetLineTextExtents(tdata->value, curStyle, &width, &height, &descent, &extlead);
			}
			else {
				succeded = GetLineTextExtents(tdata->value, curStyle, &width, &height);
			}
			if (succeded) {
				fullWidth += width;
				if (fullHeight < height)
					fullHeight = height;
				if (_descent) {
					if (fullDescent < descent)
						fullDescent = descent;
				}
				if (_extlead) {
					if (fullExtlead < extlead)
						fullExtlead = extlead;
				}
			}
			else {
				ClearParse();
				if (copyStyle)
					delete curStyle;

				return false;
			}
			parseDataStart = i + 1;
		}
	}
	*_width = fullWidth;
	*_height = fullHeight;
	if (_descent)
		*_descent = fullDescent;
	if (_extlead)
		*_extlead = fullExtlead;

	ClearParse();
	if (copyStyle)
		delete curStyle;

	return true;
}

int Dialogue::SplitByChar(wxArrayString* charsTable, bool addSpaces/* = true*/, const wxString& textToSplit/* = L""*/)
{
	const wxString& txt = textToSplit.empty() ? GetTextNoCopy() : textToSplit;
	size_t len = txt.Len();
	bool inBrackets = false;
	wxString charWithTags;
	int wraps = 0;
	size_t i = 0;
	while (i < len) {
		wxUniChar ch = txt[i];
		if (i == len - 1) {
			charWithTags << ch;
			charsTable->Add(charWithTags);
			break;
		}
		if (!inBrackets && ch == L'\\') {
			wxUniChar nch = txt[i + 1];
			if (nch == L'N' || nch == L'n') {
				charsTable->Add("\\N");
				wraps++;
				i += 2;
				continue;
			}
		}
		charWithTags << ch;
		if (ch == L'{') {
			inBrackets = true;
		}
		else if (ch == L'}') {
			inBrackets = false;
		}
		else if (!inBrackets) {
			if (!addSpaces && wxIsspace(ch)) {
				charWithTags.Empty();
				i++;
				continue;
			}

			charsTable->Add(charWithTags);
			charWithTags.Empty();
		}
		i++;
	}
	return wraps;
}

int Dialogue::SplitByWord(wxArrayString* wordsTable, const wxString& textToSplit/* = L""*/)
{
	//wxString txt;
	const wxString& txtToSplit = textToSplit.empty() ? GetTextNoCopy() : textToSplit;
	//GetTextStripped(&txt, txtToSplit);
	size_t len = txtToSplit.Len();
	bool inBrackets = false;
	std::wstring words;
	size_t i = 0;
	size_t offset = 0;
	int wraps = 0;
	while (i < len) {
		wxUniChar ch = txtToSplit[i];
		if (i == len - 1) {
			words += ch;
			SplitWords(wordsTable, words, offset, txtToSplit);
			break;
		}
		if (!inBrackets && ch == L'\\') {
			wxUniChar nch = txtToSplit[i + 1];
			if (nch == L'N' || nch == L'n') {
				SplitWords(wordsTable, words, offset, txtToSplit);
				words = L"";
				wordsTable->Add(L"\\N");
				wraps++;
				offset += words.length();
				i += 2;
				continue;
			}
		}
		if (ch == L'{') {
			inBrackets = true;
		}
		else if (ch == L'}') {
			inBrackets = false;
		}
		
		words += ch;
		
		i++;
	}
	return wraps;
}

int Dialogue::SplitByWrap(wxArrayString* wrapsTable)
{
	const wxString& txt = (TextTl->empty()) ? &Text : &TextTl;
	size_t len = txt.Len();
	bool inBrackets = false;
	wxString wrapWithTags;
	size_t i = 0;
	while (i < len) {
		wxUniChar ch = txt[i];
		if (i == len - 1) {
			wrapWithTags << ch;
			wrapsTable->Add(wrapWithTags);
			break;
		}
		if (!inBrackets && ch == L'\\') {
			wxUniChar nch = txt[i + 1];
			if (nch == L'N' || nch == L'n') {
				wrapsTable->Add(wrapWithTags);
				wrapWithTags.Empty();
				i += 2;
				continue;
			}
		}
		wrapWithTags << ch;
		if (ch == L'{') {
			inBrackets = true;
		}
		else if (ch == L'}') {
			inBrackets = false;
		}
		i++;
	}
	return wrapsTable->GetCount() - 1;
}

void Dialogue::SplitWords(wxArrayString* wordsTable, std::wstring& wordsText, size_t offset, const wxString& text)
{
	using namespace boost::locale;
	boundary::wssegment_index index(boundary::word, wordsText.begin(), wordsText.end());
	boundary::wssegment_index::iterator p, e;
	size_t curWordPos = 0;
	size_t wordOffset = offset;
	bool block = false;
	for (p = index.begin(), e = index.end(); p != e; ++p) {
		wxString word = wxString(*p);
		size_t wordLen = word.length();
		if (p->rule() & boundary::word_any) {
			curWordPos += wordLen;
		}
		else {
			if (word[0] == L'{') {
				block = true;
				curWordPos += wordLen;
			}
			else if (word[0] == L'}') {
				block = false;
				curWordPos += wordLen;
			}
			else if (!block && iswspace(word[0])) {
				//add word
				if (curWordPos) {
					wordsTable->Add(text.substr(wordOffset, curWordPos));
					wordOffset += curWordPos;
					curWordPos = 0;
				}
				curWordPos = wordLen;
				//add space
				if (curWordPos) {
					wordsTable->Add(text.substr(wordOffset, curWordPos));
					wordOffset += curWordPos;
					curWordPos = 0;
				}
			}
			else
				curWordPos += wordLen;
		}
	}
	if (curWordPos && wordOffset < text.Len()) {
		wordsTable->Add(text.substr(wordOffset, curWordPos));
	}
}

bool Dialogue::FindTag(const wxString& tag, wxString* value)
{
	if (!parseData)
		return false;

	for (size_t i = 0; i < parseData->tags.size(); i++) {
		TagData* tagData = parseData->tags[i];
		if (tag == tagData->tagName) {
			*value = tagData->value;
			return true;
		}
	}
	return false;
}

void Dialogue::GetDefaultPosition(Styles* lineStyle, int an, const wxSize& subsSize, float* posx, float* posy)
{
	if (an % 3 == 2) {
		int marginL = (MarginL != 0) ? MarginL : wxAtoi(lineStyle->MarginL);
		int marginR = (MarginR != 0) ? MarginR : wxAtoi(lineStyle->MarginR);
		*posx = ((subsSize.x + marginL - marginR) / 2.f);
	}
	else if (an % 3 == 0) {
		*posx = (MarginR != 0) ? MarginR : wxAtoi(lineStyle->MarginR);
		*posx = subsSize.x - *posx;
	}
	else {
		*posx = (MarginL != 0) ? MarginL : wxAtoi(lineStyle->MarginL);
	}

	if (an < 4) {
		*posy = (MarginV != 0) ? MarginV : wxAtoi(lineStyle->MarginV);
		*posy = subsSize.y - *posy;
	}
	else if (an < 7) {
		*posy = (subsSize.y / 2.f);
	}
	else {
		*posy = (MarginV != 0) ? MarginV : wxAtoi(lineStyle->MarginV);
	}
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
				else if (Actor->Replace(L"[hidden]", emptyString)) {
					isVisible = NOT_VISIBLE;
				}
				else if (Actor->Replace(L"[visible]", emptyString)) {
					isVisible = VISIBLE_BLOCK;
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
		else if (isVisible != VISIBLE) {
			ActorWithStates.Prepend((isVisible == NOT_VISIBLE)? wxString(L"[hidden]") : 
				(isVisible == VISIBLE_BLOCK) ? wxString(L"[visible]") : emptyString);
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
	// the copy is in no file yet, so no file's row numbering changes
	dial->isVisible.Init(isVisible);
	dial->parseData = nullptr;
	return dial;
}

//Remember parse patterns need "tag1|tag2|..." without slashes.
//Remember string position is start of the value, position of tag -=tagname.len+1
ParseData* Dialogue::ParseTags(wxString *tags, size_t ntags, bool plainText, const wxString& textToParse/* = L""*/)
{
	if (parseData) {
		delete parseData; 
		parseData = nullptr;
	}
	const wxString& txt = textToParse.empty() ? GetTextNoCopy() : textToParse;
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
				TagData* newTag = new TagData((hasDrawing) ? L"pvector" : L"plain", plainStart);
				newTag->PutValue(txt.SubString(plainStart, pos - 1));
				parseData->AddData(newTag);	
			}
		}
		else if (tagsBlock && ch == L'\\'){
			pos++;
			size_t slashPos = txt.find(L'\\', pos);
			size_t bracketPos = txt.find(L'}', pos);
			size_t tagEnd = (slashPos == -1 && bracketPos == -1) ? len :
				(slashPos == -1) ? bracketPos : (bracketPos == -1) ? slashPos :
				(bracketPos < slashPos) ? bracketPos : slashPos;
			wxString tag = txt.SubString(pos, tagEnd - 1);
			if (tag.EndsWith(L')')){ tag.RemoveLast(); }
			for (size_t i = 0; i < ntags; i++){
				wxString tagName = tags[i];
				size_t tagLen = tagName.Len();
				if (tag.StartsWith(tagName) && tag.Len() > tagLen){
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

bool GetTwoValueFloat(const wxString& valuetext, float* retval, float* retval2)
{
	wxString valtext = valuetext;
	bool bracketS = valtext.StartsWith(L"(");
	bool bracketE = valtext.EndsWith(L")");
	if (bracketS || bracketE) {
		valtext = valtext.Mid(bracketS ? 1 : 0, bracketE ? valtext.length() - 2 : -1);
	}
	wxString sval;
	wxString fval = valtext.BeforeFirst(L',', &sval);
	double firstVal, secondVal;
	if (fval.ToCDouble(&firstVal) && sval.ToCDouble(&secondVal)) {
		*retval = firstVal;
		*retval2 = secondVal;
		return true;
	}
	return false;
}

bool GetMultiValueFloat(const wxString& valuetext, float* returnValuesTable, int sizeOfReturnTable, wxString* rest)
{
	if(!sizeOfReturnTable)
		return false;

	wxString valtext = valuetext;
	bool bracketS = valtext.StartsWith(L"(");
	bool bracketE = valtext.EndsWith(L")");
	if (bracketS || bracketE) {
		valtext = valtext.Mid(bracketS ? 1 : 0, bracketE ? valtext.length() - 2 : -1);
	}
	wxStringTokenizer tokenzr(valtext, L",", wxTOKEN_STRTOK);
	int counter = 0;
	while (tokenzr.HasMoreTokens()) {
		if (counter < sizeOfReturnTable) {
			double retval = 0;
			tokenzr.GetNextToken().ToCDouble(&retval);
			returnValuesTable[counter] = retval;
		}
		else {
			if (rest && tokenzr.HasMoreTokens()) {
				size_t pos = tokenzr.GetPosition();
				*rest = valtext.substr(pos);
			}
			break;
		}
		counter++;
	}
	if (counter < sizeOfReturnTable - 1) {
		for (int i = counter; i < sizeOfReturnTable; i++) {
			returnValuesTable[counter] = 0.f;
		}
	}
	return true;
}
