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


#include "ChangeTime.h"
#include "Config.h"
#include "Stylelistbox.h"
#include "KainoteMain.h"
#include "EditBox.h"
//#include "ColorPicker.h"


CTwindow::CTwindow(wxWindow* parent,kainoteFrame* kfparent,wxWindowID id,const wxPoint& pos,const wxSize& size,long style)
	: wxWindow/*wxScrolled<wxWindow>*/(parent, id, pos, size, style|wxVERTICAL)
{
    Kai=kfparent;
    form=ASS;
    panel = new wxWindow(this,1);
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	scroll=new KaiScrollbar(this,5558,wxDefaultPosition, wxDefaultSize, wxVERTICAL);
	scroll->Hide();
	scroll->SetScrollRate(30);
	isscrollbar=false;
	//SetScrollRate(0,5);
	scPos=0;
	
	wxAcceleratorEntry ctentries[1];
    ctentries[0].Set(wxACCEL_NORMAL, WXK_RETURN, ID_MOVE);
	
    wxAcceleratorTable ctaccel(1, ctentries);
    SetAcceleratorTable(ctaccel);
	

	wxFont thisFont(8,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);

	panel->SetFont(thisFont);
	Main=new wxBoxSizer(wxVERTICAL);
	
	//ramka czasu
	KaiStaticBoxSizer *timesizer=new KaiStaticBoxSizer(wxVERTICAL,panel,_("Czas"));
	wxGridSizer *timegrid=new wxGridSizer(2, 0, 0);
	MoveTime = new MappedButton(panel, ID_MOVE, _("Przesuń"), _("Przesuń czas napisów"), wxDefaultPosition, wxSize(60,22), GLOBAL_HOTKEY);
	TimeText = new TimeCtrl(panel, -1, "0:00:00.00", wxDefaultPosition, wxSize(60,20), wxALIGN_CENTER|wxTE_PROCESS_ENTER);
	Forward = new KaiCheckBox(panel, -1, _("W przód"));
	DisplayTimes = new KaiCheckBox(panel, 31221, _("Czasy"));
	MoveTagTimes = new KaiCheckBox(panel, -1, _("Przesuwaj także czasy tagów"));
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &CTwindow::OnChangeDisplayUnits, this, 31221);
	timegrid->Add(TimeText,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	timegrid->Add(MoveTime,0,wxEXPAND|wxRIGHT,2);
	timegrid->Add(Forward,1,wxEXPAND|wxLEFT|wxRIGHT,2);
	timegrid->Add(DisplayTimes,1,wxEXPAND|wxRIGHT,2);

	timesizer->Add(timegrid,0,wxEXPAND,0);
	timesizer->Add(MoveTagTimes,0,wxEXPAND|wxLEFT,2);

	//ramka przesuwania wg audio / wideo
	KaiStaticBoxSizer *VAtiming=new KaiStaticBoxSizer(wxVERTICAL,panel,_("Przesuwanie wg wideo / audio"));

	wxBoxSizer *SE=new wxBoxSizer(wxHORIZONTAL);
	StartVAtime = new KaiRadioButton(panel, -1, _("Początek"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	EndVAtime = new KaiRadioButton(panel, -1, _("Koniec"));

	SE->Add(StartVAtime,1,wxEXPAND|wxLEFT|wxRIGHT,2);
	SE->Add(EndVAtime,1,wxEXPAND|wxRIGHT,2);
	wxColour warning = Options.GetColour(WindowWarningElements);

	videotime = new KaiCheckBox(panel, ID_VIDEO, _("Przesuń znacznik\ndo czasu wideo"));
	videotime->SetForegroundColour(warning);
	videotime->Enable(false);

	audiotime = new KaiCheckBox(panel, ID_AUDIO, _("Przesuń znacznik\ndo czasu audio"));
	audiotime->SetForegroundColour(warning);
	audiotime->Enable(false);

	Connect(ID_VIDEO,ID_AUDIO,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CTwindow::AudioVideoTime);
	//TextColorPicker *picker = new TextColorPicker(this, AssColor(wxString("#AABBCC")));
	VAtiming->Add(SE,0,wxEXPAND|wxTOP,2);
	VAtiming->Add(videotime,1,wxEXPAND|wxLEFT,2);
	VAtiming->Add(audiotime,1,wxEXPAND|wxLEFT,2);
	//VAtiming->Add(picker,0,wxEXPAND|wxLEFT,2);
	wxArrayString choices;
	KaiStaticBoxSizer *linesizer=new KaiStaticBoxSizer(wxVERTICAL,panel,_("Które linijki"));
	choices.Add(_("Wszystkie linijki"));
	choices.Add(_("Zaznaczone linijki"));
	choices.Add(_("Od zaznaczonej linijki"));
	choices.Add(_("Czasy wyższe i równe"));
	choices.Add(_("Według wybranych stylów"));
	WhichLines= new KaiChoice(panel,-1,wxDefaultPosition,wxDefaultSize,choices,KAI_SCROLL_ON_FOCUS);

	wxBoxSizer *stylesizer= new wxBoxSizer(wxHORIZONTAL);
	AddStyles = new MappedButton(panel, ID_BSTYLE, "+", "", wxDefaultPosition, wxSize(22,22), 0);
	Stylestext = new KaiTextCtrl(panel, -1, "", wxDefaultPosition, wxSize(22,22), wxTE_PROCESS_ENTER);
	stylesizer->Add(AddStyles,0,wxALL,2);
	stylesizer->Add(Stylestext,1,wxEXPAND|wxBOTTOM|wxTOP|wxRIGHT,2);

	linesizer->Add(WhichLines,0,wxEXPAND|wxRIGHT|wxTOP|wxLEFT,2);
	linesizer->Add(stylesizer,1,wxEXPAND);

	KaiStaticBoxSizer *timessizer=new KaiStaticBoxSizer(wxVERTICAL,panel,_("Sposób przesuwania czasów"));
	choices.clear();
	choices.Add(_("Obydwa czasy"));
	choices.Add(_("Czas początkowy"));
	choices.Add(_("Czas końcowy"));
	
	WhichTimes= new KaiChoice(panel,-1,wxDefaultPosition,wxDefaultSize,choices,KAI_SCROLL_ON_FOCUS);
	WhichTimes->Enable(form!=TMP);
	
	timessizer->Add(WhichTimes,0,wxEXPAND|wxRIGHT|wxTOP|wxLEFT,2);

	KaiStaticBoxSizer *cesizer=new KaiStaticBoxSizer(wxVERTICAL,panel,_("Korekcja czasów końcowych"));
	choices.clear();
	choices.Add(_("Pozostaw bez zmian"));
	choices.Add(_("Skoryguj nachodzące czasy"));
	choices.Add(_("Nowe czasy"));
	CorTime = new KaiChoice(panel, -1, wxDefaultPosition, wxSize(130,-1), choices, KAI_SCROLL_ON_FOCUS);
	CorTime->SetSelection(0);
	cesizer->Add(CorTime,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	coll = new MappedButton(panel,22999,_("Opcje dodatkowe"),"",wxDefaultPosition, wxSize(-1,-1), 0);
	LeadIn=NULL;
	
	Main->Add(timesizer,0,wxEXPAND|wxALL,2);
	Main->Add(VAtiming,0,wxEXPAND|wxALL,2);
	Main->Add(linesizer,0,wxEXPAND|wxALL,2);
	Main->Add(timessizer,0,wxEXPAND|wxALL,2);
	Main->Add(cesizer,0,wxEXPAND|wxALL,2);
	Main->AddSpacer(4);
	Main->Add(coll,0,wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT,6);
	
	

	Connect(ID_MOVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&CTwindow::OnOKClick);
	Connect(ID_BSTYLE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::OnAddStyles);
	Connect(22999,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CTwindow::CollapsePane);

	DoTooltips();
	if(Options.GetInt(PostprocessorEnabling)>15){wxCommandEvent evt; evt.SetId(122); CollapsePane(evt);}
	RefVals();
	
	panel->SetSizerAndFit(Main);
}

CTwindow::~CTwindow()
{
}

bool CTwindow::SetBackgroundColour(const wxColour &col)
{
	wxWindow::SetBackgroundColour(Options.GetColour(WindowBackground));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));
	return true;
}

bool CTwindow::SetForegroundColour(const wxColour &col)
{
	wxWindow::SetForegroundColour(Options.GetColour(WindowText));
	panel->SetForegroundColour(Options.GetColour(WindowText));
	return true;
}

void CTwindow::Contents(bool addopts)
{
	bool state;
	form=Kai->GetTab()->Grid1->form;
	VideoCtrl *vb = Kai->GetTab()->Video;
	if(form<SRT){
		state=true;
		WhichLines->EnableItem(3);
		WhichLines->EnableItem(4);
	}else{
		WhichLines->EnableItem(3,false);
		WhichLines->EnableItem(4,false);
		state=false;
	}
	bool lastEnable = DisplayTimes->IsEnabled();
	DisplayTimes->Enable(vb->VFF != NULL);
	bool Enable = DisplayTimes->IsEnabled();
	bool dispTimes = DisplayTimes->GetValue();
	if(lastEnable != Enable){
		if(!vb->VFF && (!dispTimes || !Enable)){
			ChangeDisplayUnits(true);
		}else if(vb->VFF && !dispTimes){
			ChangeDisplayUnits(false);
		}
	}
	MoveTagTimes->Enable(state);
	AddStyles->Enable(state);
	Stylestext->Enable(state);
	WhichTimes->Enable(form!=TMP);
	if(vb->GetState()!=None){state=true;}else{state=false;}
	videotime->Enable(state);
	state=(Kai->GetTab()->Edit->ABox && Kai->GetTab()->Edit->ABox->audioDisplay->hasMark);
	audiotime->Enable(state);
	if(LeadIn){
		state=(form!=TMP);
		LeadIn->Enable(state);
		LeadOut->Enable(state);
		Continous->Enable(state);
		SnapKF->Enable(state && vb->VFF);
	}
	//if(addopts){RefVals();}
	
}



void CTwindow::OnAddStyles(wxCommandEvent& event)
{
	wxString result = GetCheckedElements(Kai);
	Stylestext->SetValue(result);
	if(result != ""){WhichLines->SetSelection(4);}
}

void CTwindow::OnOKClick(wxCommandEvent& event)
{
	int time=0;
	if(TimeText->HasShownFrames()){
		TabPanel *tab = ((TabPanel*)GetParent());
		if(!tab->Video->VFF){
			wxLogMessage(_("Wideo nie zostało wczytane przez FFMS2")); return;
			//time = TimeText->GetTime().mstime; 
		}else{
			/*int startFrame = tab->Edit->line->Start.orgframe;
			int endFrame = startFrame + TimeText->GetTime().orgframe;
			int startMS = tab->Video->VFF->GetMSfromFrame(startFrame);
			int endMS = tab->Video->VFF->GetMSfromFrame(endFrame);
			time = ZEROIT(endMS - startMS)+10;*/
			Options.SetInt(MoveTimesFrames,TimeText->GetTime().orgframe);
		}
	}else{
		//time = TimeText->GetTime().mstime;
		Options.SetInt(MoveTimesTime,TimeText->GetTime().mstime);
	}
    

	if(form==ASS){
		wxString sstyles=Stylestext->GetValue();
		Options.SetString(MoveTimesStyles,sstyles);
	}

	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 
	//8 Move to audio time 16 display times / frames 32 move tag times;
	Options.SetInt(MoveTimesOptions,(int)Forward->GetValue()|((int)StartVAtime->GetValue()<<1)|
		((int)videotime->GetValue()<<2)|((int)audiotime->GetValue()<<3)|
		((int)DisplayTimes->GetValue()<<4)|((int)MoveTagTimes->GetValue()<<5));

	Options.SetInt(MoveTimesWhichLines,WhichLines->GetSelection());
	Options.SetInt(MoveTimesWhichTimes,WhichTimes->GetSelection());
	Options.SetInt(MoveTimesCorrectEndTimes,CorTime->GetSelection());
	if(LeadIn){
		Options.SetInt(PostprocessorLeadIn,LITime->GetInt());
		Options.SetInt(PostprocessorLeadOut,LOTime->GetInt());
		Options.SetInt(PostprocessorThresholdStart,ThresStart->GetInt());
		Options.SetInt(PostprocessorThresholdEnd,ThresEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeStart,BeforeStart->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterStart,AfterStart->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeEnd,BeforeEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterEnd,AfterEnd->GetInt());
		//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
		//int peres= (LeadIn->GetValue())? 1 : 0
		Options.SetInt(PostprocessorEnabling,(int)LeadIn->GetValue()+((int)LeadOut->GetValue()*2)+((int)Continous->GetValue()*4)+((int)SnapKF->GetValue()*8)+16);
	}//else{int pe = Options.GetInt("Postprocessor enabling"); if(pe>=16){Options.SetInt("Postprocessor enabling", pe^ 16);} }
	int acid=event.GetId();
	TabPanel *tab = Kai->GetTab();
	if (acid==ID_MOVE){
		tab->Grid1->ChangeTimes(TimeText->HasShownFrames());
	}else if(acid==ID_CLOSE){
		Hide();
		tab->BoxSizer1->Layout();
	}
	tab->Grid1->SetFocus();
}


void CTwindow::OnSize(wxSizeEvent& event)
{
	int h,gw,gh;
	TabPanel* cur=(TabPanel*)GetParent();
	cur->Grid1->GetClientSize(&gw,&gh);
	int w;
	panel->GetBestSize(&w,&h);
	int ctw, cth;
	GetSize(&ctw,&cth);
	if(!isscrollbar && gh < h)//pojawianie scrollbara
	{
		isscrollbar=true;
		SetMinSize(wxSize(w+17,h));
		cur->BoxSizer3->Layout();
		scroll->SetSize(w, 0, 17, gh);
		scroll->SetScrollbar(scPos, gh, h, gh-10);
		scroll->Show();
		
	}
	else if(isscrollbar && gh >= h )//znikanie scrollbara
	{
		isscrollbar=false;
		scPos=0;
		scroll->Hide();
		scroll->SetScrollbar(scPos, gh, h, gh-10);
		SetMinSize(wxSize(w,h));
		panel->SetPosition(wxPoint(0,scPos));
		cur->BoxSizer3->Layout();
	}else if(scroll->IsShown()){
		scroll->SetSize(ctw-17, 0, 17, gh);
		scroll->SetScrollbar(scPos, gh, h, gh-10);
		if(scPos != scroll->GetScrollPos()){
			scPos = scroll->GetScrollPos();
			panel->SetPosition(wxPoint(0,-scPos));
			//panel->Refresh(false);
		}
	}
	
}

void CTwindow::DoTooltips()
{
	TimeText->SetToolTip(_("Czas przesunięcia"));
	videotime->SetToolTip(_("Przesuwanie zaznaczonej linijki\ndo czasu wideo ± czas przesunięcia"));
	audiotime->SetToolTip(_("Przesuwanie zaznaczonej linijki do czasu\nznacznika audio ± czas przesunięcia"));
	StartVAtime->SetToolTip(_("Przesuwa czas początkowy do czasu wideo / audio"));
	EndVAtime->SetToolTip(_("Przesuwa czas końcowy do czasu wideo / audio"));
	Forward->SetToolTip(_("Przesunięcie w przód / w tył"));
	DisplayTimes->SetToolTip(_("Przesuwa napisy o ustawiony czas / klatki"));
	MoveTagTimes->SetToolTip(_("Przesuwa czasy tagów \\move, \\t, \\fad tak,\nby ich pozycja na wideo się nie zmieniła\n(spowalnia dość znacznie przesuwanie czasów)"));
	WhichLines->SetToolTip(_("Wybór linijek do przesunięcia"));
	WhichTimes->SetToolTip(_("Wybór czasów do przesunięcia"));
	AddStyles->SetToolTip(_("Wybierz style z listy"));
	Stylestext->SetToolTip(_("Przesuń według następujących stylów (oddzielone średnikiem)"));
	CorTime->SetToolTip(_("Korekcja czasów końcowych, gdy są niewłaściwe albo nachodzą na siebie"));
	
}

void CTwindow::AudioVideoTime(wxCommandEvent &event)
{
	int id=event.GetId();
	if (id==ID_VIDEO && videotime->GetValue()){
		audiotime->SetValue(false);
	}
	else if (id==ID_AUDIO && audiotime->GetValue()){
		videotime->SetValue(false);
	}
}

void CTwindow::RefVals(CTwindow *from)
{
	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 
	//8 Move to audio time 16 display times / frames 32 move tag times;
	int mto=Options.GetInt(MoveTimesOptions);
	STime ct=(from)? from->TimeText->GetTime() : STime(Options.GetInt(MoveTimesTime), Options.GetInt(MoveTimesFrames)); 
	bool dispTimes = DisplayTimes->GetValue();
	DisplayTimes->SetValue((from)? from->DisplayTimes->GetValue() : (mto & 16) > 0);
	TabPanel *tab = ((TabPanel*)GetParent());
	if(from && (from->DisplayTimes->GetValue() != dispTimes)){
		if(!DisplayTimes->GetValue()){
			ChangeDisplayUnits(false);
		}else if(DisplayTimes->GetValue()){
			ChangeDisplayUnits(true);
		}
	
		
	}else if(!DisplayTimes->GetValue() && !tab->Video->VFF){
		ChangeDisplayUnits(true);
		DisplayTimes->Enable(false);
	}
	TimeText->SetTime(ct);
	
	videotime->SetValue((from)? from->videotime->GetValue() : (mto & 4)>0);

	Forward->SetValue((from)? from->Forward->GetValue() : (mto & 1)>0);
	DisplayTimes->SetValue((from)? from->DisplayTimes->GetValue() : (mto & 16)>0);
	MoveTagTimes->SetValue((from)? from->MoveTagTimes->GetValue() : (mto & 32)>0);
	Stylestext->SetValue( (from)? from->Stylestext->GetValue() : Options.GetString(MoveTimesStyles) );

	int cm= (from)? from->WhichLines->GetSelection() : Options.GetInt(MoveTimesWhichLines);
	if(cm>(int)WhichLines->GetCount()){cm=0;}
	WhichLines->SetSelection(cm);
   
	WhichTimes->SetSelection((from)? from->WhichTimes->GetSelection() : Options.GetInt(MoveTimesWhichTimes));
   
	if( (from)? from->StartVAtime->GetValue() : (mto & 2)>0 ){StartVAtime->SetValue(true);}
	else{EndVAtime->SetValue(true);}

	CorTime->SetSelection((from)? from->CorTime->GetSelection() : Options.GetInt(MoveTimesCorrectEndTimes));
	int enables = Options.GetInt(PostprocessorEnabling);
	if(((enables & 16) && !LeadIn) || ( !(enables & 16) && LeadIn)){
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,22999); 
		CollapsePane(evt);
	}
	if(LeadIn && from && from->LeadIn){
		LeadIn->SetValue((from)? from->LeadIn->GetValue() : (enables & 1)>0);
		LeadOut->SetValue((from)? from->LeadOut->GetValue() : (enables & 2)>0);
		Continous->SetValue((from)? from->Continous->GetValue() : (enables & 4)>0);
		SnapKF->SetValue((from)? from->SnapKF->GetValue() : (enables & 8)>0);
		LITime->SetInt((from)? from->LITime->GetInt() : Options.GetInt(PostprocessorLeadIn));
		LOTime->SetInt((from)? from->LOTime->GetInt() : Options.GetInt(PostprocessorLeadOut));
		ThresStart->SetInt((from)? from->ThresStart->GetInt() : Options.GetInt(PostprocessorThresholdStart));
		ThresEnd->SetInt((from)? from->ThresEnd->GetInt() : Options.GetInt(PostprocessorThresholdEnd));
		BeforeStart->SetInt((from)? from->BeforeStart->GetInt() : Options.GetInt(PostprocessorKeyframeBeforeStart));
		AfterStart->SetInt((from)? from->AfterStart->GetInt() : Options.GetInt(PostprocessorKeyframeAfterStart));
		BeforeEnd->SetInt((from)? from->BeforeEnd->GetInt() : Options.GetInt(PostprocessorKeyframeBeforeEnd));
		AfterEnd->SetInt((from)? from->AfterEnd->GetInt() : Options.GetInt(PostprocessorKeyframeAfterEnd));
	}
   
}

