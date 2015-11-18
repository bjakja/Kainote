
#include "AutomationScriptsDialog.h"
#include "OptionsDialog.h"
#include "Hotkeys.h"
#include "kainoteMain.h"
#include "DropFiles.h"
#include <wx/filedlg.h>


MacrosDialog::MacrosDialog(wxWindow *parent, int _script)
	: wxDialog(parent,-1,"", wxDefaultPosition, wxSize(320,100),0)
{
	script=_script;
	names.Add(_("Nazwa makra"));
	names.Add(_("Skrót"));
	actives.push_back(true);
	seld=-1;
	block=false;
	wxFont font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
	int fw,fh;
	GetTextExtent("TEKST",&fw,&fh,0,0,&font);
	fh+=4;


	ScriptsDialog *SD=(ScriptsDialog*)parent;
	kainoteFrame *Kai=(kainoteFrame*)GetGrandParent();
	std::vector<Auto::LuaMacro *> macros=Kai->Auto->Scripts[script]->Macros;
	wxArrayInt sels=Kai->GetTab()->Grid1->GetSels();
	height=(macros.size()+1)*fh;
	SetClientSize(320,height);
	wxPoint mst=wxGetMousePosition();
	mst.x+=1;
	mst.y-=(height/2);
	SetPosition(mst);
		//specjalna ró¿nica miêdzy pierwsz¹ zaznaczon¹ linijk¹ która jest zero a linijk¹ w lua która jest wiêksza o size sinfo + size styles
		int diff=(Kai->GetTab()->Grid1->file->subs->sinfo.size()+Kai->GetTab()->Grid1->file->subs->styles.size()+1);
	for(size_t i=0; i<macros.size(); i++)
	{
		names.Add(macros[i]->name);
		wxString hkeyname;
		std::map<int,wxString>::iterator it=Hkeys.hkeys.find(30100+i);
		names.Add((it!=Hkeys.hkeys.end())?it->second.AfterLast('=') : "Brak");
		
		if(!macros[i]->Validate(sels,Kai->GetTab()->Edit->ebrow, diff)){
			actives.push_back(false);
		}else{actives.push_back(true);}
	}
	CaptureMouse();
}

MacrosDialog::~MacrosDialog()
	{
	names.Clear();
	actives.clear();
	if(HasCapture()){ReleaseMouse();}
	}

