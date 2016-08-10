#ifndef STYLELIST
#define STYLELIST

#include <wx/wx.h>
#include <vector>
#include "Styles.h"


class StyleList : public wxWindow
{
	public:
		StyleList(wxWindow *parent, long id, std::vector<Styles*> *stylearray, wxComboBox *_fontseeker, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxWANTS_CHARS);
		virtual ~StyleList();

		void SetSelection(int sel,bool reset=false);
		void Scroll(int step);
		int GetSelections(wxArrayInt &sels);
		void SetArray(std::vector<Styles*> *stylearray);

	private:

		void OnPaint(wxPaintEvent& event);
		void OnSize(wxSizeEvent& event);
		void DrawFld(wxDC &dc,int w, int h);
		void OnScroll(wxScrollWinEvent& event);
		void OnMouseEvent(wxMouseEvent& event);
		void OnArrow(wxCommandEvent& event);

		int lastsel;
		int lastRow;
		bool Switchlines;
		wxArrayInt sels;
		int scPos;
		int Height;
		bool holding;
		std::vector<Styles*> *stylenames;
		wxComboBox *fontseeker;

		//wxScrollBar *scrollBar;
		wxBitmap *bmp;
		wxFont font;

		DECLARE_EVENT_TABLE()
};


#endif