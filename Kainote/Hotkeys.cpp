//  Copyright (c) 2016 - 2026, Marcin Drob

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
#include "KainoteFrame.h"
#include "config.h"
#include "OpennWrite.h"

#include "KaiMessageBox.h"
#include "ConfigConverter.h"
#include "TabPanel.h"
#include "KaiStaticText.h"
#include "Notebook.h"
#include <wx/regex.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <algorithm>


bool operator < (const idAndType match, const idAndType match1){
	if (match.Type != match1.Type){ return match.Type < match1.Type; }
	return match.id < match1.id;
};
bool operator >(const idAndType match, const idAndType match1){
	if (match.Type != match1.Type){ return match.Type < match1.Type; }
	return match.id < match1.id;
};
bool operator <= (const idAndType match, const idAndType match1){ return match.id <= match1.id; };
bool operator >= (const idAndType match, const idAndType match1){ return match.id >= match1.id; };
bool operator == (const idAndType match, const idAndType match1){
	return match.id == match1.id && match.Type == match1.Type;
};
bool operator != (const idAndType match, const idAndType match1){
	return match.id != match1.id || match.Type != match1.Type;
};
bool operator == (const idAndType &match, const int match1){
	return match1 == match.id;
};
bool operator == (const int match1, const idAndType &match){
	return match1 == match.id;
};
bool operator >= (const idAndType &match, const int match1){ return match.id >= match1; };
bool operator >= (const int match1, const idAndType &match){ return match1 >= match.id; };
bool operator <= (const idAndType &match, const int match1){ return match.id <= match1; };
bool operator <= (const int match1, const idAndType &match){ return match1 <= match.id; };
bool operator > (const idAndType &match, const int match1){ return match.id > match1; };
bool operator > (const int match1, const idAndType &match){ return match1 > match.id; };
bool operator < (const idAndType &match, const int match1){ return match.id < match1; };
bool operator < (const int match1, const idAndType &match){ return match1 < match.id; };
bool operator != (const idAndType &match, const int match1){
	return match.id != match1;
};
bool operator != (const int match1, const idAndType &match){
	return match1 != match.id;
};

Hotkeys::Hotkeys()
	:lastScriptId(30100)
{
}

void Hotkeys::FillTable()
{
	keys[WXK_BACK] = L"Backspace";
	keys[WXK_SPACE] = L"Space";
	keys[WXK_RETURN] = L"Enter";
	keys[WXK_TAB] = L"Tab";
	keys[WXK_PAUSE] = L"Pause";

	keys[WXK_LEFT] = L"Left";
	keys[WXK_RIGHT] = L"Right";
	keys[WXK_UP] = L"Up";
	keys[WXK_DOWN] = L"Down";

	keys[WXK_INSERT] = L"Insert";
	keys[WXK_DELETE] = L"Delete";
	keys[WXK_HOME] = L"Home";
	keys[WXK_END] = L"End";
	keys[WXK_PAGEUP] = L"PgUp";
	keys[WXK_PAGEDOWN] = L"PgDn";

	keys[WXK_NUMPAD0] = L"Num 0";
	keys[WXK_NUMPAD1] = L"Num 1";
	keys[WXK_NUMPAD2] = L"Num 2";
	keys[WXK_NUMPAD3] = L"Num 3";
	keys[WXK_NUMPAD4] = L"Num 4";
	keys[WXK_NUMPAD5] = L"Num 5";
	keys[WXK_NUMPAD6] = L"Num 6";
	keys[WXK_NUMPAD7] = L"Num 7";
	keys[WXK_NUMPAD8] = L"Num 8";
	keys[WXK_NUMPAD9] = L"Num 9";
	keys[WXK_NUMPAD_ADD] = L"Num +";
	keys[WXK_NUMPAD_SUBTRACT] = L"Num -";
	keys[WXK_NUMPAD_SEPARATOR] = L"Num |";
	keys[WXK_NUMPAD_MULTIPLY] = L"Num *";
	keys[WXK_NUMPAD_DIVIDE] = L"Num /";
	keys[WXK_NUMPAD_DECIMAL] = L"Num .";
	keys[WXK_NUMPAD_ENTER] = L"Num Enter";

	keys[WXK_F1] = L"F1";
	keys[WXK_F2] = L"F2";
	keys[WXK_F3] = L"F3";
	keys[WXK_F4] = L"F4";
	keys[WXK_F5] = L"F5";
	keys[WXK_F6] = L"F6";
	keys[WXK_F7] = L"F7";
	keys[WXK_F8] = L"F8";
	keys[WXK_F9] = L"F9";
	keys[WXK_F10] = L"F10";
	keys[WXK_F11] = L"F11";
	keys[WXK_F12] = L"F12";

	AudioKeys = false;
}

