// Copyright (c) 2006, Niels Martin Hansen
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

#include <deque>
#include <vector>
#include <map>
#include <wx/window.h>
#include <wx/thread.h>


//class LuaCommand;
class TabPanel;
struct lua_State;


class Menu;
#ifndef HANDLE
typedef void* HANDLE;
#endif

namespace Auto {

	enum CommandFlags {
		/// Default command type
		COMMAND_NORMAL = 0,

		/// Invoking this command toggles a setting of some sort. Any command
		/// of this type should have IsActive implemented to signal the
		/// current state of the thing being toggled, and invoking the command
		/// twice should be a no-op
		///
		/// This is mutually exclusive with COMMAND_RADIO
		COMMAND_TOGGLE = 1,

		/// Invoking this command sets a setting to a specific value. Any
		/// command of this type should have IsActive implemented, and if
		/// IsActive returns true, invoking the command should have no effect
		///
		/// This is mutually exclusive with COMMAND_TOGGLE
		COMMAND_RADIO = 2,

		/// This command has an overridden Validate method
		COMMAND_VALIDATE = 4,

		/// This command's name may change based on the state of the project
		COMMAND_DYNAMIC_NAME = 8,

		/// This command's help string may change
		COMMAND_DYNAMIC_HELP = 16,

		/// This command's icon may change based on the state of the project
		COMMAND_DYNAMIC_ICON = 32
	};

	class LuaFeature {
		int myid;
	protected:
		lua_State *L;

		void RegisterFeature();
		void UnregisterFeature();

		void GetFeatureFunction(const char *function) const;

		LuaFeature(lua_State *L) : L(L) { }
	};



	class LuaCommand : private LuaFeature {
		//wxString cmd_name;
		wxString display;
		wxString help;
		wxString hotkey;
		int cmd_type;

	public:
		LuaCommand(lua_State *L);
		~LuaCommand();

		//const char* name() const { return cmd_name.c_str(); }
		wxString StrMenu() const { return display; }
		wxString StrDisplay() const { return display; }
		wxString StrHelp() const { return help; }
		wxString StrHotkey() const { return hotkey; }
		void SetHotkey(const wxString &_hotkey){ hotkey = _hotkey; }

		int Type() const { return cmd_type; }

		void Run(TabPanel *c);
		bool Validate(const TabPanel *c);
		virtual bool IsActive(const TabPanel *c);

		static int LuaRegister(lua_State *L);
		void RunScript();
	};

	class LuaScript {
		lua_State *L;
		wxString filename;
		wxString name;
		wxString description;
		wxString author;
		wxString version;
		int LowTime, HighTime;
		std::vector<wxString> include_path;
		std::vector<LuaCommand*> macros;

		/// load script and create internal structures etc.
		void Create();
		/// destroy internal structures, unreg features and delete environment
		void Destroy();

		static int LuaInclude(lua_State *L);

	public:

		LuaScript(wxString const& filename);
		~LuaScript() { Destroy(); }

		wxString GetFilename() const { return filename; }
		/// The script's file name without path
		wxString GetPrettyFilename() const { return filename.AfterLast(L'\\'); }
		/// The script's name. Not required to be unique.

		void RegisterCommand(LuaCommand *command);
		void UnregisterCommand(LuaCommand *command);

		static LuaScript* GetScriptObject(lua_State *L);

		// Script implementation
		void Reload();// { Create(); }

		wxString GetName() const { return name; }
		wxString GetDescription() const { return description; }
		wxString GetAuthor() const { return author; }
		wxString GetVersion() const { return version; }
		bool GetLoadedState() const { 
			return L != nullptr; 
		}

		LuaCommand* GetMacro(int macro) const{
			if (macro < (int)macros.size()){
				return macros[macro];
			}
			else{ return NULL; }
		}
		std::vector<LuaCommand*> GetMacros() const{ return macros; }
		bool CheckLastModified(bool check = true);
	};
	// @class Automation
	// @brief manage lua scripts Lua



	class Automation
	{
	public:
		Automation(bool loadSubsScripts = false, bool loadNow = false);
		~Automation();

		bool Add(wxString filename, bool addToSinfo = true, bool autoload = false);
		void Remove(int script);
		void RemoveAll(bool autoload = false);
		void ReloadMacro(int script, bool autoload = false);
		void ReloadScripts(bool first = false);
		bool AddFromSubs();
		static void OnEdit(const wxString &Filename);
		bool CheckChanges();
		// make menu with all macros or error message
		void BuildMenu(Menu **bar, bool all = false);
		void ShowScriptHotkeysWindow(wxWindow *parent);
		LuaScript *FindScript(const wxString &path);

		std::vector<Auto::LuaScript*> Scripts;
		std::vector<Auto::LuaScript*> ASSScripts;
		HANDLE handle;
		HANDLE eventEndAutoload = NULL;
	private:
		wxString AutoloadPath;
		bool HasChanges;
		wxString scriptpaths;
		bool initialized;
		volatile bool breakLoading = false;
	};

	// Run a lua function on a background thread


	class LuaThreadedCall : public wxThread {
	private:
		lua_State *L;
	public:
		LuaThreadedCall(lua_State *_L);
		virtual ExitCode Entry();
	};


	class VideoFrame {
	public:
		void deleteData() {
			if (data) {
				delete[] data;
				data = nullptr;
			}
		}
		int width = 0;
		int height = 0;
		int pitch = 0;
		bool flipped = false;
		unsigned char* data = nullptr;
	};
}

