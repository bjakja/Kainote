/***************************************************************
 * Copyright (c) 2012 - 2026, Marcin Drob
 * Name:      kainoteApp.cpp
 * Purpose:   Subtitles editor and player
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * License:
 * Kainote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Kainote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Kainote.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/


 
#include "kainoteApp.h"
#include "Menu.h"
#include "ListControls.h"
#include "OpennWrite.h"
#include "Hotkeys.h"
#include "KaiMessageBox.h"
#include "FontEnumerator.h"
#include "SubsGrid.h"
#include "VideoBox.h"
#include "Notebook.h"
#include <wx/image.h>
#include <wx/ipc.h>
#include <wx/utils.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/log.h>
#include <wx/weakref.h>
#ifndef _WIN32
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <wx/tokenzr.h>
#include <wx/tooltip.h>
#endif
#include "LogHandler.h"

#include "UtilsWindows.h"
#include <versionhelpers.h>
//#include <boost/stacktrace.hpp>
#ifdef _WIN32
#include <dbghelp.h>
#endif

namespace {
#ifndef _WIN32
wxString JoinOpenPaths(const wxArrayString& openPaths)
{
	wxString joined;
	for (size_t i = 0; i < openPaths.GetCount(); ++i){
		if (i){ joined += L"|"; }
		joined += openPaths[i];
	}
	return joined;
}

void QueueOpenPaths(const wxString& packedPaths)
{
	kainoteApp *Kai = (kainoteApp *)wxTheApp;
	if (!Kai){ return; }
	wxStringTokenizer tkn(packedPaths, L"|");
	while (tkn.HasMoreTokens()){
		wxString path = tkn.NextToken();
		if (!path.empty()){
			Kai->paths.Add(path);
		}
	}
	if (!Kai->paths.empty()){
		Kai->openTimer.Start(400, true);
	}
}

const wxString KainoteIpcTopic()
{
	return L"KainoteOpenFiles";
}

wxString KainoteIpcServiceName()
{
	wxString user = wxGetUserId();
	if (user.empty()){
		user = L"user";
	}
	wxString safeUser;
	for (size_t i = 0; i < user.length(); ++i){
		const wxUniChar ch = user[i];
		if ((ch >= L'0' && ch <= L'9') || (ch >= L'A' && ch <= L'Z') ||
			(ch >= L'a' && ch <= L'z') || ch == L'_' || ch == L'-'){
			safeUser += ch;
		}
		else{
			safeUser += L'_';
		}
	}
	return wxFileName::GetTempDir() + wxFileName::GetPathSeparator() + L"kainote-" + safeUser + L".ipc";
}

void RemoveKainoteIpcSocket()
{
	wxLogNull noLog;
	wxRemoveFile(KainoteIpcServiceName());
}

class KainoteIpcConnection : public wxConnection
{
public:
	bool OnExec(const wxString& topic, const wxString& data) override
	{
		if (topic != KainoteIpcTopic()){
			return false;
		}
		QueueOpenPaths(data);
		KainoteFrame *frame = KainoteFrame::Get();
		if (frame){
			if (frame->IsIconized()){
				frame->Iconize(false);
			}
			frame->Raise();
		}
		return true;
	}
};

class KainoteIpcServer : public wxServer
{
public:
	wxConnectionBase *OnAcceptConnection(const wxString& topic) override
	{
		if (topic == KainoteIpcTopic()){
			return new KainoteIpcConnection();
		}
		return nullptr;
	}
};

bool SendOpenPathsToRunningInstance(const wxString& packedPaths)
{
	if (packedPaths.empty()){
		return false;
	}
	wxClient client;
	for (int attempt = 0; attempt < 100; ++attempt){
		wxConnectionBase *connection = client.MakeConnection(L"localhost", KainoteIpcServiceName(), KainoteIpcTopic());
		if (connection){
			const bool sent = connection->Execute(packedPaths);
			connection->Disconnect();
			return sent;
		}
		wxMilliSleep(40);
	}
	return false;
}
#endif
}

#ifdef _WIN32
typedef enum MONITOR_DPI_TYPE {
	MDT_EFFECTIVE_DPI = 0,
	MDT_ANGULAR_DPI = 1,
	MDT_RAW_DPI = 2,
	MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

STDAPI GetDpiForMonitor(
	_In_ HMONITOR hmonitor,
	_In_ MONITOR_DPI_TYPE dpiType,
	_Out_ UINT* dpiX,
	_Out_ UINT* dpiY);
#endif


#ifdef _WIN32
LONG __stdcall MyCustomFilter(EXCEPTION_POINTERS* pep)
{
	wxStandardPathsBase& paths = wxStandardPaths::Get();
	wxString exePath = wxFileName(paths.GetExecutablePath()).GetPath() + L"/MiniDump.dmp";
	HANDLE hFile = CreateFileW(exePath.wc_str(), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		// Create the minidump 

		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pep;
		mdei.ClientPointers = FALSE;

		MINIDUMP_TYPE mdt = MiniDumpNormal;

		BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, 0);

		/*if (!rv)
			_tprintf(_T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError());
		else
			_tprintf(_T("Minidump created.\n"));*/

		// Close the file 

		CloseHandle(hFile);

	}
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif



void kainoteApp::OnOutofMemory()
{
	TabPanel *tab = Notebook::GetTab();

	if (tab->grid->maxx() > 3){
		tab->grid->RemoveFirst(2);
		KaiLog(_("Zabrakło pamięci RAM, usunięto część historii"));
		return;
	}
	else if (Notebook::GetTabs()->Size() > 1){
		for (size_t i = 0; i < Notebook::GetTabs()->Size(); i++)
		{
			if (i != Notebook::GetTabs()->GetSelection()){
				if (Notebook::GetTabs()->Page(i)->grid->maxx()>3){
					Notebook::GetTabs()->Page(i)->grid->RemoveFirst(2);
					KaiLog(_("Zabrakło pamięci RAM, usunięto część historii"));
					return;
				}
			}
		}
	}

	std::exit(1);
}

IMPLEMENT_APP(kainoteApp);

bool kainoteApp::OnInit()
{

#ifndef _WIN32
	m_ipcServer = nullptr;
#endif
	m_checker = new wxSingleInstanceChecker();

	//bool wxsOK = true;

	if (!m_checker->IsAnotherRunning())
	{

#ifndef _WIN32
		RemoveKainoteIpcSocket();
		m_ipcServer = new KainoteIpcServer();
		if (!m_ipcServer->Create(KainoteIpcServiceName())){
			delete m_ipcServer;
			m_ipcServer = nullptr;
			KaiLogSilent(L"Cannot create Kainote IPC server for opening files in a running instance");
		}
#endif

#ifndef _WIN32
		// Keep the C character locale UTF-8 on Unix.  Some wx helpers still
		// convert narrow source literals through the current C locale; if the
		// process is left in the default "C" locale after wxLocale::Init() fails
		// for an ungenerated language such as th_TH.UTF-8, non-ASCII Polish
		// source strings can convert to empty wxStrings and option labels vanish.
		setlocale(LC_CTYPE, "");
		const char* ctypeLocale = setlocale(LC_CTYPE, nullptr);
		if (!ctypeLocale || (!std::strstr(ctypeLocale, "UTF-8") && !std::strstr(ctypeLocale, "utf8"))) {
			if (!setlocale(LC_CTYPE, "C.UTF-8"))
				setlocale(LC_CTYPE, "en_US.UTF-8");
		}

		// Point portable Linux runs at a usable gdk-pixbuf loader cache.
		if (!std::getenv("GDK_PIXBUF_MODULE_FILE")){
			const wxString pixbufCaches[] = {
				L"/usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/2.10.0/loaders.cache",
				L"/usr/lib64/gdk-pixbuf-2.0/2.10.0/loaders.cache",
				L"/usr/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache"
			};
			for (const auto& pixbufCache : pixbufCaches){
				if (wxFileExists(pixbufCache)){
					setenv("GDK_PIXBUF_MODULE_FILE", pixbufCache.mb_str().data(), 0);
					wxString pixbufModuleDir = pixbufCache.BeforeLast(wxFileName::GetPathSeparator()) + wxFileName::GetPathSeparator() + L"loaders";
					if (!std::getenv("GDK_PIXBUF_MODULEDIR") && wxDirExists(pixbufModuleDir))
						setenv("GDK_PIXBUF_MODULEDIR", pixbufModuleDir.mb_str().data(), 0);
					break;
				}
			}
		}
		if (!std::getenv("XDG_DATA_DIRS"))
			setenv("XDG_DATA_DIRS", "/usr/local/share:/usr/share", 0);

		// Disable native wxGTK tooltip popups; Kainote uses tooltip text for status help.
		wxToolTip::Enable(false);
#endif

		wxImage::AddHandler(new wxPNGHandler);
		wxImage::AddHandler(new wxICOHandler);
		wxImage::AddHandler(new wxCURHandler);

		//if (wxsOK)
		//{
		//do not load here float options cause here is polish or another default locale with ',' instead of '.'
		//wxHandleFatalExceptions(true);
		//0 - failed, 1 - succeeded, 2 - no config
		int isGood = Options.LoadOptions();
		if (!isGood){ KaiMessageBox(_("Nie udało się wczytać opcji.\nDziałanie programu zostanie zakończone."), _("Uwaga")); return false; }
		//0x0415 	Polish (pl) 	0x15 	LANG_POLISH 	Poland (PL) 	0x01 	SUBLANG_POLISH_POLAND
		if (isGood == 2 && GetSystemDefaultUILanguage() != 0x415){
			//what a lame language system, I need to change it.
			Options.SetString(PROGRAM_LANGUAGE, L"en");
			Options.SetString(DICTIONARY_LANGUAGE, L"en_US");
		}
			
		locale = nullptr;
		wxString lang = Options.GetString(PROGRAM_LANGUAGE);
		if (lang == L"0"){
			lang = emptyString; Options.SetString(PROGRAM_LANGUAGE, lang);
		}
		if (lang == L"1"){
			lang = L"en"; Options.SetString(PROGRAM_LANGUAGE, lang);
		}
		if (lang != emptyString && lang != L"pl"){
			locale = new wxLocale();
			const  wxLanguageInfo * li = locale->FindLanguageInfo(lang);
			if (!li){
				KaiMessageBox(L"Cannot find language, language change failed");
			}
			else{
#ifdef _WIN32
				if (!locale->Init(li->Language, wxLOCALE_DONT_LOAD_DEFAULT)){
					KaiMessageBox(L"wxLocale cannot initialize, language change failed");
				}
#else
				{
					// On Unix, wxLocale::Init() may fail when the requested OS locale
					// (for example th_TH.UTF-8) has not been generated.  The message
					// catalogs can still be loaded after Init() sets wxLocale's internal
					// language state, so silence wxWidgets' warning dialog and continue.
					wxLogNull suppressLocaleWarning;
					locale->Init(li->Language, wxLOCALE_DONT_LOAD_DEFAULT);
				}
#endif
				wxString localePath = Options.pathfull + wxFileName::GetPathSeparator() + L"Locale" + wxFileName::GetPathSeparator();
#ifndef _WIN32
				if (!wxDirExists(localePath)){
					wxString sourceLocalePath = Options.pathfull.BeforeLast(wxFileName::GetPathSeparator()) + wxFileName::GetPathSeparator() + L"Locale" + wxFileName::GetPathSeparator();
					if (wxDirExists(sourceLocalePath))
						localePath = sourceLocalePath;
				}
#endif
				locale->AddCatalogLookupPathPrefix(localePath);
				if (!locale->AddCatalog(lang, wxLANGUAGE_POLISH, L"UTF-8") &&
					!locale->AddCatalog(li->CanonicalName, wxLANGUAGE_POLISH, L"UTF-8")){
#ifdef _WIN32
					KaiMessageBox(L"Cannot find translation, language change failed");
#endif
				}
			}
		}
		
#ifdef _WIN32
		SetUnhandledExceptionFilter(MyCustomFilter);
#endif

		//on x64 it makes not working unicode toupper tolower conversion
		//setlocale(LC_CTYPE, "C");
		//locale numbers changes here cause of it is set with wxlocale, I have to change it back
		setlocale(LC_NUMERIC, "C");

		if (!Hkeys.LoadHkeys()){
			KaiMessageBox(_("Nie udało się wczytać skrótów.\nDziałanie programu zostanie zakończone."), _("Uwaga"));
			wxDELETE(locale); return false;
		}

		for (int i = 1; i < argc; i++) { paths.Add(argv[i]); }

		int posx, posy, sizex, sizey, msizex, msizey, sposx, sposy;
		Options.GetCoords(WINDOW_POSITION, &posx, &posy);
		Options.GetCoords(WINDOW_SIZE, &sizex, &sizey);
		Options.GetCoords(MONITOR_SIZE, &msizex, &msizey);
		Options.GetCoords(STYLE_MANAGER_POSITION, &sposx, &sposy);
		//wxRect(posx, posy, sizex, sizey)
#ifdef _WIN32
		wxPoint posOnScreen = wxGetMousePosition();
#else
		// Avoid wxGetMousePosition() on Wayland before a keyboard-capable seat exists.
		wxPoint posOnScreen(posx, posy);
#endif
		if (msizex && msizey) {
			if (IsWindows8OrGreater()) {
#ifdef _WIN32
				HMONITOR hmon = MonitorFromPoint(POINT(posOnScreen.x, posOnScreen.y), MONITOR_DEFAULTTONEAREST);
				unsigned int dpiy = 96, dpix = 96;
				HRESULT hr = GetDpiForMonitor(hmon, MDT_EFFECTIVE_DPI, &dpix, &dpiy);
				if (hr == S_OK)
					Options.FontsRescale(dpiy);
#endif
			}
			wxRect rt = GetMonitorRect1(0/*-1*/, nullptr, wxRect(posOnScreen.x, posOnScreen.y, 1, 1));
			if (rt.width != msizex || rt.height != msizey) {
				int mposx, mposy, vsizex, vsizey;
				Options.GetCoords(MONITOR_POSITION, &mposx, &mposy);
				Options.GetCoords(VIDEO_WINDOW_SIZE, &vsizex, &vsizey);
				int audioHeight = Options.GetInt(AUDIO_BOX_HEIGHT);
				KaiLogSilent(wxString::Format(L"Audio Height %d", audioHeight));
				float scalex = (float)rt.width / (float)msizex;
				float scaley = (float)rt.height / (float)msizey;
				//program position
				posx -= mposx;
				posy -= mposy;
				posx *= scalex;
				posy *= scaley;
				posx += rt.x;
				posy += rt.y;
				//style manager
				sposx -= mposx;
				sposy -= mposy;
				sposx *= scalex;
				sposy *= scaley;
				sposx += rt.x;
				sposy += rt.y;
				//program size
				sizex *= scalex;
				sizey *= scaley;
				//video size
				vsizex *= scalex;
				vsizey *= scaley;
				audioHeight *= scaley;
				Options.SetCoords(VIDEO_WINDOW_SIZE, vsizex, vsizey);
				Options.SetCoords(MONITOR_SIZE, rt.width, rt.height);
				Options.SetCoords(MONITOR_POSITION, rt.x, rt.y);
				Options.SetInt(AUDIO_BOX_HEIGHT, audioHeight);
				KaiLogSilent(wxString::Format(L"Audio Height scaled %d", audioHeight));
			}
			if (sizex > rt.width) {
				sizex = rt.width;
			}
			if (sizey > rt.height) {
				sizey = rt.height - 100;
			}
			//check position of spellchecker
			if (!rt.Contains(wxRect(sposx, sposy, 400, 800))) {
				sposx = rt.x + ((float)(rt.width - 400) / 2.f);
				sposy = rt.y + ((float)(rt.height - 800) / 2.f);
			}
			//check position of main window
			if (!rt.Contains(wxRect(posx, posy, sizex, sizey))) {
				posx = rt.x + ((float)(rt.width - sizex) / 2.f),
				posy = rt.y + ((float)(rt.height - sizey) / 2.f);
			}
		}
		if (sizex < 1000 || sizey < 700) {
			sizex = 1000; sizey = 700;
		}

		Options.SetCoords(WINDOW_POSITION, posx, posy);
		Options.SetCoords(WINDOW_SIZE, sizex, sizey);
		Options.SetCoords(STYLE_MANAGER_POSITION, sposx, sposy);

		Frame = nullptr;
		Frame = new KainoteFrame(wxPoint(posx, posy), wxSize(sizex, sizey));

		//handler for out of memory in new
		std::set_new_handler(OnOutofMemory);
		//start listen to font folders notifications
		FontEnum.StartListening();
		
		if (isGood == 2)
			Frame->CenterOnScreen();

		bool opevent = false;
		bool hasPaths = paths.GetCount() > 0;
		if (hasPaths){
			if (Options.GetBool(VIDEO_FULL_SCREEN_ON_START)){
				Frame->OpenFiles(paths, false, true);
				Frame->GetTab()->video->Layout();
			}
			else{
				opevent = true;

			}
		}


		Frame->Show();
		SetTopWindow(Frame);
		openTimer.SetOwner(this, 1199);
		Bind(wxEVT_TIMER, &kainoteApp::OnOpen, this, 1199);
		if (opevent){
			openTimer.Start(500, true);
		}
		bool loadCrashSession = false;
#if _DEBUG
		bool loadSession = true;
#else
		int session = Options.GetInt(LAST_SESSION_CONFIG);
		bool loadSession = (session == 2 || Options.HasCrashed()) && !hasPaths;
		if (session == 1 && !hasPaths){
			if (KaiMessageBox(_("Wczytać poprzednią sesję?"), _("Pytanie"), wxYES_NO, Frame) == wxYES){
				loadSession = true;
			}
		}
		//Check if program was bad close or crashed
		if (!hasPaths && !loadSession && Notebook::CheckLastSession() == 2) {
			if (KaiMessageBox(_("Program się skraszował albo został zamknięty w niewłaściwy sposób,\nwczytać poprzednią sesję wraz z najnowszymi napisami z autozapisu?"), _("Pytanie"), wxYES_NO, Frame) == wxYES) {
				loadCrashSession = loadSession = true;
			}
		}
#endif
		if (loadSession){
			debugtimer.SetOwner(this, 2299);
			debugtimer.Start(100, true);
			Bind(wxEVT_TIMER, [=, this](wxTimerEvent &evt){
				Frame->Tabs->LoadLastSession(loadCrashSession);
			}, 2299);
		}

#ifndef _WIN32
		if (const char* autoExitMsText = std::getenv("KAINOTE_AUTOMATION_EXIT_MS")){
			long autoExitMs = std::strtol(autoExitMsText, nullptr, 10);
			if (autoExitMs < 1)
				autoExitMs = 1000;
			automationExitTimer.SetOwner(this, 3299);
			Bind(wxEVT_TIMER, [this](wxTimerEvent &evt){
				for (wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst(); node; node = node->GetNext()){
					wxWindow* window = node->GetData();
					KaiDialog* kaiDialog = (window && window->IsKindOf(CLASSINFO(KaiDialog))) ? static_cast<KaiDialog*>(window) : nullptr;
					if (kaiDialog && kaiDialog->IsShown() && kaiDialog->IsModal()){
						int escapeId = kaiDialog->GetEscapeId();
						kaiDialog->EndModal(escapeId != wxID_NONE ? escapeId : wxID_CANCEL);
						automationExitTimer.Start(250, true);
						return;
					}
					wxDialog* wxDialogWindow = (window && window->IsKindOf(CLASSINFO(wxDialog))) ? static_cast<wxDialog*>(window) : nullptr;
					if (wxDialogWindow && wxDialogWindow->IsShown() && wxDialogWindow->IsModal()){
						int escapeId = wxDialogWindow->GetEscapeId();
						wxDialogWindow->EndModal(escapeId != wxID_NONE ? escapeId : wxID_CANCEL);
						automationExitTimer.Start(250, true);
						return;
					}
				}
				if (Frame){
					// Frame->Close() can synchronously open a modal "save changes?" dialog.
					// Re-arm the automation timer before closing so the modal loop has a
					// chance to receive the next tick and dismiss the dialog.
					automationExitTimer.Start(250, true);
					Frame->Close();
				}
			}, 3299);
			automationExitTimer.Start(autoExitMs, true);
		}
#endif

		wxString path = Options.GetString(EXTERNAL_FONTS_DIRECTORY);
		if(!path.empty())
			FontEnum.LoadExternalFontsToProcessFromThread(path);

	}
	else{
#ifndef _WIN32
		wxArrayString openPaths;
		for (int i = 1; i < argc; i++) {
			openPaths.Add(argv[i]);
		}
		wxString subs = JoinOpenPaths(openPaths);
#else
		wxString subs;
		for (int i = 1; i < argc; i++) {
			subs.Append(argv[i]);
			if (i + 1 != argc){ subs += L"|"; }
		}
#endif

		delete m_checker; // OnExit() won't be called if we return false
		m_checker = nullptr;
		if (subs.empty())
			return false;
#ifndef _WIN32
		if (!SendOpenPathsToRunningInstance(subs)){
			KaiLogSilent(wxString::Format(L"Cannot pass files to an already running instance on this platform: %s", subs));
		}
		return false;
#else
		//damn wxwidgets, why class name is not customizable?    
		int count = 0;
		HWND hWnd = nullptr;
		while (!hWnd){
			//prevent to total dedlock, when main Kainote is crashed or closed
			if (count > 100){
				KaiLogSilent(wxString::Format(L"Cannot open: %s", subs));
				return false;
			}

			hWnd = FindWindow(L"Kainote_main_windowNR", 0);
			//wait to can find it next time
			Sleep(40);
			count++;
		}
		//hwnd here must exist
		const wchar_t *text = subs.wc_str();
		COPYDATASTRUCT cds;
		cds.cbData = (subs.length() + 1) * sizeof(wchar_t);
		cds.lpData = (void *)text;
		SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
		
		return false;
#endif
	}

	return true;

}

