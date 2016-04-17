
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
	wxFont font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
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
	//specjalna różnica między pierwszą zaznaczoną linijką która jest zero a linijką w lua która jest większa o size sinfo + size styles
	int diff=(Kai->GetTab()->Grid1->SInfoSize()+Kai->GetTab()->Grid1->StylesSize()+1);
	for(size_t i=0; i<macros.size(); i++)
	{
		names.Add(macros[i]->name);
		wxString hkeyname;
		std::map<int, hdata >::iterator it;
		bool isaccel=false;
		for(it=Hkeys.hkeys.find(30100); it!=Hkeys.hkeys.end(); it++){
			wxString Rest;
			wxString scriptnum= it->second.Name.Mid(6).BeforeFirst('-', &Rest);
			if(wxAtoi(scriptnum)==script && wxAtoi(Rest)==i){
				names.Add(it->second.Accel);
				isaccel=true;
				break;
			}
		}
		if(!isaccel){names.Add(_("Brak"));}

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
	dc.SetFont(wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT));
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


			//wxLogStatus(test);
			for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++)
			{//wxLogStatus(cur->first);
				if(cur->second.Accel==hkd.hotkey){
					if(cur->second.Name==""){
						std::map<int, hdata> _hkeys;
						Hkeys.LoadDefault(_hkeys);
						Hkeys.LoadDefault(_hkeys,true);
						Notebook::GetTab()->Video->ContextMenu(wxPoint(0,0),true);
						Notebook::GetTab()->Grid1->ContextMenu(wxPoint(0,0),true);
						if(cur->second.Name==""){
							cur->second.Name = _hkeys[cur->first].Name;
						}
					}
					wxMessageBox(wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\"."), 
						cur->second.Name), _("Uwaga"), wxOK);
					CaptureMouse();
					return;
				}
			}

			names[(seld*2)+1]=hkd.hotkey;

			wxString hkeyname;
			hkeyname<<"Script"<<script<<"-"<<seld-1;
			bool strt=true;
			int lastid=30099;
			int curid=-1;
			for(auto cur=Hkeys.hkeys.rbegin(); cur!=Hkeys.hkeys.rend(); cur++){
				int id=cur->first;
				if(id>=30100){
					if(strt){lastid=id;strt=false;}
					if(cur->second.Name == hkeyname){
						curid=id; break;
					}
				}else{break;}

			}

			Hkeys.SetHKey((curid>-1)? curid : lastid+1,"G "+hkeyname, hkd.hotkey);
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


		//specjalna różnica między pierwszą zaznaczoną linijką która jest zero a linijką w lua która jest większa o size sinfo + size styles
		int diff=(pan->Grid1->SInfoSize() + pan->Grid1->StylesSize()+1);
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
		wxFont font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
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
	: wxDialog(_Kai,-1,_("Menedżer skryptów Lua"))
{

	Kai=_Kai;
	wxBoxSizer *main= new wxBoxSizer(wxVERTICAL);
	ScriptsList=new wxListCtrl(this, ID_SLIST, wxDefaultPosition, wxDefaultSize,wxLC_REPORT | wxLC_SINGLE_SEL);
	ScriptsList->InsertColumn(0,_("Lp."),wxLIST_FORMAT_LEFT,25);
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

	bs->Add(new wxButton(this,ID_BADD,_("Dodaj")),1,0,0);
	bs->Add(new wxButton(this,ID_BREMOVE,_("Usuń")),1,0,0);
	bs->AddSpacer(5);
	bs->Add(new wxButton(this,ID_BEDIT,_("Edytuj")),1,0,0);
	bs->Add(new wxButton(this,ID_BRELOAD,_("Odśwież")),1,0,0);
	bs->Add(new wxButton(this,ID_BRESCAN,_("Wczytaj skrypty Autoload")),1,0,0);
	bs->AddSpacer(5);
	bs->Add(new wxButton(this,wxID_CANCEL,_("Zamknij")),1,0,0);

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
	// tu trzeba jakieś okno zawsze na wierzchu zrobić i w niego wrzucić makra
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
	wxFileDialog *FD = new wxFileDialog(Kai, _("Wybierz plik Lua"), luarecent, 
		"", _("Pliki Lua|*.lua"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
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
			wxString scripts = grid->GetSInfo("Automation Scripts") += (Kai->Auto->Scripts[i]->filename+"|");
			grid->AddSInfo("Automation Scripts", scripts);
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
		editor = wxFileSelector(_("Wybierz edytor skryptów"), "",
			"C:\\Windows\\Notepad.exe", "exe", _("Programy (*.exe)|*.exe|Wszystkie pliki (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
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
		wxMessageBox(_("Nie można uruchomić edytora."), _("Błąd automatyzacji"), wxOK|wxICON_ERROR);
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
	//wxLogStatus("weszło");
	wxString paths=Kai->GetTab()->Grid1->GetSInfo("Automation Scripts");
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
			wxLogError(_("Błąd wczytywania skryptu Lua: %s\n%s"), onepath.c_str(), e);
		}
		catch (...) {
			error_count++;
			wxLogError(_("Nieznany błąd wczytywania skryptu Lua: %s."), onepath.c_str());
		}
	}
	if (error_count > 0) {
		wxLogWarning(_("Co najmniej jeden skrypt z pliku napisów zawiera błędy.\nZobacz opisy skryptów, by uzyskać więcej informacji."));
	}

}

BEGIN_EVENT_TABLE(MacrosDialog,wxDialog)
	EVT_PAINT(MacrosDialog::OnPaint)
	EVT_MOUSE_EVENTS(MacrosDialog::OnMouseEvents)
	END_EVENT_TABLE()