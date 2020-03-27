/***************************************************************
 * Name:      kainoteApp.cpp
 * Purpose:   Subtitles editor and player
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * Copyright: Marcin Drob aka Bjakja (http://animesub.info/forum/viewtopic.php?id=258715)
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



#include "KainoteApp.h"
#include <wx/image.h>
#include <wx/ipc.h>
#include <wx/utils.h>
#include <locale.h>
#include "OpennWrite.h"
#include "Config.h"
#include "Hotkeys.h"
#include <wx/intl.h>
#include "KaiMessageBox.h"
#include "FontEnumerator.h"
#include <DbgHelp.h>
#include <signal.h>

//wxDEFINE_EVENT(EVT_OPEN, wxThreadEvent);

void seg_handler(int sig)
{
	unsigned int   i;
	void         * stack[100];
	unsigned short frames;
	SYMBOL_INFO  * symbol;
	HANDLE         process;

	process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	frames = CaptureStackBackTrace(0, 100, stack, NULL);
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	wxString result;
	for (i = 0; i < frames; i++) {
		SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
		result += wxString::Format(L"%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
	}

	free(symbol);

	OpenWrite ow;
	ow.FileWrite(Options.pathfull + "\\CrashInfo.txt", result);
	Options.SaveOptions(true, false, true);
	KainoteFrame *Kai = ((kainoteApp *)wxTheApp)->Frame;
	Notebook::SaveLastSession(false);
	wxString Info = _("Kainote się scrashował i próbuje pozyskać istotne dane.\n") +
		_("Napisy zostały zapisane do folderu \"Recovery\".\n") +
		_("Ostatnia sesja zostanie wznowiona po następnym uruchomieniu.\n") +
		_("W przypadku, gdyby zostały uszkodzone, autozapisy są w folderze \"Subs\".\n") +
		_("Info o crashu zostało zapisane do pliku \"CrashInfo.txt\" w folderze Kainote.\n") + 
		_("Podesłanie go na adres mailowy (bjakja7@gmail.com) może pomóc rozwiązać ten problem.");
	KaiMessageBox(Info, L"Crash", wxOK, Kai);
	exit(1);
}

void std_handler(void) {
	seg_handler(1);
}


void EnableCrashingOnCrashes()
{
	typedef BOOL(WINAPI *tGetPolicy)(LPDWORD lpFlags);
	typedef BOOL(WINAPI *tSetPolicy)(DWORD dwFlags);
	const DWORD EXCEPTION_SWALLOWING = 0x1;

	HMODULE kernel32 = LoadLibraryA("kernel32.dll");
	tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32,
		"GetProcessUserModeExceptionPolicy");
	tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32,
		"SetProcessUserModeExceptionPolicy");
	if (pGetPolicy && pSetPolicy)
	{
		DWORD dwFlags;
		if (pGetPolicy(&dwFlags))
		{
			// Turn off the filter
			pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
		}
	}
}

void kainoteApp::OnOutofMemory()
{
	TabPanel *tab = Notebook::GetTab();

	if (tab->Grid->file->maxx() > 3){
		tab->Grid->file->RemoveFirst(2);
		KaiLog(_("Zabrakło pamięci RAM, usunięto część historii"));
		return;
	}
	else if (Notebook::GetTabs()->Size() > 1){
		for (size_t i = 0; i < Notebook::GetTabs()->Size(); i++)
		{
			if (i != Notebook::GetTabs()->GetSelection()){
				if (Notebook::GetTabs()->Page(i)->Grid->file->maxx()>3){
					Notebook::GetTabs()->Page(i)->Grid->file->RemoveFirst(2);
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

		signal(SIGSEGV, seg_handler);
		std::set_terminate(std_handler);
		//on x64 it makes not working unicode toupper tolower conversion
		//setlocale(LC_CTYPE, "C");

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
			Options.SetString(ProgramLanguage, L"en");
			Options.SetString(DICTIONARY_LANGUAGE, L"en_US");
		}
			
		locale = NULL;
		wxString lang = Options.GetString(ProgramLanguage);
		if (lang == L"0"){
			lang = L""; Options.SetString(ProgramLanguage, lang);
		}
		if (lang == L"1"){
			lang = L"en"; Options.SetString(ProgramLanguage, lang);
		}
		if (lang != L"" && lang != L"pl"){
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
		//locale numbers changes here cause of it is set with wxlocale, I have to change it back
		setlocale(LC_NUMERIC, "C");

		if (!Hkeys.LoadHkeys()){
			KaiMessageBox(_("Nie udało się wczytać skrótów.\nDziałanie programu zostanie zakończone."), _("Uwaga"));
			wxDELETE(locale); return false;
		}

		for (int i = 1; i < argc; i++) { paths.Add(argv[i]); }

		int posx, posy, sizex, sizey;
		Options.GetCoords(WindowPosition, &posx, &posy);
		Options.GetCoords(WindowSize, &sizex, &sizey);
		if (sizex < 500 || sizey < 350){
			sizex = 800; sizey = 650;
		}

		Frame = NULL;
		Frame = new KainoteFrame(wxPoint(posx, posy), wxSize(sizex, sizey));
		//handler for out of memory in new
		std::set_new_handler(OnOutofMemory);
		//start listen to font folders notifications
		FontEnum.StartListening(Frame);
		EnableCrashingOnCrashes();
		if (isGood == 2)
			Frame->CenterOnScreen();

		bool opevent = false;
		if (paths.GetCount() > 0){
			if (Options.GetBool(VideoFullskreenOnStart)){
				Frame->OpenFiles(paths, false, true);
				Frame->GetTab()->Video->Layout();
			}
			else{
				opevent = true;

			}
		}


		Frame->Show();
		SetTopWindow(Frame);
		timer.SetOwner(this, 1199);
		Connect(1199, wxEVT_TIMER, (wxObjectEventFunction)&kainoteApp::OnOpen);
		if (opevent){
			timer.Start(500, true);
		}
#if _DEBUG
		bool loadSession = true;
#else
		int session = Options.GetInt(LAST_SESSION_CONFIG);
		bool loadSession = (session == 2) || Options.HasCrashed();
		if (session == 1){
			if (KaiMessageBox(_("Wczytać poprzednią sesję"), _("Pytanie"), wxYES_NO, Frame) == wxYES){
				loadSession = true;
			}
		}
#endif
		if (loadSession){
			debugtimer.SetOwner(this, 2299);
#if _DEBUG
			//on debug crashes vsfilter when load via direct show
			//like there was no mutex in csri
			debugtimer.Start(400, true);
#else
			debugtimer.Start(100, true);
#endif
			Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
				Frame->Tabs->LoadLastSession(Frame);
			}, 2299);
		}

		

		//}
	}
	else{
		wxString subs;
		for (int i = 1; i < argc; i++) {
			subs.Append(argv[i]);
			if (i + 1 != argc){ subs += L"|"; }
		}

		delete m_checker; // OnExit() won't be called if we return false
		m_checker = NULL;
		//damn wxwidgets, why class name is not customizable?    
		HWND hWnd = FindWindow(L"Kainote_main_windowNR", 0);/**///wxWindow
		if (hWnd && subs != L""){
			const wchar_t *text = subs.wc_str();
			COPYDATASTRUCT cds;
			cds.cbData = (subs.length() + 1) * sizeof(wchar_t);
			cds.lpData = (void *)text;
			SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
		}
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
//	//recover<<Options.GetString("Subs Recent")<<Options.GetString("Video Recent");
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
	//recover<<Options.GetString("Subs Recent")<<"\r\n"<<Options.GetString("Video Recent");
	//OpenWrite op;
	//op.FileWrite(Options.pathfull+"\\recover.txt",recover);
	//Options.SaveOptions();

	KaiMessageBox(_T("Ups, Kainote się skraszował w przyszłości będzie można wznowić sesję po tym kraszu"), "Krasz", wxOK | wxICON_ERROR);
}
void kainoteApp::OnOpen(wxTimerEvent &evt)
{
	if (!IsBusy()){
		if (Frame->IsIconized()){ Frame->Iconize(false); }
		Frame->Raise();
		Frame->OpenFiles(paths, false);
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

