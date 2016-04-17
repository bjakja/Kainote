
#pragma once
#include "Hotkeys.h"
#include "OpennWrite.h"
#include "KainoteMain.h"
#include "config.h"
#include <wx/regex.h>
#include <wx/log.h>
#include <wx/msgdlg.h>


Hotkeys::Hotkeys()
{
}

void Hotkeys::FillTable()
{
	keys[WXK_BACK] = "Backspace";
	keys[WXK_SPACE] = "Space";
	keys[WXK_RETURN] = "Enter";
	keys[WXK_TAB] = "Tab";
	keys[WXK_PAUSE] = "Pause";

	keys[WXK_LEFT] = "Left";
	keys[WXK_RIGHT] = "Right";
	keys[WXK_UP] = "Up";
	keys[WXK_DOWN] = "Down";
	
	keys[WXK_INSERT] = "Insert";
	keys[WXK_DELETE] = "Delete";
	keys[WXK_HOME] = "Home";
	keys[WXK_END] = "End";
	keys[WXK_PAGEUP] = "PgUp";
	keys[WXK_PAGEDOWN] = "PgDn";

	keys[WXK_NUMPAD0] = "Num 0";
	keys[WXK_NUMPAD1] = "Num 1";
	keys[WXK_NUMPAD2] = "Num 2";
	keys[WXK_NUMPAD3] = "Num 3";
	keys[WXK_NUMPAD4] = "Num 4";
	keys[WXK_NUMPAD5] = "Num 5";
	keys[WXK_NUMPAD6] = "Num 6";
	keys[WXK_NUMPAD7] = "Num 7";
	keys[WXK_NUMPAD8] = "Num 8";
	keys[WXK_NUMPAD9] = "Num 9";
	keys[WXK_NUMPAD_ADD] = "Num +";
	keys[WXK_NUMPAD_SUBTRACT] = "Num -";
	keys[WXK_NUMPAD_SEPARATOR] = "Num |";
	keys[WXK_NUMPAD_MULTIPLY] = "Num *";
	keys[WXK_NUMPAD_DIVIDE] = "Num /";
	keys[WXK_NUMPAD_DECIMAL] = "Num .";
	keys[WXK_NUMPAD_ENTER] = "Num Enter";

	keys[WXK_F1] = "F1";
	keys[WXK_F2] = "F2";
	keys[WXK_F3] = "F3";
	keys[WXK_F4] = "F4";
	keys[WXK_F5] = "F5";
	keys[WXK_F6] = "F6";
	keys[WXK_F7] = "F7";
	keys[WXK_F8] = "F8";
	keys[WXK_F9] = "F9";
	keys[WXK_F10] = "F10";
	keys[WXK_F11] = "F11";
	keys[WXK_F12] = "F12";

	AudioKeys=false;
}

