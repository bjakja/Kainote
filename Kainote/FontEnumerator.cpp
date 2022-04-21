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

#include "FontEnumerator.h"
#include "kainoteFrame.h"
#include "Notebook.h"
#include "wx/msw/winundef.h"
#include <wx/arrstr.h>
#include <wx/filefn.h>
#include <unicode/utf16.h>
#include <Usp10.h>

#include <ShlObj.h>

	



FontEnumerator::FontEnumerator()
{
	Fonts = new wxArrayString();
	FontsTmp = new wxArrayString();
	FilteredFonts = nullptr;
	FilteredFontsTmp = nullptr;
}

FontEnumerator::~FontEnumerator()
{
	SetEvent(eventKillSelf[0]);
	//if (hasLocalFonts)
	//
	SetEvent(eventKillSelf[1]);
	WaitForSingleObject(checkFontsThread, 2000);
	delete Fonts;
	delete FontsTmp;
	wxDELETE(FilteredFonts);
	wxDELETE(FilteredFontsTmp);
}

void FontEnumerator::StartListening(KainoteFrame* _parent)
{
	parent = _parent;
	//Here check Windows Version and save it
	//without manifest I get only version 6.2
	//it means that user have Windows 8 without SP
	for (int i = 0; i < 2; i++){
		int * threadNum = new int(i);
		checkFontsThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CheckFontsProc, threadNum, 0, 0);
		SetThreadPriority(checkFontsThread, THREAD_PRIORITY_LOWEST);
	}
}

void FontEnumerator::EnumerateFonts(bool reenumerate)
{
	wxMutexLocker lock(enumerateMutex);
	FontsTmp->Clear();
	if(FilteredFontsTmp){FilteredFontsTmp->Clear();}
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	wxStrlcpy(lf.lfFaceName, L"\0", WXSIZEOF(lf.lfFaceName));
	lf.lfPitchAndFamily = 0;
	hdc = ::GetDC(nullptr);
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)FontEnumeratorProc,
		(LPARAM)this, 0 /* reserved */);
	FontsTmp->Sort([](const wxString &i, const wxString &j){return i.CmpNoCase(j);});
	wxArrayString *tmp = FontsTmp;
	FontsTmp = Fonts;
	Fonts = tmp;
	if(FilteredFontsTmp){
		FilteredFontsTmp->Sort([](const wxString &i, const wxString &j){return i.CmpNoCase(j);});
		wxArrayString *tmp = FilteredFontsTmp;
		FilteredFontsTmp = FilteredFonts;
		FilteredFonts = tmp;
	}

	::ReleaseDC(nullptr, hdc);
	hdc = nullptr;
}

wxArrayString *FontEnumerator::GetFonts(const wxWindow *client, std::function<void()> func)
{
	wxMutexLocker lock(enumerateMutex);
	if(Fonts->size()<1){
		EnumerateFonts(false);
	}
	if(client){
		observers[client] = func;
	}
	return Fonts;
}

wxArrayString *FontEnumerator::GetFilteredFonts(const wxWindow *client, std::function<void()> func, const wxString &_filter)
{
	wxMutexLocker lock(enumerateMutex);
	filter = _filter;
	if(!FilteredFonts){
		FilteredFonts = new wxArrayString();
		FilteredFontsTmp = new wxArrayString();
		EnumerateFonts(false);
	}
	if(client && !(observers.find(client) != observers.end())){observers[client] = func;}
	return FilteredFonts;
}

void FontEnumerator::AddClient(const wxWindow *client, std::function<void()> func)
{
	if(client){
		observers[client] = func;
	}
}

void FontEnumerator::RemoveClient(const wxWindow *client)
{
	auto it = observers.find(client);
	if(it != observers.end()){
		observers.erase(it);
	}
}
//uwaga jeśli usuwamy filtry to bezwzględnie
//trzeba zmienić wskaźnik tablicy na niefiltrowane
void FontEnumerator::RemoveFilteredClient(const wxWindow *client, bool clearFiltered)
{
	auto it = observers.find(client);
	if(it != observers.end()){
		observers.erase(it);
	}
	if(clearFiltered){
		wxDELETE(FilteredFonts);
		wxDELETE(FilteredFontsTmp);
	}
}

void FontEnumerator::RefreshClientsFonts()
{
	for(auto it = observers.begin(); it!=observers.end(); it++){
		auto func = it->second;
		func();
	}
}

static int __stdcall FontEnumeratorProc(LPLOGFONT lplf, TEXTMETRIC* lptm,
	unsigned int dwStyle, long* lParam)