int kainoteApp::OnExit()
{
#ifndef _WIN32
	if (m_ipcServer){ delete m_ipcServer; m_ipcServer = nullptr; }
	RemoveKainoteIpcSocket();
#endif
	if (m_checker){ delete m_checker; }
	wxDELETE(locale);
	return 0;
}

int kainoteApp::FilterEvent(wxEvent& event)
{
#ifndef _WIN32
	const wxEventType type = event.GetEventType();
	if (type == wxEVT_ENTER_WINDOW || type == wxEVT_MOTION || type == wxEVT_LEAVE_WINDOW){
		static wxWeakRef<wxWindow> lastTooltipWindow = nullptr;
		wxWindow *eventWindow = wxDynamicCast(event.GetEventObject(), wxWindow);
		KainoteFrame *frame = KainoteFrame::Get();
		if (eventWindow && frame){
			wxString tip = eventWindow->GetToolTipText();
			if (tip != emptyString){
				if (type == wxEVT_LEAVE_WINDOW){
					if (lastTooltipWindow == eventWindow){ lastTooltipWindow = nullptr; }
					frame->SetStatusText(emptyString, 0);
					// Keep wxGTK native tooltip popups disabled for owner-drawn controls.
					wxToolTip::Enable(false);
				}
				else{
					lastTooltipWindow = eventWindow;
					tip = tip.BeforeFirst(L'\n');
					frame->SetStatusText(tip, 0);
				}
			}
			else if (type == wxEVT_MOTION && lastTooltipWindow){
				wxWindow *top = wxGetTopLevelParent(lastTooltipWindow);
				const wxPoint mouse = wxGetMousePosition();
				if (!top || !wxRect(top->GetScreenPosition(), top->GetSize()).Contains(mouse)){
					lastTooltipWindow = nullptr;
					frame->SetStatusText(emptyString, 0);
					wxToolTip::Enable(false);
				}
			}
		}
	}
	if (type == wxEVT_LEFT_DOWN || type == wxEVT_RIGHT_DOWN || type == wxEVT_MIDDLE_DOWN ||
		type == wxEVT_LEFT_UP || type == wxEVT_RIGHT_UP || type == wxEVT_MIDDLE_UP){
		wxWindow *eventWindow = wxDynamicCast(event.GetEventObject(), wxWindow);
		if (PopupList::DismissOnExternalClick(eventWindow)){ return 1; }
		if (MenuDialog::DismissOnExternalClick(eventWindow)){ return 1; }
	}
	if (type == wxEVT_KEY_DOWN || type == wxEVT_CHAR_HOOK) {
		wxKeyEvent* keyEvent = dynamic_cast<wxKeyEvent*>(&event);
		if (keyEvent) {
			int key = keyEvent->GetKeyCode();
			if (key == WXK_ESCAPE || key == L'B' || key == L'b') {
				TabPanel* tab = Notebook::GetTab();
				if (tab && tab->video && tab->video->IsFullScreen()) {
					tab->video->SetFullscreen(false);
					return 1;
				}
			}
		}
	}
#endif
	return wxApp::FilterEvent(event);
}

