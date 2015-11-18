#ifndef BBUTTON
#define BBUTTON

#include <wx/wx.h>
#include <wx/statbmp.h>


class BitmapButton : public wxStaticBitmap
{
public:
	BitmapButton(wxWindow* parent, wxBitmap bitmap,wxBitmap bitmap1, int id, const wxPoint& pos, const wxSize& size);
    virtual ~BitmapButton();
	void ChangeBitmap(bool play);

private:
	wxBitmap bmp;
	wxBitmap bmp1;
	int idd;
	wxImage img;
	bool enter;
	void OnLeftDown(wxMouseEvent& event);
	

	DECLARE_EVENT_TABLE()
};

#endif