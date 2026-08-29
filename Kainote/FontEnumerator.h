//  Copyright (c) 2016 - 2026, Marcin Drob

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


#include <wx/arrstr.h>
#include <wx/thread.h>
#include <wx/window.h>
#include <wx/arrstr.h>
#include <map>
#include <vector>
#include <functional>
#include <atomic>
#include <memory>

#include <windows.h>
//#include <windef.h>
//#include <wingdi.h>


class Sink;

class FontEnumerator
{
public:
	FontEnumerator();
	~FontEnumerator();
	void StartListening();
	void EnumerateFonts(bool reenumerate = false);
	wxArrayString *GetFonts(const wxWindow *client, std::function<void()> func);
	wxArrayString *GetFilteredFonts(const wxWindow *client, std::function<void()> func, const wxString &filter);
	void AddClient(const wxWindow *client, std::function<void()> func);
	void RemoveClient(const wxWindow *client);
	void RemoveFilteredClient(const wxWindow *client, bool clearFiltered = true);
	bool CheckGlyphsExists(HDC dc, const wxString &textForCheck, wxString &missing);
	void ReloadExternalFontsToProcess(const wxString& newFontsPath, wxWindow *parent);
	bool LoadExternalFontsToProcess(const wxString &fontsPath);
	void LoadExternalFontsToProcessFromThread(const wxString& fontsPath);
	void RemoveExternalFontsFromProcess(const wxString& fontsPath);
	bool HasExternalFontsLoaded() {
		return hasExternalFontsLoaded;
	}
	void StopEnumeration();
	wxArrayString* Fonts;
	wxArrayString* FontsTmp;
	wxArrayString* FilteredFonts;
	wxArrayString* FilteredFontsTmp;
	wxArrayString ExternalFonts;
	HDC hdc;
	wxString filter;
private:
	struct UiRefreshState;
	void EnumerateFontsLocked(bool reenumerate);
	void RefreshClientsFonts();
	void RequestUiRefresh(bool refreshVideo);
#ifndef _WIN32
	void RestartLinuxFontWatcher(const wxString& externalFontsPath);
#endif
	static int __stdcall FontEnumeratorProc(const LOGFONT* lplf,
		const TEXTMETRIC* lptm, DWORD WXUNUSED(dwStyle), LPARAM lParam);
	static DWORD WINAPI CheckFontsProc(void* threadNumber);
	static DWORD WINAPI LoadExternalFontsProc(void* path);
	
	
	std::map<const wxWindow*, std::function<void()>> observers;
	//KainoteFrame* parent;
	HANDLE eventKillSelf[3] = { nullptr, nullptr, nullptr };
	HANDLE checkFontsThread[3] = { nullptr, nullptr, nullptr };
	HANDLE loadFontsThread = nullptr;
	wxMutex enumerateMutex;
	std::shared_ptr<UiRefreshState> uiRefreshState;
	std::atomic_bool shuttingDown{ false };
	bool hasExternalFontsLoaded = false;
	Sink* progress = nullptr;
#ifndef _WIN32
	// Updated only while the watcher is stopped.
	wxString linuxExternalFontsPath;
#endif
};

extern FontEnumerator FontEnum;
