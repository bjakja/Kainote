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
//#include <iostream>
//#include <vector>
#include "LanguageTool.h"
#include "LanguageToolInterface.h"

#define LTAPI extern "C" __declspec(dllexport)

LTAPI LanguageToolModule* CreateModule()
{
	// call the constructor of the actual implementation
	LanguageToolModule * module = LanguageTool::getInstance();
	// return the created function
	return module;
}

//int main()
//{
//
//        string text_to_check="być albo nie być, oto jest pytanie które dzisiaj tobie zadałem";
//
//        LanguageTool *languageTool = LanguageTool::getInstance();
//
//        if(languageTool->init()){
//
//            cout << "start" << endl;
//
//            vector<RuleMatch> errors = languageTool->checkText(text_to_check);
//
//            for(auto e : errors){
//                cout << e.FromPos << " : " << e.EndPos << " " << e.message <<  endl;
//                for(auto r : e.SuggestedReplacements){
//                    cout << "\t" << r << endl;
//                }
//
//            }
//            cout << endl;
//        }
//       cin.get();
//}
