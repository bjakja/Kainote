
#include <wx/clrpicker.h>
#include <wx/gbsizer.h>
#include "AutomationDialog.h"
#include "dialog_colorpicker.h"
#include "styles.h"
#include "NumCtrl.h"

namespace Auto{

LuaConfigDialogCtrl::LuaConfigDialogCtrl(lua_State *L)
	{
		// Assume top of stack is a control table (don't do checking)

		lua_getfield(L, -1, "name");
		if (lua_isstring(L, -1)) {
			name = wxString(lua_tostring(L, -1), wxConvUTF8);
		} else {
			name = _T("");
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "x");
		if (lua_isnumber(L, -1)) {
			x = lua_tointeger(L, -1);
			if (x < 0) x = 0;
		} else {
			x = 0;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "y");
		if (lua_isnumber(L, -1)) {
			y = lua_tointeger(L, -1);
			if (y < 0) y = 0;
		} else {
			y = 0;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "width");
		if (lua_isnumber(L, -1)) {
			width = lua_tointeger(L, -1);
			if (width < 1) width = 1;
		} else {
			width = 1;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "height");
		if (lua_isnumber(L, -1)) {
			height = lua_tointeger(L, -1);
			if (height < 1) height = 1;
		} else {
			height = 1;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "hint");
		if (lua_isstring(L, -1)) {
			hint = wxString(lua_tostring(L, -1), wxConvUTF8);
		} else {
			hint = _T("");
		}
		lua_pop(L, 1);
		cw=NULL;
	}

Edit::Edit(lua_State *L, wxWindow *parent, bool _state)
	: LuaConfigDialogCtrl(L)
	, state(_state)
	{
	lua_getfield(L, -1, "value");
	if (lua_isnil(L, -1))
		{
		lua_pop(L, 1);
		lua_getfield(L, -1, "text");
		}
	text = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_pop(L, 1);

	cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, (state)? 0 : wxTE_MULTILINE);
	
	cw->SetToolTip(hint);
	}

void Edit::ControlReadBack()
	{
		text = ((wxTextCtrl*)cw)->GetValue();
	}

void Edit::LuaReadBack(lua_State *L)
	{
		lua_pushfstring(L, text.mb_str(wxConvUTF8));	
	}

NumEdit::NumEdit(lua_State *L, wxWindow *parent, bool _state)
	: LuaConfigDialogCtrl(L)
	, state(_state)
	{

		lua_getfield(L, -1, "value");
		value = (state)? lua_tointeger(L, -1) : lua_tonumber(L, -1);
		text = lua_tostring(L, -1);
		lua_pop(L, 1);
		bool nomin=false;
		bool nomax=false;

		lua_getfield(L, -1, "min");
		if (lua_isnumber(L, -1)){
			min = (state)? lua_tointeger(L, -1) : lua_tonumber(L, -1);}else{nomin=true;}
		lua_pop(L, 1);

		lua_getfield(L, -1, "max");
		if (lua_isnumber(L, -1)){
			max = (state)? lua_tointeger(L, -1) : lua_tonumber(L, -1);}else{nomax=true;}
		lua_pop(L, 1);
		//wxLogStatus("min %i, max %i",min,max);
		if(nomin){min=-100000000.0f;}
		if(nomax){max=100000000.0f;}

		cw = new NumCtrl(parent, -1, text, min, max, (state), wxDefaultPosition, wxDefaultSize, 0);
		

	
	cw->SetToolTip(hint);
	}

void NumEdit::ControlReadBack()
	{
	if(state){
		value=((NumCtrl*)cw)->GetInt();
		}
	else{
		value=(float)((NumCtrl*)cw)->GetDouble();
		}
	}

void NumEdit::LuaReadBack(lua_State *L)
	{

	if(state){
		lua_pushinteger(L, (int)value);
		}
	else
		{
		lua_pushnumber(L, value);
		}
	}


DropDown::DropDown(lua_State *L, wxWindow *parent)
	: LuaConfigDialogCtrl(L)
	{
	lua_getfield(L, -1, "value");
	value = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_pop(L, 1);

	lua_getfield(L, -1, "items");
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		if (lua_isstring(L, -1)) {
			items.Add(wxString(lua_tostring(L, -1), wxConvUTF8));
			}
		lua_pop(L, 1);
		}
	lua_pop(L, 1);

	cw = new wxComboBox(parent, -1, value, wxDefaultPosition, wxDefaultSize, items, wxCB_READONLY);
	cw->SetToolTip(hint);
	}

