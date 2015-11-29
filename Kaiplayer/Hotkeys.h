
//#include <wx/string.h>
//#include <wx/accel.h>
//#include <wx/arrstr.h>
#include <wx/wx.h>
#include <map>

class hdata{
public:
	hdata(char accType, wxString accName, wxString _Accel)
	{
		Type=accType; Name=accName; Accel=_Accel;
	}
	hdata(wxString acc)
	{
		Type=acc[0]; Name=acc.Mid(2).BeforeLast('='); Accel=acc.AfterLast('=');
	}
	hdata(){Type='\0';}
	wxString Name;
	wxString Accel;
	char Type;
};

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
	int LoadHkeys(bool Audio=false);
	void LoadDefault(std::map<int, hdata> &_hkeys, bool Audio=false);
	void SaveHkeys(bool Audio=false);
	void SetHKey(int id, wxString name, int flag, int key);
	wxAcceleratorEntry GetHKey(int itemid);
	wxString GetMenuH(int id);
	void FillTable();
	void ResetKey(int id);
	wxMenuItem *SetAccMenu(wxMenu *menu, int id, const wxString &txt="", const wxString &help="", wxItemKind kind=wxITEM_NORMAL);
	int OnMapHkey(int id, wxString name,wxWindow *parent,wxString *windows, int elems);
	std::map<int, hdata> hkeys;
	std::map<int, wxString> keys;
	bool AudioKeys;
	wxArrayString lscripts;

};

	extern Hotkeys Hkeys;
