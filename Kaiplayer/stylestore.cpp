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

#include "StyleStore.h"
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
#include "StyleChange.h"
#include "KaiMessageBox.h"
#include "FontEnumerator.h"


StyleStore::StyleStore(wxWindow* parent,const wxPoint& pos)
	: KaiDialog(parent,-1,_("Menedżer stylów"),pos,wxSize(400,-1),wxDEFAULT_DIALOG_STYLE)
	,stayOnTop(false)
{
	bool isDetached = detachedEtit = Options.GetBool(StyleManagerDetachEditor);
	wxIcon icn;
	icn.CopyFromBitmap(wxBITMAP_PNG("styles"));
	SetIcon(icn);

	cc=new StyleChange(this,!isDetached);
	cc->Hide();

	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));

	wxBitmap arrowDown = wxBITMAP_PNG("arrow_list");
	wxBitmap arrowDownDouble = wxBITMAP_PNG("ARROW_LIST_DOUBLE");
	wxImage arrowcopy = arrowDown.ConvertToImage();
	arrowcopy = arrowcopy.Rotate180();
	wxImage arrowDoublecopy = arrowDownDouble.ConvertToImage();
	arrowDoublecopy = arrowDoublecopy.Rotate180();
	wxBitmap arrowUp = wxBitmap(arrowcopy);
	wxBitmap arrowUpDouble = wxBitmap(arrowDoublecopy);

	wxBoxSizer *Mainsm= new wxBoxSizer(wxVERTICAL);
	Mainall= new DialogSizer(wxHORIZONTAL);

	KaiStaticBoxSizer *catalogSizer=new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Katalog:"));
	catalogList = new KaiChoice(this, ID_CATALOG, wxDefaultPosition, wxDefaultSize, Options.dirs);
	int chc=catalogList->FindString(Options.acdir);
	catalogList->SetSelection(chc);
	newCatalog = new MappedButton(this, ID_NEWCAT, _("Nowy"));
	MappedButton *deleteCatalog = new MappedButton(this, ID_DELCAT, _("Usuń"));
	deleteCatalog->SetToolTip(_("Usuń wybrany katalog stylów"));
	catalogSizer->Add(catalogList,1,wxEXPAND|wxALL,2);
	catalogSizer->Add(newCatalog,0,wxEXPAND|wxALL,2);
	catalogSizer->Add(deleteCatalog,0,wxEXPAND|wxALL,2);


	KaiStaticBoxSizer *catalogStylesSizer=new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Style katalogu:"));
	wxBoxSizer *catalogButtonsSizer=new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *catalogMainSizer=new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *catalogMoveButtonsSizer=new wxBoxSizer(wxVERTICAL);

	Store = new StyleList(this, ID_STORESTYLES, &Options.assstore, wxDefaultPosition, wxSize(-1,520));

	storeNew = new MappedButton(this, ID_STORENEW, _("Nowy"));
	storeCopy = new MappedButton(this, ID_STORECOPY, _("Kopiuj"));
	storeEdit = new MappedButton(this, ID_STOREEDIT, _("Edytuj"));
	storeLoad = new MappedButton(this, ID_STORELOAD, _("Wczytaj"));
	storeDelete = new MappedButton(this, ID_STOREDEL, _("Usuń"));
	storeSort = new MappedButton(this, ID_STORESORT, _("Sortuj"));

	catalogButtonsSizer->Add(storeNew,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	catalogButtonsSizer->Add(storeCopy,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	catalogButtonsSizer->Add(storeEdit,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	catalogButtonsSizer->Add(storeLoad,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	catalogButtonsSizer->Add(storeDelete,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	catalogButtonsSizer->Add(storeSort,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);

	MappedButton *storeMoveToStart = new MappedButton(this, ID_STORE_MOVE_TO_START, _("Przesuń zaznaczone style na sam początek"),arrowUpDouble, wxDefaultPosition, wxSize(24,24),0);
	MappedButton *storeMoveUp = new MappedButton(this, ID_STORE_MOVE_UP, _("Przesuń zaznaczone style w górę"),arrowUp, wxDefaultPosition, wxSize(24,24),0);
	MappedButton *storeMoveDown = new MappedButton(this, ID_STORE_MOVE_DOWN, _("Przesuń zaznaczone style w dół"),arrowDown, wxDefaultPosition, wxSize(24,24),0);
	MappedButton *storeMoveToEnd = new MappedButton(this, ID_STORE_MOVE_TO_END,_("Przesuń zaznaczone style na sam koniec"),arrowDownDouble, wxDefaultPosition, wxSize(24,24),0);

	catalogMoveButtonsSizer->Add(storeMoveToStart,0,wxTOP|wxBOTTOM,2);
	catalogMoveButtonsSizer->Add(storeMoveUp,0,wxTOP|wxBOTTOM,2);
	catalogMoveButtonsSizer->Add(storeMoveDown,0,wxTOP|wxBOTTOM,2);
	catalogMoveButtonsSizer->Add(storeMoveToEnd,0,wxTOP|wxBOTTOM,2);

	catalogMainSizer->Add(catalogMoveButtonsSizer,0,wxRIGHT|wxCENTER,4);
	catalogMainSizer->Add(Store,4,wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT,2);
	catalogMainSizer->Add(catalogButtonsSizer,1,wxEXPAND,0);

	catalogStylesSizer->Add(catalogMainSizer,1,wxEXPAND|wxALL,2);

	wxBoxSizer *addToButtons=new wxBoxSizer(wxHORIZONTAL);

	addToStore = new MappedButton(this, ID_ADDTOSTORE, _("^ Dodaj do magazynu"));
	addToAss = new MappedButton(this, ID_ADDTOASS, _("v Dodaj do ASS"));
	addToButtons->Add(addToStore,1,wxEXPAND|wxALL,5);
	addToButtons->Add(addToAss,1,wxEXPAND|wxALL,5);
	//addToButtons->AddStretchSpacer(3);

	KaiStaticBoxSizer *ASSStylesSizer=new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Style pliku ASS:"));
	wxBoxSizer *ASSButtonsSizer=new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *ASSMainSizer=new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *ASSMoveButtonsSizer=new wxBoxSizer(wxVERTICAL);

	ASS = new StyleList(this, ID_ASSSTYLES, Notebook::GetTab()->Grid1->GetStyleTable(), wxDefaultPosition, wxSize(-1,520));


	assNew = new MappedButton(this, ID_ASSNEW, _("Nowy"));
	assCopy = new MappedButton(this, ID_ASSCOPY, _("Kopiuj"));
	assEdit = new MappedButton(this, ID_ASSEDIT, _("Edytuj"));
	assLoad = new MappedButton(this, ID_ASSLOAD, _("Wczytaj"));
	assDelete = new MappedButton(this, ID_ASSDEL, _("Usuń"));
	assSort = new MappedButton(this, ID_ASSSORT, _("Sortuj"));
	SClean = new MappedButton(this, ID_ASSCLEAN, _("Oczyść"));

	ASSButtonsSizer->Add(assNew,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	ASSButtonsSizer->Add(assCopy,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	ASSButtonsSizer->Add(assEdit,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	ASSButtonsSizer->Add(assLoad,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	ASSButtonsSizer->Add(assDelete,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	ASSButtonsSizer->Add(assSort,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);
	ASSButtonsSizer->Add(SClean,1,wxEXPAND|wxTOP|wxBOTTOM|wxLEFT,2);

	MappedButton *ASSMoveToStart = new MappedButton(this, ID_ASS_MOVE_TO_START, _("Przesuń zaznaczone style na sam początek"),arrowUpDouble, wxDefaultPosition, wxSize(24,24),0);
	MappedButton *ASSMoveUp = new MappedButton(this, ID_ASS_MOVE_UP, _("Przesuń zaznaczone style w górę"),arrowUp, wxDefaultPosition, wxSize(24,24),0);
	MappedButton *ASSMoveDown = new MappedButton(this, ID_ASS_MOVE_DOWN, _("Przesuń zaznaczone style w dół"),arrowDown, wxDefaultPosition, wxSize(24,24),0);
	MappedButton *ASSMoveToEnd = new MappedButton(this, ID_ASS_MOVE_TO_END,_("Przesuń zaznaczone style na sam koniec"),arrowDownDouble, wxDefaultPosition, wxSize(24,24),0);

	ASSMoveButtonsSizer->Add(ASSMoveToStart,0,wxTOP|wxBOTTOM,2);
	ASSMoveButtonsSizer->Add(ASSMoveUp,0,wxTOP|wxBOTTOM,2);
	ASSMoveButtonsSizer->Add(ASSMoveDown,0,wxTOP|wxBOTTOM,2);
	ASSMoveButtonsSizer->Add(ASSMoveToEnd,0,wxTOP|wxBOTTOM,2);

	ASSMainSizer->Add(ASSMoveButtonsSizer,0,wxRIGHT|wxCENTER,4);
	ASSMainSizer->Add(ASS,4,wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT,2);
	ASSMainSizer->Add(ASSButtonsSizer,1,wxEXPAND);

	ASSStylesSizer->Add(ASSMainSizer,1,wxEXPAND|wxALL,2);
	wxBoxSizer * buttons = new wxBoxSizer(wxHORIZONTAL);
	close = new MappedButton(this, ID_CLOSE, _("Zamknij"));
	detachEnable = new ToggleButton(this,ID_DETACH,_("Odepnij okno edycji"));
	detachEnable->SetValue(isDetached);
	buttons->Add(close,0,wxRIGHT,2);
	buttons->Add(detachEnable,0,wxLEFT,2);

	Mainsm->Add(catalogSizer,0,wxEXPAND|wxALL,2);
	Mainsm->Add(catalogStylesSizer,1,wxEXPAND|wxALL,2);
	Mainsm->Add(addToButtons,0,wxEXPAND|wxALL,2);
	Mainsm->Add(ASSStylesSizer,1,wxEXPAND|wxALL,2);
	Mainsm->Add(buttons,0,wxALIGN_CENTER|wxALL,4);



	Mainall->Add(Mainsm,0,wxEXPAND);
	if(!isDetached){
		Mainall->Add(cc,0,wxEXPAND);
		wxSize bs = wxSize(-1,cc->GetBestSize().y+29);
		Mainall->SetMinSize(bs);
	}
	SetEscapeId(ID_CLOSE);
	SetEnterId(ID_CONF);
	
	SetSizerAndFit(Mainall);

	Connect(ID_ASSSTYLES,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&StyleStore::OnAssStyleChange);
	Connect(ID_ASSSTYLES,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&StyleStore::OnSwitchLines);
	Connect(ID_STORESTYLES,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&StyleStore::OnStoreStyleChange);
	Connect(ID_ADDTOSTORE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAddToStore);
	Connect(ID_ADDTOASS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAddToAss);
	Connect(ID_CATALOG,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&StyleStore::OnChangeCatalog);
	Connect(ID_NEWCAT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnNewCatalog);
	Connect(ID_DELCAT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnDelCatalog);
	Connect(ID_STORENEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnStoreNew);
	Connect(ID_STORECOPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnStoreCopy);
	Connect(ID_STOREEDIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnStoreStyleChange);
	Connect(ID_STORELOAD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnStoreLoad);
	Connect(ID_STOREDEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnStoreDelete);
	Connect(ID_STORESORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnStoreSort);
	Connect(ID_ASSNEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAssNew);
	Connect(ID_ASSCOPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAssCopy);
	Connect(ID_ASSEDIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAssStyleChange);
	Connect(ID_ASSLOAD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAssLoad);
	Connect(ID_ASSDEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAssDelete);
	Connect(ID_ASSSORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnAssSort);
	Connect(ID_ASSCLEAN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnCleanStyles);
	Connect(ID_CONF,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnConfirm);
	Connect(ID_CLOSE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnClose);
	Connect(ID_DETACH,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&StyleStore::OnDetachEdit);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED,&StyleStore::OnStyleMove,this, ID_ASS_MOVE_TO_START, ID_STORE_MOVE_TO_END);

	DoTooltips();

	//SetMaxSize(wxSize(500,-1));
}

StyleStore::~StyleStore()
{
}

void StyleStore::OnSwitchLines(wxCommandEvent& event)
{
	Notebook::GetTab()->Edit->RefreshStyle();
}


void StyleStore::OnAssStyleChange(wxCommandEvent& event)
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
void StyleStore::OnStoreStyleChange(wxCommandEvent& event)
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

void StyleStore::OnAddToStore(wxCommandEvent& event)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	wxArrayInt sels;
	int kkk=ASS->GetSelections(sels);
	if(kkk<1){wxBell();return;}
	Store->SetSelection(wxNOT_FOUND);
	prompt=0;
	for(size_t i=0;i<sels.GetCount();i++){
		Styles *stylc = grid->GetStyle(sels[i])->Copy();
		int found=Options.FindStyle(stylc->Name);
		if(found!=-1){
			if(prompt != wxYES_TO_ALL && prompt != wxCANCEL){
				prompt = KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmienić go?"), 
					stylc->Name), _("Potwierdzenie"),wxYES_TO_ALL|wxYES|wxNO|wxCANCEL, this);
				//if(prompt == wxID_CANCEL){return;}
			}
			if( prompt == wxYES || prompt == wxYES_TO_ALL){
				Options.ChangeStyle(stylc,found);Store->SetSelection(found);
			}else{delete stylc;}
		}else{Options.AddStyle(stylc);Store->SetSelection(Options.StoreSize()-1);}
	}

	//modif();
}

void StyleStore::OnAddToAss(wxCommandEvent& event)
{
	//wxMutexLocker lock(mutex);
	Grid* grid=Notebook::GetTab()->Grid1;
	wxArrayInt sels;
	int kkk=Store->GetSelections(sels);
	if(kkk<1){wxBell();return;}
	ASS->SetSelection(wxNOT_FOUND);
	prompt=0;
	for(int i=0;i<kkk;i++)
	{
		Styles *stylc = Options.GetStyle(sels[i])->Copy();
		int found=grid->FindStyle(stylc->Name);
		if(found!=-1){
			if(prompt != wxYES_TO_ALL && prompt != wxCANCEL){
				prompt = KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmienić go?"), 
					stylc->Name), _("Potwierdzenie"),wxYES_TO_ALL|wxYES|wxNO|wxCANCEL, this);
			}
			if( prompt == wxYES || prompt == wxYES_TO_ALL){
				grid->ChangeStyle(stylc,found);ASS->SetSelection(found);
			}else{delete stylc;}
		}else{grid->AddStyle(stylc);ASS->SetSelection(grid->StylesSize()-1);}
	}
	modif();
}

void StyleStore::OnStoreDelete(wxCommandEvent& event)
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

void StyleStore::OnAssDelete(wxCommandEvent& event)
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

void StyleStore::StylesWindow(wxString newname)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	Styles *tab=NULL;
	if (selnum<0){tab=new Styles();}
	else if (stass){tab=grid->GetStyle(selnum)->Copy();}
	else{tab=Options.GetStyle(selnum)->Copy();}
	if(newname!=""){tab->Name=newname;}
	oldname=tab->Name;
	cc->UpdateValues(tab);
	if(!detachedEtit){Mainall->Fit(this);}

}

void StyleStore::changestyle(Styles *cstyl)
{
	Update();
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
		KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" jest już na liście."), cstyl->Name));
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
		int res=KaiMessageBox(_("Nazwa stylu została zmieniona, czy chcesz zmienić ją także w napisach?"), _("Potwierdzenie"), wxYES_NO);
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

