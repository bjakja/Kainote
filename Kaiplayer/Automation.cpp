
#include "Automation.h"
#include "Config.h"
#include "OpennWrite.h"
#include "KainoteMain.h"
#include "KainoteApp.h"
#include "AutomationToFile.h"
#include "AutomationProgress.h"

#include <wx/filename.h>
#include <wx/dir.h>

#include <windows.h>

namespace Auto{

struct LuaScriptReader{
public:
	LuaScriptReader(const wxString &filename);
	~LuaScriptReader();
	static const char *ReadScript(lua_State *L, void *data, size_t *size);
private:
	FILE *f;
		bool first;
		char *databuf;
		static const size_t bufsize = 512;
	//OpenWrite ow;
	//wxString Scriptdata;
	};

LuaScriptReader::LuaScriptReader(const wxString &filename)
	{
	//first=true;
	//Scriptdata=ow.FileOpen(filename,true);
	f = _tfopen(filename.c_str(), L"rb");
		if (!f)
			throw _("Nie można otworzyć skryptu");
		first = true;
		databuf = new char[bufsize];
	}

LuaScriptReader::~LuaScriptReader()
	{
	//wxLogStatus("destroy ");
	if (databuf){
		delete[] databuf;
		fclose(f);}
	}

const char *LuaScriptReader::ReadScript(lua_State *L, void *data, size_t *size)
	{
	LuaScriptReader *self = (LuaScriptReader*)(data);
	//if(self->Scriptdata.IsEmpty()){*size=0;return 0;}
	//const char * buf= self->Scriptdata.utf8_str().data();
	//*size=strlen(buf);
	//return buf;
	unsigned char *b = (unsigned char *)self->databuf;
		FILE *f = self->f;

		if (feof(f)) {
			*size = 0;
			return 0;
		}
		if (self->first) {
			// check if file is sensible and maybe skip bom
			if ((*size = fread(b, 1, 4, f)) == 4) {
				if (b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) {
					// got an utf8 file with bom
					// nothing further to do, already skipped the bom
					fseek(f, -1, SEEK_CUR);
				} else {
					// oops, not utf8 with bom
					// check if there is some other BOM in place and complain if there is...
					if ((b[0] == 0xFF && b[1] == 0xFE && b[2] == 0x00 && b[3] == 0x00) || // utf32be
						(b[0] == 0x00 && b[1] == 0x00 && b[2] == 0xFE && b[3] == 0xFF) || // utf32le
						(b[0] == 0xFF && b[1] == 0xFE) || // utf16be
						(b[0] == 0xFE && b[1] == 0xFF) || // utf16le
						(b[0] == 0x2B && b[1] == 0x2F && b[2] == 0x76) || // utf7
						(b[0] == 0x00 && b[2] == 0x00) || // looks like utf16be
						(b[1] == 0x00 && b[3] == 0x00)) { // looks like utf16le
							throw "The script file uses an unsupported character set. Only UTF-8 is supported.";
					}
					// assume utf8 without bom, and rewind file
					fseek(f, 0, SEEK_SET);
				}
			} else {
				// hmm, rather short file this...
				// doesn't have a bom, assume it's just ascii/utf8 without bom
				return self->databuf; // *size is already set
			}
			self->first = false;
		}

		*size = fread(b, 1, bufsize, f);

		return self->databuf;

	}

struct LuaStackcheck {
		lua_State *L;
		int startstack;
		void check_stack(int additional)
		{
			int top = lua_gettop(L);
			if (top - additional != startstack) {
				dump();
				//assert(top - additional == startstack);
			}
		}
		void dump()
		{
			int top = lua_gettop(L);
			
			for (int i = top; i > 0; i--) {
				lua_pushvalue(L, i);
				lua_pop(L, 1);
			}
		}
		LuaStackcheck(lua_State *_L) : L(_L) { startstack = lua_gettop(L); }
		~LuaStackcheck() { check_stack(0); }
	};


LuaScript::LuaScript(const wxString &_filename)
		: L(0)
	{
		filename=_filename;
		Create();
	}

	LuaScript::~LuaScript()
	{
		if (L) Destroy();
	}

