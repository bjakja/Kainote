#include "wx_pch.h"
#include "NewDialog.h"
#include "config.h"
#include "Stylelistbox.h"
#include <wx/intl.h>
#include <wx/string.h>
#include "kainoteMain.h"	


CTwindow::CTwindow(wxWindow* parent,kainoteFrame* kfparent,wxWindowID id,const wxPoint& pos,const wxSize& size,long style)
	: wxWindow(parent, id, pos, size, style|=wxVSCROLL, _T("id"))
{
    Kai=kfparent;
    form=Kai->Grid1->form;
    bool state;
	scpos=0;
	//SetMinSize(wxSize(210,360));
	wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_NORMAL, WXK_RETURN, ID_MOVE);
    entries[1].Set(wxACCEL_NORMAL, WXK_ESCAPE, ID_CLOSE);
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);
	int w,h,/*sw,sh,*/gw,gh;
	GetClientSize(&w,&h);
	Kai->Grid1->GetClientSize(&gw,&gh);
	scrollBar = new wxScrollBar(this,ID_SCROLL,wxDefaultPosition,wxDefaultSize,wxSB_VERTICAL);
	if(gh<358){
	scrollBar->SetScrollbar(0,gh/4,357/4,(gh/4)-1);
		}else{scrollBar->Enable(false);}
    //scrollBar->GetSize(&sw,&sh);
    scrollBar->SetSize(w-18,0,18,gh);
	panel=new wxPanel(this,-5,0,w-18,(gh>h)?gh:357);//(360-gh>0)?
	SetClientSize(w,gh);
	wxFont thisFont(8,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
    panel->SetFont(thisFont);
	Button2 = new wxButton(panel, ID_MOVE, _("Przesuñ"), wxPoint(16,320), wxSize(70,-1));
	Button3 = new wxButton(panel, ID_CLOSE, _("Zamknij"), wxPoint(108,320), wxSize(70,-1));
	timetext = new TimeCtrl(panel, -1, _("0:00:00.00"), wxPoint(32,16), wxDefaultSize, wxTE_PROCESS_ENTER);
	timetext->Bind(wxEVT_MOUSEWHEEL,&CTwindow::OnMouseEvent,this);
	RadioButton1 = new wxRadioButton(panel, ID_RADIOBUTTON1, _("Czas"), wxPoint(32,40), wxDefaultSize, wxRB_GROUP);
	//RadioButton1->SetValue(true);
	RadioButton2 = new wxRadioButton(panel, ID_RADIOBUTTON2, _("Klatki"), wxPoint(104,40));
	RadioButton11 = new wxRadioButton(panel, -1, _("W przód"), wxPoint(32,56), wxDefaultSize, wxRB_GROUP);
	//RadioButton11->SetValue(true);
	RadioButton12 = new wxRadioButton(panel, -1, _("W ty³"), wxPoint(104,56));
	RadioButton3 = new wxRadioButton(panel, -1, _("Wszystkie linijki"), wxPoint(32,128), wxDefaultSize, wxRB_GROUP);
	//RadioButton3->SetValue(true);
	RadioButton4 = new wxRadioButton(panel, -1, _("Zaznaczone linijki"), wxPoint(32,144));
	RadioButton5 = new wxRadioButton(panel, -1, _("Od zaznaczonej linijki"), wxPoint(32,160));
	if(form==_("ass")){state=true;}else{state=false;}
	RadioButton6 = new wxRadioButton(panel, -1, _("Czasy wy¿sze i równe"), wxPoint(32,176));
	RadioButton6->Enable(state);
	RadioButton7 = new wxRadioButton(panel, -1, _("Wed³ug wybranych styli"), wxPoint(32,192));
	RadioButton7->Enable(state);
	Button1 = new wxButton(panel, style, _("+"), wxPoint(32,208), wxSize(25,22));
	Button1->Enable(state);
	TextCtrl2 = new wxTextCtrl(panel, -1, wxEmptyString, wxPoint(64,208), wxSize(110,22), wxTE_PROCESS_ENTER);
	TextCtrl2->Enable(state);
	StaticBox1 = new wxStaticBox(panel, -1, _("Linijki"), wxPoint(16,80), wxSize(170,160));
	if(form!=_("tmp")){state=true;}else{state=false;}
	RadioButton8 = new wxRadioButton(panel, -1, _("Obydwa czasy"), wxPoint(32,256), wxDefaultSize, wxRB_GROUP);
	RadioButton8->Enable(state);
	RadioButton9 = new wxRadioButton(panel, -1, _("Czas pocz¹tkowy"), wxPoint(32,272));
	RadioButton9->Enable(state);
	RadioButton10 = new wxRadioButton(panel, -1, _("Czas koñcowy"), wxPoint(32,288));
	RadioButton10->Enable(state);
	StaticBox2 = new wxStaticBox(panel, -1, _("Czas"), wxPoint(16,0), wxSize(170,80));
	StaticBox3 = new wxStaticBox(panel, -1, _("Sposób zmiany czasów"), wxPoint(16,240), wxSize(170,72));
	videotime = new wxCheckBox(panel, -1, _("Przesuñ pierwsz¹ zazn.\nlinijkê do czasu wideo"), wxPoint(26,96));
	videotime->SetForegroundColour(*wxRED);
	if(Kai->Video->GetState()!=STATE_CLOSED){state=true;}else{state=false;}
	videotime->Enable(state);
	
	Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&CTwindow::OnRadioButton1Select);
	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&CTwindow::OnRadioButton2Select);
	
	
	Connect(ID_MOVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::OnOKClick);
	Connect(ID_CLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::OnOKClick);
     if(form==_("ass")){
	Connect(style,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::OnButton1Click);
	}
     ct.mstime=Options.GetInt(_("Change Time"));
     if(Options.GetBool(_("Frames"))){RadioButton2->SetValue(true);ct.SetFormat(MDVD);}else{RadioButton1->SetValue(true);ct.SetFormat(ASS);}

   timetext->SetTime(ct);
   videotime->SetValue(Options.GetBool(_("Move to video time")));
   if(Options.GetBool(_("Move time forward"))){RadioButton11->SetValue(true);}else{RadioButton12->SetValue(true);}
   int cm=Options.GetInt(_("Change mode"));
   if(cm==1){RadioButton4->SetValue(true);}else if(cm==2){RadioButton5->SetValue(true);}else if(cm==3&&form==_("ass")){RadioButton6->SetValue(true);}
   else if(cm==4&&form==_("ass")){RadioButton7->SetValue(true);}else{RadioButton3->SetValue(true);};
   if(form==_("ass")){
   TextCtrl2->SetValue(Options.GetString(_("Styles of time change")));
   }
   int sset=Options.GetInt(_("Start end times"));
   if(sset==0){RadioButton8->SetValue(true);}else if(sset==1){RadioButton9->SetValue(true);}else{RadioButton10->SetValue(true);};
   
   DoTooltips();
}