void StyleStore::OnChangeCatalog(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	Options.LoadStyles(catalogList->GetString(catalogList->GetSelection()));

	Store->SetSelection(0,true);
}

void StyleStore::OnNewCatalog(wxCommandEvent& event)
{
	NewCatalog nc(this);
	if(nc.ShowModal()==wxID_OK){
		wxString nkat=nc.TextCtrl1->GetValue();
		catalogList->SetSelection(catalogList->Append(nkat));
		Options.dirs.Add(nkat);
		Options.acdir=nkat;
		Options.clearstyles();
		Store->Refresh(false);
	}
}

void StyleStore::OnDelCatalog(wxCommandEvent& event)
{
	int cat=catalogList->GetSelection();
	if(cat==-1){return;}
	wxString Cat = catalogList->GetString(cat);
	if(Cat=="Default"){wxBell();return;}
	catalogList->Delete(cat);
	Options.dirs.RemoveAt(cat);
	Options.acdir=Options.dirs[MAX(0,cat-1)];
	catalogList->SetSelection(MAX(0,cat-1));
	wxString path;
	path<<Options.pathfull<<"\\Catalog\\"<<Cat<<".sty";
	wxRemoveFile(path);
	Options.LoadStyles(Options.acdir);
	Store->Refresh(false);
}

