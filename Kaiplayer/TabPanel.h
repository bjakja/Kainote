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
#include "SubsGrid.h"
#include "Videobox.h"
#include "EditBox.h"
#include "ChangeTime.h"
#include <wx/statline.h>

class TabPanel : public wxWindow
	{
	public:
		TabPanel(wxWindow *parent,kainoteFrame *kai, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		virtual ~TabPanel();
		bool Hide();
		//bool Show(bool show=true);

		SubsGrid* Grid;
        EditBox* Edit;
		VideoCtrl* Video;
		ShiftTimesWindow* ShiftTimes;

		wxBoxSizer* BoxSizer1;
		wxBoxSizer* BoxSizer2;
		wxBoxSizer* BoxSizer3;

		void SetAccels();
		void SetVideoWindowSizes(int w, int h);

		bool editor;
		

		wxString SubsName;
		wxString VideoName;
		wxString SubsPath;
		wxString VideoPath;
		wxWindow *lastFocusedWindow = NULL;

	private:
		wxDialog* sline;
		bool holding;
		void OnMouseEvent(wxMouseEvent& event);
		void OnFocus(wxChildFocusEvent& event);
		DECLARE_EVENT_TABLE()
	};