void DropDown::ControlReadBack()
	{
	value = ((wxComboBox*)cw)->GetValue();
	}

void DropDown::LuaReadBack(lua_State *L)
	{
	lua_pushstring(L, value.mb_str(wxConvUTF8).data());
	}

Label::Label(lua_State *L, wxWindow *parent)
	: LuaConfigDialogCtrl(L)
	{
	lua_getfield(L, -1, "label");
	label = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_pop(L, 1);

	cw = new wxStaticText(parent, -1, label);
	}

void Label::ControlReadBack()
	{
	}

void Label::LuaReadBack(lua_State *L)
	{
		lua_pushnil(L);
	}
	

Checkbox::Checkbox(lua_State *L, wxWindow *parent)
	: LuaConfigDialogCtrl(L)
	{
	lua_getfield(L, -1, "label");
	label = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_pop(L, 1);

	lua_getfield(L, -1, "value");
	value = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);

	cw = new wxCheckBox(parent, -1, label);
	cw->SetToolTip(hint);
	static_cast<wxCheckBox*>(cw)->SetValue(value);

	}

void Checkbox::ControlReadBack()
	{
	value = ((wxCheckBox*)cw)->GetValue();
	}

void Checkbox::LuaReadBack(lua_State *L)
	{
	lua_pushboolean(L, value);
	}

Color::Color(lua_State *L, wxWindow *parent)
	: LuaConfigDialogCtrl(L)
	{
	lua_getfield(L, -1, "value");
	text = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_pop(L, 1);
	AssColor kol(text);
	cw = new ButtonColorPicker(parent, kol.GetWX());
	cw->SetToolTip(hint);
	}

void Color::ControlReadBack()
	{
	text = ((ButtonColorPicker*)cw)->GetColor().GetAsString(wxC2S_HTML_SYNTAX);
	}

void Color::LuaReadBack(lua_State *L)
	{
	lua_pushstring(L, text.mb_str(wxConvUTF8).data());
	}

