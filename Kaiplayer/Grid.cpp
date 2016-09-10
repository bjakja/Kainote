#include "Grid.h"


#include <wx/intl.h>
#include <wx/string.h>
#include "Utils.h"
#include <wx/clipbrd.h>
#include "KainoteMain.h"
#include "Hotkeys.h"
#include "OpennWrite.h"
#include "TLDialog.h"
#include "MKVWrap.h"
#include "Stylelistbox.h"
#include <wx/regex.h>


Grid::Grid(wxWindow* parent, kainoteFrame* kfparent,wxWindowID id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
	: SubsGrid(parent, id, pos, size, style, name)
{
	Kai=kfparent;
}

Grid::~Grid()
{
}

void Grid::ContextMenu(const wxPoint &pos, bool dummy)
{
	VideoCtrl *VB=((TabPanel*)GetParent())->Video;
	VB->blockpaint=true;
	selarr = GetSels();
	int sels=selarr.GetCount();
	wxMenu *menu=new wxMenu;
	hidemenu=new wxMenu;
	wxMenuItem *item;
	item = Hkeys.SetAccMenu(hidemenu, 5000+LAYER,_("Ukryj warstwę"),_("Ukryj warstwę"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & LAYER)!=0);
	Hkeys.SetAccMenu(hidemenu, 5000+START,_("Ukryj czas początkowy"),_("Ukryj czas początkowy"),wxITEM_CHECK)->Check((visible & START)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+END,_("Ukryj czas końcowy"),_("Ukryj czas końcowy"),wxITEM_CHECK);
	item->Enable(form!=TMP);
	item->Check((visible & END)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+ACTOR,_("Ukryj aktora"),_("Ukryj aktora"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & ACTOR)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+STYLE,_("Ukryj styl"),_("Ukryj styl"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & STYLE)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+MARGINL,_("Ukryj lewy margines"),_("Ukryj lewy margines"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & MARGINL)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+MARGINR,_("Ukryj prawy margines"),_("Ukryj prawy margines"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & MARGINR)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+MARGINV,_("Ukryj pionowy margines"),_("Ukryj pionowy margines"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & MARGINV)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+EFFECT,_("Ukryj efekt"),_("Ukryj efekt"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & EFFECT)!=0);
	Hkeys.SetAccMenu(hidemenu, 5000+CNZ,_("Ukryj znaki na sekundę"),_("Ukryj znaki na sekundę"),wxITEM_CHECK)->Check((visible & CNZ)!=0);

	bool isen;
	isen = (sels == 1);
	Hkeys.SetAccMenu(menu, InsertBefore,_("Wstaw przed"))->Enable(isen);
	Hkeys.SetAccMenu(menu, InsertAfter,_("Wstaw po"))->Enable(isen);
	isen = (isen&&Kai->GetTab()->Video->GetState()!=None);
	Hkeys.SetAccMenu(menu, InsertBeforeVideo,_("Wstaw przed z czasem wideo"))->Enable(isen);
	Hkeys.SetAccMenu(menu, InsertAfterVideo,_("Wstaw po z czasem wideo"))->Enable(isen);
	isen = (sels >0);
	Hkeys.SetAccMenu(menu, Duplicate,_("Duplikuj linie"))->Enable(isen);
	isen = (sels == 2);
	Hkeys.SetAccMenu(menu, Swap,_("Zamień"))->Enable(isen);
	isen = (sels >= 2&&sels <= 5);
	Hkeys.SetAccMenu(menu, Join,_("Złącz linijki"))->Enable(isen);
	isen = (sels >= 2&&sels <= 50);
	Hkeys.SetAccMenu(menu, JoinToFirst,_("Złącz linijki zostaw pierwszą"))->Enable(isen);
	Hkeys.SetAccMenu(menu, JoinToLast,_("Złącz linijki zostaw ostatnią"))->Enable(isen);
	isen = (sels >0);
	Hkeys.SetAccMenu(menu, ContinousPrevious,_("Ustaw czasy jako ciągłe (poprzednia linijka)"))->Enable(isen);
	Hkeys.SetAccMenu(menu, ContinousNext,_("Ustaw czasy jako ciągłe (następna linijka)"))->Enable(isen);
	Hkeys.SetAccMenu(menu, Cut,_("Wytnij\tCtrl-X"))->Enable(isen);
	Hkeys.SetAccMenu(menu, Copy,_("Kopiuj\tCtrl-C"))->Enable(isen);
	Hkeys.SetAccMenu(menu, Paste,_("Wklej\tCtrl-V"));
	Hkeys.SetAccMenu(menu, CopyCollumns,_("Kopiuj kolumny"))->Enable(isen);
	Hkeys.SetAccMenu(menu, PasteCollumns,_("Wklej kolumny"));
	menu->Append(4444,_("Ukryj kolumny"),hidemenu);
	Hkeys.SetAccMenu(menu, NewFPS,_("Ustaw nowy FPS"));
	Hkeys.SetAccMenu(menu, FPSFromVideo,_("Ustaw FPS z wideo"))->Enable(Notebook::GetTab()->Video->GetState()!=None && sels==2);
	Hkeys.SetAccMenu(menu, PasteTranslation,_("Wklej tekst tłumaczenia"))->Enable(form<SRT && ((TabPanel*)GetParent())->SubsPath!="");
	Hkeys.SetAccMenu(menu, TranslationDialog,_("Okno przesuwania dialogów"))->Enable(showtl);
	menu->AppendSeparator();

	Hkeys.SetAccMenu(menu, RemoveText,_("Usuń tekst"))->Enable(isen);
	Hkeys.SetAccMenu(menu, Remove,_("Usuń"))->Enable(isen);
	menu->AppendSeparator();
	Hkeys.SetAccMenu(menu, FontCollector,_("Kolekcjoner czcionek"))->Enable(form<SRT);
	Hkeys.SetAccMenu(menu, SubsFromMKV,_("Wczytaj napisy z pliku MKV"))->Enable(Kai->GetTab()->VideoName.EndsWith(".mkv"));

	if(dummy){
		delete menu;
		VB->blockpaint=false;
		return;
	}
	ismenushown = true;	
	int id=GetPopupMenuSelectionFromUser(*menu,pos);
	ismenushown = false;

	byte state[256];
	/*state[VK_LSHIFT]=0;
	state[VK_RSHIFT]=0;
	state[VK_LCONTROL]=0;
	state[VK_RCONTROL]=0;
	state[VK_LMENU]=0;
	state[VK_RMENU]=0;*/

	if(GetKeyboardState(state)==FALSE){wxLogStatus(_("nie można pobrać stanu przycisków"));}
	if((state[VK_LSHIFT]>1 || state[VK_RSHIFT]>1)/* && (state[VK_LCONTROL]<1 && state[VK_RCONTROL]<1 && state[VK_LMENU]<1 && state[VK_RMENU]<1) */&&id>5000){
		wxMenuItem *item=menu->FindItem(id);
		wxString wins[1]={"Napisy"};
		int ret=-1;
		wxString name=item->GetItemLabelText();
		ret=Hkeys.OnMapHkey(id, name, this, wins, 1);
		if(ret==-1){Notebook::GetTab()->SetAccels();
		Hkeys.SaveHkeys();}
		delete menu;
		VB->blockpaint=false;
		return;
	}
	wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,id);
	OnAccelerator(evt);
	delete menu;
	VB->blockpaint=false;
	//wxLogMessage("all deleted");
}