void StyleStore::OnStoreLoad(wxCommandEvent& event)
{
	LoadStylesS(false);
}

void StyleStore::OnAssSort(wxCommandEvent& event)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	std::sort(grid->GetStyleTable()->begin(), grid->GetStyleTable()->end(),sortfunc);
	ASS->SetSelection(0,true);

	modif();
}

void StyleStore::OnAssLoad(wxCommandEvent& event)
{
	LoadStylesS(true);
}

void StyleStore::OnStoreSort(wxCommandEvent& event)
{
	Options.Sortstyles();
	Store->SetSelection(0,true);

	//modif();
}

void StyleStore::LoadStylesS(bool isass)
{
	Grid* grid=Notebook::GetTab()->Grid1;
	wxFileDialog *openFileDialog= new wxFileDialog(this, _("Wybierz plik ASS"), 
		Notebook::GetTab()->SubsPath.BeforeLast('\\'), "*.ass", _("Pliki napisów ASS(*.ass)|*.ass"), 
		wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "wxFileDialog");
	if (openFileDialog->ShowModal() == wxID_OK){
		OpenWrite op;
		wxString ass;
		op.FileOpen(openFileDialog->GetPath(), &ass);
		size_t start=ass.find("\nStyle: ");
		size_t end=ass.find("[Events]");
		if(end<=start){return;}
		std::vector<Styles*> tmps;
		wxString styless=ass.SubString(start,end);
		prompt=0;
		wxStringTokenizer styletkn(styless,"\n");
		while(styletkn.HasMoreTokens())
		{
			wxString token=styletkn.NextToken();
			if (token.StartsWith("Style: ")){
				tmps.push_back(new Styles(token));
			}
		}
		Stylelistbox stl(this);
		for(size_t i=0;i<tmps.size();i++){

			stl.CheckListBox1->AppendItem(new ItemCheckBox(false, tmps[i]->Name));
		}
		if(isass){ASS->SetSelection(wxNOT_FOUND);}else{Store->SetSelection(wxNOT_FOUND);}
		if(stl.ShowModal()== wxID_OK){
			for (size_t v=0;v<stl.CheckListBox1->GetCount();v++)
			{
				if(stl.CheckListBox1->GetItem(v, 0)->modified){

					int fstyle= (isass)? grid->FindStyle(stl.CheckListBox1->GetItem(v, 0)->name) : Options.FindStyle(stl.CheckListBox1->GetItem(v, 0)->name);
					if(fstyle==-1){
						if(isass){grid->AddStyle(tmps[v]);ASS->Refresh(false);}
						else{Options.AddStyle(tmps[v]);Store->Refresh(false);}
					}
					else{
						if(prompt != wxYES_TO_ALL && prompt != wxCANCEL){
							prompt = KaiMessageBox(wxString::Format(_("Styl o nazwie \"%s\" istnieje, podmienić go?"), 
								stl.CheckListBox1->GetItem(v, 0)->name), _("Potwierdzenie"),wxYES_TO_ALL|wxYES|wxNO|wxCANCEL, this);
							//if(prompt == wxID_CANCEL){return;}
						}
						if( prompt == wxYES || prompt == wxYES_TO_ALL  ){
							if(isass){grid->ChangeStyle(tmps[v],fstyle);ASS->SetSelection(fstyle);}
							else{Options.ChangeStyle(tmps[v],fstyle);Store->SetSelection(fstyle);}
						}else{delete tmps[v];}
					}

				}else{
					delete tmps[v];
				}
			}
		}
		tmps.clear();
	}
	openFileDialog->Destroy();
	if(isass){modif();}
}

