#include "TextEditor.h"
#include "EditBox.h"
#include "SubsGrid.h"
#include "kainoteMain.h"
#include "Hotkeys.h"
#include <wx/clipbrd.h>
#include <wx/utils.h>

listwindow::listwindow(wxWindow *parent, wxArrayString suggest, const wxPoint& pos, wxString name)
	: wxDialog(parent,-1,name,pos)
	{
	wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
	disperrs=new wxListBox(this,99887,wxDefaultPosition,wxDefaultSize,suggest);
	Connect(99887,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&listwindow::OnDoubleClick);
	sizer->Add(disperrs,1,wxEXPAND|wxALL,2);
	sizer->SetMinSize(100,100);
	SetSizerAndFit(sizer);
	}
listwindow::~listwindow()
	{
	}

void listwindow::OnDoubleClick(wxCommandEvent& event)
	{
	EndModal(wxID_OK);
	}


TextEditor::TextEditor(wxWindow *parent, const long int id, bool spellchecker, const wxPoint& pos,const wxSize& size, long style, const wxString& name)
	: wxTextCtrl(parent,id,"",pos,size,style,wxDefaultValidator,name)
	{
	spell=(spellchecker&&Options.GetBool("Editbox Spellchecker"));
	block=modified=false;
	Kai=NULL;
	fsize=10;
	SetCursor(wxCURSOR_IBEAM);
	spblock.SetOwner(this,77777);
	//wxAcceleratorEntry entries[1];
    //entries[19]=Hkeys.GetHKey("Globalny usuñ linijkê",ID_DELETE);
   // wxAcceleratorTable accel(1, entries);
   // SetAcceleratorTable(accel);
	 
	Connect(wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TextEditor::OnWrite);
	Connect(wxEVT_KEY_UP,(wxObjectEventFunction)&TextEditor::OnKeyUp);
	Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&TextEditor::OnKeyPress);
	}

TextEditor::~TextEditor()
	{
	
	}
void TextEditor::OnKeyUp(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if(key==WXK_DELETE&&event.ShiftDown()){Kai->GetTab()->Grid1->DeleteRows();}
}

void TextEditor::OnKeyPress(wxKeyEvent& event)
	{
	int key = event.GetKeyCode();
	
	if(key=='V'&&event.m_controlDown){

		DoPaste1();
		
	}
	else if(key!=WXK_RETURN&&key!=WXK_NUMPAD_ENTER){event.Skip();}else{EB->Send();return;}
	if(key==WXK_DOWN&&(event.ShiftDown()||event.ControlDown())){wxString txt=GetValue();int txtlen=txt.Len();long endsel = txtlen, startsel=txtlen;if(event.ShiftDown()){GetSelection(&startsel,&endsel);} SetSelection(endsel,txtlen);}
	if(key==WXK_UP&&(event.ShiftDown()||event.ControlDown())){wxString txt=GetValue();long endsel = 0, startsel=0;if(event.ShiftDown()){GetSelection(&startsel,&endsel);} SetSelection(0,endsel);}
	if (key == WXK_MENU || key == WXK_WINDOWS_MENU) {
		int w,h;
		long st,ed;
		GetClientSize(&w,&h);
		GetSelection(&st,&ed);
		wxPoint mpos=PositionToCoords(st);
		mpos.y+=20;
		for(size_t i=0;i<EB->line.spells.size();i+=2)
			{
				if(st>=EB->line.spells[i]&&ed<=EB->line.spells[i+1]){
					
					ContextMenu(mpos,i);
					return;
					}
			}
		ContextMenu(mpos);
	}
	if((event.GetUnicodeKey()||key==WXK_DELETE) && !event.m_controlDown && !event.m_altDown){modified=true;}
	}