void Hotkeys::LoadDefault(std::map<idAndType, hdata> &_hkeys, bool Audio)
{
	if (!Audio){
		_hkeys[idAndType(GLOBAL_QUIT, GLOBAL_HOTKEY)] = hdata(_("Exit"), L"Alt-F4");
		_hkeys[idAndType(GLOBAL_ADD_PAGE, GLOBAL_HOTKEY)] = hdata(_("Open new tab"), L"Ctrl-T");
		_hkeys[idAndType(GLOBAL_CLOSE_PAGE, GLOBAL_HOTKEY)] = hdata(_("Close current tab"), L"Ctrl-W");
		_hkeys[idAndType(GLOBAL_SHOW_SHIFT_TIMES, GLOBAL_HOTKEY)] = hdata(_("Time shift window"), L"Ctrl-I");
		_hkeys[idAndType(GLOBAL_CONVERT_TO_ASS, GLOBAL_HOTKEY)] = hdata(_("Convert to ASS"), L"F9");
		_hkeys[idAndType(GLOBAL_CONVERT_TO_SRT, GLOBAL_HOTKEY)] = hdata(_("Convert to SRT"), L"F8");
		_hkeys[idAndType(GLOBAL_CONVERT_TO_MDVD, GLOBAL_HOTKEY)] = hdata(_("Convert to MDVD"), L"F10");
		_hkeys[idAndType(GLOBAL_CONVERT_TO_MPL2, GLOBAL_HOTKEY)] = hdata(_("Convert to MPL2"), L"F11");
		_hkeys[idAndType(GLOBAL_CONVERT_TO_TMP, GLOBAL_HOTKEY)] = hdata(_("Convert to TMP"), L"Ctrl-F12");
		_hkeys[idAndType(GLOBAL_OPEN_STYLE_MANAGER, GLOBAL_HOTKEY)] = hdata(_("Style manager"), L"Ctrl-M");
		_hkeys[idAndType(GLOBAL_EDITOR, GLOBAL_HOTKEY)] = hdata(_("Enable / Disable editor"), L"Ctrl-E");
		_hkeys[idAndType(GLOBAL_OPEN_VIDEO, GLOBAL_HOTKEY)] = hdata(_("Open video"), L"Ctrl-Shift-O");
		_hkeys[idAndType(GLOBAL_SEARCH, GLOBAL_HOTKEY)] = hdata(_("Find"), L"Ctrl-F");
		_hkeys[idAndType(GLOBAL_FIND_REPLACE, GLOBAL_HOTKEY)] = hdata(_("Find and replace"), L"Ctrl-H");
		_hkeys[idAndType(GLOBAL_UNDO, GLOBAL_HOTKEY)] = hdata(_("Undo"), L"Ctrl-Z");
		_hkeys[idAndType(GLOBAL_REDO, GLOBAL_HOTKEY)] = hdata(_("Redo"), L"Ctrl-Y");
		_hkeys[idAndType(GLOBAL_HISTORY, GLOBAL_HOTKEY)] = hdata(_("History"), L"Ctrl-Shift-H");
		_hkeys[idAndType(GLOBAL_OPEN_SUBS, GLOBAL_HOTKEY)] = hdata(_("Open subtitles"), L"Ctrl-O");
		_hkeys[idAndType(GLOBAL_SAVE_SUBS, GLOBAL_HOTKEY)] = hdata(_("Save"), L"Ctrl-S");
		_hkeys[idAndType(GLOBAL_SAVE_SUBS_AS, GLOBAL_HOTKEY)] = hdata(_("Save as..."), L"Ctrl-Shift-S");
		_hkeys[idAndType(GLOBAL_REMOVE_TEXT, GLOBAL_HOTKEY)] = hdata(_("Delete text"), L"Alt-Delete");
		_hkeys[idAndType(GLOBAL_REMOVE_LINES, GLOBAL_HOTKEY)] = hdata(_("Delete line"), L"Shift-Delete");
		_hkeys[idAndType(GLOBAL_SET_START_TIME, GLOBAL_HOTKEY)] = hdata(_("Insert start time from video"), L"Ctrl-Left");
		_hkeys[idAndType(GLOBAL_SET_END_TIME, GLOBAL_HOTKEY)] = hdata(_("Insert end time from video"), L"Ctrl-Right");
		_hkeys[idAndType(GLOBAL_PLAY_PAUSE, GLOBAL_HOTKEY)] = hdata(_("Play / Pause"), L"Alt-Space");
		_hkeys[idAndType(GLOBAL_PREVIOUS_FRAME, GLOBAL_HOTKEY)] = hdata(_("Previous frame"), L"Left");
		_hkeys[idAndType(GLOBAL_NEXT_FRAME, GLOBAL_HOTKEY)] = hdata(_("Next frame"), L"Right");
		_hkeys[idAndType(GLOBAL_PREVIOUS_LINE, GLOBAL_HOTKEY)] = hdata(_("Previous line"), L"Ctrl-Up");//góra
		_hkeys[idAndType(GLOBAL_NEXT_LINE, GLOBAL_HOTKEY)] = hdata(_("Next line"), L"Ctrl-Down");//dół
		_hkeys[idAndType(GLOBAL_FIND_NEXT, GLOBAL_HOTKEY)] = hdata(_("Find next"), L"F3");
		_hkeys[idAndType(GLOBAL_JOIN_WITH_PREVIOUS, GLOBAL_HOTKEY)] = hdata(_("Merge with previous line"), L"F4");
		_hkeys[idAndType(GLOBAL_JOIN_WITH_NEXT, GLOBAL_HOTKEY)] = hdata(_("Merge with next line"), L"F5");
		_hkeys[idAndType(GLOBAL_SNAP_WITH_START, GLOBAL_HOTKEY)] = hdata(_("Change start time to nearest keyframe"), L"Shift-Left");//lewo
		_hkeys[idAndType(GLOBAL_SNAP_WITH_END, GLOBAL_HOTKEY)] = hdata(_("Change end time to nearest keyframe"), L"Shift-Right");//prawo
		_hkeys[idAndType(GLOBAL_NEXT_TAB, GLOBAL_HOTKEY)] = hdata(_("Next tab"), L"Ctrl-PgDn");
		_hkeys[idAndType(GLOBAL_PREVIOUS_TAB, GLOBAL_HOTKEY)] = hdata(_("Previous tab"), L"Ctrl-PgUp");
		_hkeys[idAndType(GLOBAL_SELECT_FROM_VIDEO, GLOBAL_HOTKEY)] = hdata(_("Select line at current video position"), L"F2");
		_hkeys[idAndType(GLOBAL_HELP, GLOBAL_HOTKEY)] = hdata(_("Help (not available in English)"), L"F1");
		_hkeys[idAndType(GRID_DUPLICATE_LINES, GRID_HOTKEY)] = hdata(_("Duplicate lines"), L"Ctrl-D");
		_hkeys[idAndType(GRID_COPY_COLUMNS, GRID_HOTKEY)] = hdata(_("Copy columns"), L"Ctrl-Shift-C");
		_hkeys[idAndType(GRID_PASTE_COLUMNS, GRID_HOTKEY)] = hdata(_("Paste columns"), L"Ctrl-Shift-V");
		_hkeys[idAndType(GRID_SHOW_PREVIEW, GRID_HOTKEY)] = hdata(_("Show subtitles preview"), L"Ctrl-Q");
		_hkeys[idAndType(VIDEO_PLAY_PAUSE, VIDEO_HOTKEY)] = hdata(_("Play / Pause"), L"Space");
		_hkeys[idAndType(VIDEO_5_SECONDS_FORWARD, VIDEO_HOTKEY)] = hdata(_("5 seconds forward"), L"L");
		_hkeys[idAndType(VIDEO_5_SECONDS_BACKWARD, VIDEO_HOTKEY)] = hdata(_("5 seconds backward"), L";");
		_hkeys[idAndType(VIDEO_MINUTE_FORWARD, VIDEO_HOTKEY)] = hdata(_("1 minute forward"), L"Up");
		_hkeys[idAndType(VIDEO_MINUTE_BACKWARD, VIDEO_HOTKEY)] = hdata(_("1 minute backward"), L"Down");
		_hkeys[idAndType(VIDEO_NEXT_FILE, VIDEO_HOTKEY)] = hdata(_("Next file"), L".");
		_hkeys[idAndType(VIDEO_PREVIOUS_FILE, VIDEO_HOTKEY)] = hdata(_("Previous file"), L",");
		_hkeys[idAndType(VIDEO_VOLUME_PLUS, VIDEO_HOTKEY)] = hdata(_("Volume up"), L"Num .");
		_hkeys[idAndType(VIDEO_VOLUME_MINUS, VIDEO_HOTKEY)] = hdata(_("Volume down"), L"Num 0");
		_hkeys[idAndType(VIDEO_NEXT_CHAPTER, VIDEO_HOTKEY)] = hdata(_("Next chapter"), L"M");
		_hkeys[idAndType(VIDEO_PREVIOUS_CHAPTER, VIDEO_HOTKEY)] = hdata(_("Previous chapter"), L"N");
		_hkeys[idAndType(EDITBOX_INSERT_BOLD, EDITBOX_HOTKEY)] = hdata(_("Add bold"), L"Ctrl-B");
		_hkeys[idAndType(EDITBOX_INSERT_ITALIC, EDITBOX_HOTKEY)] = hdata(_("Add italic"), L"Ctrl-I");
		_hkeys[idAndType(EDITBOX_SPLIT_LINE, EDITBOX_HOTKEY)] = hdata(_("Add line wrap"), L"Shift-Enter");
		_hkeys[idAndType(EDITBOX_START_DIFFERENCE, EDITBOX_HOTKEY)] = hdata(_("Insert difference from the start"), L"Ctrl-,");
		_hkeys[idAndType(EDITBOX_END_DIFFERENCE, EDITBOX_HOTKEY)] = hdata(_("Insert difference to the end"), L"Ctrl-.");
		_hkeys[idAndType(EDITBOX_FIND_NEXT_DOUBTFUL, EDITBOX_HOTKEY)] = hdata(_("Next unconfirmed line"), L"Ctrl-D");
		_hkeys[idAndType(EDITBOX_FIND_NEXT_UNTRANSLATED, EDITBOX_HOTKEY)] = hdata(_("Next untranslated line"), L"Ctrl-R");
		_hkeys[idAndType(EDITBOX_SET_DOUBTFUL, EDITBOX_HOTKEY)] = hdata(_("Mark as unconfirmed and go to the next line"), L"Alt-Down");
		_hkeys[idAndType(EDITBOX_COMMIT, EDITBOX_HOTKEY)] = hdata(_("Apply changes"), L"Ctrl-Enter");
		_hkeys[idAndType(EDITBOX_COMMIT_GO_NEXT_LINE, EDITBOX_HOTKEY)] = hdata(_("Apply the changes and go to the next line"), L"Enter");
	}
	else{
		_hkeys[idAndType(AUDIO_COMMIT, AUDIO_HOTKEY)] = hdata(_("Commit"), L"Enter");
		_hkeys[idAndType(AUDIO_COMMIT_ALT, AUDIO_HOTKEY)] = hdata(_("Commit alt"), L"G");
		_hkeys[idAndType(AUDIO_PREVIOUS, AUDIO_HOTKEY)] = hdata(_("Previous line"), L"Left");
		_hkeys[idAndType(AUDIO_PREVIOUS_ALT, AUDIO_HOTKEY)] = hdata(_("Previous line alt"), L"Z");
		_hkeys[idAndType(AUDIO_NEXT, AUDIO_HOTKEY)] = hdata(_("Next line"), L"Right");
		_hkeys[idAndType(AUDIO_NEXT_ALT, AUDIO_HOTKEY)] = hdata(_("Next line alt"), L"X");
		_hkeys[idAndType(AUDIO_PLAY, AUDIO_HOTKEY)] = hdata(_("Play"), L"Down");
		_hkeys[idAndType(AUDIO_PLAY_ALT, AUDIO_HOTKEY)] = hdata(_("Play alt"), L"S");
		_hkeys[idAndType(AUDIO_PLAY_LINE, AUDIO_HOTKEY)] = hdata(_("Play line"), L"Up");
		_hkeys[idAndType(AUDIO_PLAY_LINE_ALT, AUDIO_HOTKEY)] = hdata(_("Play line alt"), L"R");
		_hkeys[idAndType(AUDIO_STOP, AUDIO_HOTKEY)] = hdata(_("Stop"), L"H");
		_hkeys[idAndType(AUDIO_GOTO, AUDIO_HOTKEY)] = hdata(_("Go to selection"), L"B");
		_hkeys[idAndType(AUDIO_SCROLL_RIGHT, AUDIO_HOTKEY)] = hdata(_("Scroll left"), L"A");
		_hkeys[idAndType(AUDIO_SCROLL_LEFT, AUDIO_HOTKEY)] = hdata(_("Scroll right"), L"F");
		_hkeys[idAndType(AUDIO_PLAY_BEFORE_MARK, AUDIO_HOTKEY)] = hdata(_("Play before the marker"), L"Num 0");
		_hkeys[idAndType(AUDIO_PLAY_AFTER_MARK, AUDIO_HOTKEY)] = hdata(_("Play after the marker"), L"Num .");
		_hkeys[idAndType(AUDIO_PLAY_500MS_FIRST, AUDIO_HOTKEY)] = hdata(_("Play first 500ms"), L"E");
		_hkeys[idAndType(AUDIO_PLAY_500MS_LAST, AUDIO_HOTKEY)] = hdata(_("Play last 500ms"), L"D");
		_hkeys[idAndType(AUDIO_PLAY_500MS_BEFORE, AUDIO_HOTKEY)] = hdata(_("Play 500ms before"), L"Q");
		_hkeys[idAndType(AUDIO_PLAY_500MS_AFTER, AUDIO_HOTKEY)] = hdata(_("Play 500ms after"), L"W");
		_hkeys[idAndType(AUDIO_PLAY_TO_END, AUDIO_HOTKEY)] = hdata(_("Play to the end"), L"T");
		_hkeys[idAndType(AUDIO_LEAD_IN, AUDIO_HOTKEY)] = hdata(_("Add lead-in"), L"C");
		_hkeys[idAndType(AUDIO_LEAD_OUT, AUDIO_HOTKEY)] = hdata(_("Add lead-out"), L"V");
	}
}

