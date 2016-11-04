#ifndef _MAPPED_BUTTON_
#define _MAPPED_BUTTON_

#include "wx/button.h"
#include "Hotkeys.h"

class MappedButton :public wxButton
{
public:
	MappedButton(wxWindow *parent, int id, const wxString& label = wxEmptyString, const wxString& tooltip = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int window = EDITBOX_HOTKEY, long style = 0);
	MappedButton(wxWindow *parent, int id, const wxBitmap& bitmap, const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize, int window = AUDIO_HOTKEY, long style = 0);
	void SetTwoHotkeys(){twoHotkeys=true;}
	virtual ~MappedButton();
	void SetToolTip(const wxString &toolTip="");
private:
	void OnLeftClick(wxMouseEvent &evt);
	int Window;
	bool twoHotkeys;
};

#endif