void Hotkeys::LoadDefault(std::map<int, hdata> &_hkeys, bool Audio)
{
	if(!Audio){
		_hkeys[ChangeTime]= hdata('G', _("Okno zmiany czasów"),"Ctrl-I");
		_hkeys[ConvertToASS]=hdata('G', _("Konwertuj do ASS"), "F9");
		_hkeys[ConvertToSRT]=hdata('G', _("Konwertuj do SRT"), "F8");
		_hkeys[ConvertToMDVD]=hdata('G', _("Konwertuj do MDVD"), "F10");
		_hkeys[ConvertToMPL2]=hdata('G', _("Konwertuj do MPL2"), "F11");
		_hkeys[ConvertToTMP]=hdata('G', _("Konwertuj do TMP"), "Ctrl-F12");
		_hkeys[StyleManager]=hdata('G', _("Menedżer stylów"), "Ctrl-M");
		_hkeys[Editor]=hdata('G', _("Włącz / Wyłącz edytor"), "Ctrl-E");
		_hkeys[OpenVideo]=hdata('G', _("Otwórz wideo"), "Ctrl-Shift-O");
		_hkeys[Search]=hdata('G', _("Znajdź"), "Ctrl-F");
		_hkeys[FindReplace]=hdata('G', _("Znajdź i zmień"), "Ctrl-H");
		_hkeys[Undo]=hdata('G', _("Cofnij"), "Ctrl-Z");
		_hkeys[Redo]=hdata('G', _("Ponów"), "Ctrl-Y");
		_hkeys[OpenSubs]=hdata('G', _("Otwórz napisy"), "Ctrl-O");
		_hkeys[SaveSubs]=hdata('G', _("Zapisz"), "Ctrl-S");
		_hkeys[SaveSubsAs]=hdata('G', _("Zapisz jako..."), "Ctrl-Shift-S");
		_hkeys[RemoveText]=hdata('G', _("Usuń tekst"), "Alt-Delete");
		_hkeys[Remove]=hdata('G', _("Usuń linijkę"), "Shift-Delete");
		_hkeys[SetStartTime]=hdata('G', _("Wstaw czas początkowy z wideo"), "Alt-Z");
		_hkeys[SetEndTime]=hdata('G', _("Wstaw czas końcowy z wideo"), "Alt-X");
		_hkeys[PlayPauseG]=hdata('G', _("Odtwarzaj / Pauza"), "Alt-Space");
		_hkeys[Plus5SecondG]=hdata('G', _("Wideo minus 5 sekund"), "Alt-Left");//Lewo
		_hkeys[Minus5SecondG]=hdata('G', _("Wideo plus 5 sekund"), "Alt-Right");//prawo
		_hkeys[PreviousFrame]=hdata('G', _("Klatka w tył"), "Alt-A");
		_hkeys[NextFrame]=hdata('G', _("Klatka w przód"), "Alt-S");
		_hkeys[PreviousLine]=hdata('G', _("Poprzednia linijka"), "Ctrl-Up");//góra
		_hkeys[NextLine]=hdata('G', _("Następna linijka"), "Ctrl-Down");//dół
		_hkeys[JoinWithPrevious]=hdata('G', _("Scal z poprzednią linijką"), "F3");
		_hkeys[JoinWithNext]=hdata('G', _("Scal z następną linijką"), "F4");
		_hkeys[SnapWithStart]=hdata('G', _("Przyklej start do klatki kluczowej"), "Shift-Left");//lewo
		_hkeys[SnapWithEnd]=hdata('G', _("Przyklej koniec do klatki kluczowej"), "Shift-Right");//prawo
		_hkeys[NextTab]=hdata('G', _("Następna karta"), "Ctrl-PgDn");
		_hkeys[PreviousTab]=hdata('G', _("Poprzednia karta"), "Ctrl-PgUp");
		_hkeys[SelectFromVideo]=hdata('G', _("Zaznacz linię z czasem wideo"), "F2");
		_hkeys[Help]=hdata('G', _("Pomoc (niekompletna, ale jednak)"), "F1");
		_hkeys[Duplicate]=hdata('N', _("Duplikuj linie"), "Ctrl-D");
		_hkeys[PasteCollumns]=hdata('N', _("Wklej kolumny"), "Ctrl-Shift-V");
		_hkeys[PlayPause]=hdata('W', _("Odtwarzaj"), "Space");
		_hkeys[Plus5Second]=hdata('W', _("Plus 5 sekund"), "Right");//prawo
		_hkeys[Minus5Second]=hdata('W', _("Minus 5 sekund"), "Left");//lewo
		_hkeys[PlusMinute]=hdata('W', _("Plus minuta"), "Up");
		_hkeys[MinusMinute]=hdata('W', _("Minus minuta"), "Down");
		_hkeys[NextVideo]=hdata('W', _("Następny plik"), ".");
		_hkeys[PreviousVideo]=hdata('W', _("Poprzedni plik"), ",");
		_hkeys[VolumePlus]=hdata('W', _("Dźwięk głośniej"), "Num .");
		_hkeys[VolumeMinus]=hdata('W', _("Dźwięk ciszej"), "Num 0");
		_hkeys[NextChapter]=hdata('W', _("Następny rozdział"), "M");
		_hkeys[PreviousChapter]=hdata('W', _("Poprzedni rozdział"), "N");
		_hkeys[PutBold]=hdata('E', _("Wstaw pogrubienie"), "Alt-W");
		_hkeys[PutItalic]=hdata('E', _("Wstaw kursywę"), "Alt-E");
		_hkeys[SplitLine]=hdata('E', _("Wstaw znak podziału"), "Ctrl-N");
		_hkeys[StartDifference]=hdata('E', _("Wstaw różnicę początkową"), "Ctrl-,");
		_hkeys[EndDifference]=hdata('E', _("Wstaw różnicę końcową"), "Ctrl-.");
	}else{
		_hkeys[AudioCommit]=hdata('A', _("Zatwierdź"), "Enter");
		_hkeys[AudioCommitAlt]=hdata('A', _("Zatwierdź zastępcze"), "G");
		_hkeys[AudioPrevious]=hdata('A', _("Poprzednia linijka"), "Left");
		_hkeys[AudioPreviousAlt]=hdata('A', _("Poprzednia linijka zastępcze"), "Z");
		_hkeys[AudioNext]=hdata('A', _("Następna linijka"), "Right");
		_hkeys[AudioNextAlt]=hdata('A', _("Następna linijka zastępcze"), "X");
		_hkeys[AudioPlay]=hdata('A', _("Odtwarzaj"), "Down");
		_hkeys[AudioPlayAlt]=hdata('A', _("Odtwarzaj zastępcze"), "S");
		_hkeys[AudioStop]=hdata('A', _("Zatrzymaj"), "H");
		_hkeys[AudioGoto]=hdata('A', _("Przejdź do zaznaczenia"), "F");
		_hkeys[AudioPlayBeforeMark]=hdata('A', _("Odtwarzaj przed znacznikem"), "Num 0");
		_hkeys[AudioPlayAfterMark]=hdata('A', _("Odtwarzaj po znaczniku"), "Num .");
		_hkeys[AudioPlay500MSFirst]=hdata('A', _("Odtwarzaj pierwsze 500ms"), "E");
		_hkeys[AudioPlay500MSLast]=hdata('A', _("Odtwarzaj końcowe 500ms"), "D");
		_hkeys[AudioPlay500MSBefore]=hdata('A', _("Odtwarzaj 500ms przed"), "Q");
		_hkeys[AudioPlay500MSAfter]=hdata('A', _("Odtwarzaj 500ms po"), "W");
		_hkeys[AudioPlayToEnd]=hdata('A', _("Odtwarzaj do końca"), "T");
		_hkeys[AudioLeadin]=hdata('A', _("Dodaj wstęp"), "C");
		_hkeys[AudioLeadout]=hdata('A', _("Dodaj zakończenie"), "V");
	}
}

