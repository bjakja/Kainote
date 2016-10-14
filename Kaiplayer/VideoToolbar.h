
#include <wx/wx.h>
#include <vector>

class VideoToolbar: public wxWindow {
public:
	VideoToolbar (wxWindow *parent, const wxPoint &pos, const wxSize &size);
	virtual ~VideoToolbar(){
		for(auto cur = icons.begin(); cur != icons.end(); cur++){
			delete (*cur);
		}
		if(bmp)delete bmp; bmp=NULL;
	};

	int GetToggled();
private:
	
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);
	int Toggled;
	//int sel;
	bool clicked;
	bool showClipTools;
	wxBitmap *bmp;
	static std::vector< wxBitmap*> icons;
};