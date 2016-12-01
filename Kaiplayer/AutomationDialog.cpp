// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016, Marcin Drob
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/


#include "Styles.h"
#include "NumCtrl.h"
#include "ColorPicker.h"
#include "AutomationDialog.h"
#include "AutomationUtils.h"

#include <wx/clrpicker.h>
#include <wx/gbsizer.h>
#include <wx/tokenzr.h>

#include <cfloat>
#include <map>

namespace Auto{

	wxString inline_string_encode(const wxString &input) {
		wxString output;
		output.reserve(input.size());
		for (char c : input) {
			if (c <= 0x1F || c == 0x23 || c == 0x2C || c == 0x3A || c == 0x7C)
				output += printf("#%02X", (unsigned char)c);
			else
				output += c;
		}
		return output;
	}

	wxString inline_string_decode(const wxString &input) {
		wxString output;
		output.reserve(input.size());
		for (size_t i = 0; i < input.size(); ++i) {
			if (input[i] != '#' || i + 2 > input.size())
				output += input[i];
			else {
				output += (char)strtol(input.substr(i + 1, 2).utf8_str().data(), nullptr, 16);
				i += 2;
			}
		}
		return output;
	}

	inline void get_if_right_type(lua_State *L, wxString &def) {
		if (lua_isstring(L, -1))
			def = wxString(lua_tostring(L, -1), wxConvUTF8);
	}

	inline void get_if_right_type(lua_State *L, double &def) {
		if (lua_isnumber(L, -1))
			def = lua_tonumber(L, -1);
	}

	inline void get_if_right_type(lua_State *L, int &def) {
		if (lua_isnumber(L, -1))
			def = lua_tointeger(L, -1);
	}

	inline void get_if_right_type(lua_State *L, bool &def) {
		if (lua_isboolean(L, -1))
			def = !!lua_toboolean(L, -1);
	}

	template<class T>
	T get_field(lua_State *L, const char *name, T def) {
		lua_getfield(L, -1, name);
		get_if_right_type(L, def);
		lua_pop(L, 1);
		return def;
	}

	inline wxString get_field(lua_State *L, const char *name) {
		return get_field(L, name, wxString());
	}

	template<class T>
	void read_string_array(lua_State *L, T &cont) {
		lua_for_each(L, [&] {
			if (lua_isstring(L, -1))
				cont.push_back(lua_tostring(L, -1));
		});
	}

	int string_wxString_id(const wxString & str) {
		std::map<wxString, int> ids;
		if (ids.empty()) {
			ids["ok"] = wxID_OK;
			ids["yes"] = wxID_YES;
			ids["save"] = wxID_SAVE;
			ids["apply"] = wxID_APPLY;
			ids["close"] = wxID_CLOSE;
			ids["no"] = wxID_NO;
			ids["cancel"] = wxID_CANCEL;
			ids["help"] = wxID_HELP;
			ids["context_help"] = wxID_CONTEXT_HELP;
		}
		auto it = ids.find(str);
		return it == ids.end() ? -1 : it->second;
	}

	// LuaDialogControl
	LuaDialogControl::LuaDialogControl(lua_State *L)
	// Assume top of stack is a control table (don't do checking)
	: name(get_field(L, "name"))
	, hint(get_field(L, "hint"))
	, x(get_field(L, "x", 0))
	, y(get_field(L, "y", 0))
	, width(get_field(L, "width", 1))
	, height(get_field(L, "height", 1))
	{
		//LOG_D("automation/lua/dialog") << "created control: '" << name << "', (" << x << "," << y << ")(" << width << "," << height << "), " << hint;
	}

	//namespace LuaControl {
		/// A static text label
		class Label : public LuaDialogControl {
			wxString label;
		public:
			Label(lua_State *L) : LuaDialogControl(L), label(get_field(L, "label")) { }

			wxWindow *Create(wxWindow *parent) {
				return new wxStaticText(parent, -1, wxString(label));
			}

			int GetSizerFlags() const { return wxALIGN_CENTRE_VERTICAL | wxALIGN_LEFT; }

			void LuaReadBack(lua_State *L) {
				// Label doesn't produce output, so let it be nil
				lua_pushnil(L);
			}
		};

		/// A single-line text edit control
		class Edit : public LuaDialogControl {
		protected:
			wxString text;
			wxTextCtrl *cw;

