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


#pragma once
#include "Hotkeys.h"
#include "OpennWrite.h"
#include "KainoteMain.h"
#include "Config.h"
#include "KaiMessageBox.h"
#include <wx/regex.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <algorithm>

bool operator < (const idAndType match, const idAndType match1){ 
	if(match.Type != match1.Type){return match.Type < match1.Type;} 
	return match.id < match1.id;
};
bool operator > (const idAndType match, const idAndType match1){ 
	if(match.Type != match1.Type){return match.Type < match1.Type;} 
	return match.id < match1.id;
};
bool operator <= (const idAndType match, const idAndType match1){ return match.id <= match1.id;};
bool operator >= (const idAndType match, const idAndType match1){ return match.id >= match1.id;};
bool operator == (const idAndType match, const idAndType match1){
	return match.id == match1.id && match.Type == match1.Type;
};
bool operator != (const idAndType match, const idAndType match1){ 
	return match.id != match1.id || match.Type != match1.Type;
};
bool operator == (const idAndType &match, const int match1){ 
	return match1==match.id;
};
bool operator == (const int match1 ,const idAndType &match){ 
	return match1==match.id;
};
bool operator >= (const idAndType &match, const int match1){ return match.id >= match1;};
bool operator >= ( const int match1, const idAndType &match){ return match1 >= match.id;};
bool operator <= (const idAndType &match, const int match1){ return match.id <= match1;};
bool operator <= (const int match1 ,const idAndType &match){ return match1 <= match.id;};
bool operator > (const idAndType &match, const int match1){ return match.id > match1;};
bool operator > ( const int match1, const idAndType &match){ return match1 > match.id;};
bool operator < (const idAndType &match, const int match1){ return match.id < match1;};
bool operator < (const int match1 ,const idAndType &match){ return match1 < match.id;};
bool operator != (const idAndType &match, const int match1){ 
	return match.id != match1;
};
bool operator != ( const int match1, const idAndType &match){ 
	return match1 != match.id;
};

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

