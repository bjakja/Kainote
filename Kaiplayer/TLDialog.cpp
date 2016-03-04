#include "TLDialog.h"

TLDialog::TLDialog(wxWindow *parent, Grid *subsgrid)
	: wxDialog(parent,-1,_("Opcje dopasowywania tłumaczenia"), wxDefaultPosition, wxDefaultSize,wxDEFAULT_DIALOG_STYLE)
{
	Sbsgrid=subsgrid;

	wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxVERTICAL,this,_("Przesuwanie tekstu tłumaczenia"));
	wxGridSizer *sizer1 = new wxGridSizer(2,2,2);

	Up=new wxButton(this,29995,_("Usuń linię"));
	Up->SetToolTip(_("Usuwa zaznaczoną linijkę.\nTłumaczenie idzie do góry."));
	Down=new wxButton(this,29997,_("Dodaj linię"));
	Down->SetToolTip(_("Dodaje pustą linijkę przed zaznaczoną.\nTłumaczenie idzie w dół."));
	UpJoin=new wxButton(this,29998,_("Złącz linie"));
	UpJoin->SetToolTip(_("Złącza następną linijkę z zaznaczoną.\nTłumaczenie idzie do góry."));
	DownJoin=new wxButton(this,29996,_("Złącz linie"));
	DownJoin->SetToolTip(_("Złącza następną linijkę z zaznaczoną.\nOryginał idzie w górę."));
	DownDel=new wxButton(this,29994,_("Usuń linię"));
	DownDel->SetToolTip(_("Usuwa zaznaczoną linijkę.\nOryginał idzie do góry."));
	UpExt=new wxButton(this,29993,_("Dodaj linię"));
	UpExt->SetToolTip(_("Dodaje pustą linijkę przed zaznaczoną.\nOryginał idzie w dół.\nDodanej linii należy ustawić czasy."));
	
	sizer1->Add(new wxStaticText(this,-1,_("Tekst oryginału")),0,wxALL|wxEXPAND,5);
	sizer1->Add(new wxStaticText(this,-1,_("Tekst tłumaczenia")),0,wxALL|wxEXPAND,5);
	sizer1->Add(DownJoin,0,wxALL|wxEXPAND,5);
	sizer1->Add(Up,0,wxALL|wxEXPAND,5);
	sizer1->Add(DownDel,0,wxALL|wxEXPAND,5);
	sizer1->Add(Down,0,wxALL|wxEXPAND,5);
	sizer1->Add(UpExt,0,wxALL|wxEXPAND,5);
	sizer1->Add(UpJoin,0,wxALL|wxEXPAND,5);
	
	sizer->Add(sizer1,0,wxEXPAND,0);
	sizer->Add(new wxStaticText(this,-1,_("Objaśnienie:\noryginał - tekst napisów, do których przekładamy tekst innych\ntłumaczenie - tekst, który przekładamy do timingu wczytanych napisów")),0,wxEXPAND,0);
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