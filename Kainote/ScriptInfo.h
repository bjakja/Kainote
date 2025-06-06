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
#include "NumCtrl.h"
#include "MappedButton.h"
#include "ListControls.h"
#include "KaiCheckBox.h"


class ScriptInfo : public KaiDialog
{
public:

	ScriptInfo(wxWindow* parent, int w, int h);
	virtual ~ScriptInfo();

	NumCtrl* height;
	KaiTextCtrl* script;
	NumCtrl* width;
	KaiTextCtrl* update;
	MappedButton* save;
	KaiChoice* wrapstyle;
	KaiChoice* matrix;
	KaiCheckBox* scaleBorderAndShadow;
	KaiChoice* collision;
	KaiTextCtrl* editing;
	KaiTextCtrl* title;
	MappedButton* cancel;
	KaiTextCtrl* timing;
	KaiTextCtrl* translation;
	MappedButton* resolutionFromVideo;

	void DoTooltips();
private:
	wxSize res;
	void OnVideoRes(wxCommandEvent& event);
};


