//  Copyright (c) 2018 - 2020, Marcin Drob

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
#include "ListControls.h"
#include "KaiTextCtrl.h"

class FPSDialog : public KaiDialog
{
public:
	FPSDialog(wxWindow *parent);
	virtual ~FPSDialog(){};
	void OkClick(wxCommandEvent &evt);
	double ofps, nfps;
	KaiChoice *oldfps;
	KaiChoice *newfps;
};

class TreeDialog : public KaiDialog
{
public:
	TreeDialog(wxWindow *parent, const wxString & currentName);
	virtual ~TreeDialog(){}
	wxString GetDescription();

private:
	void OkClick(wxCommandEvent &evt);
	KaiTextCtrl *treeDescription;
};