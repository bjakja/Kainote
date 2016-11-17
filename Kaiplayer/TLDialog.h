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

#ifndef TLDIALOG
#define TLDIALOG

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include "Grid.h"

class TLDialog  : public wxDialog
{
public:
	TLDialog(wxWindow *parent, Grid *subsgrid);
	virtual ~TLDialog();

	
	
	wxButton *Down;
	wxButton *Up;
	wxButton *UpJoin;
	wxButton *DownJoin;
	wxButton *DownDel;
	wxButton *UpExt;

private:
	void OnUp(wxCommandEvent& event);
	void OnDownJoin(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnUpJoin(wxCommandEvent& event);
	void OnUpExt(wxCommandEvent& event);
	void OnDownDel(wxCommandEvent& event);

	Grid *Sbsgrid;
};


#endif