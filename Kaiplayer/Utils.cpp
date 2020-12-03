//  Copyright (c) 2017 - 2020, Marcin Drob

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


#include "Utils.h"
#include <ShlObj.h>
#include <wx/msw/private.h>


void SelectInFolder(const wxString & filename)
{
	CoInitialize(0);
	ITEMIDLIST *pidl = ILCreateFromPathW(filename.wc_str());
	if (pidl) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
	CoUninitialize();
}

void OpenInBrowser(const wxString &adress)
{
	WinStruct<SHELLEXECUTEINFO> sei;
	sei.lpFile = adress.c_str();
	sei.lpVerb = wxT("open");
	sei.nShow = SW_RESTORE;
	sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves
	ShellExecuteEx(&sei);
}

bool IsNumberFloat(const wxString &test) {
	bool isnumber = true;
	wxString testchars = L"0123456789.";
	for (size_t i = 0; i < test.length(); i++) {
		wxUniChar ch = test[i];
		if (testchars.Find(ch) == -1) { isnumber = false; break; }
	}
	return isnumber;
}