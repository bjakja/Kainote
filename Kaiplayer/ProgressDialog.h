//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include <wx/wx.h>
#include "KaiGauge.h"
#include <functional>
#include "MappedButton.h"
#include "KaiStaticText.h"

struct ITaskbarList3;
class ProgresDialog : public wxDialog
{
private:
	KaiGauge *gauge;
	KaiStaticText *text;
	KaiStaticText *text1;

	bool canceled;
	int oldtime;
	void OnCancel(wxCommandEvent& event);
	void OnShow(wxThreadEvent& evt);
	void OnProgress(wxThreadEvent& evt);
	void OnTitle(wxThreadEvent& evt);
	int firsttime;

public:
	ProgresDialog(wxWindow *parent, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = 0);
	virtual ~ProgresDialog();

	void Progress(int num);
	void Title(wxString title);
	bool WasCancelled();
	MappedButton *cancel;
	ITaskbarList3 *taskbar;
	int result;
};

class ProgressSink : public wxThread
{
public:
	ProgressSink(wxWindow *parent, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = 0);
	virtual ~ProgressSink();
	//Pamiêtaj o poprawnej kolejnoœci, najpierw tworzysz sink
	//póŸniej ustawiasz zadanie b¹dŸ tworzysz w¹tek
	//kolejnie pokazujesz dialog 
	//a na koñcu czekasz na wykonanie zadania / w¹tku
	void SetAndRunTask(std::function<int()> _task)
	{
		task = _task;
		Create();
		Run();
	}
	void ShowDialog();
	int ShowSecondaryDialog(std::function<int()> dialfunction);
	void Title(wxString title);
	bool WasCancelled();
	void Progress(int num);
	void EndModal();
private:
	wxThread::ExitCode Entry();
	std::function<int()> task;
	ProgresDialog *dlg;

};