Hotkeys::~Hotkeys()
{
	keys.clear();
	hkeys.clear();

}

int Hotkeys::LoadHkeys(bool Audio)
{
	if(keys.empty()){FillTable();}
	OpenWrite ow;
	wxString hkpath = (Audio)? "\\AudioHotkeys.txt" : "\\Hotkeys.txt";
	wxString acctxt = ow.FileOpen(Options.pathfull + hkpath,true);
	//failedowało z tak błachego powodu jak brak ścieżki
	//trzeba uważać na kolejność, opcje mają zdecydowane pierwszeństwo
	if(acctxt==""){LoadDefault(hkeys,Audio);return 1;}

	wxStringTokenizer hk(acctxt,"\n",wxTOKEN_STRTOK);

	bool checkVer=false;
	if(acctxt.StartsWith("[")){
		wxString token=hk.NextToken();
		int first = token.find(L"Build");
		if(first> -1){
			wxString ver = token.Mid(first+5).BeforeFirst(' ');
			int version=wxAtoi(ver);
			checkVer=(version>487);
		}
	}
	if(!checkVer){
		wxMessageBox(_("Plik skrótów jest przestarzały zostanie zamieniony domyślnym"));
		LoadDefault(hkeys,Audio);
		SaveHkeys(Audio);
	}

	int g=0;
	int scripts=30100;
	while(hk.HasMoreTokens())
	{
		wxString token=hk.NextToken();
		token.Trim(false);
		token.Trim(true);


		wxString Values=token.AfterFirst(' ');
		Values.Trim(false);
		Values.Trim(true);
		wxString Labels=token.BeforeFirst(' ');
		Labels.Trim(false);
		Labels.Trim(true);
		if(Labels.StartsWith("Script")){
			hkeys[scripts] = hdata('G',Labels,Values) ;
			scripts++;
			g++;
		}else if(Values!=""){
			if(Labels.IsNumber()){
				hkeys[wxAtoi(Labels)] = Values;
			}else{
				hkeys[GetIdValue(Labels.data())] = Values; 
			}
			g++;
		}		
	}
	if(g>10){if(Audio){AudioKeys=true;}return 1;}
	LoadDefault(hkeys,Audio);
	if(hkeys.size()>10){return 1;}
	return 0;

}

