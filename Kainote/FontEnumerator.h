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


//
#include <wx/arrstr.h>
#include <wx/thread.h>
#include <wx/window.h>
#include <wx/arrstr.h>
#include <map>
#include <vector>
#include <functional>

//#include <windows.h>
#include <windef.h>
#include <wingdi.h>

class KainoteFrame;

class FontEnumerator
{
public:
	FontEnumerator();
	~FontEnumerator();
	void StartListening(KainoteFrame* parent);
	void EnumerateFonts(bool reenumerate = false);
	wxArrayString *GetFonts(const wxWindow *client, std::function<void()> func);
	wxArrayString *GetFilteredFonts(const wxWindow *client, std::function<void()> func, const wxString &filter);
	void AddClient(const wxWindow *client, std::function<void()> func);
	void RemoveClient(const wxWindow *client);
	void RemoveFilteredClient(const wxWindow *client, bool clearFiltered = true);
	bool CheckGlyphsExists(HDC dc, const wxString &textForCheck, wxString &missing); 
	wxArrayString* Fonts;
	wxArrayString* FontsTmp;
	wxArrayString* FilteredFonts;
	wxArrayString* FilteredFontsTmp;
	HDC hdc;
	wxString filter;
private:
	void RefreshClientsFonts();
	static int __stdcall FontEnumeratorProc(LPLOGFONT lplf, TEXTMETRIC *lptm,
		unsigned int WXUNUSED(dwStyle), long* lParam);
	static DWORD CheckFontsProc(int *threadNum);
	
	
	std::map<const wxWindow*, std::function<void()>> observers;
	KainoteFrame* parent;
	HANDLE eventKillSelf[2];
	HANDLE checkFontsThread;
	wxMutex enumerateMutex;
	
	
};

extern FontEnumerator FontEnum;