Hotkeys::~Hotkeys()
{
	keys.clear();
	hkeys.clear();
	if (hotkeysNaming){
		delete hotkeysNaming;
		hotkeysNaming = nullptr;
	}
}

int Hotkeys::LoadHkeys(bool Audio)
{
	if (keys.empty()){ FillTable(); }
	OpenWrite ow;
	wxString hkpath = (Audio) ? L"/AudioHotkeys.txt" : L"/Hotkeys.txt";
	wxString acctxt;
	//it failed when there was lack of path
	//need to put everything in right order, options are first
	if (!ow.FileOpen(Options.configPath + hkpath, &acctxt, true)){ 
		LoadDefault(hkeys, Audio); 
		return 1; 
	}

	wxStringTokenizer hk(acctxt, L"\n", wxTOKEN_STRTOK);

	bool checkVer = false;
	if (acctxt.StartsWith(L"[")){
		wxString token = hk.NextToken();
		wxString ver = token.BeforeFirst(L']').Mid(1);
		/*if (ver != Options.progname){
			LoadDefault(hkeys, Audio);
		}*/
		int first = token.find(L".");//0.8.0.build
		if (first > -1){
			wxString ver = token.Mid(first + 5).BeforeFirst(L' ');
			int version = wxAtoi(ver);
			checkVer = (version > 487);
			if (version < 1141){
				acctxt = acctxt.AfterFirst(L'\n');
				ConfigConverter::Get()->ConvertHotkeys(&acctxt);
				ow.FileWrite(Options.configPath + hkpath, acctxt);
			}
		}
	}
	if (!checkVer){
		KaiMessageBox(_("The shortcuts file is outdated and will be replaced with the default one"));
		LoadDefault(hkeys, Audio);
		SaveHkeys(Audio);
		return 1;
	}

	int g = 0;
	lastScriptId = 30100;
	while (hk.HasMoreTokens())
	{
		wxString token = hk.NextToken();
		token.Trim(false);
		token.Trim(true);
		if (token.StartsWith(L"Script")){
			wxString rest;
			wxString name = token.BeforeFirst(L'=', &rest).Trim(false).Trim(true);
			rest = rest.Trim(false).Trim(true);
			if (rest.IsEmpty()){ continue; }
			hkeys[idAndType(lastScriptId, GLOBAL_HOTKEY)] = hdata(name, rest);
			lastScriptId++;
			g++;
			continue;
		}

		wxString Values = token.AfterFirst(L' ').Trim(false).Trim(true);
		int type = wxString(L"GSEVA").find(Values[0]);
		Values = Values.Remove(0, 2);
		wxString Labels = token.BeforeFirst(L' ').Trim(false).Trim(true);
		if (Values != emptyString){
			if (Labels.IsNumber()){
				hkeys[idAndType(wxAtoi(Labels), type)] = Values;
			}
			else{
				hkeys[idAndType(GetIdValue(Labels.data()), type)] = Values;
			}
			g++;
		}
	}
	if (g > 10){ 
		if (Audio){ AudioKeys = true; }
		return 1; 
	}
	LoadDefault(hkeys, Audio);
	if (hkeys.size() > 10){ return 1; }
	return 0;

}

