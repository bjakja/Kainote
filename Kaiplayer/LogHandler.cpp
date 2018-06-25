//  Copyright (c) 2018, Marcin Drob

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

#include "LogHandler.h"
#include "KaiDialog.h"
#include "KaiStaticText.h"
#include "KaiTextCtrl.h"
#include "MappedButton.h"

wxDEFINE_EVENT(EVT_DO_LOG, wxThreadEvent);

class LogWindow : public KaiDialog
{
	friend class LogHandler;
public:
	LogWindow(wxWindow *parent, LogHandler *handler);
	virtual ~LogWindow(){};
	volatile bool isReady;
private:
	void OnGetLog(wxThreadEvent &evt);
	LogHandler *handler;
	KaiTextCtrl *logText;
	KaiStaticText *lastLogText;
	DialogSizer *sizer;
	bool hiddenLastLog = false;
};

LogWindow::LogWindow(wxWindow *parent, LogHandler *_handler)
	:KaiDialog(parent,-1,_("Okno Logów"), wxDefaultPosition, wxDefaultSize)
	,handler(_handler)
{
	sizer = new DialogSizer(wxVERTICAL);
	lastLogText = new KaiStaticText(this, -1, "", wxDefaultPosition, wxSize(500, -1));
	//MappedButton *collapse = new MappedButton(this, 12456, _("Poka¿ resztê logów"));
	logText = new KaiTextCtrl(this, -1, "", wxDefaultPosition, wxSize(500, 300), wxTE_MULTILINE);
	logText->Show(false);
	MappedButton *OK = new MappedButton(this, 12457, _("Zamknij"));
	sizer->Add(lastLogText, 0, wxALIGN_CENTER | wxALL, 10);
	//sizer->Add(collapse, 0, wxALL, 4);
	sizer->Add(logText, 0, wxEXPAND | wxALL, 4);
	sizer->Add(OK, 0, wxALL | wxALIGN_CENTER, 4);

	Bind(EVT_DO_LOG, &LogWindow::OnGetLog, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		bool isShown = logText->IsShown();
		logText->Show(!isShown);
		wxSize size = GetClientSize();
		if (!isShown){
			size.x += 304;
			SetMinSize(size);
		}
		else{
			size.x -= 304;
			SetMinSize(size);
		}
		sizer->Fit(this);
	}, 12456);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		Show(false);
	}, 12457);
	SetSizerAndFit(sizer);
}

void LogWindow::OnGetLog(wxThreadEvent &evt)
{
	isReady = false;
	if (!IsShown()){ 
		Show(); 
		CenterOnParent(); 
		if (hiddenLastLog){
			lastLogText->Show();
			logText->Show(false);
			hiddenLastLog = false;
		}
	}
	lastLogText->SetLabelText(handler->lastLog);
	logText->AppendText(handler->logToAppend);
	handler->logToAppend = "";
	sizer->Fit(this);
	isReady = true;
}

LogHandler *LogHandler::sthis = NULL;

LogHandler::LogHandler(wxWindow *parent)
{
	lWindow = new LogWindow(parent, this);
}

LogHandler::~LogHandler()
{

}

void LogHandler::Create(wxWindow *parent)
{
	if (!sthis)
		sthis = new LogHandler(parent);
}

void LogHandler::Destroy()
{
	if (sthis){
		delete sthis;
		sthis = NULL;
	}
}

void LogHandler::LogMessage(const wxString &message)
{
	wxMutexLocker lock(mutex);
	//lastLog = wxString::Format(format).Trim();
	lastLog = message;
	SYSTEMTIME st;
	GetSystemTime(&st);
	logToAppend << wxString::Format(L"%02i:%02i:%02i ", st.wHour, st.wMinute, st.wSecond);
	logToAppend << lastLog << L"\n";
	if (lWindow->isReady){
		wxThreadEvent *evt = new wxThreadEvent(EVT_DO_LOG, lWindow->GetId());
		wxQueueEvent(lWindow, evt);
	}
}

void LogHandler::ShowLogWindow()
{
	if (sthis){
		if (!sthis->lWindow->hiddenLastLog){
			sthis->lWindow->logText->Show();
			sthis->lWindow->lastLogText->Show(false);
			sthis->lWindow->sizer->Fit(sthis->lWindow);
			sthis->lWindow->CenterOnParent();
			sthis->lWindow->hiddenLastLog = true;
		}
		sthis->lWindow->Show(!sthis->lWindow->IsShown());
	}
}

//void LogHandler::LogMessage1(const wxString &format, ...)
//{
//	wxMutexLocker lock(mutex);
//	lastLog = wxString::Format(format, args).Trim();
//	log << lastLog << "\n";
//	if (lWindow->isReady){
//		wxThreadEvent *evt = new wxThreadEvent(EVT_DO_LOG, lWindow->GetId());
//		wxQueueEvent(lWindow, evt);
//	}
//}