#include "TLDialog.h"

TLDialog::TLDialog(wxWindow *parent, Grid *subsgrid)
	: wxDialog(parent,-1,"Opcje przesuwania tekstu t³umaczenia", wxDefaultPosition, wxDefaultSize,wxDEFAULT_DIALOG_STYLE)
{
	Sbsgrid=subsgrid;

	wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Przesuwanie tekstu t³umaczenia");
	wxGridSizer *sizer1 = new wxGridSizer(2,2,2);
	//wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	//wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	Up=new wxButton(this,29995,"W górê usuñ t³umaczenie");
	Up->SetToolTip("Przenosi t³umaczenie w górê,\nusuwaj¹c przy tym tekst pierwszej zaznaczonej");
	Down=new wxButton(this,29997,"W dó³ zostaw puste pole");
	Down->SetToolTip("Przenosi t³umaczenie w dó³,\npozostawiaj¹c puste miejsce");
	UpJoin=new wxButton(this,29998,"W górê z³¹cz t³umaczenie");
	UpJoin->SetToolTip("Przenosi t³umaczenie w górê,\nz³¹cza t³umaczenie zamiast go usuwaæ");
	DownJoin=new wxButton(this,29996,"W górê z³¹cz orygina³");
	DownJoin->SetToolTip("Przenosi orygina³ w górê,\nz³¹czaj¹c pierwsz¹ zaznaczon¹ linijkê z nastêpn¹");
	DownDel=new wxButton(this,29994,"W górê usuñ orygina³");
	DownDel->SetToolTip("Przenosi orygina³ w górê,\nusuwaj¹c puste pole");
	UpExt=new wxButton(this,29993,"W dó³");
	UpExt->SetToolTip("Przenosi orygina³ w dó³,\npozostawiaj¹c puste pole,\ndodan¹ linijkê nale¿y stimingowaæ");
	sizer1->Add(new wxStaticText(this,-1,"Tekst t³umaczenia"),0,wxALL|wxEXPAND,5);
	sizer1->Add(new wxStaticText(this,-1,"Tekst orygina³u"),0,wxALL|wxEXPAND,5);
	sizer1->Add(Up,0,wxALL|wxEXPAND,5);
	sizer1->Add(DownJoin,0,wxALL|wxEXPAND,5);
	sizer1->Add(Down,0,wxALL|wxEXPAND,5);
	sizer1->Add(DownDel,0,wxALL|wxEXPAND,5);
	sizer1->Add(UpJoin,0,wxALL|wxEXPAND,5);
	sizer1->Add(UpExt,0,wxALL|wxEXPAND,5);
	
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