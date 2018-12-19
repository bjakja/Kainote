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

//wxDEFINE_EVENT(EVT_OPEN, wxThreadEvent);

IMPLEMENT_APP(kainoteApp);



bool kainoteApp::OnInit()
{

	m_checker = new wxSingleInstanceChecker();

	bool wxsOK = true;

	if (!m_checker->IsAnotherRunning())
	{

		setlocale(LC_NUMERIC, "C");
		//on x64 it makes not working unicode toupper tolower conversion
		//setlocale(LC_CTYPE, "C");

		wxImage::AddHandler(new wxPNGHandler);
		wxImage::AddHandler(new wxICOHandler);
		wxImage::AddHandler(new wxCURHandler);

		if (wxsOK)
		{
			//wxHandleFatalExceptions(true);
			//0 - failed, 1 - succeeded, 2 - no config
			int isGood = Options.LoadOptions();
			if (!isGood){ KaiMessageBox(_("Nie udało się wczytać opcji.\nDziałanie programu zostanie zakończone."), _("Uwaga")); return false; }
			//0x0415 	Polish (pl) 	0x15 	LANG_POLISH 	Poland (PL) 	0x01 	SUBLANG_POLISH_POLAND
			if (isGood == 2 && GetSystemDefaultUILanguage() != 0x415){
				//what a lame language system, I need to change it.
				Options.SetInt(ProgramLanguage, 1);
				Options.SetString(DictionaryLanguage, L"en_US");
			}

			locale = NULL;
			if (Options.GetInt(ProgramLanguage) != 0){
				locale = new wxLocale();
				if (!locale->Init(wxLANGUAGE_ENGLISH, wxLOCALE_DONT_LOAD_DEFAULT)){
					KaiMessageBox("wxLocale cannot initialize, language change failed");
				}
				locale->AddCatalogLookupPathPrefix(Options.pathfull + L"\\Locale\\");
				if (!locale->AddCatalog(L"en", wxLANGUAGE_POLISH, L"UTF-8")){//
					KaiMessageBox("Cannot find translation, language change failed");
				}
			}

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
			/*else if (paths.GetCount()<1){
				if(Frame->subsrec.size()>0){paths.Add(Frame->subsrec[0]);}
				if(Frame->videorec.size()>0){paths.Add(Frame->videorec[0]);}
				timer.Start(50,true);
				}*/
			Frame->Tabs->LoadLastSession(Frame);
#endif

		}
	}
	else{
		wxString subs;
		for (int i = 1; i < argc; i++) {
			subs.Append(argv[i]);
			if (i + 1 != argc){ subs += "|"; }
		}

		delete m_checker; // OnExit() won't be called if we return false
		m_checker = NULL;
		//damn wxwidgets, why class name is not customizable?    
		HWND hWnd = FindWindow(L"Kainote_main_windowNR", 0);/**///wxWindow
		if (hWnd && subs != ""){
			const wchar_t *text = subs.wc_str();
			COPYDATASTRUCT cds;
			cds.cbData = (subs.Len() + 1) * sizeof(wchar_t);
			cds.lpData = (void *)text;
			SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
		}
		return false;
	}

	return wxsOK;

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