void Hotkeys::LoadDefault(std::map<idAndType, hdata> &_hkeys, bool Audio)
{
	if(!Audio){
		_hkeys[idAndType(ChangeTime,'G')] = hdata(_("Okno zmiany czasów"),"Ctrl-I");
		_hkeys[idAndType(ConvertToASS,'G')] = hdata(_("Konwertuj do ASS"), "F9");
		_hkeys[idAndType(ConvertToSRT,'G')] = hdata(_("Konwertuj do SRT"), "F8");
		_hkeys[idAndType(ConvertToMDVD,'G')] = hdata(_("Konwertuj do MDVD"), "F10");
		_hkeys[idAndType(ConvertToMPL2,'G')] = hdata(_("Konwertuj do MPL2"), "F11");
		_hkeys[idAndType(ConvertToTMP,'G')] = hdata(_("Konwertuj do TMP"), "Ctrl-F12");
		_hkeys[idAndType(StyleManager,'G')] = hdata(_("Menedżer stylów"), "Ctrl-M");
		_hkeys[idAndType(Editor,'G')] = hdata(_("Włącz / Wyłącz edytor"), "Ctrl-E");
		_hkeys[idAndType(OpenVideo,'G')] = hdata(_("Otwórz wideo"), "Ctrl-Shift-O");
		_hkeys[idAndType(Search,'G')] = hdata(_("Znajdź"), "Ctrl-F");
		_hkeys[idAndType(FindReplace,'G')] = hdata(_("Znajdź i zmień"), "Ctrl-H");
		_hkeys[idAndType(Undo,'G')] = hdata(_("Cofnij"), "Ctrl-Z");
		_hkeys[idAndType(Redo,'G')] = hdata(_("Ponów"), "Ctrl-Y");
		_hkeys[idAndType(OpenSubs,'G')] = hdata(_("Otwórz napisy"), "Ctrl-O");
		_hkeys[idAndType(SaveSubs,'G')] = hdata(_("Zapisz"), "Ctrl-S");
		_hkeys[idAndType(SaveSubsAs,'G')] = hdata(_("Zapisz jako..."), "Ctrl-Shift-S");
		_hkeys[idAndType(RemoveText,'G')] = hdata(_("Usuń tekst"), "Alt-Delete");
		_hkeys[idAndType(Remove,'G')] = hdata(_("Usuń linijkę"), "Shift-Delete");
		_hkeys[idAndType(SetStartTime,'G')] = hdata(_("Wstaw czas początkowy z wideo"), "Alt-Z");
		_hkeys[idAndType(SetEndTime,'G')] = hdata(_("Wstaw czas końcowy z wideo"), "Alt-X");
		_hkeys[idAndType(PlayPauseG,'G')] = hdata(_("Odtwarzaj / Pauza"), "Alt-Space");
		_hkeys[idAndType(Plus5SecondG,'G')] = hdata(_("Wideo minus 5 sekund"), "Alt-Left");//Lewo
		_hkeys[idAndType(Minus5SecondG,'G')] = hdata(_("Wideo plus 5 sekund"), "Alt-Right");//prawo
		_hkeys[idAndType(PreviousFrame,'G')] = hdata(_("Klatka w tył"), "Alt-A");
		_hkeys[idAndType(NextFrame,'G')] = hdata(_("Klatka w przód"), "Alt-S");
		_hkeys[idAndType(PreviousLine,'G')] = hdata(_("Poprzednia linijka"), "Ctrl-Up");//góra
		_hkeys[idAndType(NextLine,'G')] = hdata(_("Następna linijka"), "Ctrl-Down");//dół
		_hkeys[idAndType(JoinWithPrevious,'G')] = hdata(_("Scal z poprzednią linijką"), "F3");
		_hkeys[idAndType(JoinWithNext,'G')] = hdata(_("Scal z następną linijką"), "F4");
		_hkeys[idAndType(SnapWithStart,'G')] = hdata(_("Przyklej start do klatki kluczowej"), "Shift-Left");//lewo
		_hkeys[idAndType(SnapWithEnd,'G')] = hdata(_("Przyklej koniec do klatki kluczowej"), "Shift-Right");//prawo
		_hkeys[idAndType(NextTab,'G')] = hdata(_("Następna karta"), "Ctrl-PgDn");
		_hkeys[idAndType(PreviousTab,'G')] = hdata(_("Poprzednia karta"), "Ctrl-PgUp");
		_hkeys[idAndType(SelectFromVideo,'G')] = hdata(_("Zaznacz linię z czasem wideo"), "F2");
		_hkeys[idAndType(Help,'G')] = hdata(_("Pomoc (niekompletna, ale jednak)"), "F1");
		_hkeys[idAndType(Duplicate,'N')] = hdata(_("Duplikuj linie"), "Ctrl-D");
		_hkeys[idAndType(PasteCollumns,'N')] = hdata(_("Wklej kolumny"), "Ctrl-Shift-V");
		_hkeys[idAndType(PlayPause,'W')] = hdata(_("Odtwarzaj"), "Space");
		_hkeys[idAndType(Plus5Second,'W')] = hdata(_("Plus 5 sekund"), "Right");//prawo
		_hkeys[idAndType(Minus5Second,'W')] = hdata(_("Minus 5 sekund"), "Left");//lewo
		_hkeys[idAndType(PlusMinute,'W')] = hdata(_("Plus minuta"), "Up");
		_hkeys[idAndType(MinusMinute,'W')] = hdata(_("Minus minuta"), "Down");
		_hkeys[idAndType(NextVideo,'W')] = hdata(_("Następny plik"), ".");
		_hkeys[idAndType(PreviousVideo,'W')] = hdata(_("Poprzedni plik"), ",");
		_hkeys[idAndType(VolumePlus,'W')] = hdata(_("Dźwięk głośniej"), "Num .");
		_hkeys[idAndType(VolumeMinus,'W')] = hdata(_("Dźwięk ciszej"), "Num 0");
		_hkeys[idAndType(NextChapter,'W')] = hdata(_("Następny rozdział"), "M");
		_hkeys[idAndType(PreviousChapter,'W')] = hdata(_("Poprzedni rozdział"), "N");
		_hkeys[idAndType(PutBold,'E')] = hdata(_("Wstaw pogrubienie"), "Alt-W");
		_hkeys[idAndType(PutItalic,'E')] = hdata(_("Wstaw kursywę"), "Alt-E");
		_hkeys[idAndType(SplitLine,'E')] = hdata(_("Wstaw znak podziału"), "Ctrl-N");
		_hkeys[idAndType(StartDifference,'E')] = hdata(_("Wstaw różnicę początkową"), "Ctrl-,");
		_hkeys[idAndType(EndDifference,'E')] = hdata(_("Wstaw różnicę końcową"), "Ctrl-.");
		_hkeys[idAndType(MENU_ZATW,'E')] = hdata(_("Zatwierdź zmiany"), "Ctrl-Enter");
		_hkeys[idAndType(MENU_NLINE,'E')] = hdata(_("Zatwierdź zmiany idź do następnej linii"), "Enter");
	}else{
		_hkeys[idAndType(AudioCommit,'A')] = hdata(_("Zatwierdź"), "Enter");
		_hkeys[idAndType(AudioCommitAlt,'A')] = hdata(_("Zatwierdź zastępcze"), "G");
		_hkeys[idAndType(AudioPrevious,'A')] = hdata(_("Poprzednia linijka"), "Left");
		_hkeys[idAndType(AudioPreviousAlt,'A')] = hdata(_("Poprzednia linijka zastępcze"), "Z");
		_hkeys[idAndType(AudioNext,'A')] = hdata(_("Następna linijka"), "Right");
		_hkeys[idAndType(AudioNextAlt,'A')] = hdata(_("Następna linijka zastępcze"), "X");
		_hkeys[idAndType(AudioPlay,'A')] = hdata(_("Odtwarzaj"), "Down");
		_hkeys[idAndType(AudioPlayAlt,'A')] = hdata(_("Odtwarzaj zastępcze"), "S");
		_hkeys[idAndType(AudioStop,'A')] = hdata(_("Zatrzymaj"), "H");
		_hkeys[idAndType(AudioGoto,'A')] = hdata(_("Przejdź do zaznaczenia"), "F");
		_hkeys[idAndType(AudioPlayBeforeMark,'A')] = hdata(_("Odtwarzaj przed znacznikem"), "Num 0");
		_hkeys[idAndType(AudioPlayAfterMark,'A')] = hdata(_("Odtwarzaj po znaczniku"), "Num .");
		_hkeys[idAndType(AudioPlay500MSFirst,'A')] = hdata(_("Odtwarzaj pierwsze 500ms"), "E");
		_hkeys[idAndType(AudioPlay500MSLast,'A')] = hdata(_("Odtwarzaj końcowe 500ms"), "D");
		_hkeys[idAndType(AudioPlay500MSBefore,'A')] = hdata(_("Odtwarzaj 500ms przed"), "Q");
		_hkeys[idAndType(AudioPlay500MSAfter,'A')] = hdata(_("Odtwarzaj 500ms po"), "W");
		_hkeys[idAndType(AudioPlayToEnd,'A')] = hdata(_("Odtwarzaj do końca"), "T");
		_hkeys[idAndType(AudioLeadin,'A')] = hdata(_("Dodaj wstęp"), "C");
		_hkeys[idAndType(AudioLeadout,'A')] = hdata(_("Dodaj zakończenie"), "V");
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
	wxString acctxt;
	//failedowało z tak błachego powodu jak brak ścieżki
	//trzeba uważać na kolejność, opcje mają zdecydowane pierwszeństwo
	if(!ow.FileOpen(Options.pathfull + hkpath, &acctxt, true)){LoadDefault(hkeys,Audio);return 1;}

	wxStringTokenizer hk(acctxt,"\n",wxTOKEN_STRTOK);

	bool checkVer=false;
	if(acctxt.StartsWith("[")){
		wxString token=hk.NextToken();
		wxString ver= token.BeforeFirst(']').Mid(1);
		if(ver != Options.progname){
			LoadDefault(hkeys,Audio);
		}
		int first = token.find(L".");//0.8.0.build
		if(first> -1){
			wxString ver = token.Mid(first+5).BeforeFirst(' ');
			//KaiMessageBox(ver);
			int version=wxAtoi(ver);
			checkVer=(version>487);
		}
	}
	if(!checkVer){
		KaiMessageBox(_("Plik skrótów jest przestarzały zostanie zamieniony domyślnym"));
		LoadDefault(hkeys,Audio);
		SaveHkeys(Audio);
		return 1;
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
		char type=Values[0];
		Values = Values.Remove(0,2);
		wxString Labels=token.BeforeFirst(' ');
		Labels.Trim(false);
		Labels.Trim(true);
		if(Labels.StartsWith("Script")){
			hkeys[idAndType(scripts,'G')] = hdata(Labels,Values) ;
			scripts++;
			g++;
		}else if(Values!=""){
			if(Labels.IsNumber()){
				hkeys[idAndType(wxAtoi(Labels), type)] = Values;
			}else{
				hkeys[idAndType(GetIdValue(Labels.data()), type )] = Values; 
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
	for (std::map<idAndType, hdata>::iterator cur = hkeys.begin();cur != hkeys.end();cur++) {
		if((!Audio && cur->first.Type=='A') || (Audio && !(cur->first.Type=='A')) ) {continue;}
		if(cur->first >= 30100){Texthk << cur->second.Name << " " << cur->second.Accel << "\r\n";}
		else{
			wxString idstring = GetString((Id)cur->first.id);
			if(idstring==""){idstring<<cur->first.id;}
			Texthk << idstring << " " << cur->first.Type << "=" << cur->second.Accel << "\r\n";
		}
	}
	OpenWrite ow;
	wxString hkpath=(Audio)? "\\AudioHotkeys.txt" : "\\Hotkeys.txt";
	ow.FileWrite(Options.pathfull+hkpath, Texthk);

}

//itype jest wymagany, data nie wymaga dodatkowego szukania
wxAcceleratorEntry Hotkeys::GetHKey(const idAndType itype, const hdata *data)
{
	
	wxAcceleratorEntry accelkey;
	wxString accel;
	if(!data){
		auto ahkey = hkeys.find(itype);
		if(!(ahkey!=hkeys.end())){return accelkey;}
		accel= ahkey->second.Accel;
	}else{
		accel= data->Accel;
	}
	
	
	
	if (accel==""){return accelkey;/*ResetKey(itemid);accel=hkeys[idAndType(itemid];*/}
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

	
	wxString akey= (accel.EndsWith("-"))? "-" : accel.AfterLast('-');
	int key=0;
	
	std::map<int,wxString>::iterator cur;
	for (cur = keys.begin();cur != keys.end();cur++) {
		if (cur->second == akey) {
			key = cur->first;
			break;
		}
	}

	if(key==0 && akey.Len()<2){key=static_cast<int>(akey[0]);}else if(key==0){wxLogStatus(_("Key \"%s\" nie jest prawidłowy"),akey);}
	accelkey.Set(modif,key,(itype.id<1000)? itype.id+1000 : itype.id);

	return accelkey;
}

void Hotkeys::SetHKey(const idAndType &itype, wxString name, wxString hotkey)
{
	if(hotkey!=""){
		hkeys[itype] = hdata( name, hotkey);
	}
}

wxString Hotkeys::GetMenuH(const idAndType &itype, const wxString &name)
{
	auto it=hkeys.find(itype);
	if(it!=hkeys.end()){it->second.Name = name; return it->second.Accel;}
	return "";
}

void Hotkeys::ResetKey(const idAndType *itype, int id, char type)
{
	idAndType tmpitype = (itype)? *itype : idAndType(id, type);
	std::map<idAndType,hdata> tmphkeys;
	LoadDefault(tmphkeys);LoadDefault(tmphkeys, true);
	auto it= tmphkeys.find(id);
	if(it!= tmphkeys.end())
	{
		hkeys[tmpitype] = it->second;
	}else{wxLogStatus(_("Nie można przywrócić skrótu, bo nie ma domyślnego ustawienia o idzie %i"), id);}
}

int Hotkeys::OnMapHkey(int id, wxString name,wxWindow *parent,char hotkeyWindow, bool showWindowSelection)
{
	HkeysDialog hkd(parent, name, hotkeyWindow, showWindowSelection);
	int resitem=-2;
	if(hkd.ShowModal()==wxID_OK){
		
		for(auto cur=hkeys.begin(); cur!=hkeys.end(); cur++)
		{
			if(cur->second.Accel == hkd.hotkey && (cur->first.Type == hkd.type) ){

				wxMessageDialog msg(parent, 
					wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\".\nCo zrobić?"),
					cur->second.Name), _("Uwaga"), wxYES_NO|wxCANCEL);
				msg.SetYesNoLabels (_("Zamień skróty"), _("Usuń skrót"));
				int result = msg.ShowModal();
				if(result==wxNO)
				{
					auto finditer = hkeys.find(idAndType(id,hkd.type));
					cur->second.Accel="";
					if(finditer!=hkeys.end()){
						cur->second.Accel = finditer->second.Accel;
					}
					resitem = cur->first.id;
				}else if(result==wxNO)
				{
					hkeys.erase(cur->first);
					resitem = cur->first.id;
				}else{ return resitem;}
			}
		}
		
		Hkeys.SetHKey(idAndType(id,hkd.type), hkd.hkname, hkd.hotkey);
		if(hkd.type==hotkeyWindow) return -1;
	}
	return resitem;
}


void Hotkeys::SetAccels(bool all){
	Notebook *Tabs=Notebook::GetTabs();
	if(all){
		kainoteFrame * frame = (kainoteFrame *)Tabs->GetParent();
		frame->SetAccels();
		return;
	}
	for(size_t i=0;i<Tabs->Size();i++){
		Tabs->Page(i)->SetAccels();
	}
}

//Okno dialogowe przechwytujące skróty klawiszowe
//blokujące przy okazji dostęp do opcji
HkeysDialog::HkeysDialog( wxWindow *parent, wxString name, char hotkeyWindow, bool showWindowSelection)
	: wxDialog(parent,-1,_("Mapowanie przycisków"),wxDefaultPosition,wxDefaultSize,wxCAPTION|wxWANTS_CHARS|wxCLOSE_BOX)
{
	global=NULL;
	const int elems = 5;
	wxString windows[elems] = {_("Skrót globalny"),_("Skrót pola napisów"), _("Skrót pola edycji"), _("Skrót wideo"), _("Skrót audio")};
	wxString scwins = "GNEWA";
	if(showWindowSelection){
		global=new KaiChoice(this,-1,wxDefaultPosition,wxDefaultSize,elems, windows,wxWANTS_CHARS);
		global->SetSelection(scwins.find(hotkeyWindow));
		global->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress,0,this);
	}
	wxStaticText *txt=new wxStaticText(this,-1,wxString::Format(_("Proszę wcisnąć klawisze skrótu dla \"%s\"."), name),wxDefaultPosition,wxDefaultSize,wxWANTS_CHARS);
	txt->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress,0,this);

	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	if(global){MainSizer->Add(global, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 12);}
	MainSizer->Add(txt, 0, wxALL, 12);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	//CenterOnParent();
	SetPosition(wxGetMousePosition());
	type = hotkeyWindow;
	hkname=name;
	//scr=script;
}

