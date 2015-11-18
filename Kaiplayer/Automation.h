#ifndef AUTOMAIN
#define AUTOMAIN

#include <wx/wx.h>
#include <vector>

//extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
	//}


class kainoteFrame;

namespace Auto{
// Implementation of the Macro Feature for Lua scripts
class LuaMacro {
	private:
		bool no_validate;
		lua_State *L;
		int myid;

		
	public:
		static int LuaRegister(lua_State *L);
		LuaMacro(const wxString &_name, const wxString &_description, lua_State *_L);
		virtual ~LuaMacro() { }
		void RegisterMacro();
		wxString name;

		bool Validate( const wxArrayInt &selected, int active, int added);
		void Process( wxArrayInt &selected, int active, int added, wxWindow *progress_parent);
	};

class LuaScript {

	private:
		lua_State *L;

		void Create(); // load script and create internal structures etc.
		void Destroy(); // destroy internal structures, unreg features and delete environment

		static LuaScript* GetScriptObject(lua_State *L);

		static int LuaTextExtents(lua_State *L);
		static int LuaInclude(lua_State *L);
		static int LuaModuleLoader(lua_State *L);
		static int LuaFrameFromMs(lua_State *L);
		static int LuaMsFromFrame(lua_State *L);
		static int LuaVideoSize(lua_State *L);

		wxString author;
		wxString version;
		wxPathList include_path;
		int LowTime, HighTime;

	public:
		LuaScript(const wxString &filename);
		virtual ~LuaScript();

		void Reload();
		bool CheckLastModified(bool check=true);

		std::vector<LuaMacro*> Macros;
		bool loaded;
		wxString filename;
		wxString name;
		wxString description;
	};

class LuaThreadedCall : public wxThread {
	private:
		lua_State *L;
	public:
		LuaThreadedCall(lua_State *_L);
		virtual ExitCode Entry();
	};



class Automation{

public:
	Automation(kainoteFrame *kai);
	~Automation();

	bool Add(wxString filename);
	void Remove(int script);
	void RemoveAll(bool autoload=false);
	void ReloadMacro(int script);
	void ReloadScripts(bool first=false);

	std::vector<LuaScript*> Scripts;
private:
	
	kainoteFrame *Kai;
	wxString path;
	int ALScripts;
	};

};

#endif