void Grid::OnInsertBefore()
{
	int rw=selarr[0];
	sel.clear();
	Dialogue *dialog=CopyDial(rw, false);
	dialog->Text="";
	dialog->TextTl="";
	dialog->End=dialog->Start;
	if(rw>0 && GetDial(rw-1)->End > dialog->Start){
		dialog->Start=GetDial(rw-1)->End;
	}else{dialog->Start.Change(-4000);}
	InsertRows(rw, 1, dialog, false, true);
}

void Grid::OnInsertAfter()
{
	int rw=selarr[0];
	sel.clear();
	Dialogue *dialog=CopyDial(rw, false);
	dialog->Text="";
	dialog->TextTl="";
	dialog->Start=dialog->End;
	if(rw<GetCount()-1 && GetDial(rw+1)->End > dialog->Start){
		dialog->End=GetDial(rw+1)->Start;
	}else{dialog->End.Change(4000);}
	Edit->ebrow=rw+1;
	InsertRows(rw+1, 1, dialog, false, true);
}

void Grid::OnDuplicate()
{
	int rw=selarr[0];
	//sel.clear();
	int rw1=rw+1;
	for(size_t i=1; i<selarr.GetCount(); i++){if(rw1==selarr[i]){rw1++;}else{break;} }
	int rw2=rw1-rw;
	std::vector<Dialogue *> dupl;
	for(int i=0; i<rw2; i++){

		dupl.push_back(file->CopyDial(i+rw,false));
		//sel[i+rw1]=true;

	}
	//Edit->ebrow=rw1;

	if(dupl.size()>0){
		InsertRows(rw1, dupl);
		dupl.clear();
	}
	SetModified(false);
	Refresh(false);
}


