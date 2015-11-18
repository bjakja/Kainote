
#ifndef PROGRESSDIALOG
#define PROGRESSDIALOG

#include <wx/wx.h>
#include <wx/gauge.h>

struct ITaskbarList3;
class ProgresDialog : public wxDialog
{
private:
	wxGauge *gauge;
	wxStaticText *text;
	wxStaticText *text1;
	
	bool canceled;
	int oldtime;
	void OnCancel(wxCommandEvent& event);
	//ProgressThread *thread;
	int firsttime;
public:
	ProgresDialog(wxWindow *parent, wxString title, wxPoint pos=wxDefaultPosition, wxSize size=wxDefaultSize, int style=0);
	virtual ~ProgresDialog();

	void Progress(int num);
	void Title(wxString title);
	bool WasCancelled();
	wxButton *cancel;
	ITaskbarList3 *taskbar;
};



#endif