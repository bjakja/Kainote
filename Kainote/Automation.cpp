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

/// @file auto4_lua.cpp
/// @brief Lua 5.1-based scripting engine
/// @ingroup scripting
///
#include "config.h"
#include "Automation.h"
#include "Hotkeys.h"

#include "KainoteApp.h"
#include "AutomationToFile.h"
#include "AutomationProgress.h"

#include "AutomationUtils.h"
#include "AutomationScriptReader.h"
#include "KaiMessageBox.h"
#include "AutomationHotkeysDialog.h"
#include "Notebook.h"
#include "VideoBox.h"
#include "SubsGrid.h"
#include "stylestore.h"

#include <algorithm>
#include <cassert>
#include <mutex>
#include <wx/clipbrd.h>
#include <wx/filedlg.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
//#include <thread>
//#include <tuple>


namespace Auto{


	int get_file_name(lua_State *L)
	{
		TabPanel *tab = Notebook::GetTab();
		if (tab && tab->SubsPath != L"")
			push_value(L, tab->SubsName);
		else
			lua_pushnil(L);
		return 1;
	}

	int get_translation(lua_State *L)
	{
		wxString str(check_string(L, 1));
		push_value(L, wxGetTranslation(str));
		return 1;
	}

	char *clipboard_get()
	{
		std::string data;
		wxClipboard *cb = wxClipboard::Get();
		if (cb->Open()) {
			if (cb->IsSupported(wxDF_TEXT) || cb->IsSupported(wxDF_UNICODETEXT)) {
				wxTextDataObject raw_data;
				cb->GetData(raw_data);
				data = raw_data.GetText().ToStdString();
			}
			cb->Close();
		}
		if (data.empty())
			return nullptr;
		return strndup(data);
	}

	bool clipboard_set(const char *str)
	{
		bool succeeded = false;

		//#if wxUSE_OLE
		//		// OLE needs to be initialized on each thread that wants to write to
		//		// the clipboard, which wx does not handle automatically
		//		wxClipboard cb;
		//#else
		//wxClipboard &cb = *wxTheClipboard;
		//#endif
		wxClipboard *cb = wxClipboard::Get();
		if (cb->Open()) {
			succeeded = cb->SetData(new wxTextDataObject(wxString::FromUTF8(str)));
			cb->Close();
			cb->Flush();
		}

		return succeeded;
	}

	int clipboard_init(lua_State *L)
	{
		do_register_lib_table(L, std::vector<const char *>());
		lua_createtable(L, 0, 2);
		do_register_lib_function(L, "get", "char *(*)()", clipboard_get);
		do_register_lib_function(L, "set", "bool (*)(const char *)", clipboard_set);
		lua_remove(L, -2); // ffi.cast function
		// Leaves lib table on the stack
		//register_lib_table(L, std::vector<const char *>(), "get", clipboard_get, "set", clipboard_set);
		return 1;
	}

	int frame_from_ms(lua_State *L)
	{
		int ms = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		TabPanel *tab = Notebook::GetTab();
		if (tab && tab->video->GetState() != None) {
			float FPS;
			tab->video->GetFPSAndAspectRatio(&FPS, NULL, NULL, NULL);
			int frame = (tab->video->IsDirectShow()) ? ((float)ms / 1000.f) * FPS :
				tab->video->GetFFMS2()->GetFramefromMS(ms, 0, false);
			lua_pushnumber(L, frame);
		}
		else {
			lua_pushnil(L);
		}

		return 1;
	}

	int ms_from_frame(lua_State *L)
	{
		int frame = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		TabPanel *tab = Notebook::GetTab();
		if (tab && tab->video->GetState() != None) {
			float FPS;
			tab->video->GetFPSAndAspectRatio(&FPS, NULL, NULL, NULL);
			int ms = (tab->video->IsDirectShow()) ? ((frame * 1000) / FPS) :
				tab->video->GetFFMS2()->GetMSfromFrame(frame);
			lua_pushnumber(L, ms);
		}
		else {
			lua_pushnil(L);
		}
		return 1;
	}

	int video_size(lua_State *L)
	{
		TabPanel *tab = Notebook::GetTab();
		if (tab && tab->video->GetState() != None) {
			wxSize sz = tab->video->GetVideoSize();
			int AspectRatioX, AspectRatioY;
			tab->video->GetFPSAndAspectRatio(NULL, NULL, &AspectRatioX, &AspectRatioY);
			float AR = (float)AspectRatioX / AspectRatioY;
			lua_pushnumber(L, sz.x);
			lua_pushnumber(L, sz.y);
			lua_pushnumber(L, AR);
			lua_pushnumber(L, (AR == 1.0f) ? 0 : (AR < 1.34f && AR > 1.33f) ? 1 :
				(AR < 1.78f && AR > 1.77f) ? 2 : (AR < 2.35f && AR > 2.36f) ? 3 : 4);
			return 4;
		}
		else {
			lua_pushnil(L);
			return 1;
		}
	}