HkeysDialog::~HkeysDialog()
{
}

void HkeysDialog::OnKeyPress(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if (key==0){key=event.GetUnicodeKey();}
	if(global) {
		wxString scwins = "GNEWA";
		type = scwins[global->GetSelection()];
	}
	hotkey="";
	if(key!=WXK_SHIFT && key!=WXK_ALT && key!=WXK_CONTROL){

		if(event.AltDown()){hotkey<<"Alt-";}
		if(event.ControlDown()){hotkey<<"Ctrl-";}
		if(event.ShiftDown()){hotkey<<"Shift-";}

		if(hotkey=="" && (type == 'G' || type == 'E') && (key>30 && key<127 || key>313 && key<318))//
		{
			KaiMessageBox(_("Skróty globalne i edytora muszą zawierać modyfikatory (np. Shift, Ctrl, Alt)."));return;
		}else if( event.GetModifiers() == wxMOD_CONTROL && (key == 'V' || key == 'C' || key == 'X')){
			KaiMessageBox(_("Nie możesz użyć skrótów do kopiowania, wycinania i wklejania.")); return;
		}

		wxString keytxt=Hkeys.keys[key];
		if(keytxt==""){keytxt=wchar_t(key);}
		hotkey<<keytxt;

		EndModal(wxID_OK);
	}
}


Hotkeys Hkeys;