//void kainoteApp::OnUnhandledException()
//{
//	//wxString recover;
//	//for(size_t i=0;i<Frame->Tabs->Size();i++)
//		//{
//		//recover<<"Sub"<<i<<": "<<Frame->Tabs->Page(i)->SubsPath<<"\r\n"
//			//<<"Vid"<<i<<": "<<Frame->Tabs->Page(i)->VideoPath<<"\r\n";
//		//}
//	//recover<<Options.GetString("Subs Recent")<<Options.GetString("video Recent");
//	//OpenWrite op;
//	//op.FileWrite(Options.pathfull+"\\recover.txt",Options.pathfull+"\\recover.txt");
//	//Options.SaveOptions();
//
//	wLogStatus(_T("Ups, Kainote się skraszował w przyszłości będzie można wznowić sesję po tym kraszu"), "Krasz", wxOK | wxICON_ERROR);
//}

void kainoteApp::OnFatalException()
{
	//wxString recover;
	//for(size_t i=0;i<Frame->Tabs->Size();i++)
	//{
	//recover<<"Sub"<<i<<": "<<Frame->Tabs->Page(i)->SubsPath<<"\r\n"
	//<<"Vid"<<i<<": "<<Frame->Tabs->Page(i)->VideoPath<<"\r\n";
	//}
	//recover<<Options.GetString("Subs Recent")<<"\r\n"<<Options.GetString("video Recent");
	//OpenWrite op;
	//op.FileWrite(Options.pathfull+"\\recover.txt",recover);
	//Options.SaveOptions();

	//KaiMessageBox(_T("Ups, Kainote się skraszował w przyszłości będzie można wznowić sesję po tym kraszu"), "Krasz", wxOK | wxICON_ERROR);
}
void kainoteApp::OnOpen(wxTimerEvent &evt)
{
	if (!IsBusy()){
		if (Frame->IsIconized()){ Frame->Iconize(false); }
		Frame->Raise();
		Frame->OpenFiles(paths, false);
		paths.Clear();
	}
	else{
		openTimer.Start(100, true);
	}
}

bool kainoteApp::IsBusy()
{
	wxWindowList children = Frame->GetChildren();
	for (wxWindowList::Node *node = children.GetFirst(); node; node = node->GetNext()) {
		wxWindow *current = (wxWindow *)node->GetData();
		if ((current->IsKindOf(CLASSINFO(KaiDialog)) && ((KaiDialog*)current)->IsModal()) ||
			(current->IsKindOf(CLASSINFO(wxDialog)) && ((wxDialog*)current)->IsModal()) || current->GetId() == 31555)
		{
			return true;
		}

	}
	return false;
}

