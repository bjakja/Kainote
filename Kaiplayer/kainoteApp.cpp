/***************************************************************
 * Name:      kainoteApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * Copyright: Bjakja (www.costam.com)
 * License:
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

wxDEFINE_EVENT(EVT_OPEN, wxThreadEvent);

IMPLEMENT_APP(kainoteApp);


class KaiConnection : public wxConnection
{

public:
     KaiConnection() : wxConnection() {}
	 ~KaiConnection(){ }
 
	 bool OnExec (const wxString &topic, const wxString &data)
     {
		 //wxLogStatus("on exec");
		 //kainoteApp *inst=(kainoteApp*)wxTheApp;
		 //while(inst->isopening){ Sleep(25);}
         bool result = wxGetApp().OnSecondInstance(data);
 
         return result;
     }
 

 };

class KaiServer : public wxServer
 {
public:
     virtual wxConnectionBase *OnAcceptConnection (const wxString& topic)
     {
         if (topic != "NewStart")
             return NULL;
         else
             return new KaiConnection;
     }
 };





bool kainoteApp::OnSecondInstance(wxString _paths)
{
	//wxMutexLocker lock(mutex);
	//isopening=true;
	if(Frame->IsIconized()){Frame->Iconize(false);}
	Frame->Raise();
	if(_paths==""){return true;}
	wxStringTokenizer tkn(_paths,"|");
	
	while(tkn.HasMoreTokens()){
		paths.Add(tkn.NextToken());
	}
	timer.Start(500,true);

	return true; 
}

bool kainoteApp::OnInit()
{
    
	m_checker = new wxSingleInstanceChecker();
        
    bool wxsOK = true;
	//isfirst=true;

	wxString server="4242";

	if ( !m_checker->IsAnotherRunning() )
    {
		MyServer=new KaiServer();
		if(!MyServer){
			wxLogStatus(_("Nie można utworzyć serwera DDE"));
		}
		else if (!(MyServer->Create(server))){
			delete MyServer;
			MyServer = NULL;
		}

		setlocale(LC_NUMERIC, "C");
		setlocale(LC_CTYPE, "C");

		wxImage::AddHandler(new wxPNGHandler);
		wxImage::AddHandler(new wxICOHandler);
		wxImage::AddHandler(new wxCURHandler);

		if ( wxsOK )
		{
		//wxHandleFatalExceptions(true);
			SetAppDisplayName("KaiNote");

			if(!Options.LoadOptions()){wxMessageBox(_("Nie udało się wczytać opcji.\nDziałanie programu zostanie zakończone."),_("Uwaga"));return false;}

			locale=NULL;
			if(Options.GetInt(L"Program Language") != 0){
				locale=new wxLocale();
				if(!locale->Init(wxLANGUAGE_ENGLISH, wxLOCALE_DONT_LOAD_DEFAULT)){
					wxMessageBox("wxLocale cannot initialize, language change failed");
				}
				locale->AddCatalogLookupPathPrefix(Options.pathfull+L"\\Locale\\");
				if(!locale->AddCatalog(L"en",wxLANGUAGE_POLISH,L"UTF-8")){//
					wxMessageBox("Cannot find translation, language change failed");
				}
				//wxMessageBox(wxString::Format("isload%i", locale->IsLoaded(L"en")));
			}

			if(!Hkeys.LoadHkeys()){
				wxMessageBox(_("Nie udało się wczytać skrótów.\nDziałanie programu zostanie zakończone."),_("Uwaga"));
				wxDELETE(locale);return false;
			}
			
			for (int i=1;i<argc;i++) { paths.Add(argv[i]); }

			int posx,posy,sizex,sizey;
			Options.GetCoords("Window Position",&posx,&posy);
			//SetPosition(wxPoint(posx,posy));
			Options.GetCoords("Window Size",&sizex,&sizey);
			if(sizex<500 || sizey<350){
				sizex=800;sizey=650;
			}
			//SetClientSize(wxSize(sizex,sizey));
			Frame=NULL;
    		Frame = new kainoteFrame(wxPoint(posx,posy),wxSize(sizex,sizey));
			bool opevent=false;
			if(paths.GetCount()>0){
				if(Options.GetBool("Video Fullskreen on Start")){
					
					Frame->OpenFiles(paths,false, true); paths.clear();
					Frame->GetTab()->Video->Layout();
				}
				else{
					opevent=true;
					
				}
			}
			

			Frame->Show();

    		SetTopWindow(Frame);
			timer.SetOwner(this,1199);
			Connect(1199,wxEVT_TIMER,(wxObjectEventFunction)&kainoteApp::OnOpen);
			//Bind(EVT_OPEN,&kainoteApp::OnOpen,this);
			//
			if(opevent){
				//wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,1119);
				//AddPendingEvent(evt);
				//wxLogStatus("opevent");
				//Frame->OpenFiles(pathss,false, true); paths.clear();
				//isopening=false;
				timer.Start(500,true);
				//opthread *ot=new opthread(this,pathss);
			}
		
		}
		
    }
	else{
		wxString subs;
		for (int i=1;i<argc;i++) {
			subs.Append(argv[i]);
			if(i+1!=argc){subs+="|";}
		}

		delete m_checker; // OnExit() won't be called if we return false
        m_checker = NULL;
		//wxLogStatus("execute");
		    
		wxClient *Client=new wxClient;
        KaiConnection * Connection = (KaiConnection*)Client->MakeConnection("", server, "NewStart");

		if (Connection)
        {
		
		Connection->Execute(subs);
		delete Connection;
			
		}
		delete Client;
			
			
        return false;
	}
    
    return wxsOK;

}

int kainoteApp::OnExit()
{
    delete m_checker;
	delete MyServer;
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
//	wxLogStatus(_T("Ups, Kainote się skraszował w przyszłości będzie można wznowić sesję po tym kraszu"), "Krasz", wxOK | wxICON_ERROR);
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

	wxLogStatus(_T("Ups, Kainote się skraszował w przyszłości będzie można wznowić sesję po tym kraszu"), "Krasz", wxOK | wxICON_ERROR);
}
void kainoteApp::OnOpen(wxTimerEvent &evt)
{
	if(!IsBusy()){
		Frame->OpenFiles(paths,false);paths.clear();
	}
}

bool kainoteApp::IsBusy()
{
	wxWindowList children = Frame->GetChildren();
	for (wxWindowList::Node *node=children.GetFirst(); node; node = node->GetNext()) {
            wxWindow *current = (wxWindow *)node->GetData();
            if (current->IsKindOf(CLASSINFO(wxDialog)) 
				&& (((wxDialog*)current)->IsModal() || ((wxDialog*)current)->GetId() == 31555 ))
		{
			return true;
		}
	
	}
	return false;
}

