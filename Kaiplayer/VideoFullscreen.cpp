
#include "VideoFullscreen.h"
#include "Videobox.h"
#include "Config.h"


Fullscreen::Fullscreen(wxWindow* parent, const wxPoint& pos, const wxSize &size)
     : wxFrame(parent, -1,"", pos, size, wxBORDER_NONE|wxSTAY_ON_TOP)//
{
    vb=parent;
	VideoCtrl *vc = (VideoCtrl*)parent;
	SetBackgroundColour("#000000");
	this->SetEventHandler(parent);
	if(!vc->IsDshow){panelsize = 66;}else{panelsize = 44;}
	panel=new wxPanel(this,-1,wxPoint(0,size.y - panelsize),wxSize(size.x,panelsize));
	panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR));
	vslider= new VideoSlider(panel, ID_SLIDER,wxPoint(0,1),wxSize(size.x,14));
	vslider->VB= vc;
	bprev = new BitmapButton(panel,CreateBitmapFromPngResource("backward"),CreateBitmapFromPngResource("backward1"), ID_BPREV, wxPoint(5,16), wxSize(26,26));
	bpause = new BitmapButton(panel, CreateBitmapFromPngResource("play"),CreateBitmapFromPngResource("play1"),ID_BPAUSE, wxPoint(40,16), wxSize(26,26));
	bpline = new BitmapButton(panel, CreateBitmapFromPngResource("playline"), CreateBitmapFromPngResource("playline1"),ID_BPLINE, wxPoint(75,16), wxSize(26,26));
	bstop = new BitmapButton(panel, CreateBitmapFromPngResource("stop"),CreateBitmapFromPngResource("stop1"),ID_BSTOP, wxPoint(110,16), wxSize(26,26));
	bnext = new BitmapButton(panel, CreateBitmapFromPngResource("forward"), CreateBitmapFromPngResource("forward1"),ID_BNEXT, wxPoint(145,16), wxSize(26,26));
	volslider=new VolSlider(panel,ID_VOL,Options.GetInt("Video Volume"),wxPoint(size.x-110,17),wxSize(110,25));
	wxCheckBox *showToolbar = new wxCheckBox(panel,7777,_("Pokaż pasek narzędzi"), wxPoint(180,21));
	showToolbar->SetValue(!vc->IsDshow);
	Videolabel=new wxStaticText(panel,-1,"",wxPoint(340,21));
	panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR));
	vToolbar = new VideoToolbar(panel,wxPoint(0, 44));
	vToolbar->SetSize(wxSize(size.x, 22));
	vToolbar->Show(!vc->IsDshow);
	showToolbar->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [=](wxCommandEvent &evt){
		bool show = showToolbar->GetValue();
		vToolbar->Show(show); 
		if(show){panelsize = 66;}else{panelsize = 44;}
		OnSize();
	},7777);
	Connect(ID_BPREV,ID_BNEXT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoCtrl::OnVButton);
	Connect(ID_VOL,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VideoCtrl::OnVolume);
	//Connect(wxEVT_SIZE, (wxObjectEventFunction)&Fullscreen::OnSize);
}


Fullscreen::~Fullscreen()
{
	//PopEventHandler();
	this->SetEventHandler(this);
    //Zwolnić event handler
}

void Fullscreen::OnSize()
{
	wxSize asize = GetClientSize();
	VideoCtrl *vc = (VideoCtrl*)vb;
	//if(vc->lastSize == asize){return;}
	vc->lastSize=asize;
	
	vslider->SetSize(wxSize(asize.x,14));
	volslider->SetPosition(wxPoint(asize.x-110,17));
	Videolabel->SetSize(asize.x-300,-1);
	if(vToolbar->IsShown()){vToolbar->SetSize(asize.x, 22);}
	panel->SetSize(0, asize.y - panelsize, asize.x, panelsize);
}