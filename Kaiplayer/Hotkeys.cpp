//  Copyright (c) 2016-2018, Marcin Drob

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
	:lastScriptId(30100)
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
		_hkeys[idAndType(Quit,GLOBAL_HOTKEY)] = hdata(_("Wyjście"),"Alt-F4");
		_hkeys[idAndType(ChangeTime,GLOBAL_HOTKEY)] = hdata(_("Okno zmiany czasów"),"Ctrl-I");
		_hkeys[idAndType(ConvertToASS,GLOBAL_HOTKEY)] = hdata(_("Konwertuj do ASS"), "F9");
		_hkeys[idAndType(ConvertToSRT,GLOBAL_HOTKEY)] = hdata(_("Konwertuj do SRT"), "F8");
		_hkeys[idAndType(ConvertToMDVD,GLOBAL_HOTKEY)] = hdata(_("Konwertuj do MDVD"), "F10");
		_hkeys[idAndType(ConvertToMPL2,GLOBAL_HOTKEY)] = hdata(_("Konwertuj do MPL2"), "F11");
		_hkeys[idAndType(ConvertToTMP,GLOBAL_HOTKEY)] = hdata(_("Konwertuj do TMP"), "Ctrl-F12");
		_hkeys[idAndType(StyleManager,GLOBAL_HOTKEY)] = hdata(_("Menedżer stylów"), "Ctrl-M");
		_hkeys[idAndType(Editor,GLOBAL_HOTKEY)] = hdata(_("Włącz / Wyłącz edytor"), "Ctrl-E");
		_hkeys[idAndType(OpenVideo,GLOBAL_HOTKEY)] = hdata(_("Otwórz wideo"), "Ctrl-Shift-O");
		_hkeys[idAndType(Search,GLOBAL_HOTKEY)] = hdata(_("Znajdź"), "Ctrl-F");
		_hkeys[idAndType(GLOBAL_FIND_REPLACE,GLOBAL_HOTKEY)] = hdata(_("Znajdź i zmień"), "Ctrl-H");
		_hkeys[idAndType(Undo,GLOBAL_HOTKEY)] = hdata(_("Cofnij"), "Ctrl-Z");
		_hkeys[idAndType(Redo,GLOBAL_HOTKEY)] = hdata(_("Ponów"), "Ctrl-Y");
		_hkeys[idAndType(History,GLOBAL_HOTKEY)] = hdata(_("Historia"), "Ctrl-Shift-H");
		_hkeys[idAndType(OpenSubs,GLOBAL_HOTKEY)] = hdata(_("Otwórz napisy"), "Ctrl-O");
		_hkeys[idAndType(SaveSubs,GLOBAL_HOTKEY)] = hdata(_("Zapisz"), "Ctrl-S");
		_hkeys[idAndType(SaveSubsAs,GLOBAL_HOTKEY)] = hdata(_("Zapisz jako..."), "Ctrl-Shift-S");
		_hkeys[idAndType(RemoveText,GLOBAL_HOTKEY)] = hdata(_("Usuń tekst"), "Alt-Delete");
		_hkeys[idAndType(Remove,GLOBAL_HOTKEY)] = hdata(_("Usuń linijkę"), "Shift-Delete");
		_hkeys[idAndType(SetStartTime,GLOBAL_HOTKEY)] = hdata(_("Wstaw czas początkowy z wideo"), "Ctrl-Left");
		_hkeys[idAndType(SetEndTime,GLOBAL_HOTKEY)] = hdata(_("Wstaw czas końcowy z wideo"), "Ctrl-Right");
		_hkeys[idAndType(PlayPauseG,GLOBAL_HOTKEY)] = hdata(_("Odtwórz / Pauza"), "Alt-Space");
		_hkeys[idAndType(Minus5SecondG,GLOBAL_HOTKEY)] = hdata(_("Wideo minus 5 sekund"), "Alt-Left");//Lewo
		_hkeys[idAndType(Plus5SecondG,GLOBAL_HOTKEY)] = hdata(_("Wideo plus 5 sekund"), "Alt-Right");//prawo
		_hkeys[idAndType(PreviousFrame,GLOBAL_HOTKEY)] = hdata(_("Klatka w tył"), "Left");
		_hkeys[idAndType(NextFrame,GLOBAL_HOTKEY)] = hdata(_("Klatka w przód"), "Right");
		_hkeys[idAndType(PreviousLine,GLOBAL_HOTKEY)] = hdata(_("Poprzednia linijka"), "Ctrl-Up");//góra
		_hkeys[idAndType(NextLine,GLOBAL_HOTKEY)] = hdata(_("Następna linijka"), "Ctrl-Down");//dół
		_hkeys[idAndType(JoinWithPrevious,GLOBAL_HOTKEY)] = hdata(_("Scal z poprzednią linijką"), "F3");
		_hkeys[idAndType(JoinWithNext,GLOBAL_HOTKEY)] = hdata(_("Scal z następną linijką"), "F4");
		_hkeys[idAndType(SnapWithStart,GLOBAL_HOTKEY)] = hdata(_("Przyklej start do klatki kluczowej"), "Shift-Left");//lewo
		_hkeys[idAndType(SnapWithEnd,GLOBAL_HOTKEY)] = hdata(_("Przyklej koniec do klatki kluczowej"), "Shift-Right");//prawo
		_hkeys[idAndType(NextTab,GLOBAL_HOTKEY)] = hdata(_("Następna karta"), "Ctrl-PgDn");
		_hkeys[idAndType(PreviousTab,GLOBAL_HOTKEY)] = hdata(_("Poprzednia karta"), "Ctrl-PgUp");
		_hkeys[idAndType(SelectFromVideo,GLOBAL_HOTKEY)] = hdata(_("Zaznacz linię z czasem wideo"), "F2");
		_hkeys[idAndType(Help,GLOBAL_HOTKEY)] = hdata(_("Pomoc (niekompletna, ale jednak)"), "F1");
		_hkeys[idAndType(Duplicate,GRID_HOTKEY)] = hdata(_("Duplikuj linie"), "Ctrl-D");
		_hkeys[idAndType(CopyCollumns,GRID_HOTKEY)] = hdata(_("Kopiuj kolumny"), "Ctrl-Shift-C");
		_hkeys[idAndType(PasteCollumns,GRID_HOTKEY)] = hdata(_("Wklej kolumny"), "Ctrl-Shift-V");
		_hkeys[idAndType(ShowPreview, GRID_HOTKEY)] = hdata(_("Pokaż podgląd napisów"), "Ctrl-Q");
		_hkeys[idAndType(PlayPause,VIDEO_HOTKEY)] = hdata(_("Odtwórz / Pauza"), "Space");
		_hkeys[idAndType(Plus5Second,VIDEO_HOTKEY)] = hdata(_("5 sekund do przodu"), "L");
		_hkeys[idAndType(Minus5Second,VIDEO_HOTKEY)] = hdata(_("5 sekund do tyłu"), ";");
		_hkeys[idAndType(PlusMinute,VIDEO_HOTKEY)] = hdata(_("Minuta do przodu"), "Up");
		_hkeys[idAndType(MinusMinute,VIDEO_HOTKEY)] = hdata(_("Minuta do tyłu"), "Down");
		_hkeys[idAndType(NextVideo,VIDEO_HOTKEY)] = hdata(_("Następny plik"), ".");
		_hkeys[idAndType(PreviousVideo,VIDEO_HOTKEY)] = hdata(_("Poprzedni plik"), ",");
		_hkeys[idAndType(VolumePlus,VIDEO_HOTKEY)] = hdata(_("Dźwięk głośniej"), "Num .");
		_hkeys[idAndType(VolumeMinus,VIDEO_HOTKEY)] = hdata(_("Dźwięk ciszej"), "Num 0");
		_hkeys[idAndType(NextChapter,VIDEO_HOTKEY)] = hdata(_("Następny rozdział"), "M");
		_hkeys[idAndType(PreviousChapter,VIDEO_HOTKEY)] = hdata(_("Poprzedni rozdział"), "N");
		_hkeys[idAndType(PutBold,EDITBOX_HOTKEY)] = hdata(_("Wstaw pogrubienie"), "Ctrl-I");
		_hkeys[idAndType(PutItalic,EDITBOX_HOTKEY)] = hdata(_("Wstaw kursywę"), "Ctrl-B");
		_hkeys[idAndType(SplitLine,EDITBOX_HOTKEY)] = hdata(_("Wstaw znak podziału"), "Shift-Enter");
		_hkeys[idAndType(StartDifference,EDITBOX_HOTKEY)] = hdata(_("Wstaw różnicę początkową"), "Ctrl-,");
		_hkeys[idAndType(EndDifference,EDITBOX_HOTKEY)] = hdata(_("Wstaw różnicę końcową"), "Ctrl-.");
		_hkeys[idAndType(FindNextDoubtful,EDITBOX_HOTKEY)] = hdata(_("Następne niepewne"), "Ctrl-D");
		_hkeys[idAndType(FindNextUntranslated,EDITBOX_HOTKEY)] = hdata(_("Następne nieprzetłumaczone"), "Ctrl-R");
		_hkeys[idAndType(SetDoubtful,EDITBOX_HOTKEY)] = hdata(_("Ustaw jako niepewne i przejdź dalej"), "Alt-Down");
		_hkeys[idAndType(EDITBOX_COMMIT,EDITBOX_HOTKEY)] = hdata(_("Zatwierdź zmiany"), "Ctrl-Enter");
		_hkeys[idAndType(EDITBOX_COMMIT_GO_NEXT_LINE,EDITBOX_HOTKEY)] = hdata(_("Zatwierdź zmiany idź do następnej linii"), "Enter");
	}else{
		_hkeys[idAndType(AudioCommit,AUDIO_HOTKEY)] = hdata(_("Zatwierdź"), "Enter");
		_hkeys[idAndType(AudioCommitAlt,AUDIO_HOTKEY)] = hdata(_("Zatwierdź zastępcze"), "G");
		_hkeys[idAndType(AudioPrevious,AUDIO_HOTKEY)] = hdata(_("Poprzednia linijka"), "Left");
		_hkeys[idAndType(AudioPreviousAlt,AUDIO_HOTKEY)] = hdata(_("Poprzednia linijka zastępcze"), "Z");
		_hkeys[idAndType(AudioNext,AUDIO_HOTKEY)] = hdata(_("Następna linijka"), "Right");
		_hkeys[idAndType(AudioNextAlt,AUDIO_HOTKEY)] = hdata(_("Następna linijka zastępcze"), "X");
		_hkeys[idAndType(AudioPlay,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj"), "Down");
		_hkeys[idAndType(AudioPlayAlt,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj zastępcze"), "S");
		_hkeys[idAndType(AudioPlayLine, AUDIO_HOTKEY)] = hdata(_("Odtwarzaj linię"), "Up");
		_hkeys[idAndType(AudioPlayLineAlt, AUDIO_HOTKEY)] = hdata(_("Odtwarzaj linię zastępcze"), "R");
		_hkeys[idAndType(AudioStop,AUDIO_HOTKEY)] = hdata(_("Zatrzymaj"), "H");
		_hkeys[idAndType(AudioGoto,AUDIO_HOTKEY)] = hdata(_("Przejdź do zaznaczenia"), "B");
		_hkeys[idAndType(AudioScrollRight, AUDIO_HOTKEY)] = hdata(_("Przewiń w lewo"), "A");
		_hkeys[idAndType(AudioScrollLeft, AUDIO_HOTKEY)] = hdata(_("Przewiń w prawo"), "F");
		_hkeys[idAndType(AudioPlayBeforeMark,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj przed znacznikem"), "Num 0");
		_hkeys[idAndType(AudioPlayAfterMark,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj po znaczniku"), "Num .");
		_hkeys[idAndType(AudioPlay500MSFirst,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj pierwsze 500ms"), "E");
		_hkeys[idAndType(AudioPlay500MSLast,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj końcowe 500ms"), "D");
		_hkeys[idAndType(AudioPlay500MSBefore,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj 500ms przed"), "Q");
		_hkeys[idAndType(AudioPlay500MSAfter,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj 500ms po"), "W");
		_hkeys[idAndType(AudioPlayToEnd,AUDIO_HOTKEY)] = hdata(_("Odtwarzaj do końca"), "T");
		_hkeys[idAndType(AudioLeadin,AUDIO_HOTKEY)] = hdata(_("Dodaj wstęp"), "C");
		_hkeys[idAndType(AudioLeadout,AUDIO_HOTKEY)] = hdata(_("Dodaj zakończenie"), "V");
	}
}

Hotkeys::~Hotkeys()
{
	keys.clear();
	hkeys.clear();
	if (hotkeysNaming){
		delete hotkeysNaming;
		hotkeysNaming = NULL;
	}
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
	lastScriptId=30100;
	while(hk.HasMoreTokens())
	{
		wxString token=hk.NextToken();
		token.Trim(false);
		token.Trim(true);
		if(token.StartsWith("Script")){
			wxString rest;
			wxString name = token.BeforeFirst('=', &rest).Trim(false).Trim(true);
			rest = rest.Trim(false).Trim(true);
			if(rest.IsEmpty()){continue;}
			hkeys[idAndType(lastScriptId,GLOBAL_HOTKEY)] = hdata(name, rest);
			lastScriptId++;
			g++;
			continue;
		} 

		wxString Values=token.AfterFirst(' ').Trim(false).Trim(true);
		int type=wxString("GNEWA").find(Values[0]);
		Values = Values.Remove(0,2);
		wxString Labels=token.BeforeFirst(' ').Trim(false).Trim(true);
		if(Values!=""){
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
	wxString gnewa = "GNEWA";
	wxString Texthk="["+Options.progname+"]\r\n";
	for (std::map<idAndType, hdata>::iterator cur = hkeys.begin();cur != hkeys.end();cur++) {
		if ((!Audio && cur->first.Type == AUDIO_HOTKEY) || 
			(Audio && !(cur->first.Type == AUDIO_HOTKEY)) || 
			cur->first.id < 100 || cur->second.Accel.empty()) { continue; }
		if (cur->first >= 30100){ Texthk << cur->second.Name << "=" << cur->second.Accel << "\r\n"; }
		else{
			wxString idstring = GetString((Id)cur->first.id);
			if(idstring==""){idstring<<cur->first.id;}
			Texthk << idstring << " " << gnewa[cur->first.Type] << "=" << cur->second.Accel << "\r\n";
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

	if(key==0 && akey.Len()<2){key=static_cast<int>(akey[0]);}
	else if (key == 0){ 
		KaiLog(wxString::Format(_("Skrót \"%s\" nie jest prawidłowy"), akey)); 
	}
	accelkey.Set(modif,key,(itype.id<1000)? itype.id+1000 : itype.id);

	return accelkey;
}

void Hotkeys::SetHKey(const idAndType &itype, wxString name, wxString hotkey)
{
	hkeys[itype] = hdata( name, hotkey);
}

wxString Hotkeys::GetStringHotkey(const idAndType &itype, const wxString &name)
{
	auto it=hkeys.find(itype);
	if(it!=hkeys.end()){if(name!=""){it->second.Name = name;} return it->second.Accel;}
	return "";
}

void Hotkeys::ResetKey(const idAndType *itype, int id, char type)
{
	idAndType tmpitype = (itype)? *itype : idAndType(id, type);
	std::map<idAndType,hdata> tmphkeys;
	LoadDefault(tmphkeys);LoadDefault(tmphkeys, true);
	auto it= tmphkeys.find(tmpitype);
	if(it!= tmphkeys.end())
	{
		hkeys[tmpitype] = it->second;
	}else{
		KaiLog(wxString::Format(_("Nie można przywrócić skrótu, bo nie ma domyślnego ustawienia o id %i"), tmpitype.id));
	}
}

wxString Hotkeys::GetDefaultKey(const idAndType &itype)
{
	std::map<idAndType,hdata> tmphkeys;
	LoadDefault(tmphkeys);LoadDefault(tmphkeys, true);
	auto it= tmphkeys.find(itype);
	if(it!= tmphkeys.end())
	{
		return it->second.Accel;
	}
	return "";
}
//return -2 anulowano zmianę skrótów, -1 nowy skrót, 1+ id do zmiany skrótu.
void Hotkeys::OnMapHkey(int id, wxString name,wxWindow *parent,char hotkeyWindow /*= GLOBAL_HOTKEY*/, bool showWindowSelection/*=true*/)
{
	//sanity check
	if (id < 0 && name.empty())
		return;

	HkeysDialog *hkd= new HkeysDialog(parent, (name.empty())? GetName(id) : name, hotkeyWindow, showWindowSelection);
	if(hkd->ShowModal()==wxID_OK){
		lastScriptId = 30100;
		std::vector< std::map<idAndType, hdata>::iterator> idtypes;
		for(auto cur=hkeys.begin(); cur!=hkeys.end(); cur++)
		{
			if (id < 0 && cur->first.id >= 30100){
				if (cur->second.Name == name){
					id = cur->first.id;
				}
				lastScriptId++;
			}
			if(cur->second.Accel == hkd->hotkey){
				idtypes.push_back(cur);
			}
		}
		
		if (id < 0){
			id = lastScriptId;
			lastScriptId++;
		}

		bool saveAudioHotkeys = (hkd->type == AUDIO_HOTKEY);
		if (idtypes.size()){
			bool doubledHotkey = false;
			wxString doubledHkName;
			wxString windowNames[] = { _("Globalny"), _("Napisy"), _("Edytor"), _("Wideo"), _("Audio") };
			for (auto &idtype : idtypes){
				if (idtype->first.Type == hkd->type){
					doubledHotkey = true;
					doubledHkName = Hkeys.GetName(idtype->first.id);
					if (doubledHkName.empty())
						doubledHkName = idtype->second.Name;
					break;
				}
				else{
					if (!doubledHkName.empty())
						doubledHkName += L", ";

					wxString hotkeyName = Hkeys.GetName(idtype->first.id);
					if (hotkeyName.empty())
						hotkeyName = idtype->second.Name;
					doubledHkName += windowNames[idtype->first.Type] + L" " + hotkeyName;
					if (idtype->first.Type == AUDIO_HOTKEY)
						saveAudioHotkeys = true;
				}
			}
			int result = wxCANCEL;
			if (doubledHotkey){
				KaiMessageDialog msg(parent,
					wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\".\nCo zrobić?"),
					doubledHkName), _("Uwaga"), wxYES | wxOK | wxCANCEL);
				msg.SetOkLabel(_("Zamień skróty"));
				msg.SetYesLabel(_("Usuń skrót"));
				result = msg.ShowModal();
			}
			else{
				int buttonFlag = (idtypes.size() < 2) ? wxOK : 0;
				KaiMessageDialog msg(parent,
					wxString::Format(_("Ten skrót już istnieje w %s jako skrót do \"%s\".\nCo zrobić?"),
					(idtypes.size() > 1) ? _("innych oknach") : _("innym oknie"), doubledHkName), 
					_("Uwaga"), wxYES_NO | buttonFlag | wxCANCEL);
				if (idtypes.size() < 2)
					msg.SetOkLabel(_("Zamień skróty"));
				msg.SetYesLabel(_("Usuń skrót"));
				msg.SetNoLabel(_("Ustaw mimo to"));
				result = msg.ShowModal();
			}
			if (result == wxCANCEL){ return; }
			else if (result != wxNO){
				for (auto &idtype : idtypes){
					if (doubledHotkey && idtype->first.Type != hkd->type)
						continue;

					if (result == wxOK){
						auto finditer = hkeys.find(idAndType(id, hkd->type));
						idtype->second.Accel = "";
						idtype->second.Accel = (finditer != hkeys.end())? finditer->second.Accel : "";
					}
					else if (result == wxYES){
						idtype->second.Accel = "";
					}
				}
			}
		}
		SetHKey(idAndType(id,hkd->type), hkd->hkname, hkd->hotkey);
		SetAccels(true);
		SaveHkeys();
		if (saveAudioHotkeys)
			SaveHkeys(true);
	}
	hkd->Destroy();
}

void Hotkeys::SetAccels(bool all){
	Notebook *Tabs=Notebook::GetTabs();
	KainoteFrame * frame = (KainoteFrame *)Tabs->GetParent();
	if (frame)
		frame->SetAccels(all);
}

wxString Hotkeys::GetName(const idAndType itype)
{
	auto it= hkeys.find(itype);
	if(it!= hkeys.end())
	{
		return it->second.Name;
	}

	return "";
}

const wxString & Hotkeys::GetName(int id)
{
	if (!hotkeysNaming)
		hotkeysNaming = new HotkeysNaming();

	return hotkeysNaming->GetName(id);
}

int Hotkeys::GetType(int id)
{
	if (id < PlayPause)
		return AUDIO_HOTKEY;
	else if (id < EDITBOX_CHANGE_FONT )
		return VIDEO_HOTKEY;
	else if (id < GRID_HIDE_LAYER || (id >= EDITBOX_TAG_BUTTON1 && id <= EDITBOX_TAG_BUTTON10))
		return EDITBOX_HOTKEY;
	else if (id < SaveSubs)
		return GRID_HOTKEY;

	return GLOBAL_HOTKEY;
}

const std::map<int, wxString> & Hotkeys::GetNamesTable()
{
	if (!hotkeysNaming)
		hotkeysNaming = new HotkeysNaming();

	return hotkeysNaming->GetNamesTable();
}

//Okno dialogowe przechwytujące skróty klawiszowe
//blokujące przy okazji dostęp do opcji
HkeysDialog::HkeysDialog( wxWindow *parent, wxString name, char hotkeyWindow, bool showWindowSelection)
	: KaiDialog(parent,-1,_("Mapowanie przycisków"),wxDefaultPosition,wxDefaultSize,wxCAPTION|wxWANTS_CHARS|wxCLOSE_BOX)
{
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	global=NULL;
	const int elems = 5;
	wxString windows[elems] = {_("Skrót globalny"),_("Skrót pola napisów"), _("Skrót pola edycji"), _("Skrót wideo"), _("Skrót audio")};
	if(showWindowSelection && hotkeyWindow == GLOBAL_HOTKEY){
		global=new KaiChoice(this,-1,wxDefaultPosition,wxDefaultSize,elems, windows,wxWANTS_CHARS);
		global->SetSelection(hotkeyWindow);
		global->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress,0,this);
	}
	KaiStaticText *txt=new KaiStaticText(this,-1,wxString::Format(_("Proszę wcisnąć klawisze skrótu dla \"%s\"."), name),wxDefaultPosition,wxDefaultSize,wxWANTS_CHARS);
	txt->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress,0,this);

	DialogSizer *MainSizer = new DialogSizer(wxVERTICAL);
	if(global){MainSizer->Add(global, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 12);}
	MainSizer->Add(txt, 0, wxALL, 12);
	SetSizerAndFit(MainSizer);
	MoveToMousePosition(this);
	type = hotkeyWindow;
	hkname=name;
}

HkeysDialog::~HkeysDialog()
{
}

void HkeysDialog::OnKeyPress(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if (key==0){key=event.GetUnicodeKey();}
	if(global) {
		type = global->GetSelection();
	}
	hotkey="";
	if(key!=WXK_SHIFT && key!=WXK_ALT && key!=WXK_CONTROL){

		if(event.AltDown()){hotkey<<"Alt-";}
		if(event.ControlDown()){hotkey<<"Ctrl-";}
		if(event.ShiftDown()){hotkey<<"Shift-";}

		if(hotkey=="" && (type == GLOBAL_HOTKEY || type == EDITBOX_HOTKEY) && (key>30 && key<127 /*|| key>313 && key<318*/))
		{
			KaiMessageBox(_("Skróty globalne i edytora muszą zawierać modyfikatory (np. Shift, Ctrl, Alt)."));return;
		}else if( event.GetModifiers() == wxMOD_CONTROL && (key == 'V' || key == 'C' || key == 'X' || key == 'Z')){
			KaiMessageBox(_("Nie można używać skrótów do kopiowania, wycinania i wklejania.")); return;
		}
		else if (event.AltDown() && !event.ControlDown() && key == WXK_F4){
			KaiMessageBox(_("Nie można używać skrótu zamykania programu.")); return;
		}

		wxString keytxt=Hkeys.keys[key];
		if(keytxt==""){
			if (key >= 36 && key <= 96)
				keytxt = wchar_t(key);
			else
				return;
		}
		hotkey<<keytxt;

		EndModal(wxID_OK);
	}
}




Hotkeys Hkeys;

DEFINE_ENUM(Id,IDS)