void Hotkeys::SaveHkeys(bool Audio)
{
	wxString Texthk="["+Options.progname+"]\r\n";
	for (std::map<int, hdata>::iterator cur = hkeys.begin();cur != hkeys.end();cur++) {
		if((!Audio && cur->second.Type=='A') || (Audio && !(cur->second.Type=='A')) ) {continue;}
		if(cur->first>=30100){Texthk << cur->second.Name << " " << cur->second.Accel << "\r\n";}
		else{
			wxString idstring = GetString((Id)cur->first);
			if(idstring==""){idstring<<cur->first;}
			Texthk << idstring << " " << cur->second.Type << "=" << cur->second.Accel << "\r\n";
		}
	}
	OpenWrite ow;
	wxString hkpath=(Audio)? "\\AudioHotkeys.txt" : "\\Hotkeys.txt";
	ow.FileWrite(Options.pathfull+hkpath, Texthk);

}



wxAcceleratorEntry Hotkeys::GetHKey(int itemid)
{
	wxString accel=hkeys[itemid].Accel;
	wxAcceleratorEntry accelkey;
	if (accel==""){return accelkey;/*ResetKey(itemid);accel=hkeys[itemid];*/}
	int modif=0;
	if(accel.Find("Alt-")!=-1){
		modif|=1;
	}
	if(accel.Find("Shift-")!=-1){
		modif|=4;
	}
	if(accel.Find("Ctrl-")!=-1){
		modif|=2;
	}

	//if(modif==0){accel.Prepend("-");}
	wxString akey=accel.AfterLast('-');
	int key=0;
	
	std::map<int,wxString>::iterator cur;
	for (cur = keys.begin();cur != keys.end();cur++) {
		if (cur->second == akey) {
			key = cur->first;
			break;
		}
	}

	if(key==0 && akey.Len()<2){key=static_cast<int>(akey[0]);}else if(key==0){wxLogStatus(_("Key \"%s\" nie jest prawidłowy"),akey);}
	//wxLogStatus(accname+" %i",key);
	//wxLogStatus(hkeys[itemid].Name+"modif %i, %i"+akey,modif, key);
	accelkey.Set(modif,key,itemid);

	return accelkey;
}

void Hotkeys::SetHKey(int id, wxString name, wxString hotkey)
{
	if(hotkey!=""){
		hkeys[id]=hdata(name[0], name.AfterFirst(' '), hotkey);
	}
}

wxString Hotkeys::GetMenuH(int id)
{
	auto it=hkeys.find(id);
	if(it!=hkeys.end()){return it->second.Accel;}
	return "";
}

