#ifndef STYLEPREVIEW
#define STYLEPREVIEW


#include <wx/wx.h>
#include "styles.h"
#include <vector>
typedef void csri_inst;
typedef void csri_rend;
//class VobsubApi;

class StylePreview : public wxWindow
	{
	public:

		StylePreview(wxWindow *parent, int id, const wxPoint& pos, const wxSize& size);
		virtual ~StylePreview();

		void DrawPreview(Styles *styl=NULL);



	private:
		csri_inst *instance;
		csri_rend *vobsub;
		wxMutex mutex;
		int pitch,width,height,b,g,r,b1,g1,r1;
		

		wxBitmap *bmpframe;
		wxTextCtrl *PrevText;
		Styles *styl;

		void OnPaint(wxPaintEvent& event);
		void SubsText(std::vector<byte> &buffer);
		void OnMouseEvent(wxMouseEvent& event);

		DECLARE_EVENT_TABLE()
	};

#endif