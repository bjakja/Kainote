
#include "ChangeTime.h"
#include "config.h"
#include "Stylelistbox.h"
#include "kainoteMain.h"
#include "EditBox.h"


CTwindow::CTwindow(wxWindow* parent,kainoteFrame* kfparent,wxWindowID id,const wxPoint& pos,const wxSize& size,long style)
	: wxScrolled<wxWindow>(parent, id, pos, size, style|wxVERTICAL)
{
    Kai=kfparent;
    form=ASS;
    
	bestsize=-1;
	isscrollbar=false;
	SetScrollRate(0,5);
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	
	wxAcceleratorEntry ctentries[1];
    ctentries[0].Set(wxACCEL_NORMAL, WXK_RETURN, ID_MOVE);
	
    wxAcceleratorTable ctaccel(1, ctentries);
    SetAcceleratorTable(ctaccel);
	

	wxFont thisFont(8,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);

	SetFont(thisFont);
	Main=new wxBoxSizer(wxVERTICAL);
	
	
	//ramka czasu
	wxStaticBoxSizer *timesizer=new wxStaticBoxSizer(wxVERTICAL,this,"Czas");
	wxGridSizer *timegrid=new wxGridSizer(2, 0, 0);
	MoveTime = new wxButton(this, ID_MOVE, _("Przesuñ"), wxDefaultPosition, wxSize(60,22));
	TimeText = new TimeCtrl(this, -1, _("0:00:00.00"), wxDefaultPosition, wxSize(60,22), wxTE_PROCESS_ENTER);
	Forward = new wxRadioButton(this, -1, _("W przód"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	Backward = new wxRadioButton(this, -1, _("W ty³"));

	timegrid->Add(TimeText,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	timegrid->Add(MoveTime,0,wxEXPAND|wxRIGHT,2);
	timegrid->Add(Forward,1,wxEXPAND|wxLEFT|wxRIGHT,2);
	timegrid->Add(Backward,1,wxEXPAND|wxRIGHT,2);

	timesizer->Add(timegrid,0,wxEXPAND,0);

	//ramka przesuwania wg audio / wideo
	wxStaticBoxSizer *VAtiming=new wxStaticBoxSizer(wxVERTICAL,this,"Przesuwanie wg wideo / audio");

	wxBoxSizer *SE=new wxBoxSizer(wxHORIZONTAL);
	StartVAtime = new wxRadioButton(this, -1, _("Pocz¹tek"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	EndVAtime = new wxRadioButton(this, -1, _("Koniec"));

	SE->Add(StartVAtime,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	SE->Add(EndVAtime,0,wxEXPAND|wxRIGHT,2);

	videotime = new wxCheckBox(this, ID_VIDEO, _("Przesuñ znacznik\ndo czasu wideo"));
	videotime->SetForegroundColour(*wxRED);
	videotime->Enable(false);

	audiotime = new wxCheckBox(this, ID_AUDIO, _("Przesuñ znacznik\ndo czasu audio"));
	audiotime->SetForegroundColour(*wxRED);
	audiotime->Enable(false);

	Connect(ID_VIDEO,ID_AUDIO,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CTwindow::AudioVideoTime);
	
	VAtiming->Add(SE,0,wxEXPAND|wxTOP,2);
	VAtiming->Add(videotime,0,wxEXPAND);
	VAtiming->Add(audiotime,0,wxEXPAND);

	wxArrayString choices;
	wxStaticBoxSizer *linesizer=new wxStaticBoxSizer(wxVERTICAL,this,"Które linijki");
	choices.Add(_("Wszystkie linijki"));
	choices.Add(_("Zaznaczone linijki"));
	choices.Add(_("Od zaznaczonej linijki"));
	choices.Add(_("Czasy wy¿sze i równe"));
	choices.Add(_("Wed³ug wybranych stylów"));
	WhichLines= new wxChoice(this,-1,wxDefaultPosition,wxDefaultSize,choices);

	wxBoxSizer *stylesizer= new wxBoxSizer(wxHORIZONTAL);
	AddStyles = new wxButton(this, ID_BSTYLE, _("+"),wxDefaultPosition, wxSize(22,22));
	Stylestext = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	stylesizer->Add(AddStyles,0,wxALL,2);
	stylesizer->Add(Stylestext,1,wxEXPAND|wxBOTTOM|wxTOP|wxRIGHT,2);

	linesizer->Add(WhichLines,0,wxEXPAND|wxRIGHT|wxTOP|wxLEFT,2);
	linesizer->Add(stylesizer,1,wxEXPAND);

	wxStaticBoxSizer *timessizer=new wxStaticBoxSizer(wxVERTICAL,this,"Sposób przesuwania czasów");
	choices.clear();
	choices.Add(_("Obydwa czasy"));
	choices.Add(_("Czas pocz¹tkowy"));
	choices.Add(_("Czas koñcowy"));
	
	WhichTimes= new wxChoice(this,-1,wxDefaultPosition,wxDefaultSize,choices);
	WhichTimes->Enable(form!=TMP);
	
	timessizer->Add(WhichTimes,0,wxEXPAND|wxRIGHT|wxTOP|wxLEFT,2);

	wxStaticBoxSizer *cesizer=new wxStaticBoxSizer(wxVERTICAL,this,"Korekcja czasów koñcowych");
	wxString ctchoices[3]={_("Zostaw bez zmian"), _("Skoryguj nachodz¹ce czasy"), _("Nowe czasy")};
	CorTime = new wxChoice(this, -1, wxDefaultPosition, wxSize(120,-1), 3, ctchoices);
	CorTime->SetSelection(0);
	cesizer->Add(CorTime,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	coll = new wxButton(this,22999,"Opcje dodatkowe",wxDefaultPosition, wxSize(-1,24));
	LeadIn=NULL;
	
	Main->Add(timesizer,0,wxEXPAND|wxRIGHT|wxTOP|wxLEFT,4);
	Main->Add(VAtiming,0,wxEXPAND|wxRIGHT|wxTOP|wxLEFT,4);
	Main->Add(linesizer,0,wxEXPAND|wxLEFT|wxTOP|wxRIGHT,4);
	Main->Add(timessizer,0,wxEXPAND|wxALL,4);
	Main->Add(cesizer,0,wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT,4);
	Main->Add(coll,0,wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT,4);
	
	

	Connect(ID_MOVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::OnOKClick);
	Connect(ID_BSTYLE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::OnAddStyles);
	Connect(22999,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::CollapsePane);

	DoTooltips();
	if(Options.GetInt(_("Postprocessor enabling"))>15){wxCommandEvent evt; evt.SetId(122); CollapsePane(evt);}
	RefVals();
	wxSize bsize=GetBestSize();
	bestsize=bsize.x;
	SetSizer(Main);
	Layout();
}

CTwindow::~CTwindow()
{
}

void CTwindow::Contents(bool addopts)
{
	bool state;
	if(!addopts){
		form=Kai->GetTab()->Grid1->form;
		if(form<SRT){state=true;
			if(WhichLines->GetCount()<5){
				WhichLines->Append(_("Czasy wy¿sze i równe"));
				WhichLines->Append(_("Wed³ug wybranych stylów"));
			}
		}else{
			if(WhichLines->GetCount()>3){
				WhichLines->Delete(4);
				WhichLines->Delete(3);
				if(WhichLines->GetSelection()<0){WhichLines->SetSelection(0);}

			}
			state=false;
		}
		Main->Layout();
		AddStyles->Enable(state);
		Stylestext->Enable(state);
		WhichTimes->Enable(form!=TMP);
		if(Kai->GetTab()->Video->GetState()!=None){state=true;}else{state=false;}
		videotime->Enable(state);
		state=(Kai->GetTab()->Edit->ABox && Kai->GetTab()->Edit->ABox->audioDisplay->hasMark);
		audiotime->Enable(state);
	}
	if(LeadIn){
		state=(form!=TMP);
		LeadIn->Enable(state);
		LeadOut->Enable(state);
		Continous->Enable(state);
		SnapKF->Enable(state && Kai->GetTab()->Video->VFF);
	}
	RefVals((addopts)? 0 : this);
	
}



void CTwindow::OnAddStyles(wxCommandEvent& event)
{
	wxString kkk=Kai->sftc();
	Stylestext->SetValue(kkk);
}

void CTwindow::OnOKClick(wxCommandEvent& event)
{

    Options.SetInt(_("Change Time"),TimeText->GetTime().mstime);

	if(form==ASS){
		wxString sstyles=Stylestext->GetValue();
		Options.SetString(_("Styles of time change"),sstyles);
	}

	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 8 Move to audio time;
	Options.SetInt(_("Moving time options"),(int)Forward->GetValue()|((int)StartVAtime->GetValue()<<1)|((int)videotime->GetValue()<<2)|((int)audiotime->GetValue()<<3));

	Options.SetInt(_("Change mode"),WhichLines->GetSelection());
	Options.SetInt(_("Start end times"),WhichTimes->GetSelection());
	Options.SetInt(_("Corect times"),CorTime->GetSelection());
	if(LeadIn){
		Options.SetInt(_("Lead in"),LITime->GetInt());
		Options.SetInt(_("Lead out"),LOTime->GetInt());
		Options.SetInt(_("Threshold start"),ThresStart->GetInt());
		Options.SetInt(_("Threshold end"),ThresEnd->GetInt());
		Options.SetInt(_("Keyframe before start"),BeforeStart->GetInt());
		Options.SetInt(_("Keyframe after start"),AfterStart->GetInt());
		Options.SetInt(_("Keyframe before end"),BeforeEnd->GetInt());
		Options.SetInt(_("Keyframe after end"),AfterEnd->GetInt());
		//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
		//int peres= (LeadIn->GetValue())? 1 : 0
		Options.SetInt(_("Postprocessor enabling"),(int)LeadIn->GetValue()+((int)LeadOut->GetValue()*2)+((int)Continous->GetValue()*4)+((int)SnapKF->GetValue()*8)+16);
	}//else{int pe = Options.GetInt(_("Postprocessor enabling")); if(pe>=16){Options.SetInt(_("Postprocessor enabling"), pe^ 16);} }
	int acid=event.GetId();
	if (acid==ID_MOVE){
	Kai->GetTab()->Grid1->ChangeTime();
	wxBell();}
	else if(acid==ID_CLOSE){
	Hide();
	Kai->GetTab()->BoxSizer1->Layout();}
	Kai->GetTab()->Grid1->SetFocus();
}


void CTwindow::OnSize(wxSizeEvent& event)
{
	int h,gw,gh;//,sw,sh,px,py;
	TabPanel* cur=(TabPanel*)GetParent();
	cur->Grid1->GetClientSize(&gw,&gh);
	if(bestsize<0){event.Skip();return;}
	h = GetScrollPageSize(wxVERTICAL);
	if(isscrollbar&&h==0)
	{
		isscrollbar=false;
		SetMinSize(wxSize(bestsize+20,-1));
		cur->BoxSizer3->Layout();
	}
	else if(!isscrollbar&&h>0)
	{
		isscrollbar=true;
		SetMinSize(wxSize(bestsize+40,-1));
		cur->BoxSizer3->Layout();
	}
	event.Skip();
}

void CTwindow::DoTooltips()
{
	TimeText->SetToolTip("Czas przesuniêcia");
	videotime->SetToolTip("Przesuwanie zaznaczonej linijki\ndo czasu wideo +/- czas przesuniêcia");
	audiotime->SetToolTip("Przesuwanie zaznaczonej linijki do czasu\nznacznika audio +/- czas przesuniêcia");
	StartVAtime->SetToolTip("Przesuwa czas pocz¹tkowy do czasu wideo / audio");
	EndVAtime->SetToolTip("Przesuwa czas koñcowy do czasu wideo / audio");
	Forward->SetToolTip("Przesuniêcie w przód");
	Backward->SetToolTip("Przesuniêcie w ty³");
	WhichLines->SetToolTip("Wybór linijek do przesuniêcia");
	WhichTimes->SetToolTip("Wybór czasów do przesuniêcia");
	AddStyles->SetToolTip("Wybierz style z listy");
	Stylestext->SetToolTip("Style potrzebne do przesuwania wg styli oddzielone œrednikiem");
	CorTime->SetToolTip("Korekcja czasów koñcowych, gdy s¹ niew³aœciwe albo nachodz¹ na siebie");
	
}

void CTwindow::AudioVideoTime(wxCommandEvent &event)
{
	int id=event.GetId();
	if (id==ID_VIDEO && videotime->GetValue() && audiotime->IsEnabled()){
		audiotime->SetValue(false);}
	else if (id==ID_AUDIO && audiotime->GetValue() && videotime->IsEnabled()){
		videotime->SetValue(false);}
}

void CTwindow::RefVals(CTwindow *from)
{
	STime ct=(from)? from->TimeText->GetTime() : STime(Options.GetInt(_("Change Time")));  
	TimeText->SetTime(ct);
	int mto=Options.GetInt(_("Moving time options"));
	videotime->SetValue((from)? from->videotime->GetValue() : (mto & 4)>0);

	bool movfrwd=(from)? from->Forward->GetValue() : mto & 1;
	if(movfrwd){Forward->SetValue(true);}
	else{Backward->SetValue(true);}

	Stylestext->SetValue( (from)? from->Stylestext->GetValue() : Options.GetString(_("Styles of time change")) );

	int cm= (from)? from->WhichLines->GetSelection() : Options.GetInt(_("Change mode"));
	if(cm>(int)WhichLines->GetCount()){cm=0;}
	WhichLines->SetSelection(cm);
   
	WhichTimes->SetSelection((from)? from->WhichTimes->GetSelection() : Options.GetInt(_("Start end times")));
   
	if( (from)? from->StartVAtime->GetValue() : (mto & 2)>0 ){StartVAtime->SetValue(true);}
	else{EndVAtime->SetValue(true);}

	CorTime->SetSelection((from)? from->CorTime->GetSelection() : Options.GetInt(_("Corect times")));
	int enables = Options.GetInt(_("Postprocessor enabling"));
	if(((enables & 16) && !LeadIn) || ( !(enables & 16) && LeadIn)){
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,22999); 
		CollapsePane(evt);
	}
	if(LeadIn){
		LeadIn->SetValue((from)? from->LeadIn->GetValue() : (enables & 1)>0);
		LeadOut->SetValue((from)? from->LeadOut->GetValue() : (enables & 2)>0);
		Continous->SetValue((from)? from->Continous->GetValue() : (enables & 4)>0);
		SnapKF->SetValue((from)? from->SnapKF->GetValue() : (enables & 8)>0);
		LITime->SetInt((from)? from->LITime->GetInt() : Options.GetInt(_("Lead in")));
		LOTime->SetInt((from)? from->LOTime->GetInt() : Options.GetInt(_("Lead out")));
		ThresStart->SetInt((from)? from->ThresStart->GetInt() : Options.GetInt(_("Threshold start")));
		ThresEnd->SetInt((from)? from->ThresEnd->GetInt() : Options.GetInt(_("Threshold end")));
		BeforeStart->SetInt((from)? from->BeforeStart->GetInt() : Options.GetInt(_("Keyframe before start")));
		AfterStart->SetInt((from)? from->AfterStart->GetInt() : Options.GetInt(_("Keyframe after start")));
		BeforeEnd->SetInt((from)? from->BeforeEnd->GetInt() : Options.GetInt(_("Keyframe before end")));
		AfterEnd->SetInt((from)? from->AfterEnd->GetInt() : Options.GetInt(_("Keyframe after end")));
	}
   
}

void CTwindow::CollapsePane(wxCommandEvent &event)
{
	bool hos = (LeadIn==NULL);
	if(hos){
		liosizer=new wxStaticBoxSizer(wxHORIZONTAL,this,"Wstêp / zakoñczenie");

		wxFlexGridSizer *fgsizer = new wxFlexGridSizer(2, 4, 4);
		LeadIn=new wxCheckBox(this, -1, "Wstêp",wxDefaultPosition, wxSize(117,-1));
		LITime=new NumCtrl(this,-1,"200",-10000,10000,true,wxDefaultPosition, wxSize(40,-1));
		LeadOut=new wxCheckBox(this, -1, "Zakoñczenie",wxDefaultPosition, wxSize(117,-1));
		LOTime=new NumCtrl(this,-1,"300",-10000,10000,true,wxDefaultPosition, wxSize(40,-1));
	
		fgsizer->Add(LeadIn,wxEXPAND|wxLEFT,4);
		fgsizer->Add(LITime,0);
		fgsizer->Add(LeadOut,wxEXPAND|wxLEFT,4);
		fgsizer->Add(LOTime,0);

		liosizer->Add(fgsizer,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	
		consizer=new wxStaticBoxSizer(wxVERTICAL,this,"Ustaw czasy jako ci¹g³e");
		Continous=new wxCheckBox(this, -1, "W³¹cz");

		consizer->Add(Continous,0,wxEXPAND|wxALL, 2);

		wxFlexGridSizer *fgsizer1 = new wxFlexGridSizer(2, 4, 4);
		ThresStart=new NumCtrl(this,-1,"0",0,10000,true,wxDefaultPosition, wxSize(40,-1));
		ThresEnd=new NumCtrl(this,-1,"300",0,10000,true,wxDefaultPosition, wxSize(40,-1));
	
		fgsizer1->Add(new EBStaticText(this,"Próg czasu pocz¹tku", wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer1->Add(ThresStart,0);
		fgsizer1->Add(new EBStaticText(this,"Próg czasu koñca", wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer1->Add(ThresEnd,0);
	
		consizer->Add(fgsizer1,0,wxEXPAND,0);

		snapsizer=new wxStaticBoxSizer(wxVERTICAL,this,"Wyrównaj do keyframów");
		SnapKF=new wxCheckBox(this, -1, "W³¹cz");
		SnapKF->Enable(false);
		snapsizer->Add(SnapKF,0,wxEXPAND|wxALL, 2);

		wxFlexGridSizer *fgsizer2 = new wxFlexGridSizer(2, 4, 4);
		BeforeStart=new NumCtrl(this,-1,"150",0,1000,true,wxDefaultPosition, wxSize(40,-1));
		AfterStart=new NumCtrl(this,-1,"50",0,1000,true,wxDefaultPosition, wxSize(40,-1));
		BeforeEnd=new NumCtrl(this,-1,"50",0,1000,true,wxDefaultPosition, wxSize(40,-1));
		AfterEnd=new NumCtrl(this,-1,"150",0,1000,true,wxDefaultPosition, wxSize(40,-1));
	
		fgsizer2->Add(new EBStaticText(this,"Przed czasem pocz¹tku", wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(BeforeStart,0);
		fgsizer2->Add(new EBStaticText(this,"Po czasie pocz¹tku", wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(AfterStart,0);
		fgsizer2->Add(new EBStaticText(this,"Przed czasem koñca", wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(BeforeEnd,0);
		fgsizer2->Add(new EBStaticText(this,"Po czasie koñca", wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(AfterEnd,0);
	
		snapsizer->Add(fgsizer2,0,wxEXPAND,0);


		LeadIn->SetToolTip("Wstawia wstêp do czasu pocz¹tkowego, dobre przy stosowaniu fad");
		LITime->SetToolTip("Czas wstêpu w milisekundach");
		LeadOut->SetToolTip("Wstawia zakoñczenie do czasu koñcowego, dobre przy stosowaniu fad");
		LOTime->SetToolTip("Czas zakoñczenia w milisekundach");
		ThresStart->SetToolTip("Próg wyd³u¿ania czasu pocz¹tkowego");
		ThresEnd->SetToolTip("Próg wyd³u¿ania czasu koñcowego");
		BeforeStart->SetToolTip("Maksymalne przesuniêcie do klatki kluczowej\nprzed czasem pocz¹tkowym w milisekundach");
		AfterStart->SetToolTip("Maksymalne przesuniêcie do klatki kluczowej\npo czasie pocz¹tkowym w milisekundach");
		BeforeEnd->SetToolTip("Maksymalne przesuniêcie do klatki kluczowej\nprzed czasem koñcowym w milisekundach");
		AfterEnd->SetToolTip("Maksymalne przesuniêcie do klatki kluczowej\npo czasie koñcowym w milisekundach");

		Main->Add(liosizer,0,wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT,4);
		Main->Add(consizer,0,wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT,4);
		Main->Add(snapsizer,0,wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT,4);
	
		if(event.GetId()==22999){
			Contents(true);
			((TabPanel*)GetParent())->BoxSizer3->Layout();
		}
	
	}else{
		int size=Main->GetItemCount();
		for(int i=size-1; i>=size-3; i--){
			Main->Detach(i);
		}
		liosizer->Clear(true);
		consizer->Clear(true);
		snapsizer->Clear(true);
		delete liosizer;
		delete consizer;
		delete snapsizer;
		LeadIn=NULL;
		((TabPanel*)GetParent())->BoxSizer3->Layout();
	}
	int pe = Options.GetInt(_("Postprocessor enabling"));
	Options.SetInt(_("Postprocessor enabling"), (hos)? pe | 16 : pe ^ 16);
	
}

BEGIN_EVENT_TABLE(CTwindow,wxScrolled<wxWindow>)
EVT_SIZE(CTwindow::OnSize)
END_EVENT_TABLE()
