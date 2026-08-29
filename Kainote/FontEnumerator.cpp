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

#include "FontEnumerator.h"
//#include "KainoteFrame.h"
#include "Notebook.h"
#include "ProgressDialog.h"

#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/filefn.h>
#include <condition_variable>
#include <mutex>
#include <unicode/utf16.h>
//#include <windows.h>
#include <Usp10.h>

#include <ShlObj.h>

namespace {
constexpr unsigned int REFRESH_FONT_CLIENTS = 1u;
constexpr unsigned int REFRESH_VIDEO_SUBTITLES = 2u;
}

struct FontEnumerator::UiRefreshState
{
	std::mutex mutex;
	std::condition_variable callbackFinished;
	FontEnumerator* owner = nullptr;
	unsigned int pendingRequests = 0;
	bool callbackQueued = false;
	bool callbackRunning = false;
};

FontEnumerator::FontEnumerator()
{
	Fonts = new wxArrayString();
	FontsTmp = new wxArrayString();
	FilteredFonts = nullptr;
	FilteredFontsTmp = nullptr;
	uiRefreshState = std::make_shared<UiRefreshState>();
	uiRefreshState->owner = this;
}

FontEnumerator::~FontEnumerator()
{
	StopEnumeration();
	if (uiRefreshState) {
		std::unique_lock<std::mutex> lock(uiRefreshState->mutex);
		uiRefreshState->callbackFinished.wait(lock, [this] {
			return !uiRefreshState->callbackRunning;
		});
	}
	for (auto& event : eventKillSelf) {
		if (event) {
			CloseHandle(event);
			event = nullptr;
		}
	}
	delete Fonts;
	delete FontsTmp;
	wxDELETE(FilteredFonts);
	wxDELETE(FilteredFontsTmp);
}

void FontEnumerator::StartListening()
{
	if (shuttingDown.load())
		return;
	//Here check Windows Version and save it
	//without manifest I get only version 6.2
	//it means that user have Windows 8 without SP
#ifdef _WIN32
	const int watcherCount = 2;
	for (int i = 0; i < watcherCount; i++){
		if (!eventKillSelf[i])
			eventKillSelf[i] = CreateEvent(0, FALSE, FALSE, 0);
		int * threadNum = new int(i);
		checkFontsThread[i] = CreateThread(nullptr, 0, CheckFontsProc, threadNum, 0, 0);
		if(checkFontsThread[i])
			SetThreadPriority(checkFontsThread[i], THREAD_PRIORITY_LOWEST);
	}
#else
	// Add the external root after its startup load finishes.
	RestartLinuxFontWatcher(wxString());
#endif
}

#ifndef _WIN32
void FontEnumerator::RestartLinuxFontWatcher(const wxString& externalFontsPath)
{
	if (checkFontsThread[0]) {
		if (eventKillSelf[0])
			SetEvent(eventKillSelf[0]);
		WaitForSingleObject(checkFontsThread[0], INFINITE);
		CloseHandle(checkFontsThread[0]);
		checkFontsThread[0] = nullptr;
	}
	if (shuttingDown.load())
		return;

	linuxExternalFontsPath = externalFontsPath;
	if (!eventKillSelf[0])
		eventKillSelf[0] = CreateEvent(0, FALSE, FALSE, 0);
	else
		ResetEvent(eventKillSelf[0]);
	if (!eventKillSelf[0])
		return;

	int* threadNum = new int(0);
	checkFontsThread[0] = CreateThread(
		nullptr, 0, CheckFontsProc, threadNum, 0, 0);
	if (!checkFontsThread[0]) {
		delete threadNum;
		return;
	}
	SetThreadPriority(checkFontsThread[0], THREAD_PRIORITY_LOWEST);
}
#endif

void FontEnumerator::EnumerateFonts(bool reenumerate)
{
	wxMutexLocker lock(enumerateMutex);
	EnumerateFontsLocked(reenumerate);
}

