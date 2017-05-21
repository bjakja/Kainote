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

#ifndef NUMCTRL
#define NUMCTRL

#include <wx/wx.h>
#include "KaiTextCtrl.h"
wxDECLARE_EVENT(NUMBER_CHANGED, wxCommandEvent);

wxString getdouble(double num);

class NumCtrl : public KaiTextCtrl
	{
	public:
		NumCtrl(wxWindow *parent,long id,wxString text, int rangefrom, int rangeto, bool intonly,
			const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0);
		NumCtrl(wxWindow *parent,long id,double value, double rangefrom, double rangeto, bool intonly,
			const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0);

		virtual ~NumCtrl();

		void SetString(wxString val);
		void SetInt(int val);
		void SetDouble(double val);
		wxString GetString();
		int GetInt();
		double GetDouble();

	private:

		void OnNumWrite(wxCommandEvent& event);
        void OnMouseEvent(wxMouseEvent &event);
        void OnPaste(wxCommandEvent& event);
		void OnMouseLost(wxMouseCaptureLostEvent& event);
		double rfrom;
		double rto;
		bool oint;
		double value;
		wxString oldval;
		bool holding;
		int oldpos;
		int oldposx;
		int curpos;
		/*bool isbad;*/
		DECLARE_EVENT_TABLE()
	};

#endif