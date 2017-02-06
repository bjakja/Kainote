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

#include "FontEnumerator.h"
#include "kainoteMain.h"
#include <wx/log.h>
#include <wx/filefn.h>

FontEnumerator::FontEnumerator(kainoteFrame* _parent)
{
	parent = _parent;
	Fonts = new wxArrayString();
	checkFontsThread = CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)CheckFontsProc, this, 0, 0);
	SetThreadPriority(checkFontsThread,THREAD_PRIORITY_LOWEST);
}

FontEnumerator::~FontEnumerator()
{
	SetEvent(eventKillSelf);
	WaitForSingleObject(checkFontsThread, 2000);
	delete Fonts;
}

const wxArrayString *FontEnumerator::EnumerateFonts(bool reenumerate)
{
	if(Fonts->size()<1 || reenumerate){
		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		wxStrlcpy(lf.lfFaceName, L"\0", WXSIZEOF(lf.lfFaceName));
		lf.lfPitchAndFamily = 0;
		HDC hDC = ::GetDC(NULL);
		EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)FontEnumeratorProc,
			(LPARAM)this, 0 /* reserved */);
		Fonts->Sort([](const wxString &i, const wxString &j){return i.CmpNoCase(j);});
	}
	return Fonts;
}

int CALLBACK FontEnumerator::FontEnumeratorProc(LPLOGFONT lplf, LPTEXTMETRIC lptm,
												DWORD WXUNUSED(dwStyle), LPARAM lParam)
{
	FontEnumerator *Enum = (FontEnumerator*)lParam;
	if(Enum->Fonts->Index(lplf->lfFaceName,false)==wxNOT_FOUND){
		Enum->Fonts->Add(lplf->lfFaceName);
	}
	return true;
}

void FontEnumerator::RefreshVideo()
{
	parent->GetTab()->Video->Render();
}

DWORD FontEnumerator::CheckFontsProc(void* fontEnum)
{
	FontEnumerator *fe=(FontEnumerator*)fontEnum;
	if(!fontEnum){wxLogMessage(_("Brak wskaŸnika klasy magazynu stylów.")); return 0;}

	HANDLE hDir  = NULL; 
	fe->eventKillSelf = CreateEvent(0, FALSE, FALSE, 0);
	wxString fontrealpath=wxGetOSDirectory() + "\\fonts\\";

	hDir = FindFirstChangeNotification( fontrealpath.wc_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);// | FILE_NOTIFY_CHANGE_LAST_WRITE

	if(hDir == INVALID_HANDLE_VALUE ){wxLogMessage(_("Nie mo¿na stworzyæ uchwytu notyfikacji zmian folderu czcionek.")); return 0;}
	HANDLE events_to_wait[] = {
		hDir,
		fe->eventKillSelf
	};
	while(1){
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait)/sizeof(HANDLE), events_to_wait, FALSE, INFINITE);
		if(wait_result == WAIT_OBJECT_0+0){
			fe->EnumerateFonts(true);
			fe->RefreshVideo();
			if( FindNextChangeNotification( hDir ) == 0 ){
				wxLogStatus(_("Nie mo¿na stworzyæ nastêpnego uchwytu notyfikacji zmian folderu czcionek."));
				return 0;
			}
		}else {
			break;
		}
	}

	return FindCloseChangeNotification( hDir );
}

FontEnumerator FontEnum;