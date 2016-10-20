#ifndef VIDEOSLIDER
#define VIDEOSLIDER
#include <wx/wx.h>
class VideoCtrl;

class VideoSlider : public wxWindow
{
public:
	VideoSlider(wxWindow *parent, const long int id ,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS, const wxString& name=wxPanelNameStr);
	virtual ~VideoSlider();

	void SetValue(float pos);
	VideoCtrl* VB;
	void SendTime(float pos);
protected:
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnMouseLeave(wxMouseCaptureLostEvent& event);
	void OnKeyPress(wxKeyEvent& event);
	void OnSize(wxSizeEvent& event);
	int position;
	int labelpos;
	wxString label;
	bool showlabel;
	bool block;
	bool holding;
	wxBitmap prb;
	wxBitmap prbh;
	bool onslider;
	DECLARE_EVENT_TABLE()
};

class VolSlider : public wxWindow
	{
public:
	VolSlider(wxWindow *parent, const long int id, int apos, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS, const wxString& name=wxPanelNameStr);
	virtual ~VolSlider();

	void SetValue(int pos);
	int GetValue();
	void OnMouseEvent(wxMouseEvent& event);
protected:
	void OnPaint(wxPaintEvent& event);
	
	int position;
	bool block;
	bool holding;
	bool onslider;
	wxBitmap start;
	wxBitmap end;
	wxBitmap prbh;
	wxBitmap* bmp;
	//bool blockpaint;
	DECLARE_EVENT_TABLE()
	};

#endif