void Hotkeys::SaveHkeys(bool Audio)
{
	wxString gseva = L"GSEVA";
	wxString Texthk = L"[" + Options.progname + L"]\r\n";
	for (std::map<idAndType, hdata>::iterator cur = hkeys.begin(); cur != hkeys.end(); cur++) {
		if ((!Audio && cur->first.Type == AUDIO_HOTKEY) ||
			(Audio && !(cur->first.Type == AUDIO_HOTKEY)) ||
			cur->first.id < 100 || cur->second.Accel.empty()) {
			continue;
		}
		if (cur->first >= 30100){ Texthk << cur->second.Name << L"=" << cur->second.Accel << L"\r\n"; }
		else{
			wxString idstring = GetString((Id)cur->first.id);
			if (idstring == emptyString){ idstring << cur->first.id; }
			Texthk << idstring << L" " << gseva[cur->first.Type] << L"=" << cur->second.Accel << L"\r\n";
		}
	}
	OpenWrite ow;
	wxString hkpath = (Audio) ? L"/AudioHotkeys.txt" : L"/Hotkeys.txt";
	ow.FileWrite(Options.configPath + hkpath, Texthk);

}
//itype is needed, data to avoid additional seeking
wxAcceleratorEntry Hotkeys::GetHKey(const idAndType itype, const hdata *data)
{

	wxAcceleratorEntry accelkey;
	wxString accel;
	if (!data){
		auto ahkey = hkeys.find(itype);
		if (!(ahkey != hkeys.end())){ return accelkey; }
		accel = ahkey->second.Accel;
	}
	else{
		accel = data->Accel;
	}



	if (accel == emptyString){ return accelkey;/*ResetKey(itemid);accel=hkeys[idAndType(itemid];*/ }
	int modif = GetModifier(accel);

	wxString akey = (accel.EndsWith(L"-")) ? wxString(L"-") : accel.AfterLast(L'-');
	int key = 0;

	std::map<int, wxString>::iterator cur;
	for (cur = keys.begin(); cur != keys.end(); cur++) {
		if (cur->second == akey) {
			key = cur->first;
			break;
		}
	}

	if (key == 0 && akey.length() < 2){ key = static_cast<int>(akey[0]); }
	else if (key == 0){
		KaiLog(wxString::Format(_("Shortcut \"%s\" is invalid"), akey));
	}
	accelkey.Set(modif, key, (itype.id < AUDIO_COMMIT) ? itype.id + 10 : itype.id);

	return accelkey;
}