void StyleStore::OnAssCopy(wxCommandEvent& event)
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
void StyleStore::OnStoreCopy(wxCommandEvent& event)
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

void StyleStore::OnStoreNew(wxCommandEvent& event)
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

void StyleStore::OnAssNew(wxCommandEvent& event)
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

void StyleStore::OnCleanStyles(wxCommandEvent& event)
{
	std::map<wxString, bool> lineStyles;
	wxString delStyles;
	wxString existsStyles;
	Grid *grid=Notebook::GetTab()->Grid1;
	wxString tlStyle=grid->GetSInfo("TLMode Style");

	for(int i = 0; i < grid->GetCount(); i++){
		lineStyles[grid->GetDial(i)->Style]=true;
	}

	int j = 0;
	while(j < grid->StylesSize()){
		wxString styleName = grid->GetStyle(j)->Name;
		if(lineStyles.find(styleName) != lineStyles.end()){
			existsStyles << styleName << L"\n";
		}else{
			if(styleName!=tlStyle){
				delStyles << styleName << L"\n";
				grid->DelStyle(j);
				continue;
			}else{existsStyles << styleName << L"\n";}
		}
		j++;
	}

	if(!delStyles.empty()){
		modif();
	}
	KaiMessageBox(wxString::Format(_("Używane style:\n%s\nUsunięte style:\n%s"), existsStyles, delStyles), _("Status usuniętych stylów"));
}

