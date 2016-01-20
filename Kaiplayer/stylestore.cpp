#include "stylestore.h"
#include "kainoteMain.h"

#include "config.h"
#include "NewCatalog.h"
#include "OpennWrite.h"
#include "Stylelistbox.h"
#include <wx/tokenzr.h>
#include <vector>
#include "Styles.h"
#include <wx/intl.h>
#include <wx/string.h>
#include "ColorChange.h"
#include <wx/fontenum.h>


stylestore::stylestore(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
	: wxDialog(parent,id,_("Mened¿er stylów"),pos,size,wxDEFAULT_DIALOG_STYLE)
{

	wxAcceleratorEntry centries[1];
	centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, ID_CONF);
	wxAcceleratorTable caccel(1, centries);
	this->SetAcceleratorTable(caccel);

	wxFont thisFont(8,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
	SetFont(thisFont);

	wxIcon icn;
	icn.CopyFromBitmap(wxBITMAP_PNG("styles"));
	SetIcon(icn);

	cc=new ColorChange(this,-1);
	cc->SS=this;
	cc->Hide();

	wxBoxSizer *Mainsm= new wxBoxSizer(wxVERTICAL);
	Mainall= new wxBoxSizer(wxHORIZONTAL);

	wxStaticBoxSizer *katsbs=new wxStaticBoxSizer(wxHORIZONTAL, this, _("Katalog"));
	Choice1 = new wxChoice(this, ID_CATALOG, wxDefaultPosition, wxDefaultSize, Options.dirs);
	int chc=Choice1->FindString(Options.acdir);
	Choice1->SetSelection(chc);
	Button7 = new wxButton(this, ID_NEWCAT, _("Nowy"));
	wxButton *delcat = new wxButton(this, ID_DELCAT, _("Usuñ"));
	katsbs->Add(Choice1,4,wxEXPAND|wxALL,2);
	katsbs->Add(Button7,0,wxEXPAND|wxLEFT,5);
	katsbs->Add(delcat,0,wxEXPAND|wxLEFT,5);


	wxStaticBoxSizer *katsbs1=new wxStaticBoxSizer(wxHORIZONTAL, this, _("Style katalogu"));
	wxBoxSizer *katbutt=new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *katall=new wxBoxSizer(wxHORIZONTAL);

	Store = new StyleList(this, ID_STORESTYLES, &Options.assstore, cc->sfont, wxDefaultPosition, wxSize(-1,200));

	Button5 = new wxButton(this, ID_STORENEW, _("Nowy"),wxDefaultPosition, wxSize(50,-1));
	Button8 = new wxButton(this, ID_STORECOPY, _("Kopiuj"));
	Button9 = new wxButton(this, ID_STORELOAD, _("Wczytaj"));
	Button12 = new wxButton(this, ID_STOREDEL, _("Usuñ"));
	Button13 = new wxButton(this, ID_STORESORT, _("Sortuj"));

	katbutt->Add(Button5,1,wxEXPAND|wxALL,5);
	katbutt->Add(Button8,1,wxEXPAND|wxALL,5);
	katbutt->Add(Button9,1,wxEXPAND|wxALL,5);
	katbutt->Add(Button12,1,wxEXPAND|wxALL,5);
	katbutt->Add(Button13,1,wxEXPAND|wxALL,5);

	katall->Add(Store,3,wxEXPAND,0);
	katall->Add(katbutt,1,wxEXPAND,0);

	katsbs1->Add(katall,0,wxEXPAND|wxALL,2);

	wxBoxSizer *butts=new wxBoxSizer(wxHORIZONTAL);

	Button3 = new wxButton(this, ID_ADDTOSTORE, _("^ Dodaj do magazynu"));
	Button4 = new wxButton(this, ID_ADDTOASS, _("v Dodaj do ASS"));
	butts->Add(Button3,5,wxEXPAND|wxALL,5);
	butts->Add(Button4,5,wxEXPAND|wxALL,5);
	butts->AddStretchSpacer(3);

	wxStaticBoxSizer *asssbs=new wxStaticBoxSizer(wxHORIZONTAL, this, _("Style pliku ASS"));
	wxBoxSizer *assbutt=new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *assall=new wxBoxSizer(wxHORIZONTAL);

	ASS = new StyleList(this, ID_ASSSTYLES, Notebook::GetTab()->Grid1->GetStyleTable(), cc->sfont, wxDefaultPosition, wxSize(-1,200));


	Button6 = new wxButton(this, ID_ASSNEW, _("Nowy"),wxDefaultPosition, wxSize(50,-1));
	Button10 = new wxButton(this, ID_ASSCOPY, _("Kopiuj"));
	Button11 = new wxButton(this, ID_ASSLOAD, _("Wczytaj"));
	Button14 = new wxButton(this, ID_ASSDEL, _("Usuñ"));
	Button15 = new wxButton(this, ID_ASSSORT, _("Sortuj"));

	assbutt->Add(Button6,1,wxEXPAND|wxALL,5);
	assbutt->Add(Button10,1,wxEXPAND|wxALL,5);
	assbutt->Add(Button11,1,wxEXPAND|wxALL,5);
	assbutt->Add(Button14,1,wxEXPAND|wxALL,5);
	assbutt->Add(Button15,1,wxEXPAND|wxALL,5);

	assall->Add(ASS,3,wxEXPAND,0);
	assall->Add(assbutt,1,wxEXPAND,0);

	asssbs->Add(assall,0,wxEXPAND|wxALL,2);

	Button2 = new wxButton(this, ID_CLOSE, _("Zamknij"));

	Mainsm->Add(katsbs,0,wxEXPAND|wxALL,2);
	Mainsm->Add(katsbs1,0,wxEXPAND|wxALL,2);
	Mainsm->Add(butts,0,wxEXPAND|wxALL,2);
	Mainsm->Add(asssbs,0,wxEXPAND|wxALL,2);
	Mainsm->Add(Button2,0,wxALIGN_CENTER|wxALL,2);



	Mainall->Add(Mainsm,0,wxEXPAND);
	Mainall->Add(cc,0,wxEXPAND|wxLEFT,5);
	SetEscapeId(ID_CLOSE);
	SetSizerAndFit(Mainall);

	Connect(ID_ASSSTYLES,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&stylestore::OnAssStyleChange);
	Connect(ID_ASSSTYLES,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&stylestore::OnSwitchLines);
	Connect(ID_STORESTYLES,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&stylestore::OnStoreStyleChange);
	Connect(ID_ADDTOSTORE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnAddToStore);
	Connect(ID_ADDTOASS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnAddToAss);
	Connect(ID_STORENEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnStoreNew);
	Connect(ID_ASSNEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnAssNew);
	Connect(ID_CATALOG,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&stylestore::OnChangeCatalog);
	Connect(ID_NEWCAT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnNewCatalog);
	Connect(ID_DELCAT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnDelCatalog);
	Connect(ID_STORECOPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnStoreCopy);
	Connect(ID_STORELOAD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnStoreLoad);
	Connect(ID_STOREDEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnStoreDelete);
	Connect(ID_STORESORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnStoreSort);
	Connect(ID_ASSCOPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnAssCopy);
	Connect(ID_ASSLOAD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnAssLoad);
	Connect(ID_ASSDEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnAssDelete);
	Connect(ID_ASSSORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnAssSort);
	Connect(ID_CONF,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&stylestore::OnConfirm);
	Connect(ID_CLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&stylestore::OnClose);
	Connect(wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&stylestore::OnClose);


	DoTooltips();

	stopcheck=false;
	thread = NULL;
	thread = CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)CheckFontProc, this, 0, 0);
	SetThreadPriority(thread,THREAD_PRIORITY_LOWEST);
}