void Grid::OnJoin(wxCommandEvent &event)
{
	wxString ntext;
	wxString ntltext;
	wxString en1;
	int idd=event.GetId();
	if(idd==JoinWithPrevious){
		if(Edit->ebrow==0){return;}
		selarr.Clear();
		selarr.Add(Edit->ebrow-1);
		selarr.Add(Edit->ebrow);
		en1=" ";
	}else if(idd==JoinWithNext){
		if(Edit->ebrow>=GetCount()||GetCount()<2){return;}
		selarr.Clear();
		selarr.Add(Edit->ebrow);
		selarr.Add(Edit->ebrow+1);
		en1=" ";
	}else{en1="\\N";}


	Dialogue *dialc = file->CopyDial(selarr[0]);
	Edit->ebrow=selarr[0];
	int start=INT_MAX, end=0;
	for(size_t i=0;i<selarr.size();i++)
	{
		wxString en=(i==0)?"":en1;
		Dialogue *dial=GetDial(selarr[i]);
		if(dial->Start.mstime < start){ start = dial->Start.mstime;}
		if(dial->End.mstime > end){	end = dial->End.mstime;}
		if(dial->Text!=""){ntext<<en<<dial->Text;}
		if(dial->TextTl!=""){ntltext<<en<<dial->TextTl;}
	}

	DeleteRow(selarr[1], selarr[selarr.size()-1]-selarr[1]+1);
	dialc->Start.NewTime(start);
	dialc->End.NewTime(end);
	dialc->Text=ntext;
	dialc->TextTl=ntltext;
	sel.clear();
	file->edited=true;
	SpellErrors.clear();
	SetModified();
	RepaintWindow();
}

void Grid::OnJoinToFirst(int id)
{

	Dialogue *dialc = file->CopyDial(selarr[0]);
	Dialogue *ldial = GetDial(selarr[selarr.size()-1]);
	dialc->End = ldial->End;

	if(id==JoinToLast){
		dialc->Text = ldial->Text;
		dialc->TextTl = ldial->TextTl;
	}
	Edit->ebrow=selarr[0];
	DeleteRow(selarr[1], selarr[selarr.size()-1]-selarr[1]+1);

	sel.clear();
	sel[selarr[0]]=true;
	SpellErrors.clear();
	SetModified();
	RepaintWindow();
}


void Grid::OnPaste(int id)
{

	int rw=FirstSel();
	if(rw < 0){wxBell();return;}
	if(id==PasteCollumns){
		wxString arr[ ]={_("Warstwa"),_("Czas początkowy"),_("Czas końcowy"),_("Aktor"),_("Styl"),_("Margines lewy"),_("Margines prawy"),_("Margines pionowy"),_("Efekt"),_("Tekst")};
		int vals[ ]={LAYER,START,END,ACTOR,STYLE,MARGINL,MARGINR,MARGINV,EFFECT,TXT};
		Stylelistbox slx(this,false,arr,10);
		if(slx.ShowModal()==wxID_OK)
		{
			rw=0;
			for (size_t v=0;v<slx.CheckListBox1->GetCount();v++)
			{

				if(slx.CheckListBox1->IsChecked(v)){
					rw|= vals[v];
				}
			}
		}else{return;}

	}else{
		sel.clear();}
	Freeze();
	wxString whatpaste;
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			whatpaste = data.GetText();
		}
		wxTheClipboard->Close();
		if(whatpaste==""){Thaw();return;}
	}
	wxStringTokenizer wpaste(whatpaste,"\n", wxTOKEN_STRTOK);
	int cttkns=wpaste.CountTokens();
	int rws= (id==PasteCollumns)? 0 : rw;
	std::vector<Dialogue*> tmpdial;
	while(wpaste.HasMoreTokens())
	{
		Dialogue *newdial=NULL;
		newdial= new Dialogue(wpaste.NextToken());
		newdial->State=1;
		//wxLogMessage("newdialog %i", (int)newdial);
		if(!newdial){continue;}
		if(newdial->Form!=form){newdial->Conv(form);}
		if(id==Paste){
			tmpdial.push_back(newdial);
			sel[rws]=true;
		}else{
			if(rws<(int)selarr.GetCount()){
				ChangeCell(rw, selarr[rws],newdial);
			}
			delete newdial;
		}
		rws++;
	}

	if(tmpdial.size()>0){
		InsertRows(rw, tmpdial,true);
	}
	if(sel.size()!=0){
		Edit->ebrow=sel.begin()->first;}
	scPos+=cttkns;
	SetModified();
	Thaw();
	RepaintWindow();
}