void StyleStore::StyleonVideo(Styles *styl, bool fullskreen)
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
					pan->Edit->SetLine(i);
					grid->SelectRow(i);
					grid->ScrollTo(i-4);
					wl=i;ip=-1;idr=-1;
					break;
				}
				if(dial->Start.mstime > prevtime && dial->Start.mstime<time){prevtime = dial->Start.mstime;ip=i;}
				if(dial->Start.mstime<durtime && dial->Start.mstime>time){durtime = dial->Start.mstime; idr=i;}
			}

		}
		if(ip>=0){
			pan->Edit->SetLine(ip);
			grid->SelectRow(ip);
			grid->ScrollTo(ip-4);
			wl=ip;
		}
		else if(idr>=0){
			pan->Edit->SetLine(idr);
			grid->SelectRow(idr);
			grid->ScrollTo(idr-4);
			wl=idr;
		}

		//wxString kkk;
		//KaiMessageBox(kkk<<"ip "<<ip<<"idr "<<idr);

	}


	wxString *txt=new wxString();
	(*txt)<<"[Script Info]\r\n"<<grid->GetSInfos();
	(*txt)<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
	(*txt)<<styl->styletext();
	(*txt)<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";

	if(wl<0){
		Dialogue tmpdial;
		tmpdial.Style=styl->Name;
		tmpdial.Text=_("Linijka testowa stylu");
		tmpdial.Start.NewTime(time-200);
		tmpdial.End.NewTime(time+200);
		tmpdial.GetRaw(txt);
	}
	else{
		grid->GetDial(wl)->GetRaw(txt);

	}
	//grid->SaveFile(Kai->GetTab()->tnppath,false);
	pan->Video->OpenSubs(txt);
	if(fullskreen&&!pan->Video->isFullscreen){
		pan->Video->SetFullskreen();
		this->SetWindowStyle(GetWindowStyle()|wxSTAY_ON_TOP);
	}
	//if(!fullskreen&&pan->Video->isFullscreen){pan->Video->SetFullskreen();this->SetWindowStyle(GetWindowStyle()|~wxSTAY_ON_TOP);}
	if(wl>=0){
		pan->Video->Seek(grid->GetDial(wl)->Start.mstime+5);
	}else{
		pan->Video->Render();
	}
	pan->Edit->OnVideo=true;
}



