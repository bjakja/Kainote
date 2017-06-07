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
	SetBackgroundColour(Options.GetColour(WindowBackground));
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	int vw,vh;
	Options.GetCoords(VideoWindowSize,&vw,&vh);
	if(vw<200){vw=550;vh=400;}
    Video = new VideoCtrl(this, kai, wxSize(vw,vh));
	Video->Hide();
    Grid1 = new Grid(this,kai,-1,wxDefaultPosition,wxSize(400,200),wxBORDER_SIMPLE|wxWANTS_CHARS);//
    Edit = new EditBox(this, Grid1, kai, -1);
	Edit->SetMinSize(wxSize(-1,200));
	Edit->SetLine(0);
	
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    CTime= new CTwindow(this,kai,-1,wxDefaultPosition,wxDefaultSize,wxBORDER_SIMPLE);
    CTime->Show(Options.GetBool(MoveTimesOn));
	BoxSizer3->Add(Grid1, 1, wxEXPAND, 0);
	BoxSizer3->Add(CTime, 0, wxEXPAND|wxRIGHT, 2);
    BoxSizer2->Add(Video, 0, wxEXPAND|wxALIGN_TOP, 0);
    BoxSizer2->Add(Edit, 1, wxEXPAND|wxALIGN_TOP, 0);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    BoxSizer1->Add(BoxSizer2, 0, wxEXPAND|wxALIGN_TOP, 0);
	BoxSizer1->AddSpacer(3);
    BoxSizer1->Add(BoxSizer3, 1, wxEXPAND, 0);
    SetSizerAndFit(BoxSizer1);

	SubsName=_("Bez tytułu");
	
	SetAccels();
}


TabPanel::~TabPanel(){
	if(Video->player){Video->player->Stop(false);} //fix kraszów powodowanych przez niszczenie editboxa na samym końcu i próbując pobrać audio na play kraszuje.
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
	eentries[0].Set(wxACCEL_CTRL, WXK_NUMPAD_ENTER, MENU_COMMIT);
    eentries[1].Set(wxACCEL_NORMAL, WXK_NUMPAD_ENTER, MENU_NEWLINE);
	
	for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++){
		int id=cur->first.id;
		if(cur->second.Accel=="" || cur->first.Type == AUDIO_HOTKEY || cur->first.Type == GLOBAL_HOTKEY){continue;}
		//editor
		if(cur->first.Type == EDITBOX_HOTKEY){
			eentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}else if(cur->first.Type == GRID_HOTKEY){//grid
			gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if(id>5000 &&id<=6000){Grid1->ConnectAcc(id);}
		}else if(cur->first.Type == VIDEO_HOTKEY){//video
			ventries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
			if(id>2000 && id<3990){Video->ConnectAcc(id);}
			if(id>=PlayPause && id<= Minus5Second){
				gentries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
				Grid1->ConnectAcc(id);
			}
		} 

	}
	
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
			Options.SetCoords(VideoWindowSize,ww,hh+Video->panelHeight);
		}else{Edit->SetMinSize(wxSize(-1,npos));}
		BoxSizer1->Layout();
		if(event.ShiftDown()){
			SetVideoWindowSizes(w, npos);
		}
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

void TabPanel::SetVideoWindowSizes(int w, int h)
{
	Notebook *nb = Notebook::GetTabs(); 
	for(size_t i = 0; i < nb->Size(); i++){
		if(i == nb->iter){continue;}
		TabPanel *tab = nb->Page(i);
		if(tab->Video->GetState()!=None && tab->Video->IsShown()){
			int ww,hh;
			tab->Video->CalcSize(&ww, &hh, w, h, false, true);
			tab->Video->SetMinSize(wxSize(ww,hh+tab->Video->panelHeight));
		}else{tab->Edit->SetMinSize(wxSize(-1, h));}
		tab->BoxSizer1->Layout();
	}
}

BEGIN_EVENT_TABLE(TabPanel,wxWindow)
     EVT_MOUSE_EVENTS(TabPanel::OnMouseEvent)
	 EVT_CHILD_FOCUS(TabPanel::OnFocus)
END_EVENT_TABLE()