void Grid::CopyRows(int id)
{
	int cols=0;
	if(id==CopyCollumns){
		wxString arr[ ]={_("Warstwa"),_("Czas początkowy"),_("Czas końcowy"),_("Aktor"),_("Styl"),_("Margines lewy"),_("Margines prawy"),_("Margines pionowy"),_("Efekt"),_("Tekst"),_("Tekst bez tagów")};
		int vals[ ]={LAYER,START,END,ACTOR,STYLE,MARGINL,MARGINR,MARGINV,EFFECT,TXT,TXTTL};
		Stylelistbox slx(this,false,arr,11);
		if(slx.ShowModal()==wxID_OK)
		{
			for (size_t v=0;v<slx.CheckListBox1->GetCount();v++)
			{

				if(slx.CheckListBox1->IsChecked(v)){
					cols|= vals[v];
				}
			}
		}else{return;}

	}
	selarr=GetSels();
	wxString whatcopy;
	for(size_t i=0; i<selarr.GetCount();i++)
	{	
		if(id!=CopyCollumns){
			//tłumaczenie ma pierwszeństwo w kopiowaniu
			whatcopy<<GetDial(selarr[i])->GetRaw(transl && GetDial(selarr[i])->TextTl!="");
		}else{
			whatcopy<<GetDial(selarr[i])->GetCols(cols,transl && GetDial(selarr[i])->TextTl!="");
		}
	}
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(whatcopy) );
		wxTheClipboard->Close();
		wxTheClipboard->Flush();
	}
}

void Grid::OnInsertBeforeVideo()
{
	int rw=selarr[0];
	sel.erase(sel.find(rw));
	Dialogue *dialog=CopyDial(rw, false);
	dialog->Text="";
	dialog->TextTl="";
	int time=Kai->GetTab()->Video->Tell();
	dialog->Start.NewTime(time);
	dialog->End.NewTime(time+4000);
	InsertRows(rw, 1, dialog, false, true);
}

void Grid::OnInsertAfterVideo()
{
	int rw=selarr[0];
	sel.erase(sel.find(rw));
	Dialogue *dialog=CopyDial(rw, false);
	dialog->Text="";
	dialog->TextTl="";
	int time=Kai->GetTab()->Video->Tell();
	dialog->Start.NewTime(time);
	dialog->End.NewTime(time+4000);
	Edit->ebrow=rw+1;
	InsertRows(rw+1, 1, dialog, false, true);
}


void Grid::OnAccelerator(wxCommandEvent &event)
{
	int id=event.GetId();
	VideoCtrl *vb=Kai->GetTab()->Video;
	selarr = GetSels();
	if(id==PlayPause && vb->IsShown()){vb->Pause();}
	else if(id==Plus5Second){vb->Seek(vb->Tell()+5000);}
	else if(id==Minus5Second){vb->Seek(vb->Tell()-5000);}
	else if(id==InsertBefore){OnInsertBefore();}
	else if(id==InsertAfter){OnInsertAfter();}
	else if(id==InsertBeforeVideo){OnInsertBeforeVideo();}
	else if(id==InsertAfterVideo){OnInsertAfterVideo();}
	else if(id==Duplicate){OnDuplicate();}
	else if(id==Swap){SwapRows(selarr[0],selarr[1],true);}
	else if(id==Join){wxCommandEvent evt; evt.SetId(id); OnJoin(evt);}
	else if(id==JoinToFirst || id==JoinToLast){OnJoinToFirst(id);}
	else if(id==Copy||id==CopyCollumns){CopyRows(id);}
	else if(id==Cut){CopyRows(id);DeleteRows();}
	else if(id==Paste || id==PasteCollumns){OnPaste(id);}
	else if(id==Remove){DeleteRows();}
	else if(id==RemoveText){DeleteText();}
	else if(id==PasteTranslation){OnPasteTextTl();}
	else if(id==TranslationDialog){
		static TLDialog *tld= new TLDialog(this,this);
		tld->Show();
	}
	else if(id==SubsFromMKV){OnMkvSubs(event);}
	else if(id==FPSFromVideo){OnSetFPSFromVideo();}
	else if(id==NewFPS){OnSetNewFPS();}
	else if(id==ContinousPrevious||id==ContinousNext){OnMakeContinous(id);}
	else if(id>6000){
		Kai->OnMenuSelected(event);
	}
	else if(id>5000){
		int id5000=(id-5000);
		if(visible & id5000){visible ^= id5000;}
		else{visible |= id5000;}
		SpellErrors.clear();
		Options.SetInt("Grid Hide Collums", visible);
		RepaintWindow();
	}
}