void Hotkeys::SetHKey(const idAndType &itype, wxString name, wxString hotkey)
{
	hkeys[itype] = hdata(name, hotkey);
}

wxString Hotkeys::GetStringHotkey(const idAndType &itype, const wxString &name)
{
	auto it = hkeys.find(itype);
	if (it != hkeys.end()){ if (name != emptyString){ it->second.Name = name; } return it->second.Accel; }
	return emptyString;
}

void Hotkeys::ResetKey(const idAndType *itype, int id, char type)
{
	idAndType tmpitype = (itype) ? *itype : idAndType(id, type);
	std::map<idAndType, hdata> tmphkeys;
	LoadDefault(tmphkeys); 
	LoadDefault(tmphkeys, true);
	auto it = tmphkeys.find(tmpitype);
	if (it != tmphkeys.end())
	{
		hkeys[tmpitype] = it->second;
	}
	else{
		KaiLog(wxString::Format(_("This setting does not have a default hotkey with id %i to revert to"), tmpitype.id));
	}
}

wxString Hotkeys::GetDefaultKey(const idAndType &itype)
{
	std::map<idAndType, hdata> tmphkeys;
	LoadDefault(tmphkeys); LoadDefault(tmphkeys, true);
	auto it = tmphkeys.find(itype);
	if (it != tmphkeys.end())
	{
		return it->second.Accel;
	}
	return emptyString;
}

