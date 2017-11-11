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

#pragma once

#ifndef ZEROIT
#define ZEROIT(a) ((a/10)*10)
#endif


#include "SubsTime.h"
#include <wx/colour.h>
#include <vector>

class StoreHelper {
public:
	StoreHelper(){ stored = new unsigned char(1); }
	~StoreHelper(){
		if (*deleteReference < 1 && stored){ 
			delete stored; stored = NULL; delete deleteReference; deleteReference = NULL; 
		}
		else{ 
			(*deleteReference)--; 
		}
	};
	void Store(const StoreHelper &sh, bool copy){ 
		if (*deleteReference < 1 && stored){
			delete stored; stored = NULL;
			delete deleteReference; deleteReference = NULL;
		}
		if (copy){
			stored = new unsigned char(*sh.stored);
			deleteReference = new size_t(0);
		}
		else{
			stored = sh.stored;
			if (deleteReference){ delete deleteReference; }
			deleteReference = sh.deleteReference;
			(*deleteReference)++;
		}
	};
	/*void operator =(const StoreHelper &sh){
		stored = sh.stored;
		(*deleteReference)++;
	}*/
	void operator =(const unsigned char value){
		*stored = value;
	}
	bool operator ==(const unsigned char value){
		return *stored == value;
	}
	bool operator !=(const unsigned char value){
		return *stored != value;
	}
	bool operator >(const unsigned char value){
		return *stored > value;
	}
	bool operator <(const unsigned char value){
		return *stored < value;
	}
	bool operator !(){
		return !(*stored);
	}
	unsigned char &operator *(){ return *stored; };
private:
	unsigned char *stored = NULL;
	size_t *deleteReference = new size_t(0);
};

class TagData
{
public:
	TagData(const wxString &name, unsigned int startTextPos);
	void PutValue(const wxString &name, bool multiValue = false);
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

class Dialogue
{

public:
	wxString Style, Actor, Effect, Text, TextTl;
	STime Start, End;
	int Layer;
	short MarginL, MarginR, MarginV;
	char State, Form;
	bool NonDialogue, IsComment;
	StoreHelper isVisible;
	ParseData *parseData;

	void SetRaw(const wxString &ldial);
	void GetRaw(wxString *txt,bool tl=false,const wxString &style="");
	wxString GetCols(int cols, bool tl=false,const wxString &style="");
	void Conv(char type,const wxString &pref="");
	Dialogue *Copy(bool keepstate=false, bool copyIsVisible = true);
	void ParseTags(wxString *tags, size_t n, bool plainText = false);
	void ChangeTimes(int start, int end);
	void ClearParse();
	Dialogue();
	Dialogue(const wxString &ldial,const wxString &txttl="");
	~Dialogue();
};

enum{
	NOT_VISIBLE = 0,
	VISIBLE,
	VISIBLE_BLOCK,
};