
#include <wx/wx.h>
#include <vector>

class Menu;
class MenuItem
{
public:
	MenuItem(const wxString& _label, const wxString& _help, int _id, bool _enable = true, wxBitmap *_icon = NULL, Menu *Submenu = 0, byte _type = 0);
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
	wxBitmap *icon;
	wxString label;
	wxString help;
	int id;
	byte type;
	bool enabled;
	Menu* submenu;
	bool check;
};

class MenuDialog : public wxDialog{
	friend class Menu;
	MenuDialog(Menu *parent, wxWindow *DialogParent, const wxPoint &pos, const wxSize &size, bool sendEvent = true);
	~MenuDialog(){
		wxDELETE(bmp);
	}
	private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollWinEvent& event);
	void OnLostCapture(wxFocusEvent &evt);
	
private:
	int sel;
	int fh;
	int scPos;
protected:
	wxBitmap *bmp;
	Menu *parent;
	bool withEvent;
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
		if(dialog){dialog->Destroy(); dialog=NULL;}
	};
	MenuItem *Append(const wxString& _label, const wxString& _help, int _id, bool _enable = true, wxBitmap *_icon = NULL, Menu* Submenu = NULL, byte _type = 0);
	MenuItem *Append(MenuItem *item);
	MenuItem *Prepend(const wxString& _label, const wxString& _help, int _id, bool _enable = true, wxBitmap *_icon = NULL, Menu* Submenu = NULL, byte _type = 0);
	MenuItem *Prepend(MenuItem *item);
	MenuItem *Insert(int position, const wxString& _label, const wxString& _help, int _id, bool _enable = true, wxBitmap *_icon = NULL, Menu* Submenu = NULL, byte _type = 0);
	MenuItem *Insert(int position, MenuItem *item);
	void Delete(int position);
	int GetMenuItemCount();
	MenuItem *FindItem(int id);
	MenuItem *FindItem(const wxString& label);
	MenuItem *FindItemByPosition(int pos);
	int GetPopupMenuSelection(const wxPoint &pos, wxWindow *parent);
	void PopupMenu(const wxPoint &pos, wxWindow *parent);
	void SetTitle(const wxString &_title){title = _title;};
	wxString GetTitle() const {return title;};
private:
	void CalcPosAndSize(wxWindow *parent, wxPoint *pos, wxSize *size);
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
