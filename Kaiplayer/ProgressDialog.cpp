
#include "ProgressDialog.h"
#include "kainoteApp.h"
//#include "timeconv.h"
//#define UNICODE
#include "ShObjIdl.h"


ProgresDialog::ProgresDialog(wxWindow *_parent, wxString title, wxPoint pos, wxSize size, int style)
	: wxDialog(_parent,-1,"",pos,size,style)
{
	RegisterWindowMessage ( L"TaskbarButtonCreated" );
	taskbar=NULL;
	wxBoxSizer* sizer= new wxBoxSizer(wxVERTICAL);
	text=new wxStaticText(this,-1,title);
	gauge=new wxGauge(this, -1, 100, wxDefaultPosition, wxSize(300,20), wxGA_HORIZONTAL);
	text1=new wxStaticText(this,-1,_("Upłynęło 00:00:00.00 sekund"));
	cancel= new wxButton(this,23333,_("Anuluj"));
	sizer->Add(text,0,wxALIGN_CENTER|wxALL, 3);//wxALIGN_CENTER|
	sizer->Add(gauge,0,wxALIGN_CENTER|wxALL, 3);
	sizer->Add(text1,0,wxALIGN_CENTER|wxALL, 3);
	sizer->Add(cancel,0,wxALIGN_CENTER|wxALL, 3);
	
	Connect(23333,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ProgresDialog::OnCancel);
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
		//wxLogStatus("taskbar %i", (int)taskbar);
		kainoteApp * Kaia = (kainoteApp *)wxTheApp;
		if(taskbar){taskbar->SetProgressState(Kaia->Frame->GetHWND(),TBPF_NORMAL);taskbar->SetProgressValue(Kaia->Frame->GetHWND(),0,100);}
    }
	Show();
	//bool main =wxThread::IsMain();
	//if(!main){
	wxSafeYield(this);
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
	wxSafeYield(this);
	//}
	oldtime=newtime;
	
}

void ProgresDialog::Title(wxString title)
{
	text->SetLabelText(title);
	//bool main =wxThread::IsMain();
	//if(!main){
	wxSafeYield(this);
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