stylestore::~stylestore()
{
	stopcheck=true;
	WaitForSingleObject(thread,2000);
	CloseHandle(thread);
}

void stylestore::OnSwitchLines(wxCommandEvent& event)
{
	Notebook::GetTab()->Edit->RefreshStyle();
}


void stylestore::OnAssStyleChange(wxCommandEvent& event)
{
	wxArrayInt selects;
	int kkk=ASS->GetSelections(selects);
	if(kkk<1){wxBell();return;}
	selnum=selects[0];
	stass=true;
	dummy=false;
	StylesWindow();
	//modif();
}
void stylestore::OnStoreStyleChange(wxCommandEvent& event)
{
	wxArrayInt selects;
	int kkk= Store->GetSelections(selects);
	if(kkk<1){wxBell();return;}
	selnum=selects[0];
	stass=false;
	dummy=false;
	StylesWindow();
	//modif();
}

void stylestore::OnAddToStore(wxCommandEvent& event)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	wxArrayInt sels;
	int kkk=ASS->GetSelections(sels);
	if(kkk<1){wxBell();return;}
	Store->SetSelection(wxNOT_FOUND);
	for(size_t i=0;i<sels.GetCount();i++){
		Styles *stylc = grid->GetStyle(sels[i])->Copy();
		int found=Options.FindStyle(stylc->Name);
		if(found!=-1){
			if(wxMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmieniæ go?"),
				stylc->Name), _("Potwierdzenie"),wxICON_QUESTION | wxYES_NO, this) == wxYES ){
					Options.ChangeStyle(stylc,found);Store->SetSelection(found);}
		}else{Options.AddStyle(stylc);Store->SetSelection(Options.StoreSize()-1);}
	}

	//modif();
}