void CTwindow::CollapsePane(wxCommandEvent &event)
{
	bool collapsed = (LeadIn==NULL);
	int pe = Options.GetInt(PostprocessorEnabling);
	Options.SetInt(PostprocessorEnabling, (collapsed)? pe | 16 : pe ^ 16);
	
	if(collapsed){
		liosizer=new KaiStaticBoxSizer(wxHORIZONTAL,panel,_("Wstęp i zakończenie"));

		wxFlexGridSizer *fgsizer = new wxFlexGridSizer(2, 4, 4);
		LeadIn=new KaiCheckBox(panel, -1, _("Wstęp"),wxDefaultPosition, wxSize(117,-1));
		LeadIn->SetValue((pe & 1)>0);
		LITime=new NumCtrl(panel,-1,Options.GetString(PostprocessorLeadIn),-10000,10000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
		LeadOut=new KaiCheckBox(panel, -1, _("Zakończenie"),wxDefaultPosition, wxSize(117,-1));
		LeadOut->SetValue((pe & 2)>0);
		LOTime=new NumCtrl(panel,-1,Options.GetString(PostprocessorLeadOut),-10000,10000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
	
		fgsizer->Add(LeadIn,wxEXPAND|wxLEFT,4);
		fgsizer->Add(LITime,0);
		fgsizer->Add(LeadOut,wxEXPAND|wxLEFT,4);
		fgsizer->Add(LOTime,0);

		liosizer->Add(fgsizer,0,wxEXPAND|wxLEFT|wxRIGHT,2);
	
		consizer=new KaiStaticBoxSizer(wxVERTICAL,panel,_("Ustaw czasy jako ciągłe"));
		Continous=new KaiCheckBox(panel, -1, _("Włącz"));
		Continous->SetValue((pe & 4)>0);

		consizer->Add(Continous,0,wxEXPAND|wxALL, 2);

		wxFlexGridSizer *fgsizer1 = new wxFlexGridSizer(2, 4, 4);
		ThresStart=new NumCtrl(panel,-1,Options.GetString(PostprocessorThresholdStart),0,10000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
		ThresEnd=new NumCtrl(panel,-1,Options.GetString(PostprocessorThresholdEnd),0,10000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
	
		fgsizer1->Add(new EBStaticText(panel,_("Próg czasu początku"), wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer1->Add(ThresStart,0);
		fgsizer1->Add(new EBStaticText(panel,_("Próg czasu końca"), wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer1->Add(ThresEnd,0);
	
		consizer->Add(fgsizer1,0,wxEXPAND,0);

		snapsizer=new KaiStaticBoxSizer(wxVERTICAL,panel,_("Wyrównaj do klatek kluczowych"));
		SnapKF=new KaiCheckBox(panel, -1, _("Włącz"));
		SnapKF->Enable(false);
		SnapKF->SetValue((pe & 8)>0);
		snapsizer->Add(SnapKF,0,wxEXPAND|wxALL, 2);

		wxFlexGridSizer *fgsizer2 = new wxFlexGridSizer(2, 4, 4);
		BeforeStart=new NumCtrl(panel,-1,Options.GetString(PostprocessorKeyframeBeforeStart),0,1000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
		AfterStart=new NumCtrl(panel,-1,Options.GetString(PostprocessorKeyframeAfterStart),0,1000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
		BeforeEnd=new NumCtrl(panel,-1,Options.GetString(PostprocessorKeyframeBeforeEnd),0,1000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
		AfterEnd=new NumCtrl(panel,-1,Options.GetString(PostprocessorKeyframeAfterEnd),0,1000,true,wxDefaultPosition, wxSize(40,-1),SCROLL_ON_FOCUS);
	
		fgsizer2->Add(new EBStaticText(panel,_("Przed czasem początku"), wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(BeforeStart,0);
		fgsizer2->Add(new EBStaticText(panel,_("Po czasie początku"), wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(AfterStart,0);
		fgsizer2->Add(new EBStaticText(panel,_("Przed czasem końca"), wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(BeforeEnd,0);
		fgsizer2->Add(new EBStaticText(panel,_("Po czasie końca"), wxSize(115,-1)),0,wxEXPAND|wxLEFT,4);
		fgsizer2->Add(AfterEnd,0);
	
		snapsizer->Add(fgsizer2,0,wxEXPAND,0);


		LeadIn->SetToolTip(_("Wstawia wstęp do czasu początkowego, dobre przy stosowaniu fad"));
		LITime->SetToolTip(_("Czas wstępu w milisekundach"));
		LeadOut->SetToolTip(_("Wstawia zakończenie do czasu końcowego, dobre przy stosowaniu fad"));
		LOTime->SetToolTip(_("Czas zakończenia w milisekundach"));
		ThresStart->SetToolTip(_("Próg wydłużania czasu początkowego"));
		ThresEnd->SetToolTip(_("Próg wydłużania czasu końcowego"));
		BeforeStart->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\nprzed czasem początkowym w milisekundach"));
		AfterStart->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\npo czasie początkowym w milisekundach"));
		BeforeEnd->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\nprzed czasem końcowym w milisekundach"));
		AfterEnd->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\npo czasie końcowym w milisekundach"));

		Main->Add((wxSizer*)liosizer,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,2);
		Main->Add((wxSizer*)consizer,0,wxEXPAND|wxALL,2);
		Main->Add((wxSizer*)snapsizer,0,wxEXPAND|wxALL,2);
		panel->Fit();
		if(event.GetId()==22999){
			Contents(false);
			//((TabPanel*)GetParent())->BoxSizer3->Layout();
			isscrollbar=false;
			wxSizeEvent evt;
			OnSize(evt);
		}
		//wxSizeEvent evt;
		//OnSize(evt);
	}else{
		Options.SetInt(PostprocessorLeadIn,LITime->GetInt());
		Options.SetInt(PostprocessorLeadOut,LOTime->GetInt());
		Options.SetInt(PostprocessorThresholdStart,ThresStart->GetInt());
		Options.SetInt(PostprocessorThresholdEnd,ThresEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeStart,BeforeStart->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterStart,AfterStart->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeEnd,BeforeEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterEnd,AfterEnd->GetInt());
		//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
		//int peres= (LeadIn->GetValue())? 1 : 0
		Options.SetInt(PostprocessorEnabling,(int)LeadIn->GetValue()+((int)LeadOut->GetValue()*2)+((int)Continous->GetValue()*4)+((int)SnapKF->GetValue()*8));
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
		panel->Fit();
		//wxSizeEvent evt;
		//OnSize(evt);
		int w, h,gw,gh;
		TabPanel* cur=(TabPanel*)GetParent();
		cur->Grid1->GetClientSize(&gw,&gh);
		panel->GetBestSize(&w,&h);
		scPos = 0;
		if(gh < h)//pojawianie scrollbara
		{
			SetMinSize(wxSize(w+17,h));
			cur->BoxSizer3->Layout();
			scroll->SetSize(w, 0, 17, gh);
			scroll->SetScrollbar(scPos, gh, h, gh-10);
		
		}
		else if(gh >= h )//znikanie scrollbara
		{
			isscrollbar=false;
			scroll->Hide();
			scroll->SetScrollbar(scPos, gh, h, gh-10);
			SetMinSize(wxSize(w,h));
			cur->BoxSizer3->Layout();
		}
		
		panel->SetPosition(wxPoint(0,-scPos));
		//panel->Update();
	}
	
	
}

void CTwindow::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	//wxLogStatus("scroll %i %i", newPos, scPos);
	if (scPos != newPos) {
		scPos = newPos;
		panel->SetPosition(wxPoint(0,-scPos));
		panel->Update();
	}
}

void CTwindow::OnMouseScroll(wxMouseEvent& event)
{
	if(event.GetWheelRotation() != 0){
		int step = 30 * event.GetWheelRotation() / event.GetWheelDelta();
		scPos = scroll->SetScrollPos(scPos-step); 
		panel->SetPosition(wxPoint(0,-scPos));
		panel->Update();
	}
}

void CTwindow::OnChangeDisplayUnits(wxCommandEvent& event)
{
	ChangeDisplayUnits(DisplayTimes->GetValue());
}
	
void CTwindow::ChangeDisplayUnits(bool times)
{
	STime ct = TimeText->GetTime();
	if(times){
		TimeText->ShowFrames(false);
		ct.mstime = Options.GetInt(MoveTimesTime);
		TimeText->SetTime(ct);
	}else{
		TimeText->ShowFrames(true);
		ct.orgframe = Options.GetInt(MoveTimesFrames);
		TimeText->SetTime(ct);
	}
}
BEGIN_EVENT_TABLE(CTwindow,wxWindow)
EVT_SIZE(CTwindow::OnSize)
EVT_COMMAND_SCROLL_THUMBTRACK(5558,CTwindow::OnScroll)
EVT_MOUSEWHEEL(CTwindow::OnMouseScroll)
END_EVENT_TABLE()
