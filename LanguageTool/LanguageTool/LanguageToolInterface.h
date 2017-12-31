//  Copyright (c) 2017, Marcin Drob

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

#include <list>
#include <vector>
using namespace std;

class RuleMatch{
public:
	char * message = NULL;
	int FromPos;
	int EndPos;
	list<char * > SuggestedReplacements;
	RuleMatch();
	RuleMatch(const RuleMatch &rm) = delete;
	RuleMatch(const char * mes, int from, int to) : FromPos(from), EndPos(to){
		if (mes){
			size_t meslen = strlen(mes);
			message = new char[meslen + 1];
			if (message)
				strcpy(message, mes);
		}
	}
	~RuleMatch(){
		if (message){ delete[] message; message = NULL; }
		for (auto suggest : SuggestedReplacements){
			if (suggest){
				delete[] suggest;
				//suggest = NULL;
			}
		}
		SuggestedReplacements.clear();
	}
	RuleMatch *Copy(){
		return new RuleMatch(message, FromPos, EndPos);
	}
};

class LanguageToolModule{
public:
	LanguageToolModule(){};
	virtual ~LanguageToolModule(){};
	virtual void checkText(const char * text_to_check, vector<RuleMatch*> &result){};
	virtual bool init(){ return false; };
	// delete[] every language
	virtual void getLanguages(vector<char * > &languages){};
	virtual bool setLanguage(const char * language){ return false; };
	virtual void release(){};
};

