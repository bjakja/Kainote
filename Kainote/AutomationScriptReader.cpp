//  Copyright (c) 2016-2022, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

//this code piervously was taken from Aegisub 2 it's rewritten by me almost all.

#include "AutomationScriptReader.h"

#include "AutomationUtils.h"
#include "OpennWrite.h"
#include "config.h"
#include <wx/tokenzr.h>
#include <wx/log.h>

#include <lauxlib.h>





	bool LoadFile(lua_State *L, wxString const& filename) {
		wxString script;
		const char *constbuff;
		char *buff;
		char *cpybuff;
		int size;
		bool compatybility = Options.GetBool(AUTOMATION_OLD_SCRIPTS_COMPATIBILITY);
		if (compatybility){
			OpenWrite ow;
			if (!ow.FileOpen(filename, &script)){ return false; }
			script.Replace("kainote", "aegisub");

			constbuff = script.mb_str(wxConvUTF8).data();
			size = strlen(constbuff);
		}
		else{
			FILE *f = nullptr;
			f = _wfopen(filename.wc_str(), L"rb");
			if (!f){ return false; }
			fseek(f, 0, SEEK_END);
			size = ftell(f);
			rewind(f);
			cpybuff = buff = new char[size];
			size = fread(buff, 1, size, f);
			fclose(f);
			/*wxFile f(filename, wxFile::read);
			if(f.IsOpened()){
			size = f.Length();
			f.Read(buff,size);
			}*/
			if (size >= 3 && buff[0] == -17 && buff[1] == -69 && buff[2] == -65) {
				buff += 3;
				size -= 3;
			}
		}
		wxString name = filename.AfterLast('\\');
		if (!filename.EndsWith("moon")){
			//LuaScriptReader script_reader(filename);
			bool ret = luaL_loadbuffer(L, (compatybility) ? constbuff : buff, size, name.utf8_str().data()) == 0;

			if (!compatybility){ delete[] cpybuff; }
			return ret;

		}
		// We have a MoonScript file, so we need to load it with that
		// It might be nice to have a dedicated lua state for compiling
		// MoonScript to Lua
		lua_getfield(L, LUA_REGISTRYINDEX, "moonscript");

		// Save the text we'll be loading for the line number rewriting in the
		// error handling
		lua_pushlstring(L, (compatybility) ? constbuff : buff, size);
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, ("raw moonscript: " + name).utf8_str().data());
		if (!compatybility){ delete[] cpybuff; }

		push_value(L, name);
		if (lua_pcall(L, 2, 2, 0))
			return false; // Leaves error message on stack

		// loadstring returns nil, error on error or a function on success
		if (lua_isnil(L, 1)) {
			lua_remove(L, 1);
			return false;
		}

		lua_pop(L, 1); // Remove the extra nil for the stackchecker
		return true;
	}

	static int module_loader(lua_State *L) {
		int pretop = lua_gettop(L);
		wxString module(check_string(L, -1));
		module.Replace(".", LUA_DIRSEP);

		// Get the lua package include path (which the user may have modified)
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "path");
		wxString package_paths(check_string(L, -1));
		lua_pop(L, 2);

		wxStringTokenizer token(package_paths, ";", wxTOKEN_STRTOK);

		while (token.HasMoreTokens()) {

			wxString filename = token.NextToken();
			filename.Replace("?", module);

			// If there's a .moon file at that path, load it instead of the
			// .lua file

			if (filename.EndsWith("lua")) {
				wxString moonpath = filename.BeforeLast('.') + ".moon";
				if (wxFileExists(moonpath))
					filename = moonpath;
			}

			if (!wxFileExists(filename))
				continue;


			if (!LoadFile(L, filename))
				return error(L, "Error loading Lua module \"%s\":\n%s", filename.utf8_str().data(), check_string(L, 1).utf8_str().data());
			break;

		}

		return lua_gettop(L) - pretop;
	}

	bool Install(lua_State *L, std::vector<wxString> const& include_path) {
		// set the module load path to include_path
		//if(LoadFile(L,include_path[include_path.size()-1]+"\\moonscript.lua"))
		//{

		//}
		lua_getglobal(L, "package");
		push_value(L, "path");

		push_value(L, "");
		for (auto const& path : include_path) {
			lua_pushfstring(L, "%s/?.lua;%s/?/init.lua;", path.utf8_str().data(), path.utf8_str().data());
			lua_concat(L, 2);
		}


		lua_settable(L, -3);

		// Replace the default lua module loader with our unicode compatible one
		lua_getfield(L, -1, "loaders");
		push_value(L, exception_wrapper<module_loader>);
		lua_rawseti(L, -2, 2);
		lua_pop(L, 2); // loaders, package

		luaL_loadstring(L, "return require('moonscript').loadstring");
		if (lua_pcall(L, 0, 1, 0)) {
			return false; // leave error message
		}
		lua_setfield(L, LUA_REGISTRYINDEX, "moonscript");
		return true;
	}

