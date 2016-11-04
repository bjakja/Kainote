
#include "TabPanel.h"
#include "Config.h"
#include "Hotkeys.h"
#include "KainoteMain.h"


TabPanel::TabPanel(wxWindow *parent,kainoteFrame *kai, const wxPoint &pos, const wxSize &size)
	: wxWindow(parent,-1, pos, size)
	,sline(NULL)
	,edytor(true)
	,holding(false)
{
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	int vw,vh;
	Options.GetCoords("Video Window Size",&vw,&vh);
	if(vw<200){vw=550;vh=400;}
    Video = new VideoCtrl(this, kai, wxSize(vw,vh));
	Video->Hide();
    Grid1 = new Grid(this,kai,-1,wxDefaultPosition,wxSize(400,200),wxSUNKEN_BORDER|wxWANTS_CHARS);//
    Edit = new EditBox(this, Grid1, kai, -1);
	Edit->SetMinSize(wxSize(-1,200));
	Edit->SetIt(0);
	
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    CTime= new CTwindow(this,kai,-1,wxDefaultPosition,wxDefaultSize,wxSUNKEN_BORDER);
    CTime->Show(Options.GetBool("Show Change Time"));
	BoxSizer3->Add(Grid1, 1, wxEXPAND, 0);
	BoxSizer3->Add(CTime, 0, wxEXPAND, 0);
    BoxSizer2->Add(Video, 0, wxEXPAND|wxALIGN_TOP, 0);
    BoxSizer2->Add(Edit, 1, wxEXPAND|wxALIGN_TOP, 0);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    BoxSizer1->Add(BoxSizer2, 0, wxEXPAND|wxALIGN_TOP, 0);
	BoxSizer1->AddSpacer(3);
    BoxSizer1->Add(BoxSizer3, 1, wxEXPAND, 0);
    SetSizerAndFit(BoxSizer1);

	SubsName=_("Bez tytułu");
	/*VideoName="";
	SubsPath="";
	VideoPath="";
*/
	
	SetAccels();
}


TabPanel::~TabPanel(){
	if(Video->player){Video->player->Stop();} //fix kraszów powodowanych przez niszczenie editboxa na samym końcu i próbując pobrać audio na play kraszuje.
}


void TabPanel::SetAccels()
{

	std::vector<wxAcceleratorEntry> ventries;
   
	std::vector<wxAcceleratorEntry> gentries;
	gentries.resize(3);
    gentries[0].Set(wxACCEL_CTRL, (int) 'X', Cut);
    gentries[1].Set(wxACCEL_CTRL, (int) 'C', Copy);
    gentries[2].Set(wxACCEL_CTRL, (int) 'V', Paste);

	std::vector<wxAcceleratorEntry> eentries;
	eentries.resize(2);
	eentries[0].Set(wxACCEL_CTRL, WXK_NUMPAD_ENTER, MENU_ZATW);
    eentries[1].Set(wxACCEL_NORMAL, WXK_NUMPAD_ENTER, MENU_NLINE);
	
	for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++){
		int id=cur->first.id;
		if(cur->second.Accel=="" || cur->first.Type == 'A' || cur->first.Type == 'G'){continue;}
		//editor
		if(cur->first.Type == 'E'){
			eentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}else if(cur->first.Type == 'N'){//grid
			gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if(id>5000 &&id<=6000){Grid1->ConnectAcc(id);}
		}else if(cur->first.Type == 'W'){//video
			ventries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if(id>2000 && id<3990){Video->ConnectAcc(id);}
			if(id>=PlayPause && id<= Minus5Second){
				gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				Grid1->ConnectAcc(id);
			}
		} 

	}
	/*for(int i=PlayPause; i<= Minus5Second; i++){
		idAndType itype(i, 'N');
		gentries.push_back(Hkeys.GetHKey(&itype));
		Grid1->ConnectAcc(i);
	}*/
	wxAcceleratorTable accelg(gentries.size(), &gentries[0]);
    Grid1->SetAcceleratorTable(accelg);
	wxAcceleratorTable accelv(ventries.size(), &ventries[0]);
    Video->SetAcceleratorTable(accelv);
    wxAcceleratorTable accele(eentries.size(), &eentries[0]);
    Edit->SetAcceleratorTable(accele);
	
}

void TabPanel::OnMouseEvent(wxMouseEvent& event)
	{
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();

	if(event.Leaving()){
		SetCursor(wxCURSOR_ARROW);}
	else{
		SetCursor(wxCURSOR_SIZENS);}
	

	if (left_up && holding) {
		ReleaseMouse();
		holding = false;
		int npos=event.GetY();
		if(sline){
			int x; 
			sline->GetPosition(&x,&npos);
			ScreenToClient(&x,&npos);
			sline->Destroy();
			sline=NULL;
		}
		int w,h, mw, mh;
		Video->GetClientSize(&w,&h);
		GetClientSize(&mw,&mh);
		if(npos>=mh){npos=mh-3;}
		
		if(Video->GetState()!=None&&Video->IsShown()){
			
			int ww,hh;
			Video->CalcSize(&ww,&hh,w,npos,false,true);
			Video->SetMinSize(wxSize(ww,hh+Video->panelHeight));
			Options.SetCoords("Video Window Size",ww,hh+Video->panelHeight);
		}else{Edit->SetMinSize(wxSize(w+(npos-h),npos));}
		BoxSizer1->Layout();
		//if(Video->GetState()==Paused){Video->Render();}
		
	}

	if (left_up && !holding) {
		return;
	}

	if (click) {
		holding = true;
		CaptureMouse();
		int px=2, py=event.GetY();
		ClientToScreen(&px,&py);
		sline= new wxDialog(this,-1,"",wxPoint(px,py),wxSize(GetSize().GetWidth(),3),wxSTAY_ON_TOP|wxBORDER_NONE);
		sline->SetBackgroundColour("#000000");
		sline->Show();
	}

	if (holding){
		int npos=event.GetY();
		int w=0,h=0;
		Video->GetClientSize(&w,&h);
		int limit=(Video->GetState()!=None&&Video->IsShown())? 350 : 150;
		if(npos!=h&&npos>limit){
			int px=2, py=npos;
			ClientToScreen(&px,&py);
			sline->SetPosition(wxPoint(px,py));
		}
			
	}
}

void TabPanel::OnFocus(wxChildFocusEvent& event)
{
	Notebook *nt=Notebook::GetTabs();
	//if(!nt){return;}
	if(!nt->split){return;}

	if(nt->GetTab()!=this)
	{
		nt->ChangeActiv();
	}
}


BEGIN_EVENT_TABLE(TabPanel,wxWindow)
     EVT_MOUSE_EVENTS(TabPanel::OnMouseEvent)
	 EVT_CHILD_FOCUS(TabPanel::OnFocus)
END_EVENT_TABLE()
