
#include <wx/wx.h>
#include <vector>



class MenuItem
{
public:
	MenuItem(const wxString& _label, const wxString& _help, int _id, bool _enable = true, const wxBitmap &_icon = wxBitmap(), byte _type = 0)
	{
		icon=_icon; label=_label; id=_id; enabled=_enable;type=_type;submenu=0;
	}
	~MenuItem()
	{
		if(submenu){submenu->Destroy();}
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
	wxString GetLabel(){
		return label;
	}
	wxString GetHelp(){
		return help;
	}
	wxBitmap icon;
	wxString label;
	wxString help;
	int id;
	byte type;
	bool enabled;
	Menu* submenu;
};

class Menu : public wxDialog
{
	public:
	Menu();
	virtual ~Menu(){
		wxDELETE(bmp);
		for(auto cur = items.begin(); cur!= items.end(); cur++){
			delete (*cur);
		}
	};
	MenuItem *Append(const wxString& _label, const wxString& _help, int _id, bool _enable = true, const wxBitmap &_icon = wxBitmap(), byte _type = 0);
	MenuItem *Append(MenuItem *item);
	MenuItem *Prepend(const wxString& _label, const wxString& _help, int _id, bool _enable = true, const wxBitmap &_icon = wxBitmap(), byte _type = 0);
	MenuItem *Prepend(MenuItem *item);
	MenuItem *Insert(int position, const wxString& _label, const wxString& _help, int _id, bool _enable = true, const wxBitmap &_icon = wxBitmap(), byte _type = 0);
	MenuItem *Insert(int position, MenuItem *item);
	void Delete(int position);
	int GetMenuItemCount();
	MenuItem *FindItem(int id);
	MenuItem *FindItem(const wxString& label);
	MenuItem *FindItemByPosition(int pos);


private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollWinEvent& event);
	void OnLostCapture(wxFocusEvent &evt);
	
	wxBitmap *bmp;
	int sel;
	int fh;
	int scPos;
	std::vector< MenuItem* > items;
	wxString title;
	DECLARE_EVENT_TABLE()
};

class MenuBar : public wxWindow
{
public:
	MenuBar(wxWindow *parent);
	~MenuBar(){
		for(auto cur = Menus.begin(); cur!= Menus.end(); cur++){
			(*cur)->Destroy();
		}
	}
	void Append(Menu *menu, const wxString &title);
	void Prepend(Menu *menu, const wxString &title);
	void Insert(int position, Menu *menu, const wxString &title);
private:
	std::vector< Menu* > Menus;
};