void MacrosDialog::OnPaint(wxPaintEvent &event)
	{
	wxPaintDC dc(this);
	int w,h,fw,fh;
	GetClientSize(&w,&h);
	dc.Clear();
	dc.SetFont(wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT));
	dc.GetTextExtent(names[0],&fw,&fh);
	dc.SetBrush(wxBrush("#BBBBBB"));
	dc.SetPen(*wxTRANSPARENT_PEN);

	dc.DrawRectangle(0,0,w,fh+4);
	dc.SetPen(wxPen("#555555"));
	dc.DrawLine(0,fh+3,w,fh+3);
	
	int y=0;
	for(size_t i=0; i<names.size(); i+=2)
	{
		if(i/2==seld)
		{
			dc.SetBrush(wxBrush("#65ADED"));
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.DrawRectangle(0,y,w,fh+4);
		}
		if(actives[i/2]){dc.SetTextForeground(wxColour("#000000"));}
		else{dc.SetTextForeground(wxColour("#999999"));}
		dc.SetBrush(wxBrush("#FFFFFF"));
		//dc.DrawText(names[i],5,y);
		//dc.DrawText(names[i+1],205,y);
		wxRect cur(5,y,220,fh+4);	
		dc.SetClippingRegion(cur);
		dc.DrawLabel(names[i],cur,wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
		dc.DestroyClippingRegion();
		cur=wxRect(235,y,w-237,fh+4);	
		dc.SetClippingRegion(cur);
		dc.DrawLabel(names[i+1],cur,wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
		dc.DestroyClippingRegion();
		y+=fh+4;
	}
	dc.SetPen(wxPen("#555555"));
	dc.DrawLine(230,0,230,h);
}

void MacrosDialog::OnMacro()
	{
	//int x=0;
	wxPoint mst=wxGetMousePosition();
	wxPoint mpos=ScreenToClient(mst);
	int col=(mpos.x<230)?0 : 1;
	
	ScriptsDialog *SD=(ScriptsDialog*)GetParent();
	kainoteFrame *Kai=(kainoteFrame*)GetGrandParent();
	block=true;
	if(HasCapture()){ReleaseMouse();}
	if(col){//kolumna 1

		HkeysDialog hkd(this,names[seld*2],true);
		//tu trzeba jeszcze okno wyboru skrótu klawiszowego
		
		if(hkd.ShowModal()==0){
	
			wxString test;
			if(hkd.flag & 1){
				test<<"Alt-";}
			if(hkd.flag & 2){
				test<<"Ctrl-";}
			if(hkd.flag & 4){
				test<<"Shift-";}
    
			test<<wchar_t(hkd.hkey);
	
			//wxLogStatus(test);
			for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++)
			{//wxLogStatus(cur->first);
				if(cur->second.EndsWith(test)){
					wxMessageBox("Ten skrót ju¿ istnieje i jest ustawiony jako skrót do \""+cur->second.BeforeLast('=')+"\", wybierz inny.", "Uwaga",wxOK);
					return;
				}
			}
			//wxLogStatus("Setitem");
			names[(seld*2)+1]=test;
			//wxLogStatus("Setmodif");
			wxString hkeyname;
			hkeyname<<"Script "<<script<<"-"<<seld-1;
			bool strt=true;
			int lastid=30099;
			int curid=-1;
			for(auto cur=Hkeys.hkeys.rbegin(); cur!=Hkeys.hkeys.rend(); cur++){
				int id=cur->first;
				if(id>=30100){
					if(strt){lastid=id;strt=false;}
					if(cur->second.BeforeLast('=') == hkeyname){
						curid=id; break;
					}
				}else{break;}

			}

			Hkeys.SetHKey((curid>-1)? curid : lastid+1,hkeyname, hkd.flag, hkd.hkey);
			Hkeys.SaveHkeys();
			Kai->SetAccels();
			Refresh(false);
		}
		CaptureMouse();
	}
	else//kolumna 0
	{
		TabPanel *pan=Kai->GetTab();
		
		wxArrayInt sels=pan->Grid1->GetSels(true);

		
		//specjalna ró¿nica miêdzy pierwsz¹ zaznaczon¹ linijk¹ która jest zero a linijk¹ w lua która jest wiêksza o size sinfo + size styles
		int diff=(pan->Grid1->file->subs->sinfo.size()+pan->Grid1->file->subs->styles.size()+1);
		Kai->Auto->Scripts[script]->Macros[seld-1]->Process(sels,pan->Edit->ebrow, diff, Kai);
		for(size_t i=0; i<sels.size(); i++){
			pan->Grid1->sel[sels[i]]=true;
		}
		
		
		
		//Kai->Tabs->RefreshBar();
		//if(HasCapture()){ReleaseMouse();}
		EndModal(5);
		
	}
	block=false;
}

void MacrosDialog::OnMouseEvents(wxMouseEvent &event)
{
	//if(!HasCapture()&&!block){CaptureMouse();}
	int x=event.GetX(), y=event.GetY();
	//wxLogStatus("coords %i, %i", x, y);
	if((x<0||x>320||y<0||y>height)&&event.ButtonDown())
	{
		if(HasCapture()){ReleaseMouse();}
		EndModal(0);
		return;
	}

	if(event.LeftDown()||event.LeftDClick()){
		wxFont font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
		int fw,fh;
		GetTextExtent("TEKST",&fw,&fh,0,0,&font);
		int divy=y/(fh+4);
		if(divy<(int)actives.size() && actives[divy] && divy!=0){seld=divy;
			Refresh(false);
			if(event.LeftDClick()){
				OnMacro();
			}
		}
	}
}



