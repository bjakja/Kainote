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

#ifndef _FONT_ENUMERATOR_
#define _FONT_ENUMERATOR_

#include <wx/arrstr.h>
#include <wx/thread.h>
#include <wx/window.h>
#include <windows.h>
#include <wingdi.h>
#include <map>
#include <vector>
#include <functional>



class kainoteFrame;

class FontEnumerator
{
public:
	FontEnumerator();
	~FontEnumerator();
	void StartListening(kainoteFrame* parent);
	void EnumerateFonts(bool reenumerate = false);
	wxArrayString *GetFonts(const wxWindow *client, std::function<void()> func);
	wxArrayString *GetFilteredFonts(const wxWindow *client, std::function<void()> func, const wxString &filter);
	void AddClient(const wxWindow *client, std::function<void()> func);
	void RemoveClient(const wxWindow *client);
	void RemoveFilteredClient(const wxWindow *client, bool clearFiltered=true);
	bool CheckGlyphsExists(HDC dc, const wxString &textForCheck, wxString &missing); 
private:
	void RefreshClientsFonts();
	static int CALLBACK FontEnumeratorProc(LPLOGFONT lplf, LPTEXTMETRIC lptm,
                                  DWORD WXUNUSED(dwStyle), LPARAM lParam);
	static DWORD FontEnumerator::CheckFontsProc(void* cls);
	void RefreshVideo();
	wxArrayString *Fonts;
	wxArrayString *FontsTmp;
	wxArrayString *FilteredFonts;
	wxArrayString *FilteredFontsTmp;
	wxString filter;
	std::map<const wxWindow*, std::function<void()>> observers;
	kainoteFrame* parent;
	HANDLE eventKillSelf;
	HANDLE checkFontsThread;
	wxMutex enumerateMutex;
	HDC hdc;
};

extern FontEnumerator FontEnum;
#endif
