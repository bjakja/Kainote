
//#include <wx/string.h>
//#include <wx/accel.h>
//#include <wx/arrstr.h>
#include <wx/wx.h>
#include <map>

class HkeysDialog : public wxDialog
{
	public:
	HkeysDialog(wxWindow *parent, wxString name, bool script=false, wxString *windows=0, int elems=0);
	virtual ~HkeysDialog();
	int flag, hkey;
	wxString hkname;

	private:
	void OnKeyPress(wxKeyEvent& event);
	bool scr;
};

class Hotkeys
{
private:

public:
	Hotkeys();
	~Hotkeys();
	bool LoadHkeys(bool Audio=false);
	void LoadDefault(std::map<int,wxString> &_hkeys, bool Audio=false);
	void SaveHkeys(bool Audio=false);
	void SetHKey(int id, wxString name, int flag, int key);
	wxAcceleratorEntry GetHKey(int itemid);
	wxString GetMenuH(int id);
	void FillTable();
	void ResetKey(int id);
	wxMenuItem *SetAccMenu(wxMenu *menu, int id, const wxString &txt="", const wxString &help="", wxItemKind kind=wxITEM_NORMAL);
	int OnMapHkey(int id, wxString name,wxWindow *parent,wxString *windows, int elems);
	std::map<int,wxString> hkeys;
	std::map<int, wxString> keys;
	bool AudioKeys;
	wxArrayString lscripts;

};

	extern Hotkeys Hkeys;
