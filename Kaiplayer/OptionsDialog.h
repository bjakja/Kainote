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
#include "MappedButton.h"
#include "KaiTreebook.h"
#include "KaiListCtrl.h"
#include "KaiDialog.h"
#include "config.h"
#include <vector>
class kainoteFrame;

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

private:
	std::vector<OptionsBind> handles;

	void ConOpt(wxWindow *ctrl,CONFIG option);
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
};

enum{
	ID_BOK=12233,
	ID_BCOMMIT
};



#endif