void Hotkeys::ResetKey(int id)
{
	std::map<int,hdata> tmphkeys;
	LoadDefault(tmphkeys);LoadDefault(tmphkeys, true);
	auto it= tmphkeys.find(id);
	if(it!= tmphkeys.end())
	{
		hkeys[id]=it->second;
	}else{wxLogStatus(_("Nie można przywrócić skrótu, bo nie ma domyślnego ustawienia o idzie %i"), id);}
}

int Hotkeys::OnMapHkey(int id, wxString name,wxWindow *parent, wxString *windows, int elems)
{
	HkeysDialog hkd(parent,name,false,windows,elems);
	int resitem=-1;
	if(hkd.ShowModal()==0){


		for(std::map< int, hdata >::iterator cur=hkeys.begin(); cur!=hkeys.end(); cur++)
		{
			if(cur->second.Accel == hkd.hotkey && (cur->second.Type == hkd.hkname[0]) ){

				if(wxMessageBox(wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\"\n.Usunąć powtarzający się skrót?"), cur->second.Name), _("Uwaga"),wxYES_NO)==wxYES)
				{
					hkeys.erase(cur->first);
					resitem=cur->first;
				}else{ return -2;}
			}
		}
		//wxLogStatus("Sethkey");
		Hkeys.SetHKey(id, hkd.hkname, hkd.hotkey);
		//wxLogStatus("Setitem");
	}
	return resitem;
}

wxMenuItem *Hotkeys::SetAccMenu(wxMenu *menu, int id, const wxString &txt, const wxString &help, wxItemKind kind)
{
	wxString hkey=GetMenuH(id);
	wxString mtext=(hkey!="")? txt.BeforeFirst('\t')+"\t"+hkey : txt;
	if(hkey!="" && hkeys[id].Name==""){hkeys[id].Name=txt.BeforeFirst('\t');}
	return menu->Append(id,mtext,help,kind);
}


//Okno dialogowe przechwytujące skróty klawiszowe
//blokujące przy okazji dostęp do opcji
HkeysDialog::HkeysDialog( wxWindow *parent, wxString name, bool script, wxString *windows, int elems)
	: wxDialog(parent,-1,_("Mapowanie przycisków"),wxDefaultPosition,wxDefaultSize,wxCAPTION|wxWANTS_CHARS|wxCLOSE_BOX)
{
	/*wxChoice *global=NULL;
	if(windows){
	global=new wxChoice(this,-1,wxDefaultPosition,wxDefaultSize,elems, windows,wxWANTS_CHARS);
	global->SetSelection(0);
	global->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress,0,this);
	}*/
	wxStaticText *txt=new wxStaticText(this,-1,wxString::Format(_("Proszę wcisnąć klawisze skrótu dla \"%s\"."), name),wxDefaultPosition,wxDefaultSize,wxWANTS_CHARS);
	txt->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress,0,this);

	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	//if(global){MainSizer->Add(global, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 12);}
	MainSizer->Add(txt, 0, wxALL, 12);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	//CenterOnParent();
	SetPosition(wxGetMousePosition());
	if(elems){hkname<<windows[0][0]<<" ";}
	hkname<<name;
	scr=script;
}

HkeysDialog::~HkeysDialog()
{
}

void HkeysDialog::OnKeyPress(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if (key==0){key=event.GetUnicodeKey();}

	if(key!=WXK_SHIFT&&key!=WXK_ALT&&key!=WXK_CONTROL){

		if(event.AltDown()){hotkey<<"Alt-";}
		if(event.ControlDown()){hotkey<<"Ctrl-";}
		if(event.ShiftDown()){hotkey<<"Shift-";}

		if(hotkey=="" && (hkname.StartsWith("G") || hkname.StartsWith("E") || scr ) && (key>30 && key<127 ))//|| key>313 && key<318
		{
			wxMessageBox(_("Skróty globalne i edytora muszą zawierać modyfikatory (np. Shift, Ctrl, Alt)."));return;
		}
		wxString keytxt=Hkeys.keys[key];
		if(keytxt==""){keytxt=wchar_t(key);}
		hotkey<<keytxt;

		EndModal(0);
	}
}


Hotkeys Hkeys;
