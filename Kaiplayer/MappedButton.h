#ifndef _MAPPED_BUTTON_
#define _MAPPED_BUTTON_

#include "wx/button.h"

class MappedButton :public wxButton
{
public:
	MappedButton(wxWindow *parent, int id, const wxString& label = wxEmptyString, const wxString& tooltip = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	virtual ~MappedButton();
	void SetToolTip(const wxString &toolTip="");
private:
	void OnLeftClick(wxMouseEvent &evt);
	//wxString name;
};

#endif