void StyleStore::DoTooltips()
{
	catalogList->SetToolTip(_("Katalog styli"));
	newCatalog->SetToolTip(_("Nowy katalog styli"));
	//ASS->SetToolTip(_("Style napisów"));
	//Store->SetToolTip(_("Style magazynu"));
	storeNew->SetToolTip(_("Utwórz nowy styl magazynu"));
	storeCopy->SetToolTip(_("Kopiuj styl magazynu"));
	storeLoad->SetToolTip(_("Wczytaj do magazynu styl z pliku ass"));
	storeDelete->SetToolTip(_("Usuń styl z magazynu"));
	storeSort->SetToolTip(_("Sortuj style w magazynie"));
	assNew->SetToolTip(_("Utwórz nowy styl napisów"));
	assCopy->SetToolTip(_("Kopiuj styl napisów"));
	assLoad->SetToolTip(_("Wczytaj do napisów styl z pliku ass"));
	assDelete->SetToolTip(_("Usuń styl z napisów"));
	assSort->SetToolTip(_("Sortuj style w napisach"));
	addToStore->SetToolTip(_("Dodaj do magazynu styl z napisów"));
	addToAss->SetToolTip(_("Dodaj do napisów styl z magazynu"));
	SClean->SetToolTip(_("Oczyść napisy z nieużywanych stylów"));
}

