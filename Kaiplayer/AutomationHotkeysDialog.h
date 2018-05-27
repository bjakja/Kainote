//  Copyright (c) 2018, Marcin Drob

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
#include "KaiDialog.h"
#include "KaiListCtrl.h"
#include "Automation.h"
#include "hotkeys.h"

class AutomationHotkeysDialog : public KaiDialog
{
public:
	AutomationHotkeysDialog(wxWindow *parent, Auto::Automation *Auto);
	~AutomationHotkeysDialog();
	static std::map<idAndType, hdata> allHotkeys;
private:
	void OnOK(wxCommandEvent &evt);
	void OnMapHkey(wxCommandEvent &evt);
	void OnDeleteHkey(wxCommandEvent &evt);
	void ChangeHotkey(int row, int id, const wxString &hotkey);
	Auto::Automation *automation;
	KaiListCtrl *hotkeysList;
	int lastScriptId = 30100;
};

enum{
	ID_HOTKEYS_LIST = 12345,
	ID_HOTKEYS_OK,
	ID_HOTKEYS_MAP,
	ID_HOTKEYS_DELETE,
};