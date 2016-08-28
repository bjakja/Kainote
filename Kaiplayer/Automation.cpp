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

/// @file auto4_lua.cpp
/// @brief Lua 5.1-based scripting engine
/// @ingroup scripting
///

#include "Automation.h"
#include "Hotkeys.h"

#include "KainoteApp.h"
#include "AutomationToFile.h"
#include "AutomationProgress.h"

#include "AutomationUtils.h"
#include "AutomationScriptReader.h"

#include <algorithm>
#include <cassert>
#include <mutex>
#include <wx/clipbrd.h>
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <thread>
//#include <tuple>


namespace Auto{

	
	wxString get_wxstring(lua_State *L, int idx)
	{
		return wxString::FromUTF8(lua_tostring(L, idx));
	}

	wxString check_wxstring(lua_State *L, int idx)
	{
		return wxString(check_string(L, idx));
	}


	int get_file_name(lua_State *L)
	{
		TabPanel *pan=Notebook::GetTab();
		if (pan->SubsPath!="")
			push_value(L, pan->SubsName);
		else
			lua_pushnil(L);
		return 1;
	}

	int get_translation(lua_State *L)
	{
		wxString str(check_wxstring(L, 1));
		push_value(L, wxGetTranslation(str));
		return 1;
	}

	const char *clipboard_get()
	{
		wxString data;
		wxClipboard *cb = wxClipboard::Get();
		if (cb->Open()) {
			if (cb->IsSupported(wxDF_TEXT) || cb->IsSupported(wxDF_UNICODETEXT)) {
				wxTextDataObject raw_data;
				cb->GetData(raw_data);
				data = raw_data.GetText();
			}
			cb->Close();
		}
		if (data.empty())
			return nullptr;
		return strndup(data.ToStdString());
	}

	bool clipboard_set(const char *str)
	{
		bool succeeded = false;

		//#if wxUSE_OLE
		//		// OLE needs to be initialized on each thread that wants to write to
		//		// the clipboard, which wx does not handle automatically
		//		wxClipboard cb;
		//#else
		wxClipboard &cb = *wxTheClipboard;
		//#endif
		if (cb.Open()) {
			succeeded = cb.SetData(new wxTextDataObject(wxString::FromUTF8(str)));
			cb.Close();
			cb.Flush();
		}

		return succeeded;
	}

	int clipboard_init(lua_State *L)
	{
		do_register_lib_table(L, std::vector<const char *>());
		lua_createtable(L, 0, 2);
		do_register_lib_function(L, "get", "bool (*)()", clipboard_get);
		do_register_lib_function(L, "set", "const char * (*)()", clipboard_set);
		lua_remove(L, -2); // ffi.cast function
		// Leaves lib table on the stack
		//register_lib_table(L, std::vector<const char *>(), "get", clipboard_get, "set", clipboard_set);
		return 1;
	}

	int frame_from_ms(lua_State *L)
	{
		int ms = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		VideoCtrl *video=Notebook::GetTab()->Video;
		if (video->GetState()!=None) {
			int frame=(video->IsDshow)? ((float)ms/1000.0)*video->fps : video->VFF->GetFramefromMS(ms);
			lua_pushnumber(L, frame);
		} else {
			lua_pushnil(L);
		}

		return 1;
	}

	int ms_from_frame(lua_State *L)
	{
		int frame = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		VideoCtrl *video=Notebook::GetTab()->Video;
		if (video->GetState()!=None) {
			int ms=(video->IsDshow)? ((frame*1000)/video->fps) : video->VFF->GetMSfromFrame(frame);
			//wxLogStatus("ms %i %i %i", ms , (int)((frame*1000)/video->fps), video->VFF->GetMSfromFrame(frame));
			lua_pushnumber(L, ms);
		} else {
			lua_pushnil(L);
		}
		return 1;
	}