void stylestore::OnAddToAss(wxCommandEvent& event)
{
	//wxMutexLocker lock(mutex);
	Grid* grid=Notebook::GetTab()->Grid1;
	wxArrayInt sels;
	int kkk=Store->GetSelections(sels);
	if(kkk<1){wxBell();return;}
	ASS->SetSelection(wxNOT_FOUND);
	for(int i=0;i<kkk;i++)
	{
		Styles *stylc = Options.GetStyle(sels[i])->Copy();
		int found=grid->FindStyle(stylc->Name);
		if(found!=-1){if (wxMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmieniæ go?"),
			stylc->Name), _("Potwierdzenie"),wxICON_QUESTION | wxYES_NO, this) == wxYES ){
				grid->ChangeStyle(stylc,found);ASS->SetSelection(found);}
		}else{grid->AddStyle(stylc);ASS->SetSelection(grid->StylesSize()-1);}
	}
	modif();
}

void stylestore::OnStoreDelete(wxCommandEvent& event)
{
	wxArrayInt sels;
	int kkk=Store->GetSelections(sels);
	if(kkk<1){wxBell();return;}
	for(int ii=sels.GetCount()-1;ii>=0;ii--)
	{
		Options.DelStyle(sels[ii]);
	}
	Store->SetSelection(0,true);
	//modif();
}

void stylestore::OnAssDelete(wxCommandEvent& event)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	wxArrayInt sels;
	int kkk=ASS->GetSelections(sels);
	if(kkk<1){wxBell();return;}
	for(int ii=sels.GetCount()-1;ii>=0;ii--)
	{
		grid->DelStyle(sels[ii]);
	}

	ASS->SetSelection(0,true);
	modif();
}

void stylestore::StylesWindow(wxString newname)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	Styles *tab=NULL;
	if (selnum<0){tab=new Styles();}
	else if (stass){tab=grid->GetStyle(selnum)->Copy();}
	else{tab=Options.GetStyle(selnum)->Copy();}
	if(newname!=""){tab->Name=newname;}
	oldname=tab->Name;
	cc->UpdateValues(tab);
	Mainall->Fit(this);

}

