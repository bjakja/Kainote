
#include <wx/wx.h>
#include <vector>

static int iconsize=24;

class toolitem
{
public:
	toolitem(wxBitmap _icon, const wxString& _label, int _id, bool _enable, byte _type)
	{
		icon=_icon; label=_label; id=_id; enabled=_enable;type=_type;size=iconsize;
	}
	//types 0 - normal icon, 1 -icon with submenu 2 - clickable element, 3 - spacer 
	toolitem(byte _type, int _size, int _id=-1, bool enable=false)
	{
		type = _type; size=_size; enabled=enable; id=_id;
	}
	bool Enable(bool enable)
	{
		if(enabled!=enable){enabled=enable;return true;}
		return false;
	}
	wxBitmap GetBitmap()
	{
		if(!enabled){
			return wxBitmap(icon.ConvertToImage().ConvertToGreyscale());
		}
		return icon;
	}
	int GetType(){
		return type;
	}
	wxBitmap icon;
	wxString label;
	int id;
	int size;
	byte type;
	bool enabled;
};

class KaiToolbar :public wxWindow
{
	friend class ToolbarMenu;
public:
	KaiToolbar(wxWindow *Parent, wxMenuBar *mainm, int id, bool vertical);
	virtual ~KaiToolbar();

	void AddItem(int id, const wxString &label, const wxBitmap &normal,bool enable, byte type=0);
	void InsertItem(int id, int index, const wxString &label, const wxBitmap &normal,bool enable, byte type=0);
	void AddSpacer();
	void InsertSpacer(int index);
	void UpdateId(int id, bool enable);
	bool Updatetoolbar();
	void InitToolbar();
	void AddID(int id);
	wxArrayInt ids;

private:
	void OnMouseEvent(wxMouseEvent &event);
	void OnToolbarOpts(wxCommandEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &evt);
	wxPoint FindElem(wxPoint pos);
	std::vector<toolitem*> tools;
	
	bool vertical;
	bool Clicked;
	bool wasmoved;
	int wh;
	int oldelem;
	int sel;
	wxBitmap *bmp;
	wxMenuBar *mb;
	DECLARE_EVENT_TABLE()
};

class ToolbarMenu :public wxDialog
{
	friend class KaiToolbar;
public:
	ToolbarMenu(KaiToolbar*parent, const wxPoint &pos);
	virtual ~ToolbarMenu(){wxDELETE(bmp);};

private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollWinEvent& event);
	void OnLostCapture(wxFocusEvent &evt);
	KaiToolbar*parent;
	wxBitmap *bmp;
	int sel;
	int fh;
	int scPos;
	DECLARE_EVENT_TABLE()
};