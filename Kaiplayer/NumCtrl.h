#ifndef NUMCTRL
#define NUMCTRL

#include <wx/wx.h>
wxDECLARE_EVENT(NUMBER_CHANGED, wxCommandEvent);

wxString getdouble(double num);

class NumCtrl : public wxTextCtrl
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
        //void OnKeyEvent(wxKeyEvent& event);
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