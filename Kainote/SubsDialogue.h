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

#pragma once

#ifndef ZEROIT
#define ZEROIT(a) ((a/10)*10)
#endif

#include "config.h"
#include "SubsTime.h"
#include <wx/colour.h>
#include <vector>

//isVisible helper class
class StoreHelper {
public:
	StoreHelper(){}
	StoreHelper(const StoreHelper &sh){
		Store(sh, false);
	}
	~StoreHelper(){
		if (*deleteReference < 1 && stored){
			delete stored; stored = nullptr; delete deleteReference; deleteReference = nullptr;
		}
		else{
			(*deleteReference)--;
		}
	};
	void Store(const StoreHelper &sh, bool copy){
		//assert(sh.stored);
		if (*deleteReference < 1 && stored != sh.stored){
			delete stored; stored = nullptr;
			delete deleteReference; deleteReference = nullptr;
		}
		if (copy){
			stored = new unsigned char(*sh.stored);
			if (*stored < 1){ *stored = 1; }
			deleteReference = new size_t(0);
		}
		else{
			stored = sh.stored;
			//if (deleteReference){ delete deleteReference; }
			deleteReference = sh.deleteReference;
			(*deleteReference)++;
		}
	};
	StoreHelper &operator =(const StoreHelper &sh){
		Store(sh, false);
		return *this;
	}
	void operator =(const unsigned char value){
		//assert(stored);
		*stored = value;
	}
	bool operator ==(const unsigned char value){
		//assert(stored);
		return *stored == value;
	}
	bool operator !=(const unsigned char value){
		return *stored != value;
	}
	bool operator >(const unsigned char value){
		//assert(stored);
		return *stored > value;
	}
	bool operator <(const unsigned char value){
		//assert(stored);
		return *stored < value;
	}
	bool operator !(){
		//assert(stored);
		return !(*stored);
	}
	unsigned char &operator *(){
		//assert(stored);
		return *stored;
	};
private:
	unsigned char *stored = new unsigned char(1);
	size_t *deleteReference = new size_t(0);
};

//dialogue strings helper class
class StoreTextHelper{
public:
	StoreTextHelper(){}
	StoreTextHelper(const StoreTextHelper &sh){
		Store(sh);
	}
	/*StoreTextHelper(const wxString &txt){
		StoreText(txt);
		}
		StoreTextHelper(const char *txt){
		StoreText(txt);
		}
		StoreTextHelper(const wchar_t *txt){
		StoreText(txt);
		}*/
	~StoreTextHelper(){
		if (*deleteReference < 1){
			delete stored; stored = nullptr;
			delete deleteReference; deleteReference = nullptr;
		}
		else{
			(*deleteReference)--;
		}
	};
	void Store(const StoreTextHelper &sh){
		if (*deleteReference < 1){
			delete stored; stored = nullptr;
			delete deleteReference; deleteReference = nullptr;
		}
		else{
			(*deleteReference)--;
		}
		stored = sh.stored;
		deleteReference = sh.deleteReference;
		(*deleteReference)++;

	};
	void StoreText(const wxString &txt){
		if (*deleteReference < 1){
			delete stored; stored = nullptr;
			delete deleteReference; deleteReference = nullptr;
		}
		else{
			(*deleteReference)--;
		}
		deleteReference = new size_t(0);
		stored = new wxString(txt);
	};
	StoreTextHelper &operator =(const StoreTextHelper &sh){
		Store(sh);
		return *this;
	}

	StoreTextHelper &operator =(const wxString &newString){
		StoreText(newString);
		return *this;
	}
	operator const wxString&(){
		return *stored;
	}
	const wxString& operator &() {
		return *stored;
	}