void stylestore::changestyle(Styles *cstyl)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	int mult=0;
	int fres= (stass)? grid->FindStyle(cstyl->Name, &mult) : Options.FindStyle(cstyl->Name, &mult);
	if(!dummy){
		if(stass && selnum>=grid->StylesSize()){dummy=true;}
		else if(!stass && selnum>=Options.StoreSize()){dummy=true;}
		//selnum=(stass)? grid->FindStyle(oldname) : Options.FindStyle(oldname); if(selnum<0){dummy=true;}
	}


	if(fres!=-1 && dummy || (mult>1 || mult==1 && oldname!=cstyl->Name) && !dummy)
	{
		Mainall->Fit(this);
		wxMessageBox(wxString::Format(_("Styl o nazwie \"%s\" jest ju¿ na liœcie."), cstyl->Name));
		return;
	}
	if(fres!=-1){selnum=fres;}
	else if(!dummy){selnum= (stass)? grid->FindStyle(oldname) : Options.FindStyle(oldname);}
	if(dummy){
		if (stass){
			grid->AddStyle(cstyl);
			ASS->SetSelection(grid->StylesSize()-1,true);
			selnum=grid->StylesSize()-1;}
		else{
			Options.AddStyle(cstyl);
			Store->SetSelection(Options.StoreSize()-1, true);
			selnum=Options.StoreSize()-1;}
		//dummy=false;
	}else{
		if (stass){grid->ChangeStyle(cstyl,selnum);ASS->Refresh(false);}
		else{Options.ChangeStyle(cstyl,selnum);Store->Refresh(false);}
	}
	Mainall->Fit(this);
	if(oldname!=cstyl->Name && stass && !dummy){
		int res=wxMessageBox(_("Nazwa stylu zosta³a zmieniona, czy chcesz zmieniæ j¹ tak¿e w napisach?"), _("Potwierdzenie"), wxYES_NO);
		if(res==wxYES){
			for(int i=0; i<grid->GetCount(); i++){
				if(grid->GetDial(i)->Style==oldname)
				{
					grid->CopyDial(i)->Style=cstyl->Name;
				}
			}
			grid->AdjustWidths(STYLE);
		}
	}
	oldname=cstyl->Name;
	dummy=false;
	modif();
}

void stylestore::OnChangeCatalog(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	Options.LoadStyles(Choice1->GetString(Choice1->GetSelection()));

	Store->SetSelection(0,true);
}

void stylestore::OnNewCatalog(wxCommandEvent& event)
{
	NewCatalog nc(this);
	if(nc.ShowModal()==wxID_OK){
		wxString nkat=nc.TextCtrl1->GetValue();
		Choice1->SetSelection(Choice1->Append(nkat));
		Options.dirs.Add(nkat);
		Options.acdir=nkat;
		Options.clearstyles();
		Store->Refresh(false);
	}
}

void stylestore::OnDelCatalog(wxCommandEvent& event)
{
	int cat=Choice1->GetSelection();
	if(cat==-1){return;}
	wxString Cat = Choice1->GetString(cat);
	if(Cat=="Podstawowy"){wxBell();return;}
	Choice1->Delete(cat);
	Options.dirs.RemoveAt(cat);
	Options.acdir=Options.dirs[MAX(0,cat-1)];
	Choice1->SetSelection(MAX(0,cat-1));
	wxString path;
	path<<Options.pathfull<<_T("\\Katalogi\\")<<Cat<<_T(".sty");
	wxRemoveFile(path);
	Options.LoadStyles(Options.acdir);
	Store->Refresh(false);
}

void stylestore::OnStoreLoad(wxCommandEvent& event)
{
	LoadStylesS(false);
}

void stylestore::OnAssSort(wxCommandEvent& event)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	std::sort(grid->GetStyleTable()->begin(), grid->GetStyleTable()->end(),sortfunc);
	ASS->SetSelection(0,true);

	modif();
}

void stylestore::OnAssLoad(wxCommandEvent& event)
{
	LoadStylesS(true);
}

