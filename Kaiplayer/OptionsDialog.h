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

#include <wx/wx.h>
#include "ListControls.h"
#include "MappedButton.h"
#include "KaiTreebook.h"
#include "KaiListCtrl.h"
#include "KaiDialog.h"
#include "config.h"
#include <vector>
class kainoteFrame;

class ItemHotkey : public Item{
public:
	ItemHotkey(const wxString &txt, const wxString &_accel, const idAndType &Id) : Item(){name = txt; accel = _accel; hotkeyId = Id;}
	virtual ~ItemHotkey(){		
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed = NULL);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	wxString GetName(){return name;}
	void OnMapHotkey(KaiListCtrl *theList, int y);
	void OnResetHotkey(KaiListCtrl *theList, int y);
	void OnDeleteHotkey(KaiListCtrl *theList, int y);
	void Save();
	Item* Copy(){return new ItemHotkey(*this);}
	idAndType hotkeyId;
	wxString accel;
};

class OptionsBind {
public:
	wxWindow *ctrl;
	CONFIG option;
};


class OptionsDialog : public KaiDialog
{
public:
	OptionsDialog(wxWindow *parent, kainoteFrame *kaiparent);
	virtual ~OptionsDialog();
	KaiTreebook *OptionsTree;
	KaiListCtrl *Shortcuts;
	KaiChoice* Stylelist;
	KaiChoice* Katlist;
	MappedButton *okok;
	void ConOpt(wxWindow *ctrl,CONFIG option);
	static wxString *windowNames;
	static std::map<idAndType, hdata> hotkeysCopy;
private:
	std::vector<OptionsBind> handles;

	void OnSaveClick(wxCommandEvent& event);
	void SetOptions(bool saveall=true);
	void OnMapHkey(wxCommandEvent& event);
	void OnResetHkey(wxCommandEvent& event);
	void OnDeleteHkey(wxCommandEvent& event);
	//void OnKeyPress(wxKeyEvent& event);
	void ChangeColors();
	void OnChangeCatalog(wxCommandEvent& event);

	kainoteFrame *Kai;

	unsigned char hkeymodif;
	std::vector<bool> registeredExts;
};

enum{
	ID_BCOMMIT = 12233
};