{
	FontEnumerator *Enum = (FontEnumerator*)lParam;
	if (lplf->lfOutPrecision == 1){
		// remove some .fon fonts but not all, modern, roman, script still there
		// these fonts not working with Vobsub nor D2D
		return true;
	}
	if(Enum->FontsTmp->Index(lplf->lfFaceName, false) == wxNOT_FOUND){
		Enum->FontsTmp->Add(lplf->lfFaceName);
	}
	if(Enum->FilteredFontsTmp && Enum->FilteredFontsTmp->Index(lplf->lfFaceName, false) == wxNOT_FOUND)
	{
		wxString missing;
		auto hfont = CreateFontIndirectW(lplf);
		SelectObject(Enum->hdc, hfont);
		if(Enum->CheckGlyphsExists(Enum->hdc, Enum->filter, missing) && missing.empty()){
			Enum->FilteredFontsTmp->Add(lplf->lfFaceName);
		}
		SelectObject(Enum->hdc, nullptr);
		DeleteObject(hfont);
	}
	return true;
}


DWORD FontEnumerator::CheckFontsProc(int *threadNum)
{
	
	FontEnum.eventKillSelf[*threadNum] = CreateEvent(0, FALSE, FALSE, 0);
	wxString fontrealpath;
	if (*threadNum == 0)
		fontrealpath = wxGetOSDirectory() + L"\\fonts\\";
	else{
		WCHAR appDataPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, nullptr, 0, appDataPath))){
			fontrealpath = wxString(appDataPath) + L"\\Microsoft\\Windows\\Fonts\\";
		}
		else{
			//delete num threads to not make memory leaks
			delete threadNum;
			return 0;
		}
	}

	HANDLE hDir = nullptr;
	hDir = FindFirstChangeNotification( fontrealpath.wc_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);// | FILE_NOTIFY_CHANGE_LAST_WRITE

	if (hDir == INVALID_HANDLE_VALUE){ 
		if (*threadNum == 0){
			//do not inform on system older than Windows 10 1909 that 
			//cannot create notification of folder that they do not have
			//without checking of system version it's impossible to check when it should be shown
			KaiLog(_("Nie można stworzyć uchwytu notyfikacji zmian folderu czcionek."));
		}
		//delete num threads to not make memory leaks
		delete threadNum;
		return 0; 
	}
	HANDLE events_to_wait[] = {
		hDir,
		FontEnum.eventKillSelf[*threadNum]
	};

	delete threadNum;

	while(1){
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait)/sizeof(HANDLE), events_to_wait, FALSE, INFINITE);
		if(wait_result == WAIT_OBJECT_0 + 0){
			Sleep(1000);
			FontEnum.EnumerateFonts(true);
			FontEnum.RefreshClientsFonts();
			Notebook::RefreshVideo(true);
			if(FindNextChangeNotification( hDir ) == 0){
				KaiLog(_("Nie można stworzyć następnego uchwytu notyfikacji zmian folderu czcionek."));
				return 0;
			}
		}else{
			break;
		}
	}

	return FindCloseChangeNotification( hDir );
}

//in Dc must be setted font
//disabled usp10 code cause it shows in some fonts lack of almost all glyphs.
bool FontEnumerator::CheckGlyphsExists(HDC dc, const wxString &textForCheck, wxString &missing)
{
	std::wstring utf16characters = textForCheck.wc_str();
	
	bool succeeded = true;
	//code taken from Aegisub, fixed by me.
	//SCRIPT_CACHE cache = nullptr;
	WORD *indices = new WORD[utf16characters.size()];

	// First try to check glyph coverage with Uniscribe, since it
	// handles non-BMP unicode characters
	//HRESULT hr = ScriptGetCMap(dc, &cache, utf16characters.data(),
		//utf16characters.size(), 0, indices);

	// Uniscribe doesn't like some types of fonts, so fall back to GDI
	//if (hr == E_HANDLE) {
		succeeded = (GetGlyphIndicesW(dc, utf16characters.data(), utf16characters.size(),
			indices, GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR);
		for (size_t i = 0; i < utf16characters.size(); ++i) {
			if (U16_IS_SURROGATE(utf16characters[i]))
				continue;
			if (indices[i] == 65535)
				missing << utf16characters[i];
		}
	//}
	//else if (hr == S_FALSE) {
	//	for (size_t i = 0; i < utf16characters.size(); ++i) {
	//		// Uniscribe doesn't report glyph indexes for non-BMP characters,
	//		// so we have to call ScriptGetCMap on each individual pair to
	//		// determine if it's the missing one
	//		if (U16_IS_SURROGATE(utf16characters[i])) {
	//			hr = ScriptGetCMap(dc, &cache, &utf16characters[i], 2, 0, &indices[i]);
	//			if (hr == S_FALSE) {
	//				missing<<utf16characters[i];
	//				missing<<utf16characters[i+1];
	//			}
	//			++i;
	//		}
	//		else if (indices[i] == 0) {
	//			missing<<utf16characters[i];
	//		}
	//	}
	//}else if(hr != S_OK){
	//	succeeded=false;
	//}
	//ScriptFreeCache(&cache);
	delete[] indices;
	return succeeded;
}

FontEnumerator FontEnum;