LuaConfigDialog::LuaConfigDialog(lua_State *L, wxWindow *_parent, wxString name)
		: wxDialog(_parent,-1,name)
	{
	//wxLogStatus("config dialog");

		button_pushed = 0;

			if (!lua_istable(L, -1))
				// Just to avoid deeper indentation...
				goto skipbuttons;
			// Iterate over items in table
			lua_pushnil(L); // initial key
			while (lua_next(L, -2)) {
				// Simply skip invalid items... FIXME, warn here?
				if (lua_isstring(L, -1)) {
					wxString s(lua_tostring(L, -1), wxConvUTF8);
					buttons.push_back(s);
				}
				lua_pop(L, 1);
			}
skipbuttons:
			lua_pop(L, 1);

		// assume top of stack now contains a dialog table
		if (!lua_istable(L, -1)) {
			lua_pushstring(L, "Nie mozna stworzyc dialogu konfiguracyjnego z czegos co nie jest tablica");
			lua_error(L);
			assert(false);
		}

		// Ok, so there is a table with controls
		lua_pushnil(L); // initial key
		while (lua_next(L, -2)) {
			if (lua_istable(L, -1)) {
				// Get control class
				lua_getfield(L, -1, "class");
				if (!lua_isstring(L, -1))
					goto badcontrol;
				wxString controlclass(lua_tostring(L, -1), wxConvUTF8);
				controlclass.LowerCase();
				lua_pop(L, 1);

				LuaConfigDialogCtrl *ctl;

				// Check control class and create relevant control
				if (controlclass == _T("label")) {
					ctl = new Label(L,this);
				} else if (controlclass == _T("edit")) {
					ctl = new Edit(L,this,true);
				} else if (controlclass == _T("intedit")) {
					ctl = new NumEdit(L,this,true);
				} else if (controlclass == _T("floatedit")) {
					ctl = new NumEdit(L,this,false);
				} else if (controlclass == _T("textbox")) {
					ctl = new Edit(L,this,false);
				} else if (controlclass == _T("dropdown")) {
					ctl = new DropDown(L,this);
				} else if (controlclass == _T("checkbox")) {
					ctl = new Checkbox(L,this);
				} else if (controlclass == _T("color")) {
					ctl = new Color(L,this);
				} else if (controlclass == _T("coloralpha")) {
					// FIXME
					ctl = new Edit(L,this,true);
				} else if (controlclass == _T("alpha")) {
					// FIXME
					ctl = new Edit(L,this,true);
				} else {
					goto badcontrol;
				}

				controls.push_back(ctl);

			} else {
badcontrol:
				// not a control...
				// FIXME, better error reporting?
				lua_pushstring(L, "Zla nazwa kontrolki");
				lua_error(L);
			}
			lua_pop(L, 1);
		}
		//wxLogStatus("pushed %i",(int)controls.size());
		wxGridBagSizer *s = new wxGridBagSizer(4, 4);

		for (size_t i = 0; i < controls.size(); ++i) {
			LuaConfigDialogCtrl *c = controls[i];
			if (dynamic_cast<Label*>(c)) {
				s->Add(c->cw, wxGBPosition(c->y, c->x), wxGBSpan(c->height, c->width), wxALIGN_CENTRE_VERTICAL|wxALIGN_LEFT);
			} else {
				s->Add(c->cw, wxGBPosition(c->y, c->x), wxGBSpan(c->height, c->width), wxEXPAND);
			}
		}
		//wxLogStatus("pushed controls");
		
			wxStdDialogButtonSizer *bs = new wxStdDialogButtonSizer();
			if (buttons.size() > 0) {
				for (size_t i = 0; i < buttons.size(); ++i) {
					bs->Add(new wxButton(this, 1001+(wxWindowID)i, buttons[i]));
				}
			} else {
				bs->Add(new wxButton(this, wxID_OK));
				bs->Add(new wxButton(this, wxID_CANCEL));
			}
			bs->Realize();
			//wxLogStatus("Buttons");
			
			// passing button_event as userdata because wx will then delete it
			Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LuaConfigDialog::OnButtonPush);

			wxBoxSizer *ms = new wxBoxSizer(wxVERTICAL);
			ms->Add(s, 0, wxALL, 10);
			ms->Add(bs, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 10);
			SetSizerAndFit(ms);
			//wxLogStatus("All");
			
	}

	LuaConfigDialog::~LuaConfigDialog()
	{
		button_pushed=1;
		for (size_t i = 0; i < controls.size(); ++i)
			delete controls[i];
	}

	

	int LuaConfigDialog::LuaReadBack(lua_State *L)
	{
		// First read back which button was pressed, if any
		ReadBack();
			int btn = button_pushed;
			if (btn == 0) {
				// Always cancel/closed
				lua_pushboolean(L, 0);
			} else if (buttons.size() == 0 && btn == 1) {
				lua_pushboolean(L, 1);
			} else {
				// button_pushed is index+1 to reserve 0 for Cancel
				lua_pushstring(L, buttons[btn-1].mb_str(wxConvUTF8).data());
			}
		

		// Then read controls back
		lua_newtable(L);
		for (size_t i = 0; i < controls.size(); ++i) {
			//wxLogStatus("pos %i", (int)i);
			controls[i]->LuaReadBack(L);
			lua_setfield(L, -2, controls[i]->name.mb_str(wxConvUTF8));
		}

			return 2;
	}

	void LuaConfigDialog::ReadBack()
	{
		for (size_t i = 0; i < controls.size(); ++i) {
			controls[i]->ControlReadBack();
		}
	}

	void LuaConfigDialog::OnButtonPush(wxCommandEvent &evt)
	{
		// Let button_pushed == 0 mean "cancelled", such that pushing Cancel or closing the dialog
		// will both result in button_pushed == 0
		if (evt.GetId() == wxID_OK) {
			button_pushed = 1;
			//wxLogStatus("ok");
		} else if (evt.GetId() == wxID_CANCEL) {
			button_pushed = 0;
			//wxLogStatus("can");
		} else {
			// Therefore, when buttons are numbered from 1001 to 1000+n, make sure to set it to i+1
			button_pushed = evt.GetId() - 1000;

			// hack to make sure the dialog will be closed
			// only do this for non-colour buttons
			wxColourPickerCtrl *button = dynamic_cast<wxColourPickerCtrl*> (evt.GetEventObject());
			if (button) return;
			evt.SetId(wxID_OK);
			
		}
		evt.Skip();
	}

	void LuaConfigDialog::OnClose(wxCloseEvent &event)
	{
		button_pushed=0;
		EndModal(wxID_CANCEL);
	}
	BEGIN_EVENT_TABLE(LuaConfigDialog,wxDialog)
		EVT_CLOSE(LuaConfigDialog::OnClose)
	END_EVENT_TABLE()
	};

	