void Grid::OnPasteTextTl()
{
	wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik napisów"), Kai->GetTab()->SubsPath.BeforeLast('\\'), "", _("Pliki napisów (*.ass),(*.srt),(*.sub),(*.txt)|*.ass;*.srt;*.sub;*.txt"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "wxFileDialog");
	if (FileDialog1->ShowModal() == wxID_OK){
		OpenWrite op;
		wxString pathh=FileDialog1->GetPath();
		wxString txt= op.FileOpen(pathh);
		wxString ext=pathh.AfterLast('.');
		int iline=0;

		//for(int i=0;i<GetCount();i++){file->subs.dials[i]->spells.Clear();}

		if(ext=="srt"){
			//wxString dbg;
			wxStringTokenizer tokenizer(txt,"\n",wxTOKEN_STRTOK);
			tokenizer.GetNextToken();
			wxString text1;
			while ( tokenizer.HasMoreTokens() )
			{
				wxString text=tokenizer.GetNextToken().Trim();
				if(IsNum(text)){if(text1!=""){
					//dbg<<text1<<"\n";
					//dbg<<ndl.Start.raw<<" x "<<ndl.End.raw<<" x "<<ndl.Text<<"\n"; 
					Dialogue diall=Dialogue(text1.Trim());
					if(iline<GetCount()){
						diall.Conv(form);
						CopyDial(iline)->TextTl=diall.Text;
					}
					else{
						diall.Conv(form);
						diall.Start.NewTime(0);
						diall.End.NewTime(0);
						diall.Style=GetSInfo("TLMode Style");
						diall.TextTl=diall.Text;
						diall.Text="";
						AddLine(diall.Copy());
					}
					iline++;text1="";}}
				else{text1<<text<<"\r\n";}

			}
		}else{

			wxStringTokenizer tokenizer(txt,"\n",wxTOKEN_STRTOK);
			while ( tokenizer.HasMoreTokens() )
			{
				wxString token = tokenizer.GetNextToken();
				if(!(ext=="ass"&&!token.StartsWith("Dialogue"))){  
					Dialogue diall=Dialogue(token);
					if(iline<GetCount()){
						diall.Conv(form);
						CopyDial(iline)->TextTl=diall.Text;}
					else{
						diall.Conv(form);
						diall.Start.NewTime(0);
						diall.End.NewTime(0);
						diall.Style=GetSInfo("TLMode Style");
						diall.TextTl=diall.Text;
						diall.Text="";
						AddLine(diall.Copy());
					}
					iline++;}	   
			}
		}



		Edit->SetTl(true);
		SetTlMode(true);
		AddSInfo("TLMode Showtl", "Yes");
		showtl=true;
		//Edit->SetIt(Edit->ebrow);
		SetModified();
		Refresh(false);
	}
	FileDialog1->Destroy();
}

void Grid::MoveTextTL(char mode)
{

	wxArrayInt selecs=GetSels(true);
	//wxString kkk1;
	//wxMessageBox(kkk1<<selecs[0]);
	if(selecs.GetCount()<1||!showtl||!transl)return;
	int first=selecs[0];
	int mrow=1;
	if(selecs.GetCount()>1){
		mrow=selecs[1]-first;
	}
	//wxString kkk;
	//wxMessageBox(kkk<<first<<" "<<mrow);
	if(mode<3){// w górę ^
		//tryb 2 gdzie dodaje puste linijki a tekst pl pozostaje bez zmian
		if(mode==2){
			Dialogue *insdial =GetDial(first)->Copy();
			insdial->Text="";
			InsertRows(first, mrow, insdial);
		}
		sel[first]=true;
		for(int i=first; i<GetCount(); i++)
		{
			if(i<first+mrow){
				//tryb1 gdzie łączy wszystkie nachodzące linijki w jedną
				if(mode==1){wxString mid=(GetDial(first)->TextTl!="" && GetDial(i+1)->TextTl!="")?"\\N":"";
				CopyDial(first)->TextTl << mid << GetDial(i+1)->TextTl;
				if(i!=first){CopyDial(i)->TextTl = GetDial(i+mrow)->TextTl;}}
				else{CopyDial(i)->TextTl = GetDial(i+mrow)->TextTl;}
			}
			else if(i<GetCount()-mrow){
				CopyDial(i)->TextTl = GetDial(i+mrow)->TextTl;}
			else if(GetDial(i)->Text!=""){mrow--;}

		}
		//wxString kkk1;
		//wxMessageBox(kkk1<<dial.size()<<" "<<sel.size());

		if(mrow>0){
			DeleteRow(GetCount()-mrow, mrow);
		}

		//wxString kkk;
		//wxMessageBox(kkk<<dial.size()<<" "<<sel.size());
	}else{//w dół v
		int oldgc=GetCount();
		Dialogue diall;
		diall.End.NewTime(0);
		diall.Style=GetSInfo("TLMode Style");
		for(int i=0; i<mrow; i++)
		{
			AddLine(diall.Copy());
		}

		bool onlyo=true;
		//sel[first+mrow]=true;
		for(int i=GetCount()-1; i>=first; i--)
		{
			if(i<first+mrow){
				if(mode==3){
					CopyDial(i)->TextTl="";}
				else if(mode==4||mode==5){
					if(mode==4){if(onlyo){CopyDial(first+mrow)->Start = GetDial(first)->Start; onlyo=false;}
					CopyDial(first+mrow)->Text.Prepend(GetDial(i)->Text+"\\N");mrow--;}
					DeleteRow(i);
				}
			}
			else{
				CopyDial(i)->TextTl = GetDial(i-mrow)->TextTl;}


		}

	}
	SetModified(true);
	Refresh(false);

}


