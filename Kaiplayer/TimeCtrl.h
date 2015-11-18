#ifndef TIMECTRL_H_INCLUDED
#define TIMECTRL_H_INCLUDED

#include <wx/wx.h>
#include "timeconv.h"


class TimeCtrl : public wxTextCtrl
{
    public:
	TimeCtrl(wxWindow* parent, const long int id, const wxString& val=_("0:00:00.00"), const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS,const wxValidator& validator=wxDefaultValidator, const wxString& name=wxTextCtrlNameStr);
	virtual ~TimeCtrl();

	void SetTime(STime newtime);
	STime GetTime();
	void ChangeFormat(char frm, float fps=0);


	private:
	char form;
	STime mTime;
	bool pastes;
	bool holding;
	int oldpos;
	int oldposx;
	int curpos;
	STime value;
	int grad;
	void OnTimeWrite(wxCommandEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyEvent(wxKeyEvent& event);
	void OnPaste(wxCommandEvent &event);
	void OnCopy(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
enum{
	Time_Copy=4404,
	Time_Paste=4405
};


#endif // TIMECTRL_H_INCLUDED
