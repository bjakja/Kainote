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

#ifndef __HOTKEYS__
#define __HOTKEYS__
//#include <wx/string.h>
//#include <wx/accel.h>
//#include <wx/arrstr.h>
#include <wx/wx.h>
#include <map>
#include "ListControls.h"

enum{
	GLOBAL_HOTKEY='G',
	GRID_HOTKEY='N',
	EDITBOX_HOTKEY='E',
	VIDEO_HOTKEY='W',
	AUDIO_HOTKEY='A'
};
class idAndType{
public:
	idAndType(int _id=0, char _type='G'){id= _id; Type= _type;}
	bool operator < (const idAndType match){ return id < match.id;};
	bool operator > (const idAndType match){ return id > match.id;};
	bool operator <= (const idAndType match){ return id <= match.id;};
	bool operator >= (const idAndType match){ return id >= match.id;};
	bool operator == (const idAndType match){ return id == match.id;};
	bool operator != (const idAndType match){ return id != match.id;};
	int id;
	char Type;
};

bool operator < (const idAndType match, const idAndType match1);
bool operator > (const idAndType match, const idAndType match1);
bool operator <= (const idAndType match, const idAndType match1);
bool operator >= (const idAndType match, const idAndType match1);
bool operator == (const idAndType match, const idAndType match1);
bool operator != (const idAndType match, const idAndType match1);
bool operator == (const idAndType &match, const int match1);
bool operator == (const int match1 ,const idAndType &match);
bool operator >= (const idAndType &match, const int match1);
bool operator >= ( const int match1, const idAndType &match);
bool operator <= (const idAndType &match, const int match1);
bool operator <= (const int match1 ,const idAndType &match);
bool operator > (const idAndType &match, const int match1);
bool operator > ( const int match1, const idAndType &match);
bool operator < (const idAndType &match, const int match1);
bool operator < (const int match1 ,const idAndType &match);
bool operator != (const idAndType &match, const int match1);
bool operator != ( const int match1, const idAndType &match);

class hdata{
public:
	hdata(wxString accName, wxString _Accel){
		Name=accName; Accel=_Accel;
	}
	hdata(wxString acc){
		Accel=acc;
	}
	hdata(){}
	wxString Name;
	wxString Accel;
};

class HkeysDialog : public wxDialog
{
	public:
	HkeysDialog(wxWindow *parent, wxString name, char hotkeyWindow = GLOBAL_HOTKEY, bool showWindowSelection=true);
	virtual ~HkeysDialog();
	wxString hotkey;
	wxString hkname;
	KaiChoice *global;
	char type;

	private:
	void OnKeyPress(wxKeyEvent& event);
};

class Hotkeys
{
private:

public:
	Hotkeys();
	~Hotkeys();
	int LoadHkeys(bool Audio=false);
	void LoadDefault(std::map<idAndType, hdata> &_hkeys, bool Audio=false);
	void SaveHkeys(bool Audio=false);
	void SetHKey(const idAndType &itype, wxString name, wxString hotkey);
	wxAcceleratorEntry GetHKey(const idAndType itype, const hdata *hkey=0);
	wxString GetMenuH(const idAndType &itype, const wxString &name="");
	void FillTable();
	void ResetKey(const idAndType *itype, int id=0, char type='G');
	int OnMapHkey(int id, wxString name,wxWindow *parent, char hotkeyWindow = GLOBAL_HOTKEY, bool showWindowSelection=true);
	void SetAccels(bool all=false);
	std::map<idAndType, hdata> hkeys;
	std::map<int, wxString> keys;
	bool AudioKeys;

};


	extern Hotkeys Hkeys;

#endif