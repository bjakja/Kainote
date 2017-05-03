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
#include "Menu.h"
#include <wx/regex.h>
#include "KaiMessageBox.h"

Grid::Grid(wxWindow* parent, kainoteFrame* kfparent,wxWindowID id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
	: SubsGrid(parent, id, pos, size, style, name)
{
	Kai=kfparent;
	Bind(wxEVT_COMMAND_MENU_SELECTED,[=](wxCommandEvent &evt){
		int id = ((MenuItem*)evt.GetClientData())->id;
		if(id>5000 && id<5555){
			int id5000=(id-5000);
			if(visible & id5000){visible ^= id5000;}
			else{visible |= id5000;}
			SpellErrors.clear();
			Options.SetInt(GridHideCollums, visible);
			RepaintWindow();
		}
	},ID_CHECK_EVENT);
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
	Menu *menu=new Menu('N');
	Menu *hidemenu=new Menu('N');
	MenuItem *item;
	item = hidemenu->SetAccMenu(5000+LAYER,_("Ukryj warstwę"),_("Ukryj warstwę"),true, ITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & LAYER)!=0);
	hidemenu->SetAccMenu(5000+START,_("Ukryj czas początkowy"),_("Ukryj czas początkowy"),true, ITEM_CHECK)->Check((visible & START)!=0);
	item = hidemenu->SetAccMenu(5000+END,_("Ukryj czas końcowy"),_("Ukryj czas końcowy"),true, ITEM_CHECK);
	item->Enable(form!=TMP);
	item->Check((visible & END)!=0);
	item = hidemenu->SetAccMenu(5000+ACTOR,_("Ukryj aktora"),_("Ukryj aktora"),true, ITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & ACTOR)!=0);
	item = hidemenu->SetAccMenu(5000+STYLE,_("Ukryj styl"),_("Ukryj styl"),true, ITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & STYLE)!=0);
	item = hidemenu->SetAccMenu(5000+MARGINL,_("Ukryj lewy margines"),_("Ukryj lewy margines"),true, ITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & MARGINL)!=0);
	item = hidemenu->SetAccMenu(5000+MARGINR,_("Ukryj prawy margines"),_("Ukryj prawy margines"),true, ITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & MARGINR)!=0);
	item = hidemenu->SetAccMenu(5000+MARGINV,_("Ukryj pionowy margines"),_("Ukryj pionowy margines"),true, ITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & MARGINV)!=0);
	item = hidemenu->SetAccMenu(5000+EFFECT,_("Ukryj efekt"),_("Ukryj efekt"),true, ITEM_CHECK);
	item->Enable(form<SRT);
	item->Check((visible & EFFECT)!=0);
	hidemenu->SetAccMenu(5000+CNZ,_("Ukryj znaki na sekundę"),_("Ukryj znaki na sekundę"),true, ITEM_CHECK)->Check((visible & CNZ)!=0);

	bool isen;
	isen = (sels == 1);
	menu->SetAccMenu( InsertBefore,_("Wstaw &przed"))->Enable(isen);
	menu->SetAccMenu( InsertAfter,_("Wstaw p&o"))->Enable(isen);
	isen = (isen&&Kai->GetTab()->Video->GetState()!=None);
	menu->SetAccMenu( InsertBeforeVideo,_("Wstaw przed z &czasem wideo"))->Enable(isen);
	menu->SetAccMenu( InsertAfterVideo,_("Wstaw po z c&zasem wideo"))->Enable(isen);
	isen = (sels >0);
	menu->SetAccMenu( Duplicate,_("&Duplikuj linie"))->Enable(isen);
	isen = (sels == 2);
	menu->SetAccMenu( Swap,_("Za&mień"))->Enable(isen);
	isen = (sels >= 2&&sels <= 5);
	menu->SetAccMenu( Join,_("Złącz &linijki"))->Enable(isen);
	isen = (sels >= 2&&sels <= 50);
	menu->SetAccMenu( JoinToFirst,_("Złącz linijki zostaw pierwszą"))->Enable(isen);
	menu->SetAccMenu( JoinToLast,_("Złącz linijki zostaw ostatnią"))->Enable(isen);
	isen = (sels >0);
	menu->SetAccMenu( ContinousPrevious,_("Ustaw czasy jako ciągłe (poprzednia linijka)"))->Enable(isen);
	menu->SetAccMenu( ContinousNext,_("Ustaw czasy jako ciągłe (następna linijka)"))->Enable(isen);
	menu->SetAccMenu( Cut,_("Wytnij\tCtrl-X"))->Enable(isen);
	menu->SetAccMenu( Copy,_("Kopiuj\tCtrl-C"))->Enable(isen);
	menu->SetAccMenu( Paste,_("Wklej\tCtrl-V"));
	menu->SetAccMenu( CopyCollumns,_("Kopiuj kolumny"))->Enable(isen);
	menu->SetAccMenu( PasteCollumns,_("Wklej kolumny"));
	menu->Append(4444,_("Ukryj kolumny"),hidemenu);
	menu->SetAccMenu( NewFPS,_("Ustaw nowy FPS"));
	menu->SetAccMenu( FPSFromVideo,_("Ustaw FPS z wideo"))->Enable(Notebook::GetTab()->Video->GetState()!=None && sels==2);
	menu->SetAccMenu( PasteTranslation,_("Wklej tekst tłumaczenia"))->Enable(form<SRT && ((TabPanel*)GetParent())->SubsPath!="");
	menu->SetAccMenu( TranslationDialog,_("Okno przesuwania dialogów"))->Enable(showtl);
	menu->AppendSeparator();

	menu->SetAccMenu( RemoveText,_("Usuń tekst"))->Enable(isen);
	menu->SetAccMenu( Remove,_("Usuń"))->Enable(isen);
	menu->AppendSeparator();
	menu->SetAccMenu( FontCollectorID,_("Kolekcjoner czcionek"))->Enable(form<SRT);
	menu->SetAccMenu( SubsFromMKV,_("Wczytaj napisy z pliku MKV"))->Enable(Kai->GetTab()->VideoName.EndsWith(".mkv"));

	if(dummy){
		goto done;
	}
		
	int Modifiers=0;
	int id=menu->GetPopupMenuSelection(pos, this, &Modifiers);
	
	if(id<0){goto done;}
	
	if(Modifiers == wxMOD_SHIFT){
		if( id<=5000){goto done;}
		MenuItem *item=menu->FindItem(id);
		int ret=-1;
		wxString name=item->GetLabelText();
		ret=Hkeys.OnMapHkey(id, name, this, GRID_HOTKEY);
		if(ret!=-2){
			Hkeys.SetAccels();
			Hkeys.SaveHkeys();
		}
		goto done;
	}
	OnAccelerator(wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
done:
	delete menu;
	VB->blockpaint=false;
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
		if(ntext==""){ntext=dial->Text;}
		else if(dial->Text!=""){ntext<<en<<dial->Text;}
		if(ntltext==""){ntltext=dial->TextTl;}
		else if(dial->TextTl!=""){ntltext<<en<<dial->TextTl;}
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
		Stylelistbox slx(this,false,10,arr);
		if(slx.ShowModal()==wxID_OK)
		{
			rw=0;
			for (size_t v=0;v<slx.CheckListBox1->GetCount();v++)
			{

				if(slx.CheckListBox1->GetItem(v,0)->modified){
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
	wxString token;
	wxString tmptoken;
	while(wpaste.HasMoreTokens())
	{
		Dialogue *newdial=NULL;
		token = (tmptoken.empty())? wpaste.NextToken().Trim(false).Trim() : tmptoken;
		if(IsNum(token)){
			token.Empty();
			while(wpaste.HasMoreTokens()){
				tmptoken = wpaste.NextToken().Trim(false).Trim();
				if(IsNum(tmptoken)){break;}
				token += "\r\n" + tmptoken;
			}

		}
		newdial= new Dialogue(token);
		newdial->State=1;
		if(!newdial){continue;}
		if(newdial->Form!=form){newdial->Conv(form);}
		if(newdial->NonDial){newdial->NonDial=false; newdial->IsComment=false;}
		if(id==Paste){
			tmpdial.push_back(newdial);
			sel[rws]=true;
		}else{
			if(rws<(int)selarr.GetCount()/* && selarr[rws] < GetCount()*/){
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
		Stylelistbox slx(this,false,11,arr);
		if(slx.ShowModal()==wxID_OK)
		{
			for (size_t v=0;v<slx.CheckListBox1->GetCount();v++)
			{

				if(slx.CheckListBox1->GetItem(v,0)->modified){
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
	int sels=selarr.GetCount();
	bool hasVideo = vb->GetState() != None;
	switch(id){
		case PlayPause: if(vb->IsShown()){vb->Pause();} break;
		case Plus5Second: vb->Seek(vb->Tell()+5000); break;
		case Minus5Second: vb->Seek(vb->Tell()-5000); break;
		case InsertBeforeVideo: if(sels>0 && hasVideo) OnInsertBeforeVideo(); break;
		case InsertAfterVideo: if(sels>0 && hasVideo) OnInsertAfterVideo(); break;
		case InsertBefore: if(sels>0) OnInsertBefore(); break;
		case InsertAfter: if(sels>0) OnInsertAfter(); break;
		case Duplicate: if(sels>0) OnDuplicate(); break;
		case Copy: 
		case CopyCollumns: if(sels>0) CopyRows(id); break;
		case Cut: if(sels>0) CopyRows(id);DeleteRows(); break;
		case Paste: 
		case PasteCollumns: if(sels>0) OnPaste(id); break;
		case Remove: if(sels>0) DeleteRows(); break;
		case RemoveText: if(sels>0) DeleteText(); break;
		case ContinousPrevious: 
		case ContinousNext: if(sels>0) OnMakeContinous(id); break;
		case Swap: if(sels==2){SwapRows(selarr[0],selarr[1],true);} break;
		case FPSFromVideo: if( hasVideo && sels==2){OnSetFPSFromVideo();} break;
		case Join: if(sels>1){OnJoin(event);} break;
		case JoinToFirst:
		case JoinToLast: if(sels>1){OnJoinToFirst(id);} break;
		case PasteTranslation: if(form<SRT && ((TabPanel*)GetParent())->SubsPath!=""){OnPasteTextTl();} break;
		case SubsFromMKV: if( Kai->GetTab()->VideoName.EndsWith(".mkv")){OnMkvSubs(event);} break;
		case NewFPS: OnSetNewFPS(); break;
		default:
			break;
	}
	
	if(id==TranslationDialog && showtl){
		static TLDialog *tld= new TLDialog(this,this);
		tld->Show();
	}
	else if(id>6000){
		Kai->OnMenuSelected(event);
	}
	/*else if(id>5000 && id<5555){
		int id5000=(id-5000);
		if(visible & id5000){visible ^= id5000;}
		else{visible |= id5000;}
		SpellErrors.clear();
		Options.SetInt("Grid Hide Collums", visible);
		RepaintWindow();
	}*/
}


void Grid::OnPasteTextTl()
{
	wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik napisów"), Kai->GetTab()->SubsPath.BeforeLast('\\'), "", _("Pliki napisów (*.ass),(*.srt),(*.sub),(*.txt)|*.ass;*.srt;*.sub;*.txt"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "wxFileDialog");
	if (FileDialog1->ShowModal() == wxID_OK){
		OpenWrite op;
		wxString pathh=FileDialog1->GetPath();
		wxString txt;
		if(!op.FileOpen(pathh, &txt)){return;}
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
	
	if(selecs.GetCount()<1||!showtl||!transl)return;
	int first=selecs[0];
	int mrow=1;
	if(selecs.GetCount()>1){
		mrow=selecs[1]-first;
	}
	
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
				if(mode==1){
					wxString mid=(GetDial(first)->TextTl!="" && GetDial(i+1)->TextTl!="")?"\\N":"";
					CopyDial(first)->TextTl << mid << GetDial(i+1)->TextTl;
					if(i!=first){CopyDial(i)->TextTl = GetDial(i+mrow)->TextTl;}
				}else if(i+mrow<GetCount()){
					CopyDial(i)->TextTl = GetDial(i+mrow)->TextTl;
				}
			}
			else if(i<GetCount()-mrow){
				CopyDial(i)->TextTl = GetDial(i+mrow)->TextTl;}
			else if(GetDial(i)->Text!=""){/*wxLogStatus("onlytl mrow--");*/mrow--;}

		}
		
		if(mrow>0){
			DeleteRow(GetCount()-mrow, mrow);
		}

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
		int wbutton=KaiMessageBox(_("Zapisać plik przed wczytaniem napisów z MKV?"), 
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
	if(!mw.Open(mkvpath,false)){return;}
	int isgood = (int)mw.GetSubtitles(this);
	mw.Close();
	

	if(isgood){
		if(transl){Edit->SetTl(false); transl=false;showtl=false;Kai->Menubar->Enable(SaveTranslation,false);}
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
			if(!isgood){KaiMessageBox(_("Otwieranie napisów nie powiodło się"), _("Uwaga"));}
			if(Kai->GetTab()->Video->GetState()==Paused){Kai->GetTab()->Video->Render();}
		}

		if(!Kai->GetTab()->edytor&&!Kai->GetTab()->Video->isFullscreen){Kai->HideEditor();}
		Kai->GetTab()->CTime->Contents();
		sel[Edit->ebrow]=true;
		RepaintWindow();
		Edit->HideControls();
		
	}
	if(StyleStore::HasStore() && form==ASS){StyleStore::Get()->LoadAssStyles();}
}



void Grid::ResizeSubs(float xnsize, float ynsize)
{
	float val=xnsize;
	int resizeScale = 0;
	if(ynsize!=xnsize){
		if(ynsize>xnsize){
			resizeScale = 2;
		}else{
			val = ynsize;
			resizeScale = 1;
		}
	}


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
		if(resizeScale==1){
			double fscx=100;
			resized->ScaleX.ToCDouble(&fscx);
			fscx*=xnsize;
			resized->ScaleX=getfloat(fscx);
		}else if(resizeScale==2){
			double fscy=100;
			resized->ScaleY.ToCDouble(&fscy);
			fscy*=ynsize;
			resized->ScaleY=getfloat(fscy);
		}
		double fs=0;
		resized->Fontsize.ToCDouble(&fs);
		fs*=val;
		resized->Fontsize=getfloat(fs);
		float ol=wxAtoi(resized->Outline);
		ol*=val;
		resized->Outline=getfloat(ol);
		float sh=wxAtoi(resized->Shadow);
		sh*=val;
		resized->Shadow=getfloat(sh);
		double fsp=0;
		resized->Spacing.ToCDouble(&fsp);
		fsp*=val;
		resized->Spacing=getfloat(fsp);
	}

	//dialogi, największy hardkor, wszystkie tagi zależne od rozdzielczości trzeba zmienić
	//tu zacznie się potęga szukaczki tagów
	wxRegEx onenum("\\\\(fax|fay|fs|bord|shad|pos|move|iclip|clip|org)([^\\\\}\\)]*)",wxRE_ADVANCED);
	wxRegEx drawing("\\\\p([0-9]+)[\\\\}\\)]",wxRE_ADVANCED);

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
							double vlt=0.0;
							
							//clip rysunkowy
							if (crm!=-1||crn!=-1)
							{
								wxStringTokenizer tknzr(crop," ",wxTOKEN_STRTOK);
								while(tknzr.HasMoreTokens())
								{
									wxString tkn=tknzr.NextToken();
									if(tkn!="m" && tkn!="l" && tkn !="b" && tkn != "s" && tkn != "c")
									{
										wxString lastC;
										if(tkn.EndsWith("c")){
											tkn.RemoveLast(1);
											lastC="c";
										}
										if(tkn.ToCDouble(&vlt)){
											vlt*=(ii%2==0)?xnsize:ynsize;
											wynik<<getfloat(vlt)<<lastC<<" ";
										}else{
											wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw wycinku wektorowym"), i+1, tkn);
											wynik<<tkn<<lastC<<" ";
										}
										
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
									//double vlt;

									if(ii<4 && tkn.ToCDouble(&vlt))
									{
										vlt*=(ii%2==0)?xnsize:ynsize;
										wynik<<getfloat(vlt)<<",";
										ii++;
									}
									else
									{
										if(ii<4){
											wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw tagu '\\%s'"), i+1, tkn, onenum.GetMatch(subtxt,1));
										}
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
						}//pozostałe tagi nie zaczynające się od nawiasu
						else if(crop.ToCDouble(&valtag))
						{
							valtag*=val;
							
							txt.Remove(pos+start,len);
							
							txt.insert(pos+start,getfloat((float)valtag));
							
							tdone=true;
						}else{
							bool startWithCX = crop.StartsWith("cx");
							bool startWithCY = crop.StartsWith("cy");
							bool startWithP = crop.StartsWith("p");
							if(!startWithCX && !startWithCY && !startWithP){
								wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw tagu '%s'"), i+1, crop, onenum.GetMatch(subtxt,1));
							}else if((resizeScale==1 && startWithCX) || (resizeScale==2 && startWithCY) || startWithP){
								float value = (resizeScale==1)? xnsize : (resizeScale==2)? ynsize : val;
								int restTagChars = (startWithP)? 1 : 2;
								wxString tag = "fs"+crop.Left(restTagChars);
								wxString valTxt = crop.Mid(restTagChars);
								if(valTxt.ToCDouble(&valtag))
								{
									valtag*=value;
									txt.Remove(pos+start+restTagChars,len-restTagChars);
									txt.insert(pos+start+restTagChars,getfloat((float)valtag));
									tdone=true;
								}else{
									wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw tagu '%s'"), i+1, valTxt, tag);
								}
							}
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
						if(brpos+1>=txt.Len()){draw="";}
						else{draw=txt.Mid(brpos+1);
							start=brpos+1;}
						}
					else{draw=txt.Mid(start+len-1);}
					int brpos1=draw.Find('{');
					if(brpos1!=-1){draw=draw.Mid(0,brpos1-1);len=brpos1-1;}else{len=draw.Len();}
					//dbg<<draw<<"\r\n";
					wxStringTokenizer tknzr(draw," ",wxTOKEN_STRTOK);
					wxString wynik1;
					int ii=0;
					double vlt=0;
					
					while(tknzr.HasMoreTokens())
					{
						wxString tkn=tknzr.NextToken();
						if(tkn!="m" && tkn!="l" && tkn !="b" && tkn != "s" && tkn != "c")
						{
							wxString lastC;
							if(tkn.EndsWith("c")){
								tkn.RemoveLast(1);
								lastC="c";
							}
							if(tkn.ToCDouble(&vlt)){
								vlt*=(ii%2==0)?xnsize:ynsize;
								wynik1<<getfloat(vlt)<<lastC<<" ";
							}else{
								wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw rysunku wektorowym"), i+1, tkn);
								wynik1<<tkn<<lastC<<" ";
							}
										
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
				if(SpellErrors.size() >= (size_t)i) SpellErrors[i].clear();
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
	wxArrayInt sels=GetSels();
	if(sels.size()<0){wxBell();return;}
	if(idd==ContinousPrevious){
		
		/*int diff=GetDial(fs)->End.mstime - GetDial(fs-1)->Start.mstime;*/
		for(size_t i=0; i < sels.size(); i++)
		{
			if(sels[i]<1){continue;}
			CopyDial(sels[i])->Start = GetDial(sels[i]-1)->End;
		}
	}
	else
	{
		int dialsize = GetCount()-1;
		for(size_t i=0; i < sels.size(); i++)
		{
			if(sels[i]>=dialsize){continue;}
			CopyDial(sels[i])->End = GetDial(sels[i]+1)->Start;
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
	int firstTime=first->Start.mstime;
	int secondTime=second->Start.mstime;
	int videoTime=Notebook::GetTab()->Video->Tell();
	float diffVideo = (videoTime-secondTime);
	float diffLines = (secondTime-firstTime);

	for (int i=0;i<GetCount();i++){
		Dialogue *dialc=CopyDial(i);
		dialc->Start.Change(diffVideo *((dialc->Start.mstime - firstTime) / diffLines));
		dialc->End.Change(diffVideo *((dialc->End.mstime - firstTime) / diffLines));
	}
	SetModified();
	if(form>TMP){RepaintWindow(START|END);}else{Refresh(false);}
}

class fpsdial : public KaiDialog
{
public:
	fpsdial(wxWindow *parent)
		:KaiDialog(parent,-1,_("Wybierz nowy FPS"))
	{
		DialogSizer* siz = new DialogSizer(wxHORIZONTAL);
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
		oldfps=new KaiChoice(this,-1,"",wxDefaultPosition,wxDefaultSize,fpsy,0,valid);
		oldfps->SetSelection(0);
		newfps=new KaiChoice(this,-1,"",wxDefaultPosition,wxSize(80,-1),fpsy,0,valid);
		newfps->SetSelection(2);
		sizer->Add(new wxStaticText(this,-1,_("FPS napisów")),0,wxALIGN_CENTER_VERTICAL|wxALL,4);
		sizer->Add(oldfps,0,wxEXPAND|wxALL,4);
		sizer->Add(new wxStaticText(this,-1,_("Nowy FPS napisów")),0,wxALIGN_CENTER_VERTICAL|wxALL,4);
		sizer->Add(newfps,0,wxEXPAND|wxALL,4);
		MappedButton *ok=new MappedButton(this,15555,_("Zmień fps"));
		Connect(15555,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsdial::OkClick);
		MappedButton *cancel=new MappedButton(this,wxID_CANCEL,_("Anuluj"));
		sizer->Add(ok,0,wxEXPAND|wxALL,4);
		sizer->Add(cancel,0,wxEXPAND|wxALL,4);
		siz->Add(sizer, 0, wxEXPAND);
		SetSizerAndFit(siz);
		CenterOnParent();
	}
	virtual ~fpsdial(){};
	void OkClick(wxCommandEvent &evt)
	{

		if(oldfps->GetValue().ToDouble(&ofps) && newfps->GetValue().ToDouble(&nfps)){
			EndModal(1);
		}else{KaiMessageBox(_("Niewłaściwy fps"));}
	}
	double ofps,nfps;
	KaiChoice *oldfps;
	KaiChoice *newfps;
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