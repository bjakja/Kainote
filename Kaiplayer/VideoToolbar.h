
#include <wx/wx.h>
#include <vector>

const static int toolsSize = 10;
const static int clipToolsSize = 6;
const static int moveToolsStart= toolsSize + clipToolsSize;
const static int moveToolsSize = 6;

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
	int GetMoveToggled(){
		int result = 0;
		for(int i = 0; i < moveToolsSize; i++){
			if(MoveToggled[i]){ result |= 1<<i; }
		}
		return result;
	};
	void SetClipToggled(int newtool){ clipToggled = toolsSize + newtool; Refresh(false);};
	void ShowClipTools(bool show){clipToggled = toolsSize+1; showClipTools=show; Refresh(false);}
	void ShowMoveTools(bool show){showMoveTools=show; Refresh(false);}
	bool ClipToolsShown(){return showClipTools;}
	bool MoveToolsShown(){return showMoveTools;}
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
	bool MoveToggled[moveToolsSize];
	int sel;
	bool clicked;
	bool showClipTools;
	bool showMoveTools;
	bool blockScroll;
	wxBitmap *bmp;
	static std::vector< itemdata*> icons;
};

enum{
	BUTTON_POS=1,
	BUTTON_MOVE_START=2,
	BUTTON_MOVE_END=4,
	BUTTON_CLIP=8,
	BUTTON_DRAW=16,
	BUTTON_ORG=32,
	ID_VIDEO_TOOLBAR_EVENT=21909,
	ID_VECTOR_TOOLBAR_EVENT,
	ID_MOVE_TOOLBAR_EVENT
};