CTwindow::~CTwindow()
{
	
}
void CTwindow::Contents()
	{
	bool state;
	form=Kai->Grid1->form;
	if(form==_("ass")){state=true;}else{state=false;}
	RadioButton6->Enable(state);
	RadioButton7->Enable(state);
	Button1->Enable(state);
	TextCtrl2->Enable(state);
	if(form!=_("tmp")){state=true;}else{state=false;}
	RadioButton8->Enable(state);
	RadioButton9->Enable(state);
	RadioButton10->Enable(state);
	if(Kai->Video->GetState()!=STATE_CLOSED){state=true;}else{state=false;}
	videotime->Enable(state);
	}

void CTwindow::OnRadioButton1Select(wxCommandEvent& event)
{
    timetext->ChangeFormat(ASS);
}

void CTwindow::OnRadioButton2Select(wxCommandEvent& event)
{
    timetext->ChangeFormat(MDVD,Kai->Video->fps);
}

void CTwindow::OnButton1Click(wxCommandEvent& event)
{
wxString kkk=Kai->sftc();
TextCtrl2->SetValue(kkk);
}

void CTwindow::OnOKClick(wxCommandEvent& event)
{

    ct=timetext->GetTime();
    Options.SetInt(_("Change Time"),ct.mstime);

 if(form==_("ass")){
 wxString sstyles=TextCtrl2->GetValue();
  Options.SetString(_("Styles of time change"),sstyles);
 }

 bool fram=(RadioButton2->GetValue()==true)?true:false;
 bool prz=(RadioButton11->GetValue()==true)?true:false;
 Options.SetBool(_("Frames"),fram);
 Options.SetBool(_("Move time forward"),prz);
 Options.SetBool(_("Move to video time"),videotime->GetValue());
 int lmd;
if(RadioButton3->GetValue()){lmd=0;}else if(RadioButton4->GetValue()){lmd=1;}else if(RadioButton5->GetValue()){lmd=2;}else if(form==_("ass")){if(RadioButton6->GetValue()){lmd=3;}else{lmd=4;};}else{lmd=0;}
 int seb;
 //if(form!=_("tmp")){
if(RadioButton8->GetValue()){seb=0;}else if(RadioButton9->GetValue()){seb=1;}else{seb=2;};
 //}else{seb=1;}
 Options.SetInt(_("Change mode"),lmd);
 Options.SetInt(_("Start end times"),seb);
 int acid=event.GetId();
 if (acid==ID_MOVE){
 Kai->Grid1->ChangeTime();
 wxBell();}
 else if(acid==ID_CLOSE){
Hide();
Kai->BoxSizer1->Layout();}
Kai->Grid1->SetFocus();
}


