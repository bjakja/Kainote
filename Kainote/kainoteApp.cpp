/***************************************************************
 * Copyright (c) 2012-2020, Marcin Drob
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

#ifdef guano
 
#include "KainoteApp.h"
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
#include "loghandler.h"

#include "UtilsWindows.h"






//void EnableCrashingOnCrashes()
//{
//	typedef BOOL(WINAPI *tGetPolicy)(LPDWORD lpFlags);
//	typedef BOOL(WINAPI *tSetPolicy)(DWORD dwFlags);
//	const DWORD EXCEPTION_SWALLOWING = 0x1;
//
//	HMODULE kernel32 = LoadLibraryA("kernel32.dll");
//	tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32,
//		"GetProcessUserModeExceptionPolicy");
//	tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32,
//		"SetProcessUserModeExceptionPolicy");
//	if (pGetPolicy && pSetPolicy)
//	{
//		DWORD dwFlags;
//		if (pGetPolicy(&dwFlags))
//		{
//			// Turn off the filter
//			pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
//		}
//	}
//}

void kainoteApp::OnOutofMemory()
{
	TabPanel *tab = Notebook::GetTab();

	if (tab->grid->file->maxx() > 3){
		tab->grid->file->RemoveFirst(2);
		KaiLog(_("Zabrakło pamięci RAM, usunięto część historii"));
		return;
	}
	else if (Notebook::GetTabs()->Size() > 1){
		for (size_t i = 0; i < Notebook::GetTabs()->Size(); i++)
		{
			if (i != Notebook::GetTabs()->GetSelection()){
				if (Notebook::GetTabs()->Page(i)->grid->file->maxx()>3){
					Notebook::GetTabs()->Page(i)->grid->file->RemoveFirst(2);
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

	m_checker = new wxSingleInstanceChecker();

	//bool wxsOK = true;

	if (!m_checker->IsAnotherRunning())
	{

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
			if (!li)
				KaiMessageBox(L"Cannot find language, language change failed");
			else{
				if (!locale->Init(li->Language, wxLOCALE_DONT_LOAD_DEFAULT)){
					KaiMessageBox(L"wxLocale cannot initialize, language change failed");
				}
				else{
					locale->AddCatalogLookupPathPrefix(Options.pathfull + L"\\Locale\\");
					if (!locale->AddCatalog(lang, wxLANGUAGE_POLISH, L"UTF-8")){
						KaiMessageBox(L"Cannot find translation, language change failed");
					}
				}
			}
		}

		/*if (!Options.GetBool(DONT_SHOW_CRASH_INFO)) {
			signal(SIGSEGV, seg_handler);
			std::set_terminate(std_handler);
		}*/
		//on x64 it makes not working unicode toupper tolower conversion
		//setlocale(LC_CTYPE, "C");
		//locale numbers changes here cause of it is set with wxlocale, I have to change it back
		setlocale(LC_NUMERIC, "C");

		if (!Hkeys.LoadHkeys()){
			KaiMessageBox(_("Nie udało się wczytać skrótów.\nDziałanie programu zostanie zakończone."), _("Uwaga"));
			wxDELETE(locale); return false;
		}

		for (int i = 1; i < argc; i++) { paths.Add(argv[i]); }

		int posx, posy, sizex, sizey, msizex, msizey;
		Options.GetCoords(WINDOW_POSITION, &posx, &posy);
		Options.GetCoords(WINDOW_SIZE, &sizex, &sizey);
		Options.GetCoords(MONITOR_SIZE, &msizex, &msizey);
		if (msizex && msizey) {
			wxRect rt = GetMonitorRect1(-1, nullptr, wxRect(posx, posy, sizex, sizey));
			if (rt.width != msizex || rt.height != msizey) {
				int mposx, mposy, vsizex, vsizey;
				Options.GetCoords(MONITOR_POSITION, &mposx, &mposy);
				Options.GetCoords(VIDEO_WINDOW_SIZE, &vsizex, &vsizey);
				int audioHeight = Options.GetInt(AUDIO_BOX_HEIGHT);
				float scalex = (float)rt.width / (float)msizex;
				float scaley = (float)rt.height / (float)msizey;
				posx -= mposx;
				posy -= mposy;
				posx *= scalex;
				posy *= scaley;
				if (posx == -1)
					posx = 0;
				if (posy == -1)
					posy = 0;
				sizex *= scalex;
				sizey *= scaley;
				vsizex *= scalex;
				vsizey *= scaley;
				audioHeight *= scaley;
				Options.SetCoords(VIDEO_WINDOW_SIZE, vsizex, vsizey);
				Options.SetCoords(MONITOR_SIZE, rt.width, rt.height);
				Options.SetCoords(MONITOR_POSITION, rt.x, rt.y);
				Options.SetInt(AUDIO_BOX_HEIGHT, audioHeight);
			}
			if (sizex > rt.width) {
				sizex = rt.width;
			}
			if (sizey > rt.height) {
				sizey = rt.height - 100;
			}
			if (!rt.Contains(wxRect(posx, posy, sizex, sizey))) {
				posx = rt.x + ((float)(rt.width - sizex) / 2.f),
				posy = rt.y + ((float)(rt.height - sizey) / 2.f);
			}
		}
		if (sizex < 500 || sizey < 350) {
			sizex = 800; sizey = 650;
		}
		

		Frame = nullptr;
		Frame = new KainoteFrame(wxPoint(posx, posy), wxSize(sizex, sizey));
		//handler for out of memory in new
		std::set_new_handler(OnOutofMemory);
		//start listen to font folders notifications
		FontEnum.StartListening(Frame);
		//EnableCrashingOnCrashes();
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
		Connect(1199, wxEVT_TIMER, (wxObjectEventFunction)&kainoteApp::OnOpen);
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
		/*if (loadSession){
			debugtimer.SetOwner(this, 2299);
			debugtimer.Start(100, true);
			Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
				Frame->Tabs->LoadLastSession(loadCrashSession);
			}, 2299);
		}*/

		

		//}
	}
	else{
		wxString subs;
		for (int i = 1; i < argc; i++) {
			subs.Append(argv[i]);
			if (i + 1 != argc){ subs += L"|"; }
		}

		delete m_checker; // OnExit() won't be called if we return false
		m_checker = nullptr;
		if (subs.empty())
			return false;
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
	}

	return true;

}

int kainoteApp::OnExit()
{
	if (m_checker){ delete m_checker; }
	wxDELETE(locale);
	return 0;
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

#endif