	int video_size(lua_State *L)
	{
		VideoCtrl *video=Notebook::GetTab()->Video;
		if (video->GetState()!=None) {
			wxSize sz= video->GetVideoSize();
			float AR=(float)video->ax/video->ay;
			lua_pushnumber(L, sz.x);
			lua_pushnumber(L, sz.y);
			lua_pushnumber(L, AR);
			lua_pushnumber(L, (AR==1.0f)? 0: (AR<1.34f && AR>1.33f)? 1 : (AR<1.78f && AR>1.77f)? 2 : (AR<2.35f && AR>2.36f)? 3 : 4 );
			return 4;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}

	int get_keyframes(lua_State *L)
	{
		VideoCtrl *video=Notebook::GetTab()->Video;
		if (video->GetState()!=None && video->VFF){
			wxArrayInt & value = video->VFF->KeyFrames;
			lua_createtable(L, value.size(), 0);
			for (size_t i = 0; i < value.size(); ++i) {
				push_value(L, value[i]);
				lua_rawseti(L, -2, i + 1);
			}
		}else
			lua_pushnil(L);
		return 1;
	}

	int decode_path(lua_State *L)
	{
		wxString path = check_string(L, 1);
		wxLogStatus("path before"+path);
		TabPanel *pan=Notebook::GetTab();
		if(path[0]=='?'){
			if(path[1]=='a'&&path[4]=='i') path.replace(0,5,pan->VideoPath.BeforeLast('\\')+"\\");
			else if(path[1]=='d' && path[4]=='a') path.replace(0,6,Options.pathfull+"\\");
			else if(path[1]=='d' && path[4]=='t') path.replace(0,12,Options.pathfull+"\\Dictionary\\");
			else if(path[1]=='l' && path[4]=='a') path.replace(0,7,Options.pathfull+"\\");
			else if(path[1]=='s' && path[4]=='i') path.replace(0,8,Options.pathfull+"\\Include\\");
			else if(path[1]=='t' && path[4]=='p') path.replace(0,6,pan->SubsPath.BeforeLast('\\')+"\\");
			else if(path[1]=='u' && path[4]=='r') path.replace(0,6,Options.pathfull+"\\");
			else if(path[1]=='v' && path[4]=='e') path.replace(0,7,pan->VideoPath.BeforeLast('\\')+"\\");
		}
		wxLogStatus("path after"+path);
		push_value(L, path);
		return 1;
	}

	int cancel_script(lua_State *L)
	{
		lua_pushnil(L);
		throw error_tag();
	}

	int lua_text_textents(lua_State *L)
	{
		if (!lua_istable(L, 1)) {
			lua_pushstring(L, "Pierwszy argument dla text_extents musi byc tablica");
			lua_error(L);
		}
		if (!lua_isstring(L, 2)) {
			lua_pushfstring(L, "Drugi argument dla text_extents musi byc stringiem a jest typu %s", lua_typename(L, lua_type(L, 2)));
			lua_error(L);
		}

		lua_pushvalue(L, 1);
		SubsEntry *e = AutoToFile::LuaToLine(L);
		if(!e){return 0;}
		Styles *st= e->astyle;
		lua_pop(L, 1);
		//lua_pushstring(L, "Not a style entry");
		//lua_error(L);


		wxString text(lua_tostring(L, 2), wxConvUTF8);

		double width = 0, height =0, descent =0, extlead=0;
		double fontsize = wxAtoi( st->Fontsize ) * 32;
		double spacing = wxAtoi( st->Spacing ) * 32;



		HDC thedc = CreateCompatibleDC(0);
		if (!thedc) return false;
		SetMapMode(thedc, MM_TEXT);

		LOGFONTW lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = (LONG)fontsize;
		lf.lfWeight = st->Bold ? FW_BOLD : FW_NORMAL;
		lf.lfItalic = st->Italic;
		lf.lfUnderline = st->Underline;
		lf.lfStrikeOut = st->StrikeOut;
		lf.lfCharSet = wxAtoi(st->Encoding);
		lf.lfOutPrecision = OUT_TT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = ANTIALIASED_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
		_tcsncpy(lf.lfFaceName, st->Fontname.c_str(), 32);

		HFONT thefont = CreateFontIndirect(&lf);
		if (!thefont) return false;
		SelectObject(thedc, thefont);

		SIZE sz;
		size_t thetextlen = text.length();
		const TCHAR *thetext = text.wc_str();
		if (spacing != 0 ) {
			width = 0;
			for (unsigned int i = 0; i < thetextlen; i++) {
				GetTextExtentPoint32(thedc, &thetext[i], 1, &sz);
				width += sz.cx + spacing;
				height = sz.cy;
			}
		} else {
			GetTextExtentPoint32(thedc, thetext, (int)thetextlen, &sz);
			width = sz.cx;
			height = sz.cy;
		}


		TEXTMETRIC tm;
		GetTextMetrics(thedc, &tm);
		descent = tm.tmDescent;
		extlead= tm.tmExternalLeading;

		DeleteObject(thedc);
		DeleteObject(thefont);
		//wxLogMessage("style: %f, %f, %s, %s, %s", (float)width, (float)height, text.utf8_str().data(), st.ScaleX.utf8_str().data(), st.ScaleY.utf8_str().data());
		width = (wxAtoi(st->ScaleX) / 100.0) * (width / 32);
		height = (wxAtoi(st->ScaleY) / 100.0) * (height / 32);
		descent = (wxAtoi(st->ScaleY) / 100.0) * (descent / 32);
		extlead = (wxAtoi(st->ScaleY) / 100.0) * (extlead / 32);
		wxDELETE(e);

		lua_pushnumber(L, width);
		lua_pushnumber(L, height);
		lua_pushnumber(L, descent);
		lua_pushnumber(L, extlead);
		return 4;
	}

	int project_properties(lua_State *L)
	{
		const TabPanel *c=Notebook::GetTab();
		if (!c)
			lua_pushnil(L);
		else {
			lua_createtable(L, 0, 14);
#define PUSH_FIELD(name, fieldname) set_field(L, #name, c->Grid1->GetSInfo(#name))
			PUSH_FIELD(automation_scripts, "Automation Scripts");
			PUSH_FIELD(export_filters, "");
			PUSH_FIELD(export_encoding, "");
			PUSH_FIELD(style_storage, "Last Style Storage");
			PUSH_FIELD(video_zoom, "");
			PUSH_FIELD(ar_value, "");
			PUSH_FIELD(scroll_position, "Active Line");
			PUSH_FIELD(active_row, "Active Line");
			PUSH_FIELD(ar_mode, "");
			PUSH_FIELD(video_position, "");
#undef PUSH_FIELD
			set_field(L, "audio_file", c->VideoPath);
			set_field(L, "video_file", c->VideoPath);
			set_field(L, "timecodes_file", "");
			set_field(L, "keyframes_file", "");
		}
		return 1;
	}

	int DummyFilter(lua_State *L)
	{
		return 0;
	}

	


	LuaScript::LuaScript(wxString const& filename)
		: filename(filename)
		,L(NULL)
	{
		include_path.push_back(filename.BeforeLast('\\')+"\\");
		include_path.push_back(Options.pathfull+"\\Include\\");
		Create();
	}

	void LuaScript::Create()
	{
		Destroy();

		name = GetPrettyFilename();

		// create lua environment
		L = luaL_newstate();
		if (!L) {
			description = "Could not initialize Lua state";
			return;
		}

		bool loaded = false;
		//BOOST_SCOPE_EXIT_ALL(&) { if (!loaded) Destroy(); };
		LuaStackcheck stackcheck(L);

		// register standard libs
		preload_modules(L);
		stackcheck.check_stack(0);

		// dofile and loadfile are replaced with include
		lua_pushnil(L);
		lua_setglobal(L, "dofile");
		lua_pushnil(L);
		lua_setglobal(L, "loadfile");
		push_value(L, exception_wrapper<LuaInclude>);
		lua_setglobal(L, "include");

		// Replace the default lua module loader with our unicode compatible
		// one and set the module search path
		if (!Install(L, include_path)) {
			description = get_string_or_default(L, 1);
			lua_pop(L, 1);
			return;
		}
		stackcheck.check_stack(0);

		// prepare stuff in the registry

		// store the script's filename
		push_value(L, GetFilename());
		lua_setfield(L, LUA_REGISTRYINDEX, "filename");
		stackcheck.check_stack(0);

		// reference to the script object
		push_value(L, this);
		lua_setfield(L, LUA_REGISTRYINDEX, "kainote");
		stackcheck.check_stack(0);

		// make "kainote" table
		lua_pushstring(L, "kainote");
		lua_createtable(L, 0, 13);

		set_field<LuaCommand::LuaRegister>(L, "register_macro");
		set_field<DummyFilter>(L, "register_filter");//kainote nie ma exportu wiêc by skrypty siê nie sypa³y trzeba podes³aæ coœ fa³szywego.
		set_field<lua_text_textents>(L, "text_extents");
		set_field<frame_from_ms>(L, "frame_from_ms");
		set_field<ms_from_frame>(L, "ms_from_frame");
		set_field<video_size>(L, "video_size");
		set_field<get_keyframes>(L, "keyframes");
		set_field<decode_path>(L, "decode_path");
		set_field<cancel_script>(L, "cancel");
		set_field(L, "lua_automation_version", 4);
		set_field<clipboard_init>(L, "__init_clipboard");
		set_field<get_file_name>(L, "file_name");
		set_field<get_translation>(L, "gettext");
		set_field<project_properties>(L, "project_properties");

		// store kainote table to globals
		lua_settable(L, LUA_GLOBALSINDEX);
		stackcheck.check_stack(0);

		// load user script
		if (!LoadFile(L, GetFilename())) {
			description = get_string_or_default(L, 1);
			lua_pop(L, 1);
			return;
		}
		stackcheck.check_stack(1);

		// Insert our error handler under the user's script
		lua_pushcclosure(L, add_stack_trace, 0);
		lua_insert(L, -2);

		// and execute it
		// this is where features are registered
		if (lua_pcall(L, 0, 0, -2)) {
			// error occurred, assumed to be on top of Lua stack
			description = wxString::Format("B³¹d inicjalizacji skryptu Lua \"%s\":\n\n%s", GetPrettyFilename(), get_string_or_default(L, -1));
			//lua_pop(L, 1);
			lua_pop(L, 2); // error + error handler
			return;
		}
		lua_pop(L, 1); // error handler
		stackcheck.check_stack(0);

		lua_getglobal(L, "version");
		if (lua_isnumber(L, -1) && lua_tointeger(L, -1) == 3) {
			lua_pop(L, 1); // just to avoid tripping the stackcheck in debug
			description = "Attempted to load an Automation 3 script as an Automation 4 Lua script. Automation 3 is no longer supported.";
			return;
		}

		name = get_global_string(L, "script_name");
		description = get_global_string(L, "script_description");
		author = get_global_string(L, "script_author");
		version = get_global_string(L, "script_version");

		if (name.empty())
			name = GetPrettyFilename();

		lua_pop(L, 1);
		// if we got this far, the script should be ready
		loaded = true;
		//wxLogStatus("names "+name+" "+description+" "+author+" "+version);
	}

	void LuaScript::Destroy()
	{
		// Assume the script object is clean if there's no Lua state
		if (!L) return;

		// loops backwards because commands remove themselves from macros when
		// they're unregistered
		for (auto i = macros.begin(); i != macros.end(); i++)
			delete (*i);


		lua_close(L);
		L = nullptr;
	}


	void LuaScript::RegisterCommand(LuaCommand *command)
	{
		for (auto macro : macros) {
			if (macro->StrDisplay() == command->StrDisplay()) {
				error(L, "A macro named '%s' is already defined in script '%s'",
					command->StrDisplay().utf8_str().data(), name.utf8_str().data());
			}
		}
		macros.push_back(command);
	}

	void LuaScript::UnregisterCommand(LuaCommand *command)
	{
		auto result = std::find(macros.begin(), macros.end(), command);
		macros.erase(result);
		delete (*result);
	}

	LuaScript* LuaScript::GetScriptObject(lua_State *L)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, "kainote");
		void *ptr = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return (LuaScript*)ptr;
	}


	int LuaScript::LuaInclude(lua_State *L)
	{
		const LuaScript *s = GetScriptObject(L);

		const wxString filename(check_string(L, 1));
		wxString filepath;
		bool fullpath=false;
		// Relative or absolute path
		if (filename.find("\\")!=-1 || filename.find("/")!=-1){
			filepath = filename;//s->GetFilename().BeforeLast('//') + filename;
			fullpath=true;
		}
		if (!wxFileExists(filepath)) { // Plain filename
			if(fullpath){filepath=filepath.AfterLast('\\');}
			for (auto const& dir : s->include_path) {
				//wxLogStatus("dir: \""+dir+"\" name: \""+filename);
				filepath = dir + filename;
				if (wxFileExists(filepath))
					break;
			}
		}

		if (!wxFileExists(filepath))
			return error(L, "Lua include not found: %s", filepath.mb_str(wxConvUTF8).data());

		if (!LoadFile(L, filepath))
			return error(L, "Error loading Lua include \"%s\":\n%s", filepath.mb_str(wxConvUTF8).data(), check_string(L, 1).mb_str(wxConvUTF8).data());

		int pretop = lua_gettop(L) - 1; // don't count the function value itself
		lua_call(L, 0, LUA_MULTRET);
		return lua_gettop(L) - pretop;
	}

	bool LuaScript::CheckLastModified(bool check)
	{
		FILETIME ft;

		HANDLE ffile = CreateFile(GetFilename().wc_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		GetFileTime(ffile,0,0,&ft);
		CloseHandle(ffile);
		if(check){
			if(LowTime!=(int)ft.dwLowDateTime || HighTime != ft.dwHighDateTime){
				LowTime=ft.dwLowDateTime;
				HighTime=ft.dwHighDateTime;
				return true;
			}
		}

		LowTime=ft.dwLowDateTime;
		HighTime=ft.dwHighDateTime;

		return false;
	}

	LuaThreadedCall::LuaThreadedCall(lua_State *_L)
		: wxThread(wxTHREAD_JOINABLE)
		, L(_L)
	{
		
		//wxLogStatus("create");
		Create();
		//wxLogStatus("priority");
		SetPriority(50);
		//wxLogStatus("run");
		Run();
	}

	wxThread::ExitCode LuaThreadedCall::Entry()
	{
	
		bool failed = false;
		int nargs=3, nresults=2;
		LuaProgressSink *ps = LuaProgressSink::ps;
		
		// Insert our error handler under the function to call
		lua_pushcclosure(L, add_stack_trace, 0);
		lua_insert(L, -nargs - 2);

		if (lua_pcall(L, nargs, nresults, -nargs - 2)) {
			if (!lua_isnil(L, -1) && ps) {
				// if the call failed, log the error here
				wxString errmsg(get_string_or_default(L, -1));
				errmsg.Prepend(_("Wyst¹pi³ b³¹d podczas wykonywania skryptu Lua:\n"));
				ps->SafeQueue(Auto::EVT_MESSAGE,errmsg);
			}
			lua_pop(L, 2);
			failed = true;
		}
		else
			lua_remove(L, -nresults - 1);


		//lua_gc(L, LUA_GCCOLLECT, 0);
		if(!failed && ps->lpd->pending_debug_output==""){ps->lpd->closedialog=true;}else{ps->lpd->finished=true;}

		//if (failed)
			//throw "Script threw an error";
		if (failed){ return (wxThread::ExitCode) 1;}
		return 0;
	}

	// LuaFeature
	void LuaFeature::RegisterFeature()
	{
		myid = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	void LuaFeature::UnregisterFeature()
	{
		luaL_unref(L, LUA_REGISTRYINDEX, myid);
	}

	void LuaFeature::GetFeatureFunction(const char *function) const
	{
		// get this feature's function pointers
		lua_rawgeti(L, LUA_REGISTRYINDEX, myid);
		// get pointer for validation function
		push_value(L, function);
		lua_rawget(L, -2);
		// remove the function table
		lua_remove(L, -2);
		assert(lua_isfunction(L, -1));
	}

	// LuaFeatureMacro
	int LuaCommand::LuaRegister(lua_State *L)
	{
		//static std::mutex mutex;
		auto command = new LuaCommand(L);
		//{
			//std::lock_guard<std::mutex> lock(mutex);
			//Command::reg(std::move(command));
		//}

		return 0;
	}

	LuaCommand::LuaCommand(lua_State *L)
		: LuaFeature(L)
		//, display(check_string(L, 1))
		//, help(get_string(L, 2))
		, cmd_type(COMMAND_NORMAL)
		//, wxMenuItem(NULL,-1,display,help)
	{
		
		
		display = check_string(L, 1);
		
		help=get_string(L, 2);
		
		lua_getfield(L, LUA_REGISTRYINDEX, "filename");
		//cmd_name = wxString::Format("automation/lua/%s/%s", check_string(L, -1), check_string(L, 1));
		//wxLogStatus(cmd_name);
		if (!lua_isfunction(L, 3))
			error(L, "The macro processing function must be a function");

		if (lua_isfunction(L, 4))
			cmd_type |= COMMAND_VALIDATE;

		if (lua_isfunction(L, 5))
			cmd_type |= COMMAND_TOGGLE;

		// new table for containing the functions for this feature
		lua_createtable(L, 0, 3);

		// store processing function
		push_value(L, "run");
		lua_pushvalue(L, 3);
		lua_rawset(L, -3);

		// store validation function
		push_value(L, "validate");
		lua_pushvalue(L, 4);
		lua_rawset(L, -3);

		// store active function
		push_value(L, "isactive");
		lua_pushvalue(L, 5);
		lua_rawset(L, -3);

		// store the table in the registry
		RegisterFeature();
		
		LuaScript::GetScriptObject(L)->RegisterCommand(this);
		
	}

	LuaCommand::~LuaCommand()
	{
		UnregisterFeature();
		//LuaScript::GetScriptObject(L)->UnregisterCommand(this);
	}

	static std::vector<int> selected_rows(const TabPanel *c)
	{
		auto const& sel = c->Grid1->GetSels();
		int offset = c->Grid1->SInfoSize() + c->Grid1->StylesSize()+1;
		std::vector<int> rows;
		rows.reserve(sel.size());
		for (auto line : sel)
			rows.push_back(line + offset);
		sort(rows.begin(), rows.end());
		return rows;
	}

	bool LuaCommand::Validate(const TabPanel *c)
	{
		if (!(cmd_type & COMMAND_VALIDATE)) return true;

		// Error handler goes under the function to call
		lua_pushcclosure(L, add_stack_trace, 0);

		GetFeatureFunction("validate");
		auto subsobj = new AutoToFile(L, c->Grid1->file->GetSubs(), true);

		push_value(L, selected_rows(c));
		push_value(L, c->Edit->ebrow + c->Grid1->SInfoSize() + c->Grid1->StylesSize()+1);

		int err = lua_pcall(L, 3, 2, -5 /* three args, function, error handler */);
		SAFE_DELETE(subsobj);

		if (err) {
			wxLogWarning("Runtime error in Lua macro validation function:\n%s", get_wxstring(L, -1));
			lua_pop(L, 2);
			return false;
		}

		bool result = !!lua_toboolean(L, -2);

		wxString new_help_string(get_wxstring(L, -1));
		if (new_help_string.size()) {
			help = new_help_string;
			cmd_type |= COMMAND_DYNAMIC_HELP;
		}

		lua_pop(L, 3); // two return values and error handler

		return result;
	}

	void LuaCommand::Run(TabPanel *c)
	{
		LuaStackcheck stackcheck(L);

		stackcheck.check_stack(0);

		GetFeatureFunction("run");
		auto subsobj = new AutoToFile(L, c->Grid1->file->GetSubs(), true);

		int original_offset = c->Grid1->SInfoSize() + c->Grid1->StylesSize()+1;
		auto original_sel = selected_rows(c);
		int original_active = c->Edit->ebrow + original_offset;

		push_value(L, original_sel);
		push_value(L, original_active);
		LuaProgressSink *ps = new LuaProgressSink(L, c);

		
		LuaThreadedCall call(L);
		
		ps->ShowDialog(StrDisplay());
		wxThread::ExitCode code = call.Wait();
		bool failed = (int)code == 1;
		wxLogStatus("waited ");
		
		if(ps->lpd->cancelled || failed){
			wxLogStatus("canceled ");
			subsobj->Cancel();
			c->Grid1->file->DummyUndo();
			
			delete ps;
			return;
		}
			//wxLogStatus("modal & finished %i %i", (int)ps->lpd->IsModal(), (int)ps->lpd->finished);
		//if(ps->lpd->IsModal() && ps->lpd->finished){ps->lpd->EndModal(0);}
		c->Grid1->SpellErrors.clear();
		c->Grid1->SetModified(false);
		c->Grid1->RepaintWindow();	
		//if(ps->lpd->cancelled && ps->lpd->IsModal()){ps->lpd->EndModal(0);}

		//auto lines = subsobj->ProcessingComplete(StrDisplay(c));
		wxLogStatus("select lines");
		int active_idx = original_active;

		// Check for a new active row
		if (lua_isnumber(L, -1)) {
			active_idx = lua_tointeger(L, -1);
			if (active_idx < 1 || active_idx > c->Grid1->GetCount()) {
				wxLogError("Active row %d is out of bounds (must be 1-%u)", active_idx, c->Grid1->GetCount());
				active_idx = original_active;
			}
		}

		//stackcheck.check_stack(2);
		lua_pop(L, 1);

		// top of stack will be selected lines array, if any was returned
		if (lua_istable(L, -1)) {
			lua_for_each(L, [&] {
				if (!lua_isnumber(L, -1))
					return;
				int cur = lua_tointeger(L, -1);
				if (cur < 1 || cur > c->Grid1->GetCount()) {
					wxLogError("Selected row %d is out of bounds (must be 1-%u)", cur, c->Grid1->GetCount());
					throw LuaForEachBreak();
				}

				c->Grid1->sel[cur - original_offset -1]=true;
			});

			/*AssDialogue *new_active = c->selectionController->GetActiveLine();
			if (active_line && (active_idx > 0 || !sel.count(new_active)))
			new_active = active_line;
			if (sel.empty())
			sel.insert(new_active);
			c->selectionController->SetSelectionAndActive(std::move(sel), new_active);*/
		}
		else {
			lua_pop(L, 1);

			/*Selection new_sel;
			AssDialogue *new_active = nullptr;

			int prev = original_offset;
			auto it = c->ass->Events.begin();
			for (int row : original_sel) {
			while (row > prev && it != c->ass->Events.end()) {
			++prev;
			++it;
			}
			if (it == c->ass->Events.end()) break;
			new_sel.insert(&*it);
			if (row == original_active)
			new_active = &*it;
			}

			if (new_sel.empty() && !c->ass->Events.empty())
			new_sel.insert(&c->ass->Events.front());
			if (!new_sel.count(new_active))
			new_active = *new_sel.begin();
			c->selectionController->SetSelectionAndActive(std::move(new_sel), new_active);*/
		}
		SAFE_DELETE(subsobj);
		SAFE_DELETE(ps);
		//stackcheck.check_stack(0);
	}

	bool LuaCommand::IsActive(const TabPanel *c)
	{
		if (!(cmd_type & COMMAND_TOGGLE)) return false;

		LuaStackcheck stackcheck(L);

		stackcheck.check_stack(0);

		GetFeatureFunction("isactive");
		auto subsobj = new AutoToFile(L, c->Grid1->file->GetSubs(), true);
		push_value(L, selected_rows(c));
		push_value(L, c->Edit->ebrow + c->Grid1->SInfoSize() + c->Grid1->StylesSize()+1);

		int err = lua_pcall(L, 3, 1, 0);

		bool result = false;
		if (err)
			wxLogWarning("Runtime error in Lua macro IsActive function:\n%s", get_wxstring(L, -1));
		else
			result = !!lua_toboolean(L, -1);

		// clean up stack (result or error message)
		stackcheck.check_stack(1);
		lua_pop(L, 1);
		SAFE_DELETE(subsobj);
		return result;
	}

	void LuaCommand::RunScript()
	{
		LuaScript *script = LuaScript::GetScriptObject(L);
		if(script->CheckLastModified(true)){script->Reload();}
	
		TabPanel *pan = Notebook::GetTab();
		if(Validate(pan)){Run(pan);}
		


	}



	Automation::Automation()
	{
		AutoloadPath=Options.pathfull+"\\Autoload";
		ReloadScripts(true);
	}

	Automation::~Automation()
	{
		RemoveAll(true);
	}

	bool Automation::Add(wxString filename, bool autoload)
	{
		
		std::vector<Auto::LuaScript*> &scripts = (autoload)? Scripts : ASSScripts;
		Auto::LuaScript *ls= new Auto::LuaScript(filename);
		for (size_t i = 0; i < scripts.size(); i++) {
			if (ls->GetName() == scripts[i]->GetName()){delete ls; ls=NULL; return false;}
		}
		ls->CheckLastModified(false);
		scripts.push_back(ls);
		if(!autoload){
			wxString scriptpaths = Notebook::GetTab()->Grid1->GetSInfo("Automation Scripts");
			scriptpaths<<"|"<<filename;
			Notebook::GetTab()->Grid1->AddSInfo("Automation Scripts", scriptpaths);
		}
		HasChanges=true;
		//wxLogStatus("description: " + ls->GetDescription());
		return true;
	}

	void Automation::Remove(int script)
	{
		HasChanges=true;
		std::vector<Auto::LuaScript*>::iterator i= ASSScripts.begin()+script;
		delete *i;
		ASSScripts.erase(i);
	}

	void Automation::RemoveAll(bool autoload)
	{
		
		if(autoload){
			for (auto i = Scripts.begin(); i != Scripts.end(); i++) {
				delete *i;
			}
			Scripts.clear();}
		//if(!autoload){
			for (auto i = ASSScripts.begin(); i != ASSScripts.end(); i++) {
				delete *i;
			}
			ASSScripts.clear();
		//}
	}

	void Automation::ReloadMacro(int script)
	{
		ASSScripts[script]->Reload();
	}

	/*void Automation::RunScript(int script, int macro)
	{
		if(Scripts[script]->CheckLastModified(true)){Scripts[script]->Reload();}
		Auto::LuaCommand * macros = Scripts[script]->GetMacro(macro);
		if(macro >= macros.size()){
			wxMessageBox("Wybrane makro przekracza tablicê, co nie powinno siê zdarzyæ, jedynie w przypadku b³êdu w kodzie");
		}
		TabPanel *pan = Notebook::GetTab();
		if(macros[macro]->Validate(pan)){macros[macro]->Run(pan);}


	}*/

	void Automation::ReloadScripts(bool first)
	{


		if(!first){RemoveAll(true);}
		int error_count = 0;

		//wxStringTokenizer tok(path, "|", wxTOKEN_STRTOK);
		//while (tok.HasMoreTokens()) {
		wxDir dir;
		//wxString dirname = StandardPaths::DecodePath(tok.GetNextToken());
		if (!dir.Open(AutoloadPath)) {
			//wxLogWarning("Failed to open a directory in the Automation autoload path: %s", dirname.c_str());
			return;
		}


		wxString fn;
		wxFileName script_path(AutoloadPath, "");
		bool more = dir.GetFirst(&fn, wxEmptyString, wxDIR_FILES);

		while (more) {
			script_path.SetName(fn);
			try {
				wxString fullpath = script_path.GetFullPath();
				wxString ext = fullpath.AfterLast('.').Lower();

				if((ext != "lua" && ext != "moon") || !Add(fullpath, true)){more = dir.GetNext(&fn);continue;}

				if (!Scripts[Scripts.size()-1]->GetLoadedState()) {error_count++;}


			}
			catch (const wchar_t *e) {
				error_count++;
				wxLogError(_("B³¹d wczytywania skryptu Lua: %s\n%s"), fn.c_str(), e);
			}
			catch (...) {
				error_count++;
				wxLogError(_("Nieznany b³¹d wczytywania skryptu Lua: %s."), fn.c_str());
			}

			more = dir.GetNext(&fn);
		}
		//}wxLogStatus("wesz³o");
		//wxLogStatus("po pêtli");
		if (error_count > 0) {
			wxLogWarning(_("Jeden b¹dŸ wiêcej skryptów autoload zawiera b³êdy,\n obejrzyj opisy skryptów by uzyskaæ wiêcej informacji."));
		}


	}

	void Automation::AddFromSubs()
	{
		//wxLogStatus("wesz³o");
		wxString paths=Notebook::GetTab()->Grid1->GetSInfo("Automation Scripts");
		//wxLogStatus("m"+paths);
		if(paths==""){return;}
		if(paths==scriptpaths && ASSScripts.size()>0){return;}
		paths.Trim(false);
		wxStringTokenizer token(paths,"|~$",wxTOKEN_RET_EMPTY_ALL);
		int error_count=0;
		while(token.HasMoreTokens())
		{
			wxString onepath=token.GetNextToken();
			onepath.Trim(false);
			//wxLogStatus(onepath);
			if(!wxFileExists(onepath)){continue;}

			try {
				if(!Add(onepath)){continue;}
				int last=Scripts.size()-1;

			}
			catch (const wchar_t *e) {
				error_count++;
				wxLogError(_("B³¹d wczytywania skryptu Lua: %s\n%s"), onepath.c_str(), e);
			}
			catch (...) {
				error_count++;
				wxLogError(_("Nieznany b³¹d wczytywania skryptu Lua: %s."), onepath.c_str());
			}
		}
		if (error_count > 0) {
			wxLogWarning(_("Co najmniej jeden skrypt z pliku napisów zawiera b³êdy.\nZobacz opisy skryptów, by uzyskaæ wiêcej informacji."));
		}
	}

	void Automation::OnEdit(wxString &Filename)
	{
		wxString editor=Options.GetString("Script Editor");
		if(editor=="" || wxGetKeyState(WXK_SHIFT)){
			editor = wxFileSelector(_("Wybierz edytor skryptów"), "",
				"C:\\Windows\\Notepad.exe", "exe", _("Programy (*.exe)|*.exe|Wszystkie pliki (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
			if(!wxFileExists(editor)){return;}
			Options.SetString("Script Editor",editor);
			Options.SaveOptions();
		}
		
		wxWCharBuffer editorbuf = editor.c_str(), sfnamebuf = Filename.c_str();
		wchar_t **cmdline = new wchar_t*[3];
		cmdline[0] = editorbuf.data();
		cmdline[1] = sfnamebuf.data();
		cmdline[2] = 0;

		long res = wxExecute(cmdline);
		delete cmdline;

		if (!res) {
			wxMessageBox(_("Nie mo¿na uruchomiæ edytora."), _("B³¹d automatyzacji"), wxOK|wxICON_ERROR);
		}
	}

	bool Automation::CheckChanges()
	{
		for(auto script : Scripts){
			if(script->CheckLastModified()){HasChanges=true; break;}
		}
		return HasChanges;
	}

	VOID CALLBACK callbackfunc ( PVOID   lpParameter, BOOLEAN TimerOrWaitFired) {
		Automation *auto_ = (Automation*)lpParameter;
		auto_->BuildMenu(auto_->bar);
		DeleteTimerQueueTimer(auto_->handle,0,0);
	}

	void Automation::BuildMenuWithDelay(wxMenu **_bar, int time)
	{
		bar=_bar;
		CreateTimerQueueTimer(&handle,NULL,callbackfunc,this,time,0,0);
	}
		
	void Automation::BuildMenu(wxMenu **bar)
	{
		TabPanel* c = Notebook::GetTab();
		//if(!CheckChanges()){return;}

		for(int j=(*bar)->GetMenuItemCount()-1; j>=2; j--){
			//wxLogStatus("deleted %i", j);
			(*bar)->Destroy((*bar)->FindItemByPosition(j));
		}
		AddFromSubs();
		int start=30100, i=0;
		for(auto script : Scripts){
			wxMenu *submenu=new wxMenu();
			submenu->Append(start,_("Edytuj"),_("Edytuj"));
			Actions[start]= RunFunction(start, -2, script);
			start++;
			submenu->Append(start,_("Odœwie¿"),_("Odœwie¿"));
			Actions[start]= RunFunction(start, -1, script);
			start++;
			int j=0;
			auto macros = script->GetMacros();
			//wxLogStatus("size %i", macros.size());
			for(auto macro : macros){
				wxString text; text<<"Script"<<i<<"-"<<j;
				macro->SetHotkey(text);
				Hkeys.SetAccMenu(submenu, new wxMenuItem(0,start,macro->StrDisplay(),macro->StrHelp()), text)->Enable(macro->Validate(c));
				Actions[start]= RunFunction(start, j, script);
				start++;
				j++;
			}
			(*bar)->Append(-1, script->GetName(), submenu,script->GetDescription());
			i++;
		}
		for(auto script : ASSScripts){
			wxMenu *submenu=new wxMenu();
			submenu->Append(start,_("Edytuj"),_("Edytuj"));
			Actions[start]= RunFunction(start, -2, script);
			start++;
			submenu->Append(start,_("Odœwie¿"),_("Odœwie¿"));
			Actions[start]= RunFunction(start, -1, script);
			start++;
			int j=0;
			auto macros = script->GetMacros();
			//wxLogStatus("size %i", macros.size());
			for(auto macro : macros){
				wxString text; text<<"Script"<<i<<"-"<<j;
				macro->SetHotkey(text);
				Hkeys.SetAccMenu(submenu, new wxMenuItem(0,start,macro->StrDisplay(),macro->StrHelp()), text)->Enable(macro->Validate(c));
				Actions[start]= RunFunction(start, j, script);
				start++;
				j++;
			}
			(*bar)->Append(-1, script->GetName(), submenu,script->GetDescription());
			i++;
		}
		kainoteFrame *Kai=((kainoteApp*)wxTheApp)->Frame;
		//wxLogStatus("start %i", start);
		
		HasChanges=false;
	}


//MyMenu::MyMenu(Auto::LuaScript *_script)
//	:wxMenu()
//	,script(_script)
//{
//}


void Automation::OnMenuClick(wxCommandEvent &event)
{
	wxLogStatus("menu click %i", event.GetId());
	auto action =  Actions.find(event.GetId());
	if(action!=Actions.end()){
		action->second.Run();
	}
}

void RunFunction::Run(){
		if(element==-2){
			Automation::OnEdit(script->GetFilename());
		}else if(element==-1){
			script->Reload();
		}else{
			LuaCommand *macro=script->GetMacro(element);
				if(wxGetKeyState(WXK_SHIFT)){
				wxString wins[1]={"Globalny"};
				//upewnij siê, ¿e da siê zmieniæ idy na nazwy, 
				//mo¿e i trochê spowolni operacjê ale skoñczy siê ci¹g³e wywalanie hotkeysów
				//mo¿e od razu funkcji onmaphotkey przekazaæ item by zrobi³a co trzeba
				wxString name = macro->StrHotkey();
				int ret=-1;
				kainoteFrame *Kai = ((kainoteApp*)wxTheApp)->Frame;
				ret=Hkeys.OnMapHkey( id, name, Kai, wins, 1);
				if(ret==-1){Kai->MenuBar->FindItem(id)->SetAccel(&Hkeys.GetHKey(id));Hkeys.SaveHkeys();}
				else if(ret>0){
					wxMenuItem *item= Kai->MenuBar->FindItem(ret);
					wxAcceleratorEntry entry;
					item->SetAccel(&entry);
				}
				return;
			}

			macro->RunScript();
		}
	}
}