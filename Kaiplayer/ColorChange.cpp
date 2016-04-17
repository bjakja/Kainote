
#include <wx/wx.h>
#include "ColorChange.h"
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/settings.h>
#include <wx/fontenum.h>
#include "config.h" 
#include "dialog_colorpicker.h"


wxColour Blackorwhite(wxColour kol)
{
	int result=(kol.Red()>127) + (kol.Green()>127) + (kol.Blue()>127);
	int kols=(result<2)? 255 : 0;
	return wxColour(kols,kols,kols);
}

bool sortf(wxString name1,wxString name2){
	return (name1.CmpNoCase(name2)<0);
}


ColorChange::ColorChange(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
	: wxWindow(parent,id,pos,size)
{
	Preview=NULL;
	tab=NULL;
	block=true;
	wxFont font(8,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	SetFont(font);
	wxArrayString fontList = wxFontEnumerator::GetFacenames();
	std::sort(fontList.begin(),fontList.end(),sortf);

	wxBoxSizer *Main=new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *stylename= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Nazwa stylu:"));
	wxTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	valid.SetCharExcludes(",");
	sname = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER,valid);
	stylename->Add(sname,1,wxEXPAND|wxALL,2);

	wxStaticBoxSizer *stylefont= new wxStaticBoxSizer(wxVERTICAL, this, _("Czcionka i rozmiar:"));
	wxBoxSizer *fntsizer=new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *biussizer=new wxBoxSizer(wxHORIZONTAL);
	sfont = new wxComboBox(this, ID_FONTNAME,"", wxDefaultPosition, wxDefaultSize, fontList);
	ssize = new NumCtrl(this, ID_TOUTLINE, "32",1,10000,false, wxDefaultPosition, wxSize(66,-1), wxTE_PROCESS_ENTER);

	sb = new wxCheckBox(this, ID_CBOLD, _("Pogrubienie"), wxDefaultPosition, wxSize(73,15));
	si = new wxCheckBox(this, ID_CBOLD, _("Kursywa"), wxDefaultPosition, wxSize(73,15));
	su = new wxCheckBox(this, ID_CBOLD, _("Podkreślenie"), wxDefaultPosition, wxSize(73,15));
	ss = new wxCheckBox(this, ID_CBOLD, _("Przekreślenie"), wxDefaultPosition, wxSize(73,15));

	fntsizer->Add(sfont,4,wxEXPAND|wxALL,2);
	fntsizer->Add(ssize,1,wxEXPAND|wxALL,2);

	biussizer->Add(sb,1,wxEXPAND|wxALL,2);
	biussizer->Add(si,1,wxEXPAND|wxALL,2);
	biussizer->Add(su,1,wxEXPAND|wxALL,2);
	biussizer->Add(ss,1,wxEXPAND|wxALL,2);

	stylefont->Add(fntsizer,0,wxEXPAND,0);
	stylefont->Add(biussizer,0,wxEXPAND|wxALIGN_CENTER,0);

	wxStaticBoxSizer *stylekol= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Kolory i przezroczystość:"));

	wxGridSizer *kolgrid=new wxGridSizer(4,2,2);
	
	s1 = new wxButton(this, ID_BCOLOR1, _("Pierwszy"));
	s2 = new wxButton(this, ID_BCOLOR2, _("Drugi"));
	s3 = new wxButton(this, ID_BCOLOR3, _("Obwódka"));
	s4 = new wxButton(this, ID_BCOLOR4, _("Cień"));

	alpha1 = new NumCtrl(this, ID_TOUTLINE, "0",0,255,true, wxDefaultPosition, wxSize(80,-1),wxTE_PROCESS_ENTER);
	alpha2 = new NumCtrl(this, ID_TOUTLINE, "0",0,255,true, wxDefaultPosition, wxSize(80,-1),wxTE_PROCESS_ENTER);
	alpha3 = new NumCtrl(this, ID_TOUTLINE, "0",0,255,true, wxDefaultPosition, wxSize(80,-1),wxTE_PROCESS_ENTER);
	alpha4 = new NumCtrl(this, ID_TOUTLINE, "0",0,255,true, wxDefaultPosition, wxSize(80,-1),wxTE_PROCESS_ENTER);

	kolgrid->Add(s1,1,wxEXPAND|wxALL,2);
	kolgrid->Add(s2,1,wxEXPAND|wxALL,2);
	kolgrid->Add(s3,1,wxEXPAND|wxALL,2);
	kolgrid->Add(s4,1,wxEXPAND|wxALL,2);

	kolgrid->Add(alpha1,1,wxEXPAND|wxALL,2);
	kolgrid->Add(alpha2,1,wxEXPAND|wxALL,2);
	kolgrid->Add(alpha3,1,wxEXPAND|wxALL,2);
	kolgrid->Add(alpha4,1,wxEXPAND|wxALL,2);

	stylekol->Add(kolgrid,1,wxEXPAND|wxALL,2);

	wxStaticBoxSizer *styleattr= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Obwódka:               Cień:                      Skala X:                 Skala Y:"));

	sou = new NumCtrl(this, ID_TOUTLINE, "", 0,1000000,false,wxDefaultPosition, wxSize(83,-1), wxTE_PROCESS_ENTER);
	ssh = new NumCtrl(this, ID_TOUTLINE, "", 0,1000000,false, wxDefaultPosition, wxSize(83,-1), wxTE_PROCESS_ENTER);
	ssx = new NumCtrl(this, ID_TOUTLINE, "", 1,10000000,false, wxDefaultPosition, wxSize(83,-1), wxTE_PROCESS_ENTER);
	ssy = new NumCtrl(this, ID_TOUTLINE, "", 1,10000000,false, wxDefaultPosition, wxSize(83,-1), wxTE_PROCESS_ENTER);

	styleattr->Add(sou,1,wxEXPAND|wxALL,2);
	styleattr->Add(ssh,1,wxEXPAND|wxALL,2);
	styleattr->Add(ssx,1,wxEXPAND|wxALL,2);
	styleattr->Add(ssy,1,wxEXPAND|wxALL,2);

	wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer *styleattr1= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Kąt:                     Odstępy:              Typ obwódki:"));

	san = new NumCtrl(this, ID_TOUTLINE, "",-1000000,1000000,false, wxDefaultPosition, wxSize(65,-1), wxTE_PROCESS_ENTER);
	ssp = new NumCtrl(this, ID_TOUTLINE, "",-1000000,1000000,false, wxDefaultPosition, wxSize(65,-1), wxTE_PROCESS_ENTER);
	sob = new wxCheckBox(this, ID_CBOLD, _("Prost. obw."));

	styleattr1->Add(san,1,wxEXPAND|wxALL,2);
	styleattr1->Add(ssp,1,wxEXPAND|wxALL,2);
	styleattr1->Add(sob,1,wxEXPAND|wxALL,2);

	wxStaticBoxSizer *stylemargs= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Margines lewy:     Prawy:                Pionowy:"));

	sml = new NumCtrl(this, ID_TOUTLINE, "", 0, 9999, true, wxDefaultPosition, wxSize(65,-1), wxTE_PROCESS_ENTER);
	smr = new NumCtrl(this, ID_TOUTLINE, "", 0, 9999, true, wxDefaultPosition, wxSize(65,-1), wxTE_PROCESS_ENTER);
	smv = new NumCtrl(this, ID_TOUTLINE, "", 0, 9999, true, wxDefaultPosition, wxSize(65,-1), wxTE_PROCESS_ENTER);

	stylemargs->Add(sml,1,wxEXPAND|wxALL,2);
	stylemargs->Add(smr,1,wxEXPAND|wxALL,2);
	stylemargs->Add(smv,1,wxEXPAND|wxALL,2);

	sizer1->Add(styleattr1,0,wxEXPAND,0);
	sizer1->Add(stylemargs,0,wxEXPAND,0);

	wxStaticBoxSizer *stylean= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Położenie tekstu:"));

	wxGridSizer *angrid=new wxGridSizer(3,5,2);

	rb7 = new wxRadioButton(this, ID_RAN7, "7", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	rb8 = new wxRadioButton(this, ID_RAN8, "8");
	rb9 = new wxRadioButton(this, ID_RAN9, "9");
	rb4 = new wxRadioButton(this, ID_RAN4, "4");
	rb5 = new wxRadioButton(this, ID_RAN5, "5");
	rb6 = new wxRadioButton(this, ID_RAN6, "6");
	rb1 = new wxRadioButton(this, ID_RAN1, "1");
	rb2 = new wxRadioButton(this, ID_RAN2, "2");
	rb3 = new wxRadioButton(this, ID_RAN3, "3");
	
	angrid->Add(rb7,1,wxEXPAND|wxALL,2);
	angrid->Add(rb8,1,wxEXPAND|wxALL,2);
	angrid->Add(rb9,1,wxEXPAND|wxALL,2);
	angrid->Add(rb4,1,wxEXPAND|wxALL,2);
	angrid->Add(rb5,1,wxEXPAND|wxALL,2);
	angrid->Add(rb6,1,wxEXPAND|wxALL,2);
	angrid->Add(rb1,1,wxEXPAND|wxALL,2);
	angrid->Add(rb2,1,wxEXPAND|wxALL,2);
	angrid->Add(rb3,1,wxEXPAND|wxALL,2);
	
	stylean->Add(angrid,0,wxEXPAND|wxALL,2);

	sizer2->Add(sizer1,0,wxEXPAND,0);
	sizer2->Add(stylean,0,wxEXPAND|wxLEFT,4);
	
	
	encs.Add(_("0 - ANSI"));
	encs.Add(_("1 - Domyślny"));
	encs.Add(_("2 - Symbol"));
	encs.Add(_("77 - Mac"));
	encs.Add(_("128 - Japoński"));
	encs.Add(_("129 - Koreański"));
	encs.Add(_("130 - Johab"));
	encs.Add(_("134 - Chiński GB2312"));
	encs.Add(_("135 - Chiński BIG5"));
	encs.Add(_("161 - Grecki"));
	encs.Add(_("162 - Turecki"));
	encs.Add(_("163 - Wietnamski"));
	encs.Add(_("177 - Hebrajski"));
	encs.Add(_("178 - Arabski"));
	encs.Add(_("186 - Języki Bałtyckie"));
	encs.Add(_("204 - Rosyjski"));
	encs.Add(_("222 - Tajski"));
	encs.Add(_("238 - Europa Środkowa (Polski)"));
	encs.Add(_("255 - OEM"));
	
	wxStaticBoxSizer *styleenc= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Kodowanie tekstu:"));
	senc = new wxChoice(this, ID_CENCODING, wxDefaultPosition, wxDefaultSize, encs);
	styleenc->Add(senc,1,wxEXPAND|wxALL,2);

	wxStaticBoxSizer *styleprev= new wxStaticBoxSizer(wxHORIZONTAL, this, _("Podgląd stylu:"));
	//Preview= new wxTextCtrl(this, -1, Options.GetString("Preview Text"), wxDefaultPosition, wxSize(-1,100), wxTE_PROCESS_ENTER|wxTE_MULTILINE|wxTE_CENTRE);
	Preview= new StylePreview(this, -1, wxDefaultPosition, wxSize(-1,100));
	styleprev->Add(Preview,1,wxEXPAND|wxALL,2);

	wxBoxSizer *buttons=new wxBoxSizer(wxHORIZONTAL);
	Button1 = new wxButton(this, ID_BOK, "Ok", wxDefaultPosition, wxSize(50,-1));
	Button3 = new wxButton(this, ID_BCANCEL, _("Anuluj"));
	Button2 = new wxButton(this, ID_BONVID, _("Zastosuj"));
	Button4 = new wxButton(this, ID_BONFULL, _("Zobacz na pełnym ekranie"));
	buttons->Add(Button1,0,wxEXPAND|wxALL,2);
	buttons->Add(Button2,0,wxEXPAND|wxALL,2);
	buttons->Add(Button3,0,wxEXPAND|wxALL,2);
	buttons->Add(Button4,0,wxEXPAND|wxALL,2);

	//Main sizer
	Main->Add(stylename,0,wxEXPAND|wxALL,2);
	Main->Add(stylefont,0,wxEXPAND|wxALL,2);
	Main->Add(stylekol,0,wxEXPAND|wxALL,2);
	Main->Add(styleattr,0,wxEXPAND|wxALL,2);
	Main->Add(sizer2,0,wxEXPAND|wxALL,2);
	Main->Add(styleenc,0,wxEXPAND|wxALL,2);
	Main->Add(styleprev,0,wxEXPAND|wxALL,2);
	Main->Add(buttons,0,wxEXPAND|wxALL,2);
	
	SetSizerAndFit(Main);


	Connect(ID_BCOLOR1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::Ons1Click);
	Connect(ID_BCOLOR2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::Ons2Click);
	Connect(ID_BCOLOR3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::Ons3Click);
	Connect(ID_BCOLOR4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::Ons4Click);
	Connect(ID_BOK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::OnOKClick);
	Connect(ID_BCANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::OnCancelClick);
	Connect(ID_BONVID,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::OnStyleVideo);
	Connect(ID_BONFULL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorChange::OnStyleFull);
	Connect(ID_FONTNAME,wxEVT_COMMAND_COMBOBOX_SELECTED,(wxObjectEventFunction)&ColorChange::OnUpdatePreview);
	Connect(ID_TOUTLINE,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorChange::OnUpdatePreview);
	Connect(ID_CBOLD,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ColorChange::OnUpdatePreview);
	Connect(ID_CENCODING,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ColorChange::OnUpdatePreview);
	
	DoTooltips();
	block=false;
}

ColorChange::~ColorChange()
{
	wxDELETE(tab);
}

void ColorChange::OnAllCols(int kol)
{
	wxButton *kolor=(kol==1)? s1 : (kol==2)? s2 : (kol==3)? s3 : s4;
	NumCtrl *alpha=(kol==1)? alpha1 : (kol==2)? alpha2 : (kol==3)? alpha3 : alpha4;
	wxColour koll= kolor->GetBackgroundColour();

	DialogColorPicker *ColourDialog = DialogColorPicker::Get(this,wxColour(koll.Red(),koll.Green(),koll.Blue(),alpha->GetInt()));
	wxPoint mst=wxGetMousePosition();
	int dw, dh;
	wxSize siz=ColourDialog->GetSize();
	siz.x;
	wxDisplaySize (&dw, &dh);
	mst.x-=(siz.x/2);
	mst.x=MID(0,mst.x, dw-siz.x);
	mst.y+=15;
	ColourDialog->Move(mst);
	if ( ColourDialog->ShowModal() == wxID_OK) {
		wxColour kol=ColourDialog->GetColor();
		kolor->SetBackgroundColour(kol);
		kolor->SetForegroundColour(Blackorwhite(kol));
		alpha->SetInt(kol.Alpha());
		UpdatePreview();
	}
}

void ColorChange::Ons1Click(wxCommandEvent& event)
{
	OnAllCols(1);
}

void ColorChange::Ons2Click(wxCommandEvent& event)
{
	OnAllCols(2);
}

void ColorChange::Ons3Click(wxCommandEvent& event)
{
	OnAllCols(3);
}

void ColorChange::Ons4Click(wxCommandEvent& event)
{
	OnAllCols(4);
}

void ColorChange::OnOKClick(wxCommandEvent& event)
{
	UpdateStyle();
	Hide();
	//kopiujemy, bo by zapobiec wyciekom należy tab niezwłocznie usunąć.
    SS->changestyle(tab->Copy());
	wxDELETE(tab);
	  
}

void ColorChange::OnCancelClick(wxCommandEvent& event)
{
	Hide();
	wxDELETE(tab);
	SS->Mainall->Fit(SS);
}

void ColorChange::UpdateValues(Styles *styless)
{
	block=true;
	wxDELETE(tab);
	tab=styless;
	sname->SetValue(tab->Name);
	int sell=sfont->FindString(tab->Fontname);
	if(sell==-1){
		sfont->SetValue(tab->Fontname);}
	else{sfont->SetSelection(sell);}
	  
    ssize->SetString(tab->Fontsize);
	wxColour kol=tab->PrimaryColour.GetWX();
    s1->SetBackgroundColour(kol);
	s1->SetForegroundColour(Blackorwhite(kol));
	kol = tab->SecondaryColour.GetWX();
	s2->SetBackgroundColour(kol);
	s2->SetForegroundColour(Blackorwhite(kol));
	kol = tab->OutlineColour.GetWX();
	s3->SetBackgroundColour(kol);
	s3->SetForegroundColour(Blackorwhite(kol));
	kol = tab->BackColour.GetWX();
	s4->SetBackgroundColour(kol);
	s4->SetForegroundColour(Blackorwhite(kol));
     
    alpha1->SetInt(tab->PrimaryColour.a);
    alpha2->SetInt(tab->SecondaryColour.a);
    alpha3->SetInt(tab->OutlineColour.a);
    alpha4->SetInt(tab->BackColour.a);
    sb->SetValue(tab->Bold);
    si->SetValue(tab->Italic);
    su->SetValue(tab->Underline);
    ss->SetValue(tab->StrikeOut);
    san->SetString(tab->Angle);
    ssp->SetString(tab->Spacing);
    sou->SetString(tab->Outline);
    ssh->SetString(tab->Shadow);
    sob->SetValue(tab->BorderStyle);
    //if(tab->BorderStyle){sob->SetValue(true);}else{sob->SetValue(false);};
    wxString an=tab->Alignment;
    if(an=="1"){rb1->SetValue(true);}else if(an=="2"){rb2->SetValue(true);}else if(an=="3"){rb3->SetValue(true);}else if(an=="4"){rb4->SetValue(true);}
    else if(an=="5"){rb5->SetValue(true);}else if(an=="6"){rb6->SetValue(true);}else if(an=="7"){rb7->SetValue(true);}
    else if(an=="8"){rb8->SetValue(true);}else if(an=="9"){rb9->SetValue(true);};
    ssx->SetString(tab->ScaleX);
    ssy->SetString(tab->ScaleY);
    sml->SetString(tab->MarginL);
    smr->SetString(tab->MarginR);
    smv->SetString(tab->MarginV);
    int choice=-1;
	for(size_t i=0; i<encs.size();i++){
		if(encs[i].StartsWith(tab->Encoding+" ")){choice=i;break;}
	}
    if(choice==-1){choice=1;}
    senc->SetSelection(choice);
	block=false;
	UpdatePreview();
    Show();
}

void ColorChange::OnStyleVideo(wxCommandEvent& event)
{
	UpdateStyle();
	//tu tab zostaje bo w przeciwnym wypadku dialog straci swoją klasę a przecież jest jeszcze widoczny.
	SS->changestyle(tab->Copy());
}

void ColorChange::OnStyleFull(wxCommandEvent& event)
{
	UpdateStyle();
	SS->StyleonVideo(tab,true);
}

void ColorChange::UpdateStyle()
{
	if(!tab){return;}
	tab->Name = sname->GetValue();
	tab->Fontname = sfont->GetValue();
	tab->Fontsize = ssize->GetString();
	tab->PrimaryColour.SetWX(s1->GetBackgroundColour(),alpha1->GetInt());
	tab->SecondaryColour.SetWX(s2->GetBackgroundColour(),alpha2->GetInt());
	tab->OutlineColour.SetWX(s3->GetBackgroundColour(),alpha3->GetInt());
	tab->BackColour.SetWX(s4->GetBackgroundColour(),alpha4->GetInt());
	tab->Bold = sb->GetValue();
	tab->Italic = si->GetValue();
	tab->Underline = su->GetValue();
	tab->StrikeOut = ss->GetValue();
	tab->Angle = san->GetString();
	tab->Spacing = ssp->GetString();
	tab->Outline = sou->GetString();
	tab->Shadow = ssh->GetString();
	tab->BorderStyle = sob->GetValue();
	wxString an;
	if(rb1->GetValue()){an="1";}else if(rb2->GetValue()){an="2";}else if(rb3->GetValue()){an="3";}else if(rb4->GetValue()){an="4";}
	else if(rb5->GetValue()){an="5";}else if(rb6->GetValue()){an="6";}else if(rb7->GetValue()){an="7";}
	else if(rb8->GetValue()){an="8";}else if(rb9->GetValue()){an="9";};
	tab->Alignment = an;
	tab->ScaleX = ssx->GetString();
	tab->ScaleY = ssy->GetString();
	tab->MarginL = sml->GetString();
	tab->MarginR = smr->GetString();
	tab->MarginV = smv->GetString();
	tab->Encoding = senc->GetString(senc->GetSelection()).BeforeFirst(' ');
}

void ColorChange::UpdatePreview()
{
	if(!Preview)return;
	UpdateStyle();
	Preview->DrawPreview(tab);
}

void ColorChange::OnUpdatePreview(wxCommandEvent& event)
{
	if(!block) {
		UpdatePreview();
	}
}

void ColorChange::DoTooltips()
{
	sname->SetToolTip(_("Nazwa stylu"));
	sfont->SetToolTip(_("Czcionka"));
	ssize->SetToolTip(_("Rozmiar czcionki"));
	sb->SetToolTip(_("Pogrubienie"));
	si->SetToolTip(_("Pochylenie"));
	su->SetToolTip(_("Podkreślenie"));
	ss->SetToolTip(_("Przekreślenie"));
	s1->SetToolTip(_("Kolor podstawowy"));
	s2->SetToolTip(_("Kolor zastępczy do karaoke"));
	s3->SetToolTip(_("Kolor obwódki"));
	s4->SetToolTip(_("Kolor cienia"));
	alpha1->SetToolTip(_("Przezroczystość koloru podstawowego, 0 - brak, 255 - przezroczystość"));
	alpha2->SetToolTip(_("Przezroczystość koloru zastępczego, 0 - brak, 255 - przezroczystość"));
	alpha3->SetToolTip(_("Przezroczystość koloru obwódki, 0 - brak, 255 - przezroczystość"));
	alpha4->SetToolTip(_("Przezroczystość koloru cienia, 0 - brak, 255 - przezroczystość"));
	sou->SetToolTip(_("Obwódka w pikselach"));
	ssh->SetToolTip(_("Cień w pikselach"));
	ssx->SetToolTip(_("Skala X w procentach"));
	ssy->SetToolTip(_("Skala Y w procentach"));
	san->SetToolTip(_("Kąt w stopniach"));
	ssp->SetToolTip(_("Odstępy między literami w pikselach (wartości ujemne dozwolone)"));
	sob->SetToolTip(_("Prostokątna obwódka"));
	smr->SetToolTip(_("Margines prawy"));
	sml->SetToolTip(_("Margines lewy"));
	smv->SetToolTip(_("Margines górny i dolny"));
	rb1->SetToolTip(_("Lewy dolny róg"));
	rb2->SetToolTip(_("Wyśrodkowane na dole"));
	rb3->SetToolTip(_("Prawy dolny róg"));
	rb4->SetToolTip(_("Wyśrodkowane po lewej"));
	rb5->SetToolTip(_("Wyśrodkowane"));
	rb6->SetToolTip(_("Wyśrodkowane po prawej"));
	rb7->SetToolTip(_("Lewy górny róg"));
	rb8->SetToolTip(_("Wyśrodkowane u góry"));
	rb9->SetToolTip(_("Prawy górny róg"));
	senc->SetToolTip(_("Kodowanie tekstu"));
}
