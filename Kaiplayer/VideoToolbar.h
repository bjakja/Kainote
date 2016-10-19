
#include <wx/wx.h>
#include <vector>

const static int toolsSize = 10;
const static int clipToolsSize = 7;

class itemdata{
public:
	itemdata(wxBitmap *_icon, const wxString& _help){icon=_icon; help=_help;}
	~itemdata(){delete icon;}

	wxBitmap *icon;
	wxString help;
};

class VideoToolbar: public wxWindow {
public:
	VideoToolbar (wxWindow *parent, const wxPoint &pos);
	virtual ~VideoToolbar(){
		if(bmp)delete bmp; bmp=NULL;
	};

	int GetToggled();
	int GetClipToggled(){return clipToggled-Toggled;};
	void SetClipToggled(int newtool){ clipToggled = toolsSize + newtool; Refresh(false);};
	void ShowClipTools(bool show){showClipTools=show; Refresh(false);}
	bool ClipToolsShown(){return showClipTools;}
	static void DestroyIcons(){
		for(auto cur = icons.begin(); cur != icons.end(); cur++){
			delete (*cur);
		}
	}
private:
	
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);
	int Toggled;
	int clipToggled;
	int sel;
	bool clicked;
	bool showClipTools;
	bool blockScroll;
	wxBitmap *bmp;
	static std::vector< itemdata*> icons;
};

enum{
	ID_VIDEO_TOOLBAR_EVENT=21909,
	ID_AUX_TOOLBAR_EVENT
};