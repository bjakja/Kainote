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

#include "LanguageToolLoader.h"



typedef LanguageToolModule *(*CreateModule)();

LanguageToolModule *ModuleLoader::GetModule()
{
	hDLL = LoadLibrary(L"LanguageTool/LanguageTool.dll");
	//hDLL = LoadLibraryExW(L"LanguageTool/LanguageTool.dll", NULL,
		//LOAD_WITH_ALTERED_SEARCH_PATH);
	if (hDLL){
		CreateModule pfnCreateModule = CreateModule(GetProcAddress(hDLL, "CreateModule"));
		if (pfnCreateModule){
			LTM = (*pfnCreateModule)();
		}
		return LTM;
	}
	return NULL;
}

ModuleLoader::~ModuleLoader(){
	if (hDLL)
		FreeLibrary(hDLL);
	if (LTM)
		delete LTM;
}