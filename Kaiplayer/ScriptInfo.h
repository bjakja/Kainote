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

#ifndef SCRIPTINFO_H
#define SCRIPTINFO_H


#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include "NumCtrl.h"


class ScriptInfo: public wxDialog
{
	public:

		ScriptInfo(wxWindow* parent,int w, int h);
		virtual ~ScriptInfo();

		NumCtrl* height;
		wxTextCtrl* script;
		NumCtrl* width;
		wxTextCtrl* update;
		wxButton* Button1;
		wxChoice* wrapstyle;
		wxCheckBox* CheckBox2;
		wxChoice* collision;
		wxTextCtrl* editing;
		wxTextCtrl* title;
		wxButton* Button2;
		wxCheckBox* CheckBox1;
		wxTextCtrl* timing;
		wxTextCtrl* translation;
		wxButton* Fvideo;

		void DoTooltips();
private:
		wxSize res;
		void OnVideoRes(wxCommandEvent& event);
};

#endif
