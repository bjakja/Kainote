//  Copyright (c) 2017, £ukasz G¹sowski

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

#include <iostream>
#include <list>
#include <vector>
#include <jni.h>
#include "LanguageToolInterface.h"

using namespace std;

//class RuleMatch{
//    public:
//    string message;
//    int FromPos;
//    int EndPos;
//    list<string> SuggestedReplacements;
//    RuleMatch();
//    RuleMatch(string mes, int from, int to): message(mes), FromPos(from), EndPos(to){}
//};

class LanguageTool : public LanguageToolModule{
    private:
        JavaVM *jvm;                      // Pointer to the JVM (Java Virtual Machine)
        JNIEnv *env;
		LanguageTool() : LanguageToolModule(){};
        static LanguageTool* instance;

        jclass class_LanguageToolFacade, class_RuleMatch, class_List;
		jmethodID LanguageToolFacade_checkText, List_toArray, RuleMatch_getToPos, RuleMatch_getFromPos, LanguageToolFacade_setLanguage, LanguageToolFacade_getLanguages;
        jfieldID RuleMatch_message, RuleMatch_replacements;

    public:
        LanguageTool(LanguageTool const&) = delete;
        void operator=(LanguageTool const&) = delete;
        static LanguageTool* getInstance();
		void checkText(const char * text_to_check, vector<RuleMatch> &result);
        bool init();
		void getLanguages(vector<char *> &languages);
		bool setLanguage(const char * language);
		void release();
        ~LanguageTool();
};