void stylestore::OnStoreSort(wxCommandEvent& event)
{
	Options.Sortstyles();
	Store->SetSelection(0,true);

	//modif();
}

void stylestore::LoadStylesS(bool isass)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	wxFileDialog *openFileDialog= new wxFileDialog(this, _("Wybierz plik ASS"), 
		Notebook::GetTab()->SubsPath.BeforeLast('\\'), _T("*.ass"), _("Pliki napisów ASS(*.ass)|*.ass"), 
		wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	if (openFileDialog->ShowModal() == wxID_OK){
		OpenWrite op;
		wxString ass= op.FileOpen(openFileDialog->GetPath());
		size_t start=ass.find(_T("\nStyle: "));
		size_t end=ass.find(_T("[Events]"));
		if(end<=start){return;}
		std::vector<Styles*> tmps;
		wxString styless=ass.SubString(start,end);
		//wxMessageBox(styless);
		wxStringTokenizer styletkn(styless,_T("\n"));
		while(styletkn.HasMoreTokens())
		{
			wxString token=styletkn.NextToken();
			if (token.StartsWith(_T("Style: "))){
				tmps.push_back(new Styles(token));
			}
		}
		Stylelistbox stl(this);
		for(size_t i=0;i<tmps.size();i++){

			stl.CheckListBox1->Append(tmps[i]->Name);
		}
		if(isass){ASS->SetSelection(wxNOT_FOUND);}else{Store->SetSelection(wxNOT_FOUND);}
		if(stl.ShowModal()== wxID_OK){
			for (size_t v=0;v<stl.CheckListBox1->GetCount();v++)
			{
				if(stl.CheckListBox1->IsChecked(v)){
					if(isass){
						int fstyle=grid->FindStyle(stl.CheckListBox1->GetString(v));
						if(fstyle==-1){grid->AddStyle(tmps[v]);ASS->Refresh(false);}
						else{
							if(wxMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmieniæ go?"),
								stl.CheckListBox1->GetString(v)), _("Potwierdzenie"),wxICON_QUESTION | wxYES_NO, this) == wxYES ){
									grid->ChangeStyle(tmps[v],fstyle);ASS->SetSelection(fstyle);
							}
						}
					}else{
						int fsstyle=Options.FindStyle(stl.CheckListBox1->GetString(v));
						if(fsstyle==-1){Options.AddStyle(tmps[v]);Store->Refresh(false);}
						else{
							if(wxMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmieniæ go?"),
								stl.CheckListBox1->GetString(v)), _("Potwierdzenie"),wxICON_QUESTION | wxYES_NO, this) == wxYES ){
									Options.ChangeStyle(tmps[v],fsstyle);Store->SetSelection(fsstyle);}
						}   
					}
				}
			}
		}
		tmps.clear();
	}
	openFileDialog->Destroy();
	if(isass){modif();}
}

void stylestore::OnAssCopy(wxCommandEvent& event)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	wxArrayInt selects;
	int kkk=ASS->GetSelections(selects);
	if(kkk<1){wxBell();return;}
	Styles *kstyle=grid->GetStyle(selects[0]);
	selnum=selects[0];
	stass=true;
	dummy=true;
	StylesWindow(_("Kopia ")+kstyle->Name);
	modif();
}
void stylestore::OnStoreCopy(wxCommandEvent& event)
{
	wxArrayInt selects;
	int kkk=Store->GetSelections(selects);
	if(kkk<1){wxBell();return;}
	Styles *kstyle=Options.GetStyle(selects[0]);
	selnum=selects[0];
	stass=false;
	dummy=true;
	StylesWindow(_("Kopia ")+kstyle->Name);
	//modif();
}

