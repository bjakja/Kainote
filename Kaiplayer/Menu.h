
#include <wx/wx.h>
#include <vector>

class Menu;
class MenuItem
{
public:
	MenuItem(int _id, const wxString& _label, const wxString& _help, bool _enable = true, wxBitmap *_icon = NULL, Menu *Submenu = 0, byte _type = 0);
	~MenuItem();
	bool Enable(bool enable);
	wxBitmap GetBitmap();
	int GetType(){
		return type;
	}
	wxString GetLabel(){
		return label;
	}
	wxString GetHelp(){
		return help;
	}
	void Check(bool _check=true){
		check = _check;
	}
	wxBitmap *icon;
	wxString label;
	wxString help;
	int id;
	byte type;
	bool enabled;
	Menu* submenu;
	bool check;
};

class MenuDialog : public wxDialog {
	friend class Menu;
public:
	MenuDialog(Menu *parent, wxWindow *DialogParent, const wxPoint &pos, const wxSize &size, bool sendEvent = true);
	~MenuDialog(){
		if(HasCapture()){ReleaseCapture();}
		wxDELETE(bmp);
	}

private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollWinEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt);
	//void OnKillFocus(wxFocusEvent &evt);
	//void InternalOnPopupMenuUpdate(wxUpdateUIEvent& WXUNUSED(event)){};
	int submenuShown;
	int sel;
	int scPos;
	volatile int id;
protected:
	wxBitmap *bmp;
	Menu *parent;
	bool withEvent;
	bool blockHideDialog;
	DECLARE_EVENT_TABLE()
};

class Menu 
{
	friend class MenuDialog;
	friend class MenuBar;
	public:
	Menu();
	//Menu(const wxString& title);
	virtual ~Menu(){
		for(auto cur = items.begin(); cur!= items.end(); cur++){
			delete (*cur);
		}
		//if(dialog){dialog->Destroy(); dialog=NULL;}
	};
	MenuItem *Append(int _id,const wxString& _label, const wxString& _help, bool _enable = true, wxBitmap *_icon = NULL, Menu* Submenu = NULL, byte _type = 0);
	MenuItem *Append(int _id,const wxString& _label, Menu* Submenu, const wxString& _help="", byte _type = 0, bool _enable = true, wxBitmap *_icon = NULL);
	MenuItem *Append(MenuItem *item);
	MenuItem *Prepend(int _id, const wxString& _label, const wxString& _help, bool _enable = true, wxBitmap *_icon = NULL, Menu* Submenu = NULL, byte _type = 0);
	MenuItem *Prepend(MenuItem *item);
	MenuItem *Insert(int position, int _id, const wxString& _label, const wxString& _help, bool _enable = true, wxBitmap *_icon = NULL, Menu* Submenu = NULL, byte _type = 0);
	MenuItem *Insert(int position, MenuItem *item);
	MenuItem *SetAccMenu(int id, const wxString &txt, const wxString &help="", bool enable=true, int kind=0);
	void Delete(int position);
	int GetMenuItemCount();
	MenuItem *FindItem(int id);
	MenuItem *FindItem(const wxString& label);
	MenuItem *FindItemByPosition(int pos);
	void Check(int id, bool check);
	void AppendSeparator();
	int GetPopupMenuSelection(const wxPoint &pos, wxWindow *parent, bool clientPos=true);
	void PopupMenu(const wxPoint &pos, wxWindow *parent, bool clientPos=true);
	void SetTitle(const wxString &_title){title = _title;};
	wxString GetTitle() const {return title;};
private:
	void CalcPosAndSize(wxWindow *parent, wxPoint *pos, wxSize *size, bool clientPos);
	void DestroyDialog();
	std::vector< MenuItem* > items;
	wxString title;
protected:
	MenuDialog *dialog;
};



class MenuBar : public wxWindow
{
	friend class Menu;
public:
	MenuBar(wxWindow *parent);
	~MenuBar(){
		for(auto cur = Menus.begin(); cur!= Menus.end(); cur++){
			delete (*cur);
		}
		wxDELETE(bmp);
	}
	void Append(Menu *menu, const wxString &title);
	void Prepend(Menu *menu, const wxString &title);
	void Insert(int position, Menu *menu, const wxString &title);
private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	int CalcMousePos(wxPoint *pos);
	std::vector< Menu* > Menus;
	wxWindow *parent;
	wxBitmap *bmp;
	int sel;
	bool clicked;
	int oldelem;
	DECLARE_EVENT_TABLE()
};

enum{
	ITEM_NORMAL=0,
	ITEM_CHECK,
	ITEM_SEPARATOR
};