void StyleStore::OnConfirm(wxCommandEvent& event)
{

	if(cc->IsShown())
	{cc->OnOKClick(event);}
	else{modif();OnClose(event);}
}

void StyleStore::OnClose(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	int ww,hh;
	GetPosition(&ww,&hh);
	Options.SetCoords(StyleManagerPosition,ww,hh);
	/*if(stayOnTop){
		TabPanel* pan=Notebook::GetTab();
		this->Reparent(pan->GetParent());
		stayOnTop=false;
	}*/
	Hide();
	if(detachedEtit){cc->Show(false);}
}

void StyleStore::modif()
{
	Grid* grid=Notebook::GetTab()->Grid1;
	Notebook::GetTab()->Edit->RefreshStyle();
	grid->SetModified(false);
	grid->Refresh(false);
	ASS->SetArray(grid->GetStyleTable());
}

void StyleStore::LoadAssStyles()
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

void StyleStore::ReloadFonts()
{
	wxArrayString *fontList = FontEnum.GetFonts(0,[](){});
	cc->sfont->PutArray(fontList);
	Store->Refresh(false);
	ASS->Refresh(false);
	wxLogStatus(_("Czcionki zaczytane ponownie."));
}

bool StyleStore::SetForegroundColour(const wxColour &col)
{
	wxWindow::SetForegroundColour(col);
	//wxWindow::Refresh();
	if(cc){cc->SetForegroundColour(col);}
	return true;
}

bool StyleStore::SetBackgroundColour(const wxColour &col)
{
	wxWindow::SetBackgroundColour(col);
	//wxWindow::Refresh();
	if(cc){cc->SetBackgroundColour(col);}
	return true;
}

void StyleStore::OnDetachEdit(wxCommandEvent& event)
{
	bool detach = detachEnable->GetValue();
	bool show = cc->IsShown();
	if(detachedEtit && !detach){
		cc->Destroy();
		cc = new StyleChange(this);
		if(show){StylesWindow();}
		else{cc->Show(false);}
		Mainall->Add(SS->cc,0,wxEXPAND);
		wxSize bs = wxSize(-1,cc->GetBestSize().y+29);
		Mainall->SetMinSize(bs);
		detachedEtit=false;
	}else if(!detachedEtit && detach){
		Mainall->Detach(cc);
		cc->Destroy();
		cc = new StyleChange(this,false);
		//Mainall->Fit(this);
		if(show){StylesWindow();}
		else{cc->Show(false);}
		detachedEtit=true;
	}
	Mainall->Fit(this);
	Options.SetBool(StyleManagerDetachEditor,detach);
}

