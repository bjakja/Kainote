// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016 - 2020, Marcin Drob
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

#pragma once

//#include <wx/wx.h>
#include "KaiDialog.h"
#include <memory>
#include <vector>
extern "C" {
#include <lua.hpp>
}


namespace Auto{

	class LuaDialogControl {
	public:
		/// Name of this control in the output table
		wxString name;

		/// Tooltip of this control
		wxString hint;

		int x, y, width, height;

		/// Create the associated wxControl
		virtual wxWindow *Create(wxWindow *parent) = 0;

		/// Get the default flags to use when inserting this control into a sizer
		virtual int GetSizerFlags() const { return wxEXPAND; }

		/// Push the current value of the control onto the lua stack. Must not
		/// touch the GUI as this may be called on a background thread.
		virtual void LuaReadBack(lua_State *L) = 0;

		/// Does this control have any user-changeable data that can be serialized?
		virtual bool CanSerialiseValue() const { return false; }

		/// Serialize the control's current value so that it can be stored
		/// in the script
		virtual wxString SerialiseValue() const { return L""; }

		/// Restore the control's value from a saved value in the script
		virtual void UnserialiseValue(const wxString &serialised) { }

		LuaDialogControl(lua_State *L);

		virtual ~LuaDialogControl(){};
	};

	/// A lua-generated dialog or panel in the export options dialog
	class LuaDialog {
		/// Controls in this dialog
		std::vector<LuaDialogControl*> controls;
		/// The names and IDs of buttons in this dialog if non-default ones were used
		std::vector<std::pair<int, wxString>> buttons;

		/// Does the dialog contain any buttons
		bool use_buttons;

		/// Id of the button pushed (once a button has been pushed)
		int button_pushed;

		KaiDialog *window;

	public:
		LuaDialog(lua_State *L, bool include_buttons);
		~LuaDialog(){
			window->Destroy();
			for (size_t i = 0; i < controls.size(); ++i)
				delete controls[i];

		};
		/// Push the values of the controls in this dialog onto the lua stack
		/// in a single table
		int LuaReadBack(lua_State *L);
		bool IsCancelled(){ return (button_pushed < 0); };

		// ScriptDialog implementation
		KaiDialog* CreateWindow(wxWindow *parent, wxString name);
		wxString Serialise();
		void Unserialise(const wxString &serialised);
	};

};