void Grid::OnMkvSubs(wxCommandEvent &event)
{
	int idd=event.GetId();
	if(Modified){
		int wbutton=wxMessageBox(_("Zapisać plik przed wczytaniem napisów z MKV?"), 
			_("Potwierdzenie"),wxICON_QUESTION | wxYES_NO |wxCANCEL, this);
		if (wbutton==wxYES){Kai->Save(false);}
		else if(wbutton==wxCANCEL){return;}}
	wxString mkvpath;
	if(idd==SubsFromMKV)
	{
		mkvpath=Kai->GetTab()->VideoPath;
	}
	else{
		mkvpath=event.GetString();
	}

	MatroskaWrapper mw;
	try{
		mw.Open(mkvpath,false);
	}catch(...){return;}
	bool isgood=mw.GetSubtitles(this);
	if(isgood){
		if(transl){Edit->SetTl(false); transl=false;showtl=false;Kai->MenuBar->Enable(SaveTranslation,false);}
		SetSubsForm();
		wxString ext=(form<SRT)?"ass" : "srt";
		if(form<SRT){Edit->TlMode->Enable();}else{Edit->TlMode->Enable(false);}

		Kai->GetTab()->SubsPath=mkvpath.BeforeLast('.')+_(" napisy.")+ext;
		Kai->GetTab()->SubsName=Kai->GetTab()->SubsPath.AfterLast('\\');
		//Kai->SetRecent();
		Kai->UpdateToolbar();
		Edit->RefreshStyle(true);

		Kai->Label();
		if(form==ASS){
			wxString katal=GetSInfo("Last Style Storage");

			if(katal!=""){
				for(size_t i=0;i<Options.dirs.size();i++){
					if(katal==Options.dirs[i]){Options.LoadStyles(katal);}
				}
			}


		}
		if(Kai->GetTab()->Video->GetState()!=None){Kai->GetTab()->Video->OpenSubs(SaveText());
			if(!isgood){wxMessageBox(_("Otwieranie napisów nie powiodło się"), _("Uwaga"));}
			if(Kai->GetTab()->Video->GetState()==Paused){Kai->GetTab()->Video->Render();}
		}

		if(!Kai->GetTab()->edytor&&!Kai->GetTab()->Video->isfullskreen){Kai->HideEditor();}
		Kai->GetTab()->CTime->Contents();

		RepaintWindow();
		Edit->HideControls();
	}
	mw.Close();
	if(Kai->ss && form==ASS){Kai->ss->LoadAssStyles();}
}

wxString getfloatstr(float num)
{
	wxString strnum=wxString::Format("%f",num);
	strnum.Replace(",",".");
	int rmv=0;
	for(int i=strnum.Len()-1;i>0;i--){
		if(strnum[i]=='0'){rmv++;}
		else if(strnum[i]=='.'){rmv++;break;}
		else{break;}
	}
	if(rmv){strnum.RemoveLast(rmv);}
	return strnum;
}


