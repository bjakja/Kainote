//  Copyright (c) 2016 - 2026, Marcin Drob

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


#include "ProgressDialog.h"
//#include "kainoteApp.h"
#include "SubsTime.h"
#include "KaiStaticText.h"
#include "KaiGauge.h"
#include "KainoteFrame.h"
//#include <wx/gauge.h>
#include <ShObjIdl.h>

wxDEFINE_EVENT(EVT_SHOW_DIALOG, wxThreadEvent);
wxDEFINE_EVENT(EVT_SET_TITLE, wxThreadEvent);
wxDEFINE_EVENT(EVT_SET_PROGRESS, wxThreadEvent);
wxDEFINE_EVENT(EVT_CREATE_SECONDARY_DIALOG, wxThreadEvent);
wxDEFINE_EVENT(EVT_END_MODAL, wxThreadEvent);

ProgresDialog::ProgresDialog(wxWindow *_parent, const wxString &title, const wxPoint &pos, const wxSize &size, int style)
	: wxDialog(_parent, 31555, emptyString, pos, size, style)
{
	RegisterWindowMessage(L"TaskbarButtonCreated");
	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	SetFont(*Options.GetFont());
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	text = new KaiStaticText(this, -1, title);
	gauge = new KaiGauge(this, -1, wxDefaultPosition, wxSize(300, 20), wxGA_HORIZONTAL);
	text1 = new KaiStaticText(this, -1, _("Upłynęło 00:00:00.00 sekund"));
	cancel = new MappedButton(this, 23333, _("Anuluj"));
	sizer->Add(text, 0, wxALIGN_CENTER | wxALL, 3);//wxALIGN_CENTER|
	sizer->Add(gauge, 0, wxALIGN_CENTER | wxALL, 3);
	sizer->Add(text1, 0, wxALIGN_CENTER | wxALL, 3);
	sizer->Add(cancel, 0, wxALIGN_CENTER | wxALL, 3);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ProgresDialog::OnCancel, this, 23333);
	Bind(EVT_SHOW_DIALOG, &ProgresDialog::OnShow, this);
	Bind(EVT_SET_PROGRESS, &ProgresDialog::OnProgress, this);
	Bind(EVT_SET_TITLE, &ProgresDialog::OnTitle, this);
	Bind(EVT_CREATE_SECONDARY_DIALOG, [=, this](wxThreadEvent &evt){
		std::pair<std::function<int()>, wxSemaphore*> pair = evt.GetPayload<std::pair<std::function<int()>, wxSemaphore*>>();
		wxSemaphore* sema = pair.second;
		std::function<int()> showDial = pair.first;
		result = showDial();
		sema->Post();
		oldtime = 0;
		firsttime = timeGetTime();
	});
	Bind(EVT_END_MODAL, [=, this](wxThreadEvent &evt){
		EndFrameProgress();
		if (IsModal()){
			EndModal(wxID_OK);
		}
		else{
			Hide();
		}
	});
	firsttime = timeGetTime();
	canceled = false;
	SetSizerAndFit(sizer);
	CenterOnParent();
	oldtime = 0;
	//check if toolbar was initialized if was prevent to initialize it second time
	//and release when other instance still doing task
	wasTaskbarInitialized = KainoteFrame::ProgressIsInitialized();
	if (!wasTaskbarInitialized) {
		KainoteFrame::ProgressSetup();
	}
}


ProgresDialog::~ProgresDialog()
{
	EndFrameProgress();
}

void ProgresDialog::EndFrameProgress()
{
	if (!progressEnded && !wasTaskbarInitialized) {
		KainoteFrame::ProgressEnd();
	}
	progressEnded = true;
}

void ProgresDialog::Progress(int num)
{

	int newtime = timeGetTime() - firsttime;
	if (oldtime + 10 < newtime){
		gauge->SetValue(num);
		if (!wasTaskbarInitialized) {
			KainoteFrame::ProgressParcentProgress(num, false);
		}
		SubsTime progressTime;
		progressTime.NewTime(newtime);
		text1->SetLabelText(wxString::Format(_("Upłynęło %s sekund"), progressTime.raw()));
	}
	//bool main =wxThread::IsMain();
	//if(!main){
	//wxSafeYield(this);
	//}
	oldtime = newtime;

}