void Hotkeys::OnMapHkey(int id, wxString name, wxWindow *parent, char hotkeyWindow /*= GLOBAL_HOTKEY*/, bool showWindowSelection/*=true*/)
{
	//sanity check
	if (id < 0 && name.empty())
		return;

	HkeysDialog *hkd = new HkeysDialog(parent, (name.empty()) ? GetName(id) : name, hotkeyWindow, showWindowSelection);
	if (hkd->ShowModal() == wxID_OK){
		lastScriptId = 30100;
		std::vector< std::map<idAndType, hdata>::iterator> idtypes;
		for (auto cur = hkeys.begin(); cur != hkeys.end(); cur++)
		{
			if (id < 0 && cur->first.id >= 30100){
				if (cur->second.Name == name){
					id = cur->first.id;
				}
				lastScriptId++;
			}
			if (cur->second.Accel == hkd->hotkey){
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
			wxString windowNames[] = { _("Global"), _("Subtitles"), _("Editor"), _("Video"), _("Audio") };
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
					wxString::Format(_("This hotkey already exists for \"%s\".\nWhat to do?"),
					doubledHkName), _("Warning"), wxYES | wxOK | wxCANCEL);
				msg.SetOkLabel(_("Switch hotkeys"));
				msg.SetYesLabel(_("Delete hotkey"));
				result = msg.ShowModal();
			}
			else{
				int buttonFlag = (idtypes.size() < 2) ? wxOK : 0;
				KaiMessageDialog msg(parent,
					wxString::Format(_("This shortcut already exists in %s as a shortcut for \"%s\".\nWhat would you like to do?"),
					(idtypes.size() > 1) ? _("other windows") : _("another window"), doubledHkName),
					_("Warning"), wxYES_NO | buttonFlag | wxCANCEL);
				if (idtypes.size() < 2)
					msg.SetOkLabel(_("Switch hotkeys"));
				msg.SetYesLabel(_("Delete hotkey"));
				msg.SetNoLabel(_("Set anyway"));
				result = msg.ShowModal();
			}
			if (result == wxCANCEL){ return; }
			else if (result != wxNO){
				for (auto &idtype : idtypes){
					if (doubledHotkey && idtype->first.Type != hkd->type)
						continue;

					if (result == wxOK){
						auto finditer = hkeys.find(idAndType(id, hkd->type));
						idtype->second.Accel = emptyString;
						idtype->second.Accel = (finditer != hkeys.end()) ? finditer->second.Accel : emptyString;
					}
					else if (result == wxYES){
						idtype->second.Accel = emptyString;
					}
				}
			}
		}
		SetHKey(idAndType(id, hkd->type), hkd->hkname, hkd->hotkey);
		SetAccels(true);
		SaveHkeys();
		if (saveAudioHotkeys)
			SaveHkeys(true);
	}
	hkd->Destroy();
}