void Grid::ResizeSubs(float xnsize, float ynsize)
{
	float val=ynsize;//(ynsize>xnsize)?ynsize:xnsize;
	//float fscyval=ynsize/xnsize;

	for(int i=0;i<StylesSize();i++){
		Styles *resized= file->CopyStyle(i);
		int ml=wxAtoi(resized->MarginL);
		ml*=xnsize;
		resized->MarginL="";
		resized->MarginL<<ml;
		int mr=wxAtoi(resized->MarginR);
		mr*=xnsize;
		resized->MarginR="";
		resized->MarginR<<mr;
		int mv=wxAtoi(resized->MarginV);
		mv*=ynsize;
		resized->MarginV="";
		resized->MarginV<<mv;
		//int sy=wxAtoi(resized.ScaleY);
		//sy*=fscyval;
		//resized.ScaleY="";
		//resized.ScaleY<<sy;
		double fs=0;
		resized->Fontsize.ToCDouble(&fs);
		fs*=val;
		resized->Fontsize="";
		resized->Fontsize<<getfloatstr(fs);
		float ol=wxAtoi(resized->Outline);
		ol*=val;
		resized->Outline=getfloatstr(ol);
		float sh=wxAtoi(resized->Shadow);
		sh*=val;
		resized->Shadow=getfloatstr(sh);
		//dbg<<resized.styletext()<<"\r\n";
		//ChangeStyle(resized,i);
	}

	//dialogi, największy hardkor, wszystkie tagi zależne od rozdzielczości trzeba zmienić
	//tu zacznie się potęga szukaczki tagów
	wxRegEx onenum("\\\\(fax|fay|fs|bord|shad|pos|move|iclip|clip|org)([^\\\\}]*)",wxRE_ADVANCED);
	wxRegEx drawing("\\\\p([0-9]+)[\\\\}]",wxRE_ADVANCED);

	for(int i=0;i<GetCount();i++){
		//zaczniemy od najłatwiejszego, marginesy

		bool sdone=false;
		Dialogue *diall=GetDial(i);
		if(!diall->IsComment){
			diall=diall->Copy();
			if(diall->MarginL){diall->MarginL*=xnsize;sdone=true;}
			if(diall->MarginR){diall->MarginR*=xnsize;sdone=true;}
			if(diall->MarginV){diall->MarginV*=ynsize;sdone=true;}

			wxString txt=diall->Text;
			size_t pos=0;
			bool tdone=false;
			//pętla tagów
			while(true){
				wxString subtxt=txt.Mid(pos);
				if(onenum.Matches(subtxt)){
					//dbg<<"matches\r\n";
					size_t start,len;
					if(onenum.GetMatch(&start,&len,2))
					{
						//dbg<<"start "<<start<<" end "<<len<<"\r\n";

						double valtag=0;
						wxString crop=subtxt.Mid(start,len);
						//dbg<<crop<<"\r\n";
						//na początek wszystko co w nawiasach
						if(crop.StartsWith("(",&crop))
						{
							crop.EndsWith(")",&crop);
							int crm=crop.Find('m');
							int crn=crop.Find('n');
							wxString wynik="(";
							int ii=0;
							//clip rysunkowy
							if (crm!=-1||crn!=-1)
							{
								wxStringTokenizer tknzr(crop," ",wxTOKEN_STRTOK);
								while(tknzr.HasMoreTokens())
								{
									wxString tkn=tknzr.NextToken();
									if(tkn.IsNumber())
									{
										int val=wxAtoi(tkn);
										val*=(ii%2==0)?xnsize:ynsize;
										wynik<<val<<" ";
										ii++;
									}
									else
									{
										wynik<<tkn<<" ";
									}
								}
								wynik.Trim();
							}
							else     //No i cała reszta, clip normalny, pos, move i org
							{
								wxStringTokenizer tknzr(crop,",",wxTOKEN_STRTOK);
								while(tknzr.HasMoreTokens())
								{
									wxString tkn=tknzr.NextToken();
									tkn.Trim();
									tkn.Trim(false);
									double vlt;

									if(ii<4&&tkn.ToCDouble(&vlt))
									{
										vlt*=(ii%2==0)?xnsize:ynsize;
										wynik<<getfloatstr(vlt)<<",";
										ii++;
									}
									else
									{
										wynik<<tkn<<",";
										ii++;
									}
								}
								wynik=wynik.BeforeLast(',');
							}

							wynik<<")";
							
							txt.Remove(pos+start,len);
							txt.insert(pos+start,wynik);
							
							tdone=true;
						}
						else if(crop.ToCDouble(&valtag))
						{
							
							valtag*=val;
							
							txt.Remove(pos+start,len);
							
							txt.insert(pos+start,getfloatstr((float)valtag));
							
							tdone=true;
						}
						pos+=(start+len);
					}else{break;}

				}else{break;}

			}
			//rysunki
			if(drawing.Matches(txt))
			{
				
				size_t start,len;
				if(drawing.GetMatch(&start,&len,0))
				{
					wxString draw;
					size_t brpos=txt.find('}',start+len-1);
					if(brpos>0){
						draw=txt.Mid(brpos);
						start=brpos;}
					else{draw=txt.Mid(start+len-1);}
					int brpos1=draw.Find('{');
					if(brpos1!=-1){draw=draw.Mid(0,brpos1-1);len=brpos1-1;}else{len=draw.Len();}
					//dbg<<draw<<"\r\n";
					wxStringTokenizer tknzr(draw," ",wxTOKEN_STRTOK);
					wxString wynik1;
					int ii=0;
					
					while(tknzr.HasMoreTokens())
					{
						wxString tkn=tknzr.NextToken();
						if(tkn.IsNumber())
						{
							int vala=wxAtoi(tkn);
							vala*=val;
							wynik1<<vala<<" ";
							ii++;
						}
						else
						{
							wynik1<<tkn<<" ";
						}
					}
					wynik1.Trim();
					txt.Remove(start,len);
					txt.insert(start,wynik1);
					tdone=true;
				}
			}
			

			if(tdone){
				SpellErrors[i].clear();
				diall->Text=txt; sdone=true;
			}
			if(sdone){
				file->GetSubs()->ddials.push_back(diall);
				file->GetSubs()->dials[i]=diall;
			}
			else{
				delete diall;
			}
		}
	}

	Refresh(false);
}