	int get_keyframes(lua_State *L)
	{
		TabPanel *tab = Notebook::GetTab();
		if (tab->video->GetState() != None && tab->video->HasFFMS2()){
			const wxArrayInt & value = tab->video->GetFFMS2()->GetKeyframes();
			lua_createtable(L, value.size(), 0);
			for (size_t i = 0; i < value.size(); ++i) {
				push_value(L, value[i]);
				lua_rawseti(L, -2, i + 1);
			}
		}
		else
			lua_pushnil(L);
		return 1;
	}

	int decode_path(lua_State *L)
	{
		wxString path = check_string(L, 1);
		TabPanel *tab = Notebook::GetTab();
		path.Replace(L'/', L'\\');
		wxString firstAutomation = Options.pathfull + "\\Automation";
		if (path[0] == L'?'){
			if (path[1] == L'a' && path[4] == L'i') path.replace(0, 6, (tab) ? tab->VideoPath.BeforeLast(L'\\') : wxString(L""));
			else if (path[1] == L'd' && path[4] == L'a') path.replace(0, 5, firstAutomation);
			else if (path[1] == L'd' && path[4] == L't') path.replace(0, 11, Options.pathfull + wxString(L"\\Dictionary"));
			else if (path[1] == L'l' && path[4] == L'a') path.replace(0, 6, firstAutomation);
			else if (path[1] == L's' && path[4] == L'i') path.replace(0, 7, (tab) ? tab->SubsPath.BeforeLast(L'\\') : wxString(L""));
			else if (path[1] == L't' && path[4] == L'p') path.replace(0, 5, firstAutomation + wxString(L"\\temp"));
			else if (path[1] == L'u' && path[4] == L'r') path.replace(0, 5, firstAutomation);
			else if (path[1] == L'v' && path[4] == L'e') path.replace(0, 6, (tab) ? tab->VideoPath.BeforeLast(L'\\') : wxString(L""));
		}
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
			lua_pushstring(L, "First argument of text_extents must be a table");
			lua_error(L);
		}
		if (!lua_isstring(L, 2)) {
			lua_pushfstring(L, "Second argument of text_extents must be a string but is of type %s", lua_typename(L, lua_type(L, 2)));
			lua_error(L);
		}

		lua_pushvalue(L, 1);
		SubsEntry *e = AutoToFile::LuaToLine(L);
		if (!e){ return 0; }
		if (e->lclass != L"style"){ SAFE_DELETE(e); return 0; }
		Styles *st = e->astyle;
		lua_pop(L, 1);
		//lua_pushstring(L, "Not a style entry");
		//lua_error(L);


		wxString text(lua_tostring(L, 2), wxConvUTF8);

		if(text.empty()){
			SAFE_DELETE(e);
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			return 4;
		}

		double width = 0, height = 0, descent = 0, extlead = 0;
		double fontsize = st->GetFontSizeDouble() * 32;
		double spacing = wxAtof(st->Spacing) * 32;

		SIZE sz;
		size_t thetextlen = text.length();
		const TCHAR* thetext = text.wc_str();

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
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		_tcsncpy(lf.lfFaceName, st->Fontname.wc_str(), 32);

		HFONT thefont = CreateFontIndirect(&lf);
		if (!thefont) return false;
		SelectObject(thedc, thefont);

		if (spacing != 0) {
			width = 0;
			for (unsigned int i = 0; i < thetextlen; i++) {
				GetTextExtentPoint32(thedc, &thetext[i], 1, &sz);
				width += sz.cx + spacing;
				height = sz.cy;
			}
		}
		else {
			GetTextExtentPoint32(thedc, thetext, (int)thetextlen, &sz);
			width = sz.cx;
			height = sz.cy;
		}


		TEXTMETRIC tm;
		GetTextMetrics(thedc, &tm);
		descent = tm.tmDescent;
		extlead = tm.tmExternalLeading;

		DeleteObject(thedc);
		DeleteObject(thefont);
		
		double scalex = wxAtof(st->ScaleX) / 100.0;
		double scaley = wxAtof(st->ScaleY) / 100.0;
		width = scalex * (width / 32);
		height = scaley * (height / 32);
		descent = scaley * (descent / 32);
		extlead = scaley * (extlead / 32);
		SAFE_DELETE(e);