ScriptsDialog::ScriptsDialog(kainoteFrame *_Kai)
	: wxDialog(_Kai,-1,"Mened¿er Skryptów Lua")
{
	
	Kai=_Kai;
	wxBoxSizer *main= new wxBoxSizer(wxVERTICAL);
	ScriptsList=new wxListCtrl(this, ID_SLIST, wxDefaultPosition, wxDefaultSize,wxLC_REPORT | wxLC_SINGLE_SEL);
	ScriptsList->InsertColumn(0,_("Lp"),wxLIST_FORMAT_LEFT,25);
	ScriptsList->InsertColumn(1,_("Nazwa"),wxLIST_FORMAT_LEFT,200);
	ScriptsList->InsertColumn(2,_("Nazwa pliku"),wxLIST_FORMAT_LEFT,180);
	ScriptsList->InsertColumn(3,_("Opis"),wxLIST_FORMAT_LEFT,600);

	//wxLogStatus("przed Kai->Auto");
	if(!Kai->Auto){Kai->Auto= new Auto::Automation(Kai);}
	//wxLogStatus("po Kai->Auto");
	for(size_t i=0; i<Kai->Auto->Scripts.size(); i++)
		{
		long pos = ScriptsList->InsertItem(i,wxString::Format("%i",(int)(i+1)));
		ScriptsList->SetItem(pos,1,Kai->Auto->Scripts[i]->name);
		ScriptsList->SetItem(pos,2,Kai->Auto->Scripts[i]->filename.AfterLast('\\'));
		ScriptsList->SetItem(pos,3,Kai->Auto->Scripts[i]->description);
		if(!Kai->Auto->Scripts[i]->loaded){ScriptsList->SetItemBackgroundColour(i, wxColour(255,128,128));}
		}

	Connect(ID_SLIST,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&ScriptsDialog::OnShowMacros);

	wxBoxSizer *bs = new wxBoxSizer(wxHORIZONTAL);

	bs->Add(new wxButton(this,ID_BADD,"Dodaj"),1,0,0);
	bs->Add(new wxButton(this,ID_BREMOVE,"Usuñ"),1,0,0);
	bs->AddSpacer(5);
	bs->Add(new wxButton(this,ID_BEDIT,"Edytuj"),1,0,0);
	bs->Add(new wxButton(this,ID_BRELOAD,"Odœwie¿"),1,0,0);
	bs->Add(new wxButton(this,ID_BRESCAN,"Wczytaj skrypty Autoload"),1,0,0);
	bs->AddSpacer(5);
	bs->Add(new wxButton(this,wxID_CANCEL,"Zamknij"),1,0,0);

	main->Add(ScriptsList,1,wxEXPAND|wxALL, 2);
	main->Add(bs,0,wxLEFT|wxBOTTOM|wxRIGHT, 2);

	Connect(ID_BADD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnAdd);
	Connect(ID_BREMOVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnDelete);
	Connect(ID_BEDIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnEdit);
	Connect(ID_BRELOAD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnReload);
	Connect(ID_BRESCAN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScriptsDialog::OnRescan);

	SetSizerAndFit(main);
	SetDropTarget(new DragScripts(Kai));
}

ScriptsDialog::~ScriptsDialog()
{
}


void ScriptsDialog::OnShowMacros(wxListEvent &event)
{
	TabPanel *pan=Kai->GetTab();
	pan->Video->blockpaint=true;
	long sel=event.GetIndex();
	//wxListItem item=event.GetItem();

	if(Kai->Auto->Scripts[sel]->CheckLastModified()){Kai->Auto->Scripts[sel]->Reload();}
	// tu trzeba jakieœ okno zawsze na wierzchu zrobiæ i w niego wrzuciæ makra
	MacrosDialog MD(this, sel);
	int ret = MD.ShowModal();
	//wxLogStatus("endmodal");
	//pan->Thaw();
	if(ret==5){
		//wxLogStatus("setmod");
		pan->Grid1->SetModified();
		//wxLogStatus("repaint");
		pan->Grid1->RepaintWindow();
		//wxLogStatus("all");
	}
	pan->Video->blockpaint=false;
}

