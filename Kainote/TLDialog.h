//  Copyright (c) 2016 - 2020, Marcin Drob

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
#include "MappedButton.h"

#include "SubsGrid.h"

class TLDialog  : public KaiDialog
{
public:
	TLDialog(wxWindow *parent, SubsGrid *subsgrid);
	virtual ~TLDialog();

	
	
	MappedButton *Down;
	MappedButton *Up;
	MappedButton *UpJoin;
	MappedButton *DownJoin;
	MappedButton *DownDel;
	MappedButton *UpExt;

private:
	void OnUp(wxCommandEvent& event);
	void OnDownJoin(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnUpJoin(wxCommandEvent& event);
	void OnUpExt(wxCommandEvent& event);
	void OnDownDel(wxCommandEvent& event);

	SubsGrid *Sbsgrid;
};


