// Copyright (c) 2006, Niels Martin Hansen
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
#ifndef AUTOMATION_H
#define AUTOMATION_H
#include <deque>
#include <vector>
#include <map>
#include <wx/wx.h>
#include <wx/thread.h>


//class LuaCommand;
class TabPanel;
struct lua_State;


	


namespace Auto {

	enum CommandFlags {
		/// Default command type
		COMMAND_NORMAL       = 0,

		/// Invoking this command toggles a setting of some sort. Any command
		/// of this type should have IsActive implemented to signal the
		/// current state of the thing being toggled, and invoking the command
		/// twice should be a no-op
		///
		/// This is mutually exclusive with COMMAND_RADIO
		COMMAND_TOGGLE       = 1,

		/// Invoking this command sets a setting to a specific value. Any
		/// command of this type should have IsActive implemented, and if
		/// IsActive returns true, invoking the command should have no effect
		///
		/// This is mutually exclusive with COMMAND_TOGGLE
		COMMAND_RADIO        = 2,

		/// This command has an overridden Validate method
		COMMAND_VALIDATE     = 4,

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

	/// Run a lua function on a background thread
	/// @param L Lua state
	/// @param nargs Number of arguments the function takes
	/// @param nresults Number of values the function returns
	/// @param title Title to use for the progress dialog
	/// @param parent Parent window for the progress dialog
	/// @param can_open_config Can the function open its own dialogs?
	/// @throws agi::UserCancelException if the function fails to run to completion (either due to cancelling or errors)
	//void LuaThreadedCall(lua_State *L, int nargs, int nresults, wxString const& title, wxWindow *parent, bool can_open_config);

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
		void SetHotkey(const wxString &_hotkey){hotkey = _hotkey;}

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
		wxString GetPrettyFilename() const { return filename.AfterLast('\\'); }
		/// The script's name. Not required to be unique.

		void RegisterCommand(LuaCommand *command);
		void UnregisterCommand(LuaCommand *command);

		static LuaScript* GetScriptObject(lua_State *L);

		// Script implementation
		void Reload() { Create(); }

		wxString GetName() const { return name; }
		wxString GetDescription() const { return description; }
		wxString GetAuthor() const { return author; }
		wxString GetVersion() const { return version; }
		bool GetLoadedState() const { return L != nullptr; }

		LuaCommand* GetMacro(int macro) const{ if(macro<(int)macros.size()){return macros[macro];}else{return NULL;} }
		std::vector<LuaCommand*> GetMacros() const{ return macros; }
		bool CheckLastModified(bool check=true);
	};
	/// @class Automation
	/// @brief manage lua scripts Lua


class RunFunction{
public:
	RunFunction(){id=0; element=0; script=0;};
	RunFunction(int _id, int elementToRun, Auto::LuaScript *_script){id=_id; element= elementToRun;script=_script;}
	void Run();
		
private:
	int id;
	int element;
	LuaScript *script;
};

	class Automation :public wxEvtHandler
	{

	public:
		Automation();
		~Automation();

		bool Add(wxString filename, bool autoload=false);
		void Remove(int script);
		void RemoveAll(bool autoload=false);
		void ReloadMacro(int script);
		void ReloadScripts(bool first=false);
		//void RunScript(int script, int macro);
		void AddFromSubs();
		static void OnEdit(wxString &Filename);
		bool CheckChanges();
		void BuildMenu(wxMenu **bar);
		void BuildMenuWithDelay(wxMenu **bar, int time);
		/// Get all managed scripts (both loaded and invalid)
		void OnMenuClick(wxCommandEvent &event);
		

		std::vector<Auto::LuaScript*> Scripts;
		std::vector<Auto::LuaScript*> ASSScripts;
		std::map<int, RunFunction > Actions;
		wxMenu **bar;
		HANDLE handle;
	private:
		wxString AutoloadPath;
		bool HasChanges;
		wxString scriptpaths;
		
	};



class LuaThreadedCall : public wxThread {
	private:
		lua_State *L;
	public:
		LuaThreadedCall(lua_State *_L);
		virtual ExitCode Entry();
	};
}
//class MyMenu : public wxMenu{
//public:
//	MyMenu(Auto::LuaScript *script);
//	virtual ~MyMenu(){}
//
//	bool SendEvent(int id, int checked);
//private:
//	Auto::LuaScript *script;
//};

#endif