	bool operator !=(const wxString &comptext) const{ return comptext != (*stored); };
	//bool operator !=(const char *comptext) const{ return comptext != (*stored); };
	bool operator !=(const wchar_t *comptext) const{ return comptext != (*stored); };
	bool operator ==(const wxString &comptext) const{ return comptext == (*stored); };
	//bool operator ==(const char *comptext) const{ return comptext == (*stored); };
	bool operator ==(const wchar_t *comptext) const{ return comptext == (*stored); };
	wxString &operator <<(wxString &text){
		if ((*deleteReference) > 0){
			StoreText(*stored + text);
		}
		else{ *stored << text; }
		return *stored;
	};
	/*wxString &operator <<(const char *text){
		if ((*deleteReference) > 0){
			StoreText(*stored + text);
		}
		else{ *stored << text; }
		return *stored;
	};*/
	wxString &operator <<(const wchar_t *text){
		if ((*deleteReference) > 0){
			StoreText(*stored + text);
		}
		else{ *stored << text; }
		return *stored;
	};
	//Operator copy pointer only when it was used elsewere
	wxString *operator ->(){
		if ((*deleteReference) > 0){ StoreText(*stored); }
		return stored;
	}
	wxString &CheckTlRef(StoreTextHelper &TextTl, bool condition){
		if (condition) {
			return *TextTl.Copy();
		}
		else {
			return *Copy();
		}
	}
	wxString CheckTl(const StoreTextHelper &TextTl, bool condition){
		if (condition) {
			return *TextTl.stored;
		}
		else {
			return *stored;
		}
	}
	size_t Len() const{
		return stored->length();
	}
	
	bool empty() const{
		return stored->empty();
	}
	wxString & Trim(bool fromRight = true){
		return stored->Trim(fromRight);
	}
	
	wxString *Copy(){
		if ((*deleteReference) > 0){ StoreText(*stored); }
		return stored;
	}
private:
	wxString *stored = new wxString();
	size_t *deleteReference = new size_t(0);
};

class TagData
{
public:
	TagData(const wxString &name, unsigned int startTextPos);
	void PutValue(const wxString &name, bool multiValue = false);
	void AppendValue(const wxString& name);
	wxString tagName;
	wxString value;
	bool multiValue;
	unsigned int startTextPos;
};

class ParseData
{
public:
	ParseData();
	~ParseData();
	void AddData(TagData *data);
	std::vector<TagData*> tags;
};

//states 0-2 editstate, 4 doubtful, 8 bookmark
class Dialogue
{
private:
	char State;
	void AddResetOnMDVDWraps(const wxString & prefix);
	void ReplaceSlashesToItalics();
	bool StartsWith(const wxString &text, wxUniChar ch, size_t *pos);
	bool StartsWithNoBlock(const wxString &text, wxUniChar ch, size_t *pos);
	bool EndsWith(const wxString &text, wxUniChar ch, size_t *pos);
	//StoreTextHelper Text, TextTl;
	ParseData* parseData = nullptr;
public:
	StoreTextHelper Style, Actor, Effect, Text, TextTl;
	SubsTime Start, End;
	int Layer;
	short MarginL, MarginR, MarginV;
	char Format, treeState = 0;
	bool NonDialogue, IsComment;
	StoreHelper isVisible;

	char GetState();
	//it works like XOR
	void ChangeState(char state);
	void AddState(char state);
	//Made for main dialogue state not change it with ChangeState
	void ChangeDialogueState(char state);
	bool IsDoubtful(){ return (State & 4) > 0; };
	void SetRaw(const wxString &ldial);
	void GetRaw(wxString *txt, bool tl = false, const wxString &style = emptyString, bool hideOriginalOnVideo = false);
	wxString GetCols(int cols, bool tl = false, const wxString &style = emptyString);
	void Convert(char type, const wxString &pref = emptyString);
	Dialogue *Copy(bool keepstate = false, bool copyIsVisible = true);
	//Remember parse patterns need "tag1|tag2|..." without slashes.
	//Remember string position is start of the value, position of tag -=tagname.len+1
	//vector value has tagName "pvector" to make easier to find and avoid bugs
	ParseData* ParseTags(wxString *tags, size_t n, bool plainText = false);
	void ChangeTimes(int start, int end);
	void ClearParse();
	void GetTextElement(int element, wxString *elementText, bool appendTextTL = false);
	void SetTextElement(int element, const wxString &elementText, bool appendTextTL = false);
	const wxString & GetTextNoCopy();
	//ref value after copy dialogue it can by modifiable
	wxString & GetText();
	void SetText(const wxString &text);
	Dialogue();
	Dialogue(const wxString &ldial, const wxString &txttl = emptyString);
	~Dialogue();
};

enum{
	LAYER = 1,
	START = 2,
	END = 4,
	STYLE = 8,
	ACTOR = 16,
	MARGINL = 32,
	MARGINR = 64,
	MARGINV = 128,
	EFFECT = 256,
	CPS = 512,
	TXT = 1024,
	TXTTL = 2048,
	COMMENT = 4096,
	WRAPS = 8192,
	NOT_VISIBLE = 0,
	VISIBLE,
	VISIBLE_BLOCK,
	TREE_DESCRIPTION,
	TREE_CLOSED,
	TREE_OPENED
};