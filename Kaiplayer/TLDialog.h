#ifndef TLDIALOG
#define TLDIALOG

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include "Grid.h"

class TLDialog  : public wxDialog
{
public:
	TLDialog(wxWindow *parent, Grid *subsgrid);
	virtual ~TLDialog();

	
	
	wxButton *Down;
	wxButton *Up;
	wxButton *UpJoin;
	wxButton *DownJoin;
	wxButton *DownDel;
	wxButton *UpExt;

private:
	void OnUp(wxCommandEvent& event);
	void OnDownJoin(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnUpJoin(wxCommandEvent& event);
	void OnUpExt(wxCommandEvent& event);
	void OnDownDel(wxCommandEvent& event);

	Grid *Sbsgrid;
};


#endif