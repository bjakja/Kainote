/***************************************************************
 * Name:      kainoteApp.h
 * Purpose:   Defines Application Class
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * Copyright: Bjakja (www.costam.com)
 * License:
 **************************************************************/

#ifndef KAINOTEAPP_H
#define KAINOTEAPP_H

#include <wx/app.h>
#include <wx/snglinst.h>

#include "KainoteMain.h"
#if _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#define _CRTDBG_MAP_ALLOC_NEW
//#include <cstdlib>
//#include <crtdbg.h>
#include <vld.h> 
#endif

class KaiServer;

class kainoteApp : public wxApp
{
public:
    bool OnInit();
	int OnExit();
	bool OnSecondInstance(wxString _paths);
	//void OnUnhandledException();
	void OnFatalException();
	void OnOpen(wxTimerEvent &evt);
	bool IsBusy();
	kainoteFrame* Frame;
	wxTimer timer;
	wxArrayString paths;
private:
	wxSingleInstanceChecker* m_checker;
	KaiServer *MyServer;
	wxLocale *locale;
};


DECLARE_APP(kainoteApp)

#endif 