void TextEditor::SetTextS(wxString text, bool modif)
	{
	block=true;
	wxFont boldfont(fsize,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
	SetValue(text);
	
	bool tagi=false;
	bool slash=false;
	bool val=false;
	wxString znaki="(0123456789-&";
	wxString cyfry="-0123456789ABCDEFabcdef.";
	wxString tagtest="";
	int strt=0;
	for(size_t i=0;i<text.Len();i++)
		{
		wxUniChar ch=text.GetChar(i);
		if(ch=='{'||ch=='}'){if(ch=='{'){tagi=true;}else{tagi=false;}
		SetStyle(i,i+1,wxTextAttr(*wxBLUE));}
		if(slash){tagtest+=ch;if((znaki.Find(ch)!=-1&&tagtest!="1"&&tagtest!="2"&&tagtest!="3"&&tagtest!="4")||tagtest=="fn"||ch=='('){slash=false;
			SetStyle(strt,(tagtest=="fn")?i+1:i,wxTextAttr(*wxBLACK,wxNullColour,boldfont));
			tagtest="";
			if(tagtest!="fn"){val=true;strt=i;}
			}}
		if(val){if(cyfry.Find(ch)==-1&&strt!=i){SetStyle(strt,i,wxTextAttr(wxColour("#6600FF")));}if(ch=='\\'||!tagi){val=false;}}
		if((ch=='\\'||ch=='('||ch==')'||ch==',')&&tagi){if(ch=='\\'){slash=true;}strt=i+1;
			SetStyle(i,i+1,wxTextAttr(wxColour("#FF0000")));}
		}
	
	if(text!=""&&spell){CheckText(false);}
	modified=modif;
	block=false;
	}


void TextEditor::OnWrite(wxCommandEvent& event)
	{
		
		if(block){return;}
	wxString text=GetValue();
wxString cyfry="0123456789ABCDEF.";	
		long from, to;
GetSelection(&from,&to);
from-=1;
wxUniChar ch=text.GetChar(from);
wxUniChar chw;
int klamras=text.SubString(0,from+1).Find('{',true);
int klamrae=text.SubString(0,(from-1<1)?1:(from-2)).Find('}',true);
wxFont DefFont(fsize,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);


if(klamras==-1||(klamras<klamrae&&klamrae!=-1)){
	}else{
if(from>0){chw=text.GetChar(from-1);}else{chw='~';}
if((ch=='\\'||ch=='('||ch==')'||ch==',')){
			SetStyle(from,from+1,wxTextAttr(wxColour("#FF0000"),wxNullColour,DefFont));}
if(ch=='{'||ch=='}'){
		SetStyle(from,from+1,wxTextAttr(*wxBLUE,wxNullColour,DefFont));}
if(chw=='\\'){wxFont thisFont(fsize,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
SetStyle(from,from+1,wxTextAttr(*wxBLACK,wxNullColour,thisFont));}
if(cyfry.Find(ch)!=-1&&chw!='\\'){SetStyle(from,from+1,wxTextAttr(wxColour("#6600FF"),wxNullColour,DefFont));}
if(chw=='}'||text.GetChar(from+1)=='{'||chw=='{'&&ch!='\\'){SetStyle(from,from+1,wxTextAttr(*wxBLACK,wxNullColour,DefFont));}
	}
if(spell){if(ch==' '){CheckText();return;} else {spblock.Start(1000);}}

	//EB->Send(false, true);
//modified=true;//wxLogStatus("modif = true");
if(!(EB->TextEditTl->HasFocus()&&EB->TextEdit->GetValue()!="")){ EB->UpdateChars(text);}
	}




void TextEditor::CheckText(bool rpnt)
	{
	if(!Kai->SC){spell=Kai->SpellcheckerOn();}
	if(Kai->SC){
		//bool modif=IsModified();
		wxString notchar="/?<>|\\!@#$%^&*()_+=[]\t~ :;.,\"{}";
		wxString text=GetValue();
		if(rpnt){SetStyle(0,text.Len(),wxTextAttr(wxNullColour,wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));}
		errs.clear();
		EB->line.spells.Clear();
		text+=_T(" ");

		bool block=false;
		wxString word="";
		bool slash=false;
		int lasti=0;
		int firsti=0;
		for(size_t i = 0; i<text.Len();i++)
			{
			wxUniChar ch=text.GetChar(i);
			if(notchar.Find(ch)!=-1&&!block){if(word.Len()>1){word.Trim(false);word.Trim(true);bool isgood=Kai->SC->CheckWord(word);
			if (!isgood){SetStyle(firsti,lasti+1,wxTextAttr(wxNullColour,Options.GetColour("Grid Spellchecker")));
			errs.Add(word);EB->line.spells.Add(firsti);EB->line.spells.Add(lasti+1);}
			
			}word="";firsti=i+1;}
			if(ch=='{'){block=true;}
			else if(ch=='}'){block=false;firsti=i+1;word="";}

			
			if(notchar.Find(ch)==-1&&text.GetChar(MAX(0,i-1))!='\\'&&!block){word<<ch;lasti=i;}
			else if(!block&&text.GetChar(MAX(0,i-1))=='\\'){firsti=i+1;word="";}
			}
		//if(rpnt){EB->Send(false);}
		//SetModified(modif);
		}
	}

void TextEditor::OnStartSpell(wxTimerEvent& event)
	{
	CheckText();
	spblock.Stop();
	}

void TextEditor::OnMouseEvent(wxMouseEvent &event) {

	wxPoint mpos(event.GetX(),event.GetY());

	if((event.LeftDClick() && Options.GetBool("Editbox Sugestions On Dclick") ) || event.RightUp()){
		long pos;
		bool isright=false;
		
		wxTextCtrlHitTestResult res=HitTest (mpos, &pos);
		if(res==wxTE_HT_ON_TEXT){
			wxTextAttr tstyle;
			GetStyle(pos,tstyle);
			if(tstyle.GetBackgroundColour()==Options.GetColour("Grid Spellchecker")){
				for(size_t i=0;i<EB->line.spells.size();i+=2)
					{
					if(pos>=EB->line.spells[i]&&pos<=EB->line.spells[i+1])
						{
						if(event.RightUp())
							{
							isright=true;
							ContextMenu(mpos, i);
							}
						else{
						wxPoint mst=wxGetMousePosition();
						mst.x-=50;mst.y+=15;
						OnSpellErrs(i,mst);
							}
						break;
						}
					}
				}else if(!event.RightUp()){event.Skip();}
			
			}else if(!event.RightUp()){event.Skip();}

			if(!isright && event.RightUp()){
				ContextMenu(mpos);
				}
		}else{event.Skip();}

		if (event.GetWheelRotation() != 0 && event.ControlDown()) {
			fsize += event.GetWheelRotation() / event.GetWheelDelta();
			if(fsize<7||fsize>70){return;}
			wxFont normfont(fsize,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
			SetFont(normfont);
			SetTextS(GetValue());
		return;}
}

void TextEditor::OnSpellErrs(int elem, wxPoint mpos)
	{
	
	wxString err=errs[elem/2];
	
	wxArrayString suggs= Kai->SC->Suggestions(err);
	
	listwindow lw(NULL,suggs,mpos);
	if(lw.ShowModal()==wxID_OK)
		{
		wxString Text=GetValue();
		int from=EB->line.spells[elem];
		int to=EB->line.spells[elem+1];
		Text.replace(from,to-from,lw.disperrs->GetString(lw.disperrs->GetSelection()));
		SetTextS(Text,true);
		SetSelection(to,to);
		EB->Send(false);
		modified=false;
		//wxLogStatus("modif = false");
		}
	
}


void TextEditor::DoPaste1()
	{
	//SetFocus();
	if (wxTheClipboard->Open())
			{
			if (wxTheClipboard->IsSupported( wxDF_TEXT ))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData( data );
				wxString whatpaste = data.GetText();
				whatpaste.Replace("\n"," ");
				whatpaste.Replace("\r","");
				whatpaste.Replace("\f","");
				wxString txt=GetValue();
				long strt, ennd;
				GetSelection(&strt,&ennd);
				if(strt!=ennd){txt.Remove(strt,ennd-strt);}
				txt.insert(strt,whatpaste);
				SetTextS(txt,true);
				long whre=strt+whatpaste.Len();
				SetSelection(whre,whre);
			}
        wxTheClipboard->Close();
		}
	}


void TextEditor::ContextMenu(wxPoint mpos, int error)
	{
	wxMenu menut;
	wxString err;
	wxArrayString suggs;
	if(error>=0){err=errs[error/2];}
	if(!err.IsEmpty()){
	suggs= Kai->SC->Suggestions(err);
	for(size_t i=0; i<suggs.size(); i++)
		{
		menut.Append(i+30200,suggs[i]);
		}

	if(suggs.size()>0){menut.AppendSeparator();}
		}

			

			menut.Append(TEXTM_COPY,_("&Kopiuj"))->Enable(CanCopy());
			menut.Append(TEXTM_CUT,_("Wy&tnij"))->Enable(CanCut());
			menut.Append(TEXTM_PASTE,_("&Wklej"))->Enable(CanPaste());
			
			menut.AppendSeparator();
			if(!err.IsEmpty()){
				addw=err;
				menut.Append(TEXTM_ADD,_("&Dodaj s³owo \"")+err+_("\" do s³ownika"));}
						
						

			long from,to;
			GetSelection(&from,&to);
			menut.Append(TEXTM_DEL,_("&Usuñ"))->Enable(from!=to);
			
			int id=-1;
			
			id=GetPopupMenuSelectionFromUser(menut,mpos);
			
			if(id>=30200){
		wxString Text=GetValue();
		int from=EB->line.spells[error];
		int to=EB->line.spells[error+1];
		Text.replace(from,to-from,suggs[id-30200]);
		SetTextS(Text,true);
		SetSelection(to,to);
		EB->Send(false);
				}
			else if(id==TEXTM_COPY){
				Copy();}
			else if(id==TEXTM_CUT){
				Cut();modified=true;}
			else if(id==TEXTM_PASTE){
				DoPaste1();}
			else if(id==TEXTM_DEL){
				long from, to;
				GetSelection(&from,&to);
				Remove(from,to);modified=true;}
			else if(id==TEXTM_ADD){
				bool succ=Kai->SC->AddWord(addw);
				if(!succ){wxMessageBox("B³¹d, s³owo nie zosta³o dodane.");}else{CheckText();EB->ClearErrs();}
				}
			

	}

bool TextEditor::Modified()
{
return modified;
}



BEGIN_EVENT_TABLE(TextEditor,wxTextCtrl)
EVT_TIMER(77777, TextEditor::OnStartSpell)
EVT_MOUSE_EVENTS(TextEditor::OnMouseEvent)
END_EVENT_TABLE()