void ScriptsDialog::OnAdd(wxCommandEvent &event)
{
	wxString luarecent=Options.GetString("Lua Recent Folder");
	//wxLogStatus(luarecent);
	wxFileDialog *FD = new wxFileDialog(Kai, _T("Wybierz plik Lua"), luarecent, 
		_T(""), _T("Pliki Lua|*.lua"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (FD->ShowModal() == wxID_OK){
		if(Kai->Auto->Add(FD->GetPath())){
			int i=Kai->Auto->Scripts.size()-1;
			Options.SetString("Lua Recent Folder", Kai->Auto->Scripts[i]->filename.BeforeLast('\\'));
			long pos = ScriptsList->InsertItem(i,wxString::Format("%i",(int)(i+1)));
			ScriptsList->SetItem(pos,1,Kai->Auto->Scripts[i]->name);
			ScriptsList->SetItem(pos,2,Kai->Auto->Scripts[i]->filename.AfterLast('\\'));
			ScriptsList->SetItem(pos,3,Kai->Auto->Scripts[i]->description);
			ScriptsList->ScrollList(0,11111111);//ScriptsList->
			Grid *grid=Kai->GetTab()->Grid1;
			wxString scripts = grid->GetSInfo("Kai->Automation Scripts")+=(Kai->Auto->Scripts[i]->filename+"|");
			grid->AddSInfo("Kai->Automation Scripts", scripts);
		}
	
    }
	FD->Destroy();
	
}

void ScriptsDialog::OnDelete(wxCommandEvent &event)
	{
	int i = ScriptsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;
	Kai->Auto->Remove(i);
	ScriptsList->DeleteItem(i);
	}

void ScriptsDialog::OnEdit(wxCommandEvent &event)
{
	wxString editor=Options.GetString("Script Editor");
	if(editor=="" || wxGetKeyState(WXK_SHIFT)){
		editor = wxFileSelector(_T("Wybierz edytor skryptów"), _T(""),
			_T("C:\\Windows\\Notepad.exe"), _T("exe"), _T("Programy (*.exe)|*.exe|Wszystkie pliki (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
		Options.SetString("Script Editor",editor);
		Options.SaveOptions();
	}
	int i = ScriptsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;
	wxWCharBuffer editorbuf = editor.c_str(), sfnamebuf = Kai->Auto->Scripts[i]->filename.c_str();
	wchar_t **cmdline = new wchar_t*[3];
	cmdline[0] = editorbuf.data();
	cmdline[1] = sfnamebuf.data();
	cmdline[2] = 0;

	long res = wxExecute(cmdline);
	delete cmdline;

	if (!res) {
		wxMessageBox(_T("Nie mo¿na uruchomiæ edytora."), _T("B³¹d Kai->Automatyzacji"), wxOK|wxICON_ERROR);
	}
}

void ScriptsDialog::OnReload(wxCommandEvent &event)
	{
	int i = ScriptsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;
	Kai->Auto->ReloadMacro(i);
		ScriptsList->SetItem(i,0,wxString::Format("%i",(int)(i+1)));
		ScriptsList->SetItem(i,1,Kai->Auto->Scripts[i]->name);
		ScriptsList->SetItem(i,2,Kai->Auto->Scripts[i]->filename.AfterLast('\\'));
		ScriptsList->SetItem(i,3,Kai->Auto->Scripts[i]->description);
		if(!Kai->Auto->Scripts[i]->loaded){ScriptsList->SetItemBackgroundColour(i, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));}
	}

void ScriptsDialog::OnRescan(wxCommandEvent &event)
	{
	
	Kai->Auto->ReloadScripts();
	ScriptsList->DeleteAllItems();

	for(size_t i=0; i<Kai->Auto->Scripts.size(); i++)
		{
		long pos = ScriptsList->InsertItem(i,wxString::Format("%i",(int)(i+1)));
		ScriptsList->SetItem(pos,1,Kai->Auto->Scripts[i]->name);
		ScriptsList->SetItem(pos,2,Kai->Auto->Scripts[i]->filename.AfterLast('\\'));
		ScriptsList->SetItem(pos,3,Kai->Auto->Scripts[i]->description);
		if(!Kai->Auto->Scripts[i]->loaded){ScriptsList->SetItemBackgroundColour(i, wxColour(255,128,128));}
		}
	}

void ScriptsDialog::AddFromSubs()
	{
	//wxLogStatus("wesz³o");
	wxString paths=Kai->GetTab()->Grid1->GetSInfo("Kai->Automation Scripts");
	//wxLogStatus("m"+paths);
	if(paths==""){return;}
	paths.Trim(false);
	wxStringTokenizer token(paths,"|~$",wxTOKEN_RET_EMPTY_ALL);
	int error_count=0;
	while(token.HasMoreTokens())
		{
		wxString onepath=token.GetNextToken();
		onepath.Trim(false);
		//wxLogStatus(onepath);
		if(!wxFileExists(onepath)){continue;}
		
		try {
			if(!Kai->Auto->Add(onepath)){continue;}
					int last=Kai->Auto->Scripts.size()-1;

					long pos = ScriptsList->InsertItem(last,wxString::Format("%i",(int)(last+1)));
					ScriptsList->SetItem(pos,1,Kai->Auto->Scripts[last]->name);
					ScriptsList->SetItem(pos,2,Kai->Auto->Scripts[last]->filename.AfterLast('\\'));
					ScriptsList->SetItem(pos,3,Kai->Auto->Scripts[last]->description);
					if(!Kai->Auto->Scripts[last]->loaded){ScriptsList->SetItemBackgroundColour(last, wxColour(255,128,128)); error_count++;}
					
				}
				catch (const wchar_t *e) {
					error_count++;
					wxLogError(_T("B³¹d wczytywania skryptu Lua: %s\n%s"), onepath.c_str(), e);
				}
				catch (...) {
					error_count++;
					wxLogError(_T("Nieznany b³¹d wczytywania skryptu Lua: %s."), onepath.c_str());
				}
		}
		if (error_count > 0) {
			wxLogWarning(_T("Jeden b¹dŸ wiêcej skryptów z pliku napisów zawiera b³êdy,\n obejrzyj opisy skryptów by uzyskaæ wiêcej informacji."));
		}

	}

BEGIN_EVENT_TABLE(MacrosDialog,wxDialog)
	EVT_PAINT(MacrosDialog::OnPaint)
	EVT_MOUSE_EVENTS(MacrosDialog::OnMouseEvents)
END_EVENT_TABLE()