#ifndef NUMCTRL
#define NUMCTRL

#include <wx/wx.h>
wxDECLARE_EVENT(NUMBER_CHANGED, wxCommandEvent);

class NumCtrl : public wxTextCtrl
	{
	public:
		NumCtrl(wxWindow *parent,long id,wxString text, int rangefrom, int rangeto, bool intonly,
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
		int rfrom;
		int rto;
		bool oint;
		double value;
		wxString oldval;
		bool holding;
		int oldpos;
		int oldposx;
		int curpos;

		DECLARE_EVENT_TABLE()
	};

#endif