		public:
			Edit(lua_State *L)
			: LuaDialogControl(L)
			, text(get_field(L, "value"))
			{
				// Undocumented behaviour, 'value' is also accepted as key for text,
				// mostly so a text control can stand in for other things.
				// This shouldn't be exploited and might change later.
				text = get_field(L, "text", text);
			}

			bool CanSerialiseValue() const { return true; }
			wxString SerialiseValue() const { return inline_string_encode(text); }
			void UnserialiseValue(const wxString &serialised) { text = inline_string_decode(serialised); }

			wxWindow *Create(wxWindow *parent) {
				cw = new wxTextCtrl(parent, -1, text);
				cw->SetMaxLength(0);
				cw->SetToolTip(wxString(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				text = ((wxTextCtrl*)cw)->GetValue();
				lua_pushstring(L, text.utf8_str().data());
			}
		};

		/// A color-picker button
		class Color : public LuaDialogControl {
			AssColor color;
			bool alpha;
			ButtonColorPicker *cw;

		public:
			Color(lua_State *L, bool alpha)
			: LuaDialogControl(L)
			, color(get_field(L, "value"))
			, alpha(alpha)
			{
				//wxString col = get_field(L, "value");
				//wxLogStatus("col "+col);
				//color = AssColor(col);
			}

			bool CanSerialiseValue() const { return true; }
			wxString SerialiseValue() const { return inline_string_encode(color.GetHex(alpha)); }
			void UnserialiseValue(const wxString &serialised) { color = wxColour(wxString(inline_string_decode(serialised))); }

			wxWindow *Create(wxWindow *parent) {
				//wxLogStatus("color %i %i %i %i", color.r, color.g, color.b, color.a);
				cw = new ButtonColorPicker(parent, color.GetWX(), wxDefaultSize);
				cw->SetToolTip(wxString(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				color = AssColor(((ButtonColorPicker*)cw)->GetColor());
				lua_pushstring(L, color.GetHex(alpha).utf8_str().data());
			}
		};

		/// A multiline text edit control
		class Textbox : public Edit {
		public:
			Textbox(lua_State *L) : Edit(L) { }

			// Same serialisation interface as single-line edit
			wxWindow *Create(wxWindow *parent) {
				cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
				cw->SetMinSize(wxSize(0, 30));
				cw->SetToolTip(wxString(hint));
				return cw;
			}
		};

		/// Integer only edit
		class IntEdit : public LuaDialogControl {
			NumCtrl *cw;
			int value;
			int min, max;

		public:
			IntEdit(lua_State *L)
			: LuaDialogControl(L)
			, value(get_field(L, "value", 0))
			, min(get_field(L, "min", INT_MIN))
			, max(get_field(L, "max", INT_MAX))
			{
				if (min >= max) {
					max = INT_MAX;
					min = INT_MIN;
				}
			}

			bool CanSerialiseValue() const  { return true; }
			wxString SerialiseValue() const { return std::to_string(value); }
			void UnserialiseValue(const wxString &serialised) { value = atoi(serialised.utf8_str().data()); }

			wxWindow *Create(wxWindow *parent) {
				wxString tmpval;
				tmpval<<value;
				cw = new NumCtrl(parent, -1, tmpval, min, max, true, wxDefaultPosition, wxDefaultSize);
				cw->SetToolTip(wxString(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				value=((NumCtrl*)cw)->GetInt();
				lua_pushinteger(L, value);
			}
		};

		// Float only edit
		class FloatEdit : public LuaDialogControl {
			double value;
			double min;
			double max;
			double step;
			NumCtrl *scd;

		public:
			FloatEdit(lua_State *L)
			: LuaDialogControl(L)
			, value(get_field(L, "value", 0.0))
			, min(get_field(L, "min", -DBL_MAX))
			, max(get_field(L, "max", DBL_MAX))
			, step(get_field(L, "step", 0.0))
			{
				if (min >= max) {
					max = DBL_MAX;
					min = -DBL_MAX;
				}
			}

			bool CanSerialiseValue() const { return true; }
			wxString SerialiseValue() const { return std::to_string(value); }
			void UnserialiseValue(const wxString &serialised) { value = atof(serialised.utf8_str().data()); }

			wxWindow *Create(wxWindow *parent) {
				scd = new NumCtrl(parent, -1, value, min, max, false, wxDefaultPosition, wxDefaultSize);
				scd->SetToolTip(wxString(hint));
				return scd;


			}

			void LuaReadBack(lua_State *L) {
				value=(float)((NumCtrl*)scd)->GetDouble();
				lua_pushnumber(L, value);
			}
		};

		/// A dropdown list
		class Dropdown : public LuaDialogControl {
			wxArrayString items;
			wxString value;
			wxComboBox *cw;

		public:
			Dropdown(lua_State *L)
			: LuaDialogControl(L)
			, value(get_field(L, "value"))
			{
				lua_getfield(L, -1, "items");
				read_string_array(L, items);
			}

			bool CanSerialiseValue() const { return true; }
			wxString SerialiseValue() const { return inline_string_encode(value); }
			void UnserialiseValue(const wxString &serialised) { value = inline_string_decode(serialised); }

			wxWindow *Create(wxWindow *parent) {
				cw = new wxComboBox(parent, -1, wxString(value), wxDefaultPosition, wxDefaultSize, items, wxCB_READONLY);
				cw->SetToolTip(wxString(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				value = ((wxComboBox*)cw)->GetValue();
				lua_pushstring(L, value.utf8_str().data());
			}
		};

		class Checkbox : public LuaDialogControl {
			wxString label;
			bool value;
			wxCheckBox *cw;

		public:
			Checkbox(lua_State *L)
			: LuaDialogControl(L)
			, label(get_field(L, "label"))
			, value(get_field(L, "value", false))
			{
			}

			bool CanSerialiseValue() const { return true; }
			wxString SerialiseValue() const { return value ? "1" : "0"; }
			void UnserialiseValue(const wxString &serialised) { value = serialised != "0"; }

			wxWindow *Create(wxWindow *parent) {
				cw = new wxCheckBox(parent, -1, wxString(label));
				cw->SetToolTip(wxString(hint));
				cw->SetValue(value);
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				value = ((wxCheckBox*)cw)->GetValue();
				lua_pushboolean(L, value);
			}
		};
	//}

	// LuaDialog
	LuaDialog::LuaDialog(lua_State *L, bool include_buttons)
	: use_buttons(include_buttons)
	, button_pushed(-1)
	{
		//LOG_D("automation/lua/dialog") << "creating LuaDialoug, addr: " << this;

		// assume top of stack now contains a dialog table
		if (!lua_istable(L, 1))
			error(L, "Cannot create config dialog from something non-table");

		// Ok, so there is a table with controls
		lua_pushvalue(L, 1);
		lua_for_each(L, [&] {
			if (!lua_istable(L, -1))
				error(L, "bad control table entry");

			wxString controlclass = get_field(L, "class");
			controlclass.MakeLower();

			LuaDialogControl* ctl;

			// Check control class and create relevant control
			if (controlclass == "label")
				ctl = new Label(L);
			else if (controlclass == "edit")
				ctl = new Edit(L);
			else if (controlclass == "intedit")
				ctl = new IntEdit(L);
			else if (controlclass == "floatedit")
				ctl = new FloatEdit(L);
			else if (controlclass == "textbox")
				ctl = new Textbox(L);
			else if (controlclass == "dropdown")
				ctl = new Dropdown(L);
			else if (controlclass == "checkbox")
				ctl = new Checkbox(L);
			else if (controlclass == "color")
				ctl = new Color(L, false);
			else if (controlclass == "coloralpha")
				ctl = new Color(L, true);
			else if (controlclass == "alpha")
				// FIXME
				ctl = new Edit(L);
			else
				error(L, "bad control table entry");

			controls.emplace_back(ctl);
		});
		if (include_buttons && lua_istable(L, 2)) {
			lua_pushvalue(L, 2);
			lua_for_each(L, [&]{
				wxString butt = check_string(L, -1);
				//wxLogStatus("button %s", butt);
				buttons.emplace_back(-1, butt);
			});
		}

		if (include_buttons && lua_istable(L, 3)) {
			lua_pushvalue(L, 3);
			lua_for_each(L, [&]{
				wxString butt = check_string(L, -2);
				int id = string_wxString_id(butt);
				//if (id<0){id = idstart; idstart++;}
				//wxLogStatus("button %s %i", butt, id);
				wxString label = check_string(L, -1);
				auto btn = find_if(buttons.begin(),buttons.end(),
					[&](std::pair<int, wxString>& btn) { return btn.second == label; } );
				if (btn==buttons.end())
					error(L, "Invalid button for id %s", lua_tostring(L, -2));
				btn->first = id;
			});
		}
	}

	wxDialog* LuaDialog::CreateWindow(wxWindow *parent, wxString name) {
		window = new wxDialog(parent,-1,name);

		auto s = new wxGridBagSizer(4, 4);
		for (auto& c : controls)
			s->Add(c->Create(window), wxGBPosition(c->y, c->x),
				wxGBSpan(c->height, c->width), c->GetSizerFlags());

		if (!use_buttons) {
			window->SetSizerAndFit(s);
			return window;
		}

		if (buttons.size() == 0) {
			buttons.emplace_back(wxID_OK, "");
			buttons.emplace_back(wxID_CANCEL, "");
		}

		//auto dialog = static_cast<wxDialog *>(parent);
		auto bs = new wxBoxSizer(wxHORIZONTAL);

		//auto make_button = [&](wxWindowID id, int button_pushed, wxString const& text) -> wxButton *{
		//	auto button = new wxButton(window, id, text);
		//	button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt) {
		//		this->button_pushed = button_pushed;
		//		//TransferDataFromWindow();
		//		window->EndModal(0);
		//	});

		//	if (id == wxID_OK || id == wxID_YES || id == wxID_SAVE) {
		//		//window->SetDefault();
		//		button->SetFocus();
		//		window->SetAffirmativeId(id);
		//	}

		//	if (id == wxID_CLOSE || id == wxID_NO)
		//		window->SetEscapeId(id);

		//	return button;
		//};

		//if (std::count(buttons.begin(), buttons.end(), -1) == 0) {
		
			for (size_t i = 0; i < buttons.size(); ++i){
				int id = buttons[i].first;
			
				auto button = new wxButton(window, id, buttons[i].second);
				bs->Add(button);

				button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt) {
					this->button_pushed = i;
					//TransferDataFromWindow();
					//wxLogStatus("button pushed %i %s", button_pushed, buttons[i].second);
					window->EndModal(0);
				});

				if (id == wxID_OK || id == wxID_YES || id == wxID_SAVE) {
					//window->SetDefault();
					button->SetFocus();
					window->SetAffirmativeId(id);
				}

				if (id == wxID_CLOSE || id == wxID_NO)
					window->SetEscapeId(id);
			}
		
			//bs->Realize();
		/*}
		else {
			for (size_t i = 0; i < buttons.size(); ++i)
				bs->Add(make_button(buttons[i].first, i, buttons[i].second));
		}*/

		auto ms = new wxBoxSizer(wxVERTICAL);
		ms->Add(s, 0, wxALL, 5);
		ms->Add(bs, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 10);
		window->SetSizerAndFit(ms);

		return window;
	}

	int LuaDialog::LuaReadBack(lua_State *L) {
		// First read back which button was pressed, if any
		if (use_buttons) {
			if (button_pushed < 0 || buttons[button_pushed].first == wxID_CANCEL){
				lua_pushboolean(L, false);
				//wxLogStatus("cancelled %i", button_pushed);
			}
			else
				lua_pushstring(L, buttons[button_pushed].second.utf8_str().data());
		}

		// Then read controls back
		lua_createtable(L, 0, controls.size());
		for (auto& control : controls) {
			control->LuaReadBack(L);
			lua_setfield(L, -2, control->name.utf8_str().data());
		}

		return use_buttons ? 2 : 1;
	}

	wxString LuaDialog::Serialise() {
		wxString res;

		// Format into "name1:value1|name2:value2|name3:value3"
		for (auto& control : controls) {
			if (control->CanSerialiseValue()) {
				if (!res.empty())
					res += "|";
				res += inline_string_encode(control->name) + ":" + control->SerialiseValue();
			}
		}

		return res;
	}

	void LuaDialog::Unserialise(const wxString &serialised) {
		wxStringTokenizer tok(serialised, "|", wxTOKEN_STRTOK);
		while (tok.HasMoreTokens()) {
			wxString token=tok.GetNextToken();
			int pos = token.Find(':');
			if (pos == -1) continue;

			wxString name = inline_string_decode(token.SubString(0,pos+1));
			wxString value = token.Mid(pos+1);

			// Hand value to all controls matching name
			for (auto& control : controls) {
				if (control->name == name && control->CanSerialiseValue())
					control->UnserialiseValue(value);
			}
		}
	}
};

