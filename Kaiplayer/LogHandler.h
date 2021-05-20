//  Copyright (c) 2018 - 2020, Marcin Drob

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

#include <wx/window.h>
#include <wx/thread.h>
//#define Get_Log LogHandler::Get()

class LogWindow;

class LogHandler
{
	friend class LogWindow;
private:
	LogHandler(wxWindow *parent);
	~LogHandler();
	static LogHandler *sthis;
	wxString logToAppend;
	wxString lastLog;
	LogWindow *lWindow = NULL;
	wxMutex mutex;
	wxWindow *parent;
public:
	static void Create(wxWindow *parent);
	static LogHandler *Get(){ return sthis; }
	static void Destroy();
	void LogMessage(const wxString &format, bool showMessage = true);
	static void ShowLogWindow();
	//void LogMessage1(const wxString &format, ...);
};

static void KaiLog(const wxString &text){
	LogHandler * handler = LogHandler::Get();
	if (handler)
		handler->LogMessage(text);
}

static void KaiLogSilent(const wxString &text) {
	LogHandler * handler = LogHandler::Get();
	if (handler)
		handler->LogMessage(text, false);
}

static void KaiLogDebug(const wxString &text){
#if _DEBUG
	LogHandler * handler = LogHandler::Get();
	if (handler)
		handler->LogMessage(text);
#endif
}