#ifndef AUTODIALOG
#define AUTODIALOG

#include <wx/wx.h>

#include <vector>
//extern "C" {
#include <lauxlib.h>
	//}


namespace Auto{

class LuaConfigDialogCtrl
	{

public:
	LuaConfigDialogCtrl(lua_State *L);
	virtual ~LuaConfigDialogCtrl() { if( cw ){delete cw;} };
	virtual void LuaReadBack(lua_State *L) {};
	virtual void ControlReadBack() {};
	wxString name, hint;
	int x, y, width, height;
	wxControl *cw;
	};

class Edit : public LuaConfigDialogCtrl
	{
public:
	Edit(lua_State *L, wxWindow *parent, bool _state);
	void ControlReadBack();
	void LuaReadBack(lua_State *L);
	virtual ~Edit() { };
	wxString text;
	float value, min, max;
	bool state;
	};


class NumEdit : public LuaConfigDialogCtrl
	{
public:
	NumEdit(lua_State *L, wxWindow *parent, bool _state);
	void ControlReadBack();
	void LuaReadBack(lua_State *L);
	virtual ~NumEdit() { };
	wxString text;
	float value, min, max;
	bool state;
	};


class DropDown : public LuaConfigDialogCtrl
	{
public:
	DropDown(lua_State *L, wxWindow *parent);
	void ControlReadBack();
	void LuaReadBack(lua_State *L);
	virtual ~DropDown() { };
	wxString value;
	wxArrayString items;
	};

class Label : public LuaConfigDialogCtrl
	{
public:
	Label(lua_State *L, wxWindow *parent);
	void ControlReadBack();
	void LuaReadBack(lua_State *L);
	virtual ~Label() { };
	wxString label;
	};

class Checkbox : public LuaConfigDialogCtrl
	{
public:
	Checkbox(lua_State *L, wxWindow *parent);
	void ControlReadBack();
	void LuaReadBack(lua_State *L);
	virtual ~Checkbox() { };
	bool value;
	wxString label;
	};

class Color : public LuaConfigDialogCtrl
	{
public:
	Color(lua_State *L, wxWindow *parent);
	void ControlReadBack();
	void LuaReadBack(lua_State *L);
	virtual ~Color() { };
	wxString text;
	};

class LuaConfigDialog : public wxDialog{
	private:
		std::vector<LuaConfigDialogCtrl*> controls;
		wxArrayString buttons;

		//class ButtonEventHandler : public wxEvtHandler {
		//public:
			//int *button_pushed;
			void OnButtonPush(wxCommandEvent &evt);
		//};

		//ButtonEventHandler *button_event;
		DECLARE_EVENT_TABLE()

	public:
		LuaConfigDialog(lua_State *_L, wxWindow *parent, wxString name);
		virtual ~LuaConfigDialog();
		int LuaReadBack(lua_State *L); // read back internal structure to lua structures

		//wxString Serialise();
		//void Unserialise(const wxString &serialised);
		void OnClose(wxCloseEvent &event);
		volatile int button_pushed;

		void ReadBack(); // from auto4 base
	};

	};

#endif