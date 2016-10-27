
#ifndef PROGRESSDIALOG
#define PROGRESSDIALOG

#include <wx/wx.h>
#include <wx/gauge.h>
#include <functional>

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
	void OnShow(wxThreadEvent& evt);
	void OnProgress(wxThreadEvent& evt);
	void OnTitle(wxThreadEvent& evt);
	int firsttime;
	
public:
	ProgresDialog(wxWindow *parent, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, int style=0);
	virtual ~ProgresDialog();

	void Progress(int num);
	void Title(wxString title);
	bool WasCancelled();
	wxButton *cancel;
	ITaskbarList3 *taskbar;
	int result;
};

class ProgressSink : public wxThread
{
public:
	ProgressSink(wxWindow *parent, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, int style=0);
	virtual ~ProgressSink();
	//Pamiêtaj o poprawnej kolejnoœci, najpierw tworzysz sink
	//póŸniej ustawiasz zadanie b¹dŸ tworzysz w¹tek
	//kolejnie pokazujesz dialog 
	//a na koñcu czekasz na wykonanie zadania / w¹tku
	void SetAndRunTask(std::function<int()> _task)
	{
		task = _task;
		Create();
		Run();
	}
	void ShowDialog();
	int ShowSecondaryDialog(std::function<int()> dialfunction);
	void Title(wxString title);
	bool WasCancelled();
	void Progress(int num);
	void EndModal();
private:
	wxThread::ExitCode Entry();
	std::function<int()> task;
	ProgresDialog *dlg;
	
};



#endif