	void LuaScript::Create()
	{
	//wxLogStatus("Destroy");
		Destroy();

		loaded = true;

		try {
			// create lua environment
			
			L = luaL_newstate();
			LuaStackcheck _stackcheck(L);

			// register standard libs
			lua_pushcfunction(L, luaopen_base); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_package); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_string); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_table); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_math); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_io); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_os); lua_call(L, 0, 0);
			//lua_pushcfunction(L, luaopen_bit32); lua_call(L, 0, 0);
			_stackcheck.check_stack(0);
			//wxLogStatus("libs");
			// dofile and loadfile are replaced with include
			lua_pushnil(L);
			lua_setglobal(L, "dofile");
			//wxLogStatus("dofile");
			lua_pushnil(L);
			lua_setglobal(L, "loadfile");
			//wxLogStatus("loadfile");
			lua_pushcfunction(L, LuaInclude);
			lua_setglobal(L, "include");
			//wxLogStatus("include");
			// add include_path to the module load path
			lua_getglobal(L, "package");
			lua_pushstring(L, "path");
			lua_pushstring(L, "path");
			//wxLogStatus("set package");
			lua_gettable(L, -3);
			//wxLogStatus("globals");
			
				wxFileName path(Options.pathfull+"\\Include");
				//wxFileName path1(filename.BeforeLast('\\'));
				if (path.IsOk() && !path.IsRelative() && path.DirExists()) {
					include_path.Add(path.GetLongPath());
					include_path.Add(filename.BeforeLast('\\'));
					wxCharBuffer p = path.GetLongPath().utf8_str();
					//wxLogStatus("push string");
					lua_pushfstring(L, ";%s?.lua;%s?/init.lua", p, p);
					//wxLogStatus("concat");
					lua_concat(L, 2);
				}
			
			//wxLogStatus("settable");
			lua_settable(L, -3);
			//wxLogStatus("get field");
			
			// Replace the default lua module loader with our utf-8 compatible one
			lua_getfield(L, -1, "loaders");
			//wxLogStatus("loaders");
			lua_pushcfunction(L, LuaModuleLoader);
			//wxLogStatus("function");
			lua_rawseti(L, -2, 2);
			//wxLogStatus("rawseti");
			lua_pop(L, 2);
			//wxLogStatus("pop");
			_stackcheck.check_stack(0);
			//wxLogStatus("paths");
			// prepare stuff in the registry
			// reference to the script object
			lua_pushlightuserdata(L, this);
			lua_setfield(L, LUA_REGISTRYINDEX, "kainote");
			//wxLogStatus("light userdata");
			// the "feature" table
			// integer indexed, using same indexes as "macros" vector in the base Script class
			lua_newtable(L);
			lua_setfield(L, LUA_REGISTRYINDEX, "macros");
			_stackcheck.check_stack(0);
			//wxLogStatus("macros");

			// make "kainote" table
			//lua_pushglobaltable(L);
			lua_pushstring(L, "kainote");
			lua_newtable(L);
			// kainote.register_macro
			lua_pushcfunction(L, LuaMacro::LuaRegister);
			lua_setfield(L, -2, "register_macro");
			// kainote.text_extents
			lua_pushcfunction(L, LuaTextExtents);
			lua_setfield(L, -2, "text_extents");
			// VFR handling
			lua_pushcfunction(L, LuaFrameFromMs);
			lua_setfield(L, -2, "frame_from_ms");
			lua_pushcfunction(L, LuaMsFromFrame);
			lua_setfield(L, -2, "ms_from_frame");
			lua_pushcfunction(L, LuaVideoSize);
			lua_setfield(L, -2, "video_size");
			//wxLogStatus("kainote global");
			// store kainote table to globals
			lua_settable(L, LUA_GLOBALSINDEX);
			
			_stackcheck.check_stack(0);
			//wxLogStatus("Read Script");
			// load user script
			//wxLogStatus("Read");
			LuaScriptReader script_reader(filename);
			if (lua_load(L, script_reader.ReadScript, &script_reader, name.mb_str(wxConvUTF8).data())) {
				wxString err(lua_tostring(L, -1), wxConvUTF8);
				err.Prepend(_("Błąd wczytywania skryptu Lua \"") + filename + "\":\n\n");
				throw err;
			}
			//wxLogStatus("Readed");
			_stackcheck.check_stack(1);
			//wxLogStatus("Execute");
			// and execute it
			// this is where features are registered
			// don't thread this, as there's no point in it and it seems to break on wx 2.8.3, for some reason
			if (lua_pcall(L, 0, 0, 0)) {
				// error occurred, assumed to be on top of Lua stack
				wxString err(lua_tostring(L, -1), wxConvUTF8);
				err.Prepend(_("Błąd inicjalizacji skryptu Lua \"") + filename + "\":\n\n");
				throw err;
			}
			_stackcheck.check_stack(0);
			//wxLogStatus("get script props");
			lua_getglobal(L, "script_name");
			if (lua_isstring(L, -1)) {
				name = wxString(lua_tostring(L, -1), wxConvUTF8);
			} else {
				name = filename;
			}
			lua_getglobal(L, "script_description");
			if (lua_isstring(L, -1)) {
				description = wxString(lua_tostring(L, -1), wxConvUTF8);
			}
			lua_getglobal(L, "script_author");
			if (lua_isstring(L, -1)) {
				author = wxString(lua_tostring(L, -1), wxConvUTF8);
			}
			lua_getglobal(L, "script_version");
			if (lua_isstring(L, -1)) {
				version = wxString(lua_tostring(L, -1), wxConvUTF8);
			}
			lua_pop(L, 5);
			// if we got this far, the script should be ready
			_stackcheck.check_stack(0);
			//wxLogStatus("modif");
			CheckLastModified(false);
			//wxLogStatus("all");
		}
		catch (const char *e) {
			Destroy();
			loaded = false;
			name = filename;
			description = wxString(e, wxConvUTF8);
		}
		catch (const wchar_t *e) {
			Destroy();
			loaded = false;
			name = filename;
			description = e;
		}
		catch (const wxString& e) {
			Destroy();
			loaded = false;
			name = filename;
			description = e;
		}
		catch (...) {
			Destroy();
			loaded = false;
			name = filename;
			description = _("Nieznany błąd ładowania skryptu Lua");
		}
	}

	void LuaScript::Destroy()
	{
		// Assume the script object is clean if there's no Lua state
		if (!L) return;

		// remove macros
		for (int i = 0; i < (int)Macros.size(); i++) {
			LuaMacro *m = Macros[i];
			delete m;
		}
		Macros.clear();

		// delete environment
		lua_close(L);
		L = 0;

		loaded = false;
	}

	void LuaScript::Reload()
	{
		Create();
	}

	LuaScript* LuaScript::GetScriptObject(lua_State *L)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, "kainote");
		void *ptr = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return (LuaScript*)ptr;
	}

	int LuaScript::LuaTextExtents(lua_State *L)
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

	/// @brief Module loader which uses our include rather than Lua's, for unicode file support
	/// @param L The Lua state
	/// @return Always 1 per loader_Lua?
	int LuaScript::LuaModuleLoader(lua_State *L)
	{
		int pretop = lua_gettop(L);
		wxString module(lua_tostring(L, -1), wxConvUTF8);
		module.Replace(L".", LUA_DIRSEP);

		lua_getglobal(L, "package");
		lua_pushstring(L, "path");
		lua_gettable(L, -2);
		wxString package_paths(lua_tostring(L, -1), wxConvUTF8);
		lua_pop(L, 2);

		wxStringTokenizer toker(package_paths, L";", wxTOKEN_STRTOK);
		while (toker.HasMoreTokens()) {
			wxString filename = toker.GetNextToken();
			filename.Replace(L"?", module);
			if (wxFileName::FileExists(filename)) {
				LuaScriptReader script_reader(filename);
				if (lua_load(L, script_reader.ReadScript, &script_reader, filename.utf8_str().data())) {
					lua_pushfstring(L, "Blad wczytywania modulu Lua \"%s\":\n\n%s", filename.utf8_str().data(), lua_tostring(L, -1));
					lua_error(L);
					return lua_gettop(L) - pretop;
				}
			}
		}
		return lua_gettop(L) - pretop;
	}

	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaScript::LuaInclude(lua_State *L)
	{
		LuaScript *s = GetScriptObject(L);

		if (!lua_isstring(L, 1)) {
			lua_pushstring(L, "Argument dla include musi byc stringiem");
			lua_error(L);
			return 0;
		}
		wxString fnames(lua_tostring(L, 1), wxConvUTF8);

		wxFileName fname(fnames);
		if (fname.GetDirCount() == 0) {
			// filename only
			fname = s->include_path.FindAbsoluteValidPath(fnames);
		} else if (fname.IsRelative()) {
			// relative path
			wxFileName sfname(s->name);
			fname.MakeAbsolute(sfname.GetPath(true));
		} else {
			// absolute path, do nothing
		}
		if (!fname.IsOk() || !fname.FileExists()) {
			lua_pushfstring(L, "Nie znaleziono Lua include: %s", fnames.mb_str(wxConvUTF8).data());
			lua_error(L);
		}

		LuaScriptReader script_reader(fname.GetFullPath());
		if (lua_load(L, script_reader.ReadScript, &script_reader, fname.GetFullName().mb_str(wxConvUTF8).data())) {
			lua_pushfstring(L, "Blad wczytywania Lua include \"%s\":\n\n%s", fname.GetFullPath().mb_str(wxConvUTF8).data(), lua_tostring(L, -1));
			lua_error(L);
			return 0;
		}
		int pretop = lua_gettop(L) - 1; // don't count the function value itself
		lua_call(L, 0, LUA_MULTRET);
		return lua_gettop(L) - pretop;
	}

	int LuaScript::LuaFrameFromMs(lua_State *L)
	{
		int ms = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		kainoteApp *Kaia = (kainoteApp*) wxTheApp;
		VideoCtrl *video=Kaia->Frame->GetTab()->Video;
		if (video->GetState()!=None) {
			int frame=(video->IsDshow)? ((float)ms/1000.0)*video->fps : video->VFF->GetFramefromMS(ms);
			//wxLogStatus("frame %i %i %i", frame , (int)(((float)ms/1000.0)*video->fps), video->VFF->GetFramefromMS(ms));
			lua_pushnumber(L, frame);
			return 1;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}

	int LuaScript::LuaMsFromFrame(lua_State *L)
	{
		int frame = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		kainoteApp *Kaia = (kainoteApp*) wxTheApp;
		VideoCtrl *video=Kaia->Frame->GetTab()->Video;
		if (video->GetState()!=None) {
			int ms=(video->IsDshow)? ((frame*1000)/video->fps) : video->VFF->GetMSfromFrame(frame);
			//wxLogStatus("ms %i %i %i", ms , (int)((frame*1000)/video->fps), video->VFF->GetMSfromFrame(frame));
			lua_pushnumber(L, ms);
			return 1;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}

	int LuaScript::LuaVideoSize(lua_State *L)
	{
		kainoteApp *Kaia = (kainoteApp*) wxTheApp;
		VideoCtrl *video=Kaia->Frame->GetTab()->Video;
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

	bool LuaScript::CheckLastModified(bool check)
	{
		FILETIME ft;

		HANDLE ffile = CreateFile(filename.wc_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

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


	// LuaFeatureMacro
	void LuaMacro::RegisterMacro()
	{
		// get the LuaScript objects
		lua_getfield(L, LUA_REGISTRYINDEX, "kainote");
		LuaScript *s = (LuaScript*)lua_touserdata(L, -1);
		lua_pop(L, 1);

		// add the Feature object
		s->Macros.push_back(this);

		// get the index+1 it was pushed into
		myid = (int)s->Macros.size()-1;

		// create table with the functions
		// get features table
		lua_getfield(L, LUA_REGISTRYINDEX, "macros");
		lua_pushvalue(L, -2);
		lua_rawseti(L, -2, myid);
		lua_pop(L, 1);
	}

	int LuaMacro::LuaRegister(lua_State *L)
	{
		wxString _name(lua_tostring(L, 1), wxConvUTF8);
		wxString _description(lua_tostring(L, 2), wxConvUTF8);

		LuaMacro *macro = new LuaMacro(_name, _description, L);
		(void)macro;

		return 0;
	}

	LuaMacro::LuaMacro(const wxString &_name, const wxString &_description, lua_State *_L)
		:L(_L)
		,name(_name)
	{
		// new table for containing the functions for this feature
		lua_newtable(L);
		// store processing function
		if (!lua_isfunction(L, 3)) {
			lua_pushstring(L, "Funkcja wykonawcza makra musi byc funkcja");
			lua_error(L);
		}
		lua_pushvalue(L, 3);
		lua_rawseti(L, -2, 1);
		// and validation function
		lua_pushvalue(L, 4);
		no_validate = !lua_isfunction(L, -1);
		lua_rawseti(L, -2, 2);
		// make the macro known
		RegisterMacro();
		// and remove the macro function table again
		lua_pop(L, 1);
	}

	bool LuaMacro::Validate( const wxArrayInt &selected, int active, int added)
	{
		if (no_validate)
			return true;

		lua_getfield(L, LUA_REGISTRYINDEX, "macros");
		// get this feature's function pointers
		lua_rawgeti(L, -1, myid);
		// get pointer for validation function
		lua_rawgeti(L, -1, 2);
		lua_remove(L, -2);
		lua_remove(L, -2);

		// prepare function call
		AutoToFile *subsobj = new AutoToFile(L, false);
		//(void) subsobj;
		
		lua_newtable(L);
		for (size_t i = 0; i < selected.size(); i++) {
			// We use zero-based indexing but Lua wants one-based, so add one
			lua_pushinteger(L, selected[i] + added);
			lua_rawseti(L, -2, (int)i+1);
		}

		lua_pushinteger(L, active); // active line

		// do call
		//LuaThreadedCall *call= new LuaThreadedCall(L);
		//wxThread::ExitCode code = call->Wait();
		if(lua_pcall(L, 3, 1, 0)){
			wxString errmsg(lua_tostring(L, -2), wxConvUTF8);
				errmsg<<_("\n\nWystąpił błąd podczas wykonywania skryptu Lua:\n");
			wxMessageBox(errmsg,_("Błąd skryptu lua"));
		}
		
		//(void) code;
		// get result
		bool result = !!lua_toboolean(L, -1);

		// clean up stack
		lua_pop(L, 1);
		//delete call;
		wxDELETE(subsobj);
		return result;
	}

	void LuaMacro::Process(wxArrayInt &selected, int active, int added, wxWindow *progress_parent)
	{

		lua_getfield(L, LUA_REGISTRYINDEX, "macros");
		// get this feature's function pointers
		lua_rawgeti(L, -1, myid);
		// get pointer for validation function
		lua_rawgeti(L, -1, 1);
		lua_remove(L, -2);
		lua_remove(L, -2);
		// prepare function call
		AutoToFile *subsobj = new AutoToFile(L, true);
		//(void) subsobj;
		lua_newtable(L);
		
		for (size_t i = 0; i < selected.size(); i++) {
			// We use zero-based indexing but Lua wants one-based, so add one
			lua_pushinteger(L, (int)(selected[i] + added));
			lua_rawseti(L, -2, (int)(i+1));
		} // selected items
		
		lua_pushinteger(L, active); // active line
		
		LuaProgressSink *ps = new LuaProgressSink(L, progress_parent);
		
		
		
		//wxLogStatus("ps titled");
		// do call
		// 3 args: subtitles, selected lines, active line
		// 1 result: new selected lines
		LuaThreadedCall *call=new LuaThreadedCall(L);
		ps->ShowDialog(name);
		//wxLogStatus("wait");
		wxThread::ExitCode code = call->Wait();
		//wxLogStatus("get code %i", (int)code);
		//(void) code; // ignore
		//if (code) ThrowError();
		if(ps->lpd->cancelled&&ps->lpd->IsModal()){ps->lpd->EndModal(0);}

		// top of stack will be selected lines array, if any was returned
		
		if (lua_istable(L, -1)) {
			selected.clear();
			selected.reserve(lua_objlen(L, -1));
			lua_pushnil(L);
			while (lua_next(L, -2)) {
				if (lua_isnumber(L, -1)) {
					// Lua uses one-based indexing but we want zero-based, so subtract one
					selected.Add(lua_tointeger(L, -1) - added);
				}
				lua_pop(L, 1);
			}
			std::sort(selected.begin(), selected.end());
			lua_pop(L, 1);
		}
		// either way, there will be something on the stack
		//wxLogStatus("result");
		delete ps;
		delete call;
		wxDELETE(subsobj);
		
	}

	// LuaThreadedCall

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
	//wxLogStatus("pcall");
		int result = lua_pcall(L, 3, 1, 0);
		//wxLogStatus("pcall end");
		// see if there's a progress sink window to close
		//lua_getfield(L, LUA_REGISTRYINDEX, "progress_sink");
		LuaProgressSink *ps = LuaProgressSink::ps;
		if (ps) {
			
			//wxLogStatus("progress sink pointer");
			//ps->AddDebugOutput(wxString::Format("result %i \n",result));
			if (result) {
				// if the call failed, log the error here
				wxString errmsg(lua_tostring(L, -1), wxConvUTF8);
				errmsg.Prepend(_("Wystąpił błąd podczas wykonywania skryptu Lua:\n"));
				ps->SafeQueue(Auto::EVT_MESSAGE,errmsg);
				lua_pop(L, 1);
			}
			//wxLogStatus("result");
			// don't bother protecting this with a mutex, it should be safe enough like this
			if(!result && ps->lpd->pending_debug_output==""){ps->lpd->cancelled=true;}else{ps->lpd->finished=true;}
			// tell wx to run its idle-events now, just to make the progress window notice earlier that we're done
			//ps->SetTask(wxString::Format("zrobione, rezultat %i \n",result));
			//wxLogStatus("wakeup");
			//wxWakeUpIdle();
		}
		if (result) return (wxThread::ExitCode) 1;
		else return 0;
	}


Automation::Automation(kainoteFrame *kai)
	{
	Kai=kai;
	path=Options.pathfull+"\\Autoload";
	ReloadScripts(true);
	}

Automation::~Automation()
	{
	RemoveAll();
	}

bool Automation::Add(wxString filename)
	{
		
	LuaScript *ls= new LuaScript(filename);
	for (size_t i = 0; i < Scripts.size(); i++) {
		if (ls->name == Scripts[i]->name){delete ls; ls=NULL; return false;}
		}
	Scripts.push_back(ls);
	return true;
	}

void Automation::Remove(int script)
	{
	std::vector<LuaScript*>::iterator i= Scripts.begin()+script;
	delete *i;
	Scripts.erase(i);
	}

void Automation::RemoveAll(bool autoload)
	{
	std::vector<LuaScript*>::iterator eend= (autoload)? Scripts.begin()+ALScripts : Scripts.end();
	for (std::vector<LuaScript*>::iterator i = Scripts.begin(); i != eend; i++) {
			delete *i;
		}
	if(autoload){Scripts.erase(Scripts.begin(),eend);}
	if(!autoload){Scripts.clear();}
	}

void Automation::ReloadMacro(int script)
	{
	Scripts[script]->Reload();
	}

void Automation::ReloadScripts(bool first)
	{
	
	
	if(!first){RemoveAll(true);}
	ALScripts=0;
		int error_count = 0;

		//wxStringTokenizer tok(path, "|", wxTOKEN_STRTOK);
		//while (tok.HasMoreTokens()) {
			wxDir dir;
			//wxString dirname = StandardPaths::DecodePath(tok.GetNextToken());
			if (!dir.Open(path)) {
				//wxLogWarning("Failed to open a directory in the Automation autoload path: %s", dirname.c_str());
				return;
			}
	

			wxString fn;
			wxFileName script_path(path, "");
			bool more = dir.GetFirst(&fn, wxEmptyString, wxDIR_FILES);

			while (more) {
				script_path.SetName(fn);
				try {
					wxString fullpath = script_path.GetFullPath();
					
					if(!fullpath.Lower().EndsWith("lua") || !Add(fullpath)){more = dir.GetNext(&fn);continue;}
						
					if (!Scripts[Scripts.size()-1]->loaded) {error_count++;}
						
						ALScripts++;
					
				}
				catch (const wchar_t *e) {
					error_count++;
					wxLogError(_("Błąd wczytywania skryptu Lua: %s\n%s"), fn.c_str(), e);
				}
				catch (...) {
					error_count++;
					wxLogError(_("Nieznany błąd wczytywania skryptu Lua: %s."), fn.c_str());
				}
				
				more = dir.GetNext(&fn);
			}
		//}wxLogStatus("weszło");
			//wxLogStatus("po pętli");
		if (error_count > 0) {
			wxLogWarning(_("Jeden bądź więcej skryptów autoload zawiera błędy,\n obejrzyj opisy skryptów by uzyskać więcej informacji."));
		}
		
	
	}

};