StyleStore *StyleStore::SS =NULL;

void StyleStore::ShowStore()
{
	StyleStore *SS = Get();
	bool detach = Options.GetBool(StyleManagerDetachEditor);
	/*if(SS->stayOnTop){
		TabPanel* pan=Notebook::GetTab();
		SS->Reparent(pan);
		SS->stayOnTop=false;
	}*/
	if(SS->detachedEtit && !detach){
		SS->cc->Destroy();
		SS->cc = new StyleChange(SS);
		SS->cc->Show(false);
		SS->Mainall->Add(SS->cc,0,wxEXPAND);
		wxSize bs = wxSize(-1,SS->cc->GetBestSize().y+29);
		SS->Mainall->SetMinSize(bs);
		SS->Mainall->Fit(SS);
		SS->detachedEtit=false;
	}else if(!SS->detachedEtit && detach){
		SS->Mainall->Detach(SS->cc);
		SS->cc->Destroy();
		SS->cc = new StyleChange(SS,false);
		SS->cc->Show(false);
		SS->Mainall->Fit(SS);
		SS->detachedEtit=true;
	}
	SS->Store->Refresh(false);
	int chc=SS->catalogList->FindString(Options.acdir);
	SS->catalogList->SetSelection(chc);
	SS->LoadAssStyles();
	SS->Show();
}
		
void StyleStore::ShowStyleEdit()
{
	StyleStore *SS = Get();
	if(!SS->detachedEtit){
		SS->Mainall->Detach(SS->cc);
		SS->cc->Destroy();
		SS->cc = new StyleChange(SS,false);
		SS->detachedEtit=true;
	}
	SS->cc->Show();
	SS->LoadAssStyles();

}

StyleStore *StyleStore::Get()
{
	if(!SS){
		int ww,hh;
		Options.GetCoords(StyleManagerPosition,&ww,&hh);
		SS = new StyleStore(Notebook::GetTabs()->GetParent(), wxPoint(ww,hh));
	}
	return SS;
}

void StyleStore::DestroyStore()
{
	if(SS){delete SS; SS=NULL;}
}

void StyleStore::OnStyleMove(wxCommandEvent& event)
{
	int action = event.GetId() - ID_ASS_MOVE_TO_START;
	wxArrayInt sels; 
	int selSize = (action < 4)? ASS->GetSelections(sels) : Store->GetSelections(sels);
	if(selSize < 1){wxBell(); return;}
	std::vector<Styles *> *styleTable = (action < 4)? Notebook::GetTab()->Grid1->GetStyleTable() : &Options.assstore;
	int styleTableSize = styleTable->size();
	int move = (action % 4 == 0)? -styleTableSize : (action % 4 == 1)? -1 : 
		(action % 4 == 2)? 1 : styleTableSize;
	
	
	bool moveUp = (action % 4 == 0 || action % 4 == 1);
	int lastUpSelection = 0;
	int lastDownSelection = styleTableSize-1;
	int i = (moveUp)? 0 : selSize-1;

	while((moveUp)? i < selSize : i >= 0){
		int sel = sels[i];
		int moveSelections = sel + move;
		
		if(moveUp && moveSelections < lastUpSelection){
			moveSelections = lastUpSelection;
		}else if(!moveUp && moveSelections > lastDownSelection){
			moveSelections = lastDownSelection;
		}
		Styles *Selected = (*styleTable)[sel];
		styleTable->erase(styleTable->begin() + sel);
		styleTable->insert(styleTable->begin() + moveSelections, Selected);
		sels[i] = moveSelections;
		if(moveUp){i++; lastUpSelection++;}
		else{i--; lastDownSelection--;}
	}
	if(action < 4){ASS->SetSelections(sels); Notebook::GetTab()->Grid1->file->edited = true; modif();}
	else{Store->SetSelections(sels);}
}

