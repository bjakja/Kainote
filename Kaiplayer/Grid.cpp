#include "Grid.h"


#include <wx/intl.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include "kainoteMain.h"
#include "Hotkeys.h"
#include "OpennWrite.h"
#include "TLDialog.h"
#include "mkv_wrap.h"
#include "Stylelistbox.h"
#include <wx/regex.h>



BEGIN_EVENT_TABLE(Grid,SubsGrid)
EVT_MENU(MENU_CUT,Grid::OnAccelerator)
EVT_MENU(MENU_COPY,Grid::OnAccelerator)
//EVT_MENU(MENU_DUPLICATE,Grid::OnAccelerator)
EVT_MENU(MENU_PASTE,Grid::OnAccelerator)
//EVT_MENU(MENU_PASTECOLS,Grid::OnAccelerator)
EVT_MENU_RANGE(MENU_PLAYP,MENU_M5SEC,Grid::OnAccelerator)
//EVT_MENU(MENU_PPSEC,Grid::OnAccelerator)
//EVT_MENU(MENU_MPSEC,Grid::OnAccelerator)
END_EVENT_TABLE()

Grid::Grid(wxWindow* parent, kainoteFrame* kfparent,wxWindowID id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
           : SubsGrid(parent, id, pos, size, style, name)
{
	Kai=kfparent;
}

Grid::~Grid()
{
}

void Grid::ContextMenu(const wxPoint &pos)
{
	VideoCtrl *VB=((TabPanel*)GetParent())->Video;
	VB->blockpaint=true;
    selarr = GetSels();
    int sels=selarr.GetCount();
    wxMenu *menu=new wxMenu;
	hidemenu=new wxMenu;
	wxMenuItem *item;
	item = Hkeys.SetAccMenu(hidemenu, 5000+LAYER,_("Ukryj warstwê"),_("Ukryj warstwê"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & LAYER)!=0);
	Hkeys.SetAccMenu(hidemenu, 5000+START,_("Ukryj czas pocz¹tkowy"),_("Ukryj czas pocz¹tkowy"),wxITEM_CHECK)->Check((visible & START)!=0);
	item = Hkeys.SetAccMenu(hidemenu, END,_("Ukryj czas koñcowy"),_("Ukryj czas koñcowy"),wxITEM_CHECK);
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
	item = Hkeys.SetAccMenu(hidemenu, 5000+MARGINV,_("Ukryj pionowy pargines"),_("Ukryj pionowy pargines"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & MARGINV)!=0);
	item = Hkeys.SetAccMenu(hidemenu, 5000+EFFECT,_("Ukryj efekt"),_("Ukryj efekt"),wxITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & EFFECT)!=0);
	Hkeys.SetAccMenu(hidemenu, 5000+CNZ,_("Ukryj czas na znak"),_("Ukryj czas na znak"),wxITEM_CHECK)->Check((visible & CNZ)!=0);

	bool isen;
	isen = (sels == 1);
	Hkeys.SetAccMenu(menu, MENU_INSERT_BEFORE,_("Wstaw przed"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_INSERT_AFTER,_("Wstaw po"))->Enable(isen);
	isen = (isen&&Kai->GetTab()->Video->GetState()!=None);
	Hkeys.SetAccMenu(menu, MENU_INSERT_BEFORE_VIDEO,_("Wstaw przed z czasem wideo"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_INSERT_AFTER_VIDEO,_("Wstaw po z czasem wideo"))->Enable(isen);
    isen = (sels >0);
	Hkeys.SetAccMenu(menu, MENU_DUPLICATE,_("Duplikuj Linie"))->Enable(isen);
	isen = (sels == 2);
	Hkeys.SetAccMenu(menu, MENU_SWAP,_("Zamieñ"))->Enable(isen);
    isen = (sels >= 2&&sels <= 5);
	Hkeys.SetAccMenu(menu, MENU_JOIN,_("Z³¹cz linijki"))->Enable(isen);
	isen = (sels >= 2&&sels <= 50);
	Hkeys.SetAccMenu(menu, MENU_JOINF,_("Z³¹cz linijki zostaw pierwsz¹"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_JOINL,_("Z³¹cz linijki zostaw ostatni¹"))->Enable(isen);
	isen = (sels >0);
	Hkeys.SetAccMenu(menu, MENU_CONT_PREV,_("Ustaw czasy jako ci¹g³e (poprzednia linijka)"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_CONT_NEXT,_("Ustaw czasy jako ci¹g³e (nastêpna linijka)"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_CUT,_("Wytnij\tCtrl-X"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_COPY,_("Kopiuj\tCtrl-C"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_PASTE,_("Wklej\tCtrl-V"));
	Hkeys.SetAccMenu(menu, MENU_COPYCOLS,_("Kopiuj kolumny"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_PASTECOLS,_("Wklej kolumny"));
	menu->Append(4444,"Ukryj kolumny",hidemenu);
	Hkeys.SetAccMenu(menu, MENU_NEWFPS,_("Ustaw nowy fps"));
	Hkeys.SetAccMenu(menu, MENU_FPSFROMVIDEO,_("Ustaw fps z wideo"))->Enable(Notebook::GetTab()->Video->GetState()!=None && sels==2);
	Hkeys.SetAccMenu(menu, MENU_PASTE_TEXTTL,_("Wklej tekst t³umaczenia"))->Enable(form<SRT && ((TabPanel*)GetParent())->SubsPath!="");
	Hkeys.SetAccMenu(menu, MENU_TLDIAL,_("Okno przesuwania dialogów"))->Enable(showtl);
	menu->AppendSeparator();

    Hkeys.SetAccMenu(menu, MENU_DELETE_TEXT,_("Usuñ tekst"))->Enable(isen);
	Hkeys.SetAccMenu(menu, MENU_DELETE,_("Usuñ"))->Enable(isen);
	menu->AppendSeparator();
	Hkeys.SetAccMenu(menu, ID_COLLECTOR,_("Kolekcjoner czcionek"))->Enable(form<SRT);
	Hkeys.SetAccMenu(menu, MENU_MKV_SUBS,_("Wczytaj napisy z pliku MKV"))->Enable(Kai->GetTab()->VideoName.EndsWith(".mkv"));
		
	ismenushown = true;	
	int id=GetPopupMenuSelectionFromUser(*menu,pos);
	ismenushown = false;

	byte state[256];
	if(GetKeyboardState(state)==FALSE){wxLogStatus("nie mo¿na pobraæ stanu przycisków");}
	if((state[VK_LSHIFT]>1 || state[VK_RSHIFT]>1)&&id>5000){
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
    dialog->Text=_("");
	dialog->TextTl=_("");
    dialog->End=dialog->Start;
    if(rw>0 && file->subs->dials[rw-1]->End > dialog->Start){
    dialog->Start=file->subs->dials[rw-1]->End;
    }else{dialog->Start.Change(-4000);}
    InsertRow(rw, dialog);
}

void Grid::OnInsertAfter()
{
    int rw=selarr[0];
    sel.clear();
    Dialogue *dialog=CopyDial(rw, false);
    dialog->Text=_("");
	dialog->TextTl=_("");
    dialog->Start=dialog->End;
    if(rw<GetCount()-1 && file->subs->dials[rw+1]->End > dialog->Start){
    dialog->End=file->subs->dials[rw+1]->Start;
    }else{dialog->End.Change(4000);}
	Edit->ebrow=rw+1;
    InsertRow(rw+1, dialog);
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
        file->subs->dials.insert(file->subs->dials.begin()+rw1, dupl.begin(), dupl.end());
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
	if(idd==GRID_JOINWP){
		if(Edit->ebrow==0){return;}
		selarr.Clear();
		selarr.Add(Edit->ebrow-1);
		selarr.Add(Edit->ebrow);
		en1=" ";
	}else if(idd==GRID_JOINWN){
		if(Edit->ebrow>=GetCount()||GetCount()<2){return;}
		selarr.Clear();
		selarr.Add(Edit->ebrow);
		selarr.Add(Edit->ebrow+1);
		en1=" ";
	}else{en1="\\N";}


	Dialogue *dialc = file->CopyDial(selarr[0]);
	dialc->End = file->subs->dials[selarr[selarr.size()-1]]->End;
	Edit->ebrow=selarr[0];

	for(size_t i=0;i<selarr.size();i++)
	{
		wxString en=(i==0)?"":en1;
		Dialogue *dial=GetDial(selarr[i]);
		if(dial->Text!=""){ntext<<en<<dial->Text;}
		if(dial->TextTl!=""){ntltext<<en<<dial->TextTl;}
	}
	//wxLogStatus(" erase %i %i", selarr[1], selarr[selarr.size()-1]);
	file->subs->dials.erase(file->subs->dials.begin()+selarr[1], file->subs->dials.begin()+selarr[selarr.size()-1]+1);
	dialc->Text=ntext;
	dialc->TextTl=ntltext;
	sel.clear();
	file->edited=true;
	SetModified();
	RepaintWindow();
	}

void Grid::OnJoinF(int id)
	{

	Dialogue *dialc = file->CopyDial(selarr[0]);
	Dialogue *ldial = GetDial(selarr[selarr.size()-1]);
	dialc->End = ldial->End;
	
	if(id==MENU_JOINL){
		dialc->Text = ldial->Text;
		dialc->TextTl = ldial->TextTl;
	}
	Edit->ebrow=selarr[0];
	file->subs->dials.erase(file->subs->dials.begin()+selarr[1], file->subs->dials.begin()+selarr[selarr.size()-1]+1);
	
	sel.clear();
	sel[selarr[0]]=true;
	SetModified();
	RepaintWindow();
	}


void Grid::OnPaste(int id)
{
	
	int rw=FirstSel();
	if(id==MENU_PASTECOLS){
		wxString arr[ ]={"Warstwa","Czas pocz¹tkowy","Czas koñcowy","Aktor","Styl","Margines lewy","Margines prawy","Margines pionowy","Efekt","Tekst"};
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
    wxStringTokenizer wpaste(whatpaste,_("\n"), wxTOKEN_STRTOK);
    int cttkns=wpaste.CountTokens();
	int rws= (id==MENU_PASTECOLS)? 0 : rw;
	
    while(wpaste.HasMoreTokens())
    {
		Dialogue *newdial=NULL;
        newdial= new Dialogue(wpaste.NextToken());
		newdial->State=1;
		//wxLogMessage("newdialog %i", (int)newdial);
		if(!newdial){continue;}
		if(newdial->Form!=form){newdial->Conv(form);}
		if(id==MENU_PASTE){
			file->subs->dials.insert(file->subs->dials.begin()+rws, newdial);
			file->subs->ddials.push_back(newdial);
			sel[rws]=true;
			//wxLogMessage("insert");
		}else{
			if(rws<(int)selarr.GetCount()){
				ChangeCell(rw, selarr[rws],newdial);
			}
			delete newdial;
			//wxLogMessage("insert cols");
		}
		rws++;
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
	if(id==MENU_COPYCOLS){
		wxString arr[ ]={"Warstwa","Czas pocz¹tkowy","Czas koñcowy","Aktor","Styl","Margines lewy","Margines prawy","Margines pionowy","Efekt","Tekst","Tekst bez tagów"};
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
		if(id!=MENU_COPYCOLS){
			//t³umaczenie ma pierwszeñstwo w kopiowaniu
			whatcopy<<file->subs->dials[selarr[i]]->GetRaw(transl && file->subs->dials[selarr[i]]->TextTl!="");
		}else{
			whatcopy<<file->subs->dials[selarr[i]]->GetCols(cols,transl && file->subs->dials[selarr[i]]->TextTl!="");
		}
    }
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData( new wxTextDataObject(whatcopy) );
        wxTheClipboard->Close();
    }
}

void Grid::OnInsertBeforeVideo()
{
	int rw=selarr[0];
    sel.erase(sel.find(rw));
    Dialogue *dialog=CopyDial(rw, false);
    dialog->Text=_("");
	dialog->TextTl=_("");
	int time=Kai->GetTab()->Video->Tell();
    dialog->Start.NewTime(time);
    dialog->End.NewTime(time+4000);
    InsertRow(rw, dialog);
}

void Grid::OnInsertAfterVideo()
{
	int rw=selarr[0];
    sel.erase(sel.find(rw));
    Dialogue *dialog=CopyDial(rw, false);
    dialog->Text=_("");
	dialog->TextTl=_("");
	int time=Kai->GetTab()->Video->Tell();
    dialog->Start.NewTime(time);
    dialog->End.NewTime(time+4000);
	Edit->ebrow=rw+1;
    InsertRow(rw+1, dialog);
}


void Grid::OnAccelerator(wxCommandEvent &event)
{
	int id=event.GetId();
	VideoCtrl *vb=Kai->GetTab()->Video;
	selarr = GetSels();
	if(id==MENU_PLAYP && vb->IsShown()){vb->Pause();}
	else if(id==MENU_P5SEC){vb->Seek(vb->Tell()+5000);}
	else if(id==MENU_M5SEC){vb->Seek(vb->Tell()-5000);}
	else if(id==MENU_INSERT_BEFORE){OnInsertBefore();}
	else if(id==MENU_INSERT_AFTER){OnInsertAfter();}
	else if(id==MENU_INSERT_BEFORE_VIDEO){OnInsertBeforeVideo();}
	else if(id==MENU_INSERT_AFTER_VIDEO){OnInsertAfterVideo();}
	else if(id==MENU_SWAP){SwapRows(selarr[0],selarr[1],true);}
	else if(id==MENU_DUPLICATE){OnDuplicate();}
	else if(id==MENU_JOIN){wxCommandEvent evt; evt.SetId(id); OnJoin(evt);}
	else if(id==MENU_JOINF || id==MENU_JOINL){OnJoinF(id);}
	else if(id==MENU_COPY||id==MENU_COPYCOLS){CopyRows(id);}
	else if(id==MENU_CUT){CopyRows(id);DeleteRows();}
	else if(id==MENU_PASTE || id==MENU_PASTECOLS){OnPaste(id);}
	else if(id==MENU_DELETE){DeleteRows();}
	else if(id==MENU_DELETE_TEXT){DeleteText();}
	else if(id==MENU_PASTE_TEXTTL){OnPasteTextTl();}
	else if(id==MENU_TLDIAL){
		static TLDialog *tld= new TLDialog(this,this);
		tld->Show();
	}
	else if(id==MENU_MKV_SUBS){OnMkvSubs(event);}
	else if(id==MENU_FPSFROMVIDEO){OnSetFPSFromVideo();}
	else if(id==MENU_NEWFPS){OnSetNewFPS();}
	else if(id==MENU_CONT_PREV||id==MENU_CONT_NEXT){OnMakeContinous(id);}
	else if(id>6000){
		Kai->OnMenuSelected(event);
	}
	else if(id>5000){
		int id5000=(id-5000);
		if(visible & id5000){visible ^= id5000;}
		else{visible |= id5000;}
		Options.SetInt("Grid Hide Collums", visible);
		RepaintWindow();
	}
}


void Grid::OnPasteTextTl()
{
	wxFileDialog *FileDialog1 = new wxFileDialog(this, _T("Wybierz Plik Napisów"), Kai->GetTab()->SubsPath.BeforeLast('\\'), _T(""), _T("Pliki napisów (*.ass),(*.srt),(*.sub),(*.txt)|*.ass;*.srt;*.sub;*.txt"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    if (FileDialog1->ShowModal() == wxID_OK){
		 OpenWrite op;
		 wxString pathh=FileDialog1->GetPath();
		 wxString txt= op.FileOpen(pathh);
		 wxString ext=pathh.AfterLast('.');
		 int iline=0;

		 //for(int i=0;i<GetCount();i++){file->subs.dials[i]->spells.Clear();}

		 if(ext=="srt"){
			   //wxString dbg;
               wxStringTokenizer tokenizer(txt,_T("\n"),wxTOKEN_STRTOK);
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

				wxStringTokenizer tokenizer(txt,_T("\n"),wxTOKEN_STRTOK);
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
	if(mode<3){// w górê ^
		//tryb 2 gdzie dodaje puste linijki a tekst pl pozostaje bez zmian
		if(mode==2){Dialogue *insdial =GetDial(first)->Copy();insdial->Text="";
			file->subs->dials.insert(file->subs->dials.begin()+first,mrow,insdial);
			file->subs->ddials.push_back(insdial);
		}
		sel[first]=true;
		for(int i=first; i<GetCount(); i++)
		{
			if(i<first+mrow){
				//tryb1 gdzie ³¹czy wszystkie nachodz¹ce linijki w jedn¹
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
			file->edited=true;
			file->subs->dials.erase(file->subs->dials.begin()+(GetCount()-mrow),file->subs->dials.end());
			//file->subs->dials.pop_back();
		//sel.erase(sel.begin()+(GetCount()),sel.end());
		}

		//wxString kkk;
		//wxMessageBox(kkk<<dial.size()<<" "<<sel.size());
	}else{//w dó³ v
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
            int wbutton=wxMessageBox(_T("Zapisaæ plik przed wczytaniem napisów z mkv?"), 
				_T("Potwierdzenie"),wxICON_QUESTION | wxYES_NO |wxCANCEL, this);
					if (wbutton==wxYES){Kai->Save(false);}
						else if(wbutton==wxCANCEL){return;}}
	wxString mkvpath;
	if(idd==MENU_MKV_SUBS)
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
		if(transl){Edit->SetTl(false); transl=false;showtl=false;Kai->MenuBar->Enable(ID_SAVETL,false);}
		SetSubsForm();
		wxString ext=(form<SRT)?_("ass") : _("srt");
		if(form<SRT){Edit->TlMode->Enable();}else{Edit->TlMode->Enable(false);}

    Kai->GetTab()->SubsPath=mkvpath.BeforeLast('.')+" napisy."+ext;
	Kai->GetTab()->SubsName=Kai->GetTab()->SubsPath.AfterLast('\\');
	//Kai->SetRecent();
	Kai->UpdateToolbar();
	Edit->RefreshStyle(true);

    Kai->Label();
    if(form==ASS){
		wxString katal=GetSInfo(_T("Last Style Storage"));
		
		if(katal!=_T("")){
			for(size_t i=0;i<Options.dirs.size();i++){
				if(katal==Options.dirs[i]){Options.LoadStyles(katal);}
			}
		}
		

    }
	if(Kai->GetTab()->Video->GetState()!=None){Kai->GetTab()->Video->OpenSubs(SaveText());
	if(!isgood){wxMessageBox(_T("otwieranie napisów failed"), _T("Uwaga"));}}
	
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
	wxString strnum=wxString::Format(_T("%f"),num);
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

	for(size_t i=0;i<file->subs->styles.size();i++){
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
	
	//dialogi, najwiêkszy hardkor, wszystkie tagi zale¿ne od rozdzielczoœci trzeba zmieniæ
	//tu zacznie siê potêga szukaczki tagów
	wxRegEx onenum("\\\\(fax|fay|fs|bord|shad|pos|move|iclip|clip|org)([^\\\\}]*)",wxRE_ADVANCED);
	wxRegEx drawing("\\\\p([0-9]+)[\\\\}]",wxRE_ADVANCED);
	
	for(int i=0;i<GetCount();i++){
		//zaczniemy od naj³atwiejszego, marginesy
	
		bool sdone=false;
		Dialogue *diall=GetDial(i)->Copy();
		if(!diall->IsComment){
		if(diall->MarginL){diall->MarginL*=xnsize;sdone=true;}
		if(diall->MarginR){diall->MarginR*=xnsize;sdone=true;}
		if(diall->MarginV){diall->MarginV*=ynsize;sdone=true;}

		wxString txt=diall->Text;
		size_t pos=0;
		bool tdone=false;
		//pêtla tagów
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
					//na pocz¹tek wszystko co w nawiasach
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
							wxStringTokenizer tknzr(crop,_T(" "),wxTOKEN_STRTOK);
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
						else     //No i ca³a reszta, clip normalny, pos, move i org
							{
							wxStringTokenizer tknzr(crop,_T(","),wxTOKEN_STRTOK);
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
						//dbg<<wynik<<"\r\n";
						txt.Remove(pos+start,len);
						txt.insert(pos+start,wynik);
						//dbg<<txt<<"\r\n";
						tdone=true;
						}
					else if(crop.ToCDouble(&valtag))
						{
						//wxString tagg=onenum.GetMatch(txt,1);
						valtag*=val;
						//dbg<<valtag<<"\r\n";
						txt.Remove(pos+start,len);
						//dbg<<txt<<"\r\n";
						txt.insert(pos+start,getfloatstr((float)valtag));
						//dbg<<txt<<"\r\n";
						tdone=true;
						}
					pos+=(start+len);
					}else{break;}

				}else{break;}

			}
		//rysunki
		//if(ynsize!=xnsize)
			//{
			//dbg<<"wesz³o\r\n";
			if(drawing.Matches(txt))
				{
				//dbg<<"maches"<<"\r\n";
				size_t start,len;
				if(drawing.GetMatch(&start,&len,0))
					{
					//dbg<<start<<" "<<len<<"\r\n";
				wxString draw;
				size_t brpos=txt.find('}',start+len-1);
				if(brpos>0){
				draw=txt.Mid(brpos);
				start=brpos;}
				else{draw=txt.Mid(start+len-1);}
				int brpos1=draw.Find('{');
				if(brpos1!=-1){draw=draw.Mid(0,brpos1-1);len=brpos1-1;}else{len=draw.Len();}
				//dbg<<draw<<"\r\n";
				wxStringTokenizer tknzr(draw,_T(" "),wxTOKEN_STRTOK);
				wxString wynik1;
				int ii=0;
				//float hval=(xnsize<ynsize)? xnsize/ynsize : ynsize/xnsize;
							while(tknzr.HasMoreTokens())
								{
								wxString tkn=tknzr.NextToken();
								if(tkn.IsNumber())
									{
									//if(ii%2==0){
										int vala=wxAtoi(tkn);
										vala*=val;
										wynik1<<vala<<" ";
										//}
									//else{
										//wynik1<<tkn<<" ";}
									//dbg<<wynik1;
									ii++;
									}
								else
									{
									wynik1<<tkn<<" ";
									//dbg<<wynik1;
									}
								}
							wynik1.Trim();
							//dbg<<"\r\n"<<wynik1<<"\r\n";
							txt.Remove(start,len);
						    txt.insert(start,wynik1);
							//dbg<<txt<<"\r\n";
							tdone=true;
					}
				}
			//}

		if(tdone){//diall->spells.clear();
			diall->Text=txt;sdone=true;}
		if(sdone){
		//dbg<<diall.GetRaw()<<"\r\n";
			file->subs->ddials.push_back(diall);
			file->subs->dials[i]=diall;
		}
		else{
			delete diall;
		}
		}
	}
	//wxMessageBox(dbg);
		
	Refresh(false);
}

void Grid::OnMakeContinous(int idd)
{
	int fs=FirstSel();
	if(fs<0){return;}
	if(idd==MENU_CONT_PREV){
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
		:wxDialog(parent,-1,"Wybierz nowy FPS")
	{
		wxFlexGridSizer *sizer=new wxFlexGridSizer(2,2,2);
		wxArrayString fpsy;
		 wxTextValidator valid(wxFILTER_INCLUDE_CHAR_LIST);
		wxArrayString includes;
		includes.Add(_T("0"));
		includes.Add(_T("1"));
		includes.Add(_T("2"));
		includes.Add(_T("3"));
		includes.Add(_T("4"));
		includes.Add(_T("5"));
		includes.Add(_T("6"));
		includes.Add(_T("7"));
		includes.Add(_T("8"));
		includes.Add(_T("9"));
		includes.Add(_T("."));
		valid.SetIncludes(includes);

		fpsy.Add(_T("23.976"));fpsy.Add(_T("24"));fpsy.Add(_T("25"));fpsy.Add(_T("29.97"));fpsy.Add(_T("30"));fpsy.Add(_T("60"));
		oldfps=new wxComboBox(this,-1,"",wxDefaultPosition,wxDefaultSize,fpsy,0,valid);
		oldfps->SetSelection(0);
		newfps=new wxComboBox(this,-1,"",wxDefaultPosition,wxDefaultSize,fpsy,0,valid);
		newfps->SetSelection(2);
		sizer->Add(new wxStaticText(this,-1,"FPS napisów"),0,wxALIGN_CENTER_VERTICAL|wxALL,4);
		sizer->Add(oldfps,0,wxEXPAND|wxALL,4);
		sizer->Add(new wxStaticText(this,-1,"Nowy FPS napisów"),0,wxALIGN_CENTER_VERTICAL|wxALL,4);
		sizer->Add(newfps,0,wxEXPAND|wxALL,4);
		wxButton *ok=new wxButton(this,15555,"Zmieñ fps");
		Connect(15555,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsdial::OkClick);
		wxButton *cancel=new wxButton(this,wxID_CANCEL,"Anuluj");
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
		}else{wxMessageBox("Niew³aœciwy fps");}
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
	
