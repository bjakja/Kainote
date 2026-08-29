// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
// Copyright (c) 2016 - 2026, Marcin Drob
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "AutomationScriptReader.h"

#include "AutomationUtils.h"
#include "OpennWrite.h"
#include "config.h"
#include <wx/tokenzr.h>
#include <wx/log.h>

#include <lauxlib.h>
#include <string>
#include <vector>

namespace Auto {



	bool LoadFile(lua_State *L, wxString const& filename) {
		wxString script;
		std::string compatibilityBuffer;
		std::vector<char> fileBuffer;
		const char *buffer = "";
		size_t size = 0;
		bool compatybility = Options.GetBool(AUTOMATION_OLD_SCRIPTS_COMPATIBILITY);
		if (compatybility){
			OpenWrite ow;
			if (!ow.FileOpen(filename, &script)){ return false; }
			script.Replace("kainote", "aegisub");

			wxCharBuffer encodedScript = script.mb_str(wxConvUTF8);
			compatibilityBuffer.assign(encodedScript.data(), encodedScript.length());
			buffer = compatibilityBuffer.data();
			size = compatibilityBuffer.size();
		}
		else{
			FILE *f = _wfopen(filename.wc_str(), L"rb");
			if (!f){ return false; }
			if (fseek(f, 0, SEEK_END) != 0) {
				fclose(f);
				return false;
			}
			long fileSize = ftell(f);
			if (fileSize < 0) {
				fclose(f);
				return false;
			}
			rewind(f);
			fileBuffer.resize(static_cast<size_t>(fileSize));
			size = fileBuffer.empty() ? 0 : fread(fileBuffer.data(), 1, fileBuffer.size(), f);
			bool readSucceeded = size == fileBuffer.size();
			fclose(f);
			if (!readSucceeded) { return false; }

			buffer = fileBuffer.empty() ? "" : fileBuffer.data();
			if (size >= 3 && static_cast<unsigned char>(buffer[0]) == 0xEF &&
				static_cast<unsigned char>(buffer[1]) == 0xBB &&
				static_cast<unsigned char>(buffer[2]) == 0xBF) {
				buffer += 3;
				size -= 3;
			}
		}
		//wxString name = filename.AfterLast('\\');
		if (!filename.EndsWith("moon")){
			bool ret = luaL_loadbuffer(L, buffer, size, filename.mb_str(wxConvUTF8).data()) == 0;

			return ret;

		}
		// We have a MoonScript file, so we need to load it with that
		// It might be nice to have a dedicated lua state for compiling
		// MoonScript to Lua
		lua_getfield(L, LUA_REGISTRYINDEX, "moonscript");

		// Save the text we'll be loading for the line number rewriting in the
		// error handling
		lua_pushlstring(L, buffer, size);
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, ("raw moonscript: " + filename).mb_str(wxConvUTF8).data());
		
		push_value(L, filename);
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
			filename.Replace("/?", module);

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
				return error(L, "Error loading Lua module \"%s\":\n%s", filename.mb_str(wxConvUTF8).data(), check_string(L, 1).mb_str(wxConvUTF8).data());
			break;

		}

		return lua_gettop(L) - pretop;
	}

	bool Install(lua_State *L, std::vector<wxString> const& include_path) {
		// set the module load path to include_path
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
}