		lua_pushnumber(L, width);
		lua_pushnumber(L, height);
		lua_pushnumber(L, descent);
		lua_pushnumber(L, extlead);
		return 4;
	}

	VideoFrame * check_VideoFrame(lua_State* L) {
		VideoFrame* framePtr = *static_cast<VideoFrame**>(luaL_checkudata(L, 1, "VideoFrame"));
		return framePtr;
	}

	int FrameWidth(lua_State* L) {
		VideoFrame * frame = check_VideoFrame(L);
		push_value(L, frame->width);
		return 1;
	}

	int FrameHeight(lua_State* L) {
		VideoFrame * frame = check_VideoFrame(L);
		push_value(L, frame->height);
		return 1;
	}

	int FramePixel(lua_State* L) {
		VideoFrame * frame = check_VideoFrame(L);
		size_t x = lua_tointeger(L, -2);
		size_t y = lua_tointeger(L, -1);
		lua_pop(L, 2);

		if (x < frame->width && y < frame->height) {
			if (frame->flipped)
				y = frame->height - y;

			size_t pos = y * frame->pitch + x * 4;
			// VideoFrame is stored as BGRA, but we want to return RGB
			int pixelValue = frame->data[pos + 2] * 65536 + frame->data[pos + 1] * 256 + frame->data[pos];
			push_value(L, pixelValue);
		}
		else {
			lua_pushnil(L);
		}
		return 1;
	}

	int FramePixelFormatted(lua_State* L) {
		VideoFrame * frame = check_VideoFrame(L);
		size_t x = lua_tointeger(L, -2);
		size_t y = lua_tointeger(L, -1);
		lua_pop(L, 2);

		if (x < frame->width && y < frame->height) {
			if (frame->flipped)
				y = frame->height - y;

			size_t pos = y * frame->pitch + x * 4;
			// VideoFrame is stored as BGRA, Color expects RGBA
			AssColor* color = new AssColor(frame->data[pos + 2], 
				frame->data[pos + 1], frame->data[pos], frame->data[pos + 3]);
			push_value(L, color->GetAss(true));
		}
		else {
			lua_pushnil(L);
		}
		return 1;
	}

	int FrameDestory(lua_State* L) {
		VideoFrame *frame = check_VideoFrame(L);
		frame->deleteData();
		delete frame;
		return 0;
	}

	int get_frame(lua_State* L)
	{
		// get frame number from stack
		TabPanel* tab = Notebook::GetTab();
		int frameNumber = lua_tointeger(L, 1);

		bool withSubtitles = false;
		if (lua_gettop(L) >= 2) {
			withSubtitles = lua_toboolean(L, 2);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		static const struct luaL_Reg FrameTableDefinition[] = {
			{"width", FrameWidth},
			{"height", FrameHeight},
			{"getPixel", FramePixel},
			{"getPixelFormatted", FramePixelFormatted},
			{"__gc", FrameDestory},
			{NULL, NULL}
		};

		// create and register metatable if not already done
		if (luaL_newmetatable(L, "VideoFrame")) {
			// metatable.__index = metatable
			lua_pushstring(L, "__index");
			lua_pushvalue(L, -2);
			lua_settable(L, -3);

			luaL_register(L, NULL, FrameTableDefinition);
		}

		if (tab && tab->video->HasFFMS2()) {

			RendererVideo* renderer = tab->video->GetRenderer();
			byte* frameBuff = renderer->GetFrame(frameNumber, withSubtitles);

			VideoFrame *frame = new VideoFrame();
			frame->data = frameBuff;
			tab->video->GetVideoSize(&frame->width, &frame->height);
			frame->pitch = frame->width * 4;
			frame->flipped = false;

			*static_cast<VideoFrame**>(lua_newuserdata(L, sizeof(VideoFrame*))) = frame;


			luaL_getmetatable(L, "VideoFrame");
			lua_setmetatable(L, -2);
		}
		else {
			lua_pushnil(L);
		}
		return 1;
	}


	/*int lua_get_audio_selection(lua_State* L)
	{
		
		const agi::Context* c = get_context(L);
		if (!c || !c->audioController || !c->audioController->GetTimingController()) {
			lua_pushnil(L);
			return 1;
		}
		const TimeRange range = c->audioController->GetTimingController()->GetActiveLineRange();
		push_value(L, range.begin());
		push_value(L, range.end());
		return 2;
	}*/

	int lua_set_status_text(lua_State* L)
	{
		KainoteFrame* frame = (KainoteFrame*)Notebook::GetTabs()->GetParent();
		if (!frame) {
			lua_pushnil(L);
			return 1;
		}
		wxString text = check_string(L, 1);
		lua_pop(L, 1);
		frame->SetStatusText(text, 0);
		return 0;
	}

	int lua_get_text_cursor(lua_State* L)
	{
		TabPanel* tab = Notebook::GetTab();
		long s = 0, e = 0;
		tab->edit->GetEditor()->GetSelection(&s, &e);
		push_value(L, s + 1);
		return 1;
	}

	int lua_set_text_cursor(lua_State* L)
	{
		int point = lua_tointeger(L, -1) - 1;
		lua_pop(L, 1);
		TabPanel* tab = Notebook::GetTab();
		tab->edit->GetEditor()->SetSelection(point, point);
		return 0;
	}

	int lua_get_text_selection(lua_State* L)
	{
		TabPanel* tab = Notebook::GetTab();
		long start = 0, end = 0;
		tab->edit->GetEditor()->GetSelection(&start, &end);
		start++; end++;
		push_value(L, start <= end ? start : end);
		push_value(L, start <= end ? end : start);
		return 2;
	}

	int lua_set_text_selection(lua_State* L)
	{
		int start = lua_tointeger(L, -2) - 1;
		int end = lua_tointeger(L, -1) - 1;
		lua_pop(L, 2);
		TabPanel* tab = Notebook::GetTab();
		tab->edit->GetEditor()->SetSelection(start, end);
		return 0;
	}

	int lua_is_modified(lua_State* L)
	{
		TabPanel* tab = Notebook::GetTab();
		push_value(L, tab->edit->GetEditor()->IsModified());
		return 1;
	}


	int project_properties(lua_State *L)
	{
		const TabPanel *c = Notebook::GetTab();
		if (!c)
			lua_pushnil(L);
		else {
			lua_createtable(L, 0, 14);
#define PUSH_FIELD(name, fieldname) set_field(L, #name, c->grid->GetSInfo(#name))
			PUSH_FIELD(automation_scripts, "Automation Scripts");
			PUSH_FIELD(export_filters, "");
			PUSH_FIELD(export_encoding, "");
			PUSH_FIELD(style_storage, "Last Style Storage");
			set_field(L, "video_zoom", 1);
			PUSH_FIELD(ar_value, "");
			PUSH_FIELD(scroll_position, "Active Line");
			PUSH_FIELD(active_row, "Active Line");
			PUSH_FIELD(ar_mode, "");
			set_field(L, "video_position", (c->video->HasFFMS2()) ? 
				c->video->GetFFMS2()->GetFramefromMS(c->video->Tell()) : NULL);
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
		, L(NULL)
	{
		include_path.push_back(filename.BeforeLast(L'\\') + L"\\");
		include_path.push_back(Options.pathfull + L"\\Automation\\automation\\Include\\");
		//include_path[0].Replace("\\","/");
		//include_path[1].Replace("\\","/");
		Create();
	}

	void LuaScript::Create()
	{
		Destroy();

		name = GetPrettyFilename();

		// create lua environment
		L = luaL_newstate();
		if (!L) {
			description = L"Could not initialize Lua state";
			return;
		}

		//bool loaded = false;
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
			//lua_gc(L, LUA_GCCOLLECT, 0);
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
		lua_setfield(L, LUA_REGISTRYINDEX, "aegisub");
		stackcheck.check_stack(0);

		// make "aegisub" table
		lua_pushstring(L, "aegisub");
		lua_createtable(L, 0, 13);

		set_field<LuaCommand::LuaRegister>(L, "register_macro");
		set_field<DummyFilter>(L, "register_filter");//kainote nie ma exportu więc by skrypty się nie sypały trzeba podesłać coś fałszywego.
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
		set_field<lua_set_status_text>(L, "set_status_text");
		set_field<get_frame>(L, "get_frame");
		lua_createtable(L, 0, 5);
		set_field<lua_get_text_cursor>(L, "get_cursor");
		set_field<lua_set_text_cursor>(L, "set_cursor");
		set_field<lua_get_text_selection>(L, "get_selection");
		set_field<lua_set_text_selection>(L, "set_selection");
		set_field<lua_is_modified>(L, "is_modified");
		lua_setfield(L, -2, "gui");

		// store aegisub table to globals
		lua_settable(L, LUA_GLOBALSINDEX);
		stackcheck.check_stack(0);

		// load user script
		if (!LoadFile(L, GetFilename())) {
			description = get_string_or_default(L, 1);
			lua_pop(L, 1);
			//lua_gc(L, LUA_GCCOLLECT, 0);
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
			description = wxString::Format(_("Błąd inicjalizacji skryptu Lua \"%s\":\n\n%s."), GetPrettyFilename(), get_string_or_default(L, -1));
			//lua_pop(L, 1);
			lua_pop(L, 2); // error + error handler
			//lua_gc(L, LUA_GCCOLLECT, 0);
			return;
		}
		lua_pop(L, 1); // error handler
		stackcheck.check_stack(0);

		lua_getglobal(L, "version");
		if (lua_isnumber(L, -1) && lua_tointeger(L, -1) == 3) {
			lua_pop(L, 1); // just to avoid tripping the stackcheck in debug
			description = _("Próbujesz wczytać skrypt Automatyzacji 3 jako skrypt Automatyzacji 4. Automatyzacja 3 nie jest już wspierana.");
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

		//loaded = true;
		//lua_gc(L, LUA_GCCOLLECT, 0);
	}

	void LuaScript::Reload() { Create(); }

	void LuaScript::Destroy()
	{
		// Assume the script object is clean if there's no Lua state
		if (!L) return;

		// loops backwards because commands remove themselves from macros when
		// they're unregistered
		for (auto i = macros.begin(); i != macros.end(); i++)
			delete (*i);
		macros.clear();

		lua_close(L);
		L = NULL;
	}


	void LuaScript::RegisterCommand(LuaCommand *command)
	{
		for (auto macro : macros) {
			if (macro->StrDisplay() == command->StrDisplay()) {
				error(L, wxString::Format(_("Makro o nazwie '%s' jest już zdefiniowane w skrypcie '%s'"),
					command->StrDisplay().utf8_str().data(), name.utf8_str().data()).mb_str(wxConvUTF8).data());
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
		lua_getfield(L, LUA_REGISTRYINDEX, "aegisub");
		void *ptr = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return (LuaScript*)ptr;
	}


	int LuaScript::LuaInclude(lua_State *L)
	{
		const LuaScript *s = GetScriptObject(L);
		//no i trzeba wskaźniki stringów, z powodu braku zwalniania owych stringów przy błędzie skryptu.
		const wxString *filename(new wxString(check_string(L, 1)));
		wxString *filepath = new wxString();
		bool fullpath = false;
		// Relative or absolute path
		if (filename->find("\\") != -1 || filename->find("/") != -1){
			*filepath = *filename;//s->GetFilename().BeforeLast('//') + filename;
			fullpath = true;
		}
		if (!wxFileExists(*filepath)) { // Plain filename
			if (fullpath){ *filepath = filepath->AfterLast(L'\\'); }
			for (auto const& dir : s->include_path) {
				*filepath = dir + *filename;
				if (wxFileExists(*filepath))
					break;
			}
		}

		if (!wxFileExists(*filepath)){

			lua_pushfstring(L, "Lua include not found: %s", filepath->mb_str(wxConvUTF8).data());
			delete filename; delete filepath;
			//int res = error(L, "Lua include not found");// error(L, "Lua include not found: %s", filepath->mb_str(wxConvUTF8).data());
			return lua_error(L);
		}
		if (!LoadFile(L, *filepath)){
			lua_pushfstring(L, "Error loading Lua include \"%s\":\n%s", filepath->mb_str(wxConvUTF8).data(), check_string(L, 1).mb_str(wxConvUTF8).data());
			delete filename; delete filepath;
			return lua_error(L);//error(L, "Error loading Lua include \"%s\":\n%s", filepath->mb_str(wxConvUTF8).data(), check_string(L, 1).mb_str(wxConvUTF8).data());
		}
		delete filename; delete filepath;
		int pretop = lua_gettop(L) - 1; // don't count the function value itself
		lua_call(L, 0, LUA_MULTRET);
		return lua_gettop(L) - pretop;
	}

	bool LuaScript::CheckLastModified(bool check)
	{
		FILETIME ft;

		HANDLE ffile = CreateFile(GetFilename().wc_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		GetFileTime(ffile, 0, 0, &ft);
		CloseHandle(ffile);
		if (check){
			if (LowTime != (int)ft.dwLowDateTime || HighTime != ft.dwHighDateTime){
				LowTime = ft.dwLowDateTime;
				HighTime = ft.dwHighDateTime;
				return true;
			}
		}

		LowTime = ft.dwLowDateTime;
		HighTime = ft.dwHighDateTime;

		return false;
	}

	LuaThreadedCall::LuaThreadedCall(lua_State *_L)
		: wxThread(wxTHREAD_JOINABLE)
		, L(_L)
	{
		Create();

		SetPriority(50);

		Run();
	}

	wxThread::ExitCode LuaThreadedCall::Entry()
	{

		bool failed = false;
		bool hasMessage = false;
		int nargs = 3, nresults = 2;
		LuaProgressSink *ps = LuaProgressSink::ps;

		// Insert our error handler under the function to call
		lua_pushcclosure(L, add_stack_trace, 0);
		lua_insert(L, -nargs - 2);

		if (lua_pcall(L, nargs, nresults, -nargs - 2)) {
			if (!lua_isnil(L, -1) && ps) {
				// if the call failed, log the error here
				wxString errmsg(get_string_or_default(L, -1));
				errmsg.Prepend(_("Wystąpił błąd podczas wykonywania skryptu Lua:\n"));
				ps->SafeQueue(Auto::EVT_MESSAGE, errmsg);
				hasMessage = true;
			}
			lua_pop(L, 2);
			failed = true;
		}
		else
			lua_remove(L, -nresults - 1);


		lua_gc(L, LUA_GCCOLLECT, 0);
		if (ps->Log == L"" && !hasMessage){ ps->lpd->closedialog = true; }
		else{ ps->lpd->finished = true; }

		if (failed){ return (wxThread::ExitCode) 1; }
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

		help = get_string(L, 2);

		lua_getfield(L, LUA_REGISTRYINDEX, "filename");
		//cmd_name = wxString::Format("automation/lua/%s/%s", check_string(L, -1), check_string(L, 1));
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
		auto const& sels = c->grid->file->GetSelectionsAsKeys();
		int offset = c->grid->SInfoSize() + c->grid->StylesSize() + 1;
		std::vector<int> rows;
		rows.reserve(sels.size());
		for (auto line : sels)
			rows.push_back(line + offset);
		return rows;
	}

	bool LuaCommand::Validate(const TabPanel *c)
	{
		if (!(cmd_type & COMMAND_VALIDATE)) return true;

		// Error handler goes under the function to call
		lua_pushcclosure(L, add_stack_trace, 0);

		GetFeatureFunction("validate");
		auto subsobj = new AutoToFile(L, c->grid->file->GetSubs(), true, c->grid->subsFormat);

		push_value(L, selected_rows(c));
		push_value(L, c->grid->currentLine + c->grid->SInfoSize() + c->grid->StylesSize() + 1);

		int err = lua_pcall(L, 3, 2, -5 /* three args, function, error handler */);
		SAFE_DELETE(subsobj);

		if (err) {
			KaiLog(wxString::Format("Runtime error in Lua macro validation function:\n%s", get_string(L, -1)));
			lua_pop(L, 2);
			return false;
		}

		bool result = !!lua_toboolean(L, -2);

		wxString new_help_string(get_string(L, -1));
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
		File *subs = c->grid->file->GetSubs();
		auto subsobj = new AutoToFile(L, subs, true, c->grid->subsFormat);

		int original_offset = c->grid->SInfoSize() + c->grid->StylesSize() + 1;
		auto original_sel = selected_rows(c);
		// original active do not have offset
		int original_active = c->grid->currentLine;

		push_value(L, original_sel);
		push_value(L, original_active + original_offset);

		c->grid->SaveSelections();

		LuaProgressSink *ps = new LuaProgressSink(L, c);


		LuaThreadedCall call(L);

		ps->ShowDialog(StrDisplay());
		wxThread::ExitCode code = call.Wait();
		bool failed = (int)code == 1;

		if (ps->lpd->cancelled || failed){
			SAFE_DELETE(subsobj);
			c->grid->file->DummyUndo();

			delete ps;
			return;
		}

		//c->grid->file->ReloadVisibleDialogues();
		//if(ps->lpd->cancelled && ps->lpd->IsModal()){ps->lpd->EndModal(0);}
		//c->grid->SaveSelections(true);
		original_offset = c->grid->SInfoSize() + c->grid->StylesSize() + 1;
		int active_idx = -1;

		size_t dialsCount = c->grid->file->GetCount();

		// Check for a new active row
		if (lua_isnumber(L, -1)) {
			active_idx = lua_tointeger(L, -1) - original_offset;
			if (active_idx < 0 || active_idx >= dialsCount) {
				KaiLog(wxString::Format("Active row %d is out of bounds (must be 1-%u)", active_idx, dialsCount));
				active_idx = original_active;
			}
			else
				c->grid->file->ClearSelections();
		}

		//stackcheck.check_stack(2);
		lua_pop(L, 1);

		// top of stack will be selected lines array, if any was returned
		if (lua_istable(L, -1)) {
			c->grid->file->ClearSelections();
			lua_for_each(L, [&] {
				if (!lua_isnumber(L, -1))
					return;
				int cur = lua_tointeger(L, -1) - original_offset;
				if (cur < 0 || cur >= dialsCount) {
					KaiLog(wxString::Format("Selected row %d is out of bounds (must be 1-%u)", cur, dialsCount));
					throw LuaForEachBreak();
				}
				if (active_idx == -1)
					active_idx = cur;
				c->grid->file->InsertSelection(cur);
			});

		}
		else {
			lua_pop(L, 1);

		}
		if (active_idx == -1)
			active_idx = original_active;
		c->grid->SpellErrors.clear();
		//refresh styles in style manager
		if (StyleStore::HasStore() && StyleStore::Get()->IsShown())
			StyleStore::ShowStore();
		//refresh styles in editbox
		c->edit->RefreshStyle();

		c->grid->SetModified(AUTOMATION_SCRIPT, true, false, active_idx);
		c->grid->RefreshColumns();
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
		auto subsobj = new AutoToFile(L, c->grid->file->GetSubs(), true, c->grid->subsFormat);
		push_value(L, selected_rows(c));
		push_value(L, c->grid->currentLine + c->grid->SInfoSize() + c->grid->StylesSize() + 1);

		int err = lua_pcall(L, 3, 1, 0);

		bool result = false;
		if (err)
			KaiLog(wxString::Format("Runtime error in Lua macro IsActive function:\n%s", get_string(L, -1)));
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
		//LuaScript *script = LuaScript::GetScriptObject(L);


		TabPanel *pan = Notebook::GetTab();
		if (pan && Validate(pan)){ Run(pan); }



	}

	VOID CALLBACK callbackfunc(PVOID   lpParameter, BOOLEAN TimerOrWaitFired) {
		Automation *auto_ = (Automation*)lpParameter;
		auto_->ReloadScripts(true);
		DeleteTimerQueueTimer(auto_->handle, 0, 0);
		SetEvent(auto_->eventEndAutoload);
	}

	Automation::Automation(bool loadSubsScripts, bool loadNow)
	{
		initialized = false;
		AutoloadPath = Options.pathfull + L"\\Automation\\automation\\Autoload";
		if (loadSubsScripts){ return; }
		int loadMethod = Options.GetInt(AUTOMATION_LOADING_METHOD);
		if (loadMethod < 2){
			initialized = true;
			if (loadMethod == 0 && !loadNow){
				eventEndAutoload = CreateEvent(NULL, FALSE, FALSE, NULL);
				CreateTimerQueueTimer(&handle, NULL, callbackfunc, this, 20, 0, 0);
			}
			else{
				ReloadScripts(true);
			}
		}
	}

	Automation::~Automation()
	{
		breakLoading = true;
		if (eventEndAutoload){
			WaitForSingleObject(eventEndAutoload, 50000);
		}
		RemoveAll(true);
	}

	bool Automation::Add(wxString filename, bool addToSinfo, bool autoload)
	{
		std::vector<Auto::LuaScript*> &scripts = (autoload) ? Scripts : ASSScripts;
		Auto::LuaScript *ls = new Auto::LuaScript(filename);
		for (size_t i = 0; i < scripts.size(); i++) {
			if (ls->GetFilename() == scripts[i]->GetFilename()){ delete ls; ls = NULL; return false; }
		}
		ls->CheckLastModified(false);
		scripts.push_back(ls);
		if (!autoload && addToSinfo){
			SubsGrid *grid = Notebook::GetTab()->grid;
			wxString scriptpaths = grid->GetSInfo(L"Automation Scripts");
			scriptpaths << L"|" << filename;
			grid->AddSInfo(L"Automation Scripts", scriptpaths);
			grid->SetModified(ASS_PROPERTIES, false, true, -1, false);
		}
		HasChanges = true;
		return true;
	}

	void Automation::Remove(int script)
	{
		HasChanges = true;
		std::vector<Auto::LuaScript*>::iterator i = ASSScripts.begin() + script;
		delete *i;
		ASSScripts.erase(i);
	}

	void Automation::RemoveAll(bool autoload)
	{

		if (autoload){
			for (auto i = Scripts.begin(); i != Scripts.end(); i++) {
				delete *i;
			}
			Scripts.clear();
		}
		
		for (auto i = ASSScripts.begin(); i != ASSScripts.end(); i++) {
			delete *i;
		}
		ASSScripts.clear();
		
	}

	void Automation::ReloadMacro(int script, bool autoload)
	{
		if (autoload){ Scripts[script]->Reload(); }
		else{ ASSScripts[script]->Reload(); }
	}


	void Automation::ReloadScripts(bool first)
	{
		initialized = true;
		
		if (!first){ RemoveAll(true); }
		int error_count = 0;

		
		wxDir dir;
		
		if (!dir.Open(AutoloadPath)) {
			//wxLogWarning("Failed to open a directory in the Automation autoload path: %s", dirname.c_str());
			return;
		}


		wxString fn;
		wxFileName script_path(AutoloadPath, L"");
		bool more = dir.GetFirst(&fn, wxEmptyString, wxDIR_FILES);

		while (more) {
			if (breakLoading){
				break;
			}
			script_path.SetName(fn);
			try {
				wxString fullpath = script_path.GetFullPath();
				wxString ext = fullpath.AfterLast(L'.').Lower();

				if ((ext != L"lua" && ext != L"moon") || 
					!Add(fullpath, false, true)){ 
					more = dir.GetNext(&fn); 
					continue; 
				}

				if (!Scripts[Scripts.size() - 1]->GetLoadedState()) { 
					error_count++; 
				}


			}
			catch (const wchar_t *e) {
				error_count++;
				KaiLog(wxString::Format(_("Błąd wczytywania skryptu Lua: %s\n%s"), fn.c_str(), e));
			}
			catch (...) {
				error_count++;
				KaiLog(wxString::Format(_("Nieznany błąd wczytywania skryptu Lua: %s."), fn.c_str()));
			}

			more = dir.GetNext(&fn);
		}

		if (error_count > 0) {
			KaiLog(_("Jeden bądź więcej skryptów autoload zawiera błędy.\nObejrzyj opisy skryptów, by uzyskać więcej informacji."));
		}


		//STime countTime(sw.Time());
		//KaiLog("Upłynęło %sms",countTime.GetFormatted(SRT));
	}

	bool Automation::AddFromSubs()
	{
		wxString paths = Notebook::GetTab()->grid->GetSInfo(L"Automation Scripts");

		if (paths == L""){ return false; }
		if (paths == scriptpaths && ASSScripts.size() > 0){ return false; }
		paths.Trim(false);
		wxStringTokenizer token(paths, L"|~$", wxTOKEN_RET_EMPTY_ALL);
		int error_count = 0;
		while (token.HasMoreTokens())
		{
			wxString onepath = token.GetNextToken();
			onepath.Trim(false);
			if (!wxFileExists(onepath)){ continue; }

			try {
				if (!Add(onepath, false)){ continue; }
				int last = Scripts.size() - 1;

			}
			catch (const wchar_t *e) {
				error_count++;
				KaiLog(wxString::Format(_("Błąd wczytywania skryptu Lua: %s\n%s"), onepath.c_str(), e));
			}
			catch (...) {
				error_count++;
				KaiLog(wxString::Format(_("Nieznany błąd wczytywania skryptu Lua: %s."), onepath.c_str()));
			}
		}
		if (error_count > 0) {
			KaiLog(_("Co najmniej jeden skrypt z pliku napisów zawiera błędy.\nZobacz opisy skryptów, by uzyskać więcej informacji."));
		}
		scriptpaths = paths;
		return true;
	}

	void Automation::OnEdit(const wxString &Filename)
	{
		wxString editor = Options.GetString(AUTOMATION_SCRIPT_EDITOR);
		if (editor == L"" || wxGetKeyState(WXK_SHIFT)){
			editor = wxFileSelector(_("Wybierz edytor skryptów"), L"",
				L"C:\\Windows\\Notepad.exe", L"exe", _("Programy (*.exe)|*.exe|Wszystkie pliki (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (!wxFileExists(editor)){ return; }
			Options.SetString(AUTOMATION_SCRIPT_EDITOR, editor);
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
			KaiMessageBox(_("Nie można uruchomić edytora."), _("Błąd automatyzacji"), wxOK | wxICON_ERROR);
		}
	}

	bool Automation::CheckChanges()
	{
		for (auto script : Scripts){
			if (script->CheckLastModified()){ HasChanges = true; break; }
		}
		return HasChanges;
	}


	void Automation::BuildMenu(Menu **bar, bool all)
	{
		TabPanel* c = Notebook::GetTab();
		if (!c)
			return;

		KainoteFrame *Kai = (KainoteFrame*)c->GetGrandParent();
		for (int j = (*bar)->GetMenuItemCount() - 1; j >= 4; j--){
			(*bar)->Delete(j);
		}
		if (!initialized){
			int loadMethod = Options.GetInt(AUTOMATION_LOADING_METHOD);
			if (loadMethod % 2 == 0){
				eventEndAutoload = CreateEvent(NULL, FALSE, FALSE, NULL);
				CreateTimerQueueTimer(&handle, NULL, callbackfunc, this, 5, 0, 0);
			}
			else{
				ReloadScripts(true);
			}
		}
		bool changes = AddFromSubs();

		int start = 34000, i = 0;
		//if(all){
		for (size_t g = 0; g < Scripts.size(); g++){
			auto script = Scripts[g];
			if (script->CheckLastModified(true)){ script->Reload(); }
			Menu *submenu = new Menu();
			auto macros = script->GetMacros();
			for (size_t p = 0; p < macros.size(); p++){
				auto macro = macros[p];
				wxString text;
				text << L"Script " << script->GetFilename() << L"-" << p;
				MenuItem *mi = submenu->SetAccMenu(new MenuItem(start, macro->StrDisplay(), macro->StrHelp()), text);
				mi->Enable(macro->Validate(c));
				Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
					if (evt.GetInt() == wxMOD_SHIFT){
						Hkeys.OnMapHkey(-1, text, Kai, GLOBAL_HOTKEY, false);
					}
					else{
						macro->RunScript();
					}
				}, start);
				start++;
			}
			
			if (macros.size() < 1 || script->GetName().EndsWith(L".lua") ||
				script->GetName().EndsWith(L".moon")){
				wxString strippedbug = script->GetDescription();
				strippedbug.Replace(L"\n", L"");
				if (strippedbug.Len() > 100){ strippedbug = strippedbug.SubString(0, 100) + L"..."; }
				submenu->Append(start, strippedbug, _("Błąd"));
				Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
					KaiMessageBox(script->GetDescription(), _("Pełny opis błędu Lua"));
				}, start);
				start++;
			}
			submenu->AppendSeparator();
			submenu->Append(start, _("Edytuj"), _("Edytuj"));
			Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
				Automation::OnEdit(script->GetFilename());
			}, start);
			start++;
			submenu->Append(start, _("Odśwież"), _("Odśwież"));
			Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
				script->Reload();
			}, start);
			start++;
			(*bar)->Append(-1, script->GetName(), submenu, script->GetDescription());
			//if(i%20==19){(*bar)->Break();}
			i++;
		}
		//}

		for (size_t g = 0; g < ASSScripts.size(); g++){
			auto script = ASSScripts[g];
			if (script->CheckLastModified(true)){ script->Reload(); }
			Menu *submenu = new Menu();
			auto macros = script->GetMacros();
			for (size_t p = 0; p < macros.size(); p++){
				auto macro = macros[p];
				wxString text; text << L"Script " << script->GetFilename() << L"-" << p;
				MenuItem *mi = submenu->SetAccMenu(new MenuItem(start, macro->StrDisplay(), macro->StrHelp()), text);
				mi->Enable(macro->Validate(c));
				Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
					if (evt.GetInt() == wxMOD_SHIFT){
						Hkeys.OnMapHkey(-1, text, Kai, GLOBAL_HOTKEY, false);
					}
					else{
						macro->RunScript();
					}
				}, start);
				start++;
			}
			if (macros.size() < 1){
				wxString strippedbug = script->GetDescription();
				strippedbug.Replace(L"\n", L"");
				if (strippedbug.Len() > 100){ strippedbug = strippedbug.SubString(0, 100) + L"..."; }
				submenu->Append(start, strippedbug, _("Błąd"));
				Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
					KaiMessageBox(script->GetDescription(), _("Pełny opis błędu Lua"));
				}, start);
				start++;
			}
			submenu->AppendSeparator();
			submenu->Append(start, _("Edytuj"), _("Edytuj"));
			Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
				Automation::OnEdit(script->GetFilename());
			}, start);
			start++;
			submenu->Append(start, _("Odśwież"), _("Odśwież"));
			Kai->Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
				script->Reload();
			}, start);
			start++;
			(*bar)->Append(-1, script->GetName(), submenu, script->GetDescription());
			i++;
		}


		HasChanges = false;
	}

	void Automation::ShowScriptHotkeysWindow(wxWindow *parent)
	{
		AutomationHotkeysDialog AHD(parent, this);
		AHD.ShowModal();
	}

	LuaScript *Automation::FindScript(const wxString &path)
	{
		for (size_t g = 0; g < Scripts.size(); g++){
			if (Scripts[g]->GetFilename() == path){ return Scripts[g]; }
		}
		for (size_t g = 0; g < ASSScripts.size(); g++){
			if (ASSScripts[g]->GetFilename() == path){ return ASSScripts[g]; }
		}
		return NULL;
	}


}