void Hotkeys::SetAccels(bool all){
	Notebook *Tabs = Notebook::GetTabs();
	KainoteFrame * frame = (KainoteFrame *)Tabs->GetParent();
	if (frame)
		frame->SetAccels(all);
}

wxString Hotkeys::GetName(const idAndType itype)
{
	auto it = hkeys.find(itype);
	if (it != hkeys.end())
	{
		return it->second.Name;
	}

	return emptyString;
}

const wxString & Hotkeys::GetName(int id)
{
	if (!hotkeysNaming)
		hotkeysNaming = new HotkeysNaming();

	return hotkeysNaming->GetName(id);
}

int Hotkeys::GetType(int id)
{
	if (id < VIDEO_PLAY_PAUSE)
		return AUDIO_HOTKEY;
	else if (id < EDITBOX_CHANGE_FONT)
		return VIDEO_HOTKEY;
	else if (id < GRID_HIDE_LAYER || (id >= EDITBOX_TAG_BUTTON1 && id <= EDITBOX_TAG_BUTTON20))
		return EDITBOX_HOTKEY;
	else if (id < GLOBAL_SAVE_SUBS)
		return GRID_HOTKEY;

	return GLOBAL_HOTKEY;
}

const std::map<int, wxString> & Hotkeys::GetNamesTable()
{
	if (!hotkeysNaming)
		hotkeysNaming = new HotkeysNaming();

	return hotkeysNaming->GetNamesTable();
}