void FontEnumerator::EnumerateFontsLocked(bool reenumerate)
{
	FontsTmp->Clear();
	if(FilteredFontsTmp){FilteredFontsTmp->Clear();}
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	wxStrlcpy(lf.lfFaceName, L"\0", WXSIZEOF(lf.lfFaceName));
	lf.lfPitchAndFamily = 0;
	hdc = ::GetDC(nullptr);
	EnumFontFamiliesEx(hdc, &lf, FontEnumeratorProc,
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
	if(Fonts->size() < 1){
		EnumerateFontsLocked(false);
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
		EnumerateFontsLocked(false);
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

void FontEnumerator::RequestUiRefresh(bool refreshVideo)
{
	const std::shared_ptr<UiRefreshState> state = uiRefreshState;
	if (!state)
		return;

	std::lock_guard<std::mutex> lock(state->mutex);
	if (state->owner != this || shuttingDown.load())
		return;

	state->pendingRequests |= REFRESH_FONT_CLIENTS;
	if (refreshVideo)
		state->pendingRequests |= REFRESH_VIDEO_SUBTITLES;
	if (state->callbackQueued)
		return;

	wxApp* app = wxTheApp;
	if (!app) {
		state->pendingRequests = 0;
		return;
	}

	state->callbackQueued = true;
	app->CallAfter([state]() {
		for (;;) {
			FontEnumerator* owner = nullptr;
			unsigned int requests = 0;
			{
				std::lock_guard<std::mutex> stateLock(state->mutex);
				owner = state->owner;
				if (!owner) {
					state->pendingRequests = 0;
					state->callbackQueued = false;
					state->callbackRunning = false;
					state->callbackFinished.notify_all();
					return;
				}
				requests = state->pendingRequests;
				state->pendingRequests = 0;
				state->callbackRunning = true;
			}

			// Controls retain raw array pointers; rebuild them on the UI thread.
			if (requests & REFRESH_FONT_CLIENTS) {
				owner->EnumerateFonts(true);
				owner->RefreshClientsFonts();
			}

			bool refreshVideo = false;
			{
				std::lock_guard<std::mutex> stateLock(state->mutex);
				refreshVideo = state->owner == owner &&
					(requests & REFRESH_VIDEO_SUBTITLES) != 0;
			}
			if (refreshVideo && Notebook::GetTabs())
				Notebook::RefreshVideo(true);

			{
				std::lock_guard<std::mutex> stateLock(state->mutex);
				state->callbackRunning = false;
				state->callbackFinished.notify_all();
				if (!state->owner) {
					state->pendingRequests = 0;
					state->callbackQueued = false;
					return;
				}
				if (state->pendingRequests == 0) {
					state->callbackQueued = false;
					return;
				}
			}
		}
	});
}


int __stdcall FontEnumerator::FontEnumeratorProc(const LOGFONT* lplf,
	const TEXTMETRIC* lptm, DWORD dwStyle, LPARAM lParam)

{
	FontEnumerator *Enum = reinterpret_cast<FontEnumerator*>(lParam);
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


DWORD WINAPI FontEnumerator::CheckFontsProc(void* threadNumber)
{
	int* threadNum = static_cast<int*>(threadNumber);
	if (!FontEnum.eventKillSelf[*threadNum])
		FontEnum.eventKillSelf[*threadNum] = CreateEvent(0, FALSE, FALSE, 0);
	HANDLE eventKillSelf = FontEnum.eventKillSelf[*threadNum];
	if (!eventKillSelf) {
		delete threadNum;
		return 0;
	}
	wxString fontrealpath;
	HANDLE hDir = nullptr;
#ifndef _WIN32
	if (*threadNum != 0) {
		delete threadNum;
		return 0;
	}
	fontrealpath = FontEnum.linuxExternalFontsPath;
	auto watchedDirectories = kainote_linux_font_directories();
	// Poll missing roots instead of watching a broad ancestor.
	if (!fontrealpath.empty() && wxDirExists(fontrealpath))
		watchedDirectories.push_back(fontrealpath.ToStdWstring());
	hDir = kainote_find_first_change_notifications(
		watchedDirectories, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);
#else
	if (*threadNum == 0)
		fontrealpath = wxGetOSDirectory() + L"\\fonts\\";
	else if(*threadNum == 1) {
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
	else {
		fontrealpath = Options.GetString(EXTERNAL_FONTS_DIRECTORY);
	}

	hDir = FindFirstChangeNotification( fontrealpath.wc_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);// | FILE_NOTIFY_CHANGE_LAST_WRITE
#endif

	if (hDir == INVALID_HANDLE_VALUE){ 
#ifdef _WIN32
		if (*threadNum == 0){
			//do not inform on system older than Windows 10 1909 that 
			//cannot create notification of folder that they do not have
			//without checking of system version it's impossible to check when it should be shown
			KaiLog(_("Nie można stworzyć uchwytu notyfikacji zmian folderu czcionek."));
		}
#endif
		//delete num threads to not make memory leaks
		delete threadNum;
		return 0; 
	}
	HANDLE events_to_wait[] = {
		hDir,
		eventKillSelf
	};
#ifndef _WIN32
	bool retryMissingExternalDirectory =
		!fontrealpath.empty() && !wxDirExists(fontrealpath);
#endif

	while(1){
		DWORD waitTimeout = INFINITE;
#ifndef _WIN32
		if (retryMissingExternalDirectory)
			waitTimeout = 1000;
#endif
		DWORD wait_result = WaitForMultipleObjects(
			sizeof(events_to_wait) / sizeof(HANDLE), events_to_wait, FALSE, waitTimeout);
#ifndef _WIN32
		if (wait_result == WAIT_TIMEOUT) {
			if (!fontrealpath.empty() && wxDirExists(fontrealpath)) {
				auto watchedDirectories = kainote_linux_font_directories();
				watchedDirectories.push_back(fontrealpath.ToStdWstring());
				HANDLE replacement = kainote_find_first_change_notifications(
					watchedDirectories, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);
				if (replacement != INVALID_HANDLE_VALUE) {
					FindCloseChangeNotification(hDir);
					hDir = replacement;
					events_to_wait[0] = hDir;
					retryMissingExternalDirectory = false;
					// No later event is guaranteed, so snapshot immediately.
					ProgressSinkSilent* progr = new ProgressSinkSilent(
						_("Ładowanie czcionek zewnętrznych"));
					FontEnum.progress = progr;
					FontEnum.RemoveExternalFontsFromProcess(fontrealpath);
					FontEnum.LoadExternalFontsToProcess(fontrealpath);
					progr->EndModal();
					delete progr;
					FontEnum.progress = nullptr;
					FontEnum.RequestUiRefresh(true);
				}
			}
			continue;
		}
#endif
		if(wait_result == WAIT_OBJECT_0 + 0){
			Sleep(1000);
			if (WaitForSingleObject(eventKillSelf, 0) == WAIT_OBJECT_0)
				break;
			bool reloadExternalFonts = false;
#ifdef _WIN32
			reloadExternalFonts = *threadNum == 2 && !fontrealpath.empty();
#else
			reloadExternalFonts = *threadNum == 0 && !fontrealpath.empty();
#endif
			if (reloadExternalFonts) {
				ProgressSinkSilent* progr = new ProgressSinkSilent(_("Ładowanie czcionek zewnętrznych"));
				FontEnum.progress = progr;
				FontEnum.RemoveExternalFontsFromProcess(fontrealpath);
				if (wxDirExists(fontrealpath))
					FontEnum.LoadExternalFontsToProcess(fontrealpath);
				progr->EndModal();
				delete progr;
				FontEnum.progress = nullptr;
			}
			FontEnum.RequestUiRefresh(true);
#ifndef _WIN32
			retryMissingExternalDirectory =
				!fontrealpath.empty() && !wxDirExists(fontrealpath);
#endif
			if(FindNextChangeNotification( hDir ) == 0){
				KaiLog(_("Nie można stworzyć następnego uchwytu notyfikacji zmian folderu czcionek."));
				FindCloseChangeNotification(hDir);
				delete threadNum;
				return 0;
			}
		}else{
			break;
		}
	}
	delete threadNum;
	return FindCloseChangeNotification( hDir );
}

DWORD WINAPI FontEnumerator::LoadExternalFontsProc(void* path)
{
	wxString* fontpathArgument = (wxString*)path;
	wxString fontpath = *fontpathArgument;
	delete fontpathArgument;
	FontEnum.LoadExternalFontsToProcess(fontpath);
	if (!FontEnum.progress)
		return 0;

	FontEnum.progress->EndModal();
	delete FontEnum.progress;
	FontEnum.progress = nullptr;
	FontEnum.RequestUiRefresh(false);
	//Notebook::RefreshVideo(true);

#ifndef _WIN32
	FontEnum.RestartLinuxFontWatcher(fontpath);
#else
	int* threadNum = new int(2);
	if (!FontEnum.eventKillSelf[2])
		FontEnum.eventKillSelf[2] = CreateEvent(0, FALSE, FALSE, 0);
	FontEnum.checkFontsThread[2] = CreateThread(nullptr, 0, CheckFontsProc, threadNum, 0, 0);
	if (FontEnum.checkFontsThread[2])
		SetThreadPriority(FontEnum.checkFontsThread[2], THREAD_PRIORITY_LOWEST);
#endif

	return 0;
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

void FontEnumerator::ReloadExternalFontsToProcess(const wxString& newFontsPath, wxWindow* parent)
{
	// Serialize settings reload with the startup loader.
	if (loadFontsThread) {
		WaitForSingleObject(loadFontsThread, INFINITE);
		CloseHandle(loadFontsThread);
		loadFontsThread = nullptr;
	}
#ifndef _WIN32
	RestartLinuxFontWatcher(wxString());
#endif
	if (progress) {
		Sink* progresscopy = progress;
		progress = nullptr;
		delete progresscopy;
		Sleep(1000);
		hasExternalFontsLoaded = true;
	}
	ProgressSink *progr = new ProgressSink(parent, _("Usuwanie czcionek z zewnętrznego folderu"));
	progress = progr;
	progr->SetAndRunTask([&]() {
		if (hasExternalFontsLoaded) {
			wxString path = Options.GetString(EXTERNAL_FONTS_DIRECTORY);
			RemoveExternalFontsFromProcess(path);
		}
		if (wxDirExists(newFontsPath)) {
			progress->Title(_("Wczytywanie czcionek z zewnętrznego folderu"));
			LoadExternalFontsToProcess(newFontsPath);
		}
		return 1;
		});
	progr->ShowDialog();
	progr->Wait();
	progr->EndModal();
	EnumerateFonts(true);
	RefreshClientsFonts();
	delete progress;
	progress = nullptr;
#ifndef _WIN32
	// Keep polling until a missing external root appears.
	RestartLinuxFontWatcher(newFontsPath);
#endif
}

bool FontEnumerator::LoadExternalFontsToProcess(const wxString& fontsPath)
{
	wxString seekpath = fontsPath + L"*";

	WIN32_FIND_DATAW data;
	HANDLE h = FindFirstFileW(seekpath.wc_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
	{
		KaiLog(_("Nie można wczytać zewnętrznego katalogu czcionek"));
		return false;
	}
	int fontAdded = 0;
	wxArrayString discoveredFonts;
	do {
		wxString file = wxString(data.cFileName);
		if (file == L"." || file == L".." || data.nFileSizeLow == 0) { continue; }
		wxString ext = file.AfterLast(L'.').Lower();
		if (ext == L"ttf" || ext == L"otf" || ext == L"ttc" || ext == L"pfb"/* || ext == L"pfm"*/) {
			discoveredFonts.Add(file);
		}
	} while (FindNextFileW(h, &data));
	FindClose(h);
	size_t size = discoveredFonts.Count();
	for (size_t i = 0; i < size; i++) {
		const wxString& file = discoveredFonts[i];
		if (ExternalFonts.Index(file) != wxNOT_FOUND)
			continue;
		wxString pathAndFile = fontsPath + file;
		int addResult = AddFontResourceExW(pathAndFile.wc_str(), FR_PRIVATE, nullptr);
		if (addResult == 0)
			KaiLogSilent(L"Cannot add external font file " + file + L".\n");
		else {
			fontAdded += addResult;
			ExternalFonts.Add(file);
			if (progress)
				progress->Progress(((i + 1) / (float)size) * 100);
		}
	}
	//KaiLogSilent(L"Loaded external font files " + std::to_wstring(fontAdded) + L".\n");
	if (fontAdded)
		hasExternalFontsLoaded = true;

	return fontAdded > 0;
}

void FontEnumerator::LoadExternalFontsToProcessFromThread(const wxString& fontsPath)
{
	if (shuttingDown.load())
		return;
	if (loadFontsThread) {
		WaitForSingleObject(loadFontsThread, INFINITE);
		CloseHandle(loadFontsThread);
		loadFontsThread = nullptr;
	}
	if (shuttingDown.load())
		return;
	ProgressSinkSilent* progr = new ProgressSinkSilent(_("Ładowanie czcionek zewnętrznych"));
	progress = progr;
	wxString* ppath = new wxString(fontsPath);
	loadFontsThread = CreateThread(nullptr, 0, LoadExternalFontsProc, ppath, 0, 0);
	if (!loadFontsThread) {
		delete ppath;
		if (progress == progr)
			progress = nullptr;
		delete progr;
	}
}

void FontEnumerator::RemoveExternalFontsFromProcess(const wxString& fontsPath)
{
	int fontRemoved = 0;
	size_t size = ExternalFonts.Count();
	wxArrayString remainingFonts;
	for (size_t i = 0; i < size; i++) {
		wxString pathAndFile = fontsPath + ExternalFonts[i];
		if (RemoveFontResourceExW(pathAndFile.wc_str(), FR_PRIVATE, nullptr)) {
			fontRemoved++;
			if (progress)
				progress->Progress(((i + 1) / (float)size) * 100);
		}
		else
			remainingFonts.Add(ExternalFonts[i]);
	}
		
	//KaiLogSilent(L"Removed " + std::to_wstring(fontRemoved) + L" fonts.\n");
	ExternalFonts = remainingFonts;
	hasExternalFontsLoaded = !ExternalFonts.empty();

	return;
}

void FontEnumerator::StopEnumeration()
{
	shuttingDown.store(true);
	if (uiRefreshState) {
		std::lock_guard<std::mutex> lock(uiRefreshState->mutex);
		uiRefreshState->owner = nullptr;
		uiRefreshState->pendingRequests = 0;
	}
	for (auto& event : eventKillSelf) {
		if (event)
			SetEvent(event);
	}
	// Prevent the loader from restarting the watcher during teardown.
	if (loadFontsThread) {
		WaitForSingleObject(loadFontsThread, INFINITE);
		CloseHandle(loadFontsThread);
		loadFontsThread = nullptr;
	}
	for (auto& event : eventKillSelf) {
		if (event)
			SetEvent(event);
	}
	for (auto& thread : checkFontsThread) {
		if (thread) {
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			thread = nullptr;
		}
	}
	Sink* tmpProgress = progress;
	progress = nullptr;
	delete tmpProgress;
}

FontEnumerator FontEnum;
