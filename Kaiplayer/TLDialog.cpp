#include "TLDialog.h"

TLDialog::TLDialog(wxWindow *parent, Grid *subsgrid)
	: wxDialog(parent,-1,_("Opcje dopasowywania t³umaczenia"), wxDefaultPosition, wxDefaultSize,wxDEFAULT_DIALOG_STYLE)
{
	Sbsgrid=subsgrid;

	wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxVERTICAL,this,_("Przesuwanie tekstu t³umaczenia"));
	wxGridSizer *sizer1 = new wxGridSizer(2,2,2);
	//wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	//wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	Up=new wxButton(this,29995,_("Usuñ liniê"));
	Up->SetToolTip(_("Usuwa zaznaczon¹ linijkê.\nT³umaczenie idzie do góry."));
	Down=new wxButton(this,29997,_("Dodaj liniê"));
	Down->SetToolTip(_("Dodaje pust¹ linijkê przed zaznaczon¹.\nT³umaczenie idzie w dó³."));
	UpJoin=new wxButton(this,29998,_("Z³¹cz linie"));
	UpJoin->SetToolTip(_("Z³¹cza nastêpn¹ linijkê z zaznaczon¹.\nT³umaczenie idzie do góry."));
	DownJoin=new wxButton(this,29996,_("Z³¹cz linie"));
	DownJoin->SetToolTip(_("Z³¹cza nastêpn¹ linijkê z zaznaczon¹.\nOrygina³ idzie w górê."));
	DownDel=new wxButton(this,29994,_("Usuñ liniê"));
	DownDel->SetToolTip(_("Usuwa zaznaczon¹ linijkê.\nOrygina³ idzie do góry."));
	UpExt=new wxButton(this,29993,_("Dodaj liniê"));
	UpExt->SetToolTip(_("Dodaje pust¹ linijkê przed zaznaczon¹.\nOrygina³ idzie w dó³.\nDodanej linii nale¿y ustawiæ czasy."));
	
	sizer1->Add(new wxStaticText(this,-1,_("Tekst orygina³u")),0,wxALL|wxEXPAND,5);
	sizer1->Add(new wxStaticText(this,-1,_("Tekst t³umaczenia")),0,wxALL|wxEXPAND,5);
	sizer1->Add(DownJoin,0,wxALL|wxEXPAND,5);
	sizer1->Add(Up,0,wxALL|wxEXPAND,5);
	sizer1->Add(DownDel,0,wxALL|wxEXPAND,5);
	sizer1->Add(Down,0,wxALL|wxEXPAND,5);
	sizer1->Add(UpExt,0,wxALL|wxEXPAND,5);
	sizer1->Add(UpJoin,0,wxALL|wxEXPAND,5);
	
	sizer->Add(sizer1,0,wxEXPAND,0);
	sizer->Add(new wxStaticText(this,-1,"Objaœnienie:\norygina³ - tekst napisów, do których przek³adamy tekst innych\nt³umaczenie - tekst, który przek³adamy do timingu wczytanych napisów"),0,wxEXPAND,0);
	//sizer->Add(sizer3,0,wxEXPAND,0);
	SetSizerAndFit(sizer);
	CenterOnParent();

	Connect(29997,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TLDialog::OnDown);
	Connect(29998,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TLDialog::OnUpJoin);
	Connect(29995,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TLDialog::OnUp);
	Connect(29996,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TLDialog::OnDownJoin);
	Connect(29993,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TLDialog::OnUpExt);
	Connect(29994,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TLDialog::OnDownDel);
}

TLDialog::~TLDialog()
{

}

void TLDialog::OnDown(wxCommandEvent& event)
{
	Sbsgrid->MoveTextTL(3);
}

void TLDialog::OnUp(wxCommandEvent& event)
{
	Sbsgrid->MoveTextTL(0);
}

void TLDialog::OnUpJoin(wxCommandEvent& event)
{
	Sbsgrid->MoveTextTL(1);
}

void TLDialog::OnDownJoin(wxCommandEvent& event)
{
	Sbsgrid->MoveTextTL(4);
}

void TLDialog::OnUpExt(wxCommandEvent& event)
{
	Sbsgrid->MoveTextTL(2);
}

void TLDialog::OnDownDel(wxCommandEvent& event)
{
	Sbsgrid->MoveTextTL(5);
}