void stylestore::OnStoreNew(wxCommandEvent& event)
{
	bool gname=true;
	int count=0;
	selnum=-1;
	stass=false;
	dummy=true;
	while(gname){
		wxString ns=_("Nowy Styl");
		wxString nss=(count==0)?ns:ns<<count;
		if(Options.FindStyle(nss)==-1){StylesWindow(nss);break;}else{count++;}
	}
	//modif();
}

void stylestore::OnAssNew(wxCommandEvent& event)
{
	//Styles nstyle=Styles();
	bool gname=true;
	int count=0;
	selnum=-1;
	stass=true;
	dummy=true;

	while(gname){
		wxString ns=_("Nowy Styl");
		wxString nss=(count==0)?ns:ns<<count;
		if(Options.FindStyle(nss)==-1){StylesWindow(nss);break;}else{count++;}
	}
	modif();
}

void stylestore::StyleonVideo(Styles *styl, bool fullskreen)
{
	TabPanel* pan=Notebook::GetTab();
	Grid *grid=pan->Grid1;
	if(pan->Video->GetState()==None||pan->Video->GetState()==Stopped){return;}

	int wl=-1;
	int time=pan->Video->Tell();



	if(stass){

		int prevtime=0;
		int durtime=pan->Video->GetDuration();
		int ip=-1;
		int idr=-1;
		for(int i =0;i<grid->GetCount();i++)
		{
			Dialogue *dial=grid->GetDial(i);
			if(!dial->IsComment && (dial->Text!=""||dial->TextTl!="") && dial->Style==styl->Name){
				if(time>=dial->Start.mstime && time <= dial->End.mstime){
					pan->Edit->SetIt(i);grid->SelectRow(i);grid->ScrollTo(i-4);wl=i;ip=-1;idr=-1;
					break;
				}
				if(dial->Start.mstime > prevtime && dial->Start.mstime<time){prevtime = dial->Start.mstime;ip=i;}
				if(dial->Start.mstime<durtime && dial->Start.mstime>time){durtime = dial->Start.mstime; idr=i;}
			}

		}
		if(ip>=0){pan->Edit->SetIt(ip);grid->SelectRow(ip);grid->ScrollTo(ip-4);wl=ip;}
		else if(idr>=0){pan->Edit->SetIt(idr);grid->SelectRow(idr);grid->ScrollTo(idr-4);wl=idr;}

		//wxString kkk;
		//wxMessageBox(kkk<<"ip "<<ip<<"idr "<<idr);

	}


	wxString *txt=new wxString();
	(*txt)<<_T("[Script Info]\r\n")<<grid->GetSInfos();
	(*txt)<<_T("\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n");
	(*txt)<<styl->styletext();
	(*txt)<<_T(" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n");

	if(wl<0){
		Dialogue tmpdial;
		tmpdial.Style=styl->Name;
		tmpdial.Text=_("Linijka testowa stylu");
		tmpdial.Start.NewTime(time-200);
		tmpdial.End.NewTime(time+200);
		(*txt)<<tmpdial.GetRaw();
	}
	else{
		(*txt)<<grid->GetDial(wl)->GetRaw();

	}
	//grid->SaveFile(Kai->GetTab()->tnppath,false);
	pan->Video->OpenSubs(txt);
	if(fullskreen&&!pan->Video->isfullskreen){pan->Video->SetFullskreen();this->SetWindowStyle(wxSTAY_ON_TOP|wxDEFAULT_DIALOG_STYLE);}
	if(!fullskreen&&pan->Video->isfullskreen){pan->Video->SetFullskreen();this->SetWindowStyle(wxDEFAULT_DIALOG_STYLE);}
	if(wl>=0){
		pan->Video->Seek(grid->GetDial(wl)->Start.mstime+5);
	}else{
		pan->Video->Render();}

}