void Grid::OnMakeContinous(int idd)
{
	int fs=FirstSel();
	if(fs<0){wxBell();return;}
	if(idd==ContinousPrevious){
		if(fs<1){return;}
		int diff=GetDial(fs)->End.mstime - GetDial(fs-1)->Start.mstime;
		for(int i=0;i<fs;i++)
		{
			Dialogue *dialc=CopyDial(i);
			dialc->Start.Change(diff);
			dialc->End.Change(diff);
		}
	}
	else
	{
		if(fs>=GetCount()-1){return;}
		int diff=GetDial(fs)->End.mstime - GetDial(fs+1)->Start.mstime;
		for(int i=fs+1;i<GetCount();i++)
		{
			Dialogue *dialc=CopyDial(i);
			dialc->Start.Change(diff);
			dialc->End.Change(diff);
		}
	}
	SetModified();
	Refresh(false);
}

void Grid::ConnectAcc(int id)
{
	Connect(id,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&Grid::OnAccelerator);
}

void Grid::OnSetFPSFromVideo()
{
	if(selarr.size()!=2){return;}
	Dialogue *first=GetDial(selarr[0]);
	Dialogue *second=GetDial(selarr[1]);
	int ftime=first->Start.mstime;
	int stime=second->Start.mstime;
	int vtime=Notebook::GetTab()->Video->Tell();
	float diffv=(vtime-stime);
	float diffg=(stime-ftime);

	for (int i=0;i<GetCount();i++){
		Dialogue *dialc=CopyDial(i);
		dialc->Start.Change(diffv *((dialc->Start.mstime-ftime)/diffg));
		dialc->End.Change(diffv *((dialc->End.mstime-ftime)/diffg));
	}
	SetModified();
	if(form>TMP){RepaintWindow(START|END);}else{Refresh(false);}
}

class fpsdial : public wxDialog
{
public:
	fpsdial(wxWindow *parent)
		:wxDialog(parent,-1,_("Wybierz nowy FPS"))
	{
		wxFlexGridSizer *sizer=new wxFlexGridSizer(2,2,2);
		wxArrayString fpsy;
		wxTextValidator valid(wxFILTER_INCLUDE_CHAR_LIST);
		wxArrayString includes;
		includes.Add("0");
		includes.Add("1");
		includes.Add("2");
		includes.Add("3");
		includes.Add("4");
		includes.Add("5");
		includes.Add("6");
		includes.Add("7");
		includes.Add("8");
		includes.Add("9");
		includes.Add(".");
		valid.SetIncludes(includes);

		fpsy.Add("23.976");fpsy.Add("24");fpsy.Add("25");fpsy.Add("29.97");fpsy.Add("30");fpsy.Add("60");
		oldfps=new wxComboBox(this,-1,"",wxDefaultPosition,wxDefaultSize,fpsy,0,valid);
		oldfps->SetSelection(0);
		newfps=new wxComboBox(this,-1,"",wxDefaultPosition,wxDefaultSize,fpsy,0,valid);
		newfps->SetSelection(2);
		sizer->Add(new wxStaticText(this,-1,_("FPS napisów")),0,wxALIGN_CENTER_VERTICAL|wxALL,4);
		sizer->Add(oldfps,0,wxEXPAND|wxALL,4);
		sizer->Add(new wxStaticText(this,-1,_("Nowy FPS napisów")),0,wxALIGN_CENTER_VERTICAL|wxALL,4);
		sizer->Add(newfps,0,wxEXPAND|wxALL,4);
		wxButton *ok=new wxButton(this,15555,_("Zmień fps"));
		Connect(15555,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsdial::OkClick);
		wxButton *cancel=new wxButton(this,wxID_CANCEL,_("Anuluj"));
		sizer->Add(ok,0,wxEXPAND|wxALL,4);
		sizer->Add(cancel,0,wxEXPAND|wxALL,4);
		SetSizerAndFit(sizer);
		CenterOnParent();
	}
	virtual ~fpsdial(){};
	void OkClick(wxCommandEvent &evt)
	{

		if(oldfps->GetValue().ToDouble(&ofps) && newfps->GetValue().ToDouble(&nfps)){
			EndModal(1);
		}else{wxMessageBox(_("Niewłaściwy fps"));}
	}
	double ofps,nfps;
	wxComboBox *oldfps;
	wxComboBox *newfps;
};

void Grid::OnSetNewFPS()
{
	fpsdial nfps(this);
	if(nfps.ShowModal()==1){
		double sub = nfps.ofps / nfps.nfps;

		for (int i=0;i<GetCount();i++){
			Dialogue *dialc=CopyDial(i);
			dialc->Start.NewTime(dialc->Start.mstime*sub);
			dialc->End.NewTime(dialc->End.mstime*sub);
		}
		SetModified();
		if(form>TMP){RepaintWindow(START|END);}else{Refresh(false);}
	}
}



BEGIN_EVENT_TABLE(Grid,SubsGrid)
	EVT_MENU(Cut,Grid::OnAccelerator)
	EVT_MENU(Copy,Grid::OnAccelerator)
	EVT_MENU(Paste,Grid::OnAccelerator)
END_EVENT_TABLE()