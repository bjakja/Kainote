
#include "VideoFullscreen.h"
#include "Videobox.h"
#include "config.h"


Fullscreen::Fullscreen(wxWindow* parent, const wxPoint& pos, const wxSize &size)
              : wxFrame(parent, -1,"", pos, size, wxBORDER_NONE|wxSTAY_ON_TOP)
{
    vb=parent;
	SetBackgroundColour("#000000");
	this->SetEventHandler(parent);
	panel=new wxPanel(this,-1,wxPoint(0,size.y-44),wxSize(size.x,44));
	panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	
	vslider= new VideoSlider(panel, ID_SLIDER,wxPoint(0,1),wxSize(size.x,14));
	vslider->VB=(VideoCtrl*)parent;
	bprev = new BitmapButton(panel,CreateBitmapFromPngResource("backward"),CreateBitmapFromPngResource("backward1"), ID_BPREV, wxPoint(5,16), wxSize(26,26));
	bpause = new BitmapButton(panel, CreateBitmapFromPngResource("play"),CreateBitmapFromPngResource("play1"),ID_BPAUSE, wxPoint(40,16), wxSize(26,26));
	bstop = new BitmapButton(panel, CreateBitmapFromPngResource("stop"),CreateBitmapFromPngResource("stop1"),ID_BSTOP, wxPoint(75,16), wxSize(26,26));
	bnext = new BitmapButton(panel, CreateBitmapFromPngResource("forward"), CreateBitmapFromPngResource("forward1"),ID_BNEXT, wxPoint(110,16), wxSize(26,26));
	volslider=new VolSlider(panel,ID_VOL,Options.GetInt("Video Volume"),wxPoint(size.x-110,17),wxSize(110,25));
	Videolabel=new wxStaticText(panel,-1,"",wxPoint(145,21));
	//Videolabel->SetForegroundColour(wxColour("#FFFFFF"));
	//Videolabel->SetBackgroundColour(wxColour("#808080"));
	Connect(ID_BPREV,ID_BNEXT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoCtrl::OnVButton);
	Connect(ID_VOL,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VideoCtrl::OnVolume);
}


Fullscreen::~Fullscreen()
{
	//PopEventHandler();
	this->SetEventHandler(this);
    //Zwolniæ event handler
}