void Hotkeys::ResetDefaults()
{
	std::map<idAndType, hdata> defaultHotkeys;
	LoadDefault(defaultHotkeys);
	LoadDefault(defaultHotkeys, true);
	hkeys = defaultHotkeys;
}

int Hotkeys::GetModifier(const wxString& accel)
{
	int modif = 0;
	if (accel.Find(L"Alt-") != -1) {
		modif |= 1;
	}
	if (accel.Find(L"Shift-") != -1) {
		modif |= 4;
	}
	if (accel.Find(L"Ctrl-") != -1) {
		modif |= 2;
	}
	return modif;
}

//Dialog window catching keyboard shortcuts
//blocking also access to options
HkeysDialog::HkeysDialog(wxWindow *parent, wxString name, char hotkeyWindow, bool showWindowSelection)
	: KaiDialog(parent, -1, _("Hotkey mapping"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxWANTS_CHARS | wxCLOSE_BOX)
{
	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	global = nullptr;
	const int elems = 5;
	wxString windows[elems] = { _("Global hotkey"), _("Subtitle grid hotkey"),
		_("Edit box hotkey"), _("Video hotkey"), _("Audio hotkey") };
	if (showWindowSelection /*&& hotkeyWindow == GLOBAL_HOTKEY*/){
		global = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, elems, windows, wxWANTS_CHARS);
		global->SetSelection(hotkeyWindow);
		global->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress, 0, this);
	}
	KaiStaticText *txt = new KaiStaticText(this, -1, 
		wxString::Format(_("Please enter a hotkey for \"%s\"."), name),
		wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	txt->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&HkeysDialog::OnKeyPress, 0, this);

	DialogSizer *MainSizer = new DialogSizer(wxVERTICAL);
	if (global){ MainSizer->Add(global, 0, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 12); }
	MainSizer->Add(txt, 0, wxALL, 12);
	SetSizerAndFit(MainSizer);
	MoveToMousePosition(this);
	type = hotkeyWindow;
	hkname = name;
}

HkeysDialog::~HkeysDialog()
{
}

void HkeysDialog::OnKeyPress(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if (key == 0){ key = event.GetUnicodeKey(); }
	if (global) {
		type = global->GetSelection();
	}
	hotkey = emptyString;
	if (key != WXK_SHIFT && key != WXK_ALT && key != WXK_CONTROL){

		if (event.AltDown()){ hotkey << L"Alt-"; }
		if (event.ControlDown()){ hotkey << L"Ctrl-"; }
		if (event.ShiftDown()){ hotkey << L"Shift-"; }

		if (hotkey == emptyString && (type == GLOBAL_HOTKEY || type == EDITBOX_HOTKEY) && (key > 30 && key < 127 /*|| key>313 && key<318*/))
		{
			KaiMessageBox(_("Global and editor shortcuts must include modifiers (e.g. Shift, Ctrl, Alt).")); return;
		}
		else if (event.GetModifiers() == wxMOD_CONTROL && (key == L'V' || key == L'C' || key == L'X' || key == L'Z')){
			KaiMessageBox(_("You cannot use shortcuts for copying, cutting, and pasting.")); return;
		}
		else if (event.AltDown() && !event.ControlDown() && key == WXK_F4){
			KaiMessageBox(_("You cannot use the program exit shortcut.")); return;
		}

		wxString keytxt = Hkeys.keys[key];
		if (keytxt == emptyString){
			if (key >= 36 && key <= 96)
				keytxt = wchar_t(key);
			else
				return;
		}
		hotkey << keytxt;

		EndModal(wxID_OK);
	}
}

Hotkeys Hkeys;

DEFINE_ENUM(Id, IDS)