void stylestore::DoTooltips()
{
	Choice1->SetToolTip(_("Katalog styli"));
	Button7->SetToolTip(_("Nowy katalog styli"));
	//ASS->SetToolTip(_("Style napisów"));
	//Store->SetToolTip(_("Style magazynu"));
	Button5->SetToolTip(_("Utwórz nowy styl magazynu"));
	Button8->SetToolTip(_("Kopiuj styl magazynu"));
	Button9->SetToolTip(_("Wczytaj do magazynu styl z pliku ass"));
	Button12->SetToolTip(_("Usuñ styl z magazynu"));
	Button13->SetToolTip(_("Sortuj style w magazynie"));
	Button6->SetToolTip(_("Utwórz nowy styl napisów"));
	Button10->SetToolTip(_("Kopiuj styl napisów"));
	Button11->SetToolTip(_("Wczytaj do napisów styl z pliku ass"));
	Button14->SetToolTip(_("Usuñ styl z napisów"));
	Button15->SetToolTip(_("Sortuj style w napisach"));
	Button3->SetToolTip(_("Dodaj do magazynu styl z napisów"));
	Button4->SetToolTip(_("Dodaj do napisów styl z magazynu"));
}

void stylestore::OnConfirm(wxCommandEvent& event)
{

	if(cc->IsShown())
	{cc->OnOKClick(event);}
	else{modif();OnClose(event);}
}

void stylestore::OnClose(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	int ww,hh;
	GetPosition(&ww,&hh);
	Options.SetCoords("Style Manager Position",ww,hh);
	Hide();
}

void stylestore::modif()
{
	Grid* grid=Notebook::GetTab()->Grid1;
	Notebook::GetTab()->Edit->RefreshStyle();
	grid->SetModified(false);
	grid->Refresh(false);
	ASS->SetArray(grid->GetStyleTable());
}

void stylestore::LoadAssStyles()
{
	Grid* grid=Notebook::GetTab()->Grid1;
	if (grid->StylesSize()<1){
		grid->AddStyle(new Styles());
	}
	ASS->SetArray(grid->GetStyleTable());
	int wstyle=MAX(0,grid->FindStyle(Notebook::GetTab()->Edit->line->Style));
	ASS->SetSelection(wstyle,true);
	if(cc->IsShown())
	{
		wxCommandEvent evt;
		OnAssStyleChange(evt);
	}

}

void stylestore::ReloadFonts()
{
	wxArrayString fontList = wxFontEnumerator::GetFacenames();
	std::sort(fontList.begin(),fontList.end(),sortf);
	wxString oldfname= cc->sfont->GetString(cc->sfont->GetSelection());
	cc->sfont->Clear();
	cc->sfont->Append(fontList);
	int wfont=MAX(0,cc->sfont->FindString(oldfname));
	cc->sfont->SetSelection(wfont);
	wxLogStatus("Czcionki zaczytane ponownie.");
}

DWORD stylestore::CheckFontProc(void* cls)
{
	stylestore *ss=(stylestore*)cls;
	if(!ss){wxLogStatus(_("Brak wskaŸnika klasy magazynu stylów."));return 0;}

	HANDLE hDir  = NULL; 
	wxString fontrealpath=wxGetOSDirectory() + _T("\\fonts\\");

	hDir = FindFirstChangeNotification( fontrealpath.wc_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);// | FILE_NOTIFY_CHANGE_LAST_WRITE

	if(hDir== INVALID_HANDLE_VALUE ){wxLogStatus(_("Nie mo¿na stworzyæ uchwytu notyfikacji zmian folderu czcionek.")); return 0;}
	while(1){
		while( WaitForSingleObject( hDir, WAIT_TIMEOUT ) != WAIT_OBJECT_0 ){
			if( ss->stopcheck ){
				break;
			}
		}
		if( ss->stopcheck ){
			break;
		}
		//wxLogStatus("count %i", count);
		ss->ReloadFonts();

		if( FindNextChangeNotification( hDir ) == 0 ){
			wxLogStatus(_("Nie mo¿na stworzyæ nastêpnego uchwytu notyfikacji zmian folderu czcionek."));
			return 0;
		}
	}

	return FindCloseChangeNotification( hDir );
}