void ProgresDialog::Title(wxString title)
{
	text->SetLabelText(title);
	Layout();
}

bool ProgresDialog::WasCancelled()
{
	return canceled;
}

void ProgresDialog::OnCancel(wxCommandEvent& event)
{
	canceled = true;
	EndFrameProgress();
	Hide();
}

void ProgresDialog::OnShow(wxThreadEvent& evt)
{
	CenterOnParent();
	ShowModal();
}

void ProgresDialog::OnProgress(wxThreadEvent& evt)
{
	Progress(evt.GetPayload<int>());
}

void ProgresDialog::OnTitle(wxThreadEvent& evt)
{

	Title(evt.GetPayload<wxString>());
}

//use in main thread
ProgressSink::ProgressSink(wxWindow *parent, const wxString &title, const wxPoint &pos, const wxSize &size, int style)
	: wxThread(wxTHREAD_JOINABLE)
{
	dlg = new ProgresDialog(parent, title, pos, size, style | wxBORDER_NONE);
}

ProgressSink::~ProgressSink()
{
	if (dlg)
		dlg->Destroy();
}
//shows dialog, use only from main thread
void ProgressSink::ShowDialog()
{
	if (dlg)
		dlg->ShowModal();
}

// set name of current task
void ProgressSink::Title(wxString title)
{
	if (!dlg)
		return;
	wxThreadEvent *evt = new wxThreadEvent(EVT_SET_TITLE, dlg->GetId());
	evt->SetPayload(title);
	wxQueueEvent(dlg.get(), evt);
}

bool ProgressSink::WasCancelled()
{
	return !dlg || dlg->WasCancelled();
}

void ProgressSink::Progress(int num)
{
	if (!dlg)
		return;
	wxThreadEvent *evt = new wxThreadEvent(EVT_SET_PROGRESS, dlg->GetId());
	evt->SetPayload(num);
	wxQueueEvent(dlg.get(), evt);
}
void ProgressSink::EndModal()
{
	if (!dlg)
		return;
	wxThreadEvent *evt = new wxThreadEvent(EVT_END_MODAL, dlg->GetId());
	wxQueueEvent(dlg.get(), evt);
}

wxThread::ExitCode ProgressSink::Entry()
{
	long long result = task();
	EndModal();
	return (wxThread::ExitCode)result;
}

int ProgressSink::ShowSecondaryDialog(std::function<int()> dialfunction){
	if (!dlg)
		return wxID_CANCEL;
	wxThreadEvent *evt = new wxThreadEvent(EVT_CREATE_SECONDARY_DIALOG, dlg->GetId());
	wxSemaphore sema(0, 1);
	std::pair<std::function<int()>, wxSemaphore*> pair(dialfunction, &sema);
	evt->SetPayload(pair);
	wxQueueEvent(dlg.get(), evt);
	sema.Wait();
	return dlg ? dlg->result : wxID_CANCEL;
}

ProgressSinkSilent::ProgressSinkSilent(const wxString& _title)
{
	KainoteFrame::ProgressSetupEvent(_title);
}

ProgressSinkSilent::~ProgressSinkSilent()
{
	if (!hasProgressEnded) {
		KainoteFrame::ProgressEndEvent();
	}
}

void ProgressSinkSilent::Title(wxString _title)
{
	KainoteFrame::ProgressTitleEvent(_title);
}

bool ProgressSinkSilent::WasCancelled()
{
	return false;
}

void ProgressSinkSilent::Progress(int num)
{
	KainoteFrame::ProgressParcentProgressEvent(num);
}

void ProgressSinkSilent::EndModal()
{
	KainoteFrame::ProgressEndEvent();
	hasProgressEnded = true;
}

