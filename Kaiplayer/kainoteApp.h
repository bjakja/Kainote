/***************************************************************
 * Copyright (c) 2012-2020, Marcin Drob
 * Name:      kainoteApp.h
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

#pragma once

#include <wx/app.h>
#include <wx/snglinst.h>

#include "KainoteMain.h"

#if _DEBUG
//#define _CRTDBG_MAP_ALLOC
//  #define _CRTDBG_MAP_ALLOC_NEW
//  #include <stdlib.h>
//  #include <crtdbg.h>
//
//  #define DEBUG_NEW   new( _CLIENT_BLOCK, __FILE__, __LINE__)
//  #define new DEBUG_NEW
//#include <vld.h> 
#endif

class kainoteApp : public wxApp
{
public:
	bool OnInit();
	int OnExit();
	//bool OnSecondInstance(wxString _paths);
	//void OnUnhandledException();
	void OnFatalException();
	void OnOpen(wxTimerEvent &evt);
	bool IsBusy();
	KainoteFrame* Frame;
	wxTimer timer;
	wxTimer debugtimer;
	wxArrayString paths;
private:
	wxSingleInstanceChecker* m_checker;
	wxLocale *locale;
	static void OnOutofMemory();
};


DECLARE_APP(kainoteApp)
 