void CTwindow::OnSize(wxSizeEvent& event)
	{
	int w,h,/*sw,sh,*/gw,gh,px,py;
	panel->GetClientSize(&w,&h);
	panel->GetPosition(&px,&py);
	Kai->Grid1->GetClientSize(&gw,&gh);
	
	if(gh<358){
	if(!scrollBar->IsEnabled()){
		scrollBar->Enable();}
	if(py<0){int npos=py+4;scpos-=1;panel->SetPosition(wxPoint(px,npos));}
	scrollBar->SetScrollbar(scpos,gh/4,357/4,(gh/4)-1);
		}else{if(scrollBar->IsEnabled()){scrollBar->Enable(false);scpos=0;}}
	//scrollBar->GetSize(&sw,&sh);
    scrollBar->SetSize(w,0,18,gh);
	if(gh>=357){panel->SetClientSize(w,gh);SetClientSize(w+18,gh);}//else{panel->SetClientSize(w,360);SetClientSize(w+sw,360);}
	}
void CTwindow::OnScroll(wxScrollEvent& event)
	{
	int px,py;
	panel->GetPosition(&px,&py);
	scpos=event.GetPosition();
	if(py!=scpos){py= (-scpos);panel->SetPosition(wxPoint(px,py*4));
	
		
		}

	}

void CTwindow::OnMouseEvent(wxMouseEvent& event)
	{
	if (event.GetWheelRotation() != 0) {
		int step = 4 * event.GetWheelRotation() / event.GetWheelDelta();
		int px,py,gw,gh;
	panel->GetPosition(&px,&py);
	Kai->Grid1->GetClientSize(&gw,&gh);
	if((py-step)< -(361-gh)&&py-step>=0){
		scpos=scrollBar->GetScrollPos(wxSB_VERTICAL)-(step/4);
		scrollBar->SetScrollPos(wxSB_VERTICAL,scpos);
		panel->SetPosition(wxPoint(px,py-step));}
		return;}
	}

void CTwindow::DoTooltips()
	{
	timetext->SetToolTip("Czas przesuniêcia");
	videotime->SetToolTip("Przesuwanie zaznaczonej linijki do czasu wideo +/- czas przesuniêcia");
	RadioButton1->SetToolTip("Przesuwanie wed³ug czasu");
	RadioButton2->SetToolTip("Przesuwanie wed³ug klatek (do czasu zrobienia timekodów bêdzie ma³o dok³adne)");
	RadioButton11->SetToolTip("Przesuniêcie w przód");
	RadioButton12->SetToolTip("Przesuniêcie w ty³");
	RadioButton3->SetToolTip("Przesuwanie czasu wszystkich linijek");
	RadioButton4->SetToolTip("Przesuwanie czasu zaznaczonych linijek");
	RadioButton5->SetToolTip("Przesuwanie czasu od zaznaczonej linijki");
	RadioButton6->SetToolTip("Przesuwanie czasu linijek o czasach wy¿szych b¹dŸ równych pierwszej zaznaczonej linijce");
	RadioButton7->SetToolTip("Przesuwanie czasu linijek zawieraj¹cych style wybrane poni¿ej");
	RadioButton8->SetToolTip("Przesuwanie czasu startowego i koñcowego jednoczeœnie");
	RadioButton9->SetToolTip("Przesuwanie tylko czasów startowych");
	RadioButton10->SetToolTip("Przesuwanie tylko czasów koñcowych");
	Button1->SetToolTip("Wybierz style z listy");
	TextCtrl2->SetToolTip("Style potrzebne do przesuwania wg styli oddzielone œrednikiem");
	}

BEGIN_EVENT_TABLE(CTwindow,wxWindow)
EVT_COMMAND_SCROLL(ID_SCROLL,CTwindow::OnScroll)
EVT_SIZE(CTwindow::OnSize)
EVT_MOUSE_EVENTS(CTwindow::OnMouseEvent)
END_EVENT_TABLE()