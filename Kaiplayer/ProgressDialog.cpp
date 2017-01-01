//  Copyright (c) 2016, Marcin Drob

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
#include "kainoteApp.h"
#include "ShObjIdl.h"

wxDEFINE_EVENT(EVT_SHOW_DIALOG, wxThreadEvent);
wxDEFINE_EVENT(EVT_SET_TITLE, wxThreadEvent);
wxDEFINE_EVENT(EVT_SET_PROGRESS, wxThreadEvent);
wxDEFINE_EVENT(EVT_CREATE_SECONDARY_DIALOG, wxThreadEvent);
wxDEFINE_EVENT(EVT_END_MODAL, wxThreadEvent);

ProgresDialog::ProgresDialog(wxWindow *_parent, const wxString &title, const wxPoint &pos, const wxSize &size, int style)
	: wxDialog(_parent,31555,"",pos,size,style)
{
	RegisterWindowMessage ( L"TaskbarButtonCreated" );
	SetForegroundColour(Options.GetColour("Window Text"));
	SetBackgroundColour(Options.GetColour("Window Background"));
	taskbar=NULL;
	wxBoxSizer* sizer= new wxBoxSizer(wxVERTICAL);
	text=new wxStaticText(this,-1,title);
	gauge=new wxGauge(this, -1, 100, wxDefaultPosition, wxSize(300,20), wxGA_HORIZONTAL);
	text1=new wxStaticText(this,-1,_("Upłynęło 00:00:00.00 sekund"));
	cancel= new MappedButton(this,23333,_("Anuluj"));
	sizer->Add(text,0,wxALIGN_CENTER|wxALL, 3);//wxALIGN_CENTER|
	sizer->Add(gauge,0,wxALIGN_CENTER|wxALL, 3);
	sizer->Add(text1,0,wxALIGN_CENTER|wxALL, 3);
	sizer->Add(cancel,0,wxALIGN_CENTER|wxALL, 3);
	
	Connect(23333,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ProgresDialog::OnCancel);
	Bind(EVT_SHOW_DIALOG,&ProgresDialog::OnShow, this);
	Bind(EVT_SET_PROGRESS,&ProgresDialog::OnProgress, this);
	Bind(EVT_SET_TITLE, &ProgresDialog::OnTitle, this);
	Bind(EVT_CREATE_SECONDARY_DIALOG, [=](wxThreadEvent &evt){
		std::pair<std::function<int()>,wxSemaphore*> pair = evt.GetPayload<std::pair<std::function<int()>,wxSemaphore*>>(); 
		wxSemaphore* sema = pair.second;
		std::function<int()> showDial = pair.first;
		result = showDial();
		sema->Post();
	});
	Bind(EVT_END_MODAL, [=](wxThreadEvent &evt){
		if(IsModal()){EndModal(wxID_OK);}
		else{Hide();}
	});
	firsttime=timeGetTime();
	canceled=false;
	SetSizerAndFit(sizer);
	CenterOnParent();
	oldtime=0;
	DWORD dwMajor = LOBYTE(LOWORD(GetVersion()));
	DWORD dwMinor = HIBYTE(LOWORD(GetVersion()));
	if ( dwMajor > 6 || ( dwMajor == 6 && dwMinor > 0 ) )
    {
        CoCreateInstance ( CLSID_TaskbarList ,0,CLSCTX_INPROC_SERVER,__uuidof(ITaskbarList3),(void**)&taskbar);
		kainoteApp * Kaia = (kainoteApp *)wxTheApp;
		if(taskbar){taskbar->SetProgressState(Kaia->Frame->GetHWND(),TBPF_NORMAL);taskbar->SetProgressValue(Kaia->Frame->GetHWND(),0,100);}
    }

	//if(!main){
	//wxSafeYield(this);
	//}
}


ProgresDialog::~ProgresDialog()
{
	if(taskbar){
		kainoteApp * Kaia = (kainoteApp *)wxTheApp;
		taskbar->SetProgressState(Kaia->Frame->GetHWND(),TBPF_NOPROGRESS);}
}

void ProgresDialog::Progress(int num)
{
	
	int newtime = timeGetTime()-firsttime;
	if(oldtime+5<newtime){
		gauge->SetValue(num);
		
		if(taskbar){
			kainoteApp * Kaia = (kainoteApp *)wxTheApp;
			taskbar->SetProgressValue(Kaia->Frame->GetHWND(),(ULONGLONG)num,100);}
		STime kkk;
		kkk.NewTime(newtime);
		text1->SetLabelText(wxString::Format(_("Upłynęło %s sekund"), kkk.raw()));
		
	}
	//bool main =wxThread::IsMain();
	//if(!main){
	//wxSafeYield(this);
	//}
	oldtime=newtime;
	
}

void ProgresDialog::Title(wxString title)
{
	text->SetLabelText(title);
	//bool main =wxThread::IsMain();
	//if(!main){
	//wxSafeYield(this);
	//}
}

bool ProgresDialog::WasCancelled()
{
	return canceled;
}

void ProgresDialog::OnCancel(wxCommandEvent& event)
{
	canceled=true;
	if(taskbar){
		kainoteApp * Kaia = (kainoteApp *)wxTheApp;
		taskbar->SetProgressState(Kaia->Frame->GetHWND(),TBPF_NOPROGRESS);}
	Hide();
}

void ProgresDialog::OnShow(wxThreadEvent& evt)
{
	bool main =wxThread::IsMain();
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

//lepiej wygonać w main thread
//resztę funkcji można wykonyać w innym watku
ProgressSink::ProgressSink(wxWindow *parent, const wxString &title, const wxPoint &pos, const wxSize &size, int style)
	:wxThread(wxTHREAD_JOINABLE)
{
	dlg = new ProgresDialog(parent, title, pos, size, style);
}
	
ProgressSink::~ProgressSink()
{
	dlg->Destroy();
}
//pokazuje nasz dialog, nie pokazywać z innego wątku niż głowny
void ProgressSink::ShowDialog()
{
	//wxThreadEvent *evt = new wxThreadEvent(EVT_SET_TITLE, dlg->GetId());
	//wxQueueEvent(dlg, evt);
	dlg->ShowModal();
}

// ustawia nazwę obecnego zadania
void ProgressSink::Title(wxString title)
{
	wxThreadEvent *evt = new wxThreadEvent(EVT_SET_TITLE, dlg->GetId());
	evt->SetPayload(title);
	wxQueueEvent(dlg, evt);
}
	
bool ProgressSink::WasCancelled()
{
	return dlg->WasCancelled();
}
	
void ProgressSink::Progress(int num)
{
	wxThreadEvent *evt = new wxThreadEvent(EVT_SET_PROGRESS, dlg->GetId());
	evt->SetPayload(num);
	wxQueueEvent(dlg,evt);
}
void ProgressSink::EndModal()
{
	wxThreadEvent *evt = new wxThreadEvent(EVT_END_MODAL, dlg->GetId());
	wxQueueEvent(dlg,evt);
}

wxThread::ExitCode ProgressSink::Entry()
{
	int result = task();
	EndModal();
	return (wxThread::ExitCode)result;
}

int ProgressSink::ShowSecondaryDialog(std::function<int()> dialfunction){
	wxThreadEvent *evt = new wxThreadEvent(EVT_CREATE_SECONDARY_DIALOG, dlg->GetId());
	wxSemaphore sema(0,1);
	std::pair<std::function<int()>,wxSemaphore*> pair(dialfunction, &sema);
	evt->SetPayload(pair);
	wxQueueEvent(dlg, evt);
	sema.Wait();
	return dlg->result;
}