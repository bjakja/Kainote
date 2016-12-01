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

#ifndef OPTIONSDIALOG
#define OPTIONSDIALOG

#include <wx/wx.h>
#include <wx/treebook.h>
#include <wx/listctrl.h>
#include "ListControls.h"
#include <vector>
class kainoteFrame;

class OptionsBind {
public:
	wxWindow *ctrl;
	wxString option;
};


class OptionsDialog : public wxDialog
{
public:
	OptionsDialog(wxWindow *parent, kainoteFrame *kaiparent);
	virtual ~OptionsDialog();
	wxTreebook *OptionsTree;
	wxListCtrl *Shortcuts;
	KaiChoice* Stylelist;
	KaiChoice* Katlist;
	wxButton *okok;

private:
	std::vector<OptionsBind> handles;

	void ConOpt(wxWindow *ctrl,wxString option);
	void OnSaveClick(wxCommandEvent& event);
	void SetOptions(bool saveall=true);
	void OnMapHkey(wxListEvent& event);
	void OnResetHkey(wxListEvent& event);
	//void OnKeyPress(wxKeyEvent& event);
	void OnChangeCatalog(wxCommandEvent& event);

	kainoteFrame *Kai;

	unsigned char hkeymodif;
};

enum{
	ID_BOK=12233,
	ID_BCOMMIT
};



#endif